//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
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
