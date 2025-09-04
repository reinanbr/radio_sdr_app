#!/bin/bash

# Script para conceder permissões necessárias ao aplicativo SDR Radio
# Uso: ./grant_permissions.sh

echo "=== Concedendo permissões ao SDR Radio ==="
echo "Data/Hora: $(date)"
echo ""

# Verificar se o aplicativo está instalado
if ! adb shell pm list packages | grep -q "com.sdrradio.app"; then
    echo "❌ Aplicativo SDR Radio não está instalado"
    exit 1
fi

echo "✅ Aplicativo SDR Radio está instalado"
echo ""

# Conceder permissões necessárias
echo "Concedendo permissões..."

# Permissões de armazenamento
echo "Concedendo permissão de leitura de armazenamento..."
adb shell pm grant com.sdrradio.app android.permission.READ_EXTERNAL_STORAGE

echo "Concedendo permissão de escrita de armazenamento..."
adb shell pm grant com.sdrradio.app android.permission.WRITE_EXTERNAL_STORAGE

# Permissão de notificações
echo "Concedendo permissão de notificações..."
adb shell pm grant com.sdrradio.app android.permission.POST_NOTIFICATIONS

# Permissão de áudio
echo "Concedendo permissão de gravação de áudio..."
adb shell pm grant com.sdrradio.app android.permission.RECORD_AUDIO

echo ""
echo "Permissões concedidas. Verificando status..."

# Verificar status das permissões
echo ""
echo "Status das permissões após concessão:"
adb shell dumpsys package com.sdrradio.app | grep -A 5 -B 5 "runtime permissions"

echo ""
echo "=== Concessão de permissões concluída ==="
echo ""
echo "Agora você pode testar o aplicativo. As permissões foram concedidas via ADB." 