#include "sdr_manager.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>

// Definições de log
#define LOG_TAG "SDRManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Simulação da estrutura rtlsdr_dev para desenvolvimento
struct rtlsdr_dev {
    bool is_open;
    double frequency;
    int sample_rate;
    float gain;
    bool auto_gain;
    int bandwidth;
    bool streaming;
};

// Simulação das funções RTL-SDR
extern "C" {
    int rtlsdr_open(rtlsdr_dev** dev, int index) {
        *dev = new rtlsdr_dev();
        (*dev)->is_open = true;
        (*dev)->frequency = 100.0e6;
        (*dev)->sample_rate = 2048000;
        (*dev)->gain = 20.0;
        (*dev)->auto_gain = true;
        (*dev)->bandwidth = 2500000;
        (*dev)->streaming = false;
        LOGI("Simulated RTL-SDR device opened");
        return 0;
    }
    
    void rtlsdr_close(rtlsdr_dev* dev) {
        if (dev) {
            dev->is_open = false;
            dev->streaming = false;
            delete dev;
            LOGI("Simulated RTL-SDR device closed");
        }
    }
    
    int rtlsdr_set_center_freq(rtlsdr_dev* dev, uint32_t freq) {
        if (dev && dev->is_open) {
            dev->frequency = freq;
            LOGI("Frequency set to %u Hz", freq);
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_set_sample_rate(rtlsdr_dev* dev, uint32_t rate) {
        if (dev && dev->is_open) {
            dev->sample_rate = rate;
            LOGI("Sample rate set to %u Hz", rate);
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_set_tuner_gain(rtlsdr_dev* dev, int gain) {
        if (dev && dev->is_open) {
            dev->gain = gain;
            dev->auto_gain = false;
            LOGI("Gain set to %d", gain);
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_set_tuner_gain_mode(rtlsdr_dev* dev, int mode) {
        if (dev && dev->is_open) {
            dev->auto_gain = (mode == 1);
            LOGI("Gain mode set to %s", dev->auto_gain ? "auto" : "manual");
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_set_bandwidth(rtlsdr_dev* dev, uint32_t bw) {
        if (dev && dev->is_open) {
            dev->bandwidth = bw;
            LOGI("Bandwidth set to %u Hz", bw);
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_read_sync(rtlsdr_dev* dev, void* buf, int len, int* n_read) {
        if (dev && dev->is_open && dev->streaming) {
            // Simular dados I/Q
            uint8_t* buffer = static_cast<uint8_t*>(buf);
            for (int i = 0; i < len; i += 2) {
                // Gerar dados I/Q simulados (ruído + sinal)
                float noise_i = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
                float noise_q = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
                
                // Adicionar sinal simulado (onda senoidal)
                float time = static_cast<float>(i) / dev->sample_rate;
                float signal = 0.1f * sin(2.0f * M_PI * 1000.0f * time);
                
                buffer[i] = static_cast<uint8_t>((noise_i + signal) * 128.0f + 128.0f);
                buffer[i + 1] = static_cast<uint8_t>((noise_q + signal) * 128.0f + 128.0f);
            }
            *n_read = len;
            return 0;
        }
        return -1;
    }
    
    int rtlsdr_get_device_count() {
        return 1; // Simular um dispositivo disponível
    }
    
    int rtlsdr_get_device_name(uint32_t index, char* manufacturer, char* product, char* serial) {
        if (index == 0) {
            strcpy(manufacturer, "RTL-SDR");
            strcpy(product, "RTL2838");
            strcpy(serial, "00000001");
            return 0;
        }
        return -1;
    }
}

SDRManager::SDRManager()
    : device_(nullptr)
    , deviceOpen_(false)
    , streaming_(false)
    , running_(false)
    , currentFrequency_(100.0e6)
    , currentSampleRate_(2048000)
    , currentGain_(20.0)
    , currentAutoGain_(true)
    , currentBandwidth_(2500000) {
    
    LOGI("SDRManager constructor called");
}

SDRManager::~SDRManager() {
    LOGI("SDRManager destructor called");
    shutdown();
}

bool SDRManager::initialize() {
    LOGI("Initializing SDRManager");
    
    // Verificar se há dispositivos disponíveis
    int deviceCount = rtlsdr_get_device_count();
    if (deviceCount == 0) {
        LOGE("No RTL-SDR devices found");
        return false;
    }
    
    LOGI("Found %d RTL-SDR device(s)", deviceCount);
    return true;
}

void SDRManager::shutdown() {
    LOGI("Shutting down SDRManager");
    
    stopStreaming();
    closeDevice();
    running_ = false;
    
    if (processThread_.joinable()) {
        processThread_.join();
    }
    
    LOGI("SDRManager shutdown complete");
}

bool SDRManager::openDevice(int deviceIndex) {
    LOGI("Opening RTL-SDR device %d", deviceIndex);
    
    if (deviceOpen_) {
        LOGW("Device already open, closing first");
        closeDevice();
    }
    
    int result = rtlsdr_open(&device_, deviceIndex);
    if (result != 0) {
        LOGE("Failed to open RTL-SDR device: %d", result);
        return false;
    }
    
    deviceOpen_ = true;
    
    // Configurar parâmetros padrão
    setFrequency(currentFrequency_);
    setSampleRate(currentSampleRate_);
    setBandwidth(currentBandwidth_);
    
    if (currentAutoGain_) {
        setAutoGain(true);
    } else {
        setGain(currentGain_);
    }
    
    LOGI("RTL-SDR device opened successfully");
    return true;
}

void SDRManager::closeDevice() {
    LOGI("Closing RTL-SDR device");
    
    if (streaming_) {
        stopStreaming();
    }
    
    if (device_ && deviceOpen_) {
        rtlsdr_close(device_);
        device_ = nullptr;
        deviceOpen_ = false;
        LOGI("RTL-SDR device closed");
    }
}

bool SDRManager::isDeviceOpen() const {
    return deviceOpen_;
}

bool SDRManager::setFrequency(double frequency) {
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    currentFrequency_ = frequency;
    uint32_t freqHz = static_cast<uint32_t>(frequency);
    
    int result = rtlsdr_set_center_freq(device_, freqHz);
    if (result != 0) {
        LOGE("Failed to set frequency: %d", result);
        return false;
    }
    
    LOGI("Frequency set to %.1f MHz", frequency / 1e6);
    return true;
}

bool SDRManager::setSampleRate(int sampleRate) {
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    currentSampleRate_ = sampleRate;
    
    int result = rtlsdr_set_sample_rate(device_, sampleRate);
    if (result != 0) {
        LOGE("Failed to set sample rate: %d", result);
        return false;
    }
    
    LOGI("Sample rate set to %d Hz", sampleRate);
    return true;
}

bool SDRManager::setGain(float gain) {
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    currentGain_ = gain;
    currentAutoGain_ = false;
    
    int gainInt = static_cast<int>(gain * 10); // Converter para unidades de 0.1 dB
    
    int result = rtlsdr_set_tuner_gain(device_, gainInt);
    if (result != 0) {
        LOGE("Failed to set gain: %d", result);
        return false;
    }
    
    LOGI("Gain set to %.1f dB", gain);
    return true;
}

bool SDRManager::setAutoGain(bool enabled) {
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    currentAutoGain_ = enabled;
    
    int mode = enabled ? 1 : 0;
    int result = rtlsdr_set_tuner_gain_mode(device_, mode);
    if (result != 0) {
        LOGE("Failed to set auto gain: %d", result);
        return false;
    }
    
    LOGI("Auto gain %s", enabled ? "enabled" : "disabled");
    return true;
}

bool SDRManager::setBandwidth(int bandwidth) {
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    currentBandwidth_ = bandwidth;
    
    int result = rtlsdr_set_bandwidth(device_, bandwidth);
    if (result != 0) {
        LOGE("Failed to set bandwidth: %d", result);
        return false;
    }
    
    LOGI("Bandwidth set to %d Hz", bandwidth);
    return true;
}

bool SDRManager::startStreaming() {
    LOGI("Starting SDR streaming");
    
    if (!device_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    if (streaming_) {
        LOGW("Streaming already active");
        return true;
    }
    
    // Iniciar thread de processamento
    running_ = true;
    streaming_ = true;
    processThread_ = std::thread(&SDRManager::processLoop, this);
    
    LOGI("SDR streaming started");
    return true;
}

void SDRManager::stopStreaming() {
    LOGI("Stopping SDR streaming");
    
    streaming_ = false;
    running_ = false;
    
    if (processThread_.joinable()) {
        processThread_.join();
    }
    
    LOGI("SDR streaming stopped");
}

bool SDRManager::isStreaming() const {
    return streaming_;
}

void SDRManager::setDataCallback(DataCallback callback) {
    dataCallback_ = callback;
}

void SDRManager::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = callback;
}

int SDRManager::getDeviceCount() const {
    return rtlsdr_get_device_count();
}

std::string SDRManager::getDeviceName(int deviceIndex) const {
    char manufacturer[256], product[256], serial[256];
    
    int result = rtlsdr_get_device_name(deviceIndex, manufacturer, product, serial);
    if (result == 0) {
        return std::string(manufacturer) + " " + product + " (" + serial + ")";
    }
    
    return "Unknown Device";
}

double SDRManager::getCurrentFrequency() const {
    return currentFrequency_;
}

int SDRManager::getCurrentSampleRate() const {
    return currentSampleRate_;
}

float SDRManager::getCurrentGain() const {
    return currentGain_;
}

void SDRManager::processLoop() {
    LOGI("SDRManager process loop started");
    
    const int bufferSize = 16384; // 16KB buffer
    std::vector<uint8_t> buffer(bufferSize);
    
    while (running_ && streaming_) {
        if (!device_ || !deviceOpen_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        int n_read = 0;
        int result = rtlsdr_read_sync(device_, buffer.data(), bufferSize, &n_read);
        
        if (result == 0 && n_read > 0) {
            // Copiar dados para um novo buffer
            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + n_read);
            
            // Chamar callback com os dados
            if (dataCallback_) {
                dataCallback_(data);
            }
        } else if (result != 0) {
            LOGE("Error reading from RTL-SDR: %d", result);
            if (errorCallback_) {
                errorCallback_("Error reading from RTL-SDR device");
            }
            break;
        }
        
        // Pequena pausa para não sobrecarregar o sistema
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    LOGI("SDRManager process loop ended");
}

void SDRManager::rtlsdrCallback(unsigned char* buf, uint32_t len, void* ctx) {
    auto* manager = static_cast<SDRManager*>(ctx);
    if (manager && manager->dataCallback_) {
        std::vector<uint8_t> data(buf, buf + len);
        manager->dataCallback_(data);
    }
} 