//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=85307207e3b8f9c4a6840f45c1896c59f8f18e3bce3e9753d2b831e1d7ea443a
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
