package com.sdrradio.app.utils;

import android.util.Log;

public class SpectrumAnalyzer {
    private static final String TAG = "SpectrumAnalyzer";
    
    private static final int FFT_SIZE = 1024;
    private float[] fftBuffer = new float[FFT_SIZE * 2];
    private float[] window = new float[FFT_SIZE];
    private float[] spectrum = new float[FFT_SIZE / 2];
    
    // FFT twiddle factors
    private float[] cosTable = new float[FFT_SIZE];
    private float[] sinTable = new float[FFT_SIZE];
    
    public SpectrumAnalyzer() {
        initializeFFT();
    }
    
    private void initializeFFT() {
        // Initialize window function (Hamming window)
        for (int i = 0; i < FFT_SIZE; i++) {
            window[i] = 0.54f - 0.46f * (float) Math.cos(2 * Math.PI * i / (FFT_SIZE - 1));
        }
        
        // Initialize twiddle factors
        for (int i = 0; i < FFT_SIZE; i++) {
            cosTable[i] = (float) Math.cos(2 * Math.PI * i / FFT_SIZE);
            sinTable[i] = (float) Math.sin(2 * Math.PI * i / FFT_SIZE);
        }
    }
    
    public float[] analyzeSpectrum(float[] iqData) {
        if (iqData == null || iqData.length < FFT_SIZE) {
            return new float[FFT_SIZE / 2];
        }
        
        // Apply window function and prepare FFT input
        for (int i = 0; i < FFT_SIZE; i++) {
            fftBuffer[i * 2] = iqData[i] * window[i]; // Real part
            fftBuffer[i * 2 + 1] = 0; // Imaginary part
        }
        
        // Perform FFT
        performFFT(fftBuffer, FFT_SIZE);
        
        // Calculate power spectrum
        for (int i = 0; i < FFT_SIZE / 2; i++) {
            float real = fftBuffer[i * 2];
            float imag = fftBuffer[i * 2 + 1];
            spectrum[i] = (float) Math.sqrt(real * real + imag * imag);
        }
        
        // Convert to dB
        for (int i = 0; i < spectrum.length; i++) {
            if (spectrum[i] > 0) {
                spectrum[i] = 20 * (float) Math.log10(spectrum[i]);
            } else {
                spectrum[i] = -100; // -100 dB floor
            }
        }
        
        return spectrum;
    }
    
    private void performFFT(float[] data, int n) {
        // Bit-reversal permutation
        int j = 0;
        for (int i = 0; i < n - 1; i++) {
            if (i < j) {
                // Swap real parts
                float temp = data[i * 2];
                data[i * 2] = data[j * 2];
                data[j * 2] = temp;
                
                // Swap imaginary parts
                temp = data[i * 2 + 1];
                data[i * 2 + 1] = data[j * 2 + 1];
                data[j * 2 + 1] = temp;
            }
            
            int k = n / 2;
            while (k <= j) {
                j -= k;
                k /= 2;
            }
            j += k;
        }
        
        // FFT computation
        for (int step = 1; step < n; step *= 2) {
            int jump = step * 2;
            float delta = (float) Math.PI / step;
            
            for (int group = 0; group < step; group++) {
                float angle = group * delta;
                float cosVal = (float) Math.cos(angle);
                float sinVal = (float) Math.sin(angle);
                
                for (int pair = group; pair < n; pair += jump) {
                    int match = pair + step;
                    
                    float real1 = data[pair * 2];
                    float imag1 = data[pair * 2 + 1];
                    float real2 = data[match * 2];
                    float imag2 = data[match * 2 + 1];
                    
                    float realTemp = cosVal * real2 - sinVal * imag2;
                    float imagTemp = cosVal * imag2 + sinVal * real2;
                    
                    data[match * 2] = real1 - realTemp;
                    data[match * 2 + 1] = imag1 - imagTemp;
                    data[pair * 2] = real1 + realTemp;
                    data[pair * 2 + 1] = imag1 + imagTemp;
                }
            }
        }
    }
    
    public float[] getWaterfallData(float[] spectrum) {
        // Convert spectrum to waterfall colors
        float[] waterfall = new float[spectrum.length];
        
        for (int i = 0; i < spectrum.length; i++) {
            // Normalize to 0-1 range
            float normalized = (spectrum[i] + 100) / 100; // Assuming -100 to 0 dB range
            waterfall[i] = Math.max(0, Math.min(1, normalized));
        }
        
        return waterfall;
    }
    
    public float calculateSignalStrength(float[] spectrum) {
        if (spectrum == null || spectrum.length == 0) {
            return 0;
        }
        
        // Calculate average signal strength
        float sum = 0;
        for (float value : spectrum) {
            sum += value;
        }
        
        return sum / spectrum.length;
    }
    
    public float findPeakFrequency(float[] spectrum, float sampleRate) {
        if (spectrum == null || spectrum.length == 0) {
            return 0;
        }
        
        // Find peak in spectrum
        int peakIndex = 0;
        float peakValue = spectrum[0];
        
        for (int i = 1; i < spectrum.length; i++) {
            if (spectrum[i] > peakValue) {
                peakValue = spectrum[i];
                peakIndex = i;
            }
        }
        
        // Convert to frequency
        return (float) peakIndex * sampleRate / (2 * spectrum.length);
    }
    
    public float[] applySmoothing(float[] spectrum, int windowSize) {
        if (spectrum == null || spectrum.length < windowSize) {
            return spectrum;
        }
        
        float[] smoothed = new float[spectrum.length];
        
        for (int i = 0; i < spectrum.length; i++) {
            float sum = 0;
            int count = 0;
            
            for (int j = Math.max(0, i - windowSize / 2); 
                 j < Math.min(spectrum.length, i + windowSize / 2 + 1); j++) {
                sum += spectrum[j];
                count++;
            }
            
            smoothed[i] = sum / count;
        }
        
        return smoothed;
    }
    
    public float[] detectSignals(float[] spectrum, float threshold) {
        if (spectrum == null) {
            return new float[0];
        }
        
        // Simple peak detection
        java.util.List<Float> peaks = new java.util.ArrayList<>();
        
        for (int i = 1; i < spectrum.length - 1; i++) {
            if (spectrum[i] > threshold && 
                spectrum[i] > spectrum[i - 1] && 
                spectrum[i] > spectrum[i + 1]) {
                peaks.add((float) i);
            }
        }
        
        float[] result = new float[peaks.size()];
        for (int i = 0; i < peaks.size(); i++) {
            result[i] = peaks.get(i);
        }
        
        return result;
    }
    
    public float[] getFrequencyBins(float sampleRate) {
        float[] bins = new float[FFT_SIZE / 2];
        float binWidth = sampleRate / FFT_SIZE;
        
        for (int i = 0; i < bins.length; i++) {
            bins[i] = i * binWidth;
        }
        
        return bins;
    }
    
    public void setFFTSize(int size) {
        if (size != FFT_SIZE && isPowerOfTwo(size)) {
            // Reinitialize with new size
            // This would require reallocating arrays and recalculating twiddle factors
            Log.w(TAG, "FFT size change not implemented yet");
        }
    }
    
    private boolean isPowerOfTwo(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }
    
    public int getFFTSize() {
        return FFT_SIZE;
    }
    
    public float[] getSpectrum() {
        return spectrum.clone();
    }
} 