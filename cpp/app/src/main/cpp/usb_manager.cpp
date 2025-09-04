#include "usb_manager.h"
#include <android/log.h>
#include <cstring>

// Definições de log
#define LOG_TAG "USBManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Simulação das estruturas libusb para desenvolvimento
struct libusb_context {
    bool initialized;
};

struct libusb_device {
    uint16_t vendor_id;
    uint16_t product_id;
    std::string serial;
};

struct libusb_device_handle {
    libusb_device* device;
    bool claimed;
};

typedef int libusb_hotplug_callback_handle;

// Simulação das funções libusb
extern "C" {
    int libusb_init(libusb_context** ctx) {
        *ctx = new libusb_context();
        (*ctx)->initialized = true;
        LOGI("Simulated libusb initialized");
        return 0;
    }
    
    void libusb_exit(libusb_context* ctx) {
        if (ctx) {
            delete ctx;
            LOGI("Simulated libusb exited");
        }
    }
    
    int libusb_get_device_list(libusb_context* ctx, libusb_device*** list) {
        if (!ctx || !ctx->initialized) return -1;
        
        // Simular um dispositivo RTL-SDR
        *list = new libusb_device*[1];
        (*list)[0] = new libusb_device();
        (*list)[0]->vendor_id = 0x0bda;
        (*list)[0]->product_id = 0x2838;
        (*list)[0]->serial = "00000001";
        
        return 1; // 1 dispositivo
    }
    
    void libusb_free_device_list(libusb_device** list, int unref_devices) {
        if (list) {
            delete list[0];
            delete[] list;
        }
    }
    
    int libusb_open(libusb_device* dev, libusb_device_handle** handle) {
        if (!dev) return -1;
        
        *handle = new libusb_device_handle();
        (*handle)->device = dev;
        (*handle)->claimed = false;
        
        LOGI("Simulated USB device opened: %04x:%04x", dev->vendor_id, dev->product_id);
        return 0;
    }
    
    void libusb_close(libusb_device_handle* handle) {
        if (handle) {
            delete handle;
            LOGI("Simulated USB device closed");
        }
    }
    
    int libusb_claim_interface(libusb_device_handle* handle, int interface_number) {
        if (!handle) return -1;
        
        handle->claimed = true;
        LOGI("Simulated USB interface %d claimed", interface_number);
        return 0;
    }
    
    int libusb_release_interface(libusb_device_handle* handle, int interface_number) {
        if (!handle) return -1;
        
        handle->claimed = false;
        LOGI("Simulated USB interface %d released", interface_number);
        return 0;
    }
    
    int libusb_bulk_transfer(libusb_device_handle* handle, unsigned char endpoint, 
                           unsigned char* data, int length, int* transferred, unsigned int timeout) {
        if (!handle || !handle->claimed) return -1;
        
        // Simular transferência de dados
        *transferred = length;
        LOGI("Simulated bulk transfer: %d bytes", length);
        return 0;
    }
    
    int libusb_control_transfer(libusb_device_handle* handle, uint8_t request_type, 
                              uint8_t request, uint16_t value, uint16_t index,
                              unsigned char* data, uint16_t length, unsigned int timeout) {
        if (!handle || !handle->claimed) return -1;
        
        LOGI("Simulated control transfer: request=0x%02x, value=0x%04x", request, value);
        return length;
    }
    
    int libusb_get_device_descriptor(libusb_device* dev, void* desc) {
        if (!dev) return -1;
        // Simular descritor do dispositivo
        return 0;
    }
    
    int libusb_get_string_descriptor_ascii(libusb_device_handle* handle, uint8_t desc_index,
                                         unsigned char* data, int length) {
        if (!handle) return -1;
        
        // Simular string descritor
        const char* serial = "00000001";
        int len = strlen(serial);
        if (len > length) len = length;
        memcpy(data, serial, len);
        return len;
    }
    
    int libusb_has_capability(uint32_t capability) {
        // Simular capacidades do libusb
        return 1; // Suporta hotplug
    }
    
    int libusb_hotplug_register_callback(libusb_context* ctx, int events, int flags,
                                       int vendor_id, int product_id, int dev_class,
                                       int (*cb_fn)(libusb_context*, libusb_device*, int, void*),
                                       void* user_data, libusb_hotplug_callback_handle* handle) {
        if (!ctx) return -1;
        
        *handle = 1; // Simular handle
        LOGI("Simulated hotplug callback registered");
        return 0;
    }
    
    void libusb_hotplug_deregister_callback(libusb_context* ctx, libusb_hotplug_callback_handle handle) {
        LOGI("Simulated hotplug callback deregistered");
    }
    
    int libusb_handle_events(libusb_context* ctx) {
        if (!ctx) return -1;
        // Simular processamento de eventos
        return 0;
    }
}

USBManager::USBManager()
    : context_(nullptr)
    , deviceHandle_(nullptr)
    , initialized_(false)
    , deviceOpen_(false)
    , monitoring_(false)
    , running_(false)
    , hotplugSupported_(false)
    , hotplugHandle_(0) {
    
    LOGI("USBManager constructor called");
}

