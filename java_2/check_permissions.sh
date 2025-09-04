#!/bin/bash

# Script para verificar permissões do aplicativo SDR Radio
# Uso: ./check_permissions.sh

echo "=== Verificando permissões do SDR Radio ==="
echo "Data/Hora: $(date)"
echo ""

# Verificar se o aplicativo está instalado
echo "Verificando se o aplicativo está instalado..."
if adb shell pm list packages | grep -q "com.sdrradio.app"; then
    echo "✅ Aplicativo SDR Radio está instalado"
else
    echo "❌ Aplicativo SDR Radio NÃO está instalado"
    exit 1
fi

echo ""

# Verificar permissões concedidas
echo "Permissões concedidas ao aplicativo:"
adb shell dumpsys package com.sdrradio.app | grep -E "(permission|granted)" | head -20

echo ""

# Verificar permissões específicas
echo "Verificando permissões específicas:"
permissions=(
    "android.permission.USB_PERMISSION"
    "android.permission.RECORD_AUDIO"
    "android.permission.WRITE_EXTERNAL_STORAGE"
    "android.permission.READ_EXTERNAL_STORAGE"
    "android.permission.INTERNET"
    "android.permission.ACCESS_NETWORK_STATE"
    "android.permission.WAKE_LOCK"
    "android.permission.FOREGROUND_SERVICE"
    "android.permission.POST_NOTIFICATIONS"
)

for permission in "${permissions[@]}"; do
    result=$(adb shell dumpsys package com.sdrradio.app | grep -A 1 -B 1 "$permission" | grep -E "(granted|denied|requested)" || echo "Não encontrada")
    echo "$permission: $result"
done

echo ""

# Verificar dispositivos USB conectados
echo "Dispositivos USB conectados:"
adb shell lsusb 2>/dev/null || echo "lsusb não disponível"

echo ""

# Verificar se o dispositivo tem suporte a USB Host
echo "Verificando suporte a USB Host:"
adb shell getprop ro.hardware.usb.host 2>/dev/null || echo "Propriedade não encontrada"

echo ""
echo "=== Verificação concluída ===" 