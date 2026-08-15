//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=int8_t keeps its documented MIN value of -128 when it is returned from a function.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's narrowest signed base type survives a
// function-call boundary at its MIN edge. The same constant is printed
// twice: once read straight out of the type's minValue and once after
// travelling through a function return. The CHECK requires both to be
// -128, so a widening step that drops the sign bit shows up as a
// mismatch between the two.

//TEST:INTERPRET(filecheck=CHECK):

int8_t returnsInt8Min() { return int8_t(-128); }

void main()
{
    int8_t direct = int8_t.minValue;
    int8_t returned = returnsInt8Min();
    // CHECK: direct=-128 returned=-128
    printf("direct=%d returned=%d\n", int(direct), int(returned));
}
