//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=`const` written as a type modifier in a typedef is rejected, and the aliased type still checks as the unmodified type.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes checkTypeModifier's ConstModifier branch. Modifiers on a type
// expression are turned into `Val`s that get attached as a ModifiedType; `const`
// has no such `Val`, so the branch diagnoses and returns null. The caller then
// finds an empty modifier list and falls back to typing the expression as the
// bare base type, which is why the alias is still usable below without any
// follow-on error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

typedef const int ConstInt;
//CHECK:^^^^^ E20018
//CHECK:^^^^^ cannot use 'const' as a type modifier

int test()
{
    ConstInt x = 1;
    return x;
}
