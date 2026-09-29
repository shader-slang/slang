//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An atomic operation that writes a derivative-carrying value into memory loses the
// derivative, the same way a store to non-differentiable memory does, so inside a
// differentiable function the loss must be acknowledged with `no_diff` or `detach()`.
// Integer atomics and the value an atomic returns need no acknowledgement. The test is
// exhaustive, so every accepted form below is also checked to produce no diagnostic.

RWStructuredBuffer<float> floats;
RWStructuredBuffer<Atomic<float>> atomicFloats;
RWStructuredBuffer<Atomic<uint>> counters;
RWByteAddressBuffer bytes;

[Differentiable]
float lossesAreReported(float x)
{
    atomicFloats[0].add(x);
//CHECK:               ^ derivative is lost in atomic operation
//CHECK:               ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[1].exchange(x * 2.0f);
//CHECK:                    ^ derivative is lost in atomic operation
//CHECK:                    ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    InterlockedAdd(floats[0], x);
//CHECK:          ^ derivative is lost in atomic operation
//CHECK:          ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    bytes.InterlockedAddF32(0, x);
//CHECK:                   ^ derivative is lost in atomic operation
//CHECK:                   ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    return x * x;
}

[Differentiable]
float acknowledgedLossesAreAccepted(float x)
{
    no_diff atomicFloats[2].add(x);
    no_diff(InterlockedAdd(floats[1], x));
    atomicFloats[3].add(detach(x));
    counters[0].add(1u);
    float original = no_diff atomicFloats[4].add(x);
    floats[2] = original;
    return x * x + atomicFloats[5].add(detach(x));
}
