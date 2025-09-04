package com.sdrradio.app.utils;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import java.util.concurrent.atomic.AtomicBoolean;

public class AudioProcessor {
    private static final String TAG = "AudioProcessor";
    
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_OUT_MONO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int BUFFER_SIZE = AudioTrack.getMinBufferSize(SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT);
    
    private AudioTrack audioTrack;
    private final AtomicBoolean isPlaying = new AtomicBoolean(false);
    private Thread audioThread;
    
    // Demodulation parameters
    private float carrierFrequency = 0;
    private float modulationIndex = 1.0f;
    private boolean isFM = true;
    
    // Audio processing
    private float[] audioBuffer = new float[BUFFER_SIZE / 2];
    private short[] outputBuffer = new short[BUFFER_SIZE / 2];
    
    public AudioProcessor() {
        initializeAudioTrack();
    }
    
    private void initializeAudioTrack() {
        try {
            audioTrack = new AudioTrack.Builder()
                .setAudioAttributes(new android.media.AudioAttributes.Builder()
                    .setUsage(android.media.AudioAttributes.USAGE_MEDIA)
                    .setContentType(android.media.AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build())
                .setAudioFormat(new android.media.AudioFormat.Builder()
                    .setEncoding(AUDIO_FORMAT)
                    .setSampleRate(SAMPLE_RATE)
                    .setChannelMask(CHANNEL_CONFIG)
                    .build())
                .setBufferSizeInBytes(BUFFER_SIZE)
                .setTransferMode(AudioTrack.MODE_STREAM)
                .build();
            
            Log.i(TAG, "AudioTrack inicializado com sucesso");
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao inicializar AudioTrack", e);
        }
    }
    
    public void startPlayback() {
        if (isPlaying.get() || audioTrack == null) return;
        
        try {
            audioTrack.play();
            isPlaying.set(true);
            
            audioThread = new Thread(() -> {
                while (isPlaying.get()) {
                    try {
                        // Generate audio data (simulated for now)
                        generateAudioData();
                        
                        // Write to audio track
                        audioTrack.write(outputBuffer, 0, outputBuffer.length);
                        
                        Thread.sleep(10); // 10ms delay
                        
                    } catch (Exception e) {
                        Log.e(TAG, "Erro durante reprodução de áudio", e);
                        break;
                    }
                }
            });
            
            audioThread.start();
            Log.i(TAG, "Reprodução de áudio iniciada");
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao iniciar reprodução", e);
        }
    }
    
    public void stopPlayback() {
        isPlaying.set(false);
        
        if (audioThread != null) {
            try {
                audioThread.join(1000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            audioThread = null;
        }
        
        if (audioTrack != null) {
            audioTrack.stop();
        }
        
        Log.i(TAG, "Reprodução de áudio parada");
    }
    
    private void generateAudioData() {
        // This is a simplified audio generation
        // In a real implementation, you would demodulate the I/Q data from RTL-SDR
        
        for (int i = 0; i < audioBuffer.length; i++) {
            // Generate a simple sine wave for demonstration
            float time = (float) i / SAMPLE_RATE;
            audioBuffer[i] = (float) Math.sin(2 * Math.PI * 440 * time) * 0.5f; // 440 Hz tone
            
            // Convert to 16-bit PCM
            outputBuffer[i] = (short) (audioBuffer[i] * 32767);
        }
    }
    
    public void processIQData(float[] iData, float[] qData) {
        if (!isPlaying.get()) return;
        
        // Demodulate I/Q data to audio
        if (isFM) {
            demodulateFM(iData, qData);
        } else {
            demodulateAM(iData, qData);
        }
    }
    
    private void demodulateFM(float[] iData, float[] qData) {
        // FM demodulation using phase difference
        for (int i = 1; i < iData.length && i < audioBuffer.length; i++) {
            // Calculate phase difference
            float phase1 = (float) Math.atan2(qData[i-1], iData[i-1]);
            float phase2 = (float) Math.atan2(qData[i], iData[i]);
            
            float phaseDiff = phase2 - phase1;
            
            // Normalize phase difference
            if (phaseDiff > Math.PI) {
                phaseDiff -= 2 * Math.PI;
            } else if (phaseDiff < -Math.PI) {
                phaseDiff += 2 * Math.PI;
            }
            
            // Convert to audio
            audioBuffer[i-1] = phaseDiff * modulationIndex;
        }
    }
    
    private void demodulateAM(float[] iData, float[] qData) {
        // AM demodulation using envelope detection
        for (int i = 0; i < iData.length && i < audioBuffer.length; i++) {
            // Calculate magnitude
            float magnitude = (float) Math.sqrt(iData[i] * iData[i] + qData[i] * qData[i]);
            
            // Remove DC component and apply gain
            audioBuffer[i] = (magnitude - 1.0f) * modulationIndex;
        }
    }
    
    public void setCarrierFrequency(float frequency) {
        this.carrierFrequency = frequency;
    }
    
    public void setModulationIndex(float index) {
        this.modulationIndex = index;
    }
    
    public void setModulationType(boolean isFM) {
        this.isFM = isFM;
    }
    
    public void setVolume(float volume) {
        if (audioTrack != null) {
            audioTrack.setVolume(volume);
        }
    }
    
    public boolean isPlaying() {
        return isPlaying.get();
    }
    
    public void release() {
        stopPlayback();
        
        if (audioTrack != null) {
            audioTrack.release();
            audioTrack = null;
        }
        
        Log.i(TAG, "AudioProcessor liberado");
    }
} 