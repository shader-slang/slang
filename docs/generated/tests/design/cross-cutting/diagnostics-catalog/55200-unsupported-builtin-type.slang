//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E55200 (unsupported-builtin-type) - unsupported builtin type
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=55200
//META: catalog_name=unsupported-builtin-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `int64_t` is not a supported builtin type on the Metal backend,
// triggering the unsupported-builtin-type error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -entry main -stage compute

RWStructuredBuffer<int64_t> o;

[numthreads(1,1,1)]
void main() {
    o[0] = int64_t(1);
}

// CHECK: E55200
