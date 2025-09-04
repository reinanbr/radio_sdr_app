#include <jni.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <vector>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>

#define LOG_TAG "SDRRadio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Estrutura para dados de áudio
struct AudioData {
    std::vector<float> samples;
    double frequency;
    double signal_strength;
    long timestamp;
};

// Estrutura para configuração do SDR
struct SDRConfig {
    uint32_t sample_rate;
    uint32_t center_freq;
    int gain;
    int ppm_error;
    bool auto_gain;
};

class SDRRadio {
private:
    SDRConfig config;
    bool is_connected;
    bool is_scanning;
    bool is_recording;
    
    // Threads
    pthread_t scan_thread;
    pthread_t audio_thread;
    pthread_mutex_t data_mutex;
    
    // Data buffers
    std::queue<AudioData> audio_queue;
    std::vector<float> frequency_spectrum;
    
    // Callbacks
    jobject java_callback;
    JavaVM* jvm;
    
    // Static callback functions
    static void* scan_thread_func(void* arg);
    static void* audio_thread_func(void* arg);
    
    // Audio processing
    void update_frequency_spectrum();
    void save_audio_data(const AudioData& data);
    void process_audio_data(const std::vector<uint8_t>& rf_data);
    std::vector<float> demodulate_am(const std::vector<uint8_t>& rf_data);
    std::vector<float> demodulate_fm(const std::vector<uint8_t>& rf_data);
    
public:
    SDRRadio();
    ~SDRRadio();
    
    // Device management
    bool connect_device();
    void disconnect_device();
    bool is_device_connected() const { return is_connected; }
    
    // Configuration
    bool set_sample_rate(uint32_t rate);
    bool set_center_frequency(uint32_t freq);
    bool set_gain(int gain);
    bool set_auto_gain(bool enabled);
    bool set_ppm_error(int ppm);
    
    // Scanning and recording
    bool start_scanning();
    void stop_scanning();
    bool start_recording();
    void stop_recording();
    
    // Data access
    std::vector<float> get_frequency_spectrum();
    double get_current_frequency();
    double get_signal_strength();
    
    // Java interface
    void set_java_callback(JNIEnv* env, jobject callback);
    void notify_java_callback(const char* event, const char* data);
};

SDRRadio::SDRRadio() : is_connected(false), is_scanning(false), is_recording(false), java_callback(nullptr), jvm(nullptr) {
    // Inicializar configuração padrão
    config.sample_rate = 2048000;  // 2.048 MHz
    config.center_freq = 100000000; // 100 MHz
    config.gain = 0;
    config.ppm_error = 0;
    config.auto_gain = true;
    
    // Inicializar mutex
    pthread_mutex_init(&data_mutex, nullptr);
    
    // Inicializar espectro de frequência
    frequency_spectrum.resize(1024, 0.0f);
    
    LOGI("SDRRadio initialized");
}

SDRRadio::~SDRRadio() {
    disconnect_device();
    pthread_mutex_destroy(&data_mutex);
    LOGI("SDRRadio destroyed");
}

bool SDRRadio::connect_device() {
    if (is_connected) {
        LOGI("Device already connected");
        return true;
    }
    
    // Simular conexão bem-sucedida
    is_connected = true;
    LOGI("RTL-SDR device connected successfully (simulated)");
    return true;
}

void SDRRadio::disconnect_device() {
    if (!is_connected) return;
    
    stop_scanning();
    stop_recording();
    
    is_connected = false;
    LOGI("RTL-SDR device disconnected");
}

bool SDRRadio::start_scanning() {
    if (!is_connected) {
        LOGE("Device not connected");
        return false;
    }
    
    if (is_scanning) {
        LOGI("Already scanning");
        return true;
    }
    
    is_scanning = true;
    
    // Iniciar thread de escaneamento
    if (pthread_create(&scan_thread, nullptr, scan_thread_func, this) != 0) {
        LOGE("Failed to create scan thread");
        is_scanning = false;
        return false;
    }
    
    LOGI("Started frequency scanning");
    return true;
}

void SDRRadio::stop_scanning() {
    if (!is_scanning) return;
    
    is_scanning = false;
    
    if (scan_thread) {
        pthread_join(scan_thread, nullptr);
    }
    
    LOGI("Stopped frequency scanning");
}

