//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#inputs-and-outputs
//META: doc_section_digest=3818e672e7a18026a0b8da65a28b332ecca673b023400f4ffc5b7524324873b4
//META: purpose=The checker attaches a Type to every Expr, so a mixed int/float sum is typed float and selects the float overload at the enclosing call.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "types attached" half of the output contract by making the
// attached type observable. Two `probe` overloads differ only in parameter
// type, so which one runs is decided purely by the `Type*` the checker
// hung on the `i + f` expression. CHECK asserts the float overload ran,
// which is only possible if the sum was typed float rather than int.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float v)
{
    return 1;
}

int probe(int v)
{
    return 2;
}

void main()
{
    int i = 3;
    float f = 0.5f;
    // CHECK: k=1
    printf("k=%d\n", probe(i + f));
}
