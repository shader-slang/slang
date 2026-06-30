//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Case 1: Constrained generic parameter as enum underlying type (was segfault)
enum class TestEnum1<T : IArithmetic> : T
//CHECK:                                ^ enum 'TestEnum1' cannot inherit from type 'T' that is neither an interface not a builtin integer type
//CHECK:                                ^ type 'T' cannot be used for inheritance
{
    A = T(0),
    B = T(1)
}

// Case 2: Unconstrained generic parameter as enum underlying type (was ICE)
enum class TestEnum2<T> : T
//CHECK:                  ^ enum 'TestEnum2' cannot inherit from type 'T' that is neither an interface not a builtin integer type
//CHECK:                  ^ type 'T' cannot be used for inheritance
{
    X,
    Y
}
