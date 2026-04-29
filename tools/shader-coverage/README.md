# Shader Coverage Instrumentation

A gcov-style coverage facility for shaders compiled by Slang. Instruments
a `.slang` shader so that each executed source statement increments a
counter at runtime; the counter buffer is read back by the host and
converted to LCOV `.info` for rendering by `genhtml`, Codecov, VS
Code Coverage Gutters, or any other LCOV consumer.

For the maintainer-facing architectural rationale behind the current
design, including why AST-time synthesis is paired with post-emit
coverage metadata, see
[`docs/design/shader-coverage.md`](../../docs/design/shader-coverage.md).

Not to be confused with `tools/coverage/`, which measures C++ coverage
of the Slang compiler itself.

---

## Quick start

Compile any `.slang` shader with `-trace-coverage`:

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -o shader.spv
# Produces shader.spv plus shader.spv.coverage-mapping.json
```

Every executable statement in the shader gets instrumented to
increment a counter at runtime. The compiler synthesizes a
`RWStructuredBuffer<uint> __slang_coverage` parameter that any
reflection-driven host (slang-rhi, slangpy, custom Vulkan/D3D12
wrappers) discovers and binds the same way as any other shader
parameter. The `.coverage-mapping.json` sidecar tells the host how
many counters to allocate and which source line each slot
corresponds to.

After the host dispatches the shader and reads the counter buffer
back, [`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py)
or [`slang-coverage-rt`](../../source/slang-coverage-rt/) converts
the snapshot to LCOV `.info` for `genhtml`, Codecov, VS Code
Coverage Gutters, etc.

For the pipeline architecture, design rationale, and alternatives
weighed, see
[`docs/design/shader-coverage.md`](../../docs/design/shader-coverage.md).

## Pinning the coverage buffer at an explicit slot

By default parameter binding auto-allocates a slot for
`__slang_coverage`. Pass `-trace-coverage-binding <index> <space>`
to pin it at a specific `(register, space)` pair instead:

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage-binding 7 0 -o shader.spv
# __slang_coverage lands at register(u7) on HLSL,
# DescriptorSet 0 / Binding 7 on SPIR-V.
```

`-trace-coverage-binding` implies `-trace-coverage`. The override
is ignored (with the user's declaration winning) when the user has
already declared a `__slang_coverage` themselves. Use this when the
host needs the slot fixed at compile time — for example when
pre-building a D3D12 root signature before reflection runs.

---

## Accessing the manifest

Two paths, both carrying the same raw per-slot attribution data plus
the coverage buffer's binding. A slot may have no real source file/line;
that is preserved in metadata and filtered out later when exporting LCOV.

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

`slang-coverage-to-lcov.py` applies gcov/LCOV-style reporting rules at
export time: entries without a real source file or with a non-positive
line number are skipped instead of being written as synthetic `SF:` /
`DA:` records.

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
| `-trace-coverage-binding <index> <space>` | Pins the synthesized `__slang_coverage` buffer at the explicit `(register index, space)` pair, instead of letting parameter binding auto-allocate. Implies `-trace-coverage`. Useful when the host needs the slot fixed at compile time (e.g. for a pre-built D3D12 root signature). Ignored if the user already declares `__slang_coverage` themselves; the user declaration wins. |

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
counts together. Entries that do not resolve to a real source file
and positive source line are skipped to match normal gcov/LCOV line
coverage semantics.

---

## Supported features

| Backend | Default `-trace-coverage` | `-trace-coverage-binding=N:0` | `-trace-coverage-binding=N:M` (M ≠ 0) |
|---|---|---|---|
| CPU | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Vulkan (incl. MoltenVK on macOS) | Supported | Supported | Skipped — slang-rhi follow-up pending |
| D3D12 | Supported | Supported | Supported (compiler reflection fix landed) |
| CUDA | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Metal (direct) | Pre-existing slang-rhi binding-init quirk; counter values unreliable. Tracked at [shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724). Use Vulkan via MoltenVK on Apple silicon. | (untested) | (untested) |
| GLSL / WebGPU | Supported codegen | (untested) | (untested) |

LCOV output is byte-identical across CPU / Vulkan / D3D12 / CUDA in
the supported cells.

### Format scope

- **Line coverage only** — emits `DA:` records; no `FN:` / `BRDA:`
  (function / branch) coverage yet. Per-inst slot assignment is
  forward-compatible with branch coverage: each branch point would
  get its own slot, and the LCOV emitter would grow a `BRDA:`
  writer.
- **Column position is dropped.** Only `(file, line)` reaches LCOV.
- **Counter type is `uint32`.** Saturates at ~4 × 10⁹ hits per
  slot. Multiple ops on the same source line accumulate
  independently before LCOV-emit-time aggregation.

## Current limitations

- **`-trace-coverage-binding=N:M` runtime status with `M != 0`** is
  partially wired through. The compiler now emits the correct
  `(set, register)` decoration — the Slang reflection bug that
  used to leave `DescriptorSetInfo::spaceOffset = 0` is fixed —
  so D3D12's slang-rhi root-signature builder accepts the layout
  and dispatch works end-to-end. Vulkan and WebGPU remain pending
  a slang-rhi follow-up: their binding-data builder assumes ≤ 1
  descriptor set per shader object, so non-zero space mis-binds.
  The companion `examples/shader-coverage-demo` skips Vulkan /
  WebGPU dispatch with a clear `[coverage] skip` message in this
  case rather than letting the assertion fire. Tracked at
  [shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959).
- **Metal counter values** are unreliable due to a pre-existing
  slang-rhi Metal binding/initialization quirk — atomic writes
  appear to land in the wrong buffer. Not a coverage-feature bug;
  tracked at
  [shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724).
  On Apple silicon, use Vulkan via MoltenVK.

