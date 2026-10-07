//TEST:SIMPLE: -no-codegen

// Lookup for a default-value expression can occur before its containing callable has checked
// effective-`this` information. Lookup uses a small declaration-spelling bootstrap during that
// interval; treating `this` as immutable would reject these valid header expressions. This test is
// intentionally limited to header checking: materializing a default expression that mutates its
// receiver is a separate, pre-existing limitation of default-argument expansion.

struct HeaderLookup
{
    static const int elementCount = 4;
    int value;

    // A parameter type is semantically checked while its callable advances from `ScopesWired` to
    // `SignatureChecked`. The `this` expression must use a preview of the effective parameter
    // information instead of trying to ensure that this method has already completed that work.
    [mutating]
    void arrayParameter(int[this.elementCount] values)
    {
        value = values[0];
    }

    [mutating]
    int mutateAndReturn()
    {
        value++;
        return value;
    }

    [mutating]
    void mutatingDefault(int replacement = mutateAndReturn())
    {
        value = replacement;
    }

    [__ref]
    void refDefault(int replacement = mutateAndReturn())
    {
        value = replacement;
    }
}
