//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=d9357662d809198b672de964a655fbd41120a6b17b51411c05a2be64457de64f
//META: purpose=Implicit-conversion ranking inserts an int-to-float coercion at a typed initialization site, preserving the source value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "coercion site checks" concern: the initializer is an `int`
// expression while the declared type is `float`, so the conversion checker
// must insert a widening coercion rather than reject the initialization.
// CHECK asserts the converted value, which proves the coercion happened and
// preserved the operand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 7;
    float f = i;
    // CHECK: f=7.5
    printf("f=%.1f\n", f + 0.5f);
}
