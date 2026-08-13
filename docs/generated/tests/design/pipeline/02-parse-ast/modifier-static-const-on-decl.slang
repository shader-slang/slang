//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=e78e1fa50e5d6d7678c40996a6f91cc20ba30ced884ab9bc2c4a154f6aba1dfc
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
