//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Verify that multi-level pointers (int**) inside a ConstantBuffer are lowered
// to ulong because Metal rejects pointer-to-pointer types in buffer pointee
// types. Single-level pointers (int*) should stay as typed device pointers.

struct MyParams
{
    int* single;
    int** multi;
};

ConstantBuffer<MyParams> params;

RWStructuredBuffer<int> output;

// The struct should have single-level pointer intact, but multi-level lowered to ulong.
// CHECK-LABEL: struct MyParams
// CHECK: int device* single
// CHECK: ulong multi
// CHECK: };

// The kernel body should cast ulong back to a typed pointer for `multi`.
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = *params.multi;
    output[0] = *params.single + *p;
}
