//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=c20a44348c0739e8c347913c83450bf1d0dc3c34d664be2becfd728a37d9b098
//META: purpose=`groupshared` is a simple HLSL-flavored storage modifier; it lowers to `groupshared` in HLSL emit and to `shared` in GLSL emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per `_common.md`'s multi-backend rule, this modifier's spelling
// differs per target, so we test the emit of every shader text-emit
// backend. HLSL keeps the original keyword; GLSL maps it to `shared`,
// Metal to the `threadgroup` address space, WGSL to
// `var<workgroup>`, and SPIR-V to a variable in the `Workgroup`
// storage class.
//
// The entry point writes the shared array, synchronizes, and reads a
// different lane's slot back out through `outBuf`. The read-back is
// what keeps the declaration alive: with a write-only `sharedBuf` the
// Metal and SPIR-V backends dead-code-eliminate the variable and emit
// an empty entry point, so the group-shared storage would never appear
// on those two targets.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;
groupshared int sharedBuf[4];

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    sharedBuf[tid.x] = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = sharedBuf[3 - tid.x];
    //CHECK_HLSL: groupshared
    //CHECK_GLSL: shared
}

//CHECK_METAL: threadgroup array<int
//CHECK_WGSL: var<workgroup> sharedBuf
//CHECK_SPIRV: %sharedBuf = OpVariable %{{.*}} Workgroup
