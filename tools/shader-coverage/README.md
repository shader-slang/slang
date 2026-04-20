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
   `IncrementCoverageCounter(uid)` IR placeholder, tagged with an
   `IRDebugLocationDecoration` carrying the statement's source
   position. UIDs increment monotonically from zero.
2. **IR pass** (`source/slang/slang-ir-coverage-instrument.cpp`).
   Before the user's module-scope globals are collected into a uniform
   struct, the pass:
   - Synthesizes an implicit `RWStructuredBuffer<uint> __slang_coverage`
     at a reserved register binding (space 31, binding 0) — unless the
     user already declared a buffer with that exact name, in which case
     it reuses theirs.
   - Walks every `IncrementCoverageCounter` placeholder, reads its
     attached source location, dedupes into a `(file, line) → counter
     index` map, and rewrites each placeholder as
     `AtomicAdd(__slang_coverage[counterIdx], 1, Relaxed)`.
   - Writes a JSON manifest (when `SLANG_COVERAGE_MANIFEST_PATH` is
     set) and prints a `slang-coverage-info: counters=<N>` line to
     stderr so the host knows the buffer size.
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

Because `IncrementCoverageCounter` defaults to side-effectful in the
DCE analysis, placeholders survive optimizations untouched. Debug
info is usually stripped early when the user hasn't asked for `-g`;
when `-trace-coverage` is on, that strip is deferred until after our
pass has read the source locations off the placeholders.

---

## Runtime flow

```
┌─────────────────┐  -trace-coverage      ┌──────────────────────┐
│  shader.slang   │ ────────────────────► │  target code + sidecar
└─────────────────┘                       │  shader.slangcov      │
                                          └──────────────────────┘
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

### Quick start

```bash
# 1. Compile with coverage on.
SLANG_COVERAGE_MANIFEST_PATH=build/shader.slangcov \
slangc shader.slang \
    -target spirv \
    -stage compute -entry main \
    -trace-coverage \
    -o build/shader.spv
# stderr: `slang-coverage-info: counters=<N>`

# 2. Allocate a uint32[N] buffer on the host.  Bind it at the
#    register/binding found in `shader.slangcov` → "buffer".  Dispatch.
#    After dispatch, read the UAV back to `build/shader.buffer.bin`.

# 3. Convert and render.
python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest build/shader.slangcov \
    --counters build/shader.buffer.bin \
    --output   build/shader.lcov
genhtml build/shader.lcov -o build/shader-cov-html/
```

---

## Flag and environment reference

| Interface | Effect |
|---|---|
| `-trace-coverage` (CLI) | Enables the feature. Emits placeholders during lowering; synthesizes the buffer (if not user-declared); rewrites placeholders to atomic increments. |
| `SLANG_COVERAGE_MANIFEST_PATH` (env) | Writes a JSON sidecar at the given path describing each counter's source location and the buffer's binding. Unset → no sidecar. |
| `SLANG_COVERAGE_DUMP_MANIFEST` (env) | When set to any value, also prints `slang-coverage-manifest: <index>,<file>,<line>` lines to stderr. |
| stderr `slang-coverage-info: counters=<N>` | Always printed when `-trace-coverage` is active. Tells the host the counter-buffer size. |

---

## Sidecar format (`.slangcov`)

Plain JSON, version 1:

```json
{
  "version": 1,
  "counters": 6,
  "buffer": {
    "name": "__slang_coverage",
    "element_type": "uint32",
    "element_stride": 4,
    "synthesized": true,
    "space": 31,
    "binding": 0,
    "descriptor_set": 31
  },
  "entries": [
    { "index": 0, "file": "shader.slang", "line": 21 },
    { "index": 1, "file": "shader.slang", "line": 22 },
    { "index": 2, "file": "shader.slang", "line": 24 },
    { "index": 3, "file": "shader.slang", "line": 25 },
    { "index": 4, "file": "shader.slang", "line": 27 },
    { "index": 5, "file": "shader.slang", "line": 29 }
  ]
}
```

- `counters` — length of the flat `uint32` counter buffer.
- `buffer.space` / `buffer.binding` — Vulkan-style descriptor set +
  binding (populated when the pass has SPIR-V-shaped layout info).
- `buffer.uav_register` — HLSL-style `u<N>` register (populated for
  D3D12).
- `buffer.synthesized` — `true` when Slang created the buffer,
  `false` when the user declared it in the shader.
- `entries` — one per counter, in index order. Multiple entries may
  share the same `(file, line)` when several statements live on one
  source line; their hits are summed at the LCOV conversion step.

## Counter buffer format

`uint32_t counters[N]` — flat little-endian array. No header, no
version prefix. `N` is the `counters` field from the sidecar.

---

## The converter — `slang-coverage-to-lcov.py`

```
--manifest <file.slangcov>       (required)
--counters <file.bin>            Binary uint32 little-endian
  OR
