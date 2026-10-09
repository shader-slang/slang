//TEST:SIMPLE: -no-codegen

// Consider `int[this.elementCount]` in `arrayParameter` below. Lookup runs while that method's
// signature is still in progress and records only that the member base is a `this` value. When
// expression construction rebuilds the member access, it asks the shared effective-receiver
// routine for `this`'s type and mode. That routine can compute the answer without recursively
// trying to finish the same signature. The signature check later publishes the result and emits
// any receiver-mode modifier diagnostics.

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
