#include "sdr_radio.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>

SDRRadio::SDRRadio() : device(nullptr), is_connected(false), is_scanning(false), is_recording(false), java_callback(nullptr), jvm(nullptr) {
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
    
    int device_count = rtlsdr_get_device_count();
    if (device_count == 0) {
        LOGE("No RTL-SDR devices found");
        return false;
    }
    
    LOGI("Found %d RTL-SDR device(s)", device_count);
    
    // Tentar abrir o primeiro dispositivo
    int result = rtlsdr_open(&device, 0);
    if (result != 0) {
        LOGE("Failed to open RTL-SDR device: %d", result);
        return false;
    }
    
    // Configurar dispositivo
    if (rtlsdr_set_sample_rate(device, config.sample_rate) != 0) {
        LOGE("Failed to set sample rate");
        rtlsdr_close(device);
        device = nullptr;
        return false;
    }
    
    if (rtlsdr_set_center_freq(device, config.center_freq) != 0) {
        LOGE("Failed to set center frequency");
        rtlsdr_close(device);
        device = nullptr;
        return false;
    }
    
    if (config.auto_gain) {
        if (rtlsdr_set_tuner_gain_mode(device, 0) != 0) {
            LOGE("Failed to enable auto gain");
        }
    } else {
        if (rtlsdr_set_tuner_gain_mode(device, 1) != 0) {
            LOGE("Failed to set manual gain mode");
        } else if (rtlsdr_set_tuner_gain(device, config.gain) != 0) {
            LOGE("Failed to set gain");
        }
    }
    
    if (config.ppm_error != 0) {
        if (rtlsdr_set_freq_correction(device, config.ppm_error) != 0) {
            LOGE("Failed to set frequency correction");
        }
    }
    
    // Configurar callback
    if (rtlsdr_read_async(device, rtlsdr_callback, this, 0, 0) != 0) {
        LOGE("Failed to set async callback");
        rtlsdr_close(device);
        device = nullptr;
        return false;
    }
    
    is_connected = true;
    LOGI("RTL-SDR device connected successfully");
    return true;
}

void SDRRadio::disconnect_device() {
    if (!is_connected) return;
    
    stop_scanning();
    stop_recording();
    
    if (device) {
        rtlsdr_cancel_async(device);
        rtlsdr_close(device);
        device = nullptr;
    }
    
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

void SDRRadio::rtlsdr_callback(unsigned char* buf, uint32_t len, void* ctx) {
    SDRRadio* radio = static_cast<SDRRadio*>(ctx);
    if (radio) {
        radio->process_audio_data(buf, len);
    }
}

void SDRRadio::process_audio_data(unsigned char* buf, uint32_t len) {
    pthread_mutex_lock(&data_mutex);
    
    // Converter dados I/Q para float
    std::vector<float> samples;
    samples.reserve(len / 2);
    
    for (uint32_t i = 0; i < len; i += 2) {
        float i_sample = (buf[i] - 128.0f) / 128.0f;
        float q_sample = (buf[i + 1] - 128.0f) / 128.0f;
        float magnitude = std::sqrt(i_sample * i_sample + q_sample * q_sample);
        samples.push_back(magnitude);
    }
    
    // Criar dados de áudio
    AudioData audio_data;
    audio_data.samples = samples;
    audio_data.frequency = config.center_freq;
    audio_data.timestamp = time(nullptr);
    
    // Calcular força do sinal
    float sum = 0.0f;
    for (float sample : samples) {
        sum += sample;
    }
    audio_data.signal_strength = 20.0 * std::log10(sum / samples.size());
    
    // Adicionar à fila
    audio_queue.push(audio_data);
    
    // Manter apenas os últimos 100 frames
    while (audio_queue.size() > 100) {
        audio_queue.pop();
    }
    
    // Atualizar espectro de frequência
    update_frequency_spectrum();
    
    pthread_mutex_unlock(&data_mutex);
}

void SDRRadio::update_frequency_spectrum() {
    if (audio_queue.empty()) return;
    
    const AudioData& latest = audio_queue.back();
    
    // Simular espectro de frequência (em uma implementação real, usar FFT)
    for (size_t i = 0; i < frequency_spectrum.size(); ++i) {
        float freq_offset = (i - frequency_spectrum.size() / 2.0f) / frequency_spectrum.size();
        float signal = latest.signal_strength * std::exp(-freq_offset * freq_offset * 100.0f);
        frequency_spectrum[i] = signal;
    }
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
        
        // Notificar Java sobre nova frequência
        std::ostringstream oss;
        oss << "{\"frequency\":" << freq << ",\"signal\":" << radio->get_signal_strength() << "}";
        radio->notify_java_callback("frequency_update", oss.str().c_str());
    }
    
    return nullptr;
}

void* SDRRadio::audio_thread_func(void* arg) {
    SDRRadio* radio = static_cast<SDRRadio*>(arg);
    
    while (radio->is_recording) {
        pthread_mutex_lock(&radio->data_mutex);
        
        if (!radio->audio_queue.empty()) {
            const AudioData& data = radio->audio_queue.front();
            radio->save_audio_data(data);
            radio->audio_queue.pop();
        }
        
        pthread_mutex_unlock(&radio->data_mutex);
        
        usleep(10000); // 10ms
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
        const char* data = "data";
        
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
        file.write(data, 4);
        file.write(reinterpret_cast<const char*>(&data_size), 4);
        
        // Dados de áudio
        file.write(reinterpret_cast<const char*>(data.samples.data()), data.samples.size() * sizeof(float));
        
        file.close();
        
        LOGI("Saved audio to %s", filename.str().c_str());
    }
}

bool SDRRadio::set_sample_rate(uint32_t rate) {
    if (!is_connected) return false;
    
    if (rtlsdr_set_sample_rate(device, rate) == 0) {
        config.sample_rate = rate;
        return true;
    }
    return false;
}

bool SDRRadio::set_center_frequency(uint32_t freq) {
    if (!is_connected) return false;
    
    if (rtlsdr_set_center_freq(device, freq) == 0) {
        config.center_freq = freq;
        return true;
    }
    return false;
}

bool SDRRadio::set_gain(int gain) {
    if (!is_connected) return false;
    
    if (rtlsdr_set_tuner_gain_mode(device, 1) == 0 && 
        rtlsdr_set_tuner_gain(device, gain) == 0) {
        config.gain = gain;
        config.auto_gain = false;
        return true;
    }
    return false;
}

bool SDRRadio::set_auto_gain(bool enabled) {
    if (!is_connected) return false;
    
    if (rtlsdr_set_tuner_gain_mode(device, enabled ? 0 : 1) == 0) {
        config.auto_gain = enabled;
        return true;
    }
    return false;
}

bool SDRRadio::set_ppm_error(int ppm) {
    if (!is_connected) return false;
    
    if (rtlsdr_set_freq_correction(device, ppm) == 0) {
        config.ppm_error = ppm;
        return true;
    }
    return false;
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
    double strength = -100.0; // Default weak signal
    
    if (!audio_queue.empty()) {
        strength = audio_queue.back().signal_strength;
    }
    
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