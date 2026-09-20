#!/usr/bin/env bash
#
# Shared helper: apply every *.patch/*.diff from a directory to the git repo
# in the current working directory, in alphabetical order. Idempotent - a
# patch already applied (source tree reused between runs) is detected via
# `git apply --reverse --check` and skipped instead of failing the build.
#
# Usage: source this file, then call:
#   apply_patches_from "/absolute/path/to/patch-dir"
# from inside the target git repo's working directory.

apply_patches_from() {
	local patch_dir="$1"
	local -a patches
	local p

	shopt -s nullglob
	patches=("$patch_dir"/*.patch "$patch_dir"/*.diff)
	shopt -u nullglob

	if [[ ${#patches[@]} -eq 0 ]]; then
		return 0
	fi

	IFS=$'\n' patches=($(sort <<<"${patches[*]}")); unset IFS

	echo "Applying patches from $patch_dir"
	for p in "${patches[@]}"; do
		if git apply --check "$p" 2>/dev/null; then
			git apply "$p"
			echo "  applied: $(basename "$p")"
		elif git apply --reverse --check "$p" 2>/dev/null; then
			echo "  already applied, skipping: $(basename "$p")"
		else
			echo "  FAILED to apply: $(basename "$p")" >&2
			return 1
		fi
	done
}
