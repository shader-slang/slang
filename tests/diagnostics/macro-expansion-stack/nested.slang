//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=CHECK):-target spirv

#define INNER UndefinedType x;
#define OUTER INNER

void test()
{
    OUTER
}

// Error inside INNER's body, at the definition of INNER (line 4, col 15
// in the definition file).  The CHECK pinning :4:15 verifies that the
// loc-remapping arithmetic recovers the correct definition-file position.
// CHECK: :4:15
//CHECK: undefined identifier 'UndefinedType'.

// First note: INNER was expanded inside OUTER's definition (line 5, col 15
// = the "INNER" token in "#define OUTER INNER").  This pinning verifies
// that the macro expansion chain walks back one level correctly.
//CHECK: expanded from macro 'INNER'
// CHECK: :5:15

// Second note: OUTER was expanded at the call site (line 9, col 5).
// This pinning verifies the second level of chain-walking.
//CHECK: expanded from macro 'OUTER'
// CHECK: :9:5
