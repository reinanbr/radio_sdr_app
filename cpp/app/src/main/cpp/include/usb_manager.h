#ifndef USB_MANAGER_H
#define USB_MANAGER_H

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>

// Forward declarations para libusb
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class USBManager {
public:
    // Estrutura para informações do dispositivo
    struct DeviceInfo {
        std::string name;
        uint16_t vendorId;
        uint16_t productId;
        std::string serialNumber;
        int deviceIndex;
    };

    // Callbacks
    using DeviceConnectedCallback = std::function<void(const DeviceInfo&)>;
    using DeviceDisconnectedCallback = std::function<void(const DeviceInfo&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    USBManager();
    ~USBManager();

    // Inicialização e finalização
    bool initialize();
    void shutdown();

    // Controle de dispositivos
    bool openDevice(int deviceIndex = 0);
    bool openDeviceByVendorProduct(uint16_t vendorId, uint16_t productId);
    void closeDevice();
    bool isDeviceOpen() const;

    // Enumeração de dispositivos
    std::vector<DeviceInfo> enumerateDevices();
    int getDeviceCount() const;

    // Operações USB
    bool claimInterface(int interfaceNumber = 0);
    bool releaseInterface(int interfaceNumber = 0);
    
    // Transferências
    int bulkTransfer(unsigned char endpoint, unsigned char* data, int length, unsigned int timeout = 1000);
    int controlTransfer(uint8_t requestType, uint8_t request, uint16_t value, uint16_t index, unsigned char* data, uint16_t length, unsigned int timeout = 1000);

    // Callbacks
    void setDeviceConnectedCallback(DeviceConnectedCallback callback);
    void setDeviceDisconnectedCallback(DeviceDisconnectedCallback callback);
    void setErrorCallback(ErrorCallback callback);

    // Thread de monitoramento
    void startMonitoring();
    void stopMonitoring();
    void monitorLoop();

    // Informações do dispositivo atual
    DeviceInfo getCurrentDeviceInfo() const;
    libusb_device_handle* getCurrentDeviceHandle() const;

private:
    // Callback estático para libusb
    static int hotplugCallback(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void* user_data);

    // Inicialização do libusb
    bool initializeLibUSB();
    void destroyLibUSB();

    // Contexto libusb
    libusb_context* context_;
    libusb_device_handle* deviceHandle_;
    
    // Estado
    std::atomic<bool> initialized_;
    std::atomic<bool> deviceOpen_;
    std::atomic<bool> monitoring_;
    std::atomic<bool> running_;
    
    // Thread de monitoramento
    std::thread monitorThread_;
    std::mutex mutex_;
    
    // Callbacks
    DeviceConnectedCallback deviceConnectedCallback_;
    DeviceDisconnectedCallback deviceDisconnectedCallback_;
    ErrorCallback errorCallback_;
    
    // Dispositivo atual
    DeviceInfo currentDevice_;
    std::mutex deviceMutex_;
    
    // Suporte a hotplug
    bool hotplugSupported_;
    libusb_hotplug_callback_handle hotplugHandle_;
};

#endif // USB_MANAGER_H 