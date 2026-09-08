//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,diag=diag):-E

#define CONCAT(a, b) a ## b
//diag:                ^^ token paste failure
//diag:                ^^ token pasting with '##' resulted in the invalid token ')CONCAT'

#define A a
#define B b

#define A2 A
#define B2 B

// Gives error (as trys to concat unexpanded input)
// <source>:11:1: error: pasting formed ')CONCAT', an invalid preprocessing token
//
// Old Slang output: aabb ;

CONCAT(CONCAT(A2, A2), CONCAT(B2, B2));
/*diag:
^^^^^^ expanded from macro 'CONCAT'
*/

// CHECK: CONCAT ( a , a ) CONCAT ( b , b )
