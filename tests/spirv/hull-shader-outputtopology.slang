//TEST:SIMPLE(filecheck=SPIRV_POINT):-target spirv -stage hull -entry hullMain_point
//TEST:SIMPLE(filecheck=SPIRV_TRICW):-target spirv -stage hull -entry hullMain_triangle_cw
//TEST:SIMPLE(filecheck=SPIRV_TRICCW):-target spirv -stage hull -entry hullMain_triangle_ccw
//TEST:SIMPLE(filecheck=SPIRV_ISOLINE):-target spirv -stage hull -entry hullMain_isoline_line

// SPIRV_POINT: OpExecutionMode %hullMain_point PointMode
// SPIRV_TRICW: OpExecutionMode %hullMain_triangle_cw VertexOrderCw
// SPIRV_TRICCW: OpExecutionMode %hullMain_triangle_ccw VertexOrderCcw

// Isoline hull: Isolines (from [domain("isoline")]) is the only execution
// mode needed to express line output. OutputLinesEXT is mesh-shader-only and
// must not appear anywhere on this entry point (issue #11370). CHECK-NOTs
// before AND after the CHECK so FileCheck scans the full output, not just
// the prefix up to the Isolines match.
// SPIRV_ISOLINE-NOT: OutputLinesEXT
// SPIRV_ISOLINE:     OpExecutionMode %hullMain_isoline_line Isolines
// SPIRV_ISOLINE-NOT: OutputLinesEXT

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

struct IsolinePatchConst {
    float Edges[2] : SV_TessFactor;
};

IsolinePatchConst isolinePatchConst() {
    IsolinePatchConst o;
    o.Edges[0] = 1;
    o.Edges[1] = 1;
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

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("isolinePatchConst")]
Out hullMain_isoline_line() {
    Out o;
    o.x = 0;
    return o;
}
