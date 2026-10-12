//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
// A separate run so that the NOZERO-NOT check applies to the whole output.
//TEST:SIMPLE(filecheck=NOZERO): -target wgsl -entry computeMain -stage compute
// The f16 entry point is checked as text only; Tint does not compile it here.
//TEST:SIMPLE(filecheck=HALF): -target wgsl -entry halfMain -stage compute
// Validate the computeMain WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type
// A 64-bit integer matrix is lowered to an array of vectors and rejected like any i64 vector.
//DIAGNOSTIC_TEST:SIMPLE(diag=I64): -target wgsl -entry int64Main -stage compute

// WGSL has no unary '-' on matrices and no single-scalar matrix constructor, so a matrix negation
// cannot be written as "-m" or as "matCxR<T>(0) - m" (#13571). The WGSL emitter multiplies by the
// scalar -1 instead, which WGSL supports natively for float matrices of every shape.

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0], stride=4):name=input
RWStructuredBuffer<float> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const float2x2 M2 = float2x2(1.0, 2.0, -3.0, 4.0);

int sum(float2x2 m)
{
    return int(m[0][0] + m[0][1] + m[1][0] + m[1][1]);
}

int sum(float3x3 m)
{
    return int(dot(m[0] + m[1] + m[2], float3(1.0)));
}

int sum(float2x3 m)
{
    return int(dot(m[0] + m[1], float3(1.0)));
}

// NOZERO-NOT: mat{{[0-9]}}x{{[0-9]}}<f32>(0)
// SPV: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 m = float2x2(input[0], input[1], input[2], input[3]);
    float2x2 n = float2x2(input[4], input[5], input[6], input[7]);
    float3x3 m3 = float3x3(
        input[0], input[1], input[2],
        input[3], input[4], input[5],
        input[6], input[7], input[8]);
    float2x3 m23 = float2x3(input[0], input[1], input[2], input[3], input[4], input[5]);

    // WGSL-DAG: (f32(-1) * m_{{[0-9]+}})
    // BUF: -10
    outputBuffer[0] = sum(-m);

    // WGSL-DAG: (f32(-1) * m3_{{[0-9]+}})
    // BUF-NEXT: -45
    outputBuffer[1] = sum(-m3);

    // WGSL-DAG: (f32(-1) * m23_{{[0-9]+}})
    // BUF-NEXT: -21
    outputBuffer[2] = sum(-m23);

    // WGSL-DAG: (f32(-1) * (f32(-1) * n_{{[0-9]+}}))
    // BUF-NEXT: {{^}}26{{$}}
    outputBuffer[3] = sum(-(-n));

    // The operand binds looser than '*', so it keeps its own parentheses.
    // WGSL-DAG: (f32(-1) * (m_{{[0-9]+}} + n_{{[0-9]+}}))
    // BUF-NEXT: -36
    outputBuffer[4] = sum(-(m + n));

    // A negated static const matrix in mul(), the reporter's shape; it is not constant-folded.
    // WGSL-DAG: (f32(-1) * mat2x2<f32>(1.0f, 2.0f, -3.0f, 4.0f))
    // BUF-NEXT: -5
    float2 p = mul(float2(input[0], input[1]), -M2);
    outputBuffer[5] = int(p.x + p.y);
}

// HALF: enable f16;
// HALF-NOT: mat2x2<f16>(0)
// HALF: (f16(-1) * mat2x2<f16>(
[numthreads(1, 1, 1)]
void halfMain(uniform RWStructuredBuffer<half4> halfBuffer)
{
    half2x2 a = half2x2(halfBuffer[0].x, halfBuffer[0].y, halfBuffer[0].z, halfBuffer[0].w);
    half2x2 r = -a;
    halfBuffer[1] = half4(r[0], r[1]);
}

// The matrix is converted from a float matrix, so no 64-bit vector exists until legalizeMatrixTypes
// lowers the matrix itself; the resulting vector type has no source location.
//I64: disallowed vector element type
//I64: vector with disallowed element type 'int64_t' encountered
[numthreads(1, 1, 1)]
void int64Main(uniform RWStructuredBuffer<float2x2> int64Buffer)
{
    matrix<int64_t, 2, 2> m = matrix<int64_t, 2, 2>(int64Buffer[0]);
    int64Buffer[1] = float2x2(-m);
}
