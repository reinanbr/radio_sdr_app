# SDR Radio Android

Um aplicativo Android em C++ que permite conectar um RTL-SDR via OTG e reproduzir as frequências captadas como rádio, com controle completo de frequência, ganho e demodulação.

## Características

- **Conexão USB OTG**: Suporte completo para dispositivos RTL-SDR via USB OTG
- **Controle de Frequência**: Ajuste de frequência de 24 MHz a 1766 MHz
- **Controle de Ganho**: Ganho manual ou automático ajustável
- **Demodulação**: Suporte para AM e FM
- **Interface Moderna**: UI Material Design 3 com controles intuitivos
- **Processamento de Áudio**: Filtros, AGC e redução de ruído
- **Monitoramento em Tempo Real**: Indicador de força do sinal
- **Multithreading**: Processamento assíncrono para performance otimizada

## Requisitos

### Hardware
- Dispositivo Android com suporte a USB OTG
- RTL-SDR (RTL2838) compatível
- Cabo USB OTG

### Software
- Android 5.0 (API 21) ou superior
- Android NDK r25.1.8937393 ou superior
- Android SDK 34
- CMake 3.22.1 ou superior

## Instalação

### 1. Clone o repositório
```bash
git clone https://github.com/seu-usuario/sdr-radio-android.git
cd sdr-radio-android
```

### 2. Configure o ambiente de desenvolvimento
- Instale o Android Studio
- Instale o Android NDK
- Configure as variáveis de ambiente ANDROID_HOME e ANDROID_NDK_HOME

### 3. Compile o projeto
```bash
./gradlew assembleDebug
```

### 4. Instale no dispositivo
```bash
./gradlew installDebug
```

## Uso

### 1. Conectar o RTL-SDR
- Conecte o RTL-SDR ao dispositivo Android via cabo USB OTG
- O aplicativo detectará automaticamente o dispositivo

### 2. Configurar parâmetros
- **Frequência**: Use o slider ou campo de texto para definir a frequência desejada
- **Taxa de Amostragem**: Configure a taxa de amostragem (padrão: 2.048 MHz)
- **Ganho**: Escolha entre ganho automático ou manual
- **Tipo de Demodulação**: AM ou FM

### 3. Iniciar o rádio
- Clique em "Iniciar Rádio" para começar a recepção
- O áudio será reproduzido através dos alto-falantes do dispositivo

### 4. Monitorar o sinal
- Observe o indicador de força do sinal em tempo real
- Ajuste os parâmetros conforme necessário para melhor recepção

## Arquitetura

### Componentes Principais

#### C++ (NDK)
- **SDRRadio**: Classe principal que coordena todos os componentes
- **SDRManager**: Gerencia o dispositivo RTL-SDR
- **AudioManager**: Controla a reprodução de áudio via OpenSL ES
- **USBManager**: Gerencia a conexão USB e hotplug
- **AudioProcessor**: Processa dados I/Q e aplica demodulação

#### Java/Kotlin
- **MainActivity**: Interface do usuário principal
- **SDRRadio**: Wrapper Java para o código C++

### Fluxo de Dados
```
RTL-SDR → USBManager → SDRManager → AudioProcessor → AudioManager → Speakers
```

## Configuração Avançada

### Parâmetros de Compilação
```gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64'
        }
    }
    
    externalNativeBuild {
        cmake {
            arguments "-DANDROID_STL=c++_shared"
        }
    }
}
```

### Permissões Necessárias
```xml
<uses-permission android:name="android.permission.USB_PERMISSION" />
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />
<uses-feature android:name="android.hardware.usb.host" android:required="true" />
```

## Personalização

### Adicionar Novos Tipos de Demodulação
1. Implemente a demodulação no `AudioProcessor`
2. Adicione a interface no `SDRManager`
3. Atualize a UI em `MainActivity`

### Suporte a Outros Dispositivos SDR
1. Implemente o driver no `SDRManager`
2. Adicione os IDs USB no `device_filter.xml`
3. Atualize a detecção no `USBManager`

## Solução de Problemas

### Dispositivo não detectado
- Verifique se o cabo USB OTG está funcionando
- Confirme se o dispositivo suporta USB OTG
- Verifique as permissões USB

### Sem áudio
- Verifique se o volume está ligado
- Confirme se as permissões de áudio foram concedidas
- Verifique se o RTL-SDR está funcionando corretamente

### Performance ruim
- Reduza a taxa de amostragem
- Ajuste o ganho para reduzir interferência
- Feche outros aplicativos em background

## Contribuição

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Agradecimentos

- [RTL-SDR](https://www.rtl-sdr.com/) - Hardware e documentação
- [librtlsdr](https://github.com/librtlsdr/librtlsdr) - Biblioteca RTL-SDR
- [OpenSL ES](https://www.khronos.org/opensles/) - API de áudio
- [Material Design](https://material.io/) - Design system

## Suporte

Para suporte, abra uma issue no GitHub ou entre em contato através do email: suporte@sdr-radio-android.com

## Roadmap

- [ ] Suporte a gravação de áudio
- [ ] Analisador de espectro em tempo real
- [ ] Suporte a múltiplos dispositivos SDR
- [ ] Modos de demodulação adicionais (SSB, CW)
- [ ] Interface de programação de frequências
- [ ] Suporte a streaming via rede
- [ ] Integração com bancos de dados de frequências 