//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=At parse time both function calls and type constructions take the unified form `A(B)` — the `## AST data model` note explicitly says expressions and types share the Expr representation in a freshly-parsed AST.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `int(7)` is a type construction; `identity(7)` is a function call.
// Both reach the parser as the same Expr shape (an InvokeExpr whose
// callee is an identifier). The semantic checker later disambiguates;
// the parser does not.

//TEST:INTERPRET(filecheck=CHECK):

int identity(int v) { return v; }

void main()
{
    int x = int(7);
    int y = identity(7);
    // CHECK: x=7 y=7
    printf("x=%d y=%d\n", x, y);
}
