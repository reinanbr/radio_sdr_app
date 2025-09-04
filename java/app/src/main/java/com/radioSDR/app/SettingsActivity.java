package com.radioSDR.app;

import android.os.Bundle;
import android.view.MenuItem;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

public class SettingsActivity extends AppCompatActivity {
    
    private Spinner spinnerSampleRate;
    private SeekBar seekBarFreqCorrection;
    private TextView tvFreqCorrectionValue;
    private Switch switchAGC;
    private SeekBar seekBarBandwidth;
    private TextView tvBandwidthValue;
    private SeekBar seekBarSquelch;
    private TextView tvSquelchValue;
    
    // Native methods
    public native boolean setSampleRate(int rate);
    public native boolean setFrequencyCorrection(int ppm);
    public native boolean setBandwidth(int bandwidth);
    public native boolean setSquelch(int squelch);
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
        
        // Setup action bar
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle("Configurações");
        }
        
        initViews();
        setupListeners();
        loadSettings();
    }
    
    private void initViews() {
        spinnerSampleRate = findViewById(R.id.spinnerSampleRate);
        seekBarFreqCorrection = findViewById(R.id.seekBarFreqCorrection);
        tvFreqCorrectionValue = findViewById(R.id.tvFreqCorrectionValue);
        switchAGC = findViewById(R.id.switchAGC);
        seekBarBandwidth = findViewById(R.id.seekBarBandwidth);
        tvBandwidthValue = findViewById(R.id.tvBandwidthValue);
        seekBarSquelch = findViewById(R.id.seekBarSquelch);
        tvSquelchValue = findViewById(R.id.tvSquelchValue);
    }
    
    private void setupListeners() {
        seekBarFreqCorrection.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    int ppm = progress - 200; // Range: -200 to +200 PPM
                    tvFreqCorrectionValue.setText(String.format("%d PPM", ppm));
                    setFrequencyCorrection(ppm);
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        
        seekBarBandwidth.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    int bandwidth = 1000 + progress * 100; // Range: 1kHz to 250kHz
                    tvBandwidthValue.setText(String.format("%d Hz", bandwidth));
                    setBandwidth(bandwidth);
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        
        seekBarSquelch.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    int squelch = progress - 100; // Range: -100 to 0 dB
                    tvSquelchValue.setText(String.format("%d dB", squelch));
                    setSquelch(squelch);
                }
            }
            
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
    }
    
    private void loadSettings() {
        // Load default values
        seekBarFreqCorrection.setProgress(200); // 0 PPM
        tvFreqCorrectionValue.setText("0 PPM");
        
        seekBarBandwidth.setProgress(200); // 21kHz
        tvBandwidthValue.setText("21000 Hz");
        
        seekBarSquelch.setProgress(80); // -20 dB
        tvSquelchValue.setText("-20 dB");
        
        switchAGC.setChecked(true);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
