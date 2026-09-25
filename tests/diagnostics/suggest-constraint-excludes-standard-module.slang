// Regression for issue #13140: the missing-constraint suggestion (note E30032) is limited to
// user-declared interfaces. A standard-library module is imported here (requiring
// `-experimental-feature`), and even though `slang.numerics` declares `sin` on
// `ITrigonometricFunctions`, no suggestion is offered for it — standard-library and core-module
// interfaces are deliberately excluded, so a builtin requirement name is never suggested as a
// constraint. Exhaustive mode enforces the absence: a leaked E30032 note would be unannotated and
// fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -experimental-feature

import slang.numerics;

float useStandardModuleMember<T>(T v)
{
    return v.sin();
//CHECK:     ^^^ member not found
//CHECK:     ^^^ 'sin' is not a member of 'T'.
}
