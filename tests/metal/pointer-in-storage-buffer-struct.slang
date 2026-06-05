//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Struct with multi-level pointer field used exclusively as element type of
// RWStructuredBuffer (not in ConstantBuffer or ParameterBlock). The struct's
// int** field must be lowered to ulong to avoid Metal's pointer-to-pointer
// restriction in device buffer pointee types.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct PtrHolder
{
    int** indirect;
    int value;
};

RWStructuredBuffer<PtrHolder> buf;
RWStructuredBuffer<int> output;

// CHECK: ulong
// CHECK-NOT: device{{.*}}device

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = buf[tid.x].indirect;
    int* p = *pp;
    output[tid.x] = *p + buf[tid.x].value;
}
