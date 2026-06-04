//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
//META: purpose=Overload resolution selects the exact-type `int` candidate over the `uint` candidate for an `int`-typed argument because no signedness conversion is needed.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on conversion-cost ranking with a single candidate
// per integer signedness: int-typed call prefers the int overload.

//TEST:INTERPRET(filecheck=CHECK):

int f(int x) { return 100; }
int f(uint x) { return 200; }

void main()
{
    int v = 5;
    // CHECK: pick=100
    printf("pick=%d\n", f(v));
}
