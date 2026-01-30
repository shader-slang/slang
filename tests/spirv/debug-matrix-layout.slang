// Validate DebugTypeMatrix column major argument for buffer and local matrices.
// Expect matrix layout to affect buffer matrices, but not local matrices which are always row-major (ColMajor in SPIR-V).
//
//TEST:SIMPLE(filecheck=CHECK,CHECK_COL):-target spirv-asm -g2 -O0 -matrix-layout-column-major
//TEST:SIMPLE(filecheck=CHECK,CHECK_ROW):-target spirv-asm -g2 -O0 -matrix-layout-row-major

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


// Check struct members have correct matrix layouts
//CHECK_ROW-DAG: OpMemberDecorate {{.*}} 0 ColMajor
//CHECK_COL-DAG: OpMemberDecorate {{.*}} 0 RowMajor
//CHECK-DAG: OpMemberDecorate {{.*}} 1 RowMajor
//CHECK-DAG: OpMemberDecorate {{.*}} 2 ColMajor

// When checking we have the correct debug info, we verify by:
// 1. Getting the ids of the column-major and row-major DebugTypeMatrix (DTM_COLMAJOR and DTM_ROWMAJOR)
// 2. Getting the OpString id for the member/variable to check (STR_member/variable)
// 3. Check the DebugTypeMember/DebugLocalVariable has the expected id of the member/variable and id of the column-major or row-major DebugTypeMatrix

//CHECK-DAG: [[FALSE:%[a-zA-Z0-9_]+]] = OpConstantFalse %bool
//CHECK-DAG: [[TRUE:%[a-zA-Z0-9_]+]] = OpConstantTrue %bool
//CHECK-DAG: [[DTM_COLMAJOR:%[0-9]+]] = OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} [[FALSE]]
//CHECK-DAG: [[DTM_ROWMAJOR:%[0-9]+]] = OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} [[TRUE]]

// Buffer fields (capture via member -> type -> ordering)
//CHECK-DAG: [[STR_M:%[0-9]+]] = OpString "m"
//CHECK_COL-DAG: DebugTypeMember [[STR_M]] [[DTM_COLMAJOR]]
//CHECK_ROW-DAG: DebugTypeMember [[STR_M]] [[DTM_ROWMAJOR]]

//CHECK-DAG: [[STR_MCOL:%[0-9]+]] = OpString "mcol"
//CHECK-DAG: DebugTypeMember [[STR_MCOL]] [[DTM_COLMAJOR]]

//CHECK-DAG: [[STR_MROW:%[0-9]+]] = OpString "mrow"
//CHECK-DAG: DebugTypeMember [[STR_MROW]] [[DTM_ROWMAJOR]]

// Locals (capture via DebugLocalVariable -> type -> ordering)
//CHECK-DAG: [[STR_M_LOCAL:%[0-9]+]] = OpString "m_local"
//CHECK-DAG: DebugLocalVariable [[STR_M_LOCAL]] [[DTM_ROWMAJOR]]

//CHECK-DAG: [[STR_MCOL_LOCAL:%[0-9]+]] = OpString "mcol_local"
//CHECK-DAG: DebugLocalVariable [[STR_MCOL_LOCAL]] [[DTM_ROWMAJOR]]

//CHECK-DAG: [[STR_MROW_LOCAL:%[0-9]+]] = OpString "mrow_local"
//CHECK-DAG: DebugLocalVariable [[STR_MROW_LOCAL]] [[DTM_ROWMAJOR]]

