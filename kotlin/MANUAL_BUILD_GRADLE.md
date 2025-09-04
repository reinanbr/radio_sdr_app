# 🔨 Manual de Build - Gradle para SDR Radio KT

## 📋 Índice

1. [Pré-requisitos](#pré-requisitos)
2. [Download e Instalação](#download-e-instalação)
3. [Configuração do Ambiente](#configuração-do-ambiente)
4. [Build do Projeto](#build-do-projeto)
5. [Instalação no Device](#instalação-no-device)
6. [Troubleshooting](#troubleshooting)
7. [Comandos Úteis](#comandos-úteis)

---

## 🎯 Pré-requisitos

### Sistema Operacional
- **Linux** (Ubuntu 18.04+, Debian 10+, CentOS 7+)
- **macOS** (10.14+)
- **Windows** (10/11 com WSL2 recomendado)

### Software Necessário
- **Java JDK 11** ou superior
- **Android SDK** (API 26+)
- **Android NDK** (versão 25.1.8937393)
- **CMake** (versão 3.22.1+)
- **Git** (para clonar o repositório)

### Hardware Recomendado
- **RAM**: 8GB mínimo, 16GB recomendado
- **Espaço**: 10GB livre para SDK + builds
- **CPU**: Multi-core para builds paralelos

---

## 📥 Download e Instalação

### 1. Instalar Java JDK

#### Ubuntu/Debian:
```bash
# Atualizar repositórios
sudo apt update

# Instalar OpenJDK 11
sudo apt install openjdk-11-jdk

# Verificar instalação
java -version
javac -version
```

#### macOS:
```bash
# Usando Homebrew
brew install openjdk@11

# Configurar PATH
echo 'export PATH="/opt/homebrew/opt/openjdk@11/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# Verificar instalação
java -version
```

#### Windows:
```bash
# Baixar do site oficial Oracle ou usar Chocolatey
choco install openjdk11

# Verificar instalação
java -version
```

### 2. Instalar Android SDK

#### Linux/macOS:
```bash
# Criar diretório para Android SDK
mkdir -p ~/Android/Sdk
cd ~/Android/Sdk

# Baixar Command Line Tools
wget https://dl.google.com/android/repository/commandlinetools-linux-8512546_latest.zip

# Extrair
unzip commandlinetools-linux-8512546_latest.zip

# Criar estrutura de diretórios
mkdir -p cmdline-tools/latest
mv cmdline-tools/* cmdline-tools/latest/
rmdir cmdline-tools/latest/cmdline-tools

# Configurar PATH
echo 'export ANDROID_HOME=~/Android/Sdk' >> ~/.bashrc
echo 'export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin' >> ~/.bashrc
echo 'export PATH=$PATH:$ANDROID_HOME/platform-tools' >> ~/.bashrc
source ~/.bashrc
```

#### Windows:
```bash
# Baixar Android Studio ou Command Line Tools
# Configurar variáveis de ambiente
set ANDROID_HOME=C:\Users\%USERNAME%\AppData\Local\Android\Sdk
set PATH=%PATH%;%ANDROID_HOME%\cmdline-tools\latest\bin
set PATH=%PATH%;%ANDROID_HOME%\platform-tools
```

### 3. Instalar Componentes Android

```bash
# Aceitar licenças
yes | sdkmanager --licenses

# Instalar componentes essenciais
sdkmanager "platform-tools"
sdkmanager "platforms;android-33"
sdkmanager "build-tools;33.0.0"
sdkmanager "ndk;25.1.8937393"
sdkmanager "cmake;3.22.1"

# Verificar instalação
sdkmanager --list_installed
```

### 4. Instalar CMake (se necessário)

#### Ubuntu/Debian:
```bash
sudo apt install cmake
```

#### macOS:
```bash
brew install cmake
```

#### Windows:
```bash
# CMake já vem com Android NDK
# Ou baixar do site oficial
```

---

## ⚙️ Configuração do Ambiente

### 1. Configurar Variáveis de Ambiente

#### Linux/macOS:
```bash
# Adicionar ao ~/.bashrc ou ~/.zshrc
export ANDROID_HOME=~/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.1.8937393
export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin
export PATH=$PATH:$ANDROID_HOME/platform-tools
export PATH=$PATH:$ANDROID_HOME/ndk/25.1.8937393
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

# Recarregar configurações
source ~/.bashrc
```

#### Windows:
```cmd
# Adicionar ao PATH do sistema
ANDROID_HOME=C:\Users\%USERNAME%\AppData\Local\Android\Sdk
ANDROID_NDK_HOME=%ANDROID_HOME%\ndk\25.1.8937393
JAVA_HOME=C:\Program Files\Java\jdk-11

# Adicionar ao PATH
%ANDROID_HOME%\cmdline-tools\latest\bin
%ANDROID_HOME%\platform-tools
%ANDROID_NDK_HOME%
%JAVA_HOME%\bin
```

### 2. Verificar Configuração

```bash
# Verificar todas as variáveis
echo "ANDROID_HOME: $ANDROID_HOME"
echo "ANDROID_NDK_HOME: $ANDROID_NDK_HOME"
echo "JAVA_HOME: $JAVA_HOME"

# Verificar comandos
which adb
which gradle
which cmake
which ndk-build

# Verificar versões
java -version
gradle -version
cmake --version
```

### 3. Configurar local.properties

```bash
# Criar arquivo local.properties no projeto
cat > local.properties << EOF
sdk.dir=$ANDROID_HOME
ndk.dir=$ANDROID_NDK_HOME
EOF
```

---

## 🔨 Build do Projeto

### 1. Clonar o Repositório

```bash
# Clonar o projeto
git clone https://github.com/seu-usuario/sdr_radio_kt.git
cd sdr_radio_kt

# Verificar estrutura
ls -la
```

### 2. Download do RTL-SDR

```bash
# Executar script de download
chmod +x download_rtlsdr.sh
./download_rtlsdr.sh

# Verificar se foi baixado
ls -la app/src/main/cpp/rtl-sdr/
```

### 3. Build com Gradle Wrapper

#### Build Debug:
```bash
# Limpar builds anteriores
./gradlew clean

# Build debug
./gradlew assembleDebug

# Verificar APK gerado
ls -la app/build/outputs/apk/debug/
```

#### Build Release:
```bash
# Build release
./gradlew assembleRelease

# Verificar APK gerado
ls -la app/build/outputs/apk/release/
```

#### Build para ABI específica:
```bash
# Build apenas para ARM64
./gradlew assembleDebug -PabiFilters=arm64-v8a

# Build para múltiplas ABIs
./gradlew assembleDebug -PabiFilters=arm64-v8a,armeabi-v7a
```

### 4. Build Nativo (C++)

```bash
# Build apenas código nativo
./gradlew externalNativeBuildDebug

# Build nativo para release
./gradlew externalNativeBuildRelease

# Limpar build nativo
./gradlew cleanExternalNativeBuild
```

### 5. Verificação do Build

```bash
# Verificar APK
aapt dump badging app/build/outputs/apk/debug/app-debug.apk

# Verificar bibliotecas nativas
unzip -l app/build/outputs/apk/debug/app-debug.apk | grep "\.so"

# Verificar tamanho
ls -lh app/build/outputs/apk/debug/app-debug.apk
```

---

## 📱 Instalação no Device

### 1. Preparar Device Android

```bash
# Habilitar modo desenvolvedor
# Settings > About Phone > Tap Build Number 7 times

# Habilitar USB Debugging
# Settings > Developer Options > USB Debugging

# Conectar device via USB
adb devices

# Verificar se device está conectado
adb shell getprop ro.product.model
```

### 2. Instalar APK

```bash
# Instalar APK debug
adb install -r app/build/outputs/apk/debug/app-debug.apk

# Ou usar script de instalação
chmod +x install.sh
./install.sh
```

### 3. Verificar Instalação

```bash
# Verificar se app foi instalado
adb shell pm list packages | grep sdrradio

# Verificar permissões
adb shell dumpsys package com.sdrradio.kt | grep permission

# Verificar bibliotecas nativas
adb shell cat /proc/$(adb shell pidof com.sdrradio.kt)/maps | grep sdrradio
```

### 4. Testar Aplicação

```bash
# Executar script de teste
chmod +x test_app.sh
./test_app.sh

# Ou testar manualmente
adb shell am start -n com.sdrradio.kt/.MainActivity

# Capturar logs
adb logcat -s SDRRadio:V MainActivity:V
```

---

## 🔧 Troubleshooting

### Problemas Comuns

#### 1. **Erro de Gradle Wrapper**
```
Error: Could not find or load main class org.gradle.wrapper.GradleWrapperMain
```
**Solução:**
```bash
# Regenerar wrapper
gradle wrapper --gradle-version 7.6

# Ou baixar manualmente
wget https://services.gradle.org/distributions/gradle-7.6-bin.zip
unzip gradle-7.6-bin.zip
./gradle-7.6/bin/gradle wrapper
```

#### 2. **Erro de NDK**
```
NDK not found. Please define NDK_HOME or ndk.dir
```
**Solução:**
```bash
# Verificar NDK instalado
sdkmanager --list_installed | grep ndk

# Instalar NDK se necessário
sdkmanager "ndk;25.1.8937393"

# Configurar local.properties
echo "ndk.dir=$ANDROID_HOME/ndk/25.1.8937393" >> local.properties
```

#### 3. **Erro de CMake**
```
CMake not found
```
**Solução:**
```bash
# Instalar CMake
sdkmanager "cmake;3.22.1"

# Verificar instalação
$ANDROID_HOME/cmake/3.22.1/bin/cmake --version
```

#### 4. **Erro de Compilação C++**
```
fatal error: 'rtl-sdr.h' file not found
```
**Solução:**
```bash
# Verificar se RTL-SDR foi baixado
ls -la app/src/main/cpp/rtl-sdr/

# Rebaixar se necessário
./download_rtlsdr.sh

# Limpar e rebuildar
./gradlew clean
./gradlew assembleDebug
```

#### 5. **Erro de Permissão**
```
Permission denied
```
**Solução:**
```bash
# Dar permissão aos scripts
chmod +x gradlew
chmod +x *.sh

# Verificar permissões
ls -la gradlew
ls -la *.sh
```

#### 6. **Erro de Memória**
```
OutOfMemoryError: Java heap space
```
**Solução:**
```bash
# Aumentar heap do Gradle
echo 'org.gradle.jvmargs=-Xmx4096m -XX:MaxPermSize=512m' >> gradle.properties

# Ou usar variável de ambiente
export GRADLE_OPTS="-Xmx4096m -XX:MaxPermSize=512m"
```

### Comandos de Debug

```bash
# Verificar versões
./gradlew --version
java -version
cmake --version

# Verificar configuração
./gradlew properties

# Build com debug
./gradlew assembleDebug --info
./gradlew assembleDebug --debug

# Verificar dependências
./gradlew dependencies

# Limpar cache
./gradlew cleanBuildCache
```

---

## 📋 Comandos Úteis

### Gradle Básico
```bash
# Limpar projeto
./gradlew clean

# Build debug
./gradlew assembleDebug

# Build release
./gradlew assembleRelease

# Build nativo
./gradlew externalNativeBuildDebug

# Testes
./gradlew test
./gradlew connectedAndroidTest

# Verificar
./gradlew check
./gradlew lint
```

### Gradle Avançado
```bash
# Build paralelo
./gradlew assembleDebug --parallel

# Build offline
./gradlew assembleDebug --offline

# Build com profile
./gradlew assembleDebug --profile

# Build específico
./gradlew :app:assembleDebug

# Dependências
./gradlew dependencies
./gradlew dependencyInsight --dependency androidx.core:core-ktx
```

### ADB Útil
```bash
# Listar devices
adb devices

# Instalar APK
adb install -r app-debug.apk

# Desinstalar app
adb uninstall com.sdrradio.kt

# Capturar logs
adb logcat -s SDRRadio:V

# Executar app
adb shell am start -n com.sdrradio.kt/.MainActivity

# Parar app
adb shell am force-stop com.sdrradio.kt

# Screenshot
adb shell screencap /sdcard/screenshot.png
adb pull /sdcard/screenshot.png
```

### Scripts Automatizados
```bash
# Build completo
./build_complete.sh

# Teste completo
./test_complete.sh

# Deploy automático
./deploy.sh

# Limpeza completa
./clean_all.sh
```

---

## 🎯 Exemplo de Workflow Completo

### 1. Setup Inicial
```bash
# Clonar projeto
git clone https://github.com/seu-usuario/sdr_radio_kt.git
cd sdr_radio_kt

# Configurar ambiente
export ANDROID_HOME=~/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.1.8937393
export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools

# Criar local.properties
echo "sdk.dir=$ANDROID_HOME" > local.properties
echo "ndk.dir=$ANDROID_NDK_HOME" >> local.properties
```

### 2. Download Dependências
```bash
# Download RTL-SDR
./download_rtlsdr.sh

# Verificar downloads
ls -la app/src/main/cpp/rtl-sdr/
```

### 3. Build e Teste
```bash
# Build debug
./gradlew clean assembleDebug

# Verificar APK
ls -la app/build/outputs/apk/debug/

# Conectar device
adb devices

# Instalar e testar
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.sdrradio.kt/.MainActivity

# Capturar logs
adb logcat -s SDRRadio:V MainActivity:V
```

### 4. Build Release
```bash
# Build release
./gradlew assembleRelease

# Assinar APK (se necessário)
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore my-release-key.keystore app-release-unsigned.apk alias_name

# Otimizar APK
zipalign -v 4 app-release-unsigned.apk app-release.apk
```

---

## 📊 Monitoramento de Performance

### Build Time
```bash
# Medir tempo de build
time ./gradlew assembleDebug

# Build com profile
./gradlew assembleDebug --profile
# Verificar relatório em build/reports/profile/
```

### APK Size
```bash
# Analisar tamanho do APK
./gradlew assembleDebug
du -h app/build/outputs/apk/debug/app-debug.apk

# Analisar conteúdo
aapt dump badging app/build/outputs/apk/debug/app-debug.apk
```

### Memory Usage
```bash
# Monitorar uso de memória durante build
./gradlew assembleDebug --profile --scan
```

---

## 🎯 Conclusão

Este manual fornece um **workflow completo** para build do projeto SDR Radio KT usando Gradle:

### ✅ **Benefícios:**
- **Automatização**: Scripts para download e build
- **Reprodutibilidade**: Ambiente configurado consistentemente
- **Debugging**: Comandos para troubleshooting
- **Performance**: Otimizações de build

### 🔧 **Aspectos Técnicos:**
- **Gradle Wrapper**: Versão consistente do Gradle
- **NDK Integration**: Build de código C++ nativo
- **Multi-ABI**: Suporte para diferentes arquiteturas
- **CI/CD Ready**: Scripts prontos para automação

### 📈 **Próximos Passos:**
- Configurar CI/CD (GitHub Actions, Jenkins)
- Otimizar build times
- Implementar testes automatizados
- Configurar distribuição automática

O projeto está pronto para desenvolvimento e distribuição! 🚀📱 