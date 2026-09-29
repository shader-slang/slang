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

    // These wrappers remain calls until after the differentiability check.
    atomicFloats[0].reduceAdd(x);
//CHECK:                     ^ derivative is lost in atomic operation
//CHECK:                     ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[0].reduceSub(x);
//CHECK:                     ^ derivative is lost in atomic operation
//CHECK:                     ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[0].reduceMin(x);
//CHECK:                     ^ derivative is lost in atomic operation
//CHECK:                     ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[0].reduceMax(x);
//CHECK:                     ^ derivative is lost in atomic operation
//CHECK:                     ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    __atomic_reduce_add(floats[0], x);
//CHECK:               ^ derivative is lost in atomic operation
//CHECK:               ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    __atomic_reduce_sub(floats[0], x);
//CHECK:               ^ derivative is lost in atomic operation
//CHECK:               ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    __atomic_reduce_min(floats[0], x);
//CHECK:               ^ derivative is lost in atomic operation
//CHECK:               ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    __atomic_reduce_max(floats[0], x);
//CHECK:               ^ derivative is lost in atomic operation
//CHECK:               ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[0] += x;
//CHECK:            ^^ derivative is lost in atomic operation
//CHECK:            ^^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    atomicFloats[0] -= x;
//CHECK:            ^^ derivative is lost in atomic operation
//CHECK:            ^^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    InterlockedCompareExchangeFloatBitwise(floats[0], x, 0.0f);
//CHECK:                                  ^ derivative is lost in atomic operation
//CHECK:                                  ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    float previous;
    InterlockedCompareExchangeFloatBitwise(floats[0], 0.0f, x, previous);
//CHECK:                                  ^ derivative is lost in atomic operation
//CHECK:                                  ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    return x * x;
}

[Differentiable]
float acknowledgedLossesAreAccepted(float x)
{
    no_diff atomicFloats[2].add(x);
    no_diff(InterlockedAdd(floats[1], x));
    atomicFloats[3].add(detach(x));
    counters[0].add(1u);
    no_diff atomicFloats[0].reduceAdd(x);
    atomicFloats[0].reduceSub(detach(x));
    no_diff atomicFloats[0].reduceMin(x);
    atomicFloats[0].reduceMax(detach(x));
    no_diff __atomic_reduce_add(floats[0], x);
    __atomic_reduce_sub(floats[0], detach(x));
    no_diff __atomic_reduce_min(floats[0], x);
    __atomic_reduce_max(floats[0], detach(x));
    atomicFloats[0] += detach(x);
    atomicFloats[0] -= detach(x);
    no_diff (atomicFloats[0] += x);
    no_diff (atomicFloats[0] -= x);
    no_diff InterlockedCompareExchangeFloatBitwise(floats[0], x, x);
    float original = no_diff atomicFloats[4].add(x);
    floats[2] = original;
    return x * x + atomicFloats[5].add(detach(x));
}

// An atomic's out result is derivative-free when reused by another atomic or a store.
[Differentiable]
float originalValuesAreAccepted(float x)
{
    float original;
    no_diff InterlockedAdd(floats[0], x, original);
    InterlockedAdd(floats[1], original);
    atomicFloats[0].add(original);
    floats[2] = original;
    InterlockedCompareExchangeFloatBitwise(floats[0], 0.0f, detach(x), original);
    atomicFloats[0].add(original);
    return x * x + original;
}

// Detaching an atomic output must not hide derivatives from later writes or phi nodes.
[Differentiable]
float overwrittenOriginalIsChecked(float x)
{
    float original;
    InterlockedAdd(floats[0], detach(x), original);
    if (x > 0.0f)
        original = x;
    InterlockedAdd(floats[1], original);
//CHECK:          ^ derivative is lost in atomic operation
//CHECK:          ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    return x * x;
}
