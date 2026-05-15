Shader Coverage Design
======================

This document describes the shader coverage implementation in Slang
and the role of the main pieces in the pipeline.

Overview
--------

Shader coverage has two separate jobs:

1. insert execution counters into generated shader code
2. let the host discover the counter buffer and map counter slots back
   to source locations

These are handled by two different mechanisms:

- IR-time synthesis of `RWStructuredBuffer<uint> __slang_coverage`
  in the `slang-ir-coverage-instrument` pass
- post-emit metadata exposed as `ICoverageTracingMetadata` for
  today's line-compatible counter attribution view and
  `ISyntheticResourceMetadata` for hidden resource binding

The first is about getting the buffer into the compiled shader. The
second is about letting the host discover where the buffer lives
through synthetic-resource metadata and attribute counter values back
to source through coverage metadata.

Binding and attribution
-----------------------

Coverage instrumentation has two jobs that don't reduce to one
primitive:

- **Binding** — getting the host to allocate a GPU buffer of the
  right size and put it where the shader expects. The synthesized
  `__slang_coverage` buffer is created inside the IR coverage pass
  rather than via an AST decl. This keeps it out of Slang's public
  reflection surface (no synthetic decl leaking into IDE / language-
  server / `IComponentType::getLayout()` views) and produces exactly
  one buffer per linked program by construction. The binding the buffer
  ends up at is reported via `ISyntheticResourceMetadata`, and hosts
  use that information to declare the slot in their own pipeline-layout
  / root-signature / descriptor-set machinery.

- **Attribution** — turning counter values back into source
  locations. Reflection doesn't carry "slot 7 →
  `physics.slang:22`"; that's not what reflection is about.
  Reflection knows parameter names, types, and layouts. We need a
  side-channel that records the per-slot semantic intent for the
  current line coverage mode.
  `ICoverageTracingMetadata` (and its on-disk twin
  `.coverage-mapping.json`) is that channel.

The two channels carry complementary data — binding info answers
"where" and attribution answers "what." A host needs both: without
the binding it cannot allocate or bind the buffer; without the
attribution it cannot interpret the counter values it reads back.
The metadata is also intentionally a little richer than LCOV line
coverage — some slots may not map to a real source file and line,
and that fact is preserved in the metadata and JSON sidecar. The
LCOV conversion step then applies gcov-style reporting rules by
filtering those entries out of line-oriented output.

Future branch, function, and source-region coverage should grow this
attribution side of the design, not the binding side. In particular,
`ISyntheticResourceMetadata` should remain about hidden resource
binding, while richer source-based coverage metadata can describe how
runtime counters map to source regions, functions, and branch outcomes.

### Buffer synthesis at IR-pass time

Coverage is a runtime-instrumentation concern. It has no language-
visible behaviour, no surface in the source program, and no role in
semantic checking — counter ops are inserted by the front-end to
mark statement boundaries, and the buffer they target exists purely
to receive atomic increments at dispatch.

Putting buffer synthesis in the IR pass keeps it at the layer that
owns the rest of the feature:

- **One buffer per linked program** by construction. The pass runs
  on linked-program IR; there is no opportunity to produce more
  than one.
- **Not in user-facing reflection.** No synthetic decl appears in
  `IModule::getLayout()`, IDE completion, or language-server
  views. Hosts that need to bind the buffer consult
  `ISyntheticResourceMetadata`; hosts that need to interpret counter
  values consult `ICoverageTracingMetadata`.
- **Self-contained pass.** `slang-ir-coverage-instrument.cpp` owns
  buffer creation, layout assignment, target-policy selection,
  counter-op rewriting, and metadata recording. Disabling
  `-trace-coverage` keeps the rest of the compiler unaware of the
  feature's existence.

Hosts integrate by reading the hidden binding record from
`ISyntheticResourceMetadata` and declaring the slot in their own
pipeline-layout / root-signature / descriptor-set code, just like
they would for any user-declared resource — the metadata-driven
binding info is the canonical source for any compiler-synthesized
resource.

Pipeline architecture
---------------------

Enabling `-trace-coverage` runs three pipeline stages:

1. **AST lowering** (`source/slang/slang-lower-to-ir.cpp`). Before
   each *executable* statement is lowered to IR, the front-end
   emits an `IncrementCoverageCounter` IR op:
   - The op is opaque (zero operands, void return) — it doesn't
     reference a buffer at this point. The IR coverage pass will
     rewrite it later.
   - Purely structural compound statements (`BlockStmt`, `SeqStmt`,
     `EmptyStmt`) are filtered to keep counter density proportional
     to real execution events.
   - The op's source position rides on the standard per-instruction
     `sourceLoc` field — no operands, no debug decoration — so it
     survives `stripDebugInfo` and every IR transform that preserves
     operands (inline, clone, link).

