// Validate buffer storage decorations and matrix debug types for buffer and local matrices.
// Expect explicit or command-line layout to affect buffer member decorations, while every
// DebugTypeMatrix uses SPIR-V's column-vector form (Column Major = true).
//
//TEST:SIMPLE(filecheck=CHECK,CHECK_COL):-target spirv-asm -g2 -matrix-layout-column-major
//TEST:SIMPLE(filecheck=CHECK,CHECK_ROW):-target spirv-asm -g2 -matrix-layout-row-major
//TEST:SIMPLE(filecheck=CHECK_NO_ROW_VECTOR):-target spirv-asm -g2 -matrix-layout-column-major

struct S
{
                 float3x2 m;    // default (will be set by command-line option)
    column_major float3x2 mcol; // explicit column-major
    row_major    float3x2 mrow; // explicit row-major
};

RWStructuredBuffer<S> outputBuffer;

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

    outputBuffer[0].mcol = mcol_local;
    outputBuffer[0].m    = m_local;
    outputBuffer[0].mrow = mrow_local;
}


// Check struct members have correct matrix layouts.
//CHECK_ROW-DAG: OpMemberDecorate {{.*}} 0 ColMajor
//CHECK_COL-DAG: OpMemberDecorate {{.*}} 0 RowMajor
//CHECK-DAG: OpMemberDecorate {{.*}} 1 RowMajor
//CHECK-DAG: OpMemberDecorate {{.*}} 2 ColMajor

// A float3x2 SPIR-V matrix value is three vec2 columns. To verify the debug info:
// 1. Capture that canonical DebugTypeMatrix ID.
// 2. Capture each buffer member or local variable's OpString ID.
// 3. Verify its DebugTypeMember or DebugLocalVariable refers to that canonical type.
//CHECK-DAG: [[TWO:%[a-zA-Z0-9_]+]] = OpConstant %uint 2{{$}}
//CHECK-DAG: [[THREE:%[a-zA-Z0-9_]+]] = OpConstant %uint 3{{$}}
//CHECK-DAG: [[TRUE:%[a-zA-Z0-9_]+]] = OpConstantTrue %bool
//CHECK-DAG: [[VEC2:%[0-9]+]] = OpExtInst %void {{.*}} DebugTypeVector {{.*}} [[TWO]]
//CHECK-DAG: [[DTM:%[0-9]+]] = OpExtInst %void {{.*}} DebugTypeMatrix [[VEC2]] [[THREE]] [[TRUE]]

// Buffer members all refer to the canonical matrix debug type.
//CHECK-DAG: [[STR_M:%[0-9]+]] = OpString "m"
//CHECK-DAG: DebugTypeMember [[STR_M]] [[DTM]]

//CHECK-DAG: [[STR_MCOL:%[0-9]+]] = OpString "mcol"
//CHECK-DAG: DebugTypeMember [[STR_MCOL]] [[DTM]]

//CHECK-DAG: [[STR_MROW:%[0-9]+]] = OpString "mrow"
//CHECK-DAG: DebugTypeMember [[STR_MROW]] [[DTM]]

// Locals refer to the same canonical matrix debug type.
//CHECK-DAG: [[STR_M_LOCAL:%[0-9]+]] = OpString "m_local"
//CHECK-DAG: DebugLocalVariable [[STR_M_LOCAL]] [[DTM]]

//CHECK-DAG: [[STR_MCOL_LOCAL:%[0-9]+]] = OpString "mcol_local"
//CHECK-DAG: DebugLocalVariable [[STR_MCOL_LOCAL]] [[DTM]]

//CHECK-DAG: [[STR_MROW_LOCAL:%[0-9]+]] = OpString "mrow_local"
//CHECK-DAG: DebugLocalVariable [[STR_MROW_LOCAL]] [[DTM]]

// Reject the unsupported row-vector DebugTypeMatrix form anywhere in the module.
//CHECK_NO_ROW_VECTOR: OpCapability Shader
//CHECK_NO_ROW_VECTOR-NOT: OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} %false
//CHECK_NO_ROW_VECTOR: OpFunctionEnd

