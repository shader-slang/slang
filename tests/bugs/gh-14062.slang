// Generic code over `__BuiltinFloatingPointType` converts a `double` value to `T` with a
// single rounding step. In the `double` specialization, `T(0.1l)` and `T(1e-300l)` keep their
// full precision instead of being rounded through `float` first, which turned `0.1l` into
// 0.10000000149011612 and `1e-300l` into 0 (shader-slang/slang#14062).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -render-feature double,int64,int16,half

//TEST_INPUT:ubuffer(data=[1], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0], count=10, stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

T tenth<T : __BuiltinFloatingPointType>() { return T(0.1l); }
T tenthUnsuffixed<T : __BuiltinFloatingPointType>() { return T(0.1); }
T tiny<T : __BuiltinFloatingPointType>() { return T(1e-300l); }
T fromDouble<T : __BuiltinFloatingPointType>(double value) { return T(value); }

[numthreads(1, 1, 1)]
void computeMain()
{
    // A run-time value, so that the conversion in `fromDouble` cannot be folded.
    double third = double(inputBuffer[0]) / 3.0l;

    // Bits of 0.1 as a `double` (0x3FB999999999999A), not of `double(0.1f)`.
    // CHECK: 4591870180066957722
    outputBuffer[0] = bit_cast<uint64_t>(tenth<double>());

    // Bits of 1e-300 as a `double`, not 0.
    // CHECK-NEXT: 118622047889322841
    outputBuffer[1] = bit_cast<uint64_t>(tiny<double>());

    // Bits of 1.0/3.0 as a `double`.
    // CHECK-NEXT: 4599676419421066581
    outputBuffer[2] = bit_cast<uint64_t>(fromDouble<double>(third));

    // The `float` and `half` specializations get the `float` and `half` values of the literals.
    // CHECK-NEXT: 1036831949
    outputBuffer[3] = bit_cast<uint32_t>(tenth<float>());
    // CHECK-NEXT: 0
    outputBuffer[4] = bit_cast<uint32_t>(tiny<float>());
    // CHECK-NEXT: 11878
    outputBuffer[5] = bit_cast<uint16_t>(tenth<half>());

    // An unsuffixed literal has type `float`, so the generic `T(0.1)` gives the same value as the
    // concrete `double(0.1)`: both are `double(0.1f)` (0x3FB99999A0000000), and this test pins that
    // unchanged value.
    // CHECK-NEXT: 1
    outputBuffer[6] = bit_cast<uint64_t>(tenthUnsuffixed<double>()) == bit_cast<uint64_t>(double(0.1)) ? 1 : 0;
    // CHECK-NEXT: 4591870180174331904
    outputBuffer[7] = bit_cast<uint64_t>(tenthUnsuffixed<double>());

    // In the `float` and `half` specializations, a run-time `double` converts the same way as the
    // concrete `float(d)` and `half(d)`. For `half`, 1 + 2^-11 + 2^-40 tells the paths apart: rounded
    // once it gives 0x3C01 on CUDA and Vulkan, rounded through `float` first it gives 0x3C00.
    double nearHalfTie = 1.0l + 1.0l / 2048.0l + 1.0l / 1099511627776.0l + double(inputBuffer[0] - 1);
    // CHECK-NEXT: 1
    outputBuffer[8] = bit_cast<uint32_t>(fromDouble<float>(nearHalfTie)) == bit_cast<uint32_t>(float(nearHalfTie)) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[9] = bit_cast<uint16_t>(fromDouble<half>(nearHalfTie)) == bit_cast<uint16_t>(half(nearHalfTie)) ? 1 : 0;
}
