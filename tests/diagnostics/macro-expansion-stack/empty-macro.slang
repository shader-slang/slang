//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// An empty-body macro: no per-invocation expansion view is created because
// macro->tokens.m_tokens.getCount() == 0. An error at the call site of EMPTY
// should produce a clean diagnostic with NO "expanded from macro 'EMPTY'" note.
#define EMPTY

void test()
{
    EMPTY UndefinedType x;
}

// CHECK: undefined identifier 'UndefinedType'
// CHECK-NOT: expanded from macro 'EMPTY'
