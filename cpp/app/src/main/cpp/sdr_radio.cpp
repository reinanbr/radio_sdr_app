#include "sdr_radio.h"
#include "sdr_manager.h"
#include "audio_manager.h"
#include "usb_manager.h"
#include <jni.h>
#include <android/log.h>

// Implementação da classe SDRRadio
SDRRadio::SDRRadio() 
    : callback_(nullptr)
    , running_(false)
    , deviceConnected_(false)
    , radioRunning_(false)
    , currentFrequency_(100.0)
    , currentSampleRate_(2048000)
    , currentGain_(20.0)
    , currentAutoGain_(true) {
    
    LOGI("SDRRadio constructor called");
}

SDRRadio::~SDRRadio() {
    LOGI("SDRRadio destructor called");
    shutdown();
}

bool SDRRadio::initialize() {
    LOGI("Initializing SDRRadio");
    
    try {
        // Inicializar gerenciadores
        sdrManager_ = std::make_unique<SDRManager>();
        audioManager_ = std::make_unique<AudioManager>();
        usbManager_ = std::make_unique<USBManager>();
        
        if (!sdrManager_->initialize()) {
            LOGE("Failed to initialize SDRManager");
            return false;
        }
        
        if (!audioManager_->initialize()) {
            LOGE("Failed to initialize AudioManager");
            return false;
        }
        
        if (!usbManager_->initialize()) {
            LOGE("Failed to initialize USBManager");
            return false;
        }
        
        // Configurar callbacks
        sdrManager_->setDataCallback([this](const std::vector<uint8_t>& data) {
            // Processar dados do SDR e enviar para áudio
            if (radioRunning_) {
                // Converter dados I/Q para áudio
                std::vector<float> audioData = convertIQToAudio(data);
                audioManager_->writeAudioData(audioData);
            }
        });
        
        sdrManager_->setErrorCallback([this](const std::string& error) {
            if (callback_) {
                callback_->onError(error);
            }
        });
        
        audioManager_->setErrorCallback([this](const std::string& error) {
            if (callback_) {
                callback_->onError(error);
            }
        });
        
        usbManager_->setDeviceConnectedCallback([this](const USBManager::DeviceInfo& device) {
            deviceConnected_ = true;
            if (callback_) {
                callback_->onDeviceConnected();
            }
        });
        
        usbManager_->setDeviceDisconnectedCallback([this](const USBManager::DeviceInfo& device) {
            deviceConnected_ = false;
            radioRunning_ = false;
            if (callback_) {
                callback_->onDeviceDisconnected();
            }
        });
        
        usbManager_->setErrorCallback([this](const std::string& error) {
            if (callback_) {
                callback_->onError(error);
            }
        });
        
        // Iniciar thread de processamento
        running_ = true;
        processThread_ = std::thread(&SDRRadio::processLoop, this);
        
        LOGI("SDRRadio initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOGE("Exception during initialization: %s", e.what());
        return false;
    }
}

void SDRRadio::shutdown() {
    LOGI("Shutting down SDRRadio");
    
    running_ = false;
    radioRunning_ = false;
    
    if (processThread_.joinable()) {
        processThread_.join();
    }
    
    if (sdrManager_) {
        sdrManager_->shutdown();
    }
    
    if (audioManager_) {
        audioManager_->shutdown();
    }
    
    if (usbManager_) {
        usbManager_->shutdown();
    }
    
    LOGI("SDRRadio shutdown complete");
}

bool SDRRadio::connectDevice() {
    LOGI("Connecting to RTL-SDR device");
    
    if (!usbManager_) {
        LOGE("USBManager not initialized");
        return false;
    }
    
    // Tentar conectar ao primeiro dispositivo RTL-SDR disponível
    if (usbManager_->openDeviceByVendorProduct(0x0bda, 0x2838)) {
        LOGI("Connected to RTL-SDR v3");
        return true;
    }
    
    if (usbManager_->openDeviceByVendorProduct(0x0bda, 0x2832)) {
        LOGI("Connected to RTL-SDR v4");
        return true;
    }
    
    // Tentar conectar ao primeiro dispositivo disponível
    if (usbManager_->openDevice(0)) {
        LOGI("Connected to generic RTL-SDR device");
        return true;
    }
    
    LOGE("No RTL-SDR device found");
    return false;
}

void SDRRadio::disconnectDevice() {
    LOGI("Disconnecting RTL-SDR device");
    
    if (radioRunning_) {
        stopRadio();
    }
    
    if (usbManager_) {
        usbManager_->closeDevice();
    }
    
    deviceConnected_ = false;
}

bool SDRRadio::isDeviceConnected() const {
    return deviceConnected_;
}

bool SDRRadio::startRadio(double frequency, int sampleRate, float gain, bool autoGain) {
    LOGI("Starting radio: freq=%.1f MHz, sampleRate=%d, gain=%.1f, autoGain=%s", 
         frequency, sampleRate, gain, autoGain ? "true" : "false");
    
    if (!deviceConnected_) {
        LOGE("Device not connected");
        return false;
    }
    
    try {
        // Configurar SDR
        if (!sdrManager_->setFrequency(frequency)) {
            LOGE("Failed to set frequency");
            return false;
        }
        
        if (!sdrManager_->setSampleRate(sampleRate)) {
            LOGE("Failed to set sample rate");
            return false;
        }
        
        if (autoGain) {
            if (!sdrManager_->setAutoGain(true)) {
                LOGE("Failed to set auto gain");
                return false;
            }
        } else {
            if (!sdrManager_->setGain(gain)) {
                LOGE("Failed to set gain");
                return false;
            }
        }
        
        // Iniciar streaming do SDR
        if (!sdrManager_->startStreaming()) {
            LOGE("Failed to start SDR streaming");
            return false;
        }
        
        // Iniciar áudio
        if (!audioManager_->startAudio(44100, 1)) {
            LOGE("Failed to start audio");
            sdrManager_->stopStreaming();
            return false;
        }
        
        // Atualizar configurações
        currentFrequency_ = frequency;
        currentSampleRate_ = sampleRate;
        currentGain_ = gain;
        currentAutoGain_ = autoGain;
        
        radioRunning_ = true;
        
        if (callback_) {
            callback_->onRadioStarted();
        }
        
        LOGI("Radio started successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOGE("Exception while starting radio: %s", e.what());
        return false;
    }
}

void SDRRadio::stopRadio() {
    LOGI("Stopping radio");
    
    radioRunning_ = false;
    
    if (sdrManager_) {
        sdrManager_->stopStreaming();
    }
    
    if (audioManager_) {
        audioManager_->stopAudio();
    }
    
    if (callback_) {
        callback_->onRadioStopped();
    }
    
    LOGI("Radio stopped");
}

bool SDRRadio::isRadioRunning() const {
    return radioRunning_;
}

void SDRRadio::setFrequency(double frequency) {
    currentFrequency_ = frequency;
    if (radioRunning_ && sdrManager_) {
        sdrManager_->setFrequency(frequency);
    }
}

void SDRRadio::setSampleRate(int sampleRate) {
    currentSampleRate_ = sampleRate;
    if (radioRunning_ && sdrManager_) {
        sdrManager_->setSampleRate(sampleRate);
    }
}

void SDRRadio::setGain(float gain) {
    currentGain_ = gain;
    if (radioRunning_ && sdrManager_) {
        sdrManager_->setGain(gain);
    }
}

void SDRRadio::setAutoGain(bool autoGain) {
    currentAutoGain_ = autoGain;
    if (radioRunning_ && sdrManager_) {
        sdrManager_->setAutoGain(autoGain);
    }
}

void SDRRadio::setCallback(SDRCallback* callback) {
    callback_ = callback;
}

void SDRRadio::processLoop() {
    LOGI("SDRRadio process loop started");
    
    while (running_) {
        // Processar eventos e atualizar status
        if (radioRunning_) {
            // Calcular força do sinal (simulado por enquanto)
            float signalStrength = calculateSignalStrength();
            if (callback_) {
                callback_->onSignalStrengthChanged(signalStrength);
            }
        }
        
        // Aguardar um pouco antes da próxima iteração
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    LOGI("SDRRadio process loop ended");
}

// Função auxiliar para converter dados I/Q para áudio
std::vector<float> SDRRadio::convertIQToAudio(const std::vector<uint8_t>& iqData) {
    std::vector<float> audioData;
    
    // Converter dados I/Q (8-bit) para float
    for (size_t i = 0; i < iqData.size(); i += 2) {
        if (i + 1 < iqData.size()) {
            // Converter I e Q para float (-1.0 a 1.0)
            float i_val = (iqData[i] - 128.0f) / 128.0f;
            float q_val = (iqData[i + 1] - 128.0f) / 128.0f;
            
            // Aplicar demodulação AM simples (magnitude)
            float magnitude = std::sqrt(i_val * i_val + q_val * q_val);
            audioData.push_back(magnitude);
        }
    }
    
    return audioData;
}

// Função auxiliar para calcular força do sinal
float SDRRadio::calculateSignalStrength() {
    // Simulação simples - em um implementação real, isso seria calculado
    // baseado nos dados I/Q recebidos
    static float strength = 0.0f;
    strength += 0.1f;
    if (strength > 100.0f) strength = 0.0f;
    return strength;
}

// Implementação das funções JNI
extern "C" {

JNIEXPORT jlong JNICALL Java_com_example_sdrradio_SDRRadio_nativeInit(JNIEnv* env, jobject thiz) {
    auto* sdrRadio = new SDRRadio();
    if (!sdrRadio->initialize()) {
        delete sdrRadio;
        return 0;
    }
    return reinterpret_cast<jlong>(sdrRadio);
}

JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeDestroy(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (sdrRadio) {
        delete sdrRadio;
    }
}

JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeConnectDevice(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (!sdrRadio) return JNI_FALSE;
    return sdrRadio->connectDevice() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeDisconnectDevice(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (sdrRadio) {
        sdrRadio->disconnectDevice();
    }
}

JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeStartRadio(JNIEnv* env, jobject thiz, jlong nativePtr, jdouble frequency, jint sampleRate, jfloat gain, jboolean autoGain) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (!sdrRadio) return JNI_FALSE;
    return sdrRadio->startRadio(frequency, sampleRate, gain, autoGain) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeStopRadio(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (sdrRadio) {
        sdrRadio->stopRadio();
    }
}

JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeIsDeviceConnected(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (!sdrRadio) return JNI_FALSE;
    return sdrRadio->isDeviceConnected() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeIsRadioRunning(JNIEnv* env, jobject thiz, jlong nativePtr) {
    auto* sdrRadio = reinterpret_cast<SDRRadio*>(nativePtr);
    if (!sdrRadio) return JNI_FALSE;
    return sdrRadio->isRadioRunning() ? JNI_TRUE : JNI_FALSE;
}

} // extern "C" 