//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
// Default values on `extension` generic parameters have no effect — the
// parameters are inferred when the extension is applied, never supplied
// as arguments — so the compiler must reject them.

struct Foo<T> {}
struct Pair<A, B> {}
struct Holder<int N> {}

interface IFoo {}
struct ImplFoo : IFoo {}

// Error case 1: type parameter has a default value.
extension<T = int> Foo<T>
//CHECK:  ^ default value
{
}

// Error case 2: only the parameter with a default reports an error.
extension<X, Y = float> Pair<X, Y>
//CHECK:     ^ default value
{
}

// Error case 3: value-generic parameter has a default value.
extension<int M = 4> Holder<M>
//CHECK:      ^ default value
{
}

// Error case 4: dual-fire — when a param has BOTH a default AND is
// unreferenced, both 30857 (default) and 30855 (unreferenced) must fire;
// neither should suppress the other.
extension<T = int> int
//CHECK:  ^ E30857
//CHECK:  ^ E30855
{
}

// Error case 5: constraint-plus-default form. The default is still
// meaningless when the param has a `: IFoo` constraint.
extension<T : IFoo = ImplFoo> Foo<T>
//CHECK:  ^ default value
{
}
