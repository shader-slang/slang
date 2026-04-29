Shader Coverage Design
======================

This document describes the phase-1 shader coverage implementation in
Slang, the background that led to it, and the role of the main pieces in
the pipeline.

Overview
--------

Shader coverage has two separate jobs:

1. insert execution counters into generated shader code
2. let the host discover the counter buffer and map counter slots back
   to source locations

In the current implementation, those jobs are handled by two different
mechanisms:

- AST-time synthesis of `RWStructuredBuffer<uint> __slang_coverage`
- post-emit coverage metadata exposed as `ICoverageTracingMetadata`

The first is about binding and reflection visibility. The second is
about reporting and attribution.

Binding and attribution
-----------------------

Coverage instrumentation has two jobs that don't reduce to one
primitive:

- **Binding** — getting the host to allocate a GPU buffer of the
  right size and put it where the shader expects. Reflection
  already solves this for every other shader resource: hosts
  (slang-rhi, slangpy, custom Vulkan/D3D12 wrappers) walk
  `ProgramLayout` to discover parameters and bind them. AST-time
  synthesis puts `__slang_coverage` into that pipeline, so the
  buffer is bound the same way any other `RWStructuredBuffer<uint>`
  would be — no coverage-specific code on the host.

- **Attribution** — turning counter values back into source
  locations. Reflection doesn't carry "slot 7 →
  `physics.slang:22`"; that's not what reflection is about.
  Reflection knows parameter names, types, and layouts. We need a
  side-channel that records the per-slot semantic intent.
  `ICoverageTracingMetadata` (and its on-disk twin
  `.coverage-mapping.json`) is that channel.

The two channels carry complementary data, not duplicate data —
binding info answers "where" and attribution answers "what." A host
needs both: without the binding it cannot allocate or bind the
buffer; without the attribution it cannot interpret the counter
values it reads back. The metadata is also intentionally a little
richer than LCOV line coverage — some slots may not map to a real
source file and line, and that fact is preserved in the metadata
and JSON sidecar. The LCOV conversion step then applies gcov-style
reporting rules by filtering those entries out of line-oriented
output.

An IR-only synthesis approach was considered and rejected:
`ProgramLayout` is built from AST-declared parameters and is frozen
before the IR pass runs, so an IR-only buffer would be invisible to
every reflection-driven host. Hosts would then need a separate code
path to look up the binding via the metadata API, which defeats the
"fits into existing integration patterns" goal. Reflection-visibility
is what makes the feature drop-in for slang-rhi, slangpy, and the
custom Vulkan/D3D12 hosts that walk reflection data — they don't
need to know that this particular parameter came from
`-trace-coverage` rather than the user's source.

Pipeline architecture
---------------------

Enabling `-trace-coverage` runs four pipeline stages:

1. **AST-check time** (`source/slang/slang-check-synthesize-coverage.{h,cpp}`).
   Runs during semantic check, before parameter binding. The
   synthesizer:
   - Creates a `RWStructuredBuffer<uint> __slang_coverage` `VarDecl`
     in the module scope. The decl flows through Slang's normal
     reflection and layout pipeline, so every backend and every
     reflection-driven host sees it as a first-class shader
     parameter — no coverage-specific code on the host.
   - Skips synthesis if the user has already declared a
     `__slang_coverage` themselves, or if a transitively-imported
     module already carries one. The synthesizer walks imports to
     dedupe — important for multi-file shaders that would otherwise
     collide on an explicit binding.
   - When `-trace-coverage-binding <index> <space>` is specified,
     attaches an explicit `register(uN, spaceM)` semantic to the
     synthesized decl so parameter binding pins it to the requested
     slot rather than auto-allocating one.
   - Marks the decl with a `SynthesizedModifier`. Downstream tooling
     that walks parameters (IDEs, language server, reflection
     viewers, doc generators) can filter on this marker to keep the
     synthesized buffer out of user-facing views. The buffer stays
     reflection-visible to runtime hosts that need to bind it; the
     marker only affects *editor-side* consumers.

