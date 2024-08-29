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

import common;
import lighting;
import setup;
import material;
import types;

// ------------------------------------------------------------------------------------------------
// MDL hit group shader
// ------------------------------------------------------------------------------------------------

[shader("closesthit")]
void closesthit(inout RadianceHitInfo payload, Attributes attrib)
{
    // setup MDL state
    Shading_state_material mdl_state;
    setup_mdl_shading_state(mdl_state, attrib);

	mdl_init(mdl_state);

    // thin-walled materials are allowed to have a different back side
    // buy the can't have volumetric properties
	const bool thin_walled = mdl_thin_walled(mdl_state);

    // for thin-walled materials there is no 'inside'
    const bool inside = has_flag(payload.flags, FLAG_INSIDE);
    const float ior1 = (inside && !thin_walled) ? -1.0f :  1.0f;
    const float ior2 = (inside && !thin_walled) ?  1.0f : -1.0f;

    // Sample Light Sources for next event estimation
    //---------------------------------------------------------------------------------------------

    float3 to_light;
    float light_pdf;
    float3 radiance_over_pdf = sample_lights(mdl_state, to_light, light_pdf, payload.seed);

    // do not next event estimation (but delay the adding of contribution)
    float3 contribution = float3(0.0f, 0.0f, 0.0f);
    const bool next_event_valid = ((dot(to_light, mdl_state.geom_normal) > 0.0f) != inside)
                                  && (light_pdf != 0.0f)
                                  && !has_flag(payload.flags, FLAG_LAST_PATH_SEGMENT);

    if (next_event_valid)
    {
        // call generated mdl function to evaluate the scattering BSDF
        Bsdf_evaluate_data eval_data = (Bsdf_evaluate_data)0;
        eval_data.ior1 = ior1;
        eval_data.ior2 = ior2;
        eval_data.k1 = -WorldRayDirection();
        eval_data.k2 = to_light;

        // Always use surface scattering
		mdl_surface_scattering_evaluate(eval_data, mdl_state);

        // compute lighting for this light
        if(eval_data.pdf > 0.0f)
        {
            const float mis_weight = (light_pdf == DIRAC)
                ? 1.0f
                : light_pdf / (light_pdf + eval_data.pdf);

            // sample weight
            const float3 w = payload.weight * radiance_over_pdf * mis_weight;
			contribution += w * eval_data.bsdf_diffuse;
			contribution += w * eval_data.bsdf_glossy;
        }
    }

    // Sample direction of the next ray
    //---------------------------------------------------------------------------------------------

    // not a camera ray anymore
    remove_flag(payload.flags, FLAG_CAMERA_RAY);

    Bsdf_sample_data sample_data = (Bsdf_sample_data) 0;
    sample_data.ior1 = ior1;                    // IOR current medium
    sample_data.ior2 = ior2;                    // IOR other side
    sample_data.k1 = -WorldRayDirection();      // outgoing direction
    sample_data.xi = rnd4(payload.seed);        // random sample number

    // Always use surface scattering
	mdl_surface_scattering_sample(sample_data, mdl_state);

    // stop on absorb
    if (sample_data.event_type == BSDF_EVENT_ABSORB)
    {
        add_flag(payload.flags, FLAG_DONE);
        // no not return here, we need to do next event estimation first
    }
    else
    {
        // flip inside/outside on transmission
        // setup next path segment
        payload.ray_direction_next = sample_data.k2;
        payload.weight *= sample_data.bsdf_over_pdf;
        if ((sample_data.event_type & BSDF_EVENT_TRANSMISSION) != 0)
        {
            toggle_flag(payload.flags, FLAG_INSIDE);
            // continue on the opposite side
			payload.ray_origin_next = offset_ray(mdl_state.position.val, -mdl_state.geom_normal);
        }
        else
        {
            // continue on the current side
			payload.ray_origin_next = offset_ray(mdl_state.position.val, mdl_state.geom_normal);
        }

        if ((sample_data.event_type & BSDF_EVENT_SPECULAR) != 0)
            payload.last_bsdf_pdf = DIRAC;
        else
            payload.last_bsdf_pdf = sample_data.pdf;
    }

    // Add contribution from next event estimation if not shadowed
    //---------------------------------------------------------------------------------------------

    // cast a shadow ray; assuming light is always outside
    RayDesc ray;
	ray.Origin = offset_ray(mdl_state.position.val, mdl_state.geom_normal * (inside ? -1.0f : 1.0f));
    ray.Direction = to_light;
    ray.TMin = 0.0f;
    ray.TMax = far_plane_distance;

    // prepare the ray and payload but trace at the end to reduce the amount of data that has
    // to be recovered after coming back from the shadow trace
    ShadowHitInfo shadow_payload;
    shadow_payload.isHit = false;
    shadow_payload.seed = payload.seed;

    TraceRay(
        SceneBVH,               // AccelerationStructure
        RAY_FLAG_NONE,          // RayFlags
        0xFF /* allow all */,   // InstanceInclusionMask
        RAY_TYPE_SHADOW,        // RayContributionToHitGroupIndex
        RAY_TYPE_COUNT,         // MultiplierForGeometryContributionToHitGroupIndex
        RAY_TYPE_SHADOW,        // MissShaderIndex
        ray,
        shadow_payload);

    // add to ray contribution from next event estimation
    if (!shadow_payload.isHit)
        payload.contribution += contribution;
}

[shader("anyhit")]
void anyhit(inout RadianceHitInfo payload, Attributes attrib)
{
    // back face culling
    if (has_flag(material_flags, MATERIAL_FLAG_SINGLE_SIDED)) {
        if (is_back_face()) {
            IgnoreHit();
            return;
        }
    }

    // early out if there is no opacity function
    if (has_flag(material_flags, MATERIAL_FLAG_OPAQUE))
        return;

    // setup MDL state
    Shading_state_material mdl_state;
    setup_mdl_shading_state(mdl_state, attrib);

    // evaluate the cutout opacity
    const float opacity = mdl_standalone_geometry_cutout_opacity(mdl_state);

    // do alpha blending the stochastically way
    if (rnd(payload.seed) < opacity)
        return;

    IgnoreHit();
}

// ------------------------------------------------------------------------------------------------
// MDL shadow group shader
// ------------------------------------------------------------------------------------------------

[shader("anyhit")]
void shadow(inout ShadowHitInfo payload, Attributes attrib)
{
    // back face culling
    if (has_flag(material_flags, MATERIAL_FLAG_SINGLE_SIDED)) {
        if (is_back_face()) {
            IgnoreHit();
            return;
        }
    }

    // early out if there is no opacity function
    if (has_flag(material_flags, MATERIAL_FLAG_OPAQUE))
    {
        payload.isHit = true;
        AcceptHitAndEndSearch();
        return;
    }

    // setup MDL state
    Shading_state_material mdl_state;
    setup_mdl_shading_state(mdl_state, attrib);

    // evaluate the cutout opacity
    const float opacity = mdl_standalone_geometry_cutout_opacity(mdl_state);

    // do alpha blending the stochastically way
    if (rnd(payload.seed) < opacity)
    {
        payload.isHit = true;
        AcceptHitAndEndSearch();
        return;
    }

    IgnoreHit();
}
