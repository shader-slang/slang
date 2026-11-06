// This example shows how to use Slang's automatic differentiation feature
// to compute the derivatives.

// For more information on automatic differentiation, see documentation at:
// https://shader-slang.com/slang/user-guide/autodiff.html

import playground;

// Define a simple function and mark it as differentiable.
[Differentiable]
float sumOfSquares(float x, float y)
{
    return x * x + y * y;
}

void printMain()
{
    let result = sumOfSquares(2.0, 3.0);
    printf("Result: %f\n", result);

    // Use forward differentiation to compute the gradient of the output w.r.t. x only.
    let diffX = fwd_diff(sumOfSquares)(diffPair(2.0, 1.0), diffPair(3.0, 0.0));
    printf("dF wrt x: %f\n", diffX.d);

    // Use forward differentiation to propagate the gradient from input parameters to
    // output value.
    let diffXY = fwd_diff(sumOfSquares)(diffPair(2.0, 1.0), diffPair(3.0, 1.0));
    printf("dF wrt x and y: %f\n", diffXY.d);

    // Use backward diffeerentiation to propagate the gradient from the output to
    // all input parameters at the same time.

    // Create a differentiable pair to pass in the primal value and to receivve the gradient.
    DifferentialPair<float> dpX = diffPair(2.0, 0.0);
    DifferentialPair<float> dpY = diffPair(3.0, 0.0);
    // Propagate the gradient of the output (1.0f) to the input parameters.
    bwd_diff(sumOfSquares)(dpX, dpY, 1.0);
    printf("dF wrt x computed using backward differentiation: %f\n", dpX.d);
    printf("dF wrt y computed using backward differentiation: %f\n", dpY.d);
}