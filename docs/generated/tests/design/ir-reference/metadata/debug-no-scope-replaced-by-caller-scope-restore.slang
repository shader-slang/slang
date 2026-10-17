//META: generated=true
//META: model=claude-opus-5-5[1m]
//META: generated_at=2026-09-27T12:30:00+00:00
//META: source_commit=6eb89786ca882d71049c8568638e247f60864b6f
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=ca7fb4545f10893ab3c9d211926e944ee94d03a581ab401070eaed7fe9f324b8
//META: purpose=An inlined region is closed by a DebugScope that restores the caller's function scope with no inlining-context operand, not by DebugNoScope.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the caller-scope restore that closes an inlined region. Capturing
// both DebugFunction ids keeps another function's scope from satisfying the
// restore check, and at -g1 DebugSource omits this file's text, so the CHECK
// patterns cannot match themselves.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after performForceInlining -o - -entry main -stage compute -g1

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

// CHECK: %[[CALLEE:[0-9]+]]{{.*}}= DebugFunction("helper"
// CHECK: %[[CALLER:[0-9]+]]{{.*}}= DebugFunction("main"
// CHECK: %[[IA:[0-9]+]]{{.*}}= DebugInlinedAt({{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %{{[0-9]+}}, %[[CALLER]])
// CHECK-NEXT: DebugScope(%[[CALLEE]], %[[IA]])
// CHECK-NOT: DebugNoScope
// CHECK-NOT: DebugScope(
// CHECK: DebugScope(%[[CALLER]])
