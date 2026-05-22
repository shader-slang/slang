//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Expression checking rejects `new T` when T is a struct (E30065): `new` is only valid on classes.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expression-checking boundary: the `new` operator's restriction
// surfaces as E30065 when applied to a struct rather than a class.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int x; }

void main()
{
    var s = new S();
//CHECK:    ^^^ `new` can only be used to initialize a class
// CHECK: E30065

}
