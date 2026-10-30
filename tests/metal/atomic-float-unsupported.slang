// Metal has no `atomic_float` load, exchange or compare-exchange in threadgroup memory or on
// textures (including a `RWBuffer`, which is a `texture_buffer` in MSL), no `atomic_half`, and no
// floating-point atomic min or max, so these float atomics report E55204. The same operations on
// device memory compile; see atomic-float-device.slang.

// The test is non-exhaustive because reading the uninitialized `sharedAtomic` also warns.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target metal -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<Atomic<float>> floatBuffer;
RWStructuredBuffer<Atomic<half>> halfBuffer;
RWTexture2D<float> floatTexture;
RWBuffer<float> floatTypedBuffer;
groupshared Atomic<float> sharedAtomic;
groupshared float sharedFloat;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float result = 0.0f;
    float original;

    result += sharedAtomic.load();
//CHECK:                       ^ intrinsic operation 'floating point atomic operation outside device memory' is not supported for the current target.
    result += sharedAtomic.exchange(1.0f);
//CHECK:                           ^ intrinsic operation 'floating point atomic operation outside device memory' is not supported for the current target.
    result += sharedAtomic.compareExchange(0.0f, 2.0f);
//CHECK:                                  ^ intrinsic operation 'floating point atomic operation outside device memory' is not supported for the current target.
    InterlockedExchange(sharedFloat, 3.0f, original);
//CHECK:               ^ intrinsic operation 'floating point atomic operation outside device memory' is not supported for the current target.
    result += original;
    InterlockedCompareExchange(sharedFloat, 0.0f, 4.0f, original);
//CHECK:                      ^ intrinsic operation 'floating point atomic operation outside device memory' is not supported for the current target.
    result += original;

    InterlockedExchange(floatTexture[uint2(0, 0)], 5.0f, original);
//CHECK:               ^ intrinsic operation 'floating point atomic operation on a texture' is not supported for the current target.
    result += original;
    InterlockedExchange(floatTypedBuffer[0], 5.0f, original);
//CHECK:               ^ intrinsic operation 'floating point atomic operation on a texture' is not supported for the current target.
    result += original;

    result += halfBuffer[0].load();
//CHECK:                        ^ intrinsic operation 'floating point atomic operation on a type other than 32-bit float' is not supported for the current target.

    result += floatBuffer[0].max(6.0f);
//CHECK:                        ^ intrinsic operation 'floating point atomic min or max' is not supported for the current target.
    result += floatBuffer[0].min(7.0f);
//CHECK:                        ^ intrinsic operation 'floating point atomic min or max' is not supported for the current target.

    outputBuffer[tid.x] = result;
}
