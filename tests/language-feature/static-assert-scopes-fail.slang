// A failing `static_assert` outside a function body is reported even though nothing references its
// enclosing scope.

// The diagnostics fire during the per-target `static_assert` check before backend emission, so the
// same annotations hold on every target; we run two shipping backends to exercise the check on both.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

struct S
{
    int a;

    static_assert(1 == 2, "one is not two");
//CHECK:         ^ static assertion failed
//CHECK:         ^ static assertion failed, one is not two
}

static_assert(sizeof(S) == 8, "S must be eight bytes");
//CHECK:     ^ static assertion failed
//CHECK:     ^ static assertion failed, S must be eight bytes

static_assert(sizeof(S) == 8);
//CHECK:     ^ static assertion failed
//CHECK:     ^ static assertion failed.

namespace N
{
    static_assert(false, "failed in namespace");
//CHECK:         ^ static assertion failed
//CHECK:         ^ static assertion failed, failed in namespace
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
