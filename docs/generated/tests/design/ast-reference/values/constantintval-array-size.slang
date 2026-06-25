//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=76a84a059e7493d5ffee00d494fa33d266abc65cffaa634a39c7c87cf041e48b
//META: purpose=A ConstantIntVal is the leaf IntVal used for a known compile-time integer; it stands in for the literal array size at type-check time and the array carries that many elements.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `ConstantIntVal` as the leaf for "a literal
// compile-time integer". When used as an array dimension, the
// surrounding `ArrayExpressionType` is parameterised by that
// IntVal; here a sized `int[5]` carries five independently
// addressable elements.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a[5];
    for (int i = 0; i < 5; ++i) { a[i] = i + 1; }
    // CHECK: arr=1,2,3,4,5
    printf("arr=%d,%d,%d,%d,%d\n", a[0], a[1], a[2], a[3], a[4]);
}
