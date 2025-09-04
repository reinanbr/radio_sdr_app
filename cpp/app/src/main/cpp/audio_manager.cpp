#include "audio_manager.h"
#include <android/log.h>
#include <algorithm>
#include <cmath>

// Definições de log
#define LOG_TAG "AudioManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

AudioManager::AudioManager()
    : engineObject_(nullptr)
    , engineEngine_(nullptr)
    , outputMixObject_(nullptr)
    , playerObject_(nullptr)
    , playerPlay_(nullptr)
    , playerBufferQueue_(nullptr)
    , playerVolume_(nullptr)
    , initialized_(false)
    , playing_(false)
    , running_(false)
    , sampleRate_(44100)
    , channels_(1)
    , volume_(1.0f) {
    
    LOGI("AudioManager constructor called");
}

AudioManager::~AudioManager() {
    LOGI("AudioManager destructor called");
    shutdown();
}

bool AudioManager::initialize() {
    LOGI("Initializing AudioManager");
    
    if (initialized_) {
        LOGW("AudioManager already initialized");
        return true;
    }
    
    if (!initializeOpenSL()) {
        LOGE("Failed to initialize OpenSL ES");
        return false;
    }
    
    initialized_ = true;
    running_ = true;
    
    // Iniciar thread de processamento
    processThread_ = std::thread(&AudioManager::processAudioQueue, this);
    
    LOGI("AudioManager initialized successfully");
    return true;
}

void AudioManager::shutdown() {
    LOGI("Shutting down AudioManager");
    
    running_ = false;
    playing_ = false;
    
    if (processThread_.joinable()) {
        processCondition_.notify_all();
        processThread_.join();
    }
    
    destroyOpenSL();
    clearAudioQueue();
    
    initialized_ = false;
    LOGI("AudioManager shutdown complete");
}

bool AudioManager::startAudio(int sampleRate, int channels) {
    LOGI("Starting audio: sampleRate=%d, channels=%d", sampleRate, channels);
    
    if (!initialized_) {
        LOGE("AudioManager not initialized");
        return false;
    }
    
    if (playing_) {
        LOGW("Audio already playing");
        return true;
    }
    
    sampleRate_ = sampleRate;
    channels_ = channels;
    
    // Configurar volume
    if (playerVolume_) {
        SLmillibel volume = static_cast<SLmillibel>(volume_ * 1000);
        (*playerVolume_)->SetVolumeLevel(playerVolume_, volume);
    }
    
    // Iniciar reprodução
    if (playerPlay_) {
        SLresult result = (*playerPlay_)->SetPlayState(playerPlay_, SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("Failed to start audio playback: %d", result);
            return false;
        }
    }
    
    playing_ = true;
    LOGI("Audio started successfully");
    return true;
}

void AudioManager::stopAudio() {
    LOGI("Stopping audio");
    
    if (playerPlay_) {
        (*playerPlay_)->SetPlayState(playerPlay_, SL_PLAYSTATE_STOPPED);
    }
    
    playing_ = false;
    clearAudioQueue();
    
    LOGI("Audio stopped");
}

bool AudioManager::isAudioPlaying() const {
    return playing_;
}

bool AudioManager::writeAudioData(const std::vector<float>& audioData) {
    if (!playing_) {
        return false;
    }
    
    // Converter float para int16_t
    std::vector<int16_t> int16Data;
    int16Data.reserve(audioData.size());
    
    for (float sample : audioData) {
        // Clamp entre -1.0 e 1.0
        sample = std::max(-1.0f, std::min(1.0f, sample));
        
        // Converter para int16_t
        int16_t intSample = static_cast<int16_t>(sample * 32767.0f);
        int16Data.push_back(intSample);
    }
    
    return writeAudioData(int16Data);
}

bool AudioManager::writeAudioData(const std::vector<int16_t>& audioData) {
    if (!playing_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(audioQueueMutex_);
    
    // Adicionar dados à fila
    audioQueue_.push(audioData);
    
    // Notificar thread de processamento
    processCondition_.notify_one();
    
    return true;
}

void AudioManager::setVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
    
    if (playerVolume_) {
        SLmillibel volumeDb = static_cast<SLmillibel>(volume_ * 1000);
        (*playerVolume_)->SetVolumeLevel(playerVolume_, volumeDb);
    }
}

void AudioManager::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void AudioManager::setChannels(int channels) {
    channels_ = channels;
}

void AudioManager::setAudioFinishedCallback(AudioFinishedCallback callback) {
    audioFinishedCallback_ = callback;
}

void AudioManager::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

int AudioManager::getSampleRate() const {
    return sampleRate_;
}

int AudioManager::getChannels() const {
    return channels_;
}

float AudioManager::getVolume() const {
    return volume_;
}

