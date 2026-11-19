// WGSL's `@builtin(primitive_index)` is only a fragment-stage input, so the WGSL legalizer rejects
// `SV_PrimitiveID` everywhere else with E30702. The front end reports most of these uses first, but
// it does not check the stage of a global `in` varying, and `-ignore-capabilities` silences its
// stage checks.
// https://github.com/shader-slang/slang/issues/13617

//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL): -target wgsl -stage vertex -entry globalInputVertexMain
//DIAGNOSTIC_TEST:SIMPLE(diag=VSIN): -target wgsl -stage vertex -entry vertexInputMain -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=VSOUT): -target wgsl -stage vertex -entry vertexOutputMain -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=FSOUT): -target wgsl -stage fragment -entry fragmentOutputMain -ignore-capabilities
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTPARAM): -target wgsl -stage fragment -entry fragmentOutParamMain -ignore-capabilities

// The legalized global carries no source location, so these annotations match by text only.
//GLOBAL: system value semantic 'sv_primitiveid' cannot be used as input in 'vertex' shader stage
in uint globalPrimitive : SV_PrimitiveID;

float4 globalInputVertexMain() : SV_Position
{
    return float4(float(globalPrimitive));
}

float4 vertexInputMain(uint primitive : SV_PrimitiveID) : SV_Position
//VSIN:                     ^^^^^^^^^ system value semantic 'sv_primitiveid' cannot be used as input in 'vertex' shader stage
{
    return float4(float(primitive));
}

// A field of a legalized output struct reports its error at the struct's name.
struct VertexOutput
/*VSOUT:
       ^^^^^^^^^^^^ system value semantic 'sv_primitiveid' cannot be used as output in 'vertex' shader stage
*/
{
    float4 position : SV_Position;
    uint primitive : SV_PrimitiveID;
};

VertexOutput vertexOutputMain()
{
    VertexOutput output;
    output.position = float4(0);
    output.primitive = 0;
    return output;
}

struct FragmentOutput
/*FSOUT:
       ^^^^^^^^^^^^^^ system value semantic 'sv_primitiveid' cannot be used as output in 'pixel' shader stage
*/
{
    float4 color : SV_Target0;
    uint primitive : SV_PrimitiveID;
};

FragmentOutput fragmentOutputMain(float4 position : SV_Position)
{
    FragmentOutput output;
    output.color = position;
    output.primitive = 0;
    return output;
}

float4 fragmentOutParamMain(float4 position : SV_Position, out uint primitive : SV_PrimitiveID) : SV_Target0
//OUTPARAM:                                                         ^^^^^^^^^ system value semantic 'sv_primitiveid' cannot be used as output in 'pixel' shader stage
{
    primitive = 0;
    return position;
}
