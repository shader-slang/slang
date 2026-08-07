//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

#define INNER UndefinedType x;
#define OUTER INNER

void test()
{
    OUTER
}

// Error inside INNER's body:
//CHECK: undefined identifier 'UndefinedType'.

// First note: INNER was expanded inside OUTER's definition:
//CHECK: expanded from macro 'INNER'

// Second note: OUTER was expanded at the call site:
//CHECK: expanded from macro 'OUTER'
