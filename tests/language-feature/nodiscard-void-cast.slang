// Language feature test: void-cast to prevent a `[NoDiscard]` warning
//
// A `(void)` cast is the idiomatic way to silence the `[NoDiscard]`
// discarded-result error: casting the result to `void` counts as *using*
// it, so `(void)incrementValue(...)` type-checks without the error.
//
// This test verifies the following:
// - a `(void)` cast compiles (which was not the case before GitHub issue 11315 was fixed)
// - `void`-casting the return value of a `[NoDiscard]` function silences the related warning
// - a `(void)` cast does not remove the side effects of the expression cast to void
//
// See also https://github.com/shader-slang/slang/issues/11315

//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -warnings-as-errors all
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry computeMain -stage compute -warnings-as-errors all
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK_BUF):-vk -compute -output-using-type -emit-spirv-directly -xslang -warnings-as-errors -xslang all

//TEST_INPUT:ubuffer(data=[12345678], count=1, stride=4):out,name=output
RWStructuredBuffer<uint> output;

enum StatusCode
{
    Success = 0,
    Overflow = 1
}

[NoDiscard] StatusCode incrementValue(inout uint val)
{
    uint prevVal = val;

    ++val;

    // overflow detection
    return prevVal < val ? StatusCode.Success : StatusCode.Overflow;
}

// Compilation success checks:
// CHECK_SPIRV: OpEntryPoint
// CHECK_HLSL: void computeMain

// Side effect check
// CHECK_BUF: 12345679

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // ignore overflow status code
    (void)incrementValue(output[0]);
}
