#!/usr/bin/env bash
# NOTE:
# Use /usr/bin/env to find shell interpreter for better portability.
# Reference: https://en.wikipedia.org/wiki/Shebang_%28Unix%29#Portability

# NOTE:
# Exit immediately if any commands (even in pipeline)
# exits with a non-zero status.
set -e
set -o pipefail

# WARNING:
# This is not reliable when using POSIX sh
# and current script file is sourced by `source` or `.`
CURRENT_SOURCE_FILE_PATH="${BASH_SOURCE[0]:-$0}"
CURRENT_SOURCE_FILE_NAME="$(basename -- "$CURRENT_SOURCE_FILE_PATH")"

# This function log messages to stderr works like printf
# with a prefix of the current script name.
# Arguments:
#   $1 - The format string.
#   $@ - Arguments to the format string, just like printf.
function log() {
	local format="$1"
	shift
	# shellcheck disable=SC2059
	printf "$CURRENT_SOURCE_FILE_NAME: $format\n" "$@" >&2 || true
}

# shellcheck disable=SC2016
USAGE="$CURRENT_SOURCE_FILE_NAME"'

This script prints the project version to STDOUT
for automatically creating tag in CI.

'"
Usage:
  $CURRENT_SOURCE_FILE_NAME -h
  $CURRENT_SOURCE_FILE_NAME

Options:
  -h	Show this screen."

CURRENT_SOURCE_FILE_DIR="$(dirname -- "$CURRENT_SOURCE_FILE_PATH")"
cd -- "$CURRENT_SOURCE_FILE_DIR"

function main() {
	while getopts ':h' option; do
		case "$option" in
		h)
			echo "$USAGE"
			exit
			;;
		\?)
			log "[ERROR] Unknown option: -%s" "$OPTARG"
			exit 1
			;;
		esac
	done
	shift $((OPTIND - 1))

	source ./utils.sh

	check_ci

	BUILD_DIR=../build_get-project-version

	configure_cmake_project .. $BUILD_DIR >&2
	cd $BUILD_DIR
	version=$(cmake --system-information | awk -F= '$1~/CMAKE_PROJECT_VERSION:STATIC/{print$2}')
	if [ -z "$version" ]; then
		false
	fi
	echo "v$version"
}

main "$@"
