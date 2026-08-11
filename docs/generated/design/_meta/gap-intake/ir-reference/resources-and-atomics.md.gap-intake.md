---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T17:05:00Z
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 10
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 3
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/resources-and-atomics.md

## Summary

Ten gaps, all reported by `design/ir-reference/resources-and-atomics`. Seven
fixed, three deferred. This page's `watched_paths` were widened first to include
`core.meta.slang` and `hlsl.meta.slang`: six of the ten gaps ask what a user
would _write_ to reach an opcode, and that surface is declared in the core
module, which the page documented but did not watch.

Four of the seven fixes were one edit, not four. The gaps for `waveGetActiveMask`,
`imageSubscript`, `StructuredBufferGetDimensions` and `atomic_reduce` are all the
same observation — the `__intrinsic_op` sits on an underscore-prefixed
declaration behind an ordinary Slang wrapper, so the opcode is absent from the
lowering snapshot and appears only after inlining — and the page already
explained that pattern for `Interlocked*`. Generalizing that section and
tabulating the other four cases addresses them coherently, where four scattered
notes would have repeated the same explanation four times without ever stating
the rule.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                                                  | Fix summary                                                                                                                     |
| ------------ | -------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| e8a0eb0eafa7 | fixed    | `core.meta.slang:4052-4063` (newly watched) defines `enum MemoryOrder` with each enumerator spliced from the C++ constant — `Relaxed = $(kIRMemoryOrder_Relaxed)` and so on — so the mapping is exact and cannot drift.                                                                                                                                   | added the `MemoryOrder.*` spellings under the `IRMemoryOrder` block, noting the splice                                          |
| 972e44de2c38 | fixed    | `hlsl.meta.slang:15532-15533` puts `__intrinsic_op($(kIROp_WaveGetActiveMask))` on `__WaveGetActiveMask()`; the public `WaveGetActiveMask()` at 15538 calls it at 15556.                                                                                                                                                                                  | covered by the generalized wrapper section, with the line numbers                                                               |
| b33105c1d0d7 | fixed    | Same pattern: the subscript is the core-module `__ref` accessor, so `rwtex[coord]` is a call until that accessor inlines. Consistent with the `Interlocked*` case the page already documents.                                                                                                                                                             | covered by the generalized wrapper section                                                                                      |
| 0ee661b40073 | fixed    | Same pattern: the opcode is created inside the core-module `GetDimensions` body, not at the call site.                                                                                                                                                                                                                                                    | covered by the generalized wrapper section                                                                                      |
| f5d27f386fc6 | fixed    | Same pattern: the `[ForceInline]` `__target_switch` body is still a call immediately after lowering, so the unused-result `atomicAdd` the paragraph describes appears only after inlining.                                                                                                                                                                | covered by the generalized wrapper section; the "after inlining" qualifier the gap asked for is now the rule the section states |
| a88cfc08a133 | fixed    | Confirmed the honest answer is the gap's second option: no public wrapper exists. `__getEquivalentStructuredBuffer` (`hlsl.meta.slang:10213-10219`) and `__getRegisterIndex` / `__getRegisterSpace` (`26389-26393`) are underscore-prefixed with no ordinary Slang function calling them, unlike `InterlockedAdd`.                                        | added a paragraph after the table stating no public wrapper exists, and that these therefore appear without an inlining step    |
| c797e6b42b45 | fixed    | Interlock surfaces are `beginInvocationInterlock()` / `endInvocationInterlock()` (`core.meta.slang:3538,3543`), matching the opcodes' own `__intrinsic_op` lines at 3537/3542. The HLSL barrier family is at `hlsl.meta.slang:7797, 9943, 11835, 15678, 15713`. `ControlBarrier` genuinely has no spelling.                                               | added a user-surface paragraph after the barrier table, including the fragment-stage requirement                                |
| 925a5389cc94 | deferred | `DescriptorHandle` appears in `hlsl.meta.slang`, so the surface exists, but I could not establish which shader construct plus compile option actually drives SPIR-V legalization to emit the implicit heap arrays — the legalization pass is not in this page's watched set even after widening, and naming a surface I have not traced would be a guess. | —                                                                                                                               |
| 7ef0acfec3ad | deferred | Could not confirm which attribute produces a per-vertex input array. The gap proposes `nointerpolation`; the two opcodes' `__intrinsic_op` lines do not state a required attribute or stage, and I found nothing in the newly watched core-module files that ties them to one.                                                                            | —                                                                                                                               |
| f742df1cbf5e | deferred | Asks for a minimal OptiX example whose entry-point uniform is shaped so as to force the SBT lookup rather than fold. That is a question about emitted code, answerable by running the compiler; the tree's build is Linux x86-64 and the host is arm64, so I could not observe the fold boundary.                                                         | —                                                                                                                               |
