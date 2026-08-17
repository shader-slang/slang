//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<T>::sub(v)` lowers to `atomicSub(%ptr, %val, %order)`, a dedicated opcode rather than an `atomicAdd` of a negated value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `sub` method the atomic table lists for
// `IArithmeticAtomicable` element types produces its own `atomicSub`
// opcode. The subtrahend is derived from `SV_DispatchThreadID` so the
// folder cannot collapse it, and the CHECK pins the three-operand
// `(ptr, val, order)` shape immediately inside `func %main` — the same
// shape the sibling `atomicAdd` row commits to.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].sub(idx + 2);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicSub(%{{[^,]+}}, %{{[^,]+}}, {{[^,)]+}})