2. **AST lowering** (`source/slang/slang-lower-to-ir.cpp`). Before
   each *executable* statement is lowered to IR, the front-end
   emits an `IncrementCoverageCounter` IR op. Purely structural
   compound statements (`BlockStmt`, `SeqStmt`, `EmptyStmt`) are
   filtered to keep counter density proportional to real execution
   events. The op's source position rides on the standard
   per-instruction `sourceLoc` field — no operands, no debug
   decoration — so it survives `stripDebugInfo` and every IR
   transform that preserves operands (inline, clone, link).

3. **IR pass** (`source/slang/slang-ir-coverage-instrument.cpp`).
   Runs after parameter binding has assigned the coverage buffer a
   binding slot, but before `collectGlobalUniformParameters` packs
   it into the `GlobalParams` struct. The pass:
   - Locates the coverage buffer by name (always present post-AST-
     synthesis). A user-declared buffer with the wrong type produces
     a `Diagnostics::CoverageBufferWrongType` warning (E45100)
     anchored at the user's declaration; instrumentation is then
     skipped cleanly.
   - Assigns a counter slot to each `IncrementCoverageCounter` op
     (per-inst UID — consecutive index in traversal order; multiple
     ops on the same source line get distinct slots, which keeps the
     door open for branch/function coverage later).
   - Rewrites each op as `AtomicAdd(__slang_coverage[slot], 1,
     Relaxed)`.
   - Records `(slot → file, line)` plus the buffer's binding on the
     artifact's `ICoverageTracingMetadata`. A slot is unattributable
     when its `IncrementCoverageCounter` op carried an invalid
     `sourceLoc`, or one whose humane-loc has no positive line —
     typically because the underlying statement came from code
     synthesis (autodiff reverse-mode, generic specialization,
     struct constructor synthesis) rather than user-authored
     source. The LCOV converter skips these slots so the report
     stays line-oriented; consumers reading
     `ICoverageTracingMetadata` directly see them as null
     `getEntryFile` / zero `getEntryLine`.

4. **Emission.** Each backend already handles `kIROp_AtomicAdd` on
   `RWStructuredBuffer<uint>`:
   - HLSL/DXIL → `InterlockedAdd`
   - SPIR-V → `OpAtomicIAdd`
   - GLSL → `atomicAdd`
   - Metal → Metal atomic builtins
   - WGSL → `atomicAdd`
   - CUDA → `atomicAdd`
   - CPU → `_slang_atomic_add_u32` prelude helper (GCC/Clang
     `__atomic_fetch_add`, MSVC `_InterlockedExchangeAdd`)

The `IncrementCoverageCounter` op is side-effectful by default in
DCE analysis, so it survives optimizations untouched until the
coverage pass rewrites it.

Where each stage lives
----------------------

| Path | Role |
|---|---|
| `source/slang/slang-check-synthesize-coverage.{h,cpp}` | Injects `__slang_coverage` `VarDecl` during semantic check; walks transitively imported modules to dedupe |
| `source/slang/slang-check-decl.cpp` | Hook that invokes the synthesizer from `checkModule` |
| `source/slang/slang-ir-coverage-instrument.{h,cpp}` | IR pass — rewrites counter ops, writes metadata, diagnoses wrong-type user buffer |
| `source/slang/slang-ir-insts.lua` | Declares the `IncrementCoverageCounter` IR op |
| `source/slang/slang-lower-to-ir.cpp` | Emits counter ops during AST lowering; filters `BlockStmt` / `SeqStmt` / `EmptyStmt` |
| `source/slang/slang-emit.cpp` | Integrates the pass into the pipeline + allocates metadata |
| `source/slang/slang-options.cpp` | Registers the `-trace-coverage` and `-trace-coverage-binding` CLI flags |
| `source/slang/slang-diagnostics.lua` | `Diagnostics::CoverageBufferWrongType` (warning E45100) |
| `source/slang/slang-end-to-end-request.cpp` | Writes the `.coverage-mapping.json` sidecar from slangc |
| `include/slang.h` | `slang::ICoverageTracingMetadata` public interface |
| `source/compiler-core/slang-artifact-associated-impl.{h,cpp}` | `ArtifactPostEmitMetadata` implements the interface |
| `prelude/slang-cpp-prelude.h` | CPU-target atomic helpers (`_slang_atomic_add_u32/i32`) |
| `source/slang/slang-emit-cpp.cpp` | CPU emitter's `kIROp_AtomicAdd` handling |
| `tests/language-feature/coverage/` | End-to-end tests |
| `tools/slang-unit-test/unit-test-descriptor-set-space-offset-reflection.cpp` | Reflection unit test for `DescriptorSetInfo::spaceOffset` (regression-watch for the non-zero space mis-binding bug) |

