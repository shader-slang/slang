// The optional `static_assert` message must be coercible to `String`. A passing condition isolates
// the message type-check so only the coercion failure is reported (see `visitStaticAssertDecl` in
// slang-check-decl.cpp, which coerces the message to `String` regardless of the condition value).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

[numthreads(1, 1, 1)]
void computeMain()
{
    static_assert(true, 42);
    //CHECK: E30019
    //CHECK: expected an expression of type 'String', got 'int'
}
