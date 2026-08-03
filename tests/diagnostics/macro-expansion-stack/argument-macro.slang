//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// A macro invocation passed as the argument of another function-like macro.
// INNER's body has the error and INNER is invoked as an argument to WRAP.
// The chain should be:
//   error in INNER's body
//   -> expanded from macro 'INNER' (INNER invoked inside WRAP's argument)
//   -> expanded from macro 'WRAP'  (WRAP invoked at the call site)
#define INNER UndefinedType x;
#define WRAP(y) y

void test()
{
    WRAP(INNER)
}

// CHECK: undefined identifier
// CHECK: expanded from macro 'INNER'
// CHECK: expanded from macro 'WRAP'
