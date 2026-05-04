# shader-coverage-inprocess-demo

A reference for the **in-process** shader-coverage integration pattern.
Shows how a host that compiles Slang shaders through the C++ API queries
`slang::ICoverageTracingMetadata`, serializes the canonical manifest,
and emits LCOV — with no `slang-rhi`, no `slang-coverage-rt` library,
and no graphics-API dispatch.

This demo is the **producer-side and reporting-side**; the missing
piece in between (allocate counter buffer → bind → dispatch → read
back) is your engine's responsibility, since it depends on which
graphics API you use. The synthetic counter values in `main.cpp` are
explicitly labeled as a stand-in for the real readback.

For a full end-to-end demo that *does* dispatch (via `slang-rhi`) and
uses the helper library, see [`shader-coverage-demo`](../shader-coverage-demo/)
in PR #10897.

---

## What this demonstrates

1. **Multi-module compile under `-trace-coverage`.** `app.slang` imports
   `physics.slang`, which imports `math.slang`. The IR coverage pass
   instruments statements in all three files and dedupes the synthesized
   `__slang_coverage` buffer across the import graph, so a single
   counter buffer covers every instrumented slot.
2. **Typed metadata access.** `slang::ICoverageTracingMetadata::getCounterCount()`,
   `getEntryInfo(slot, ...)`, and `getBufferInfo(...)` give the host
   counter count, per-slot `(file, line)` attribution, and the buffer's
   target binding info — no JSON, no parsing.
3. **In-memory manifest serialization.** The new public helper
   `slang_writeCoverageManifestJson(metadata, &blob)` produces the
   canonical `.coverage-mapping.json` bytes. Same shape as `slangc`'s
   sidecar, available without disk I/O.
4. **Hand-rolled LCOV emission** (~30 LOC). Aggregates hits by
   `(file, line)`, filters slots without real source attribution,
   produces standard LCOV `.info` consumable by both `genhtml` and
   `slang-coverage-html`.

## What this deliberately omits

- **GPU dispatch and counter readback.** The synthetic counter loop in
  step 5 of `main.cpp` is the seam where you plug in your engine's
  readback. The slot-→-source attribution and LCOV emission are
  identical regardless of how the counter values arrive.
- **The `slang-coverage-rt` helper library.** Tier-1 customers don't
  need it: typed metadata access plus a small LCOV writer is the entire
  integration. The rt library (PR #10897) is a convenience for hosts
  that want richer reporting features without writing them.

---

## Run it

```bash
# Build:
cmake --build --preset debug --target shader-coverage-inprocess-demo

# Run (from repo root):
./build/Debug/bin/shader-coverage-inprocess-demo

# Render HTML:
python tools/coverage-html/slang-coverage-html.py coverage.lcov --output-dir coverage-html/
# Or:
genhtml coverage.lcov -o coverage-html/
```

Open `coverage-html/index.html` in a browser to see line-by-line
coverage attribution across all three Slang modules.

---

## Adapting to your host

Replace step 5 of `main.cpp` (the synthetic counter loop) with your
real readback. The shape is:

```cpp
// 1. Read the slot count from the metadata.
uint32_t n = coverage->getCounterCount();

// 2. Read the buffer's (space, binding) — used by your pipeline-layout
//    / root-signature code to declare the slot.
slang::CoverageBufferInfo bufferInfo;
coverage->getBufferInfo(&bufferInfo);

// 3. Allocate uint32_t[n], bind it at (bufferInfo.space, bufferInfo.binding),
//    dispatch your compute shader, read the buffer back into:
std::vector<uint32_t> rawCounters(n);
// ... your readback code here ...

// 4. Convert uint32_t to uint64_t for accumulation across multiple
//    dispatches (LCOV `DA:` records take any non-negative integer).
std::vector<uint64_t> hits(n);
for (uint32_t i = 0; i < n; ++i)
    hits[i] = rawCounters[i];

// 5. Pass `hits` into writeLcovReport in place of the synthetic loop.
```

For multi-dispatch / multi-test-case workflows, accumulate across
readbacks (`hits[i] += newReadback[i]`) and emit one LCOV per test or
per session. The `slang-coverage-rt` library encapsulates this pattern
with `slang_coverage_accumulate` / `slang_coverage_reset_accumulator`
if you'd rather not implement it inline.
