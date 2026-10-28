// Regression test for https://github.com/shader-slang/slang/issues/13969
//
// `vector<T,N> == vector<T,N>` has type `vector<bool,N>` whether the element type is concrete or a
// generic parameter, so returning it as a scalar `bool` is the same type mismatch in every form below.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

bool eqConcrete(float3 a, float3 b)
{
    return a == b;
//CHECK:     ^^ error E30019
//CHECK:     ^^ expected an expression of type 'bool', got 'vector<bool,3>'
}

bool eqFloatingPoint<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return a == b;
//CHECK:     ^^ error E30019
//CHECK:     ^^ expected an expression of type 'bool', got 'vector<bool,3>'
}

bool neqInteger<T : __BuiltinIntegerType>(vector<T, 3> a, vector<T, 3> b)
{
    return a != b;
//CHECK:     ^^ error E30019
//CHECK:     ^^ expected an expression of type 'bool', got 'vector<bool,3>'
}

bool eqArithmetic<T : __BuiltinArithmeticType>(vector<T, 3> a, vector<T, 3> b)
{
    return a == b;
//CHECK:     ^^ error E30019
//CHECK:     ^^ expected an expression of type 'bool', got 'vector<bool,3>'
}
