// SPIR-V and GLSL have only an integer atomic compare-exchange, and neither can reinterpret a
// floating-point variable as an integer one: logical SPIR-V cannot bitcast a pointer, and GLSL
// cannot reinterpret an lvalue. A floating-point compare-exchange on a `float`, `half` or `double`
// location (a buffer element, groupshared memory, a texel or a pointer) therefore reports E55204
// on these targets. `RWByteAddressBuffer` stores `uint` words, so its
// `InterlockedCompareExchangeFloatBitwise` method compiles; see
// tests/hlsl-intrinsic/atomic/byte-address-buffer-float-bitwise-cas.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-via-glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<Atomic<float>> floatAtomics;
RWStructuredBuffer<Atomic<half>> halfAtomics;
RWStructuredBuffer<Atomic<double>> doubleAtomics;
RWStructuredBuffer<float> floats;
groupshared Atomic<float> sharedAtomic;
RWTexture2D<float> floatTexture;
uniform float* floatPointer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float result = 0.0f;
    float original;
    sharedAtomic.store(0.0f);

    result += floatAtomics[0].compareExchange(0.0f, 1.0f);
//CHECK:                                     ^ error E55204
//CHECK:                                     ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += halfAtomics[0].compareExchange(half(0.0), half(1.0));
//CHECK:                                    ^ error E55204
//CHECK:                                    ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += float(doubleAtomics[0].compareExchange(0.0, 1.0));
//CHECK:                                            ^ error E55204
//CHECK:                                            ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += sharedAtomic.compareExchange(0.0f, 1.0f);
//CHECK:                                  ^ error E55204
//CHECK:                                  ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.

    InterlockedCompareExchange(floats[0], 0.0f, 1.0f, original);
//CHECK:                      ^ error E55204
//CHECK:                      ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += original;
    InterlockedCompareStore(floats[1], 0.0f, 1.0f);
//CHECK:                   ^ error E55204
//CHECK:                   ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    InterlockedCompareExchangeFloatBitwise(floats[2], 0.0f, 1.0f, original);
//CHECK:                                  ^ error E55204
//CHECK:                                  ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += original;
    InterlockedCompareStoreFloatBitwise(floats[3], 0.0f, 1.0f);
//CHECK:                               ^ error E55204
//CHECK:                               ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.

    InterlockedCompareExchange(floatTexture[uint2(0, 0)], 0.0f, 1.0f, original);
//CHECK:                      ^ error E55204
//CHECK:                      ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += original;

    InterlockedCompareExchange(*floatPointer, 0.0f, 1.0f, original);
//CHECK:                      ^ error E55204
//CHECK:                      ^ intrinsic operation 'floating point atomic compare-exchange' is not supported for the current target.
    result += original;

    outputBuffer[tid.x] = result;
}
