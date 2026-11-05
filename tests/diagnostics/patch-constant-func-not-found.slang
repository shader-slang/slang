// patch-constant-func-not-found.slang

// A `[patchconstantfunc]` name that does not resolve is reported once, as E31006
// at the attribute, not also as an entry-point lookup failure (E38000) or an
// empty "undefined identifier" (E30015).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry hullMain -stage hull

// CHECK-NOT: E30015
// CHECK-NOT: E38000
// CHECK: error[E31006]: function not found for attribute
// CHECK: Could not find function 'constnts' for attribute 'patchconstantfunc'
// CHECK-NOT: E31006
// CHECK-NOT: E30015
// CHECK-NOT: E38000

struct HullOutput
{
    float2 position;
};

struct PatchOutput
{
    float edgeFactors[4] : SV_TessFactor;
    float insideFactors[2] : SV_InsideTessFactor;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constnts")]
HullOutput hullMain()
{
    HullOutput result;
    result.position = 1;
    return result;
}

PatchOutput constants()
{
    PatchOutput result = {};
    return result;
}
