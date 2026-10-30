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
// TWO-NOT: @builtin(clip_distances)
// TWO: error[E55202]
// TWO: 'sv_clipdistance' is not supported

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

// The largest array clip_distances takes (N = 8).
//TEST:SIMPLE(filecheck=EIGHT): -target wgsl -stage vertex -entry vsEight
// EIGHT: enable clip_distances;
// EIGHT: @builtin(clip_distances) {{.*}} : array<f32, i32(8)>

struct VSOutEight
{
    float4 position : SV_Position;
    float clip[8] : SV_ClipDistance;
};

VSOutEight vsEight(uint vid: SV_VertexID)
{
    VSOutEight o;
    o.position = float4(float(vid), 0, 0, 1);
    for (int i = 0; i < 8; i++)
        o.clip[i] = float(i) - 4.0;
    return o;
}

// Nine distances do not fit clip_distances (N <= 8).
//TEST:SIMPLE(filecheck=NINE): -target wgsl -stage vertex -entry vsNine
// NINE-NOT: @builtin(clip_distances)
// NINE: error[E55202]
// NINE: 'sv_clipdistance' is not supported

struct VSOutNine
{
    float4 position : SV_Position;
    float clip[9] : SV_ClipDistance;
};

VSOutNine vsNine(uint vid: SV_VertexID)
{
    VSOutNine o;
    o.position = float4(float(vid), 0, 0, 1);
    for (int i = 0; i < 9; i++)
        o.clip[i] = 1;
    return o;
}

// clip_distances is a vertex output only. A fragment shader that takes the vertex output struct
// (a common HLSL pattern), or a bare SV_ClipDistance parameter, is reported rather than given an
// input @builtin(clip_distances), which WGSL rejects.
//TEST:SIMPLE(filecheck=FSTRUCT): -target wgsl -stage fragment -entry psStruct
// FSTRUCT-NOT: @builtin(clip_distances)
// FSTRUCT: error[E55202]
float4 psStruct(VSOut i) : SV_Target
{
    return i.position;
}

//TEST:SIMPLE(filecheck=FPARAM): -target wgsl -stage fragment -entry psParam
// FPARAM-NOT: @builtin(clip_distances)
// FPARAM: error[E55202]
// FPARAM: 'sv_clipdistance' is not supported
float4 psParam(float4 position: SV_Position, float d[2]: SV_ClipDistance) : SV_Target
{
    return position + d[0];
}
