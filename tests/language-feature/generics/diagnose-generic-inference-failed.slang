//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// When generic argument inference fails because the call arguments
// don't match any viable specialization, error 39999 is emitted.
// The error is emitted twice (initial check + re-check phase).

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
//CHECK:          ^ could not specialize generic for arguments of type (int)
//CHECK:          ^ could not specialize generic for arguments of type (int)
}
