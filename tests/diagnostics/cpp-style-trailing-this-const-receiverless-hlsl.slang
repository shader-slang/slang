//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-receiverless-hlsl.slang -no-codegen

// The parser records trailing `const` on every `FuncDecl`. Effective-receiver computation during
// signature checking then rejects it on a free function and on a static method wrapped in a
// `GenericDecl` because neither has an instance receiver.

void freeFunction() const
/*CHECK:
                    ^^^^^ receiver-mode specifier requires an instance receiver
                    ^^^^^ receiver-mode specifier 'const' is only allowed on an instance member that has a `this` parameter.
*/
{}

struct Value
{
    static void genericStatic<T>() const
    /*CHECK:
                                   ^^^^^ receiver-mode specifier requires an instance receiver
                                   ^^^^^ receiver-mode specifier 'const' is only allowed on an instance member that has a `this` parameter.
    */
    {}
};
