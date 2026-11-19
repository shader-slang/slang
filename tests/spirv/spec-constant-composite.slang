//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// A module-scope composite whose constituents include a specialization constant
// must be emitted as OpSpecConstantComposite; OpConstantComposite may only take
// non-specialization constants. Nested composites (matrix rows, arrays of
// vectors, arrays of arrays) become OpSpecConstantComposite whenever an inner
// composite does.

// CHECK-DAG: %[[N:[0-9A-Za-z_]+]] = OpSpecConstant %int 4
// CHECK-DAG: %[[M:[0-9A-Za-z_]+]] = OpSpecConstant %int 5
// CHECK-DAG: %[[F:[0-9A-Za-z_]+]] = OpSpecConstant %float 2
// CHECK-DAG: %[[B:[0-9A-Za-z_]+]] = OpSpecConstantTrue %bool
// CHECK-DAG: %[[DIV64:[0-9A-Za-z_]+]] = OpSpecConstantOp %int SDiv %int_64 %[[N]]
// CHECK-DAG: %[[DIV32:[0-9A-Za-z_]+]] = OpSpecConstantOp %int SDiv %int_32 %[[N]]
// CHECK-DAG: %[[K:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %int_64 %[[N]]

// int quotients[2] = { 64 / N, 32 / N }
// CHECK-DAG: OpSpecConstantComposite %_arr_int_int_2 %[[DIV64]] %[[DIV32]]
// int withK[2] = { k, 3 }
// CHECK-DAG: OpSpecConstantComposite %_arr_int_int_2 %[[K]] %int_3
// int params[2] = { N + 0, M * 1 }
// CHECK-DAG: OpSpecConstantComposite %_arr_int_int_2 %[[N]] %[[M]]
// int fill[3] = { N, N, N }
// CHECK-DAG: OpSpecConstantComposite %_arr_int_int_3 %[[N]] %[[N]] %[[N]]
// int2(64 / N, 3)
// CHECK-DAG: OpSpecConstantComposite %v2int %[[DIV64]] %int_3
// bool2(B, false)
// CHECK-DAG: OpSpecConstantComposite %v2bool %[[B]] %false
// bool2(true, C), where C is a spec constant that defaults to false
// CHECK-DAG: %[[C:[0-9A-Za-z_]+]] = OpSpecConstantFalse %bool
// CHECK-DAG: OpSpecConstantComposite %v2bool %true %[[C]]
// P pairs[2] = { { 64 / N, 11 }, { 2, 3 } }
// CHECK-DAG: %[[P0:[0-9A-Za-z_]+]] = OpSpecConstantComposite %P %[[DIV64]] %int_11
// CHECK-DAG: %[[P1:[0-9A-Za-z_]+]] = OpConstantComposite %P %int_2 %int_3
// CHECK-DAG: OpSpecConstantComposite %_arr_P_int_2 %[[P0]] %[[P1]]

// Containers whose inner composite is a specialization constant. On SPIR-V an
// int2x2 is an array of int2, like int2[2]; float2x2 is a matrix of rows.
// CHECK-DAG: %[[V23:[0-9A-Za-z_]+]] = OpConstantComposite %v2int %int_2 %int_3
// CHECK-DAG: %[[V45:[0-9A-Za-z_]+]] = OpConstantComposite %v2int %int_4 %int_5
// CHECK-DAG: %[[A23:[0-9A-Za-z_]+]] = OpConstantComposite %_arr_int_int_2 %int_2 %int_3
// CHECK-DAG: OpSpecConstantComposite %_arr_v2int_int_2 %[[MROW:[0-9A-Za-z_]+]] %[[V23]]
// CHECK-DAG: %[[MROW]] = OpSpecConstantComposite %v2int %[[DIV64]] %int_1
// CHECK-DAG: OpSpecConstantComposite %_arr_v2int_int_2 %[[VROW:[0-9A-Za-z_]+]] %[[V45]]
// CHECK-DAG: %[[VROW]] = OpSpecConstantComposite %v2int %[[DIV64]] %int_1
// CHECK-DAG: OpSpecConstantComposite %_arr__arr_int_int_2_int_2 %[[NROW:[0-9A-Za-z_]+]] %[[A23]]
// CHECK-DAG: %[[NROW]] = OpSpecConstantComposite %_arr_int_int_2 %[[DIV64]] %int_1
// CHECK-DAG: %[[FROW:[0-9A-Za-z_]+]] = OpSpecConstantComposite %v2float %[[F]] %float_1
// CHECK-DAG: OpSpecConstantComposite %mat2v2float %[[FROW]]
// float2x2(F) builds its rows from the scalar.
// CHECK-DAG: %[[FSPLAT:[0-9A-Za-z_]+]] = OpSpecConstantComposite %v2float %[[F]] %[[F]]
// CHECK-DAG: OpSpecConstantComposite %mat2v2float %[[FSPLAT]] %[[FSPLAT]]

