//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A ScopeDecl is the anonymous scope the parser creates for a block statement; a name declared inside shadows the outer one only within the block.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a nested block introduces its own scope. The same name is
// declared inside and outside a block with different values; the CHECK
// asserts the inner value while inside and the outer value afterwards, which
// requires the block to have its own scope rather than reusing the function's.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    {
        int x = 2;
        //CHECK: inner=2
        printf("inner=%d\n", x);
    }
    //CHECK: outer=1
    printf("outer=%d\n", x);
}
