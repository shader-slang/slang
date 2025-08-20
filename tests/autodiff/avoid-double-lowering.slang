//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
// CHECK-NOT: an internal error threw
// CHECK-NOT: error:

interface IFoo<T, int N>: IDifferentiable
    where T : __BuiltinFloatingPointType
{
    This myFunc(This other);
}

struct Foo<T, int N>
    where T : __BuiltinFloatingPointType
{
    [BackwardDifferentiable]
    This myFunc(This other)
    {
        [MaxIters(N)]
        for (int i = 0; i < N; i++) {}

        return this;
    }
}

extension <T, int N> Foo<T, N> : IFoo<T, N>
    where T : __BuiltinFloatingPointType {}

