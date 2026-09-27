# The original large array store also exceeds the O3 compile bound

## Motivation

Slice283 corrected two small nested-array reproducers by reducing whole-store alignment to one byte.
Its 65,536-element O3 candidate timed out, but no original large control existed. This slice supplies
that missing comparison before choosing a production correction.

## Proposed solution

No compiler change. Compile the original pointer-copy reproducer at 65,536 elements under the same
120-second and 4 GiB virtual-memory bounds, using the same direct libNVVM replay and target SM80.

| Mode    | Original, fresh284         | Alignment-one candidate, inherited283 |
| ------- | -------------------------- | ------------------------------------- |
| NVVM O0 | Compile-only success       | Compile-only success                  |
| NVVM O3 | 120-second process timeout | 120-second process timeout            |

The original also fails this size gate. This does not demonstrate an annotation-induced regression,
but matching timeouts cannot establish identical causes or general scalability. No large module was
loaded or executed on the GPU, and no failed candidate was retried.

## Change summary

The completed plan, this report, [two-cell evidence](research-evidence.slice-284.json) and navigation
are the only changes. Raw sources, logs and PTX remain under `build/nvvm-original-array-size284`.
All 37 runtime artifacts, 8 qualified sources, 2 configurations, 576 main inputs and 22 dependency pins
remain exact accepted279. Full279/targeted233/cadence0 and all 1,740 corpus outcomes remain inherited.
The NVRTC and NVVM production padding defects from280 remain open.

## Concepts and vocabulary

A compile bound is this experiment's resource limit, not a language restriction. A compile-only
success establishes emitted PTX, not shader correctness, assembly success or feasible GPU storage.
Original and candidate differ in the guaranteed store alignment, not their physical type or address.

## Process report

The source uses `Cell = { i16, { i16, i32 } }`, with semantic offsets 0/4/8 and a 12-byte stride.
Two independent derivations produce identical original-large bytes: change only array capacity and
count bound in281's original three-element source, or restore precisely two store annotations from
alignment one to four in283's large candidate. Allocations, loads, scalar stores and pointer signatures
stay unchanged. The Slang/LLVM representation is intentional and valid; there is no producer repair.

Original O0 retains pointer parameters and emits two 32-bit copy loops, each covering 786,432 bytes.
The inherited candidate emits byte-copy loops over the same size. Both allocate 2,359,296 bytes of
local depot storage. This code difference is observable, but no runtime comparison was performed.
Original O3 was terminated at the bound with process code124 and no completed compiler return or PTX.
Both frozen controls ran exactly once, independently; all executed/passed/ignored counts are zero.

A fresh author and separate reused-context reviewer audit exact deltas, process limits and provenance;
root independently checks both source derivations and all referenced artifacts. Next open a bounded
provider correction with an annotation-only constructed/snapshot/phi/unaligned promotion gate, real
wrapped and multidimensional Slang fixtures, and the required full regression checkpoint. Retain the
large O3 limitation separately; do not add a size cutoff or change optimization policy.
