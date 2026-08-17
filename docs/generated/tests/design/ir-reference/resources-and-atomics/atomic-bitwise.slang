//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<T>::and(v)` / `or(v)` / `xor(v)` lower to `atomicAnd` / `atomicOr` / `atomicXor` with the standard `(ptr, val, order)` operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the three bitwise read-modify-write atomics each get
// their own opcode. They share one file because they share one operand
// shape; each call uses a distinct mask literal so the CHECKs cannot
// match each other's line, and each allows exactly one trailing operand
// — the memory order.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int pa = abuf[idx].and(0xff);
    int po = abuf[idx].or(0xf0);
    int px = abuf[idx].xor(0x0f);
    rwbuf[idx] = pa + po + px;
}

// IR-LABEL: func %main
// IR: atomicAnd(%{{[^,]+}}, 255 : Int, {{[^,)]+}})
// IR: atomicOr(%{{[^,]+}}, 240 : Int, {{[^,)]+}})
// IR: atomicXor(%{{[^,]+}}, 15 : Int, {{[^,)]+}})
