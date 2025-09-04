package com.sdrradio.app.utils;

import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbRequest;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.atomic.AtomicBoolean;

public class RTLSDROperations {
    private static final String TAG = "RTLSDROperations";
    
    // RTL-SDR constants
    private static final int RTL2832U_VENDOR_ID = 0x0bda;
    private static final int RTL2832U_PRODUCT_ID = 0x2838;
    private static final int RTL2832U_PRODUCT_ID_V4 = 0x2832;
    
    // USB endpoints
    private static final int USB_ENDPOINT_IN = 0x81;
    private static final int USB_ENDPOINT_OUT = 0x02;
    private static final int USB_TIMEOUT = 5000;
    
    // RTL2832U registers
    private static final int DEMOD_CTL = 0x00;
    private static final int GPO = 0x19;
    private static final int GPI = 0x1a;
    private static final int SYSINTE = 0x01;
    private static final int SYSINTS = 0x02;
    private static final int SYSINTE_1 = 0x01;
    private static final int SYSINTS_1 = 0x02;
    private static final int SYSINTE_2 = 0x01;
    private static final int SYSINTS_2 = 0x02;
    private static final int SYSINTE_3 = 0x01;
    private static final int SYSINTS_3 = 0x02;
    private static final int SYSINTE_4 = 0x01;
    private static final int SYSINTS_4 = 0x02;
    private static final int SYSINTE_5 = 0x01;
    private static final int SYSINTS_5 = 0x02;
    private static final int SYSINTE_6 = 0x01;
    private static final int SYSINTS_6 = 0x02;
    private static final int SYSINTE_7 = 0x01;
    private static final int SYSINTS_7 = 0x02;
    private static final int SYSINTE_8 = 0x01;
    private static final int SYSINTS_8 = 0x02;
    private static final int SYSINTE_9 = 0x01;
    private static final int SYSINTS_9 = 0x02;
    private static final int SYSINTE_A = 0x01;
    private static final int SYSINTS_A = 0x02;
    private static final int SYSINTE_B = 0x01;
    private static final int SYSINTS_B = 0x02;
    private static final int SYSINTE_C = 0x01;
    private static final int SYSINTS_C = 0x02;
    private static final int SYSINTE_D = 0x01;
    private static final int SYSINTS_D = 0x02;
    private static final int SYSINTE_E = 0x01;
    private static final int SYSINTS_E = 0x02;
    private static final int SYSINTE_F = 0x01;
    private static final int SYSINTS_F = 0x02;
    
    // R820T tuner registers
    private static final int R820T_I2C_ADDR = 0x34;
    private static final int R820T_CTRL = 0x05;
    private static final int R820T_CTRL_SYN = 0x40;
    private static final int R820T_CTRL_VCO = 0x80;
    private static final int R820T_CTRL_XTAL = 0x10;
    private static final int R820T_CTRL_LNA = 0x08;
    private static final int R820T_CTRL_MIXER = 0x04;
    private static final int R820T_CTRL_VCO_CAL = 0x02;
    private static final int R820T_CTRL_LNA_CAL = 0x01;
    
    private UsbDeviceConnection connection;
    private UsbDevice device;
    private UsbInterface usbInterface;
    private UsbEndpoint endpointIn;
    private UsbEndpoint endpointOut;
    private UsbRequest usbRequest;
    
    private int currentFrequency = 100000000; // 100 MHz default
    private int sampleRate = 2048000; // 2.048 MHz
    private int gain = 20; // 20 dB default
    private boolean autoGain = true;
    
    private final AtomicBoolean isInitialized = new AtomicBoolean(false);
    private final AtomicBoolean isStreaming = new AtomicBoolean(false);
    
    private Thread streamingThread;
    private final Object streamingLock = new Object();
    
