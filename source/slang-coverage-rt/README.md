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

## Integration in ~30 lines of host code

```c
#include "slang-coverage.h"

// 1. Parse the manifest (sidecar written by slangc, or JSON built from
//    slang::ICoverageTracingMetadata — the two shapes are identical).
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
- **Compile API.** Serialize
  `slang::ICoverageTracingMetadata` at host-side — the demo's
  `buildManifestJson` in `examples/shader-coverage-demo/main.cpp`
  is a concrete reference.

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
library to plugin authors** — for example, an application runtime
(Omniverse, a game engine) wanting plugins to share one coverage
runtime so reports aggregate cleanly across plugins. In that mode the
ABI matters across versions; the public header's `structSize`-prefixed
structs (currently `SlangCoverageBindingInfo`) provide the version-
gating mechanism.

If you need a shared build, set `SLANG_LIB_TYPE=SHARED` and rebuild;
the library follows that variable.
