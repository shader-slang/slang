//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Declaration checking rejects two locals with the same name in the same block as a conflicting declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-decl.cpp handles decl checking, including name-conflict
// detection. Two locals of the same name in the same block must be
// reported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 1;
    float x = 2;
//CHECK:  ^ conflicting declaration
//CHECK:  ^ declaration of 'x' conflicts with existing declaration
// CHECK: E30200
}
