//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
//META: purpose=The checker synthesizes nested initializer-list construction so a struct containing another struct can be initialized by a brace-enclosed list of brace-enclosed lists.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on synthesized initializer-list constructor: nesting
// works — `{{1, 2}, 3}` initializes the outer struct field-by-field
// with the inner struct itself initialized by its own list.

//TEST:INTERPRET(filecheck=CHECK):

struct Inner { int a; int b; }
struct Outer { Inner i; int c; }

void main()
{
    Outer o = {{1, 2}, 3};
    // CHECK: sum=6
    printf("sum=%d\n", o.i.a + o.i.b + o.c);
}
