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

Add `-trace-coverage` to any compile, in-process or via `slangc`:

```bash
# Via slangc (writes a sidecar alongside the output file)
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -o shader.spv
# -> shader.spv
# -> shader.spv.coverage-mapping.json   (optional sidecar; see below)
```

Every executable statement in the shader gets instrumented to
increment a counter at runtime. The compiler synthesizes a
`RWStructuredBuffer<uint> __slang_coverage` directly in the IR
coverage pass — no AST decl, so it does not appear in Slang's
public reflection. Hosts discover the buffer's `(set, binding)`
through `slang::ICoverageTracingMetadata` (queried in-process from
the compiled artifact) and declare the slot in their own pipeline-
layout / root-signature / descriptor-set machinery before binding
the counter buffer at dispatch time. The same metadata tells the
host how many counters to allocate and which source line each slot
corresponds to.

The `.coverage-mapping.json` sidecar is **optional** — it's a
serialization of the same metadata for cross-process / offline
workflows where the dispatch happens in a different program from
the compile (typical for precompiled shader pipelines). In-process
hosts that compile via the C++ API can ignore the sidecar entirely
and read the metadata directly from the artifact.

After the host dispatches the shader and reads the counter buffer
back, the host can either consume the slot→source attribution
directly or convert the snapshot to LCOV `.info` via
[`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py). LCOV is
consumable by `genhtml`, Codecov, VS Code Coverage Gutters, etc. A
companion C helper library (`slang-coverage-rt`) and an end-to-end
demo are queued as a follow-up PR for hosts that prefer a linked-in
runtime over the Python converter.

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

## Integration workflows

Two equally supported paths, each suited to a different host
architecture. Both expose the same data: counter count, per-slot
`(file, line)`, and the coverage buffer's `(set, binding)`. A slot
may have no real source file/line; that is preserved in the metadata
and filtered out later when exporting LCOV.

### A. In-process compile (Slang C++ API)

```
shader.slang ── compile (C++ API) ──► in-memory artifact + IMetadata
                                                 │
                                       castAs<ICoverageTracingMetadata>
                                       → (set, binding), slot→(file, line)
                                                 ▼
                                       host: allocate, bind, dispatch,
                                             readback, consume directly
                                             (own LCOV writer, telemetry,
                                              dashboard, ...)
```

For applications that compile shaders at runtime via the Slang C++
API: query coverage data directly from the artifact's metadata. No
sidecar file is created or read, and no extra runtime library is
needed — the public metadata interface is everything you need to
allocate, bind, and attribute counters.

```cpp
ComPtr<slang::IMetadata> metadata;
linked->getEntryPointMetadata(0, 0, metadata.writeRef(), ...);

auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
    slang::ICoverageTracingMetadata::getTypeGuid());

uint32_t n = coverage->getCounterCount();

slang::CoverageBufferInfo bufferInfo;
coverage->getBufferInfo(&bufferInfo);
// bufferInfo.space, bufferInfo.binding (-1 when not assigned for this target)

for (uint32_t i = 0; i < n; ++i) {
    slang::CoverageEntryInfo entry;
    if (SLANG_SUCCEEDED(coverage->getEntryInfo(i, &entry))) {
        // entry.file, entry.line — match against your counter[i] readback
    }
}
```

The host allocates a `uint32_t[n]` counter buffer, declares its slot
in its own pipeline-layout / root-signature at the reported
`(set, binding)`, dispatches the shader, reads the counters back,
and consumes the attribution data however it likes — direct
telemetry, a custom LCOV writer, a dashboard, etc.

#### Producing the canonical manifest JSON in-process

If a host wants the same `.coverage-mapping.json` bytes that `slangc`
writes as a sidecar — for example to feed
[`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py) without
going through a file, or to ship the manifest to a separate
analysis process — call `slang_writeCoverageManifestJson`:

```cpp
ComPtr<ISlangBlob> manifest;
slang_writeCoverageManifestJson(coverage, manifest.writeRef());
// manifest->getBufferPointer() / getBufferSize() are the exact
// bytes slangc would have written to <output>.coverage-mapping.json.
```

