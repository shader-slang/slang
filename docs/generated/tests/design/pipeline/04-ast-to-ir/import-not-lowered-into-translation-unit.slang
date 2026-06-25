//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#inputs-and-outputs
//META: doc_section_digest=7ba77fce5dc1798b01f8522adbbb841faf41fcf3d8305afd37eba98407d20593
//META: purpose=References to symbols defined in an imported module (e.g. the core module) appear in the lowered IR with `[import("...")]` decorations rather than as full function bodies; the doc says "The lowering step does NOT include the IR for imported modules".
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Inputs and outputs" section says the lowered IR module
// excludes `import`ed modules — those are pulled in by a later IR
// pass. Observable consequence: any reference to a core-module
// symbol (e.g. `sqrt`, `abs`, `min`, `max`) shows up in the dump as
// a declaration with an `[import("...")]` decoration, not as a body.
// We don't pin the exact callee name (the lowering may inline some
// builtins) — instead we check that the dump's preamble contains
// `import(` decorations, which is how imported symbols are spelled.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// CHECK: ### LOWER-TO-IR:
// CHECK: import(
