#!/bin/bash

# Script para capturar logs do aplicativo SDR Radio
# Uso: ./capture_logs.sh

echo "=== Capturando logs do SDR Radio ==="
echo "Data/Hora: $(date)"
echo ""

# Limpar logs antigos
adb logcat -c

echo "Logs limpos. Agora conecte o dispositivo RTL-SDR e teste o aplicativo."
echo "Pressione Ctrl+C para parar a captura quando terminar o teste."
echo ""

# Capturar logs em tempo real e salvar em arquivo
adb logcat | grep -E "(MainActivity|RTLSDROperations|USB|device|connect|error|fail|FATAL|AndroidRuntime)" > sdr_radio_logs.txt

echo ""
echo "Logs salvos em: sdr_radio_logs.txt"
echo "Total de linhas capturadas: $(wc -l < sdr_radio_logs.txt)" 