//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Smallest-negative-int64 exception: `9223372036854775808LL` preceded by unary minus yields `-9223372036854775808` of type `int64_t` per the doc's explicit rule.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference makes an explicit exception:
//   "If the literal is `9223372036854775808`, `9223372036854775808L`,
//    or `9223372036854775808LL`, and it is preceded by unary minus,
//    the resulting value is `-9223372036854775808` and the type is
//    `int64_t`."
// Without this carve-out, the bare literal would lex-reject as too
// large for any integer type. Verify the LL-suffixed form works,
// observed via printf with %lld.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int64_t v = -9223372036854775808LL;
    //CHECK: v=-9223372036854775808
    printf("v=%lld\n", v);
}
