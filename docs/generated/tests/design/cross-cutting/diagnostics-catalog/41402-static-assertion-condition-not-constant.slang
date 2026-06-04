//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E41402
