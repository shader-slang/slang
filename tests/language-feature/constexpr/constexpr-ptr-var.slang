// Test that `constexpr T*` (constexpr on a C-style pointer variable) is
// rejected with E20017. The PointerTypeExpr check in checkModifier fires
// before the constexpr→const rewrite, so E31227 is not emitted — only
// E20017 fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: 'const' not allowed on C-style pointer declaration

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int* p;
}
