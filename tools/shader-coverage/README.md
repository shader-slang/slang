# Shader Coverage Instrumentation

A gcov-style coverage facility for shaders compiled by Slang. Instruments
a `.slang` shader so that each executed source statement increments a
counter at runtime; the counter buffer is read back by the host and
converted to LCOV `.info` for rendering by `genhtml`, Codecov, VS
Code Coverage Gutters, or any other LCOV consumer.

Not to be confused with `tools/coverage/`, which measures C++ coverage
of the Slang compiler itself.

---

## Compiler-side flow

Enable with `-trace-coverage` on the `slangc` CLI.

1. **AST-check time** (`source/slang/slang-check-synthesize-coverage.{h,cpp}`).
   A `RWStructuredBuffer<uint> __slang_coverage` `VarDecl` is synthesized
   in the module scope before parameter binding runs. This lets the
   buffer flow through Slang's normal reflection and layout pipeline,
   so every backend and every reflection-driven host (slang-rhi,
   slangpy, custom Vulkan/D3D12 hosts) sees it as a first-class
   shader parameter. Skipped if the user has already declared a
   `__slang_coverage` themselves.
2. **AST lowering** (`source/slang/slang-lower-to-ir.cpp`). Before
   each statement is lowered to IR, the front-end emits an
   `IncrementCoverageCounter` IR op. The op's source position is
   carried on the standard per-instruction `sourceLoc` field — no
   operands, no debug decoration — so it survives `stripDebugInfo`
   and every IR transform that preserves operands (inline, clone,
   link).
3. **IR pass** (`source/slang/slang-ir-coverage-instrument.cpp`).
   Runs after parameter binding has assigned the coverage buffer a
   binding slot, but before `collectGlobalUniformParameters` packs
   it into the `GlobalParams` struct. The pass:
   - Locates the coverage buffer by name (always present post-AST-
     synthesis).
   - Assigns a counter slot to each `IncrementCoverageCounter` op
     (per-inst UID — consecutive index in traversal order; multiple
     ops on the same source line get distinct slots, which keeps the
     door open for branch/function coverage later).
   - Rewrites each op as `AtomicAdd(__slang_coverage[slot], 1,
     Relaxed)`.
   - Records `(slot → file, line)` and the buffer's binding on the
     artifact's `ICoverageTracingMetadata` (see next section).
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
the DCE analysis, so it survives optimizations untouched until the
coverage pass rewrites it.

---

## Accessing the manifest

Two paths, both carrying the same `(slot → file, line)` mapping plus
the coverage buffer's binding.

### `.coverage-mapping.json` sidecar (slangc CLI)

When slangc writes a compiled artifact to a file, it also writes
`<output>.coverage-mapping.json` alongside whenever the artifact
carries coverage tracing data. Consumable by the Python LCOV
converter in this directory and any other external tool.

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -o shader.spv
# -> shader.spv
# -> shader.spv.coverage-mapping.json
```

### `slang::ICoverageTracingMetadata` (compile API)

For hosts using the compile API directly (slang-rhi, slangpy, custom
integrations), the same data is available on the artifact's
`IMetadata`:

```cpp
ComPtr<slang::IMetadata> metadata;
linked->getEntryPointMetadata(0, 0, metadata.writeRef(), ...);

auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
    slang::ICoverageTracingMetadata::getTypeGuid());

uint32_t n = coverage->getCounterCount();
int32_t space = coverage->getBufferSpace();
int32_t binding = coverage->getBufferBinding();
for (uint32_t i = 0; i < n; ++i) {
    const char* file = coverage->getEntryFile(i);
    uint32_t line    = coverage->getEntryLine(i);
}
```

Extensible: future revisions will add branch/function coverage and
column data through the same interface.

---

## Runtime flow

```
┌─────────────────┐  -trace-coverage      ┌──────────────────────┐
│  shader.slang   │ ────────────────────► │  target code (spv/…) │
└─────────────────┘                       └──────────────────────┘
                                                   │
                                    bind coverage  │
                                    buffer, run    ▼
                               ┌─────────────────────────────────┐
                               │ Host program                    │
                               │  · uint32 counters[N]           │
                               │  · dispatch                     │
                               │  · read UAV back                │
                               └─────────────────────────────────┘
                                                   │
                                                   ▼
                  slang-coverage-to-lcov.py  ┌─────────────┐
                                             │ shader.lcov │
                                             └─────────────┘
                                                   │
                                                   ▼
                                    genhtml, Codecov, VS Code, ...
