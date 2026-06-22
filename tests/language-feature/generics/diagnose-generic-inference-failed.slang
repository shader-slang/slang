//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// When an ordinary generic parameter cannot be inferred from the call
// arguments, the focused "generic parameter could not be inferred" diagnostic
// is emitted (here `T` is only inferable from a `Wrapper<T>` argument, but the
// call passes a plain `int`).

struct Wrapper<T>
{
    T inner;
}

T unwrap<T>(Wrapper<T> w)
/*CHECK:
  ^^^^^^ see declaration of func unwrap<T> -> T
*/
{
    return w.inner;
}

void test()
{
    int x = unwrap(42);
//CHECK:          ^ generic parameter could not be inferred
//CHECK:          ^ could not infer generic argument for parameter 'T'
}
