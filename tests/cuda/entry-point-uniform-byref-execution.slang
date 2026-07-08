// Execute the by-reference descriptor-table ABI end-to-end on a real CUDA device:
// the host (slang-rhi) must allocate the payload buffer from the pooled global
// upload, write the four buffer descriptors into it, patch the device pointer at
// the parameter's kernel-argument offset, and the kernel must read correct values
// through a runtime index. The second run compiles the same shader with
// -cuda-entry-point-params-by-value, executing the legacy by-value ABI (whose
// runtime indexing goes through the dynamic-index local-copy legalization
// instead) - the two ABIs must produce identical results.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj -xslang -cuda-entry-point-params-by-value

struct TensorList
{
    RWStructuredBuffer<float> tensors[4];
}

//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0 40.0], stride=4):name=list.tensors[0]
//TEST_INPUT:ubuffer(data=[11.0 21.0 31.0 41.0], stride=4):name=list.tensors[1]
//TEST_INPUT:ubuffer(data=[12.0 22.0 32.0 42.0], stride=4):name=list.tensors[2]
//TEST_INPUT:ubuffer(data=[13.0 23.0 33.0 43.0], stride=4):name=list.tensors[3]
//TEST_INPUT:ubuffer(data=[3 1 2 0], stride=4):name=indices
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output

// output[i] = tensors[indices[i]][i]:
// CHECK: 13.0
// CHECK: 21.0
// CHECK: 32.0
// CHECK: 40.0

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform TensorList list,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    uint i = tid.x;
    output[i] = list.tensors[indices[i]][i];
}
