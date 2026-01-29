// Validate DebugTypeMatrix column major argument for local and function parameter and return value matrices.
// Expect matrix layout to always be row-major as the layout modifier only applies to buffer types (ConstantBuffer/RWStructuredBuffer/etc).
//
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -O0 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -O0 -matrix-layout-column-major

struct S
{
    float3x2      m;            // default (will be set by command-line option)
    column_major float3x2 mcol; // explicit column-major
    row_major float3x2 mrow;    // explicit row-major
};

RWStructuredBuffer<float> outputBuffer;

float3x2 ProcessMatrix(float3x2 in)
{
    float3x2 out = in + in;
    return out;
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    float3x2 m_local = float3x2(
        float2(1.0, 2.0),
        float2(3.0, 4.0),
        float2(5.0, 6.0));

    column_major float3x2 mcol_local = float3x2(
        float2(1.0, 2.0),
        float2(3.0, 4.0),
        float2(5.0, 6.0));

    row_major float3x2 mrow_local = float3x2(
        float2(1.0, 2.0),
        float2(3.0, 4.0),
        float2(5.0, 6.0));

    outputBuffer[0] = mcol_local[0][0];
    outputBuffer[1] = m_local[0][0];
    outputBuffer[2] = mrow_local[0][0];

    S s_local;
    s_local.m = ProcessMatrix(m_local);
    s_local.mcol = ProcessMatrix(mcol_local);
    s_local.mrow = ProcessMatrix(mrow_local);

    outputBuffer[3] = s_local.m[0][0];
    outputBuffer[4] = s_local.mcol[0][0];
    outputBuffer[5] = s_local.mrow[0][0];
}


// Check there is no debug info with the SPIR-V DebugTypeMatrix column-major operand false (meaning all matrices are row-major)
//CHECK-NOT: OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} %false
//CHECK: OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} %true
