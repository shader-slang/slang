//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#parser
//META: doc_section_digest=27f3acccf1dfb3b65737707c2253f03d2be2f611459aaabcf0ced5dde22202b8
//META: purpose=Verifies a user `operator+` declaration parses and a `a + b` use of it evaluates to the overload's result.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the operator-name parser accepts `+` as an overloadable
// operator, producing a function named `operator+`. This exercises the
// success side of the same branch that operator-decl-invalid-symbol pins on
// the failure side. Two V values are summed via the user overload; CHECK
// asserts the result component is 5 (2+3), which is only reachable if the
// declaration parsed and bound as the `+` operator.

//TEST:INTERPRET(filecheck=CHECK):

struct V { int x; }
V operator+(V a, V b) { V r; r.x = a.x + b.x; return r; }

void main()
{
    V a; a.x = 2;
    V b; b.x = 3;
    V c = a + b;
//CHECK: 5
    printf("%d\n", c.x);
}
