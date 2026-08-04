//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Type checking resolves a typedef'd name to its base type so an operation on the alias is the same as on the underlying type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-type.cpp resolves type references. Verify the
// user-visible consequence by computing through a typealias and
// confirming the result matches the int form.

//TEST:INTERPRET(filecheck=CHECK):

typealias U32 = uint;

void main()
{
    U32 n = 5;
    uint r = n + 3;
    // CHECK: r=8
    printf("r=%u\n", r);
}
