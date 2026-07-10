//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=A builtin comparison operator over builtin vector operands is rewritten by the checker and still yields the correct elementwise result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker's builtin-operator rewrite covers comparison
// operators with vector operands. The operands come from a function
// parameter so the comparison survives to runtime; CHECK asserts the
// per-component `<` results, which are only correct if the rewritten
// operator kept elementwise less-than semantics.

//TEST:INTERPRET(filecheck=CHECK):

int2 makeVec(int a, int b)
{
    return int2(a, b);
}

void main()
{
    int2 lhs = makeVec(1, 9);
    int2 rhs = makeVec(5, 4);
    bool2 lt = lhs < rhs;
    // CHECK: lt=1,0
    printf("lt=%d,%d\n", int(lt.x), int(lt.y));
}
