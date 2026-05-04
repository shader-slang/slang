# shader-coverage-inprocess-demo

End-to-end Vulkan compute dispatch via the raw Vulkan API (no
`slang-rhi`) demonstrating Slang's `-trace-coverage` instrumentation.
Compiles a multi-module Slang program in-process, queries
`slang::ICoverageTracingMetadata` for the synthesized
`__slang_coverage` buffer's `(set, binding)`, declares the slot in
the host's own `VkDescriptorSetLayout`, dispatches the shader, reads
real counter values back, and emits LCOV.

This is the canonical **tier-1 customer pattern**: Slang C++ API for
compile + the host's own Vulkan code for dispatch + binding the
synthesized buffer through the host's pipeline-layout machinery.

For the LCOV-via-helper-library pattern, see
[`shader-coverage-demo`](../shader-coverage-demo/) in PR #10897 (uses
`slang-coverage-rt` + `slang-rhi`).

---

## What this demonstrates

1. **Multi-module compile under `-trace-coverage`.** `app.slang` imports
   `physics.slang`, which imports `math.slang`. The IR coverage pass
   instruments statements in all three files and dedupes the synthesized
   `__slang_coverage` buffer across the import graph, so a single
   counter buffer covers every instrumented slot.
2. **Typed metadata access.** `slang::ICoverageTracingMetadata::getCounterCount()`,
   `getEntryInfo(slot, ...)`, and `getBufferInfo(...)` give counter
   count, per-slot `(file, line)` attribution, and the buffer's
   target binding info — no JSON, no parsing.
3. **Declaring the synthesized buffer in the host's Vulkan pipeline.**
   The buffer's `(set, binding)` from the metadata becomes a
   `VkDescriptorSetLayoutBinding` entry; the host wires it through
   `vkUpdateDescriptorSets` like any other storage buffer. No
   reflection-driven binding, no slang-rhi.
4. **Real GPU dispatch + counter readback.** 64-thread compute
   dispatch, atomic increments land in a device-local counter buffer,
   readback via a host-visible staging buffer.
5. **Inline LCOV emission** (~30 LOC). Aggregates hits by
   `(file, line)`, filters slots without real source attribution.
   Output consumable by `genhtml` and `slang-coverage-html`.

## What this deliberately omits

- **`slang-rhi`** — the demo uses raw Vulkan to mirror the tier-1
  customer pattern. The `vulkan-api.{h,cpp}` thin wrapper handles
  loader / proc-loading / device selection (~400 LOC, copied from
  `examples/hello-world`).
- **`slang-coverage-rt`** — tier-1 customers don't need it: typed
  metadata access plus a small inline LCOV writer is the entire
  integration. The rt library (PR #10897) is a convenience for hosts
  that want richer reporting features without writing them.

---

## Run it

### macOS (Vulkan via MoltenVK)

```bash
# Build:
cmake --build --preset debug --target shader-coverage-inprocess-demo

# Run from repo root:
DYLD_LIBRARY_PATH=/usr/local/lib \
VK_ICD_FILENAMES=/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json \
./build/Debug/bin/shader-coverage-inprocess-demo

# Render HTML:
python tools/coverage-html/slang-coverage-html.py coverage.lcov --output-dir coverage-html/
```

### Windows (Vulkan)

```bat
cmake.exe --build --preset debug --target shader-coverage-inprocess-demo
build\Debug\bin\shader-coverage-inprocess-demo.exe
python tools\coverage-html\slang-coverage-html.py coverage.lcov --output-dir coverage-html\
```

Expected output:

```
[in-process] 15 counter slots; outputBuffer at binding 0, __slang_coverage at binding 1 (space 0)
[in-process] dispatched 64 workgroups; counter buffer total hits = 768
[in-process] wrote coverage.lcov
```

After rendering, `coverage-html/index.html` shows three file pages
with overall ~80% coverage — the 0-hit lines are the deliberately
unreachable branches in `physics.slang` and `math.slang`.

---

## Pipeline shape

```
Slang C++ API
    │
    ├─ createSession (+ -trace-coverage)
    ├─ loadModule("app") → links app.slang + physics.slang + math.slang
    ├─ createCompositeComponentType + link
    ├─ getEntryPointCode(0, 0) → SPIR-V blob
    └─ getEntryPointMetadata(0, 0) → IMetadata
        └─ castAs<ICoverageTracingMetadata>
            ├─ getCounterCount() → N
            └─ getBufferInfo() → (set, binding)
                                          │
Vulkan (raw API via vulkan-api.h/cpp)     │
    │                                     │
    ├─ initialize loader + device         │
    ├─ vkCreateDescriptorSetLayout {  ◄───┘ declare slot at (set, binding)
    │     binding 0 (outputBuffer),
    │     binding 1 (__slang_coverage) }
    ├─ vkCreatePipelineLayout / vkCreateComputePipelines
    ├─ allocate VkBuffer {outputBuffer, coverageBuffer, staging}
    ├─ zero-init coverageBuffer (staging copy)
    ├─ vkUpdateDescriptorSets — wire both buffers
    ├─ vkCmdBindPipeline / vkCmdBindDescriptorSets / vkCmdDispatch(64,1,1)
    ├─ vkCmdCopyBuffer coverageBuffer → staging
    └─ map staging → host-side uint32_t[N]
                                          │
LCOV emit                                 │
    └─ aggregate by (file, line) ◄────────┘
       → coverage.lcov
       → slang-coverage-html / genhtml → static HTML
```

---

## Adapting to your host

Replace the `vulkan-api.{h,cpp}` setup boilerplate with your existing
Vulkan code. The coverage-specific code is small and concentrated:

```cpp
// 1. Compile via the C++ API with `-trace-coverage`.
//    [see compileShaderAndCreatePipeline()]

// 2. Read the slot count + (set, binding).
slang::ICoverageTracingMetadata* coverage = ...;
uint32_t n = coverage->getCounterCount();
slang::CoverageBufferInfo bufferInfo;
coverage->getBufferInfo(&bufferInfo);

// 3. Declare the slot in your VkDescriptorSetLayout at
//    bufferInfo.space / bufferInfo.binding alongside your user globals.

// 4. Allocate a uint32_t[n] device-local storage buffer + zero-init it.
//    Bind it through vkUpdateDescriptorSets at that (set, binding).

// 5. Dispatch the shader normally.

// 6. Copy the counter buffer back to a host-visible staging buffer
//    after dispatch completes; read into a std::vector<uint32_t>.

// 7. Walk slot indices, query getEntryInfo(slot, &entry), aggregate
//    by (entry.file, entry.line), emit LCOV. (Or use slang-coverage-rt
//    for canned LCOV emission — see #10897.)
```

For multi-dispatch / per-frame accumulation, sum readbacks into
`std::vector<uint64_t>` host-side (the device-local buffer remains
`uint32_t` per-slot). The `slang-coverage-rt` library encapsulates
this pattern with `slang_coverage_accumulate` /
`slang_coverage_reset_accumulator` if you'd rather not implement
it inline.
