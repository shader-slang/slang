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

module types;

import common;

public struct Derived_float {
    public float val;
    public float dx;
    public float dy;
};

public struct Derived_float2 {
    public float2 val;
    public float2 dx;
    public float2 dy;
};

public struct Derived_float3 {
    public float3 val;
    public float3 dx;
    public float3 dy;
};

public struct Derived_float4 {
    public float4 val;
    public float4 dx;
    public float4 dy;
};

public struct Shading_state_material
{
    /// The result of state::normal().
    /// It represents the shading normal as determined by the renderer.
    /// This field will be updated to the result of \c "geometry.normal" by BSDF init functions,
    /// if requested during code generation.
    public float3            normal;

    /// The result of state::geometry_normal().
    /// It represents the geometry normal as determined by the renderer.
    public float3            geom_normal;

    /// The result of state::position().
    /// It represents the position where the material should be evaluated.
    public Derived_float3    position;

    /// The result of state::animation_time().
    /// It represents the time of the current sample in seconds.
    public float             animation_time;

    /// An array containing the results of state::texture_coordinate(i).
    /// The i-th entry represents the texture coordinates of the i-th texture space at the
    /// current position.
    public Derived_float3    text_coords[1];

    /// An array containing the results of state::texture_tangent_u(i).
    /// The i-th entry represents the texture tangent vector of the i-th texture space at the
    /// current position, which points in the direction of the projection of the tangent to the
    /// positive u axis of this texture space onto the plane defined by the original
    /// surface normal.
    public float3            tangent_u[1];

    /// An array containing the results of state::texture_tangent_v(i).
    /// The i-th entry represents the texture bitangent vector of the i-th texture space at the
    /// current position, which points in the general direction of the positive v axis of this
    /// texture space, but is orthogonal to both the original surface normal and the tangent
    /// of this texture space.
    public float3            tangent_v[1];

    /// The texture results lookup table.
    /// Values will be modified by BSDF init functions to avoid duplicate texture fetches
    /// and duplicate calculation of values.
    /// This field is only relevant for code generated with
    /// #mi::neuraylib::IMdl_backend::translate_material_df() or
    /// #mi::neuraylib::ILink_unit::add_material_df(). In other cases this may be NULL.
    public float4            text_results[32];
    
    /// An offset for accesses to the read-only data segment. Will be added before
    /// calling any "mdl_read_rodata_as_*" function.
    /// The data of the read-only data segment is accessible as the first segment
    /// (index 0) returned by #mi::neuraylib::ITarget_code::get_ro_data_segment_data().
    public uint ro_data_segment_offset;

    /// A 4x4 transformation matrix transforming from world to object coordinates.
    /// It is used by the state::transform_*() methods.
    /// This field is only used if the uniform state is included.
    public float4x4          world_to_object;

    /// A 4x4 transformation matrix transforming from object to world coordinates.
    /// It is used by the state::transform_*() methods.
    /// This field is only used if the uniform state is included.
    public float4x4          object_to_world;

    /// The result of state::object_id().
    /// It is an application-specific identifier of the hit object as provided in a scene.
    /// It can be used to make instanced objects look different in spite of the same used material.
    /// This field is only used if the uniform state is included.
    public uint              object_id;

    /// The result of state::meters_per_scene_unit().
    /// The field is only used if the \c "fold_meters_per_scene_unit" option is set to false.
    /// Otherwise, the value of the \c "meters_per_scene_unit" option will be used in the code.
    public float             meters_per_scene_unit;

    /// An offset to add to any argument block read accesses.
    public uint              arg_block_offset;

    /// A user-defined public structure that allows to pass renderer information; for instance about the
    /// hit-point or buffer references; to mdl run-time functions. This is especially required for
    /// the scene data access. The fields of this public structure are not altered by generated code.
    public RENDERER_STATE_TYPE renderer_state;
};

/// The texture wrap modes as defined by \c tex::wrap_mode in the MDL specification.
public typedef int Tex_wrap_mode;
public static const Tex_wrap_mode TEX_WRAP_CLAMP = 0;
public static const Tex_wrap_mode TEX_WRAP_REPEAT = 1;
public static const Tex_wrap_mode TEX_WRAP_MIRRORED_REPEAT = 2;
public static const Tex_wrap_mode TEX_WRAP_CLIP = 3;

/// The type of events created by BSDF importance sampling.
public typedef int Bsdf_event_type;
public static const Bsdf_event_type BSDF_EVENT_ABSORB = 0;
public static const Bsdf_event_type BSDF_EVENT_DIFFUSE = 1;
public static const Bsdf_event_type BSDF_EVENT_GLOSSY = (1 << 1);
public static const Bsdf_event_type BSDF_EVENT_SPECULAR = (1 << 2);
public static const Bsdf_event_type BSDF_EVENT_REFLECTION = (1 << 3);
public static const Bsdf_event_type BSDF_EVENT_TRANSMISSION = (1 << 4);

