//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An extension with a generic parameter that is not referenced by the
// target type should produce error 30855.

struct Foo<T> { T value; }

extension<T, U> Foo<T>
//CHECK:     ^ generic parameter 'U' is not referenced by extension target type 'Foo<T>'
//CHECK:     ^ generic parameter 'U' is not referenced by extension target type 'Foo<T>'
{
    U getExtra() { U result; return result; }
}
