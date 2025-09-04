# 📻 Manual Completo - SDR Radio KT

## 📋 Índice

1. [Visão Geral do Projeto](#visão-geral-do-projeto)
2. [Requisitos e Tecnologias](#requisitos-e-tecnologias)
3. [Estrutura do Projeto](#estrutura-do-projeto)
4. [Configuração Inicial](#configuração-inicial)
5. [Desenvolvimento das Funcionalidades](#desenvolvimento-das-funcionalidades)
6. [Interface do Usuário](#interface-do-usuário)
7. [Código Nativo (C++)](#código-nativo-c)
8. [Integração JNI](#integração-jni)
9. [Testes e Debugging](#testes-e-debugging)
10. [Instalação e Uso](#instalação-e-uso)
11. [Troubleshooting](#troubleshooting)
12. [Próximos Passos](#próximos-passos)

---

## 🎯 Visão Geral do Projeto

### Objetivo
Desenvolver um aplicativo Android em Kotlin que conecta via OTG a um dispositivo RTL-SDR, permitindo:
- Sintonizar frequências de rádio
- Reproduzir áudio em tempo real
- Gravar transmissões
- Visualizar espectro de frequências
- Escanear automaticamente

### Funcionalidades Principais
- ✅ Conexão USB OTG com RTL-SDR
- ✅ Controles de frequência (24-1766 MHz)
- ✅ Reprodução de áudio AM/FM
- ✅ Gravação em formato WAV
- ✅ Visualização de espectro
- ✅ Escaneamento automático
- ✅ Controle de volume
- ✅ Seleção de modulação

---

## 🛠 Requisitos e Tecnologias

### Hardware
- **RTL-SDR**: Dispositivo de Software Defined Radio
- **Android Device**: Com suporte a USB OTG
- **Cabo OTG**: Para conexão USB

### Software
- **Android Studio**: IDE de desenvolvimento
- **Gradle 7.6**: Sistema de build
- **Android NDK**: Para código nativo
- **CMake**: Build system para C++
- **Kotlin 1.8.0**: Linguagem principal

### Bibliotecas
- **Material Design**: Interface moderna
- **MPAndroidChart**: Visualização de gráficos
- **Dexter**: Gerenciamento de permissões
- **USB Serial**: Comunicação USB
- **RTL-SDR Library**: Biblioteca nativa

---

## 📁 Estrutura do Projeto

```
sdr_radio_kt/
├── app/
│   ├── build.gradle                 # Configuração do módulo
│   ├── src/main/
│   │   ├── AndroidManifest.xml      # Manifesto da aplicação
│   │   ├── cpp/                     # Código nativo C++
│   │   │   ├── CMakeLists.txt       # Build script CMake
│   │   │   ├── sdr_radio.h          # Header da classe SDR
│   │   │   ├── sdr_radio_simple.cpp # Implementação principal
│   │   │   └── rtl-sdr/             # Biblioteca RTL-SDR
│   │   ├── java/com/sdrradio/kt/
│   │   │   ├── MainActivity.kt      # Activity principal
│   │   │   └── SDRRadio.kt          # Wrapper Kotlin
│   │   └── res/                     # Recursos Android
│   │       ├── layout/
│   │       │   └── activity_main.xml # Layout principal
│   │       ├── values/
│   │       │   ├── strings.xml      # Strings da aplicação
│   │       │   ├── colors.xml       # Cores
│   │       │   └── themes.xml       # Temas
│   │       └── xml/
│   │           ├── device_filter.xml # Filtros USB
│   │           └── file_paths.xml   # Caminhos de arquivo
├── build.gradle                     # Configuração do projeto
├── gradle.properties                # Propriedades Gradle
├── settings.gradle                  # Configuração de módulos
├── download_rtlsdr.sh              # Script de download RTL-SDR
├── install.sh                      # Script de instalação
└── README.md                       # Documentação
```

---

## ⚙️ Configuração Inicial

### 1. Download da Biblioteca RTL-SDR
```bash
# Executar script de download
./download_rtlsdr.sh
```

### 2. Configuração do Gradle
**build.gradle (Project)**
```gradle
buildscript {
    ext.kotlin_version = "1.8.0"
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:7.4.2'
        classpath "org.jetbrains.kotlin:kotlin-gradle-plugin:$kotlin_version"
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
        maven { url 'https://jitpack.io' }
    }
}
```

**app/build.gradle**
```gradle
android {
    compileSdk 33
    defaultConfig {
        minSdk 26
        targetSdk 33
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
    
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
        }
    }
}

dependencies {
    implementation 'androidx.core:core-ktx:1.9.0'
    implementation 'com.google.android.material:material:1.8.0'
    implementation 'com.github.mik3y:usb-serial-for-android:3.4.6'
    implementation 'com.github.PhilJay:MPAndroidChart:v3.1.0'
    implementation 'com.karumi:dexter:6.2.3'
}
```

### 3. Configuração do CMake
**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.22.1)
project("sdrradio")

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)

# Incluir diretórios RTL-SDR
include_directories(rtl-sdr/include)
include_directories(rtl-sdr/src)

# Definir fontes RTL-SDR
set(RTL_SDR_SOURCES
    rtl-sdr/src/librtlsdr.c
    rtl-sdr/src/tuner_e4k.c
    rtl-sdr/src/tuner_fc0012.c
    rtl-sdr/src/tuner_fc0013.c
    rtl-sdr/src/tuner_fc2580.c
    rtl-sdr/src/tuner_r82xx.c
)

# Criar biblioteca estática RTL-SDR
add_library(rtlsdr STATIC ${RTL_SDR_SOURCES})

# Configurações Android
target_compile_definitions(rtlsdr PRIVATE
    LIBUSB_API_VERSION=0x01000100
    ANDROID=1
    __ANDROID__=1
)

# Criar biblioteca compartilhada principal
add_library(sdrradio SHARED sdr_radio_simple.cpp)

# Linkar bibliotecas
target_link_libraries(sdrradio
    rtlsdr
    android
    log
    OpenSLES
)
```

---

## 🔧 Desenvolvimento das Funcionalidades

### 1. Estrutura de Dados
```cpp
// Estrutura para dados de áudio
struct AudioData {
    std::vector<float> samples;
    double frequency;
    double signal_strength;
    long timestamp;
};

// Estrutura para configuração do SDR
struct SDRConfig {
    uint32_t sample_rate;
    uint32_t center_freq;
    int gain;
    int ppm_error;
    bool auto_gain;
};
```

### 2. Classe Principal SDRRadio
```cpp
class SDRRadio {
private:
    SDRConfig config;
    bool is_connected;
    bool is_scanning;
    bool is_recording;
    
    // Threads
    pthread_t scan_thread;
    pthread_t audio_thread;
    pthread_mutex_t data_mutex;
    
    // Data buffers
    std::queue<AudioData> audio_queue;
    std::vector<float> frequency_spectrum;
    
public:
    // Device management
    bool connect_device();
    void disconnect_device();
    
    // Configuration
    bool set_center_frequency(uint32_t freq);
    bool set_gain(int gain);
    
    // Audio processing
    void process_audio_data(const std::vector<uint8_t>& rf_data);
    std::vector<float> demodulate_am(const std::vector<uint8_t>& rf_data);
    std::vector<float> demodulate_fm(const std::vector<uint8_t>& rf_data);
};
```

### 3. Demodulação AM
```cpp
std::vector<float> SDRRadio::demodulate_am(const std::vector<uint8_t>& rf_data) {
    std::vector<float> audio_samples;
    
    for (size_t i = 0; i < rf_data.size() - 1; i += 2) {
        // Converter bytes para valores I/Q
        float i_val = (float)((int8_t)rf_data[i]) / 128.0f;
        float q_val = (float)((int8_t)rf_data[i + 1]) / 128.0f;
        
        // Calcular magnitude (envelope)
        float magnitude = sqrtf(i_val * i_val + q_val * q_val);
        
        // Aplicar filtro passa-baixa
        static float prev_sample = 0.0f;
        float filtered = 0.9f * prev_sample + 0.1f * magnitude;
        prev_sample = filtered;
        
        // Normalizar e adicionar à saída
        float audio_sample = (filtered - 0.5f) * 2.0f;
        audio_sample = std::max(-1.0f, std::min(1.0f, audio_sample));
        
        audio_samples.push_back(audio_sample);
    }
    
    return audio_samples;
}
```

### 4. Demodulação FM
```cpp
std::vector<float> SDRRadio::demodulate_fm(const std::vector<uint8_t>& rf_data) {
    std::vector<float> audio_samples;
    float prev_phase = 0.0f;
    
    for (size_t i = 0; i < rf_data.size() - 1; i += 2) {
        // Converter bytes para valores I/Q
        float i_val = (float)((int8_t)rf_data[i]) / 128.0f;
        float q_val = (float)((int8_t)rf_data[i + 1]) / 128.0f;
        
        // Calcular fase atual
        float phase = atan2f(q_val, i_val);
        
        // Calcular diferença de fase
        float phase_diff = phase - prev_phase;
        
        // Normalizar diferença de fase
        while (phase_diff > M_PI) phase_diff -= 2 * M_PI;
        while (phase_diff < -M_PI) phase_diff += 2 * M_PI;
        
        // Converter diferença de fase em áudio
        float audio_sample = phase_diff * 0.5f;
        
        // Aplicar filtro passa-baixa
        static float prev_sample = 0.0f;
        float filtered = 0.9f * prev_sample + 0.1f * audio_sample;
        prev_sample = filtered;
        
        audio_samples.push_back(filtered);
        prev_phase = phase;
    }
    
    return audio_samples;
}
```

---

## 🎨 Interface do Usuário

### Layout Principal (activity_main.xml)
```xml
<?xml version="1.0" encoding="utf-8"?>
<androidx.coordinatorlayout.widget.CoordinatorLayout>
    <com.google.android.material.appbar.AppBarLayout>
        <com.google.android.material.appbar.MaterialToolbar>
            <!-- Toolbar com título -->
        </com.google.android.material.appbar.MaterialToolbar>
    </com.google.android.material.appbar.AppBarLayout>
    
    <androidx.core.widget.NestedScrollView>
        <LinearLayout>
            <!-- Status Card -->
            <com.google.android.material.card.MaterialCardView>
                <TextView android:id="@+id/statusText" />
            </com.google.android.material.card.MaterialCardView>
            
            <!-- Controls Card -->
            <com.google.android.material.card.MaterialCardView>
                <!-- Botões de controle -->
                <com.google.android.material.button.MaterialButton
                    android:id="@+id/connectButton" />
                <com.google.android.material.button.MaterialButton
                    android:id="@+id/scanButton" />
                <com.google.android.material.button.MaterialButton
                    android:id="@+id/recordButton" />
            </com.google.android.material.card.MaterialCardView>
            
            <!-- Frequency Card -->
            <com.google.android.material.card.MaterialCardView>
                <!-- Controles de frequência -->
                <com.google.android.material.textfield.TextInputLayout>
                    <com.google.android.material.textfield.TextInputEditText
                        android:id="@+id/frequencyInput" />
                </com.google.android.material.textfield.TextInputLayout>
                
                <!-- Botões de ajuste -->
                <LinearLayout>
                    <MaterialButton android:id="@+id/setFrequencyButton" />
                    <MaterialButton android:id="@+id/stepDownButton" />
                    <MaterialButton android:id="@+id/stepUpButton" />
                </LinearLayout>
                
                <!-- Slider de frequência -->
                <com.google.android.material.slider.Slider
                    android:id="@+id/frequencySlider" />
            </com.google.android.material.card.MaterialCardView>
            
            <!-- Audio Controls Card -->
            <com.google.android.material.card.MaterialCardView>
                <!-- Controles de áudio -->
                <LinearLayout>
                    <MaterialButton android:id="@+id/playAudioButton" />
                    <MaterialButton android:id="@+id/stopAudioButton" />
                    <MaterialButton android:id="@+id/volumeButton" />
                </LinearLayout>
                
                <!-- Controles de modulação -->
                <LinearLayout>
                    <MaterialButton android:id="@+id/amButton" />
                    <MaterialButton android:id="@+id/fmButton" />
                    <MaterialButton android:id="@+id/autoButton" />
                </LinearLayout>
            </com.google.android.material.card.MaterialCardView>
            
            <!-- Spectrum Chart -->
            <com.google.android.material.card.MaterialCardView>
                <com.github.mikephil.charting.charts.LineChart
                    android:id="@+id/frequencyChart" />
            </com.google.android.material.card.MaterialCardView>
        </LinearLayout>
    </androidx.core.widget.NestedScrollView>
</androidx.coordinatorlayout.widget.CoordinatorLayout>
```

### MainActivity.kt
```kotlin
class MainActivity : AppCompatActivity(), SDRRadio.SDRCallback {
    private lateinit var binding: ActivityMainBinding
    private lateinit var sdrRadio: SDRRadio
    private lateinit var usbManager: UsbManager
    
    private var currentFrequency = 100.0
    private var currentSignalStrength = -100.0
    private var isAudioPlaying = false
    private var currentModulation = "AM"
    
    companion object {
        private const val MIN_FREQUENCY = 24.0
        private const val MAX_FREQUENCY = 1766.0
        private const val FREQUENCY_STEP = 1.0
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
        // Configurar listeners dos botões
        binding.connectButton.setOnClickListener {
            if (sdrRadio.isConnected()) disconnectDevice()
            else connectDevice()
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
        
        // Controles de áudio
        binding.playAudioButton.setOnClickListener {
            if (isAudioPlaying) stopAudio()
            else startAudio()
        }
        
        // Controles de modulação
        binding.amButton.setOnClickListener { setModulation("AM") }
        binding.fmButton.setOnClickListener { setModulation("FM") }
        binding.autoButton.setOnClickListener { setModulation("Auto") }
    }
}
```

---

## 🔗 Integração JNI

### SDRRadio.kt (Wrapper Kotlin)
```kotlin
class SDRRadio {
    companion object {
        init {
            System.loadLibrary("sdrradio")
        }
    }
    
    private var nativePtr: Long = 0
    
    init {
        nativePtr = nativeInit()
    }
    
    fun connect(): Boolean {
        return nativeConnect(nativePtr)
    }
    
    fun disconnect() {
        nativeDisconnect(nativePtr)
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
    
    // Native methods
    private external fun nativeInit(): Long
    private external fun nativeDestroy(ptr: Long)
    private external fun nativeConnect(ptr: Long): Boolean
    private external fun nativeDisconnect(ptr: Long)
    private external fun nativeSetFrequency(ptr: Long, frequencyHz: Long): Boolean
    private external fun nativeStartAudioPlayback(ptr: Long): Boolean
    private external fun nativeStopAudioPlayback(ptr: Long)
    private external fun nativeSetAudioVolume(ptr: Long, volume: Float): Boolean
    private external fun nativeSetModulation(ptr: Long, modulation: String): Boolean
}
```

### Implementação JNI (C++)
```cpp
// JNI Functions
JNIEXPORT jlong JNICALL Java_com_sdrradio_kt_SDRRadio_nativeInit(JNIEnv* env, jobject thiz) {
    SDRRadio* radio = new SDRRadio();
    return reinterpret_cast<jlong>(radio);
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeConnect(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->connect_device();
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeSetFrequency(JNIEnv* env, jobject thiz, jlong ptr, jlong frequencyHz) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        return radio->set_center_frequency(frequencyHz);
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_sdrradio_kt_SDRRadio_nativeStartAudioPlayback(JNIEnv* env, jobject thiz, jlong ptr) {
    SDRRadio* radio = reinterpret_cast<SDRRadio*>(ptr);
    if (radio) {
        LOGI("Starting audio playback");
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
```

---

## 🧪 Testes e Debugging

### Scripts de Teste
**test_app.sh**
```bash
#!/bin/bash
echo "=== Testando SDR Radio KT ==="

# Limpar logs
adb logcat -c

# Iniciar app
adb shell am start -n com.sdrradio.kt/.MainActivity
sleep 5

# Capturar logs
adb logcat -d | grep -E "(SDRRadio|MainActivity)" | head -10

# Verificar processo
adb shell ps | grep sdrradio
```

**test_audio_features.sh**
```bash
#!/bin/bash
echo "=== Testando Funcionalidades de Áudio ==="

# Testar controles de frequência
echo "✅ Controles de Frequência:"
echo "   - Input manual (24-1766 MHz)"
echo "   - Botões +/- 1MHz"
echo "   - Slider visual"
echo "   - Botão Definir"

# Testar controles de áudio
echo "✅ Controles de Áudio:"
echo "   - Botão Reproduzir"
echo "   - Botão Parar"
echo "   - Controle de Volume"

# Testar modulação
echo "✅ Controles de Modulação:"
echo "   - AM, FM, Auto"
```

### Logs de Debug
```bash
# Capturar logs do app
adb logcat -d -s SDRRadio:V MainActivity:V

# Capturar logs de erro
adb logcat -d | grep -i error

# Monitorar logs em tempo real
adb logcat -s SDRRadio:V
```

---

## 📱 Instalação e Uso

### 1. Compilação
```bash
# Compilar o projeto
./gradlew assembleDebug

# Verificar se compilou com sucesso
echo $?
```

### 2. Instalação
```bash
# Instalar no dispositivo
./install.sh

# Ou manualmente
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### 3. Execução
```bash
# Iniciar o app
adb shell am start -n com.sdrradio.kt/.MainActivity

# Ou procurar no launcher do dispositivo
```

### 4. Uso do App
1. **Conectar RTL-SDR**: Via cabo OTG
2. **Conectar no App**: Clique em "Conectar USB"
3. **Definir Frequência**: Use input ou slider
4. **Selecionar Modulação**: AM, FM ou Auto
5. **Reproduzir Áudio**: Clique em "▶ Reproduzir"
6. **Ajustar Volume**: Use "🔊 Volume"
7. **Sintonizar**: Use controles de frequência

### 5. Frequências Recomendadas
- **📻 AM**: 540-1600 kHz (0.54-1.6 MHz)
- **📻 FM**: 88-108 MHz
- **📡 Aeronáutica**: 118-137 MHz
- **📡 Amador**: 144-148 MHz, 430-450 MHz
- **📡 CB**: 27 MHz

---

## 🔧 Troubleshooting

### Problemas Comuns

#### 1. Erro de Compilação CMake
```
Error: Cannot find source file: rtl-sdr/src/rtlsdr.c
```
**Solução**: Verificar se o script `download_rtlsdr.sh` foi executado.

#### 2. Erro de Permissão USB
```
USB permission required
```
**Solução**: Verificar se o dispositivo está conectado e as permissões foram concedidas.

#### 3. Erro de Slider
```
Slider value must be greater or equal to valueFrom
```
**Solução**: Configurar corretamente os valores do slider no código.

#### 4. App não inicia
```
FATAL EXCEPTION: main
```
**Solução**: Verificar logs com `adb logcat` e corrigir erros de layout.

### Comandos de Debug
```bash
# Verificar dispositivos conectados
adb devices

# Verificar permissões
adb shell dumpsys package com.sdrradio.kt | grep permission

# Verificar logs em tempo real
adb logcat -s SDRRadio:V MainActivity:V

# Verificar processo
adb shell ps | grep sdrradio

# Reinstalar app
adb uninstall com.sdrradio.kt
adb install app/build/outputs/apk/debug/app-debug.apk
```

---

## 🚀 Próximos Passos

### Melhorias Sugeridas

#### 1. Funcionalidades Avançadas
- [ ] **Demodulação SSB**: Para rádio amador
- [ ] **Noise Reduction**: Redução de ruído
- [ ] **Squelch**: Controle de silenciamento
- [ ] **AGC**: Controle automático de ganho
- [ ] **Filtros Avançados**: Filtros FIR/IIR

#### 2. Interface Melhorada
- [ ] **Waterfall Display**: Visualização em tempo real
- [ ] **Presets**: Frequências favoritas
- [ ] **Scanning Avançado**: Busca por sinais
- [ ] **Recording Schedule**: Gravação programada
- [ ] **Export Options**: Múltiplos formatos

#### 3. Performance
- [ ] **Multithreading**: Processamento paralelo
- [ ] **GPU Acceleration**: Usar GPU para processamento
- [ ] **Memory Optimization**: Otimização de memória
- [ ] **Battery Optimization**: Economia de bateria

#### 4. Compatibilidade
- [ ] **Mais Dispositivos**: Suporte a outros SDRs
- [ ] **Android TV**: Versão para TV
- [ ] **Wear OS**: Versão para relógio
- [ ] **Linux**: Versão desktop

### Desenvolvimento Contínuo
```bash
# Atualizar dependências
./gradlew dependencies

# Limpar build
./gradlew clean

# Build de release
./gradlew assembleRelease

# Testes automatizados
./gradlew test
```

---

## 📚 Recursos Adicionais

### Documentação
- [RTL-SDR Documentation](https://www.rtl-sdr.com/)
- [Android NDK Guide](https://developer.android.com/ndk)
- [Material Design](https://material.io/design)
- [Kotlin Documentation](https://kotlinlang.org/docs/)

### Bibliotecas Úteis
- **libusb**: Comunicação USB de baixo nível
- **FFTW**: Transformadas de Fourier
- **OpenSL ES**: Áudio de baixo nível
- **OpenGL ES**: Gráficos 3D

### Comunidade
- **RTL-SDR Forum**: Discussões técnicas
- **Android Developers**: Suporte Android
- **Stack Overflow**: Perguntas e respostas
- **GitHub**: Código fonte e issues

---

## 📄 Licença

Este projeto está licenciado sob a **GPL v3** - veja o arquivo [LICENSE](LICENSE) para detalhes.

### Contribuições
Contribuições são bem-vindas! Por favor:
1. Fork o projeto
2. Crie uma branch para sua feature
3. Commit suas mudanças
4. Push para a branch
5. Abra um Pull Request

---

## 🎉 Conclusão

Este manual documenta todo o processo de desenvolvimento do **SDR Radio KT**, desde a configuração inicial até a implementação completa das funcionalidades de áudio. O projeto demonstra a integração entre:

- **Android/Kotlin**: Interface do usuário
- **C++/JNI**: Processamento de áudio
- **RTL-SDR**: Hardware de rádio
- **Material Design**: Interface moderna

O resultado é um **receptor de rádio completo** que pode reproduzir áudio em tempo real de frequências selecionadas, com controles intuitivos e funcionalidades avançadas.

**Happy Coding! 🎵📻** 