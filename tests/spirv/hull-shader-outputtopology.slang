//TEST:SIMPLE(filecheck=SPIRV_POINT):-target spirv -stage hull -entry hullMain_point
//TEST:SIMPLE(filecheck=SPIRV_TRICW):-target spirv -stage hull -entry hullMain_triangle_cw
//TEST:SIMPLE(filecheck=SPIRV_TRICCW):-target spirv -stage hull -entry hullMain_triangle_ccw

// SPIRV_POINT: OpExecutionMode %hullMain_point PointMode
// SPIRV_TRICW: OpExecutionMode %hullMain_triangle_cw VertexOrderCw
// SPIRV_TRICCW: OpExecutionMode %hullMain_triangle_ccw VertexOrderCcw

struct Out {
    float x;
};

struct PatchConst {
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

PatchConst patchConst() {
    PatchConst o;
    o.EdgeTessFactor[0] = 1;
    o.EdgeTessFactor[1] = 1;
    o.EdgeTessFactor[2] = 1;
    o.EdgeTessFactor[3] = 1;
    o.InsideTessFactor[0] = 1;
    o.InsideTessFactor[1] = 1;
    return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("point")]
[outputcontrolpoints(4)]
[patchconstantfunc("patchConst")]
Out hullMain_point() {
    Out o;
    o.x = 0;
    return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("patchConst")]
Out hullMain_triangle_cw() {
    Out o;
    o.x = 0;
    return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("patchConst")]
Out hullMain_triangle_ccw() {
    Out o;
    o.x = 0;
    return o;
}
