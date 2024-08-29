module setup;

import common;
import types;
import runtime;

public typedef int MaterialFlags;
public static const MaterialFlags MATERIAL_FLAG_NONE = 0;
public static const MaterialFlags MATERIAL_FLAG_OPAQUE = 1 << 0; // allows to skip opacity evaluation
public static const MaterialFlags MATERIAL_FLAG_SINGLE_SIDED = 1 << 1; // geometry is only visible from the front side

// ------------------------------------------------------------------------------------------------
// defined in the global root signature
// ------------------------------------------------------------------------------------------------

// Ray tracing acceleration structure, accessed as a SRV
public RaytracingAccelerationStructure SceneBVH : register(t0, space0);

// ------------------------------------------------------------------------------------------------
// defined in the local root signature
// ------------------------------------------------------------------------------------------------

// mesh data
public StructuredBuffer<uint> indices: register(t2, space0);

// geometry data
// as long as there are only a few values here, place them directly instead of a constant buffer
cbuffer _Geometry_constants_0 : register(b2, space0) { uint geometry_vertex_buffer_byte_offset; }
cbuffer _Geometry_constants_1 : register(b3, space0) { uint geometry_vertex_stride; }
cbuffer _Geometry_constants_2 : register(b4, space0) { uint geometry_index_offset; }
cbuffer _Geometry_constants_3 : register(b5, space0) { uint geometry_scene_data_info_offset; }

public cbuffer Material_constants : register(b0, space3)
{
    // shared for all material compiled from the same MDL material
    // - none -

    // individual properties of the different material instances
    public int material_id;
    public uint material_flags;
}

// ------------------------------------------------------------------------------------------------
// helper
// ------------------------------------------------------------------------------------------------


// fetch vertex data with known layout
public float3 fetch_vertex_data_float3(const uint index, const uint byte_offset)
{
    const uint address =
        geometry_vertex_buffer_byte_offset + // base address for this part of the mesh
        geometry_vertex_stride * index +     // offset to the selected vertex
        byte_offset;                         // offset within the vertex

    return asfloat(vertices.Load3(address));
}

// fetch vertex data with known layout
public float4 fetch_vertex_data_float4(const uint index, const uint byte_offset)
{
    const uint address =
        geometry_vertex_buffer_byte_offset + // base address for this part of the mesh
        geometry_vertex_stride * index +     // offset to the selected vertex
        byte_offset;                         // offset within the vertex

    return asfloat(vertices.Load4(address));
}

public bool is_back_face()
{
    // get vertex indices for the hit triangle
    const uint index_offset = 3 * PrimitiveIndex() + geometry_index_offset;
    const uint3 vertex_indices = uint3(indices[index_offset + 0],
                                       indices[index_offset + 1],
                                       indices[index_offset + 2]);

    // get position of the hit point
    const float3 pos0 = fetch_vertex_data_float3(vertex_indices.x, VERT_BYTEOFFSET_POSITION);
    const float3 pos1 = fetch_vertex_data_float3(vertex_indices.y, VERT_BYTEOFFSET_POSITION);
    const float3 pos2 = fetch_vertex_data_float3(vertex_indices.z, VERT_BYTEOFFSET_POSITION);

    // compute geometry normal and check for back face hit
    const float3 geom_normal = normalize(cross(pos1 - pos0, pos2 - pos0));
    return dot(geom_normal, ObjectRayDirection()) > 0.0f;
}

