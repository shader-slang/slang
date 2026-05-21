# Prompt: tests-agentic/cross-cutting/serialization/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/cross-cutting/serialization/`,
anchored to
[`docs/llm-generated/cross-cutting/serialization.md`](../../../docs/llm-generated/cross-cutting/serialization.md).

Audience: nightly CI. The bundle exercises the **serialization
machinery** documented in the source doc: AST + IR + container
(`.slang-module`) payloads driven through the generic
`serialize(serializer, value)` pattern, the Fossil and RIFF backends,
the source-location stream, and the IR-module version gate that keeps
older modules loadable by newer compilers.

The bundle is **small by construction**. Most claims in the source doc
are internal — they describe C++ headers, template patterns, and
binary chunk layouts that are not directly observable from `slangc`.
The user-facing surface that survives is:

- module compilation produces a serialized container
  (`slangc <src> -o foo.slang-module`);
- the container can be re-read by the compiler
  (`slangc -dump-module foo.slang-module`,
  `slangc -get-module-info foo.slang-module`);
- the version range supported by this build is exposed via
  `slangc -get-supported-module-versions`;
- container payloads include a downstream IR blob when
  `-embed-downstream-ir` is set, observed as an
  `EmbeddedDownstreamIR(...)` marker in `-dump-module` output;
- source-location streams round-trip into `#line` directives in
  emitted text, or into a `.zip` source-map archive when
  `-line-directive-mode source-map` is requested.

## The translation rule: claims to observations

`serialization.md` describes:

- **What is serialized** (`#what-is-serialized`) — three flavors:
  AST modules, IR modules, and the RIFF container that bundles
  them. The user-facing observable: `slangc <src> -o foo.slang-module`
  produces a file, and that file can be loaded back by the compiler
  via `-dump-module` or `-get-module-info`.
- **The `serialize()` pattern** (`#the-serialize-pattern`) — internal
  template machinery. Not directly observable from slangc CLI. Treat
  as out of scope.
- **Backends** (`#backends`, `#generic-structural-backend`,
  `#fossil-backend`) — internal C++ implementation. The Fossil
  format's memory-mappable property is internal; the only
  user-facing observable is that a `.slang-module` written by this
  build can be read back by the same build.
- **RIFF container format** (`#riff-container-format`) — internal
  chunk layout. Observable consequence: a `.slang-module`
  bundles AST + IR + (optionally) downstream IR blobs; the
  presence of an embedded downstream-IR chunk is visible in
  `-dump-module` output as an `EmbeddedDownstreamIR(...)` marker.
- **Source-location serialization**
  (`#source-location-serialization`) — round-tripping locations
  through serialization. Observable consequence: emitted text
  carries `#line` directives that point back to the source file
  (default `line-directive-mode standard`), and the
  `-line-directive-mode source-map` mode emits a separate zip with
  `.map` files.
- **Versioning and backwards compatibility**
  (`#versioning-and-backwards-compatibility`) — observable
  consequence: `slangc -get-supported-module-versions` prints the
  inclusive `[min, max]` range this compiler accepts, and
  `slangc -get-module-info <file>` prints the version baked into a
  serialized module. The two together let a caller decide whether
  to attempt a load.
- **Round-trip and repro files** (`#round-trip-and-repro-files`) —
  the doc itself says repro is **deprecated**; per CLAUDE.md
  `-dump-repro` / `-load-repro` are unmaintained. **Do not test
  repro.** Treat as out of scope.
- **Adding a new serialized field**
  (`#adding-a-new-serialized-field`) — a developer workflow,
  not a user-observable behaviour. Out of scope.

### Observable claims (write tests for these)

- **A `.slang-module` round-trips through the compiler**
  (`#what-is-serialized`): `slangc src -o out.slang-module` then
  `slangc -dump-module out.slang-module` prints the IR of the
  serialized module's contents.
- **The serialized module carries its name and version**
  (`#riff-container-format`, `#versioning-and-backwards-compatibility`):
  `slangc -get-module-info out.slang-module` prints the module name,
  module version (an integer), and compiler version.
