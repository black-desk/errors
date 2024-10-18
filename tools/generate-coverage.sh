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

# shellcheck disable=SC2016
USAGE="$CURRENT_SOURCE_FILE_NAME"'

This script generate coverage.info file with lcov.

You can use this script in github action like this:

```yaml
# ...
steps:
  # ...
  - name: Build project with cmake by preset CI
    # Your cmake presets should set CXXFLAGS with --coverage.
    run: |
      cmake --workflow --preset CI &&
      ./tools/generate-lcov-coverage.sh
  - name: Upload results to Codecov
    uses: codecov/codecov-action@v4
    with:
      token: ${{ secrets.CODECOV_TOKEN }}
  # ...
```

'"
Usage:
  $CURRENT_SOURCE_FILE_NAME -h
  $CURRENT_SOURCE_FILE_NAME

Options:
  -h	Show this screen."

CURRENT_SOURCE_FILE_DIR="$(dirname -- "$CURRENT_SOURCE_FILE_PATH")"
cd -- "$CURRENT_SOURCE_FILE_DIR"

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

	. ./utils.sh

	check_ci

	local coverage_info_file
	coverage_info_file="coverage.info"

	cd ..

	lcov --capture \
		--directory . \
		--output-file "$coverage_info_file"
	lcov --remove "$coverage_info_file" \
		--output-file "$coverage_info_file" \
		"/usr/*" \
		"${CPM_SOURCE_CACHE:-${HOME}/.cache}/*" \
		"$(realpath "/tests")/*" \
		"$(realpath "/examples")/*"
	# lcov may report absulute path in new version
	# which confuses codecov,
	# so we need to replace it here.
	sed -i "s/${PWD//\//\\\/}\///g" "$coverage_info_file"
	lcov --list "$coverage_info_file"
}

main "$@"
