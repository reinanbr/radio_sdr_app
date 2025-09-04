# Radio SDR Android App

Um aplicativo Android completo para rádio SDR que funciona com dispositivos RTL-SDR via USB OTG, desenvolvido em Java/C++.

## Características Principais

### Interface do Usuário
- **Interface gráfica intuitiva** para controle de frequência
- **Sintonização precisa** com entrada direta e controle deslizante
- **Visualizador de espectro** em tempo real com FFT
- **Display de cascata (waterfall)** para análise temporal
- **Controles de volume** e mudo integrados

### Processamento de Sinal
- **Demodulação múltipla**: FM, AM, USB, LSB
- **Controle de ganho** automático e manual
- **Filtros digitais** configuráveis
- **Controle de squelch** para eliminar ruído
- **AGC (Automatic Gain Control)** para áudio limpo

### Conectividade Hardware
- **Suporte completo RTL-SDR** via USB OTG
- **Auto-detecção** de dispositivos compatíveis
- **Configuração automática** de parâmetros
- **Múltiplas taxas de amostragem** (250kHz - 3.2MHz)

### Recursos Avançados
- **Gravação de áudio** em tempo real
- **Correção de frequência** (PPM)
- **Configurações personalizáveis** 
- **Análise de espectro** com FFT otimizada
- **Processamento nativo** em C++ para performance máxima

## Arquitetura do Projeto

### Estrutura de Diretórios
```
radio_sdr/
├── app/
│   ├── src/main/
│   │   ├── java/com/radioSDR/app/     # Código Java
│   │   │   ├── MainActivity.java      # Activity principal
│   │   │   ├── SpectrumActivity.java  # Visualizador de espectro
│   │   │   └── SettingsActivity.java  # Configurações
│   │   ├── cpp/                       # Código nativo C++
│   │   │   ├── radiosdr_jni.cpp      # Interface JNI
│   │   │   ├── sdr_controller.cpp     # Controle RTL-SDR
│   │   │   ├── signal_processor.cpp   # Processamento DSP
│   │   │   ├── audio_processor.cpp    # Processamento áudio
│   │   │   ├── spectrum_analyzer.cpp  # Análise de espectro
│   │   │   ├── demodulator.cpp       # Demoduladores
│   │   │   └── librtlsdr/            # Biblioteca RTL-SDR
│   │   └── res/                      # Recursos Android
│   └── build.gradle                  # Configuração build
├── build.gradle                      # Build global
├── settings.gradle                   # Configurações projeto
└── README.md                        # Este arquivo
```

### Componentes C++ Nativos

#### SDRController (`sdr_controller.cpp`)
- Comunicação direta com hardware RTL-SDR
- Controle de frequência, ganho e taxa de amostragem
- Buffer circular para dados IQ
- Thread de leitura assíncrona

#### SignalProcessor (`signal_processor.cpp`)
- Filtros digitais (passa-baixa, passa-banda)
- Controle automático de ganho (AGC)
- Controle de squelch
- Preparação para demodulação

#### Demodulator (`demodulator.cpp`)
- **FM**: Demodulação por diferença de fase
- **AM**: Detecção de envelope
- **SSB (USB/LSB)**: Demodulação por produto
- Decimação para taxa de áudio (48kHz)

#### SpectrumAnalyzer (`spectrum_analyzer.cpp`)
- FFT otimizada para análise de espectro
- Janelamento (Hamming) para reduzir vazamento
- Averaging para suavização do espectro
- Buffer circular para display waterfall

#### AudioProcessor (`audio_processor.cpp`)
- Conversão float para PCM 16-bit
- Controle de volume digital
- Limitador para prevenir clipping
- Buffer de saída para AudioTrack

## Requisitos do Sistema

### Android
- **Android 6.0+** (API 23 ou superior)
- **Suporte USB OTG** (USB Host)
- **Permissão de áudio** para gravação
- **Pelo menos 2GB RAM** para processamento eficiente

### Hardware RTL-SDR
- **Chipset RTL2832U** com tuner R820T/R820T2
- **Cabo USB OTG** compatível
- **Dispositivos testados**:
  - RTL-SDR Blog V3/V4
  - NooElec NESDR Smart/Mini
  - Generic RTL2832U dongles

