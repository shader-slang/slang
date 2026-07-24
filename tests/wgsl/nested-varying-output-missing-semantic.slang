//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain

// Regression for shader-slang/slang#8183: a vertex output struct that both nests another struct and
// has a field with no user semantic must still get varying locations synthesized for WGSL and Metal.
// Both conditions are required to exercise this path — giving `originalBitangent` a semantic, or
// removing the nested `Vertex original` member, takes a different path.

struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VsOutput
{
    Vertex original;
    float3 originalBitangent;
    float4 svPosition : SV_Position;
};

[shader("vertex")]
VsOutput vertexMain(Vertex input)
{
    VsOutput o;
    o.original = input;
    o.originalBitangent = input.normal;
    o.svPosition = float4(input.position, 1.0);
    return o;
}

// For WGSL, the nested struct's user semantics and the unsemanticed `originalBitangent` each get a
// @location, and SV_Position becomes @builtin(position). The flattened leaves keep their nested
// user-semantic locations (0,1,2) and `originalBitangent` composes to location 3.

//WGSL: @location(0) position
//WGSL: @location(1) normal
//WGSL: @location(2) tangent
//WGSL: @location(3) originalBitangent
//WGSL: @builtin(position) svPosition

// For Metal, the flattened output struct carries the nested user semantics via [[user(...)]], the
// unsemanticed `originalBitangent` gets a synthesized [[user(_SLANG_ATTR_3)]] (location 3, matching
// its @location(3) on WGSL), and SV_Position becomes [[position]].
//METAL: struct vertexMain_Result
//METAL: {{\[\[}}user(POSITION){{\]\]}}
//METAL: {{\[\[}}user(NORMAL){{\]\]}}
//METAL: {{\[\[}}user(TANGENT){{\]\]}}
//METAL: originalBitangent{{.*}}{{\[\[}}user(_SLANG_ATTR_3){{\]\]}}
//METAL: {{\[\[}}position{{\]\]}}
