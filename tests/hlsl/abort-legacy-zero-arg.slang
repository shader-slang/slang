//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Verify that the bare zero-argument abort() call still resolves to the
// pre-existing HLSL SM 4.0 builtin declaration, without triggering the
// `abort` capability requirement of the new variadic
// abort(format, args...) overload (which requires a format argument, so
// overload resolution between the two is unambiguous).

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        abort();
    }
}

// CHECK-NOT: E36107
// CHECK: abort();
// CHECK-NOT: E36107
