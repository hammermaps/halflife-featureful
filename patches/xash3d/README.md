# patches/xash3d

Patch files applied to the freshly cloned `xash3d-fwgs` engine checkout before
it's built by `build-xash.sh`.

## Format

- One `git apply`-compatible patch per file, extension `.patch` or `.diff`.
- Patches are applied in alphabetical order — prefix with a number if order
  matters, e.g. `0001-fix-something.patch`, `0002-other-fix.patch`.
- Generate a patch from inside the `xash3d-fwgs` checkout with:
  ```
  git diff > ../patches/xash3d/0001-my-change.patch
  ```
  or, for a committed change:
  ```
  git format-patch -1 <commit> -o ../patches/xash3d/
  ```

## Behavior

`build-xash.sh` applies every patch here right after cloning (or updating)
`xash3d-fwgs`, before running `./waf configure`/`./waf build`. Patches that
are already applied (e.g. because the source tree is reused between runs)
are detected and skipped instead of failing the build.

This directory is empty by default — drop patches in here as needed.
