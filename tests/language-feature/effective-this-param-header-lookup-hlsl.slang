//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/effective-this-param-header-lookup-hlsl.slang -no-codegen

// A member function's receiver mode must be established before its parameter type expressions are
// checked. An explicit `this` and the implicit `this` used by an unqualified member reference in
// parameter types, defaults, and the body must all use that same mode.

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

    void mutatingBody()
    {
        value = mutateAndReturn();
    }

    int invokeLambda()
    {
        // A compiler-synthesized lambda call operator uses `in` rather than inheriting the HLSL
        // default for source member functions, so an immutable closure value can invoke it.
        let read = () => value;
        return read();
    }
};
