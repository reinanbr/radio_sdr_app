#include "signal_processor.h"
#include <android/log.h>
#include <algorithm>
#include <cmath>
#include <numeric>

#define LOG_TAG "Signal_Processor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SignalProcessor::SignalProcessor()
    : demod_type_(DemodulationType::FM)
    , bandwidth_hz_(200000)  // 200 kHz default
    , squelch_db_(-50)       // -50 dB default
    , squelch_threshold_(0.001f)
    , audio_read_pos_(0)
    , audio_write_pos_(0)
    , agc_gain_(1.0f)
    , agc_target_(0.5f)
    , agc_attack_(0.01f)
    , agc_decay_(0.001f) {
    
    // Initialize demodulator
    demodulator_ = std::make_unique<Demodulator>();
    demodulator_->setType(demod_type_);
    
    // Initialize audio buffer
    audio_buffer_.resize(AUDIO_BUFFER_SIZE, 0.0f);
    
    // Initialize filter
    setBandwidth(bandwidth_hz_);
    
    LOGI("Signal processor initialized");
}

SignalProcessor::~SignalProcessor() {
    LOGI("Signal processor destroyed");
}

void SignalProcessor::processSamples(const std::vector<std::complex<float>>& samples) {
    if (samples.empty()) {
        return;
    }
    
    // Make a copy for processing
    std::vector<std::complex<float>> processed_samples = samples;
    
    // Apply bandpass filter
    applyBandpassFilter(processed_samples);
    
    // Demodulate to audio
    std::vector<float> audio = demodulator_->demodulate(processed_samples);
    
    if (audio.empty()) {
        return;
    }
    
    // Apply AGC
    float power = 0.0f;
    for (float sample : audio) {
        power += sample * sample;
    }
    power = std::sqrt(power / audio.size());
    
    if (power > 0.0f) {
        float target_gain = agc_target_ / power;
        if (target_gain > agc_gain_) {
            agc_gain_ += (target_gain - agc_gain_) * agc_attack_;
        } else {
            agc_gain_ += (target_gain - agc_gain_) * agc_decay_;
        }
        
        // Limit gain
        agc_gain_ = std::max(0.1f, std::min(10.0f, agc_gain_));
        
        for (float& sample : audio) {
            sample *= agc_gain_;
        }
    }
    
    // Apply squelch
    applySquelch(audio);
    
    // Add to audio buffer
    size_t write_pos = audio_write_pos_.load();
    for (float sample : audio) {
        audio_buffer_[write_pos] = sample;
        write_pos = (write_pos + 1) % AUDIO_BUFFER_SIZE;
    }
    audio_write_pos_.store(write_pos);
    
    // If buffer is full, advance read position
    size_t read_pos = audio_read_pos_.load();
    size_t available = (write_pos >= read_pos) ? 
        (write_pos - read_pos) : (AUDIO_BUFFER_SIZE - read_pos + write_pos);
    
    if (available > AUDIO_BUFFER_SIZE / 2) {
        read_pos = (write_pos + AUDIO_BUFFER_SIZE / 2) % AUDIO_BUFFER_SIZE;
        audio_read_pos_.store(read_pos);
    }
}

std::vector<float> SignalProcessor::getAudioSamples() {
    size_t read_pos = audio_read_pos_.load();
    size_t write_pos = audio_write_pos_.load();
    
    size_t available = (write_pos >= read_pos) ? 
        (write_pos - read_pos) : (AUDIO_BUFFER_SIZE - read_pos + write_pos);
    
    if (available == 0) {
        return {};
    }
    
    std::vector<float> result;
    result.reserve(available);
    
    for (size_t i = 0; i < available; ++i) {
        result.push_back(audio_buffer_[read_pos]);
        read_pos = (read_pos + 1) % AUDIO_BUFFER_SIZE;
    }
    
    audio_read_pos_.store(read_pos);
    return result;
}

bool SignalProcessor::setBandwidth(int bandwidth_hz) {
    bandwidth_hz_ = bandwidth_hz;
    
    // Design a simple low-pass filter
    int filter_length = 64;
    float cutoff = static_cast<float>(bandwidth_hz) / 2048000.0f; // Assuming 2.048 MHz sample rate
    
    filter_taps_.clear();
    filter_taps_.resize(filter_length);
    
    // Hamming window with sinc function
    for (int i = 0; i < filter_length; ++i) {
        float n = i - (filter_length - 1) / 2.0f;
        float hamming = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (filter_length - 1));
        
        if (n == 0) {
            filter_taps_[i] = 2.0f * cutoff * hamming;
        } else {
            filter_taps_[i] = std::sin(2.0f * M_PI * cutoff * n) / (M_PI * n) * hamming;
        }
    }
    
    // Normalize
    float sum = std::accumulate(filter_taps_.begin(), filter_taps_.end(), 0.0f);
    for (float& tap : filter_taps_) {
        tap /= sum;
    }
    
    // Initialize delay line
    filter_delay_line_.clear();
    filter_delay_line_.resize(filter_length, std::complex<float>(0, 0));
    
    LOGD("Bandwidth set to %d Hz", bandwidth_hz);
    return true;
}

bool SignalProcessor::setSquelch(int squelch_db) {
    squelch_db_ = squelch_db;
    squelch_threshold_ = std::pow(10.0f, squelch_db / 20.0f);
    
    LOGD("Squelch set to %d dB", squelch_db);
    return true;
}

void SignalProcessor::setDemodulationType(DemodulationType type) {
    demod_type_ = type;
    if (demodulator_) {
        demodulator_->setType(type);
    }
    LOGD("Demodulation type set to %d", static_cast<int>(type));
}

void SignalProcessor::applyBandpassFilter(std::vector<std::complex<float>>& samples) {
    if (filter_taps_.empty()) {
        return;
    }
    
    for (size_t i = 0; i < samples.size(); ++i) {
        // Shift delay line
        for (int j = filter_delay_line_.size() - 1; j > 0; --j) {
            filter_delay_line_[j] = filter_delay_line_[j - 1];
        }
        filter_delay_line_[0] = samples[i];
        
        // Apply filter
        std::complex<float> output(0, 0);
        for (size_t j = 0; j < filter_taps_.size() && j < filter_delay_line_.size(); ++j) {
            output += filter_delay_line_[j] * filter_taps_[j];
        }
        
        samples[i] = output;
    }
}

void SignalProcessor::applySquelch(std::vector<float>& audio) {
    float power = 0.0f;
    for (float sample : audio) {
        power += sample * sample;
    }
    power = std::sqrt(power / audio.size());
    
    if (power < squelch_threshold_) {
        std::fill(audio.begin(), audio.end(), 0.0f);
    }
}

float SignalProcessor::calculatePower(const std::vector<std::complex<float>>& samples) {
    float power = 0.0f;
    for (const auto& sample : samples) {
        power += std::norm(sample);
    }
    return power / samples.size();
}
