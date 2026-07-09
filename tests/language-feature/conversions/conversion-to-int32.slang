//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// See https://github.com/shader-slang/slang/issues/11996

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// Conversions to 32-bit signed integer from all fundamental scalar types

//TEST_INPUT:ubuffer(data=[0], count=71, stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

static uint i = 0;

[noinline] void storeValue(bool     v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(int8_t   v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(uint8_t  v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(int16_t  v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(uint16_t v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(int32_t  v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(uint32_t v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(int64_t  v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(uint64_t v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(half     v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(float    v) { outputBuffer[i++] = (int32_t)v; }
[noinline] void storeValue(double   v) { outputBuffer[i++] = (int32_t)v; }


[numthreads(1, 1, 1)]
void computeMain()
{
    // conversions from bool
    storeValue(false);
    storeValue(true);
    // CHECK: 0
    // CHECK-NEXT: 1

    // conversions from int8_t
    storeValue(int8_t(-128));
    storeValue(int8_t(-42));
    storeValue(int8_t(67));
    storeValue(int8_t(127));
    // CHECK-NEXT: -128
    // CHECK-NEXT: -42
    // CHECK-NEXT: 67
    // CHECK-NEXT: 127

    // conversions from uint8_t
    storeValue(uint8_t(0));
    storeValue(uint8_t(42));
    storeValue(uint8_t(128));
    storeValue(uint8_t(255));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 42
    // CHECK-NEXT: 128
    // CHECK-NEXT: 255

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
    // CHECK-NEXT: -32768
    // CHECK-NEXT: -4242
    // CHECK-NEXT: -128
    // CHECK-NEXT: -1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 67
    // CHECK-NEXT: 127
    // CHECK-NEXT: 6767
    // CHECK-NEXT: 32767

    // conversions from uint16_t
    storeValue(uint16_t(0));
    storeValue(uint16_t(42));
    storeValue(uint16_t(128));
    storeValue(uint16_t(255));
    storeValue(uint16_t(32767));
    storeValue(uint16_t(32768));
    storeValue(uint16_t(65535));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 42
    // CHECK-NEXT: 128
    // CHECK-NEXT: 255
    // CHECK-NEXT: 32767
    // CHECK-NEXT: 32768
    // CHECK-NEXT: 65535

    // conversions from int32_t
    storeValue(0x00000000);
    storeValue(0x0000007F);
    storeValue(0x000000FF);
    storeValue(0x40414243);
    storeValue(int32_t(0xFEFDFCFB));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 127
    // CHECK-NEXT: 255
    // CHECK-NEXT: 1078018627
    // CHECK-NEXT: -16909061

    // conversions from uint32_t
    storeValue(0x00000000U);
    storeValue(0x0000007FU);
    storeValue(0x000000FFU);
    storeValue(0x40414243U);
    storeValue(0xFEFDFCFBU);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 127
    // CHECK-NEXT: 255
    // CHECK-NEXT: 1078018627
    // CHECK-NEXT: -16909061

    // conversions from int64_t
    storeValue(0x0000000000000000LL);
    storeValue(0x000000000000007FLL);
    storeValue(0x00000000000000FFLL);
    storeValue(0x1112131440414243LL);
    storeValue(int64_t(0xF1F2F3F4FEFDFCFBLL));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 127
    // CHECK-NEXT: 255
    // CHECK-NEXT: 1078018627
    // CHECK-NEXT: -16909061

    // conversions from uint64_t
    storeValue(0x0000000000000000LLU);
    storeValue(0x000000000000007FLLU);
    storeValue(0x00000000000000FFLLU);
    storeValue(0x1112131440414243LLU);
    storeValue(0xF1F2F3F4FEFDFCFBLLU);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 127
    // CHECK-NEXT: 255
    // CHECK-NEXT: 1078018627
    // CHECK-NEXT: -16909061

    // conversions from half float
    storeValue(-65504.0hf);
    storeValue(-128.0hf);
    storeValue(-67.9hf);
    storeValue(0.001hf);
    storeValue(42.9hf);
    storeValue(85.0hf);
    storeValue(127.0hf);
    storeValue(65504.0hf);
    // CHECK-NEXT: -65504
    // CHECK-NEXT: -128
    // CHECK-NEXT: -67
    // CHECK-NEXT: 0
    // CHECK-NEXT: 42
    // CHECK-NEXT: 85
    // CHECK-NEXT: 127
    // CHECK-NEXT: 65504

    // conversions from float
    storeValue(-2147483648.0f); // minimum int32_t number is representable by float
    storeValue(-32768.0f);
    storeValue(-67.9f);
    storeValue(0.001f);
    storeValue(42.9f);
    storeValue(85.0f);
    storeValue(32767.0f);
    storeValue(2147483520.0f); // maximum int32_t number representable by float
    // CHECK-NEXT: -2147483648
    // CHECK-NEXT: -32768
    // CHECK-NEXT: -67
    // CHECK-NEXT: 0
    // CHECK-NEXT: 42
    // CHECK-NEXT: 85
    // CHECK-NEXT: 32767
    // CHECK-NEXT: 2147483520

    // conversions from double
    storeValue(-2147483648.9LF);
    storeValue(-32768.0LF);
    storeValue(-67.9LF);
    storeValue(0.001LF);
    storeValue(42.9LF);
    storeValue(85.0LF);
    storeValue(32767.0LF);
    storeValue(2147483647.999LF);
    // CHECK-NEXT: -2147483648
    // CHECK-NEXT: -32768
    // CHECK-NEXT: -67
    // CHECK-NEXT: 0
    // CHECK-NEXT: 42
    // CHECK-NEXT: 85
    // CHECK-NEXT: 32767
    // CHECK-NEXT: 2147483647
}
