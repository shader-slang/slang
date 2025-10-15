//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    int get_result();
}

struct FooImpl : IFoo
{
    int get_result() { return data; }
    int data;
}

Optional<IFoo> generate_foo(int i)
{
    if (i % 5 == 0)
    {
        FooImpl result = {};
        result.data = i;
        return { result };
    }
    else
    {
        return {};
    }
}

void main()
{
    // CHECK: hasValue: 1
    let result_foo = generate_foo(100);
    printf("hasValue: %d\n", (int)result_foo.hasValue);
}
