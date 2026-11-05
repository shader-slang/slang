// The `[patchconstantfunc]` lookup reports a missing function as E31006 alone and an overloaded
// name as an ambiguous reference (E39999), and a prototype followed by its definition is one
// function (shader-slang/slang#14053).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hullMain -stage hull
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSPELLED,exit-code=-1):-target hlsl -entry hullMain -stage hull -DMISSPELLED
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AMBIGUOUS,exit-code=-1):-target hlsl -entry hullMain -stage hull -DAMBIGUOUS

// HLSL-NOT: error[
// HLSL: patchconstantfunc("constants

// MISSPELLED-NOT: E38000

// AMBIGUOUS-NOT: E31006
// AMBIGUOUS: error[E39999]: ambiguous reference to 'constants'
// AMBIGUOUS-NOT: E31006
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

#if defined(AMBIGUOUS)
HscOut constants(uint patchID: SV_PrimitiveID)
{
    return (HscOut)0;
}
#endif

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
