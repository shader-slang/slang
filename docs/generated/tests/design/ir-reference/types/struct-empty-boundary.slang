//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Boundary: an empty struct still surfaces a parent `struct %Name` opcode (no field children).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// an empty struct still surfaces a parent `struct %Name` opcode (no
// field children). A struct with zero fields still emits the parent
// `struct %Name` IR opcode — the container exists even when empty.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Empty {}

uniform RWStructuredBuffer<int> buf;

void useIt(Empty e) {}

[numthreads(1, 1, 1)]
void main()
{
    Empty e;
    useIt(e);
    buf[0] = 0;
}

// IR-LABEL: struct %Empty
