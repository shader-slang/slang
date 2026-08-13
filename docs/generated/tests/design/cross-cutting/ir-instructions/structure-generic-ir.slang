//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=d6d21d15e8b14ba2b57495447b3188f156d4c4168b8574097962151b591b1497
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
