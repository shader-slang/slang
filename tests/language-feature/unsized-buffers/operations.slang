//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-ir -DNO_ATOMICS=1
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -DNO_ATOMICS=1
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute

// Unsized buffers retain native resources where the target provides them. On CUDA and CPU,
// the four fields must each contain only a pointer, without a count or a wrapper object.
__UnsizedStructuredBuffer<uint> inputValues;
__UnsizedRWStructuredBuffer<uint> outputValues;
__UnsizedByteAddressBuffer inputBytes;
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
// CUDA: uint{{.*}}* inputValues
// CUDA: uint{{.*}}* outputValues
// CUDA: uchar{{.*}}* inputBytes
// CUDA: uchar{{.*}}* outputBytes
// CUDA: atomicAdd

// CPP: struct GlobalParams
// CPP: uint32_t{{.*}}* inputValues
// CPP: uint32_t{{.*}}* outputValues
// CPP: uint8_t{{.*}}* inputBytes
// CPP: uint8_t{{.*}}* outputBytes

// HLSL-DAG: StructuredBuffer<uint{{ *}}> inputValues
// HLSL-DAG: RWStructuredBuffer<uint{{ *}}> outputValues
// HLSL-DAG: ByteAddressBuffer inputBytes
// HLSL-DAG: RWByteAddressBuffer outputBytes
// HLSL: InterlockedAdd

// SPIRV: OpTypeRuntimeArray
// SPIRV: OpAtomicIAdd

// GLSL: buffer
// GLSL: atomicAdd

// WGSL: var<storage
// WGSL: fn computeMain

// METAL: device
// METAL: atomic_fetch_add_explicit
