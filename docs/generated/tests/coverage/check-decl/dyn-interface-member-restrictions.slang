//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: purpose=Verifies that `dyn interface` rejects associated types, generic methods, computed writable receivers, and `[Differentiable]` methods under `-std 2026`.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `validateDynInterfaceUsage` only runs when the module is
// language version 2026-or-later and experimental dynamic dispatch was not
// requested, so this test compiles with `-std 2026`. The associated type, generic
// method, writable-receiver family, and differentiable method exercise distinct
// restrictions in that member loop. The E33075 cases cover explicit `[mutating]`,
// the implicit HLSL writable default, and a setter; the final `[nonmutating]` method
// is the accepted control.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl docs/generated/tests/coverage/check-decl/dyn-interface-member-restrictions.slang -std 2026 -target hlsl

dyn interface IFoo
{
    associatedtype T;
//CHECK:           ^ E33073
//CHECK:           ^ dyn interfaces cannot have associatedType members.

    int generic<U>(U u);
//CHECK:^^^^^^^ E33074
//CHECK:^^^^^^^ dyn interfaces cannot have generic methods.

    [mutating] void bump();
//CHECK:            ^^^^ E33075
//CHECK:            ^^^^ `dyn` interface member 'bump' has a writable receiver; use `[nonmutating]` to make it read-only.

    void implicitWritable();
//CHECK: ^^^^^^^^^^^^^^^^ E33075
//CHECK: ^^^^^^^^^^^^^^^^ `dyn` interface member 'implicitWritable' has a writable receiver; use `[nonmutating]` to make it read-only.

    property int value
    {
        get;
        set;
//CHECK:^^^ E33075
//CHECK:^^^ `dyn` interface member 'set' has a writable receiver; use `[nonmutating]` to make it read-only.
    }

    [nonmutating] [Differentiable] float diff(float x);
//CHECK:                                 ^^^^ E33076
//CHECK:                                 ^^^^ dyn interfaces cannot have [Differentiable] methods.

    [nonmutating] int plain();
}
