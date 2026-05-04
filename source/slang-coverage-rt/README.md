# slang-coverage-rt

Host-side helper library for Slang's `-trace-coverage` shader
instrumentation. CPU-only, graphics-API-independent: the caller owns
and binds the GPU buffer; this library parses the compiler's manifest,
accumulates counter snapshots, and emits LCOV.

- **What this is.** A small C ABI (7 functions, no dependencies) that
  turns a `(manifest, counters)` pair into an LCOV `.info` file.
- **What this isn't.** A binding layer. The library never touches a
  graphics API — the host allocates, binds, dispatches, and reads back
  the coverage buffer, then hands the values here.

For background on the compiler side of the feature, see
[`tools/shader-coverage/README.md`](../../tools/shader-coverage/README.md).
For a runnable end-to-end sample, see
[`examples/shader-coverage-demo/`](../../examples/shader-coverage-demo/).

---

## How this library fits

`slang-coverage-rt` is the **graphics-API-independent** half of the
host-side story. It handles parsing the manifest, accumulating
counter snapshots, and emitting LCOV. It deliberately doesn't know
about Vulkan / D3D12 / CUDA / Metal — your host already knows how
to bind a buffer in its own API, and the library hands you the
information needed to do so.

Customer-side integration falls into three tiers:

- **Tier 1 — raw graphics API.** Production engines with their own
  RHI, custom Vulkan/D3D12/CUDA hosts, content-creation
  applications. The library + `slang::ICoverageTracingMetadata`
  are one viable integration:
  query `(set, binding)`, declare in your pipeline-layout / root-
  signature, allocate the buffer, bind, dispatch, read back, feed
  to `slang_coverage_accumulate`, save LCOV.
- **Tier 2 — slang-rhi-based hosts.** slangpy, slang-test, indie or
  educational projects already on slang-rhi. The library is still
  the Tier-1 surface for parsing/accumulating/serializing; on top
  of that, slang-rhi's `ShaderProgramDesc::extraDescriptorBindings`
  + `IShaderObject::setExtraBinding` provide the binding path so
  you don't write descriptor declarations directly.
- **Tier 3 — start from the demo.** Anyone wanting a working
  reference forks `examples/shader-coverage-demo` and adapts it.
  The demo is built on Tier 2 (slang-rhi); Tier-1 customers swap
  the dispatch loop for their own RHI.

### When you don't need this library

This library is a convenience layer, not a requirement. Skip it
entirely if either fits:

- **In-process compile + custom telemetry.** Hosts compiling shaders
  via the Slang C++ API can read counter count, per-slot
  `(file, line)`, and `(set, binding)` from
  `slang::ICoverageTracingMetadata` directly using its typed
  accessors. If your output is internal telemetry, a custom
  dashboard, or an in-house LCOV writer, the metadata API is
  everything you need.
- **Python-only LCOV pipeline.** If you already use
  [`tools/shader-coverage/slang-coverage-to-lcov.py`](../../tools/shader-coverage/slang-coverage-to-lcov.py)
  to emit LCOV from a sidecar + counter snapshot, you don't need
  this library either; the Python script covers the same conversion
  in a different language.

The library's specific value-add is: in-memory C-ABI parsing of the
manifest JSON, additive counter accumulation across dispatches, and
LCOV serialization without depending on Python.

---

## Integration in ~30 lines of host code

