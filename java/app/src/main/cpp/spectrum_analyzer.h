#ifndef SPECTRUM_ANALYZER_H
#define SPECTRUM_ANALYZER_H

#include <vector>
#include <complex>
#include <mutex>
#include <deque>

class SpectrumAnalyzer {
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer();
    
    void updateSpectrum(const std::vector<std::complex<float>>& samples);
    std::vector<float> getSpectrum();
    std::vector<float> getWaterfall();
    
    void setFFTSize(int size);
    int getFFTSize() const { return fft_size_; }
    
private:
    void performFFT(const std::vector<std::complex<float>>& input, std::vector<std::complex<float>>& output);
    void applyWindow(std::vector<std::complex<float>>& samples);
    std::vector<float> calculateMagnitudes(const std::vector<std::complex<float>>& fft_result);
    void applyAveraging(std::vector<float>& magnitudes);
    
    int fft_size_;
    std::vector<float> window_;
    std::vector<std::complex<float>> fft_input_;
    std::vector<std::complex<float>> fft_output_;
    
    // Spectrum averaging
    std::vector<float> averaged_spectrum_;
    float averaging_factor_;
    
    // Waterfall display
    std::deque<std::vector<float>> waterfall_history_;
    static const size_t WATERFALL_HEIGHT = 100;
    
    std::mutex spectrum_mutex_;
    
    // Simple FFT implementation (for small sizes)
    void fft_recursive(std::vector<std::complex<float>>& x);
    bool is_power_of_two(int n);
    int next_power_of_two(int n);
};

#endif // SPECTRUM_ANALYZER_H
