package com.sdrradio.kt

import android.Manifest
import android.app.AlertDialog
import android.content.pm.PackageManager
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.karumi.dexter.Dexter
import com.karumi.dexter.MultiplePermissionsReport
import com.karumi.dexter.PermissionToken
import com.karumi.dexter.listener.PermissionRequest
import com.karumi.dexter.listener.multi.MultiplePermissionsListener
import com.sdrradio.kt.databinding.ActivityMainBinding
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : AppCompatActivity(), SDRRadio.SDRCallback {
    
    private lateinit var binding: ActivityMainBinding
    private lateinit var sdrRadio: SDRRadio
    private lateinit var usbManager: UsbManager
    
    private var currentFrequency = 100.0 // Frequência inicial em MHz
    private var currentSignalStrength = -100.0
    private var isAudioPlaying = false
    private var currentModulation = "AM" // AM, FM, Auto
    
    companion object {
        private const val TAG = "MainActivity"
        private const val PERMISSION_REQUEST_CODE = 100
        
        // Constantes para frequências do RTL-SDR
        private const val MIN_FREQUENCY = 24.0 // MHz
        private const val MAX_FREQUENCY = 1766.0 // MHz
        private const val FREQUENCY_STEP = 1.0 // MHz
    }
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        usbManager = getSystemService(USB_SERVICE) as UsbManager
        sdrRadio = SDRRadio()
        sdrRadio.setCallback(this)
        
        setupUI()
        requestPermissions()
        setupChart()
    }
    
    private fun setupUI() {
        binding.connectButton.setOnClickListener {
            if (sdrRadio.isConnected()) {
                disconnectDevice()
            } else {
                connectDevice()
            }
        }
        
        binding.scanButton.setOnClickListener {
            if (sdrRadio.isScanning()) {
                stopScanning()
            } else {
                startScanning()
            }
        }
        
        binding.recordButton.setOnClickListener {
            if (sdrRadio.isRecording()) {
                stopRecording()
            } else {
                startRecording()
            }
        }
        
        // Controles de frequência
        binding.setFrequencyButton.setOnClickListener {
            setFrequencyFromInput()
        }
        
        binding.stepDownButton.setOnClickListener {
            stepFrequency(-FREQUENCY_STEP)
        }
        
        binding.stepUpButton.setOnClickListener {
            stepFrequency(FREQUENCY_STEP)
        }
        
        binding.frequencySlider.addOnChangeListener { _, value, fromUser ->
            if (fromUser) {
                currentFrequency = value.toDouble()
                updateFrequencyDisplay()
                setFrequencyOnDevice()
            }
        }
        
        binding.frequencyInput.setOnEditorActionListener { _, actionId, _ ->
            if (actionId == android.view.inputmethod.EditorInfo.IME_ACTION_DONE) {
                setFrequencyFromInput()
                true
            } else {
                false
            }
        }
        
        // Controles de áudio
        binding.playAudioButton.setOnClickListener {
            if (isAudioPlaying) {
                stopAudio()
            } else {
                startAudio()
            }
        }
        
        binding.stopAudioButton.setOnClickListener {
            stopAudio()
        }
        
        binding.volumeButton.setOnClickListener {
            showVolumeDialog()
        }
        
        // Controles de modulação
        binding.amButton.setOnClickListener {
            setModulation("AM")
        }
        
        binding.fmButton.setOnClickListener {
            setModulation("FM")
        }
        
        binding.autoButton.setOnClickListener {
            setModulation("Auto")
        }
    }
    
    private fun setupChart() {
        binding.frequencyChart.apply {
            description.isEnabled = false
            setTouchEnabled(true)
            isDragEnabled = true
            setScaleEnabled(true)
            setPinchZoom(true)
            
            xAxis.apply {
                setDrawGridLines(false)
                setDrawAxisLine(true)
            }
            
            axisLeft.apply {
                setDrawGridLines(true)
                setDrawAxisLine(true)
            }
            
            axisRight.isEnabled = false
            legend.isEnabled = false
        }
        
        // Configurar o slider de frequência
        binding.frequencySlider.apply {
            valueFrom = MIN_FREQUENCY.toFloat()
            valueTo = MAX_FREQUENCY.toFloat()
            stepSize = 0.1f
            value = currentFrequency.toFloat()
        }
    }
    
    private fun requestPermissions() {
        Dexter.withContext(this)
            .withPermissions(
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.RECORD_AUDIO,
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.ACCESS_COARSE_LOCATION
            )
            .withListener(object : MultiplePermissionsListener {
                override fun onPermissionsChecked(report: MultiplePermissionsReport?) {
                    if (report?.areAllPermissionsGranted() == true) {
                        Log.i(TAG, "All permissions granted")
                    } else {
                        Log.w(TAG, "Some permissions denied")
                        Toast.makeText(this@MainActivity, "Some permissions are required for full functionality", Toast.LENGTH_LONG).show()
                    }
                }
                
                override fun onPermissionRationaleShouldBeShown(
                    permissions: MutableList<PermissionRequest>?,
                    token: PermissionToken?
                ) {
                    token?.continuePermissionRequest()
                }
            }).check()
    }
    
    private fun connectDevice() {
        if (checkUsbPermission()) {
            binding.connectButton.isEnabled = false
            binding.statusText.text = "Connecting..."
            
            val success = sdrRadio.connect()
            if (success) {
                binding.connectButton.text = getString(R.string.disconnect_usb)
                binding.scanButton.isEnabled = true
                binding.recordButton.isEnabled = true
                
                // Habilitar controles de frequência
                binding.setFrequencyButton.isEnabled = true
                binding.stepDownButton.isEnabled = true
                binding.stepUpButton.isEnabled = true
                binding.frequencySlider.isEnabled = true
                binding.frequencyInput.isEnabled = true
                
                // Habilitar controles de áudio
                binding.playAudioButton.isEnabled = true
                binding.stopAudioButton.isEnabled = true
                binding.volumeButton.isEnabled = true
                binding.amButton.isEnabled = true
                binding.fmButton.isEnabled = true
                binding.autoButton.isEnabled = true
                
                // Definir frequência inicial
                setFrequencyOnDevice()
            } else {
                binding.statusText.text = "Connection failed"
                Toast.makeText(this, "Failed to connect RTL-SDR", Toast.LENGTH_SHORT).show()
            }
            binding.connectButton.isEnabled = true
        } else {
            Toast.makeText(this, "USB permission required", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun disconnectDevice() {
        sdrRadio.disconnect()
        binding.connectButton.text = getString(R.string.connect_usb)
        binding.scanButton.isEnabled = false
        binding.recordButton.isEnabled = false
        binding.scanButton.text = getString(R.string.start_scanning)
        binding.recordButton.text = getString(R.string.record_audio)
        
        // Desabilitar controles de frequência
        binding.setFrequencyButton.isEnabled = false
        binding.stepDownButton.isEnabled = false
        binding.stepUpButton.isEnabled = false
        binding.frequencySlider.isEnabled = false
        binding.frequencyInput.isEnabled = false
        
        // Desabilitar controles de áudio
        binding.playAudioButton.isEnabled = false
        binding.stopAudioButton.isEnabled = false
        binding.volumeButton.isEnabled = false
        binding.amButton.isEnabled = false
        binding.fmButton.isEnabled = false
        binding.autoButton.isEnabled = false
        
        // Parar áudio se estiver tocando
        if (isAudioPlaying) {
            stopAudio()
        }
    }
    
    private fun startScanning() {
        val success = sdrRadio.startScanning()
        if (success) {
            binding.scanButton.text = getString(R.string.stop_scanning)
        } else {
            Toast.makeText(this, "Failed to start scanning", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun stopScanning() {
        sdrRadio.stopScanning()
        binding.scanButton.text = getString(R.string.start_scanning)
    }
    
    private fun startRecording() {
        val success = sdrRadio.startRecording()
        if (success) {
            binding.recordButton.text = getString(R.string.stop_recording)
        } else {
            Toast.makeText(this, "Failed to start recording", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun stopRecording() {
        sdrRadio.stopRecording()
        binding.recordButton.text = getString(R.string.record_audio)
        Toast.makeText(this, "Recording saved to /sdcard/sdr_recordings/", Toast.LENGTH_LONG).show()
    }
    
    private fun checkUsbPermission(): Boolean {
        val deviceList = usbManager.deviceList
        for (device in deviceList.values) {
            if (isRTLDevice(device)) {
                if (!usbManager.hasPermission(device)) {
                    val permissionIntent = android.app.PendingIntent.getBroadcast(
                        this, 0, android.content.Intent("com.sdrradio.kt.USB_PERMISSION"), 0
                    )
                    usbManager.requestPermission(device, permissionIntent)
                    return false
                }
                return true
            }
        }
        return false
    }
    
    private fun isRTLDevice(device: UsbDevice): Boolean {
        return device.vendorId == 0x0bda
    }
    
    private fun updateFrequencyDisplay() {
        val freqText = String.format("%.3f MHz", currentFrequency)
        binding.frequencyText.text = freqText
        
        val signalText = String.format("Sinal: %.1f dB", currentSignalStrength)
        binding.signalStrengthText.text = signalText
        
        // Atualizar slider e input
        binding.frequencySlider.value = currentFrequency.toFloat()
        binding.frequencyInput.setText(String.format("%.1f", currentFrequency))
    }
    
    private fun setFrequencyFromInput() {
        val inputText = binding.frequencyInput.text.toString()
        if (inputText.isNotEmpty()) {
            try {
                val newFreq = inputText.toDouble()
                if (newFreq in MIN_FREQUENCY..MAX_FREQUENCY) {
                    currentFrequency = newFreq
                    updateFrequencyDisplay()
                    setFrequencyOnDevice()
                } else {
                    Toast.makeText(this, "Frequência deve estar entre $MIN_FREQUENCY e $MAX_FREQUENCY MHz", Toast.LENGTH_SHORT).show()
                }
            } catch (e: NumberFormatException) {
                Toast.makeText(this, "Frequência inválida", Toast.LENGTH_SHORT).show()
            }
        }
    }
    
    private fun stepFrequency(step: Double) {
        val newFreq = currentFrequency + step
        if (newFreq in MIN_FREQUENCY..MAX_FREQUENCY) {
            currentFrequency = newFreq
            updateFrequencyDisplay()
            setFrequencyOnDevice()
        }
    }
    
    private fun setFrequencyOnDevice() {
        if (sdrRadio.isConnected()) {
            val freqHz = (currentFrequency * 1000000).toLong()
            val success = sdrRadio.setFrequency(freqHz)
            if (success) {
                Log.i(TAG, "Frequência definida para ${currentFrequency} MHz")
            } else {
                Log.e(TAG, "Erro ao definir frequência")
                Toast.makeText(this, "Erro ao definir frequência", Toast.LENGTH_SHORT).show()
            }
        }
    }
    
    private fun startAudio() {
        if (sdrRadio.isConnected()) {
            isAudioPlaying = true
            binding.playAudioButton.text = getString(R.string.stop_audio)
            binding.statusText.text = getString(R.string.playing_audio)
            
            // Iniciar reprodução de áudio
            val success = sdrRadio.startAudioPlayback()
            if (!success) {
                Toast.makeText(this, getString(R.string.error_audio), Toast.LENGTH_SHORT).show()
                stopAudio()
            }
        } else {
            Toast.makeText(this, getString(R.string.usb_not_connected), Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun stopAudio() {
        isAudioPlaying = false
        binding.playAudioButton.text = getString(R.string.play_audio)
        binding.statusText.text = getString(R.string.status_connected)
        
        sdrRadio.stopAudioPlayback()
    }
    
    private fun showVolumeDialog() {
        val volumeLevels = arrayOf("Baixo", "Médio", "Alto", "Máximo")
        AlertDialog.Builder(this)
            .setTitle("Controle de Volume")
            .setItems(volumeLevels) { _, which ->
                val volume = when (which) {
                    0 -> 0.25f
                    1 -> 0.5f
                    2 -> 0.75f
                    3 -> 1.0f
                    else -> 0.5f
                }
                sdrRadio.setAudioVolume(volume)
                Toast.makeText(this, "Volume definido para ${volumeLevels[which]}", Toast.LENGTH_SHORT).show()
            }
            .setNegativeButton("Cancelar", null)
            .show()
    }
    
    private fun setModulation(modulation: String) {
        currentModulation = modulation
        
        // Atualizar aparência dos botões
        binding.amButton.setBackgroundColor(if (modulation == "AM") getColor(R.color.blue) else getColor(R.color.gray))
        binding.fmButton.setBackgroundColor(if (modulation == "FM") getColor(R.color.blue) else getColor(R.color.gray))
        binding.autoButton.setBackgroundColor(if (modulation == "Auto") getColor(R.color.blue) else getColor(R.color.gray))
        
        // Aplicar modulação no dispositivo
        if (sdrRadio.isConnected()) {
            val success = sdrRadio.setModulation(modulation)
            if (success) {
                Log.i(TAG, "Modulação definida para $modulation")
                Toast.makeText(this, "Modulação: $modulation", Toast.LENGTH_SHORT).show()
            } else {
                Log.e(TAG, "Erro ao definir modulação")
                Toast.makeText(this, "Erro ao definir modulação", Toast.LENGTH_SHORT).show()
            }
        }
    }
    
    private fun updateChart() {
        val spectrum = sdrRadio.getFrequencySpectrum()
        if (spectrum != null) {
            val entries = mutableListOf<Entry>()
            for (i in spectrum.indices) {
                entries.add(Entry(i.toFloat(), spectrum[i]))
            }
            
            val dataSet = LineDataSet(entries, "Spectrum").apply {
                color = ContextCompat.getColor(this@MainActivity, R.color.blue)
                setDrawCircles(false)
                setDrawValues(false)
                lineWidth = 2f
            }
            
            val lineData = LineData(dataSet)
            binding.frequencyChart.data = lineData
            binding.frequencyChart.invalidate()
        }
    }
    
    override fun onStatusChanged(status: String) {
        runOnUiThread {
            binding.statusText.text = status
            when (status) {
                "Connected" -> binding.statusText.setTextColor(ContextCompat.getColor(this, R.color.green))
                "Disconnected" -> binding.statusText.setTextColor(ContextCompat.getColor(this, R.color.red))
                else -> binding.statusText.setTextColor(ContextCompat.getColor(this, R.color.orange))
            }
        }
    }
    
    override fun onScanningStarted() {
        runOnUiThread {
            Toast.makeText(this, "Started frequency scanning", Toast.LENGTH_SHORT).show()
        }
    }
    
    override fun onScanningStopped() {
        runOnUiThread {
            Toast.makeText(this, "Stopped frequency scanning", Toast.LENGTH_SHORT).show()
        }
    }
    
    override fun onRecordingStarted() {
        runOnUiThread {
            Toast.makeText(this, "Started audio recording", Toast.LENGTH_SHORT).show()
        }
    }
    
    override fun onRecordingStopped() {
        runOnUiThread {
            Toast.makeText(this, "Stopped audio recording", Toast.LENGTH_SHORT).show()
        }
    }
    
    override fun onFrequencyUpdate(frequency: Double, signalStrength: Double) {
        currentFrequency = frequency
        currentSignalStrength = signalStrength
        
        runOnUiThread {
            updateFrequencyDisplay()
            updateChart()
        }
    }
    
    override fun onError(error: String) {
        runOnUiThread {
            Toast.makeText(this, "Error: $error", Toast.LENGTH_SHORT).show()
            Log.e(TAG, "SDR Error: $error")
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        sdrRadio.destroy()
    }
} 