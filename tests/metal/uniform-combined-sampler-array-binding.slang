// Texture and sampler arrays produced by resource-type legalization also get
// base `[[texture(N)]]` / `[[sampler(N)]]` attributes. `lowerCombinedTextureSamplers`
// rewrites each `Sampler2D` as a `{texture, sampler}` struct, and
// `legalizeResourceTypes` then splits an array of such structs, or of a user
// struct like `TwoTex`, into one array per field. Each field's offset is scaled
// by the element count, so `arr.b` starts two texture slots after `arr.a`.
// `cmpSamps` is a plain `SamplerComparisonState` array, another element type
// that the array unwrap newly binds.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
// Metal's compiler must accept the attributed array signatures end-to-end.
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

struct TwoTex
{
    Texture2D a;
    Texture2D b;
    SamplerState s;
};

uniform Sampler2D combined[4];
uniform TwoTex arr[2];
uniform Texture2D<float> depthTexs[2];
uniform SamplerComparisonState cmpSamps[2];
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x % 4;
    uint j = tid.x % 2;
    output[tid.x] = combined[i].SampleLevel(float2(0, 0), 0).x +
                    arr[j].a.SampleLevel(arr[j].s, float2(0, 0), 0).x +
                    arr[j].b.SampleLevel(arr[j].s, float2(0, 0), 0).x +
                    depthTexs[j].SampleCmpLevelZero(cmpSamps[j], float2(0, 0), 0.5);
}

// MTL-DAG: {{combined_texture_[0-9]+}} {{\[\[}}texture(0)]]
// MTL-DAG: {{combined_sampler_[0-9]+}} {{\[\[}}sampler(0)]]
// MTL-DAG: {{arr_a_[0-9]+}} {{\[\[}}texture(4)]]
// MTL-DAG: {{arr_b_[0-9]+}} {{\[\[}}texture(6)]]
// MTL-DAG: {{arr_s_[0-9]+}} {{\[\[}}sampler(4)]]
// MTL-DAG: {{depthTexs_[0-9]+}} {{\[\[}}texture(8)]]
// MTL-DAG: {{cmpSamps_[0-9]+}} {{\[\[}}sampler(6)]]

// LIB: define void @computeMain
