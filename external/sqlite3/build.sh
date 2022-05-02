#!/usr/bin/env bash

set -e
declare BASE_DIR
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-${0}}")"; pwd)"
readonly BASE_DIR
cd "${BASE_DIR}"

declare -r TARGET="${TARGET:-i686-w64-mingw32}"
declare -r CC="${CC:-"${TARGET}-gcc"}"
declare -r SRC_SQLITE3_C="src/sqlite3.c"

if [[ ! -f "${SRC_SQLITE3_C}" ]]; then
	printf '\033[1;31mERROR\033[m The file "%s" does not exist or not a regular file\n' "${SRC_SQLITE3_C}" >&2
	printf '\033[1;36mHINT\033[m Download latest SQLite3 amalgamation from https://www.sqlite.org/download.html and extract it to src/\n'
	exit 1
fi

mkdir -p "lib/${TARGET}"

"${CC}" \
	"${SRC_SQLITE3_C}" \
	-o "lib/${TARGET}/sqlite3.dll" \
	-shared \
	-static-libgcc \
	-Wl,--out-implib,"lib/${TARGET}/sqlite3.lib" \
	-O3 -g0 -ggdb0
