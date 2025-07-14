//TEST:INTERPRET(filecheck=CHECK):
interface IBar
{}

interface IFoo : IBar
{
    void execute();
}

struct Impl : IFoo
{
    void execute()
    {
        printf("Impl::execute();\n");
    }
}

extension<T:IBar> T : IFoo
{
    void execute()
    {
        printf("Extension::execute();\n");
    }
}

struct Base : IBar{}

void test<T:IFoo>(T t)
{
    t.execute();
}

void main()
{
    // CHECK: Impl::execute();
    Impl f;
    test(f);
    // CHECK: Extension::execute();
    Base b;
    test(b);
}