//TEST:SIMPLE: -no-codegen

// Default-value expressions are checked while their containing callable is still transitioning to
// SignatureChecked. Lookup must use a small declaration-spelling bootstrap during that interval;
// querying checked effective-`this` information would be recursive, while treating `this` as
// immutable would reject these valid header expressions. This test is intentionally limited to
// header checking: materializing a default expression that mutates its receiver is a separate,
// pre-existing limitation of default-argument expansion.

struct HeaderLookup
{
    int value;

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
