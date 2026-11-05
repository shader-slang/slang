//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `__floatAsInt` folds a one-argument call on a literal only when the call converts the
// literal to the floating-point type, as in `float(5)`. A call that reinterprets the bits
// or computes a new value must not be folded as if it were that conversion.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Error: `bit_cast` reinterprets the bits of 0x3F800000u, it does not convert the value
    int a = __floatAsInt(bit_cast<float>(0x3F800000u));
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    // Error: `asfloat` reinterprets the bits as well
    int b = __floatAsInt(asfloat(0x3F800000u));
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    // Error: `sqrt(4.0f)` is 2.0f, not 4.0f
    int c = __floatAsInt(sqrt(4.0f));
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    outputBuffer[0] = a + b + c;
}
