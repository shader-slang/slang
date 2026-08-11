---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:28:05Z
target_doc: target-pipelines/cuda.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 10
actions:
  fixed: 9
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for target-pipelines/cuda.md

## Summary

Ten gaps, all from `docs/generated/tests/design/target-pipelines/cuda`.
Nothing was escalated: no gap turned out to be a compiler defect. Nine are
fixed and one is deferred — the PTX fingerprint, which no Slang source
produces (nvrtc writes that text) and which this host cannot settle by
running the compiler. The page grew from 79,761 to 84,439 bytes against its
98,304-byte cap.

The one `drift-from-source` gap (`7057bdb1d195`) resolved as a documentation
gap, not a compiler bug: `handleAutoBindNames` does write `__kernel__<name>`
exactly as the doc said, but only into the `ExternCpp` *linkage* name, and
`CLikeSourceEmitter::generateName` returns from its entry-point branch before
it ever consults `ExternCpp` — so the observed unprefixed
`extern "C" __global__ void myKernel(...)` is what the source prescribes for a
kernel compiled as the selected entry point. The doc now says which arm the
rename is visible on and why. Two fixes say more than the gap asked because
the source says more: the `__ldg` fallback arm is characterised by the exact
set of type ops `createLoadFuncForType` recognizes rather than by one example
type, and the `kIROp_DispatchKernel` bullet names both producers of the opcode
plus the two reasons neither normally reaches CUDA text.

