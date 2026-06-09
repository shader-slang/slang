//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
