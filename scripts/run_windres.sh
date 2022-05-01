#!/usr/bin/env bash

set -e
declare BASE_DIR
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-${0}}")"; pwd)"
readonly BASE_DIR
cd "${BASE_DIR}/.."

declare -i exit_code=0
declare -r TARGET="${TARGET:-i686-w64-mingw32}"
declare -r WINDRES="${WINDRES:-"${TARGET}-windres"}"

mkdir -p "obj/${TARGET}"

declare line _line
while IFS='' read -rd '' line; do
	printf '\033[1;34mFile: %s\033[m\n' "${line}"
	_line="${line#src/}"
	"${WINDRES}" -i"${line}" -o"obj/${TARGET}/${_line%.rc}.o" -I{src,res} || exit_code=1
done < <(find src -type f -name '*.rc' -print0)

exit "${exit_code}"