bool SDRRadio::start_recording() {
    if (!is_connected) {
        LOGE("Device not connected");
        return false;
    }
    
    if (is_recording) {
        LOGI("Already recording");
        return true;
    }
    
    is_recording = true;
    
    // Iniciar thread de áudio
    if (pthread_create(&audio_thread, nullptr, audio_thread_func, this) != 0) {
        LOGE("Failed to create audio thread");
        is_recording = false;
        return false;
    }
    
    LOGI("Started audio recording");
    return true;
}

void SDRRadio::stop_recording() {
    if (!is_recording) return;
    
    is_recording = false;
    
    if (audio_thread) {
        pthread_join(audio_thread, nullptr);
    }
    
    LOGI("Stopped audio recording");
}

void* SDRRadio::scan_thread_func(void* arg) {
    SDRRadio* radio = static_cast<SDRRadio*>(arg);
    
    // Escaneamento de frequências de 24 MHz a 1766 MHz
    uint32_t start_freq = 24000000;  // 24 MHz
    uint32_t end_freq = 1766000000;  // 1766 MHz
    uint32_t step_freq = 1000000;    // 1 MHz step
    
    for (uint32_t freq = start_freq; freq <= end_freq && radio->is_scanning; freq += step_freq) {
        radio->set_center_frequency(freq);
        
        // Aguardar um pouco para estabilizar
        usleep(100000); // 100ms
        
        // Simular força do sinal
        double signal = -50.0 + (rand() % 50); // -50 a 0 dB
        
        // Notificar Java sobre nova frequência
        std::ostringstream oss;
        oss << "{\"frequency\":" << freq << ",\"signal\":" << signal << "}";
        radio->notify_java_callback("frequency_update", oss.str().c_str());
    }
    
    return nullptr;
}

void* SDRRadio::audio_thread_func(void* arg) {
    SDRRadio* radio = static_cast<SDRRadio*>(arg);
    
    while (radio->is_recording) {
        // Simular dados RF recebidos
        std::vector<uint8_t> rf_data;
        rf_data.reserve(2048);
        
        // Gerar dados RF simulados (I/Q samples)
        for (int i = 0; i < 2048; i += 2) {
            // Simular portadora AM com modulação de áudio
            float time_val = (float)i / 2048.0f;
            float audio_signal = 0.3f * sinf(2.0f * M_PI * 440.0f * time_val); // 440 Hz
            float carrier = 0.7f + 0.3f * audio_signal; // AM modulation
            
            // Converter para I/Q
            float i_sample = carrier * cosf(2.0f * M_PI * 1000.0f * time_val);
            float q_sample = carrier * sinf(2.0f * M_PI * 1000.0f * time_val);
            
            // Adicionar ruído
            i_sample += 0.1f * ((float)rand() / RAND_MAX - 0.5f);
            q_sample += 0.1f * ((float)rand() / RAND_MAX - 0.5f);
            
            // Converter para bytes
            rf_data.push_back((uint8_t)(i_sample * 127.0f + 128.0f));
            rf_data.push_back((uint8_t)(q_sample * 127.0f + 128.0f));
        }
        
        // Processar dados RF em áudio
        radio->process_audio_data(rf_data);
        
        usleep(50000); // 50ms
    }
    
    return nullptr;
}

void SDRRadio::save_audio_data(const AudioData& data) {
    // Gerar nome do arquivo com timestamp
    std::ostringstream filename;
    filename << "/sdcard/sdr_recordings/";
    filename << "sdr_" << data.timestamp << "_";
    filename << std::fixed << std::setprecision(3) << (data.frequency / 1000000.0) << "MHz.wav";
    
    // Criar diretório se não existir
    system("mkdir -p /sdcard/sdr_recordings");
    
    // Salvar como arquivo WAV simples
    std::ofstream file(filename.str(), std::ios::binary);
    if (file.is_open()) {
        // Header WAV básico
        const char* riff = "RIFF";
        const char* wave = "WAVE";
        const char* fmt = "fmt ";
        const char* data_chunk = "data";
        
        uint32_t file_size = 36 + data.samples.size() * sizeof(float);
        uint32_t fmt_size = 16;
        uint16_t audio_format = 3; // IEEE float
        uint16_t num_channels = 1;
        uint32_t sample_rate = 48000;
        uint16_t bits_per_sample = 32;
        uint32_t byte_rate = sample_rate * num_channels * bits_per_sample / 8;
        uint16_t block_align = num_channels * bits_per_sample / 8;
        uint32_t data_size = data.samples.size() * sizeof(float);
        
        file.write(riff, 4);
        file.write(reinterpret_cast<const char*>(&file_size), 4);
        file.write(wave, 4);
        file.write(fmt, 4);
        file.write(reinterpret_cast<const char*>(&fmt_size), 4);
        file.write(reinterpret_cast<const char*>(&audio_format), 2);
        file.write(reinterpret_cast<const char*>(&num_channels), 2);
        file.write(reinterpret_cast<const char*>(&sample_rate), 4);
        file.write(reinterpret_cast<const char*>(&byte_rate), 4);
        file.write(reinterpret_cast<const char*>(&block_align), 2);
        file.write(reinterpret_cast<const char*>(&bits_per_sample), 2);
        file.write(data_chunk, 4);
        file.write(reinterpret_cast<const char*>(&data_size), 4);
        
        // Dados de áudio
        file.write(reinterpret_cast<const char*>(data.samples.data()), data.samples.size() * sizeof(float));
        
        file.close();
        
        LOGI("Saved audio to %s", filename.str().c_str());
    }
}

