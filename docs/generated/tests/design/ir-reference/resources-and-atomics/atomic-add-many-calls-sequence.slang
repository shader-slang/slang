//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=A single thread issuing eight `Atomic<int>::add(...)` calls in sequence emits eight distinct `atomicAdd` opcodes; the IR opcode shape is the same for every call.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: a chain of eight atomic-add calls. Each surfaces a
// separate `atomicAdd` opcode in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int s = 0;
    s += abuf[idx].add(1);
    s += abuf[idx].add(2);
    s += abuf[idx].add(3);
    s += abuf[idx].add(4);
    s += abuf[idx].add(5);
    s += abuf[idx].add(6);
    s += abuf[idx].add(7);
    s += abuf[idx].add(8);
    rwbuf[idx] = s;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, 1 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 2 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 3 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 4 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 5 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 6 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 7 : Int, {{.*}})
// IR: atomicAdd(%{{[^,]+}}, 8 : Int, {{.*}})
