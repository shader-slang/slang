//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test for errors 31152 (interface requirement cannot have derivative)
// and 31153 (interface requirement cannot be used as derivative).

[Differentiable]
float myFunc(float x)
{
    return x * x;
}

// Case 1: Marking an interface requirement as a derivative target (error 31152).
// Using [ForwardDerivativeOf] on a method that points to an interface requirement.
interface IBar
{
    [Differentiable]
    float original(float x);

    [ForwardDerivativeOf(original)]
    /*CHECK:
     ^^^^^^^^^^^^^^^^^^^ interface requirement cannot have derivative
     ^^^^^^^^^^^^^^^^^^^ cannot associate an interface requirement with a derivative.
    */
    DifferentialPair<float> fwd_original(DifferentialPair<float> x);
}

// Case 2: Using an interface requirement as a derivative (error 31153).
// An interface requirement function used as a derivative for a free function.
interface IBaz
{
    [ForwardDerivativeOf(myFunc)]
    /*CHECK:
     ^^^^^^^^^^^^^^^^^^^ interface requirement cannot be used as derivative
     ^^^^^^^^^^^^^^^^^^^ cannot use an interface requirement as a derivative.
    */
    DifferentialPair<float> fwd_compute(DifferentialPair<float> x);
}
