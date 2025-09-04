package com.radioSDR.app;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.MenuItem;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Description;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;

import java.util.ArrayList;
import java.util.List;

public class SpectrumActivity extends AppCompatActivity {
    
    private LineChart chartSpectrum;
    private LineChart chartWaterfall;
    private Handler updateHandler;
    private Runnable updateRunnable;
    
    // Native methods (same as MainActivity)
    public native float[] getSpectrumData();
    public native float[] getWaterfallData();
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_spectrum);
        
        // Setup toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
            getSupportActionBar().setTitle("Visualizador de Espectro");
        }
        
        initViews();
        setupCharts();
        startUpdates();
    }
    
    private void initViews() {
        chartSpectrum = findViewById(R.id.chartSpectrum);
        chartWaterfall = findViewById(R.id.chartWaterfall);
    }
    
    private void setupCharts() {
        setupSpectrumChart();
        setupWaterfallChart();
    }
    
    private void setupSpectrumChart() {
        chartSpectrum.setDrawGridBackground(false);
        chartSpectrum.setDrawBorders(true);
        chartSpectrum.setTouchEnabled(true);
        chartSpectrum.setDragEnabled(true);
        chartSpectrum.setScaleEnabled(true);
        chartSpectrum.setPinchZoom(true);
        
        Description desc = new Description();
        desc.setText("Espectro de Frequência");
        chartSpectrum.setDescription(desc);
        
        XAxis xAxis = chartSpectrum.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setGranularity(1f);
        xAxis.setLabelCount(10);
        
        YAxis leftAxis = chartSpectrum.getAxisLeft();
        leftAxis.setDrawGridLines(true);
        leftAxis.setAxisMinimum(-120f);
        leftAxis.setAxisMaximum(0f);
        
        YAxis rightAxis = chartSpectrum.getAxisRight();
        rightAxis.setEnabled(false);
    }
    
    private void setupWaterfallChart() {
        chartWaterfall.setDrawGridBackground(false);
        chartWaterfall.setDrawBorders(true);
        chartWaterfall.setTouchEnabled(true);
        chartWaterfall.setDragEnabled(true);
        chartWaterfall.setScaleEnabled(true);
        chartWaterfall.setPinchZoom(true);
        
        Description desc = new Description();
        desc.setText("Cascata (Waterfall)");
        chartWaterfall.setDescription(desc);
        
        XAxis xAxis = chartWaterfall.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setGranularity(1f);
        
        YAxis leftAxis = chartWaterfall.getAxisLeft();
        leftAxis.setDrawGridLines(true);
        
        YAxis rightAxis = chartWaterfall.getAxisRight();
        rightAxis.setEnabled(false);
    }
    
    private void startUpdates() {
        updateHandler = new Handler(Looper.getMainLooper());
        updateRunnable = new Runnable() {
            @Override
            public void run() {
                updateSpectrum();
                updateWaterfall();
                updateHandler.postDelayed(this, 50); // Update every 50ms for smooth display
            }
        };
        updateHandler.post(updateRunnable);
    }
    
    private void updateSpectrum() {
        float[] spectrumData = getSpectrumData();
        if (spectrumData != null && spectrumData.length > 0) {
            List<Entry> entries = new ArrayList<>();
            for (int i = 0; i < spectrumData.length; i++) {
                entries.add(new Entry(i, spectrumData[i]));
            }
            
            LineDataSet dataSet = new LineDataSet(entries, "FFT");
            dataSet.setDrawCircles(false);
            dataSet.setDrawValues(false);
            dataSet.setColor(getColor(R.color.blue));
            dataSet.setLineWidth(2f);
            dataSet.setMode(LineDataSet.Mode.LINEAR);
            
            LineData lineData = new LineData(dataSet);
            chartSpectrum.setData(lineData);
            chartSpectrum.invalidate();
        }
    }
    
    private void updateWaterfall() {
        float[] waterfallData = getWaterfallData();
        if (waterfallData != null && waterfallData.length > 0) {
            List<Entry> entries = new ArrayList<>();
            for (int i = 0; i < waterfallData.length; i++) {
                entries.add(new Entry(i, waterfallData[i]));
            }
            
            LineDataSet dataSet = new LineDataSet(entries, "Waterfall");
            dataSet.setDrawCircles(false);
            dataSet.setDrawValues(false);
            dataSet.setColor(getColor(R.color.green));
            dataSet.setLineWidth(1f);
            
            LineData lineData = new LineData(dataSet);
            chartWaterfall.setData(lineData);
            chartWaterfall.invalidate();
        }
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (updateHandler != null && updateRunnable != null) {
            updateHandler.removeCallbacks(updateRunnable);
        }
    }
}
