//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// This test invokes a body-erroring macro many times to verify that the
// O(1)-per-invocation side-table approach (registerMacroExpansion + binary
// search in findMacroExpansion) works at scale. In the original design before
// this PR, each invocation allocated a SourceFile/SourceView, causing O(N)
// memory growth that led to performance regressions for large translation units.
//
// The test asserts that compilation still reports the correct "expanded from
// macro" note regardless of how many times the macro is invoked — if a future
// change reintroduces per-invocation SourceView allocation and causes memory or
// performance problems, this test would catch functional regressions too.

#define BODY_ERR UndefinedType x;

void test()
{
    // 50 invocations — enough to confirm the side-table path handles repeated
    // invocations without loss of correctness.
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
    BODY_ERR BODY_ERR BODY_ERR BODY_ERR BODY_ERR
}

// Each invocation reports an error with an "expanded from macro 'BODY_ERR'" note.
//CHECK: undefined identifier 'UndefinedType'.
//CHECK: expanded from macro 'BODY_ERR'
