#!/usr/bin/env bash
# NOTE:
# Use /usr/bin/env to find shell interpreter for better portability.
# Reference: https://en.wikipedia.org/wiki/Shebang_%28Unix%29#Portability

# NOTE:
# Exit immediately if any commands (even in pipeline)
# exits with a non-zero status.
set -e
set -o pipefail

# This function generates an absolute path from a likely relative path argument.
# Arguments:
#   $1 - The file path which can be relative or absolute.
# Returns:
#   The absolute path of the given file path.
function get_absolute_path() {
	local file_path
	file_path="$1"
	shift

	if [ -n "${file_path%%/*}" ]; then
		file_path="$PWD/$file_path"
	fi
	echo "$file_path"
}

# This function checks if the script is running in a CI environment.
# If not, it logs a warning message.
function check_ci() {
	# NOTE:
	# GitHub actions sets CI environment variable.
	# Reference: https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/store-information-in-variables#default-environment-variables
	if [ -n "$CI" ]; then
		return
	fi
	log "[WARNING] This script is meant to be run in CI environment"
}

# This function runs a command in a specified directory.
# Arguments:
#   $1 - The directory in which to run the command.
#   $@ - The command to run.
function run_in_directory() {
	local directory
	directory="$1"
	shift

	# NOTE:
	# Do not use pushd and popd here for POSIX sh compliance.
	local old_pwd
	old_pwd="$PWD"
	cd -- "$directory"
	"$@"
	cd -- "$old_pwd"
}

# This function configures a CMake project.
#
# Arguments:
#   $1 - The source directory of the project.
#   $2 - The directory where the build will take place.
#   $@ - Additional arguments to pass to the cmake command.
function configure_cmake_project() {
	local source_dir
	source_dir="$(get_absolute_path "$1")"
	shift

	local binary_dir
	binary_dir="$(get_absolute_path "$1")"
	shift

	# NOTE:
	# Do not use cmake -S and -B options for better compatibility.

	mkdir -p "$binary_dir"

	run_in_directory "$binary_dir" cmake "$source_dir" "$@"
}

# This function builds a CMake project.
#
# Arguments:
#  $1 - The directory where the build is located.
#  $@ - Additional arguments to pass to the cmake --build command.
function build_cmake_project() {
	local binary_dir
	binary_dir="$(get_absolute_path "$1")"
	shift

	run_in_directory "$binary_dir" cmake --build . "$@"
}
