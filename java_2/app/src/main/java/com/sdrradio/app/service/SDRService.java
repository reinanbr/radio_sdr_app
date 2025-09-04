package com.sdrradio.app.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import com.sdrradio.app.MainActivity;
import com.sdrradio.app.R;
import com.sdrradio.app.utils.RTLSDROperations;

public class SDRService extends Service {
    private static final String TAG = "SDRService";
    private static final String CHANNEL_ID = "SDR_Radio_Channel";
    private static final int NOTIFICATION_ID = 1;
    
    private RTLSDROperations rtlSdrOperations;
    private boolean isRunning = false;
    
    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        rtlSdrOperations = new RTLSDROperations();
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null) {
            String action = intent.getAction();
            
            if ("START_SDR".equals(action)) {
                startSDRProcessing();
            } else if ("STOP_SDR".equals(action)) {
                stopSDRProcessing();
            }
        }
        
        return START_STICKY;
    }
    
    private void startSDRProcessing() {
        if (isRunning) return;
        
        isRunning = true;
        startForeground(NOTIFICATION_ID, createNotification("SDR Radio Ativo", "Processando sinais de rádio"));
        
        // Start background processing
        new Thread(() -> {
            while (isRunning) {
                try {
                    // Process SDR data in background
                    if (rtlSdrOperations != null && rtlSdrOperations.isInitialized()) {
                        // Process spectrum data
                        float[] spectrumData = rtlSdrOperations.getSpectrumData();
                        if (spectrumData != null) {
                            // Process spectrum data here
                            processSpectrumData(spectrumData);
                        }
                    }
                    
                    Thread.sleep(100); // 10 FPS
                    
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    break;
                } catch (Exception e) {
                    Log.e(TAG, "Erro durante processamento SDR", e);
                }
            }
        }).start();
        
        Log.i(TAG, "SDR Service iniciado");
    }
    
    private void stopSDRProcessing() {
        isRunning = false;
        stopForeground(true);
        stopSelf();
        Log.i(TAG, "SDR Service parado");
    }
    
    private void processSpectrumData(float[] spectrumData) {
        // Process spectrum data in background
        // This could include signal detection, recording, etc.
        
        // Example: Detect strong signals
        for (int i = 0; i < spectrumData.length; i++) {
            if (spectrumData[i] > 50) { // Threshold for strong signal
                Log.d(TAG, "Sinal forte detectado na posição " + i);
            }
        }
    }
    
    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "SDR Radio",
                NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Canal para notificações do SDR Radio");
            
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }
    
    private Notification createNotification(String title, String content) {
        Intent notificationIntent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
            this, 0, notificationIntent, PendingIntent.FLAG_IMMUTABLE
        );
        
        return new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle(title)
            .setContentText(content)
            .setSmallIcon(R.mipmap.ic_launcher)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
            .build();
    }
    
    public void updateNotification(String title, String content) {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.notify(NOTIFICATION_ID, createNotification(title, content));
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        stopSDRProcessing();
        
        if (rtlSdrOperations != null) {
            rtlSdrOperations.close();
        }
        
        Log.i(TAG, "SDR Service destruído");
    }
    
    public boolean isRunning() {
        return isRunning;
    }
} 