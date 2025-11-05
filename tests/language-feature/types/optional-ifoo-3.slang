//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    T get_result<T:__BuiltinArithmeticType>();
}

struct FooImpl : IFoo
{
    T get_result<T:__BuiltinArithmeticType>() { return (T)data; }
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
    // CHECK-NEXT: result: 100.0
    let result_foo = generate_foo(100);
    printf("hasValue: %d\n", (int)result_foo.hasValue);
    printf("result: %f\n", (float)result_foo.value.get_result<float>());
}
