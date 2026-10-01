// Std430 remains outside both actual CUDA backends; no capability override is used.
//TEST:SIMPLE(filecheck=BAD): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -o nvvm-layout-unsupported.ptx
//TEST:SIMPLE(filecheck=BAD): -target cuda -entry computeMain -stage compute -capability cuda_sm_8_0 -o nvvm-layout-unsupported.cu
// BAD: result code = {{-?[1-9][0-9]*}}
// BAD: error[E36107]

struct Record
{
    float3 value;
    bool flag;
};

[CUDAKernel]
void computeMain(
    uniform LayoutPtr<Record, Std430DataLayout> input,
    uniform int index,
    uniform Ptr<uint64_t> output)
{
    output[0] = uint64_t(input + index);
}
