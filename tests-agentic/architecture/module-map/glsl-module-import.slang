//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:31:34+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/architecture/module-map.md#sourceslang-glsl-module-embedded-glsl-module
//META: doc_section_digest=3e4d7fcc49f66ab4a160eff7e30bc7f511593beeafb0b5c1d0845baa0690474d
//META: purpose=The GLSL module (glsl.meta.slang) is embedded into libslang; `import glsl;` resolves with no extra search path.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the consequence of the embedded GLSL module: the name
// `glsl` resolves to a built-in module supplied by libslang. If the
// embedded GLSL module were missing, `import glsl;` would fail with
// an unresolved-module diagnostic.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

import glsl;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //CHECK: void main
}