public static const Bsdf_event_type BSDF_EVENT_DIFFUSE_REFLECTION = (BSDF_EVENT_DIFFUSE  | BSDF_EVENT_REFLECTION);
public static const Bsdf_event_type BSDF_EVENT_DIFFUSE_TRANSMISSION = (BSDF_EVENT_DIFFUSE  | BSDF_EVENT_TRANSMISSION);
public static const Bsdf_event_type BSDF_EVENT_GLOSSY_REFLECTION = (BSDF_EVENT_GLOSSY   | BSDF_EVENT_REFLECTION);
public static const Bsdf_event_type BSDF_EVENT_GLOSSY_TRANSMISSION = (BSDF_EVENT_GLOSSY   | BSDF_EVENT_TRANSMISSION);
public static const Bsdf_event_type BSDF_EVENT_SPECULAR_REFLECTION = (BSDF_EVENT_SPECULAR | BSDF_EVENT_REFLECTION);
public static const Bsdf_event_type BSDF_EVENT_SPECULAR_TRANSMISSION = (BSDF_EVENT_SPECULAR | BSDF_EVENT_TRANSMISSION);
public static const Bsdf_event_type BSDF_EVENT_FORCE_32_BIT = 0xffffffffU;

public typedef int Edf_event_type;
public static const Edf_event_type EDF_EVENT_NONE = 0;
public static const Edf_event_type EDF_EVENT_EMISSION = 1;
public static const Edf_event_type EDF_EVENT_FORCE_32_BIT = 0xffffffffU;

/// MBSDFs can consist of two parts, which can be selected using this enumeration.
public typedef int Mbsdf_part;
public static const Mbsdf_part MBSDF_DATA_REFLECTION = 0;
public static const Mbsdf_part MBSDF_DATA_TRANSMISSION = 1;

/// Input and output public structure for BSDF sampling data.
public struct Bsdf_sample_data {
    public float3 ior1;                    ///< mutual input: IOR current medium
    public float3 ior2;                    ///< mutual input: IOR other side
    public float3 k1;                      ///< mutual input: outgoing direction

    public float3 k2;                      ///< output: incoming direction
    public float4 xi;                      ///< input: pseudo-random sample numbers in range [0, 1)
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
    public float3 bsdf_over_pdf;           ///< output: bsdf * dot(normal, k2) / pdf
    public Bsdf_event_type event_type;     ///< output: the type of event for the generated sample
    public int handle;                     ///< output: handle of the sampled elemental BSDF (lobe)
};

/// Input and output public structure for BSDF evaluation data.
public struct Bsdf_evaluate_data {
    public float3 ior1;                    ///< mutual input: IOR current medium
    public float3 ior2;                    ///< mutual input: IOR other side
    public float3 k1;                      ///< mutual input: outgoing direction

    public float3 k2;                      ///< input: incoming direction
    public float3 bsdf_diffuse;            ///< output: (diffuse part of the) bsdf * dot(normal, k2)
    public float3 bsdf_glossy;             ///< output: (glossy part of the) bsdf * dot(normal, k2)
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
};

/// Input and output public structure for BSDF PDF calculation data.
public struct Bsdf_pdf_data {
    public float3 ior1;                    ///< mutual input: IOR current medium
    public float3 ior2;                    ///< mutual input: IOR other side
    public float3 k1;                      ///< mutual input: outgoing direction

    public float3 k2;                      ///< input: incoming direction
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
};

/// Input and output public structure for BSDF auxiliary calculation data.
public struct Bsdf_auxiliary_data {
    public float3 ior1;                    ///< mutual input: IOR current medium
    public float3 ior2;                    ///< mutual input: IOR other side
    public float3 k1;                      ///< mutual input: outgoing direction

	public float3 albedo_diffuse;          ///< output: (diffuse part of the) albedo
	public float3 albedo_glossy;           ///< output: (glossy part of the) albedo
	public float3 normal;                  ///< output: normal

    public float3 roughness;               ///< output: glossy rougness_u,
                                    ///  glossy roughness_v, bsdf_weight
};

/// Input and output public structure for EDF sampling data.
public struct Edf_sample_data
{
    public float4 xi;                      ///< input: pseudo-random sample numbers in range [0, 1)
    public float3 k1;                      ///< output: outgoing direction
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
    public float3 edf_over_pdf;            ///< output: edf * dot(normal,k1) / pdf
    public Edf_event_type event_type;      ///< output: the type of event for the generated sample
    public int handle;                     ///< output: handle of the sampled elemental EDF (lobe)
};

/// Input and output public structure for EDF evaluation data.
public struct Edf_evaluate_data
{
    public float3 k1;                      ///< input: outgoing direction
    public float cos;                      ///< output: dot(normal, k1)
    public float3 edf;                     ///< output: edf
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
};

/// Input and output public structure for EDF PDF calculation data.
public struct Edf_pdf_data
{
    public float3 k1;                      ///< input: outgoing direction
    public float pdf;                      ///< output: pdf (non-projected hemisphere)
};

/// Input and output public structure for EDF PDF calculation data.
public struct Edf_auxiliary_data
{
    public float3 k1;                      ///< input: outgoing direction
};
