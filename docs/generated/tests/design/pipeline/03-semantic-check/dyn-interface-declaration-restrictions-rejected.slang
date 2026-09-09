//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#dyn-interface-restrictions
//META: doc_section_digest=733bfc84d5a926cb0990d8b19fc9bdb571304a51dd223d8d0eb5820ebe12888a
//META: purpose=With the gate open, a `dyn interface` may not be generic (E33072) nor declare an associated type (E33073), a generic method (E33074), a `[mutating]` method (E33075), a `[Differentiable]` method (E33076), or a non-`dyn` base interface (E33077).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the six declaration-side restrictions a `dyn interface` is subject
// to once the language version opens the gate. Each restriction gets its own
// interface so one rejection cannot mask another, and the file is compiled with
// `-std 2026` because that is the version at which the validation stops being a
// no-op. Each CHECK is caret-anchored to the construct the rule names and pins
// one code; the set of six codes is what shows each rule is checked separately
// rather than folded into a single "unsupported dyn interface" report.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -std 2026

dyn interface IGen<T>
//CHECK:      ^^^^ E33072
{
    int get();
}

dyn interface IAssoc
{
    associatedtype A;
//CHECK:           ^ E33073
    int get();
}

dyn interface IGenMethod
{
    int pick<T>(T v);
//CHECK:^^^^ E33074
}

dyn interface IMut
{
    [mutating] void bump();
//CHECK:            ^^^^ E33075
}

dyn interface IDiff
{
    [Differentiable] float f(float x);
//CHECK:                   ^ E33076
}

interface IPlain
{
    int q();
}

dyn interface IBadBase : IPlain
//CHECK:                 ^^^^^^ E33077
{
    int get();
}

void main()
{
}
