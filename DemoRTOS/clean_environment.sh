#!/usr/bin/env bash
# ============================================================
# clean_environment.sh — Limpieza profunda del entorno local
# Elimina artefactos de build y dependencias descargadas.
# ============================================================

set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

GREEN='\033[0;32m'; CYAN='\033[0;36m'; RESET='\033[0m'
info()  { echo -e "${CYAN}[clean]${RESET} $*"; }
ok()    { echo -e "${GREEN}[  ok ]${RESET} $*"; }

info "Eliminando build y artefactos..."
rm -rf build
rm -f freertos_rps.elf freertos_rps.bin output.map
find . -type f \( -name '*.o' -o -name '*.obj' -o -name '*.d' \) -delete

info "Eliminando dependencias descargadas..."
rm -rf FreeRTOS-Kernel FreeRTOS-Demo demo_support

ok "Limpieza completa terminada."
echo ""
echo "Para restaurar todo y recompilar:"
echo "  chmod +x setup.sh && ./setup.sh"
