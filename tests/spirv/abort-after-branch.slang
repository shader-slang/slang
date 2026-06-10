//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// OpAbortKHR is a block terminator. When abort() is called on one arm of a
// branch, the SPIRV legalization pass must strip the abort block's original
// IR branch (replacing it with `unreachable`), so the merge block's phis no
// longer name the abort block as a predecessor — otherwise the emitted SPIRV
// would contain an OpPhi whose parent never branches to it. This test pins
// that the abort block ends at OpAbortKHR, immediately followed by the next
// block's label.

RWStructuredBuffer<float> outBuf;

float pickValue(uint x)
{
    float v;
    if (x > 100)
    {
        abort("oops: %u", x);
        v = 0.0;
    }
    else
    {
        v = 1.0;
    }
    return v;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = pickValue(tid.x);
}

// CHECK: OpAbortKHR
// CHECK-NEXT: %{{[a-zA-Z0-9_]+}} = OpLabel
