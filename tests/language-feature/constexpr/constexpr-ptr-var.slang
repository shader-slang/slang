// Test that `constexpr T*` (constexpr on a C-style pointer variable) emits
// E31227 (constexpr→const rewrite notice) followed by E20017 ('const' not
// allowed on C-style pointer). Both diagnostics fire: E31227 because constexpr
// is being rewritten, then E20017 because the rewritten `const T*` is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: constexpr on variable declarations is not a supported Slang feature
//CHECK: 'const' not allowed on C-style pointer declaration

[numthreads(1, 1, 1)]
void computeMain()
{
    constexpr int* p;
}
