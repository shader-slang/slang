# Shader Coverage — Customer Preview (2026-05-04)

This branch is a snapshot of two stacked upstream pull requests built
specifically around the **in-process** integration story — compiling
Slang shaders through the C++ API, querying coverage metadata via the
typed accessors, and emitting LCOV from in-memory data. No `slang-rhi`
involvement, no graphics-API dispatch in the demo, no auxiliary
helper-library dependency.

- **PR [#10886]** — compiler groundwork (IR-time buffer synthesis,
  IR coverage pass, `slang::ICoverageTracingMetadata` public API,
  `slang_writeCoverageManifestJson` public helper, `.coverage-mapping.json`
  sidecar).
- **PR [#11038]** — `examples/shader-coverage-inprocess-demo`: a
  reference customer-integration walkthrough that compiles a
  multi-module program, queries the metadata typed accessors, serializes
  the canonical manifest, emits LCOV, and points at
  `tools/coverage-html/slang-coverage-html.py` for HTML rendering.

[#10886]: https://github.com/shader-slang/slang/pull/10886
[#11038]: https://github.com/shader-slang/slang/pull/11038

The broader ecosystem PR [#10897] (the `slang-coverage-rt` host helper
library + GPU-dispatch demo via `slang-rhi`) is **not** included in
this preview — it depends on slang-rhi PR #726, which is still landing
upstream. If/when needed for this integration we'll spin a separate
preview branch with that stack.

## What's new since 2026-04-24

If you tested the previous preview branch
(`preview/shader-coverage-2026-04-24`), the changes that affect host
integration are:

- **IR-time buffer synthesis.** The `__slang_coverage` buffer is now
  synthesized in the IR coverage pass, *not* during AST lowering. The
  effect on customer code is positive: the buffer no longer appears
  in Slang's public reflection at all, removing a "phantom global"
  problem that could surprise reflection-driven binding code. Hosts
  read the buffer's slot from `ICoverageTracingMetadata` instead.
- **`slang::ICoverageTracingMetadata` is now struct-based.** Per-entry
  and buffer-info reads use `CoverageEntryInfo` and `CoverageBufferInfo`
  output structs with leading `structSize` fields for ABI versioning.
  If your host code was using the per-attribute methods from the prior
  preview, switch to the struct-out form — see
  `tools/slang-unit-test/unit-test-coverage-tracing-metadata.cpp` for
  the canonical pattern.
- **`slang_writeCoverageManifestJson(metadata, &blob)`** — a new
  public free function that serializes any
  `slang::ICoverageTracingMetadata` artifact to the canonical
  `.coverage-mapping.json` shape on demand. Bytes are byte-identical
  to what `slangc` writes as a sidecar. Useful for in-process hosts
  that want the manifest format without going through disk.
- **In-process demo** (the headline addition for this preview). The
  new `examples/shader-coverage-inprocess-demo` is a complete,
  documented walk through the in-process integration pattern — multi-
  module compile, typed metadata access, manifest serialization,
  inline LCOV emission. It uses only `slang.h` (plus the standard
  `tools/coverage-html/slang-coverage-html.py` rendering step).

## What this branch is for

This is a **preview for customer integration testing** while the
upstream PRs go through review. The expected workflow is:

1. Build and run the demo (instructions below) to see the canonical
   in-process pattern end-to-end.
2. Compare against your existing host code — your hand-written
   `ICoverageTracingMetadata` queries should map cleanly onto the
   demo's pattern with the new struct-based accessors.
3. If anything in the demo's pattern doesn't match your needs (API
   ergonomics, missing fields, awkward sequence), tell us — we can
   still adjust before #10886 / #11038 land upstream.

## Build and run on Windows

The demo is built as part of the standard examples in this repo. From
the repo root in a Visual Studio Developer Command Prompt or similar:

```bat
:: 1. Configure (one-time per checkout). The dev preset uses Visual
::    Studio 2022; adjust the preset to your toolchain.
cmake.exe --preset vs2022 -DSLANG_IGNORE_ABORT_MSG=ON

:: 2. Build the demo target.
cmake.exe --build --preset debug --target shader-coverage-inprocess-demo

:: 3. Run from the repo root (the demo's working dir matters because
::    its shader files are loaded via relative search paths).
build\Debug\bin\shader-coverage-inprocess-demo.exe
```

Expected output: the binary prints `30 counter slots instrumented
across app.slang + physics.slang + math.slang`, writes
`coverage-mapping.json` and `coverage.lcov` to the current directory,
and ends with the next-step rendering command.

## Render the HTML report

```bat
python tools\coverage-html\slang-coverage-html.py coverage.lcov --output-dir coverage-html\
```

Open `coverage-html\index.html` in a browser. You should see three
file pages — one per Slang module — with overall coverage around
83% (the synthetic counter pattern in `main.cpp` deliberately leaves
some slots at zero hits to demonstrate hit-vs-miss attribution in the
rendered HTML).

## What to look at first

- **`examples/shader-coverage-inprocess-demo/main.cpp`** — the canonical
  in-process pipeline. Pay attention to step 5 (the synthetic-counter
  block) — that's the seam where you wire in your real GPU readback.
  The slot-→-source attribution and LCOV emission stay the same
  regardless of how the counter values arrive.
- **`examples/shader-coverage-inprocess-demo/README.md`** — short walk
  through the demo with an explicit "Adapting to your host" section.
- **`include/slang.h`** — search for `ICoverageTracingMetadata`,
  `CoverageEntryInfo`, `CoverageBufferInfo`, and
  `slang_writeCoverageManifestJson` to see the public API surface.
- **`tools/slang-unit-test/unit-test-coverage-tracing-metadata.cpp`** —
  the unit-test exercise of the typed metadata API; another
  copy-friendly pattern.

## Feedback channel

Comments on the PRs above are the easiest way to flag anything. For
multi-paragraph feedback or platform-specific issues that don't fit
inline review threads, ping the team and we'll loop you into the
right channel.

## Branch composition

```
preview/shader-coverage-2026-05-04  (this branch)
└── PREVIEW.md
└── feature/shader-coverage-inprocess-demo  (PR #11038)
    └── examples/shader-coverage-inprocess-demo/
        └── feature/shader-coverage  (PR #10886)
            └── compiler instrumentation, public API, sidecar writer, helper
                └── master (upstream)
```

Reset against this branch as needed; we'll cut a new dated preview
branch (`preview/shader-coverage-2026-05-XX`) whenever upstream moves
significantly.
