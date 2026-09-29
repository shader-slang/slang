//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=9e5354e38bb09fc16bac90544687941aec8ef947b56cd9f9b65ce3ddb041f315
//META: purpose=An `Atomic<uint>` buffer element's `add` lowers to the `atomicAdd` member of the `AtomicOperation` family and emits as each target's atomic add.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `atomicLoad / atomicStore / atomicAdd / ...` row's
// `AtomicOperation` family with the `ptr, val?` operand shape. Declaring the
// buffer element as `Atomic<uint>` is the one surface that is expressible on
// every text-emit target, so the fan-out covers all seven; each CHECK
// asserts the target's own atomic-add spelling and, for Metal and WGSL, that
// the storage type itself became an atomic. The added amount comes from a
// `uniform` so the operation cannot be constant-folded into a plain store.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform RWStructuredBuffer<Atomic<uint> > counter;
uniform RWStructuredBuffer<uint> outbuf;
uniform uint amount;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint prev = counter[0].add(amount);
    outbuf[1] = prev;
}

// HLSL-DAG: InterlockedAdd(
// GLSL-DAG: atomicAdd(
// SPV-DAG: OpAtomicIAdd %uint
// METAL-DAG: atomic_fetch_add_explicit(
// WGSL-DAG: atomicAdd(
// CUDA-DAG: atomicAdd(
// CPP-DAG: _slang_atomic_add_u32(
