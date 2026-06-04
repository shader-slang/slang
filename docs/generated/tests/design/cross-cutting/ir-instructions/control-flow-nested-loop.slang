//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=Stress probe of the documented `loop` terminator: two nested `for` loops produce two distinct `loop(...)` terminators in the IR, each with its own back-edge block parameters.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `loop` is the loop terminator and that block
// `param`s replace SSA `phi`. A nested-loop stress case must produce
// at least two `loop` terminators (outer + inner) so the depth-of-
// nesting axis is exercised on the same opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            sum += i * j;
    buf[0] = sum;
}

// IR-LABEL: func %main
// IR: loop(%{{.*}}
// IR: loop(%{{.*}}
