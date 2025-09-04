#!/bin/bash

# Script para capturar logs detalhados do aplicativo SDR Radio
# Uso: ./capture_detailed_logs.sh

echo "=== Capturando logs detalhados do SDR Radio ==="
echo "Data/Hora: $(date)"
echo ""

# Limpar logs antigos
adb logcat -c

echo "Logs limpos. Agora siga estes passos:"
echo "1. Conecte o dispositivo RTL-SDR via USB OTG"
echo "2. Abra o aplicativo SDR Radio"
echo "3. Tente conectar o dispositivo no aplicativo"
echo "4. Pressione Ctrl+C para parar a captura"
echo ""

# Capturar logs detalhados
adb logcat | grep -E "(MainActivity|RTLSDROperations|USB|device|connect|error|fail|FATAL|AndroidRuntime|Toast)" > sdr_detailed_logs.txt

echo ""
echo "Logs detalhados salvos em: sdr_detailed_logs.txt"
echo "Total de linhas capturadas: $(wc -l < sdr_detailed_logs.txt)"
echo ""
echo "Últimas 20 linhas dos logs:"
echo "=========================="
tail -20 sdr_detailed_logs.txt 