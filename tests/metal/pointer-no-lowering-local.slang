//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that pointers used in non-buffer contexts (function-local, passed
// between functions) are not lowered to ulong. Only pointers stored as data
// inside buffer pointee types need lowering.
//
// See: https://github.com/shader-slang/slang/issues/10841

int readThrough(int* p)
{
    return *p;
}

void writeThrough(int* p, int val)
{
    *p = val;
}

RWStructuredBuffer<int> output;

// Function signatures must use typed device pointers, not ulong.
// CHECK: readThrough
// CHECK: int device*
// CHECK-NOT: ulong

// CHECK: writeThrough
// CHECK: int device*
// CHECK-NOT: ulong

// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform int* src, uniform int* dst)
{
    int val = readThrough(src);
    writeThrough(dst, val);
    output[0] = val;
}
