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

For the host-facing binding contract used by `slang-rhi` and direct
hosts, see
[`docs/design/shader-coverage-host-interface.md`](../../docs/design/shader-coverage-host-interface.md).

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
public reflection. Hosts discover the hidden resource binding through
`slang::ISyntheticResourceMetadata` and use
`slang::ICoverageTracingMetadata` to learn how many counters to
allocate and how source coverage entries map to those counters.

The `.coverage-mapping.json` sidecar is **optional** — it's a
serialization of the same metadata for cross-process / offline
workflows where the dispatch happens in a different program from
the compile (typical for precompiled shader pipelines). In-process
hosts that compile via the C++ API can ignore the sidecar entirely
and read the metadata directly from the artifact.

After the host dispatches the shader and reads the counter buffer
back, the host can either consume the source-entry attribution
directly or convert the snapshot to LCOV `.info` via
[`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py). LCOV is
consumable by `genhtml`, Codecov, VS Code Coverage Gutters, etc.

For the pipeline architecture, design rationale, and alternatives
weighed, see
[`docs/design/shader-coverage.md`](../../docs/design/shader-coverage.md)
and
[`docs/design/shader-coverage-host-interface.md`](../../docs/design/shader-coverage-host-interface.md).
For examples showing where each coverage mode inserts counters, see
[`docs/design/shader-coverage-counter-placement.md`](../../docs/design/shader-coverage-counter-placement.md).

## Pinning the coverage buffer at an explicit slot

By default the IR coverage pass auto-allocates a non-conflicting
location for `__slang_coverage`. For Vulkan / SPIR-V descriptor-set
targets, it uses the descriptor set after the highest shader-visible
set at binding 0 so coverage does not extend or fill holes in a
user-owned descriptor set layout. If no shader-visible or
host-reserved descriptor sets exist, set 0 is the fresh set. If the
host pipeline layout reserves descriptor sets that the shader IR does
not reference, pass one `-trace-coverage-reserved-space <space>` per
reserved set to keep auto-allocation out of those spaces.

Pass `-trace-coverage-binding <index> <space>` to pin the coverage
buffer at a specific `(register, space)` pair instead:

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage-binding 7 0 -o shader.spv
# __slang_coverage lands at DescriptorSet 0 / Binding 7 on SPIR-V.
```

`-trace-coverage-binding` implies `-trace-coverage`. Use this when
the host needs the slot fixed at compile time before any host
reflection / metadata reads run.

`-trace-coverage-reserved-space` is repeatable and idempotent; passing
the same space more than once has the same effect as passing it once.
It does not pin a specific binding. It is for descriptor-backed hosts
whose runtime pipeline layout owns whole descriptor sets
that may be invisible to Slang for a particular entry point. It applies
to Khronos descriptor-set targets. Metal, CPU, CUDA, and D3D do not
use this Khronos descriptor-set auto-allocation
model, so the option is ignored with a warning for those targets:

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -trace-coverage-reserved-space 0 -o shader.spv
# If no higher shader-visible sets exist, __slang_coverage lands at
# DescriptorSet 1 / Binding 0.
```

---

## Integration workflows

Two equally supported paths, each suited to a different host
architecture. For today's line coverage mode, both expose the same
data: runtime counter count, source-entry attribution, and the
coverage buffer's hidden binding. The typed API exposes this through
`ICoverageTracingMetadata` plus `ISyntheticResourceMetadata`; the
sidecar serializes the same contract.
A source entry may have no real source file/line; that is preserved in the
metadata and filtered out later when exporting LCOV.

### A. In-process compile (Slang C++ API)

```
shader.slang ── compile (C++ API) ──► in-memory artifact + IMetadata
                                                 │
                                       castAs<ICoverageTracingMetadata>
                                       castAs<ISyntheticResourceMetadata>
                                       → binding, source entries
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
auto* syntheticResources = (slang::ISyntheticResourceMetadata*)metadata->castAs(
    slang::ISyntheticResourceMetadata::getTypeGuid());

uint32_t counterCount = coverage->getCounterCount();
uint32_t entryCount = coverage->getEntryCount();
SLANG_CHECK(syntheticResources != nullptr);

// The current coverage implementation emits one synthetic resource:
// the hidden counter buffer. Coverage semantics still come from
// ICoverageTracingMetadata; ISyntheticResourceMetadata only describes
// how to bind that hidden resource.
SLANG_CHECK(syntheticResources->getResourceCount() == 1);
uint32_t coverageResourceIndex = 0;

slang::SyntheticResourceInfo resourceInfo = {};
if (SLANG_SUCCEEDED(syntheticResources->getResourceInfo(coverageResourceIndex, &resourceInfo)))
{
    // Descriptor-backed targets: resourceInfo.space, resourceInfo.binding.
    // CPU/CUDA targets: resourceInfo.uniformOffset, resourceInfo.uniformStride.
}

for (uint32_t i = 0; i < entryCount; ++i) {
    slang::CoverageEntryInfo entry;
    if (SLANG_SUCCEEDED(coverage->getEntryInfo(i, &entry))) {
        // Current source-entry kinds:
        //   Line     -> file / line attribution
        //   Function -> functionName / functionMangledName
        //   Branch   -> branchSiteID / branchArmID / branchArmKind
        // entry.counterIndex selects counters[entry.counterIndex].
    }
}
```

The host allocates a `uint32_t[counterCount]` counter buffer, binds it
using the hidden binding information reported through
`ISyntheticResourceMetadata`, dispatches the shader, reads the
counters back, and consumes the source entries however it likes —
direct telemetry, a custom LCOV writer, a dashboard, etc. In the
current line/function/branch producers, entries and counters are
one-to-one. Future source-region modes may expose source entries that
are not identical to runtime counter slots, including entries with no
direct runtime counter of their own. Hosts should use
`entry.counterIndex` and be prepared for future extended entry data
rather than assuming the entry index equals the counter index.

#### Producing the canonical manifest JSON in-process

If a host wants the same `.coverage-mapping.json` bytes that `slangc`
writes as a sidecar — for example to feed
[`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py) without
going through a file, or to ship the manifest to a separate
analysis process — call `slang_writeCoverageManifestJson` with the
coverage interface obtained from the artifact metadata object. The
serializer includes the buffer binding fields when that same object
also supports `ISyntheticResourceMetadata`, which is the normal Slang
artifact case: `space` / `binding` for descriptor-backed targets and
`uniform_offset` / `uniform_stride` for CPU/CUDA uniform-marshaling
targets when available.

