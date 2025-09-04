#ifndef SDR_RADIO_H
#define SDR_RADIO_H

#include <jni.h>
#include <android/log.h>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// Definições de log
#define LOG_TAG "SDRRadio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Forward declarations
class SDRManager;
class AudioManager;
class USBManager;

// Callback interface para notificações do Java
class SDRCallback {
public:
    virtual ~SDRCallback() = default;
    virtual void onDeviceConnected() = 0;
    virtual void onDeviceDisconnected() = 0;
    virtual void onRadioStarted() = 0;
    virtual void onRadioStopped() = 0;
    virtual void onSignalStrengthChanged(float strength) = 0;
    virtual void onError(const std::string& error) = 0;
};

// Classe principal do SDR Radio
class SDRRadio {
public:
    SDRRadio();
    ~SDRRadio();

    // Inicialização e finalização
    bool initialize();
    void shutdown();

    // Controle do dispositivo
    bool connectDevice();
    void disconnectDevice();
    bool isDeviceConnected() const;

    // Controle do rádio
    bool startRadio(double frequency, int sampleRate, float gain, bool autoGain);
    void stopRadio();
    bool isRadioRunning() const;

    // Configurações
    void setFrequency(double frequency);
    void setSampleRate(int sampleRate);
    void setGain(float gain);
    void setAutoGain(bool autoGain);

    // Callbacks
    void setCallback(SDRCallback* callback);

    // Thread de processamento
    void processLoop();

private:
    std::unique_ptr<SDRManager> sdrManager_;
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<USBManager> usbManager_;
    
    SDRCallback* callback_;
    
    std::thread processThread_;
    std::atomic<bool> running_;
    std::atomic<bool> deviceConnected_;
    std::atomic<bool> radioRunning_;
    
    std::mutex mutex_;
    std::condition_variable condition_;
    
    // Configurações atuais
    double currentFrequency_;
    int currentSampleRate_;
    float currentGain_;
    bool currentAutoGain_;
};

// Funções JNI
extern "C" {
    JNIEXPORT jlong JNICALL Java_com_example_sdrradio_SDRRadio_nativeInit(JNIEnv* env, jobject thiz);
    JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeDestroy(JNIEnv* env, jobject thiz, jlong nativePtr);
    JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeConnectDevice(JNIEnv* env, jobject thiz, jlong nativePtr);
    JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeDisconnectDevice(JNIEnv* env, jobject thiz, jlong nativePtr);
    JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeStartRadio(JNIEnv* env, jobject thiz, jlong nativePtr, jdouble frequency, jint sampleRate, jfloat gain, jboolean autoGain);
    JNIEXPORT void JNICALL Java_com_example_sdrradio_SDRRadio_nativeStopRadio(JNIEnv* env, jobject thiz, jlong nativePtr);
    JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeIsDeviceConnected(JNIEnv* env, jobject thiz, jlong nativePtr);
    JNIEXPORT jboolean JNICALL Java_com_example_sdrradio_SDRRadio_nativeIsRadioRunning(JNIEnv* env, jobject thiz, jlong nativePtr);
}

#endif // SDR_RADIO_H 