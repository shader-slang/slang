//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// See https://github.com/shader-slang/slang/issues/11996

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type -xslang -DNO_INT8=1
// Notes:
// - HLSL does not have 8-bit integer types
// - HLSL has 16-bit integers when enabled (Slang does it automatically for the default profile)

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type -xslang -DNO_DOUBLE=1
// Note: Metal does not have the double type

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgsl -compute -output-using-type -render-feature half -xslang -DNO_DOUBLE=1 -xslang -DNO_INT8=1 -xslang -DNO_INT16=1 -xslang -DNO_INT64=1
// Note: WGSL has only the 32-bit integers and 16-bit and 32-bit floats.
// The 16-bit float (half) requires the f16 WGSL extension, so gate the test
// on the "half" render feature.

// Conversions to 16-bit float from all fundamental scalar types
//
// In double-to-half, float-to-half, and integer-to-half conversions, precisely
// convertible values are used to avoid the effects of rounding. The
// rounding behavior is implementation-defined at least on SPIR-V targets.

//TEST_INPUT:ubuffer(data=[0], count=71, stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

static uint i = 0;

[noinline] void storeValue(bool     v) { outputBuffer[i++] = (half)v; }

#ifndef NO_INT8
[noinline] void storeValue(int8_t   v) { outputBuffer[i++] = (half)v; }
[noinline] void storeValue(uint8_t  v) { outputBuffer[i++] = (half)v; }
#endif

#ifndef NO_INT16
[noinline] void storeValue(int16_t  v) { outputBuffer[i++] = (half)v; }
[noinline] void storeValue(uint16_t v) { outputBuffer[i++] = (half)v; }
#endif

[noinline] void storeValue(int32_t  v) { outputBuffer[i++] = (half)v; }
[noinline] void storeValue(uint32_t v) { outputBuffer[i++] = (half)v; }

#ifndef NO_INT64
[noinline] void storeValue(int64_t  v) { outputBuffer[i++] = (half)v; }
[noinline] void storeValue(uint64_t v) { outputBuffer[i++] = (half)v; }
#endif

[noinline] void storeValue(half     v) { outputBuffer[i++] = (half)v; }
[noinline] void storeValue(float    v) { outputBuffer[i++] = (half)v; }

#ifndef NO_DOUBLE
[noinline] void storeValue(double   v) { outputBuffer[i++] = (half)v; }
#endif


