//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=Eight atomic-add calls to eight distinct buffer elements each produce a documented `atomicAdd` opcode; the shape is uniform across operands.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: parallel pattern — many atomic adds, each to a
// distinct location. Verifies that distinct pointer operands all
// surface the same documented opcode name.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    s += abuf[0].add(1);
    s += abuf[1].add(1);
    s += abuf[2].add(1);
    s += abuf[3].add(1);
    s += abuf[4].add(1);
    s += abuf[5].add(1);
    s += abuf[6].add(1);
    s += abuf[7].add(1);
    rwbuf[tid.x] = s;
}

// IR-LABEL: func %main
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
// IR: atomicAdd({{.*}})
