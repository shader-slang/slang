//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=Boundary probe of the documented `switch` terminator with multiple case values: the IR `switch(...)` instruction takes the scrutinee, a default block, a merge block, and per-case `(value, block)` pairs as variadic operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `switch` as a `TerminatorInst`. A four-arm `switch`
// (three concrete cases plus `default`) is enough to confirm the
// variadic operand shape (`scrutinee, merge, default, value0, block0,
// value1, block1, ...`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    switch (a)
    {
        case 0: buf[0] = 10; break;
        case 1: buf[0] = 11; break;
        case 2: buf[0] = 12; break;
        default: buf[0] = 99; break;
    }
}

// IR-LABEL: func %main
// IR: switch(%a, %{{.*}}, %{{.*}}, 0 : Int, %{{.*}}, 1 : Int, %{{.*}}, 2 : Int, %{{.*}})
