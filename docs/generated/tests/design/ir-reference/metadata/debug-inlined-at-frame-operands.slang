//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=DebugInlinedAt records one frame of an inlining chain as line, column, source file and the debug function it was inlined into, with the outer-frame operand absent at the outermost frame.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the inlining-frame record produced when a call is inlined under
// debug info. Observation uses the snapshot taken right after force-inlining,
// because that is the pass that creates the record, and debug level 1 keeps
// the source record from embedding a copy of this file. The CHECK pins four
// operands and a closing parenthesis, so the optional outer-frame operand is
// shown to be absent for this single-level inline, and the captured source id
// is required to be the module's source record.

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

// CHECK: %[[SRC:[0-9]+]]{{.*}}= DebugSource(
// CHECK: = DebugInlinedAt({{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %[[SRC]], %[[CALLER:[0-9]+]])
