#!/bin/bash

echo "Instalando SDR Radio KT no dispositivo Android..."

# Verificar se o ADB está disponível
if ! command -v adb &> /dev/null; then
    echo "Erro: ADB não encontrado. Certifique-se de que o Android SDK está instalado e no PATH."
    exit 1
fi

# Verificar se há dispositivos conectados
if ! adb devices | grep -q "device$"; then
    echo "Erro: Nenhum dispositivo Android conectado."
    echo "Certifique-se de que:"
    echo "1. O dispositivo está conectado via USB"
    echo "2. A depuração USB está habilitada"
    echo "3. O dispositivo está autorizado"
    exit 1
fi

# Instalar o APK
echo "Instalando APK..."
adb install -r app/build/outputs/apk/debug/app-debug.apk

if [ $? -eq 0 ]; then
    echo "Instalação concluída com sucesso!"
    echo "O app 'SDR Radio KT' foi instalado no dispositivo."
    echo ""
    echo "Para abrir o app, execute:"
    echo "adb shell am start -n com.sdrradio.kt/.MainActivity"
    echo ""
    echo "Ou procure por 'SDR Radio KT' no launcher do dispositivo."
else
    echo "Erro na instalação. Verifique se:"
    echo "1. O dispositivo tem espaço suficiente"
    echo "2. A instalação de apps de fontes desconhecidas está habilitada"
    echo "3. O dispositivo não tem um app com o mesmo package name"
fi 