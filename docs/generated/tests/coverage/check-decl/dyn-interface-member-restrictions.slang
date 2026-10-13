//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the member restrictions a `dyn interface` is subject to under -std 2026 (no associated types, generic methods, [mutating] or [Differentiable] methods).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `validateDynInterfaceUsage` only runs when the module is
// language version 2026-or-later and experimental dynamic dispatch was not
// requested, so this test compiles with `-std 2026`. Each member below trips a
// distinct arm of that member loop; the annotations pin the observed code and
// message for every arm. The diagnostic test is exhaustive, so it also pins
// that the trailing plain method produces no diagnostic at all.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026 -target hlsl

dyn interface IFoo
{
        associatedtype T;
//CHECK:               ^ E33073
//CHECK:               ^ dyn interfaces cannot have associatedType members.

        int generic<U>(U u);
//CHECK:    ^^^^^^^ E33074
//CHECK:    ^^^^^^^ dyn interfaces cannot have generic methods.

        [mutating] void bump();
//CHECK:                ^^^^ E33075
//CHECK:                ^^^^ dyn interfaces cannot have [mutating] methods.

        [Differentiable] float diff(float x);
//CHECK:                       ^^^^ E33076
//CHECK:                       ^^^^ dyn interfaces cannot have [Differentiable] methods.

        int plain();
}
