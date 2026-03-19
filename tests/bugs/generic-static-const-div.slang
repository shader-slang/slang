// generic-static-const-div.slang

// Regression test for https://github.com/shader-slang/slang/pull/10391
//
// When a generic struct has a `static const int` computed via operations
// (like `/` or `?:`) on generic value params or interface requirements,
// the constant folder produces a FuncCallIntVal. The lowering must not
// route these through lowerSimpleVal, because the resulting constexpr IR
// ops are hoistable and their operands (generic params) become invalid
// after specialization, leading to null-operand crashes.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-DAG: OpConstant %uint 771
// CHECK-DAG: OpConstant %uint 192
// CHECK-DAG: OpConstant %uint 193
// CHECK-DAG: OpConstant %uint 20

interface IVec
{
    static const int Size;
}

struct Vec<let N : int> : IVec
{
    static const int Size = N;
}

struct Layer<InputVec : IVec, OutputVec : IVec, let HasBias : bool = true>
{
    static const int ParameterCount =
        OutputVec.Size * InputVec.Size + (HasBias ? OutputVec.Size : 0);

    static const int HalfParams = ParameterCount / 2;
}

typealias L1 = Layer<Vec<2>, Vec<128>>;
typealias L2 = Layer<Vec<128>, Vec<3>>;
typealias L3 = Layer<Vec<4>, Vec<5>, false>;

static const int TotalParams = L1.ParameterCount + L2.ParameterCount;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // L1: 128*2 + 128 = 384, L2: 3*128 + 3 = 387
    // Total = 384 + 387 = 771
    // BUF: 771
    outputBuffer[0] = TotalParams;
    // L1.HalfParams = 384/2 = 192
    // BUF-NEXT: 192
    outputBuffer[1] = L1.HalfParams;
    // L2.HalfParams = 387/2 = 193
    // BUF-NEXT: 193
    outputBuffer[2] = L2.HalfParams;
    // L3: 5*4 + 0 = 20 (HasBias = false)
    // BUF-NEXT: 20
    outputBuffer[3] = L3.ParameterCount;
}
