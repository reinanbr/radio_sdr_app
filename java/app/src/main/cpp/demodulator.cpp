#include "demodulator.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>

#define LOG_TAG "Demodulator"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Demodulator::Demodulator()
    : type_(DemodulationType::FM)
    , last_sample_(0, 0)
    , decimation_counter_(0) {
    
    LOGI("Demodulator initialized");
}

Demodulator::~Demodulator() {
    LOGI("Demodulator destroyed");
}

void Demodulator::setType(DemodulationType type) {
    type_ = type;
    LOGD("Demodulation type set to %d", static_cast<int>(type));
}

std::vector<float> Demodulator::demodulate(const std::vector<std::complex<float>>& samples) {
    switch (type_) {
        case DemodulationType::AM:
            return demodulateAM(samples);
        case DemodulationType::FM:
            return demodulateFM(samples);
        case DemodulationType::USB:
            return demodulateSSB(samples, true);
        case DemodulationType::LSB:
            return demodulateSSB(samples, false);
        default:
            return demodulateFM(samples);
    }
}

std::vector<float> Demodulator::demodulateAM(const std::vector<std::complex<float>>& samples) {
    std::vector<float> audio;
    audio.reserve(samples.size() / DECIMATION_FACTOR + 1);
    
    for (size_t i = 0; i < samples.size(); ++i) {
        if (decimation_counter_++ >= DECIMATION_FACTOR) {
            decimation_counter_ = 0;
            
            // AM demodulation: envelope detection
            float magnitude = std::abs(samples[i]);
            
            // DC removal and scaling
            float audio_sample = (magnitude - 0.5f) * 2.0f;
            
            // Limit output
            audio_sample = std::max(-1.0f, std::min(1.0f, audio_sample));
            
            audio.push_back(audio_sample);
        }
    }
    
    return audio;
}

std::vector<float> Demodulator::demodulateFM(const std::vector<std::complex<float>>& samples) {
    std::vector<float> audio;
    audio.reserve(samples.size() / DECIMATION_FACTOR + 1);
    
    for (size_t i = 0; i < samples.size(); ++i) {
        if (decimation_counter_++ >= DECIMATION_FACTOR) {
            decimation_counter_ = 0;
            
            // FM demodulation: phase difference
            float audio_sample = 0.0f;
            
            if (i > 0) {
                std::complex<float> current = samples[i];
                std::complex<float> previous = (i > 0) ? samples[i-1] : last_sample_;
                
                // Calculate phase difference
                std::complex<float> diff = current * std::conj(previous);
                audio_sample = std::arg(diff) / (2.0f * M_PI);
                
                // Scale and limit
                audio_sample *= 10.0f; // Adjust sensitivity
                audio_sample = std::max(-1.0f, std::min(1.0f, audio_sample));
            }
            
            audio.push_back(audio_sample);
        }
    }
    
    if (!samples.empty()) {
        last_sample_ = samples.back();
    }
    
    return audio;
}

std::vector<float> Demodulator::demodulateSSB(const std::vector<std::complex<float>>& samples, bool upper) {
    std::vector<float> audio;
    audio.reserve(samples.size() / DECIMATION_FACTOR + 1);
    
    for (size_t i = 0; i < samples.size(); ++i) {
        if (decimation_counter_++ >= DECIMATION_FACTOR) {
            decimation_counter_ = 0;
            
            // SSB demodulation: take real part (USB) or imaginary part (LSB)
            float audio_sample;
            if (upper) {
                audio_sample = samples[i].real();
            } else {
                audio_sample = samples[i].imag();
            }
            
            // Scale and limit
            audio_sample *= 2.0f;
            audio_sample = std::max(-1.0f, std::min(1.0f, audio_sample));
            
            audio.push_back(audio_sample);
        }
    }
    
    return audio;
}
