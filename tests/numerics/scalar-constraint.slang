// Verify that a shaped builtin type cannot satisfy a scalar-only numeric constraint.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

import slang.numerics;

void requiresScalar<T : IScalarFloatingPoint>(T value)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    requiresScalar(float3(0.0));
/*CHECK:
                  ^ type argument doesn't conform to interface
                  ^ type argument 'vector<float,3>' does not conform to the required interface 'IScalarFloatingPoint'
*/
}
