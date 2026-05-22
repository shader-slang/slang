//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Statement checking rejects `break` inside a `defer` block (E30108).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Statement-checking boundary: `defer` has its own scope rules,
// disallowing `break` (E30108) even when the defer is inside a
// loop that would otherwise make `break` legal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 10; i++) { defer { break; } }
//CHECK:                                   ^^^^^ 'break' inside defer
//CHECK:                                   ^^^^^ 'break' must not appear inside a defer statement.
// CHECK: E30108
}
