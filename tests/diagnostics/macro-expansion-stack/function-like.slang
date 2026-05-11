//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Case 1: error is in the macro body, not the argument.
// The "expanded from" note should point back to the call site.
#define WRAP(x) UndefinedBodyType y; x

void test()
{
    WRAP(1)
}

// CHECK: UndefinedBodyType
// CHECK: undefined identifier
// CHECK: expanded from macro 'WRAP'
// CHECK: function-like.slang:9

// Case 2: error is in an argument at the call site.
// Argument tokens retain their call-site locs and are NOT remapped into
// the expansion view, so the error should point directly to the call site
// with no "expanded from" note.
#define PASS_THRU(x) x

void test2()
{
    PASS_THRU(UndefinedArgType z)
//diag:      ^ undefined identifier
//diag:      ^ undefined identifier 'UndefinedArgType'.
}
