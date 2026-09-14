//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A type expression used in value position has the type *of* a type (its kind), which the checker renders as `typeof(T)` when it does not match the expected value type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that naming a type where a value is expected does not simply
// fail as an undefined name: the type expression is checked and carries
// a type-of-a-type, which the mismatch diagnostic renders. CHECK pins
// `got 'typeof(float)'`, which is only printable if the type expression
// had a type of its own.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = float;
//CHECK:    ^^^^^ type mismatch in expression
//CHECK:    ^^^^^ expected an expression of type 'int', got 'typeof(float)'
    printf("x=%d\n", x);
}
