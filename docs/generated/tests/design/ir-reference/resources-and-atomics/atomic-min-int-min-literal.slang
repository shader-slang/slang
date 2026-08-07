//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=534618adad82acb65da181567998c07a77381ecf27996abdf7535de5b49ec7b3
//META: purpose=`Atomic<int>::min(INT_MIN)` (degenerate — argument is the absolute minimum) still lowers to the documented `atomicMin` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: smallest possible argument to a min RMW. The
// opcode is emitted regardless of whether the comparison can change
// the stored value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].min(-2147483648);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicMin(%{{[^,]+}}, {{.*}}-2147483648{{.*}})
