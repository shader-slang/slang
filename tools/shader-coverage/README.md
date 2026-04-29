# Shader Coverage Instrumentation

A gcov-style coverage facility for shaders compiled by Slang. Instruments
a `.slang` shader so that each executed source statement increments a
counter at runtime; the counter buffer is read back by the host and
converted to LCOV `.info` for rendering by `genhtml`, Codecov, VS
Code Coverage Gutters, or any other LCOV consumer.

For the maintainer-facing architectural rationale behind the current
design, including why IR-time buffer synthesis is paired with post-
emit coverage metadata for binding-info propagation, see
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
`RWStructuredBuffer<uint> __slang_coverage` directly in the IR
coverage pass — no AST decl, so it does not appear in Slang's
public reflection. Hosts discover the buffer's `(set, binding)` via
`slang::ICoverageTracingMetadata` (or the `.coverage-mapping.json`
sidecar) and declare the slot in their own pipeline-layout / root-
signature / descriptor-set machinery before binding the counter
buffer at dispatch time. The same metadata tells the host how many
counters to allocate and which source line each slot corresponds
to.

After the host dispatches the shader and reads the counter buffer
back, [`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py)
or [`slang-coverage-rt`](../../source/slang-coverage-rt/) converts
the snapshot to LCOV `.info` for `genhtml`, Codecov, VS Code
Coverage Gutters, etc.

For the pipeline architecture, design rationale, and alternatives
weighed, see
[`docs/design/shader-coverage.md`](../../docs/design/shader-coverage.md).

## Pinning the coverage buffer at an explicit slot

By default the IR coverage pass auto-allocates a slot in space 0
for `__slang_coverage` that doesn't collide with any other global
parameter's offset. Pass `-trace-coverage-binding <index> <space>`
to pin it at a specific `(register, space)` pair instead:

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage-binding 7 0 -o shader.spv
# __slang_coverage lands at register(u7) on HLSL,
# DescriptorSet 0 / Binding 7 on SPIR-V.
```

`-trace-coverage-binding` implies `-trace-coverage`. Use this when
the host needs the slot fixed at compile time — for example when
pre-building a D3D12 root signature before any host reflection /
metadata reads run.

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

The host reads the coverage buffer's `(set, binding)` from
`slang::ICoverageTracingMetadata` (or the sidecar), declares the
slot in its pipeline-layout / root-signature, allocates and binds a
counter buffer at that slot, dispatches the shader, and reads the
counter array back. Hosts on slang-rhi additionally have
`ShaderProgramDesc::extraDescriptorBindings` and
`IShaderObject::setExtraBinding` to streamline this; Tier-1 hosts
on raw Vulkan / D3D12 / CUDA do the slot declaration directly in
their native binding API.

---

## CLI reference

| Flag | Effect |
|---|---|
| `-trace-coverage` | Enables the feature. The IR coverage pass synthesizes `__slang_coverage` as an `IRGlobalParam` directly in the linked program IR (no AST decl), rewrites counter ops to atomic increments, and emits `<output>.coverage-mapping.json` sidecar when writing to a file. |
| `-trace-coverage-binding <index> <space>` | Pins the synthesized `__slang_coverage` buffer at the explicit `(register index, space)` pair, instead of letting the IR pass auto-allocate. Implies `-trace-coverage`. Useful when the host needs the slot fixed at compile time (e.g. for a pre-built D3D12 root signature). |

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

The compiler-side instrumentation (counter ops, buffer synthesis,
metadata generation) works across all backends. End-to-end host
dispatch additionally depends on the host's binding-declaration
path; the slang-rhi `ExtraDescriptorBinding` API currently only
implements Vulkan, with other backends scoped as follow-ups.
Customers integrating directly against raw Vulkan / D3D12 / CUDA /
etc. (Tier 1, the dominant customer profile) declare the slot via
their native API and are not blocked by the slang-rhi rollout.

### Compiler instrumentation

| Backend | Default `-trace-coverage` | `-trace-coverage-binding=N:0` | `-trace-coverage-binding=N:M` (M ≠ 0) |
|---|---|---|---|
| CPU | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Vulkan (incl. MoltenVK on macOS) | Supported | Supported | Compiles correctly — slang-rhi multi-set follow-up pending for end-to-end |
| D3D12 | Supported | Supported | Supported |
| CUDA | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Metal (direct) | Counter values unreliable due to a pre-existing slang-rhi binding quirk. Tracked at [shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724). Use Vulkan via MoltenVK on Apple silicon. | (untested) | (untested) |
| GLSL / WebGPU | Supported codegen | (untested) | (untested) |

### End-to-end dispatch (slang-rhi-based hosts)

| Backend | End-to-end dispatch via slang-rhi `ExtraDescriptorBinding` |
|---|---|
| Vulkan | Verified — `examples/shader-coverage-demo` produces real per-line LCOV |
| D3D12 / Metal / CUDA / CPU / WGSL | Pending — same design extends per-backend; tracked as cross-repo follow-up |

LCOV output is byte-identical across the verified cells.

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

- **slang-rhi `ExtraDescriptorBinding` rollout is Vulkan-first.**
  The Vulkan path is verified end-to-end via the demo; D3D12,
  Metal, CUDA, CPU, and WGSL paths through slang-rhi need the
  matching pipeline-layout merge code. Customers integrating
  directly against raw graphics APIs (Tier 1) are not affected —
  they declare the slot via their native API based on
  `ICoverageTracingMetadata`.
- **`-trace-coverage-binding=N:M` runtime status with `M != 0`**.
  The compiler emits the correct `(set, register)` decoration on
  every backend. D3D12 honors non-zero spaces end-to-end via
  slang-rhi. Vulkan / WebGPU remain pending a slang-rhi follow-up:
  their binding-data builder assumes ≤ 1 descriptor set per shader
  object, so non-zero space mis-binds. Tracked at
  [shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959).
- **Metal counter values** are unreliable due to a pre-existing
  slang-rhi Metal binding/initialization quirk — atomic writes
  appear to land in the wrong buffer. Not a coverage-feature bug;
  tracked at
  [shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724).
  On Apple silicon, use Vulkan via MoltenVK.

