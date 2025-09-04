package com.example.sdrradio;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.android.material.button.MaterialButton;
import com.google.android.material.slider.Slider;
import com.google.android.material.switchmaterial.SwitchMaterial;
import com.google.android.material.textfield.TextInputEditText;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private static final int PERMISSION_REQUEST_CODE = 1001;
    
    // UI Components
    private TextView deviceStatusText;
    private MaterialButton connectButton;
    private TextInputEditText frequencyInput;
    private Slider frequencySlider;
    private TextInputEditText sampleRateInput;
    private Slider gainSlider;
    private SwitchMaterial autoGainSwitch;
    private MaterialButton startRadioButton;
    private MaterialButton stopRadioButton;
    private TextView signalStrengthText;
    
    // Current settings
    private double currentFrequency = 100.0;
    private int currentSampleRate = 2048000;
    private float currentGain = 20.0f;
    private boolean currentAutoGain = true;
    private boolean isDeviceConnected = false;
    private boolean isRadioRunning = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        // Initialize UI components
        initializeViews();
        
        // Request permissions
        requestPermissions();
        
        // Initialize UI state
        updateUIState();
    }
    
    private void initializeViews() {
        deviceStatusText = findViewById(R.id.deviceStatusText);
        connectButton = findViewById(R.id.connectButton);
        frequencyInput = findViewById(R.id.frequencyInput);
        frequencySlider = findViewById(R.id.frequencySlider);
        sampleRateInput = findViewById(R.id.sampleRateInput);
        gainSlider = findViewById(R.id.gainSlider);
        autoGainSwitch = findViewById(R.id.autoGainSwitch);
        startRadioButton = findViewById(R.id.startRadioButton);
        stopRadioButton = findViewById(R.id.stopRadioButton);
        signalStrengthText = findViewById(R.id.signalStrengthText);
        
        // Set up listeners
        connectButton.setOnClickListener(v -> onConnectButtonClicked());
        startRadioButton.setOnClickListener(v -> onStartRadioClicked());
        stopRadioButton.setOnClickListener(v -> onStopRadioClicked());
        
        // Frequency slider listener
        frequencySlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                currentFrequency = value;
                frequencyInput.setText(String.format("%.1f", value));
                updateFrequencyDisplay();
            }
        });
        
        // Gain slider listener
        gainSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                currentGain = value;
                updateGainDisplay();
            }
        });
        
        // Auto gain switch listener
        autoGainSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            currentAutoGain = isChecked;
            gainSlider.setEnabled(!isChecked);
            updateGainDisplay();
        });
    }
    
    private void requestPermissions() {
        String[] permissions = {
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
        };
        
        boolean allGranted = true;
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                allGranted = false;
                break;
            }
        }
        
        if (!allGranted) {
            ActivityCompat.requestPermissions(this, permissions, PERMISSION_REQUEST_CODE);
        }
    }
    
    private void onConnectButtonClicked() {
        if (isDeviceConnected) {
            // Disconnect
            isDeviceConnected = false;
            isRadioRunning = false;
            showToast("Dispositivo desconectado");
        } else {
            // Connect (simulated)
            isDeviceConnected = true;
            showToast("Dispositivo conectado (simulado)");
        }
        updateUIState();
    }
    
    private void onStartRadioClicked() {
        if (!isDeviceConnected) {
            showToast("Dispositivo não conectado");
            return;
        }
        
        // Get current settings from UI
        try {
            String freqText = frequencyInput.getText().toString();
            if (!freqText.isEmpty()) {
                currentFrequency = Double.parseDouble(freqText);
            }
            
            String sampleRateText = sampleRateInput.getText().toString();
            if (!sampleRateText.isEmpty()) {
                currentSampleRate = Integer.parseInt(sampleRateText);
            }
        } catch (NumberFormatException e) {
            showToast("Valores inválidos");
            return;
        }
        
        isRadioRunning = true;
        showToast("Rádio iniciado (simulado) - Frequência: " + currentFrequency + " MHz");
        updateUIState();
    }
    
    private void onStopRadioClicked() {
        isRadioRunning = false;
        showToast("Rádio parado");
        updateUIState();
    }
    
    private void updateUIState() {
        // Update device status
        if (isDeviceConnected) {
            deviceStatusText.setText("Dispositivo conectado (simulado)");
            deviceStatusText.setTextColor(getResources().getColor(R.color.green));
        } else {
            deviceStatusText.setText("Dispositivo não conectado");
            deviceStatusText.setTextColor(getResources().getColor(R.color.red));
        }
        
        // Update button states
        startRadioButton.setEnabled(isDeviceConnected && !isRadioRunning);
        stopRadioButton.setEnabled(isDeviceConnected && isRadioRunning);
        
        // Update button texts
        if (isRadioRunning) {
            startRadioButton.setText("Parar Rádio");
        } else {
            startRadioButton.setText("Iniciar Rádio");
        }
        
        // Update frequency display
        updateFrequencyDisplay();
        
        // Update gain display
        updateGainDisplay();
        
        // Update signal strength (simulated)
        if (isRadioRunning) {
            signalStrengthText.setText("Sinal: -45 dB (simulado)");
        } else {
            signalStrengthText.setText("Sinal: 0 dB");
        }
    }
    
    private void updateFrequencyDisplay() {
        frequencyInput.setText(String.format("%.1f", currentFrequency));
        frequencySlider.setValue((float) currentFrequency);
    }
    
    private void updateGainDisplay() {
        gainSlider.setValue(currentGain);
        gainSlider.setEnabled(!currentAutoGain);
    }
    
    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            
            if (!allGranted) {
                showToast("Permissões necessárias não concedidas");
            }
        }
    }
} 