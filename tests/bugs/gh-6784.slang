//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target glsl -entry compute -stage compute
struct Params {
    float output_size_x;
    float output_size_y;
    float sample_delta;
    float sky_intensity;
}

ConstantBuffer<Params> params;
Texture2D<float3> input_equirect_map;
SamplerState common_samler;
RWTexture2D<float4> output_irradiance;
static const float PI = 3.1415926535;

float2 DirToUV(float3 dir) {
    float phi = atan2(dir.y, dir.x) + PI;      
    float theta = acos(dir.z);                
    return float2(phi / (2 * PI), theta / PI);
}

float3 SphericalToDir(float theta, float phi) {
    return float3(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
}

[shader("compute")]
[numthreads(16, 16, 1)]
void compute(uint3 tid: SV_DispatchThreadID) {
    float2 uv = (tid.xy + 0.5) / float2(params.output_size_x, params.output_size_y);
    float phi = uv.x * 2 * PI;
    float theta = (1 - uv.y) * PI; 
    float3 N = float3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
    float3 up = (0, 1, 0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float3 irradiance = 0;
    float nSample = 0;

    for (float sample_phi = 0.0; sample_phi < 2.0 * PI; sample_phi += params.sample_delta) {
        for (float sample_theta = 0.0; sample_theta < 0.5 * PI; sample_theta + params.sample_delta) {
    // CHECK:      tests/bugs/gh-6784.slang([[#@LINE-1]]): warning 41008: loop might execute indefinitely
    // CHECK-NEXT: for (float sample_theta = 0.0; sample_theta < 0.5 * PI; sample_theta + params.sample_delta) {
    // CHECK-NEXT:   ^~~ 
            float3 tangent_sample = SphericalToDir(sample_theta, sample_phi);
            float3 world_sample = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N;
            float2 sample_uv = DirToUV(world_sample);
            float3 color = input_equirect_map.Sample(common_samler, sample_uv);
            irradiance += color * cos(theta) * sin(theta);
            nSample++;
        }
    }
    irradiance = PI * irradiance / nSample;
    output_irradiance[tid.xy] = float4(irradiance * params.sky_intensity, 1.0f);
}
