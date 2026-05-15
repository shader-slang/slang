// int64-uint64-type.slang
//
// Verifies that int64_t and uint64_t emit as i64 and u64 in WGSL,
// not as the HLSL-style names "int64_t" / "uint64_t".
// Also exercises the wgsl-spirv-asm path (Dawn SPIR-V compilation).

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -stage compute -entry computeMain

// --- WGSL text output ---
// WGSL-DAG: array<u64>
// WGSL-DAG: array<i64>
// WGSL: u64(42)
// WGSL: i64(-1)
// WGSL-NOT: uint64_t
// WGSL-NOT: int64_t

// --- SPIR-V assembly output (Dawn path) ---
// OpTypeInt width=64: unsigned (signedness 0) and signed (signedness 1)
// SPIRV-DAG: OpTypeInt 64 0
// SPIRV-DAG: OpTypeInt 64 1

RWStructuredBuffer<uint64_t> outputU64;
RWStructuredBuffer<int64_t> outputI64;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputU64[tid.x] = uint64_t(42);
    outputI64[tid.x] = int64_t(-1);
}