- **The compiler exposes its supported module-version range**
  (`#versioning-and-backwards-compatibility`):
  `slangc -get-supported-module-versions` prints a `[min, max]`
  range with `min <= max`. The version baked into a freshly-built
  module falls within that range.
- **Embedding downstream IR adds an EmbeddedDownstreamIR chunk**
  (`#riff-container-format`): `slangc src -target spirv
  -embed-downstream-ir -o out.slang-module` then `-dump-module`
  shows `EmbeddedDownstreamIR(...)`; without `-embed-downstream-ir`
  that marker is absent.
- **Multi-target containers carry one downstream-IR chunk per target**
  (`#riff-container-format`): `-target dxil -embed-downstream-ir
  -target spirv -embed-downstream-ir` produces two
  `availableInDownstreamIR` markers on each public symbol when
  dumped with `-dump-ir`.
- **Source-location stream survives compilation to text**
  (`#source-location-serialization`): emitted HLSL / GLSL / Metal /
  CUDA / WGSL / C++ carries `#line` (or target-specific equivalent)
  directives pointing back to the source file. SPIR-V assembly
  carries `OpSource` / `OpLine`.
- **`-line-directive-mode source-map` emits a separate source-map**
  (`#source-location-serialization`): when the requested output is
  a `.zip`, the archive contains `.map` files alongside the emitted
  text.

### Not testable through slangc CLI (record under `## Out of scope`)

- The **`serialize(serializer, value)` template pattern**
  (`#the-serialize-pattern`) — a C++ idiom; not observable from CLI.
- The **`SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS` macro**
  (`#fossil-backend`) — a compile-time toggle baked into the
  binary; we cannot flip it from the CLI.
- The **Fossil memory-mappable property** (`#fossil-backend`) — an
  internal performance claim; not visible from emitted output.
- The **RIFF chunk-code layout** (`#riff-container-format`) — would
  require reading raw bytes; the doc itself defers chunk codes to
  source files.
- **Round-trip via `-dump-repro` / `-load-repro`**
  (`#round-trip-and-repro-files`) — deprecated per
  CLAUDE.md "AVOID These Debugging Options". Do not test.
- The **"Adding a new serialized field" walkthrough**
  (`#adding-a-new-serialized-field`) — developer workflow.
- The **`Unrecognized` opcode behaviour**
  (`#versioning-and-backwards-compatibility`) — internal
  deserializer placeholder; testing it would require constructing
  a module whose opcode set is outside the current compiler's,
  which the CLI cannot do.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. **6 to 12** `.slang` test files. The bundle is intentionally
   small: many doc claims are internal and end up under
   `## Out of scope`.
3. Mix:
   - **`TEST:COMPILE` + `TEST:SIMPLE` pairs** that build a
     `.slang-module` then inspect it (`-dump-module`,
     `-get-module-info`). This is the dominant pattern.
   - **`TEST:SIMPLE` direct slangc commands** for `-get-supported-
     module-versions` (no input file needed).
   - **multi-target `TEST:SIMPLE`** tests that observe source-location
     round-trip across backends.
4. **Do not use** `INTERPRET` for any claim here — `slangi` does not
   exercise serialized-module loading. `DIAGNOSTIC_TEST` is also not
   the natural fit; the serialization layer's user-facing diagnostics
   are about malformed modules, which we cannot construct.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/cross-cutting/serialization.md`

No secondary doc citations. If you find a behaviour that needs a doc
anchor that doesn't exist, record it under `## Doc gaps observed`.

## Source files you may consult for _verification only_

You may look at these to confirm a CLI spelling or to spot-check that
the chunk markers in `-dump-module` output are stable. You may **not**
mine them for behavioural claims the doc does not make.

- `source/slang/slang-serialize-container.cpp` (chunk codes)
- `source/slang/slang-ir-insts.lua` (`Unrecognized`, `module_version`)
- `tests/ir/dump-module.slang`, `tests/ir/dump-module-info.slang`
- `tests/modules/multi-target-module.slang`
- `tests/feature/source-map/emit-source-map.slang`

