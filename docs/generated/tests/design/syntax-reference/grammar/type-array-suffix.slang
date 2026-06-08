//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=0ce0b3e0544deb35556a1e8955b041223e597dbd359e81100c105e6f289f6efa
//META: purpose=`CoreType` accepts the array form `Type '[' Expr ']'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the array-type spelling `int[4]` as a function-parameter
// type. We sum the elements to prove the array is the right size.

//TEST:INTERPRET(filecheck=CHECK):

int sumOf(int xs[4])
{
    return xs[0] + xs[1] + xs[2] + xs[3];
}

void main()
{
    int xs[4] = { 1, 2, 3, 4 };
    // CHECK: r=10
    printf("r=%d\n", sumOf(xs));
}