void SDRRadio::process_audio_data(const std::vector<uint8_t>& rf_data) {
    if (rf_data.empty()) return;
    
    // Demodular os dados RF em áudio
    std::vector<float> audio_samples;
    
    // Tentar demodulação AM primeiro (mais comum para rádio)
    audio_samples = demodulate_am(rf_data);
    
    // Se não houver sinal AM, tentar FM
    if (audio_samples.empty()) {
        audio_samples = demodulate_fm(rf_data);
    }
    
    if (!audio_samples.empty()) {
        // Criar estrutura de dados de áudio
        AudioData audio_data;
        audio_data.samples = audio_samples;
        audio_data.frequency = config.center_freq;
        audio_data.signal_strength = get_signal_strength();
        audio_data.timestamp = time(nullptr);
        
        // Adicionar à fila de áudio
        pthread_mutex_lock(&data_mutex);
        audio_queue.push(audio_data);
        pthread_mutex_unlock(&data_mutex);
        
        LOGI("Processed audio data: %zu samples", audio_samples.size());
    }
}

std::vector<float> SDRRadio::demodulate_am(const std::vector<uint8_t>& rf_data) {
    std::vector<float> audio_samples;
    audio_samples.reserve(rf_data.size() / 2);
    
    // Demodulação AM simples (envelope detection)
    for (size_t i = 0; i < rf_data.size() - 1; i += 2) {
        // Converter bytes para valores I/Q
        float i_val = (float)((int8_t)rf_data[i]) / 128.0f;
        float q_val = (float)((int8_t)rf_data[i + 1]) / 128.0f;
        
        // Calcular magnitude (envelope)
        float magnitude = sqrtf(i_val * i_val + q_val * q_val);
        
        // Aplicar filtro passa-baixa simples (média móvel)
        static float prev_sample = 0.0f;
        float filtered = 0.9f * prev_sample + 0.1f * magnitude;
        prev_sample = filtered;
        
        // Normalizar e adicionar à saída
        float audio_sample = (filtered - 0.5f) * 2.0f; // Centralizar em 0
        audio_sample = std::max(-1.0f, std::min(1.0f, audio_sample)); // Clipping
        
        audio_samples.push_back(audio_sample);
    }
    
    return audio_samples;
}

std::vector<float> SDRRadio::demodulate_fm(const std::vector<uint8_t>& rf_data) {
    std::vector<float> audio_samples;
    audio_samples.reserve(rf_data.size() / 2);
    
    // Demodulação FM simples (discriminador de frequência)
    float prev_phase = 0.0f;
    
    for (size_t i = 0; i < rf_data.size() - 1; i += 2) {
        // Converter bytes para valores I/Q
        float i_val = (float)((int8_t)rf_data[i]) / 128.0f;
        float q_val = (float)((int8_t)rf_data[i + 1]) / 128.0f;
        
        // Calcular fase atual
        float phase = atan2f(q_val, i_val);
        
        // Calcular diferença de fase
        float phase_diff = phase - prev_phase;
        
        // Normalizar diferença de fase
        while (phase_diff > M_PI) phase_diff -= 2 * M_PI;
        while (phase_diff < -M_PI) phase_diff += 2 * M_PI;
        
        // Converter diferença de fase em áudio
        float audio_sample = phase_diff * 0.5f; // Escalar para áudio
        
        // Aplicar filtro passa-baixa
        static float prev_sample = 0.0f;
        float filtered = 0.9f * prev_sample + 0.1f * audio_sample;
        prev_sample = filtered;
        
        // Clipping
        filtered = std::max(-1.0f, std::min(1.0f, filtered));
        
        audio_samples.push_back(filtered);
        prev_phase = phase;
    }
    
    return audio_samples;
}

