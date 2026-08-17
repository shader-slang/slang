//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#spirvasmoperand
//META: doc_section_digest=30d7b2afee044e4800553745578314d872096bca3b9b7d00e70fdc652dd523c8
//META: purpose=Each token of an inline-asm instruction becomes a typed operand instruction: the result marker and a named id are distinct operand kinds carried in the SPIRVAsmInst operand list.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that inline-asm tokens survive as typed operands rather than text.
// The block names an intermediate id and then feeds it to a second
// instruction whose result is the block's value. The first CHECK shows the
// named id appearing as the result-name operand of one instruction, and the
// second shows the result marker plus a reference to that same name on the
// next -- a substitution only possible because each token is its own
// instruction.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

float f(float x)
{
    return spirv_asm
    {
        %tmp : $$float = OpFMul $x $x;
        result:$$float = OpExtInst glsl450 Sqrt %tmp
    };
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = f(float(tid.x));
}

// CHECK: SPIRVAsmInst({{[0-9]+}} : UInt, Float, %"tmp",
// CHECK-NEXT: SPIRVAsmInst({{[0-9]+}} : UInt, Float, result, {{.*}}%"tmp")
