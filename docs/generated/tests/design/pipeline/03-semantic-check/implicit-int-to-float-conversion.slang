//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Implicit-conversion ranking inserts an int-to-float coercion at a typed initialization site so the value preserves the source value as float.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Files table assigns slang-check-conversion.cpp the
// responsibility for "implicit-conversion ranking and coercion
// site checks". Verify the user-visible consequence: assigning an
// int to a float local converts losslessly.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 7;
    float f = i;
    // CHECK: f=7
    printf("f=%g\n", f);
}
