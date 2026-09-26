# Six-slide Monday outline

## 1. A working direct NVVM path with a correctness anchor

Shared Slang front end/IR feeds either CUDA source + NVRTC or direct NVVM IR + libNVVM; both emit
PTX assembled by the same ptxas. Master 6eb89786c is merged. Accepted262 covers 1713 runtime cells,
1674 correct and39 unresolved, with explicit serial closure of one PCH infrastructure incident.
Provider ABI42 / LLVM14 / CUDA12.9.2, SM80 target on L4 bound these results. Material runtime is unassessed.

## 2. Material compiles near parity; no broad win yet

Show [the original accepted wall-time chart](../2026-09-26/material/wall-time.svg). Evaluation is
NVRTC 1.363s versus NVVM 1.355s; sampling 1.390s versus 1.460s, about 5% slower. Keep two rounds,
18 measured samples/cell and IQR visible. These are fresh processes with warmed filesystem/toolkit
caches. Assembly is separate; no GPU execution speed is measured.

## 3. Most time is shared compiler work

Show [stage attribution](attribution/stage-attribution.svg). In the separately qualified instrumented
run, shared builtin/front-end/link work is 76%/72% of NVVM O3 wall time; libNVVM compilation 13%/18%.
Vendor call medians: evaluation 185ms NVVM versus 201ms NVRTC; sampling 284ms versus 207ms.
Host direct preparation/emission 37–45ms plus verification 10–12ms consumes the evaluation advantage.
The second serialization costs 8–9ms, below 1% of wall time. Do not promise a large gain from bypassing NVRTC.
This session's wall times are separate observations, not a before/after optimization result.

## 4. Explain the extra stack and concrete extra arithmetic

Use the [layout table](README.md#why-stack-and-registers-differ): evaluation retains the same graph,
592 bytes through CUDA versus784 through NVVM's padded vector layout. Both have zero spills.
NVVM loses known absorption/retroreflection constants and retains six exponentials per material
entry; NVRTC retains none. This identifies avoidable work, but does not assign the precise physical
register difference or predict GPU speed. Sampling's extra 32 CUDA bytes are still unattributed.

## 5. Quality is mostly equal on simple shaders; a general direction is visible

Show [the original corpus register chart](../2026-09-26/quality/entry-registers.svg):10/12 equal,
one lower and one higher NVVM O3 register count; executable text smaller in 3/12, equal in 9/12.
The separate-storage probe removes constant exponentials and reduces NVVM 21→18 registers and
80→32 stack bytes, with independent runtime checks. Label this a **source-level probe**, not a
compiler optimization; both backends benefit and the material asymmetry is not reproduced yet.

## 6. Next experiments and an honest pitch

The pitch is a functioning direct backend, a reproducible correctness/measurement harness, and a
concrete optimization roadmap. Next: reproduce the material's constant-loss asymmetry; test a
general interprocedural/aggregate transformation using existing field-aware alias analysis; measure
both material entries and the fixed corpus with correctness and paired timing gates. Consider a
one-shot serializer separately; defer broad local-layout changes until their ABI scope is justified.
Concurrent PCH reliability, three column-major mismatch modes and material runtime contracts remain
independent follow-ups. No optimization was promoted in this follow-up; the loop is stopped.

Appendix: [full explanation](README.md), [stage tables](attribution/stage-attribution.md),
[research report](../../report.slice-264-material-attribution.md), [refresh recipe](../../RESULTS.md).
