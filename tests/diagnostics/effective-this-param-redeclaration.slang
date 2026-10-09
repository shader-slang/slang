//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/effective-this-param-redeclaration.slang -no-codegen

// Receiver mode is not part of overload identity, but every declaration of one method must agree
// on the effective receiver parameter used by calls and lowering.

struct Value
{
    void constThenDefault() const;
//CHECK: ^^^^^^^^^^^^^^^^ see previous declaration of 'constThenDefault'
    void constThenDefault() {}
//CHECK: ^^^^^^^^^^^^^^^^ function receiver mismatch
//CHECK: ^^^^^^^^^^^^^^^^ function 'constThenDefault' has receiver type 'Value' with parameter-passing mode 'inout', but its previous declaration has receiver type 'Value' with parameter-passing mode 'in'

    void defaultThenConst();
//CHECK: ^^^^^^^^^^^^^^^^ see previous declaration of 'defaultThenConst'
    void defaultThenConst() const {}
//CHECK: ^^^^^^^^^^^^^^^^ function receiver mismatch
//CHECK: ^^^^^^^^^^^^^^^^ function 'defaultThenConst' has receiver type 'Value' with parameter-passing mode 'in', but its previous declaration has receiver type 'Value' with parameter-passing mode 'inout'

    [nonmutating]
    void nonmutatingThenMutating();
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^ see previous declaration of 'nonmutatingThenMutating'
    [mutating]
    void nonmutatingThenMutating() {}
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^ function receiver mismatch
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^ function 'nonmutatingThenMutating' has receiver type 'Value' with parameter-passing mode 'inout', but its previous declaration has receiver type 'Value' with parameter-passing mode 'in'

    static void staticThenInstance();
//CHECK:        ^^^^^^^^^^^^^^^^^^ see previous declaration of 'staticThenInstance'
    void staticThenInstance() {}
//CHECK: ^^^^^^^^^^^^^^^^^^ function receiver mismatch
//CHECK: ^^^^^^^^^^^^^^^^^^ function 'staticThenInstance' and its previous declaration disagree on whether they have an instance receiver

    void instanceThenStatic();
//CHECK: ^^^^^^^^^^^^^^^^^^ see previous declaration of 'instanceThenStatic'
    static void instanceThenStatic() {}
//CHECK:        ^^^^^^^^^^^^^^^^^^ function receiver mismatch
//CHECK:        ^^^^^^^^^^^^^^^^^^ function 'instanceThenStatic' and its previous declaration disagree on whether they have an instance receiver
};
