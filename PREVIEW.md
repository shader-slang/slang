# Shader Coverage — Customer Preview (2026-04-24)

This branch is a snapshot of two stacked upstream pull requests that
together deliver `-trace-coverage`, an end-to-end shader-coverage
instrumentation feature for Slang:

- **PR [#10886]** — compiler groundwork (AST-time buffer synthesis, IR
  instrumentation pass, `ICoverageTracingMetadata` public API,
  `.coverage-mapping.json` sidecar)
- **PR [#10897]** — runtime helper library (`slang-coverage-rt`) and
  example (`examples/shader-coverage-demo`)

[#10886]: https://github.com/shader-slang/slang/pull/10886
[#10897]: https://github.com/shader-slang/slang/pull/10897

## What this branch is for

This is a **preview for customer integration testing** while the
upstream PRs go through review. Try the demo end-to-end, integrate
the feature into your own build if you like, and leave feedback on
the PR this branch is attached to.

## Stability contract

- **This branch will not be force-pushed.** Safe to clone, check out,
  and iterate against.
- New commits may be added on top to address feedback.
- When the upstream PRs land, this branch will be archived and you
  can switch to the released version.

## Backend support matrix

| Backend | Status |
|---|---|
| CPU | ✅ Fully working — byte-identical LCOV to the reference |
| Vulkan (incl. via MoltenVK on macOS) | ✅ Fully working — validated on desktop NVIDIA and Apple M4 |
| D3D12 | ✅ Fully working — validated on desktop Windows NVIDIA |
| CUDA | ✅ Fully working — validated on desktop Windows NVIDIA CUDA |
| Metal (direct) | ⚠️ Pipeline runs; counter values are unreliable due to a pre-existing slang-rhi Metal binding quirk. Not a coverage-feature bug — tracked separately. Use Vulkan via MoltenVK on Apple silicon for now. |

Expected LCOV output on the demo shader: **~84.6% coverage** (22 of
26 lines), with the unreachable `"unknown type"` error-branch flagged
red — that's the dead-code-detection signal the feature exists to
surface.

## Build

First-time builds take 5–20 minutes depending on machine. Subsequent
incremental builds are ~seconds. If you iterate often, set
`-DSLANG_USE_SCCACHE=ON` at configure time for faster rebuilds.

### Linux / macOS

```bash
git clone --branch preview/shader-coverage-2026-04-24 \
    --recurse-submodules \
    https://github.com/jvepsalainen-nv/slang.git
cd slang
cmake --preset default
cmake --build --preset debug --target shader-coverage-demo
```

### Windows

```powershell
git clone --branch preview/shader-coverage-2026-04-24 `
    --recurse-submodules `
    https://github.com/jvepsalainen-nv/slang.git
cd slang
cmake --preset vs2022 -DSLANG_IGNORE_ABORT_MSG=ON
cmake --build --preset debug --target shader-coverage-demo
```

## Run the demo

```bash
# Pick any of: cpu, vulkan, d3d12, cuda
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=cpu
# -> produces coverage.lcov and simulate.coverage-mapping.json
```

## Render the LCOV report

### Linux

```bash
sudo apt install lcov
genhtml coverage.lcov -o coverage-html/
xdg-open coverage-html/index.html
```

### macOS

```bash
brew install lcov
genhtml coverage.lcov -o coverage-html/
open coverage-html/index.html
```

### Windows (no Perl required)

```powershell
dotnet tool install --global dotnet-reportgenerator-globaltool
reportgenerator -reports:coverage.lcov -targetdir:coverage-html -reporttypes:Html
start coverage-html\index.html
```

## Integrating into your own host

If you have a Vulkan / D3D12 / CUDA host that runs compiled Slang
shaders and you want to add coverage *without* using the demo's
slang-rhi path, see the "SPIR-V integration" section of
[`examples/shader-coverage-demo/README.md`][demo-readme] — ~30 lines
of additional host code, no new runtime dependency beyond
`libslang-coverage-rt`.

[demo-readme]: examples/shader-coverage-demo/README.md

## Further reading

- **Architecture**: [`docs/design/shader-coverage.md`][arch]
- **Demo usage**: [`examples/shader-coverage-demo/README.md`][demo-readme]
- **Runtime library**: [`source/slang-coverage-rt/README.md`][rt-readme]
- **Tools + CLI reference**: [`tools/shader-coverage/README.md`][tools-readme]

[arch]: docs/design/shader-coverage.md
[rt-readme]: source/slang-coverage-rt/README.md
[tools-readme]: tools/shader-coverage/README.md

## How to give feedback

- **Line-specific comments**: open the files-changed tab on this PR
  and click the `+` next to any line. Great for "why does this do X?"
  or "this header is unclear" feedback.
- **Broader threads**: comment on the PR's main Conversation tab.
- **Build / run issues**: mention `@jvepsalainen-nv` on the PR with
  your OS, compiler, and the error output.
- **Integration questions**: also welcome on the PR — we can discuss
  there and, if useful, promote the answer into the docs above.

All feedback on this branch flows back into the upstream PRs
(#10886 / #10897) so it lands in the final shipped version.
