//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#append-and-consume-buffers
//META: doc_section_digest=9b3916f6b072d28628147b6bbc63ceda33f37e485011da27f65d136633a21691
//META: purpose=`AppendStructuredBuffer<S>::Append(s)` on a struct element type still lowers to a single `StructuredBufferAppend(%buf, %s)` opcode regardless of element-type shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the element operand stays a single value even when the
// element type is an aggregate. A two-field struct is appended, and the
// CHECK requires one `StructuredBufferAppend(%abuf, ...)` under
// `func %main` — the struct is not split into per-field appends.

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
