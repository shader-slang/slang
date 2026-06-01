// Regression test for shader-slang/slang#11409.
//
// A generic type specialized with its own enclosing struct (`Foo<Bar>` inside
// `struct Bar`) is legal when the generic only stores the type argument behind a
// pointer (`Foo<T> { T* t; }`), so Bar's storage size is finite. When Bar is
// additionally used through an interface (dynamic dispatch via `ThirdThing<Bar>`
// with `Bar : IQuux`), the witness table triggers the interface-dependency walk
// in any-value-inference.
//
// Before the fix, that walk (`_findDependenciesOfTypeInSet`) had no visited-set
// guard, so it recursed forever through the unspecialized `IRSpecialize(Foo, Bar)`
// type-argument operand (Bar -> Foo<Bar> -> Bar -> ...) and overflowed the stack,
// crashing slangc. The program is valid and must compile cleanly.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct Foo<T>
{
    T* t;
}

interface IQuux
{
    void discombobulate();
}

struct Bar : IQuux
{
    Foo<Bar> b;
    void discombobulate() { printf("%p\n", b.t); }
}

struct ThirdThing<T : IQuux>
{
    T t;
    void dosomething() { t.discombobulate(); }
}

// CHECK-NOT: error[E
[numthreads(1, 1, 1)]
void main()
{
    ThirdThing<Bar> c;
    c.dosomething();
}
