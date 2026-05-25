//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// RWStructuredBuffer with pointer element types inside a ParameterBlock.
// The buffer's pointer element produces pointer-to-pointer in the argument
// buffer struct, which Metal rejects. Tests both single-level (int*) and
// double-level (int**) element types to verify all depths are lowered.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct SingleElem
{
    RWStructuredBuffer<int*> ptrs;
};

struct DoubleElem
{
    RWStructuredBuffer<int**> indirect;
};

ParameterBlock<SingleElem> params_single;
ParameterBlock<DoubleElem> params_double;

// Both argument-buffer structs must contain ulong (lowered pointer element).
// CHECK-LABEL: struct SingleElem
// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK: };

// CHECK-LABEL: struct DoubleElem
// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK: };

// Kernel body casts ulong back to typed pointers.
// CHECK-LABEL: computeMain
// CHECK: int device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = params_single.ptrs[tid.x];
    int** pp = params_double.indirect[tid.x];
    *p = **pp + int(tid.x);
}
