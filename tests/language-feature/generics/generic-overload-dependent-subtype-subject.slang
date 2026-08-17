//TEST:INTERPRET(filecheck=CHECK):

interface ITag
{
}

struct Tag : ITag
{
}

struct Wrapper<T>
{
    typealias Assoc = T;
}

int pick<T>(T value)
{
    return 1;
}

int pick<T>(T value)
    where Wrapper<T>.Assoc : ITag
{
    return 2;
}

void main()
{
    Tag value;
    printf("%d\n", pick(value));
    // CHECK: 2
}
