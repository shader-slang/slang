//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
// Test unreferenced generic parameters in extensions

// Error case 1: Simple unreferenced generic parameter
extension<T> int
//CHECK:  ^ generic parameter 'T' is not referenced by extension target type 'int'
{
    void foo() {}
}

// Error case 2: Multiple generic parameters, some unused
extension<T, U> vector<T, 3>
//CHECK:     ^ generic parameter 'U' is not referenced by extension target type 'vector<T,3>'
{
    void bar() {}
}

// Error case 3: Parameters only used in inheritance declaration should be errors
interface IFooGeneric<S> {}
struct MyType {}

extension<S> MyType : IFooGeneric<S>
//CHECK:  ^ generic parameter 'S' is not referenced by extension target type 'MyType'
{
    void shouldFail() {}
}
