//TEST:SIMPLE: -no-codegen

// Parameter types can refer to `this` before signature checking has attached the containing
// method's receiver parameter information. Default arguments also perform member lookup and must
// use the same receiver mode as an explicit `this` expression. This test stops after header
// checking because default-argument expansion does not materialize a default expression that
// mutates its receiver.

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
