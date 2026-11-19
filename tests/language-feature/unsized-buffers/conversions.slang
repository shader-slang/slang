//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=COMPILE): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=COMPILE): -target metal -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -llvm -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RESULT): -vk -compute -output-using-type

// Public names remain aliases for the sized types. Conversion discards the count only on
// pointer targets and preserves the same resource on native buffer targets.
//TEST_INPUT:ubuffer(data=[10 20], stride=4):name=inputValues
StructuredBuffer<uint> inputValues;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputValues
RWStructuredBuffer<uint> outputValues;
//TEST_INPUT:ubuffer(data=[100 200], stride=4):name=inputBytes
ByteAddressBuffer inputBytes;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=outputBytes
RWByteAddressBuffer outputBytes;

[noinline]
void update(
    __UnsizedStructuredBuffer<uint> readValues,
    __UnsizedRWStructuredBuffer<uint> writeValues,
    __UnsizedByteAddressBuffer readBytes,
    __UnsizedRWByteAddressBuffer writeBytes,
    uint index)
{
    uint value = readValues[index] + readBytes.Load(index * 4) + 1;
    writeValues[index] = value;
    writeBytes.Store(index * 4, value);
    uint original = writeBytes.Load(index * 4);
    writeBytes.Store(index * 4, original + 3);
    writeValues[index] = writeBytes.Load(index * 4) + original;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    __SizedStructuredBuffer<uint> sizedRead = inputValues;
    __SizedRWStructuredBuffer<uint> sizedWrite = outputValues;
    __SizedByteAddressBuffer sizedReadBytes = inputBytes;
    __SizedRWByteAddressBuffer sizedWriteBytes = outputBytes;

    // Implicit conversions at the call boundary cover all four buffer types.
    update(sizedRead, sizedWrite, sizedReadBytes, sizedWriteBytes, 0);
    update(
        __UnsizedStructuredBuffer<uint>(sizedRead),
        __UnsizedRWStructuredBuffer<uint>(sizedWrite),
        __UnsizedByteAddressBuffer(sizedReadBytes),
        __UnsizedRWByteAddressBuffer(sizedWriteBytes),
        1);
    outputValues[2] = outputBytes.Load(0);
    outputValues[3] = outputBytes.Load(4);
}

// COMPILE: computeMain
// CUDA: void update{{.*}}(UnsizedStructuredBuffer<uint{{ *}}>{{.*}}UnsizedRWStructuredBuffer<uint{{ *}}>{{.*}}UnsizedByteAddressBuffer{{.*}}UnsizedRWByteAddressBuffer
// CUDA: computeMain
// CPP: void update{{.*}}(UnsizedStructuredBuffer<uint32_t{{ *}}>{{.*}}UnsizedRWStructuredBuffer<uint32_t{{ *}}>{{.*}}UnsizedByteAddressBuffer{{.*}}UnsizedRWByteAddressBuffer
// CPP: computeMain
// SPIRV: OpEntryPoint GLCompute
// GLSL: void main()
// RESULT: 225
// RESULT: 445
// RESULT: 114
// RESULT: 224
