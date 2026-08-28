// Exercises the smallest useful raw CUDA program structure through the direct NVVM route.
// The helper, branch, and loop make this a file-backed composition test rather than an empty
// routing check.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

int chooseAndAccumulate(int left, int right)
{
    int selected = left < right ? left : right;
    int result = 0;
    for (int i = 0; i < selected; ++i)
        result += i;
    return result;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = chooseAndAccumulate(left, right);
}

// CHECK: .visible .entry computeMain
// CHECK: st.global.u32
