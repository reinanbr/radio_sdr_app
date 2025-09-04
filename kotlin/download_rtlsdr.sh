#!/bin/bash

echo "Baixando RTL-SDR..."

# Criar diretório para RTL-SDR
mkdir -p app/src/main/cpp/rtl-sdr

# Baixar RTL-SDR do repositório oficial
wget -O rtl-sdr.tar.gz https://github.com/osmocom/rtl-sdr/archive/refs/heads/master.tar.gz

# Extrair arquivos
tar -xzf rtl-sdr.tar.gz --strip-components=1 -C app/src/main/cpp/rtl-sdr

# Limpar arquivo temporário
rm rtl-sdr.tar.gz

echo "RTL-SDR baixado com sucesso!"
echo "Diretório: app/src/main/cpp/rtl-sdr" 