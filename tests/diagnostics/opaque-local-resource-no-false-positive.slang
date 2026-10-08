// Issue #10526: the opaque-local check is deliberately *narrower* than
// `isOpaqueType` — buffer-backed resources (StructuredBuffer/ByteAddressBuffer/
// etc.) and `RayQuery` lower through SPIR-V variable pointers / OpTypeRayQueryKHR
// and *can* legitimately be selected through control flow into a local. They
// must NOT be rejected. This guards the carve-out against a regression that
// broadened the check to all opaque types.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment

StructuredBuffer<float> b0, b1;

// StructuredBuffer selected via `?:` — must compile, no 56004.
[noinline]
StructuredBuffer<float> selBuffer(bool b)
{
    return b ? b1 : b0;
}

// RayQuery declared as a local and selected via `if`/`else` — must compile.
[noinline]
float useRayQuery(bool b)
{
    RayQuery<RAY_FLAG_NONE> q;
    return b ? 1.0f : 0.0f;
}

[shader("fragment")]
float main(bool b) : SV_Target
{
    return selBuffer(b)[0] + useRayQuery(b);
}

// CHECK-NOT: opaque type in local variable
// CHECK-NOT: 56004
// CHECK: OpEntryPoint
