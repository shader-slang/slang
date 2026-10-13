//TEST:SIMPLE: -no-codegen

// Lookup for a default-value expression can run while its containing callable's signature is still
// being checked. Lookup records that expression checking should attempt an instance base without
// computing its mode. Expression checking then validates the reconstructed `this` expression and
// obtains its effective receiver mode. This test stops after header checking because
// default-argument expansion does not materialize a default expression that mutates its receiver.

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
