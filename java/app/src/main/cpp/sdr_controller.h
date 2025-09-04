#ifndef SDR_CONTROLLER_H
#define SDR_CONTROLLER_H

#include <vector>
#include <complex>
#include <cstdint>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

extern "C" {
#include "rtl-sdr.h"
}

class SDRController {
public:
    SDRController();
    ~SDRController();
    
    bool initDevice(int fd);
    void closeDevice();
    bool isDeviceOpen() const;
    
    bool setFrequency(uint32_t freq_hz);
    bool setGain(int gain_tenths_db);
    bool setAutoGain(bool enable);
    bool setSampleRate(uint32_t rate);
    bool setFrequencyCorrection(int ppm);
    
    bool startReading();
    void stopReading();
    bool readSamples(std::vector<std::complex<float>>& samples);
    
    uint32_t getCurrentFrequency() const { return current_frequency_; }
    uint32_t getCurrentSampleRate() const { return current_sample_rate_; }
    int getCurrentGain() const { return current_gain_; }
    
private:
    static void asyncCallback(unsigned char *buf, uint32_t len, void *ctx);
    void processBuffer(unsigned char *buf, uint32_t len);
    
    rtlsdr_dev_t* device_;
    std::atomic<bool> device_open_;
    std::atomic<bool> reading_active_;
    
    uint32_t current_frequency_;
    uint32_t current_sample_rate_;
    int current_gain_;
    bool auto_gain_;
    
    // Buffer management
    std::vector<std::complex<float>> sample_buffer_;
    std::mutex buffer_mutex_;
    std::condition_variable buffer_cv_;
    size_t buffer_read_pos_;
    size_t buffer_write_pos_;
    static const size_t BUFFER_SIZE = 16384;
    
    std::thread read_thread_;
};

#endif // SDR_CONTROLLER_H
