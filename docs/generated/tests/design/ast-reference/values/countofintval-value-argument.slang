//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=3b7a66f73a4b30e3c18815ae985a8f256039debd8a5caa97ed928ac84dcbb5bd
//META: purpose=The argument of a countof is any compile-time value, not only a type, so counting a bound value pack yields the same integer as counting its type pack.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "argument is any Val, not only a type" half of the
// countof claim by spelling both forms in one generic: `countof(args)`
// counts the value pack and `countof(T)` counts the type pack. CHECK
// asserts both print 4 for a four-argument call, so the value-argument
// form is accepted and folds to the same integer.

//TEST:INTERPRET(filecheck=CHECK):

int valueCount<each T>(expand each T args)
{
    return countof(args);
}

int typeCount<each T>(expand each T args)
{
    return countof(T);
}

void main()
{
    // CHECK: value=4 type=4
    printf("value=%d type=%d\n", valueCount(1, 2, 3, 4), typeCount(1, 2, 3, 4));
}
