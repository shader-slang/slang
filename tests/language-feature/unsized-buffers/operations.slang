//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-ir -DNO_ATOMICS=1
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -DNO_ATOMICS=1
//TEST:SIMPLE(filecheck=WGSL-SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -DNO_ATOMICS=1
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -wgpu -compute -output-using-type -xslang -DNO_ATOMICS=1
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=ATOMIC-RESULT): -metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=ATOMIC-RESULT): -vk -compute -output-using-type

// Unsized buffers retain native resources where the target provides them. On CUDA and CPU,
// the four fields use prelude structs that each contain a data pointer without a count.
//TEST_INPUT:ubuffer(data=[10], stride=4):name=inputValues
__UnsizedStructuredBuffer<uint> inputValues;
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputValues
__UnsizedRWStructuredBuffer<uint> outputValues;
//TEST_INPUT:ubuffer(data=[0 100 200], stride=4):name=inputBytes
__UnsizedByteAddressBuffer inputBytes;
//TEST_INPUT:ubuffer(data=[7 0 0], stride=4):out,name=outputBytes
__UnsizedRWByteAddressBuffer outputBytes;

void addOne(inout uint value)
{
    value += 1;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;
    uint value = inputValues.Load(index) + inputValues[index];
    uint2 pair = inputBytes.Load<uint2>(index * 8 + 4);
    outputValues[index] = value + pair.x;
    addOne(outputValues[index]);
#ifndef NO_ATOMICS
    // These atomics are unavailable on CPU and WGSL even for the sized buffer types.
    InterlockedAdd(outputValues[index], pair.y);
#endif
    outputBytes.Store<uint2>(index * 8 + 4, uint2(outputValues[index], pair.y));
    uint oldValue = 0;
#ifndef NO_ATOMICS
    outputBytes.InterlockedAdd(index * 8, 1, oldValue);
#endif
    outputValues[index] += outputBytes.Load(index * 8) + oldValue;
}

// CUDA: struct GlobalParams
// CUDA: UnsizedStructuredBuffer<uint{{ *}}> inputValues
// CUDA: UnsizedRWStructuredBuffer<uint{{ *}}> outputValues
// CUDA: UnsizedByteAddressBuffer inputBytes
// CUDA: UnsizedRWByteAddressBuffer outputBytes
// CUDA: atomicAdd

// CPP: struct GlobalParams
// CPP: UnsizedStructuredBuffer<uint32_t{{ *}}> inputValues
// CPP: UnsizedRWStructuredBuffer<uint32_t{{ *}}> outputValues
// CPP: UnsizedByteAddressBuffer inputBytes
// CPP: UnsizedRWByteAddressBuffer outputBytes

// HLSL-DAG: StructuredBuffer<uint{{ *}}> inputValues
// HLSL-DAG: RWStructuredBuffer<uint{{ *}}> outputValues
// HLSL-DAG: ByteAddressBuffer inputBytes
// HLSL-DAG: RWByteAddressBuffer outputBytes
// HLSL: InterlockedAdd

// SPIRV: OpTypeRuntimeArray
// SPIRV: OpAtomicIAdd

// GLSL: buffer
// GLSL: atomicAdd

// WGSL-DAG: @binding(0) @group(0) var<storage, read> inputValues{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(1) @group(0) var<storage, read_write> outputValues{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(2) @group(0) var<storage, read> inputBytes{{[_0-9]*}} : array<u32>;
// WGSL-DAG: @binding(3) @group(0) var<storage, read_write> outputBytes{{[_0-9]*}} : array<u32>;
// WGSL: fn computeMain
// WGSL: inputBytes{{[_0-9]*}}[({{.*}})/4]
// WGSL: outputValues{{[_0-9]*}}[{{.*}}] = inputValues{{[_0-9]*}}[
// WGSL: outputBytes{{[_0-9]*}}[({{.*}})/4] = outputValues{{[_0-9]*}}[
// WGSL-SPIRV: OpEntryPoint GLCompute

// METAL-DAG: const uint device* inputValues{{[_0-9]*}}
// METAL-DAG: uint device* outputValues{{[_0-9]*}}
// METAL-DAG: const uint32_t device* inputBytes{{[_0-9]*}}
// METAL-DAG: uint32_t device* outputBytes{{[_0-9]*}}
// METAL: void computeMain
// METAL: inputBytes{{[_0-9]*}}[({{.*}})>>2]
// METAL: atomic_fetch_add_explicit
// METAL: outputBytes{{[_0-9]*}}[({{.*}})>>2] =
// METALLIB: define void @computeMain
// RESULT: 128
// RESULT: 7
// RESULT: 121
// RESULT: 200
// ATOMIC-RESULT: 336
