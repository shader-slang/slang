//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Negative locks for the CUDA by-reference conversion: parameters that do NOT carry
// a descriptor table keep the by-value kernel-argument fast path, and a
// source-written ParameterBlock is not double-wrapped.
//
// - `single` is a pointer-backed struct but holds no fixed-size descriptor array;
// - `opts` contains only plain data (including a plain-data array);
// - `block` is already an explicit parameter group.

struct MyTensor
{
    float* data;
    uint stride;
}

struct Options
{
    uint counts[8];
    float scale;
}

struct BlockArgs
{
    RWStructuredBuffer<float> buffers[4];
}

// CHECK: __global__ void computeMain(MyTensor_0 single_0, Options_0 opts_0, BlockArgs_0* block_0, RWStructuredBuffer<float> output_0)

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform MyTensor single,
    uniform Options opts,
    uniform ParameterBlock<BlockArgs> block,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = *(single.data + tid.x) * opts.scale + opts.counts[tid.x % 8] +
                    block.buffers[tid.x % 4][tid.x];
}
