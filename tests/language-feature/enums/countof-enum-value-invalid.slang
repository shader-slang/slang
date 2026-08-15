// countof accepts an enum TYPE but not an enum-typed VALUE (see shader-slang/slang#12549):
// countof of an enum value or an enum case must still be rejected with E30083.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum DataType
{
    A,
    B,
    C,
}

int f()
{
    DataType e = DataType.A;
    return countof(e);
//CHECK:   ^^^^^^^ E30083
//CHECK:   ^^^^^^^ argument to countof can only be a type pack, tuple, array, or enum type
}

int g()
{
    // An enum case is also a value, not a type, so this is rejected too.
    return countof(DataType.A);
//CHECK:   ^^^^^^^ E30083
//CHECK:   ^^^^^^^ argument to countof can only be a type pack, tuple, array, or enum type
}
