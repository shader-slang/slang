// DispatchedWorkgroupSize() reads the workgroup size the dispatch was launched with: an
// implicit [[threads_per_threadgroup]] parameter on Metal and blockDim on CUDA. Targets
// whose workgroup size is fixed by [numthreads] fold it to the [numthreads] constant.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// Run on the CPU, where the size is the [numthreads] size.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain({{.*}}uint3 [[SIZE:[A-Za-z0-9_]+]] {{\[\[}}threads_per_threadgroup{{\]\]}}
// METAL: [[SIZE]]
// METALLIB: @computeMain

// CUDA: blockDim

// HLSL-NOT: threads_per_threadgroup
// HLSL: uint3(int3(int(4), int(2), int(1)))

// SPV: OpEntryPoint
// SPV-NOT: BuiltIn WorkgroupSize

// WGSL: vec3<i32>(i32(4), i32(2), i32(1))

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The size is read in a function the entry point calls, not in the entry point itself.
uint flatIndex(uint3 groupThreadID)
{
    uint3 size = DispatchedWorkgroupSize();
    return groupThreadID.y * size.x + groupThreadID.x;
}

// BUF-COUNT-8: 421
[numthreads(4, 2, 1)]
void computeMain(uint3 gtid: SV_GroupThreadID)
{
    uint3 size = DispatchedWorkgroupSize();
    outputBuffer[flatIndex(gtid)] = size.x * 100 + size.y * 10 + size.z;
}
