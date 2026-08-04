//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A ParamDecl is a function parameter; in/out/inout direction qualifiers govern whether the callee's mutation is visible at the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of ParamDecl: function parameters
// carry direction. An `inout` parameter sees the caller's value
// passed in and propagates the callee's mutation back to the caller's
// argument. If ParamDecl could not carry direction the caller would
// not observe the change.

//TEST:INTERPRET(filecheck=CHECK):

void bumpInOut(inout int x) { x = x + 10; }
void bumpInOnly(in int x)   { x = x + 100; }

void main()
{
    int a = 1;
    bumpInOut(a);
    int b = 1;
    bumpInOnly(b);
    //CHECK: a=11 b=1
    printf("a=%d b=%d\n", a, b);
}