The output is byte-identical to slangc's sidecar, so anything that
parses sidecar files (the Python LCOV converter, custom external
tools, and the forthcoming `slang-coverage-rt` C library) accepts
the in-memory bytes as well.

### B. Precompiled with sidecar (slangc CLI)

```
shader.slang ── slangc -trace-coverage ──► shader.spv
                                            shader.spv.coverage-mapping.json
                                                       │
                          (ship binary + sidecar — possibly later, possibly
                          on a different machine, possibly without Slang linked)
                                                       ▼
                                       host: parse sidecar → (set, binding),
                                             slot→(file, line)
                                             allocate, bind, dispatch, readback
                                             emit LCOV via
                                             slang-coverage-to-lcov.py
                                                       │
                                                       ▼
                                       LCOV → genhtml, Codecov, VS Code, ...
```

For workflows that compile offline and dispatch later — possibly on
a different machine, possibly without Slang linked: when `slangc`
writes a compiled artifact to a file with `-trace-coverage` on, it
also writes `<output>.coverage-mapping.json` next to it.

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -o shader.spv
# -> shader.spv
# -> shader.spv.coverage-mapping.json
```

Hosts that aren't linked against Slang still get the data: the
[`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py) Python
script consumes this format today, and the fields match the
in-process API one-for-one. A companion C helper library
(`slang-coverage-rt`) for hosts that prefer a linked-in runtime
ships in a follow-up PR.

`slang-coverage-to-lcov.py` applies gcov/LCOV-style reporting rules
at export time: entries without a real source file or with a
non-positive line number are skipped instead of being written as
synthetic `SF:` / `DA:` records.

The metadata interface is extensible: future revisions add
branch/function coverage and column data through the same API and
sidecar shape.

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
dispatch is the host's responsibility — the host reads
`(set, binding)` from `ICoverageTracingMetadata` and declares the
slot in its own pipeline layout / root signature.

### Compiler instrumentation

| Backend | Default `-trace-coverage` | `-trace-coverage-binding=N:0` | `-trace-coverage-binding=N:M` (M ≠ 0) |
|---|---|---|---|
| CPU | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Vulkan / SPIR-V (incl. MoltenVK on macOS) | Supported | Supported | Compiler-side decoration correct |
| D3D12 / HLSL | Supported | Supported | Supported |
| CUDA | Supported | (no-op — backend uses uniform offsets) | (no-op) |
| Metal (direct) | Compiles. End-to-end dispatch is unreliable due to a pre-existing slang-rhi Metal binding quirk ([shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724)) — not a coverage-feature defect. | (untested) | (untested) |
| GLSL | Supported codegen | (untested) | (untested) |
| WGSL / WebGPU | **Not supported** — `-trace-coverage` emits a warning (E45102) and skips instrumentation. WGSL requires the synthesized counter buffer to use `atomic<u32>` element type, which the IR coverage pass does not yet produce. Use `-target spirv` for Vulkan-based WebGPU workflows as a workaround. | (n/a) | (n/a) |

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

- **`-trace-coverage-binding=N:M` with `M != 0`** — the compiler
  emits the correct `(set, register)` decoration on every backend.
  Whether the host's binding code routes that correctly depends on
  the host. A pre-existing slang-rhi limitation around multi-
  descriptor-set support is tracked at
  [shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959);
  hosts using their own pipeline-layout code are unaffected.
- **Metal end-to-end dispatch** — a pre-existing slang-rhi Metal
  binding/initialization quirk causes atomic writes to land in the
  wrong buffer. Not a coverage-feature defect; tracked at
  [shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724).
  On Apple silicon, use Vulkan via MoltenVK.
- **Entry-point uniform parameters can collide with the auto-allocated
  coverage buffer.** The coverage pass's auto-allocator walks
  module-scope globals only, so a shader that declares uniforms as
  parameters of the entry-point function (HLSL-legacy `void main(uniform
  Buf b, ...)` style) may end up sharing a register slot with
  `__slang_coverage`. Workaround: declare uniforms at module scope
  (modern Slang convention) — that path works correctly.

