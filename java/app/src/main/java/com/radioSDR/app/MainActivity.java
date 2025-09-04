package com.radioSDR.app;

import android.Manifest;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Description;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "RadioSDR";
    private static final String ACTION_USB_PERMISSION = "com.radioSDR.app.USB_PERMISSION";
    private static final int PERMISSION_REQUEST_CODE = 1001;
    
    // Permissões básicas para versões anteriores ao Android 13
    private static final String[] REQUIRED_PERMISSIONS_LEGACY = {
        Manifest.permission.RECORD_AUDIO
    };
    
    // Permissões para Android 13+ (API 33+)
    private static final String[] REQUIRED_PERMISSIONS_API33 = {
        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.POST_NOTIFICATIONS
    };
    
    // UI Components
    private TextView tvConnectionStatus;
    private Button btnConnect, btnStart, btnSpectrum, btnSettings, btnRecord;
    private EditText etFrequency;
    private SeekBar seekBarFrequency, seekBarGain, seekBarVolume;
    private Switch switchAutoGain;
    private TextView tvGainValue;
    private RadioGroup radioGroupDemod;
    private ImageButton btnMute;
    private LineChart chartSpectrum;
    
    // USB and RTL-SDR
    private UsbManager usbManager;
    private UsbDevice rtlDevice;
    private UsbDeviceConnection connection;
    private boolean isConnected = false;
    private boolean isRunning = false;
    
    // Audio
    private AudioTrack audioTrack;
    private boolean isMuted = false;
    private boolean isRecording = false;
    
    // SDR Parameters
    private double currentFrequency = 88.5; // MHz
    private int currentGain = 248; // 0.1 dB units
    private boolean autoGain = true;
    private int sampleRate = 2048000; // 2.048 MHz
    private DemodulationType demodType = DemodulationType.FM;
    
    // Native library (temporariamente desabilitado para teste)
    // static {
    //     System.loadLibrary("radiosdr");
    // }
    
    // Native methods (temporariamente desabilitados para teste)
    // public native boolean initRTLSDR(int fd);
    // public native void closeRTLSDR();
    // public native boolean setFrequency(double freq);
    // public native boolean setGain(int gain);
    // public native boolean setAutoGain(boolean enable);
    // public native boolean setSampleRate(int rate);
    // public native boolean startReading();
    // public native void stopReading();
    // public native float[] getSpectrumData();
    // public native short[] getAudioData();
    
    // Métodos stub para teste
    public boolean initRTLSDR(int fd) { return true; }
    public void closeRTLSDR() { }
    public boolean setFrequency(double freq) { return true; }
    public boolean setGain(int gain) { return true; }
    public boolean setAutoGain(boolean enable) { return true; }
    public boolean setSampleRate(int rate) { return true; }
    public boolean startReading() { return true; }
    public void stopReading() { }
    public float[] getSpectrumData() { return new float[0]; }
    public short[] getAudioData() { return new short[0]; }
    
    public enum DemodulationType {
        FM, AM, USB, LSB
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        initViews();
        initUSB();
        setupListeners();
        checkPermissions();
        setupSpectrum();
    }
    
    private void initViews() {
        tvConnectionStatus = findViewById(R.id.tvConnectionStatus);
        btnConnect = findViewById(R.id.btnConnect);
        btnStart = findViewById(R.id.btnStart);
        btnSpectrum = findViewById(R.id.btnSpectrum);
        btnSettings = findViewById(R.id.btnSettings);
        btnRecord = findViewById(R.id.btnRecord);
        etFrequency = findViewById(R.id.etFrequency);
        seekBarFrequency = findViewById(R.id.seekBarFrequency);
        seekBarGain = findViewById(R.id.seekBarGain);
        seekBarVolume = findViewById(R.id.seekBarVolume);
        switchAutoGain = findViewById(R.id.switchAutoGain);
        tvGainValue = findViewById(R.id.tvGainValue);
        radioGroupDemod = findViewById(R.id.radioGroupDemod);
        btnMute = findViewById(R.id.btnMute);
        chartSpectrum = findViewById(R.id.chartSpectrum);
    }
    
    private void initUSB() {
        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        
        // Register USB receiver
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        
        // For Android 13+ (API 33+), specify receiver export flag
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(usbReceiver, filter);
        }
    }
    
    private void setupListeners() {
        btnConnect.setOnClickListener(v -> {
            Log.d(TAG, "Botão Conectar clicado!");
            Toast.makeText(this, "Botão Conectar pressionado", Toast.LENGTH_SHORT).show();
            connectToDevice();
        });
        btnStart.setOnClickListener(v -> toggleSDR());
        btnSpectrum.setOnClickListener(v -> openSpectrum());
        btnSettings.setOnClickListener(v -> openSettings());
        btnRecord.setOnClickListener(v -> toggleRecording());
        btnMute.setOnClickListener(v -> toggleMute());
        
        // Frequency controls
        etFrequency.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
            
            @Override
            public void afterTextChanged(Editable s) {
                try {
                    double freq = Double.parseDouble(s.toString());
                    if (freq >= 24.0 && freq <= 1766.0) {
                        currentFrequency = freq;
                        seekBarFrequency.setProgress((int)((freq - 24.0) * 1000.0 / 1742.0));
                        if (isConnected) {
                            setFrequency(freq);
                        }
                    }
                } catch (NumberFormatException e) {
                    // Ignore invalid input
                }
            }
        });
        
        seekBarFrequency.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    double freq = 24.0 + (progress * 1742.0 / 1000.0);
                    currentFrequency = freq;
                    etFrequency.setText(String.format("%.1f", freq));
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        
        // Gain controls
        switchAutoGain.setOnCheckedChangeListener((buttonView, isChecked) -> {
            autoGain = isChecked;
            seekBarGain.setEnabled(!isChecked);
            if (isConnected) {
                setAutoGain(isChecked);
            }
        });
        
        seekBarGain.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && !autoGain) {
                    currentGain = progress;
                    tvGainValue.setText(String.format("%.1f dB", progress / 10.0));
                    if (isConnected) {
                        setGain(progress);
                    }
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        
        // Volume control
        seekBarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && audioTrack != null) {
                    float volume = progress / 100.0f;
                    audioTrack.setVolume(volume);
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        
        // Demodulation type
        radioGroupDemod.setOnCheckedChangeListener((group, checkedId) -> {
            if (checkedId == R.id.radioFM) {
                demodType = DemodulationType.FM;
            } else if (checkedId == R.id.radioAM) {
                demodType = DemodulationType.AM;
            } else if (checkedId == R.id.radioUSB) {
                demodType = DemodulationType.USB;
            } else if (checkedId == R.id.radioLSB) {
                demodType = DemodulationType.LSB;
            }
        });
    }
    
    private void setupSpectrum() {
        chartSpectrum.setDrawGridBackground(false);
        chartSpectrum.setDrawBorders(false);
        chartSpectrum.setTouchEnabled(true);
        chartSpectrum.setDragEnabled(true);
        chartSpectrum.setScaleEnabled(true);
        
        Description desc = new Description();
        desc.setText("");
        chartSpectrum.setDescription(desc);
        
        XAxis xAxis = chartSpectrum.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setGranularity(1f);
        
        YAxis leftAxis = chartSpectrum.getAxisLeft();
        leftAxis.setDrawGridLines(true);
        
        YAxis rightAxis = chartSpectrum.getAxisRight();
        rightAxis.setEnabled(false);
        
        // Start spectrum update timer
        Handler handler = new Handler(Looper.getMainLooper());
        Runnable spectrumUpdater = new Runnable() {
            @Override
            public void run() {
                if (isRunning) {
                    updateSpectrum();
                }
                handler.postDelayed(this, 100); // Update every 100ms
            }
        };
        handler.post(spectrumUpdater);
    }
    
    private void updateSpectrum() {
        if (!isRunning) return;
        
        float[] spectrumData = getSpectrumData();
        if (spectrumData != null && spectrumData.length > 0) {
            List<Entry> entries = new ArrayList<>();
            for (int i = 0; i < spectrumData.length; i++) {
                entries.add(new Entry(i, spectrumData[i]));
            }
            
            LineDataSet dataSet = new LineDataSet(entries, "Spectrum");
            dataSet.setDrawCircles(false);
            dataSet.setDrawValues(false);
            dataSet.setColor(getColor(R.color.blue));
            dataSet.setLineWidth(1f);
            
            LineData lineData = new LineData(dataSet);
            chartSpectrum.setData(lineData);
            chartSpectrum.invalidate();
        }
    }
    
    private void checkPermissions() {
        // Determinar quais permissões usar baseado na versão do Android
        String[] permissions = android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU ? 
            REQUIRED_PERMISSIONS_API33 : REQUIRED_PERMISSIONS_LEGACY;
        
        List<String> permissionsToRequest = new ArrayList<>();
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                permissionsToRequest.add(permission);
                Log.d(TAG, "Permissão necessária: " + permission);
            }
        }
        
        if (!permissionsToRequest.isEmpty()) {
            Log.d(TAG, "Solicitando " + permissionsToRequest.size() + " permissões");
            Toast.makeText(this, "Permissões necessárias para gravação de áudio", Toast.LENGTH_LONG).show();
            ActivityCompat.requestPermissions(this, 
                permissionsToRequest.toArray(new String[0]), 
                PERMISSION_REQUEST_CODE);
        } else {
            Log.d(TAG, "Todas as permissões já foram concedidas");
            Toast.makeText(this, "Todas as permissões concedidas", Toast.LENGTH_SHORT).show();
        }
    }
    
    private void connectToDevice() {
        Log.d(TAG, "=== INICIANDO connectToDevice() ===");
         Toast.makeText(this, "=== INICIANDO connectToDevice() ===", Toast.LENGTH_SHORT).show();
        
        if (usbManager == null) {
            Log.e(TAG, "ERRO: usbManager é null!");
            Toast.makeText(this, "Erro: Sistema USB não inicializado", Toast.LENGTH_LONG).show();
            return;
        }
        
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        
        Log.d(TAG, "Procurando por dispositivos RTL-SDR...");
        Toast.makeText(this, "Procurando por dispositivos RTL-SDR...", Toast.LENGTH_SHORT).show();
        Log.d(TAG, "Dispositivos USB conectados: " + deviceList.size());
        Toast.makeText(this, "Dispositivos USB conectados: " + deviceList.size(), Toast.LENGTH_SHORT).show();

        // Lista todos os dispositivos USB conectados para debug
        for (UsbDevice device : deviceList.values()) {
            Log.d(TAG, String.format("Dispositivo encontrado: VID=0x%04X, PID=0x%04X, Nome=%s", 
                device.getVendorId(), device.getProductId(), device.getDeviceName()));
        }
        
        for (UsbDevice device : deviceList.values()) {
            if (isRTLSDRDevice(device)) {
                Log.d(TAG, "Dispositivo RTL-SDR encontrado!");
                Toast.makeText(this, "Dispositivo RTL-SDR encontrado!", Toast.LENGTH_SHORT).show();
                rtlDevice = device;
                
                // Verificar se já temos permissão
                if (usbManager.hasPermission(device)) {
                    Log.d(TAG, "Permissão USB já concedida - conectando diretamente");
                    Toast.makeText(this, "Conectando ao dispositivo...", Toast.LENGTH_SHORT).show();
                    connectDevice(device);
                } else {
                    Log.d(TAG, "Solicitando permissão USB para o dispositivo");
                    Toast.makeText(this, "Solicitando permissão USB...\nPermita o acesso quando solicitado.", 
                        Toast.LENGTH_LONG).show();
                    requestUSBPermission(device);
                }
                return;
            }
        }
        
        // Não encontrou dispositivo RTL-SDR
        Log.w(TAG, "Nenhum dispositivo RTL-SDR encontrado");
        String message = "Nenhum dispositivo RTL-SDR encontrado.\n\n";
        
        if (deviceList.isEmpty()) {
            message += "• Nenhum dispositivo USB conectado\n";
            message += "• Conecte o dongle RTL-SDR via cabo USB OTG";
        } else {
            message += "Dispositivos USB conectados:\n";
            for (UsbDevice device : deviceList.values()) {
                message += String.format("• VID=0x%04X, PID=0x%04X\n", 
                    device.getVendorId(), device.getProductId());
            }
            message += "\nVerifique se é um dispositivo RTL-SDR compatível.";
        }
        
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }
    
    private boolean isRTLSDRDevice(UsbDevice device) {
        int vendorId = device.getVendorId();
        int productId = device.getProductId();
        
        Log.d(TAG, String.format("Verificando dispositivo: VID=0x%04X, PID=0x%04X", vendorId, productId));
        
        // Lista expandida de dispositivos RTL-SDR conhecidos
        boolean isRTLSDR = false;
        
        // Realtek RTL2832U baseados
        if (vendorId == 0x0BDA) { // Realtek
            isRTLSDR = (productId == 0x2832 || productId == 0x2838 || 
                       productId == 0x2834 || productId == 0x2836);
            if (isRTLSDR) Log.d(TAG, "Dispositivo Realtek RTL2832U detectado");
        }
        
        // RTL2832U R820T2 TCXO+BIAS T+NF (variante específica)
        // Esta variante inclui:
        // - R820T2 tuner (melhor performance que R820T)
        // - TCXO (Temperature Compensated Crystal Oscillator) para maior estabilidade
        // - BIAS T para alimentar amplificadores LNA externos
        // - NF (Noise Figure) otimizado para baixo ruído
        if (vendorId == 0x0BDA && productId == 0x2838) {
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo RTL2832U R820T2 TCXO+BIAS T+NF detectado");
        }
        
        // RTL2832U com tuner R820T2 (versões com TCXO)
        if (vendorId == 0x0BDA && (productId == 0x2838 || productId == 0x2839)) {
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo RTL2832U com tuner R820T2 detectado");
        }
        
        // Outros chips comuns
        if (vendorId == 0x0483 && productId == 0x5740) { // STMicroelectronics
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo STMicroelectronics detectado");
        }
        
        // Dongles genéricos comuns
        if (vendorId == 0x1554 && productId == 0x5020) { // PixelView
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo PixelView detectado");
        }
        
        if (vendorId == 0x15F4 && productId == 0x0131) { // HanfTek
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo HanfTek detectado");
        }
        
        if (vendorId == 0x1D19) { // Dexatek
            isRTLSDR = (productId == 0x1101 || productId == 0x1102 || productId == 0x1103);
            if (isRTLSDR) Log.d(TAG, "Dispositivo Dexatek detectado");
        }
        
        // Terratec
        if (vendorId == 0x0CCD && productId == 0x00A9) {
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo Terratec detectado");
        }
        
        // NooElec dongles e outras variantes RTL2832U
        if (vendorId == 0x0BDA && productId == 0x2838) {
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo NooElec NESDR ou RTL2832U R820T2 detectado");
        }
        
        // Generic RTL2832U dongles com diferentes PIDs
        if (vendorId == 0x0BDA && (productId >= 0x2830 && productId <= 0x283F)) {
            isRTLSDR = true;
            Log.d(TAG, "Dispositivo RTL2832U genérico detectado (PID range 0x2830-0x283F)");
        }
        
        if (!isRTLSDR) {
            Log.d(TAG, "Dispositivo não é RTL-SDR compatível");
        }
        
        return isRTLSDR;
    }
    
    private void requestUSBPermission(UsbDevice device) {
        Log.d(TAG, "=== SOLICITANDO PERMISSÃO USB ===");
        Log.d(TAG, String.format("Dispositivo: VID=0x%04X, PID=0x%04X, Nome=%s", 
            device.getVendorId(), device.getProductId(), device.getDeviceName()));
        
        PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, 
            new Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_IMMUTABLE);
        
        Log.d(TAG, "Chamando usbManager.requestPermission()...");
        usbManager.requestPermission(device, permissionIntent);
        Log.d(TAG, "Solicitação de permissão enviada - aguardando resposta do usuário");
    }
    
    private void toggleSDR() {
        if (isRunning) {
            stopSDR();
        } else {
            startSDR();
        }
    }
    
    private void startSDR() {
        if (!isConnected) {
            Toast.makeText(this, R.string.error_device_init, Toast.LENGTH_SHORT).show();
            return;
        }
        
        // Initialize audio
        initAudio();
        
        // Start RTL-SDR reading
        if (startReading()) {
            isRunning = true;
            btnStart.setText(R.string.stop_sdr);
            btnRecord.setEnabled(true);
            
            // Start audio processing thread
            startAudioProcessing();
        } else {
            Toast.makeText(this, R.string.error_device_init, Toast.LENGTH_SHORT).show();
        }
    }
    
    private void stopSDR() {
        isRunning = false;
        btnStart.setText(R.string.start_sdr);
        btnRecord.setEnabled(false);
        
        stopReading();
        
        if (audioTrack != null) {
            audioTrack.stop();
            audioTrack.release();
            audioTrack = null;
        }
        
        if (isRecording) {
            toggleRecording();
        }
    }
    
    private void initAudio() {
        int sampleRateAudio = 48000;
        int bufferSize = AudioTrack.getMinBufferSize(
            sampleRateAudio,
            AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        );
        
        audioTrack = new AudioTrack(
            AudioManager.STREAM_MUSIC,
            sampleRateAudio,
            AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_16BIT,
            bufferSize,
            AudioTrack.MODE_STREAM
        );
        
        audioTrack.play();
    }
    
    private void startAudioProcessing() {
        Thread audioThread = new Thread(() -> {
            while (isRunning) {
                short[] audioData = getAudioData();
                if (audioData != null && audioData.length > 0 && audioTrack != null && !isMuted) {
                    audioTrack.write(audioData, 0, audioData.length);
                }
                
                try {
                    Thread.sleep(10); // Small delay to prevent excessive CPU usage
                } catch (InterruptedException e) {
                    break;
                }
            }
        });
        audioThread.start();
    }
    
    private void toggleMute() {
        isMuted = !isMuted;
        btnMute.setImageResource(isMuted ? 
            android.R.drawable.ic_lock_silent_mode : 
            android.R.drawable.ic_lock_silent_mode_off);
    }
    
    private void toggleRecording() {
        if (isRecording) {
            // Stop recording
            isRecording = false;
            btnRecord.setText(R.string.record);
        } else {
            // Start recording
            isRecording = true;
            btnRecord.setText(R.string.stop_record);
        }
    }
    
    private void openSpectrum() {
        Intent intent = new Intent(this, SpectrumActivity.class);
        startActivity(intent);
    }
    
    private void openSettings() {
        Intent intent = new Intent(this, SettingsActivity.class);
        startActivity(intent);
    }
    
    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "USB Receiver: " + action);
            
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
                            Log.d(TAG, "Permissão USB concedida para: " + device.getDeviceName());
                            Toast.makeText(context, "Permissão USB concedida", Toast.LENGTH_SHORT).show();
                            connectDevice(device);
                        }
                    } else {
                        Log.w(TAG, "Permissão USB negada");
                        Toast.makeText(context, R.string.usb_permission_denied, Toast.LENGTH_SHORT).show();
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null) {
                    Log.d(TAG, String.format("Dispositivo USB conectado: VID=0x%04X, PID=0x%04X", 
                        device.getVendorId(), device.getProductId()));
                    
                    if (isRTLSDRDevice(device)) {
                        Log.d(TAG, "Dispositivo RTL-SDR detectado automaticamente!");
                        Toast.makeText(context, "Dispositivo RTL-SDR detectado!\nClique em 'Conectar' para usar.", 
                            Toast.LENGTH_LONG).show();
                        rtlDevice = device;
                    } else {
                        Log.d(TAG, "Dispositivo USB conectado não é RTL-SDR");
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null) {
                    Log.d(TAG, "Dispositivo USB desconectado: " + device.getDeviceName());
                    
                    if (device.equals(rtlDevice)) {
                        Log.d(TAG, "Dispositivo RTL-SDR desconectado");
                        Toast.makeText(context, "Dispositivo RTL-SDR desconectado", Toast.LENGTH_SHORT).show();
                        disconnectDevice();
                    }
                }
            }
        }
    };
    
    private void connectDevice(UsbDevice device) {
        Log.d(TAG, "=== CONECTANDO DISPOSITIVO ===");
        Log.d(TAG, String.format("Dispositivo: VID=0x%04X, PID=0x%04X, Nome=%s", 
            device.getVendorId(), device.getProductId(), device.getDeviceName()));
        
        Log.d(TAG, "Abrindo conexão USB...");
        connection = usbManager.openDevice(device);
        
        if (connection != null) {
            Log.d(TAG, "Conexão USB aberta com sucesso");
            int fd = connection.getFileDescriptor();
            Log.d(TAG, "File descriptor: " + fd);
            
            Log.d(TAG, "Inicializando RTL-SDR nativo...");
            if (initRTLSDR(fd)) {
                Log.d(TAG, "RTL-SDR inicializado com sucesso!");
                isConnected = true;
                tvConnectionStatus.setText(R.string.device_connected);
                tvConnectionStatus.setTextColor(getColor(R.color.green));
                btnStart.setEnabled(true);
                
                // Set initial parameters
                Log.d(TAG, "Configurando parâmetros iniciais...");
                setFrequency(currentFrequency);
                setSampleRate(sampleRate);
                setAutoGain(autoGain);
                if (!autoGain) {
                    setGain(currentGain);
                }
                
                Toast.makeText(this, "Dispositivo RTL-SDR conectado com sucesso!", Toast.LENGTH_LONG).show();
                Log.d(TAG, "=== CONEXÃO CONCLUÍDA COM SUCESSO ===");
            } else {
                Log.e(TAG, "ERRO: Falha ao inicializar RTL-SDR nativo");
                connection.close();
                connection = null;
                Toast.makeText(this, "Erro ao inicializar RTL-SDR", Toast.LENGTH_LONG).show();
            }
        } else {
            Log.e(TAG, "ERRO: Falha ao abrir conexão USB");
            Toast.makeText(this, "Erro ao abrir conexão USB", Toast.LENGTH_LONG).show();
        }
    }
    
    private void disconnectDevice() {
        if (isRunning) {
            stopSDR();
        }
        
        closeRTLSDR();
        
        if (connection != null) {
            connection.close();
            connection = null;
        }
        
        isConnected = false;
        tvConnectionStatus.setText(R.string.device_disconnected);
        tvConnectionStatus.setTextColor(getColor(R.color.red));
        btnStart.setEnabled(false);
        
        Toast.makeText(this, R.string.device_disconnected, Toast.LENGTH_SHORT).show();
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == PERMISSION_REQUEST_CODE) {
            List<String> deniedPermissions = new ArrayList<>();
            List<String> grantedPermissions = new ArrayList<>();

            for (int i = 0; i < permissions.length; i++) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    grantedPermissions.add(permissions[i]);
                    Log.d(TAG, "Permissão concedida: " + permissions[i]);
                } else {
                    deniedPermissions.add(permissions[i]);
                    Log.w(TAG, "Permissão negada: " + permissions[i]);
                }
            }

            if (deniedPermissions.isEmpty()) {
                Toast.makeText(this, "Todas as permissões foram concedidas!", Toast.LENGTH_SHORT).show();
                Log.d(TAG, "Todas as permissões concedidas - pode conectar dispositivo RTL-SDR");
            } else {
                StringBuilder message = new StringBuilder("Permissões negadas: ");
                for (String permission : deniedPermissions) {
                    if (permission.contains("RECORD_AUDIO")) {
                        message.append("Áudio ");
                    } else if (permission.contains("NOTIFICATION")) {
                        message.append("Notificações ");
                    }
                }
                message.append("\nAlgumas funcionalidades podem não funcionar corretamente.");
                Toast.makeText(this, message.toString(), Toast.LENGTH_LONG).show();

                // Logs detalhados
                if (deniedPermissions.contains(Manifest.permission.RECORD_AUDIO)) {
                    Log.w(TAG, "ATENÇÃO: Permissão de áudio negada - processamento de áudio pode não funcionar");
                }
            }
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        
        if (isRunning) {
            stopSDR();
        }
        
        disconnectDevice();
        unregisterReceiver(usbReceiver);
    }
}
