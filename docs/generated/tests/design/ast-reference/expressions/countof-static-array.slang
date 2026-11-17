//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=Verifies that `countof(e)` on a static array yields the array's element count.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The element-count operand is documented to accept an array as well as a
// pack. Two arrays of different lengths and different element types are
// probed so a result that tracks the element size instead of the length is
// distinguishable from the correct answer; the CHECKs assert the lengths.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[9];
    double b[6];
    // CHECK: a=9
    printf("a=%d\n", countof(a));
    // CHECK: b=6
    printf("b=%d\n", countof(b));
}
