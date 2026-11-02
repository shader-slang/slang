// The `[patchconstantfunc]` lookup reports its own error when the named function is missing, with
// no E38000 "no function found matching entry point name" in front of it, and a prototype followed
// by its definition is one function (shader-slang/slang#14053).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hullMain -stage hull
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSPELLED):-target hlsl -entry hullMain -stage hull -DMISSPELLED

// HLSL-NOT: error[
// HLSL: patchconstantfunc("constants

// MISSPELLED-NOT: E38000
// MISSPELLED: error[E31006]: function not found for attribute
// MISSPELLED-NOT: E38000

struct HsOut
{
    float2 pos;
};

struct HscOut
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

HscOut constants();

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
#if defined(MISSPELLED)
[patchconstantfunc("constantz")]
#else
[patchconstantfunc("constants")]
#endif
HsOut hullMain()
{
    HsOut o;
    o.pos = 1;
    return o;
}

HscOut constants()
{
    HscOut o;
    o.EdgeTessFactor[0] = 1;
    o.EdgeTessFactor[1] = 2;
    o.EdgeTessFactor[2] = 3;
    o.EdgeTessFactor[3] = 4;
    o.InsideTessFactor[0] = 0.5;
    o.InsideTessFactor[1] = 0.5;
    return o;
}
