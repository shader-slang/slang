//TEST:SIMPLE(filecheck=CHECK):-target spirv

// Regression test for issue #7406: Optional<T> should implicitly coerce to
// Optional<U> when T is coercible to U in function return position.

interface IFoo
{
    int get();
}

struct FooImpl : IFoo
{
    int x;
    int get() { return x; }
}

// Concrete -> interface coercion in function return position (the main issue #7406 case)
Optional<IFoo> castToInterface(Optional<FooImpl> v)
{
    return v; // implicit coercion: Optional<FooImpl> -> Optional<IFoo>
}

// Generic version
__generic<T : IFoo>
Optional<IFoo> wrapGeneric(Optional<T> v)
{
    return v; // implicit coercion: Optional<T> -> Optional<IFoo>
}

// Numeric promotion in function return position
Optional<float> promoteToFloat(Optional<int> v)
{
    return v; // implicit coercion: Optional<int> -> Optional<float>
}

// CHECK-NOT: error[
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
