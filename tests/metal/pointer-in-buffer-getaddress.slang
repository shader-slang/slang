//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Test __getAddress on a pointer field accessed through a uniform pointer.
// __getAddress(data->ptr) where ptr is int** produces int*** — a multi-level
// pointer that exercises our lowering pass.
//
// Note: __getAddress on structured buffer subscripts (e.g. buf[i]) is a
// pre-existing frontend limitation on all targets. The working pattern is
// accessing through pointer fields as shown here.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Inner
{
    int** ptr;
    int value;
};

uniform Inner* data;
RWStructuredBuffer<int> output;

// The Inner struct should have ptr lowered to ulong.
// CHECK-LABEL: struct Inner
// CHECK: ulong
// CHECK: };

// The kernel casts ulong back to a typed pointer for dereferencing.
// (__getAddress + deref optimizes to direct field access, so the triple
// pointer collapses to a double-pointer cast in the generated code.)
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int*** ppp = __getAddress(data->ptr);
    output[tid.x] = ***ppp;
}
