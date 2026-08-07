//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#other-control-flow-opcodes
//META: doc_section_digest=f5793a717c7bbfcde423407642d2b04f1b04ae204499660ecf91fed86254707e
//META: purpose=A __requireTargetExtension call lowers to the RequireTargetExtension instruction carrying the extension name as its operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the RequireTargetExtension row: the intrinsic call becomes its
// own opcode carrying the named extension, which is how the GLSL backend
// later learns to emit the matching `#extension` line. The CHECK pins the
// instruction and its string operand inside the entry point's block.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    __requireTargetExtension("GL_EXT_shader_atomic_int64");
    buf[0] = n;
}

// CHECK-LABEL: func %main
// CHECK: RequireTargetExtension("GL_EXT_shader_atomic_int64")
