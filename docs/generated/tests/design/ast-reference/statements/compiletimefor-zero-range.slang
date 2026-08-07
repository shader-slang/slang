//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=e6b1411878ad1410d1a25b47fb11f2d5bfdbd0f414a7e77f7f2b914774e3bc68
//META: purpose=A CompileTimeForStmt with a zero-length range (`$for(i in Range(0))`) unrolls to zero copies of the body; the surrounding accumulator is unchanged by the unrolled-no-times loop.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the lower edge of the compile-time range. Verifies a
// zero-length `Range(0)` instantiates the body zero times, so none of
// its side effects appear. The CHECK asserts the accumulator keeps its
// pre-loop value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 7;
    $for(i in Range(0))
    {
        sum = sum + 100;
    }
    // CHECK: sum=7
    printf("sum=%d\n", sum);
}
