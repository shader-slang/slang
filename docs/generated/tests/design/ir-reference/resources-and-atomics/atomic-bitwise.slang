//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<T>::and(v)` / `or(v)` / `xor(v)` lower to `atomicAnd` / `atomicOr` / `atomicXor` with the standard `(ptr, val, order)` operand shape.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's atomic-operations table lists the three bitwise atomics
// together. Group them in one test to keep the bundle compact while
// still asserting each opcode independently.

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
// IR: atomicAnd(%{{[^,]+}}, 255 : Int, {{.*}})
// IR: atomicOr(%{{[^,]+}}, 240 : Int, {{.*}})
// IR: atomicXor(%{{[^,]+}}, 15 : Int, {{.*}})
