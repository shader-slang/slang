//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:31:34+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/architecture/module-map.md#sourceslang-glsl-module-embedded-glsl-module
//META: doc_section_digest=3e4d7fcc49f66ab4a160eff7e30bc7f511593beeafb0b5c1d0845baa0690474d
//META: purpose=The GLSL module also backs the GLSL frontend; with `-allow-glsl`, slangc accepts GLSL source syntax and round-trips it to GLSL emit.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that GLSL-style source — `#version` directive and
// `layout(local_size_x=...)` compute-shader form — is accepted as
// input when `-allow-glsl` is passed. The output should contain the
// `#version` header that signals GLSL emit.

//TEST:SIMPLE(filecheck=CHECK):-allow-glsl -target glsl -entry main -stage compute

#version 450
layout(local_size_x = 1) in;
void main()
{
    //CHECK: #version
}
