//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=85307207e3b8f9c4a6840f45c1896c59f8f18e3bce3e9753d2b831e1d7ea443a
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
