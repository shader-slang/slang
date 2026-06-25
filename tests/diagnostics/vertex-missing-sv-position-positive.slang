//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

// Regression test for #8753 (positive cases).
//
// These vertex shapes should NOT warn:
//  * Direct return with SV_Position semantic on the function
//  * Struct return with an SV_Position-tagged field
//  * `out`-parameter SV_Position
//  * Indexed semantic (`SV_Position0`)
//  * Nested struct that contains an SV_Position-tagged field
//  * Void return with no out parameters (e.g. GLSL-style entry points
//    that write `gl_Position` via a global)

[shader("vertex")]
float4 directReturn(uint vid : SV_VertexID) : SV_Position
{
    return float4(0, 0, 0, 1);
}

struct VSOutput
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VSOutput structReturn(uint vid : SV_VertexID)
{
    VSOutput o;
    o.pos = float4(0, 0, 0, 1);
    return o;
}

[shader("vertex")]
void outParam(uint vid : SV_VertexID, out float4 pos : SV_Position)
{
    pos = float4(0, 0, 0, 1);
}

struct VSOutputIndexed
{
    float4 pos : SV_Position0;
};

[shader("vertex")]
VSOutputIndexed indexedSemantic(uint vid : SV_VertexID)
{
    VSOutputIndexed o;
    o.pos = float4(0, 0, 0, 1);
    return o;
}

struct InnerOut
{
    float4 pos : SV_Position;
};

struct OuterOut
{
    float3 color;
    InnerOut inner;
};

[shader("vertex")]
OuterOut nestedStruct(uint vid : SV_VertexID)
{
    OuterOut o;
    o.color = float3(0, 0, 0);
    o.inner.pos = float4(0, 0, 0, 1);
    return o;
}

[shader("vertex")]
void noOutputs(uint vid : SV_VertexID)
{
}
