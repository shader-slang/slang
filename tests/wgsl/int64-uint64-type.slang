// int64-uint64-type.slang
//
// Verifies that int64_t and uint64_t emit as i64 and u64 in WGSL,
// not as the HLSL-style names "int64_t" / "uint64_t".

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

// Negative assertions: HLSL-style names must never appear in WGSL output.
// Placed before all positive checks so they scan the entire output.
//CHECK-NOT: uint64_t
//CHECK-NOT: int64_t

RWStructuredBuffer<uint64_t> outputU64;
RWStructuredBuffer<int64_t> outputI64;

// CHECK-DAG: array<u64>
// CHECK-DAG: array<i64>

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: u64(42)
    outputU64[tid.x] = uint64_t(42);

    // CHECK: i64(-1)
    outputI64[tid.x] = int64_t(-1);
}
