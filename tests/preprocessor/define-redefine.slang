//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Confirm that we handle redefinition
// of an existing define (with a diagnostic)

#define FOO 1.0f
//diag: ^^^ see previous definition of 'FOO'

float foo() { return FOO + 2.0; }

#define FOO 2.0f
//diag: ^^^ macro redefined
//diag: ^^^ redefinition of macro 'FOO'
