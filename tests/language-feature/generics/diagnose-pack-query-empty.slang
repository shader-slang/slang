//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test: pack queries (__first, __last) on empty concrete packs
// and on unconstrained packs produce errors 30411/30412.

// Concrete empty pack: error 30411
int badFirstEmpty()
{
    return __first(makeTuple());
//CHECK: __first requires a non-empty pack
}

int badLastEmpty()
{
    return __last(makeTuple());
//CHECK: __last requires a non-empty pack
}

// Unconstrained pack: error 30412
int badFirstUnconstrained<each T>()
{
    return countof(Tuple<__first(T)>);
//CHECK: __first requires a non-empty pack
}

int badLastUnconstrained<each T>()
{
    return countof(Tuple<__last(T)>);
//CHECK: __last requires a non-empty pack
}
