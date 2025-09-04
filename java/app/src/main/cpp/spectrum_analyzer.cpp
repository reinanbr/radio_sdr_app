#include "spectrum_analyzer.h"
#include <android/log.h>
#include <algorithm>
#include <cmath>
#include <numeric>

#define LOG_TAG "Spectrum_Analyzer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SpectrumAnalyzer::SpectrumAnalyzer()
    : fft_size_(1024)
    , averaging_factor_(0.1f) {
    
    setFFTSize(fft_size_);
    
    LOGI("Spectrum analyzer initialized with FFT size %d", fft_size_);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    LOGI("Spectrum analyzer destroyed");
}

void SpectrumAnalyzer::setFFTSize(int size) {
    // Ensure power of 2
    fft_size_ = next_power_of_two(size);
    
    // Initialize window (Hamming)
    window_.clear();
    window_.resize(fft_size_);
    for (int i = 0; i < fft_size_; ++i) {
        window_[i] = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (fft_size_ - 1));
    }
    
    // Initialize buffers
    fft_input_.resize(fft_size_);
    fft_output_.resize(fft_size_);
    averaged_spectrum_.resize(fft_size_ / 2, -100.0f); // Initialize with low values
    
    LOGD("FFT size set to %d", fft_size_);
}

void SpectrumAnalyzer::updateSpectrum(const std::vector<std::complex<float>>& samples) {
    if (samples.size() < static_cast<size_t>(fft_size_)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(spectrum_mutex_);
    
    // Take the last fft_size_ samples
    size_t start_idx = samples.size() - fft_size_;
    std::copy(samples.begin() + start_idx, samples.end(), fft_input_.begin());
    
    // Apply window
    applyWindow(fft_input_);
    
    // Perform FFT
    performFFT(fft_input_, fft_output_);
    
    // Calculate magnitudes in dB
    std::vector<float> magnitudes = calculateMagnitudes(fft_output_);
    
    // Apply averaging
    applyAveraging(magnitudes);
    
    // Add to waterfall history
    if (waterfall_history_.size() >= WATERFALL_HEIGHT) {
        waterfall_history_.pop_back();
    }
    waterfall_history_.push_front(magnitudes);
}

std::vector<float> SpectrumAnalyzer::getSpectrum() {
    std::lock_guard<std::mutex> lock(spectrum_mutex_);
    return averaged_spectrum_;
}

std::vector<float> SpectrumAnalyzer::getWaterfall() {
    std::lock_guard<std::mutex> lock(spectrum_mutex_);
    
    std::vector<float> waterfall_data;
    waterfall_data.reserve(waterfall_history_.size() * (fft_size_ / 2));
    
    for (const auto& line : waterfall_history_) {
        waterfall_data.insert(waterfall_data.end(), line.begin(), line.end());
    }
    
    return waterfall_data;
}

void SpectrumAnalyzer::performFFT(const std::vector<std::complex<float>>& input, std::vector<std::complex<float>>& output) {
    output = input;
    fft_recursive(output);
}

void SpectrumAnalyzer::applyWindow(std::vector<std::complex<float>>& samples) {
    for (size_t i = 0; i < samples.size() && i < window_.size(); ++i) {
        samples[i] *= window_[i];
    }
}

std::vector<float> SpectrumAnalyzer::calculateMagnitudes(const std::vector<std::complex<float>>& fft_result) {
    std::vector<float> magnitudes;
    magnitudes.reserve(fft_result.size() / 2);
    
    // Only take the first half (positive frequencies)
    for (size_t i = 0; i < fft_result.size() / 2; ++i) {
        float magnitude = std::abs(fft_result[i]);
        
        // Convert to dB, with protection against log(0)
        float db = 20.0f * std::log10(std::max(magnitude, 1e-10f));
        
        magnitudes.push_back(db);
    }
    
    return magnitudes;
}

void SpectrumAnalyzer::applyAveraging(std::vector<float>& magnitudes) {
    if (averaged_spectrum_.size() != magnitudes.size()) {
        averaged_spectrum_ = magnitudes;
        return;
    }
    
    for (size_t i = 0; i < magnitudes.size(); ++i) {
        averaged_spectrum_[i] = (1.0f - averaging_factor_) * averaged_spectrum_[i] + 
                               averaging_factor_ * magnitudes[i];
    }
}

// Simple recursive FFT implementation
void SpectrumAnalyzer::fft_recursive(std::vector<std::complex<float>>& x) {
    int n = x.size();
    
    if (n <= 1) return;
    
    // Divide
    std::vector<std::complex<float>> even(n / 2);
    std::vector<std::complex<float>> odd(n / 2);
    
    for (int i = 0; i < n / 2; ++i) {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }
    
    // Conquer
    fft_recursive(even);
    fft_recursive(odd);
    
    // Combine
    for (int k = 0; k < n / 2; ++k) {
        std::complex<float> t = std::polar(1.0f, -2.0f * static_cast<float>(M_PI) * k / n) * odd[k];
        x[k] = even[k] + t;
        x[k + n / 2] = even[k] - t;
    }
}

bool SpectrumAnalyzer::is_power_of_two(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

int SpectrumAnalyzer::next_power_of_two(int n) {
    if (is_power_of_two(n)) {
        return n;
    }
    
    int power = 1;
    while (power < n) {
        power <<= 1;
    }
    
    return power;
}
