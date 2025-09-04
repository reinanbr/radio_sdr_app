# 🔧 Troubleshooting - SDR Radio Apps

Este documento contém soluções para problemas comuns encontrados durante o desenvolvimento e uso das aplicações SDR.

## 📱 Problemas de Build

### ❌ Gradle Build Failures

#### Problema: "Unsupported class file major version"
```
BUG! exception in phase 'semantic analysis' in source unit '_BuildScript_' 
Unsupported class file major version 65
```

**Causa**: Incompatibilidade entre versões de Java e Gradle.

**Solução**:
```bash
# Verificar versões de Java disponíveis
update-alternatives --list java

# Usar Java 17 para React Native
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Limpar cache Gradle
rm -rf ~/.gradle/caches
./gradlew clean

# Build novamente
./gradlew assembleDebug
```

#### Problema: "Kotlin version conflicts"
```
Class 'kotlin.Unit' was compiled with an incompatible version of Kotlin
```

**Causa**: Conflito entre versões de Kotlin no projeto.

**Solução**:
```gradle
// android/build.gradle
buildscript {
    ext {
        kotlinVersion = "1.8.10" // Para RN 0.72
        // ou
        kotlinVersion = "1.9.0"  // Para RN 0.74+
    }
}
```

#### Problema: "Metro bundler errors"
```
Error: Package subpath './src/lib/TerminalReporter' is not defined by "exports"
```

**Causa**: Versões incompatíveis do Metro com React Native.

**Solução**:
```bash
# Instalar versões específicas compatíveis
npm install --exact react-native@0.72.15
npm install --legacy-peer-deps

# Ou usar yarn
yarn install --ignore-engines
```

### ❌ USB Permission Issues

#### Problema: RTL-SDR não é detectado
```
USB device not detected by Android
```

**Diagnóstico**:
```bash
# Verificar dispositivos USB via ADB
adb shell lsusb

# Verificar permissões USB
adb shell dumpsys usb

# Ver logs específicos
adb logcat -s "USB"
```

**Soluções**:

1. **Verificar device_filter.xml**:
```xml
<!-- app/src/main/res/xml/device_filter.xml -->
<usb-device vendor-id="0x0bda" product-id="0x2838" />
<usb-device vendor-id="0x0bda" product-id="0x2832" />
```

2. **Verificar AndroidManifest.xml**:
```xml
<uses-permission android:name="android.permission.USB_PERMISSION" />
<uses-feature android:name="android.hardware.usb.host" android:required="true" />
```

3. **Forçar dialog de permissão**:
```bash
adb shell am start -a android.hardware.usb.action.USB_DEVICE_ATTACHED
```

## 📋 Problemas de Runtime

### ❌ Audio Recording Issues

#### Problema: "Audio permission denied"
```
java.lang.SecurityException: Permission denial
```

**Solução**:
```xml
<!-- AndroidManifest.xml -->
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />
```

**Request runtime permission** (Android 6+):
```java
if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) 
    != PackageManager.PERMISSION_GRANTED) {
    ActivityCompat.requestPermissions(this, 
        new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_CODE);
}
```

### ❌ Storage Permission Issues

#### Problema: "Cannot write to external storage"

**Para Android 13+ (API 33+)**:
```xml
<uses-permission android:name="android.permission.READ_MEDIA_AUDIO" />
<uses-permission android:name="android.permission.READ_MEDIA_VIDEO" />
<uses-permission android:name="android.permission.READ_MEDIA_IMAGES" />
```

**Para Android 11-12 (API 30-32)**:
```xml
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.MANAGE_EXTERNAL_STORAGE" />
```

## 🔧 Ambiente de Desenvolvimento

### ❌ Android Studio Issues

#### Problema: "Sync failed" após atualização
**Solução**:
```bash
# Invalidate caches
File → Invalidate Caches and Restart

# Clean projeto
./gradlew clean

# Rebuild
Build → Rebuild Project
```

#### Problema: "NDK not configured"
**Solução**:
```bash
# Instalar NDK via SDK Manager
Tools → SDK Manager → SDK Tools → NDK

# Ou via linha de comando
sdkmanager --install "ndk;23.1.7779620"

# Verificar local.properties
ndk.dir=/path/to/Android/Sdk/ndk/23.1.7779620
```

### ❌ JDK Version Issues

#### Problema: Multiple JDK versions
**Verificar versões**:
```bash
java -version
javac -version
update-alternatives --list java
```

**Configurar versão específica**:
```bash
# Temporário
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Permanente (add to ~/.bashrc)
echo 'export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64' >> ~/.bashrc
```

## 📱 Problemas de Dispositivo

### ❌ Performance Issues

#### Problema: App muito lento
**Diagnóstico**:
```bash
# CPU/Memory usage
adb shell top | grep com.sdrradio

# Memory profiling
adb shell dumpsys meminfo com.sdrradio.app

# Battery usage
adb shell dumpsys batterystats | grep com.sdrradio
```

**Otimizações**:
1. **Reduzir sample rate** quando possível
2. **Usar threading** para processamento pesado
3. **Implementar pooling** de objetos
4. **Otimizar UI updates** (60fps máximo)

#### Problema: Battery drain excessivo
**Soluções**:
1. **Background processing limits**:
```xml
<uses-permission android:name="android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS" />
```

2. **Foreground service** para processamento contínuo:
```java
startForegroundService(new Intent(this, SDRService.class));
```

### ❌ Compatibility Issues

#### Problema: App crash em dispositivos específicos
**Diagnóstico**:
```bash
# Crash logs
adb logcat -b crash

# Device info
adb shell getprop ro.build.version.release
adb shell getprop ro.product.cpu.abi
```

**Soluções**:
1. **Multi-APK** para arquiteturas diferentes
2. **API level checks** no código
3. **Feature detection** antes de usar

## 🔍 Debug Tools

### Logs Específicos
```bash
# SDR logs
adb logcat -s "SDRRadio"

# USB logs
adb logcat -s "USB" -s "UsbManager"

# Audio logs
adb logcat -s "AudioManager" -s "AudioRecord"

# Performance logs
adb logcat -s "Choreographer"
```

### Profiling
```bash
# Memory dump
adb shell am dumpheap com.sdrradio.app /sdcard/dump.hprof

# CPU profiling
adb shell simpleperf record -p $(adb shell pidof com.sdrradio.app)

# Network profiling
adb shell tcpdump -i any -w /sdcard/capture.pcap
```

## 🆘 Getting Help

### Community Resources
- **GitHub Issues**: Para bugs específicos
- **GitHub Discussions**: Para perguntas gerais
- **StackOverflow**: Tag `rtl-sdr` + `android`
- **Reddit**: r/RTLSDR, r/AndroidDev

### Professional Support
- **Email**: Para questões comerciais/privadas
- **Consulting**: Desenvolvimento customizado
- **Training**: Workshops e treinamentos

### Reporting Bugs
Ao reportar bugs, inclua sempre:
- **Dispositivo** e versão Android
- **Projeto** específico (kotlin/java/cpp/react_native)
- **Passos** para reproduzir
- **Logs** relevantes
- **Hardware SDR** utilizado

---

**💡 Dica**: Mantenha sempre backups dos projetos funcionais antes de fazer atualizações grandes!
