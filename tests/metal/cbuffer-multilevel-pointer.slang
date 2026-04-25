//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that multi-level pointers (int**) inside a ConstantBuffer are lowered
// to ulong to avoid `device T* device*` in Metal struct definitions bound via
// [[buffer(N)]]. Single-level pointers (int*) should stay as typed device pointers.

struct MyParams
{
    int* single;
    int** multi;
};

ConstantBuffer<MyParams> params;

RWStructuredBuffer<int> output;

// The struct should have single-level pointer intact, but multi-level lowered to ulong.
// CHECK: struct MyParams
// CHECK: int device* single
// CHECK: ulong multi
// CHECK: };

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = *params.multi;
    output[0] = *params.single + *p;
}
