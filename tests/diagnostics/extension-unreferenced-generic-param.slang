//TEST:SIMPLE(filecheck=CHECK): -target spirv
// Test unreferenced generic parameters in extensions

// Error case 1: Simple unreferenced generic parameter
// CHECK: error 30855:
extension<T> int
{
    void foo() {}
}

// Error case 2: Multiple generic parameters, some unused
// CHECK: error 30855:
extension<T, U> vector<T, 3>
{
    void bar() {}
}

// Valid case 1: Generic parameter used in target type
extension<T> vector<T, 4>
{
    void validMethod() {}
}

// Valid case 2: Extension extending the generic parameter itself
interface IFoo {}

extension<T:IFoo> T
{
    void validSelfExtension() {}
}

// Error case 3: Parameters only used in inheritance declaration should be errors
interface IFooGeneric<S> {}
struct MyType {}

// Both T and S should be errors - inheritance declarations don't count as references
// CHECK: error 30855:
// CHECK: error 30855:
extension<T, S> MyType : IFooGeneric<S>
{
    void test() {}
}

// Error case 4: Even single parameter only in inheritance should be an error
// CHECK: error 30855:
extension<S> MyType : IFooGeneric<S>
{
    void shouldFail() {}
}

// Valid case 3: Parameter used in target type (not just inheritance)
extension<S> vector<S, 3> : IFooGeneric<S>
{
    void testValid() {}
}

// Error case 5: Parameter only used in extension inheritance should still be an error
interface IBar<T> {}
struct TestStruct {}

// Both T and U should be errors since they're only in inheritance declaration
// CHECK: error 30855:
// CHECK: error 30855:
extension<T, U> TestStruct : IBar<T>
{
    void shouldFail() {}
}