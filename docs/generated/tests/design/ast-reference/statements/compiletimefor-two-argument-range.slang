//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=e6b1411878ad1410d1a25b47fb11f2d5bfdbd0f414a7e77f7f2b914774e3bc68
//META: purpose=A compile-time `for` accepts a two-argument `Range(begin, end)` whose begin bound is used, whereas the one-argument `Range(end)` form leaves the begin bound implicit.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit begin bound is honoured rather than ignored.
// The accumulator shifts a decimal digit per iteration, so the CHECK
// value 234 records both the exact set of iterated values (2, 3, 4) and
// their order; a run that started at 0 or included 5 could not produce
// it.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int digits = 0;
    $for(i in Range(2, 5))
    {
        digits = digits * 10 + i;
    }
    // CHECK: digits=234
    printf("digits=%d\n", digits);
}
