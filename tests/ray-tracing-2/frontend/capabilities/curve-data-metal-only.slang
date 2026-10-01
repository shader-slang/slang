//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -target hlsl -profile cs_6_0 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -target metal -capability metallib_3_0 -restrictive-capability-check -no-codegen
//TEST:SIMPLE: -experimental-feature -entry main -target metal -capability metallib_3_1 -restrictive-capability-check -no-codegen

import slang.raytracing;

// Direct use of the view must preserve its target/version requirement independently of a stage
// context's CurvePrimitive requirement. No property access or structural entry selection is needed.
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    rt::CurveData curve = {};
}

//HLSL: unavailable features in entry point
//METAL: entry point uses capabilities not in specified profile
//METAL: Missing capabilities are: 'metallib_3_1'
