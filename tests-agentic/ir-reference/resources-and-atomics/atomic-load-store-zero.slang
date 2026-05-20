//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<int>::store(0)` followed by `::load()` still lowers to the documented `atomicStore(%ptr, 0, ...)` and `atomicLoad(%ptr, ...)` opcode shapes; the zero value is preserved as `0 : Int`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: store the lower-edge value 0 and load it back.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    abuf[idx].store(0);
    int v = abuf[idx].load();
    rwbuf[idx] = v;
}

// IR-LABEL: func %main
// IR: atomicStore(%{{[^,]+}}, 0 : Int, {{.*}})
// IR: atomicLoad(%{{[^,]+}}, {{.*}})