## Test directives

The standard pattern for "compile a module then inspect it":

```
//TEST:COMPILE: tests-agentic/cross-cutting/serialization/<src>.slang -o tests-agentic/cross-cutting/serialization/<out>.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module tests-agentic/cross-cutting/serialization/<out>.slang-module
```

For a no-input CLI probe:

```
//TEST:SIMPLE(filecheck=CHECK): -get-supported-module-versions
```

For multi-target source-loc observation:

```
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute -g2
```

## Lessons captured for serialization tests

These add to the universal lessons in `_common.md`. Apply them ALL:

- **`//TEST:COMPILE:` writes its output file relative to the
  repository root**, not relative to the .slang file. Always spell
  the path as `tests-agentic/cross-cutting/serialization/foo.slang-module`,
  not `./foo.slang-module`.
- **`-dump-module` requires a `.slang-module` argument**; pointing it
  at a raw `.slang` source is a different code path (it loads the
  source and dumps the resulting IR — see `tests/ir/dump-module.slang`
  for the precedent that calls both forms out as distinct).
- **The version printed by `-get-module-info` is an integer that
  changes over time**. Match it with a `{{[0-9]+}}` wildcard, not a
  literal value.
- **The compiler-version line in `-get-module-info` output contains
  a git description with hashes**. Match with `{{.+}}`, never a
  literal.
- **`-embed-downstream-ir` requires a `-target` that has downstream
  IR**: SPIR-V (`-target spirv`), DXIL (`-target dxil`), or DXBC.
  CUDA/HLSL/GLSL/Metal/WGSL/CPP text targets do **not** produce a
  downstream-IR blob.
- **For SPIR-V embedding, pair with `-skip-spirv-validation`** unless
  the test source is known to pass validation. The agentic runner
  has no SPIRV-Tools binary; validation runs against the in-tree
  copy and can flag SSA / decoration issues unrelated to the claim.
- **`-line-directive-mode source-map` writes a zip**, not flat text.
  The `-o` argument must end in `.zip` for the runner to accept it,
  and the test cannot easily FileCheck the zip contents — observe the
  default `#line` form by **not** passing `-line-directive-mode
  source-map`, and use a dedicated check for the source-map zip case
  only if a follow-up CLI command (`unzip`) is reasonable. The
  cleanest approach is: assert `#line` in the default emitter
  output and treat the zip emission as a follow-up CLI-form claim.
- **`#line` directives appear with absolute or relative paths**
  containing the test filename. Match the filename with a
  `{{.*serialization.*\.slang}}` wildcard rather than the full path.
- **`SPIR-V assembly` carries source locations as `OpSource` and
  `OpLine`** — match these markers, not `#line`.
- **Metal `#line` directives use the same syntax as C/C++**, but
  WGSL does not emit `#line` (the WGSL grammar has no such
  directive); record that as a backend-specific observation.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `cross-cutting/serialization.md`.
- [ ] At least one test exercises a `TEST:COMPILE` → `-dump-module`
      round-trip.
- [ ] At least one test exercises `-get-module-info`.
- [ ] At least one test exercises `-get-supported-module-versions`.
- [ ] At least one test exercises `-embed-downstream-ir` and
      observes the `EmbeddedDownstreamIR(...)` marker.
- [ ] At least one test exercises a source-loc round-trip through
      emitted `#line` / `OpLine` directives.
- [ ] No test exercises `-dump-repro` / `-load-repro` (deprecated).
- [ ] No test exercises `INTERPRET` (irrelevant to the serialization
      layer).
- [ ] `## Out of scope (no-GPU runner)` enumerates the
      Fossil-memory-map property, raw RIFF chunk inspection,
      repro round-trip, and the C++ `serialize()` template.
- [ ] Total file count is **6–12**; a bigger bundle would be
      stretching the small set of CLI-observable claims.
