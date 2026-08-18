//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the GLSL mapping of SV_StartVertexLocation/SV_StartInstanceLocation to gl_BaseVertex/gl_BaseInstance (forcing #version 460) and of indexed SV_ClipDistanceN semantics to slots of a single gl_ClipDistance array.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `sv_startvertexlocation` / `sv_startinstancelocation` branches
// (slang-ir-glsl-legalize.cpp:982-996) and the `sv_clipdistance` branch
// (:518) of `getGLSLSystemValueInfo`. Two behaviours are worth pinning
// together here. First, the base-vertex/base-instance builtins only exist
// from GLSL 4.60, so requesting them raises the emitted `#version` for the
// whole shader — and because GLSL declares them `int` while HLSL declares the
// semantics `uint`, the legalizer records a `requiredType` of int and the
// emitter inserts a `uint(...)` conversion at each read. Second, an *indexed*
// clip-distance semantic is not a separate builtin: the branch reads the
// semantic's index and turns `SV_ClipDistance0` / `SV_ClipDistance1` into
// element 0 and element 1 of one `gl_ClipDistance` array sized to fit. CHECK
// asserts the version bump, the two conversions, and the single two-element
// array with its per-index stores.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage vertex

struct VOut
{
    float4 pos : SV_Position;
    float psize : SV_PointSize;
    float clip0 : SV_ClipDistance0;
    float clip1 : SV_ClipDistance1;
}

[shader("vertex")]
VOut main(uint startVert : SV_StartVertexLocation, uint startInst : SV_StartInstanceLocation)
{
    VOut o;
    o.pos = float4(float(startVert), float(startInst), 0, 1);
    o.psize = 1.5;
    o.clip0 = 0.25;
    o.clip1 = 0.75;
    return o;
}

// gl_BaseVertex / gl_BaseInstance require GLSL 4.60.
//GLSL: #version 460
// The two indexed clip-distance semantics share one array of the right size.
//GLSL: out float  gl_ClipDistance[2];
// Both base-location builtins are int in GLSL, hence the uint(...) casts.
//GLSL: vec4(float(uint(gl_BaseVertex)), float(uint(gl_BaseInstance)), 0.0, 1.0)
//GLSL-DAG: gl_Position =
//GLSL-DAG: gl_PointSize =
//GLSL-DAG: gl_ClipDistance[0] =
//GLSL-DAG: gl_ClipDistance[1] =
