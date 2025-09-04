package com.example.sdrradio;

import android.util.Log;

/**
 * Classe Java que integra com o código C++ para controle do RTL-SDR
 */
public class SDRRadio {
    private static final String TAG = "SDRRadio";
    
    // Ponteiro nativo para o objeto C++
    private long nativePtr;
    
    // Callback para notificações
    private SDRCallback callback;
    
    // Estado atual
    private boolean isDeviceConnected = false;
    private boolean isRadioRunning = false;
    
    /**
     * Interface para callbacks do SDR
     */
    public interface SDRCallback {
        void onDeviceConnected();
        void onDeviceDisconnected();
        void onRadioStarted();
        void onRadioStopped();
        void onSignalStrengthChanged(float strength);
        void onError(String error);
    }
    
    /**
     * Construtor
     */
    public SDRRadio() {
        Log.d(TAG, "Creating SDRRadio instance");
        nativePtr = nativeInit();
        if (nativePtr == 0) {
            Log.e(TAG, "Failed to initialize native SDRRadio");
            throw new RuntimeException("Failed to initialize SDRRadio");
        }
    }
    
    /**
     * Destrutor - deve ser chamado quando o objeto não for mais necessário
     */
    public void destroy() {
        Log.d(TAG, "Destroying SDRRadio instance");
        if (nativePtr != 0) {
            nativeDestroy(nativePtr);
            nativePtr = 0;
        }
    }
    
    /**
     * Definir callback para notificações
     */
    public void setCallback(SDRCallback callback) {
        this.callback = callback;
    }
    
    /**
     * Conectar ao dispositivo RTL-SDR
     */
    public boolean connectDevice() {
        Log.d(TAG, "Connecting to RTL-SDR device");
        if (nativePtr == 0) {
            Log.e(TAG, "Native pointer is null");
            return false;
        }
        
        boolean result = nativeConnectDevice(nativePtr);
        if (result) {
            isDeviceConnected = true;
            if (callback != null) {
                callback.onDeviceConnected();
            }
        }
        return result;
    }
    
    /**
     * Desconectar do dispositivo RTL-SDR
     */
    public void disconnectDevice() {
        Log.d(TAG, "Disconnecting from RTL-SDR device");
        if (nativePtr != 0) {
            nativeDisconnectDevice(nativePtr);
            isDeviceConnected = false;
            isRadioRunning = false;
            if (callback != null) {
                callback.onDeviceDisconnected();
            }
        }
    }
    
    /**
     * Verificar se o dispositivo está conectado
     */
    public boolean isDeviceConnected() {
        if (nativePtr != 0) {
            isDeviceConnected = nativeIsDeviceConnected(nativePtr);
        }
        return isDeviceConnected;
    }
    
    /**
     * Iniciar o rádio
     */
    public boolean startRadio(double frequency, int sampleRate, float gain, boolean autoGain) {
        Log.d(TAG, String.format("Starting radio: freq=%.1f MHz, sampleRate=%d, gain=%.1f, autoGain=%s", 
                                frequency, sampleRate, gain, autoGain));
        
        if (nativePtr == 0) {
            Log.e(TAG, "Native pointer is null");
            return false;
        }
        
        if (!isDeviceConnected) {
            Log.e(TAG, "Device not connected");
            return false;
        }
        
        boolean result = nativeStartRadio(nativePtr, frequency, sampleRate, gain, autoGain);
        if (result) {
            isRadioRunning = true;
            if (callback != null) {
                callback.onRadioStarted();
            }
        }
        return result;
    }
    
    /**
     * Parar o rádio
     */
    public void stopRadio() {
        Log.d(TAG, "Stopping radio");
        if (nativePtr != 0) {
            nativeStopRadio(nativePtr);
            isRadioRunning = false;
            if (callback != null) {
                callback.onRadioStopped();
            }
        }
    }
    
    /**
     * Verificar se o rádio está rodando
     */
    public boolean isRadioRunning() {
        if (nativePtr != 0) {
            isRadioRunning = nativeIsRadioRunning(nativePtr);
        }
        return isRadioRunning;
    }
    
    /**
     * Obter status do dispositivo
     */
    public boolean getDeviceStatus() {
        return isDeviceConnected;
    }
    
    /**
     * Obter status do rádio
     */
    public boolean getRadioStatus() {
        return isRadioRunning;
    }
    
    // Métodos nativos JNI
    private native long nativeInit();
    private native void nativeDestroy(long nativePtr);
    private native boolean nativeConnectDevice(long nativePtr);
    private native void nativeDisconnectDevice(long nativePtr);
    private native boolean nativeStartRadio(long nativePtr, double frequency, int sampleRate, float gain, boolean autoGain);
    private native void nativeStopRadio(long nativePtr);
    private native boolean nativeIsDeviceConnected(long nativePtr);
    private native boolean nativeIsRadioRunning(long nativePtr);
    
    // Carregar biblioteca nativa
    static {
        try {
            System.loadLibrary("sdrradio");
            Log.d(TAG, "Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library", e);
            throw new RuntimeException("Failed to load native library", e);
        }
    }
} 