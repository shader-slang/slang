/******************************************************************************
 * Copyright (c) 2019-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

module common;

public static const float M_PI = 3.14159265358979323846;
public static const float M_ONE_OVER_PI = 0.318309886183790671538;
public static const float DIRAC = -1.0f;

public cbuffer SceneConstants : register(b1)
{
    public float total_time;
    public float delta_time;

    // (progressive) rendering
    public uint progressive_iteration;
    public uint max_ray_depth;
    public uint iterations_per_frame;

    // tone mapping
    public float exposure_compensation;
    public float firefly_clamp_threshold;
    public float burn_out;

    // one additional point light for illustration
    public uint point_light_enabled;
    public float3 point_light_position;
    public float3 point_light_intensity;

    // gamma correction while rendering to the frame buffer
    public float output_gamma_correction;

    // environment light
    public float environment_intensity_factor;
    public float environment_inv_integral;

    // when auxiliary buffers are enabled, this index is used to select to one to display
    public uint display_buffer_index;

    // enable animation mode, progression is limited, mdl state will have an animation_time
    public uint enable_animiation;

    /// replace the background with a constant color when visible to the camera
    public uint background_color_enabled;
    public float3 background_color;

    /// uv transformations
    public float2 uv_scale;
    public float2 uv_offset;
    public uint uv_repeat;
    public uint uv_saturate;

    // rotation of the environment [0, 1]
    public float environment_rotation;

    // defines the scale of the scene
    public float meters_per_scene_unit;

    // far plane that defines the maximum ray travel distance
    public float far_plane_distance;
}


// Ray typed, has to match with CPU version
public typedef uint RayType;
public static const RayType RAY_TYPE_RADIANCE = 0;
public static const RayType RAY_TYPE_SHADOW = 1;
public static const RayType RAY_TYPE_COUNT = (RAY_TYPE_SHADOW + 1);

public typedef uint RadianceHitInfoFlags;
public static const RadianceHitInfoFlags FLAG_NONE = 0;
public static const RadianceHitInfoFlags FLAG_INSIDE = 1;
public static const RadianceHitInfoFlags FLAG_DONE = 2;
public static const RadianceHitInfoFlags FLAG_FIRST_PATH_SEGMENT = 4;
public static const RadianceHitInfoFlags FLAG_LAST_PATH_SEGMENT = 8;
public static const RadianceHitInfoFlags FLAG_CAMERA_RAY = 16;

public void add_flag(inout uint flags, uint to_add)
{
    flags |= to_add; 
}

public void toggle_flag(inout uint flags, uint to_toggle)
{
    flags ^= to_toggle;
}

public void remove_flag(inout uint flags, uint to_remove)
{
    flags &= ~to_remove; 
}

public bool has_flag(int flags, uint to_check)
{
    return (flags & to_check) != 0;
}

// payload for RAY_TYPE_RADIANCE
public struct RadianceHitInfo
{
    public float3 contribution;
    public float3 weight;

    public float3 ray_origin_next;
    public float3 ray_direction_next;

    public uint seed;
    public float last_bsdf_pdf;
    public uint flags;
};

// payload for RAY_TYPE_SHADOW
public struct ShadowHitInfo
{
    public bool isHit;
    public uint seed;
};

// Attributes output by the ray tracing when hitting a surface
public struct Attributes
{
    public float2 bary;
};

// Helper to make NaN and INF values visible in the output image.
float3 encode_errors(float3 color)
{
    return any(isnan(color) | isinf(color)) ? float3(0.0f, 0.0f, 1.0e+30f) : color;
}

//-------------------------------------------------------------------------------------------------
// Scene Data API
//-------------------------------------------------------------------------------------------------

/// interpolation of the data over the primitive
public typedef uint SceneDataInterpolationMode;
public static const SceneDataInterpolationMode SCENE_DATA_INTERPOLATION_MODE_NONE = 0;
public static const SceneDataInterpolationMode SCENE_DATA_INTERPOLATION_MODE_LINEAR = 1;
public static const SceneDataInterpolationMode SCENE_DATA_INTERPOLATION_MODE_NEAREST = 2;

/// Scope a scene data element belongs to
public typedef uint SceneDataKind;
public static const SceneDataKind SCENE_DATA_KIND_NONE = 0;
public static const SceneDataKind SCENE_DATA_KIND_VERTEX = 1;
public static const SceneDataKind SCENE_DATA_KIND_INSTANCE = 2;

/// Basic element type of the scene data
public typedef uint SceneDataElementType;
public static const SceneDataElementType SCENE_DATA_ELEMENT_TYPE_FLOAT = 0;
public static const SceneDataElementType SCENE_DATA_ELEMENT_TYPE_INT = 1;
public static const SceneDataElementType SCENE_DATA_ELEMENT_TYPE_COLOR = 2;

// Infos about the interleaved vertex layout (compressed)
public struct SceneDataInfo
{
    /// Scope a scene data element belongs to (4 bits)
    public inline SceneDataKind GetKind()
    {
        return (SceneDataKind) ((packed_data.x & 0xF0000000u) >> 28);
    }

    /// Basic element type of the scene data (4 bits)
    public inline SceneDataElementType GetElementType()
    {
        return (SceneDataElementType) ((packed_data.x & 0x0F000000u) >> 24);
    }

    /// Interpolation of the data over the primitive (4 bits)
    public inline SceneDataInterpolationMode GetInterpolationMode()
    {
        return (SceneDataInterpolationMode) ((packed_data.x & 0x00F00000u) >> 20);
    }

    /// Indicates whether there the scene data is uniform. (1 bit)
    public bool GetUniform()
    {
        return (packed_data.x & 0x00010000u) > 0;
    }

    /// Offset between two elements. For interleaved vertex buffers, this is the vertex size in byte.
    /// For non-interleaved buffers, this is the element size in byte. (16 bit)
    public uint GetByteStride()
    {
        return (packed_data.x & 0x0000FFFFu);
    }

    /// The offset to the data element within an interleaved vertex buffer, or the absolute
    /// offset to the base (e.g. of the geometry data) in non-interleaved buffers
    public uint GetByteOffset()
    {
        return packed_data.y;
    }

    // use getter function to unpack, see scene.cpp for corresponding c++ code for packing
    uint2 packed_data;
};

// renderer state object that is passed to mdl runtime functions
public struct DXRRendererState
{
    // index offset for the first info object relevant for this geometry
    public uint scene_data_info_offset;

    // global offset in the data buffer (for object, geometry, ...)
    public uint scene_data_geometry_byte_offset;

    // vertex indices if the hit triangle (from index buffer)
    public uint3 hit_vertex_indices;

    // barycentric coordinates of the hit point within the triangle
    public float3 barycentric;

    // true if the hit point was on the backside of a triangle, based on geom normal and ray direction
    public bool hit_backface;
};

// use this structure as renderer state in the MDL shading state material
public typedef DXRRendererState RENDERER_STATE_TYPE; 

// Positions, normals, and tangents are mandatory for this renderer. The vertex buffer always
// contains this data at the beginning of the (interleaved) per vertex data.
public typedef uint VertexByteOffset;
public static const VertexByteOffset VERT_BYTEOFFSET_POSITION = 0;
public static const VertexByteOffset VERT_BYTEOFFSET_NORMAL = 12;
public static const VertexByteOffset VERT_BYTEOFFSET_TANGENT = 24;

//-------------------------------------------------------------------------------------------------
// random number generator based on the Optix SDK
//-------------------------------------------------------------------------------------------------
uint tea(uint N, uint val0, uint val1)
{
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;

    for (uint n = 0; n < N; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }

    return v0;
}

// Generate random uint in [0, 2^24)
uint lcg(inout uint prev)
{
    const uint LCG_A = 1664525u;
    const uint LCG_C = 1013904223u;
    prev = (LCG_A * prev + LCG_C);
    return prev & 0x00FFFFFF;
}

// Generate random float in [0, 1)
public float rnd(inout uint prev)
{
    return ((float) lcg(prev) / (float) 0x01000000);
}

public float2 rnd2(inout uint prev)
{
    return float2((float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000);
}

public float3 rnd3(inout uint prev)
{
    return float3((float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000);
}

public float4 rnd4(inout uint prev)
{
    return float4((float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000,
                  (float) lcg(prev) / (float) 0x01000000);
}

//-------------------------------------------------------------------------------------------------
// Math helper
//-------------------------------------------------------------------------------------------------

// convert float4x3 to 4x4, to be compatible with the slang compiler
public float4x4 to4x4(float3x4 source)
{
    return float4x4(source[0], source[1], source[2], float4(0.0f, 0.0f, 0.0f, 1.0f));
}

//-------------------------------------------------------------------------------------------------
// Avoiding self intersections (see Ray Tracing Gems, Ch. 6)
//-------------------------------------------------------------------------------------------------

public float3 offset_ray(const float3 p, const float3 n)
{
    const float origin = 1.0f / 32.0f;
    const float float_scale = 1.0f / 65536.0f;
    const float int_scale = 256.0f;

    const int3 of_i = int3(int_scale * n);

    float3 p_i = float3(asfloat(asint(p.x) + ((p.x < 0.0f) ? -of_i.x : of_i.x)),
                        asfloat(asint(p.y) + ((p.y < 0.0f) ? -of_i.y : of_i.y)),
                        asfloat(asint(p.z) + ((p.z < 0.0f) ? -of_i.z : of_i.z)));

    return float3(abs(p.x) < origin ? p.x + float_scale * n.x : p_i.x,
                  abs(p.y) < origin ? p.y + float_scale * n.y : p_i.y,
                  abs(p.z) < origin ? p.z + float_scale * n.z : p_i.z);
}