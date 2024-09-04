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

Targets
----------------------
*Capabilities to specify code generation targets (`glsl`, `spirv`...)*

`textualTarget`
> Represents a non-assembly code generation target.<br>


`hlsl`
> Represents the HLSL code generation target.<br>


`glsl`
> Represents the GLSL code generation target.<br>


`c`
> Represents the C programming language code generation target.<br>


`cpp`
> Represents the C++ programming language code generation target.<br>


`cuda`
> Represents the CUDA code generation target.<br>


`metal`
> Represents the Metal programming language code generation target.<br>


`spirv`
> Represents the SPIR-V code generation target.<br>


Stages
----------------------
*Capabilities to specify code generation stages (`vertex`, `fragment`...)*

`vertex`
> Vertex shader stage<br>


`fragment`
> Fragment shader stage<br>


`compute`
> Compute shader stage<br>


`hull`
> Hull shader stage<br>


`domain`
> Domain shader stage<br>


`geometry`
> Geometry shader stage<br>


`pixel`
> Pixel shader stage<br>


`tesscontrol`
> Tessellation Control shader stage<br>


`tesseval`
> Tessellation Evaluation shader stage<br>


`raygen`
> Ray-Generation shader stage & ray-tracing capabilities<br>


`raygeneration`
> Ray-Generation shader stage & ray-tracing capabilities<br>


`intersection`
> Intersection shader stage & ray-tracing capabilities<br>


`anyhit`
> Any-Hit shader stage & ray-tracing capabilities<br>


`closesthit`
> Closest-Hit shader stage & ray-tracing capabilities<br>


`callable`
> Callable shader stage & ray-tracing capabilities<br>


`miss`
> Ray-Miss shader stage & ray-tracing capabilities<br>


`mesh`
> Mesh shader stage & mesh shader capabilities<br>


`amplification`
> Amplification shader stage & mesh shader capabilities<br>


Versions
----------------------
*Capabilities to specify versions of a code generation target (`sm_5_0`, `GLSL_400`...)*

`glsl_spirv_1_0`
> Represents SPIR-V 1.0 through glslang.<br>


`glsl_spirv_1_1`
> Represents SPIR-V 1.1 through glslang.<br>


`glsl_spirv_1_2`
> Represents SPIR-V 1.2 through glslang.<br>


`glsl_spirv_1_3`
> Represents SPIR-V 1.3 through glslang.<br>


`glsl_spirv_1_4`
> Represents SPIR-V 1.4 through glslang.<br>


`glsl_spirv_1_5`
> Represents SPIR-V 1.5 through glslang.<br>


`glsl_spirv_1_6`
> Represents SPIR-V 1.6 through glslang.<br>


`metallib_2_3`
> Represents MetalLib 2.3.<br>


`metallib_2_4`
> Represents MetalLib 2.4.<br>


`metallib_3_0`
> Represents MetalLib 3.0.<br>


`metallib_3_1`
> Represents MetalLib 3.1.<br>


`metallib_latest`
> Represents the latest MetalLib version.<br>


`hlsl_nvapi`
> Represents HLSL NVAPI support.<br>


`dxil_lib`
> Represents capabilities required for DXIL Library compilation.<br>


`spirv_1_0`
> Represents SPIR-V 1.0 version.<br>


`spirv_1_1`
> Represents SPIR-V 1.1 version, which includes SPIR-V 1.0.<br>


`spirv_1_2`
> Represents SPIR-V 1.2 version, which includes SPIR-V 1.1.<br>


`spirv_1_3`
> Represents SPIR-V 1.3 version, which includes SPIR-V 1.2.<br>


`spirv_1_4`
> Represents SPIR-V 1.4 version, which includes SPIR-V 1.3.<br>


`spirv_1_5`
> Represents SPIR-V 1.5 version, which includes SPIR-V 1.4 and additional extensions.<br>


`spirv_1_6`
> Represents SPIR-V 1.6 version, which includes SPIR-V 1.5 and additional extensions.<br>


`spirv_latest`
> Represents the latest SPIR-V version.<br>


