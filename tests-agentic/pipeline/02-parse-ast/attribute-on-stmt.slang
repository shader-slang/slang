//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=19f0544fc4722bd28dd43a33e103486d87957caa89e6ec1608e33172cd506a6c
//META: purpose=Attribute parsing — `[name(args)]` — works wherever modifier parsing does; the `[unroll]` attribute attaches to a for-loop Stmt and the loop still executes correctly.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[unroll]` is an AttributeBase modifier attached to the ForStmt.
// The loop runs three iterations and the sum proves the body parsed
// as a statement under the attributed for.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = 0;
    [unroll]
    for (int i = 0; i < 3; i = i + 1) { s = s + 1; }
    // CHECK: s=3
    printf("s=%d\n", s);
}
