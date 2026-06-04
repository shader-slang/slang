//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Store a multi-level pointer into a struct field in an RWStructuredBuffer.
// The stored pointer value must be cast to ulong before writing.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Params
{
    int** ptr;
    int value;
};

RWStructuredBuffer<Params> buf;
RWStructuredBuffer<int*> sources;

// The struct should have the multi-level pointer field lowered to ulong.
// CHECK: ulong
// CHECK-NOT: device{{.*}}device

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = buf[0].ptr;
    int* p = *pp;
    buf[tid.x].ptr = pp;
    buf[tid.x].value = *p;
}
