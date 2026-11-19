//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -llvm -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RESULT): -vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -wgpu -compute -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type

// Provisional __Sized names alias the existing public buffer types. Conversion discards the
// count only on pointer targets and preserves the same resource on native buffer targets.
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

// Native targets preserve the resources, access rights, and byte addressing through conversion.
// WGSL-DAG: @binding(0) @group(0) var<storage, read> inputValues{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(1) @group(0) var<storage, read_write> outputValues{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(2) @group(0) var<storage, read> inputBytes{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(3) @group(0) var<storage, read_write> outputBytes{{[_0-9]*}} : array<u32>;
// WGSL: inputValues{{[_0-9]*}}[{{.*}}] + inputBytes{{[_0-9]*}}[({{.*}})/4]
// WGSL: outputBytes{{[_0-9]*}}[({{.*}})/4] =
// WGSL: outputValues{{[_0-9]*}}[{{.*}}] = outputBytes{{[_0-9]*}}[({{.*}})/4]
// WGSL: fn computeMain()
// METAL: void update{{[_0-9]*}}(const uint device* readValues{{[_0-9]*}}, uint device* writeValues{{[_0-9]*}}, const uint32_t device* readBytes{{[_0-9]*}}, uint32_t device* writeBytes{{[_0-9]*}}, uint index{{[_0-9]*}})
// METAL: readValues{{[_0-9]*}}[{{.*}}] + as_type<uint>(readBytes{{[_0-9]*}}[({{.*}})>>2])
// METAL: writeBytes{{[_0-9]*}}[({{.*}})>>2] =
// METAL: *(writeValues{{[_0-9]*}}+{{.*}}) = as_type<uint>(writeBytes{{[_0-9]*}}[({{.*}})>>2])
// METAL: void computeMain(const uint device* inputValues{{[_0-9]*}} {{\[\[}}buffer(0){{\]\]}}, uint device* outputValues{{[_0-9]*}} {{\[\[}}buffer(1){{\]\]}}, const uint32_t device* inputBytes{{[_0-9]*}} {{\[\[}}buffer(2){{\]\]}}, uint32_t device* outputBytes{{[_0-9]*}} {{\[\[}}buffer(3){{\]\]}})
// METALLIB: define void @computeMain

// RESULT: 225
// RESULT: 445
// RESULT: 114
// RESULT: 224
