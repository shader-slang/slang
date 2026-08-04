//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
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
