//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7b099ee0bdd12ca353b0a6ed1f3489fd213a284affa197e35f4f7055fb4aa4f9
//META: purpose=Fires diagnostic E30301 (is-operator-cannot-use-interface-as-rhs) - cannot use 'is' with interface on right-hand side
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30301
//META: catalog_name=is-operator-cannot-use-interface-as-rhs
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `is` with an interface type as the right-hand side is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IA {}
interface IB {}

bool f(IA a)
{
    return a is IB;
}

void main() {}

//CHECK: 30301
