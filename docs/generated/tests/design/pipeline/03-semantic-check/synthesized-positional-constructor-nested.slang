//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=c2e407a8f2f30473161ca1872b47d857bf827b2378eb5ccd399e2c4f1cecb0b6
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
