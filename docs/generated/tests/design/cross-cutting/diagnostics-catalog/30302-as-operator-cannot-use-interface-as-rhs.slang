//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b8eedb4779ac3d234927d77f9205c9ffebbf8c11d737f81a09841f1fd1d5ba92
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

//CHECK: 30302
