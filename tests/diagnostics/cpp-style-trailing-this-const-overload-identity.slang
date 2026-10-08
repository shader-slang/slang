//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-overload-identity.slang -no-codegen

// Trailing `const` selects the effective receiver mode; it does not participate in overload
// identity. These declarations therefore have the same signature.

struct Value
{
    void method() {}
    /*CHECK:
         ^^^^^^ see previous declaration of 'method'
    */

    void method() const {}
    /*CHECK:
         ^^^^^^ function receiver mismatch
         ^^^^^^ function 'method' has receiver type 'Value' with parameter-passing mode 'in', but its previous declaration has receiver type 'Value' with parameter-passing mode 'inout'
    */
};
