//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Checking the argument of `[Use]` looks up `makePayload` before normal interface-requirement
// checking reaches either declaration. Lookup visits the two requirements newest-first, so the
// early signature check must recognize their identical direct function types and group them as one
// redeclaration family before overload resolution checks the attribute call. The test succeeds
// only if the call is unambiguous and compilation produces no diagnostic.

struct Payload {}

[__AttributeUsage(_AttributeTargets.Function)]
struct UseAttribute
{
    Payload value;
}

interface IRedeclaredOperation
{
    [Use(makePayload(1))]
    static void trigger();

    static __associatedfunc functype(int) -> Payload makePayload;
    static __associatedfunc functype(int) -> Payload makePayload;
}
