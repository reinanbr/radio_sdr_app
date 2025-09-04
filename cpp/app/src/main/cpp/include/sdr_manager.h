#ifndef SDR_MANAGER_H
#define SDR_MANAGER_H

#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

// Forward declaration para RTL-SDR
struct rtlsdr_dev;

class SDRManager {
public:
    // Callback para dados recebidos
    using DataCallback = std::function<void(const std::vector<uint8_t>&)>;
    
    SDRManager();
    ~SDRManager();

    // Inicialização e finalização
    bool initialize();
    void shutdown();

    // Controle do dispositivo
    bool openDevice(int deviceIndex = 0);
    void closeDevice();
    bool isDeviceOpen() const;

    // Configurações do dispositivo
    bool setFrequency(double frequency);
    bool setSampleRate(int sampleRate);
    bool setGain(float gain);
    bool setAutoGain(bool enabled);
    bool setBandwidth(int bandwidth);

    // Controle de streaming
    bool startStreaming();
    void stopStreaming();
    bool isStreaming() const;

    // Callbacks
    void setDataCallback(DataCallback callback);
    void setErrorCallback(std::function<void(const std::string&)> callback);

    // Informações do dispositivo
    int getDeviceCount() const;
    std::string getDeviceName(int deviceIndex = 0) const;
    double getCurrentFrequency() const;
    int getCurrentSampleRate() const;
    float getCurrentGain() const;

    // Thread de processamento
    void processLoop();

private:
    // Callback estático para RTL-SDR
    static void rtlsdrCallback(unsigned char* buf, uint32_t len, void* ctx);

    rtlsdr_dev* device_;
    std::atomic<bool> deviceOpen_;
    std::atomic<bool> streaming_;
    std::atomic<bool> running_;
    
    std::thread processThread_;
    std::mutex mutex_;
    
    // Callbacks
    DataCallback dataCallback_;
    std::function<void(const std::string&)> errorCallback_;
    
    // Configurações atuais
    double currentFrequency_;
    int currentSampleRate_;
    float currentGain_;
    bool currentAutoGain_;
    int currentBandwidth_;
    
    // Buffer para dados
    std::vector<uint8_t> buffer_;
    std::mutex bufferMutex_;
};

#endif // SDR_MANAGER_H 