```cpp
ComPtr<ISlangBlob> manifest;
slang_writeCoverageManifestJson(coverage, manifest.writeRef());
// manifest->getBufferPointer() / getBufferSize() are the exact
// bytes slangc would have written to <output>.coverage-mapping.json.
```

The output is byte-identical to slangc's sidecar, so anything that
parses sidecar files (the Python LCOV converter, custom external
tools, or host-side manifest parsers) accepts the in-memory bytes as
well.

### B. Precompiled with sidecar (slangc CLI)

```
shader.slang ── slangc -trace-coverage ──► shader.spv
                                            shader.spv.coverage-mapping.json
                                                       │
                          (ship binary + sidecar — possibly later, possibly
                          on a different machine, possibly without Slang linked)
                                                       ▼
                                       host: parse sidecar → (set, binding),
                                             source entries
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

Use `-coverage-mapping-output <path>` when the compiled artifact is
written to stdout, or when the build needs a stable manifest path
instead of the default path derived from `-o`. The flag only controls
where the coverage mapping is written; it does not enable
instrumentation by itself, so use it with `-trace-coverage`,
`-trace-function-coverage`, or `-trace-branch-coverage`.

```bash
slangc shader.slang -target spirv -stage compute -entry main \
    -trace-coverage -coverage-mapping-output shader.coverage.json
# -> SPIR-V binary on stdout
# -> shader.coverage.json
```

Hosts that aren't linked against Slang still get the data: the
sidecar contains both the hidden binding and the source-entry
attribution.
The [`slang-coverage-to-lcov.py`](./slang-coverage-to-lcov.py) Python
script consumes this format after dispatch when converting readback
counters to LCOV.

`slang-coverage-to-lcov.py` applies gcov/LCOV-style reporting rules
at export time: entries without a real source file or with a
non-positive line number are skipped instead of being written as
synthetic `SF:` / `DA:` records.

The sidecar schema follows `ICoverageTracingMetadata`: it records the
runtime counter count and a list of source coverage entries. Current
entries use `kind: "line"`, `kind: "function"`, or `kind: "branch"`,
`mode: "count"`, and a numeric `counter` index. Future source-region
coverage can add entries with richer source ranges and either direct,
shared, or derived counter data while keeping the same hidden binding
contract.

---

## CLI reference

| Flag                                      | Effect                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-trace-coverage`                         | Enables per-statement line coverage. The IR coverage pass synthesizes `__slang_coverage` as an `IRGlobalParam` directly in the linked program IR (no AST decl), rewrites marker ops to atomic increments, and emits `<output>.coverage-mapping.json` sidecar when writing to a file.                                                                                                                                                                         |
| `-trace-function-coverage`                | Adds per-function-entry source entries and counters. Can be used with or without `-trace-coverage`; it shares the same synthesized counter buffer and metadata object.                                                                                                                                                                                                                                                                                        |
| `-trace-branch-coverage`                  | Adds per-branch-arm source entries and counters for `if`/`else`, loop-condition true/false, and source `switch` case/default dispatch arms, including the implicit no-match default path when no `default` label exists. Expression-level short-circuit and ternary branches are not instrumented yet. Can be used with or without `-trace-coverage`; it shares the same synthesized counter buffer and metadata object.                                          |
| `-coverage-mapping-output <path>`         | Writes the coverage mapping JSON sidecar to an explicit path instead of the default `<output>.coverage-mapping.json`. Use this when the compiled artifact is written to stdout or the build needs a stable manifest path. Requires at least one coverage tracing mode, is rejected for container outputs, and is valid only when exactly one compiled artifact carries coverage metadata.                                                                         |
| `-trace-coverage-binding <index> <space>` | Pins the synthesized `__slang_coverage` buffer at the explicit `(register index, space)` pair, instead of letting the IR pass auto-allocate. Implies `-trace-coverage`. Useful when the host needs the slot fixed at compile time.                                                                                                                                                                                                                            |
| `-trace-coverage-reserved-space <space>`  | Marks a whole Khronos descriptor set as externally occupied during auto-allocation. Repeat the option for multiple spaces; duplicates are idempotent.                                                                                                                                                                                                                                                                                                          |

