# Shader Coverage Instrumentation

A gcov-style coverage facility for shaders compiled by Slang. Instruments
a `.slang` shader so that each executed source line increments a
counter at runtime; the counter buffer is read back by the host and
converted to LCOV `.info` for rendering by `genhtml`, Codecov, VS
Code Coverage Gutters, or any other LCOV consumer.

Not to be confused with `tools/coverage/`, which measures C++ coverage
of the Slang compiler itself.

---

## Compiler-side flow

Enable with `-trace-coverage` on the `slangc` CLI.

1. **AST lowering** (`source/slang/slang-lower-to-ir.cpp`). Before each
   statement is lowered to IR, the front-end emits an
   `IncrementCoverageCounter` IR op. Its source position is carried on
   the standard per-instruction `sourceLoc` field — no operands, no
   debug decoration, so the op is independent of debug-info state.
2. **IR pass** (`source/slang/slang-ir-coverage-instrument.cpp`).
   Before the user's module-scope globals are collected into a uniform
   struct, the pass:
   - Synthesizes an implicit `RWStructuredBuffer<uint> __slang_coverage`
     at a reserved register binding (space 31, binding 0).
   - Collects every `IncrementCoverageCounter` op, deduplicates by
     `(file, line)`, sorts the unique keys lexicographically, and
     assigns a counter slot index to each.
   - Rewrites each op as `AtomicAdd(__slang_coverage[counterIdx], 1,
     Relaxed)`.
3. **Emission.** Each backend already handles `kIROp_AtomicAdd` on
   `RWStructuredBuffer<uint>`:
   - HLSL/DXIL → `InterlockedAdd`
   - SPIR-V → `OpAtomicIAdd`
   - GLSL → `atomicAdd`
   - Metal → Metal atomic builtins
   - WGSL → `atomicAdd`
   - CUDA → `atomicAdd`
   - CPU → `_slang_atomic_add_u32` prelude helper (GCC/Clang
     `__atomic_fetch_add`, MSVC `_InterlockedExchangeAdd`)

The `IncrementCoverageCounter` op is side-effectful by default in the
DCE analysis, so it survives optimizations untouched until the coverage
pass rewrites it.

Slot assignment is **stable** across unrelated source edits: slot N is
the Nth `(file, line)` after lexicographic sort. Adding a statement in
file A line 50 only shifts slots for file A lines ≥ 50; slots for
unrelated files are unchanged.

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

> **Status:** the `counters=N` count and `(slot → file, line)`
> mapping must currently be reconstructed by the host from its own
> knowledge of the shader source. A proper post-emit-metadata API
> exposing this info via `IArtifactPostEmitMetadata` is planned; see
> "Pending work" below.

---

## CLI reference

| Flag | Effect |
|---|---|
| `-trace-coverage` | Enables the feature. Synthesizes the coverage buffer at a reserved binding; rewrites counter ops to atomic increments. |

---

## Counter buffer format

`uint32_t counters[N]` — flat little-endian array, no header. Indexed
by slot (see slot assignment above).

---

## The converter — `slang-coverage-to-lcov.py`

```
--manifest <file.slangcov>       Slot → (file, line) mapping supplied
                                 by the host (see "Pending work").
--counters <file.bin>            Binary uint32 little-endian
  OR
--counters-text <file-or-'->     Whitespace-separated decimal ints
                                 ('-' reads stdin)
--output <file.lcov>             Default: stdout
--test-name <name>               Default: 'slang_coverage' (LCOV
                                 disallows hyphens in test names)
```

---

## Current scope

- Line coverage only — emits `DA:` records; no `FN:` / `BRDA:`
  (function / branch) coverage yet.
- Column position is dropped; only `(file, line)` reaches LCOV.
- Counter type is `uint32`; saturates at ~4 × 10⁹ hits.

## Pending work

- **Post-emit metadata API.** Manifest data (counter count, chosen
  binding, `slot → file/line` map) is internally available after the
  coverage pass runs but not yet exposed to compiler clients. The
  planned path is a new accessor on `IArtifactPostEmitMetadata` so
  hosts can query it via the standard reflection/metadata API.
- **Configurable binding.** The reserved `(space=31, binding=0)` is a
  prototype default. A future `-coverage-binding` CLI option (or
  equivalent) will let integrators pick a non-conflicting slot.

---

## Related files in the Slang tree

| Path | Role |
|---|---|
| `source/slang/slang-ir-coverage-instrument.{h,cpp}` | The IR pass — synthesizes buffer, rewrites counter ops |
| `source/slang/slang-ir-insts.lua` | Declares `IncrementCoverageCounter` IR op |
| `source/slang/slang-lower-to-ir.cpp` | Emits counter ops during AST lowering |
| `source/slang/slang-emit.cpp` | Integrates the pass into the pipeline |
| `source/slang/slang-options.cpp` | Registers `-trace-coverage` CLI flag |
| `prelude/slang-cpp-prelude.h` | CPU-target atomic helpers (`_slang_atomic_add_u32/i32`) |
| `source/slang/slang-emit-cpp.cpp` | CPU emitter's handling of `kIROp_AtomicAdd` |
| `tests/language-feature/coverage/` | End-to-end tests |
