---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-02T00:00:00Z
target_doc: pipeline/06-emit.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 7
actions:
  fixed: 0
  rejected_bogus: 7
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/06-emit.md

## Summary

Nothing was escalated. All seven gaps were reopened records of text that the previous intake cycle already added to the document; each was re-checked against the current source at 2518a942d and the existing text still matches it, so all seven are rejected-bogus (the document already says what the gap asks for). The refresh in this cycle only re-derived line citations and corrected the SPIR-V downstream conditions and the HLSL fold-policy name; it did not touch the sections these gaps anchor to except for refreshed line numbers.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 16397e63407b | rejected-bogus | `### C++` already says the `IRFunc` becomes a `_main_N` workhorse and that three `SLANG_PRELUDE_EXPORT` shims `main_N_Thread`, `main_N_Group`, `main_N` follow; source `source/slang/slang-emit-cpp.cpp:1003`, `:2417` (`Stage::Compute` gate), `:2430-2470` | — |
| a0c041cb4ea5 | rejected-bogus | `### CUDA` already names `GlobalParams`, `extern "C" __constant__ ... SLANG_globalParams` and the `#define`; source `source/slang/slang-emit-cuda.cpp:421-434`, `source/slang/slang-emit.cpp:1368`. The `slang_bit_cast` claim for C++ is not confirmed in the watched set and is not documented | — |
| fd888a29cac2 | rejected-bogus | `## Source-writer abstraction`, `LineDirectiveMode` bullet, gives `#line 12 "foo.slang"`, `#line 12 0` and the no-directive modes; step 1 of the dispatcher links to it; source `source/slang/slang-emit-source-writer.cpp:419-430,474` | — |
| e5c2e132e05b | rejected-bogus | `## Operator precedence and parenthesization` already carries a five-row source/emitted table and the two-sided rule; source `source/slang/slang-emit-c-like.cpp:760-781` | — |
| 4f4361b73eac | rejected-bogus | `## Preludes`, paragraph after the table, states the HLSL prelude is the `SLANG_HLSL_ENABLE_NVAPI` guard and that `#pragma pack_matrix` comes from front matter; source `source/slang/slang-global-session.cpp:125-128`, `source/slang/slang-emit-hlsl.cpp:2534` | — |
| bc8c39ac62ab | rejected-bogus | `### Shared C-like base` already states all of C++, Metal and WGSL diagnose `PreciseQualifierUnsupportedOnTarget`; source `source/slang/slang-emit-cpp.cpp:1305`, `source/slang/slang-emit-metal.cpp:221`, `source/slang/slang-emit-wgsl.cpp:63` | — |
| fea637a3d519 | rejected-bogus | `### VM` already explains both surfaces use `HostVM` / `emitHostVMCode` and that only `Stage::Dispatch` entry points are emitted; source `source/slang/slang-code-gen.cpp:1201`, `source/slang/slang-emit-vm.cpp:1353` | — |
