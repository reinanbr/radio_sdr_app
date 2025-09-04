#!/bin/bash

# Script de compilação para o RadioSDR Android App
# Executar a partir do diretório raiz do projeto

echo "==================================="
echo "Radio SDR - Script de Compilação"
echo "==================================="

# Verifica se o Android SDK está configurado
if [ -z "$ANDROID_HOME" ]; then
    echo "Erro: ANDROID_HOME não está definido"
    echo "Configure o caminho do Android SDK:"
    echo "export ANDROID_HOME=/caminho/para/android-sdk"
    exit 1
fi

# Verifica se o NDK está disponível
if [ ! -d "$ANDROID_HOME/ndk" ]; then
    echo "Erro: Android NDK não encontrado em $ANDROID_HOME/ndk"
    echo "Instale o NDK através do Android Studio ou SDK Manager"
    exit 1
fi

echo "Android SDK: $ANDROID_HOME"
echo "NDK encontrado: $(ls $ANDROID_HOME/ndk/ | head -1)"

# Limpa builds anteriores
echo ""
echo "Limpando builds anteriores..."
./gradlew clean

# Compila o projeto
echo ""
echo "Compilando projeto..."
./gradlew assembleDebug

if [ $? -eq 0 ]; then
    echo ""
    echo "==================================="
    echo "✅ Compilação concluída com sucesso!"
    echo "==================================="
    echo ""
    echo "APK gerado em:"
    echo "app/build/outputs/apk/debug/app-debug.apk"
    echo ""
    echo "Para instalar no dispositivo:"
    echo "adb install app/build/outputs/apk/debug/app-debug.apk"
    echo ""
else
    echo ""
    echo "==================================="
    echo "❌ Erro na compilação!"
    echo "==================================="
    echo ""
    echo "Verifique as mensagens de erro acima."
    echo "Possíveis soluções:"
    echo "1. Verifique se todas as dependências estão instaladas"
    echo "2. Verifique se o NDK está corretamente configurado"
    echo "3. Execute './gradlew --stacktrace assembleDebug' para mais detalhes"
    exit 1
fi