--counters-text <file-or-'->     Whitespace-separated decimal ints
                                  ('-' reads stdin)
--output <file.lcov>             Default: stdout
--test-name <name>               Default: 'slang_coverage' (LCOV
                                 disallows hyphens in test names)
```

Binary mode is the normal path after a real GPU dispatch. Text mode
is a convenience for testing — pipe `slang-test` buffer output
directly in:

```bash
./build/Debug/bin/slang-test tests/my-coverage-test.slang 2>&1 \
  | awk '/^type: uint32_t/{p=1; next} p && /^[0-9]+$/{print}' \
  | python3 tools/shader-coverage/slang-coverage-to-lcov.py \
      --manifest build/test.slangcov \
      --counters-text - \
      --output build/test.lcov
```

---

## Example output

For a small compute shader with a 4-iteration loop containing an
if/else:

```
TN:slang_coverage
SF:tests/language-feature/coverage/coverage-basic.slang
DA:21,3
DA:22,2
DA:24,8
DA:25,2
DA:27,2
DA:29,1
end_of_record
```

`lcov --summary` reports `lines: 100.0% (6 of 6 lines)`; `genhtml`
produces a per-line-annotated report in the usual gcov style.

Each counter is the number of times a statement on that line was
entered, summed across all shader invocations that ran. Because
multiple statements on the same source line share a counter slot
(dedupe by `(file, line)`), the displayed hit count may exceed the
number of invocations.

---

## Current scope

- Line coverage only — emits `DA:` records; no `FN:` / `BRDA:`
  (function / branch) coverage yet.
- Column position from the placeholder's debug decoration is dropped
  when the sidecar is written; only `(file, line)` reaches LCOV.
- Counter type is `uint32`; will saturate at 4 × 10⁹ hits.
- Manifest delivery is via file sidecar + stderr; no embedded-compiler
  API for querying UID → source location yet.
- Sidecar path must be supplied via `SLANG_COVERAGE_MANIFEST_PATH`;
  deriving it from `-o` automatically is a natural follow-up.

---

## Related files in the Slang tree

| Path | Role |
|---|---|
| `source/slang/slang-ir-coverage-instrument.{h,cpp}` | The IR pass — synthesizes buffer, rewrites placeholders, writes manifest |
| `source/slang/slang-ir-insts.lua` | Declares `IncrementCoverageCounter(uid)` IR op |
| `source/slang/slang-lower-to-ir.cpp` | Emits placeholders during AST lowering |
| `source/slang/slang-emit.cpp` | Integrates the pass into the pipeline |
| `source/slang/slang-options.cpp` | Registers `-trace-coverage` CLI flag |
| `prelude/slang-cpp-prelude.h` | CPU-target atomic helpers (`_slang_atomic_add_u32/i32`) |
| `source/slang/slang-emit-cpp.cpp` | CPU emitter's handling of `kIROp_AtomicAdd` |
| `tests/language-feature/coverage/` | End-to-end tests |
