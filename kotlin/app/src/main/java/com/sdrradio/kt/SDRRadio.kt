package com.sdrradio.kt

import android.util.Log
import org.json.JSONObject
import java.io.File

class SDRRadio {
    companion object {
        private const val TAG = "SDRRadio"
        
        init {
            try {
                System.loadLibrary("sdrradio")
                Log.i(TAG, "Native library loaded successfully")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Failed to load native library", e)
            }
        }
    }
    
    private var nativePtr: Long = 0
    private var callback: SDRCallback? = null
    private var isConnected = false
    private var isScanning = false
    private var isRecording = false
    
    init {
        nativePtr = nativeInit()
        Log.i(TAG, "SDRRadio initialized with native pointer: $nativePtr")
    }
    
    fun setCallback(callback: SDRCallback) {
        this.callback = callback
    }
    
    fun connect(): Boolean {
        if (isConnected) {
            Log.i(TAG, "Already connected")
            return true
        }
        
        val result = nativeConnect(nativePtr)
        isConnected = result
        if (result) {
            Log.i(TAG, "RTL-SDR connected successfully")
            callback?.onStatusChanged("Connected")
        } else {
            Log.e(TAG, "Failed to connect RTL-SDR")
            callback?.onStatusChanged("Connection failed")
        }
        return result
    }
    
    fun disconnect() {
        if (!isConnected) return
        
        stopScanning()
        stopRecording()
        nativeDisconnect(nativePtr)
        isConnected = false
        Log.i(TAG, "RTL-SDR disconnected")
        callback?.onStatusChanged("Disconnected")
    }
    
    fun startScanning(): Boolean {
        if (!isConnected) {
            Log.e(TAG, "Cannot start scanning: device not connected")
            return false
        }
        
        if (isScanning) {
            Log.i(TAG, "Already scanning")
            return true
        }
        
        val result = nativeStartScanning(nativePtr)
        isScanning = result
        if (result) {
            Log.i(TAG, "Started frequency scanning")
            callback?.onScanningStarted()
        } else {
            Log.e(TAG, "Failed to start scanning")
        }
        return result
    }
    
    fun stopScanning() {
        if (!isScanning) return
        
        nativeStopScanning(nativePtr)
        isScanning = false
        Log.i(TAG, "Stopped frequency scanning")
        callback?.onScanningStopped()
    }
    
    fun startRecording(): Boolean {
        if (!isConnected) {
            Log.e(TAG, "Cannot start recording: device not connected")
            return false
        }
        
        if (isRecording) {
            Log.i(TAG, "Already recording")
            return true
        }
        
        val result = nativeStartRecording(nativePtr)
        isRecording = result
        if (result) {
            Log.i(TAG, "Started audio recording")
            callback?.onRecordingStarted()
        } else {
            Log.e(TAG, "Failed to start recording")
        }
        return result
    }
    
    fun stopRecording() {
        if (!isRecording) return
        
        nativeStopRecording(nativePtr)
        isRecording = false
        Log.i(TAG, "Stopped audio recording")
        callback?.onRecordingStopped()
    }
    
    fun getCurrentFrequency(): Double {
        return nativeGetFrequency(nativePtr)
    }
    
    fun setFrequency(frequencyHz: Long): Boolean {
        return nativeSetFrequency(nativePtr, frequencyHz)
    }
    
    fun startAudioPlayback(): Boolean {
        return nativeStartAudioPlayback(nativePtr)
    }
    
    fun stopAudioPlayback() {
        nativeStopAudioPlayback(nativePtr)
    }
    
    fun setAudioVolume(volume: Float): Boolean {
        return nativeSetAudioVolume(nativePtr, volume)
    }
    
    fun setModulation(modulation: String): Boolean {
        return nativeSetModulation(nativePtr, modulation)
    }
    
    fun getSignalStrength(): Double {
        return nativeGetSignalStrength(nativePtr)
    }
    
    fun getFrequencySpectrum(): FloatArray? {
        return nativeGetSpectrum(nativePtr)
    }
    
    fun getRecordingsDirectory(): File {
        return File("/sdcard/sdr_recordings")
    }
    
    fun getRecordings(): List<File> {
        val dir = getRecordingsDirectory()
        return if (dir.exists() && dir.isDirectory) {
            dir.listFiles()?.filter { it.isFile && it.extension == "wav" }?.sortedByDescending { it.lastModified() } ?: emptyList()
        } else {
            emptyList()
        }
    }
    
    // Callback method called from native code
    fun onSDRCallback(event: String, data: String) {
        Log.d(TAG, "Native callback: $event - $data")
        
        when (event) {
            "frequency_update" -> {
                try {
                    val json = JSONObject(data)
                    val frequency = json.getDouble("frequency")
                    val signal = json.getDouble("signal")
                    callback?.onFrequencyUpdate(frequency, signal)
                } catch (e: Exception) {
                    Log.e(TAG, "Error parsing frequency update", e)
                }
            }
            "error" -> {
                callback?.onError(data)
            }
            else -> {
                Log.w(TAG, "Unknown callback event: $event")
            }
        }
    }
    
    fun isConnected(): Boolean = isConnected
    fun isScanning(): Boolean = isScanning
    fun isRecording(): Boolean = isRecording
    
    fun destroy() {
        disconnect()
        if (nativePtr != 0L) {
            nativeDestroy(nativePtr)
            nativePtr = 0
        }
        Log.i(TAG, "SDRRadio destroyed")
    }
    
    // Native methods
    private external fun nativeInit(): Long
    private external fun nativeDestroy(ptr: Long)
    private external fun nativeConnect(ptr: Long): Boolean
    private external fun nativeDisconnect(ptr: Long)
    private external fun nativeStartScanning(ptr: Long): Boolean
    private external fun nativeStopScanning(ptr: Long)
    private external fun nativeStartRecording(ptr: Long): Boolean
    private external fun nativeStopRecording(ptr: Long)
    private external fun nativeGetFrequency(ptr: Long): Double
    private external fun nativeSetFrequency(ptr: Long, frequencyHz: Long): Boolean
    private external fun nativeStartAudioPlayback(ptr: Long): Boolean
    private external fun nativeStopAudioPlayback(ptr: Long)
    private external fun nativeSetAudioVolume(ptr: Long, volume: Float): Boolean
    private external fun nativeSetModulation(ptr: Long, modulation: String): Boolean
    private external fun nativeGetSignalStrength(ptr: Long): Double
    private external fun nativeGetSpectrum(ptr: Long): FloatArray?
    
    // Interface para callbacks
    interface SDRCallback {
        fun onStatusChanged(status: String)
        fun onScanningStarted()
        fun onScanningStopped()
        fun onRecordingStarted()
        fun onRecordingStopped()
        fun onFrequencyUpdate(frequency: Double, signalStrength: Double)
        fun onError(error: String)
    }
} 