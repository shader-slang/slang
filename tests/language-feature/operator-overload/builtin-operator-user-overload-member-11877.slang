// Member/`extension` operator coverage for https://github.com/shader-slang/slang/issues/11877
//
// `checkOverloadedBuiltinOperatorDecl` (slang-check-decl.cpp) counts an instance-method operator's
// implicit `this` receiver as an operand (via `calcThisType`), so the rejection depends on the
// receiver type as well as the explicit parameters. This test pins that receiver logic, which the
// other tests (all free functions) do not exercise:
//
//   - A member `operator*` on a *user* struct (`this` is `Matrixf`, a user type) has a non-builtin
//     operand, so the fast path never shadows it and it must remain legal.
//   - An `extension` `operator*` on the *builtin* `float4x4` (`this` is `float4x4`, and the explicit
//     operand is `float4x4`) is all-builtin, so the fast path would shadow it and it must be
//     rejected with E30073 -- exactly as a free `operator*(float4x4, float4x4)` is.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Member operator on a user type: `this` (Matrixf) is not a builtin type, so this is honored, NOT
// rejected. (No E30073 for `Matrixf::operator*`.)
struct Matrixf
{
    float4x4 m;
    Matrixf operator*(Matrixf rhs) { return rhs; }
}

// Extension member operator whose receiver AND parameter are the builtin `float4x4`: all-builtin, so
// the builtin fast path shadows it and it is rejected, just like the free-function form.
extension float4x4
{
    float4x4 operator*(float4x4 rhs) { return rhs; }
}
//CHECK: cannot overload builtin operator '*'
