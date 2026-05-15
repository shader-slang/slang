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

- `cmake --preset default`: configure the default Ninja Multi-Config build in `build/`.
- `cmake --build --preset debug`: build the Debug configuration.
- `cmake --build --preset releaseWithDebugInfo`: build an optimized configuration with symbols.
- `cmake --workflow --preset release`: configure, build, and package a release build.
- `build/Debug/bin/slang-test`: run the test suite from the repository root.
- `build/Release/bin/slang-test -use-test-server -server-count 8`: run tests in parallel
  using test servers.
- `cmake --preset vs2026`: preferred Windows configuration preset for Visual Studio 2026.

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
