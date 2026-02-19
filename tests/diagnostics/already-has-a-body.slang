//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that function redefinition produces the expected error and note

int foo(){ /* original */ }
/*CHECK:
    ^ see previous definition of 'foo'
*/
int foo(){ /* redefinition */ }
/*CHECK:
    ^ function 'foo' already has a body
    ^ redeclared here
*/
