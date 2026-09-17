//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12929
//
// Check that precision loss in integer literal -> float conversion is diagnosed.
//
// diag=CHECK is an exhaustive match: a source line with no //CHECK asserts that
// no diagnostic is emitted for it.

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Exactly representable in float -> NO warning.
    float sum = 0;
    sum += 1;
    sum += 2;

    // contiguous-exact-range boundary
    sum += 16777215;
    sum += 16777216; // last representable
    sum += 16777218; // now every second is representable

    sum += 16777217;
//CHECK:   ^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^ precision lost in conversion from 'int' to 'float'

    // largest representable
    float h = 18446744073692774400LLU;

    float i = 18446743000000000000LLU;


    output[0] = sum;
}
