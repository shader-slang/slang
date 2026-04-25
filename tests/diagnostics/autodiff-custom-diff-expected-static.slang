//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test for error 31156 (expected static custom derivative).
// A static member function with a non-static derivative should trigger this error.

struct MyStruct
{
    float val;

    [ForwardDerivative(fwd_static_method)]
    /*CHECK:
     ^^^^^^^^^^^^^^^^^ expected static custom derivative
     ^^^^^^^^^^^^^^^^^ expected a static definition for the custom derivative.
    */
    static float static_method(float x)
    {
        return x * x;
    }

    DifferentialPair<float> fwd_static_method(DifferentialPair<float> x)
    {
        return diffPair(0.0, 0.0);
    }
}
