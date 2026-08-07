//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#untypedresourcehandle-and-untypedsamplerhandle
//META: doc_section_digest=75ee899053793b3043363438c88abda61127a16198c9bd25ae6ca84b91201696
//META: purpose=Indexing ResourceDescriptorHeap yields a value of the nullary IR type UntypedResourceHandle, carrying no operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The value `ResourceDescriptorHeap[i]` produces has its own opaque IR
// type that records nothing about the resource it will become — the heap
// index lives in the value, not the type. A compute kernel reads a
// RWStructuredBuffer out of the heap with a runtime index (so the fetch
// is not folded away) and stores through it; the IR dump is FileChecked
// inside `func %main` for a `let` typed UntypedResourceHandle feeding a
// call, and for the concrete buffer type appearing only on the following
// conversion — the handle type itself never names an element type.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

uniform uint heapIndex;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<uint> fetched = ResourceDescriptorHeap[heapIndex];
    outputBuffer[0] = fetched[0];
}

// IR: func %main
// IR: : UntypedResourceHandle{{.*}}= call
// IR: let %fetched{{.*}}: RWStructuredBuffer(UInt
