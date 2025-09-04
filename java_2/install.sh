#!/bin/bash

# SDR Radio Android - Script de Instalação Rápida
# Este script automatiza o processo de build e instalação

set -e  # Para o script se houver erro

echo "📻 SDR Radio Android - Instalação Rápida"
echo "========================================"

# Verificar se o ADB está disponível
if ! command -v adb &> /dev/null; then
    echo "❌ ADB não encontrado. Instale o Android SDK primeiro."
    exit 1
fi

# Verificar se há dispositivos conectados
echo "🔍 Verificando dispositivos Android..."
if ! adb devices | grep -q "device$"; then
    echo "❌ Nenhum dispositivo Android conectado."
    echo "   Conecte um dispositivo e habilite a depuração USB."
    exit 1
fi

echo "✅ Dispositivo Android detectado:"
adb devices | grep "device$"

# Verificar se o Gradle 7.6 está disponível
if [ ! -d "gradle-7.6" ]; then
    echo "📥 Baixando Gradle 7.6..."
    wget https://services.gradle.org/distributions/gradle-7.6-bin.zip
    unzip gradle-7.6-bin.zip
    rm gradle-7.6-bin.zip
fi

# Verificar se o wrapper JAR existe
if [ ! -f "gradle/wrapper/gradle-wrapper.jar" ]; then
    echo "🔧 Configurando Gradle wrapper..."
    mkdir -p gradle/wrapper
    cp gradle-7.6/lib/plugins/gradle-wrapper-7.6.jar gradle/wrapper/gradle-wrapper.jar
fi

# Limpar build anterior
echo "🧹 Limpando build anterior..."
./gradle-7.6/bin/gradle clean

# Build do projeto
echo "🔨 Compilando projeto..."
./gradle-7.6/bin/gradle assembleDebug

# Verificar se o APK foi gerado
if [ ! -f "app/build/outputs/apk/debug/app-debug.apk" ]; then
    echo "❌ Erro: APK não foi gerado."
    exit 1
fi

echo "✅ APK gerado com sucesso!"

# Instalar no dispositivo
echo "📱 Instalando no dispositivo..."
./gradle-7.6/bin/gradle installDebug

# Verificar instalação
if adb shell pm list packages | grep -q "com.sdrradio.app"; then
    echo "✅ Aplicativo instalado com sucesso!"
    
    # Perguntar se quer abrir o app
    read -p "🚀 Deseja abrir o aplicativo agora? (s/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Ss]$ ]]; then
        echo "🔓 Abrindo SDR Radio..."
        adb shell am start -n com.sdrradio.app/.MainActivity
    fi
else
    echo "❌ Erro: Aplicativo não foi instalado corretamente."
    exit 1
fi

echo ""
echo "🎉 Instalação concluída com sucesso!"
echo ""
echo "📋 Próximos passos:"
echo "1. Conecte um RTL-SDR via cabo OTG"
echo "2. Abra o aplicativo SDR Radio"
echo "3. Conceda as permissões necessárias"
echo "4. Clique em 'Conectar Dispositivo'"
echo "5. Use o slider para sintonizar frequências"
echo ""
echo "📚 Para mais informações, consulte o MANUAL_COMPLETO.md" 