//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:07:37Z
//META: source_commit=c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#abort
//META: doc_section_digest=c7693fdd37023bb5a9e34f71071374da718341c95afdd7a9ab459dbdb06785c9
//META: purpose=Boundary: the minimal abort(format) call still carries both documented operands — the format string plus a variadic argument pack that is empty (a makeValuePack with no elements).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the Abort operand shape: even with no
// variadic arguments the front end emits Abort with the format string
// plus the argument-pack operand, where the pack is an element-less
// makeValuePack of type TypePack (no element types). Capturing the
// pack's id and closing the paren right after it in the Abort match
// rejects any operands beyond the documented format+pack pair.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int g;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (g < 0)
        abort("negative input");
    buf[0] = g;
}

// CHECK: let %[[PACK:[0-9a-zA-Z_]+]] : TypePack = makeValuePack
// CHECK-LABEL: func %main
// CHECK: Abort("negative input", %[[PACK]])
