//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Consider `replacement = read()` in the two generic methods below. In the instance method,
// lookup crosses the linked generic wrapper with a `this` value and can call `read`. In the static
// method, the wrapper is a receiverless boundary, so lookup reaches `HeaderLookup` only as a type
// and diagnoses the attempted call to the instance method.

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
