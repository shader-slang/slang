//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that RWStructuredBuffer<uint64_t> and RWStructuredBuffer<int**>
// coexist correctly when lowering produces the same storage type (ulong).
// The two buffers must remain distinct parameters with correct access
// patterns: addrs is read as raw ulong, ptrs is cast to a pointer type.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Params
{
    RWStructuredBuffer<uint64_t> addrs;
    RWStructuredBuffer<int**> ptrs;
};

ParameterBlock<Params> params;

RWStructuredBuffer<int> output;

// Both resources lower to ulong device* but must remain distinct fields.
// CHECK-LABEL: struct Params
// CHECK: ulong device* addrs
// CHECK: ulong device* ptrs
// CHECK: };

// The ptrs access must cast through a pointer type; addrs must not.
// CHECK-LABEL: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint64_t addr = params.addrs[tid.x];
    int** pp = params.ptrs[tid.x];
    int* p = *pp;
    output[tid.x] = *p + int(addr);
}
