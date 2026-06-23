//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#identifier-expressions
//META: doc_section_digest=caa6a8edb60e77c5f7baea1fd474acf5d581589b89a50b719ca26405a5bcf66e
//META: purpose=An identifier expression looks up the name in the environment and yields the value of the matching declaration.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An identifier expression (bare name) resolves to the declared variable in scope.
// Tests that reading a local and a module-scope constant via bare identifiers works.
//TEST:INTERPRET(filecheck=CHECK):

static const int answer = 42;

int getAnswer()
{
    // 'answer' is an identifier expression resolving to the module-scope constant.
    return answer;
}

void main()
{
    int x = 7;
    // 'x' is an identifier expression resolving to the local variable.
    int y = x;
    int z = getAnswer();
    //CHECK: 7
    //CHECK: 42
    printf("%d\n", y);
    printf("%d\n", z);
}
