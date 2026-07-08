//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:07:37Z
//META: source_commit=c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#abort
//META: doc_section_digest=c7693fdd37023bb5a9e34f71071374da718341c95afdd7a9ab459dbdb06785c9
//META: purpose=Boundary: the minimal abort(format) call — an empty variadic pack — lowers to an Abort instruction whose sole operand is the format string.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the Abort operand shape: with no
// variadic arguments, the front end emits Abort with exactly one
// operand. The CHECK's closing paren immediately after the single
// operand reference rejects any extra operands, and the format
// literal itself must be present in the dumped function.

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

// CHECK-LABEL: func %main
// CHECK-DAG: "negative input"
// CHECK-DAG: Abort(%{{[0-9a-zA-Z_]+}})
