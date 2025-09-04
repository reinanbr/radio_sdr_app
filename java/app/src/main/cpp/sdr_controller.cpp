#include "sdr_controller.h"
#include <android/log.h>
#include <algorithm>
#include <cstring>

#define LOG_TAG "SDR_Controller"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

SDRController::SDRController()
    : device_(nullptr)
    , device_open_(false)
    , reading_active_(false)
    , current_frequency_(88500000)  // 88.5 MHz
    , current_sample_rate_(2048000) // 2.048 MHz
    , current_gain_(248)            // 24.8 dB
    , auto_gain_(true)
    , buffer_read_pos_(0)
    , buffer_write_pos_(0) {
    
    sample_buffer_.resize(BUFFER_SIZE);
}

SDRController::~SDRController() {
    closeDevice();
}

bool SDRController::initDevice(int fd) {
    LOGI("Initializing RTL-SDR device with file descriptor: %d", fd);
    
    // Open device using file descriptor
    int result = rtlsdr_open_fd(&device_, fd);
    if (result != 0) {
        LOGE("Failed to open RTL-SDR device: %d", result);
        return false;
    }
    
    // Reset device
    rtlsdr_reset_buffer(device_);
    
    // Set initial parameters
    if (!setSampleRate(current_sample_rate_)) {
        LOGE("Failed to set initial sample rate");
        closeDevice();
        return false;
    }
    
    if (!setFrequency(current_frequency_)) {
        LOGE("Failed to set initial frequency");
        closeDevice();
        return false;
    }
    
    if (!setAutoGain(auto_gain_)) {
        LOGE("Failed to set initial auto gain");
        closeDevice();
        return false;
    }
    
    if (!auto_gain_ && !setGain(current_gain_)) {
        LOGE("Failed to set initial manual gain");
        closeDevice();
        return false;
    }
    
    device_open_.store(true);
    LOGI("RTL-SDR device initialized successfully");
    return true;
}

void SDRController::closeDevice() {
    if (device_open_.load()) {
        stopReading();
        
        if (device_) {
            rtlsdr_close(device_);
            device_ = nullptr;
        }
        
        device_open_.store(false);
        LOGI("RTL-SDR device closed");
    }
}

bool SDRController::isDeviceOpen() const {
    return device_open_.load();
}

bool SDRController::setFrequency(uint32_t freq_hz) {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    int result = rtlsdr_set_center_freq(device_, freq_hz);
    if (result != 0) {
        LOGE("Failed to set frequency to %u Hz: %d", freq_hz, result);
        return false;
    }
    
    current_frequency_ = freq_hz;
    LOGD("Frequency set to %u Hz", freq_hz);
    return true;
}

bool SDRController::setGain(int gain_tenths_db) {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    if (auto_gain_) {
        LOGD("Auto gain is enabled, manual gain setting ignored");
        return true;
    }
    
    int result = rtlsdr_set_tuner_gain(device_, gain_tenths_db);
    if (result != 0) {
        LOGE("Failed to set gain to %d (0.1 dB): %d", gain_tenths_db, result);
        return false;
    }
    
    current_gain_ = gain_tenths_db;
    LOGD("Gain set to %d (0.1 dB)", gain_tenths_db);
    return true;
}

bool SDRController::setAutoGain(bool enable) {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    int result = rtlsdr_set_tuner_gain_mode(device_, enable ? 0 : 1);
    if (result != 0) {
        LOGE("Failed to set auto gain mode: %d", result);
        return false;
    }
    
    auto_gain_ = enable;
    LOGD("Auto gain %s", enable ? "enabled" : "disabled");
    return true;
}

bool SDRController::setSampleRate(uint32_t rate) {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    int result = rtlsdr_set_sample_rate(device_, rate);
    if (result != 0) {
        LOGE("Failed to set sample rate to %u Hz: %d", rate, result);
        return false;
    }
    
    current_sample_rate_ = rate;
    LOGD("Sample rate set to %u Hz", rate);
    return true;
}

