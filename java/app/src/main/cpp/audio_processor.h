#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <vector>
#include <atomic>
#include <mutex>

class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();
    
    void processAudio(const std::vector<float>& audio_samples);
    std::vector<int16_t> getAudioBuffer();
    
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    
private:
    void applyVolume(std::vector<int16_t>& samples);
    void applyLimiter(std::vector<int16_t>& samples);
    
    std::atomic<float> volume_;
    
    // Audio buffer for output
    std::vector<int16_t> audio_buffer_;
    std::atomic<size_t> audio_read_pos_;
    std::atomic<size_t> audio_write_pos_;
    std::mutex buffer_mutex_;
    static const size_t AUDIO_BUFFER_SIZE = 16384;
    
    // Audio processing parameters
    static const int16_t MAX_AMPLITUDE = 32000;
    static constexpr float LIMITER_THRESHOLD = 0.95f;
};

#endif // AUDIO_PROCESSOR_H