public void setup_mdl_shading_state(out Shading_state_material mdl_state, Attributes attrib)
{
    // get vertex indices for the hit triangle
    const uint index_offset = 3 * PrimitiveIndex() + geometry_index_offset;
    const uint3 vertex_indices = uint3(indices[index_offset + 0],
                                       indices[index_offset + 1],
                                       indices[index_offset + 2]);

    // coordinates inside the triangle
    const float3 barycentric = float3(1.0f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    // mesh transformations
    const float4x4 object_to_world = to4x4(ObjectToWorld());
    const float4x4 world_to_object = to4x4(WorldToObject());

    // get position of the hit point
    const float3 pos0 = fetch_vertex_data_float3(vertex_indices.x, VERT_BYTEOFFSET_POSITION);
    const float3 pos1 = fetch_vertex_data_float3(vertex_indices.y, VERT_BYTEOFFSET_POSITION);
    const float3 pos2 = fetch_vertex_data_float3(vertex_indices.z, VERT_BYTEOFFSET_POSITION);
    float3 hit_position = pos0 * barycentric.x + pos1 * barycentric.y + pos2 * barycentric.z;
    hit_position = mul(object_to_world, float4(hit_position, 1)).xyz;

    // get normals (geometry normal and interpolated vertex normal)
    const float3 geom_normal = normalize(cross(pos1 - pos0, pos2 - pos0));
    const float3 normal = normalize(fetch_vertex_data_float3(vertex_indices.x, VERT_BYTEOFFSET_NORMAL) * barycentric.x
                                    + fetch_vertex_data_float3(vertex_indices.y, VERT_BYTEOFFSET_NORMAL) * barycentric.y
                                    + fetch_vertex_data_float3(vertex_indices.z, VERT_BYTEOFFSET_NORMAL) * barycentric.z);

    // transform normals using inverse transpose
    // -  world_to_object = object_to_world^-1
    // -  mul(v, world_to_object) = mul(object_to_world^-T, v)
    float3 world_geom_normal = normalize(mul(float4(geom_normal, 0), world_to_object).xyz);
    float3 world_normal = normalize(mul(float4(normal, 0), world_to_object).xyz);

    // reconstruct tangent frame from vertex data
    float3 world_tangent, world_binormal;
    float4 tangent0 = fetch_vertex_data_float4(vertex_indices.x, VERT_BYTEOFFSET_TANGENT) * barycentric.x
                      + fetch_vertex_data_float4(vertex_indices.y, VERT_BYTEOFFSET_TANGENT) * barycentric.y
                      + fetch_vertex_data_float4(vertex_indices.z, VERT_BYTEOFFSET_TANGENT) * barycentric.z;
    tangent0.xyz = normalize(tangent0.xyz);
    world_tangent = normalize(mul(object_to_world, float4(tangent0.xyz, 0)).xyz);
    world_tangent = normalize(world_tangent - dot(world_tangent, world_normal) * world_normal);
    world_binormal = cross(world_normal, world_tangent) * tangent0.w;

    // flip normals to the side of the incident ray
    const bool backfacing_primitive = dot(world_geom_normal, WorldRayDirection()) > 0.0;
    if (backfacing_primitive)
        world_geom_normal *= -1.0f;

    if (dot(world_normal, WorldRayDirection()) > 0.0)
        world_normal *= -1.0f;

    // handle low tessellated meshes with smooth normals
    float3 k2 = reflect(WorldRayDirection(), world_normal);
    if (dot(world_geom_normal, k2) < 0.0f)
        world_normal = world_geom_normal;

    // fill the actual state fields used by MD
    mdl_state.normal = world_normal;
    mdl_state.geom_normal = world_geom_normal;
	// currently not supported
	mdl_state.position.val = hit_position;
	mdl_state.position.dx = float3(0, 0, 0);
	mdl_state.position.dy = float3(0, 0, 0);
    mdl_state.animation_time = (enable_animiation >= 0) ? total_time : 0.0f;
    mdl_state.tangent_u[0] = world_tangent;
    mdl_state.tangent_v[0] = world_binormal;
    mdl_state.ro_data_segment_offset = 0;
    mdl_state.world_to_object = world_to_object;
    mdl_state.object_to_world = object_to_world;
    mdl_state.object_id = 0;
    mdl_state.meters_per_scene_unit = meters_per_scene_unit;
    mdl_state.arg_block_offset = 0;

    // fill the renderer state information
    mdl_state.renderer_state.scene_data_info_offset = geometry_scene_data_info_offset;
    mdl_state.renderer_state.scene_data_geometry_byte_offset = geometry_vertex_buffer_byte_offset;
    mdl_state.renderer_state.hit_vertex_indices = vertex_indices;
    mdl_state.renderer_state.barycentric = barycentric;
    mdl_state.renderer_state.hit_backface = backfacing_primitive;

    // get texture coordinates using a manually added scene data element with the scene data id
    // defined as `SCENE_DATA_ID_TEXCOORD_0`
    // (see end of target code generation on application side)
    float2 texcoord0 = scene_data_lookup_float2(mdl_state, 1 /* SCENE_DATA_ID_TEXCOORD_0 */, float2(0.0f, 0.0f), false);

    // apply uv transformations
    texcoord0 = texcoord0 * uv_scale + uv_offset;
    if (uv_repeat != 0)
        texcoord0 = texcoord0 - floor(texcoord0);
    if (uv_saturate != 0)
        texcoord0 = saturate(texcoord0);

	// would make sense in a rasterizer. for a ray tracers this is not straight forward
	mdl_state.text_coords[0].val = float3(texcoord0, 0);
	mdl_state.text_coords[0].dx = float3(0, 0, 0);
	mdl_state.text_coords[0].dy = float3(0, 0, 0);
}

