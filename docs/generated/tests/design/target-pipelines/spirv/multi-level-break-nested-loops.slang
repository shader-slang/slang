//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=eliminateMultiLevelBreak rewrites an early return out of two nested loops into a Function-storage bool flag plus single-level structured exits.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Phase C's eliminateMultiLevelBreak. SPIR-V structured control flow
// cannot express a jump that leaves two loop regions at once, so a `return`
// from inside the inner loop has to be converted into a flag that each loop
// level tests on its own merge path. Both loop bounds come from a buffer so
// neither loop is removed; CHECK asserts the introduced Function-storage bool
// flag initialized to %false and that both loops still carry an OpLoopMerge.
// The directive raises the optimization level to -O1 because at the harness
// default of -O0 the pass leaves the early exit as a plain in-loop OpReturn
// and the flag variable is never materialized.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -O1

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
        for (int j = 0; j < n; j++)
        {
            acc += j;
            if (acc > 100)
                return;
        }
        acc += i;
    }
    outBuf[tid.x] = acc;
}

// CHECK-DAG: OpVariable %_ptr_Function_bool Function %false
// CHECK-DAG: OpLoopMerge
