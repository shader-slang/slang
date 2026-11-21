// WGSL takes dual-source blending only as one pair at @location(0), whose two members have the
// same type, with no other color output. Metal has none of these limits here.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=LOC,exit-code=2): -target wgsl -stage fragment -entry locationMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=TYPE,exit-code=2): -target wgsl -stage fragment -entry typeMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MIXED,exit-code=2): -target wgsl -stage fragment -entry mixedMain
//TEST:SIMPLE(filecheck=MTL-MIXED): -target metal -stage fragment -entry mixedMain

// LOC: error[E39035]
// LOC: location 1

// TYPE: error[E39036]

// MIXED: error[E39037]
// MIXED-NOT: error[E39037]

// MTL-MIXED: float4 color_{{[0-9]+}} {{\[\[}}color(0), index(0){{\]\]}};
// MTL-MIXED: float4 blend_{{[0-9]+}} {{\[\[}}color(0), index(1){{\]\]}};

struct LocationOut
{
    [[vk::location(1), vk::index(0)]] float4 color;
    [[vk::location(1), vk::index(1)]] float4 blend;
};

LocationOut locationMain(float4 p: SV_Position)
{
    LocationOut o;
    o.color = p;
    o.blend = p * 0.5;
    return o;
}

struct TypeOut
{
    [[vk::location(0), vk::index(0)]] float4 color;
    [[vk::location(0), vk::index(1)]] float3 blend;
};

TypeOut typeMain(float4 p: SV_Position)
{
    TypeOut o;
    o.color = p;
    o.blend = p.xyz;
    return o;
}

struct MixedOut
{
    [[vk::location(0), vk::index(0)]] float4 color : SV_Target0;
    [[vk::location(0), vk::index(1)]] float4 blend : SV_Target1;
    [[vk::location(1)]] float4 other : SV_Target2;
};

MixedOut mixedMain(float4 p: SV_Position)
{
    MixedOut o;
    o.color = p;
    o.blend = p * 0.5;
    o.other = p * 0.25;
    return o;
}
