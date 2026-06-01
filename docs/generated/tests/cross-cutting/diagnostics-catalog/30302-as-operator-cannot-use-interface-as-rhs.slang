//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30302 (as-operator-cannot-use-interface-as-rhs) - cannot use 'as' with interface on right-hand side
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30302
//META: catalog_name=as-operator-cannot-use-interface-as-rhs
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `as` with an interface type RHS is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IA {}
interface IB {}

void f(IA a)
{
    Optional<IB> b = a as IB;
}

void main() {}

// CHECK: 30302