2. **IR pass** (`source/slang/slang-ir-coverage-instrument.cpp`).
   Runs after `linkIR` has produced the linked-program IR, and
   before `collectGlobalUniformParameters` packs globals into the
   `GlobalParams` struct. The pass:
   - **Synthesizes the coverage buffer** as a fresh `IRGlobalParam`
     of type `RWStructuredBuffer<uint>`, with a target-aware layout:
     `UnorderedAccess` for D3D-style targets, `MetalBuffer` for
     Metal, `DescriptorTableSlot` for Khronos / SPIR-V / GLSL. For
     CPU and CUDA targets it additionally reports `Uniform` size,
     which the global-uniform packaging pass uses to fold the buffer
     into the standard `GlobalParams` struct. WGSL targets currently
     skip instrumentation entirely (warning E45102) until the
     synthesized type is wrapped in `Atomic<...>`.
   - **Picks a (set, binding)** — either honoring
     `-trace-coverage-binding <reg> <space>` if supplied, or
     auto-allocating a non-conflicting location for the chosen
     resource kind. Hosts can also pass
     `-trace-coverage-reserved-space <space>` one or more times to
     mark descriptor sets that belong to the runtime
     pipeline layout even if the compiled shader does not reference
     them. Duplicate reserved spaces are idempotent. The option applies
     to Khronos descriptor-set targets; Metal, CPU, CUDA, and D3D
     ignore it with a warning. On Khronos /
     SPIR-V / GLSL descriptor-set targets,
     auto-allocation picks the descriptor set after the highest
     shader-visible or host-reserved set and binds coverage at binding
     0 so the compiler does not mutate or fill holes in a user-owned
     set layout. D3D register-space reservation is left to a follow-up
     design so this PR does not freeze D3D allocation policy.
   - **Extends the program-scope var layout** to include the new
     buffer as a struct field so `collectGlobalUniformParameters`
     packs it alongside user globals on targets that pack ordinary
     uniforms (CPU, CUDA). Graphics targets don't pack and the
     extension is a no-op for them; the buffer flows through emit
     as a standalone `IRGlobalParam`.
   - **Assigns a counter slot to each `IncrementCoverageCounter`
     op** for the current exact line mode (per-inst UID —
     consecutive index in traversal order; multiple ops on the same
     source line get distinct slots and are aggregated by the LCOV
     exporter).
   - **Rewrites each op as `AtomicAdd(__slang_coverage[slot], 1,
     Relaxed)`**.
   - **Records `(slot → file, line)` on the artifact's
     `ICoverageTracingMetadata` and the synthesized buffer binding on
     `ISyntheticResourceMetadata`.** A slot is unattributable when its
     `IncrementCoverageCounter` op carried an invalid `sourceLoc`, or
     one whose humane-loc has no positive line — typically because the
     underlying statement came from code synthesis (autodiff reverse-
     mode, generic specialization, struct constructor synthesis)
     rather than user-authored source. The LCOV converter skips these
     slots so the report stays line-oriented; consumers reading
     `ICoverageTracingMetadata` directly see them as
     `entry.file == nullptr` / `entry.line == 0` after `getEntryInfo`.

3. **Emission.** Each backend already handles `kIROp_AtomicAdd` on
   `RWStructuredBuffer<uint>`:
   - HLSL/DXIL → `InterlockedAdd`
   - SPIR-V → `OpAtomicIAdd`
   - GLSL → `atomicAdd`
   - Metal → Metal atomic builtins
   - WGSL → not reached today; coverage instrumentation is skipped
     before rewrite for WGSL targets
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
| `source/slang/slang-ir-coverage-instrument.{h,cpp}` | IR pass — synthesizes the buffer, extends program-scope layout, rewrites counter ops, writes metadata |
| `source/slang/slang-ir-insts.lua` | Declares the `IncrementCoverageCounter` IR op |
| `source/slang/slang-lower-to-ir.cpp` | Emits counter ops during AST lowering; filters `BlockStmt` / `SeqStmt` / `EmptyStmt` |
| `source/slang/slang-emit.cpp` | Integrates the pass into the pipeline + allocates metadata + plumbs `-trace-coverage-binding` / `-trace-coverage-reserved-space` |
| `source/slang/slang-options.cpp` | Registers the `-trace-coverage`, `-trace-coverage-binding`, and `-trace-coverage-reserved-space` CLI flags |
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
versioning, or file I/O. The artifact's `IMetadata` exposes the two
query interfaces needed by the host: `ICoverageTracingMetadata`
returns the current line-compatible counter count and per-slot
`(file, line)`, while
`ISyntheticResourceMetadata` returns the chosen hidden-resource
binding.

