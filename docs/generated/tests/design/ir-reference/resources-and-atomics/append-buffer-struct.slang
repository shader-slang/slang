//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#append-and-consume-buffers
//META: doc_section_digest=e274afd53362e00b53effe2d33f7a820c8f72cd2555df63f0be832ce50f6509d
//META: purpose=`AppendStructuredBuffer<S>::Append(s)` on a struct element type still lowers to a single `StructuredBufferAppend(%buf, %s)` opcode regardless of element-type shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's append-and-consume table says the element operand is
// optional ("because some lowerings split write and append"); the
// natural surface emits both operands together.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Item { int a; float b; }
uniform AppendStructuredBuffer<Item> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    Item it;
    it.a = idx;
    it.b = (float)idx;
    abuf.Append(it);
    rwbuf[idx] = idx;
}

// IR-LABEL: func %main
// IR: StructuredBufferAppend(%abuf, {{.*}})
