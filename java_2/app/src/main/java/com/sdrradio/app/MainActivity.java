package com.sdrradio.app;

import android.Manifest;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.sdrradio.app.databinding.ActivityMainBinding;
import com.sdrradio.app.service.SDRService;
import com.sdrradio.app.utils.RTLSDROperations;
import com.sdrradio.app.utils.AudioProcessor;
import com.sdrradio.app.utils.SpectrumAnalyzer;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private static final int PERMISSION_REQUEST_CODE = 1001;
    private static final int USB_PERMISSION_REQUEST_CODE = 1002;
    private static final String ACTION_USB_PERMISSION = "com.sdrradio.app.USB_PERMISSION";
    
    private ActivityMainBinding binding;
    private UsbManager usbManager;
    private UsbDevice rtlSdrDevice;
    private UsbDeviceConnection deviceConnection;
    private RTLSDROperations rtlSdrOperations;
    private AudioProcessor audioProcessor;
    private SpectrumAnalyzer spectrumAnalyzer;
    private MediaRecorder mediaRecorder;
    private File recordingFile;
    
    private boolean isConnected = false;
    private boolean isPlaying = false;
    private boolean isRecording = false;
    private int currentFrequency = 100000000; // 100 MHz default (dentro do intervalo 24MHz-1.766GHz)
    private int currentVolume = 50;
    private float signalStrength = 0;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    private Runnable spectrumUpdateRunnable;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        
        setSupportActionBar(binding.toolbar);
        
        initializeComponents();
        setupListeners();
        checkPermissions();
        initializeUSB();
        setupSpectrumChart();
        
        // Initialize frequency slider after layout is ready
        binding.getRoot().post(() -> {
            binding.frequencySlider.setValue(currentFrequency);
            updateFrequencyDisplay();
        });
    }
    
    private void initializeComponents() {
        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        rtlSdrOperations = new RTLSDROperations();
        audioProcessor = new AudioProcessor();
        spectrumAnalyzer = new SpectrumAnalyzer();
        
        // Initialize volume slider
        binding.volumeSlider.setValue(currentVolume);
        
        // Set audio mode
        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        audioManager.setMode(AudioManager.MODE_NORMAL);
        audioManager.setSpeakerphoneOn(true);
    }
    
    private void setupListeners() {
        // Connect button
        binding.connectButton.setOnClickListener(v -> {
            if (isConnected) {
                disconnectDevice();
            } else {
                connectDevice();
            }
        });
        
        // Frequency controls
        binding.frequencySlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                currentFrequency = (int) value;
                updateFrequencyDisplay();
                if (isConnected) {
                    setFrequency(currentFrequency);
                }
            }
        });
        
        binding.freqDownButton.setOnClickListener(v -> {
            currentFrequency -= 1000000; // 1 MHz step
            if (currentFrequency < 24000000) currentFrequency = 24000000;
            binding.frequencySlider.setValue(currentFrequency);
            updateFrequencyDisplay();
            if (isConnected) {
                setFrequency(currentFrequency);
            }
        });
        
        binding.freqUpButton.setOnClickListener(v -> {
            currentFrequency += 1000000; // 1 MHz step
            if (currentFrequency > 1766000000) currentFrequency = 1766000000;
            binding.frequencySlider.setValue(currentFrequency);
            updateFrequencyDisplay();
            if (isConnected) {
                setFrequency(currentFrequency);
            }
        });
        
        // Volume controls
        binding.volumeSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                currentVolume = (int) value;
                setVolume(currentVolume);
            }
        });
        
        binding.muteButton.setOnClickListener(v -> {
            if (currentVolume > 0) {
                binding.volumeSlider.setValue(0);
                currentVolume = 0;
                setVolume(0);
            } else {
                binding.volumeSlider.setValue(50);
                currentVolume = 50;
                setVolume(50);
            }
        });
        
        binding.playButton.setOnClickListener(v -> {
            if (isPlaying) {
                stopAudio();
            } else {
                startAudio();
            }
        });
        
        // Recording controls
        binding.recordButton.setOnClickListener(v -> {
            if (isRecording) {
                stopRecording();
            } else {
                startRecording();
            }
        });
        
        // Quick actions FAB
        binding.fabQuickActions.setOnClickListener(v -> showQuickActionsDialog());
    }
    
    private void checkPermissions() {
        Log.d(TAG, "Verificando permissões...");
        
        String[] permissions = {
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
        };
        
        List<String> permissionsToRequest = new ArrayList<>();
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "Permissão necessária: " + permission);
                permissionsToRequest.add(permission);
            } else {
                Log.d(TAG, "Permissão já concedida: " + permission);
            }
        }
        
        if (!permissionsToRequest.isEmpty()) {
            Log.d(TAG, "Solicitando " + permissionsToRequest.size() + " permissões...");
            
            // Mostrar diálogo explicativo antes de solicitar permissões
            new AlertDialog.Builder(this)
                .setTitle("Permissões Necessárias")
                .setMessage("O aplicativo precisa das seguintes permissões para funcionar:\n\n" +
                           "• Microfone: Para gravar áudio\n" +
                           "• Armazenamento: Para salvar gravações\n\n" +
                           "Clique em 'Conceder' para continuar.")
                .setPositiveButton("Conceder", (dialog, which) -> {
                    ActivityCompat.requestPermissions(this, 
                        permissionsToRequest.toArray(new String[0]), 
                        PERMISSION_REQUEST_CODE);
                })
                .setNegativeButton("Cancelar", (dialog, which) -> {
                    Toast.makeText(this, "Permissões necessárias não concedidas", Toast.LENGTH_LONG).show();
                    finish();
                })
                .setCancelable(false)
                .show();
        } else {
            Log.d(TAG, "Todas as permissões já concedidas");
        }
    }
    
    private void initializeUSB() {
        // Register USB receiver
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(usbReceiver, filter);
        }
        
        // Check for already connected devices
        findRTLSDRDevice();
    }
    
    private void findRTLSDRDevice() {
        Log.d(TAG, "Procurando dispositivos RTL-SDR...");
        if (usbManager == null) {
            Log.e(TAG, "UsbManager é null");
            return;
        }
        
        Map<String, UsbDevice> deviceList = usbManager.getDeviceList();
        Log.d(TAG, "Total de dispositivos USB encontrados: " + deviceList.size());
        
        for (UsbDevice device : deviceList.values()) {
            Log.d(TAG, "Dispositivo: " + device.getDeviceName() + 
                      " (VID: 0x" + Integer.toHexString(device.getVendorId()) + 
                      ", PID: 0x" + Integer.toHexString(device.getProductId()) + ")");
            
            if (isRTLSDRDevice(device)) {
                Log.d(TAG, "RTL-SDR encontrado: " + device.getDeviceName());
                rtlSdrDevice = device;
                updateDeviceStatus("RTL-SDR encontrado: " + device.getDeviceName());
                return;
            }
        }
        
        Log.w(TAG, "Nenhum dispositivo RTL-SDR encontrado");
        updateDeviceStatus("Nenhum dispositivo RTL-SDR encontrado");
    }
    
    private boolean isRTLSDRDevice(UsbDevice device) {
        // RTL-SDR vendor ID is 0x0bda
        boolean isRTL = device.getVendorId() == 0x0bda && 
               (device.getProductId() == 0x2838 || device.getProductId() == 0x2832);
        
        Log.d(TAG, "Verificando se é RTL-SDR: VID=0x" + Integer.toHexString(device.getVendorId()) + 
                  ", PID=0x" + Integer.toHexString(device.getProductId()) + " -> " + isRTL);
        
        return isRTL;
    }
    
    private void connectDevice() {
        Log.d(TAG, "Tentando conectar dispositivo...");
        
        if (rtlSdrDevice == null) {
            Log.e(TAG, "Nenhum dispositivo RTL-SDR encontrado");
            Toast.makeText(this, "Nenhum dispositivo RTL-SDR encontrado", Toast.LENGTH_SHORT).show();
            return;
        }
        
        Log.d(TAG, "Dispositivo encontrado: " + rtlSdrDevice.getDeviceName());
        
        if (!usbManager.hasPermission(rtlSdrDevice)) {
            Log.d(TAG, "Solicitando permissão USB...");
            requestUsbPermission();
            return;
        }
        
        Log.d(TAG, "Permissão USB já concedida");
        
        try {
            Log.d(TAG, "Abrindo conexão USB...");
            deviceConnection = usbManager.openDevice(rtlSdrDevice);
            if (deviceConnection == null) {
                Log.e(TAG, "Erro ao abrir conexão USB");
                Toast.makeText(this, "Erro ao abrir conexão USB", Toast.LENGTH_SHORT).show();
                return;
            }
            
            Log.d(TAG, "Conexão USB aberta com sucesso");
            
            // Initialize RTL-SDR
            Log.d(TAG, "Inicializando RTL-SDR...");
            if (rtlSdrOperations.initialize(deviceConnection, rtlSdrDevice)) {
                Log.d(TAG, "RTL-SDR inicializado com sucesso");
                isConnected = true;
                updateConnectionStatus();
                setFrequency(currentFrequency);
                startSpectrumUpdate();
                Toast.makeText(this, "Dispositivo conectado com sucesso", Toast.LENGTH_SHORT).show();
            } else {
                Log.e(TAG, "Erro ao inicializar RTL-SDR");
                Toast.makeText(this, "Erro ao inicializar RTL-SDR", Toast.LENGTH_SHORT).show();
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao conectar dispositivo", e);
            Toast.makeText(this, "Erro ao conectar dispositivo: " + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }
    
    private void disconnectDevice() {
        if (isRecording) {
            stopRecording();
        }
        if (isPlaying) {
            stopAudio();
        }
        
        stopSpectrumUpdate();
        
        if (rtlSdrOperations != null) {
            rtlSdrOperations.close();
        }
        
        if (deviceConnection != null) {
            deviceConnection.close();
            deviceConnection = null;
        }
        
        isConnected = false;
        updateConnectionStatus();
        Toast.makeText(this, "Dispositivo desconectado", Toast.LENGTH_SHORT).show();
    }
    

    
    private void setFrequency(int frequency) {
        if (rtlSdrOperations != null && isConnected) {
            rtlSdrOperations.setFrequency(frequency);
            updateFrequencyDisplay();
        }
    }
    
    private void updateFrequencyDisplay() {
        float freqMHz = currentFrequency / 1000000.0f;
        binding.currentFrequencyText.setText(String.format(Locale.getDefault(), "%.3f MHz", freqMHz));
    }
    
    private void setVolume(int volume) {
        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        int maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
        int volumeLevel = (volume * maxVolume) / 100;
        audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volumeLevel, 0);
    }
    
    private void startAudio() {
        if (!isConnected) {
            Toast.makeText(this, "Conecte um dispositivo primeiro", Toast.LENGTH_SHORT).show();
            return;
        }
        
        try {
            audioProcessor.startPlayback();
            isPlaying = true;
            binding.playButton.setText("⏸");
            binding.playButton.setBackgroundTintList(getColorStateList(R.color.accent_orange));
            Toast.makeText(this, "Reprodução iniciada", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Log.e(TAG, "Erro ao iniciar reprodução", e);
            Toast.makeText(this, "Erro ao iniciar reprodução", Toast.LENGTH_SHORT).show();
        }
    }
    
    private void stopAudio() {
        try {
            audioProcessor.stopPlayback();
            isPlaying = false;
            binding.playButton.setText("▶");
            binding.playButton.setBackgroundTintList(getColorStateList(R.color.primary_blue));
            Toast.makeText(this, "Reprodução parada", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Log.e(TAG, "Erro ao parar reprodução", e);
        }
    }
    
    private void startRecording() {
        if (!isConnected) {
            Toast.makeText(this, "Conecte um dispositivo primeiro", Toast.LENGTH_SHORT).show();
            return;
        }
        
        try {
            // Create recording directory
            File recordingsDir = new File(Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_MUSIC), "SDR_Recordings");
            if (!recordingsDir.exists()) {
                recordingsDir.mkdirs();
            }
            
            // Create recording file
            String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(new Date());
            recordingFile = new File(recordingsDir, "SDR_Recording_" + timestamp + ".wav");
            
            // Initialize MediaRecorder
            mediaRecorder = new MediaRecorder();
            mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
            mediaRecorder.setOutputFile(recordingFile.getAbsolutePath());
            
            mediaRecorder.prepare();
            mediaRecorder.start();
            
            isRecording = true;
            binding.recordButton.setText("Parar Gravação");
            binding.recordButton.setBackgroundTintList(getColorStateList(R.color.accent_green));
            binding.recordingStatusText.setText("Gravando...");
            binding.recordingStatusText.setTextColor(getColor(R.color.accent_red));
            
            Toast.makeText(this, "Gravação iniciada", Toast.LENGTH_SHORT).show();
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao iniciar gravação", e);
            Toast.makeText(this, "Erro ao iniciar gravação", Toast.LENGTH_SHORT).show();
        }
    }
    
    private void stopRecording() {
        try {
            if (mediaRecorder != null) {
                mediaRecorder.stop();
                mediaRecorder.release();
                mediaRecorder = null;
            }
            
            isRecording = false;
            binding.recordButton.setText("Iniciar Gravação");
            binding.recordButton.setBackgroundTintList(getColorStateList(R.color.accent_red));
            binding.recordingStatusText.setText("Gravação salva em: " + recordingFile.getName());
            binding.recordingStatusText.setTextColor(getColor(R.color.accent_green));
            
            Toast.makeText(this, "Gravação salva: " + recordingFile.getName(), Toast.LENGTH_LONG).show();
            
        } catch (Exception e) {
            Log.e(TAG, "Erro ao parar gravação", e);
            Toast.makeText(this, "Erro ao parar gravação", Toast.LENGTH_SHORT).show();
        }
    }
    
    private void setupSpectrumChart() {
        LineChart chart = binding.spectrumChart;
        chart.getDescription().setEnabled(false);
        chart.setTouchEnabled(true);
        chart.setDragEnabled(true);
        chart.setScaleEnabled(true);
        chart.setPinchZoom(true);
        chart.setBackgroundColor(getColor(R.color.card_background));
        
        XAxis xAxis = chart.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setTextColor(getColor(R.color.text_primary));
        xAxis.setDrawGridLines(true);
        xAxis.setGridColor(getColor(R.color.background_light));
        
        chart.getAxisLeft().setTextColor(getColor(R.color.text_primary));
        chart.getAxisLeft().setGridColor(getColor(R.color.background_light));
        chart.getAxisRight().setEnabled(false);
        chart.getLegend().setEnabled(false);
    }
    
    private void startSpectrumUpdate() {
        spectrumUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                if (isConnected && rtlSdrOperations != null) {
                    // Get spectrum data
                    float[] spectrumData = rtlSdrOperations.getSpectrumData();
                    if (spectrumData != null) {
                        updateSpectrumChart(spectrumData);
                        updateSignalStrength(spectrumData);
                    }
                }
                handler.postDelayed(this, 100); // Update every 100ms
            }
        };
        handler.post(spectrumUpdateRunnable);
    }
    
    private void stopSpectrumUpdate() {
        if (spectrumUpdateRunnable != null) {
            handler.removeCallbacks(spectrumUpdateRunnable);
        }
    }
    
    private void updateSpectrumChart(float[] spectrumData) {
        LineChart chart = binding.spectrumChart;
        List<Entry> entries = new ArrayList<>();
        
        for (int i = 0; i < spectrumData.length; i++) {
            entries.add(new Entry(i, spectrumData[i]));
        }
        
        LineDataSet dataSet = new LineDataSet(entries, "Spectrum");
        dataSet.setColor(getColor(R.color.spectrum_medium));
        dataSet.setLineWidth(2f);
        dataSet.setDrawCircles(false);
        dataSet.setDrawValues(false);
        
        LineData lineData = new LineData(dataSet);
        chart.setData(lineData);
        chart.invalidate();
    }
    
    private void updateSignalStrength(float[] spectrumData) {
        // Calculate average signal strength
        float sum = 0;
        for (float value : spectrumData) {
            sum += value;
        }
        signalStrength = sum / spectrumData.length;
        
        // Update UI
        int progress = (int) Math.min(100, signalStrength);
        binding.signalStrengthIndicator.setProgress(progress);
        binding.signalStrengthText.setText(String.format(Locale.getDefault(), "%.1f dB", signalStrength));
        
        // Update color based on signal strength
        int color;
        if (signalStrength < 20) {
            color = getColor(R.color.signal_weak);
        } else if (signalStrength < 40) {
            color = getColor(R.color.signal_medium);
        } else if (signalStrength < 60) {
            color = getColor(R.color.signal_strong);
        } else {
            color = getColor(R.color.signal_excellent);
        }
        binding.signalStrengthIndicator.setIndicatorColor(color);
    }
    
    private void updateConnectionStatus() {
        if (isConnected) {
            binding.connectButton.setText("Desconectar");
            binding.connectButton.setBackgroundTintList(getColorStateList(R.color.accent_red));
            binding.deviceStatusText.setText("Conectado");
            binding.deviceStatusText.setTextColor(getColor(R.color.accent_green));
        } else {
            binding.connectButton.setText("Conectar Dispositivo");
            binding.connectButton.setBackgroundTintList(getColorStateList(R.color.primary_blue));
            binding.deviceStatusText.setText("Desconectado");
            binding.deviceStatusText.setTextColor(getColor(R.color.text_secondary));
        }
    }
    
    private void updateDeviceStatus(String status) {
        binding.deviceStatusText.setText(status);
    }
    
    private void showQuickActionsDialog() {
        String[] actions = {
            "Rádio FM (88-108 MHz)",
            "Rádio AM (535-1605 kHz)",
            "Aeronáutica (118-137 MHz)",
            "Marítima (156-174 MHz)",
            "Meteorologia (162 MHz)",
            "Rádio Amador (144-148 MHz)"
        };
        
        new AlertDialog.Builder(this)
            .setTitle("Frequências Rápidas")
            .setItems(actions, (dialog, which) -> {
                switch (which) {
                    case 0: // FM
                        currentFrequency = 100000000; // 100 MHz
                        break;
                    case 1: // AM
                        currentFrequency = 1000000; // 1 MHz
                        break;
                    case 2: // Aeronáutica
                        currentFrequency = 120000000; // 120 MHz
                        break;
                    case 3: // Marítima
                        currentFrequency = 160000000; // 160 MHz
                        break;
                    case 4: // Meteorologia
                        currentFrequency = 162000000; // 162 MHz
                        break;
                    case 5: // Rádio Amador
                        currentFrequency = 146000000; // 146 MHz
                        break;
                }
                binding.frequencySlider.setValue(currentFrequency);
                updateFrequencyDisplay();
                if (isConnected) {
                    setFrequency(currentFrequency);
                }
            })
            .show();
    }
    
    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
                            Log.d(TAG, "Permissão USB concedida para: " + device.getDeviceName());
                            Toast.makeText(MainActivity.this, "Permissão USB concedida", Toast.LENGTH_SHORT).show();
                            // Tentar conectar novamente
                            connectDevice();
                        }
                    } else {
                        Log.e(TAG, "Permissão USB negada para: " + (device != null ? device.getDeviceName() : "dispositivo desconhecido"));
                        Toast.makeText(MainActivity.this, "Permissão USB negada", Toast.LENGTH_LONG).show();
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null && isRTLSDRDevice(device)) {
                    Log.d(TAG, "RTL-SDR conectado: " + device.getDeviceName());
                    rtlSdrDevice = device;
                    updateDeviceStatus("RTL-SDR conectado: " + device.getDeviceName());
                    Toast.makeText(MainActivity.this, "RTL-SDR detectado", Toast.LENGTH_SHORT).show();
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null && device.equals(rtlSdrDevice)) {
                    Log.d(TAG, "RTL-SDR desconectado: " + device.getDeviceName());
                    if (isConnected) {
                        disconnectDevice();
                    }
                    rtlSdrDevice = null;
                    updateDeviceStatus("RTL-SDR desconectado");
                    Toast.makeText(MainActivity.this, "RTL-SDR desconectado", Toast.LENGTH_SHORT).show();
                }
            }
        }
    };
    
    private void requestUsbPermission() {
        Log.d(TAG, "Solicitando permissão USB para dispositivo: " + rtlSdrDevice.getDeviceName());
        
        // Mostrar diálogo explicativo
        new AlertDialog.Builder(this)
            .setTitle("Permissão USB Necessária")
            .setMessage("O aplicativo precisa de permissão para acessar o dispositivo RTL-SDR via USB.\n\n" +
                       "Dispositivo: " + rtlSdrDevice.getDeviceName() + "\n" +
                       "Vendor ID: 0x" + Integer.toHexString(rtlSdrDevice.getVendorId()) + "\n" +
                       "Product ID: 0x" + Integer.toHexString(rtlSdrDevice.getProductId()) + "\n\n" +
                       "Clique em 'Conceder' para continuar.")
            .setPositiveButton("Conceder", (dialog, which) -> {
                PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, 
                    new Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_IMMUTABLE);
                
                usbManager.requestPermission(rtlSdrDevice, permissionIntent);
                
                Toast.makeText(this, "Solicitando permissão USB...", Toast.LENGTH_SHORT).show();
            })
            .setNegativeButton("Cancelar", (dialog, which) -> {
                Toast.makeText(this, "Permissão USB negada", Toast.LENGTH_SHORT).show();
            })
            .setCancelable(false)
            .show();
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean allGranted = true;
            for (int i = 0; i < permissions.length; i++) {
                if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                    Log.e(TAG, "Permissão negada: " + permissions[i]);
                    allGranted = false;
                } else {
                    Log.d(TAG, "Permissão concedida: " + permissions[i]);
                }
            }
            
            if (!allGranted) {
                Log.e(TAG, "Algumas permissões foram negadas");
                Toast.makeText(this, "Permissões necessárias não concedidas", Toast.LENGTH_LONG).show();
            } else {
                Log.d(TAG, "Todas as permissões foram concedidas");
                Toast.makeText(this, "Todas as permissões concedidas", Toast.LENGTH_SHORT).show();
            }
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        
        if (isRecording) {
            stopRecording();
        }
        
        if (isPlaying) {
            stopAudio();
        }
        
        if (isConnected) {
            disconnectDevice();
        }
        
        if (usbReceiver != null) {
            unregisterReceiver(usbReceiver);
        }
        
        stopSpectrumUpdate();
    }
} 