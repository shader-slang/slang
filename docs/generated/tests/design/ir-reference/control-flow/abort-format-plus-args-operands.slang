//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:07:37Z
//META: source_commit=c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#abort
//META: doc_section_digest=c7693fdd37023bb5a9e34f71071374da718341c95afdd7a9ab459dbdb06785c9
//META: purpose=The front-end-emitted Abort carries the format string plus the variadic argument pack: call arguments after the format appear as additional operands on the Abort instruction.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the variadic side of the Abort operand shape: passing two
// uniform-derived values after the format string produces an Abort
// with operands beyond the format. The CHECK requires a comma after
// the first operand — i.e. at least one argument-pack operand rides
// on the instruction — and the format literal is present in the dump.
// Uniform operands keep the arguments from constant-folding away.

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

// CHECK-LABEL: func %main
// CHECK-DAG: "bounds: a=%d b=%d"
// CHECK-DAG: Abort(%{{[0-9a-zA-Z_]+}}, %
