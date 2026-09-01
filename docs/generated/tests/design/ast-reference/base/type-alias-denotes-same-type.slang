//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#type-val
//META: doc_section_digest=617a194ed6ec24148aa785801ab591efc2b4d803a1ca1e6680b6f00d76499121
//META: purpose=Every type the front end works with is a value, so an alias and the type it names denote one and the same type at an overload-resolution site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a type alias does not create a second type: `MyInt`
// must resolve to the very same value as `int`, so a variable declared
// with the alias selects the `int` overload rather than the `float`
// one or failing to match. CHECK asserts the `int` probe fires for
// both spellings.

//TEST:INTERPRET(filecheck=CHECK):

typealias MyInt = int;

int kind(int v)   { return 1; }
int kind(float v) { return 2; }

void main()
{
    MyInt aliased = 5;
    int   direct  = 5;
    //CHECK: aliased=1
    printf("aliased=%d\n", kind(aliased));
    //CHECK: direct=1
    printf("direct=%d\n", kind(direct));
}
