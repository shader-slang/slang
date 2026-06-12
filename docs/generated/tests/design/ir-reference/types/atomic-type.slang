//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=a2958c27179f43de9a6c91173654a84b9300b21241a75525e0ed05ed4d9bc705
//META: purpose=An Atomic<T> field surfaces the Atomic opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Vectors, matrices, and composite" table lists
// `Atomic` (C++ wrapper `AtomicType`) with operand
// `elementType: IRType`. A struct field of type `Atomic<uint>`
// surfaces an `Atomic(UInt)` operand on the field row in the IR
// dump.

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
