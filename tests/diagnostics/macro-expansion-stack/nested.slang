//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

#define INNER UndefinedType x;
#define OUTER INNER

void test()
{
    OUTER
}

// Error inside INNER's body:
// CHECK: INNER
// CHECK: undefined identifier

// First note: INNER was expanded inside OUTER's definition:
// CHECK: expanded from macro 'INNER'
// CHECK: OUTER

// Second note: OUTER was expanded at the call site:
// CHECK: expanded from macro 'OUTER'
// CHECK: nested.slang:8
