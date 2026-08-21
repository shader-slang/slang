//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=No DebugInlinedVariable opcode is produced anywhere, even when a call is inlined under debug info.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "no producer at HEAD" note on the inlined-variable row. The
// shader forces a call with its own local to be inlined under debug info,
// which is precisely the situation the opcode exists for; the positive
// CHECK-DAGs show the inliner did run and did emit its other debug records, so
// the CHECK-NOT is a real negative rather than a vacuous one. Debug level 1
// also keeps the source record from embedding a copy of this file.

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

// CHECK-DAG: = DebugInlinedAt(
// CHECK-DAG: DebugScope(
// CHECK-NOT: DebugInlinedVariable
