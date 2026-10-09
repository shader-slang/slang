//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/effective-this-param-header-lookup-hlsl.slang -no-codegen

// Consider `int[this.elementCount]` in `arrayParameter` below. Lookup runs while that method's
// signature is still in progress and records only that the member base is a `this` value. When
// expression construction rebuilds the member access, it asks the shared effective-receiver
// routine for `this`'s type and HLSL-default `inout` mode. That routine can compute the answer
// without recursively trying to finish the same signature. The signature check later publishes
// the result and emits any receiver-mode modifier diagnostics.

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