bool SDRRadio::set_sample_rate(uint32_t rate) {
    if (!is_connected) return false;
    config.sample_rate = rate;
    return true;
}

bool SDRRadio::set_center_frequency(uint32_t freq) {
    if (!is_connected) return false;
    config.center_freq = freq;
    return true;
}

bool SDRRadio::set_gain(int gain) {
    if (!is_connected) return false;
    config.gain = gain;
    config.auto_gain = false;
    return true;
}

bool SDRRadio::set_auto_gain(bool enabled) {
    if (!is_connected) return false;
    config.auto_gain = enabled;
    return true;
}

bool SDRRadio::set_ppm_error(int ppm) {
    if (!is_connected) return false;
    config.ppm_error = ppm;
    return true;
}

std::vector<float> SDRRadio::get_frequency_spectrum() {
    pthread_mutex_lock(&data_mutex);
    std::vector<float> spectrum = frequency_spectrum;
    pthread_mutex_unlock(&data_mutex);
    return spectrum;
}

double SDRRadio::get_current_frequency() {
    return config.center_freq;
}

double SDRRadio::get_signal_strength() {
    pthread_mutex_lock(&data_mutex);
    double strength = -50.0 + (rand() % 50); // Simular força do sinal
    pthread_mutex_unlock(&data_mutex);
    return strength;
}

void SDRRadio::set_java_callback(JNIEnv* env, jobject callback) {
    if (java_callback) {
        env->DeleteGlobalRef(java_callback);
    }
    
    java_callback = env->NewGlobalRef(callback);
    env->GetJavaVM(&jvm);
}

void SDRRadio::notify_java_callback(const char* event, const char* data) {
    if (!java_callback || !jvm) return;
    
    JNIEnv* env;
    if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        return;
    }
    
    // Encontrar classe e método de callback
    jclass callback_class = env->GetObjectClass(java_callback);
    jmethodID method_id = env->GetMethodID(callback_class, "onSDRCallback", "(Ljava/lang/String;Ljava/lang/String;)V");
    
    if (method_id) {
        jstring event_str = env->NewStringUTF(event);
        jstring data_str = env->NewStringUTF(data);
        env->CallVoidMethod(java_callback, method_id, event_str, data_str);
        env->DeleteLocalRef(event_str);
        env->DeleteLocalRef(data_str);
    }
    
    env->DeleteLocalRef(callback_class);
    jvm->DetachCurrentThread();
}

// JNI implementations
extern "C" {

JNIEXPORT jlong JNICALL Java_com_sdrradio_kt_SDRRadio_nativeInit(JNIEnv* env, jobject thiz) {
    SDRRadio* radio = new SDRRadio();
    return reinterpret_cast<jlong>(radio);
}

JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeDestroy(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        delete radio;
    }
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeConnect(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->connect_device();
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeDisconnect(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        radio->disconnect_device();
    }
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartScanning(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->start_scanning();
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStopScanning(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        radio->stop_scanning();
    }
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartRecording(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->start_recording();
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStopRecording(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        radio->stop_recording();
    }
}

JNIEXPORT jdouble JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetFrequency(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->get_current_frequency();
    }
    return 0.0;
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeSetFrequency(JNIEnv* env, jobject thiz, jlong ptr, jlong frequencyHz) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->set_center_frequency(frequencyHz);
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartAudioPlayback(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        // Simular início da reprodução de áudio
        LOGI("Starting audio playback");
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStopAudioPlayback(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        // Simular parada da reprodução de áudio
        LOGI("Stopping audio playback");
    }
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeSetAudioVolume(JNIEnv* env, jobject thiz, jlong ptr, jfloat volume) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        // Simular definição de volume
        LOGI("Setting audio volume to %.2f", volume);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeSetModulation(JNIEnv* env, jobject thiz, jlong ptr, jstring modulation) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        const char* mod_str = env->GetStringUTFChars(modulation, nullptr);
        LOGI("Setting modulation to %s", mod_str);
        env->ReleaseStringUTFChars(modulation, mod_str);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jdouble JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetSignalStrength(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->get_signal_strength();
    }
    return -100.0;
}

JNIEXPORT jfloatArray JNICALL Java_com_sdrradio_kt_SDRRadio_nativeGetSpectrum(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        std::vector<float> spectrum = radio->get_frequency_spectrum();
        jfloatArray result = env->NewFloatArray(spectrum.size());
        env->SetFloatArrayRegion(result, 0, spectrum.size(), spectrum.data());
        return result;
    }
    return nullptr;
}

} 