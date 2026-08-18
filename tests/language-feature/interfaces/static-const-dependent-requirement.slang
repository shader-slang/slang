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

__generic<T : IWrapperRequirement>
int readIndirect(T value)
{
    let result = value.make();
    return result.value;
}

void main()
{
    WrapperRequirementImpl<7> value = {};
    let result = value.make();
    printf("%d\n", result.value);
    // CHECK: 7

    printf("%d\n", readIndirect(value));
    // CHECK: 7
}
