//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
//META: purpose=Overload resolution prefers the candidate that requires no implicit conversion (lowest conversion cost) over one that requires a numeric widening.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The overload-resolution doc says candidates are ranked by total
// conversion cost. With `f(int)` and `f(float)` both reachable for
// an `int` argument, the int-typed candidate must win because it
// needs no implicit conversion.

//TEST:INTERPRET(filecheck=CHECK):

int f(int x) { return 1; }
int f(float x) { return 2; }

void main()
{
    // CHECK: pick=1
    printf("pick=%d\n", f(5));
}
