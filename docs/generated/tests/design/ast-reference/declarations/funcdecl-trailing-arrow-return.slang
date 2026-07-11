//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A FuncDecl supports the modern function form `func name(...) -> T`; the arrow-return spelling produces the same callable as the C-style spelling.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modern `func` form parses as a FuncDecl and the
// trailing arrow return type takes effect at the call site.

//TEST:INTERPRET(filecheck=CHECK):

func triple(int x) -> int { return x * 3; }

void main()
{
    //CHECK: 33
    printf("%d\n", triple(11));
}
