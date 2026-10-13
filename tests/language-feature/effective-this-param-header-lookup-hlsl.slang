//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/effective-this-param-header-lookup-hlsl.slang -no-codegen

// Consider `int[this.elementCount]` in `arrayParameter` below. Signature checking publishes the
// HLSL-default `inout` receiver before checking the array parameter, so semantic checking can
// resolve the explicit `this` without recursively restarting the signature. The unqualified
// `mutateAndReturn()` calls in default arguments exercise the same ordering through lookup:
// `_lookUpInScopes()` appends a `ThisValue` breadcrumb, and `ConstructLookupResultExpr()` uses the
// one receiver description already published for the method.

struct HeaderLookup
{
    static const int elementCount = 4;
    int value;

    int mutateAndReturn()
    {
        value++;
        return value;
    }

    void arrayParameter(int[this.elementCount] values)
    {
        value = values[0];
    }

    void defaultParameter(int replacement = mutateAndReturn())
    {
        value = replacement;
    }

    int genericInstanceDefault<T>(T unused, int replacement = mutateAndReturn())
    {
        value = replacement;
        return replacement;
    }

    static int genericStaticDefault<T>(T unused, int count = elementCount)
    {
        return count;
    }

    int invokeLambda()
    {
        // A compiler-synthesized lambda call operator uses a read-only receiver rather than the
        // default for methods written in HLSL.
        let read = () => value;
        return read();
    }
};
