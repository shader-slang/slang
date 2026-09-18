//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12929
//
// Check that precision loss in integer literal -> float conversion is diagnosed.
//
// diag=CHECK is an exhaustive match: a source line with no //CHECK asserts that
// no diagnostic is emitted for it.

RWStructuredBuffer<float> output;

static const int kLossy = 16777217;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Exactly representable in float -> NO warning.
    float sum = 0;
    sum += 1;
    sum += 2;

    // contiguous-exact-range boundary
    sum += 16777215;
    sum += 0x0000000000FFFFFFLLU;
    sum += 16777216; // last representable
    sum += 16777218; // now every second is representable
    sum += 16777220; // now every second is representable

    sum += -16777215;
    sum += -16777216; // last representable
    sum += -16777218; // now every second is representable
    sum += -16777220; // now every second is representable

    // largest representables
    sum += 18446742974197923840LLU;
    sum += 0xFFFFFF0000000000LLU;

    // Two's complement minimum is a power of two, so it is exact
    sum += -9223372036854775808;
    sum += -0x8000000000000000LL;

    // One ULP less
    sum += -9223371487098961920LL;
    sum += -0x7FFFFF8000000000LL;

    // 32-bit unsigned source, 24-bit span
    sum += 4294967040u; // 0xFFFFFF00

    // An explicit cast opts out of the diagnostic entirely
    sum += (float)16777217;
    sum += float(16777217);

    sum += 16777217;
//CHECK:   ^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^ precision loss in converting value '16777217' to 'float'

    sum += -16777217;
//CHECK:    ^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^ precision loss in converting value '-16777217' to 'float'

    // one mantissa bit too many at various scales
    sum += 0xFFFFFF8000000000LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '18446743523953737728' to 'float'

    sum += 0x7FFFFFC000000000LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '9223371761976868864' to 'float'

    sum += -0x7FFFFFC000000000LL;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '-9223371761976868864' to 'float'

    sum += 0x8000008000000000LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '9223372586610589696' to 'float'

    sum += 0x0000FFFFFF800000LLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '281474968322048' to 'float'

    sum += -0x0000FFFFFF800000LL;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '-281474968322048' to 'float'

    sum += -281474968322048;
//CHECK:    ^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^^^^^^^^ precision loss in converting value '-281474968322048' to 'float'

    sum += 0x0000000001FFFFFFLLU;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '33554431' to 'float'

    sum += -33554431;
//CHECK:    ^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:    ^^^^^^^^ precision loss in converting value '-33554431' to 'float'

    sum += 9223372036854775807;
//CHECK:   ^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^ precision loss in converting value '9223372036854775807' to 'float'

    sum += 18446744073709551615u;
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^^^^^^^^^^^ precision loss in converting value '18446744073709551615' to 'float'

    sum += 4294967295u; // 0xFFFFFFFF, 32-bit span
//CHECK:   ^^^^^^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^^^^^^ precision loss in converting value '4294967295' to 'float'

    // The check runs on the constant-folded value, not on the literal token.
    sum += kLossy;
//CHECK:   ^^^^^^ implicit conversion not recommended: precision loss
//CHECK:   ^^^^^^ precision loss in converting value '16777217' to 'float'

    sum += 16777216 + 1;
//CHECK:            ^ implicit conversion not recommended: precision loss
//CHECK:            ^ precision loss in converting value '16777217' to 'float'

    output[0] = sum;
}
