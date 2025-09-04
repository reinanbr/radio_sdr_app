# 📻 Manual Completo - SDR Radio Android

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Arquitetura do Sistema](#arquitetura-do-sistema)
3. [Requisitos](#requisitos)
4. [Instalação e Configuração](#instalação-e-configuração)
5. [Estrutura do Projeto](#estrutura-do-projeto)
6. [Compilação e Build](#compilação-e-build)
7. [Instalação no Dispositivo](#instalação-no-dispositivo)
8. [Como Usar](#como-usar)
9. [Troubleshooting](#troubleshooting)
10. [Desenvolvimento](#desenvolvimento)
11. [Referências Técnicas](#referências-técnicas)

---

## 🎯 Visão Geral

O **SDR Radio Android** é um aplicativo completo para Software Defined Radio (SDR) que permite sintonizar, reproduzir e gravar sinais de rádio usando dispositivos RTL-SDR via USB OTG.

### Funcionalidades Principais:
- 🔌 **Detecção automática** de dispositivos RTL-SDR
- 📻 **Sintonização** de 24 MHz a 1.766 GHz
- 🔊 **Reprodução de áudio** em tempo real
- 📹 **Gravação** com timestamp automático
- 📊 **Analisador de espectro** em tempo real
- 📶 **Medidor de força do sinal**
- ⚡ **Presets de frequências** (FM, AM, Aeronáutica, etc.)
- 🎨 **Interface moderna** com tema escuro

---

## 🏗️ Arquitetura do Sistema

### Componentes Principais:

```
┌─────────────────────────────────────────────────────────────┐
│                    SDR Radio Android                        │
├─────────────────────────────────────────────────────────────┤
│  MainActivity (UI Principal)                                │
│  ├── RTLSDROperations (Comunicação USB)                     │
│  ├── AudioProcessor (Processamento de Áudio)                │
│  ├── SpectrumAnalyzer (Análise FFT)                         │
│  ├── SDRService (Processamento em Background)               │
│  └── USBReceiver (Detecção de Dispositivos)                 │
└─────────────────────────────────────────────────────────────┘
```

### Fluxo de Dados:
1. **USB OTG** → RTL-SDR
2. **RTL-SDR** → RTLSDROperations (I/Q Data)
3. **I/Q Data** → SpectrumAnalyzer (FFT)
4. **I/Q Data** → AudioProcessor (Demodulação)
5. **Áudio** → AudioTrack (Reprodução)
6. **Dados** → UI (Visualização)

---

## 📱 Requisitos

### Hardware:
- **Dispositivo Android** com suporte a USB OTG
- **RTL-SDR** (RTL2832U + R820T/R820T2)
- **Cabo OTG** compatível
- **Antena** (opcional, mas recomendada)

### Software:
- **Android 5.0+** (API 21)
- **Gradle 7.6**
- **Android SDK** com Build Tools
- **Java 8+**

### Permissões:
- `USB_PERMISSION`
- `RECORD_AUDIO`
- `WRITE_EXTERNAL_STORAGE`
- `READ_EXTERNAL_STORAGE`

---

## ⚙️ Instalação e Configuração

### 1. Preparação do Ambiente

```bash
# Verificar se o Android SDK está instalado
which adb
# Saída esperada: /path/to/android/sdk/platform-tools/adb

# Verificar se o Java está instalado
java -version
# Saída esperada: Java 8 ou superior
```

### 2. Clone do Projeto

```bash
git clone https://github.com/seu-usuario/sdr-radio.git
cd sdr-radio
```

### 3. Download do Gradle

```bash
# Baixar Gradle 7.6
wget https://services.gradle.org/distributions/gradle-7.6-bin.zip

# Extrair
unzip gradle-7.6-bin.zip

# Copiar wrapper JAR
mkdir -p gradle/wrapper
cp gradle-7.6/lib/plugins/gradle-wrapper-7.6.jar gradle/wrapper/gradle-wrapper.jar
```

### 4. Configuração do Gradle

**gradle.properties:**
```properties
org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
org.gradle.parallel=true
android.useAndroidX=true
android.enableJetifier=true
org.gradle.caching=true
org.gradle.configuration-cache=true
android.suppressUnsupportedCompileSdk=34
kotlin.stdlib.default.dependency=false
```

**build.gradle (root):**
```gradle
buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:7.2.2'
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

---

## 📁 Estrutura do Projeto

```
sdr_radio/
├── app/
│   ├── src/main/
│   │   ├── java/com/sdrradio/app/
│   │   │   ├── MainActivity.java              # Activity principal
│   │   │   ├── service/
│   │   │   │   └── SDRService.java            # Serviço em background
│   │   │   ├── receiver/
│   │   │   │   └── USBReceiver.java           # Receiver USB
│   │   │   └── utils/
│   │   │       ├── RTLSDROperations.java      # Operações RTL-SDR
│   │   │       ├── AudioProcessor.java        # Processamento de áudio
│   │   │       └── SpectrumAnalyzer.java      # Análise de espectro
│   │   ├── res/
│   │   │   ├── layout/
│   │   │   │   └── activity_main.xml          # Layout principal
│   │   │   ├── values/
│   │   │   │   ├── colors.xml                 # Cores
│   │   │   │   ├── strings.xml                # Strings
│   │   │   │   └── themes.xml                 # Temas
│   │   │   ├── xml/
│   │   │   │   ├── device_filter.xml          # Filtro USB
│   │   │   │   ├── backup_rules.xml           # Regras de backup
│   │   │   │   └── data_extraction_rules.xml  # Regras de extração
│   │   │   └── mipmap-*/                      # Ícones do launcher
│   │   └── AndroidManifest.xml                # Manifesto
│   ├── build.gradle                           # Configuração do módulo
│   └── proguard-rules.pro                     # Regras ProGuard
├── gradle/
│   └── wrapper/
│       ├── gradle-wrapper.jar                 # Gradle wrapper
│       └── gradle-wrapper.properties          # Propriedades do wrapper
├── build.gradle                               # Configuração root
├── settings.gradle                            # Configuração do projeto
├── gradle.properties                          # Propriedades do Gradle
├── gradlew                                    # Script do wrapper
└── README.md                                  # Documentação
```

---

## 🔨 Compilação e Build

### 1. Limpeza do Projeto

```bash
./gradle-7.6/bin/gradle clean
```

### 2. Build Debug

```bash
./gradle-7.6/bin/gradle assembleDebug
```

### 3. Verificação do APK

```bash
# Verificar se o APK foi gerado
ls -la app/build/outputs/apk/debug/
# Deve mostrar: app-debug.apk
```

### 4. Build Release (Opcional)

```bash
./gradle-7.6/bin/gradle assembleRelease
```

---

## 📱 Instalação no Dispositivo

### 1. Preparação do Dispositivo

```bash
# Habilitar depuração USB no dispositivo Android
# Configurações > Sobre o telefone > Toque 7 vezes no "Número da versão"
# Configurações > Opções do desenvolvedor > Depuração USB (ativar)

# Verificar se o dispositivo está conectado
adb devices
# Saída esperada:
# List of devices attached
# XXXXXXXX    device
```

### 2. Instalação do APK

```bash
# Instalar APK debug
./gradle-7.6/bin/gradle installDebug

# Ou instalar manualmente
adb install app/build/outputs/apk/debug/app-debug.apk
```

### 3. Verificação da Instalação

```bash
# Verificar se o app está instalado
adb shell pm list packages | grep sdrradio
# Saída esperada: package:com.sdrradio.app

# Abrir o aplicativo
adb shell am start -n com.sdrradio.app/.MainActivity
```

---

## 🎮 Como Usar

### 1. Primeira Execução

1. **Abra o aplicativo** no dispositivo Android
2. **Conecte o RTL-SDR** via cabo OTG
3. **Conceda as permissões** quando solicitado:
   - USB
   - Áudio
   - Armazenamento

### 2. Conectar Dispositivo

1. **Clique em "Conectar Dispositivo"**
2. **Aguarde a detecção** do RTL-SDR
3. **Status deve mostrar** "Conectado"

### 3. Sintonizar Frequências

#### Método 1: Slider
- **Arraste o slider** para a frequência desejada
- **Frequência atual** é exibida em MHz

#### Método 2: Botões +/-
- **Clique em "+"** para aumentar 1 MHz
- **Clique em "-"** para diminuir 1 MHz

#### Método 3: Presets Rápidos
- **Clique no FAB** (botão flutuante)
- **Selecione um preset**:
  - FM (88-108 MHz)
  - AM (535-1605 kHz)
  - Aeronáutica (118-137 MHz)
  - Marítima (156-174 MHz)
  - Meteorologia (162 MHz)
  - Rádio Amador (144-148 MHz)

### 4. Reproduzir Áudio

1. **Clique no botão "▶"** para iniciar
2. **Ajuste o volume** com o slider
3. **Use "Mutar"** para silenciar
4. **Clique em "⏸"** para pausar

### 5. Gravar Áudio

1. **Clique em "Iniciar Gravação"**
2. **Status mostra** "Gravando..."
3. **Clique em "Parar Gravação"** para finalizar
4. **Arquivo salvo** em "SDR_Recordings"

### 6. Analisar Espectro

- **Gráfico em tempo real** mostra a distribuição de frequências
- **Medidor de força** indica a intensidade do sinal
- **Cores mudam** conforme a força do sinal:
  - 🔴 Vermelho: Sinal fraco
  - 🟠 Laranja: Sinal médio
  - 🟢 Verde: Sinal forte
  - 🔵 Azul: Sinal excelente

---

## 🔧 Troubleshooting

### Problema: Dispositivo não detectado

**Sintomas:**
- "Nenhum dispositivo RTL-SDR encontrado"
- Botão "Conectar" não funciona

**Soluções:**
1. **Verificar cabo OTG:**
   ```bash
   # Testar se o OTG funciona
   adb shell lsusb
   ```

2. **Verificar RTL-SDR:**
   ```bash
   # Testar em computador primeiro
   # Usar software como SDR# ou GQRX
   ```

3. **Verificar permissões USB:**
   ```bash
   # Verificar permissões
   adb shell dumpsys usb
   ```

### Problema: Sem áudio

**Sintomas:**
- Aplicativo conectado mas sem som
- Botão play não funciona

**Soluções:**
1. **Verificar permissões de áudio:**
   ```bash
   adb shell dumpsys audio
   ```

2. **Verificar volume:**
   - Ajustar volume do dispositivo
   - Verificar se não está no modo silencioso

3. **Verificar sinais:**
   - Testar em frequências conhecidas (FM comercial)
   - Verificar se há sinais na área

### Problema: Erro de conexão USB

**Sintomas:**
- "Erro ao conectar dispositivo"
- Aplicativo trava

**Soluções:**
1. **Reconectar hardware:**
   ```bash
   # Desconectar e reconectar RTL-SDR
   adb shell dumpsys usb
   ```

2. **Reiniciar aplicativo:**
   ```bash
   adb shell am force-stop com.sdrradio.app
   adb shell am start -n com.sdrradio.app/.MainActivity
   ```

3. **Verificar outros apps:**
   - Fechar outros aplicativos que usem USB
   - Reiniciar dispositivo se necessário

### Problema: Gravação não funciona

**Sintomas:**
- "Erro ao iniciar gravação"
- Arquivos não são salvos

**Soluções:**
1. **Verificar permissões de armazenamento:**
   ```bash
   adb shell dumpsys package com.sdrradio.app | grep permission
   ```

2. **Verificar espaço em disco:**
   ```bash
   adb shell df /sdcard
   ```

3. **Verificar diretório:**
   ```bash
   adb shell ls -la /sdcard/Music/SDR_Recordings/
   ```

### Problema: Build falha

**Sintomas:**
- Erros de compilação
- Dependências não encontradas

**Soluções:**
1. **Limpar cache:**
   ```bash
   ./gradle-7.6/bin/gradle clean
   rm -rf .gradle/
   rm -rf build/
   ```

2. **Verificar dependências:**
   ```bash
   ./gradle-7.6/bin/gradle dependencies
   ```

3. **Atualizar Gradle:**
   ```bash
   # Baixar versão mais recente se necessário
   wget https://services.gradle.org/distributions/gradle-8.0-bin.zip
   ```

---

## 💻 Desenvolvimento

### Estrutura de Classes

#### MainActivity.java
```java
public class MainActivity extends AppCompatActivity {
    // Componentes principais
    private ActivityMainBinding binding;
    private UsbManager usbManager;
    private RTLSDROperations rtlSdrOperations;
    private AudioProcessor audioProcessor;
    private SpectrumAnalyzer spectrumAnalyzer;
    
    // Estados
    private boolean isConnected = false;
    private boolean isPlaying = false;
    private boolean isRecording = false;
    private int currentFrequency = 100000000; // 100 MHz
}
```

#### RTLSDROperations.java
```java
public class RTLSDROperations {
    // Constantes USB
    private static final int RTL2832U_VENDOR_ID = 0x0bda;
    private static final int RTL2832U_PRODUCT_ID = 0x2838;
    
    // Métodos principais
    public boolean initialize(UsbDeviceConnection connection, UsbDevice device);
    public void setFrequency(int frequency);
    public void setGain(int gain);
    public float[] getSpectrumData();
    public void startStreaming();
    public void stopStreaming();
}
```

#### AudioProcessor.java
```java
public class AudioProcessor {
    // Configurações de áudio
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_OUT_MONO;
    
    // Métodos principais
    public void startPlayback();
    public void stopPlayback();
    public void processIQData(float[] iData, float[] qData);
    public void demodulateFM(float[] iData, float[] qData);
    public void demodulateAM(float[] iData, float[] qData);
}
```

#### SpectrumAnalyzer.java
```java
public class SpectrumAnalyzer {
    // Configurações FFT
    private static final int FFT_SIZE = 1024;
    private float[] fftBuffer = new float[FFT_SIZE * 2];
    
    // Métodos principais
    public float[] analyzeSpectrum(float[] iqData);
    private void performFFT(float[] data, int n);
    public float[] getWaterfallData(float[] spectrum);
    public float calculateSignalStrength(float[] spectrum);
}
```

### Adicionando Novas Funcionalidades

#### 1. Nova Demodulação (SSB)
```java
// Em AudioProcessor.java
public void demodulateSSB(float[] iData, float[] qData) {
    // Implementar demodulação SSB
    for (int i = 0; i < iData.length; i++) {
        // Algoritmo SSB
        float ssbSignal = iData[i] * Math.cos(phase) + qData[i] * Math.sin(phase);
        audioBuffer[i] = ssbSignal;
    }
}
```

#### 2. Nova Frequência Preset
```java
// Em MainActivity.java
private void showQuickActionsDialog() {
    String[] actions = {
        // ... presets existentes ...
        "Nova Banda (frequência)"  // Adicionar novo preset
    };
}
```

#### 3. Nova Análise de Espectro
```java
// Em SpectrumAnalyzer.java
public float[] analyzeWaterfall(float[] spectrum) {
    // Implementar waterfall display
    float[] waterfall = new float[spectrum.length];
    // Algoritmo de waterfall
    return waterfall;
}
```

### Debugging

#### Logs do Sistema
```bash
# Ver logs do aplicativo
adb logcat -s MainActivity:SDRService:RTLSDROperations

# Ver logs de USB
adb logcat -s UsbDeviceManager:UsbHostManager

# Ver logs de áudio
adb logcat -s AudioManager:AudioTrack
```

#### Debug Remoto
```bash
# Conectar debugger
adb forward tcp:5005 tcp:5005

# No Android Studio:
# Run > Edit Configurations > Remote JVM Debug
# Host: localhost, Port: 5005
```

---

## 📚 Referências Técnicas

### RTL-SDR Especificações

#### Hardware
- **Chipset**: RTL2832U + R820T/R820T2
- **Frequência**: 24 MHz - 1.766 GHz
- **Taxa de Amostragem**: 2.048 MHz
- **Resolução**: 8 bits
- **Interface**: USB 2.0

#### Registradores RTL2832U
```c
// Registradores principais
#define DEMOD_CTL    0x00
#define GPO          0x19
#define GPI          0x1a
#define SYSINTE      0x01
#define SYSINTS      0x02
```

#### Registradores R820T
```c
// Registradores do tuner
#define R820T_CTRL       0x05
#define R820T_CTRL_SYN   0x40
#define R820T_CTRL_VCO   0x80
#define R820T_CTRL_XTAL  0x10
```

### FFT e Processamento de Sinal

#### Configurações FFT
```java
// Tamanho da FFT
private static final int FFT_SIZE = 1024;

// Janela de Hamming
window[i] = 0.54f - 0.46f * cos(2 * PI * i / (FFT_SIZE - 1));

// Conversão para dB
spectrum[i] = 20 * log10(magnitude);
```

#### Demodulação FM
```java
// Demodulação por diferença de fase
float phase1 = atan2(qData[i-1], iData[i-1]);
float phase2 = atan2(qData[i], iData[i]);
float phaseDiff = phase2 - phase1;

// Normalização
if (phaseDiff > PI) phaseDiff -= 2 * PI;
if (phaseDiff < -PI) phaseDiff += 2 * PI;

// Conversão para áudio
audioBuffer[i-1] = phaseDiff * modulationIndex;
```

#### Demodulação AM
```java
// Demodulação por envelope
for (int i = 0; i < iData.length; i++) {
    float magnitude = sqrt(iData[i] * iData[i] + qData[i] * qData[i]);
    audioBuffer[i] = (magnitude - 1.0f) * modulationIndex;
}
```

### USB Communication

#### Endpoints
```java
// Endpoints USB
private static final int USB_ENDPOINT_IN = 0x81;
private static final int USB_ENDPOINT_OUT = 0x02;
private static final int USB_TIMEOUT = 5000;
```

#### Control Transfers
```java
// Escrita de registrador
private void writeDemodReg(int reg, int value) {
    byte[] data = {0x01, (byte) reg, (byte) value};
    connection.controlTransfer(0x40, 0x01, 0, 0, data, data.length, USB_TIMEOUT);
}

// Leitura de registrador
private int readDemodReg(int reg) {
    byte[] data = new byte[1];
    connection.controlTransfer(0xC0, 0x01, 0, reg, data, data.length, USB_TIMEOUT);
    return data[0] & 0xFF;
}
```

### Audio Processing

#### Configurações de Áudio
```java
// Configurações do AudioTrack
private static final int SAMPLE_RATE = 44100;
private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_OUT_MONO;
private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
private static final int BUFFER_SIZE = AudioTrack.getMinBufferSize(SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT);
```

#### Gravação de Áudio
```java
// Configuração do MediaRecorder
mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
mediaRecorder.setOutputFile(recordingFile.getAbsolutePath());
```

### Performance e Otimização

#### Otimizações de Memória
```java
// Reutilização de buffers
private float[] audioBuffer = new float[BUFFER_SIZE / 2];
private short[] outputBuffer = new short[BUFFER_SIZE / 2];

// Evitar alocação de objetos em loops
for (int i = 0; i < spectrumData.length; i++) {
    entries.add(new Entry(i, spectrumData[i]));
}
```

#### Otimizações de CPU
```java
// Uso de threads separadas
private Thread streamingThread;
private Thread audioThread;

// Sincronização
private final Object streamingLock = new Object();
private final AtomicBoolean isStreaming = new AtomicBoolean(false);
```

#### Otimizações de UI
```java
// Atualização da UI na thread principal
handler.post(() -> {
    updateSpectrumChart(spectrumData);
    updateSignalStrength(spectrumData);
});

// Redução da frequência de atualização
handler.postDelayed(this, 100); // 10 FPS
```

---

## 📄 Licença

Este projeto está licenciado sob a Licença MIT.

```
MIT License

Copyright (c) 2024 SDR Radio Android

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🤝 Contribuições

Contribuições são bem-vindas! Por favor:

1. **Fork** o projeto
2. **Crie uma branch** para sua feature
3. **Commit** suas mudanças
4. **Push** para a branch
5. **Abra um Pull Request**

### Diretrizes de Contribuição:
- Siga o estilo de código existente
- Adicione testes para novas funcionalidades
- Atualize a documentação
- Verifique se o build passa

---

## 📞 Suporte

### Problemas Conhecidos:
- Alguns dispositivos podem ter problemas com USB OTG
- Performance pode variar dependendo do hardware
- Demodulação SSB não implementada ainda

### Comunidade:
- **GitHub Issues**: Para reportar bugs
- **GitHub Discussions**: Para discussões gerais
- **Wiki**: Para documentação adicional

### Recursos Adicionais:
- [RTL-SDR Blog](https://www.rtl-sdr.com/)
- [Android USB Host API](https://developer.android.com/guide/topics/connectivity/usb/host)
- [Material Design Guidelines](https://material.io/design)

---

**Desenvolvido com ❤️ para a comunidade SDR**

*Este manual foi criado para facilitar o uso e desenvolvimento do aplicativo SDR Radio Android. Para dúvidas ou sugestões, consulte a seção de suporte.* 