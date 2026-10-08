//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-trailing-this-const-requires-this.slang -no-codegen

// Trailing `const` is an HLSL receiver-mode spelling that is valid only on a non-static member
// function. It makes the receiver immutable when used on a generic member function.

void freeFunction() const
/*CHECK:
                    ^^^^^ receiver-mode specifier requires an instance receiver
                    ^^^^^ receiver-mode specifier 'const' is only allowed on an instance member that has a `this` parameter.
*/
{}

struct Value
{
    int value;

    static void staticMethod() const
    /*CHECK:
                               ^^^^^ receiver-mode specifier requires an instance receiver
                               ^^^^^ receiver-mode specifier 'const' is only allowed on an instance member that has a `this` parameter.
    */
    {}

    void reset<T>() const
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }
};