---

## Counter buffer format

`uint32_t counters[N]` — flat little-endian array, no header. Indexed
by `CoverageEntryInfo::counterIndex` / manifest `counter`. Saturates
at ~4 × 10⁹ hits per slot (see [Current limitations](#current-limitations)).

---

## The converter — `slang-coverage-to-lcov.py`

```
--manifest <file.coverage-mapping.json>  Source entry → counter mapping.
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

Emits line coverage as `DA:`, function coverage as `FN:` / `FNDA:`,
and branch coverage as `BRDA:`. Line entries aggregate by `(file,
line)` at LCOV-emission time, so multiple slots on the same source line
contribute their hit counts together. Entries that do not resolve to a
real source file and positive source line are skipped to match normal
gcov/LCOV source-file semantics.

---

## Supported features

The compiler-side instrumentation (counter ops, buffer synthesis,
metadata generation) works across the supported backends listed below.
End-to-end host dispatch is the host's responsibility — the host reads
the hidden resource location from `ISyntheticResourceMetadata` and
declares the slot in its own pipeline layout / root signature.

### Compiler instrumentation

| Backend                                   | Default `-trace-coverage`                                                                                                                                                                                                                                                                         | `-trace-coverage-binding=N:0`          | `-trace-coverage-binding=N:M` (M ≠ 0) |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------- | ------------------------------------- |
| CPU source                                | Supported                                                                                                                                                                                                                                                                                         | (no-op — backend uses uniform offsets) | (no-op)                               |
| Vulkan / SPIR-V (incl. MoltenVK on macOS) | Supported. Auto-allocation uses the descriptor set after the highest shader-visible or host-reserved set at binding 0.                                                                                                                                                                            | Supported                              | Compiler-side decoration correct      |
| D3D12 / HLSL                              | Compiler metadata/codegen supported. D3D12 runtime binding policy is follow-up; hosts should query `ISyntheticResourceMetadata` and declare the reported UAV slot in their root signature.                                                                                                        | Supported                              | Compiler-side decoration correct      |
| CUDA                                      | Supported                                                                                                                                                                                                                                                                                         | (no-op — backend uses uniform offsets) | (no-op)                               |
| Metal (direct)                            | Compiles. End-to-end dispatch is unreliable due to a pre-existing slang-rhi Metal binding quirk ([shader-slang/slang-rhi#724](https://github.com/shader-slang/slang-rhi/issues/724)) — not a coverage-feature defect.                                                                             | (untested)                             | (untested)                            |
| GLSL                                      | Supported codegen                                                                                                                                                                                                                                                                                 | (untested)                             | (untested)                            |
| LLVM-emitted CPU                          | **Not supported** — coverage tracing emits a warning (E45102) and skips instrumentation until the synthesized resource + atomic sequence has a verified LLVM lowering path.                                                                                                                       | (n/a)                                  | (n/a)                                 |
| WGSL / WebGPU                             | **Not supported** — coverage tracing emits a warning (E45102) and skips instrumentation. WGSL requires the synthesized counter buffer to use `atomic<u32>` element type, which the IR coverage pass does not yet produce. Use `-target spirv` for Vulkan-based WebGPU workflows as a workaround. | (n/a)                                  | (n/a)                                 |

### Format scope

- **Line/function/branch coverage.** The converter emits `DA:`,
  `FN:` / `FNDA:`, and `BRDA:` records from v2 source-entry metadata.
  Source-region coverage remains future work and will need a defined
  projection to LCOV.
- **Branch coverage is initial.** It covers `if`/`else` arms,
  `for` / `while` / `do while` condition true/false arms, and
  source `switch` case/default dispatch arms, including the implicit
  no-match default path when no `default` label exists.
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
- **Auto-allocation can add a descriptor set on Vulkan / SPIR-V.**
  Direct hosts must include the reported coverage `(set, binding)` in
  their pipeline layout and bind the counter buffer there. Hosts that
  require a fixed existing set can use `-trace-coverage-binding`.
  Hosts that reserve descriptor sets outside the shader IR can use
  `-trace-coverage-reserved-space`.
