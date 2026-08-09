//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=888243734c872495cb014c721c075987fe7d0342f0ed8a4fbd71890cda43218d
//META: purpose=A fixed-size array type uses the `Array` IR type with element-type and element-count operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `Array`/`ArrayType` row ("Fixed-size array;
// hoistable") with operands `(elementType, elementCount)`. A local
// `int[4]` appears as an `Array(Int, 4 : Int)` type in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int i;
uniform StructuredBuffer<int> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { in_buf[0], in_buf[1], in_buf[2], in_buf[3] };
    buf[0] = arr[i];
}

// IR-LABEL: func %main
// IR: Array(Int, 4 : Int)
