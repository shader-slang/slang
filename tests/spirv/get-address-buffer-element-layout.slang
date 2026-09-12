//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// For a non-default-layout struct-element buffer, cross-element arithmetic on the pointer
// from `__getAddress(buf[i])` must use the buffer's own element stride, not the default —
// i.e. it must match `&buf[i]`. Here the `Std140`-laid-out `Pair` has 16-byte stride, so the
// element pointer carries `ArrayStride 16` (shader-slang/slang#12581).

//CHECK-NOT: error:
//CHECK-DAG: OpDecorate %[[PAIR_PTR:_ptr_StorageBuffer_Pair[A-Za-z0-9_]*]] ArrayStride 16
//CHECK-DAG: %[[PAIR_PTR]] = OpTypePointer StorageBuffer

struct Pair
{
    float a;
    float b;
}

RWStructuredBuffer<Pair, Std140DataLayout> buf;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    Pair* p = __getAddress(buf[0]);
    output[0] = (p + 1).a;
}