```c
#include "slang-coverage.h"

// 1. Parse the manifest. Two ways to get the JSON file:
//      - slangc writes `<output>.coverage-mapping.json` automatically
//        when `-trace-coverage` is on; pass that path directly.
//      - For in-process compiles, call `slang_writeCoverageManifestJson(
//        metadata, &blob)` to produce the same bytes in memory, then
//        write them to a temp file and pass that path.
SlangCoverageContext* ctx = NULL;
slang_coverage_create("shader.spv.coverage-mapping.json", &ctx);

// 2. Size and locate the counter buffer.
uint32_t n = slang_coverage_counter_count(ctx);
const SlangCoverageBindingInfo* bind = slang_coverage_binding(ctx);
// bind->space / bind->binding → Vulkan descriptor set / binding
// bind->uavRegister          → HLSL u<N> register (D3D12)
// Fields set to -1 were absent from the manifest for this target.

// 3. Host allocates & binds a `uint32_t[n]` buffer at the reported
//    slot. Dispatch the instrumented shader. Read the buffer back
//    after each dispatch (or batch of dispatches, per test case).

// 4. Feed snapshots in. Multiple calls accumulate additively —
//    call per frame, per test case, or once at shutdown.
slang_coverage_accumulate(ctx, counters, n);

// 5. Emit LCOV. Consumable by genhtml, Codecov, VS Code Coverage
//    Gutters, and any other LCOV-aware tool.
slang_coverage_save_lcov(ctx, "coverage.lcov", "my_test_run");

slang_coverage_destroy(ctx);
```

The public header ([`include/slang-coverage.h`](include/slang-coverage.h))
carries per-function docstrings that should be treated as the spec.
The rest of this README covers things the header cannot: lifecycle
patterns, manifest shape, and error modes.

---

## Lifecycle and threading

- **One context per manifest.** `slang_coverage_create` loads and
  parses the JSON once; calling it repeatedly on the same manifest
  is wasteful. Destroy with `slang_coverage_destroy` (nullptr-safe).
- **Accumulator is additive.** Every `slang_coverage_accumulate` call
  merges a new snapshot into the context's running totals. Typical
  patterns:
  - **Per frame:** read the UAV back each frame, call `_accumulate`,
    save LCOV on exit — one report for the whole run.
  - **Per test:** call `_reset_accumulator` between tests; save
    LCOV per test — multiple reports, merged later via `lcov -a`.
- **Not thread-safe.** Serialize access to a single context. Parallel
  tests should each hold their own `SlangCoverageContext`.
- **No GPU lifetime awareness.** The library never retains pointers
  into the caller-provided `counters` array beyond the duration of
  the `_accumulate` call; the host is free to reuse or free the
  buffer immediately after the call returns.

---

## Manifest format

Version-1 JSON, shape documented here for reference. Two producers
write this shape:

- **slangc sidecar.** Writing any artifact with `-trace-coverage`
  produces `<output>.coverage-mapping.json` alongside.
- **Compile API.** Call
  `slang_writeCoverageManifestJson(metadata, &blob)` to produce the
  same bytes in-memory from a `slang::ICoverageTracingMetadata`
  artifact. Byte-identical to the slangc sidecar; consumers that
  parse the JSON (Python LCOV converter, custom external tools) read
  either form interchangeably. The current rt library entry point
  (`slang_coverage_create`) takes a file path only — feeding it the
  in-memory blob requires staging the bytes to a temp file or
  waiting for an in-memory variant in a follow-up.

```json
{
  "version": 1,
  "counters": 41,
  "buffer": {
    "name": "__slang_coverage",
    "element_type": "uint32",
    "element_stride": 4,
    "synthesized": true,
    "space": 0,
    "binding": 2
  },
  "entries": [
    {"index": 0, "file": "physics.slang", "line": 17},
    {"index": 1, "file": "physics.slang", "line": 22}
  ]
}
```

Fields absent in a given build target become `-1` on
`SlangCoverageBindingInfo` (for example, CPU builds don't populate
UAV registers). `counters` ≥ `entries.size()`; trailing slots may
be reserved for future use. Unknown keys are ignored, so forward
compatibility with v1 consumers is preserved across manifest
additions.

**Runtime status of non-zero `space`.** The compiler writes the
user-requested `space` value into the JSON correctly regardless of
its value, so the manifest itself is always accurate. At dispatch
time, however, only D3D12 currently honors non-zero `space` values
end-to-end via slang-rhi; Vulkan and WebGPU remain pending a
slang-rhi follow-up that adds multi-descriptor-set support to the
binding-data builder. Tracked at
[shader-slang/slang#10959](https://github.com/shader-slang/slang/issues/10959).
For consumers that read the sidecar without dispatching themselves
(LCOV converters, CI scripts), the `space` field is reliable
unconditionally.

---

## Error codes

Functions return `SlangResult` (the same type the rest of the Slang
C API uses). Use `SLANG_FAILED(r)` for the failure check; specific
codes are listed below for matching against expected error paths.

| Code | Meaning |
|---|---|
| `SLANG_OK` | Success. |
| `SLANG_E_INVALID_ARG` | Null pointer; `testName` contains `-` (forbidden by LCOV); `_accumulate` called with `count != counter_count`. |
| `SLANG_E_NOT_FOUND` | Manifest path does not exist. |
| `SLANG_E_CANNOT_OPEN` | Manifest read or LCOV write failed (file existed but I/O failed). |
| `SLANG_FAIL` | Manifest is not well-formed v1 JSON, or declares an unsupported `version`. |

### Migrating older host code

If you have host code written against an older preview, two
patterns may need updating:

**Reflection-cursor binding for `__slang_coverage` (slang-rhi
hosts).** The buffer is not in Slang's public reflection. Replace
any `cursor["__slang_coverage"].setBinding(buf)` with the
`ExtraDescriptorBinding` field on `ShaderProgramDesc` plus
`IShaderObject::setExtraBinding(set, binding, Binding(buf))` at
dispatch time. See `examples/shader-coverage-demo/main.cpp` for
the canonical pattern. Tier 1 hosts (raw Vulkan / D3D12 / CUDA)
are unaffected — they read the binding from
`ICoverageTracingMetadata` and declare the slot in their own
pipeline-layout API as usual.

**`SlangCoverageResult` enum.** Replaced by `SlangResult`. Map old
codes to new per the table below, or use `SLANG_FAILED(r)` for a
uniform failure check.

If your existing host code compiles against the new headers and
fails with errors like `'SLANG_COVERAGE_OK' was not declared`,
replace per the table below — or switch to `SLANG_FAILED(r)` for a
uniform failure check.

| Old code                                   | New code              |
|---|---|
| `SLANG_COVERAGE_OK`                        | `SLANG_OK`            |
| `SLANG_COVERAGE_ERROR_INVALID_ARGUMENT`    | `SLANG_E_INVALID_ARG` |
| `SLANG_COVERAGE_ERROR_FILE_NOT_FOUND`      | `SLANG_E_NOT_FOUND`   |
| `SLANG_COVERAGE_ERROR_IO_FAILED`           | `SLANG_E_CANNOT_OPEN` |
| `SLANG_COVERAGE_ERROR_PARSE_FAILED`        | `SLANG_FAIL`          |
| `SLANG_COVERAGE_ERROR_UNSUPPORTED_VERSION` | `SLANG_FAIL`          |
| `SLANG_COVERAGE_ERROR_OUT_OF_RANGE`        | `SLANG_E_INVALID_ARG` |

`slang_coverage_accumulate` requires `count` to equal
`slang_coverage_counter_count(ctx)` exactly; mismatched sizes
return `SLANG_E_INVALID_ARG`.

---

## Build integration

The library is built as a static library (`slang-coverage-rt.lib/.a`)
when `SLANG_ENABLE_SHADER_COVERAGE_RT=ON` (the default). Link and
add the include directory:

```cmake
target_link_libraries(my-host PRIVATE slang-coverage-rt)
target_include_directories(my-host PRIVATE
    ${slang_SOURCE_DIR}/source/slang-coverage-rt/include)
```

No transitive runtime dependency beyond the C++ standard library.

### Static vs dynamic linking

The library ships as a static library by default. For most embed-in-
test-suite use cases that's the right pick:

- **ABI freedom**: each customer build re-links from source, so adding
  fields to `SlangCoverageBindingInfo` or new functions doesn't require
  bumping a shared-library SONAME or coordinating across a release
  cadence.
- **Smaller deployment footprint**: one binary to ship, no separate
  `.so` / `.dll` to install or version-pin.
- **Simpler symbol resolution**: no runtime loader interaction, no
  `RPATH` / `LD_LIBRARY_PATH` games on Linux.

A shared-library shape is appropriate when **the host distributes the
library to plugin authors** — for example, a game engine or
content-creation application wanting plugins to share one coverage
runtime so reports aggregate cleanly across plugins. In that mode the
ABI matters across versions; the public header's `structSize`-prefixed
structs (currently `SlangCoverageBindingInfo`) provide the version-
gating mechanism.

If you need a shared build, set `SLANG_LIB_TYPE=SHARED` and rebuild;
the library follows that variable.
