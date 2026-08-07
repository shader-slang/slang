//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=6195cb94dbbd650ed3781180b621c12cc25bdf23da298eb8248708408790b4b2
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
