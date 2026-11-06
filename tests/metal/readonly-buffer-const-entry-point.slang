// `uniform` entry-point parameters of read-only buffer types are emitted as `const device`
// pointers on Metal, like global ones (#14154). Writable ones stay pointers to mutable memory.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

// METALLIB: @computeMain

// CHECK-DAG: {{\[\[}}kernel{{\]\]}} void computeMain
// CHECK-DAG: const float device* entryPointParams_extra_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: const uint32_t device* entryPointParams_extraRaw_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: {{[(,] }}float device* entryPointParams_result_{{[0-9]+}} {{\[\[}}buffer(

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(
    uint tid: SV_GroupIndex,
    uniform StructuredBuffer<float> extra,
    uniform ByteAddressBuffer extraRaw,
    uniform RWStructuredBuffer<float> result)
{
    result[tid] = extra[tid] + float(extraRaw.Load(tid * 4));
}
