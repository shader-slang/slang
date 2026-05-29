//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=dd2ec8aef79db3f253a059c2cf01af45d95108a02c8daa3922585ed0db6730ef
//META: purpose=Boundary: an empty struct still surfaces a parent `struct %Name` opcode (no field children).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the `struct` opcode is the parent container for
// `field` children. A struct with zero fields still emits the
// parent `struct %Name` IR opcode — the container exists even
// when empty.

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
