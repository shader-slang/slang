---
layout: user-guide
---

Capability Atoms
============================

### Sections:

1. [Targets](#Targets)
2. [Stages](#Stages)
3. [Versions](#Versions)
4. [Extensions](#Extensions)
5. [Compound Capabilities](#Compound-Capabilities)
6. [Other](#Other)

<br>

Targets
----------------------


`textualTarget`<br>
> Represents a non-assembly code generation target.<br>

---


`hlsl`<br>
> Represents the HLSL code generation target.<br>

---


`glsl`<br>
> Represents the GLSL code generation target.<br>

---


`c`<br>
> Represents the C programming language code generation target.<br>

---


`cpp`<br>
> Represents the C++ programming language code generation target.<br>

---


`cuda`<br>
> Represents the CUDA code generation target.<br>

---


`metal`<br>
> Represents the Metal programming language code generation target.<br>

---


`spirv`<br>
> Represents the SPIR-V code generation target.<br>

---

<br>

Stages
----------------------


`vertex`<br>
> Vertex shader stage<br>

---


`fragment`<br>
> Fragment shader stage<br>

---


`compute`<br>
> Compute shader stage<br>

---


`hull`<br>
> Hull shader stage<br>

---


`domain`<br>
> Domain shader stage<br>

---


`geometry`<br>
> Geometry shader stage<br>

---


`pixel`<br>
> Pixel shader stage<br>

---


`tesscontrol`<br>
> Tessellation Control shader stage<br>

---


`tesseval`<br>
> Tessellation Evaluation shader stage<br>

---


`raygen`<br>
> Ray-Generation shader stage & ray-tracing capabilities<br>

---


`raygeneration`<br>
> Ray-Generation shader stage & ray-tracing capabilities<br>

---


`intersection`<br>
> Intersection shader stage & ray-tracing capabilities<br>

---


`anyhit`<br>
> Any-Hit shader stage & ray-tracing capabilities<br>

---


`closesthit`<br>
> Closest-Hit shader stage & ray-tracing capabilities<br>

---


`callable`<br>
> Callable shader stage & ray-tracing capabilities<br>

---


`miss`<br>
> Ray-Miss shader stage & ray-tracing capabilities<br>

---


`mesh`<br>
> Mesh shader stage & mesh shader capabilities<br>

---


`amplification`<br>
> Amplification shader stage & mesh shader capabilities<br>

---

<br>

Versions
----------------------


`glsl_spirv_1_0`<br>
> Represents SPIR-V 1.0 through glslang.<br>

---


`glsl_spirv_1_1`<br>
> Represents SPIR-V 1.1 through glslang.<br>

---


`glsl_spirv_1_2`<br>
> Represents SPIR-V 1.2 through glslang.<br>

---


`glsl_spirv_1_3`<br>
> Represents SPIR-V 1.3 through glslang.<br>

---


`glsl_spirv_1_4`<br>
> Represents SPIR-V 1.4 through glslang.<br>

---


`glsl_spirv_1_5`<br>
> Represents SPIR-V 1.5 through glslang.<br>

---


`glsl_spirv_1_6`<br>
> Represents SPIR-V 1.6 through glslang.<br>

---


`metallib_2_3`<br>
> Represents MetalLib 2.3.<br>

---


`metallib_2_4`<br>
> Represents MetalLib 2.4.<br>

---


`metallib_3_0`<br>
> Represents MetalLib 3.0.<br>

---


`metallib_3_1`<br>
> Represents MetalLib 3.1.<br>

---


`metallib_latest`<br>
> Represents the latest MetalLib version.<br>

---


`hlsl_nvapi`<br>
> Represents HLSL NVAPI support.<br>

---


`dxil_lib`<br>
> Represents capabilities required for DXIL Library compilation.<br>

---


`spirv_1_0`<br>
> Represents SPIR-V 1.0 version.<br>

---


`spirv_1_1`<br>

---


`spirv_1_2`<br>

---


`spirv_1_3`<br>

---


`spirv_1_4`<br>

---


`spirv_1_5`<br>

---


`spirv_1_6`<br>

---


`spirv_latest`<br>

---


`sm_4_0_version`<br>
> HLSL shader model 4.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_4_0`<br>
> HLSL shader model 4.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_4_1_version`<br>
> HLSL shader model 4.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_4_1`<br>
> HLSL shader model 4.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_5_0_version`<br>
> HLSL shader model 5.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_5_0`<br>
> HLSL shader model 5.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_5_1_version`<br>
> HLSL shader model 5.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_5_1`<br>
> HLSL shader model 5.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_0_version`<br>
> HLSL shader model 6.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_0`<br>
> HLSL shader model 6.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_1_version`<br>
> HLSL shader model 6.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_1`<br>
> HLSL shader model 6.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_2_version`<br>
> HLSL shader model 6.2 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_2`<br>
> HLSL shader model 6.2 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_3_version`<br>
> HLSL shader model 6.3 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_3`<br>
> HLSL shader model 6.3 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_4_version`<br>
> HLSL shader model 6.4 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_4`<br>
> HLSL shader model 6.4 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_5_version`<br>
> HLSL shader model 6.5 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_5`<br>
> HLSL shader model 6.5 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_6_version`<br>
> HLSL shader model 6.6 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_6`<br>
> HLSL shader model 6.6 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`sm_6_7_version`<br>
> HLSL shader model 6.7 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>

---


`sm_6_7`<br>
> HLSL shader model 6.7 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>

---


`GLSL_130`<br>
> GLSL 130 and related capabilities of other targets.<br>

---


`GLSL_140`<br>
> GLSL 140 and related capabilities of other targets.<br>

---


`GLSL_150`<br>
> GLSL 150 and related capabilities of other targets.<br>

---


`GLSL_330`<br>
> GLSL 330 and related capabilities of other targets.<br>

---


`GLSL_400`<br>
> GLSL 400 and related capabilities of other targets.<br>

---


`GLSL_410`<br>
> GLSL 410 and related capabilities of other targets.<br>

---


`GLSL_420`<br>
> GLSL 420 and related capabilities of other targets.<br>

---


`GLSL_430`<br>
> GLSL 430 and related capabilities of other targets.<br>

---


`GLSL_440`<br>
> GLSL 440 and related capabilities of other targets.<br>

---


`GLSL_450`<br>
> GLSL 450 and related capabilities of other targets.<br>

---


`GLSL_460`<br>
> GLSL 460 and related capabilities of other targets.<br>

---


`cuda_sm_1_0`<br>
> cuda 1.0 and related capabilities of other targets.<br>

---


`cuda_sm_2_0`<br>
> cuda 2.0 and related capabilities of other targets.<br>

---


`cuda_sm_3_0`<br>
> cuda 3.0 and related capabilities of other targets.<br>

---


`cuda_sm_3_5`<br>
> cuda 3.5 and related capabilities of other targets.<br>

---


`cuda_sm_4_0`<br>
> cuda 4.0 and related capabilities of other targets.<br>

---


`cuda_sm_5_0`<br>
> cuda 5.0 and related capabilities of other targets.<br>

---


`cuda_sm_6_0`<br>
> cuda 6.0 and related capabilities of other targets.<br>

---


`cuda_sm_7_0`<br>
> cuda 7.0 and related capabilities of other targets.<br>

---


`cuda_sm_8_0`<br>
> cuda 8.0 and related capabilities of other targets.<br>

---


`cuda_sm_9_0`<br>
> cuda 9.0 and related capabilities of other targets.<br>

---

<br>

Extensions
----------------------


`SPV_EXT_fragment_shader_interlock`<br>
> Represents the SPIR-V extension for fragment shader interlock operations.<br>

---


`SPV_EXT_physical_storage_buffer`<br>
> Represents the SPIR-V extension for physical storage buffer.<br>

---


`SPV_EXT_fragment_fully_covered`<br>
> Represents the SPIR-V extension for SPV_EXT_fragment_fully_covered.<br>

---


`SPV_EXT_descriptor_indexing`<br>
> Represents the SPIR-V extension for descriptor indexing.<br>

---


`SPV_EXT_shader_atomic_float_add`<br>
> Represents the SPIR-V extension for atomic float add operations.<br>

---


`SPV_EXT_shader_atomic_float16_add`<br>
> Represents the SPIR-V extension for atomic float16 add operations.<br>

---


`SPV_EXT_shader_atomic_float_min_max`<br>
> Represents the SPIR-V extension for atomic float min/max operations.<br>

---


`SPV_EXT_mesh_shader`<br>
> Represents the SPIR-V extension for mesh shaders.<br>

---


`SPV_EXT_demote_to_helper_invocation`<br>
> Represents the SPIR-V extension for demoting to helper invocation.<br>

---


`SPV_KHR_fragment_shader_barycentric`<br>
> Represents the SPIR-V extension for fragment shader barycentric.<br>

---


`SPV_KHR_non_semantic_info`<br>
> Represents the SPIR-V extension for non-semantic information.<br>

---


`SPV_KHR_ray_tracing`<br>
> Represents the SPIR-V extension for ray tracing.<br>

---


`SPV_KHR_ray_query`<br>
> Represents the SPIR-V extension for ray queries.<br>

---


`SPV_KHR_ray_tracing_position_fetch`<br>
> Represents the SPIR-V extension for ray tracing position fetch.<br>
> Should be used with either SPV_KHR_ray_query or SPV_KHR_ray_tracing.<br>

---


`SPV_KHR_shader_clock`<br>
> Represents the SPIR-V extension for shader clock.<br>

---


`SPV_NV_shader_subgroup_partitioned`<br>
> Represents the SPIR-V extension for shader subgroup partitioned.<br>

---


`SPV_NV_ray_tracing_motion_blur`<br>
> Represents the SPIR-V extension for ray tracing motion blur.<br>

---


`SPV_NV_shader_invocation_reorder`<br>
> Represents the SPIR-V extension for shader invocation reorder.<br>
> Requires SPV_KHR_ray_tracing.<br>

---


`SPV_NV_shader_image_footprint`<br>
> Represents the SPIR-V extension for shader image footprint.<br>

---


`SPV_NV_compute_shader_derivatives`<br>
> Represents the SPIR-V extension for compute shader derivatives.<br>

---


`SPV_GOOGLE_user_type`<br>
> Represents the SPIR-V extension for SPV_GOOGLE_user_type.<br>

---


`spvAtomicFloat32AddEXT`<br>
> Represents the SPIR-V capability for atomic float 32 add operations.<br>

---


`spvAtomicFloat16AddEXT`<br>
> Represents the SPIR-V capability for atomic float 16 add operations.<br>

---


`spvInt64Atomics`<br>
> Represents the SPIR-V capability for 64-bit integer atomics.<br>

---


`spvAtomicFloat32MinMaxEXT`<br>
> Represents the SPIR-V capability for atomic float 32 min/max operations.<br>

---


`spvAtomicFloat16MinMaxEXT`<br>
> Represents the SPIR-V capability for atomic float 16 min/max operations.<br>

---


`spvDerivativeControl`<br>
> Represents the SPIR-V capability for 'derivative control' operations.<br>

---


`spvImageQuery`<br>
> Represents the SPIR-V capability for image query operations.<br>

---


`spvImageGatherExtended`<br>
> Represents the SPIR-V capability for extended image gather operations.<br>

---


`spvSparseResidency`<br>
> Represents the SPIR-V capability for sparse residency.<br>

---


`spvImageFootprintNV`<br>
> Represents the SPIR-V capability for image footprint.<br>

---


`spvMinLod`<br>
> Represents the SPIR-V capability for using minimum LOD operations.<br>

---


`spvFragmentShaderPixelInterlockEXT`<br>
> Represents the SPIR-V capability for using SPV_EXT_fragment_shader_interlock.<br>

---


`spvFragmentBarycentricKHR`<br>
> Represents the SPIR-V capability for using SPV_KHR_fragment_shader_barycentric.<br>

---


`spvFragmentFullyCoveredEXT`<br>
> Represents the SPIR-V capability for using SPV_EXT_fragment_fully_covered functionality.<br>

---


`spvGroupNonUniformBallot`<br>
> Represents the SPIR-V capability for group non-uniform ballot operations.<br>

---


`spvGroupNonUniformShuffle`<br>
> Represents the SPIR-V capability for group non-uniform shuffle operations.<br>

---


`spvGroupNonUniformArithmetic`<br>
> Represents the SPIR-V capability for group non-uniform arithmetic operations.<br>

---


`spvGroupNonUniformQuad`<br>
> Represents the SPIR-V capability for group non-uniform quad operations.<br>

---


`spvGroupNonUniformVote`<br>
> Represents the SPIR-V capability for group non-uniform vote operations.<br>

---


`spvGroupNonUniformPartitionedNV`<br>
> Represents the SPIR-V capability for group non-uniform partitioned operations.<br>

---


`spvRayTracingMotionBlurNV`<br>
> Represents the SPIR-V capability for ray tracing motion blur.<br>

---


`spvMeshShadingEXT`<br>
> Represents the SPIR-V capability for mesh shading.<br>

---


`spvRayTracingKHR`<br>
> Represents the SPIR-V capability for ray tracing.<br>

---


`spvRayTracingPositionFetchKHR`<br>
> Represents the SPIR-V capability for ray tracing position fetch.<br>

---


`spvRayQueryKHR`<br>
> Represents the SPIR-V capability for ray query.<br>

---


`spvRayQueryPositionFetchKHR`<br>
> Represents the SPIR-V capability for ray query position fetch.<br>

---


`spvShaderInvocationReorderNV`<br>
> Represents the SPIR-V capability for shader invocation reorder.<br>

---


`spvShaderClockKHR`<br>
> Represents the SPIR-V capability for shader clock.<br>

---


`spvShaderNonUniformEXT`<br>
> Represents the SPIR-V capability for non-uniform resource indexing.<br>

---


`spvShaderNonUniform`<br>
> Represents the SPIR-V capability for non-uniform resource indexing.<br>

---


`spvDemoteToHelperInvocationEXT`<br>
> Represents the SPIR-V capability for demoting to helper invocation.<br>

---


`spvDemoteToHelperInvocation`<br>
> Represents the SPIR-V capability for demoting to helper invocation.<br>

---


`GL_EXT_buffer_reference`<br>
> Represents the GL_EXT_buffer_reference extension.<br>

---


`GL_EXT_buffer_reference_uvec2`<br>
> Represents the GL_EXT_buffer_reference_uvec2 extension.<br>

---


`GL_EXT_debug_printf`<br>
> Represents the GL_EXT_debug_printf extension.<br>

---


`GL_EXT_demote_to_helper_invocation`<br>
> Represents the GL_EXT_demote_to_helper_invocation extension.<br>

---


`GL_EXT_fragment_shader_barycentric`<br>
> Represents the GL_EXT_fragment_shader_barycentric extension.<br>

---


`GL_EXT_mesh_shader`<br>
> Represents the GL_EXT_mesh_shader extension.<br>

---


`GL_EXT_nonuniform_qualifier`<br>
> Represents the GL_EXT_nonuniform_qualifier extension.<br>

---


`GL_EXT_ray_query`<br>
> Represents the GL_EXT_ray_query extension.<br>

---


`GL_EXT_ray_tracing`<br>
> Represents the GL_EXT_ray_tracing extension.<br>

---


`GL_EXT_ray_tracing_position_fetch_ray_tracing`<br>
> Represents the GL_EXT_ray_tracing_position_fetch_ray_tracing extension.<br>

---


`GL_EXT_ray_tracing_position_fetch_ray_query`<br>
> Represents the GL_EXT_ray_tracing_position_fetch_ray_query extension.<br>

---


`GL_EXT_ray_tracing_position_fetch`<br>
> Represents the GL_EXT_ray_tracing_position_fetch extension.<br>

---


`GL_EXT_samplerless_texture_functions`<br>
> Represents the GL_EXT_samplerless_texture_functions extension.<br>

---


`GL_EXT_shader_atomic_float`<br>
> Represents the GL_EXT_shader_atomic_float extension.<br>

---


`GL_EXT_shader_atomic_float_min_max`<br>
> Represents the GL_EXT_shader_atomic_float_min_max extension.<br>

---


`GL_EXT_shader_atomic_float2`<br>
> Represents the GL_EXT_shader_atomic_float2 extension.<br>

---


`GL_EXT_shader_atomic_int64`<br>
> Represents the GL_EXT_shader_atomic_int64 extension.<br>

---


`GL_EXT_shader_explicit_arithmetic_types_int64`<br>
> Represents the GL_EXT_shader_explicit_arithmetic_types_int64 extension.<br>

---


`GL_EXT_shader_image_load_store`<br>
> Represents the GL_EXT_shader_image_load_store extension.<br>

---


`GL_EXT_shader_realtime_clock`<br>
> Represents the GL_EXT_shader_realtime_clock extension.<br>

---


`GL_EXT_texture_query_lod`<br>
> Represents the GL_EXT_texture_query_lod extension.<br>

---


`GL_EXT_texture_shadow_lod`<br>
> Represents the GL_EXT_texture_shadow_lod extension.<br>

---


`GL_ARB_derivative_control`<br>
> Represents the GL_ARB_derivative_control extension.<br>

---


`GL_ARB_fragment_shader_interlock`<br>
> Represents the GL_ARB_fragment_shader_interlock extension.<br>

---


`GL_ARB_gpu_shader5`<br>
> Represents the GL_ARB_gpu_shader5 extension.<br>

---


`GL_ARB_shader_image_load_store`<br>
> Represents the GL_ARB_shader_image_load_store extension.<br>

---


`GL_ARB_shader_image_size`<br>
> Represents the GL_ARB_shader_image_size extension.<br>

---


`GL_ARB_texture_multisample`<br>
> Represents the GL_ARB_texture_multisample extension.<br>

---


`GL_ARB_shader_texture_image_samples`<br>
> Represents the GL_ARB_shader_texture_image_samples extension.<br>

---


`GL_ARB_sparse_texture`<br>
> Represents the GL_ARB_sparse_texture extension.<br>

---


`GL_ARB_sparse_texture2`<br>
> Represents the GL_ARB_sparse_texture2 extension.<br>

---


`GL_ARB_sparse_texture_clamp`<br>
> Represents the GL_ARB_sparse_texture_clamp extension.<br>

---


`GL_ARB_texture_gather`<br>
> Represents the GL_ARB_texture_gather extension.<br>

---


`GL_ARB_texture_query_levels`<br>
> Represents the GL_ARB_texture_query_levels extension.<br>

---


`GL_ARB_shader_clock`<br>
> Represents the GL_ARB_shader_clock extension.<br>

---


`GL_ARB_shader_clock64`<br>
> Represents the GL_ARB_shader_clock64 extension.<br>

---


`GL_ARB_gpu_shader_int64`<br>
> Represents the GL_ARB_gpu_shader_int64 extension.<br>

---


`GL_KHR_memory_scope_semantics`<br>
> Represents the GL_KHR_memory_scope_semantics extension.<br>

---


`GL_KHR_shader_subgroup_arithmetic`<br>
> Represents the GL_KHR_shader_subgroup_arithmetic extension.<br>

---


`GL_KHR_shader_subgroup_ballot`<br>
> Represents the GL_KHR_shader_subgroup_ballot extension.<br>

---


`GL_KHR_shader_subgroup_basic`<br>
> Represents the GL_KHR_shader_subgroup_basic extension.<br>

---


`GL_KHR_shader_subgroup_clustered`<br>
> Represents the GL_KHR_shader_subgroup_clustered extension.<br>

---


`GL_KHR_shader_subgroup_quad`<br>
> Represents the GL_KHR_shader_subgroup_quad extension.<br>

---


`GL_KHR_shader_subgroup_shuffle`<br>
> Represents the GL_KHR_shader_subgroup_shuffle extension.<br>

---


`GL_KHR_shader_subgroup_shuffle_relative`<br>
> Represents the GL_KHR_shader_subgroup_shuffle_relative extension.<br>

---


`GL_KHR_shader_subgroup_vote`<br>
> Represents the GL_KHR_shader_subgroup_vote extension.<br>

---


`GL_NV_compute_shader_derivatives`<br>
> Represents the GL_NV_compute_shader_derivatives extension.<br>

---


`GL_NV_fragment_shader_barycentric`<br>
> Represents the GL_NV_fragment_shader_barycentric extension.<br>

---


`GL_NV_gpu_shader5`<br>
> Represents the GL_NV_gpu_shader5 extension.<br>

---


`GL_NV_ray_tracing`<br>
> Represents the GL_NV_ray_tracing extension.<br>

---


`GL_NV_ray_tracing_motion_blur`<br>
> Represents the GL_NV_ray_tracing_motion_blur extension.<br>

---


`GL_NV_shader_atomic_fp16_vector`<br>
> Represents the GL_NV_shader_atomic_fp16_vector extension.<br>

---


`GL_NV_shader_invocation_reorder`<br>
> Represents the GL_NV_shader_invocation_reorder extension.<br>

---


`GL_NV_shader_subgroup_partitioned`<br>
> Represents the GL_NV_shader_subgroup_partitioned extension.<br>

---


`GL_NV_shader_texture_footprint`<br>
> Represents the GL_NV_shader_texture_footprint extension.<br>

---

<br>

Compound Capabilities
----------------------


`any_target`<br>
> All code-gen targets<br>

---


`any_textual_target`<br>
> All non-asm code-gen targets<br>

---


`any_gfx_target`<br>
> All slang-gfx compatible code-gen targets<br>

---


`any_cpp_target`<br>
> All "cpp syntax" code-gen targets<br>

---


`cpp_cuda`<br>
> CPP and CUDA code-gen targets<br>

---


`cpp_cuda_spirv`<br>
> CPP, CUDA and SPIRV code-gen targets<br>

---


`cpp_cuda_glsl_spirv`<br>
> CPP, CUDA, GLSL and SPIRV code-gen targets<br>

---


`cpp_cuda_glsl_hlsl`<br>
> CPP, CUDA, GLSL, and HLSL code-gen targets<br>

---


`cpp_cuda_glsl_hlsl_spirv`<br>
> CPP, CUDA, GLSL, HLSL, and SPIRV code-gen targets<br>

---


`cpp_cuda_glsl_hlsl_metal_spirv`<br>
> CPP, CUDA, GLSL, HLSL, Metal and SPIRV code-gen targets<br>

---


`cpp_cuda_hlsl`<br>
> CPP, CUDA, and HLSL code-gen targets<br>

---


`cpp_cuda_hlsl_spirv`<br>
> CPP, CUDA, HLSL, and SPIRV code-gen targets<br>

---


`cpp_cuda_hlsl_metal_spirv`<br>
> CPP, CUDA, HLSL, Metal, and SPIRV code-gen targets<br>

---


`cpp_glsl`<br>
> CPP, and GLSL code-gen targets<br>

---


`cpp_glsl_hlsl_spirv`<br>
> CPP, GLSL, HLSL, and SPIRV code-gen targets<br>

---


`cpp_glsl_hlsl_metal_spirv`<br>
> CPP, GLSL, HLSL, Metal, and SPIRV code-gen targets<br>

---


`cpp_hlsl`<br>
> CPP, and HLSL code-gen targets<br>

---


`cuda_glsl_hlsl`<br>
> CUDA, GLSL, and HLSL code-gen targets<br>

---


`cuda_hlsl_metal_spirv`<br>
> CUDA, HLSL, Metal, and SPIRV code-gen targets<br>

---


`cuda_glsl_hlsl_spirv`<br>
> CUDA, GLSL, HLSL, and SPIRV code-gen targets<br>

---


`cuda_glsl_hlsl_metal_spirv`<br>
> CUDA, GLSL, HLSL, Metal, and SPIRV code-gen targets<br>

---


`cuda_glsl_spirv`<br>
> CUDA, GLSL, and SPIRV code-gen targets<br>

---


`cuda_glsl_metal_spirv`<br>
> CUDA, GLSL, Metal, and SPIRV code-gen targets<br>

---


`cuda_hlsl`<br>
> CUDA, and HLSL code-gen targets<br>

---


`cuda_hlsl_spirv`<br>
> CUDA, HLSL, SPIRV code-gen targets<br>

---


`glsl_hlsl_spirv`<br>
> GLSL, HLSL, and SPIRV code-gen targets<br>

---


`glsl_hlsl_metal_spirv`<br>
> GLSL, HLSL, Metal, and SPIRV code-gen targets<br>

---


`glsl_metal_spirv`<br>
> GLSL, Metal, and SPIRV code-gen targets<br>

---


`glsl_spirv`<br>
> GLSL, and SPIRV code-gen targets<br>

---


`hlsl_spirv`<br>
> HLSL, and SPIRV code-gen targets<br>

---


`nvapi`<br>
> NVAPI capability for HLSL<br>

---


`raytracing`<br>
> Capabilities needed for minimal raytracing support<br>

---


`ser`<br>
> Capabilities needed for shader-execution-reordering<br>

---


`motionblur`<br>
> Capabilities needed for raytracing-motionblur<br>

---


`rayquery`<br>
> Capabilities needed for compute-shader rayquery<br>

---


`raytracing_motionblur`<br>
> Capabilities needed for compute-shader rayquery and motion-blur<br>

---


`ser_motion`<br>
> Capabilities needed for shader-execution-reordering and motion-blur<br>

---


`shaderclock`<br>
> Capabilities needed for realtime clocks<br>

---


`fragmentshaderinterlock`<br>
> Capabilities needed for interlocked-fragment operations<br>

---


`atomic64`<br>
> Capabilities needed for int64/uint64 atomic operations<br>

---


`atomicfloat`<br>
> Capabilities needed to use GLSL-tier-1 float-atomic operations<br>

---


`atomicfloat2`<br>
> Capabilities needed to use GLSL-tier-2 float-atomic operations<br>

---


`fragmentshaderbarycentric`<br>
> Capabilities needed to use fragment-shader-barycentric's<br>

---


`shadermemorycontrol`<br>
> (gfx targets) Capabilities needed to use memory barriers<br>

---


`waveprefix`<br>
> Capabilities needed to use HLSL tier wave operations<br>

---


`bufferreference`<br>
> Capabilities needed to use GLSL buffer-reference's<br>

---


`bufferreference_int64`<br>
> Capabilities needed to use GLSL buffer-reference's with int64<br>

---


`any_stage`<br>
> Collection of all shader stages<br>

---


`amplification_mesh`<br>
> Collection of shader stages<br>

---


`raytracing_stages`<br>
> Collection of shader stages<br>

---


`anyhit_closesthit`<br>
> Collection of shader stages<br>

---


`raygen_closesthit_miss`<br>
> Collection of shader stages<br>

---


`anyhit_closesthit_intersection`<br>
> Collection of shader stages<br>

---


`anyhit_closesthit_intersection_miss`<br>
> Collection of shader stages<br>

---


`raygen_closesthit_miss_callable`<br>
> Collection of shader stages<br>

---


`compute_tesscontrol_tesseval`<br>
> Collection of shader stages<br>

---


`compute_fragment`<br>
> Collection of shader stages<br>

---


`compute_fragment_geometry_vertex`<br>
> Collection of shader stages<br>

---


`domain_hull`<br>
> Collection of shader stages<br>

---


`raytracingstages_fragment`<br>
> Collection of shader stages<br>

---


`raytracingstages_compute`<br>
> Collection of shader stages<br>

---


`raytracingstages_compute_amplification_mesh`<br>
> Collection of shader stages<br>

---


`raytracingstages_compute_fragment`<br>
> Collection of shader stages<br>

---


`raytracingstages_compute_fragment_geometry_vertex`<br>
> Collection of shader stages<br>

---


`meshshading`<br>
> Ccapabilities required to use mesh shading features<br>

---


`shadermemorycontrol_compute`<br>
> (gfx targets) Capabilities required to use memory barriers that only work for raytracing & compute shader stages<br>

---


`subpass`<br>
> Capabilities required to use Subpass-Input's<br>

---


`appendstructuredbuffer`<br>
> Capabilities required to use AppendStructuredBuffer<br>

---


`atomic_hlsl`<br>
> (hlsl only) Capabilities required to use hlsl atomic operations<br>

---


`atomic_hlsl_nvapi`<br>
> (hlsl only) Capabilities required to use hlsl NVAPI atomics<br>

---


`atomic_hlsl_sm_6_6`<br>
> (hlsl only) Capabilities required to use hlsl sm_6_6 atomics<br>

---


`byteaddressbuffer`<br>
> Capabilities required to use ByteAddressBuffer<br>

---


`byteaddressbuffer_rw`<br>
> Capabilities required to use RWByteAddressBuffer<br>

---


`consumestructuredbuffer`<br>
> Capabilities required to use ConsumeStructuredBuffer<br>

---


`structuredbuffer`<br>
> Capabilities required to use StructuredBuffer<br>

---


`structuredbuffer_rw`<br>
> Capabilities required to use RWStructuredBuffer<br>

---


`fragmentprocessing`<br>
> Capabilities required to use fragment derivative operations (without GLSL derivativecontrol)<br>

---


`fragmentprocessing_derivativecontrol`<br>
> Capabilities required to use fragment derivative operations (with GLSL derivativecontrol)<br>

---


`getattributeatvertex`<br>
> Capabilities required to use 'getAttributeAtVertex'<br>

---


`memorybarrier`<br>
> Capabilities required to use sm_5_0 style memory barriers<br>

---


`texture_sm_4_0`<br>
> Capabilities required to use sm_4_0 texture operations<br>

---


`texture_sm_4_1`<br>
> Capabilities required to use sm_4_1 texture operations<br>

---


`texture_sm_4_1_samplerless`<br>
> Capabilities required to use sm_4_1 samplerless texture operations<br>

---


`texture_sm_4_1_compute_fragment`<br>
> Capabilities required to use 'compute/fragment shader only' texture operations<br>
> Note: we do not require 'compute'/'fragment' capabilities since this seems to be<br>
> incorrect behavior.<br>

---


`texture_sm_4_0_fragment`<br>
> Capabilities required to use 'fragment shader only' texture operations<br>

---


`texture_sm_4_1_clamp_fragment`<br>
> Capabilities required to use 'fragment shader only' texture clamp operations<br>

---


`texture_sm_4_1_vertex_fragment_geometry`<br>
> Capabilities required to use 'fragment/geometry shader only' texture clamp operations<br>

---


`texture_gather`<br>
> Capabilities required to use 'vertex/fragment/geometry shader only' texture gather operations<br>

---


`image_samples`<br>
> Capabilities required to query image (RWTexture) sample info<br>

---


`image_size`<br>
> Capabilities required to query image (RWTexture) size info<br>

---


`texture_size`<br>
> Capabilities required to query texture sample info<br>

---


`texture_querylod`<br>
> Capabilities required to query texture LOD info<br>

---


`texture_querylevels`<br>
> Capabilities required to query texture level info<br>

---


`texture_shadowlod`<br>
> Capabilities required to query shadow texture lod info<br>

---


`atomic_glsl_float1`<br>
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-1 float-atomic operations<br>

---


`atomic_glsl_float2`<br>
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-2 float-atomic operations<br>

---


`atomic_glsl_halfvec`<br>
> (GLSL/SPIRV) Capabilities required to use NVAPI GLSL-fp16 float-atomic operations<br>

---


`atomic_glsl`<br>
> (GLSL/SPIRV) Capabilities required to use GLSL-400 atomic operations<br>

---


`atomic_glsl_int64`<br>
> (GLSL/SPIRV) Capabilities required to use int64/uint64 atomic operations<br>

---


`image_loadstore`<br>
> (GLSL/SPIRV) Capabilities required to use image load/image store operations<br>

---


`nonuniformqualifier`<br>
> Capabilities required to use NonUniform qualifier<br>

---


`printf`<br>
> Capabilities required to use 'printf'<br>

---


`texturefootprint`<br>
> Capabilities required to use basic TextureFootprint operations<br>

---


`texturefootprintclamp`<br>
> Capabilities required to use TextureFootprint clamp operations<br>

---


`shader5_sm_4_0`<br>
> Capabilities required to use sm_4_0 features apart of GL_ARB_gpu_shader5<br>

---


`shader5_sm_5_0`<br>
> Capabilities required to use sm_5_0 features apart of GL_ARB_gpu_shader5<br>

---


`subgroup_basic`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic'<br>

---


`subgroup_ballot`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot'<br>

---


`subgroup_ballot_activemask`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot_activemask'<br>

---


`subgroup_basic_ballot`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic_ballot'<br>

---


`subgroup_vote`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'<br>

---


`shaderinvocationgroup`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'<br>

---


`subgroup_arithmetic`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_arithmetic'<br>

---


`subgroup_shuffle`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shuffle'<br>

---


`subgroup_shufflerelative`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shufle_relative'<br>

---


`subgroup_clustered`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_clustered'<br>

---


`subgroup_quad`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_quad'<br>

---


`subgroup_partitioned`<br>
> Capabilities required to use GLSL-style subgroup operations 'subgroup_partitioned'<br>

---


`atomic_glsl_hlsl_nvapi_cuda_metal_float1`<br>
> (All implemented targets) Capabilities required to use atomic operations of GLSL tier-1 float atomics<br>

---


`atomic_glsl_hlsl_nvapi_cuda5_int64`<br>
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_5 tier atomics)<br>

---


`atomic_glsl_hlsl_nvapi_cuda6_int64`<br>
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_6 tier atomics)<br>

---


`atomic_glsl_hlsl_nvapi_cuda9_int64`<br>
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_9 tier atomics)<br>

---


`atomic_glsl_hlsl_cuda_metal`<br>
> (All implemented targets) Capabilities required to use atomic operations<br>

---


`atomic_glsl_hlsl_cuda9_int64`<br>
> (All implemented targets) Capabilities required to use atomic operations (cuda_sm_9 tier atomics)<br>

---


`helper_lane`<br>
> Capabilities required to enable helper-lane demotion<br>

---


`breakpoint`<br>
> Capabilities required to enable shader breakpoints<br>

---


`raytracing_allstages`<br>
> Collection of capabilities for raytracing with all raytracing stages.<br>

---


`raytracing_anyhit`<br>
> Collection of capabilities for raytracing with the shader stage of anyhit.<br>

---


`raytracing_intersection`<br>
> Collection of capabilities for raytracing with the shader stage of intersection.<br>

---


`raytracing_anyhit_closesthit`<br>
> Collection of capabilities for raytracing with the shader stages of anyhit and closesthit.<br>

---


`raytracing_anyhit_closesthit_intersection`<br>
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, and intersection.<br>

---


`raytracing_raygen_closesthit_miss`<br>
> Collection of capabilities for raytracing with the shader stages of raygen, closesthit, and miss.<br>

---


`raytracing_anyhit_closesthit_intersection_miss`<br>
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, intersection, and miss.<br>

---


`raytracing_raygen_closesthit_miss_callable`<br>
> Collection of capabilities for raytracing the shader stages of raygen, closesthit, miss, and callable.<br>

---


`raytracing_position`<br>
> Collection of capabilities for raytracing + ray_tracing_position_fetch and the shader stages of anyhit and closesthit.<br>

---


`raytracing_motionblur_anyhit_closesthit_intersection_miss`<br>
> Collection of capabilities for raytracing + motion blur and the shader stages of anyhit, closesthit, intersection, and miss.<br>

---


`raytracing_motionblur_raygen_closesthit_miss`<br>
> Collection of capabilities for raytracing + motion blur and the shader stages of raygen, closesthit, and miss.<br>

---


`rayquery_position`<br>
> Collection of capabilities for rayquery + ray_tracing_position_fetch.<br>

---


`ser_raygen`<br>
> Collection of capabilities for raytracing + shader execution reordering and the shader stage of raygen.<br>

---


`ser_raygen_closesthit_miss`<br>
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of raygen, closesthit, and miss.<br>

---


`ser_any_closesthit_intersection_miss`<br>
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, intersection, and miss.<br>

---


`ser_anyhit_closesthit_intersection`<br>
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, and intersection.<br>

---


`ser_anyhit_closesthit`<br>
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit and closesthit.<br>

---


`ser_motion_raygen_closesthit_miss`<br>
> Collection of capabilities for raytracing + motion blur + shader execution reordering and the shader stages of raygen, closesthit, and miss.<br>

---


`ser_motion_raygen`<br>
> Collection of capabilities for raytracing raytracing + motion blur + shader execution reordering and the shader stage of raygen.<br>

---

<br>

Other
----------------------


`SPIRV_1_0`<br>

---


`SPIRV_1_1`<br>

---


`SPIRV_1_2`<br>

---


`SPIRV_1_3`<br>

---


`SPIRV_1_4`<br>

---


`SPIRV_1_5`<br>

---


`SPIRV_1_6`<br>

---


`DX_4_0`<br>

---


`DX_4_1`<br>

---


`DX_5_0`<br>

---


`DX_5_1`<br>

---


`DX_6_0`<br>

---


`DX_6_1`<br>

---


`DX_6_2`<br>

---


`DX_6_3`<br>

---


`DX_6_4`<br>

---


`DX_6_5`<br>

---


`DX_6_6`<br>

---


`DX_6_7`<br>

---


`GLSL_410_SPIRV_1_0`<br>

---


`GLSL_420_SPIRV_1_0`<br>

---


`GLSL_430_SPIRV_1_0`<br>

---


`METAL_2_3`<br>

---


`METAL_2_4`<br>

---


`METAL_3_0`<br>

---


`METAL_3_1`<br>

---


`GLSL_430_SPIRV_1_0_compute`<br>

---


`all`<br>

---
