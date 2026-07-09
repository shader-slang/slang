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

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgsl -compute -output-using-type -xslang -DNO_DOUBLE=1 -xslang -DNO_INT8=1 -xslang -DNO_INT16=1 -xslang -DNO_INT64=1
// Note: WGSL has only the 32-bit integers and 16-bit and 32-bit floats

// Conversions to 32-bit float from all fundamental scalar types
//
// In double-to-float and integer-to-float conversions, precisely
// convertible values are used to avoid the effects of rounding. The
// rounding behavior is implementation-defined at least on SPIR-V targets.

//TEST_INPUT:ubuffer(data=[0], count=71, stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

static uint i = 0;

[noinline] void storeValue(bool     v) { outputBuffer[i++] = (float)v; }

#ifndef NO_INT8
[noinline] void storeValue(int8_t   v) { outputBuffer[i++] = (float)v; }
[noinline] void storeValue(uint8_t  v) { outputBuffer[i++] = (float)v; }
#endif

#ifndef NO_INT16
[noinline] void storeValue(int16_t  v) { outputBuffer[i++] = (float)v; }
[noinline] void storeValue(uint16_t v) { outputBuffer[i++] = (float)v; }
#endif

[noinline] void storeValue(int32_t  v) { outputBuffer[i++] = (float)v; }
[noinline] void storeValue(uint32_t v) { outputBuffer[i++] = (float)v; }

#ifndef NO_INT64
[noinline] void storeValue(int64_t  v) { outputBuffer[i++] = (float)v; }
[noinline] void storeValue(uint64_t v) { outputBuffer[i++] = (float)v; }
#endif

[noinline] void storeValue(half     v) { outputBuffer[i++] = (float)v; }
[noinline] void storeValue(float    v) { outputBuffer[i++] = (float)v; }

#ifndef NO_DOUBLE
[noinline] void storeValue(double   v) { outputBuffer[i++] = (float)v; }
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
    storeValue(float(-128.0));
    storeValue(float(-42.0));
    storeValue(float(67.0));
    storeValue(float(127.0));
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
    storeValue(float(0.0));
    storeValue(float(42.0));
    storeValue(float(128.0));
    storeValue(float(255.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 42.0
    // CHECK-NEXT: 128.0
    // CHECK-NEXT: 255.0

    // conversions from int16_t
#ifndef NO_INT16
    storeValue(int16_t(-32768));
    storeValue(int16_t(-4242));
    storeValue(int16_t(-128));
    storeValue(int16_t(-1));
    storeValue(int16_t(0));
    storeValue(int16_t(1));
    storeValue(int16_t(67));
    storeValue(int16_t(127));
    storeValue(int16_t(6767));
    storeValue(int16_t(32767));
#else
    storeValue(float(-32768.0));
    storeValue(float(-4242.0));
    storeValue(float(-128.0));
    storeValue(float(-1.0));
    storeValue(float(0.0));
    storeValue(float(1.0));
    storeValue(float(67.0));
    storeValue(float(127.0));
    storeValue(float(6767.0));
    storeValue(float(32767.0));
#endif
    // CHECK-NEXT: -32768.0
    // CHECK-NEXT: -4242.0
    // CHECK-NEXT: -128.0
    // CHECK-NEXT: -1.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 67.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 6767.0
    // CHECK-NEXT: 32767.0

    // conversions from uint16_t
#ifndef NO_INT16
    storeValue(uint16_t(0));
    storeValue(uint16_t(42));
    storeValue(uint16_t(128));
    storeValue(uint16_t(255));
    storeValue(uint16_t(32767));
    storeValue(uint16_t(32768));
    storeValue(uint16_t(65535));
#else
    storeValue(float(0.0));
    storeValue(float(42.0));
    storeValue(float(128.0));
    storeValue(float(255.0));
    storeValue(float(32767.0));
    storeValue(float(32768.0));
    storeValue(float(65535.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 42.0
    // CHECK-NEXT: 128.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 32767.0
    // CHECK-NEXT: 32768.0
    // CHECK-NEXT: 65535.0

    // conversions from int32_t
    storeValue(0x00000000);
    storeValue(0x0000007F);
    storeValue(0x000000FF);
    storeValue(1078018688);
    storeValue(-16909060);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1078018688.0
    // CHECK-NEXT: -16909060.0

    // conversions from uint32_t
    storeValue(0x00000000U);
    storeValue(0x0000007FU);
    storeValue(0x000000FFU);
    storeValue(1078018688U);
    storeValue(4278058240U);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1078018688.0
    // CHECK-NEXT: 4278058240.0

    // conversions from int64_t
#ifndef NO_INT64
    storeValue(0x0000000000000000LL);
    storeValue(0x000000000000007FLL);
    storeValue(0x00000000000000FFLL);
    storeValue(1230066676385447936LL);
    storeValue(-1012478754087239680LL);
#else
    storeValue(float(0.0));
    storeValue(float(127.0));
    storeValue(float(255.0));
    storeValue(float(1230066676385447936.0));
    storeValue(float(-1012478754087239680.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1230066676385447936.0
    // CHECK-NEXT: -1012478754087239680.0

    // conversions from uint64_t
#ifndef NO_INT64
    storeValue(0x0000000000000000LLU);
    storeValue(0x000000000000007FLLU);
    storeValue(0x00000000000000FFLLU);
    storeValue(1230066676385447936LLU);
    storeValue(17434265388341788672LLU);
#else
    storeValue(float(0.0));
    storeValue(float(127.0));
    storeValue(float(255.0));
    storeValue(float(1230066676385447936.0));
    storeValue(float(17434265388341788672.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1230066676385447936.0
    // CHECK-NEXT: 17434265388341788672.0

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
    storeValue(-9223372036854775808.0f); // minimum int64_t number is representable by float
    storeValue(-32768.0f);
    storeValue(-67.25f);
    storeValue(127.9375f);
    storeValue(42.25f);
    storeValue(85.0f);
    storeValue(32767.0f);
    storeValue(9223371487098961920.0f); // maximum int64_t number representable by float
    // CHECK-NEXT: -9223372036854775808.0
    // CHECK-NEXT: -32768.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 32767.0
    // CHECK-NEXT: 9223371487098961920.0

    // conversions from double
#ifndef NO_DOUBLE
    storeValue(-9223372036854775808.0LF); // minimum int64_t number is representable by float
    storeValue(-32768.0LF);
    storeValue(-67.25LF);
    storeValue(127.9375LF);
    storeValue(42.25LF);
    storeValue(85.0LF);
    storeValue(32767.0LF);
    storeValue(9223371487098961920.0LF);  // maximum int64_t number representable by float
#else
    storeValue(float(-9223372036854775808.0));
    storeValue(float(-32768.0));
    storeValue(float(-67.25));
    storeValue(float(127.9375));
    storeValue(float(42.25));
    storeValue(float(85.0));
    storeValue(float(32767.0));
    storeValue(float(9223371487098961920.0));
#endif
    // CHECK-NEXT: -9223372036854775808.0
    // CHECK-NEXT: -32768.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 32767.0
    // CHECK-NEXT: 9223371487098961920.0
}
