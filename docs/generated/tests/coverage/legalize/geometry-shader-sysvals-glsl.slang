//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the geometry-stage system-value mapping in the GLSL legalizer, where SV_PrimitiveID splits into gl_PrimitiveIDIn on input and gl_PrimitiveID on output and SV_GSInstanceID becomes gl_InvocationID.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `sv_primitiveid` / `sv_rendertargetarrayindex` /
// `sv_viewportarrayindex` branches of `getGLSLSystemValueInfo`
// (slang-ir-glsl-legalize.cpp:750-810) for the geometry stage. GLSL is the
// odd one out here: HLSL spells both the incoming and the emitted primitive
// index `SV_PrimitiveID`, but GLSL splits them into two distinct builtins, so
// the legalizer has to branch on `LayoutResourceKind` rather than on the
// semantic name alone. CHECK asserts both spellings appear in the same
// shader — `gl_PrimitiveIDIn` where the value is read and `gl_PrimitiveID`
// where it is written — plus the `int`-typed gl_Layer / gl_ViewportIndex
// stores that the uint-to-int `requiredType` conversion produces, and
// `gl_InvocationID` for SV_GSInstanceID.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage geometry

struct GsIn { float4 pos : SV_Position; }
struct GsOut
{
    float4 pos : SV_Position;
    uint layer : SV_RenderTargetArrayIndex;
    uint vp : SV_ViewportArrayIndex;
    uint pid : SV_PrimitiveID;
}

[maxvertexcount(3)]
[instance(2)]
[shader("geometry")]
void main(
    triangle GsIn ip[3],
    uint gsid : SV_GSInstanceID,
    uint pid : SV_PrimitiveID,
    inout TriangleStream<GsOut> outStream)
{
    for (int i = 0; i < 3; i++)
    {
        GsOut o;
        o.pos = ip[i].pos + float(gsid);
        o.layer = gsid;
        o.vp = 1;
        o.pid = pid;
        outStream.Append(o);
    }
    outStream.RestartStrip();
}

//GLSL-DAG: layout(triangles) in;
//GLSL-DAG: layout(invocations = 2) in;
//GLSL-DAG: layout(triangle_strip) out;
//GLSL-DAG: layout(max_vertices = 3) out;
// The per-vertex input struct is read through the gl_in[] interface block.
//GLSL: gl_in[{{.*}}].gl_Position
// SV_GSInstanceID -> gl_InvocationID.
//GLSL: gl_InvocationID
// Reading SV_PrimitiveID gives gl_PrimitiveIDIn; writing it gives gl_PrimitiveID.
//GLSL: = uint(gl_PrimitiveIDIn);
//GLSL-DAG: gl_Layer = int(
//GLSL-DAG: gl_ViewportIndex = int(
//GLSL-DAG: gl_PrimitiveID = int(
//GLSL-DAG: EmitVertex();
//GLSL-DAG: EndPrimitive();
