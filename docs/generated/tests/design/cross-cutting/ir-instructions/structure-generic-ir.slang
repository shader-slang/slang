//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=cdcc1238b1d73cfdad47a675356ee5e312b4cda22f2d9f5d255ae4ad1e0d4b21
//META: purpose=A generic function declaration lowers to a `generic` inst (type-level computation parent).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `generic`/`IRGeneric` structure row ("Type-level
// computation parent ending in `yield`"). A generic helper `idOf<T>`
// produces a `generic` inst in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T idOf<T>(T x) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = idOf(a);
}

// IR: generic
