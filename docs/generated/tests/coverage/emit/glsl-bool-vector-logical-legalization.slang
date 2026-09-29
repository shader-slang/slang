//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins that `&&`, `||` and `^` on `bool3` are legalized for GLSL by round-tripping both operands through `uvec3`, while every other back end keeps the operator on the boolean vector.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes `GLSLSourceEmitter::_tryEmitLogicalBinOp` and the
// `_emitLegalizedBoolVectorBinOp` helper it delegates to. GLSL's `&&`/`||`/`^^`
// are defined only on scalar `bool`, so a component-wise logical operation on a
// `bvec3` cannot be emitted directly. The emitter rewrites it as a *bitwise*
// operation on `uvec3` and converts the result back: `bvec3(uvec3(a)&uvec3(b))`.
// The CHECKs pin exactly that triple-cast shape for each of the three operators,
// which is the observable signature of this legalization -- an emitter that
// dropped it would print `a_0 && b_0` and fail these patterns.
// The operands are derived from SV_DispatchThreadID so the comparisons are runtime
// values (a constant pair would be folded away before emit), and the three results
// are combined into one buffer store so DCE cannot remove any of them.
//
// The other text targets are pinned for contrast, since none of them needs the
// rewrite: HLSL and Metal keep `(a && b)` / `(a || b)` / `(a ^ b)` on the boolean
// vector; SPIR-V has native `OpLogicalAnd`/`OpLogicalOr` on `%v3bool` and expresses
// the boolean XOR as `OpLogicalNotEqual`; WGSL has no vector `&&`, and lowers the
// short-circuiting forms into `select(...)` instead.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = tid < uint3(2, 3, 4);
    bool3 b = tid > uint3(0, 0, 0);
    bool3 andResult = a && b;
    bool3 orResult = a || b;
    bool3 xorResult = a ^ b;
    outputBuffer[0] = uint(andResult.x) + 2u * uint(orResult.y) + 4u * uint(xorResult.z);
}

// The comparisons themselves already use GLSL's vector-compare builtins.
//GLSL-DAG: lessThan(gl_GlobalInvocationID
//GLSL-DAG: greaterThan(gl_GlobalInvocationID
// Each logical/bitwise vector operator is legalized through `uvec3`.
//GLSL-DAG: bvec3(uvec3(a_{{[0-9]+}})&uvec3(b_{{[0-9]+}}))
//GLSL-DAG: bvec3(uvec3(a_{{[0-9]+}})|uvec3(b_{{[0-9]+}}))
//GLSL-DAG: bvec3(uvec3(a_{{[0-9]+}})^uvec3(b_{{[0-9]+}}))

// SPIR-V keeps the operations on `%v3bool`; boolean XOR is a not-equal compare.
//SPV-DAG: OpLogicalAnd %v3bool
//SPV-DAG: OpLogicalOr %v3bool
//SPV-DAG: OpLogicalNotEqual %v3bool

// HLSL and Metal apply the operators directly to the boolean vectors.
//HLSL: uint((a_{{[0-9]+}} && b_{{[0-9]+}}).x)
//HLSL-SAME: uint((a_{{[0-9]+}} || b_{{[0-9]+}}).y)
//METAL: uint((a_{{[0-9]+}} && b_{{[0-9]+}}).x)
//METAL-SAME: uint((a_{{[0-9]+}} ^ b_{{[0-9]+}}).z)

// WGSL turns the short-circuiting forms into `select`, and keeps `^` as-is.
//WGSL-DAG: select(vec3<bool>(false), b_{{[0-9]+}}, a_{{[0-9]+}})
//WGSL-DAG: select(b_{{[0-9]+}}, vec3<bool>(true), a_{{[0-9]+}})
