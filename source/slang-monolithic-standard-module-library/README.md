# slang-monolithic-standard-module-library (optional)

Enabled with `-DSLANG_BUILD_MONOLITHIC_STANDARD_MODULE_LIBRARY=ON`. Builds a single
monolithic shared library (`slang-standard-modules.dll` /
`libslang-standard-modules-<ver>.so`) that holds all serialized standard modules
(`*.slang-module`), as an **optional alternative** to installing the loose
`slang-standard-module-<version>/` directory.

When enabled, this library is **installed in place of** the loose tree: the loose
files are still built (they are this library's generator input) but their
`install()` rules in `../standard-modules` are skipped (guarded on
`NOT SLANG_BUILD_MONOLITHIC_STANDARD_MODULE_LIBRARY`). With the option off, the loose
tree is built and installed as before and this library is not built.

This directory only packages build output: the standard-module *sources* and
their compilation live in `source/standard-modules`.

## How it is generated

- `tools/slang-generate-monolithic-standard-module-library` is a build-time generator (registered as a
  `generator()`, so it is built for the host and reused when cross-compiling). It
  scans the already-built versioned module tree, **LZ4-compresses** each
  `*.slang-module` with `LZ4CompressionSystem`, and emits
  `slang-monolithic-standard-module-library-generated.cpp`: one
  `static const unsigned char[]` of compressed bytes per module plus a
  `path -> {data, compressedSize, uncompressedSize}` table keyed by import path
  (`slang/neural`, `slang/numerics/differentiable`, `experimental/workgraph`,
  ...). Because it scans the build output, new modules are included with no edit
  here. The custom command `DEPENDS` on the actual built `.slang-module` files
  (the `SLANG_STANDARD_MODULE_FILES` manifest that each module subdirectory
  appends to), so editing a module regenerates the payload instead of leaving it
  stale. The generated source is written per-configuration (under `$<CONFIG>`) so
  multi-config builds do not race on or reuse one another's payload.
- `slang-monolithic-standard-module-library.cpp` wraps that table and exports a
  build-tag version stamp and
  `slang_getStandardModuleFromMonolithicLibrary(path, &data, &compressedSize, &uncompressedSize)`.
- The compiler loads it in the standard-module import fallback
  (`source/slang/slang-session.cpp`,
  `tryLoadStandardModuleFromMonolithicLibrary`): load the library beside
  `slang.dll`, verify the build tag, look up by import path, **decompress with the
  same `LZ4CompressionSystem`**, and feed the bytes to the same
  `loadModuleImpl(..., ModuleBlobType::IR, ...)` used for the loose files. The
  `-experimental-feature` gate is unaffected (enforced during import resolution,
  independent of where the bytes come from).

## Why LZ4

The serialized standard modules are large, and a plain C byte array is several
source bytes per payload byte, so an uncompressed embed produces a generated
source that is large and expensive to compile. Compressing the payload first
keeps that source small enough to build cheaply; modules are decompressed on
first import in each session, only for modules that are actually imported.

LZ4 is used because Slang already links it (the core module uses riff-lz4), so no
new dependency is introduced and the same code compresses and decompresses.
