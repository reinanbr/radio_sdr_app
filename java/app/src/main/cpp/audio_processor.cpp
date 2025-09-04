#include "audio_processor.h"
#include <android/log.h>
#include <algorithm>
#include <cmath>

#define LOG_TAG "Audio_Processor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

AudioProcessor::AudioProcessor()
    : volume_(0.5f)
    , audio_read_pos_(0)
    , audio_write_pos_(0) {
    
    audio_buffer_.resize(AUDIO_BUFFER_SIZE, 0);
    
    LOGI("Audio processor initialized");
}

AudioProcessor::~AudioProcessor() {
    LOGI("Audio processor destroyed");
}

void AudioProcessor::processAudio(const std::vector<float>& audio_samples) {
    if (audio_samples.empty()) {
        return;
    }
    
    // Convert float samples to int16_t
    std::vector<int16_t> int_samples;
    int_samples.reserve(audio_samples.size());
    
    for (float sample : audio_samples) {
        // Clamp to [-1.0, 1.0] range
        sample = std::max(-1.0f, std::min(1.0f, sample));
        
        // Convert to int16_t
        int16_t int_sample = static_cast<int16_t>(sample * MAX_AMPLITUDE);
        int_samples.push_back(int_sample);
    }
    
    // Apply volume
    applyVolume(int_samples);
    
    // Apply limiter to prevent clipping
    applyLimiter(int_samples);
    
    // Add to audio buffer
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        
        size_t write_pos = audio_write_pos_.load();
        for (int16_t sample : int_samples) {
            audio_buffer_[write_pos] = sample;
            write_pos = (write_pos + 1) % AUDIO_BUFFER_SIZE;
        }
        audio_write_pos_.store(write_pos);
        
        // If buffer is getting full, advance read position
        size_t read_pos = audio_read_pos_.load();
        size_t available = (write_pos >= read_pos) ? 
            (write_pos - read_pos) : (AUDIO_BUFFER_SIZE - read_pos + write_pos);
        
        if (available > AUDIO_BUFFER_SIZE * 3 / 4) {
            read_pos = (write_pos + AUDIO_BUFFER_SIZE / 4) % AUDIO_BUFFER_SIZE;
            audio_read_pos_.store(read_pos);
        }
    }
}

std::vector<int16_t> AudioProcessor::getAudioBuffer() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    size_t read_pos = audio_read_pos_.load();
    size_t write_pos = audio_write_pos_.load();
    
    size_t available = (write_pos >= read_pos) ? 
        (write_pos - read_pos) : (AUDIO_BUFFER_SIZE - read_pos + write_pos);
    
    if (available == 0) {
        return {};
    }
    
    // Return up to 1024 samples at a time to avoid large allocations
    size_t to_read = std::min(available, static_cast<size_t>(1024));
    
    std::vector<int16_t> result;
    result.reserve(to_read);
    
    for (size_t i = 0; i < to_read; ++i) {
        result.push_back(audio_buffer_[read_pos]);
        read_pos = (read_pos + 1) % AUDIO_BUFFER_SIZE;
    }
    
    audio_read_pos_.store(read_pos);
    return result;
}

void AudioProcessor::setVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
    LOGD("Volume set to %.2f", volume_.load());
}

void AudioProcessor::applyVolume(std::vector<int16_t>& samples) {
    float vol = volume_.load();
    
    if (vol != 1.0f) {
        for (int16_t& sample : samples) {
            float adjusted = static_cast<float>(sample) * vol;
            sample = static_cast<int16_t>(std::max(-32768.0f, std::min(32767.0f, adjusted)));
        }
    }
}

void AudioProcessor::applyLimiter(std::vector<int16_t>& samples) {
    int16_t threshold = static_cast<int16_t>(MAX_AMPLITUDE * LIMITER_THRESHOLD);
    
    for (int16_t& sample : samples) {
        if (sample > threshold) {
            sample = threshold;
        } else if (sample < -threshold) {
            sample = -threshold;
        }
    }
}
