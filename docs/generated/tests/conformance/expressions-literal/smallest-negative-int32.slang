//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Smallest-negative-int32 exception: the literal `2147483648` preceded by unary minus yields `-2147483648` of type `int` per the doc's explicit rule.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference makes an explicit exception:
//   "If the literal is `2147483648` or `2147483648L` and it is
//    preceded by unary minus, the resulting value is `-2147483648`
//    and the type is `int`."
// Without this carve-out, `2147483648` itself would be `int64_t`
// (no `int` representation), and the unary minus would land on the
// wider type. Verify the value is parseable in `int`-typed context
// (the carve-out's value claim). The type claim is recorded as a
// doc-gap in the bundle README — overload resolution against no
// context picks `int64_t` rather than `int`, suggesting the doc's
// "type is `int`" rule applies only in `int`-typed context, not
// unconditionally.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = -2147483648;
    //CHECK: v=-2147483648
    printf("v=%d\n", v);
}