```

The host binds the coverage buffer by its reflected name / binding
(standard reflection API) and reads the counter array back after
dispatch.

---

## CLI reference

| Flag | Effect |
|---|---|
| `-trace-coverage` | Enables the feature. Synthesizes `__slang_coverage` at AST-check time; rewrites counter ops to atomic increments; emits `<output>.coverage-mapping.json` sidecar when writing to a file. |

---

## Counter buffer format

`uint32_t counters[N]` — flat little-endian array, no header. Indexed
by slot. Saturates at ~4 × 10⁹ hits per slot (see *Current scope*).

---

## The converter — `slang-coverage-to-lcov.py`

```
--manifest <file.coverage-mapping.json>  Slot → (file, line) mapping.
                                 Produced by slangc alongside the
                                 compiled artifact, or hand-built from
                                 ICoverageTracingMetadata.
--counters <file.bin>            Binary uint32 little-endian
  OR
--counters-text <file-or-'->     Whitespace-separated decimal ints
                                 ('-' reads stdin)
--output <file.lcov>             Default: stdout
--test-name <name>               Default: 'slang_coverage' (LCOV
                                 disallows hyphens in test names)
```

Aggregates counter values by `(file, line)` at LCOV-emission time,
so multiple slots on the same source line contribute their hit
counts together.

---

## Current scope

- Line coverage only — emits `DA:` records; no `FN:` / `BRDA:`
  (function / branch) coverage yet. Per-inst slot assignment is
  forward-compatible with branch coverage: each branch point gets
  its own slot and the LCOV emitter would grow a `BRDA:` writer.
- Column position is dropped; only `(file, line)` reaches LCOV.
- Counter type is `uint32`; saturates at ~4 × 10⁹ hits per slot.

## Pending work

- **Configurable binding.** The binding assigned to `__slang_coverage`
  is whatever the normal parameter-binding phase picks. A future
  `-coverage-binding` CLI option would let integrators pin the slot
  explicitly, which matters for engines with fixed descriptor-set
  layouts.
- **Branch / function coverage.** `BRDA:` and `FN:` LCOV records,
  driven by extending `ICoverageTracingMetadata` with additional
  entry types and having the IR pass insert extra counter ops at
  branch points.

---

## Related files in the Slang tree

| Path | Role |
|---|---|
| `source/slang/slang-check-synthesize-coverage.{h,cpp}` | Injects `__slang_coverage` `VarDecl` during semantic check |
| `source/slang/slang-check-decl.cpp` | Hook that invokes the synthesizer from `checkModule` |
| `source/slang/slang-ir-coverage-instrument.{h,cpp}` | IR pass — rewrites counter ops, writes metadata |
| `source/slang/slang-ir-insts.lua` | Declares the `IncrementCoverageCounter` IR op |
| `source/slang/slang-lower-to-ir.cpp` | Emits counter ops during AST lowering |
| `source/slang/slang-emit.cpp` | Integrates the pass into the pipeline + allocates metadata |
| `source/slang/slang-options.cpp` | Registers the `-trace-coverage` CLI flag |
| `source/slang/slang-end-to-end-request.cpp` | Writes the `.coverage-mapping.json` sidecar from slangc |
| `include/slang.h` | `slang::ICoverageTracingMetadata` public interface |
| `source/compiler-core/slang-artifact-associated-impl.{h,cpp}` | `ArtifactPostEmitMetadata` implements the interface |
| `prelude/slang-cpp-prelude.h` | CPU-target atomic helpers (`_slang_atomic_add_u32/i32`) |
| `source/slang/slang-emit-cpp.cpp` | CPU emitter's `kIROp_AtomicAdd` handling |
| `tests/language-feature/coverage/` | End-to-end tests |
