//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=74085b8937a2f64c1cc81b32d872d42eeb0f485c431b126cafe3c352926dd739
//META: purpose=A BlockStmt containing a single statement executes that statement and introduces a scope; a local declared inside is not visible after the block closes.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a block whose body slot holds exactly one statement still
// introduces a scope. The single inner declaration reuses the outer
// name; the CHECK asserts the outer variable is unchanged after the
// block, so the inner declaration stayed confined to it.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outer = 7;
    {
        int outer = 99;
    }
    // CHECK: outer=7
    printf("outer=%d\n", outer);
}
