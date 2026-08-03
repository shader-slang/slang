//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

#define DECLARE_VAR UndefinedType x;

void test()
{
    DECLARE_VAR
}

// The error is reported at the token inside the macro body:
// CHECK: DECLARE_VAR
// CHECK: undefined identifier

// A single "expanded from" note should point back to the invocation site:
// CHECK: expanded from macro 'DECLARE_VAR'
// CHECK: single-level.slang:7