Four fixes rest on files outside this page's `watched_paths`:
`source/core/slang-type-text-util.cpp` (the `-target` name table),
`source/slang/slang-options.cpp` (CLI spellings), `source/slang/slang-parser.cpp`
(the `__dispatch_kernel` keyword; the CUDA-side producer is in a watched file)
and `source/slang/slang-diagnostics.lua` (`E45105` / `E45114`). Each is a
declarative table read directly and cited by line, matching the precedent set
in the `target-pipelines/hlsl.md` intake, but the manifest should gain those
paths so the claims stay tracked. See rows `c47f6376ff53`, `23fe34a4ec1e`,
`7d9a27123dfe`, `b0910e127d33` and `2a6f07fe3e9a`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 2a6f07fe3e9a | fixed | The opcode's CUDA-side producer is watched `source/slang/slang-ir-pytorch-cpp-binding.cpp:1064` (`generateCUDAWrapperForFunc`, line 1009, called only from `generateHostFunctionsForAutoBindCuda` at `:1237-1255`), and the same file rewrites every `IRDispatchKernel` into `kIROp_CudaKernelLaunch` at `:438-462` (`generateCppBindingForFunc`, PyTorch arm) — so the `<<<...>>>` emitter arm at watched `source/slang/slang-emit-cuda.cpp:1388-1404` is not reached on either ordinary arm, which is why no test could pin it. The surface keyword is `source/slang/slang-parser.cpp:3217-3231` (`__dispatch_kernel`, registered at `:10871`), lowered at `source/slang/slang-lower-to-ir.cpp:5978-5992`; used in `tests/autodiff/cuda-kernel-export.slang:44`. **`slang-parser.cpp` and `slang-lower-to-ir.cpp` are outside `watched_paths`.** | named both producers of `kIROp_DispatchKernel` (the `__dispatch_kernel` keyword, cross-linked to the AST page, and the generated `[AutoPyBindCUDA]` host wrapper) and stated why neither normally reaches CUDA text; the requested `-target cuda` example was not added because the source shows the arm is not reachable that way |
| c47f6376ff53 | fixed | `{SLANG_CUDA_SOURCE, "cu", "cuda,cu", ...}`, `{SLANG_CUDA_HEADER, "cuh", "cuh", ...}`, `{SLANG_PTX, "ptx", "ptx", ...}` at `source/core/slang-type-text-util.cpp:82-84`. Corroborated by the bundle's own verified directives: 85 tests use `-target cuda` and `coopvec-lowered-on-cuda-header.slang` uses `-target cuh`. **`slang-type-text-util.cpp` is outside `watched_paths`.** | added the three `-target` spellings (`cuda`/`cu`, `cuh`, `ptx`) to the intro sentence that enumerates the `CodeGenTarget` values |
| 805da7fc5a61 | deferred | `.version` / `.target` / `.visible .entry` are nvrtc's output format, not Slang's: nothing under `watched_paths` writes PTX text (the CUDA pipeline stops at CUDA C++, per watched `source/slang/slang-emit.cpp:2972-2973`). The only test, `ptx-downstream-nvrtc-emit.slang`, carries `//META: requires-tool=nvrtc` and the bundle README (line 63-66) records it as ignored on a runner without the CUDA toolchain, so its CHECK lines are unverified. Settling it needs a `slangc -target ptx` run with nvrtc present — impossible here (the tree's build is Linux x86-64, this host is arm64). Follow-up: run the bundle test on a CUDA-toolchain runner, then document the fingerprint from the verified output. | — |
| 0c3f169de800 | fixed | Watched `source/slang/slang-ir-legalize-varying-params.cpp:1912-1916` says so in the source comment: "A recursive terminate-reaching call chain cannot be flattened by inlining ... In practice such recursion is already rejected upstream (E55201, 'recursion not allowed'), so this guard primarily guarantees the pass itself always terminates." Cycle check at `:1917`, residual-call diagnostic at `:1983-1996`. E55201 is the verified CHECK of `stress-recursive-function-rejected.slang:24-25`. | noted that the recursive arm is shadowed by `checkForRecursiveFunctions` / `E55201` and that the cycle check exists for termination, leaving "a call `inlineCall` declines to flatten" as the reachable shape |
| 7057bdb1d195 | fixed | Watched `source/slang/slang-ir-pytorch-cpp-binding.cpp:1332-1355`: the rename is applied to the `IRExternCppDecoration` and only when one is present (`:1342`). Watched `source/slang/slang-emit-c-like.cpp:1219-1248` returns `generateEntryPointNameImpl(...)` for any inst with an `IREntryPointDecoration`, before the `IRExternCppDecoration` branch at `:1251-1255`; watched `source/slang/slang-emit-cuda.cpp:435-440` prefixes such a function `extern "C" __global__`. So the observed unprefixed name is what the source prescribes — a doc gap, not a compiler bug. The wrapper takes the original name at `:1090-1093`. | added a paragraph stating the rewrite touches only the `ExternCpp` linkage name, that an entry point's symbol name wins over it (so `-target cuda` emits `extern "C" __global__ void myKernel(...)` unprefixed), and that the prefixed name matters on the `PyTorchCppBinding` arm |
| 7b1f9994557e | fixed | Watched `source/slang/slang-ir-cuda-immutable-load.cpp`: `createLoadFuncForType` handles only the scalar ops at `:85-98`, `kIROp_VectorType` `:101`, `kIROp_MatrixType` `:148`, `kIROp_ArrayType` `:215`, `kIROp_StructType` `:242`, and returns an empty `LoadMethod` off the end of the switch at `:269`; the array and struct arms discard the half-built function and return empty when a leaf fails (`:231-235`, `:257-261`); `processInst` leaves the original load in place when `emitImmutableLoad` yields null (`:292-330`). | replaced the unillustrated "no leaf is `__ldg`-able" sentence with the recognized type-op set and the two bail-out paths, so the surviving load shape is derivable (an opaque leaf — resource handle or pointer — not a composite of the five recognized ops) |
| 23fe34a4ec1e | fixed | CLI spellings at `source/slang/slang-options.cpp`: `-trace-coverage-counter-width <bits>` `:676-691`, `-trace-coverage-boolean` `:641-650`, `-validate-uniformity` `:1205-1208`, `-fspv-reflect` `:898-901`, `-embed-downstream-ir` `:1218-1221`. `coverage-counter-width-bytes-invalid` is id `45114` at `source/slang/slang-diagnostics.lua:5258-5262`, its CLI bit-width counterpart `45113` at `:5251-5256`; watched `source/slang/slang-emit.cpp:1171-1186` states the CLI-validates-bits / API-validates-bytes split in-source. **`slang-options.cpp` and `slang-diagnostics.lua` are outside `watched_paths`.** | added the `slangc` spelling inline to the five option rows and the `E45114` (API bytes) / `E45113` (CLI bits) split to the `TraceCoverageCounterByteWidth` row, rather than widening the table with a sixth column |
| 7d9a27123dfe | fixed | `coverage-uniform-layout-unavailable` is id `45105` at `source/slang/slang-diagnostics.lua:5203-5207`; the diagnose site is `source/slang/slang-ir-coverage-instrument.cpp:1500`. The enabling options are named in watched `source/slang/slang-code-gen.cpp:1440-1446` (`TraceCoverage` / `TraceFunctionCoverage` / `TraceBranchCoverage`), spelled `-trace-coverage`, `-trace-function-coverage`, `-trace-branch-coverage` at `source/slang/slang-options.cpp:622-640`; `reqSet.coverageTracing` is set by `kIROp_Increment*CoverageCounter` in watched `source/slang/slang-emit.cpp:598-601`. **`slang-diagnostics.lua` and `slang-options.cpp` are outside `watched_paths`.** | added `E45105`, the three coverage options and their `slangc` spellings, and the opcode that actually sets `reqSet.coverageTracing`, to Phase A row 13 |
| b0910e127d33 | fixed | Watched `source/slang/slang-compiler-options.h:363-366`: `shouldPerformMinimumOptimizations()` returns `getBoolOption(CompilerOptionName::MinimumSlangOptimization)`; watched `source/slang/slang-emit.cpp:1345-1352` copies it onto `fastIRSimplificationOptions.minimalOptimization` via `IRSimplificationOptions::getDefault`. CLI spelling `-minimum-slang-optimization` at `source/slang/slang-options.cpp:516-519`. **`slang-options.cpp` is outside `watched_paths`.** | one generalized note on Phase B row 33a naming the option, its `slangc` spelling and the read path, and pointing at every other arm the same flag selects (rows 49/49a/49b, 57a/57b, Phase C row 26) |
| 7b1666501f66 | fixed | Watched `source/slang/slang-emit-cpp.cpp:1305-1314`: `emitTempModifiers` diagnoses and returns without writing anything ("C/C++ (and, via inheritance, CUDA) has no `precise` keyword; drop it and warn"), so the temporary is still emitted and the compile continues. `E56005` and the warning severity are the verified CHECK of `precise-qualifier-dropped-warning.slang:28` (listed as tested in the bundle README, line 130); the printed form `warning[E56005]: 'precise' qualifier is not supported on target ...` is recorded in `docs/generated/tests/design/pipeline/06-emit/README.md:140`. | added the warning severity, the `E56005` code and the drop-and-continue behaviour to the `precise` bullet in Phase D |
