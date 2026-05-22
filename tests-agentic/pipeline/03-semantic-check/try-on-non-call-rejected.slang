//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Expression checking rejects a `try` clause applied to a non-call expression (E30090).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `try` is only valid as a prefix on a function call or operator
// invocation; applying it to a literal is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main() throws int { int v = try 5; return v; }
//CHECK:                        ^^^ expression in 'try' must be a call
//CHECK:                        ^^^ expression in a 'try' clause must be a call to a function or operator overload.
// CHECK: E30090