bool SDRController::setFrequencyCorrection(int ppm) {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    int result = rtlsdr_set_freq_correction(device_, ppm);
    if (result != 0) {
        LOGE("Failed to set frequency correction to %d PPM: %d", ppm, result);
        return false;
    }
    
    LOGD("Frequency correction set to %d PPM", ppm);
    return true;
}

bool SDRController::startReading() {
    if (!device_open_.load()) {
        LOGE("Device not open");
        return false;
    }
    
    if (reading_active_.load()) {
        LOGD("Reading already active");
        return true;
    }
    
    // Reset buffer
    rtlsdr_reset_buffer(device_);
    
    // Clear our internal buffer
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_read_pos_ = 0;
        buffer_write_pos_ = 0;
    }
    
    reading_active_.store(true);
    
    // Start async reading
    int result = rtlsdr_read_async(device_, asyncCallback, this, 0, 16384);
    if (result != 0) {
        LOGE("Failed to start async reading: %d", result);
        reading_active_.store(false);
        return false;
    }
    
    LOGI("Started reading from RTL-SDR");
    return true;
}

void SDRController::stopReading() {
    if (reading_active_.load()) {
        reading_active_.store(false);
        rtlsdr_cancel_async(device_);
        
        if (read_thread_.joinable()) {
            read_thread_.join();
        }
        
        LOGI("Stopped reading from RTL-SDR");
    }
}

void SDRController::asyncCallback(unsigned char *buf, uint32_t len, void *ctx) {
    SDRController* controller = static_cast<SDRController*>(ctx);
    if (controller && controller->reading_active_.load()) {
        controller->processBuffer(buf, len);
    }
}

void SDRController::processBuffer(unsigned char *buf, uint32_t len) {
    if (len % 2 != 0) {
        LOGE("Invalid buffer length: %u", len);
        return;
    }
    
    size_t num_samples = len / 2;
    std::vector<std::complex<float>> new_samples(num_samples);
    
    // Convert unsigned 8-bit IQ to complex float
    for (size_t i = 0; i < num_samples; ++i) {
        float i_val = (buf[2*i] - 127.4f) / 127.4f;      // I component
        float q_val = (buf[2*i + 1] - 127.4f) / 127.4f;  // Q component
        new_samples[i] = std::complex<float>(i_val, q_val);
    }
    
    // Add to circular buffer
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        
        for (const auto& sample : new_samples) {
            sample_buffer_[buffer_write_pos_] = sample;
            buffer_write_pos_ = (buffer_write_pos_ + 1) % BUFFER_SIZE;
            
            // If buffer is full, advance read position
            if (buffer_write_pos_ == buffer_read_pos_) {
                buffer_read_pos_ = (buffer_read_pos_ + 1) % BUFFER_SIZE;
            }
        }
    }
    
    buffer_cv_.notify_one();
}

bool SDRController::readSamples(std::vector<std::complex<float>>& samples) {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    
    // Wait for data or timeout
    if (buffer_cv_.wait_for(lock, std::chrono::milliseconds(100), 
                           [this] { return buffer_read_pos_ != buffer_write_pos_ || !reading_active_.load(); })) {
        
        size_t available = 0;
        if (buffer_write_pos_ >= buffer_read_pos_) {
            available = buffer_write_pos_ - buffer_read_pos_;
        } else {
            available = BUFFER_SIZE - buffer_read_pos_ + buffer_write_pos_;
        }
        
        if (available > 0) {
            size_t to_read = std::min(available, samples.capacity());
            samples.resize(to_read);
            
            for (size_t i = 0; i < to_read; ++i) {
                samples[i] = sample_buffer_[buffer_read_pos_];
                buffer_read_pos_ = (buffer_read_pos_ + 1) % BUFFER_SIZE;
            }
            
            return true;
        }
    }
    
    return false;
}