`sm_4_0_version`
> HLSL shader model 4.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_4_0`
> HLSL shader model 4.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_4_1_version`
> HLSL shader model 4.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_4_1`
> HLSL shader model 4.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_5_0_version`
> HLSL shader model 5.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_5_0`
> HLSL shader model 5.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_5_1_version`
> HLSL shader model 5.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_5_1`
> HLSL shader model 5.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_0_version`
> HLSL shader model 6.0 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_0`
> HLSL shader model 6.0 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_1_version`
> HLSL shader model 6.1 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_1`
> HLSL shader model 6.1 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_2_version`
> HLSL shader model 6.2 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_2`
> HLSL shader model 6.2 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_3_version`
> HLSL shader model 6.3 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_3`
> HLSL shader model 6.3 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_4_version`
> HLSL shader model 6.4 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_4`
> HLSL shader model 6.4 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_5_version`
> HLSL shader model 6.5 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_5`
> HLSL shader model 6.5 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_6_version`
> HLSL shader model 6.6 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_6`
> HLSL shader model 6.6 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`sm_6_7_version`
> HLSL shader model 6.7 and related capabilities of other targets.<br>
> Without related GLSL/SPIRV extensions.<br>


`sm_6_7`
> HLSL shader model 6.7 and related capabilities of other targets.<br>
> With related GLSL/SPIRV extensions.<br>


`GLSL_130`
> GLSL 130 and related capabilities of other targets.<br>


`GLSL_140`
> GLSL 140 and related capabilities of other targets.<br>


`GLSL_150`
> GLSL 150 and related capabilities of other targets.<br>


`GLSL_330`
> GLSL 330 and related capabilities of other targets.<br>


`GLSL_400`
> GLSL 400 and related capabilities of other targets.<br>


`GLSL_410`
> GLSL 410 and related capabilities of other targets.<br>


`GLSL_420`
> GLSL 420 and related capabilities of other targets.<br>


`GLSL_430`
> GLSL 430 and related capabilities of other targets.<br>


`GLSL_440`
> GLSL 440 and related capabilities of other targets.<br>


`GLSL_450`
> GLSL 450 and related capabilities of other targets.<br>


`GLSL_460`
> GLSL 460 and related capabilities of other targets.<br>


`cuda_sm_1_0`
> cuda 1.0 and related capabilities of other targets.<br>


`cuda_sm_2_0`
> cuda 2.0 and related capabilities of other targets.<br>


`cuda_sm_3_0`
> cuda 3.0 and related capabilities of other targets.<br>


`cuda_sm_3_5`
> cuda 3.5 and related capabilities of other targets.<br>


`cuda_sm_4_0`
> cuda 4.0 and related capabilities of other targets.<br>


`cuda_sm_5_0`
> cuda 5.0 and related capabilities of other targets.<br>


`cuda_sm_6_0`
> cuda 6.0 and related capabilities of other targets.<br>


`cuda_sm_7_0`
> cuda 7.0 and related capabilities of other targets.<br>


`cuda_sm_8_0`
> cuda 8.0 and related capabilities of other targets.<br>


`cuda_sm_9_0`
> cuda 9.0 and related capabilities of other targets.<br>


Extensions
----------------------
*Capabilities to specify extensions (`GL_EXT`, `SPV_EXT`...)*

`SPV_EXT_fragment_shader_interlock`
> Represents the SPIR-V extension for fragment shader interlock operations.<br>


`SPV_EXT_physical_storage_buffer`
> Represents the SPIR-V extension for physical storage buffer.<br>


`SPV_EXT_fragment_fully_covered`
> Represents the SPIR-V extension for SPV_EXT_fragment_fully_covered.<br>


`SPV_EXT_descriptor_indexing`
> Represents the SPIR-V extension for descriptor indexing.<br>


`SPV_EXT_shader_atomic_float_add`
> Represents the SPIR-V extension for atomic float add operations.<br>


`SPV_EXT_shader_atomic_float16_add`
> Represents the SPIR-V extension for atomic float16 add operations.<br>


`SPV_EXT_shader_atomic_float_min_max`
> Represents the SPIR-V extension for atomic float min/max operations.<br>


`SPV_EXT_mesh_shader`
> Represents the SPIR-V extension for mesh shaders.<br>


`SPV_EXT_demote_to_helper_invocation`
> Represents the SPIR-V extension for demoting to helper invocation.<br>


`SPV_KHR_fragment_shader_barycentric`
> Represents the SPIR-V extension for fragment shader barycentric.<br>


`SPV_KHR_non_semantic_info`
> Represents the SPIR-V extension for non-semantic information.<br>


`SPV_KHR_ray_tracing`
> Represents the SPIR-V extension for ray tracing.<br>


`SPV_KHR_ray_query`
> Represents the SPIR-V extension for ray queries.<br>


`SPV_KHR_ray_tracing_position_fetch`
> Represents the SPIR-V extension for ray tracing position fetch.<br>
> Should be used with either SPV_KHR_ray_query or SPV_KHR_ray_tracing.<br>


`SPV_KHR_shader_clock`
> Represents the SPIR-V extension for shader clock.<br>


`SPV_NV_shader_subgroup_partitioned`
> Represents the SPIR-V extension for shader subgroup partitioned.<br>


`SPV_NV_ray_tracing_motion_blur`
> Represents the SPIR-V extension for ray tracing motion blur.<br>


`SPV_NV_shader_invocation_reorder`
> Represents the SPIR-V extension for shader invocation reorder.<br>
> Requires SPV_KHR_ray_tracing.<br>


`SPV_NV_shader_image_footprint`
> Represents the SPIR-V extension for shader image footprint.<br>


`SPV_NV_compute_shader_derivatives`
> Represents the SPIR-V extension for compute shader derivatives.<br>


`SPV_GOOGLE_user_type`
> Represents the SPIR-V extension for SPV_GOOGLE_user_type.<br>


`spvAtomicFloat32AddEXT`
> Represents the SPIR-V capability for atomic float 32 add operations.<br>


`spvAtomicFloat16AddEXT`
> Represents the SPIR-V capability for atomic float 16 add operations.<br>


`spvInt64Atomics`
> Represents the SPIR-V capability for 64-bit integer atomics.<br>


`spvAtomicFloat32MinMaxEXT`
> Represents the SPIR-V capability for atomic float 32 min/max operations.<br>


`spvAtomicFloat16MinMaxEXT`
> Represents the SPIR-V capability for atomic float 16 min/max operations.<br>


`spvDerivativeControl`
> Represents the SPIR-V capability for 'derivative control' operations.<br>


`spvImageQuery`
> Represents the SPIR-V capability for image query operations.<br>


`spvImageGatherExtended`
> Represents the SPIR-V capability for extended image gather operations.<br>


`spvSparseResidency`
> Represents the SPIR-V capability for sparse residency.<br>


`spvImageFootprintNV`
> Represents the SPIR-V capability for image footprint.<br>


`spvMinLod`
> Represents the SPIR-V capability for using minimum LOD operations.<br>


`spvFragmentShaderPixelInterlockEXT`
> Represents the SPIR-V capability for using SPV_EXT_fragment_shader_interlock.<br>


`spvFragmentBarycentricKHR`
> Represents the SPIR-V capability for using SPV_KHR_fragment_shader_barycentric.<br>


`spvFragmentFullyCoveredEXT`
> Represents the SPIR-V capability for using SPV_EXT_fragment_fully_covered functionality.<br>


`spvGroupNonUniformBallot`
> Represents the SPIR-V capability for group non-uniform ballot operations.<br>


`spvGroupNonUniformShuffle`
> Represents the SPIR-V capability for group non-uniform shuffle operations.<br>


`spvGroupNonUniformArithmetic`
> Represents the SPIR-V capability for group non-uniform arithmetic operations.<br>


`spvGroupNonUniformQuad`
> Represents the SPIR-V capability for group non-uniform quad operations.<br>


`spvGroupNonUniformVote`
> Represents the SPIR-V capability for group non-uniform vote operations.<br>


`spvGroupNonUniformPartitionedNV`
> Represents the SPIR-V capability for group non-uniform partitioned operations.<br>


`spvRayTracingMotionBlurNV`
> Represents the SPIR-V capability for ray tracing motion blur.<br>


`spvMeshShadingEXT`
> Represents the SPIR-V capability for mesh shading.<br>


`spvRayTracingKHR`
> Represents the SPIR-V capability for ray tracing.<br>


`spvRayTracingPositionFetchKHR`
> Represents the SPIR-V capability for ray tracing position fetch.<br>


`spvRayQueryKHR`
> Represents the SPIR-V capability for ray query.<br>


`spvRayQueryPositionFetchKHR`
> Represents the SPIR-V capability for ray query position fetch.<br>


`spvShaderInvocationReorderNV`
> Represents the SPIR-V capability for shader invocation reorder.<br>


`spvShaderClockKHR`
> Represents the SPIR-V capability for shader clock.<br>


`spvShaderNonUniformEXT`
> Represents the SPIR-V capability for non-uniform resource indexing.<br>


`spvShaderNonUniform`
> Represents the SPIR-V capability for non-uniform resource indexing.<br>


`spvDemoteToHelperInvocationEXT`
> Represents the SPIR-V capability for demoting to helper invocation.<br>


`spvDemoteToHelperInvocation`
> Represents the SPIR-V capability for demoting to helper invocation.<br>


`GL_EXT_buffer_reference`
> Represents the GL_EXT_buffer_reference extension.<br>


`GL_EXT_buffer_reference_uvec2`
> Represents the GL_EXT_buffer_reference_uvec2 extension.<br>


`GL_EXT_debug_printf`
> Represents the GL_EXT_debug_printf extension.<br>


`GL_EXT_demote_to_helper_invocation`
> Represents the GL_EXT_demote_to_helper_invocation extension.<br>


`GL_EXT_fragment_shader_barycentric`
> Represents the GL_EXT_fragment_shader_barycentric extension.<br>


`GL_EXT_mesh_shader`
> Represents the GL_EXT_mesh_shader extension.<br>


`GL_EXT_nonuniform_qualifier`
> Represents the GL_EXT_nonuniform_qualifier extension.<br>


`GL_EXT_ray_query`
> Represents the GL_EXT_ray_query extension.<br>


`GL_EXT_ray_tracing`
> Represents the GL_EXT_ray_tracing extension.<br>


`GL_EXT_ray_tracing_position_fetch_ray_tracing`
> Represents the GL_EXT_ray_tracing_position_fetch_ray_tracing extension.<br>


`GL_EXT_ray_tracing_position_fetch_ray_query`
> Represents the GL_EXT_ray_tracing_position_fetch_ray_query extension.<br>


`GL_EXT_ray_tracing_position_fetch`
> Represents the GL_EXT_ray_tracing_position_fetch extension.<br>


`GL_EXT_samplerless_texture_functions`
> Represents the GL_EXT_samplerless_texture_functions extension.<br>


`GL_EXT_shader_atomic_float`
> Represents the GL_EXT_shader_atomic_float extension.<br>


`GL_EXT_shader_atomic_float_min_max`
> Represents the GL_EXT_shader_atomic_float_min_max extension.<br>


`GL_EXT_shader_atomic_float2`
> Represents the GL_EXT_shader_atomic_float2 extension.<br>


`GL_EXT_shader_atomic_int64`
> Represents the GL_EXT_shader_atomic_int64 extension.<br>


`GL_EXT_shader_explicit_arithmetic_types_int64`
> Represents the GL_EXT_shader_explicit_arithmetic_types_int64 extension.<br>


`GL_EXT_shader_image_load_store`
> Represents the GL_EXT_shader_image_load_store extension.<br>


`GL_EXT_shader_realtime_clock`
> Represents the GL_EXT_shader_realtime_clock extension.<br>


`GL_EXT_texture_query_lod`
> Represents the GL_EXT_texture_query_lod extension.<br>


`GL_EXT_texture_shadow_lod`
> Represents the GL_EXT_texture_shadow_lod extension.<br>


`GL_ARB_derivative_control`
> Represents the GL_ARB_derivative_control extension.<br>


`GL_ARB_fragment_shader_interlock`
> Represents the GL_ARB_fragment_shader_interlock extension.<br>


`GL_ARB_gpu_shader5`
> Represents the GL_ARB_gpu_shader5 extension.<br>


`GL_ARB_shader_image_load_store`
> Represents the GL_ARB_shader_image_load_store extension.<br>


`GL_ARB_shader_image_size`
> Represents the GL_ARB_shader_image_size extension.<br>


`GL_ARB_texture_multisample`
> Represents the GL_ARB_texture_multisample extension.<br>


`GL_ARB_shader_texture_image_samples`
> Represents the GL_ARB_shader_texture_image_samples extension.<br>


`GL_ARB_sparse_texture`
> Represents the GL_ARB_sparse_texture extension.<br>


`GL_ARB_sparse_texture2`
> Represents the GL_ARB_sparse_texture2 extension.<br>


`GL_ARB_sparse_texture_clamp`
> Represents the GL_ARB_sparse_texture_clamp extension.<br>


`GL_ARB_texture_gather`
> Represents the GL_ARB_texture_gather extension.<br>


`GL_ARB_texture_query_levels`
> Represents the GL_ARB_texture_query_levels extension.<br>


`GL_ARB_shader_clock`
> Represents the GL_ARB_shader_clock extension.<br>


`GL_ARB_shader_clock64`
> Represents the GL_ARB_shader_clock64 extension.<br>


`GL_ARB_gpu_shader_int64`
> Represents the GL_ARB_gpu_shader_int64 extension.<br>


`GL_KHR_memory_scope_semantics`
> Represents the GL_KHR_memory_scope_semantics extension.<br>


`GL_KHR_shader_subgroup_arithmetic`
> Represents the GL_KHR_shader_subgroup_arithmetic extension.<br>


`GL_KHR_shader_subgroup_ballot`
> Represents the GL_KHR_shader_subgroup_ballot extension.<br>


`GL_KHR_shader_subgroup_basic`
> Represents the GL_KHR_shader_subgroup_basic extension.<br>


`GL_KHR_shader_subgroup_clustered`
> Represents the GL_KHR_shader_subgroup_clustered extension.<br>


`GL_KHR_shader_subgroup_quad`
> Represents the GL_KHR_shader_subgroup_quad extension.<br>


`GL_KHR_shader_subgroup_shuffle`
> Represents the GL_KHR_shader_subgroup_shuffle extension.<br>


`GL_KHR_shader_subgroup_shuffle_relative`
> Represents the GL_KHR_shader_subgroup_shuffle_relative extension.<br>


`GL_KHR_shader_subgroup_vote`
> Represents the GL_KHR_shader_subgroup_vote extension.<br>


`GL_NV_compute_shader_derivatives`
> Represents the GL_NV_compute_shader_derivatives extension.<br>


`GL_NV_fragment_shader_barycentric`
> Represents the GL_NV_fragment_shader_barycentric extension.<br>


`GL_NV_gpu_shader5`
> Represents the GL_NV_gpu_shader5 extension.<br>


`GL_NV_ray_tracing`
> Represents the GL_NV_ray_tracing extension.<br>


`GL_NV_ray_tracing_motion_blur`
> Represents the GL_NV_ray_tracing_motion_blur extension.<br>


`GL_NV_shader_atomic_fp16_vector`
> Represents the GL_NV_shader_atomic_fp16_vector extension.<br>


`GL_NV_shader_invocation_reorder`
> Represents the GL_NV_shader_invocation_reorder extension.<br>


`GL_NV_shader_subgroup_partitioned`
> Represents the GL_NV_shader_subgroup_partitioned extension.<br>


`GL_NV_shader_texture_footprint`
> Represents the GL_NV_shader_texture_footprint extension.<br>


Compound Capabilities
----------------------
*Capabilities to specify capabilities created by other capabilities (`raytracing`, `meshshading`...)*

`any_target`
> All code-gen targets<br>


`any_textual_target`
> All non-asm code-gen targets<br>


`any_gfx_target`
> All slang-gfx compatible code-gen targets<br>


`any_cpp_target`
> All "cpp syntax" code-gen targets<br>


`cpp_cuda`
> CPP and CUDA code-gen targets<br>


`cpp_cuda_spirv`
> CPP, CUDA and SPIRV code-gen targets<br>


`cpp_cuda_glsl_spirv`
> CPP, CUDA, GLSL and SPIRV code-gen targets<br>


`cpp_cuda_glsl_hlsl`
> CPP, CUDA, GLSL, and HLSL code-gen targets<br>


`cpp_cuda_glsl_hlsl_spirv`
> CPP, CUDA, GLSL, HLSL, and SPIRV code-gen targets<br>


`cpp_cuda_glsl_hlsl_metal_spirv`
> CPP, CUDA, GLSL, HLSL, Metal and SPIRV code-gen targets<br>


`cpp_cuda_hlsl`
> CPP, CUDA, and HLSL code-gen targets<br>


`cpp_cuda_hlsl_spirv`
> CPP, CUDA, HLSL, and SPIRV code-gen targets<br>


`cpp_cuda_hlsl_metal_spirv`
> CPP, CUDA, HLSL, Metal, and SPIRV code-gen targets<br>


`cpp_glsl`
> CPP, and GLSL code-gen targets<br>


`cpp_glsl_hlsl_spirv`
> CPP, GLSL, HLSL, and SPIRV code-gen targets<br>


`cpp_glsl_hlsl_metal_spirv`
> CPP, GLSL, HLSL, Metal, and SPIRV code-gen targets<br>


`cpp_hlsl`
> CPP, and HLSL code-gen targets<br>


`cuda_glsl_hlsl`
> CUDA, GLSL, and HLSL code-gen targets<br>


`cuda_hlsl_metal_spirv`
> CUDA, HLSL, Metal, and SPIRV code-gen targets<br>


`cuda_glsl_hlsl_spirv`
> CUDA, GLSL, HLSL, and SPIRV code-gen targets<br>


`cuda_glsl_hlsl_metal_spirv`
> CUDA, GLSL, HLSL, Metal, and SPIRV code-gen targets<br>


`cuda_glsl_spirv`
> CUDA, GLSL, and SPIRV code-gen targets<br>


`cuda_glsl_metal_spirv`
> CUDA, GLSL, Metal, and SPIRV code-gen targets<br>


`cuda_hlsl`
> CUDA, and HLSL code-gen targets<br>


`cuda_hlsl_spirv`
> CUDA, HLSL, SPIRV code-gen targets<br>


`glsl_hlsl_spirv`
> GLSL, HLSL, and SPIRV code-gen targets<br>


`glsl_hlsl_metal_spirv`
> GLSL, HLSL, Metal, and SPIRV code-gen targets<br>


`glsl_metal_spirv`
> GLSL, Metal, and SPIRV code-gen targets<br>


`glsl_spirv`
> GLSL, and SPIRV code-gen targets<br>


`hlsl_spirv`
> HLSL, and SPIRV code-gen targets<br>


`nvapi`
> NVAPI capability for HLSL<br>


`raytracing`
> Capabilities needed for minimal raytracing support<br>


`ser`
> Capabilities needed for shader-execution-reordering<br>


`motionblur`
> Capabilities needed for raytracing-motionblur<br>


`rayquery`
> Capabilities needed for compute-shader rayquery<br>


`raytracing_motionblur`
> Capabilities needed for compute-shader rayquery and motion-blur<br>


`ser_motion`
> Capabilities needed for shader-execution-reordering and motion-blur<br>


`shaderclock`
> Capabilities needed for realtime clocks<br>


`fragmentshaderinterlock`
> Capabilities needed for interlocked-fragment operations<br>


`atomic64`
> Capabilities needed for int64/uint64 atomic operations<br>


`atomicfloat`
> Capabilities needed to use GLSL-tier-1 float-atomic operations<br>


`atomicfloat2`
> Capabilities needed to use GLSL-tier-2 float-atomic operations<br>


`fragmentshaderbarycentric`
> Capabilities needed to use fragment-shader-barycentric's<br>


`shadermemorycontrol`
> (gfx targets) Capabilities needed to use memory barriers<br>


`waveprefix`
> Capabilities needed to use HLSL tier wave operations<br>


`bufferreference`
> Capabilities needed to use GLSL buffer-reference's<br>


`bufferreference_int64`
> Capabilities needed to use GLSL buffer-reference's with int64<br>


`any_stage`
> Collection of all shader stages<br>


`amplification_mesh`
> Collection of shader stages<br>


`raytracing_stages`
> Collection of shader stages<br>


`anyhit_closesthit`
> Collection of shader stages<br>


`raygen_closesthit_miss`
> Collection of shader stages<br>


`anyhit_closesthit_intersection`
> Collection of shader stages<br>


`anyhit_closesthit_intersection_miss`
> Collection of shader stages<br>


`raygen_closesthit_miss_callable`
> Collection of shader stages<br>


`compute_tesscontrol_tesseval`
> Collection of shader stages<br>


`compute_fragment`
> Collection of shader stages<br>


`compute_fragment_geometry_vertex`
> Collection of shader stages<br>


`domain_hull`
> Collection of shader stages<br>


`raytracingstages_fragment`
> Collection of shader stages<br>


`raytracingstages_compute`
> Collection of shader stages<br>


`raytracingstages_compute_amplification_mesh`
> Collection of shader stages<br>


`raytracingstages_compute_fragment`
> Collection of shader stages<br>


`raytracingstages_compute_fragment_geometry_vertex`
> Collection of shader stages<br>


`meshshading`
> Ccapabilities required to use mesh shading features<br>


`shadermemorycontrol_compute`
> (gfx targets) Capabilities required to use memory barriers that only work for raytracing & compute shader stages<br>


`subpass`
> Capabilities required to use Subpass-Input's<br>


`appendstructuredbuffer`
> Capabilities required to use AppendStructuredBuffer<br>


`atomic_hlsl`
> (hlsl only) Capabilities required to use hlsl atomic operations<br>


`atomic_hlsl_nvapi`
> (hlsl only) Capabilities required to use hlsl NVAPI atomics<br>


`atomic_hlsl_sm_6_6`
> (hlsl only) Capabilities required to use hlsl sm_6_6 atomics<br>


`byteaddressbuffer`
> Capabilities required to use ByteAddressBuffer<br>


`byteaddressbuffer_rw`
> Capabilities required to use RWByteAddressBuffer<br>


`consumestructuredbuffer`
> Capabilities required to use ConsumeStructuredBuffer<br>


`structuredbuffer`
> Capabilities required to use StructuredBuffer<br>


`structuredbuffer_rw`
> Capabilities required to use RWStructuredBuffer<br>


`fragmentprocessing`
> Capabilities required to use fragment derivative operations (without GLSL derivativecontrol)<br>


`fragmentprocessing_derivativecontrol`
> Capabilities required to use fragment derivative operations (with GLSL derivativecontrol)<br>


`getattributeatvertex`
> Capabilities required to use 'getAttributeAtVertex'<br>


`memorybarrier`
> Capabilities required to use sm_5_0 style memory barriers<br>


`texture_sm_4_0`
> Capabilities required to use sm_4_0 texture operations<br>


`texture_sm_4_1`
> Capabilities required to use sm_4_1 texture operations<br>


`texture_sm_4_1_samplerless`
> Capabilities required to use sm_4_1 samplerless texture operations<br>


`texture_sm_4_1_compute_fragment`
> Capabilities required to use 'compute/fragment shader only' texture operations<br>
> Note: we do not require 'compute'/'fragment' capabilities since this seems to be<br>
> incorrect behavior.<br>


`texture_sm_4_0_fragment`
> Capabilities required to use 'fragment shader only' texture operations<br>


`texture_sm_4_1_clamp_fragment`
> Capabilities required to use 'fragment shader only' texture clamp operations<br>


`texture_sm_4_1_vertex_fragment_geometry`
> Capabilities required to use 'fragment/geometry shader only' texture clamp operations<br>


`texture_gather`
> Capabilities required to use 'vertex/fragment/geometry shader only' texture gather operations<br>


`image_samples`
> Capabilities required to query image (RWTexture) sample info<br>


`image_size`
> Capabilities required to query image (RWTexture) size info<br>


`texture_size`
> Capabilities required to query texture sample info<br>


`texture_querylod`
> Capabilities required to query texture LOD info<br>


`texture_querylevels`
> Capabilities required to query texture level info<br>


`texture_shadowlod`
> Capabilities required to query shadow texture lod info<br>


`atomic_glsl_float1`
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-1 float-atomic operations<br>


`atomic_glsl_float2`
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-2 float-atomic operations<br>


`atomic_glsl_halfvec`
> (GLSL/SPIRV) Capabilities required to use NVAPI GLSL-fp16 float-atomic operations<br>


`atomic_glsl`
> (GLSL/SPIRV) Capabilities required to use GLSL-400 atomic operations<br>


`atomic_glsl_int64`
> (GLSL/SPIRV) Capabilities required to use int64/uint64 atomic operations<br>


`image_loadstore`
> (GLSL/SPIRV) Capabilities required to use image load/image store operations<br>


`nonuniformqualifier`
> Capabilities required to use NonUniform qualifier<br>


`printf`
> Capabilities required to use 'printf'<br>


`texturefootprint`
> Capabilities required to use basic TextureFootprint operations<br>


`texturefootprintclamp`
> Capabilities required to use TextureFootprint clamp operations<br>


`shader5_sm_4_0`
> Capabilities required to use sm_4_0 features apart of GL_ARB_gpu_shader5<br>


`shader5_sm_5_0`
> Capabilities required to use sm_5_0 features apart of GL_ARB_gpu_shader5<br>


`subgroup_basic`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic'<br>


`subgroup_ballot`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot'<br>


`subgroup_ballot_activemask`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot_activemask'<br>


`subgroup_basic_ballot`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic_ballot'<br>


`subgroup_vote`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'<br>


`shaderinvocationgroup`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'<br>


`subgroup_arithmetic`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_arithmetic'<br>


`subgroup_shuffle`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shuffle'<br>


`subgroup_shufflerelative`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shufle_relative'<br>


`subgroup_clustered`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_clustered'<br>


`subgroup_quad`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_quad'<br>


`subgroup_partitioned`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_partitioned'<br>


`atomic_glsl_hlsl_nvapi_cuda_metal_float1`
> (All implemented targets) Capabilities required to use atomic operations of GLSL tier-1 float atomics<br>


`atomic_glsl_hlsl_nvapi_cuda5_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_5 tier atomics)<br>


