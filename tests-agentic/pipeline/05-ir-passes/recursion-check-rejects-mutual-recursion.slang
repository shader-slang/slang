//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=61134c3253681818e651a9810123d97ae5bcc929737f8bed5d159bd896cf3248
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

int fb(int n) { return n <= 0 ? 1 : fa(n - 1); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = fa(3);
}
