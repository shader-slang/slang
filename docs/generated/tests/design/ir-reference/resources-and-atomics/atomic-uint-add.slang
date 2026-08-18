//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<uint>::add(v)` lowers to `atomicAdd` exactly as for `int`, with the value typed as `UInt` in the operand position.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The atomic family is documented in terms of a pointer operand;
// the element type is whatever the `Atomic<T>` is parameterized on.
// Exercise the `uint` element type to confirm the opcode is the
// same name, with the value carrying `UInt` typing. The CHECK pins a
// single trailing memory-order operand per the documented
// `ptr, val, memoryOrder` shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<uint>> abuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint prev = abuf[idx].add(1u);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, 1 : UInt, {{[^,)]+}})
