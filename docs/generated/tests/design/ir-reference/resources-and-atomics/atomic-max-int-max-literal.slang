//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<int>::max(INT_MAX)` (degenerate — argument is the absolute maximum) still lowers to the documented `atomicMax` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: largest possible argument to a max RMW.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].max(2147483647);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicMax(%{{[^,]+}}, 2147483647 : Int, {{.*}})
