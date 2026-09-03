//TEST:INTERPRET(filecheck=CHECK):
// Regression test for https://github.com/shader-slang/slang/issues/9810

struct Foo
{
    int Bar<int N>()
    {
        return N;
    }
};

void main()
{
    Foo f;
    Foo* foo = &f;

    // A generic member call through a pointer must compile (no error 30101)
    // AND return the correct value.
    //CHECK: 42
    printf("%d\n", foo->Bar<42>());
}
