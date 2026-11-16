// `uniform` entry-point parameters of read-only buffer types are emitted as `const device`
// pointers on Metal, like global ones (#14154). Writable ones stay pointers to mutable memory.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -mtl -shaderobj -output-using-type

// METALLIB: @computeMain

// CHECK-DAG: {{\[\[}}kernel{{\]\]}} void computeMain
// CHECK-DAG: const float device* entryPointParams_extra_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: const uint32_t device* entryPointParams_extraRaw_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-DAG: {{[(,] }}float device* entryPointParams_result_{{[0-9]+}} {{\[\[}}buffer(

//TEST_INPUT:ubuffer(data=[0.5 0.25], stride=4):name=extra
//TEST_INPUT:ubuffer(data=[9 10]):name=extraRaw
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=result

// BUFFER: 9.5
// BUFFER-NEXT: 10.25

[numthreads(2, 1, 1)]
[shader("compute")]
void computeMain(
    uint tid: SV_GroupIndex,
    uniform StructuredBuffer<float> extra,
    uniform ByteAddressBuffer extraRaw,
    uniform RWStructuredBuffer<float> result)
{
    result[tid] = extra[tid] + float(extraRaw.Load(tid * 4));
}
