//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=CHECK):-target spirv

#define INNER UndefinedType x;
#define OUTER INNER

void test()
{
    OUTER
}

// Error is in INNER's body (line 4, col 15 in this file).  The two "expanded from"
// notes walk back through INNER's expansion inside OUTER's body (line 5, col 15)
// and then OUTER's call site (line 9, col 5).  Together these verify that the
// macro expansion chain is walked correctly.
//CHECK: undefined identifier 'UndefinedType'.
//CHECK: expanded from macro 'INNER'
//CHECK: expanded from macro 'OUTER'
