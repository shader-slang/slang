//TEST:SIMPLE: -no-codegen

// Consider `int[this.elementCount]` in `arrayParameter` below. Signature checking publishes the
// method's receiver information before checking the array parameter, so semantic checking can
// resolve the explicit `this` without recursively restarting the signature. The unqualified
// `mutateAndReturn()` calls in default arguments exercise the same ordering through lookup:
// `_lookUpInScopes()` appends a `ThisValue` breadcrumb, and `ConstructLookupResultExpr()` uses the
// one receiver description already published for the method.

struct HeaderLookup
{
    static const int elementCount = 4;
    int value;

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

    [mutating]
    int genericInstanceDefault<T>(T unused, int replacement = mutateAndReturn())
    {
        value = replacement;
        return replacement;
    }

    static int genericStaticDefault<T>(T unused, int count = elementCount)
    {
        return count;
    }
}
