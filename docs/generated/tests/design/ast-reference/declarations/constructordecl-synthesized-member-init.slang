//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=77300e5bb37435bb7b157816bcb412b5ea19e8971e767a3eb68e890fccdc3431
//META: purpose=A struct with no user-written `__init` still gets a synthesized member-wise constructor; a brace initializer fills fields in declaration order.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a constructor exists even when the user wrote none. The struct
// declares only fields; a positional brace initializer is used and the CHECK
// asserts a sum that distinguishes correct field ordering from any other
// assignment of the two literals.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair
{
    int a;
    int b;
}

void main()
{
    Pair p = {3, 4};
    //CHECK: 7
    printf("%d\n", p.a + p.b);
}
