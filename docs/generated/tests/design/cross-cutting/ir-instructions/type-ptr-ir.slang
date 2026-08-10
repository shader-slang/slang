//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=888243734c872495cb014c721c075987fe7d0342f0ed8a4fbd71890cda43218d
//META: purpose=A local array allocation is typed `Ptr(valueType)` and an element address adds the optional access-qualifier and address-space operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `Ptr`/`PtrType` type row and its operand shape
// `valueType, accessQualifierOperand?, addressSpaceOperand?, dataLayout?`.
// A local `int nums[4]` is allocated by `var`, whose result type the IR
// prints as `Ptr(Array(Int, 4 : Int))` — the one-operand form carrying just
// the value type. Taking an element address through `getElementPtr` produces
// the longer form with the optional trailing operands present, so both the
// mandatory and the optional halves of the row are observed in one dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int nums[4];
    for (int i = 0; i < 4; ++i)
        nums[i] = a + i;
    outbuf[0] = nums[2];
}

// IR-LABEL: func %main
// IR: Ptr(Array(Int, 4 : Int)){{.*}}= var
// IR: Ptr(Int, {{.*}}getElementPtr(%{{.*}}, %{{.*}})
