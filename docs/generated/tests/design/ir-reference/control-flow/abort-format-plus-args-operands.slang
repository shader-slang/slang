//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:07:37Z
//META: source_commit=c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#abort
//META: doc_section_digest=c7693fdd37023bb5a9e34f71071374da718341c95afdd7a9ab459dbdb06785c9
//META: purpose=The front-end-emitted Abort carries the format string plus the variadic argument pack: call arguments after the format ride on the Abort as a single makeValuePack operand holding them.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the variadic side of the Abort operand shape: passing two
// uniform-derived values after the format string produces an Abort
// whose second operand is the variadic argument pack — a
// makeValuePack of type TypePack(Int, Int) built from exactly those
// two values. Capturing the pack's id ties the Abort operand to the
// pack that packages the call arguments, and the format literal is
// pinned inline on the Abort itself. Uniform operands keep the
// arguments from constant-folding away.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > b)
        abort("bounds: a=%d b=%d", a, b);
    buf[0] = a + b;
}

// CHECK: let %[[PACK:[0-9a-zA-Z_]+]] : TypePack(Int, Int) = makeValuePack(%a, %b)
// CHECK-LABEL: func %main
// CHECK: Abort("bounds: a=%d b=%d", %[[PACK]])
