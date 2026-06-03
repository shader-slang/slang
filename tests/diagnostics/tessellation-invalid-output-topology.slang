//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry hullMain_triangle -stage hull -target spirv

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
/*CHECK:
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
