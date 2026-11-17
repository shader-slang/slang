// On the C++ CPU target, atomics on `half`, which the C++ prelude has no helper for, report E55204
// at the call.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

RWStructuredBuffer<half> h;
RWStructuredBuffer<Atomic<half>> ah;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    InterlockedAdd(h[0], half(1));
//CHECK:          ^ unsupported intrinsic operation
//CHECK:          ^ intrinsic operation 'atomic add on half' is not supported for the current target.
    ah[0].max(half(2));
//CHECK:     ^ unsupported intrinsic operation
//CHECK:     ^ intrinsic operation 'atomic max on half' is not supported for the current target.
}
