// Pin the lowered codegen shape of fast-pathed builtin operators in (non-GLSL) HLSL
// semantics:
//  - `vector * scalar` uses the SPIR-V OpVectorTimesScalar simplification (the operands stay
//    mixed vector/scalar through lowering rather than being splat + component-wise multiplied).
//  - `matrix * matrix` is a *component-wise* multiply, NOT an algebraic matrix product, so no
//    OpMatrixTimesMatrix is emitted (that is the GLSL-only semantics, which the fast path
//    bails on).

//TEST:SIMPLE(filecheck=VTS):-target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=NOMATMUL):-target spirv -entry computeMain -stage compute -emit-spirv-directly

//TEST_INPUT: set o = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> o;

[numthreads(1, 1, 1)]
void computeMain(uint3 d : SV_DispatchThreadID)
{
    float3 v = float3(d);
    float s = (float)d.x;
    float3 vs = v * s; // vector * scalar

    float2x2 a = float2x2(v.x, v.y, v.z, s);
    float2x2 b = float2x2(s, v.z, v.y, v.x);
    float2x2 m = a * b; // component-wise multiply (HLSL), not a matrix product

    o[0] = vs.x + m[0][0] + m[1][1];

    // VTS: OpVectorTimesScalar
    // NOMATMUL-NOT: OpMatrixTimesMatrix
}
