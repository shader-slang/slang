//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

// WGSL has a native unary '-' only for signed float/int vectors; matrices and unsigned-integer
// vectors have none, so the WGSL emitter lowers vector/matrix negation uniformly to the
// subtraction "(type(0) - operand)". "operand" is the right-hand side of that subtraction, so an
// additive operand such as "a + b" -- equal precedence to '-', and the RHS of the left-associative
// '-' -- must be wrapped in its own parentheses: "-(a + b)" must emit "(vec3<f32>(0) - (a + b))".
// Without the inner parentheses the expression reassociates to "-a + b", silently changing the
// result. This covers all three lowered element shapes: signed vectors, unsigned vectors, matrices.
//
// The inputs are derived from SV_DispatchThreadID so the negation is applied to runtime values;
// negation of a compile-time constant is folded away before emission and would not exercise this
// path.

RWStructuredBuffer<float3> outVec;
RWStructuredBuffer<uint3> outUVec;
RWStructuredBuffer<float2x2> outMat;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Signed float vectors: both -(a + b) and -(a - b) fold into the store below.
    float3 a = float3(tid) + float3(1, 2, 3);
    float3 b = float3(tid) + float3(10, 20, 30);
    float3 neg = -(a + b);
    float3 dif = -(a - b);
    //WGSL: vec3<f32>(0) - (a{{.*}} + b
    //WGSL: vec3<f32>(0) - (a{{.*}} - b
    outVec[0] = neg + dif;

    // Unsigned-integer vectors: no native WGSL unary '-', so the same subtraction lowering runs.
    uint3 ua = tid + uint3(1, 2, 3);
    uint3 ub = tid + uint3(10, 20, 30);
    //WGSL: vec3<u32>(0) - (
    outUVec[0] = -(ua + ub);

    // Matrices: no native WGSL unary '-', so the same subtraction lowering runs.
    float2x2 m1 = float2x2(float(tid.x), 1, 2, 3);
    float2x2 m2 = float2x2(float(tid.y), 4, 5, 6);
    //WGSL: mat2x2<f32>(0) - (
    outMat[0] = -(m1 + m2);
}
