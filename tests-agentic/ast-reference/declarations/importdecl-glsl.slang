//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=An ImportDecl makes the declarations of another module visible in the current scope; importing glsl makes its names usable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of ImportDecl. `import glsl;` is the
// surface form that produces an ImportDecl referencing the `glsl`
// core module; the import succeeds and the program compiles to GLSL.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

import glsl;

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outBuf;

//CHECK: void main
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    outBuf[0] = 7;
}
