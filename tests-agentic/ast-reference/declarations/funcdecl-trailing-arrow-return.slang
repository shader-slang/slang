//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
