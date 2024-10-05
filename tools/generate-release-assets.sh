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

This script generate release assets
then print paths to the generated assets to STDOUT.

You can use this script in github action like this:

```yaml
name: Example workflow
on:
  push:
    tags: *

jobs:
  auto_release:
    name: Automatic release
    runs-on: ubuntu-latest
    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Run tools/'"$CURRENT_SOURCE_FILE_NAME"'
        id: generate_release_assets
        run: |
          ASSETS=$(tools/'"$CURRENT_SOURCE_FILE_NAME"')
          echo assets="$ASSETS" >> $GITHUB_OUTPUT

      - name: Automatic release
        uses: marvinpinto/action-automatic-releases@latest
        with:
          repo_token: ${{ secrets.GITHUB_TOKEN }}
          prerelease: false
          files: ${{ steps.generate_release_assets.outputs.assets }}
```

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

check_ci

BUILD_DIR="../build_generate-release-assets"

configure_cmake_project .. $BUILD_DIR -DCMAKE_BUILD_TYPE=Release >&2

cd "$BUILD_DIR"

../tools/amalgamate.py -c ../tools/errors.json -s .

realpath "$BUILD_DIR"/errors.hpp
