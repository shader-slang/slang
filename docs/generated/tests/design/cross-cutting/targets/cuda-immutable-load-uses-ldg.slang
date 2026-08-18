//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=1103b6c6793a74ba9a2a797712002157cc506284940af6ab698172cadfb6ec8a
//META: purpose=The CUDA-specific immutable-load lowering pass rewrites reads from `uniform` globals as `__ldg(...)` calls on CUDA only, while HLSL, GLSL, SPIR-V, Metal, WGSL and C++ read the field directly.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's "How target choice affects IR" list, CUDA carries a
// dedicated pass `slang-ir-cuda-immutable-load.cpp`. Its observable
// effect is that a load from a `uniform` (read-only) global lowers
// to a `__ldg(&...)` intrinsic on CUDA. The same load on every
// other text-emit target is a plain field reference.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform int gValue;
uniform RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gValue;
}

// CUDA: __ldg(
// HLSL-NOT: __ldg

// Every shader text target other than CUDA reads the uniform field
// through its own ordinary member access: GLSL and WGSL index the
// uniform block variable with `.`, Metal dereferences the `constant*`
// argument with `->`, and SPIR-V walks to the member with an
// `OpAccessChain` into the `Uniform` storage class and reads it with a
// plain `OpLoad`. Each `-NOT` pins the other half of the claim: no
// `__ldg` appears on a non-CUDA target.

// GLSL-NOT: __ldg
// GLSL: = globalParams_{{[0-9]+}}.gValue_{{[0-9]+}};

// SPIRV-NOT: __ldg
// SPIRV: OpAccessChain %_ptr_Uniform_int %globalParams %int_0
// SPIRV: OpLoad %int

// METAL-NOT: __ldg
// METAL: = globalParams_{{[0-9]+}}->gValue_{{[0-9]+}};

// WGSL-NOT: __ldg
// WGSL: = globalParams_{{[0-9]+}}.gValue_{{[0-9]+}};

// CPP-NOT: __ldg
