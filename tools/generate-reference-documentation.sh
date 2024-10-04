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

This script generates reference documentation from master branch using Doxygen,
and commit it to the gh-pages branch.

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

	if [ "$(git branch --show-current)" != "master" ]; then
		log "[ERROR] This script should be run from the master branch"
		exit 1
	fi

	BUILD_DIR=../build_generate-reference-documentation

	# create a new branch gh-pages if it not exists
	if ! git show-ref --verify --quiet refs/heads/gh-pages; then
		git checkout --orphan gh-pages
		git reset --hard
		git commit --allow-empty -m "Initial commit"
		git switch master
	fi

	configure_cmake_project .. $BUILD_DIR >&2
	build_cmake_project $BUILD_DIR >&2

	git switch gh-pages

	rm ../docs -rf
	cp -r $BUILD_DIR/html ../docs

	git add --all
	git commit -m "Update reference documentation"
}

main "$@"
