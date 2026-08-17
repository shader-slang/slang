//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=74f15b021c9b1a1fc35f0749401acce4c72e39d8f31ca869b4e8328ed16cdc37
//META: purpose=A DebugScope opens a lexical scope whose operand 0 references the enclosing scope, a DebugFunction for a function-level scope, and whose operand 1 records the inlining context.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies scope chaining terminates at the owning function record. The
// snapshot is taken right after force-inlining, the pass that builds these
// chains, and debug level 1 keeps the module's source record from embedding a
// copy of this file. The CHECK captures the inlined callee's function record
// id and the inlining-frame id created at the call site, then requires the
// scope instruction to carry exactly those two ids in that order.

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

// CHECK: %[[DF:[0-9]+]]{{.*}}= DebugFunction("helper"
// CHECK: %[[IA:[0-9]+]]{{.*}}= DebugInlinedAt(
// CHECK-NEXT: DebugScope(%[[DF]], %[[IA]])
