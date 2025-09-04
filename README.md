# SDR Radio Android Apps 📻

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Android](https://img.shields.io/badge/Platform-Android-green.svg)](https://android.com)
[![Gradle](https://img.shields.io/badge/Gradle-9.0.0-brightgreen.svg)](https://gradle.org)
[![Java](https://img.shields.io/badge/Java-21-orange.svg)](https://openjdk.org)
[![Kotlin](https://img.shields.io/badge/Kotlin-2.0.20-purple.svg)](https://kotlinlang.org)

Um conjunto de aplicativos Android para **Software Defined Radio (SDR)** compatíveis com dispositivos **RTL-SDR** via USB. Este projeto oferece múltiplas implementações em diferentes linguagens e frameworks para máxima flexibilidade de desenvolvimento.

## 📱 Screenshots

<!-- Adicione screenshots aqui -->
| Home Screen | Spectrum Analyzer | Settings |
|-------------|-------------------|----------|
| ![Home](./screenshots/home.png) | ![Spectrum](./screenshots/spectrum.png) | ![Settings](./screenshots/settings.png) |

## 🏗️ Estrutura do Projeto

Este repositório contém **5 projetos Android** independentes:

```
radio_sdr/
├── java/           # Implementação em Java puro
├── java_2/         # Implementação Java com bibliotecas extras
├── kotlin/         # Implementação em Kotlin (recomendada)
├── cpp/            # Implementação com código nativo C++
└── react_native/   # Implementação React Native cross-platform
```

### 📂 Descrição dos Projetos

| Projeto | Linguagem | Características | Status |
|---------|-----------|-----------------|--------|
| **java** | Java 21 | Implementação básica, fácil de entender | ✅ Estável |
| **java_2** | Java 21 | Com bibliotecas USB e gráficos avançados | ✅ Estável |
| **kotlin** | Kotlin 2.0.20 | Moderno, conciso, com corrotinas | ✅ **Recomendado** |
| **cpp** | Java + C++ | Performance otimizada para DSP | ✅ Estável |
| **react_native** | Expo + React Native 0.74.5 | Cross-platform iOS/Android | ✅ **Funcionando** |

## 🔧 Pré-requisitos

### Ambiente de Desenvolvimento
- **Android Studio** Arctic Fox ou superior
- **JDK 21** (OpenJDK recomendado)
- **Android SDK** API 24-35
- **Gradle 9.0.0** (incluído via wrapper)

### Instalação do JDK 21 (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install openjdk-21-jdk
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
```

### Dispositivo de Teste
- **Android 7.0+** (API 24+)
- **USB OTG** suporte
- **RTL-SDR dongle** (opcional para testes completos)

## 📋 Características Principais

### ✅ Permissões Configuradas
- 🎙️ **Áudio**: Gravação e configurações de áudio
- 💾 **Armazenamento**: Acesso completo (incluindo Android 13+)
- 🔌 **USB**: Detecção automática de dispositivos RTL-SDR
- 📱 **Sistema**: Notificações e serviços em background

### ✅ Compatibilidade RTL-SDR
- Suporte para dispositivos USB RTL-SDR (Realtek RTL2832U)
- Detecção automática via Intent Filters
- Configuração de permissões USB

### ✅ Funcionalidades
- Análise de espectro em tempo real
- Gravação de sinais
- Interface de usuário intuitiva
- Configurações avançadas

## 🚀 Como Fazer Build

### 1. Projeto Java

```bash
cd java/
chmod +x gradlew
./gradlew clean
./gradlew assembleDebug
```

**APK gerado em:** `java/app/build/outputs/apk/debug/app-debug.apk`

### 2. Projeto Java_2 (com bibliotecas extras)

```bash
cd java_2/
chmod +x gradlew
./gradlew clean
./gradlew assembleDebug
```

**APK gerado em:** `java_2/app/build/outputs/apk/debug/app-debug.apk`

### 3. Projeto Kotlin (Recomendado)

```bash
cd kotlin/
chmod +x gradlew
./gradlew clean
./gradlew assembleDebug
```

**APK gerado em:** `kotlin/app/build/outputs/apk/debug/app-debug.apk`

### 4. Projeto C++ (Performance)

```bash
cd cpp/
chmod +x gradlew
./gradlew clean
./gradlew assembleDebug
```

**APK gerado em:** `cpp/app/build/outputs/apk/debug/app-debug.apk`

### 5. Projeto React Native (Cross-Platform) ✅ NOVO!

**Pré-requisitos:**
- Node.js 18+ e npm
- Expo CLI global
- Expo Go app no dispositivo móvel

```bash
cd react_native/

# Instalar dependências
npm install

# Instalar Expo CLI (se não tiver)
npm install -g @expo/cli

# Iniciar servidor Expo
npm start

# 📱 Escaneie o QR code com:
# - Android: Expo Go app
# - iOS: Camera app nativa

# Para build nativo (opcional):
expo build:android  # Gera APK
expo build:ios      # Gera IPA
```

**Servidor rodando em:** `exp://192.168.x.x:8081`
**APK nativo:** `react_native/build/outputs/apk/`

> ✅ **Sucesso!** Projeto Expo funcionando perfeitamente com QR code para teste instant.

## 📦 Como Instalar

### Método 1: Via ADB (Recomendado)

1. **Habilite Depuração USB** no dispositivo Android
2. **Conecte o dispositivo** ao computador
3. **Instale via Gradle**:

```bash
# Para cada projeto, dentro da pasta correspondente:
./gradlew installDebug
```

### Método 2: Instalação Manual

1. **Copie o APK** para o dispositivo
2. **Habilite "Fontes Desconhecidas"** nas configurações
3. **Toque no APK** e instale

### Método 3: Via ADB Manual

```bash
# Verifique dispositivos conectados
adb devices

# Instale o APK específico
adb install java/app/build/outputs/apk/debug/app-debug.apk
# ou
adb install kotlin/app/build/outputs/apk/debug/app-debug.apk
# ou
adb install cpp/app/build/outputs/apk/debug/app-debug.apk
# ou
adb install java_2/app/build/outputs/apk/debug/app-debug.apk
```

## 🛠️ Desenvolvimento

### Comandos Úteis

```bash
# Limpar todos os builds
./gradlew clean

# Build de release (para produção)
./gradlew assembleRelease

# Executar testes
./gradlew test

# Verificar dependências
./gradlew dependencies

# Gerar relatório de build
./gradlew build --scan
```

### Estrutura de Dependências

Cada projeto utiliza:
- **Android Gradle Plugin**: 8.8.0
- **Gradle**: 9.0.0
- **Java**: 21
- **Kotlin**: 2.0.20 (projetos Kotlin)
- **Android SDK**: 24-35

## 📱 Permissões Solicitadas

```xml
<!-- Áudio -->
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />

<!-- Armazenamento -->
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.MANAGE_EXTERNAL_STORAGE" />

<!-- Android 13+ Media granular -->
<uses-permission android:name="android.permission.READ_MEDIA_AUDIO" />
<uses-permission android:name="android.permission.READ_MEDIA_VIDEO" />
<uses-permission android:name="android.permission.READ_MEDIA_IMAGES" />

<!-- USB -->
<uses-permission android:name="android.permission.USB_PERMISSION" />

<!-- Sistema -->
<uses-permission android:name="android.permission.POST_NOTIFICATIONS" />
```

## 🔌 Dispositivos RTL-SDR Suportados

O aplicativo detecta automaticamente os seguintes dispositivos:

| Vendor ID | Product ID | Descrição |
|-----------|------------|-----------|
| 0x0bda | 0x2838 | RTL2838 DVB-T |
| 0x0bda | 0x2832 | RTL2832U DVB-T |
| 0x0bda | 0x2831 | RTL2831U DVB-T |
| 0x0bda | 0x2830 | RTL2830 DVB-T |

## 🐛 Solução de Problemas

### Build Failures

**Erro de permissão:**
```bash
chmod +x gradlew
```

**JDK incorreto:**
```bash
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
```

**Gradle não baixa:**
```bash
./gradlew wrapper --gradle-version=9.0.0
```

### Problemas de Instalação

**Dispositivo não encontrado:**
```bash
adb devices
# Se vazio, habilite Depuração USB
```

**Permissão negada:**
```bash
adb kill-server
sudo adb start-server
```

## 📖 Documentação Adicional

- [Manual Completo](MANUAL_COMPLETO.md)
- [Guia RTL-SDR](MANUAL_RTL_SDR_CPP.md)
- [Configuração Build](MANUAL_BUILD_GRADLE.md)

## 🤝 Contribuindo

1. **Fork** o projeto
2. **Crie** uma branch para sua feature (`git checkout -b feature/nova-feature`)
3. **Commit** suas mudanças (`git commit -am 'Adiciona nova feature'`)
4. **Push** para a branch (`git push origin feature/nova-feature`)
5. **Abra** um Pull Request

### Diretrizes de Contribuição

- Mantenha o código limpo e comentado
- Siga as convenções de nomenclatura
- Adicione testes quando apropriado
- Atualize a documentação

## � Troubleshooting

Para problemas comuns de build, runtime e configuração, consulte nosso guia detalhado:

### 📋 Documentos de Ajuda
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Soluções para problemas comuns
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Como contribuir para o projeto
- **[PROJECT_CONFIG.md](PROJECT_CONFIG.md)** - Configurações técnicas detalhadas
- **[ROADMAP.md](ROADMAP.md)** - Planos futuros do projeto

### 🚨 Problemas Conhecidos
- **React Native**: Em configuração, problemas de compatibilidade Gradle/Kotlin
- **USB OTG**: Requer dispositivo físico, não funciona em emuladores
- **Permissões**: Android 13+ requer permissões granulares de mídia

### 💬 Suporte
- **GitHub Issues**: Para bugs e feature requests
- **GitHub Discussions**: Para perguntas e discussões
- **Email**: Para questões comerciais

## �📄 Licença

Este projeto está licenciado sob a **MIT License** - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👥 Autores

- **[@reinanbr](https://github.com/reinanbr)** - Desenvolvimento principal

## 🙏 Agradecimentos

- Comunidade **RTL-SDR**
- Desenvolvedores **Android Open Source**
- Contribuidores do projeto

## 📊 Status do Build

| Projeto | Status | APK | Tamanho |
|---------|--------|-----|---------|
| Java | ![Build](https://img.shields.io/badge/build-passing-brightgreen) | ✅ | ~8MB |
| Java_2 | ![Build](https://img.shields.io/badge/build-passing-brightgreen) | ✅ | ~12MB |
| Kotlin | ![Build](https://img.shields.io/badge/build-passing-brightgreen) | ✅ | ~10MB |
| C++ | ![Build](https://img.shields.io/badge/build-passing-brightgreen) | ✅ | ~15MB |

---

**⭐ Se este projeto foi útil, considere dar uma estrela!**

**🐛 Encontrou um bug? [Abra uma issue](https://github.com/reinanbr/radio_sdr_app/issues)**

**💡 Tem uma ideia? [Contribua conosco](https://github.com/reinanbr/radio_sdr_app/pulls)**
