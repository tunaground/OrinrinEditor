#!/usr/bin/env bash

set -e
declare BASE_DIR
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-${0}}")"; pwd)"
readonly BASE_DIR
cd "${BASE_DIR}/.."

# MinGW-w64で<strsafe.h>の後に<shlwapi.h>をインクルードすると謎のエラーでビルドできない
# インクルードする順序を入れ替えるかSTRSAFE_NO_DEPRECATEを定義するとビルドできるようになる

declare -i exit_code=0
declare -i IS_DEBUG="${IS_DEBUG:-0}"
declare -r TARGET="${TARGET:-i686-w64-mingw32}"
declare -r WINDRES="${WINDRES:-"${TARGET}-windres"}"
declare -r CC="${CC:-"${TARGET}-gcc-win32"}"
declare -r CXX="${CXX:-"${TARGET}-g++-win32"}"
declare -r OUTPUT="OrinrinCollector"
declare -ar SOURCES=(src/{OrinrinCollector.cpp,stdafx.cpp} "obj/${TARGET}/OrinrinCollector.o")
# shellcheck disable=SC2054
declare -ar CFLAGS_ALL=(
	-D{WIN32,_WINDOWS,_ORCOLL}
	-D{UNICODE,_UNICODE}
	-DSTRSAFE_NO_DEPRECATE
	-W{all,extra}
	-m{unicode,win32,windows}
	-static-libgcc -static-libstdc++
	-ffunction-sections -fdata-sections -Wl,--gc-sections
)
declare -ar CFLAGS_DEBUG=(-D_DEBUG -Og -g3 -ggdb3)
declare -ar CFLAGS_RELEASE=(-DNDEBUG -O2 -g0)
declare -ar CXXFLAGS_ALL=("${CFLAGS_ALL[@]}")
declare -ar CXXFLAGS_DEBUG=("${CFLAGS_DEBUG[@]}")
declare -ar CXXFLAGS_RELEASE=("${CFLAGS_RELEASE[@]}")
declare -ar LDFLAGS=(-l{shell32,comdlg32,comctl32,shlwapi})
case "${IS_DEBUG}" in
	0)
		declare -ar CFLAGS=("${CFLAGS_ALL[@]}" "${CFLAGS_RELEASE[@]}")
		declare -ar CXXFLAGS=("${CXXFLAGS_ALL[@]}" "${CXXFLAGS_RELEASE[@]}")
	;;
	1)
		declare -ar CFLAGS=("${CXXFLAGS_ALL[@]}" "${CFLAGS_DEBUG[@]}")
		declare -ar CXXFLAGS=("${CXXFLAGS_ALL[@]}" "${CXXFLAGS_DEBUG[@]}")
	;;
	*)
		printf '\033[1;31mERROR\033[m The variable IS_DEBUG is neither 0 nor 1 (%s)\n' "${IS_DEBUG}" >&2
		exit 1
	;;
esac

mkdir -p {obj,bin}"/${TARGET}"

"${CXX}" -o"bin/${TARGET}/${OUTPUT}" "${CXXFLAGS[@]}" "${SOURCES[@]}" "${LDFLAGS[@]}"

exit "${exit_code}"
