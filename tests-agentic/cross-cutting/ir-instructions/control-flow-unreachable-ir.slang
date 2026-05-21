//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=A function whose every control-flow path ends in `return` produces an `unreachable` terminator in the synthesized join block, exercising the documented `unreachable` exit terminator.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `unreachable` next to `return_val` and `discard` as a
// "return and exit terminator". A function whose `if`/`else` arms
// both `return` has no fall-through; the merge block terminates with
// `unreachable`, which is precisely what the doc claims that opcode
// is for.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int chooser(int x)
{
    if (x > 0) return 1;
    else return -1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = chooser(a);
}

// IR-LABEL: func %chooser
// IR: unreachable
