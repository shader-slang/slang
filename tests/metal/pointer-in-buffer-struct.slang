//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Metal's [[buffer(N)]] validation rejects pointer-to-pointer types
// (e.g. `device int *device *`) in buffer pointee types, regardless of
// nesting depth. The type itself is valid MSL — the restriction is only
// on buffer-bound parameters. The fix lowers pointer fields in
// buffer-backed structs to ulong, with casts at point of use.
//
// This test covers single-pointer fields (which MSL already accepts in
// buffer structs, but which we lower uniformly for consistency) and
// verifies the lowered output for the basic case.
//
// See also: pointer-in-buffer-double.slang, pointer-in-buffer-triple.slang
// See: https://github.com/shader-slang/slang/issues/10841

struct CopyParams
{
    int* src;
    int* dst;
};

ParameterBlock<CopyParams> params;

// Verify struct fields are lowered to ulong, not device pointers.
// CHECK: struct CopyParams
// CHECK: ulong
// CHECK-NOT: device*
// CHECK: };

// Verify the kernel body contains casts back to typed pointers.
// CHECK: computeMain
// CHECK: (int device*)

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.dst[tid.x] = params.src[tid.x];
}
