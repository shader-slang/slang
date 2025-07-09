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

// Valid case 3: Generic parameter used in inheritance constraint
interface IFooGeneric<S> {}
struct MyType {}

// Error: T is not used, but S is validly used in inheritance constraint
// CHECK: ([[# @LINE+1]]): error 30855:
extension<T, S> MyType : IFooGeneric<S>
{
    void test() {}
}

// Valid case 4: Both parameters used properly
extension<S> MyType : IFooGeneric<S>
{
    void testValid() {}
}