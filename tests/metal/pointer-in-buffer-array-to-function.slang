//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Passing a whole array-of-pointer field to a non-inlined function.
// The pass must reconstruct the original-typed array (via element-wise
// CastIntToPtr) for consumers that expect the un-lowered type.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct S
{
    int** arr[2];
    int tag;
};

S makeS(int** a, int** b, int t)
{
    S s;
    s.arr[0] = a;
    s.arr[1] = b;
    s.tag = t;
    return s;
}

int derefFirst(int** arr[2])
{
    return **arr[0];
}

ConstantBuffer<S> cb;
RWStructuredBuffer<int> output;

// CHECK-LABEL: struct S
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    S s = makeS(cb.arr[0], cb.arr[1], cb.tag);
    output[tid.x] = derefFirst(s.arr);
}
