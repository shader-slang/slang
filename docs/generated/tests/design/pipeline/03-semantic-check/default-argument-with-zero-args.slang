//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
//META: purpose=Overload checking accepts a zero-argument call to a function with a default argument; the default-value expression is filled in at check time.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on function arity: 0 supplied args + 1 defaulted
// parameter is accepted; the synthesized default flows through.

//TEST:INTERPRET(filecheck=CHECK):

int g(int a = 10) { return a; }

void main()
{
    // CHECK: g=10
    printf("g=%d\n", g());
}
