// SV_ClipDistance as a float array maps to WGSL's clip_distances built-in (an array<f32, N>,
// N <= 8) under `enable clip_distances;`.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage vertex -entry vsMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage vertex -entry vsMain

struct VSOut
{
    float4 position : SV_Position;
    float clip[3] : SV_ClipDistance;
};

// SPV: OpEntryPoint
// SPV: BuiltIn ClipDistance
// WGSL: enable clip_distances;
// WGSL: @builtin(clip_distances) clip_{{[0-9]+}} : array<f32, i32(3)>
VSOut vsMain(uint vid: SV_VertexID)
{
    VSOut o;
    o.position = float4(float(vid), 0, 0, 1);
    o.clip[0] = 1;
    o.clip[1] = 0;
    o.clip[2] = -1;
    return o;
}

// WGSL has a single clip_distances output: a second array (SV_ClipDistance1) is reported instead of
// emitting a second @builtin(clip_distances).
//TEST:SIMPLE(filecheck=TWO): -target wgsl -stage vertex -entry vsTwoArrays
// TWO: error[E55202]
// TWO: 'sv_clipdistance' is not supported
// TWO-NOT: @builtin(clip_distances)

struct VSOutTwo
{
    float4 position : SV_Position;
    float clipA[2] : SV_ClipDistance0;
    float clipB[2] : SV_ClipDistance1;
};

VSOutTwo vsTwoArrays(uint vid: SV_VertexID)
{
    VSOutTwo o;
    o.position = float4(float(vid), 0, 0, 1);
    o.clipA[0] = 1;
    o.clipA[1] = 0;
    o.clipB[0] = -1;
    o.clipB[1] = 1;
    return o;
}
