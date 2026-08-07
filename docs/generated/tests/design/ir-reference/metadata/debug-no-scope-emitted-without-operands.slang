//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=74f15b021c9b1a1fc35f0749401acce4c72e39d8f31ca869b4e8328ed16cdc37
//META: purpose=DebugNoScope is emitted with zero operands, so its declared scope accessor must not be called on an instruction from that emitter.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the zero-operand shape of the scope terminator. Inlining a call
// under debug info closes the inlined region with this marker. The CHECK
// requires the mnemonic to be followed by end-of-line -- no parenthesised
// operand list at all -- which is the observable consequence of the emitter
// creating it with no operands even though the declaration allows one.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after performForceInlining -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int helper(int x)
{
    int t = x * 3;
    return t + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(int(tid.x));
}

// CHECK: DebugNoScope{{$}}
// CHECK-NOT: DebugNoScope(
