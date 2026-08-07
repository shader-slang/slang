//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=NOCHECK):-target spirv -enable-machine-readable-diagnostics

// An empty-body macro: no per-invocation expansion view is created because
// macro->tokens.m_tokens.getCount() == 0. An error at the call site of EMPTY
// should produce a clean diagnostic with NO "expanded from macro 'EMPTY'" note.
#define EMPTY

void test()
{
    EMPTY UndefinedType x;
}

//CHECK: undefined identifier 'UndefinedType'
// NOCHECK-NOT: expanded from macro 'EMPTY'
