//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=54f732694185a0ac0e3a99ea776a2df81f46ad04014e1f6987caf714e9e03dcc
//META: purpose=An Atomic<T> field surfaces the Atomic opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An Atomic<T> field surfaces the Atomic opcode parameterised by its
// element type in the IR dump. A struct field of type `Atomic<uint>`
// surfaces an `Atomic(UInt)` operand on the field row in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Counter { Atomic<uint> n; }

uniform RWStructuredBuffer<Counter> gBuffer;
uniform RWStructuredBuffer<uint> outBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outBuffer[0] = gBuffer[0].n.load();
}

// IR-LABEL: struct %Counter
// IR: field(%{{.*}}, Atomic(UInt
