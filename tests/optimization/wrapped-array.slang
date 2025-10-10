//TEST:SIMPLE(filecheck=CHECK):-target spirv

// CHECK: OpEntryPoint

// Make sure we never load the entire TensorList struct to local registers,
// instead, we should specialize the fetch function to directly load the
// element tensor from gTensors.

// CHECK-NOT: OpLoad %TensorList_std140

struct RWTensor<T, let D : int>
{
    int dims[D];
    RWStructuredBuffer<T> buffer;
    T getv(vector<uint, D> index)
    {
        int flat_index = 0;
        int stride = 1;
        for (int i = D - 1; i >= 0; --i)
        {
            flat_index += index[i] * stride;
            stride *= dims[i];
        }
        return buffer[flat_index];
    }
}
struct TensorList<let N : int>
{
    RWTensor<float, 2> tensors[N];

    float fetch(int tensor_index, uint2 index)
    {
        return tensors[tensor_index].getv(index);
    }
}

float sum_indirect<let N : int>(uint2 tid, TensorList<N> tensor_list, uint tensor_indices[N])
{
    float result = 0.0;
    for (int i = 0; i < N; i++)
    {
        result += tensor_list.fetch(tensor_indices[i], tid);
    }
    return result;
}

uniform TensorList<32> gTensors;
uniform uint gTensorIndices[32];

uniform float* result;

[numthreads(1,1,1)]
void computeMain(uint2 tid : SV_DispatchThreadID)
{
    *result = sum_indirect(tid, gTensors, gTensorIndices);
}