[numthreads(1, 1, 1)]
void computeMain()
{
    // conversions from bool
    storeValue(false);
    storeValue(true);
    // CHECK: 0.0
    // CHECK-NEXT: 1.0

    // conversions from int8_t
#ifndef NO_INT8
    storeValue(int8_t(-128));
    storeValue(int8_t(-42));
    storeValue(int8_t(67));
    storeValue(int8_t(127));
#else
    storeValue(half(-128.0));
    storeValue(half(-42.0));
    storeValue(half(67.0));
    storeValue(half(127.0));
#endif
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -42.0
    // CHECK-NEXT: 67.0
    // CHECK-NEXT: 127.0

    // conversions from uint8_t
#ifndef NO_INT8
    storeValue(uint8_t(0));
    storeValue(uint8_t(42));
    storeValue(uint8_t(128));
    storeValue(uint8_t(255));
#else
    storeValue(half(0.0));
    storeValue(half(42.0));
    storeValue(half(128.0));
    storeValue(half(255.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 42.0
    // CHECK-NEXT: 128.0
    // CHECK-NEXT: 255.0

    // conversions from int16_t
#ifndef NO_INT16
    storeValue(int16_t(-32768));
    storeValue(int16_t(-4240));
    storeValue(int16_t(-128));
    storeValue(int16_t(-1));
    storeValue(int16_t(0));
    storeValue(int16_t(1));
    storeValue(int16_t(67));
    storeValue(int16_t(127));
    storeValue(int16_t(6768));
    storeValue(int16_t(32752));
#else
    storeValue(half(-32768.0));
    storeValue(half(-4240.0));
    storeValue(half(-128.0));
    storeValue(half(-1.0));
    storeValue(half(0.0));
    storeValue(half(1.0));
    storeValue(half(67.0));
    storeValue(half(127.0));
    storeValue(half(6768.0));
    storeValue(half(32752.0));
#endif
    // CHECK-NEXT: -32768.0
    // CHECK-NEXT: -4240.0
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -1.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 67.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 6768.0
    // CHECK-NEXT: 32752.0

    // conversions from uint16_t
#ifndef NO_INT16
    storeValue(uint16_t(0));
    storeValue(uint16_t(42));
    storeValue(uint16_t(128));
    storeValue(uint16_t(255));
    storeValue(uint16_t(32752));
    storeValue(uint16_t(32768));
    storeValue(uint16_t(65504));
#else
    storeValue(half(0.0));
    storeValue(half(42.0));
    storeValue(half(128.0));
    storeValue(half(255.0));
    storeValue(half(32752.0));
    storeValue(half(32768.0));
    storeValue(half(65504.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 42.0
    // CHECK-NEXT: 128.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 32752.0
    // CHECK-NEXT: 32768.0
    // CHECK-NEXT: 65504.0

    // conversions from int32_t
    storeValue(0x00000000);
    storeValue(0x0000007F);
    storeValue(0x000000FF);
    storeValue(65504);
    storeValue(-16912);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 65504.0
    // CHECK-NEXT: -16912.0

    // conversions from uint32_t
    storeValue(0x00000000U);
    storeValue(0x0000007FU);
    storeValue(0x000000FFU);
    storeValue(65504U);
    storeValue(16912U);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 65504.0
    // CHECK-NEXT: 16912.0

    // conversions from int64_t
#ifndef NO_INT64
    storeValue(0x00000000LL);
    storeValue(0x0000007FLL);
    storeValue(0x000000FFLL);
    storeValue(65504LL);
    storeValue(-16912LL);
#else
    storeValue(half(0.0));
    storeValue(half(127.0));
    storeValue(half(255.0));
    storeValue(half(65504.0));
    storeValue(half(-16912.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 65504.0
    // CHECK-NEXT: -16912.0

    // conversions from uint64_t
#ifndef NO_INT64
    storeValue(0x00000000LLU);
    storeValue(0x0000007FLLU);
    storeValue(0x000000FFLLU);
    storeValue(65504LLU);
    storeValue(16912LLU);
#else
    storeValue(half(0.0));
    storeValue(half(127.0));
    storeValue(half(255.0));
    storeValue(half(65504.0));
    storeValue(half(16912.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 65504.0
    // CHECK-NEXT: 16912.0

    // conversions from half float
    storeValue(-65504.0hf);
    storeValue(-128.0hf);
    storeValue(-67.25hf);
    storeValue(127.9375hf);
    storeValue(42.25hf);
    storeValue(85.0hf);
    storeValue(127.0hf);
    storeValue(65504.0hf);
    // CHECK-NEXT: -65504.0
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 65504.0

    // conversions from float
    storeValue(-65504.0f);
    storeValue(-128.0f);
    storeValue(-67.25f);
    storeValue(127.9375f);
    storeValue(42.25f);
    storeValue(85.0f);
    storeValue(127.0f);
    storeValue(65504.0f);
    // CHECK-NEXT: -65504.0
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 65504.0

    // conversions from double
#ifndef NO_DOUBLE
    storeValue(-65504.0LF);
    storeValue(-128.0LF);
    storeValue(-67.25LF);
    storeValue(127.9375LF);
    storeValue(42.25LF);
    storeValue(85.0LF);
    storeValue(127.0LF);
    storeValue(65504.0LF);
#else
    storeValue(half(-65504.0));
    storeValue(half(-128.0));
    storeValue(half(-67.25));
    storeValue(half(127.9375));
    storeValue(half(42.25));
    storeValue(half(85.0));
    storeValue(half(127.0));
    storeValue(half(65504.0));
#endif
    // CHECK-NEXT: -65504.0
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 65504.0
}
