//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=74d850d9a54f442aac4cfdea60e05bde1723fd77cf6e54f9a6d3ec72cade3289
//META: purpose=Negative: the check-recursion validation pass diagnoses mutual recursion (`fa` → `fb` → `fa`) just as it does direct self-recursion.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Mutual recursion variant — alternative input shape for the
// check-recursion pass. The diagnostic attaches to the first call
// site in the cycle (`fa` calls `fb`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int fb(int n);

int fa(int n) { return n <= 0 ? 0 : fb(n - 1); }
//CHECK:                              ^ recursion not allowed
//CHECK:                              ^ recursion detected in call to 'fb', but the current code generation target does not allow recursion.
// CHECK: E55201

int fb(int n) { return n <= 0 ? 1 : fa(n - 1); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = fa(3);
}
