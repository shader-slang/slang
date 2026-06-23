//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Statement checking detects duplicate `case` values inside a switch and rejects with E30605.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Switch-scope rule: duplicate case labels surface as E30605.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 1;
    switch(x) { case 1: break; case 1: break; }
//CHECK:                       ^^^^ duplicate cases in switch
//CHECK:                       ^^^^ duplicate cases not allowed within a 'switch' statement
// CHECK: E30605
}
