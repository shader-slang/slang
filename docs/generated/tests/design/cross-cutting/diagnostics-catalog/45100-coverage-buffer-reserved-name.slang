//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E45100 (coverage-buffer-reserved-name) - `__slang_coverage` is reserved by `-trace-coverage`
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=45100
//META: catalog_name=coverage-buffer-reserved-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declaring a global named `__slang_coverage` while building with
// -trace-coverage collides with the synthesized counter buffer and
// produces a reserved-name warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute -trace-coverage

RWStructuredBuffer<int> __slang_coverage;

[numthreads(1,1,1)]
void main() {
    __slang_coverage[0] = 1;
}

// CHECK: E45100