Two reporting channels
----------------------

Coverage metadata is exposed through two channels because two
distinct audiences need it in two distinct shapes. The metadata API
is the canonical form; the sidecar is its on-disk serialization,
generated from the API at `slangc`'s request — there's no second
producer that could drift out of sync.

### In-process consumers — `ICoverageTracingMetadata`

In-process consumers are programs that compile and dispatch the
shader in the same process: test runners, in-engine compile
pipelines, slangpy bindings, JIT compilers. They hold the compiled
artifact in memory, the canonical metadata is right there, and they
need typed access without going through serialization, schema
versioning, or file I/O. `ICoverageTracingMetadata` is exactly that
shape: a query interface on the artifact's `IMetadata`, returning
counter count, per-slot `(file, line)`, and the chosen
`(space, binding)`.

Today the in-tree consumers are:

- **`slangc` itself**, queried by `_maybeWriteCoverageMapping` to
  produce the sidecar. This is what makes the metadata API the
  canonical form — `slangc` is its own first consumer.
- **`examples/shader-coverage-demo`**'s compile-API path
  (`writeManifestFromMetadata` in `main.cpp`).

The intended longer-term audience is in-process integrators that
don't exist in-tree today: slangpy bindings, in-engine compile
pipelines, custom test runners that JIT-compile shaders. They're
the audience the API shape is designed for.

### Cross-process / offline consumers — `<output>.coverage-mapping.json`

The other audience runs *later*, possibly on a different machine,
without Slang linked: a runtime dispatching the precompiled shader;
a CI script processing test output; a Python tool converting to
LCOV; a custom dashboard. By the time these consumers run, the
`slangc` invocation that produced the artifact has long since
exited, so the in-process API is unreachable. They need the
metadata frozen to disk in a language-agnostic format. The
`<output>.coverage-mapping.json` sidecar is that disk form.

This is the pre-built-shader pattern: shaders are compiled offline,
binaries plus sidecars ship with the game engine, DCC tool, or
vendor runtime, and a later dispatch on the end user's machine
attributes counters back to source using the sidecar alone — no
Slang toolchain needed on the target. The same companion-file role
is played by `.pdb` files for native Windows binaries and `.dSYM`
bundles for Mach-O: build-time information preserved for
runtime/post-mortem use.

Today the in-tree consumers are:

- **`tools/shader-coverage/slang-coverage-to-lcov.py`** — Python
  converter. Reads the sidecar plus a counter-buffer snapshot,
  emits LCOV consumable by `genhtml`, `reportgenerator`, VS Code
  Coverage Gutters, Codecov.
- **`source/slang-coverage-rt/`** — C helper library.
  `slang_coverage_create(path, ...)` parses the sidecar so hosts
  can accumulate hits and emit reports programmatically without
  linking the Slang compiler.

The intended longer-term audience is external integrators and CI
pipelines that need to attribute coverage values without linking
against Slang: Codecov adapters, in-engine coverage exporters,
language bindings that don't ship with the compiler. None of those
exist in-tree today; the file format is shaped for them.