`atomic_glsl_hlsl_nvapi_cuda6_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_6 tier atomics)<br>


`atomic_glsl_hlsl_nvapi_cuda9_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_9 tier atomics)<br>


`atomic_glsl_hlsl_cuda_metal`
> (All implemented targets) Capabilities required to use atomic operations<br>


`atomic_glsl_hlsl_cuda9_int64`
> (All implemented targets) Capabilities required to use atomic operations (cuda_sm_9 tier atomics)<br>


`helper_lane`
> Capabilities required to enable helper-lane demotion<br>


`breakpoint`
> Capabilities required to enable shader breakpoints<br>


`raytracing_allstages`
> Collection of capabilities for raytracing with all raytracing stages.<br>


`raytracing_anyhit`
> Collection of capabilities for raytracing with the shader stage of anyhit.<br>


`raytracing_intersection`
> Collection of capabilities for raytracing with the shader stage of intersection.<br>


`raytracing_anyhit_closesthit`
> Collection of capabilities for raytracing with the shader stages of anyhit and closesthit.<br>


`raytracing_anyhit_closesthit_intersection`
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, and intersection.<br>


`raytracing_raygen_closesthit_miss`
> Collection of capabilities for raytracing with the shader stages of raygen, closesthit, and miss.<br>


`raytracing_anyhit_closesthit_intersection_miss`
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, intersection, and miss.<br>


