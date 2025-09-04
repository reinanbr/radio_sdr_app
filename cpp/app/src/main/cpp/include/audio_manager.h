#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <queue>
#include <functional>

class AudioManager {
public:
    // Callback para quando o áudio termina de tocar
    using AudioFinishedCallback = std::function<void()>;
    
    AudioManager();
    ~AudioManager();

    // Inicialização e finalização
    bool initialize();
    void shutdown();

    // Controle de áudio
    bool startAudio(int sampleRate = 44100, int channels = 1);
    void stopAudio();
    bool isAudioPlaying() const;

    // Envio de dados de áudio
    bool writeAudioData(const std::vector<float>& audioData);
    bool writeAudioData(const std::vector<int16_t>& audioData);

    // Configurações de áudio
    void setVolume(float volume); // 0.0 a 1.0
    void setSampleRate(int sampleRate);
    void setChannels(int channels);

    // Callbacks
    void setAudioFinishedCallback(AudioFinishedCallback callback);
    void setErrorCallback(std::function<void(const std::string&)> callback);

    // Informações
    int getSampleRate() const;
    int getChannels() const;
    float getVolume() const;

private:
    // Callbacks do OpenSL ES
    static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller, void* context);
    static void playCallback(SLPlayItf caller, void* context, SLuint32 event);

    // Inicialização do OpenSL ES
    bool initializeOpenSL();
    void destroyOpenSL();

    // Gerenciamento de buffers
    void processAudioQueue();
    void clearAudioQueue();

    // Engine e objetos OpenSL ES
    SLObjectItf engineObject_;
    SLEngineItf engineEngine_;
    SLObjectItf outputMixObject_;
    SLObjectItf playerObject_;
    SLPlayItf playerPlay_;
    SLAndroidSimpleBufferQueueItf playerBufferQueue_;
    SLVolumeItf playerVolume_;

    // Estado
    std::atomic<bool> initialized_;
    std::atomic<bool> playing_;
    std::atomic<bool> running_;
    
    // Configurações
    int sampleRate_;
    int channels_;
    float volume_;
    
    // Buffers de áudio
    std::queue<std::vector<int16_t>> audioQueue_;
    std::mutex audioQueueMutex_;
    std::vector<int16_t> currentBuffer_;
    
    // Callbacks
    AudioFinishedCallback audioFinishedCallback_;
    std::function<void(const std::string&)> errorCallback_;
    
    // Thread de processamento
    std::thread processThread_;
    std::mutex processMutex_;
    std::condition_variable processCondition_;
};

#endif // AUDIO_MANAGER_H 