### Rule of thumb

- `slangc` CLI workflow, consumer runs later → sidecar
- compile-API workflow, consumer runs in-process → metadata API

Either channel alone would force the wrong cost on one audience.
Sidecar-only would make in-process integrators round-trip through
disk and a JSON parser to get data they could otherwise get from a
typed virtual call — and `slangc` itself would become a consumer of
its own output file rather than a producer-via-API. API-only would
force every offline workflow (Python tools, CI scripts, custom
dashboards, any non-Slang-linked tool) to embed a Slang process to
query the metadata, defeating the point of shipping precompiled
shaders without the toolchain.

Alternative designs
-------------------

Other designs are possible. For example, a future API could allow some
hosts to bind the coverage buffer through explicit `(space, binding)`
information without requiring the resource to appear in reflection.

That would change the tradeoff for some integrations, but it would solve
a different problem from source attribution metadata. It would also need
to account for hosts and tools that currently rely on reflection-visible
resources.

This document does not treat the current phase-1 design as the only
possible long-term design. It records the approach chosen for the
current implementation and the constraints that made it a practical fit.

Design direction for phase 1
----------------------------

The phase-1 implementation therefore uses this split:

- AST-time synthesis for binding and discoverability
- `ICoverageTracingMetadata` for reporting and attribution

That separation is intended to make incremental coverage work practical
without committing later phases to one specific host integration model.

Roadmap beyond phase 1
----------------------

Phase 1 ships line coverage end-to-end across all five Slang backends.
The directions below are the work that's been scoped but deferred
out of this version. The list is grouped by category, not strictly
by ordering; per-test attribution and branch coverage are the
highest-leverage near-term picks.

### New LCOV record types

Capabilities the LCOV format already names; phase 1 doesn't yet emit
them. Required features to add.

- **Branch coverage** (`BRDA:` records). Per-branch-arm counters;
  the existing per-inst slot model is forward-compatible.
- **Function coverage** (`FN:` / `FNH:` records). Per-entry-point
  counter.
- **Per-test attribution** (`TN:` groupings). Extends
  `slang_coverage_accumulate` with an optional test name. Turns
  coverage from a flat aggregate into a test-quality signal —
  often the highest-leverage next feature for teams running
  coverage in CI.
- **Specialization-aware metadata.** Per-specialization counter
  attribution for codebases with heavy generic / template use —
  matters most for neural-slang.

### Cross-repo follow-ups

Tracked outside this repository:

- **slang-rhi multi-descriptor-set support.** Closes the Vulkan /
  WebGPU non-zero-`space` gap documented in *Current limitations*
  above. Compiler-side fix landed in phase 1; the slang-rhi half
  is the remaining work.
  [shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959).
- **slang-rhi raw-binding API.** Would let hosts bind the coverage
  buffer via explicit `(space, binding)` from the metadata API
  without going through reflection. If it lands, AST-time
  synthesis in phase 1 becomes optional rather than load-bearing.
  (optional)

### GPU-specific differentiators (optional)

Capabilities that don't exist elsewhere in the shader coverage /
profiling ecosystem.

- **Hot-line profiling.** Sample the same counters over time for
  per-line heatmaps. No external profiler install required;
  cross-vendor.
- **Per-warp / per-thread attribution.** Divergence coverage on
  CUDA / SPIR-V; counters become 2D (slot × lane).
- **Native vendor-tool exports.** Nsight, RenderDoc, PIX — so
  graphics engineers see coverage in tools they already use.

### Tooling (optional)

Adoption-helping additions, not gating any feature work:

- Codecov / Coveralls upload shim — one-liner CI recipe for
  pushing per-PR coverage to SaaS dashboards.
- Python bindings for `ICoverageTracingMetadata` + pytest plugin
  for slangpy users (unified Python + shader coverage in one
  report).
