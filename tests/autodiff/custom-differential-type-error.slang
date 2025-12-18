//TEST:SIMPLE(filecheck=CHECK):-target spirv

interface IFoo : IDifferentiable
{
    float myFunc();
}

struct Bar: IDifferentiable
{
    float y;
}

struct Foo :  IFoo
{
    typealias Differential = Bar;

    // CHECK: ([[# @LINE+1]]): error 30027: 'x' is not a member of 'typeof(Foo.Differential)'.
    [DerivativeMember(Differential.x)]
    float x;

    [BackwardDifferentiable]
    float myFunc()
    {
        return x * x;
    };
}
