//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
// Default values on `extension` generic parameters are meaningless (those
// parameters are always inferred from the target type's type arguments),
// so the compiler must reject them.

struct Foo<T> {}
struct Pair<A, B> {}
struct Holder<int N> {}

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
