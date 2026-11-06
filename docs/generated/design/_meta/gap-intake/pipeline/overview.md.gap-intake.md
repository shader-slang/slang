---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-02T00:00:00Z
target_doc: pipeline/overview.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 4
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/overview.md

## Summary

Nothing was escalated: no observation contradicts what the watched
source says the compiler does. The four reopened gaps (previously
recorded as fixed) were re-verified against the current source and the
document text; all four were fixed again with a stronger edit, in the
same pass that refreshed the document to `2518a942d`. The refresh also
corrected a claim the earlier fix had left stale: the SPIR-V optimizer
now also runs at `-O0` when `-Xspirv-opt` flags are given.

## Actions

| Gap ID       | Action | Evidence | Fix summary |
| ------------ | ------ | -------- | ----------- |
| ec552f809731 | fixed  | `source/slang/slang-lower-to-ir.cpp:15638` (`constructSSA` unconditional; only `simplifyCFG` / `peepholeOptimize` at `:15642-15646` are gated on `shouldPerformMinimumOptimizations`), `source/slang/slang-lower-to-ir.cpp:6580-6700` (`SelectExpr` and short-circuit logical-and / logical-or emit join-block params), `source/slang/slang-emit.cpp:2794` (`eliminatePhis` before emit), `source/slang/slang-emit.cpp:3624-3628` (`needsOptimization` is true for a level other than `None` or when `-Xspirv-opt` args exist); the gap's premise that an optimization pass establishes the form is not what the source says | corrected the SPIR-V optimizer gate to include `-Xspirv-opt` flags and stated that without the optimizer SPIR-V holds no `OpPhi` for a value joined across a branch |
| c70735963289 | fixed  | `source/slang/slang-compile-request.cpp:702-727` (`checkAllTranslationUnits` seeds `loadedModules` from `additionalLoadedModules`, skips `isChecked` units, adds each checked module after checking, with the `TODO` that a later-added unit is not discoverable) | added the user-visible consequence for a `slangc` command line with two files (later file can `import` the earlier by module name, not the reverse) and the `additionalLoadedModules` seed; re-derived the line citation to 702 |
| f5121a986fd4 | fixed  | `source/slang/slang-emit.cpp:3076-3082` (Torch glue uses `TorchCppSourceEmitter`, a separate case from the CPP/GLSL/HLSL/CUDA/Metal/WGSL source-language switch at `:3040-3075`), `source/slang/slang-ir-pytorch-cpp-binding.cpp:1288-1310` (work list built from `IRTorchEntryPointDecoration` or `IRCudaKernelDecoration` functions only), `source/slang/core.meta.slang:4915,4950` (`[TorchEntryPoint]`, `[AutoPyBindCUDA]`) | removed Torch from the list of emitters that emit for any entry point and gave it its own clause naming the two annotations |
| 01340fd96350 | fixed  | `source/slang/slang-emit.cpp:2745-2748` (Khronos non-direct-SPIR-V guard), `:1670-1673` (PyTorch-binding-only passes), `source/slang/slang-emit-hlsl.cpp:454` (`[numthreads(`), `source/slang/slang-emit-wgsl.cpp:219` (`@workgroup_size(`), `source/slang/slang-emit-cpp.cpp:939` (`// [numthreads(` comment), `source/slang/slang-emit-spirv.cpp:6533` (LocalSize execution mode); no `NumThreads` handling in `slang-emit-metal.cpp` or `slang-emit-cuda.cpp`; printed forms in `docs/generated/tests/design/pipeline/overview/pipeline-is-target-sensitive.slang` | turned the per-target `numthreads` prose into a seven-line per-target block (HLSL, GLSL, SPIR-V, WGSL, C++, Metal, CUDA) so the example is unmistakable |
