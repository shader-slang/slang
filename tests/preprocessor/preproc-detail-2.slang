//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-E
//TEST:SIMPLE(filecheck=CHECK):-E

// Macro parameters must have unique names

#define A(x, x) a x b x c
//diag:      ^ duplicate parameter
//diag:      ^ redefinition of macro parameter 'x'

A(,)

// CHECK: a b c
