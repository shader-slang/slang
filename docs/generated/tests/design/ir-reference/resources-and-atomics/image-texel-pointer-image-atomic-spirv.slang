//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=6c8a0e2f5621fa8fe9870a96fba7f97261b6b3930a336ced3ad67bd4bfd658e6
//META: purpose=GLSL `imageAtomicAdd` and `imageAtomicExchange` on a storage image both route through the documented `ImageTexelPointer` opcode, observable as `OpImageTexelPointer` instructions feeding `OpAtomicIAdd` / `OpAtomicExchange` in the emitted SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ImageTexelPointer` opcode catalogued for GLSL
// imageAtomic* lowering: an atomic on an image texel must first form a
// pointer to that texel. Two members of the imageAtomic* family are
// exercised so the observation covers the family, not one intrinsic;
// the CHECK-DAGs assert one OpImageTexelPointer per atomic and the
// consuming OpAtomicIAdd / OpAtomicExchange — the emit-time form the
// opcode row commits to. The returned previous values escape to a
// buffer so neither access is dead-code-eliminated.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

import glsl;

RWTexture2D<uint> img;
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint prevAdd = imageAtomicAdd(img, int2(tid.xy), 1u);
    uint prevXchg = imageAtomicExchange(img, int2(tid.xy), 7u);
    outBuf[tid.x] = prevAdd + prevXchg;
}

// SPV-DAG: OpImageTexelPointer
// SPV-DAG: OpAtomicIAdd
// SPV-DAG: OpImageTexelPointer
// SPV-DAG: OpAtomicExchange
