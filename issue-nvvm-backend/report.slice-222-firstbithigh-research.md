# Isolate CUDA unsigned firstbithigh signedness

## Motivation

The initial slice 221 regression used `firstbithigh(mask)` for a partition endpoint and failed on
masks with bit31 set. For example, `firstbithigh(0x80000000u)` should be 31, but CUDA source execution
produced 30. This independent helper behavior needs its own correctness evidence before a shared
prelude change.

## Proposed solution

This slice researches the existing implementation without changing production code. Load raw 32-bit/64-bit
words at runtime and compare scalar/two-component vector results with an independent integer oracle.
The measured next fix is to move the negative-input complement from CUDA U32 to I32, matching the
already-correct CPU32 and CUDA64 signedness split. That fix belongs to the next slice and requires a
full checkpoint because the prelude is shared.

## Change summary

Completed plan, this report, compact semantic evidence and STATUS only. Raw dynamic shader, Python
CUDA-driver harness, results, PTX and assembled cubins remain under `build/nvvm-loop/slice-222-semantics`.
No compiler/provider/prelude, registered test, corpus selection or oracle changed.

## Concepts and vocabulary

_Unsigned highest bit_ is the index of the highest1. _Signed highest bit_ searches for the highest0
when the original value is negative; otherwise it searches for the highest1. Zero, and signed all-ones,
produce the uint32 all-ones sentinel. Vector overloads apply the scalar rule independently.

## Process report

The standard-module documentation in `hlsl.meta.slang` states both signedness rules explicitly, as
does the [HLSL reference](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/firstbithigh).
`firstbithigh<T>` produces `$P_firstbithigh($0)` with `nvvmFirstBitHigh` semantic metadata. CUDA source
emission selects a typed prelude helper; direct emission selects FIRST_BIT_HIGH with the operand's
signedness descriptor. The input is canonical: the problem is not a front-end type or IR shape.

`U32_firstbithigh` casts its unsigned word to int32, complements if negative, then calls `__clz`.
`I32_firstbithigh` delegates to that helper without its own complement. This puts signed behavior in
the shared unsigned implementation. CUDA64 and CPU32 instead complement only in the signed helper.
The direct provider's IntegerBit operation checks `SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER` before
complementing, then uses defined-zero LLVM ctlz and width-minus-one-minus-count. That typed path is
already correct for all cases measured here.

The dynamic CUDAKernel receives Ptr<int>, reads96 input words per 32-lane batch and writes 384 results:
four scalar operations and two components of each of four vector operations. Neighbor components use
lane^1 so they exercise distinct runtime values. The Python oracle uses integer bit_length, explicitly
complementing within 32 or 64 bits only for signed-negative operands. It derives no expected values from
NVRTC or NVVM output. Inputs remain unchanged after every execution.

The probe covers 96 unique 32-bit values and 192 unique 64-bit values: every power-of-two with adjacent
values, zero, all-ones and alternating bits. Six batches cover all 64-bit values and repeat the 32-bit
set twice. All three modes compile and assemble. Across 18 GPU executions, 6912 output words are
checked; 6888 match and 24 differ. NVRTC has two mismatching batches and 24 wrong words, entirely in
unsigned32 scalar/vector slots for four inputs with bit31 set. Both direct modes pass all six batches
and 2304 words each. Signed32, signed64 and unsigned64 all match on every mode.

The minimal case returns 30 on NVRTC and 31 on both direct modes. Unsigned0xffffffff returns0xffffffff
on NVRTC instead of 31. The prelude file is byte-identical to accepted full 220, and the retained
pre-change221 fixture diagnostic also demonstrates this predates the221 admission. No introducing
revision was bisected. This finding is separate from the51 registered failures, which remain unchanged.

GPU smoke 4/4 passes. All 23 accepted tested-source hashes,12 artifacts and 549 runtime-input hashes
match221. Registered corpus/material evidence is explicitly inherited; no fresh registered-cell or
material runtime claim is made. Full checkpoint 220 and implementation cadence 1 remain unchanged.

The helper/fallback inventory contains no production additions. Probe-only `candidates` enumerates
integer boundaries and `expected` encodes the documented oracle; both stay in ignored raw research.
Fresh-context delegation remains unavailable; local parent review uses WORKFLOW's fallback.

Research is accepted on 2026-09-25. Next register a dynamic unsigned/signed regression, prove the
CUDA failure before the change, move the complement to the signed helper, replay these exact inputs
and expectations, and complete full frozen/discovery/material validation before accepting the fix.
