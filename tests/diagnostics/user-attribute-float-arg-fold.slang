//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

// Folding a floating-point argument of a user-defined attribute for reflection reports a
// floating-point constant whose definition depends on itself, and adds no integer diagnostics:
// `1 / 0` passed to a `half` parameter is reported once, by the coercion's range check
// (issue #13952).

[__AttributeUsage(_AttributeTargets.Function)]
struct floatProbeAttribute
{
    float value;
};

[__AttributeUsage(_AttributeTargets.Function)]
struct halfProbeAttribute
{
    half value;
};

static const float CIRCULAR_A = CIRCULAR_B;
//CHECK:           ^^^^^^^^^^ error E39999
static const float CIRCULAR_B = CIRCULAR_A;

// The circular definition is reported where `floatProbe(CIRCULAR_A)` folds it.
[floatProbe(CIRCULAR_A)]
[halfProbe(1 / 0)]
//CHECK:     ^ error E30002
//CHECK:     ^ implicit conversion not recommended
//CHECK:     ^ implicit conversion from 'int' to 'half' is not recommended
[numthreads(1, 1, 1)]
void main()
{
}
