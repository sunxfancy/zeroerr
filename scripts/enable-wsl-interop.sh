#!/bin/bash
# Persist WSL -> Windows .exe interop (needed for cmake.exe from Makefile).
# Run: sudo bash scripts/enable-wsl-interop.sh

set -euo pipefail

CONF_DIR=/etc/binfmt.d
CONF_FILE=$CONF_DIR/WSLInterop.conf
MAGIC=':WSLInterop:M::MZ::/init:PF'

mkdir -p "$CONF_DIR"
echo "$MAGIC" > "$CONF_FILE"
echo "Wrote $CONF_FILE"

# Register immediately for this session (ignore if already present).
if [[ ! -e /proc/sys/fs/binfmt_misc/WSLInterop ]]; then
  echo "$MAGIC" > /proc/sys/fs/binfmt_misc/register
  echo "Registered WSLInterop for current session"
else
  echo "WSLInterop already registered"
fi

ls -la /proc/sys/fs/binfmt_misc/WSLInterop
command -v cmake.exe >/dev/null && cmake.exe --version | head -1 || true
echo "Done. If cmake.exe still fails, run: wsl --shutdown  (from PowerShell) then reopen WSL."
