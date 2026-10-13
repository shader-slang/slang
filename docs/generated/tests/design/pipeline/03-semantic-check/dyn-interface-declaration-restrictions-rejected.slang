//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#dyn-interface-restrictions
//META: doc_section_digest=34b8cbbe9b21fa61793bb31e24f989376f77dc0297fb3ad8b728b7ca592c2c22
//META: purpose=With the gate open, a `dyn interface` may not be generic (E33072), declare an associated type (E33073), a generic method (E33074), a callable whose computed receiver mode requires writable storage (E33075), a `[Differentiable]` method (E33076), or inherit a non-`dyn` interface (E33077).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the declaration-side restrictions on a `dyn interface` once the language-version
// gate opens. The E33075 cases exercise the checked receiver mode rather than one source spelling:
// an explicit `[mutating]` method, an HLSL method using the dialect's implicit writable default,
// and a setter all fail, while explicit read-only methods remain valid.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl docs/generated/tests/design/pipeline/03-semantic-check/dyn-interface-declaration-restrictions-rejected.slang -std 2026 -no-codegen

dyn interface IGen<T>
//CHECK:      ^^^^ E33072
//CHECK:      ^^^^ dyn interfaces cannot be generic: 'IGen'.
{
    [nonmutating] int get();
}

dyn interface IAssoc
{
    associatedtype A;
//CHECK:           ^ E33073
//CHECK:           ^ dyn interfaces cannot have associatedType members.
    [nonmutating] int get();
}

dyn interface IGenMethod
{
    [nonmutating] int pick<T>(T v);
//CHECK:              ^^^^ E33074
//CHECK:              ^^^^ dyn interfaces cannot have generic methods.
}

dyn interface IMut
{
    [mutating] void bump();
//CHECK:            ^^^^ E33075
//CHECK:            ^^^^ `dyn` interface member 'bump' has a writable receiver; use `[nonmutating]` to make it read-only.
}

dyn interface IImplicitWritable
{
    void bump();
//CHECK: ^^^^ E33075
//CHECK: ^^^^ `dyn` interface member 'bump' has a writable receiver; use `[nonmutating]` to make it read-only.
}

dyn interface ISetter
{
    property int value
    {
        get;
        set;
//CHECK:^^^ E33075
//CHECK:^^^ `dyn` interface member 'set' has a writable receiver; use `[nonmutating]` to make it read-only.
    }
}

dyn interface IDiff
{
    [nonmutating] [Differentiable] float f(float x);
//CHECK:                                 ^ E33076
//CHECK:                                 ^ dyn interfaces cannot have [Differentiable] methods.
}

interface IPlain
{
    [nonmutating] int q();
}

dyn interface IBadBase : IPlain
//CHECK:                 ^^^^^^ E33077
//CHECK:                 ^^^^^^ dyn interface 'IBadBase' may only inherit 'dyn' interfaces. 'IPlain' is not a dyn interface.
{
    [nonmutating] int get();
}

void main()
{
}
