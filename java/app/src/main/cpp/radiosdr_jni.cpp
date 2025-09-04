#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <memory>

#include "sdr_controller.h"
#include "signal_processor.h"
#include "audio_processor.h"
#include "spectrum_analyzer.h"

#define LOG_TAG "RadioSDR_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Global instances
static std::unique_ptr<SDRController> sdrController;
static std::unique_ptr<SignalProcessor> signalProcessor;
static std::unique_ptr<AudioProcessor> audioProcessor;
static std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer;

static std::atomic<bool> isRunning{false};
static std::thread processingThread;

// Processing loop function
void processingLoop() {
    LOGI("Processing loop started");
    
    while (isRunning.load()) {
        if (sdrController && sdrController->isDeviceOpen()) {
            // Read IQ samples from RTL-SDR
            std::vector<std::complex<float>> samples;
            if (sdrController->readSamples(samples)) {
                // Process samples through signal processor
                if (signalProcessor) {
                    signalProcessor->processSamples(samples);
                    
                    // Update spectrum analyzer
                    if (spectrumAnalyzer) {
                        spectrumAnalyzer->updateSpectrum(samples);
                    }
                    
                    // Demodulate and send to audio processor
                    if (audioProcessor) {
                        std::vector<float> audioSamples = signalProcessor->getAudioSamples();
                        audioProcessor->processAudio(audioSamples);
                    }
                }
            }
        }
        
        // Small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LOGI("Processing loop ended");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_initRTLSDR(JNIEnv *env, jobject thiz, jint fd) {
    LOGI("Initializing RTL-SDR with file descriptor: %d", fd);
    
    try {
        sdrController = std::make_unique<SDRController>();
        signalProcessor = std::make_unique<SignalProcessor>();
        audioProcessor = std::make_unique<AudioProcessor>();
        spectrumAnalyzer = std::make_unique<SpectrumAnalyzer>();
        
        if (sdrController->initDevice(fd)) {
            LOGI("RTL-SDR device initialized successfully");
            return JNI_TRUE;
        } else {
            LOGE("Failed to initialize RTL-SDR device");
            return JNI_FALSE;
        }
    } catch (const std::exception& e) {
        LOGE("Exception during RTL-SDR initialization: %s", e.what());
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_radioSDR_app_MainActivity_closeRTLSDR(JNIEnv *env, jobject thiz) {
    LOGI("Closing RTL-SDR");
    
    // Stop processing if running
    if (isRunning.load()) {
        isRunning.store(false);
        if (processingThread.joinable()) {
            processingThread.join();
        }
    }
    
    // Clean up objects
    sdrController.reset();
    signalProcessor.reset();
    audioProcessor.reset();
    spectrumAnalyzer.reset();
    
    LOGI("RTL-SDR closed");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_setFrequency(JNIEnv *env, jobject thiz, jdouble freq) {
    if (sdrController) {
        uint32_t freq_hz = static_cast<uint32_t>(freq * 1e6); // Convert MHz to Hz
        bool result = sdrController->setFrequency(freq_hz);
        LOGI("Set frequency to %.3f MHz: %s", freq, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_setGain(JNIEnv *env, jobject thiz, jint gain) {
    if (sdrController) {
        bool result = sdrController->setGain(gain); // gain in 0.1 dB units
        LOGI("Set gain to %.1f dB: %s", gain / 10.0f, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_setAutoGain(JNIEnv *env, jobject thiz, jboolean enable) {
    if (sdrController) {
        bool result = sdrController->setAutoGain(enable == JNI_TRUE);
        LOGI("Set auto gain %s: %s", enable ? "enabled" : "disabled", result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_setSampleRate(JNIEnv *env, jobject thiz, jint rate) {
    if (sdrController) {
        bool result = sdrController->setSampleRate(rate);
        LOGI("Set sample rate to %d Hz: %s", rate, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_MainActivity_startReading(JNIEnv *env, jobject thiz) {
    if (sdrController && !isRunning.load()) {
        if (sdrController->startReading()) {
            isRunning.store(true);
            processingThread = std::thread(processingLoop);
            LOGI("Started SDR reading and processing");
            return JNI_TRUE;
        } else {
            LOGE("Failed to start SDR reading");
            return JNI_FALSE;
        }
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_radioSDR_app_MainActivity_stopReading(JNIEnv *env, jobject thiz) {
    if (isRunning.load()) {
        isRunning.store(false);
        if (processingThread.joinable()) {
            processingThread.join();
        }
        
        if (sdrController) {
            sdrController->stopReading();
        }
        
        LOGI("Stopped SDR reading and processing");
    }
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_radioSDR_app_MainActivity_getSpectrumData(JNIEnv *env, jobject thiz) {
    if (spectrumAnalyzer) {
        std::vector<float> spectrum = spectrumAnalyzer->getSpectrum();
        if (!spectrum.empty()) {
            jfloatArray result = env->NewFloatArray(spectrum.size());
            env->SetFloatArrayRegion(result, 0, spectrum.size(), spectrum.data());
            return result;
        }
    }
    return nullptr;
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_radioSDR_app_MainActivity_getAudioData(JNIEnv *env, jobject thiz) {
    if (audioProcessor) {
        std::vector<int16_t> audio = audioProcessor->getAudioBuffer();
        if (!audio.empty()) {
            jshortArray result = env->NewShortArray(audio.size());
            env->SetShortArrayRegion(result, 0, audio.size(), audio.data());
            return result;
        }
    }
    return nullptr;
}

// SpectrumActivity native methods
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_radioSDR_app_SpectrumActivity_getSpectrumData(JNIEnv *env, jobject thiz) {
    return Java_com_radioSDR_app_MainActivity_getSpectrumData(env, thiz);
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_radioSDR_app_SpectrumActivity_getWaterfallData(JNIEnv *env, jobject thiz) {
    if (spectrumAnalyzer) {
        std::vector<float> waterfall = spectrumAnalyzer->getWaterfall();
        if (!waterfall.empty()) {
            jfloatArray result = env->NewFloatArray(waterfall.size());
            env->SetFloatArrayRegion(result, 0, waterfall.size(), waterfall.data());
            return result;
        }
    }
    return nullptr;
}

// SettingsActivity native methods
extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_SettingsActivity_setSampleRate(JNIEnv *env, jobject thiz, jint rate) {
    return Java_com_radioSDR_app_MainActivity_setSampleRate(env, thiz, rate);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_SettingsActivity_setFrequencyCorrection(JNIEnv *env, jobject thiz, jint ppm) {
    if (sdrController) {
        bool result = sdrController->setFrequencyCorrection(ppm);
        LOGI("Set frequency correction to %d PPM: %s", ppm, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_SettingsActivity_setBandwidth(JNIEnv *env, jobject thiz, jint bandwidth) {
    if (signalProcessor) {
        bool result = signalProcessor->setBandwidth(bandwidth);
        LOGI("Set bandwidth to %d Hz: %s", bandwidth, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_radioSDR_app_SettingsActivity_setSquelch(JNIEnv *env, jobject thiz, jint squelch) {
    if (signalProcessor) {
        bool result = signalProcessor->setSquelch(squelch);
        LOGI("Set squelch to %d dB: %s", squelch, result ? "success" : "failed");
        return result ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}
