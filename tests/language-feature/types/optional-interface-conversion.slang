//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

interface IFoo
{
    int getValue();
}

interface IFooUser
{
    associatedtype FooConcrete : IFoo;

    Optional<FooConcrete> make_foo();
}

Optional<IFoo> foo_operator<FooUser : IFooUser>(FooUser foo_user)
{
    return foo_user.make_foo();  // Should convert Optional<FooConcrete> to Optional<IFoo>
}

struct FooImpl : IFoo
{
    int getValue() { return 42; }
}

Optional<FooImpl> make_optional(bool make)
{
    if (make)
        return FooImpl();
    return none;
}

Optional<IFoo> make_optional2(bool make)
{
    return make_optional(make); // Should convert Optional<FooImpl> to Optional<IFoo>
}

// Test case with concrete types
struct ConcreteUser : IFooUser
{
    typedef FooImpl FooConcrete;

    Optional<FooImpl> make_foo()
    {
        return FooImpl();
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ConcreteUser user;
    Optional<IFoo> result1 = foo_operator(user);
    Optional<IFoo> result2 = make_optional2(true);

    // Just test that the conversions compile and the optionals exist
    bool hasValue1 = result1.hasValue;
    bool hasValue2 = result2.hasValue;
}

// CHECK: OpReturn