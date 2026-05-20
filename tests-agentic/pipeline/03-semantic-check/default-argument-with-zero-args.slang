//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
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