    public boolean initialize(UsbDeviceConnection connection, UsbDevice device) {
        Log.d(TAG, "Iniciando inicialização do RTL-SDR...");
        this.connection = connection;
        this.device = device;
        
        try {
            // Find USB interface
            Log.d(TAG, "Procurando interface USB...");
            usbInterface = findInterface();
            if (usbInterface == null) {
                Log.e(TAG, "Interface não encontrada");
                return false;
            }
            Log.d(TAG, "Interface encontrada: " + usbInterface.getInterfaceClass());
            
            // Claim interface
            Log.d(TAG, "Reivindicando interface...");
            if (!connection.claimInterface(usbInterface, true)) {
                Log.e(TAG, "Erro ao reivindicar interface");
                return false;
            }
            Log.d(TAG, "Interface reivindicada com sucesso");
            
            // Find endpoints
            Log.d(TAG, "Procurando endpoints...");
            endpointIn = findEndpoint(usbInterface, UsbConstants.USB_DIR_IN);
            endpointOut = findEndpoint(usbInterface, UsbConstants.USB_DIR_OUT);
            
            if (endpointIn == null || endpointOut == null) {
                Log.e(TAG, "Endpoints não encontrados - IN: " + (endpointIn != null) + ", OUT: " + (endpointOut != null));
                return false;
            }
            Log.d(TAG, "Endpoints encontrados - IN: " + endpointIn.getEndpointNumber() + ", OUT: " + endpointOut.getEndpointNumber());
            
            // For now, skip hardware initialization to test basic USB connection
            Log.d(TAG, "Pulando inicialização do hardware para teste básico...");
            
            isInitialized.set(true);
            Log.i(TAG, "RTL-SDR conectado com sucesso (modo teste)");
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Erro durante inicialização", e);
            return false;
        }
    }
    
