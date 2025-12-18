//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry compute_main -stage compute

//TEST:SIMPLE(filecheck=SPV): -target spirv

// Check that we can specialize buffer loads through bindless handles, and
// do not load big struct elements into registers unnecessarily.

struct Bottom
{
    float bigArray[1024];
    float bottomGetValue(int index) { return bigArray[index]; }
}

struct Middle
{
    Bottom bottom;
    float middleGetValue(int index) { return bottom.bottomGetValue(index); }
}

struct Top
{
    StructuredBuffer<Middle>.Handle middle;

    // Calling `middleGetValue` on `middle[0]` should not causing the entire `Middle`
    // struct to be loaded into registers. Instead, we should be able to specialize
    // `middleGetValue` to take a `StructuredBuffer<Middle>.Handle` and an `int`
    // index, and recursively specialize `bottomGetValue` to only load the `Bottom.bigArray[index]` element.
    float topGetValue(int index) { return middle[0].middleGetValue(index); }
}

struct Root
{
    Top top;
}

ConstantBuffer<Root> cb;

RWStructuredBuffer<float> outputBuffer;

// SPV: OpEntryPoint
// SPV-NOT: OpLoad %Middle
// SPV: %[[REG:[A-Za-z0-9_]+]] = OpLoad %float
// SPV: OpStore {{.*}} %[[REG]]

// Check that the generated CUDA code contains a specialized `bottomGetValue` function that has
// the complete parameter list to access the `bigArray` element directly, without needing to load
// the entire `Bottom` struct from the caller.
//
// CUDA-DAG: __device__ float Bottom_bottomGetValue{{.*}}(StructuredBuffer<Middle{{.*}}> {{.*}}, int {{.*}}, int {{.*}})
// PTX: compute_main

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = cb.top.topGetValue(0);
}
