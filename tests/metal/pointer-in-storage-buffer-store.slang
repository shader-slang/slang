//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// RWStructuredBuffer<int*> with element store. Writing a pointer value into
// the buffer must cast the pointer to ulong before the store, since the
// buffer element type is lowered to ulong on Metal.
//
// See: https://github.com/shader-slang/slang/issues/10841

RWStructuredBuffer<int*> ptrs;
int* source;

// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    ptrs[tid.x] = source + tid.x;
}
