//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=16e17f0dd7f2f633f3f55fe47544e0f4c04b1b5b2ce0bf083439c29eeb4a3acc
//META: purpose=getBuiltinArithmeticCommonType makes float beat int, so an int + float expression has type float.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "float beats int" half of the usual arithmetic conversions the
// fast path applies before building a BuiltinOperatorExpr. An overload probe
// with one int and one float overload observes the type of `i + f`; CHECK
// asserts the float overload fired, which is only true if the common type is
// float.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int x)   { return 1; }
int probe(float x) { return 2; }

void main()
{
    int i = 3;
    float f = 4.0f;
    int tag = probe(i + f);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
