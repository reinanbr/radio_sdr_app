#include <android/log.h>
#include <cstring>
#include <cmath>
#include <cstdlib>

// Definições de log
#define LOG_TAG "RTL-SDR-Sim"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Estrutura simulada do dispositivo RTL-SDR
struct rtlsdr_dev {
    bool is_open;
    double frequency;
    int sample_rate;
    float gain;
    bool auto_gain;
    int bandwidth;
    bool streaming;
    int device_index;
    char serial[256];
    char manufacturer[256];
    char product[256];
};

// Variáveis globais para simulação
static int device_count = 1;
static bool initialized = false;

// Funções simuladas da biblioteca RTL-SDR
extern "C" {

int rtlsdr_open(rtlsdr_dev** dev, int index) {
    if (!initialized) {
        LOGI("Initializing simulated RTL-SDR library");
        initialized = true;
    }
    
    if (index >= device_count) {
        LOGE("Device index %d out of range", index);
        return -1;
    }
    
    *dev = new rtlsdr_dev();
    (*dev)->is_open = true;
    (*dev)->frequency = 100.0e6; // 100 MHz padrão
    (*dev)->sample_rate = 2048000; // 2.048 MHz padrão
    (*dev)->gain = 20.0;
    (*dev)->auto_gain = true;
    (*dev)->bandwidth = 2500000; // 2.5 MHz padrão
    (*dev)->streaming = false;
    (*dev)->device_index = index;
    
    strcpy((*dev)->serial, "00000001");
    strcpy((*dev)->manufacturer, "RTL-SDR");
    strcpy((*dev)->product, "RTL2838");
    
    LOGI("Simulated RTL-SDR device %d opened", index);
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

int rtlsdr_get_device_count() {
    return device_count;
}

int rtlsdr_get_device_name(uint32_t index, char* manufacturer, char* product, char* serial) {
    if (index >= device_count) {
        return -1;
    }
    
    strcpy(manufacturer, "RTL-SDR");
    strcpy(product, "RTL2838");
    strcpy(serial, "00000001");
    
    return 0;
}

int rtlsdr_set_center_freq(rtlsdr_dev* dev, uint32_t freq) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->frequency = freq;
    LOGI("Frequency set to %u Hz (%.1f MHz)", freq, freq / 1e6);
    return 0;
}

uint32_t rtlsdr_get_center_freq(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return 0;
    }
    
    return static_cast<uint32_t>(dev->frequency);
}

int rtlsdr_set_sample_rate(rtlsdr_dev* dev, uint32_t rate) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->sample_rate = rate;
    LOGI("Sample rate set to %u Hz", rate);
    return 0;
}

uint32_t rtlsdr_get_sample_rate(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return 0;
    }
    
    return dev->sample_rate;
}

int rtlsdr_set_tuner_gain(rtlsdr_dev* dev, int gain) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->gain = gain / 10.0f; // Converter de 0.1 dB para dB
    dev->auto_gain = false;
    LOGI("Gain set to %.1f dB", dev->gain);
    return 0;
}

int rtlsdr_get_tuner_gain(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return 0;
    }
    
    return static_cast<int>(dev->gain * 10.0f);
}

int rtlsdr_set_tuner_gain_mode(rtlsdr_dev* dev, int mode) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->auto_gain = (mode == 1);
    LOGI("Gain mode set to %s", dev->auto_gain ? "auto" : "manual");
    return 0;
}

int rtlsdr_set_bandwidth(rtlsdr_dev* dev, uint32_t bw) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->bandwidth = bw;
    LOGI("Bandwidth set to %u Hz", bw);
    return 0;
}

uint32_t rtlsdr_get_bandwidth(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return 0;
    }
    
    return dev->bandwidth;
}

int rtlsdr_read_sync(rtlsdr_dev* dev, void* buf, int len, int* n_read) {
    if (!dev || !dev->is_open || !dev->streaming) {
        return -1;
    }
    
    uint8_t* buffer = static_cast<uint8_t*>(buf);
    
    // Simular dados I/Q com ruído e sinal
    static float time_offset = 0.0f;
    float time_step = 1.0f / dev->sample_rate;
    
    for (int i = 0; i < len; i += 2) {
        float time = time_offset + i * time_step;
        
        // Ruído de fundo
        float noise_i = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.05f;
        float noise_q = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.05f;
        
        // Sinal simulado (onda senoidal modulada em AM)
        float carrier_freq = 1000.0f; // 1 kHz
        float audio_freq = 100.0f; // 100 Hz
        float modulation_depth = 0.3f;
        
        float audio_signal = sin(2.0f * M_PI * audio_freq * time);
        float carrier = sin(2.0f * M_PI * carrier_freq * time);
        float modulated = (1.0f + modulation_depth * audio_signal) * carrier;
        
        // Adicionar ruído e sinal
        float i_val = noise_i + modulated * 0.1f;
        float q_val = noise_q + modulated * 0.1f;
        
        // Converter para 8-bit
        buffer[i] = static_cast<uint8_t>((i_val + 1.0f) * 127.5f);
        buffer[i + 1] = static_cast<uint8_t>((q_val + 1.0f) * 127.5f);
    }
    
    time_offset += len * time_step;
    *n_read = len;
    
    return 0;
}

int rtlsdr_read_async(rtlsdr_dev* dev, void (*cb)(unsigned char*, uint32_t, void*), void* ctx, uint32_t buf_num, uint32_t buf_len) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->streaming = true;
    LOGI("Async reading started with %d buffers of %d bytes", buf_num, buf_len);
    
    // Em uma implementação real, isso iniciaria uma thread para streaming
    // Por simplicidade, apenas simulamos o início do streaming
    
    return 0;
}

int rtlsdr_cancel_async(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    dev->streaming = false;
    LOGI("Async reading cancelled");
    return 0;
}

int rtlsdr_set_direct_sampling(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Direct sampling %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_get_direct_sampling(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    return 0; // Simular desabilitado
}

int rtlsdr_set_offset_tuning(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Offset tuning %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_get_offset_tuning(rtlsdr_dev* dev) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    return 0; // Simular desabilitado
}

int rtlsdr_set_agc_mode(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("AGC mode %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_set_ds_mode(rtlsdr_dev* dev, int mode) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Direct sampling mode set to %d", mode);
    return 0;
}

int rtlsdr_set_testmode(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Test mode %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_set_dithering(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Dithering %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_set_bias_tee(rtlsdr_dev* dev, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Bias tee %s", on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_set_bias_tee_gpio(rtlsdr_dev* dev, int gpio, int on) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Bias tee GPIO %d %s", gpio, on ? "enabled" : "disabled");
    return 0;
}

int rtlsdr_set_bias_tee_gain(rtlsdr_dev* dev, int gain) {
    if (!dev || !dev->is_open) {
        return -1;
    }
    
    LOGI("Bias tee gain set to %d", gain);
    return 0;
}

} // extern "C" 