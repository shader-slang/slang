//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A ModernParamDecl marked `inout` is mutable and copies the callee's change back to the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the escape hatch from the modern parameter's default immutability:
// with `inout` the name-first parameter can be assigned and the caller sees
// the new value. The CHECK asserts the caller-side variable after the call.

//TEST:INTERPRET(filecheck=CHECK):

func bump(inout x: int)
{
    x = x + 5;
}

void main()
{
    int a = 1;
    bump(a);
    //CHECK: 6
    printf("%d\n", a);
}
