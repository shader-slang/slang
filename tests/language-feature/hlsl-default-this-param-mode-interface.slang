//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/hlsl-default-this-param-mode-interface.slang -no-codegen

// Interface conformance compares effective receiver modes, not the spelling that selected them.
// HLSL's default therefore matches another default, while `[nonmutating]` and trailing `const`
// match in either direction.

interface IMutable
{
    void update();
};

struct Mutable : IMutable
{
    int value;

    void update()
    {
        value++;
    }
};

interface INonmutating
{
    [nonmutating]
    int read();
};

struct TrailingConst : INonmutating
{
    int value;

    int read() const
    {
        return value;
    }
};

interface ITrailingConst
{
    int read() const;
};

struct Nonmutating : ITrailingConst
{
    int value;

    [nonmutating]
    int read()
    {
        return value;
    }
};
