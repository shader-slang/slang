//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=A `for` loop lowers to the `loop` terminator and uses block `param`s to carry the SSA induction value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `for` loop in source lowers to a `loop` terminator (the doc's
// claim that the `TerminatorInst` family includes `loop`). The loop
// header block also carries `param` declarations — the doc's claim
// that "block parameters replace SSA `phi`."

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += i;
    buf[0] = sum;
}

// IR-LABEL: func %main
// IR: loop(%{{.*}}, %{{.*}}, %{{.*}}{{.*}})
// IR: param %{{.*}}
