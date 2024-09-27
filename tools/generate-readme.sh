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

This script generate the top level README.md file for the repository.

'"
Usage:
  $CURRENT_SOURCE_FILE_NAME -h
  $CURRENT_SOURCE_FILE_NAME

Options:
  -h	Show this screen."

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

CURRENT_SOURCE_FILE_DIR="$(dirname -- "$CURRENT_SOURCE_FILE_PATH")"
cd -- "$CURRENT_SOURCE_FILE_DIR"

source ./utils.sh

# ------------------------------------------------------------------------------

cd ..

podman run -it --rm \
	-v "$PWD:/data" \
	docker.io/blackdesk/pandocx:latest \
	--template ./.README.template.md \
	".README.md" -o "./README.md" -t gfm-yaml_metadata_block \
	-s \
	-V toc-title:"Table of Contents" \
	--shift-heading-level-by=1 \
	--table-of-contents \
	--toc-depth=4 \
	-M include-auto \
	--from markdown+east_asian_line_breaks \
	--lua-filter include-files.lua \
	--lua-filter include-code-files.lua
