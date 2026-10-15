// Regression for shader-slang/slang#13446: a `bool` matrix in a buffer is stored with the layout
// reflection reports, and on SPIR-V its rows or columns hold 32-bit integers, the way a `bool`
// field does, because a storage or uniform buffer cannot contain `OpTypeBool`.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-row-major

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute -matrix-layout-column-major

// Only element `[0][1]` of `m` and element `[2][0]` of `r` are true, so a read from any other
// word, including the one a row-vector layout puts `m[0][1]` at, returns false.
struct Bools
{
    column_major bool2x3 m;
    row_major bool3x2 r;
}

//TEST_INPUT:ubuffer(data=[0 0 1 0 0 0 0 0 0 0 1 0], stride=48):name input
RWStructuredBuffer<Bools> input;

// Under std140 every row or column starts on a 16-byte boundary: `m` is three 16-byte columns and
// `r` three 16-byte rows. Here only `cbInput.m[1][0]` and `cbInput.r[0][1]` are true.
//TEST_INPUT:cbuffer(data=[0 1 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0]):name cbInput
ConstantBuffer<Bools> cbInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input[0].m[0][1] ? 1 : 0;
    output[1] = input[0].m[1][0] ? 1 : 0;
    output[2] = input[0].r[2][0] ? 1 : 0;
    output[3] = input[0].r[0][0] ? 1 : 0;
    bool2x3 whole = input[0].m;
    output[4] = whole[0][1] ? 1 : 0;
    output[5] = any(whole[1]) ? 1 : 0;

    input[0].m[1][2] = true;
    input[0].m[0][1] = false;
    output[6] = input[0].m[1][2] ? 1 : 0;
    output[7] = input[0].m[0][1] ? 1 : 0;

    output[8] = cbInput.m[1][0] ? 1 : 0;
    output[9] = cbInput.m[0][1] ? 1 : 0;
    output[10] = cbInput.r[0][1] ? 1 : 0;
}

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1

// SPIRV-DAG: %_MatrixStorage_bool2x3_ColMajorstd430 = OpTypeStruct %_arr_v2int_int_3
// SPIRV-DAG: %_MatrixStorage_bool2x3_ColMajorstd140 = OpTypeStruct %_arr_v2int_int_3_0
// SPIRV-DAG: OpMemberDecorate %Bools_std140 1 Offset 48
