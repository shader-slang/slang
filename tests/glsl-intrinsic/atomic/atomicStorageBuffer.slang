//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_SPIRV

// currently tests do not handle atomic_uint well
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly 
#version 430


//#define TEST_when_shader_atomic_float2_is_available

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[1];
} outputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=int32Buffer
buffer MyBlockName1
{
    int data[1];
} int32Buffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=8):out,name=int64Buffer
buffer MyBlockName2
{
    uint64_t data[1];
} int64Buffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=uint32Buffer
buffer MyBlockName3
{
    uint data[1];
} uint32Buffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=8):out,name=uint64Buffer
buffer MyBlockName4
{
    uint64_t data[1];
} uint64Buffer;

//TEST_INPUT:ubuffer(data=[0.0f], stride=2):out,name=float16Buffer
buffer MyBlockName5
{
    half data[1];
} float16Buffer;

//TEST_INPUT:ubuffer(data=[0.0f], stride=2):out,name=float32Buffer
buffer MyBlockName6
{
    float data[1];
} float32Buffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=8):out,name=float64Buffer
buffer MyBlockName7
{
    double data[1];
} float64Buffer;

bool testAtomicInt32()
{
    return true
        && (int32Buffer.data[0] = 5) == 5
        && atomicAdd(int32Buffer.data[0], 1) == 5
        && int32Buffer.data[0] == 6

        && (int32Buffer.data[0] = 5) == 5
        && atomicMin(int32Buffer.data[0], 1) == 5
        && int32Buffer.data[0] == 1

        && (int32Buffer.data[0] = 5) == 5
        && atomicMax(int32Buffer.data[0], 1) == 5
        && int32Buffer.data[0] == 5

        && (int32Buffer.data[0] = 5) == 5
        && atomicExchange(int32Buffer.data[0], 2) == 5
        && int32Buffer.data[0] == 2

        && (int32Buffer.data[0] = 5) == 5
        && atomicAnd(int32Buffer.data[0], 1) == 5
        && int32Buffer.data[0] == 1

        && (int32Buffer.data[0] = 5) == 5
        && atomicOr(int32Buffer.data[0], 2) == 5
        && int32Buffer.data[0] == 7

        && (int32Buffer.data[0] = 5) == 5
        && atomicXor(int32Buffer.data[0], 3) == 5
        && int32Buffer.data[0] == 6

        && (int32Buffer.data[0] = 5) == 5
        && atomicCompSwap(int32Buffer.data[0], 5, 2) == 5
        && int32Buffer.data[0] == 2

        && (int32Buffer.data[0] = 5) == 5
        && atomicCompSwap(int32Buffer.data[0], 4, 2) == 5
        && int32Buffer.data[0] == 5
        ;
}
bool testAtomicInt64()
{
    return true
        && (int64Buffer.data[0] = 5) == 5
        && atomicAdd(int64Buffer.data[0], 1) == 5
        && int64Buffer.data[0] == 6

        && (int64Buffer.data[0] = 5) == 5
        && atomicMin(int64Buffer.data[0], 1) == 5
        && int64Buffer.data[0] == 1

        && (int64Buffer.data[0] = 5) == 5
        && atomicMax(int64Buffer.data[0], 1) == 5
        && int64Buffer.data[0] == 5

        && (int64Buffer.data[0] = 5) == 5
        && atomicExchange(int64Buffer.data[0], 2) == 5
        && int64Buffer.data[0] == 2

        && (int64Buffer.data[0] = 5) == 5
        && atomicAnd(int64Buffer.data[0], 1) == 5
        && int64Buffer.data[0] == 1

        && (int64Buffer.data[0] = 5) == 5
        && atomicOr(int64Buffer.data[0], 2) == 5
        && int64Buffer.data[0] == 7

        && (int64Buffer.data[0] = 5) == 5
        && atomicXor(int64Buffer.data[0], 3) == 5
        && int64Buffer.data[0] == 6

        && (int64Buffer.data[0] = 5) == 5
        && atomicCompSwap(int64Buffer.data[0], 5, 2) == 5
        && int64Buffer.data[0] == 2

        && (int64Buffer.data[0] = 5) == 5
        && atomicCompSwap(int64Buffer.data[0], 4, 2) == 5
        && int64Buffer.data[0] == 5
        ;
}
bool testAtomicUint32()
{
    return true
        && (uint32Buffer.data[0] = 5) == 5
        && atomicAdd(uint32Buffer.data[0], 1) == 5
        && uint32Buffer.data[0] == 6

        && (uint32Buffer.data[0] = 5) == 5
        && atomicMin(uint32Buffer.data[0], 1) == 5
        && uint32Buffer.data[0] == 1

        && (uint32Buffer.data[0] = 5) == 5
        && atomicMax(uint32Buffer.data[0], 1) == 5
        && uint32Buffer.data[0] == 5

        && (uint32Buffer.data[0] = 5) == 5
        && atomicExchange(uint32Buffer.data[0], 2) == 5
        && uint32Buffer.data[0] == 2

        && (uint32Buffer.data[0] = 5) == 5
        && atomicAnd(uint32Buffer.data[0], 1) == 5
        && uint32Buffer.data[0] == 1

        && (uint32Buffer.data[0] = 5) == 5
        && atomicOr(uint32Buffer.data[0], 2) == 5
        && uint32Buffer.data[0] == 7

        && (uint32Buffer.data[0] = 5) == 5
        && atomicXor(uint32Buffer.data[0], 3) == 5
        && uint32Buffer.data[0] == 6

        && (uint32Buffer.data[0] = 5) == 5
        && atomicCompSwap(uint32Buffer.data[0], 5, 2) == 5
        && uint32Buffer.data[0] == 2

        && (uint32Buffer.data[0] = 5) == 5
        && atomicCompSwap(uint32Buffer.data[0], 4, 2) == 5
        && uint32Buffer.data[0] == 5
        ;
}
bool testAtomicUint64()
{
    return true
        && (uint64Buffer.data[0] = 5) == 5
        && atomicAdd(uint64Buffer.data[0], 1) == 5
        && uint64Buffer.data[0] == 6

        && (uint64Buffer.data[0] = 5) == 5
        && atomicMin(uint64Buffer.data[0], 1) == 5
        && uint64Buffer.data[0] == 1

        && (uint64Buffer.data[0] = 5) == 5
        && atomicMax(uint64Buffer.data[0], 1) == 5
        && uint64Buffer.data[0] == 5

        && (uint64Buffer.data[0] = 5) == 5
        && atomicExchange(uint64Buffer.data[0], 2) == 5
        && uint64Buffer.data[0] == 2

        && (uint64Buffer.data[0] = 5) == 5
        && atomicAnd(uint64Buffer.data[0], 1) == 5
        && uint64Buffer.data[0] == 1

        && (uint64Buffer.data[0] = 5) == 5
        && atomicOr(uint64Buffer.data[0], 2) == 5
        && uint64Buffer.data[0] == 7

        && (uint64Buffer.data[0] = 5) == 5
        && atomicXor(uint64Buffer.data[0], 3) == 5
        && uint64Buffer.data[0] == 6

        && (uint64Buffer.data[0] = 5) == 5
        && atomicCompSwap(uint64Buffer.data[0], 5, 2) == 5
        && uint64Buffer.data[0] == 2

        && (uint64Buffer.data[0] = 5) == 5
        && atomicCompSwap(uint64Buffer.data[0], 4, 2) == 5
        && uint64Buffer.data[0] == 5
        ;
}

