//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#inheritance-control-attributes
//META: doc_section_digest=ed1cb4776f6b73ff1ef35d159b6b15ddb1bfdd8a40dd898084d8f96a7719263a
//META: purpose=An `[open]` interface accepts implementations; a basic implementation works through dynamic dispatch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `OpenAttribute` marks an interface or type as open for extension.
// A struct can implement the open interface and the value-receiving
// function can dispatch through the interface-typed parameter.

//TEST:INTERPRET(filecheck=CHECK):

[open]
interface IShape
{
    int sides();
}

struct Triangle : IShape
{
    int sides() { return 3; }
}

int countSides(IShape s)
{
    return s.sides();
}

void main()
{
    Triangle t;
    //CHECK: sides=3
    printf("sides=%d\n", countSides(t));
}
