//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#body
//META: doc_section_digest=fdc286f331c0c685dd422ee1d30a0c012594b17c6e2b446f9862de4629795eb8
//META: purpose=If the function result type is `void`, the result type clause may be elided from a `func` declaration.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "If the function result type is `void`, the result type clause
// may be elided."  We use a traditional-syntax `func` with no arrow clause.
// Note: func + in/out/inout parameters causes an ICE; we use a simple func here.

//TEST:INTERPRET(filecheck=CHECK):

func doNothing(x: int) { }

void main()
{
    doNothing(5);
    // CHECK: ok
    printf("ok\n");
}