`raytracing_raygen_closesthit_miss_callable`
> Collection of capabilities for raytracing the shader stages of raygen, closesthit, miss, and callable.<br>


`raytracing_position`
> Collection of capabilities for raytracing + ray_tracing_position_fetch and the shader stages of anyhit and closesthit.<br>


`raytracing_motionblur_anyhit_closesthit_intersection_miss`
> Collection of capabilities for raytracing + motion blur and the shader stages of anyhit, closesthit, intersection, and miss.<br>


`raytracing_motionblur_raygen_closesthit_miss`
> Collection of capabilities for raytracing + motion blur and the shader stages of raygen, closesthit, and miss.<br>


`rayquery_position`
> Collection of capabilities for rayquery + ray_tracing_position_fetch.<br>


`ser_raygen`
> Collection of capabilities for raytracing + shader execution reordering and the shader stage of raygen.<br>


`ser_raygen_closesthit_miss`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of raygen, closesthit, and miss.<br>


`ser_any_closesthit_intersection_miss`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, intersection, and miss.<br>


`ser_anyhit_closesthit_intersection`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, and intersection.<br>


`ser_anyhit_closesthit`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit and closesthit.<br>


`ser_motion_raygen_closesthit_miss`
> Collection of capabilities for raytracing + motion blur + shader execution reordering and the shader stages of raygen, closesthit, and miss.<br>


