//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=CHECK):-target spirv

// A macro invocation passed as the argument of another function-like macro.
// INNER's body has the error and INNER is invoked as an argument to WRAP.
// Because argument tokens are macro-expanded before substitution, the error
// loc sits in INNER's expansion range; the note chain reports INNER's call
// site (inside WRAP's argument list) but does not chain further through WRAP
// since the substituted tokens are not remapped into WRAP's body range.
#define INNER UndefinedType x;
#define WRAP(y) y

void test()
{
    WRAP(INNER)
}

//CHECK: undefined identifier 'UndefinedType'.
//CHECK: expanded from macro 'INNER'
// CHECK-NOT: expanded from macro 'WRAP'