USBManager::~USBManager() {
    LOGI("USBManager destructor called");
    shutdown();
}

bool USBManager::initialize() {
    LOGI("Initializing USBManager");
    
    if (initialized_) {
        LOGW("USBManager already initialized");
        return true;
    }
    
    if (!initializeLibUSB()) {
        LOGE("Failed to initialize libusb");
        return false;
    }
    
    // Verificar suporte a hotplug
    hotplugSupported_ = (libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0);
    LOGI("Hotplug support: %s", hotplugSupported_ ? "yes" : "no");
    
    initialized_ = true;
    LOGI("USBManager initialized successfully");
    return true;
}

void USBManager::shutdown() {
    LOGI("Shutting down USBManager");
    
    stopMonitoring();
    closeDevice();
    
    if (initialized_) {
        destroyLibUSB();
        initialized_ = false;
    }
    
    LOGI("USBManager shutdown complete");
}

bool USBManager::openDevice(int deviceIndex) {
    LOGI("Opening USB device %d", deviceIndex);
    
    if (!initialized_) {
        LOGE("USBManager not initialized");
        return false;
    }
    
    if (deviceOpen_) {
        LOGW("Device already open, closing first");
        closeDevice();
    }
    
    libusb_device** list;
    int count = libusb_get_device_list(context_, &list);
    if (count <= 0) {
        LOGE("No USB devices found");
        return false;
    }
    
    if (deviceIndex >= count) {
        LOGE("Device index %d out of range (0-%d)", deviceIndex, count - 1);
        libusb_free_device_list(list, 1);
        return false;
    }
    
    int result = libusb_open(list[deviceIndex], &deviceHandle_);
    libusb_free_device_list(list, 1);
    
    if (result != 0) {
        LOGE("Failed to open USB device: %d", result);
        return false;
    }
    
    deviceOpen_ = true;
    
    // Obter informações do dispositivo
    updateDeviceInfo(list[deviceIndex]);
    
    LOGI("USB device opened successfully");
    return true;
}

bool USBManager::openDeviceByVendorProduct(uint16_t vendorId, uint16_t productId) {
    LOGI("Opening USB device %04x:%04x", vendorId, productId);
    
    if (!initialized_) {
        LOGE("USBManager not initialized");
        return false;
    }
    
    if (deviceOpen_) {
        LOGW("Device already open, closing first");
        closeDevice();
    }
    
    libusb_device** list;
    int count = libusb_get_device_list(context_, &list);
    if (count <= 0) {
        LOGE("No USB devices found");
        return false;
    }
    
    // Procurar dispositivo com vendor/product ID específicos
    for (int i = 0; i < count; i++) {
        if (list[i]->vendor_id == vendorId && list[i]->product_id == productId) {
            int result = libusb_open(list[i], &deviceHandle_);
            libusb_free_device_list(list, 1);
            
            if (result == 0) {
                deviceOpen_ = true;
                updateDeviceInfo(list[i]);
                LOGI("USB device %04x:%04x opened successfully", vendorId, productId);
                return true;
            } else {
                LOGE("Failed to open USB device %04x:%04x: %d", vendorId, productId, result);
                return false;
            }
        }
    }
    
    libusb_free_device_list(list, 1);
    LOGE("USB device %04x:%04x not found", vendorId, productId);
    return false;
}

void USBManager::closeDevice() {
    LOGI("Closing USB device");
    
    if (deviceHandle_ && deviceOpen_) {
        libusb_close(deviceHandle_);
        deviceHandle_ = nullptr;
        deviceOpen_ = false;
        
        // Limpar informações do dispositivo
        std::lock_guard<std::mutex> lock(deviceMutex_);
        currentDevice_ = DeviceInfo();
        
        LOGI("USB device closed");
    }
}

bool USBManager::isDeviceOpen() const {
    return deviceOpen_;
}

std::vector<USBManager::DeviceInfo> USBManager::enumerateDevices() {
    std::vector<DeviceInfo> devices;
    
    if (!initialized_) {
        LOGE("USBManager not initialized");
        return devices;
    }
    
    libusb_device** list;
    int count = libusb_get_device_list(context_, &list);
    if (count <= 0) {
        return devices;
    }
    
    for (int i = 0; i < count; i++) {
        DeviceInfo info;
        info.deviceIndex = i;
        info.vendorId = list[i]->vendor_id;
        info.productId = list[i]->product_id;
        info.serialNumber = list[i]->serial;
        info.name = "USB Device " + std::to_string(i);
        
        devices.push_back(info);
    }
    
    libusb_free_device_list(list, 1);
    return devices;
}

int USBManager::getDeviceCount() const {
    if (!initialized_) return 0;
    
    libusb_device** list;
    int count = libusb_get_device_list(context_, &list);
    libusb_free_device_list(list, 1);
    return count;
}