bool testAtomicFloat16()
{
    return true
#ifdef TEST_when_shader_atomic_float2_is_available
        && (float16Buffer.data[0] = half(5)) == half(5)
        && atomicAdd(float16Buffer.data[0], half(1)) == half(5)
        && float16Buffer.data[0] == half(6)

        && (float16Buffer.data[0] = half(5)) == half(5)
        && atomicMin(float16Buffer.data[0], half(1)) == half(5)
        && float16Buffer.data[0] == half(1)

        && (float16Buffer.data[0] = half(5)) == half(5)
        && atomicMax(float16Buffer.data[0], half(1)) == half(5)
        && float16Buffer.data[0] == half(5)

        && (float16Buffer.data[0] = half(5)) == half(5)
        && atomicExchange(float16Buffer.data[0], half(2)) == half(5)
        && float16Buffer.data[0] == half(2)
#endif // TEST_when_shader_atomic_float2_is_available
        ;
}

bool testAtomicFloat32()
{
    return true

        && (float32Buffer.data[0] = float(5)) == float(5)
        && atomicAdd(float32Buffer.data[0], float(1)) == float(5)
        && float32Buffer.data[0] == float(6)

#ifdef TEST_when_shader_atomic_float2_is_available
        && (float32Buffer.data[0] = float(5)) == float(5)
        && atomicMin(float32Buffer.data[0], float(1)) == float(5)
        && float32Buffer.data[0] == float(1)

        && (float32Buffer.data[0] = float(5)) == float(5)
        && atomicMax(float32Buffer.data[0], float(1)) == float(5)
        && float32Buffer.data[0] == float(5)

        && (float32Buffer.data[0] = float(5)) == float(5)
        && atomicExchange(float32Buffer.data[0], float(2)) == float(5)
        && float32Buffer.data[0] == float(2)
#endif // TEST_when_shader_atomic_float2_is_available
        ;
}

bool testAtomicFloat64()
{
    return true

        && (float64Buffer.data[0] = double(5)) == double(5)
        && atomicAdd(float64Buffer.data[0], double(1)) == double(5)
        && float64Buffer.data[0] == double(6)

#ifdef TEST_when_shader_atomic_float2_is_available
        && (float64Buffer.data[0] = double(5)) == double(5)
        && atomicMin(float64Buffer.data[0], double(1)) == double(5)
        && float64Buffer.data[0] == double(1)

        && (float64Buffer.data[0] = double(5)) == double(5)
        && atomicMax(float64Buffer.data[0], double(1)) == double(5)
        && float64Buffer.data[0] == double(5)

        && (float64Buffer.data[0] = double(5)) == double(5)
        && atomicExchange(float64Buffer.data[0], double(2)) == double(5)
        && float64Buffer.data[0] == double(2)
#endif // TEST_when_shader_atomic_float2_is_available
        ;
}

layout(local_size_x = 1) in;
void computeMain()
{
    // testing has the following pattern in groups:
    // set, op, test
    // this is to remove dependency on other ops when testing
    outputBuffer.data[0] = true
            && testAtomicInt32()
            && testAtomicInt64()
            && testAtomicUint32()
            && testAtomicUint64()
            && testAtomicFloat16()
            && testAtomicFloat32()
            && testAtomicFloat64()
            ;    
    // to ensure these buffers don't mess-up result
    int32Buffer.data[0] = 2;
    int64Buffer.data[0] = 2;
    uint32Buffer.data[0] = 2;
    uint64Buffer.data[0] = 2;
    float16Buffer.data[0] = half(2.0f);
    float32Buffer.data[0] = float(2.0f);
    float64Buffer.data[0] = double(2.0);
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // BUF: 1
}
