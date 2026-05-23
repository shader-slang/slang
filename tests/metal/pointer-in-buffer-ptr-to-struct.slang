//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Pointer-to-struct where the pointed-to struct contains multi-level pointers.
// Metal's rejection is transitive through pointer fields: if Outer contains
// `Inner*` and Inner contains `int**`, Outer cannot be a buffer pointee type.
// The pass must follow pointer-to-struct fields during recursion.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Inner
{
    int** deep;
    int value;
};

struct Outer
{
    Inner* nested;
    int tag;
};

ConstantBuffer<Outer> cb;
RWStructuredBuffer<int> output;

// The inner struct's multi-level pointer should be lowered to ulong.
// CHECK-LABEL: struct Inner
// CHECK: ulong
// CHECK: };

// CHECK-NOT: device{{.*}}device

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Inner* inner = cb.nested;
    int** pp = inner->deep;
    output[tid.x] = **pp + inner->value + cb.tag;
}
