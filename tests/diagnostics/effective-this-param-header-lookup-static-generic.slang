//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Consider `replacement = read()` in the two generic methods below. In the instance method,
// `_lookUpInScopes()` crosses the linked generic wrapper and appends a `ThisValue` breadcrumb, so
// `ConstructLookupResultExpr()` can form the call to `read`. In the static method, the wrapper is
// receiverless, so `_lookUpInScopes()` appends a `ThisType` breadcrumb for `HeaderLookup` instead.
// `ConstructLookupResultExpr()` then diagnoses the attempted call to the instance method.

interface IHeaderConstraint
{
}

struct HeaderLookup
{
    int value;

    int read()
    {
        return value;
    }

    int genericInstanceDefault<T : IHeaderConstraint>(T unused, int replacement = read())
    {
        return replacement;
    }

    static int genericStaticDefault<T : IHeaderConstraint>(T unused, int replacement = read())
//CHECK:                                                                               ^^^^ type cannot refer to non-static member
//CHECK:                                                                               ^^^^ type 'HeaderLookup' cannot be used to refer to non-static member 'read'
    {
        return replacement;
    }
}