// A literal-only composite stays OpConstantComposite.
// CHECK-DAG: OpConstantComposite %_arr_int_int_2 %int_7 %int_9

[vk::constant_id(0)]
const int N = 4;
[vk::constant_id(1)]
const int M = 5;
[vk::constant_id(2)]
const float F = 2.0;
[vk::constant_id(3)]
const bool B = true;
[vk::constant_id(4)]
const bool C = false;

static const int k = 64 + N;

struct P
{
    int a;
    int b;
};

struct H
{
    static const int quotients[2] = { 64 / N, 32 / N };
    static const int withK[2] = { k, 3 };
    static const int params[2] = { N + 0, M * 1 };
    static const int2 vec = int2(64 / N, 3);
    static const int2x2 mat = int2x2(64 / N, 1, 2, 3);
    static const float2x2 fmat = float2x2(F, 1.0, 2.0, 3.0);
    static const float2x2 fsplat = float2x2(F);
    static const int fill[3] = { N, N, N };
    static const bool2 flags = bool2(B, false);
    static const int nested[2][2] = { { 64 / N, 1 }, { 2, 3 } };
    static const int2 vecs[2] = { int2(64 / N, 1), int2(4, 5) };
    static const int literals[2] = { 7, 9 };
    static const bool2 flags2 = bool2(true, C);
    static const P pairs[2] = { { 64 / N, 11 }, { 2, 3 } };
};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Indexing with the thread ID keeps each composite live instead of folding
    // the element accesses away.
    int i = int(tid.x);
    int j = i + 1;

    // N = 4, M = 5:
    // BUF: 16
    outputBuffer[0] = H.quotients[i];
    // BUF-NEXT: 8
    outputBuffer[1] = H.quotients[j];
    // BUF-NEXT: 68
    outputBuffer[2] = H.withK[i];
    // BUF-NEXT: 3
    outputBuffer[3] = H.withK[j];
    // BUF-NEXT: 4
    outputBuffer[4] = H.params[i];
    // BUF-NEXT: 5
    outputBuffer[5] = H.params[j];
    // BUF-NEXT: 16
    outputBuffer[6] = H.vec[i];
    // BUF-NEXT: 3
    outputBuffer[7] = H.vec[j];
    // BUF-NEXT: 16
    outputBuffer[8] = H.mat[i][i];
    // BUF-NEXT: {{^}}1{{$}}
    outputBuffer[9] = H.mat[i][j];
    // BUF-NEXT: 2
    outputBuffer[10] = H.mat[j][i];
    // BUF-NEXT: 16
    outputBuffer[11] = H.nested[i][i];
    // BUF-NEXT: 3
    outputBuffer[12] = H.nested[j][j];
    // BUF-NEXT: 16
    outputBuffer[13] = H.vecs[i][i];
    // BUF-NEXT: {{^}}1{{$}}
    outputBuffer[14] = H.vecs[i][j];
    // BUF-NEXT: 4
    outputBuffer[15] = H.vecs[j][i];

    // A function-local construct whose operands are all module-scope values. SPIR-V
    // legalization may hoist it to the constants section or leave it in the function,
    // so the test checks its value rather than its opcode; spirv-val checks the opcode
    // in either form. Its literal differs from `H.vec`, so its read-back values are its own.
    int2 local = int2(64 / N, 6);
    // BUF-NEXT: 16
    outputBuffer[16] = local[i];
    // BUF-NEXT: 6
    outputBuffer[17] = local[j];

    // BUF-NEXT: 2
    outputBuffer[18] = int(H.fmat[i][i]);
    // BUF-NEXT: 3
    outputBuffer[19] = int(H.fmat[j][j]);
    // BUF-NEXT: {{^}}1{{$}}
    outputBuffer[20] = H.flags[i] ? 1 : 0;
    // BUF-NEXT: 0
    outputBuffer[21] = H.flags[j] ? 1 : 0;

    // BUF-NEXT: 2
    outputBuffer[22] = int(H.fsplat[j][j]);
    // BUF-NEXT: 4
    outputBuffer[23] = H.fill[j];

    // BUF-NEXT: 7
    outputBuffer[24] = H.literals[i];
    // BUF-NEXT: 9
    outputBuffer[25] = H.literals[j];

    // BUF-NEXT: 0
    outputBuffer[26] = H.flags2[j] ? 1 : 0;
    // BUF-NEXT: 16
    outputBuffer[27] = H.pairs[i].a;
    // BUF-NEXT: 11
    outputBuffer[28] = H.pairs[i].b;
    // BUF-NEXT: 3
    outputBuffer[29] = H.pairs[j].b;
}
