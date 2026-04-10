// x-macro-line-continuation.slang
//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):

// Tests line continuations on diagnostic output of macros
// Using non-exhaustive because the macro error causes cascading parse errors

#define X(M) \
    M(0) \
    M(1) \
    M(2) \
    M(3) \
    M(4, 4) \
/*diag:
     ^ wrong macro argument count
     ^ wrong number of arguments to macro (expected 1, got 2)
*/
    M(5) \
    M(6) \
    M(7)

#define A(x) + x + x + x

int sum()
{
	return X(A);
}
