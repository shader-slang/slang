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

// Error case 3: Generic parameter used in inheritance clause but not in target type
interface IBar<T> {}

// CHECK: ([[# @LINE+1]]): error 30855:
extension<T> float : IBar<T>
{
    void baz() {}
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