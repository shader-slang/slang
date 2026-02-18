//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,diag=diag):-E

// Macro parameters must have unique names

#define A(x, x) a x b x c
//diag:      ^ duplicate parameter
//diag:      ^ redefinition of macro parameter 'x'

A(,)

// CHECK: a b c
