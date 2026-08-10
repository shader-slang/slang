//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=bf6116c2e5b561cd38063b60dd513143618c8f41f5c801a0b352865012956ca2
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
