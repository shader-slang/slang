//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
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
