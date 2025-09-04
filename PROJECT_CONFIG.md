# 📡 SDR Radio Android Apps - Configuração do Projeto

## 🎯 Visão Geral

Este projeto oferece múltiplas implementações de aplicações Android para Software Defined Radio (SDR), cada uma com características e objetivos específicos.

## 🏗️ Estrutura dos Projetos

### 🟢 kotlin/ - **RECOMENDADO**
- **Linguagem**: Kotlin (100%)
- **Foco**: Implementação moderna e eficiente
- **Target**: Usuários avançados e desenvolvedores
- **Features**: DSP otimizado, interface Material Design

### 🔵 java/ - **ESTÁVEL**
- **Linguagem**: Java (100%)
- **Foco**: Compatibilidade e simplicidade
- **Target**: Iniciantes e compatibilidade máxima
- **Features**: Interface simples, fácil manutenção

### 🟡 java_2/ - **AVANÇADO**
- **Linguagem**: Java com componentes nativos
- **Foco**: Performance e funcionalidades extras
- **Target**: Usuários intermediários
- **Features**: Análise avançada, logs detalhados

### 🔴 cpp/ - **PERFORMANCE**
- **Linguagem**: C++ com JNI
- **Foco**: Máxima performance
- **Target**: Aplicações críticas de performance
- **Features**: DSP nativo, processamento otimizado

## ⚙️ Configuração Técnica

### 📋 Requisitos
```properties
# Build System
Gradle: 9.0.0
Android Gradle Plugin: 8.8.0
JVM: OpenJDK 21

# Android
Compile SDK: 35 (Android 15)
Min SDK: 24 (Android 7.0)
Target SDK: 35

# NDK (apenas cpp/)
NDK: 25.1.8937393
CMake: 3.22.1
```

### 🔧 Dependências Principais
```gradle
// UI
implementation 'com.google.android.material:material:1.11.0'
implementation 'androidx.appcompat:appcompat:1.6.1'

// USB/Hardware
implementation 'com.android.support:support-usb:28.0.0'

// Logging
implementation 'com.orhanobut:logger:2.2.0'
```

## 🚀 Quick Start

### 1️⃣ Preparação do Ambiente
```bash
# Instalar JDK 21
sudo apt install openjdk-21-jdk

# Configurar JAVA_HOME
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64

# Verificar instalação
java -version
javac -version
```

### 2️⃣ Clone e Build
```bash
# Clone o repositório
git clone https://github.com/usuario/radio_sdr_app.git
cd radio_sdr_app

# Build projeto recomendado (Kotlin)
cd kotlin
./gradlew assembleDebug

# Instalar no dispositivo
./gradlew installDebug
```

### 3️⃣ Configuração do Dispositivo
```bash
# Habilitar Developer Options e USB Debugging
# Conectar dispositivo via ADB
adb devices

# Verificar permissões USB
adb shell dumpsys usb
```

## 📱 Configuração por Projeto

### 🟢 Kotlin
```bash
cd kotlin/
./gradlew clean assembleDebug
# APK: app/build/outputs/apk/debug/app-debug.apk
```

### 🔵 Java
```bash
cd java/
./gradlew clean assembleDebug
# APK: app/build/outputs/apk/debug/app-debug.apk
```

### 🟡 Java_2
```bash
cd java_2/
./gradlew clean assembleDebug
# APK: app/build/outputs/apk/debug/app-debug.apk
```

### 🔴 C++
```bash
cd cpp/
# Primeiro build pode ser mais lento (compilação nativa)
./gradlew clean assembleDebug
# APK: app/build/outputs/apk/debug/app-debug.apk
```

## 🔧 Configurações Avançadas

### 📊 Performance Profiling
```bash
# Habilitar profiling (em gradle.properties)
android.enableProfiler=true
org.gradle.jvmargs=-XX:+UseG1GC -Xmx4g

# Build com informações de debug
./gradlew assembleDebug -Pandroid.injected.invoked.from.ide=true
```

### 🧪 Testing
```bash
# Executar todos os testes
./gradlew test

# Testes específicos
./gradlew testDebugUnitTest

# Coverage report
./gradlew createDebugCoverageReport
```

### 📋 Lint e Code Quality
```bash
# Android Lint
./gradlew lint

# Detekt (Kotlin)
./gradlew detekt

# Spotbugs (Java)
./gradlew spotbugsMain
```

## 🔌 Hardware SDR Suportado

### ✅ Testado e Suportado
- **RTL2832U**: dongles genéricos (0x0bda:0x2838)
- **RTL-SDR Blog V3**: versão aprimorada
- **NooElec NESDR**: série completa
- **FlightAware Pro Stick**: versão filtrada

### ⚠️ Experimental
- **HackRF One**: suporte básico
- **SDRplay RSP**: em desenvolvimento
- **PlutoSDR**: planejado para v1.2

### 📋 Configuração USB
```xml
<!-- Filtros automáticos configurados em: -->
app/src/main/res/xml/device_filter.xml

<!-- Vendor ID: 0x0bda (Realtek) -->
<!-- Product IDs: 0x2832, 0x2838, 0x2834, etc. -->
```

## 🛠️ Troubleshooting

### 🔧 Problemas Comuns

#### Build Failures
```bash
# Limpar completamente
./gradlew clean
rm -rf .gradle/
rm -rf app/build/

# Redownload dependencies
./gradlew build --refresh-dependencies
```

#### USB Não Detectado
```bash
# Verificar dispositivos USB
adb shell lsusb

# Verificar permissões
adb shell dumpsys usb

# Force permission dialog
adb shell am start -a android.hardware.usb.action.USB_DEVICE_ATTACHED
```

#### Performance Issues
```bash
# Verificar CPU/RAM usage
adb shell top | grep com.sdrradio

# Memory profiling
adb shell dumpsys meminfo com.sdrradio.app
```

### 📊 Logs e Debug
```bash
# Logs específicos do app
adb logcat -s "SDRRadio"

# Logs completos com filtros
adb logcat | grep -E "(SDR|USB|Audio)"

# Salvar logs em arquivo
adb logcat -s "SDRRadio" > debug_logs.txt
```

## 📋 Configurações Recomendadas

### 🎯 Para Desenvolvimento
```properties
# gradle.properties local
org.gradle.jvmargs=-Xmx8g -Dfile.encoding=UTF-8
org.gradle.parallel=true
org.gradle.caching=true
android.useAndroidX=true
android.enableJetifier=true
```

### 🚀 Para Produção
```gradle
// build.gradle (app)
android {
    buildTypes {
        release {
            minifyEnabled true
            shrinkResources true
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
}
```

### 📱 Para Testing
```bash
# Dispositivos recomendados para teste
# - Android 7+ (API 24+)
# - USB OTG support
# - 3GB+ RAM
# - ARM64 preferred

# Emuladores não suportam USB OTG
# Necessário device físico para testes completos
```

## 🆘 Suporte

### 📚 Documentação
- **README.md**: Instruções básicas
- **CONTRIBUTING.md**: Como contribuir
- **ROADMAP.md**: Planos futuros

### 💬 Comunidade
- **GitHub Issues**: Bugs e feature requests
- **GitHub Discussions**: Perguntas e discussões
- **Email**: Contato direto para questões específicas

### 🐛 Reportar Problemas
1. Use o template de bug report
2. Inclua logs relevantes
3. Especifique hardware usado
4. Descreva passos para reproduzir

---

**Última atualização**: Dezembro 2024  
**Versão**: v1.0.0  
**Maintainer**: [@username](https://github.com/username)
