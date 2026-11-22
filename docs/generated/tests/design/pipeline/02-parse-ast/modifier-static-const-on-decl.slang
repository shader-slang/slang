//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=70d77929d730a0398ad6cf62c037992abc0cd9b4600f071779b6380151df5290
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
