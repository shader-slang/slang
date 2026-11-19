// Regression test for https://github.com/shader-slang/slang/issues/14183.
//
// The C++ backend lowers only integer `Atomic<T>.add` (see
// tests/language-feature/atomic-t/atomic-cpu-integer-add.slang); each other atomic operation is
// reported at the user's call with the operation and its value type. The `Atomic<T>` methods
// declared in extensions, and the `reduce*` and compound-assignment wrappers that inline to them,
// reach C++ emission because the extensions do not repeat the struct's `[require]`. Capability
// checking also does not visit property accessors, so the accessors below reach it with `load`,
// `store`, `exchange` and `compareExchange`.
//
// The test runs in exhaustive mode, so the integer `add`, `+=` and `reduceAdd` lines at the top of
// `computeMain` also check that the supported operation produces no diagnostic.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target shader-sharedlib
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target host-callable

struct AtomicCounter
{
    Atomic<uint> count;
    property uint value
    {
        [constref] get { return count.load(); }
//CHECK:                                  ^ unsupported intrinsic operation
//CHECK:                                  ^ intrinsic operation 'atomicLoad on uint' is not supported for the current target.
        [mutating] set { count.store(newValue); }
    }
    property uint previous
    {
        [mutating] get { return count.exchange(0); }
//CHECK:                                      ^ unsupported intrinsic operation
//CHECK:                                      ^ intrinsic operation 'atomicExchange on uint' is not supported for the current target.
        [mutating] set { count.compareExchange(0, newValue); }
    }
}

RWStructuredBuffer<Atomic<float>> floatBuffer;
RWStructuredBuffer<Atomic<int>> intBuffer;
RWStructuredBuffer<AtomicCounter> counterBuffer;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    uniform Atomic<float>* pf,
    uniform Atomic<double>* pd,
    uniform Atomic<int>* pi,
    uniform Atomic<uint>* pu,
    uniform Atomic<int64_t>* pl,
    uniform Atomic<half>* ph)
{
    pi->add(1);
    intBuffer[0] += 1;
    pi->reduceAdd(1);
    pf->add(1.0f);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicAdd on float' is not supported for the current target.
    pf->max(1.0f);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicMax on float' is not supported for the current target.
    pd->sub(1.0);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicSub on double' is not supported for the current target.
    ph->add(half(1.0));
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicAdd on half' is not supported for the current target.
    pi->sub(1);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicSub on int' is not supported for the current target.
    pi->min(1);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicMin on int' is not supported for the current target.
    pi->max(1);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicMax on int' is not supported for the current target.
    pu->and(1u);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicAnd on uint' is not supported for the current target.
    pu->or(1u);
//CHECK:  ^ unsupported intrinsic operation
//CHECK:  ^ intrinsic operation 'atomicOr on uint' is not supported for the current target.
    pu->xor(1u);
//CHECK:   ^ unsupported intrinsic operation
//CHECK:   ^ intrinsic operation 'atomicXor on uint' is not supported for the current target.
    pi->increment();
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicInc on int' is not supported for the current target.
    pl->decrement();
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicDec on int64_t' is not supported for the current target.
    pf->reduceAdd(1.0f);
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicAdd on float' is not supported for the current target.
    pi->reduceSub(1);
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicSub on int' is not supported for the current target.
    pi->reduceMin(1);
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicMin on int' is not supported for the current target.
    pi->reduceMax(1);
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicMax on int' is not supported for the current target.
    pf->reduceSub(1.0f);
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicSub on float' is not supported for the current target.
    floatBuffer[0] += 1.0f;
//CHECK:           ^^ unsupported intrinsic operation
//CHECK:           ^^ intrinsic operation 'atomicAdd on float' is not supported for the current target.
    intBuffer[0] -= 1;
//CHECK:         ^^ unsupported intrinsic operation
//CHECK:         ^^ intrinsic operation 'atomicSub on int' is not supported for the current target.
    intBuffer[0]++;
//CHECK:        ^^ unsupported intrinsic operation
//CHECK:        ^^ intrinsic operation 'atomicInc on int' is not supported for the current target.
    intBuffer[1] = 5;
//CHECK:         ^ unsupported intrinsic operation
//CHECK:         ^ intrinsic operation 'atomicStore on int' is not supported for the current target.
    outputBuffer[0] = counterBuffer[0].value;
    counterBuffer[0].value = 3;
//CHECK:                   ^ unsupported intrinsic operation
//CHECK:                   ^ intrinsic operation 'atomicStore on uint' is not supported for the current target.
    outputBuffer[1] = counterBuffer[0].previous;
    counterBuffer[0].previous = 4;
//CHECK:                      ^ unsupported intrinsic operation
//CHECK:                      ^ intrinsic operation 'atomicCompareExchange on uint' is not supported for the current target.
}
