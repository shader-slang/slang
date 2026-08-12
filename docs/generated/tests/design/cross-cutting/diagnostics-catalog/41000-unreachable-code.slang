//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=90781f3b39a20124c1c6e11f596f07bfe89014317e8079184304b4ea002da162
//META: purpose=Fires diagnostic E41000 (unreachable-code) - unreachable code detected
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41000
//META: catalog_name=unreachable-code
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A statement that follows an unconditional return is unreachable
// and the IR validation pass diagnoses it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main() {
    return;
    int x = 1;
}

//CHECK: E41000
