//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#sampler-and-buffer-layout-types
//META: doc_section_digest=dff5178219bf6c9d69e968d080af5b89c8ed266cacc3131cbee7ff9c13886e9e
//META: purpose=The core-module layout types Std140DataLayout, Std430DataLayout and ScalarDataLayout reach the IR as the Std140Layout, Std430Layout and ScalarLayout marker opcodes in a structured buffer's data-layout operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Six of the buffer-layout markers are real core-module types that a
// user can name as the second generic argument of a structured buffer,
// and the marker lands in the buffer type's data-layout operand rather
// than changing the buffer opcode. Three buffers are declared with three
// different explicit layout arguments and all are read so none is dead;
// each global_param row is FileChecked for
// `StructuredBuffer(Float, <Marker>`, which shows the marker occupies
// operand 1 and that the choice of marker is what varies.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -stage compute -entry main -dump-ir -o /dev/null

StructuredBuffer<float, Std140DataLayout> b140;
StructuredBuffer<float, Std430DataLayout> b430;
StructuredBuffer<float, ScalarDataLayout> bScalar;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = b140[0] + b430[0] + bScalar[0];
}

// IR-DAG: let %b140{{[A-Za-z0-9_]*}}{{.*}}: StructuredBuffer(Float, Std140Layout{{.*}}= global_param
// IR-DAG: let %b430{{[A-Za-z0-9_]*}}{{.*}}: StructuredBuffer(Float, Std430Layout{{.*}}= global_param
// IR-DAG: let %bScalar{{.*}}: StructuredBuffer(Float, ScalarLayout{{.*}}= global_param
// IR-DAG: let %outputBuffer{{.*}}: RWStructuredBuffer(Float, DefaultLayout{{.*}}= global_param
