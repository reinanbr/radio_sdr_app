#include <android/log.h>
#include <cmath>
#include <vector>
#include <algorithm>

// Definições de log
#define LOG_TAG "AudioProcessor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Constantes para processamento de áudio
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 2.0f * PI;
constexpr int AUDIO_SAMPLE_RATE = 44100;
constexpr int SDR_SAMPLE_RATE = 2048000;
constexpr float DECIMATION_RATIO = static_cast<float>(SDR_SAMPLE_RATE) / AUDIO_SAMPLE_RATE;

// Filtros simples para demodulação
class AudioProcessor {
private:
    // Filtros de baixa passagem
    std::vector<float> lpf_coeffs_;
    std::vector<float> lpf_buffer_;
    int lpf_buffer_index_;
    
    // Filtros de alta passagem
    std::vector<float> hpf_coeffs_;
    std::vector<float> hpf_buffer_;
    int hpf_buffer_index_;
    
    // Demodulador AM
    float am_demod_gain_;
    float am_demod_dc_block_;
    
    // Demodulador FM
    float fm_demod_gain_;
    float fm_prev_sample_;
    
    // AGC (Automatic Gain Control)
    float agc_gain_;
    float agc_target_;
    float agc_attack_;
    float agc_decay_;
    
    // Noise gate
    float noise_gate_threshold_;
    float noise_gate_ratio_;
    
public:
    AudioProcessor() 
        : lpf_buffer_index_(0)
        , hpf_buffer_index_(0)
        , am_demod_gain_(1.0f)
        , am_demod_dc_block_(0.0f)
        , fm_demod_gain_(1.0f)
        , fm_prev_sample_(0.0f)
        , agc_gain_(1.0f)
        , agc_target_(0.3f)
        , agc_attack_(0.01f)
        , agc_decay_(0.001f)
        , noise_gate_threshold_(0.01f)
        , noise_gate_ratio_(0.1f) {
        
        initializeFilters();
        LOGI("AudioProcessor initialized");
    }
    
    ~AudioProcessor() {
        LOGI("AudioProcessor destroyed");
    }
    
    // Processar dados I/Q e converter para áudio
    std::vector<float> processIQData(const std::vector<uint8_t>& iq_data, const std::string& demod_type = "AM") {
        std::vector<float> audio_data;
        
        if (iq_data.empty()) {
            return audio_data;
        }
        
        // Converter dados I/Q de 8-bit para float
        std::vector<float> i_samples, q_samples;
        for (size_t i = 0; i < iq_data.size(); i += 2) {
            if (i + 1 < iq_data.size()) {
                float i_val = (iq_data[i] - 128.0f) / 128.0f;
                float q_val = (iq_data[i + 1] - 128.0f) / 128.0f;
                i_samples.push_back(i_val);
                q_samples.push_back(q_val);
            }
        }
        
        // Aplicar demodulação
        if (demod_type == "AM") {
            audio_data = demodulateAM(i_samples, q_samples);
        } else if (demod_type == "FM") {
            audio_data = demodulateFM(i_samples, q_samples);
        } else {
            // Demodulação AM padrão
            audio_data = demodulateAM(i_samples, q_samples);
        }
        
        // Aplicar filtros e processamento
        audio_data = applyFilters(audio_data);
        audio_data = applyAGC(audio_data);
        audio_data = applyNoiseGate(audio_data);
        
        // Decimar para taxa de áudio
        audio_data = decimate(audio_data);
        
        return audio_data;
    }
    
    // Configurar parâmetros
    void setAMDemodGain(float gain) { am_demod_gain_ = gain; }
    void setFMDemodGain(float gain) { fm_demod_gain_ = gain; }
    void setAGCTarget(float target) { agc_target_ = target; }
    void setAGCAttack(float attack) { agc_attack_ = attack; }
    void setAGCDecay(float decay) { agc_decay_ = decay; }
    void setNoiseGateThreshold(float threshold) { noise_gate_threshold_ = threshold; }
    void setNoiseGateRatio(float ratio) { noise_gate_ratio_ = ratio; }
    
private:
    void initializeFilters() {
        // Filtro de baixa passagem simples (FIR)
        lpf_coeffs_ = {0.1f, 0.2f, 0.4f, 0.2f, 0.1f};
        lpf_buffer_.resize(lpf_coeffs_.size(), 0.0f);
        
        // Filtro de alta passagem simples (FIR)
        hpf_coeffs_ = {0.1f, -0.2f, 0.4f, -0.2f, 0.1f};
        hpf_buffer_.resize(hpf_coeffs_.size(), 0.0f);
    }
    
    std::vector<float> demodulateAM(const std::vector<float>& i_samples, const std::vector<float>& q_samples) {
        std::vector<float> audio_data;
        audio_data.reserve(i_samples.size());
        
        for (size_t i = 0; i < i_samples.size(); ++i) {
            // Demodulação AM: magnitude do sinal complexo
            float magnitude = std::sqrt(i_samples[i] * i_samples[i] + q_samples[i] * q_samples[i]);
            
            // Remover componente DC
            magnitude -= am_demod_dc_block_;
            am_demod_dc_block_ = am_demod_dc_block_ * 0.999f + magnitude * 0.001f;
            
            // Aplicar ganho
            magnitude *= am_demod_gain_;
            
            audio_data.push_back(magnitude);
        }
        
        return audio_data;
    }
    
