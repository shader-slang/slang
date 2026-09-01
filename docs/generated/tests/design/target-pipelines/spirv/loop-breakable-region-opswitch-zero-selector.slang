//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#opswitch-case-literals-follow-the-selector-width
//META: doc_section_digest=261553d1527d9ea1e4d05d9bd40bce371d25c59c042c492c960808b79eb5b1ee
//META: purpose=A loop body is emitted as a breakable region whose OpSwitch always uses a 32-bit zero selector, independent of any user switch.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second OpSwitch producer described for SPIR-V emit: besides the
// value-switch path whose literals follow the selector width, the loop
// lowering emits an OpSwitch as a breakable region, and that one always uses a
// constant 32-bit zero selector. The shader contains a data-dependent loop
// with an early exit and no user-written switch at all, so any OpSwitch in the
// output must come from the loop lowering; CHECK asserts the loop merge and
// the zero-selector OpSwitch.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;
StructuredBuffer<int> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int n = inBuf[tid.x];
    int acc = 0;
    for (int i = 0; i < n; i++)
    {
        acc += i;
        if (acc > 50)
            break;
    }
    outBuf[tid.x] = acc;
}

// CHECK-DAG: OpLoopMerge
// CHECK-DAG: OpSwitch %int_0
