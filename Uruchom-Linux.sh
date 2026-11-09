#!/bin/sh
# Linux compatibility launcher. The game itself remains a Windows x64 build.
set -eu
task_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ ! -f "$task_dir/Rdzen.exe" ]; then
    printf '%s\n' 'Brak Rdzen.exe. Rozpakuj cala paczke ZIP do jednego folderu.' >&2
    exit 1
fi
case "$(uname -m)" in
    x86_64|amd64) ;;
    *) printf '%s\n' 'Ta paczka wymaga Linux x86_64 i Wine (lub Steam Proton).' >&2; exit 1 ;;
esac
if [ -n "${RDZEN_WINE:-}" ]; then
    task_wine=$RDZEN_WINE
elif command -v wine >/dev/null 2>&1; then
    task_wine=wine
elif command -v wine64 >/dev/null 2>&1; then
    task_wine=wine64
else
    printf '%s\n' 'Nie znaleziono Wine. Zainstaluj Wine 64-bit dla swojej dystrybucji.' \
        'Alternatywa: dodaj Rdzen.exe jako gre spoza Steam i wlacz Steam Play / Proton.' \
        'Szczegoly: LINUX.txt. To nie jest natywna wersja Linux.' >&2
    exit 127
fi
if ! command -v "$task_wine" >/dev/null 2>&1; then
    printf '%s\n' "Nie znaleziono programu: $task_wine" >&2
    exit 127
fi
# Keep game-specific Wine data separate; honor an explicitly selected prefix.
if [ -z "${WINEPREFIX:-}" ]; then
    WINEPREFIX="${XDG_DATA_HOME:-$HOME/.local/share}/rdzen/wine"
    export WINEPREFIX
fi
case "$WINEPREFIX" in
    /*) ;;
    *) printf '%s\n' 'WINEPREFIX musi byc sciezka absolutna.' >&2; exit 1 ;;
esac
mkdir -p -- "$WINEPREFIX"
cd -- "$task_dir"
printf '%s\n' 'Uruchamianie przez Wine. Pierwszy start moze potrwac dluzej.'
exec "$task_wine" "$task_dir/Rdzen.exe" "$@"
