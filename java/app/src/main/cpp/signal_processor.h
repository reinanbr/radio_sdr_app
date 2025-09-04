#ifndef SIGNAL_PROCESSOR_H
#define SIGNAL_PROCESSOR_H

#include <vector>
#include <complex>
#include <memory>
#include <atomic>

#include "demodulator.h"

class SignalProcessor {
public:
    SignalProcessor();
    ~SignalProcessor();
    
    void processSamples(const std::vector<std::complex<float>>& samples);
    std::vector<float> getAudioSamples();
    
    bool setBandwidth(int bandwidth_hz);
    bool setSquelch(int squelch_db);
    void setDemodulationType(DemodulationType type);
    
    int getBandwidth() const { return bandwidth_hz_; }
    int getSquelch() const { return squelch_db_; }
    DemodulationType getDemodulationType() const { return demod_type_; }
    
private:
    void applyBandpassFilter(std::vector<std::complex<float>>& samples);
    void applySquelch(std::vector<float>& audio);
    float calculatePower(const std::vector<std::complex<float>>& samples);
    
    std::unique_ptr<Demodulator> demodulator_;
    DemodulationType demod_type_;
    
    int bandwidth_hz_;
    int squelch_db_;
    float squelch_threshold_;
    
    // Filter coefficients and state
    std::vector<float> filter_taps_;
    std::vector<std::complex<float>> filter_delay_line_;
    
    // Audio buffer
    std::vector<float> audio_buffer_;
    std::atomic<size_t> audio_read_pos_;
    std::atomic<size_t> audio_write_pos_;
    static const size_t AUDIO_BUFFER_SIZE = 8192;
    
    // AGC
    float agc_gain_;
    float agc_target_;
    float agc_attack_;
    float agc_decay_;
};

#endif // SIGNAL_PROCESSOR_H
