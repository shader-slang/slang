//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=1d69c6377e76dbde08b14056d074bcc495099f7ebad74b5fa5cca67d869f272a
//META: purpose=Verifies the parser splits a trailing `>>` into two generic-argument closers rather than a right-shift operator.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `Array<Array<int, 2>, 3>` ends in `>>`, which the lexer
// tokenizes as a single right-shift. The generic-argument parser must split
// that token to close both generic-argument lists. If the split did not
// happen the type would fail to parse; CHECK asserts the program runs and
// the nested element read returns 9, proving both closers were consumed.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Array<Array<int, 2>, 3> a;
    a[2][1] = 9;
//CHECK: 9
    printf("%d\n", a[2][1]);
}
