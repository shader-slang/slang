//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=8135956851ca195027226ec4800f952b5b9939ca3939bc07d946ccdb02975fc6
//META: purpose=Overload checking accepts a function taking 8 positional arguments and resolves at the call site; each argument flows through to the body.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on function arity ("many args" >= 8): verify the checker
// resolves all eight positional arguments correctly.

//TEST:INTERPRET(filecheck=CHECK):

int sum8(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

void main()
{
    // CHECK: sum=36
    printf("sum=%d\n", sum8(1, 2, 3, 4, 5, 6, 7, 8));
}
