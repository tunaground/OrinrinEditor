#!/usr/bin/env bash

set -e
declare BASE_DIR
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-${0}}")"; pwd)"
readonly BASE_DIR
cd "${BASE_DIR}/.."

# MinGW-w64で<strsafe.h>の後に<shlwapi.h>をインクルードすると謎のエラーでビルドできない
# インクルードする順序を入れ替えるかSTRSAFE_NO_DEPRECATEを定義するとビルドできるようになる

declare -i exit_code=0
declare -i ENABLE_INCREMENTAL="${ENABLE_INCREMENTAL:-1}"
declare -i IS_DEBUG="${IS_DEBUG:-0}"
declare -r TARGET="${TARGET:-i686-w64-mingw32}"
declare -r WINDRES="${WINDRES:-"${TARGET}-windres"}"
declare -r CC="${CC:-"${TARGET}-gcc-win32"}"
declare -r CXX="${CXX:-"${TARGET}-g++-win32"}"
declare -r OUTPUT="OrinrinCollector"
declare -ar SOURCES=(
	src/{OrinrinCollector.cpp,stdafx.cpp}
)
declare -a objs=(
	"obj/${TARGET}/OrinrinCollector.rc.o"
)
# shellcheck disable=SC2054
declare -ar CFLAGS_ALL=(
	-D{WIN32,_WINDOWS,_ORCOLL}
	-D{UNICODE,_UNICODE}
	-DSTRSAFE_NO_DEPRECATE
	-W{all,extra}
	-m{unicode,win32,windows}
	-static-libgcc
	-ffunction-sections -fdata-sections -Wl,--gc-sections
)
declare -ar CFLAGS_DEBUG=(-D_DEBUG -Og -g3 -ggdb3)
declare -ar CFLAGS_RELEASE=(-DNDEBUG -O2 -g0 -ggdb0)
declare -ar CXXFLAGS_ALL=("${CFLAGS_ALL[@]}" -static-libstdc++)
declare -ar CXXFLAGS_DEBUG=("${CFLAGS_DEBUG[@]}")
declare -ar CXXFLAGS_RELEASE=("${CFLAGS_RELEASE[@]}")
declare -ar LDFLAGS=(-l{shell32,comdlg32,comctl32,shlwapi})

source scripts/_build_base_c++.sh

exit "${exit_code}"
