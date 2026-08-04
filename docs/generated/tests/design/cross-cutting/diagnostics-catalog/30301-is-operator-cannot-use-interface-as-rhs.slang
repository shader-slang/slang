//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30301
