package com.sdrradio.app.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;

public class USBReceiver extends BroadcastReceiver {
    private static final String TAG = "USBReceiver";
    
    public interface USBEventListener {
        void onUSBDeviceAttached(UsbDevice device);
        void onUSBDeviceDetached(UsbDevice device);
    }
    
    private USBEventListener listener;
    
    public USBReceiver() {
        // Default constructor
    }
    
    public USBReceiver(USBEventListener listener) {
        this.listener = listener;
    }
    
    public void setListener(USBEventListener listener) {
        this.listener = listener;
    }
    
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        
        if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device != null) {
                Log.i(TAG, "Dispositivo USB conectado: " + device.getDeviceName());
                
                // Check if it's an RTL-SDR device
                if (isRTLSDRDevice(device)) {
                    Log.i(TAG, "RTL-SDR detectado: " + device.getDeviceName());
                    if (listener != null) {
                        listener.onUSBDeviceAttached(device);
                    }
                }
            }
            
        } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device != null) {
                Log.i(TAG, "Dispositivo USB desconectado: " + device.getDeviceName());
                
                // Check if it's an RTL-SDR device
                if (isRTLSDRDevice(device)) {
                    Log.i(TAG, "RTL-SDR desconectado: " + device.getDeviceName());
                    if (listener != null) {
                        listener.onUSBDeviceDetached(device);
                    }
                }
            }
        }
    }
    
    private boolean isRTLSDRDevice(UsbDevice device) {
        // RTL-SDR vendor ID is 0x0bda
        return device.getVendorId() == 0x0bda && 
               (device.getProductId() == 0x2838 || device.getProductId() == 0x2832);
    }
} 