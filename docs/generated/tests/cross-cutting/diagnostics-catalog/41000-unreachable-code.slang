//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E41000 (unreachable-code) - unreachable code detected
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41000
//META: catalog_name=unreachable-code
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A statement that follows an unconditional return is unreachable
// and the IR validation pass diagnoses it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main() {
    return;
    int x = 1;
}

// CHECK: E41000
