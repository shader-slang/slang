//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Type resolution follows a chain of `typealias` declarations and resolves all the way to the underlying primitive type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on typedef-resolves-to-base-type: a 3-step alias chain
// still resolves correctly to the underlying `int`.

//TEST:INTERPRET(filecheck=CHECK):

typealias A = int;
typealias B = A;
typealias C = B;

void main()
{
    C x = 7;
    // CHECK: x=7
    printf("x=%d\n", x);
}
