//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `dyn interface` may not inherit a non-`dyn` interface (E33077) and may not be generic (E33072).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: two whole-declaration checks that `validateDynInterfaceUsage`
// performs after its member loop. `IChild` inherits a non-`dyn` interface and
// `IGeneric` is a generic interface; both are rejected under `-std 2026`. The
// annotations pin the observed codes and messages, and because the diagnostic
// test is exhaustive it also pins that the non-generic `IPlain` and the `dyn`
// members themselves are accepted silently.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026 -target hlsl

interface IPlain
{
        int b();
}

dyn interface IChild : IPlain
//CHECK:               ^^^^^^ E33077
//CHECK:               ^^^^^^ dyn interface 'IChild' may only inherit 'dyn' interfaces. 'IPlain' is not a dyn interface.
{
        int c();
}

dyn interface IGeneric<T>
//CHECK:      ^^^^^^^^ E33072
//CHECK:      ^^^^^^^^ dyn interfaces cannot be generic: 'IGeneric'.
{
        T get();
}
