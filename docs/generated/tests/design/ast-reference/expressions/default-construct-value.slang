//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A `T()` zero-argument invocation default-constructs a value of T, observable as zero-initialized fields of a plain struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// DefaultConstructExpr is documented as "synthesized expression for a
// default-constructed value". Its user-spellable surface is `T()`
// with no arguments. For a plain struct of ints, all fields default
// to zero.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec { int x; int y; int z; }

void main()
{
    Vec v = Vec();
    // CHECK: x=0 y=0 z=0
    printf("x=%d y=%d z=%d\n", v.x, v.y, v.z);
}
