//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<int>::add(0)` (degenerate zero increment) still lowers to the documented `atomicAdd(%ptr, 0 : Int, %order)` opcode shape; constant folding does not remove the call.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: zero is the additive identity. The compiler does
// not eliminate the atomic side effect even though the value is 0;
// the opcode must still surface.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].add(0);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, 0 : Int, {{.*}})
