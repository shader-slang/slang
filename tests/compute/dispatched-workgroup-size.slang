// DispatchedWorkgroupSize() reads the workgroup size the dispatch was launched with: an
// implicit [[dispatch_threads_per_threadgroup]] parameter on Metal and blockDim on CUDA. Both
// are the requested size, also in the smaller edge threadgroups of a non-uniform Metal dispatch.
// Targets whose workgroup size is fixed by [numthreads] fold it to the [numthreads] constant.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// An entry point that reads the size only through a helper function.
//TEST:SIMPLE(filecheck=METALHELPER): -target metal -stage compute -entry helperOnly
//TEST:SIMPLE(filecheck=CUDAHELPER): -target cuda -stage compute -entry helperOnly

// Run on the CPU, where the size is the [numthreads] size.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain({{.*}}uint3 [[SIZE:[A-Za-z0-9_]+]] {{\[\[}}dispatch_threads_per_threadgroup{{\]\]}}
// METAL: [[SIZE]]
// METALLIB: @computeMain

// CUDA: blockDim

// HLSL-NOT: threads_per_threadgroup
// HLSL: uint3(int3(4L, 2L, 1L))

// SPV: OpEntryPoint
// SPV-NOT: BuiltIn WorkgroupSize
// SPV-DAG: OpConstant %{{u?int}} 4
// SPV-DAG: OpConstant %{{u?int}} 2

// WGSL: vec3<i32>(i32(4), i32(2), i32(1))

// METALHELPER: {{\[\[}}kernel{{\]\]}} void helperOnly({{.*}}uint3 [[HSIZE:[A-Za-z0-9_]+]] {{\[\[}}dispatch_threads_per_threadgroup{{\]\]}}
// METALHELPER: [[HSIZE]]
// CUDAHELPER: blockDim
// CUDAHELPER: void helperOnly(

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

// Reads the size only through `flatIndex`.
[numthreads(4, 2, 1)]
void helperOnly(uint3 gtid: SV_GroupThreadID)
{
    outputBuffer[flatIndex(gtid)] = 1;
}
