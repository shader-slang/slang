// Verify that the broad builtin arithmetic constraint excludes Boolean and user-defined values.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

// CHECK: type argument 'bool' does not conform to the required interface
// CHECK: type argument 'CustomArithmetic' does not conform to the required interface

import slang.numerics;

void requireBuiltinScalarArithmetic<T : IBuiltinScalarArithmetic>()
{}

struct CustomArithmetic : IScalarNumeric, INumericalExtrema
{
    int value;

    typealias Scalar = This;
    typealias Mask = bool;

    __init<TSource : IBuiltinScalarIntegerType>(TSource value)
    {
        this.value = __intCast<int>(value);
    }

    static This fromScalar(This value)
    {
        return value;
    }

    static This zero()
    {
        return {0};
    }

    static This one()
    {
        return {1};
    }

    This plus(This other)
    {
        return {value + other.value};
    }

    This minus(This other)
    {
        return {value - other.value};
    }

    This multipliedBy(This other)
    {
        return {value * other.value};
    }

    This minimum(This other)
    {
        return {min(value, other.value)};
    }

    This maximum(This other)
    {
        return {max(value, other.value)};
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    requireBuiltinScalarArithmetic<bool>();
    requireBuiltinScalarArithmetic<CustomArithmetic>();
}