bool USBManager::claimInterface(int interfaceNumber) {
    if (!deviceHandle_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    int result = libusb_claim_interface(deviceHandle_, interfaceNumber);
    if (result != 0) {
        LOGE("Failed to claim interface %d: %d", interfaceNumber, result);
        return false;
    }
    
    LOGI("Interface %d claimed successfully", interfaceNumber);
    return true;
}

bool USBManager::releaseInterface(int interfaceNumber) {
    if (!deviceHandle_ || !deviceOpen_) {
        LOGE("Device not open");
        return false;
    }
    
    int result = libusb_release_interface(deviceHandle_, interfaceNumber);
    if (result != 0) {
        LOGE("Failed to release interface %d: %d", interfaceNumber, result);
        return false;
    }
    
    LOGI("Interface %d released successfully", interfaceNumber);
    return true;
}

int USBManager::bulkTransfer(unsigned char endpoint, unsigned char* data, int length, unsigned int timeout) {
    if (!deviceHandle_ || !deviceOpen_) {
        LOGE("Device not open");
        return -1;
    }
    
    int transferred = 0;
    int result = libusb_bulk_transfer(deviceHandle_, endpoint, data, length, &transferred, timeout);
    
    if (result != 0) {
        LOGE("Bulk transfer failed: %d", result);
        return result;
    }
    
    return transferred;
}

int USBManager::controlTransfer(uint8_t requestType, uint8_t request, uint16_t value, uint16_t index,
                              unsigned char* data, uint16_t length, unsigned int timeout) {
    if (!deviceHandle_ || !deviceOpen_) {
        LOGE("Device not open");
        return -1;
    }
    
    int result = libusb_control_transfer(deviceHandle_, requestType, request, value, index, data, length, timeout);
    
    if (result < 0) {
        LOGE("Control transfer failed: %d", result);
        return result;
    }
    
    return result;
}

void USBManager::setDeviceConnectedCallback(DeviceConnectedCallback callback) {
    deviceConnectedCallback_ = callback;
}

void USBManager::setDeviceDisconnectedCallback(DeviceDisconnectedCallback callback) {
    deviceDisconnectedCallback_ = callback;
}

void USBManager::setErrorCallback(ErrorCallback callback) {
    errorCallback_ = callback;
}

void USBManager::startMonitoring() {
    LOGI("Starting USB device monitoring");
    
    if (!initialized_) {
        LOGE("USBManager not initialized");
        return;
    }
    
    if (monitoring_) {
        LOGW("Monitoring already active");
        return;
    }
    
    monitoring_ = true;
    running_ = true;
    monitorThread_ = std::thread(&USBManager::monitorLoop, this);
    
    LOGI("USB device monitoring started");
}

void USBManager::stopMonitoring() {
    LOGI("Stopping USB device monitoring");
    
    monitoring_ = false;
    running_ = false;
    
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }
    
    LOGI("USB device monitoring stopped");
}

void USBManager::monitorLoop() {
    LOGI("USB monitoring thread started");
    
    while (running_ && monitoring_) {
        if (context_) {
            libusb_handle_events(context_);
        }
        
        // Pequena pausa para não sobrecarregar o sistema
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    LOGI("USB monitoring thread ended");
}

USBManager::DeviceInfo USBManager::getCurrentDeviceInfo() const {
    std::lock_guard<std::mutex> lock(deviceMutex_);
    return currentDevice_;
}

libusb_device_handle* USBManager::getCurrentDeviceHandle() const {
    return deviceHandle_;
}

bool USBManager::initializeLibUSB() {
    LOGI("Initializing libusb");
    
    int result = libusb_init(&context_);
    if (result != 0) {
        LOGE("Failed to initialize libusb: %d", result);
        return false;
    }
    
    LOGI("libusb initialized successfully");
    return true;
}

void USBManager::destroyLibUSB() {
    LOGI("Destroying libusb");
    
    if (context_) {
        libusb_exit(context_);
        context_ = nullptr;
        LOGI("libusb destroyed");
    }
}

void USBManager::updateDeviceInfo(libusb_device* device) {
    std::lock_guard<std::mutex> lock(deviceMutex_);
    
    currentDevice_.vendorId = device->vendor_id;
    currentDevice_.productId = device->product_id;
    currentDevice_.serialNumber = device->serial;
    currentDevice_.name = "RTL-SDR Device";
    currentDevice_.deviceIndex = 0; // Simplificado para simulação
}

int USBManager::hotplugCallback(libusb_context* ctx, libusb_device* dev, int event, void* user_data) {
    auto* manager = static_cast<USBManager*>(user_data);
    if (!manager) return 0;
    
    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        LOGI("USB device connected");
        if (manager->deviceConnectedCallback_) {
            DeviceInfo info;
            info.vendorId = dev->vendor_id;
            info.productId = dev->product_id;
            info.serialNumber = dev->serial;
            info.name = "RTL-SDR Device";
            info.deviceIndex = 0;
            
            manager->deviceConnectedCallback_(info);
        }
    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        LOGI("USB device disconnected");
        if (manager->deviceDisconnectedCallback_) {
            DeviceInfo info;
            info.vendorId = dev->vendor_id;
            info.productId = dev->product_id;
            info.serialNumber = dev->serial;
            info.name = "RTL-SDR Device";
            info.deviceIndex = 0;
            
            manager->deviceDisconnectedCallback_(info);
        }
    }
    
    return 0;
} 