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

// Error case 3: Generic parameter used in inheritance clause but not in target type or constraints
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

// Valid case 3a: Generic parameter used in constraint (should be allowed)
interface IFooGeneric<S> {}

extension<T:IFooGeneric<S>, S> T
{
    S getFirst()
    {
        return default(S);
    }
}

// Valid case 3b: Different syntax - inheritance in extension
struct MyType {}

extension<T, S> MyType : IFooGeneric<S>
{
    void test() {}
}