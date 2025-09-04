---
name: 🐛 Bug Report
about: Reporte um bug para nos ajudar a melhorar
title: '[BUG] '
labels: ['bug', 'needs-triage']
assignees: ''
---

## 📱 Informações do Dispositivo

**Dispositivo:** (ex: Samsung Galaxy S21, Xiaomi Redmi Note 10)
**Android:** (ex: Android 13, API 33)
**Arquitetura:** (ex: arm64-v8a, armeabi-v7a)
**RAM:** (ex: 6GB)

## 📻 Hardware SDR

**Dongle RTL-SDR:** (ex: RTL2832U, modelo específico)
**USB OTG:** ✅ Sim / ❌ Não
**Cabo/Adaptador:** (descrição do cabo usado)
**Antena:** (tipo de antena conectada)

## 🏗️ Versão do App

**Projeto:** (ex: kotlin, java, java_2, cpp)
**Versão/Commit:** (ex: v1.0.0, commit hash)
**APK:** Debug / Release
**Build Date:** (quando foi compilado)

## 🐛 Descrição do Bug

Uma descrição clara e concisa do que está acontecendo.

## 🔄 Passos para Reproduzir

1. Vá para '...'
2. Clique em '...'
3. Role até '...'
4. Veja o erro

## ✅ Comportamento Esperado

Uma descrição clara do que você esperava que acontecesse.

## ❌ Comportamento Atual

O que realmente acontece (incluindo mensagens de erro).

## 📱 Screenshots/Vídeos

Se aplicável, adicione screenshots ou vídeos para explicar o problema.

## 📋 Logs

<details>
<summary>📋 Clique para expandir logs</summary>

```
Cole aqui os logs do logcat:
adb logcat -s "SDRRadio"

Ou logs do app, se disponíveis.
```

</details>

## 🔧 Informações Adicionais

### Permissões
- [ ] RECORD_AUDIO concedida
- [ ] Acesso a arquivos concedido  
- [ ] USB_PERMISSION concedida
- [ ] Permissões solicitadas corretamente

### USB/RTL-SDR
- [ ] Dispositivo USB detectado pelo Android
- [ ] RTL-SDR aparece em lsusb (se acessível)
- [ ] LED do dongle acende quando conectado
- [ ] Testado em outros apps SDR

### App
- [ ] Primeira instalação
- [ ] Atualização de versão anterior
- [ ] Instalação limpa (dados limpos)
- [ ] Testado em outros dispositivos

## 🧪 Testes Realizados

**Tentativas de solução:**
- [ ] Reiniciei o app
- [ ] Reconectei o USB
- [ ] Reiniciei o dispositivo
- [ ] Testei outro dongle RTL-SDR
- [ ] Concedi permissões manualmente

**Outros apps testados:**
- [ ] SDR Touch - ✅ Funciona / ❌ Não funciona
- [ ] RF Analyzer - ✅ Funciona / ❌ Não funciona
- [ ] Outros: _______________

## 🔍 Contexto Adicional

Adicione qualquer outro contexto sobre o problema aqui.

### 🎯 Frequência/Sinal
Se o bug está relacionado a recepção:
- **Frequência:** (ex: 88.5 MHz, 144.800 MHz)
- **Tipo de sinal:** (ex: FM, AM, digital)
- **Intensidade:** (ex: sinal forte local, sinal fraco)
- **Localização:** (ambiente urbano/rural)

### ⚡ Performance
Se o bug afeta performance:
- **CPU/RAM durante uso:** (%)
- **Temperatura do dispositivo:** (normal/quente)
- **Duração até o problema:** (imediato/após X minutos)

---

**Checkist do Reporter:**
- [ ] Testei na versão mais recente
- [ ] Procurei por issues similares
- [ ] Incluí todas as informações necessárias
- [ ] Adicionei logs relevantes
