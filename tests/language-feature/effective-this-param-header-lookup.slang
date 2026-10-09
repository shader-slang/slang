//TEST:SIMPLE: -no-codegen

// A callable's receiver mode must be established before its parameter type expressions are checked.
// An explicit `this` and the implicit `this` used by an unqualified member reference in parameter
// types, defaults, and the body must all use that same mode.

struct HeaderLookup
{
    static const int elementCount = 4;
    int value;

    // The explicit `this` in this parameter type must already observe the method's `[mutating]`
    // mode.
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
    void mutatingBody()
    {
        // The unqualified call below must use the containing method's writable receiver.
        value = mutateAndReturn();
    }
}
