//TEST:SIMPLE(filecheck=CHECK): -target spirv
// Test unreferenced generic parameters in extensions

// Error case 1: Simple unreferenced generic parameter
// CHECK: ([[# @LINE+1]]): error 30855:
extension<T> int
{
    void foo() {}
}

// Error case 2: Multiple generic parameters, some unused
// CHECK: ([[# @LINE+1]]): error 30855:
extension<T, U> vector<T, 3>
{
    void bar() {}
}

// Error case 3: Parameters only used in inheritance declaration should be errors
interface IFooGeneric<S> {}
struct MyType {}

// CHECK: ([[# @LINE+1]]): error 30855:
extension<S> MyType : IFooGeneric<S>
{
    void shouldFail() {}
}
