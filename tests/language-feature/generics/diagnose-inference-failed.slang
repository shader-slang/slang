// Test that the compiler emits an error when it cannot infer generic type
// arguments from the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

T convert<T>(int val)
{
    T result;
    return result;
}

void test()
{
    // The generic parameter T cannot be inferred from the arguments
    // because T only appears as a return type, not in the parameter list.
    let x = convert(42);
//CHECK:           ^ could not specialize generic for arguments of type (int)
}
