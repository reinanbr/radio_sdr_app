#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <vector>
#include <complex>

enum class DemodulationType {
    FM,
    AM,
    USB,
    LSB
};

class Demodulator {
public:
    Demodulator();
    ~Demodulator();
    
    void setType(DemodulationType type);
    std::vector<float> demodulate(const std::vector<std::complex<float>>& samples);
    
private:
    std::vector<float> demodulateAM(const std::vector<std::complex<float>>& samples);
    std::vector<float> demodulateFM(const std::vector<std::complex<float>>& samples);
    std::vector<float> demodulateSSB(const std::vector<std::complex<float>>& samples, bool upper);
    
    DemodulationType type_;
    std::complex<float> last_sample_;  // For FM phase difference calculation
    
    // Decimation for audio output
    static const int DECIMATION_FACTOR = 42; // 2048000 / 42 ≈ 48000 Hz
    int decimation_counter_;
};

#endif // DEMODULATOR_H
