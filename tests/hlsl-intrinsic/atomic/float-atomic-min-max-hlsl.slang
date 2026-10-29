// HLSL has no floating-point atomic min/max, so float, half and double InterlockedMin/Max and
// Atomic<T>.min/max report a Slang error instead of emitting HLSL that DXC rejects.
// https://github.com/shader-slang/slang/issues/14026

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -profile cs_6_6

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<half> halfBuffer;
RWStructuredBuffer<double> doubleBuffer;
RWStructuredBuffer<Atomic<float>> atomicFloatBuffer;
RWStructuredBuffer<Atomic<half>> atomicHalfBuffer;
RWStructuredBuffer<Atomic<double>> atomicDoubleBuffer;
RWStructuredBuffer<int> intBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    InterlockedMin(floatBuffer[0], float(tid));
//CHECK:          ^ unsupported intrinsic operation
//CHECK:          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    InterlockedMax(floatBuffer[1], float(tid));
//CHECK:          ^ unsupported intrinsic operation
//CHECK:          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    InterlockedMin(halfBuffer[0], half(tid));
//CHECK:          ^ unsupported intrinsic operation
//CHECK:          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    InterlockedMax(doubleBuffer[0], double(tid));
//CHECK:          ^ unsupported intrinsic operation
//CHECK:          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    atomicFloatBuffer[0].min(float(tid));
//CHECK:                    ^ unsupported intrinsic operation
//CHECK:                    ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    atomicFloatBuffer[1].max(float(tid));
//CHECK:                    ^ unsupported intrinsic operation
//CHECK:                    ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.

    float orig;
    InterlockedMax(floatBuffer[2], float(tid), orig);
//CHECK:          ^ error E55204
//CHECK:          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    atomicHalfBuffer[0].min(half(tid));
//CHECK:                   ^ error E55204
//CHECK:                   ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    atomicDoubleBuffer[0].max(double(tid));
//CHECK:                     ^ error E55204
//CHECK:                     ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.
    atomicFloatBuffer[2].reduceMin(float(tid));
//CHECK:                          ^ error E55204
//CHECK:                          ^ intrinsic operation 'floating point atomic operation' is not supported for the current target.

    // Integer min/max are valid HLSL and must not be diagnosed.
    InterlockedMin(intBuffer[0], int(tid));
    InterlockedMax(intBuffer[1], int(tid));
}
