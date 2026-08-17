//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#inputs-and-outputs
//META: doc_section_digest=51cd307451e57824ff6e611b882f6c834f023977ef696226f1a783598a50b7b9
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
