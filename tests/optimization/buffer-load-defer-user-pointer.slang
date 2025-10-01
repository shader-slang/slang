//TEST:SIMPLE(filecheck=SPV): -target spirv -O0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry compute_main -stage compute

// Check that we can specialize buffer loads through user pointers, and
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
    StructuredBuffer<Middle*>.Handle middle;

    // Calling `middleGetValue` on `middle[0]` should not causing the entire `Middle`
    // struct to be loaded into registers. Instead, we should be able to specialize
    // `middleGetValue` to take a `Middle*` and recursively specialize `bottomGetValue`
    // to only load the `Bottom.bigArray[index]` element.
    float topGetValue(int index) { return middle[0].middleGetValue(index); }
}

struct Root
{
    Top top;
}

ConstantBuffer<Root> cb;

RWStructuredBuffer<float> outputBuffer;

// Check that the generated CUDA code never loads a `Middle` or `Bottom` struct into a local var.
// CUDA-NOT: Middle{{[_A-Za-z0-9]*}} {{[a-zA-Z0-9_]+}} =
// CUDA-NOT: Bottom{{[_A-Za-z0-9]*}} {{[a-zA-Z0-9_]+}} =
// CUDA-NOT: Top{{[_A-Za-z0-9]*}} {{[a-zA-Z0-9_]+}} =

// Check that the generated CUDA code can be compiled by nvrtc correctly into PTX.
// PTX: compute_main

// Check that the generated (unoptimized) SPIR-V contains a specialized Bottom_bottomGetValue function
// that takes in a Bottom* and use access chain to load the required array element directly, without 
// needing to load the entire Bottom struct.
// SPV: %Bottom_bottomGetValue = OpFunction %float None
// SPV: OpFunctionParameter %_ptr_PhysicalStorageBuffer_Middle_natural
// SPV: %[[INDEX:[A-Za-z0-9_]+]] = OpFunctionParameter %int
// SPV: %[[PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_PhysicalStorageBuffer_float %{{.*}} %[[INDEX]]
// SPV: %[[VALUE:[A-Za-z0-9_]+]] = OpLoad %float %[[PTR]]
// SPV: OpReturnValue %[[VALUE]]

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = cb.top.topGetValue(0);
}
