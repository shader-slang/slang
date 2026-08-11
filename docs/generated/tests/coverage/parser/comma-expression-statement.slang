//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#parser
//META: doc_section_digest=27f3acccf1dfb3b65737707c2253f03d2be2f611459aaabcf0ced5dde22202b8
//META: purpose=Verifies a comma operator within an expression-statement parses and both side-effects execute left-to-right.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the expression-statement parser accepts the comma
// operator at the top level of a statement, so `a = 1, b = 2;` is one
// statement whose two assignments both run. CHECK asserts a+b == 3, which is
// only reachable if both comma-separated assignments were parsed and lowered.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 0, b = 0;
    a = 1, b = 2;
//CHECK: 3
    printf("%d\n", a + b);
}
