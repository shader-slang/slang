// Verify that a user-defined scalar capability does not satisfy a builtin representation
// constraint.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

import slang.numerics;

struct CustomScalar : IScalarShapedType
{
    typealias Scalar = This;
    typealias Mask = bool;

    static This fromScalar(This value)
    {
        return value;
    }
}

void requiresBuiltinScalar<T : IBuiltinScalarShapedType>(T value) {}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    requiresBuiltinScalar(CustomScalar());
/*CHECK:
                         ^ type argument doesn't conform to interface
*/
}
