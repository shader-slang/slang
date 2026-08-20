//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=CHECK):-target spirv

// An empty-body macro. HandleDefineDirective always appends an EndOfFile sentinel, so
// macro->tokens.m_tokens.getCount() is 1 (not 0). Registration is skipped when the
// sentinel's loc is invalid, or proceeds but produces a size-1 range with no non-EOF
// body tokens to remap. Either way, no body locs are remapped into the expansion range,
// so no "expanded from macro 'EMPTY'" note is emitted for an error at the call site.
#define EMPTY

void test()
{
    EMPTY UndefinedType x;
}

//CHECK: undefined identifier 'UndefinedType'
// CHECK-NOT: expanded from macro 'EMPTY'
