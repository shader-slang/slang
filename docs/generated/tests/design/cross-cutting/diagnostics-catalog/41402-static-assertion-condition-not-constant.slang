//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cdd3835077699bfa9a49945707eb4f75bf3d9fa21ba0fc08d49f08349414f217
//META: purpose=Fires diagnostic E41402 (static-assertion-condition-not-constant) - static assertion condition not compile-time constant
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=41402
//META: catalog_name=static-assertion-condition-not-constant
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static_assert whose condition depends on a uniform runtime
// value is not a compile-time constant — error fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main(uniform bool b) {
    static_assert(b, "needs constant");
}

//CHECK: E41402
