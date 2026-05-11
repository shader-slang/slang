//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):


#define SOME +
#define THING +

#define A SOME
#define B THING

#define PASTE2(x, y) x##y
#define PASTE(x, y) PASTE2(x, y)
//CHECK:            ^^^^^^ expanded from macro 'PASTE2'


PASTE(A, B)
//CHECK: unexpected token
//CHECK: unexpected '++', expected identifier
//CHECK: see token pasted location
/*CHECK:
^^^^^ expanded from macro 'PASTE'
*/
