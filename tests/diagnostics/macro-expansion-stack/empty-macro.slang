//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//TEST:SIMPLE(filecheck=CHECK):-target spirv

// An empty-body macro. HandleDefineDirective always appends an EndOfFile sentinel, so
// macro->tokens.m_tokens.getCount() is 1 (not 0), and for a #define with no replacement
// tokens like this one, that sentinel's loc is valid (it points just past `#define EMPTY`
// in this file). So registration proceeds and produces a size-1 expansion range -- but the
// body-token scan in MacroInvocation's constructor only remaps non-EOF tokens, and there are
// none here, so no body locs are ever remapped into that range. No "expanded from macro
// 'EMPTY'" note is emitted for an error at the call site.
#define EMPTY

void test()
{
    EMPTY UndefinedType x;
}

//CHECK: undefined identifier 'UndefinedType'
// CHECK-NOT: expanded from macro 'EMPTY'