### Desenvolvimento
- **Android Studio** 2022.3+
- **NDK** 25.0+ instalado
- **CMake** 3.18.1+
- **SDK** Android 33/34

## Compilação e Instalação

### Pré-requisitos
```bash
# Configurar Android SDK
export ANDROID_HOME=/caminho/para/android-sdk
export PATH=$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools

# Verificar NDK
ls $ANDROID_HOME/ndk/
```

### Compilação Automática
```bash
# Dar permissão ao script
chmod +x build.sh

# Executar build
./build.sh
```

### Compilação Manual
```bash
# Limpar projeto
./gradlew clean

# Compilar debug
./gradlew assembleDebug

# Ou compilar release
./gradlew assembleRelease
```

### Instalação
```bash
# Via ADB
adb install app/build/outputs/apk/debug/app-debug.apk

# Ou transferir APK para dispositivo e instalar manualmente
```

## Uso Detalhado

### Primeira Configuração
1. **Conectar RTL-SDR** via cabo USB OTG
2. **Abrir aplicativo** - aparecerá como "Radio SDR"
3. **Conceder permissões**:
   - Permissão USB quando solicitada
   - Permissão de áudio para gravação
4. **Tocar "Conectar Dispositivo"**
5. **Aguardar status** mudar para "Dispositivo Conectado"

### Operação Básica
1. **Sintonizar frequência**:
   - Digite frequência em MHz no campo de texto
   - Ou use o controle deslizante
   - Exemplo: 88.5 para FM local

2. **Configurar ganho**:
   - Use "Ganho Automático" para operação normal
   - Ou desative e ajuste manualmente

3. **Escolher demodulação**:
   - **FM**: Para rádio FM comercial
   - **AM**: Para rádio AM e aviação civil
   - **USB/LSB**: Para radioamador e utilities

4. **Iniciar recepção**:
   - Toque "Iniciar SDR"
   - Aguarde o áudio começar
   - Ajuste volume conforme necessário

### Recursos Avançados

#### Visualizador de Espectro
- Toque "Espectro" para tela dedicada
- **Gráfico superior**: Espectro de frequência atual
- **Gráfico inferior**: Display waterfall temporal
- Use gestos para zoom e pan

#### Configurações
- Toque "Configurações" para opções avançadas:
  - **Taxa de amostragem**: 250kHz a 3.2MHz
  - **Correção de frequência**: -200 a +200 PPM
  - **Largura de banda**: 1kHz a 250kHz
  - **Squelch**: -100dB a 0dB

#### Gravação
- Toque "Gravar" durante recepção
- Áudio salvo em formato WAV
- Toque novamente para parar gravação

## Solução de Problemas

### Dispositivo não conecta
- Verificar cabo USB OTG
- Tentar cabo USB diferente
- Reiniciar aplicativo
- Verificar se RTL-SDR funciona no PC

### Sem áudio
- Verificar volume do dispositivo
- Verificar se não está mudo
- Tentar frequência conhecida (FM local)
- Ajustar ganho

### Performance ruim
- Fechar outros aplicativos
- Reduzir taxa de amostragem
- Dispositivo pode estar aquecendo

### Erro de compilação
- Verificar NDK instalado
- Atualizar Android Studio
- Limpar projeto: `./gradlew clean`
- Verificar espaço em disco

## Desenvolvimento

### Adicionando novos demoduladores
1. Editar `demodulator.h` - adicionar enum
2. Implementar em `demodulator.cpp`
3. Atualizar UI em `MainActivity.java`

### Otimizações de performance
- Usar NEON SIMD quando disponível
- Implementar FFT otimizada (FFTW)
- Cache de filtros pré-calculados

### Recursos futuros planejados
- Suporte a múltiplos dongles
- Gravação IQ raw
- Plugins de demodulação
- Interface web remota
- Suporte HackRF/PlutoSDR

## Licença

Este projeto é open source. Consulte bibliotecas individuais para suas licenças específicas.

## Contribuição

Contribuições são bem-vindas! Por favor:
1. Fork o projeto
2. Crie branch para feature
3. Commit suas mudanças
4. Push para branch
5. Abra Pull Request

## Contato

Para dúvidas, problemas ou sugestões, abra uma issue no repositório do projeto.