A companion free function — `slang_writeCoverageManifestJson` —
serializes an `ICoverageTracingMetadata` to the canonical
`.coverage-mapping.json` shape on demand. When the same metadata object
also supports `ISyntheticResourceMetadata` (the normal Slang artifact
case), the serializer includes the buffer binding fields as well:
`space` / `binding` for descriptor-backed targets and
`uniform_offset` / `uniform_stride` for CPU/CUDA uniform-marshaling
targets when those locations are available.
Hosts that want the sidecar bytes without going through disk (to feed
the Python LCOV converter or a network channel) call it directly;
hosts that consume the typed accessors don't need it.

Today the in-tree consumer is:

- **`slangc` itself** — its `_maybeWriteCoverageMapping` calls
  `slang_writeCoverageManifestJson` and writes the bytes to disk.
  This is what makes the metadata API + manifest shape *one*
  contract, not two: `slangc` is its own first consumer of the
  public serializer.

A reference end-to-end host integration that uses both the typed
metadata path and `slang_writeCoverageManifestJson` is planned as a
follow-up.

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

Today the in-tree consumer is:

- **`tools/shader-coverage/slang-coverage-to-lcov.py`** — Python
  converter. Reads the sidecar plus a counter-buffer snapshot,
  emits LCOV consumable by `genhtml`, `reportgenerator`, VS Code
  Coverage Gutters, Codecov.

A helper library for hosts that want to accumulate hits and emit
reports programmatically without linking the Slang compiler is a
possible follow-up.

The intended longer-term audience is external integrators and CI
pipelines that need to attribute coverage values without linking
against Slang: Codecov adapters, in-engine coverage exporters,
language bindings that don't ship with the compiler. The file format
is shaped for them.

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

Host integration workflows
--------------------------

Two equally-supported workflows, each suited to a different host
architecture. In-process hosts query `ICoverageTracingMetadata` for
counter attribution and `ISyntheticResourceMetadata` for binding;
offline hosts consume the serialized sidecar produced from those same
metadata interfaces.

### A. In-process compile (Slang C++ API)

Audience: applications that compile shaders at runtime via Slang's
C++ API — production engines, custom shader runtimes, content-
creation applications, compute applications.

The host queries metadata directly from the compiled artifact, reads
the hidden resource binding from `ISyntheticResourceMetadata` and
per-slot `(file, line)` attribution from `ICoverageTracingMetadata`,
and declares the slot in its own pipeline-layout / root-signature code.
No file I/O is involved; the
`.coverage-mapping.json` sidecar is not produced or read.

The host is free to consume the per-slot attribution in whatever
shape suits it — write its own LCOV, feed an internal dashboard,
log directly to telemetry, etc.

### B. Precompiled with sidecar (slangc CLI)

Audience: workflows that compile shaders offline (typically via
`slangc`) and dispatch them later, possibly on a different machine
or in a process where Slang isn't linked. Game engines shipping
with prebuilt shaders, vendor runtimes, CI pipelines.

`slangc` writes `<output>.coverage-mapping.json` next to each
compiled artifact when `-trace-coverage` is on. The sidecar is the
on-disk serialization of the coverage attribution plus synthetic
resource binding metadata. The dispatching host reads the sidecar,
declares the slot in its pipeline layout, and proceeds as in workflow
A. The Python LCOV converter under `tools/shader-coverage/` consumes
the same sidecar later when converting readback counters to LCOV.

### Convenience layers (planned follow-up)

A helper library could provide a C ABI that handles
`.coverage-mapping.json` parsing, counter accumulation across
dispatches, and LCOV serialization for hosts that want LCOV output
without implementing the format themselves. It would not be required
for either workflow above.

Roadmap
-------

The current implementation provides line coverage instrumentation
end-to-end across all Slang backends. Directions scoped for follow-
up work, grouped by category. Per-test attribution and branch
coverage are the highest-leverage near-term picks.

The current `ICoverageTracingMetadata` shape is intentionally a
line-compatible view. Branch, function, and lower-density region
coverage are expected to need a richer source-based metadata model
where runtime counters can map to source ranges, functions, and branch
outcomes. LCOV should remain a compatibility export (`DA:`,
`FN/FNDA:`, `BRDA:`), not the only internal coverage model.

### New LCOV record types

Capabilities the LCOV format already names that the current
implementation does not yet emit.

- **Branch coverage** (`BRDA:` records). Per-branch-arm counters,
  represented through richer source coverage metadata before export.
- **Function coverage** (`FN:` / `FNH:` records). Per-function-entry
  counters with function names and source ranges.
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

- **slang-rhi multi-descriptor-set support.** Required for hosts
  that dispatch via slang-rhi to use non-zero `space` values.
  Hosts using their own pipeline-layout code are unaffected.
  [shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959).

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
