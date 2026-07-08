//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:09:48Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope
//META: doc_section_digest=39f7ccab66bac47197cd0a95044a8180619767ca0b6e93966a0e57e3dc200e0e
//META: purpose=A DebugScope for a nested block chains to its enclosing scope by taking another DebugScope as its first operand.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies nested lexical scopes chain scope-to-scope rather than each
// pointing at the DebugFunction directly. The entry point declares locals
// inside if/else blocks nested under a function-level local, so distinct
// lexical scopes exist under -g. The CHECKs capture the id of a DebugScope
// definition and require a later DebugScope whose first operand is that
// id — a scope whose enclosing scope is itself a DebugScope.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int outer = int(tid.x);
    if (outer > 0)
    {
        int inner = outer + 1;
        buf[0] = inner;
    }
    else
    {
        int alt = outer - 1;
        buf[0] = alt;
    }
}

// CHECK: %[[OUTER:[0-9]+]]{{[^0-9].*}}= DebugScope(
// CHECK: DebugScope(%[[OUTER]]{{[^0-9]}}
