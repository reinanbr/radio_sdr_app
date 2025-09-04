# 🔧 Manual de Integração C++ com Drives Nativos - Android Kotlin/Java

## 📋 Índice

1. [Visão Geral da Integração](#visão-geral-da-integração)
2. [Arquitetura JNI](#arquitetura-jni)
3. [Configuração do Projeto](#configuração-do-projeto)
4. [Desenvolvimento do Drive C++](#desenvolvimento-do-drive-c)
5. [Interface JNI](#interface-jni)
6. [Wrapper Kotlin/Java](#wrapper-kotlinjava)
7. [Build System](#build-system)
8. [Debugging e Performance](#debugging-e-performance)
9. [Exemplos Práticos](#exemplos-práticos)
10. [Troubleshooting](#troubleshooting)

---

## 🎯 Visão Geral da Integração

### O que é JNI?
**JNI (Java Native Interface)** é a ponte que permite código Java/Kotlin chamar funções escritas em C/C++ e vice-versa.

### Por que usar C++ em Android?
- **Performance**: Código nativo é mais rápido
- **Reutilização**: Bibliotecas C++ existentes
- **Hardware**: Acesso direto a drivers e hardware
- **Legacy**: Integração com código C++ existente

### Arquitetura Completa
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Android App   │    │   JNI Layer     │    │   C++ Drive     │
│   (Kotlin/Java) │◄──►│   (Interface)   │◄──►│   (Native)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
   UI/UX Layer            Data Conversion         Hardware Access
```

---

## 🏗️ Arquitetura JNI

### Fluxo de Dados
```
Kotlin/Java → JNI → C++ → Hardware
     ↑         ↑      ↑
   UI Layer  Bridge  Driver
```

### Tipos de Integração

#### 1. **Synchronous (Síncrona)**
```kotlin
// Kotlin chama C++ e aguarda resposta
val result = nativeFunction(param)
```

#### 2. **Asynchronous (Assíncrona)**
```kotlin
// C++ executa em background e notifica Kotlin
nativeStartAsyncOperation()
// ... depois recebe callback
```

#### 3. **Callback-based (Baseada em Callback)**
```kotlin
// C++ chama funções Kotlin
nativeSetCallback(callback)
```

---

## ⚙️ Configuração do Projeto

### 1. Estrutura de Diretórios
```
app/
├── src/
│   └── main/
│       ├── cpp/                    # Código C++ nativo
│       │   ├── CMakeLists.txt      # Build script
│       │   ├── native_lib.cpp      # Implementação JNI
│       │   ├── drive/              # Drive C++ específico
│       │   │   ├── drive.h
│       │   │   └── drive.cpp
│       │   └── utils/              # Utilitários
│       │       ├── logger.h
│       │       └── memory.h
│       ├── java/                   # Código Java/Kotlin
│       │   └── com/
│       │       └── example/
│       │           └── app/
│       │               ├── MainActivity.kt
│       │               └── NativeWrapper.kt
│       └── jniLibs/                # Bibliotecas compiladas
│           ├── arm64-v8a/
│           ├── armeabi-v7a/
│           ├── x86/
│           └── x86_64/
```

### 2. Configuração Gradle (app/build.gradle)
```gradle
android {
    compileSdk 33
    
    defaultConfig {
        minSdk 21
        targetSdk 33
        
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
    
    buildFeatures {
        viewBinding true
    }
    
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.22.1"
        }
    }
    
    ndkVersion "25.1.8937393"
}

dependencies {
    implementation 'androidx.core:core-ktx:1.9.0'
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'com.google.android.material:material:1.8.0'
}
```

### 3. Configuração CMake (CMakeLists.txt)
```cmake
cmake_minimum_required(VERSION 3.22.1)
project("nativeapp")

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir diretórios
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/drive)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Definir fontes
set(SOURCES
    native_lib.cpp
    drive/drive.cpp
    utils/logger.cpp
)

# Criar biblioteca compartilhada
add_library(nativeapp SHARED ${SOURCES})

# Configurações Android
target_compile_definitions(nativeapp PRIVATE
    ANDROID=1
    __ANDROID__=1
)

# Flags de compilação
target_compile_options(nativeapp PRIVATE
    -Wall
    -Wextra
    -fPIC
    -O2
)

# Linkar bibliotecas
target_link_libraries(nativeapp
    android
    log
    m
    c
)
```

---

## 🔧 Desenvolvimento do Drive C++

### 1. Header do Drive (drive.h)
```cpp
#ifndef DRIVE_H
#define DRIVE_H

#include <jni.h>
#include <android/log.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Macros de logging
#define LOG_TAG "NativeDrive"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Estrutura de configuração
struct DriveConfig {
    int device_id;
    std::string device_path;
    int timeout_ms;
    bool auto_reconnect;
};

// Estrutura de dados
struct DriveData {
    std::vector<uint8_t> buffer;
    size_t size;
    long timestamp;
    int status;
};

// Callback para notificações
using DriveCallback = std::function<void(const std::string& event, const DriveData& data)>;

// Classe principal do Drive
class NativeDrive {
private:
    DriveConfig config;
    bool is_connected;
    bool is_initialized;
    DriveCallback callback;
    
    // Métodos privados
    bool initialize_hardware();
    bool connect_device();
    void disconnect_device();
    void process_data(const std::vector<uint8_t>& data);
    
public:
    // Constructor/Destructor
    NativeDrive();
    explicit NativeDrive(const DriveConfig& config);
    ~NativeDrive();
    
    // Gerenciamento de conexão
    bool connect();
    void disconnect();
    bool is_device_connected() const { return is_connected; }
    
    // Configuração
    bool set_config(const DriveConfig& new_config);
    DriveConfig get_config() const { return config; }
    
    // Operações de dados
    bool send_data(const std::vector<uint8_t>& data);
    std::vector<uint8_t> receive_data(size_t max_size = 1024);
    bool start_streaming();
    void stop_streaming();
    
    // Callbacks
    void set_callback(DriveCallback cb) { callback = cb; }
    void clear_callback() { callback = nullptr; }
    
    // Status e informações
    std::string get_device_info();
    int get_last_error();
    void reset_error();
};

#endif // DRIVE_H
```

### 2. Implementação do Drive (drive.cpp)
```cpp
#include "drive.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstring>

NativeDrive::NativeDrive() : is_connected(false), is_initialized(false) {
    config.device_id = 0;
    config.device_path = "/dev/device0";
    config.timeout_ms = 5000;
    config.auto_reconnect = true;
    
    LOGI("NativeDrive created with default config");
}

NativeDrive::NativeDrive(const DriveConfig& cfg) : config(cfg), is_connected(false), is_initialized(false) {
    LOGI("NativeDrive created with custom config: %s", cfg.device_path.c_str());
}

NativeDrive::~NativeDrive() {
    if (is_connected) {
        disconnect();
    }
    LOGI("NativeDrive destroyed");
}

bool NativeDrive::connect() {
    if (is_connected) {
        LOGW("Device already connected");
        return true;
    }
    
    LOGI("Attempting to connect to device: %s", config.device_path.c_str());
    
    // Inicializar hardware
    if (!initialize_hardware()) {
        LOGE("Failed to initialize hardware");
        return false;
    }
    
    // Conectar dispositivo
    if (!connect_device()) {
        LOGE("Failed to connect device");
        return false;
    }
    
    is_connected = true;
    LOGI("Device connected successfully");
    
    return true;
}

void NativeDrive::disconnect() {
    if (!is_connected) return;
    
    LOGI("Disconnecting device");
    
    stop_streaming();
    disconnect_device();
    
    is_connected = false;
    LOGI("Device disconnected");
}

bool NativeDrive::send_data(const std::vector<uint8_t>& data) {
    if (!is_connected) {
        LOGE("Cannot send data: device not connected");
        return false;
    }
    
    LOGD("Sending %zu bytes", data.size());
    
    // Simular envio de dados
    // Em implementação real, aqui seria a comunicação com hardware
    
    if (callback) {
        DriveData response_data;
        response_data.buffer = data; // Echo back
        response_data.size = data.size();
        response_data.timestamp = time(nullptr);
        response_data.status = 0;
        
        callback("data_sent", response_data);
    }
    
    return true;
}

std::vector<uint8_t> NativeDrive::receive_data(size_t max_size) {
    if (!is_connected) {
        LOGE("Cannot receive data: device not connected");
        return {};
    }
    
    // Simular recebimento de dados
    std::vector<uint8_t> data;
    data.reserve(max_size);
    
    // Gerar dados simulados
    for (size_t i = 0; i < max_size && i < 256; ++i) {
        data.push_back(static_cast<uint8_t>(i % 256));
    }
    
    LOGD("Received %zu bytes", data.size());
    
    if (callback) {
        DriveData callback_data;
        callback_data.buffer = data;
        callback_data.size = data.size();
        callback_data.timestamp = time(nullptr);
        callback_data.status = 0;
        
        callback("data_received", callback_data);
    }
    
    return data;
}

bool NativeDrive::start_streaming() {
    if (!is_connected) {
        LOGE("Cannot start streaming: device not connected");
        return false;
    }
    
    LOGI("Starting data streaming");
    
    // Em implementação real, aqui iniciaria thread de streaming
    // Por enquanto, apenas simula
    
    return true;
}

void NativeDrive::stop_streaming() {
    LOGI("Stopping data streaming");
    
    // Em implementação real, aqui pararia thread de streaming
}

std::string NativeDrive::get_device_info() {
    std::string info = "Device: " + config.device_path;
    info += ", Connected: " + std::string(is_connected ? "Yes" : "No");
    info += ", ID: " + std::to_string(config.device_id);
    
    return info;
}

// Métodos privados
bool NativeDrive::initialize_hardware() {
    LOGI("Initializing hardware...");
    
    // Simular inicialização
    usleep(100000); // 100ms
    
    is_initialized = true;
    LOGI("Hardware initialized");
    
    return true;
}

bool NativeDrive::connect_device() {
    LOGI("Connecting to device...");
    
    // Simular conexão
    usleep(200000); // 200ms
    
    LOGI("Device connected");
    return true;
}

void NativeDrive::disconnect_device() {
    LOGI("Disconnecting device...");
    
    // Simular desconexão
    usleep(50000); // 50ms
    
    LOGI("Device disconnected");
}
```

---

## 🔗 Interface JNI

### 1. Implementação JNI (native_lib.cpp)
```cpp
#include <jni.h>
#include "drive.h"
#include <memory>
#include <string>

// Variáveis globais
static std::unique_ptr<NativeDrive> g_drive;
static JavaVM* g_jvm = nullptr;
static jobject g_callback = nullptr;

// Função para obter JNIEnv
JNIEnv* getJNIEnv() {
    JNIEnv* env;
    int status = g_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (status < 0) {
        status = g_jvm->AttachCurrentThread(&env, nullptr);
        if (status < 0) {
            return nullptr;
        }
    }
    return env;
}

// Callback wrapper para C++
void drive_callback_wrapper(const std::string& event, const DriveData& data) {
    JNIEnv* env = getJNIEnv();
    if (!env || !g_callback) return;
    
    // Obter classe do callback
    jclass callback_class = env->GetObjectClass(g_callback);
    if (!callback_class) return;
    
    // Obter método onDriveEvent
    jmethodID method_id = env->GetMethodID(callback_class, "onDriveEvent", 
                                         "(Ljava/lang/String;[BJ)V");
    if (!method_id) return;
    
    // Converter dados
    jstring event_str = env->NewStringUTF(event.c_str());
    jbyteArray data_array = env->NewByteArray(data.buffer.size());
    env->SetByteArrayRegion(data_array, 0, data.buffer.size(), 
                           reinterpret_cast<const jbyte*>(data.buffer.data()));
    
    // Chamar callback Java
    env->CallVoidMethod(g_callback, method_id, event_str, data_array, data.timestamp);
    
    // Limpar referências locais
    env->DeleteLocalRef(event_str);
    env->DeleteLocalRef(data_array);
    env->DeleteLocalRef(callback_class);
}

// Funções JNI

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_app_NativeWrapper_nativeInit(JNIEnv* env, jobject thiz) {
    LOGI("Initializing native drive");
    
    // Criar instância do drive
    DriveConfig config;
    config.device_id = 0;
    config.device_path = "/dev/device0";
    config.timeout_ms = 5000;
    config.auto_reconnect = true;
    
    g_drive = std::make_unique<NativeDrive>(config);
    
    // Configurar callback
    g_drive->set_callback(drive_callback_wrapper);
    
    LOGI("Native drive initialized");
    return reinterpret_cast<jlong>(g_drive.get());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_app_NativeWrapper_nativeDestroy(JNIEnv* env, jobject thiz, jlong ptr) {
    LOGI("Destroying native drive");
    
    if (g_drive) {
        g_drive->disconnect();
        g_drive.reset();
    }
    
    LOGI("Native drive destroyed");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_app_NativeWrapper_nativeConnect(JNIEnv* env, jobject thiz, jlong ptr) {
    if (!g_drive) {
        LOGE("Drive not initialized");
        return JNI_FALSE;
    }
    
    return g_drive->connect() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_app_NativeWrapper_nativeDisconnect(JNIEnv* env, jobject thiz, jlong ptr) {
    if (g_drive) {
        g_drive->disconnect();
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_app_NativeWrapper_nativeSendData(JNIEnv* env, jobject thiz, jlong ptr, 
                                                 jbyteArray data) {
    if (!g_drive) {
        LOGE("Drive not initialized");
        return JNI_FALSE;
    }
    
    // Converter jbyteArray para vector
    jsize length = env->GetArrayLength(data);
    jbyte* bytes = env->GetByteArrayElements(data, nullptr);
    
    std::vector<uint8_t> cpp_data(bytes, bytes + length);
    
    bool result = g_drive->send_data(cpp_data);
    
    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
    
    return result ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_app_NativeWrapper_nativeReceiveData(JNIEnv* env, jobject thiz, jlong ptr, 
                                                    jint max_size) {
    if (!g_drive) {
        LOGE("Drive not initialized");
        return nullptr;
    }
    
    std::vector<uint8_t> data = g_drive->receive_data(max_size);
    
    if (data.empty()) {
        return nullptr;
    }
    
    // Converter vector para jbyteArray
    jbyteArray result = env->NewByteArray(data.size());
    env->SetByteArrayRegion(result, 0, data.size(), 
                           reinterpret_cast<const jbyte*>(data.data()));
    
    return result;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_app_NativeWrapper_nativeStartStreaming(JNIEnv* env, jobject thiz, jlong ptr) {
    if (!g_drive) {
        LOGE("Drive not initialized");
        return JNI_FALSE;
    }
    
    return g_drive->start_streaming() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_app_NativeWrapper_nativeStopStreaming(JNIEnv* env, jobject thiz, jlong ptr) {
    if (g_drive) {
        g_drive->stop_streaming();
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_app_NativeWrapper_nativeGetDeviceInfo(JNIEnv* env, jobject thiz, jlong ptr) {
    if (!g_drive) {
        return env->NewStringUTF("Drive not initialized");
    }
    
    std::string info = g_drive->get_device_info();
    return env->NewStringUTF(info.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_app_NativeWrapper_nativeSetCallback(JNIEnv* env, jobject thiz, jlong ptr, 
                                                    jobject callback) {
    // Salvar referência global para o callback
    if (g_callback) {
        env->DeleteGlobalRef(g_callback);
    }
    
    g_callback = env->NewGlobalRef(callback);
    
    // Salvar JVM para uso posterior
    env->GetJavaVM(&g_jvm);
    
    LOGI("Callback set");
}

// JNI_OnLoad - chamado quando a biblioteca é carregada
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    LOGI("Native library loaded");
    return JNI_VERSION_1_6;
}

// JNI_OnUnload - chamado quando a biblioteca é descarregada
extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    if (g_callback) {
        JNIEnv* env = getJNIEnv();
        if (env) {
            env->DeleteGlobalRef(g_callback);
        }
        g_callback = nullptr;
    }
    
    g_drive.reset();
    g_jvm = nullptr;
    
    LOGI("Native library unloaded");
}
```

---

## 📱 Wrapper Kotlin/Java

### 1. Interface de Callback
```kotlin
interface DriveCallback {
    fun onDriveEvent(event: String, data: ByteArray, timestamp: Long)
}
```

### 2. Wrapper Principal (NativeWrapper.kt)
```kotlin
class NativeWrapper {
    companion object {
        init {
            System.loadLibrary("nativeapp")
        }
    }
    
    private var nativePtr: Long = 0
    private var callback: DriveCallback? = null
    
    init {
        nativePtr = nativeInit()
    }
    
    fun destroy() {
        nativeDestroy(nativePtr)
        nativePtr = 0
    }
    
    fun connect(): Boolean {
        return nativeConnect(nativePtr)
    }
    
    fun disconnect() {
        nativeDisconnect(nativePtr)
    }
    
    fun sendData(data: ByteArray): Boolean {
        return nativeSendData(nativePtr, data)
    }
    
    fun receiveData(maxSize: Int = 1024): ByteArray? {
        return nativeReceiveData(nativePtr, maxSize)
    }
    
    fun startStreaming(): Boolean {
        return nativeStartStreaming(nativePtr)
    }
    
    fun stopStreaming() {
        nativeStopStreaming(nativePtr)
    }
    
    fun getDeviceInfo(): String {
        return nativeGetDeviceInfo(nativePtr)
    }
    
    fun setCallback(callback: DriveCallback) {
        this.callback = callback
        nativeSetCallback(nativePtr, callback)
    }
    
    // Implementação do callback
    fun onDriveEvent(event: String, data: ByteArray, timestamp: Long) {
        callback?.onDriveEvent(event, data, timestamp)
    }
    
    // Native methods
    private external fun nativeInit(): Long
    private external fun nativeDestroy(ptr: Long)
    private external fun nativeConnect(ptr: Long): Boolean
    private external fun nativeDisconnect(ptr: Long)
    private external fun nativeSendData(ptr: Long, data: ByteArray): Boolean
    private external fun nativeReceiveData(ptr: Long, maxSize: Int): ByteArray?
    private external fun nativeStartStreaming(ptr: Long): Boolean
    private external fun nativeStopStreaming(ptr: Long)
    private external fun nativeGetDeviceInfo(ptr: Long): String
    private external fun nativeSetCallback(ptr: Long, callback: DriveCallback)
}
```

### 3. Uso na Activity (MainActivity.kt)
```kotlin
class MainActivity : AppCompatActivity(), DriveCallback {
    private lateinit var binding: ActivityMainBinding
    private lateinit var nativeWrapper: NativeWrapper
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // Inicializar wrapper nativo
        nativeWrapper = NativeWrapper()
        nativeWrapper.setCallback(this)
        
        setupUI()
    }
    
    private fun setupUI() {
        binding.connectButton.setOnClickListener {
            if (nativeWrapper.connect()) {
                binding.statusText.text = "Conectado"
                binding.connectButton.isEnabled = false
                binding.disconnectButton.isEnabled = true
            } else {
                binding.statusText.text = "Erro ao conectar"
            }
        }
        
        binding.disconnectButton.setOnClickListener {
            nativeWrapper.disconnect()
            binding.statusText.text = "Desconectado"
            binding.connectButton.isEnabled = true
            binding.disconnectButton.isEnabled = false
        }
        
        binding.sendButton.setOnClickListener {
            val data = "Hello from Android!".toByteArray()
            if (nativeWrapper.sendData(data)) {
                binding.logText.append("Dados enviados: ${data.size} bytes\n")
            } else {
                binding.logText.append("Erro ao enviar dados\n")
            }
        }
        
        binding.receiveButton.setOnClickListener {
            val data = nativeWrapper.receiveData()
            if (data != null) {
                binding.logText.append("Dados recebidos: ${data.size} bytes\n")
            } else {
                binding.logText.append("Nenhum dado recebido\n")
            }
        }
        
        binding.streamButton.setOnClickListener {
            if (nativeWrapper.startStreaming()) {
                binding.logText.append("Streaming iniciado\n")
            } else {
                binding.logText.append("Erro ao iniciar streaming\n")
            }
        }
        
        binding.stopStreamButton.setOnClickListener {
            nativeWrapper.stopStreaming()
            binding.logText.append("Streaming parado\n")
        }
        
        binding.infoButton.setOnClickListener {
            val info = nativeWrapper.getDeviceInfo()
            binding.logText.append("Info: $info\n")
        }
    }
    
    // Implementação do callback
    override fun onDriveEvent(event: String, data: ByteArray, timestamp: Long) {
        runOnUiThread {
            binding.logText.append("Evento: $event, Dados: ${data.size} bytes\n")
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        nativeWrapper.destroy()
    }
}
```

---

## 🔨 Build System

### 1. Gradle Configuration (build.gradle)
```gradle
android {
    compileSdk 33
    
    defaultConfig {
        applicationId "com.example.app"
        minSdk 21
        targetSdk 33
        versionCode 1
        versionName "1.0"
        
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
    
    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
    
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.22.1"
        }
    }
    
    ndkVersion "25.1.8937393"
    
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
    
    kotlinOptions {
        jvmTarget = '1.8'
    }
    
    buildFeatures {
        viewBinding true
    }
}
```

### 2. CMake Advanced (CMakeLists.txt)
```cmake
cmake_minimum_required(VERSION 3.22.1)
project("nativeapp")

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurações de debug/release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")
endif()

# Incluir diretórios
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/drive)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Definir fontes
set(DRIVE_SOURCES
    drive/drive.cpp
)

set(UTILS_SOURCES
    utils/logger.cpp
    utils/memory.cpp
)

set(JNI_SOURCES
    native_lib.cpp
)

# Criar biblioteca estática para drive
add_library(drive STATIC ${DRIVE_SOURCES})

# Configurações para drive
target_compile_definitions(drive PRIVATE
    ANDROID=1
    __ANDROID__=1
)

target_compile_options(drive PRIVATE
    -Wall
    -Wextra
    -fPIC
)

# Criar biblioteca estática para utils
add_library(utils STATIC ${UTILS_SOURCES})

target_compile_definitions(utils PRIVATE
    ANDROID=1
    __ANDROID__=1
)

target_compile_options(utils PRIVATE
    -Wall
    -Wextra
    -fPIC
)

# Criar biblioteca compartilhada principal
add_library(nativeapp SHARED ${JNI_SOURCES})

# Configurações para biblioteca principal
target_compile_definitions(nativeapp PRIVATE
    ANDROID=1
    __ANDROID__=1
)

target_compile_options(nativeapp PRIVATE
    -Wall
    -Wextra
    -fPIC
    -O2
)

# Linkar bibliotecas
target_link_libraries(nativeapp
    drive
    utils
    android
    log
    m
    c
)

# Configurações específicas do Android
set_target_properties(nativeapp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}
)

# Instalar bibliotecas
install(TARGETS nativeapp
    LIBRARY DESTINATION lib/${ANDROID_ABI}
)
```

---

## 🐛 Debugging e Performance

### 1. Logging Avançado
```cpp
// utils/logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <android/log.h>
#include <string>

class Logger {
public:
    enum Level {
        VERBOSE = ANDROID_LOG_VERBOSE,
        DEBUG = ANDROID_LOG_DEBUG,
        INFO = ANDROID_LOG_INFO,
        WARN = ANDROID_LOG_WARN,
        ERROR = ANDROID_LOG_ERROR
    };
    
    static void log(Level level, const char* tag, const char* format, ...);
    static void log(Level level, const char* tag, const std::string& message);
    
    static void hexdump(const char* tag, const uint8_t* data, size_t size);
};

#define LOG_V(tag, ...) Logger::log(Logger::VERBOSE, tag, __VA_ARGS__)
#define LOG_D(tag, ...) Logger::log(Logger::DEBUG, tag, __VA_ARGS__)
#define LOG_I(tag, ...) Logger::log(Logger::INFO, tag, __VA_ARGS__)
#define LOG_W(tag, ...) Logger::log(Logger::WARN, tag, __VA_ARGS__)
#define LOG_E(tag, ...) Logger::log(Logger::ERROR, tag, __VA_ARGS__)

#endif // LOGGER_H
```

### 2. Performance Monitoring
```cpp
// utils/performance.h
#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <chrono>
#include <string>

class PerformanceMonitor {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string operation_name;
    
public:
    PerformanceMonitor(const std::string& name) : operation_name(name) {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    ~PerformanceMonitor() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        LOGI("Performance: %s took %lld us", operation_name.c_str(), duration.count());
    }
};

#define PERF_MONITOR(name) PerformanceMonitor perf_monitor(name)

#endif // PERFORMANCE_H
```

### 3. Memory Management
```cpp
// utils/memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <memory>
#include <vector>

template<typename T>
class SafeBuffer {
private:
    std::unique_ptr<T[]> buffer;
    size_t size_;
    
public:
    SafeBuffer(size_t size) : size_(size) {
        buffer = std::make_unique<T[]>(size);
    }
    
    T* data() { return buffer.get(); }
    const T* data() const { return buffer.get(); }
    size_t size() const { return size_; }
    
    void resize(size_t new_size) {
        buffer = std::make_unique<T[]>(new_size);
        size_ = new_size;
    }
};

#endif // MEMORY_H
```

---

## 📝 Exemplos Práticos

### 1. Exemplo de Drive Serial
```cpp
// drive/serial_drive.h
class SerialDrive : public NativeDrive {
private:
    int serial_fd;
    std::string port_path;
    int baud_rate;
    
public:
    SerialDrive(const std::string& port, int baud = 9600);
    ~SerialDrive();
    
    bool connect() override;
    void disconnect() override;
    bool send_data(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> receive_data(size_t max_size = 1024) override;
};
```

### 2. Exemplo de Drive USB
```cpp
// drive/usb_drive.h
class UsbDrive : public NativeDrive {
private:
    int usb_fd;
    uint16_t vendor_id;
    uint16_t product_id;
    
public:
    UsbDrive(uint16_t vid, uint16_t pid);
    ~UsbDrive();
    
    bool connect() override;
    void disconnect() override;
    bool send_data(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> receive_data(size_t max_size = 1024) override;
};
```

### 3. Exemplo de Drive Bluetooth
```cpp
// drive/bluetooth_drive.h
class BluetoothDrive : public NativeDrive {
private:
    int bt_socket;
    std::string device_address;
    
public:
    BluetoothDrive(const std::string& address);
    ~BluetoothDrive();
    
    bool connect() override;
    void disconnect() override;
    bool send_data(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> receive_data(size_t max_size = 1024) override;
};
```

---

## 🔧 Troubleshooting

### Problemas Comuns

#### 1. **Erro de Compilação JNI**
```
error: undefined reference to 'JNI_OnLoad'
```
**Solução**: Verificar se a função JNI_OnLoad está definida corretamente.

#### 2. **Erro de Linkagem**
```
error: cannot find -lnativeapp
```
**Solução**: Verificar se a biblioteca está sendo compilada corretamente.

#### 3. **Erro de Permissão**
```
Permission denied
```
**Solução**: Verificar permissões no AndroidManifest.xml.

#### 4. **Crash na Execução**
```
SIGSEGV
```
**Solução**: Verificar ponteiros nulos e gerenciamento de memória.

### Comandos de Debug
```bash
# Verificar bibliotecas carregadas
adb shell cat /proc/$(adb shell pidof com.example.app)/maps | grep nativeapp

# Capturar logs nativos
adb logcat -s NativeDrive:V

# Verificar permissões
adb shell dumpsys package com.example.app | grep permission

# Testar biblioteca
adb shell "cd /data/local/tmp && ./test_native"
```

---

## 🎯 Conclusão

Esta integração C++ com drives nativos para Android oferece:

### ✅ **Vantagens:**
- **Performance**: Código nativo mais rápido
- **Flexibilidade**: Acesso direto a hardware
- **Reutilização**: Bibliotecas C++ existentes
- **Controle**: Gerenciamento fino de recursos

### 🔧 **Aspectos Técnicos:**
- **JNI**: Ponte robusta Java-C++
- **CMake**: Build system moderno
- **Threading**: Operações assíncronas
- **Memory Management**: Gerenciamento seguro

### 📈 **Aplicações:**
- Drivers de hardware
- Processamento de sinais
- Comunicação serial/USB/Bluetooth
- Algoritmos de alta performance

A integração C++ é essencial para aplicações Android que precisam de performance e acesso direto a hardware! 🚀📱 