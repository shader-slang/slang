// Dual-source blending needs exactly one output with vk::index(0) and one with vk::index(1) at a
// location, as direct fields of the fragment output struct (WGSL @blend_src, Metal index()).
//DIAGNOSTIC_TEST:SIMPLE(diag=LONE): -target wgsl -stage fragment -entry loneMain
//DIAGNOSTIC_TEST:SIMPLE(diag=LONE): -target metal -stage fragment -entry loneMain
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target wgsl -stage fragment -entry nestedMain

struct LoneOut
{
    [[vk::location(0), vk::index(1)]] float4 blend;
//LONE:                                      ^^^^^ unpaired dual-source blending output
//LONE:                                      ^^^^^ dual-source blending at location 0 needs exactly one output with '[[vk::index(0)]]' and one with '[[vk::index(1)]]' at that location
};

LoneOut loneMain(float4 p: SV_Position)
{
    LoneOut o;
    o.blend = p;
    return o;
}

struct Inner
{
    [[vk::location(0), vk::index(1)]] float4 blend;
//NESTED:                                    ^^^^^ nested dual-source blending output
//NESTED:                                    ^^^^^ an output with '[[vk::index(...)]]' must be a direct field of the fragment shader's output struct, not of a nested struct
};

struct NestedOut
{
    [[vk::location(0), vk::index(0)]] float4 color;
    [[vk::location(1)]] Inner inner;
};

NestedOut nestedMain(float4 p: SV_Position)
{
    NestedOut o;
    o.color = p;
    o.inner.blend = p;
    return o;
}
