//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that triple-level pointers (int***) inside a ConstantBuffer are
// also lowered to ulong.

struct MyParams
{
    int*** deep;
};

ConstantBuffer<MyParams> params;

RWStructuredBuffer<int> output;

// The struct should have the triple pointer lowered to ulong.
// CHECK-LABEL: struct MyParams
// CHECK: ulong deep
// CHECK: };

// The kernel body should cast ulong back to a typed pointer for `deep`.
// CHECK-LABEL: computeMain
// CHECK: int device* device*

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int** pp = *params.deep;
    int* p = *pp;
    output[0] = *p;
}
