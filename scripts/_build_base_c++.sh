#!/usr/bin/env bash
# Usage: source(or .) _build_base.sh
# Don't run this script directly like "scripts/_build_bash.sh".
# Instead, set these variables:
# 	CFLAGS_ALL CFLAGS_DEBUG CFLAGS_RELEASE
# 	CXXFLAGS_ALL CXXFLAGS_DEBUG CXXFLAGS_RELEASE
# 	LDFLAGS
# 	CXX TARGET OUTPUT
# 	IS_DEBUG ENABLE_INCREMENTAL
# 	SOURCES objs
# And source this script like "source _build_base_c++.sh"
# or ". _build_base_c++.sh".
# Note: This script may run "exit" command

if [[ -v CHECK_VARIABLES ]]; then
	printf '\033[1;31mERROR\033[m The variable CHECK_VARIABLES is set\n' >&2
	exit 1
fi

declare -ar CHECK_VARIABLES=(
	CFLAGS_ALL CFLAGS_DEBUG CFLAGS_RELEASE
	CXXFLAGS_ALL CXXFLAGS_DEBUG CXXFLAGS_RELEASE
	LDFLAGS
	CXX TARGET OUTPUT
	IS_DEBUG ENABLE_INCREMENTAL
	SOURCES objs
)

declare check_variable
declare -i check_variables_errored=0
for check_variable in "${CHECK_VARIABLES[@]}"; do
	if [[ ! -v "${check_variable}" ]]; then
		printf '\033[1;31mERROR\033[m The variable %s is not set\n' "${check_variable}" >&2
		check_variables_errored=1
	fi
done
if [[ ! "${check_variables_errored}" == "0" ]]; then
	exit 1
fi

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
if [[ ! "${ENABLE_INCREMENTAL}" =~ ^(0|1)$ ]]; then
	printf '\033[1;31mERROR\033[m The variable ENABLE_INCREMENTAL is neither 0 nor 1 (%s)\n' "${ENABLE_INCREMENTAL}" >&2
fi

mkdir -p {obj,bin}"/${TARGET}"

# シンプルにビルドするならこれ:
# "${CXX}" -o"bin/${TARGET}/${OUTPUT}" "${CXXFLAGS[@]}" "${SOURCES[@]}" "${objs[@]}" "${LDFLAGS[@]}"

declare source
for source in "${SOURCES[@]}"; do
	if [[ ! "${source}" =~ ^src/ ]]; then
		printf '\033[1;31mERROR\033[m The file path "%s" does not starts with "src/"\n' "${source}" >&2
		exit 1
	fi
done
declare _source obj _obj dir
for source in "${SOURCES[@]}"; do
	#if [[ ! "${source,,}" =~ \.c(c|pp|xx|\+\+)?$ ]]; then
	#	printf '\033[1;33mWARNING\033[m Skipped building "%s" (not a C/C++ source file?)\n' "${source}" >&2
	#	continue
	#fi
	_source="./${source#src/}"
	dir="${_source%/*}"
	mkdir -p "obj/${TARGET}/${OUTPUT}/${dir}"
	_obj="${_source}.o"
	obj="obj/${TARGET}/${OUTPUT}/${_obj}"
	objs+=("${obj}")
	if [[ "${ENABLE_INCREMENTAL}" == "0" ]] || [[ "${obj}" -ot "${source}" ]]; then
		"${CXX}" -c -o"${obj}" "${CXXFLAGS[@]}" "${source}" "${LDFLAGS[@]}"
	fi
done

"${CXX}" -o"bin/${TARGET}/${OUTPUT}" "${CXXFLAGS[@]}" "${objs[@]}" "${LDFLAGS[@]}"
