// Regression for issue #13140: the missing-constraint suggestion skips only the embedded core
// module, so interfaces from an explicitly imported module are offered, including a standard
// module such as `slang.numerics` (which needs `-experimental-feature`). Here `sin` is declared by
// `ITrigonometricFunctions`, and `where T : ITrigonometricFunctions` is valid advice.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -experimental-feature

import slang.numerics;

float useStandardModuleMember<T>(T v)
{
    return v.sin();
//CHECK:     ^^^ member not found
//CHECK:     ^^^ 'sin' is not a member of 'T'.
//CHECK:     ^^^ consider adding a constraint such as 'where T : ITrigonometricFunctions', since 'sin' is declared by that interface
}
