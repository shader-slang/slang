//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=19f0544fc4722bd28dd43a33e103486d87957caa89e6ec1608e33172cd506a6c
//META: purpose=Attribute parsing is modifier parsing in disguise — the `[name(args)]` token sequence becomes an `AttributeBase` modifier attached to the declaration that follows.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[ForceInline]` is an attribute that attaches to a function decl as
// an AttributeBase modifier. If attribute syntax did not parse, the
// program would fail at parse time long before any inlining or
// semantic checking ran.

//TEST:INTERPRET(filecheck=CHECK):

[ForceInline]
int twice(int v) { return v + v; }

void main()
{
    // CHECK: r=10
    printf("r=%d\n", twice(5));
}
