//TEST:INTERPRET(filecheck=CHECK):

struct Wrapper<let N : uint>
{
    int value;
}

interface IWrapperRequirement
{
    static const uint kWidth;

    Wrapper<kWidth> make();
}

struct WrapperRequirementImpl<let TWidth : uint> : IWrapperRequirement
{
    static const uint kWidth = TWidth;

    Wrapper<kWidth> make()
    {
        Wrapper<kWidth> result = { int(kWidth) };
        return result;
    }
}

void main()
{
    WrapperRequirementImpl<7> value = {};
    let result = value.make();
    printf("%d\n", result.value);
    // CHECK: 7
}
