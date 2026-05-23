# Repository Guidelines

## Project Structure & Module Organization

Slang is a shading-language compiler and runtime implemented primarily in C++20 and built with
CMake.

Key directories:
- `source/`: core implementation, including `source/slang/`, `source/core/`,
  `source/compiler-core/`, and tools like `source/slangc/`.
- `include/`: public API headers.
- `prelude/` and `source/standard-modules/`: standard/prelude headers.
- `tests/`: test suites grouped by feature or target.
- `tools/`: test infrastructure and developer tools.
- `docs/`: documentation.
- `examples/`: runnable samples.
- `cmake/`: CMake helpers.
- `external/`: vendored dependencies.

## Build, Test, and Development Commands

Slang build setup is platform-specific, especially under WSL. For compiler builds, use the
`slang-build` skill from `skills/slang-build` in the `shader-slang/slang-skills` repository
instead of following hard-coded commands in this file.

If the skill is unavailable because skills cannot be installed or network access is limited, use
`docs/building.md` as the fallback build reference.

Examples:
- `/slang-build build debug`: build the Debug configuration.
- `/slang-build build release`: build the Release configuration.
- `/slang-build rebuild debug`: discard the existing build directory and rebuild Debug.
- `/slang-build configure releasewithdebug`: configure an optimized build with symbols.
- `/slang-build clean`: rename and remove the existing build directory.

Do not infer WSL build commands from generic Linux instructions. Follow the platform detection,
host-tool selection, CMake preset choice, and clean-build steps defined by the skill.

After building, run tests from the repository root using the generated `slang-test` binary in
the directory for the selected configuration:
- `build/Debug/bin/slang-test`: run the Debug test suite.
- `build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8`: run optimized
  tests with symbols in parallel using test servers.
- `build/Release/bin/slang-test -use-test-server -server-count 8`: run Release tests in
  parallel using test servers.

On Windows-hosted builds, use the `.exe` suffix if that is the generated binary name.

## Coding Style & Naming Conventions

Formatting:
- Use four-space indentation for C, C++, headers, and Slang files.
- Run `./extras/formatting.sh` before committing to apply rules from `.clang-format`
  and `.editorconfig`.
- Follow Allman braces, a 100-column limit, left-aligned pointers, and final newlines.

Conventions:
- Follow `docs/design/coding-conventions.md`.
- Avoid STL containers, iostreams, RTTI, and exceptions for ordinary errors.
- Use `UpperCamelCase` for types and `lowerCamelCase` for values.
- Use `SLANG_`-prefixed `SCREAMING_SNAKE_CASE` for macros.
- Prefer comments that explain why code exists.

## Testing Guidelines

Add tests near related coverage in `tests/`.

Slang tests:
- Use leading directives such as `//TEST(smoke):SIMPLE:`.
- Use `//DISABLE_TEST` only with a clear reason.
- For targeted runs, pass a prefix, for example
  `build/Debug/bin/slang-test tests/diagnostics/my-test`.

Unit tests live under `tools/slang-unit-test` and typically use `SLANG_UNIT_TEST(name)`.

## Commit & Pull Request Guidelines

- Use short, imperative commit subjects, for example `Reject invalid descriptor heap access`.
- Keep PRs small and based on `master`.
- PRs require passing workflows, review approval, and a `pr: non-breaking` or
  `pr: breaking change` label.
- Human contributors should sign the CLA when prompted.
- For formatting failures, run installed hooks from `./extras/install-git-hooks.sh` or
  request the format bot with `/format`.
