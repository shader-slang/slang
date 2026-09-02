//TEST:SIMPLE:

struct Foo
{
    int Bar<int N>()
    {
        return N;
    }
};

void test()
{
    Foo f;
    Foo* foo = &f;

    // Verify calling a generic member function via a pointer succeeds
    // without emitting error 30101 (cannot dereference type 'Foo').
    int result = foo->Bar<42>();
}
