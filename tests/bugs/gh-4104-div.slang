//TEST:SIMPLE(filecheck=SOURCE): -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=SOURCE): -stage compute -entry computeMain -target glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -dx12 -compute -entry computeMain -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -compute -entry computeMain -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -compute -entry computeMain -output-using-type -emit-spirv-directly

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Note, it appears that -2 works fine.
static const uint32_t u32Const = uint(0xffffffff); // -1 or max
static const int32_t i32Const = 1;
static const uint64_t u64Const = 0xffffffffffffffffULL; // -1 or max
static const int64_t i64Const = 1;

bool Test_CompileTimeDiv()
{
    // SOURCE-LABEL: Test_CompileTimeDiv
    // SOURCE-NOT: % {{.*}}-
    return true
        && ((1u / u32Const) == 0u)
        && ((1 / u32Const) == 0)
        && ((uint32_t(1) / u32Const) == uint32_t(0))
        && ((int32_t(1) / u32Const) == uint32_t(0))
        && ((i32Const / u32Const) == uint32_t(0))
        && ((uint32_t(1) / u64Const) == uint64_t(0))
        && ((int32_t(1) / u64Const) == uint64_t(0))
        && ((i32Const / u64Const) == uint64_t(0))
        && ((uint64_t(1) / u64Const) == uint64_t(0))
        && ((int64_t(1) / u64Const) == uint64_t(0))
        && ((i64Const / u64Const) == uint64_t(0))
        ;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // BUF:1
    // SOURCE-LABEL: Test_CompileTimeDiv
    outputBuffer[0] = int(true
        && uint(-1) == u32Const
        && uint64_t(-1) == u64Const
        && Test_CompileTimeDiv());
    // outputBuffer[0] = (int)((i64Const / u64Const) & 0xffffffff)
}