bool AudioManager::initializeOpenSL() {
    LOGI("Initializing OpenSL ES");
    
    // Criar engine
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to create OpenSL engine: %d", result);
        return false;
    }
    
    // Realizar engine
    result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to realize OpenSL engine: %d", result);
        return false;
    }
    
    // Obter interface do engine
    result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to get engine interface: %d", result);
        return false;
    }
    
    // Criar output mix
    result = (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to create output mix: %d", result);
        return false;
    }
    
    // Realizar output mix
    result = (*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to realize output mix: %d", result);
        return false;
    }
    
    // Configurar player de áudio
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
    };
    
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        static_cast<SLuint32>(channels_),
        static_cast<SLuint32>(sampleRate_ * 1000), // Hz to mHz
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        channels_ == 1 ? (SLuint32)SL_SPEAKER_FRONT_CENTER : 
                        (SLuint32)(SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT),
        SL_BYTEORDER_LITTLEENDIAN
    };
    
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    
    SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX, outputMixObject_
    };
    
    SLDataSink audioSnk = {&loc_outmix, nullptr};
    
    const SLInterfaceID ids[3] = {
        SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAY
    };
    
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    
    // Criar player
    result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &playerObject_, &audioSrc, &audioSnk, 3, ids, req);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to create audio player: %d", result);
        return false;
    }
    
    // Realizar player
    result = (*playerObject_)->Realize(playerObject_, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to realize audio player: %d", result);
        return false;
    }
    
    // Obter interface de reprodução
    result = (*playerObject_)->GetInterface(playerObject_, SL_IID_PLAY, &playerPlay_);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to get play interface: %d", result);
        return false;
    }
    
    // Obter interface de buffer queue
    result = (*playerObject_)->GetInterface(playerObject_, SL_IID_BUFFERQUEUE, &playerBufferQueue_);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to get buffer queue interface: %d", result);
        return false;
    }
    
    // Obter interface de volume
    result = (*playerObject_)->GetInterface(playerObject_, SL_IID_VOLUME, &playerVolume_);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to get volume interface: %d", result);
        return false;
    }
    
    // Registrar callback do buffer queue
    result = (*playerBufferQueue_)->RegisterCallback(playerBufferQueue_, bufferQueueCallback, this);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to register buffer queue callback: %d", result);
        return false;
    }
    
    // Registrar callback de reprodução
    result = (*playerPlay_)->RegisterCallback(playerPlay_, playCallback, this);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Failed to register play callback: %d", result);
        return false;
    }
    
    LOGI("OpenSL ES initialized successfully");
    return true;
}

void AudioManager::destroyOpenSL() {
    LOGI("Destroying OpenSL ES");
    
    if (playerObject_) {
        (*playerObject_)->Destroy(playerObject_);
        playerObject_ = nullptr;
        playerPlay_ = nullptr;
        playerBufferQueue_ = nullptr;
        playerVolume_ = nullptr;
    }
    
    if (outputMixObject_) {
        (*outputMixObject_)->Destroy(outputMixObject_);
        outputMixObject_ = nullptr;
    }
    
    if (engineObject_) {
        (*engineObject_)->Destroy(engineObject_);
        engineObject_ = nullptr;
        engineEngine_ = nullptr;
    }
    
    LOGI("OpenSL ES destroyed");
}

void AudioManager::processAudioQueue() {
    LOGI("Audio processing thread started");
    
    while (running_) {
        std::unique_lock<std::mutex> lock(processMutex_);
        processCondition_.wait(lock, [this] { return !running_ || !audioQueue_.empty(); });
        
        if (!running_) {
            break;
        }
        
        // Processar dados de áudio da fila
        std::lock_guard<std::mutex> queueLock(audioQueueMutex_);
        
        while (!audioQueue_.empty() && playing_) {
            currentBuffer_ = audioQueue_.front();
            audioQueue_.pop();
            
            // Enviar buffer para OpenSL ES
            if (playerBufferQueue_ && !currentBuffer_.empty()) {
                SLresult result = (*playerBufferQueue_)->Enqueue(playerBufferQueue_, 
                                                               currentBuffer_.data(), 
                                                               currentBuffer_.size() * sizeof(int16_t));
                if (result != SL_RESULT_SUCCESS) {
                    LOGE("Failed to enqueue audio buffer: %d", result);
                    if (errorCallback_) {
                        errorCallback_("Failed to enqueue audio buffer");
                    }
                }
            }
        }
    }
    
    LOGI("Audio processing thread ended");
}

void AudioManager::clearAudioQueue() {
    std::lock_guard<std::mutex> lock(audioQueueMutex_);
    while (!audioQueue_.empty()) {
        audioQueue_.pop();
    }
    currentBuffer_.clear();
}

void AudioManager::bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller, void* context) {
    auto* manager = static_cast<AudioManager*>(context);
    if (manager) {
        // Buffer foi processado, podemos enviar mais dados
        manager->processCondition_.notify_one();
    }
}

void AudioManager::playCallback(SLPlayItf caller, void* context, SLuint32 event) {
    auto* manager = static_cast<AudioManager*>(context);
    if (manager && event == SL_PLAYEVENT_HEADATEND) {
        // Reprodução terminou
        if (manager->audioFinishedCallback_) {
            manager->audioFinishedCallback_();
        }
    }
} 