//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#append-and-consume-buffers
//META: doc_section_digest=9b3916f6b072d28628147b6bbc63ceda33f37e485011da27f65d136633a21691
//META: purpose=`AppendStructuredBuffer::Append(v)` lowers to `StructuredBufferAppend(%buf, %v)`; `ConsumeStructuredBuffer::Consume()` lowers to `StructuredBufferConsume(%buf)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two append/consume methods each lower to their own
// opcode rather than to a shared buffer-write. `Append` is given a
// thread-derived value and `Consume` takes only the buffer, so the two
// CHECKs under `func %main` pin two operands for the first and one for
// the second — the operand counts that distinguish the pair.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform AppendStructuredBuffer<int> abuf;
uniform ConsumeStructuredBuffer<int> cbuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    abuf.Append(idx);
    int c = cbuf.Consume();
    rwbuf[idx] = c;
}

// IR-LABEL: func %main
// IR: StructuredBufferAppend(%abuf, %idx)
// IR: StructuredBufferConsume(%cbuf)
