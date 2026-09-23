//TEST:SIMPLE: -no-codegen -Wno-38109

// The generic method records T as its effective parameter type. After substitution, a
// non-copyable specialization must adjust the original In mode to BorrowIn.

interface IReadable
{
    int read();
}

struct CopyableValue : IReadable
{
    int value;

    int read()
    {
        return value;
    }
}

[__NonCopyableType]
struct NonCopyableValue : IReadable
{
    int value;

    int read()
    {
        return value;
    }
}

extension<T : IReadable> T
{
    int readTwice()
    {
        return this.read() * 2;
    }
}

void exerciseSpecializations()
{
    CopyableValue copyable = {2};
    NonCopyableValue nonCopyable;
    nonCopyable.value = 3;

    int a = copyable.readTwice();
    int b = nonCopyable.readTwice();
}
