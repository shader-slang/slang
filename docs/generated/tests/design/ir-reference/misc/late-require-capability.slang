//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#compiler-dictionary-and-late-capability-requirements
//META: doc_section_digest=fc0d5f50b673be2d3dcdb59c6ba77ed859daa9fda1a7df05cc36a96f62f883dd
//META: purpose=A `__requireCapability(...)` statement also emits a `LateRequireCapability` inst whose single operand is the encoded capability set.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second inst that a capability requirement lowers to: the
// deferred check itself, carried as `LateRequireCapability(<capability set>)`
// inside the function body rather than as a decoration. The CHECK anchors at
// the entry point so the match comes from the user function, and asserts the
// inst takes exactly one `%`-named operand — the set built by the capability
// encoding above it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    __requireCapability(spirv_1_3);
    outputBuffer[0] = 2;
}

// IR-LABEL: func %main
// IR: LateRequireCapability(%{{[A-Za-z0-9_]+}})
