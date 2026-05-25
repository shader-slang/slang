//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Multi-level pointer fields (int**, int***) in a ParameterBlock struct.
// Metal rejects pointer-to-pointer types in buffer pointee types. Both
// double and triple pointer depths must be lowered to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct PBParams
{
    int** double_ptr;
    int*** triple_ptr;
};

ParameterBlock<PBParams> params;

// Both multi-level pointer fields lowered to ulong.
// CHECK-LABEL: struct PBParams
// CHECK: ulong double_ptr
// CHECK: ulong triple_ptr
// CHECK-NOT: device*
// CHECK: };

// Kernel body reconstructs typed pointers.
// CHECK-LABEL: computeMain
// CHECK: int device* device*
// CHECK: int device* device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.double_ptr[tid.x][0] = int(tid.x);
    params.triple_ptr[0][tid.x][0] = int(tid.x);
}