`ser_motion_raygen`
> Collection of capabilities for raytracing raytracing + motion blur + shader execution reordering and the shader stage of raygen.<br>


Other
----------------------
*Capabilities which may be deprecated*

`SPIRV_1_0`
> Use `spirv_1_0` instead<br>


`SPIRV_1_1`
> Use `spirv_1_1` instead<br>


`SPIRV_1_2`
> Use `spirv_1_2` instead<br>


`SPIRV_1_3`
> Use `spirv_1_3` instead<br>


`SPIRV_1_4`
> Use `spirv_1_4` instead<br>


`SPIRV_1_5`
> Use `spirv_1_5` instead<br>


`SPIRV_1_6`
> Use `spirv_1_6` instead<br>


`DX_4_0`
> Use `sm_4_0` instead<br>


`DX_4_1`
> Use `sm_4_1` instead<br>


`DX_5_0`
> Use `sm_5_0` instead<br>


`DX_5_1`
> Use `sm_5_1` instead<br>


`DX_6_0`
> Use `sm_6_0` instead<br>


`DX_6_1`
> Use `sm_6_1` instead<br>


`DX_6_2`
> Use `sm_6_2` instead<br>


`DX_6_3`
> Use `sm_6_3` instead<br>


`DX_6_4`
> Use `sm_6_4` instead<br>


`DX_6_5`
> Use `sm_6_5` instead<br>


`DX_6_6`
> Use `sm_6_6` instead<br>


`DX_6_7`
> Use `sm_6_7` instead<br>


`GLSL_410_SPIRV_1_0`
> User should not use this capability<br>


`GLSL_420_SPIRV_1_0`
> User should not use this capability<br>


`GLSL_430_SPIRV_1_0`
> User should not use this capability<br>


`METAL_2_3`
> Use `metallib_2_3` instead<br>


`METAL_2_4`
> Use `metallib_2_4` instead<br>


`METAL_3_0`
> Use `metallib_3_0` instead<br>


`METAL_3_1`
> Use `metallib_3_1` instead<br>


`GLSL_430_SPIRV_1_0_compute`
> User should not use this capability<br>


`all`
> User should not use this capability<br>

