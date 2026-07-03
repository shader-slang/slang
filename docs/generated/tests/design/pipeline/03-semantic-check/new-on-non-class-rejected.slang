//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
