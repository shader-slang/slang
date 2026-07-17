//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type -xslang -DNO_INT8=1
// Notes:
// - HLSL does not have 8-bit integer types
// - HLSL has 16-bit integers when enabled (Slang does it automatically for the default profile)

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

// Note: Metal does not have the double type, so metal is not tested here

// Note: WGSL does not have the double type, so wgsl is not tested here

// Conversions to 64-bit double from all fundamental scalar types
//
// In 64-bit-integer-to-double conversions, precisely convertible values are
// used to avoid the effects of rounding. The rounding behavior is
// implementation-defined at least on SPIR-V targets.

//TEST_INPUT:ubuffer(data=[0], count=71, stride=8):out,name=outputBuffer
RWStructuredBuffer<double> outputBuffer;

static uint i = 0;

[noinline] void storeValue(bool     v) { outputBuffer[i++] = (double)v; }

#ifndef NO_INT8
[noinline] void storeValue(int8_t   v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(uint8_t  v) { outputBuffer[i++] = (double)v; }
#endif

[noinline] void storeValue(int16_t  v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(uint16_t v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(int32_t  v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(uint32_t v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(int64_t  v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(uint64_t v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(half     v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(float    v) { outputBuffer[i++] = (double)v; }
[noinline] void storeValue(double   v) { outputBuffer[i++] = (double)v; }


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
    storeValue(double(-128.0));
    storeValue(double(-42.0));
    storeValue(double(67.0));
    storeValue(double(127.0));
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
    storeValue(double(0.0));
    storeValue(double(42.0));
    storeValue(double(128.0));
    storeValue(double(255.0));
#endif
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 42.0
    // CHECK-NEXT: 128.0
    // CHECK-NEXT: 255.0

    // conversions from int16_t
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
    storeValue(uint16_t(0));
    storeValue(uint16_t(42));
    storeValue(uint16_t(128));
    storeValue(uint16_t(255));
    storeValue(uint16_t(32767));
    storeValue(uint16_t(32768));
    storeValue(uint16_t(65535));
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
    storeValue(0x40414243);
    storeValue(int32_t(0xFEFDFCFB));
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1078018627.0
    // CHECK-NEXT: -16909061.0

    // conversions from uint32_t
    storeValue(0x00000000U);
    storeValue(0x0000007FU);
    storeValue(0x000000FFU);
    storeValue(0x40414243U);
    storeValue(0xFEFDFCFBU);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1078018627.0
    // CHECK-NEXT: 4278058235.0

    // conversions from int64_t
    storeValue(0x0000000000000000LL);
    storeValue(0x000000000000007FLL);
    storeValue(0x00000000000000FFLL);
    storeValue(1230066625923858944LL);
    storeValue(-1012478732629312256LL);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1230066625923858944.0
    // CHECK-NEXT: -1012478732629312256.0

    // conversions from uint64_t
    storeValue(0x0000000000000000LLU);
    storeValue(0x000000000000007FLLU);
    storeValue(0x00000000000000FFLLU);
    storeValue(1230066625923858944LLU);
    storeValue(17434265341080240128LLU);
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 127.0
    // CHECK-NEXT: 255.0
    // CHECK-NEXT: 1230066625923858944.0
    // CHECK-NEXT: 17434265341080240128.0

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
    storeValue(-9223372036854775808.0LF); // minimum int64_t number is representable by double
    storeValue(-32768.0LF);
    storeValue(-67.25LF);
    storeValue(127.9375LF);
    storeValue(42.25LF);
    storeValue(85.0LF);
    storeValue(32767.0LF);
    storeValue(9223372036854774784.0LF);  // maximum int64_t number representable by double
    // CHECK-NEXT: -9223372036854775808.0
    // CHECK-NEXT: -32768.0
    // CHECK-NEXT: -67.25
    // CHECK-NEXT: 127.9375
    // CHECK-NEXT: 42.25
    // CHECK-NEXT: 85.0
    // CHECK-NEXT: 32767.0
    // CHECK-NEXT: 9223372036854774784.0
}
