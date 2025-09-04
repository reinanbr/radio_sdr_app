#ifndef SDR_RADIO_H
#define SDR_RADIO_H

#include <jni.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <queue>
#include <vector>

// RTL-SDR includes
extern "C" {
#include "rtl-sdr.h"
}

#define LOG_TAG "SDRRadio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Estrutura para dados de áudio
struct AudioData {
    std::vector<float> samples;
    double frequency;
    double signal_strength;
    long timestamp;
};

// Estrutura para configuração do SDR
struct SDRConfig {
    uint32_t sample_rate;
    uint32_t center_freq;
    int gain;
    int ppm_error;
    bool auto_gain;
};

class SDRRadio {
private:
    rtlsdr_dev_t* device;
    SDRConfig config;
    bool is_connected;
    bool is_scanning;
    bool is_recording;
    
    // Audio engine
    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    SLObjectItf outputMixObject;
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    
    // Threads
    pthread_t scan_thread;
    pthread_t audio_thread;
    pthread_mutex_t data_mutex;
    
    // Data buffers
    std::queue<AudioData> audio_queue;
    std::vector<float> frequency_spectrum;
    
    // Callbacks
    jobject java_callback;
    JavaVM* jvm;
    
    // Static callback functions
    static void rtlsdr_callback(unsigned char* buf, uint32_t len, void* ctx);
    static void* scan_thread_func(void* arg);
    static void* audio_thread_func(void* arg);
    
    // Audio processing
    void process_audio_data(unsigned char* buf, uint32_t len);
    void update_frequency_spectrum();
    void save_audio_data(const AudioData& data);
    
public:
    SDRRadio();
    ~SDRRadio();
    
    // Device management
    bool connect_device();
    void disconnect_device();
    bool is_device_connected() const { return is_connected; }
    
    // Configuration
    bool set_sample_rate(uint32_t rate);
    bool set_center_frequency(uint32_t freq);
    bool set_gain(int gain);
    bool set_auto_gain(bool enabled);
    bool set_ppm_error(int ppm);
    
    // Scanning and recording
    bool start_scanning();
    void stop_scanning();
    bool start_recording();
    void stop_recording();
    
    // Data access
    std::vector<float> get_frequency_spectrum();
    double get_current_frequency();
    double get_signal_strength();
    
    // Java interface
    void set_java_callback(JNIEnv* env, jobject callback);
    void notify_java_callback(const char* event, const char* data);
};

// JNI functions
extern "C" {
    JNIEXPORT jlong JNICALL Java_com_sdrradio_kt_SDRRadio_nativeInit(JNIEnv* env, jobject thiz);
    JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeDestroy(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeConnect(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeDisconnect(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartScanning(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStopScanning(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartRecording(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStopRecording(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jdouble JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetFrequency(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jdouble JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetSignalStrength(JNIEnv* env, jobject thiz, jlong ptr);
    JNIEXPORT jfloatArray JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetSpectrum(JNIEnv* env, jobject thiz, jlong ptr);
}

#endif // SDR_RADIO_H 