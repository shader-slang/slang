// This test is for issue #9007 where padding to alignment in entry point parameter structs
// causes TypeLayoutReflection::getSize() to return a padding size that, if used for sizing a parameter
// buffer, causes cuLaunchKernel to fail

// The COMPARE_COMPUTE test should continue to pass after the padding workaround in slang-rhi is removed
// The REFLECTION test is to verify that structs contained within the entry point parameter struct
// still have the trailing padding at the end of their layouts, unlike the entry point parameter struct

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target cuda -no-codegen

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=input
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
//TEST_INPUT:set count = 4
//TEST_INPUT:set padding.big = 8
//TEST_INPUT:set padding.little = 2

struct Padding
{
    uint64_t big;
    uint16_t little;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Padding padding,
    uniform StructuredBuffer<float> input,
    uniform RWStructuredBuffer<float> output,
    uniform uint count)
{
    if (tid >= count)
        return;

    // CHECK: 11.0
    // CHECK: 12.0
    // CHECK: 13.0
    // CHECK: 14.0
    output[tid] = input[tid] + padding.big + padding.little;
}

// REFLECT: "name": "padding",
// REFLECT: "binding": {"kind": "uniform", "offset": 0, "size": 16
// REFLECT: "name": "big",
// REFLECT: "binding": {"kind": "uniform", "offset": 0, "size": 8
// REFLECT: "name": "little",
// REFLECT: "binding": {"kind": "uniform", "offset": 8, "size": 2
// REFLECT: "name": "input",
// REFLECT: "binding": {"kind": "uniform", "offset": 16, "size": 16
// REFLECT: "name": "output",
// REFLECT: "binding": {"kind": "uniform", "offset": 32, "size": 16
// REFLECT: "name": "count",
// REFLECT: "binding": {"kind": "uniform", "offset": 48, "size": 4
