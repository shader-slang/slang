//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature -Xslang -floating-point-mode -Xslang precise
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-features half -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -render-features half -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature

// Verify the half both-NaN result through public extrema witnesses on NaN-preserving paths.
// Vulkan checks instruction selection here rather than the runtime result: GLSL.std.450 permits
// special values to be ignored without SPV_KHR_float_controls2, which Slang does not yet enable.
// The sibling numeric-extrema-nan test retains Vulkan coverage of half numeric-operand selection.

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1

// SPIRV: %[[HALF:[A-Za-z0-9_]+]] = OpTypeFloat 16
// SPIRV: OpExtInst %[[HALF]] {{.*}} NMin
// SPIRV: OpExtInst %[[HALF]] {{.*}} NMax

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
//TEST_INPUT:ubuffer(data=[0x7fc00000], stride=4):name=inputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;
StructuredBuffer<uint> inputBuffer;

T numericMinimum<T : INumericExtrema>(T left, T right)
{
    return min(left, right);
}

T numericMaximum<T : INumericExtrema>(T left, T right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    half nanValue = half(asfloat(inputBuffer[0]));
    outputBuffer[0] = int(isnan(numericMinimum(nanValue, nanValue)));
    outputBuffer[1] = int(isnan(numericMaximum(nanValue, nanValue)));
}
