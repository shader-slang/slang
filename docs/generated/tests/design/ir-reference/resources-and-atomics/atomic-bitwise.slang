//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=52293a26550b22915d50787927526213ba913b7231afe19be04ac5605d79d8b4
//META: purpose=`Atomic<T>::and(v)` / `or(v)` / `xor(v)` lower to `atomicAnd` / `atomicOr` / `atomicXor` with the standard `(ptr, val, order)` operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's atomic-operations table lists the three bitwise atomics
// together. Group them in one test to keep the bundle compact while
// still asserting each opcode independently; each CHECK pins a single
// trailing memory-order operand per the documented
// `ptr, val, memoryOrder` shape.

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
