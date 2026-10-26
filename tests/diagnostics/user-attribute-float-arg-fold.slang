//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

// Folding a floating-point argument of a user-defined attribute reports the same diagnostics the
// integer folder reports for an integer argument (issue #13952).

[__AttributeUsage(_AttributeTargets.Function)]
struct floatProbeAttribute
{
    float value;
};

static const float CIRCULAR_A = CIRCULAR_B;
//CHECK:           ^^^^^^^^^^ error E39999
static const float CIRCULAR_B = CIRCULAR_A;

[floatProbe(1 / 0)]
//CHECK:      ^ error E30002
[floatProbe(CIRCULAR_A)]
[numthreads(1, 1, 1)]
void main()
{
}
