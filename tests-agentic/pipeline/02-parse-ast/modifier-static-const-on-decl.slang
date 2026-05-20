//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=19f0544fc4722bd28dd43a33e103486d87957caa89e6ec1608e33172cd506a6c
//META: purpose=Modifier tokens collected before a decl keyword attach to the resulting Decl through the Modifier chain rooted at ModifiableSyntaxNode; `static const int K = 21;` declares a usable compile-time-known binding.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `static` and `const` are modifier tokens that precede the `int K`
// declaration. The parser collects both into the Modifier chain on
// the VarDecl; the binding is then usable at any later point.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 21;

void main()
{
    int r = K + K;
    // CHECK: r=42
    printf("r=%d\n", r);
}
