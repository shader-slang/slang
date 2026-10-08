//DIAGNOSTIC_TEST:SIMPLE(diag=HULL): -entry hullMain_triangle -stage hull -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=HULL): -entry hullMain_triangle -stage hull -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=HULL): -entry hullMain_triangle -stage hull -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=HULL): -entry hullMain_triangle -stage hull -target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=DOMAIN): -entry domainMain_triangle -stage domain -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=DOMAIN): -entry domainMain_triangle -stage domain -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=DOMAIN): -entry domainMain_triangle -stage domain -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=DOMAIN): -entry domainMain_triangle -stage domain -target metal

struct Out
{
    float x;
};

struct PatchConst
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

PatchConst patchConst()
{
    PatchConst o;
    o.EdgeTessFactor[0] = 1;
    o.EdgeTessFactor[1] = 1;
    o.EdgeTessFactor[2] = 1;
    o.InsideTessFactor = 1;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle")]
/*HULL:
 ^^^^^^^^^^^^^^ invalid output topology
 ^^^^^^^^^^^^^^ Invalid output topology 'triangle' for stage 'hull', must be one of: 'point', 'line', 'triangle_cw', 'triangle_ccw'
*/
[outputcontrolpoints(3)]
[patchconstantfunc("patchConst")]
Out hullMain_triangle()
{
    Out o;
    o.x = 0;
    return o;
}

[shader("domain")]
[domain("tri")]
[outputtopology("triangle")]
/*DOMAIN:
 ^^^^^^^^^^^^^^ invalid output topology
 ^^^^^^^^^^^^^^ Invalid output topology 'triangle' for stage 'domain', must be one of: 'point', 'line', 'triangle_cw', 'triangle_ccw'
*/
float4 domainMain_triangle(
    PatchConst patchConstants,
    OutputPatch<Out, 3> patch,
    float3 bary : SV_DomainLocation)
    : SV_Position
{
    return float4(bary.x, bary.y, bary.z, patchConstants.InsideTessFactor);
}
