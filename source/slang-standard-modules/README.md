# slang-standard-modules (optional embedded-module library)

Enabled with `-DSLANG_EMBED_STANDARD_MODULES=ON`. Builds a single shared library
(`slang-standard-modules.dll` / `libslang-standard-modules-<ver>.so`) that embeds
all serialized standard modules (`*.slang-module`), as an **optional alternative**
to installing the loose `slang-standard-module-<version>/` directory. When enabled,
this library is **installed in place of** the loose tree: the loose files are
still built (they are this library's generator input) but their `install()` rules
in `../standard-modules` are skipped (guarded on `NOT SLANG_EMBED_STANDARD_MODULES`).
With the option off, the loose tree is built and installed as before and this
library is not built.

Like `source/slang-glsl-module`, this is a runtime-loaded `MODULE` library; it is
a directory sibling of that and `source/slang-core-module`. The standard-module
*sources* and their compilation live in `source/standard-modules`; this directory
only packages their build output.

## How it is generated (mirrors slang-glsl-module, with LZ4)

- `tools/slang-embed-standard-modules` is a build-time generator (registered as
  a `generator()`, so it is built for the host and reused when cross-compiling).
  It scans the already-built versioned module tree, **LZ4-compresses** each
  `*.slang-module` with `LZ4CompressionSystem`, and emits
  `slang-standard-modules-generated.cpp`: one `static const unsigned char[]` of
  compressed bytes per module plus a `path -> {data, compressedSize,
  uncompressedSize}` table keyed by import path (`slang/neural`,
  `slang/numerics/differentiable`, `experimental/workgraph`, ...). Because it
  scans the build output, new modules are embedded with no edit here. The custom
  command `DEPENDS` on the actual built `.slang-module` files (the
  `SLANG_STANDARD_MODULE_FILES` manifest that each module subdirectory appends
  to), so editing a module regenerates the payload instead of leaving it stale.
  The generated source is written per-configuration (under `$<CONFIG>`) so
  multi-config builds do not race on or reuse one another's payload.
- `slang-embedded-standard-modules.cpp` wraps that table and exports
  `slang_getEmbeddedModuleBuildTagString` (version stamp, as the other embedded
  modules do) and
  `slang_getEmbeddedStandardModule(path, &data, &compressedSize, &uncompressedSize)`.
- The compiler loads it in the standard-module import fallback
  (`source/slang/slang-session.cpp`, `tryLoadStandardModuleFromDLL`): load the
  library beside `slang.dll`, verify the build tag, look up by import path,
  **decompress with the same `LZ4CompressionSystem`**, and feed the bytes to the
  same `loadModuleImpl(..., ModuleBlobType::IR, ...)` used for the loose files.
  The `-experimental-feature` gate is unaffected (enforced during import
  resolution, independent of where the bytes come from).

## Why LZ4

The glsl module is small, so embedding it as a plain C byte array is fine. The
serialized standard modules are much larger, and a plain byte array is several
source bytes per payload byte, so an uncompressed embed produces a source that is
large and expensive to compile. Compressing the payload first keeps the generated
source small enough to build cheaply; modules are decompressed on first import in
each session, only for modules that are actually imported.

LZ4 is used because Slang already links it (the core module uses riff-lz4), so no
new dependency is introduced and the same code compresses and decompresses.