    std::vector<float> demodulateFM(const std::vector<float>& i_samples, const std::vector<float>& q_samples) {
        std::vector<float> audio_data;
        audio_data.reserve(i_samples.size());
        
        for (size_t i = 0; i < i_samples.size(); ++i) {
            // Demodulação FM: derivada da fase
            float phase = std::atan2(q_samples[i], i_samples[i]);
            
            // Calcular diferença de fase
            float phase_diff = phase - fm_prev_sample_;
            
            // Normalizar diferença de fase
            if (phase_diff > PI) phase_diff -= TWO_PI;
            if (phase_diff < -PI) phase_diff += TWO_PI;
            
            // Aplicar ganho
            float audio_sample = phase_diff * fm_demod_gain_;
            
            audio_data.push_back(audio_sample);
            fm_prev_sample_ = phase;
        }
        
        return audio_data;
    }
    
    std::vector<float> applyFilters(const std::vector<float>& audio_data) {
        std::vector<float> filtered_data;
        filtered_data.reserve(audio_data.size());
        
        for (float sample : audio_data) {
            // Aplicar filtro de baixa passagem
            sample = applyLowPassFilter(sample);
            
            // Aplicar filtro de alta passagem
            sample = applyHighPassFilter(sample);
            
            filtered_data.push_back(sample);
        }
        
        return filtered_data;
    }
    
    float applyLowPassFilter(float sample) {
        // Implementação simples de filtro FIR
        lpf_buffer_[lpf_buffer_index_] = sample;
        
        float output = 0.0f;
        for (size_t i = 0; i < lpf_coeffs_.size(); ++i) {
            int index = (lpf_buffer_index_ + i) % lpf_buffer_.size();
            output += lpf_coeffs_[i] * lpf_buffer_[index];
        }
        
        lpf_buffer_index_ = (lpf_buffer_index_ + 1) % lpf_buffer_.size();
        return output;
    }
    
    float applyHighPassFilter(float sample) {
        // Implementação simples de filtro FIR
        hpf_buffer_[hpf_buffer_index_] = sample;
        
        float output = 0.0f;
        for (size_t i = 0; i < hpf_coeffs_.size(); ++i) {
            int index = (hpf_buffer_index_ + i) % hpf_buffer_.size();
            output += hpf_coeffs_[i] * hpf_buffer_[index];
        }
        
        hpf_buffer_index_ = (hpf_buffer_index_ + 1) % hpf_buffer_.size();
        return output;
    }
    
    std::vector<float> applyAGC(const std::vector<float>& audio_data) {
        std::vector<float> agc_data;
        agc_data.reserve(audio_data.size());
        
        for (float sample : audio_data) {
            float abs_sample = std::abs(sample);
            
            // Calcular erro do AGC
            float error = agc_target_ - abs_sample;
            
            // Aplicar ataque/decay
            float rate = (error > 0) ? agc_attack_ : agc_decay_;
            agc_gain_ += error * rate;
            
            // Limitar ganho
            agc_gain_ = std::max(0.1f, std::min(10.0f, agc_gain_));
            
            // Aplicar ganho
            agc_data.push_back(sample * agc_gain_);
        }
        
        return agc_data;
    }
    
    std::vector<float> applyNoiseGate(const std::vector<float>& audio_data) {
        std::vector<float> gated_data;
        gated_data.reserve(audio_data.size());
        
        for (float sample : audio_data) {
            float abs_sample = std::abs(sample);
            
            if (abs_sample < noise_gate_threshold_) {
                // Aplicar redução de ruído
                sample *= noise_gate_ratio_;
            }
            
            gated_data.push_back(sample);
        }
        
        return gated_data;
    }
    
    std::vector<float> decimate(const std::vector<float>& audio_data) {
        std::vector<float> decimated_data;
        
        // Decimar para taxa de áudio
        for (size_t i = 0; i < audio_data.size(); i += static_cast<int>(DECIMATION_RATIO)) {
            if (i < audio_data.size()) {
                decimated_data.push_back(audio_data[i]);
            }
        }
        
        return decimated_data;
    }
};

// Funções de interface C para uso externo
extern "C" {

AudioProcessor* audio_processor_create() {
    return new AudioProcessor();
}

void audio_processor_destroy(AudioProcessor* processor) {
    if (processor) {
        delete processor;
    }
}

void audio_processor_process_iq(AudioProcessor* processor, 
                              const uint8_t* iq_data, int iq_length,
                              float* audio_data, int* audio_length,
                              const char* demod_type) {
    if (!processor || !iq_data || !audio_data || !audio_length) {
        return;
    }
    
    std::vector<uint8_t> iq_vector(iq_data, iq_data + iq_length);
    std::string demod_str = demod_type ? demod_type : "AM";
    
    std::vector<float> result = processor->processIQData(iq_vector, demod_str);
    
    // Copiar resultado para buffer de saída
    int copy_length = std::min(static_cast<int>(result.size()), *audio_length);
    std::copy(result.begin(), result.begin() + copy_length, audio_data);
    *audio_length = copy_length;
}

void audio_processor_set_am_gain(AudioProcessor* processor, float gain) {
    if (processor) {
        processor->setAMDemodGain(gain);
    }
}

void audio_processor_set_fm_gain(AudioProcessor* processor, float gain) {
    if (processor) {
        processor->setFMDemodGain(gain);
    }
}

void audio_processor_set_agc_target(AudioProcessor* processor, float target) {
    if (processor) {
        processor->setAGCTarget(target);
    }
}

void audio_processor_set_noise_gate_threshold(AudioProcessor* processor, float threshold) {
    if (processor) {
        processor->setNoiseGateThreshold(threshold);
    }
}

} // extern "C" 