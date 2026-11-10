//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4df83dc71e0587111a9da4d14a547f230fc7c2be546870abd078c2d1480d3144
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

//CHECK: E45100
