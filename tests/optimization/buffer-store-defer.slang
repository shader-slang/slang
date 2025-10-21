//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry compute_main
//TEST:SIMPLE(filecheck=PTX): -target ptx -stage compute -entry compute_main
//TEST:SIMPLE(filecheck=SPV): -target spirv

struct Bottom
{
    float bigArray[1024];

    [mutating]
    void setVal(int index, float value) { bigArray[index] = value; }
}

struct Root
{
    Bottom top[2];
    [mutating]
    void setTopVal(int x, int y, float value)
    {
        top[x].setVal(y, value);
    }
}

RWStructuredBuffer<Root> sb;

// Check that we don't load the entire `Root` struct, modify it, and then write it back.
// Instead we should generate a single store instruction to write the single float value
// directly to the buffer.

// SPV: OpEntryPoint
// SPV: OpLabel
// SPV-NEXT: OpAccessChain
// SPV-NOT: OpCompositeInsert
// SPV-NOT: OpLoad
// SPV: OpStore
// SPV-NOT: OpLoad
// SPV-NOT: OpCompositeInsert
// SPV: OpStore
// SPV: OpReturn

// CUDA: __device__ void Bottom_setVal_0(int [[INDEX0:[A-Za-z0-9_]+]], int [[INDEX1:[A-Za-z0-9_]+]], int [[INDEX2:[A-Za-z0-9_]+]], float [[VAL:[A-Za-z0-9_]+]])
// CUDA:  (&(&(globalParams{{.*}}->sb{{.*}}){{\[}}[[INDEX0]]{{\]}})->top{{.*}}{{\[}}[[INDEX1]]{{\]}})->bigArray{{.*}}{{\[}}[[INDEX2]]{{\]}} = [[VAL]];
// PTX: compute_main

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    sb[0].setTopVal(1, 2, 100.0f);

    sb[3].top[1].setVal(8, 200.0f);
}
