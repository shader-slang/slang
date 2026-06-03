//TEST:INTERPRET(filecheck=CHECK):

interface IStaticSizedArrayMethod
{
    static const uint kInputWidth;
    static const uint kOutputWidth;

    void eval(const int input[kInputWidth], out int output[kOutputWidth]);
}

struct StaticSizedArrayImpl<let TInputWidth : uint, let TOutputWidth : uint>
    : IStaticSizedArrayMethod
{
    static const uint kInputWidth = TInputWidth;
    static const uint kOutputWidth = TOutputWidth;

    void eval(const int input[kInputWidth], out int output[kOutputWidth])
    {
        for (uint i = 0; i < kOutputWidth; ++i)
        {
            output[i] = input[0] + int(i);
        }
    }
}

void main()
{
    StaticSizedArrayImpl<2, 3> value = {};
    int input[2] = {10, 20};
    int output[3] = {};
    value.eval(input, output);
    printf("%d %d %d\n", output[0], output[1], output[2]);
    // CHECK: 10 11 12
}
