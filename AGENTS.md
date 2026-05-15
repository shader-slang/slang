# Repository Guidelines

## Project Structure & Module Organization

Slang is a shading-language compiler and runtime implemented primarily in C++17 and built with CMake. Core implementation lives in `source/`, with major components such as `source/slang/`, `source/core/`, `source/compiler-core/`, and command-line tools like `source/slangc/`. Public API headers are in `include/`. Standard/prelude headers are in `prelude/` and `source/standard-modules/`. Tests live under `tests/`, grouped by feature or target, while test infrastructure and developer tools are under `tools/`. Documentation is in `docs/`, runnable samples are in `examples/`, CMake helpers are in `cmake/`, and vendored dependencies are in `external/`.

## Build, Test, and Development Commands

- `cmake --preset default`: configure the default Ninja Multi-Config build in `build/`.
- `cmake --build --preset debug`: build the Debug configuration.
- `cmake --build --preset releaseWithDebugInfo`: build an optimized configuration with symbols.
- `cmake --workflow --preset release`: configure, build, and package a release build.
- `build/Debug/bin/slang-test`: run the test suite from the repository root.
- `build/Release/bin/slang-test -use-test-server -server-count 8`: run tests in parallel using test servers.
- `cmake --preset vs2026`: preferred Windows configuration preset for Visual Studio 2026.

## Coding Style & Naming Conventions

Use four-space indentation for C, C++, headers, and Slang files. Run `./extras/formatting.sh` before committing; it applies the project formatting rules from `.clang-format` and `.editorconfig`, including Allman braces, a 100-column limit, left-aligned pointers, and final newlines. Follow `docs/design/coding-conventions.md`: avoid STL containers, iostreams, RTTI, and exceptions for ordinary errors. Use `UpperCamelCase` for types, `lowerCamelCase` for values, and `SLANG_`-prefixed `SCREAMING_SNAKE_CASE` for macros. Prefer comments that explain why code exists.

## Testing Guidelines

Add tests near related coverage in `tests/`. Slang test files are selected with leading directives such as `//TEST(smoke):SIMPLE:`; use `//DISABLE_TEST` only with a clear reason. Unit tests are under `tools/slang-unit-test`, typically declared with `SLANG_UNIT_TEST(name)`. For targeted runs, pass a prefix, for example `build/Debug/bin/slang-test tests/diagnostics/my-test`.

## Commit & Pull Request Guidelines

Use short, imperative commit subjects, for example `Reject invalid descriptor heap access`. Keep PRs small and based on `master`. PRs require passing workflows, review approval, and a `pr: non-breaking` or `pr: breaking change` label. Sign the CLA when prompted. For formatting failures, run installed hooks from `./extras/install-git-hooks.sh` or request the format bot with `/format`.
