//TEST:SIMPLE(filecheck=HULL):   -target spirv-asm -stage hull   -entry hullMain
//TEST:SIMPLE(filecheck=DOMAIN): -target spirv-asm -stage domain -entry domainMain

// For isoline tessellation, the SPIR-V output topology is expressed solely by
// the `Isolines` execution mode emitted via the [domain("isoline")] attribute.
// `OutputLinesEXT` is a mesh-shader execution mode (SPV_EXT_mesh_shader) and
// must not be emitted on a tessellation entry point — `spirv-val` rejects it
// because it requires the MeshShadingEXT capability.

// HULL-NOT: OutputLinesEXT
// HULL: OpExecutionMode %hullMain Isolines
// HULL-NOT: OutputLinesEXT

// DOMAIN-NOT: OutputLinesEXT
// DOMAIN: OpExecutionMode %domainMain Isolines
// DOMAIN-NOT: OutputLinesEXT

struct VS_OUT
{
    float3 position : POSITION;
};

struct HS_OUT
{
    float3 position : POSITION;
};

struct HSC_OUT
{
    float EdgeTessFactor[2] : SV_TessFactor;
};

HSC_OUT constants(InputPatch<VS_OUT, 2> patch)
{
    HSC_OUT o;
    o.EdgeTessFactor[0] = 1;
    o.EdgeTessFactor[1] = 1;
    return o;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("constants")]
HS_OUT hullMain(InputPatch<VS_OUT, 2> patch, uint i : SV_OutputControlPointID)
{
    HS_OUT o;
    o.position = patch[i].position;
    return o;
}

struct DS_OUT
{
    float4 position : SV_Position;
};

[domain("isoline")]
DS_OUT domainMain(HSC_OUT input, float2 uv : SV_DomainLocation, OutputPatch<HS_OUT, 2> patch)
{
    DS_OUT o;
    o.position = float4(lerp(patch[0].position, patch[1].position, uv.x), 1.0);
    return o;
}