    private UsbInterface findInterface() {
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            UsbInterface intf = device.getInterface(i);
            if (intf.getInterfaceClass() == UsbConstants.USB_CLASS_VENDOR_SPEC) {
                return intf;
            }
        }
        return null;
    }
    
    private UsbEndpoint findEndpoint(UsbInterface intf, int direction) {
        for (int i = 0; i < intf.getEndpointCount(); i++) {
            UsbEndpoint endpoint = intf.getEndpoint(i);
            if (endpoint.getDirection() == direction) {
                return endpoint;
            }
        }
        return null;
    }
    
    private boolean initializeRTL2832U() {
        try {
            // Reset demodulator
            writeDemodReg(DEMOD_CTL, 0x10);
            Thread.sleep(10);
            writeDemodReg(DEMOD_CTL, 0x00);
            
            // Configure system
            writeDemodReg(SYSINTE, 0x00);
            writeDemodReg(SYSINTE_1, 0x00);
            writeDemodReg(SYSINTE_2, 0x00);
            writeDemodReg(SYSINTE_3, 0x00);
            writeDemodReg(SYSINTE_4, 0x00);
            writeDemodReg(SYSINTE_5, 0x00);
            writeDemodReg(SYSINTE_6, 0x00);
            writeDemodReg(SYSINTE_7, 0x00);
            writeDemodReg(SYSINTE_8, 0x00);
            writeDemodReg(SYSINTE_9, 0x00);
            writeDemodReg(SYSINTE_A, 0x00);
            writeDemodReg(SYSINTE_B, 0x00);
            writeDemodReg(SYSINTE_C, 0x00);
            writeDemodReg(SYSINTE_D, 0x00);
            writeDemodReg(SYSINTE_E, 0x00);
            writeDemodReg(SYSINTE_F, 0x00);
            
            // Configure GPIO
            writeDemodReg(GPO, 0x00);
            writeDemodReg(GPI, 0x00);
            
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao inicializar RTL2832U", e);
            return false;
        }
    }
    
    private boolean initializeTuner() {
        try {
            // Reset tuner
            writeTunerReg(R820T_CTRL, 0x00);
            Thread.sleep(10);
            
            // Configure tuner
            writeTunerReg(R820T_CTRL, R820T_CTRL_XTAL);
            
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao inicializar tuner", e);
            return false;
        }
    }
    
    public void setFrequency(int frequency) {
        if (!isInitialized.get()) return;
        
        currentFrequency = frequency;
        
        try {
            // Calculate PLL parameters
            int xtalFreq = 28800000; // 28.8 MHz crystal
            int pllRef = xtalFreq / 4; // 7.2 MHz reference
            
            int n = (frequency * 4) / pllRef;
            int m = (frequency * 4) % pllRef;
            
            // Set PLL registers
            writeTunerReg(0x05, (n >> 8) & 0xFF);
            writeTunerReg(0x06, n & 0xFF);
            writeTunerReg(0x07, (m >> 8) & 0xFF);
            writeTunerReg(0x08, m & 0xFF);
            
            // Enable PLL
            writeTunerReg(R820T_CTRL, R820T_CTRL_SYN | R820T_CTRL_XTAL);
            
            Log.i(TAG, "Frequência definida: " + frequency + " Hz");
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao definir frequência", e);
        }
    }
    
    public void setGain(int gain) {
        this.gain = gain;
        this.autoGain = false;
        
        if (isInitialized.get()) {
            try {
                // Set manual gain
                int lnaGain = Math.min(15, gain / 3);
                int mixerGain = Math.min(15, (gain % 3) * 5);
                
                writeTunerReg(0x05, (lnaGain << 4) | mixerGain);
                
            } catch (Exception e) {
                Log.e(TAG, "Erro ao definir ganho", e);
            }
        }
    }
    
    public void setAutoGain(boolean auto) {
        autoGain = auto;
        
        if (isInitialized.get()) {
            try {
                if (auto) {
                    writeTunerReg(R820T_CTRL, R820T_CTRL_LNA_CAL | R820T_CTRL_MIXER);
                } else {
                    setGain(gain);
                }
            } catch (Exception e) {
                Log.e(TAG, "Erro ao configurar ganho automático", e);
            }
        }
    }
    
    public void setSampleRate(int sampleRate) {
        this.sampleRate = sampleRate;
        
        if (isInitialized.get()) {
            try {
                // Configure sample rate
                int divider = 28800000 / sampleRate;
                writeDemodReg(0x9F, (divider >> 8) & 0xFF);
                writeDemodReg(0xA0, divider & 0xFF);
                
            } catch (Exception e) {
                Log.e(TAG, "Erro ao definir taxa de amostragem", e);
            }
        }
    }
    
    public void startStreaming() {
        if (!isInitialized.get() || isStreaming.get()) return;
        
        synchronized (streamingLock) {
            isStreaming.set(true);
            
            streamingThread = new Thread(() -> {
                ByteBuffer buffer = ByteBuffer.allocate(endpointIn.getMaxPacketSize());
                buffer.order(ByteOrder.LITTLE_ENDIAN);
                
                while (isStreaming.get()) {
                    try {
                        int bytesRead = connection.bulkTransfer(endpointIn, buffer.array(), buffer.capacity(), USB_TIMEOUT);
                        
                        if (bytesRead > 0) {
                            // Process received data
                            processReceivedData(buffer.array(), bytesRead);
                        }
                        
                    } catch (Exception e) {
                        Log.e(TAG, "Erro durante streaming", e);
                        break;
                    }
                }
            });
            
            streamingThread.start();
        }
    }
    
    public void stopStreaming() {
        synchronized (streamingLock) {
            isStreaming.set(false);
            
            if (streamingThread != null) {
                try {
                    streamingThread.join(1000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
                streamingThread = null;
            }
        }
    }
    
    private void processReceivedData(byte[] data, int length) {
        // Convert I/Q data to audio
        // This is a simplified implementation
        // In a real application, you would implement proper demodulation
        
        float[] iqData = new float[length / 2];
        for (int i = 0; i < length; i += 2) {
            int iSample = (data[i] & 0xFF) - 128;
            int qSample = (data[i + 1] & 0xFF) - 128;
            iqData[i / 2] = (float) Math.sqrt(iSample * iSample + qSample * qSample);
        }
        
        // Store spectrum data for display
        spectrumData = iqData;
    }
    
    private float[] spectrumData = new float[1024];
    
    public float[] getSpectrumData() {
        return spectrumData;
    }
    
    private void writeDemodReg(int reg, int value) {
        byte[] data = {0x01, (byte) reg, (byte) value};
        connection.controlTransfer(0x40, 0x01, 0, 0, data, data.length, USB_TIMEOUT);
    }
    
    private int readDemodReg(int reg) {
        byte[] data = new byte[1];
        connection.controlTransfer(0xC0, 0x01, 0, reg, data, data.length, USB_TIMEOUT);
        return data[0] & 0xFF;
    }
    
    private void writeTunerReg(int reg, int value) {
        byte[] data = {0x02, (byte) reg, (byte) value};
        connection.controlTransfer(0x40, 0x01, 0, 0, data, data.length, USB_TIMEOUT);
    }
    
    private int readTunerReg(int reg) {
        byte[] data = new byte[1];
        connection.controlTransfer(0xC0, 0x01, 0, reg, data, data.length, USB_TIMEOUT);
        return data[0] & 0xFF;
    }
    
    public void close() {
        stopStreaming();
        
        if (connection != null && usbInterface != null) {
            connection.releaseInterface(usbInterface);
        }
        
        isInitialized.set(false);
        Log.i(TAG, "RTL-SDR fechado");
    }
    
    public boolean isInitialized() {
        return isInitialized.get();
    }
    
    public boolean isStreaming() {
        return isStreaming.get();
    }
    
    public int getCurrentFrequency() {
        return currentFrequency;
    }
    
    public int getSampleRate() {
        return sampleRate;
    }
    
    public int getGain() {
        return gain;
    }
    
    public boolean isAutoGain() {
        return autoGain;
    }
} 