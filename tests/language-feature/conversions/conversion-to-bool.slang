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

// Conversions to Boolean from all fundamental scalar types

//TEST_INPUT:ubuffer(data=[0], count=65, stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static uint i = 0;

[noinline] void storeValue(bool     v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }

#ifndef NO_INT8
[noinline] void storeValue(int8_t   v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
[noinline] void storeValue(uint8_t  v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
#endif

#ifndef NO_INT16
[noinline] void storeValue(int16_t  v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
[noinline] void storeValue(uint16_t v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
#endif

[noinline] void storeValue(int32_t  v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
[noinline] void storeValue(uint32_t v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }

#ifndef NO_INT64
[noinline] void storeValue(int64_t  v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
[noinline] void storeValue(uint64_t v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
#endif

[noinline] void storeValue(half     v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
[noinline] void storeValue(float    v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }

#ifndef NO_DOUBLE
[noinline] void storeValue(double   v) { outputBuffer[i++] = ((bool)v) ? 1U : 0U; }
#endif


[numthreads(1, 1, 1)]
void computeMain()
{
    // conversions from bool
    storeValue(false);
    storeValue(true);
    // CHECK: 0
    // CHECK-NEXT: 1

    // conversions from int8_t
#ifndef NO_INT8
    storeValue(int8_t(0));
    storeValue(int8_t(-128));
    storeValue(int8_t(127));
    storeValue(int8_t(42));
#else
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from uint8_t
#ifndef NO_INT8
    storeValue(uint8_t(0));
    storeValue(uint8_t(42));
    storeValue(uint8_t(128));
    storeValue(uint8_t(255));
#else
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

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
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from uint16_t
#ifndef NO_INT16
    storeValue(uint16_t(0));
    storeValue(uint16_t(42));
    storeValue(uint16_t(128));
    storeValue(uint16_t(255));
    storeValue(uint16_t(4242));
    storeValue(uint16_t(6767));
    storeValue(uint16_t(65535));
#else
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from int32_t
    storeValue(0x00000000);
    storeValue(0x0000007F);
    storeValue(0x000000FF);
    storeValue(0x40414243);
    storeValue(int32_t(0xFEFDFCFB));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from uint32_t
    storeValue(0x00000000U);
    storeValue(0x0000007FU);
    storeValue(0x00800000U);
    storeValue(0x80000000U);
    storeValue(0xFEFDFCFBU);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from int64_t
#ifndef NO_INT64
    storeValue(0x0000000000000000LL);
    storeValue(0x000000000000007FLL);
    storeValue(0x0001000000000000LL);
    storeValue(int64_t(0x8000000000000000LL));
    storeValue(int64_t(0xF1F2F3F4FEFDFCFBLL));
#else
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from uint64_t
#ifndef NO_INT64
    storeValue(0x0000000000000000LLU);
    storeValue(0x0000000000000001LLU);
    storeValue(0x0000080000000000LLU);
    storeValue(0x8000000000000000LLU);
    storeValue(0xF1F2F3F4FEFDFCFBLLU);
#else
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from half float
    storeValue(0.0hf);
    storeValue(0.9hf);
    storeValue(126.9hf);
    storeValue(127.0hf);
    storeValue(255.0hf);
    storeValue(255.9hf);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from float
    storeValue(0.0f);
    storeValue(-0.0f);
    storeValue(0.1f);
    storeValue(127.0f);
    storeValue(255.0f);
    storeValue(1.0#INFf);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1

    // conversions from double
#ifndef NO_DOUBLE
    storeValue(0.0LF);
    storeValue(-0.0LF);
    storeValue(1.79769E+308LF);
    storeValue(-1.79769E+308LF);
    storeValue(255.0LF);
    storeValue(1.0#INFLF);
#else
    storeValue(0U);
    storeValue(0U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
    storeValue(1U);
#endif
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
}
