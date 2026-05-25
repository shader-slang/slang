//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Verify pointer lowering at different depth levels inside ConstantBuffers:
// - Single-level pointers (int*) must be preserved as typed device pointers
// - Multi-level pointers (int**, int***) must be lowered to ulong
//
// See: https://github.com/shader-slang/slang/issues/10841

struct SinglePtr
{
    int* data;
    float* weights;
};

struct DoublePtr
{
    int* single;
    int** multi;
};

struct TriplePtr
{
    int*** deep;
};

ConstantBuffer<SinglePtr> cb_single;
ConstantBuffer<DoublePtr> cb_double;
ConstantBuffer<TriplePtr> cb_triple;

RWStructuredBuffer<int> output;

// Double: single-level preserved, multi-level lowered.
// CHECK-LABEL: struct DoublePtr
// CHECK: int device* single
// CHECK: ulong multi
// CHECK: };

// Triple: lowered to ulong.
// CHECK-LABEL: struct TriplePtr
// CHECK: ulong deep
// CHECK: };

// Single-level pointers preserved as typed device pointers.
// CHECK-LABEL: struct SinglePtr
// CHECK: int device* data
// CHECK: float device* weights
// CHECK-NOT: ulong
// CHECK: };

// Kernel body reconstructs typed pointers from ulong.
// CHECK-LABEL: computeMain
// CHECK: int device* device*
// CHECK: int device* device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = *cb_double.multi;
    int** pp = *cb_triple.deep;
    output[0] = *cb_single.data + *p + **pp;
}
