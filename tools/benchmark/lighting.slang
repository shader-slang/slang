module environment;

import types;
import common;

//-------------------------------------------------------------------------------------------------
// Environment
//-------------------------------------------------------------------------------------------------

// Environment map and sample data for importance sampling
Texture2D<float4> environment_texture : register(t0, space1);
StructuredBuffer<Environment_sample_data> environment_sample_buffer : register(t1, space1);
SamplerState sampler_latlong : register(s3);

struct Environment_sample_data
{
    uint alias;
    float q;
};

float3 environment_evaluate(float3 normalized_dir,
                                   out float pdf)
{
    // assuming lat long
    float u = atan2(normalized_dir.z, normalized_dir.x) * 0.5f * M_ONE_OVER_PI + 0.5f;
    u -= environment_rotation;
    if (u < 0.0f)
        u += 1.0f;
    const float v = acos(-normalized_dir.y) * M_ONE_OVER_PI;

    // get radiance and calculate pdf
    float3 t = environment_texture.SampleLevel(
        sampler_latlong, float2(u, v), /*mipmaplevel=*/ 0.0f, /*mipoffset=*/0).xyz;
    pdf = max(t.x, max(t.y, t.z)) * environment_inv_integral;
    return t * environment_intensity_factor;
}

float3 environment_sample(inout uint seed,
                                 out float3 to_light,
                                 out float pdf)
{
    float3 xi;
    xi.x = rnd(seed);
    xi.y = rnd(seed);
    xi.z = rnd(seed);


    uint width;
    uint height;
    environment_texture.GetDimensions(width, height);

    const uint size = width * height;
    const uint idx = min(uint(xi.x * float(size)), size - 1);

    uint env_idx;
    if (xi.y < environment_sample_buffer[idx].q)
    {
        env_idx = idx;
        xi.y /= environment_sample_buffer[idx].q;
    }
    else
    {
        env_idx = environment_sample_buffer[idx].alias;
        xi.y = (xi.y - environment_sample_buffer[idx].q) / (1.0f - environment_sample_buffer[idx].q);
    }

    const uint py = env_idx / width;
    const uint px = env_idx % width;

    // uniformly sample spherical area of pixel
    const float u = float(px + xi.y) / float(width);
    float u_rot = u + environment_rotation;
    if (u_rot > 1.0f)
        u_rot -= 1.0f;

    const float phi = u_rot * (2.0f * M_PI) - M_PI;
    float sin_phi;
    float cos_phi;
    sincos(phi, sin_phi, cos_phi);

    const float step_theta = M_PI / float(height);
    const float theta0 = float(py) * step_theta;
    const float cos_theta = cos(theta0) * (1.0f - xi.z) + cos(theta0 + step_theta) * xi.z;
    const float theta = acos(cos_theta);
    const float sin_theta = sin(theta);
    to_light = float3(cos_phi * sin_theta, -cos_theta, sin_phi * sin_theta);

    // lookup filtered value and calculate pdf
    const float v = theta * M_ONE_OVER_PI;
    float3 t = environment_texture.SampleLevel(sampler_latlong, float2(u, v), /*mipmaplevel=*/ 0.0f, /*mipoffset=*/0).xyz;
    pdf = max(t.x, max(t.y, t.z)) * environment_inv_integral;
    return t * environment_intensity_factor;
}


// selects one light source randomly
public float3 sample_lights(inout Shading_state_material state, out float3 to_light, out float light_pdf, inout uint seed)
{
    unmodified(state);

    float p_select_light = 1.0f;
    if (point_light_enabled != 0)
    {
        // keep it simple and use either point light or environment light, each with the same
        // probability. If the environment factor is zero, we always use the point light
        // Note: see also miss shader
        p_select_light = environment_intensity_factor > 0.0f ? 0.5f : 1.0f;

        // in general, you would select the light depending on the importance of it
        // e.g. by incorporating their luminance

        // randomly select one of the lights
        if (rnd(seed) <= p_select_light)
        {
            light_pdf = DIRAC; // infinity

            // compute light direction and distance
			to_light = point_light_position - state.position.val;

            const float inv_distance2 = 1.0f / dot(to_light, to_light);
            to_light *= sqrt(inv_distance2);

            return point_light_intensity * inv_distance2 * 0.25f * M_ONE_OVER_PI / p_select_light;
        }

        // probability to select the environment instead
        p_select_light = (1.0f - p_select_light);
    }

    // light from the environment
    float3 radiance = environment_sample(seed, to_light, light_pdf);

    // return radiance over pdf
    light_pdf *= p_select_light;
    return radiance / light_pdf;
}