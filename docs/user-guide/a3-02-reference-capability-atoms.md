---
layout: user-guide
---

Capability Atoms
============================

### Sections:

1. [Targets](#targets)
2. [Stages](#stages)
3. [Versions](#versions)
4. [Extensions](#extensions)
5. [Compound Capabilities](#compound-capabilities)
6. [Other](#other)

Targets
----------------------
*Capabilities to specify code generation targets (`glsl`, `spirv`...)*

`c`
> Represents the C programming language code generation target.

`cpp`
> Represents the C++ programming language code generation target.

`cuda`
> Represents the CUDA code generation target.

`glsl`
> Represents the GLSL code generation target.

`hlsl`
> Represents the HLSL code generation target.

`llvm`
> Represents the LLVM IR target.

`metal`
> Represents the Metal programming language code generation target.

`slangvm`
> Represents the Slang VM bytecode target.

`spirv`
> Represents the SPIR-V code generation target.

`textualTarget`
> Represents a non-assembly code generation target.

`wgsl`
> Represents the WebGPU shading language code generation target.

Stages
----------------------
*Capabilities to specify code generation stages (`vertex`, `fragment`...)*

`amplification`
> Amplification shader stage & mesh shader capabilities

`anyhit`
> Any-Hit shader stage & ray-tracing capabilities

`callable`
> Callable shader stage & ray-tracing capabilities

`closesthit`
> Closest-Hit shader stage & ray-tracing capabilities

`compute`
> Compute shader stage

`dispatch`
> Dispatch shader stage

`domain`
> Domain shader stage

`fragment`
> Fragment shader stage

`geometry`
> Geometry shader stage

`hull`
> Hull shader stage

`intersection`
> Intersection shader stage & ray-tracing capabilities

`mesh`
> Mesh shader stage & mesh shader capabilities

`miss`
> Ray-Miss shader stage & ray-tracing capabilities

`pixel`
> Pixel shader stage

`raygen`
> Ray-Generation shader stage & ray-tracing capabilities

`raygeneration`
> Ray-Generation shader stage & ray-tracing capabilities

`task`
> Task shader stage & mesh shader capabilities

`tesscontrol`
> Tessellation Control shader stage

`tesseval`
> Tessellation Evaluation shader stage

`vertex`
> Vertex shader stage

Versions
----------------------
*Capabilities to specify versions of a code generation target (`sm_5_0`, `GLSL_400`...)*

`GLSL_130`
> GLSL 130 and related capabilities of other targets.

`GLSL_140`
> GLSL 140 and related capabilities of other targets.

`GLSL_150`
> GLSL 150 and related capabilities of other targets.

`GLSL_330`
> GLSL 330 and related capabilities of other targets.

`GLSL_400`
> GLSL 400 and related capabilities of other targets.

`GLSL_410`
> GLSL 410 and related capabilities of other targets.

`GLSL_420`
> GLSL 420 and related capabilities of other targets.

`GLSL_430`
> GLSL 430 and related capabilities of other targets.

`GLSL_440`
> GLSL 440 and related capabilities of other targets.

`GLSL_450`
> GLSL 450 and related capabilities of other targets.

`GLSL_460`
> GLSL 460 and related capabilities of other targets.

`cuda_sm_1_0`
> cuda 1.0 and related capabilities of other targets.

`cuda_sm_2_0`
> cuda 2.0 and related capabilities of other targets.

`cuda_sm_3_0`
> cuda 3.0 and related capabilities of other targets.

`cuda_sm_3_5`
> cuda 3.5 and related capabilities of other targets.

`cuda_sm_4_0`
> cuda 4.0 and related capabilities of other targets.

`cuda_sm_5_0`
> cuda 5.0 and related capabilities of other targets.

`cuda_sm_6_0`
> cuda 6.0 and related capabilities of other targets.

`cuda_sm_7_0`
> cuda 7.0 and related capabilities of other targets.

`cuda_sm_8_0`
> cuda 8.0 and related capabilities of other targets.

`cuda_sm_9_0`
> cuda 9.0 and related capabilities of other targets.

`dxil_lib`
> Represents capabilities required for DXIL Library compilation.

`glsl_spirv_1_0`
> Represents SPIR-V 1.0 through glslang.

`glsl_spirv_1_1`
> Represents SPIR-V 1.1 through glslang.

`glsl_spirv_1_2`
> Represents SPIR-V 1.2 through glslang.

`glsl_spirv_1_3`
> Represents SPIR-V 1.3 through glslang.

`glsl_spirv_1_4`
> Represents SPIR-V 1.4 through glslang.

`glsl_spirv_1_5`
> Represents SPIR-V 1.5 through glslang.

`glsl_spirv_1_6`
> Represents SPIR-V 1.6 through glslang.

`hlsl_2018`
> Represent HLSL compatibility support.

`hlsl_nvapi`
> Represents HLSL NVAPI support.

`metallib_2_3`
> Represents MetalLib 2.3.

`metallib_2_4`
> Represents MetalLib 2.4.

`metallib_3_0`
> Represents MetalLib 3.0.

`metallib_3_1`
> Represents MetalLib 3.1.

`metallib_latest`
> Represents the latest MetalLib version.

`sm_4_0`
> HLSL shader model 4.0 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_4_0_version`
> HLSL shader model 4.0 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_4_1`
> HLSL shader model 4.1 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_4_1_version`
> HLSL shader model 4.1 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_5_0`
> HLSL shader model 5.0 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_5_0_version`
> HLSL shader model 5.0 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_5_1`
> HLSL shader model 5.1 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_5_1_version`
> HLSL shader model 5.1 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_0`
> HLSL shader model 6.0 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_0_version`
> HLSL shader model 6.0 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_1`
> HLSL shader model 6.1 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_1_version`
> HLSL shader model 6.1 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_2`
> HLSL shader model 6.2 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_2_version`
> HLSL shader model 6.2 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_3`
> HLSL shader model 6.3 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_3_version`
> HLSL shader model 6.3 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_4`
> HLSL shader model 6.4 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_4_version`
> HLSL shader model 6.4 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_5`
> HLSL shader model 6.5 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_5_version`
> HLSL shader model 6.5 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_6`
> HLSL shader model 6.6 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_6_version`
> HLSL shader model 6.6 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_7`
> HLSL shader model 6.7 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_7_version`
> HLSL shader model 6.7 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_8`
> HLSL shader model 6.8 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_8_version`
> HLSL shader model 6.8 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`sm_6_9`
> HLSL shader model 6.9 and related capabilities of other targets.
> Includes related GLSL/SPIRV extensions.

`sm_6_9_version`
> HLSL shader model 6.9 and related capabilities of other targets.
> Does not include related GLSL/SPIRV extensions.

`spirv_1_0`
> Represents SPIR-V 1.0 version.

`spirv_1_1`
> Represents SPIR-V 1.1 version, which includes SPIR-V 1.0.

`spirv_1_2`
> Represents SPIR-V 1.2 version, which includes SPIR-V 1.1.

`spirv_1_3`
> Represents SPIR-V 1.3 version, which includes SPIR-V 1.2.

`spirv_1_4`
> Represents SPIR-V 1.4 version, which includes SPIR-V 1.3.

`spirv_1_5`
> Represents SPIR-V 1.5 version, which includes SPIR-V 1.4 and additional extensions.

`spirv_1_6`
> Represents SPIR-V 1.6 version, which includes SPIR-V 1.5 and additional extensions.

`spirv_latest`
> Represents the latest SPIR-V version.

Extensions
----------------------
*Capabilities to specify extensions (`GL_EXT`, `SPV_EXT`...)*

`GL_ARB_derivative_control`
> Represents the GL_ARB_derivative_control extension.

`GL_ARB_fragment_shader_interlock`
> Represents the GL_ARB_fragment_shader_interlock extension.

`GL_ARB_gpu_shader5`
> Represents the GL_ARB_gpu_shader5 extension.

`GL_ARB_gpu_shader_int64`
> Represents the GL_ARB_gpu_shader_int64 extension.

`GL_ARB_shader_clock`
> Represents the GL_ARB_shader_clock extension.

`GL_ARB_shader_clock64`
> Represents the GL_ARB_shader_clock64 extension.

`GL_ARB_shader_image_load_store`
> Represents the GL_ARB_shader_image_load_store extension.

`GL_ARB_shader_image_size`
> Represents the GL_ARB_shader_image_size extension.

`GL_ARB_shader_texture_image_samples`
> Represents the GL_ARB_shader_texture_image_samples extension.

`GL_ARB_sparse_texture`
> Represents the GL_ARB_sparse_texture extension.

`GL_ARB_sparse_texture2`
> Represents the GL_ARB_sparse_texture2 extension.

`GL_ARB_sparse_texture_clamp`
> Represents the GL_ARB_sparse_texture_clamp extension.

`GL_ARB_texture_gather`
> Represents the GL_ARB_texture_gather extension.

`GL_ARB_texture_multisample`
> Represents the GL_ARB_texture_multisample extension.

`GL_ARB_texture_query_levels`
> Represents the GL_ARB_texture_query_levels extension.

`GL_EXT_buffer_reference`
> Represents the GL_EXT_buffer_reference extension.

`GL_EXT_buffer_reference_uvec2`
> Represents the GL_EXT_buffer_reference_uvec2 extension.

`GL_EXT_debug_printf`
> Represents the GL_EXT_debug_printf extension.

`GL_EXT_demote_to_helper_invocation`
> Represents the GL_EXT_demote_to_helper_invocation extension.

`GL_EXT_device_group`
> Represents the GL_EXT_device_group extension.

`GL_EXT_fragment_shader_barycentric`
> Represents the GL_EXT_fragment_shader_barycentric extension.

`GL_EXT_maximal_reconvergence`
> Represents the GL_EXT_maximal_reconvergence extension.

`GL_EXT_mesh_shader`
> Represents the GL_EXT_mesh_shader extension.

`GL_EXT_nonuniform_qualifier`
> Represents the GL_EXT_nonuniform_qualifier extension.

`GL_EXT_ray_query`
> Represents the GL_EXT_ray_query extension.

`GL_EXT_ray_tracing`
> Represents the GL_EXT_ray_tracing extension.

`GL_EXT_ray_tracing_position_fetch`
> Represents the GL_EXT_ray_tracing_position_fetch extension.

`GL_EXT_ray_tracing_position_fetch_ray_query`
> Represents the GL_EXT_ray_tracing_position_fetch_ray_query extension.

`GL_EXT_ray_tracing_position_fetch_ray_tracing`
> Represents the GL_EXT_ray_tracing_position_fetch_ray_tracing extension.

`GL_EXT_samplerless_texture_functions`
> Represents the GL_EXT_samplerless_texture_functions extension.

`GL_EXT_shader_atomic_float`
> Represents the GL_EXT_shader_atomic_float extension.

`GL_EXT_shader_atomic_float2`
> Represents the GL_EXT_shader_atomic_float2 extension.

`GL_EXT_shader_atomic_float_min_max`
> Represents the GL_EXT_shader_atomic_float_min_max extension.

`GL_EXT_shader_atomic_int64`
> Represents the GL_EXT_shader_atomic_int64 extension.

`GL_EXT_shader_explicit_arithmetic_types`
> Represents the GL_EXT_shader_explicit_arithmetic_types extension.

`GL_EXT_shader_explicit_arithmetic_types_int64`
> Represents the GL_EXT_shader_explicit_arithmetic_types_int64 extension.

`GL_EXT_shader_image_load_store`
> Represents the GL_EXT_shader_image_load_store extension.

`GL_EXT_shader_quad_control`
> Represents the GL_EXT_shader_quad_control extension.

`GL_EXT_shader_realtime_clock`
> Represents the GL_EXT_shader_realtime_clock extension.

`GL_EXT_texture_query_lod`
> Represents the GL_EXT_texture_query_lod extension.

`GL_EXT_texture_shadow_lod`
> Represents the GL_EXT_texture_shadow_lod extension.

`GL_KHR_memory_scope_semantics`
> Represents the GL_KHR_memory_scope_semantics extension.

`GL_KHR_shader_subgroup_arithmetic`
> Represents the GL_KHR_shader_subgroup_arithmetic extension.

`GL_KHR_shader_subgroup_ballot`
> Represents the GL_KHR_shader_subgroup_ballot extension.

`GL_KHR_shader_subgroup_basic`
> Represents the GL_KHR_shader_subgroup_basic extension.

`GL_KHR_shader_subgroup_clustered`
> Represents the GL_KHR_shader_subgroup_clustered extension.

`GL_KHR_shader_subgroup_quad`
> Represents the GL_KHR_shader_subgroup_quad extension.

`GL_KHR_shader_subgroup_rotate`
> Represents the GL_KHR_shader_subgroup_rotate extension.

`GL_KHR_shader_subgroup_shuffle`
> Represents the GL_KHR_shader_subgroup_shuffle extension.

`GL_KHR_shader_subgroup_shuffle_relative`
> Represents the GL_KHR_shader_subgroup_shuffle_relative extension.

`GL_KHR_shader_subgroup_vote`
> Represents the GL_KHR_shader_subgroup_vote extension.

`GL_NV_cluster_acceleration_structure`
> Represents the GL_NV_cluster_acceleration_structure extension.

`GL_NV_compute_shader_derivatives`
> Represents the GL_NV_compute_shader_derivatives extension.
> 
> This capability enables the use of implicit derivatives in compute, ray tracing, and mesh stages.
> 
> This capability changes the interpretation of GLSL implicit-LOD texture sampling functions as follows, matching
> the GLSL shader specification:
> - Derivatives enabled: Implicit-LOD `texture()` functions are assumed to use implicit LOD.
> - Derivatives disabled: Implicit-LOD `texture()` functions are assumed to use the base texture.
> 
> This applies to GLSL as both source and target.

`GL_NV_cooperative_vector`
> Represents the GL_NV_cooperative_vector extension.

`GL_NV_fragment_shader_barycentric`
> Represents the GL_NV_fragment_shader_barycentric extension.

`GL_NV_gpu_shader5`
> Represents the GL_NV_gpu_shader5 extension.

`GL_NV_ray_tracing`
> Represents the GL_NV_ray_tracing extension.

`GL_NV_ray_tracing_motion_blur`
> Represents the GL_NV_ray_tracing_motion_blur extension.

`GL_NV_shader_atomic_fp16_vector`
> Represents the GL_NV_shader_atomic_fp16_vector extension.

`GL_NV_shader_invocation_reorder`
> Represents the GL_NV_shader_invocation_reorder extension.

`GL_NV_shader_subgroup_partitioned`
> Represents the GL_NV_shader_subgroup_partitioned extension.

`GL_NV_shader_texture_footprint`
> Represents the GL_NV_shader_texture_footprint extension.

`SPV_EXT_demote_to_helper_invocation`
> Represents the SPIR-V extension for demoting to helper invocation.

`SPV_EXT_descriptor_indexing`
> Represents the SPIR-V extension for descriptor indexing.

`SPV_EXT_fragment_fully_covered`
> Represents the SPIR-V extension for SPV_EXT_fragment_fully_covered.

`SPV_EXT_fragment_shader_interlock`
> Represents the SPIR-V extension for fragment shader interlock operations.

`SPV_EXT_mesh_shader`
> Represents the SPIR-V extension for mesh shaders.

`SPV_EXT_physical_storage_buffer`
> Represents the SPIR-V extension for physical storage buffer.

`SPV_EXT_replicated_composites`
> Represents the SPIR-V extension for SPV_EXT_replicated_composites.

`SPV_EXT_shader_atomic_float16_add`
> Represents the SPIR-V extension for atomic float16 add operations.

`SPV_EXT_shader_atomic_float_add`
> Represents the SPIR-V extension for atomic float add operations.

`SPV_EXT_shader_atomic_float_min_max`
> Represents the SPIR-V extension for atomic float min/max operations.

`SPV_GOOGLE_user_type`
> Represents the SPIR-V extension for SPV_GOOGLE_user_type.

`SPV_KHR_compute_shader_derivatives`
> Represents the SPIR-V extension for compute shader derivatives.

`SPV_KHR_cooperative_matrix`
> Represents the SPIR-V extension for SPV_KHR_cooperative_matrix.

`SPV_KHR_device_group`
> Represents the SPIR-V extension for device-group information.

`SPV_KHR_fragment_shader_barycentric`
> Represents the SPIR-V extension for fragment shader barycentric.

`SPV_KHR_maximal_reconvergence`
> Represents the SPIR-V extension for maximal reconvergence.

`SPV_KHR_non_semantic_info`
> Represents the SPIR-V extension for non-semantic information.

`SPV_KHR_quad_control`
> Represents the SPIR-V extension for quad group control.

`SPV_KHR_ray_query`
> Represents the SPIR-V extension for ray queries.

`SPV_KHR_ray_tracing`
> Represents the SPIR-V extension for ray tracing.

`SPV_KHR_ray_tracing_position_fetch`
> Represents the SPIR-V extension for ray tracing position fetch.
> Should be used with either SPV_KHR_ray_query or SPV_KHR_ray_tracing.

`SPV_KHR_shader_clock`
> Represents the SPIR-V extension for shader clock.

`SPV_KHR_subgroup_rotate`
> Represents the SPIR-V extension enables rotating values across invocations within a subgroup.

`SPV_KHR_vulkan_memory_model`
> Represents the SPIR-V extension for SPV_KHR_vulkan_memory_model.

`SPV_NV_bindless_texture`
> Represents the SPIR-V extension for SPV_NV_bindless_texture.

`SPV_NV_cluster_acceleration_structure`
> Represents the SPIR-V extension for cluster acceleration structure.

`SPV_NV_compute_shader_derivatives`
> Represents the SPIR-V extension for compute shader derivatives.

`SPV_NV_cooperative_matrix2`
> Represents the SPIR-V extension for SPV_NV_cooperative_matrix2.

`SPV_NV_cooperative_vector`
> Represents the SPIR-V extension for SPV_NV_cooperative_vector.

`SPV_NV_linear_swept_spheres`
> Represents the SPIR-V extension for linear swept spheres.

`SPV_NV_ray_tracing_motion_blur`
> Represents the SPIR-V extension for ray tracing motion blur.

`SPV_NV_shader_image_footprint`
> Represents the SPIR-V extension for shader image footprint.

`SPV_NV_shader_invocation_reorder`
> Represents the SPIR-V extension for shader invocation reorder.
> Requires SPV_KHR_ray_tracing.

`SPV_NV_shader_subgroup_partitioned`
> Represents the SPIR-V extension for shader subgroup partitioned.

`SPV_NV_tensor_addressing`
> Represents the SPIR-V extension for SPV_NV_tensor_addressing.

`spvAtomicFloat16AddEXT`
> Represents the SPIR-V capability for atomic float 16 add operations.

`spvAtomicFloat16MinMaxEXT`
> Represents the SPIR-V capability for atomic float 16 min/max operations.

`spvAtomicFloat32AddEXT`
> Represents the SPIR-V capability for atomic float 32 add operations.

`spvAtomicFloat32MinMaxEXT`
> Represents the SPIR-V capability for atomic float 32 min/max operations.

`spvAtomicFloat64AddEXT`
> Represents the SPIR-V capability for atomic float 64 add operations.

`spvAtomicFloat64MinMaxEXT`
> Represents the SPIR-V capability for atomic float 64 min/max operations.

`spvBindlessTextureNV`
> Represents the SPIR-V capability for the bindless texture.

`spvCooperativeMatrixBlockLoadsNV`
> Represents the SPIR-V capability for cooperative matrix 2

`spvCooperativeMatrixConversionsNV`
> Represents the SPIR-V capability for cooperative matrix 2

`spvCooperativeMatrixKHR`
> Represents the SPIR-V capability for cooperative matrices

`spvCooperativeMatrixPerElementOperationsNV`
> Represents the SPIR-V capability for cooperative matrix 2

`spvCooperativeMatrixReductionsNV`
> Represents the SPIR-V capability for cooperative matrix 2

`spvCooperativeMatrixTensorAddressingNV`
> Represents the SPIR-V capability for cooperative matrix 2

`spvCooperativeVectorNV`
> Represents the SPIR-V capability for cooperative vectors

`spvCooperativeVectorTrainingNV`
> Represents the SPIR-V capability for cooperative vector training

`spvDemoteToHelperInvocation`
> Represents the SPIR-V capability for demoting to helper invocation.

`spvDemoteToHelperInvocationEXT`
> Represents the SPIR-V capability for demoting to helper invocation.

`spvDerivativeControl`
> Represents the SPIR-V capability for 'derivative control' operations.

`spvDeviceGroup`
> Represents the SPIR-V capability for DeviceGroup.

`spvFragmentBarycentricKHR`
> Represents the SPIR-V capability for using SPV_KHR_fragment_shader_barycentric.

`spvFragmentFullyCoveredEXT`
> Represents the SPIR-V capability for using SPV_EXT_fragment_fully_covered functionality.

`spvFragmentShaderPixelInterlockEXT`
> Represents the SPIR-V capability for using SPV_EXT_fragment_shader_interlock.

`spvGroupNonUniformArithmetic`
> Represents the SPIR-V capability for group non-uniform arithmetic operations.

`spvGroupNonUniformBallot`
> Represents the SPIR-V capability for group non-uniform ballot operations.

`spvGroupNonUniformPartitionedNV`
> Represents the SPIR-V capability for group non-uniform partitioned operations.

`spvGroupNonUniformQuad`
> Represents the SPIR-V capability for group non-uniform quad operations.

`spvGroupNonUniformRotateKHR`
> Represents the SPIR-V capability for group non-uniform rotate operations.

`spvGroupNonUniformShuffle`
> Represents the SPIR-V capability for group non-uniform shuffle operations.

`spvGroupNonUniformVote`
> Represents the SPIR-V capability for group non-uniform vote operations.

`spvImageFootprintNV`
> Represents the SPIR-V capability for image footprint.

`spvImageGatherExtended`
> Represents the SPIR-V capability for extended image gather operations.

`spvImageQuery`
> Represents the SPIR-V capability for image query operations.

`spvInt64Atomics`
> Represents the SPIR-V capability for 64-bit integer atomics.

`spvMaximalReconvergenceKHR`
> Represents the SPIR-V capability for maximal reconvergence.

`spvMeshShadingEXT`
> Represents the SPIR-V capability for mesh shading.

`spvMinLod`
> Represents the SPIR-V capability for using minimum LOD operations.

`spvQuadControlKHR`
> Represents the SPIR-V capability for quad group control.

`spvRayQueryKHR`
> Represents the SPIR-V capability for ray query.

`spvRayQueryPositionFetchKHR`
> Represents the SPIR-V capability for ray query position fetch.

`spvRayTracingClusterAccelerationStructureNV`
> Represents the SPIR-V capability for cluster acceleration structure.

`spvRayTracingKHR`
> Represents the SPIR-V capability for ray tracing.

`spvRayTracingLinearSweptSpheresGeometryNV`
> Represents the SPIR-V capability for linear swept spheres.

`spvRayTracingMotionBlurNV`
> Represents the SPIR-V capability for ray tracing motion blur.

`spvRayTracingPositionFetchKHR`
> Represents the SPIR-V capability for ray tracing position fetch.

`spvReplicatedCompositesEXT`
> Represents the SPIR-V capability for replicated composites

`spvShaderClockKHR`
> Represents the SPIR-V capability for shader clock.

`spvShaderInvocationReorderNV`
> Represents the SPIR-V capability for shader invocation reorder.

`spvShaderNonUniform`
> Represents the SPIR-V capability for non-uniform resource indexing.

`spvShaderNonUniformEXT`
> Represents the SPIR-V capability for non-uniform resource indexing.

`spvSparseResidency`
> Represents the SPIR-V capability for sparse residency.

`spvTensorAddressingNV`
> Represents the SPIR-V capability for tensor addressing

`spvVulkanMemoryModelDeviceScopeKHR`
> Represents the SPIR-V capability for vulkan memory model.

`spvVulkanMemoryModelKHR`
> Represents the SPIR-V capability for vulkan memory model.

Compound Capabilities
----------------------
*Capabilities to specify capabilities created by other capabilities (`raytracing`, `meshshading`...)*

`amplification_mesh`
> Collection of shader stages

`any_cpp_target`
> All "cpp syntax" code-gen targets

`any_gfx_target`
> All slang-gfx compatible code-gen targets

`any_stage`
> Collection of all shader stages

`any_target`
> All code-gen targets

`any_textual_target`
> All non-asm code-gen targets

`anyhit_closesthit`
> Collection of shader stages

`anyhit_closesthit_intersection`
> Collection of shader stages

`anyhit_closesthit_intersection_miss`
> Collection of shader stages

`appendstructuredbuffer`
> Capabilities required to use AppendStructuredBuffer

`atomic64`
> Capabilities needed for int64/uint64 atomic operations

`atomic_glsl`
> (GLSL/SPIRV) Capabilities required to use GLSL-400 atomic operations

`atomic_glsl_float1`
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-1 float-atomic operations

`atomic_glsl_float2`
> (GLSL/SPIRV) Capabilities required to use GLSL-tier-2 float-atomic operations

`atomic_glsl_halfvec`
> (GLSL/SPIRV) Capabilities required to use NVAPI GLSL-fp16 float-atomic operations

`atomic_glsl_hlsl_cuda9_int64`
> (All implemented targets) Capabilities required to use atomic operations (cuda_sm_9 tier atomics)

`atomic_glsl_hlsl_cuda_metal`
> (All implemented targets) Capabilities required to use atomic operations

`atomic_glsl_hlsl_nvapi_cuda5_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_5 tier atomics)

`atomic_glsl_hlsl_nvapi_cuda6_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_6 tier atomics)

`atomic_glsl_hlsl_nvapi_cuda9_int64`
> (All implemented targets) Capabilities required to use atomic operations of int64 (cuda_sm_9 tier atomics)

`atomic_glsl_hlsl_nvapi_cuda_metal_float1`
> (All implemented targets) Capabilities required to use atomic operations of GLSL tier-1 float atomics

`atomic_glsl_int64`
> (GLSL/SPIRV) Capabilities required to use int64/uint64 atomic operations

`atomic_hlsl`
> (hlsl only) Capabilities required to use hlsl atomic operations

`atomic_hlsl_nvapi`
> (hlsl only) Capabilities required to use hlsl NVAPI atomics

`atomic_hlsl_sm_6_6`
> (hlsl only) Capabilities required to use hlsl sm_6_6 atomics

`atomicfloat`
> Capabilities needed to use GLSL-tier-1 float-atomic operations

`atomicfloat2`
> Capabilities needed to use GLSL-tier-2 float-atomic operations

`breakpoint`
> Capabilities required to enable shader breakpoints

`bufferreference`
> Capabilities needed to use GLSL buffer-reference's

`bufferreference_int64`
> Capabilities needed to use GLSL buffer-reference's with int64

`byteaddressbuffer`
> Capabilities required to use ByteAddressBuffer

`byteaddressbuffer_rw`
> Capabilities required to use RWByteAddressBuffer

`compute_fragment`
> Collection of shader stages

`compute_fragment_geometry_vertex`
> Collection of shader stages

`compute_tesscontrol_tesseval`
> Collection of shader stages

`consumestructuredbuffer`
> Capabilities required to use ConsumeStructuredBuffer

`cooperative_matrix`
> Capabilities needed to use cooperative matrices

`cooperative_matrix_2`
> Capabilities needed to use tensor addressing

`cooperative_matrix_block_load`
> Capabilities needed to use decodeFunc with cooperative matrix load

`cooperative_matrix_conversion`
> Capabilities needed to convert cooperative matrices, all the conversions can be supported by cuda

`cooperative_matrix_map_element`
> Capabilities needed to use MapElement operation with cooperative matrix

`cooperative_matrix_reduction`
> Capabilities needed to use reduction operations with cooperative matrix

`cooperative_matrix_spirv`
> Capabilities needed to use reduction operations with cooperative matrix

`cooperative_matrix_tensor_addressing`
> Capabilities needed to load or store with tensor_addressing extension

`cooperative_vector`
> Capabilities needed to use cooperative vectors
> Note that cpp and cuda are supported via a fallback non-cooperative implementation
> No HLSL shader model bound yet

`cooperative_vector_training`
> Capabilities needed to train cooperative vectors

`cpp_cuda`
> CPP and CUDA code-gen targets

`cpp_cuda_glsl_hlsl`
> CPP, CUDA, GLSL, and HLSL code-gen targets

`cpp_cuda_glsl_hlsl_llvm`
> CPP, CUDA, GLSL, HLSL, and LLVM code-gen targets

`cpp_cuda_glsl_hlsl_metal_spirv`
> CPP, CUDA, GLSL, HLSL, Metal and SPIRV code-gen targets

`cpp_cuda_glsl_hlsl_metal_spirv_llvm`
> CPP, CUDA, GLSL, HLSL, Metal, SPIRV, and LLVM code-gen targets

`cpp_cuda_glsl_hlsl_metal_spirv_wgsl`
> CPP, CUDA, GLSL, HLSL, Metal, SPIRV and WGSL code-gen targets

`cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm`
> CPP, CUDA, GLSL, HLSL, Metal, SPIRV, WGSL and LLVM code-gen targets

`cpp_cuda_glsl_hlsl_spirv`
> CPP, CUDA, GLSL, HLSL, and SPIRV code-gen targets

`cpp_cuda_glsl_hlsl_spirv_llvm`
> CPP, CUDA, GLSL, HLSL, SPIRV, and LLVM code-gen targets

`cpp_cuda_glsl_hlsl_spirv_wgsl`
> CPP, CUDA, GLSL, HLSL, SPIRV and WGSL code-gen targets

`cpp_cuda_glsl_hlsl_spirv_wgsl_llvm`
> CPP, CUDA, GLSL, HLSL, SPIRV, WGSL and LLVM code-gen targets

`cpp_cuda_glsl_spirv`
> CPP, CUDA, GLSL and SPIRV code-gen targets

`cpp_cuda_hlsl`
> CPP, CUDA, and HLSL code-gen targets

`cpp_cuda_hlsl_metal_spirv`
> CPP, CUDA, HLSL, Metal, and SPIRV code-gen targets

`cpp_cuda_hlsl_spirv`
> CPP, CUDA, HLSL, and SPIRV code-gen targets

`cpp_cuda_llvm`
> CPP, CUDA and LLVM code-gen targets

`cpp_cuda_metal_spirv`
> CPP, CUDA, Metal, and SPIRV code-gen targets

`cpp_cuda_spirv`
> CPP, CUDA and SPIRV code-gen targets

`cpp_cuda_spirv_llvm`
> CPP, CUDA, SPIRV and LLVM code-gen targets

`cpp_glsl`
> CPP, and GLSL code-gen targets

`cpp_glsl_hlsl_metal_spirv`
> CPP, GLSL, HLSL, Metal, and SPIRV code-gen targets

`cpp_glsl_hlsl_metal_spirv_wgsl`
> CPP, GLSL, HLSL, Metal, SPIRV and WGSL code-gen targets

`cpp_glsl_hlsl_spirv`
> CPP, GLSL, HLSL, and SPIRV code-gen targets

`cpp_glsl_hlsl_spirv_wgsl`
> CPP, GLSL, HLSL, SPIRV and WGSL code-gen targets

`cpp_hlsl`
> CPP, and HLSL code-gen targets

`cpp_llvm`
> CPP and LLVM code-gen targets

`cuda_glsl_hlsl`
> CUDA, GLSL, and HLSL code-gen targets

`cuda_glsl_hlsl_metal_spirv`
> CUDA, GLSL, HLSL, Metal, and SPIRV code-gen targets

`cuda_glsl_hlsl_metal_spirv_wgsl`
> CUDA, GLSL, HLSL, Metal, SPIRV and WGSL code-gen targets

`cuda_glsl_hlsl_spirv`
> CUDA, GLSL, HLSL, and SPIRV code-gen targets

`cuda_glsl_hlsl_spirv_llvm`
> CUDA, GLSL, HLSL, SPIRV, and LLVM code-gen targets

`cuda_glsl_hlsl_spirv_wgsl`
> CUDA, GLSL, HLSL, SPIRV, and WGSL code-gen targets

`cuda_glsl_metal_spirv`
> CUDA, GLSL, Metal, and SPIRV code-gen targets

`cuda_glsl_metal_spirv_wgsl`
> CUDA, GLSL, Metal, SPIRV and WGSL code-gen targets

`cuda_glsl_metal_spirv_wgsl_llvm`
> CUDA, GLSL, Metal, SPIRV, WGSL and LLVM code-gen targets

`cuda_glsl_spirv`
> CUDA, GLSL, and SPIRV code-gen targets

`cuda_hlsl`
> CUDA, and HLSL code-gen targets

`cuda_hlsl_metal_spirv`
> CUDA, HLSL, Metal, and SPIRV code-gen targets

`cuda_hlsl_spirv`
> CUDA, HLSL, SPIRV code-gen targets

`cuda_spirv`
> CUDA and SPIRV code-gen targets

`domain_hull`
> Collection of shader stages

`fragmentprocessing`
> Capabilities required to use fragment derivative operations (without GLSL derivativecontrol)

`fragmentprocessing_derivativecontrol`
> Capabilities required to use fragment derivative operations (with GLSL derivativecontrol)

`fragmentshaderbarycentric`
> Capabilities needed to use fragment-shader-barycentric's

`fragmentshaderinterlock`
> Capabilities needed for interlocked-fragment operations

`getattributeatvertex`
> Capabilities required to use 'getAttributeAtVertex'

`glsl_hlsl_metal_spirv`
> GLSL, HLSL, Metal, and SPIRV code-gen targets

`glsl_hlsl_metal_spirv_wgsl`
> GLSL, HLSL, Metal, SPIRV and WGSL code-gen targets

`glsl_hlsl_spirv`
> GLSL, HLSL, and SPIRV code-gen targets

`glsl_hlsl_spirv_wgsl`
> GLSL, HLSL, SPIRV and WGSL code-gen targets

`glsl_metal_spirv`
> GLSL, Metal, and SPIRV code-gen targets

`glsl_metal_spirv_wgsl`
> GLSL, Metal, SPIRV and WGSL code-gen targets

`glsl_spirv`
> GLSL, and SPIRV code-gen targets

`glsl_spirv_wgsl`
> GLSL, SPIRV, and WGSL code-gen targets

`helper_lane`
> Capabilities required to enable helper-lane demotion

`hlsl_spirv`
> HLSL, and SPIRV code-gen targets

`image_loadstore`
> (GLSL/SPIRV) Capabilities required to use image load/image store operations

`image_samples`
> Capabilities required to query image (RWTexture) sample info

`image_size`
> Capabilities required to query image (RWTexture) size info

`implicit_derivatives_sampling`
> Capabilities required for implicit derivatives sampling.
> 
> This capability is required by texture sampling functions such as `_Texture.Sample()`
> where the level of detail is determined by implicit derivatives.
> 
> @remark In GLSL, implicit level-of-detail `texture()` functions use the base texture when
> the implicit derivatives are unavailable. When this capability is not present, invocations of
> these functions are translated to invocations of `_Texture.SampleLevelZero()`.
> 
> @remark Implicit derivatives for the compute stage can be enabled by declaring capability `GL_NV_compute_shader_derivatives` (GLSL),
> `SPV_KHR_compute_shader_derivatives` (SPIR-V), or profile `cs_6_6` (HLSL).
> 

`memorybarrier`
> Capabilities required to use sm_5_0 style memory barriers

`meshshading`
> Ccapabilities required to use mesh shading features

`motionblur`
> Capabilities needed for raytracing-motionblur

`nonuniformqualifier`
> Capabilities required to use NonUniform qualifier

`nvapi`
> NVAPI capability for HLSL

`pack_vector`
> Capabilities required to use pack/unpack intrinsics on packed vector data

`printf`
> Capabilities required to use 'printf'

`quad_control`
> Capabilities required to enable quad group control

`raygen_closesthit_miss`
> Collection of shader stages

`raygen_closesthit_miss_callable`
> Collection of shader stages

`rayquery`
> Capabilities needed for compute-shader rayquery

`rayquery_position`
> Collection of capabilities for rayquery + ray_tracing_position_fetch.

`raytracing`
> Capabilities needed for minimal raytracing support

`raytracing_allstages`
> Collection of capabilities for raytracing with all raytracing stages.

`raytracing_anyhit`
> Collection of capabilities for raytracing with the shader stage of anyhit.

`raytracing_anyhit_closesthit`
> Collection of capabilities for raytracing with the shader stages of anyhit and closesthit.

`raytracing_anyhit_closesthit_intersection`
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, and intersection.

`raytracing_anyhit_closesthit_intersection_miss`
> Collection of capabilities for raytracing with the shader stages of anyhit, closesthit, intersection, and miss.

`raytracing_intersection`
> Collection of capabilities for raytracing with the shader stage of intersection.

`raytracing_lss`
> Collection of capabilities for linear swept spheres.

`raytracing_lss_ho`
> hit object APIs allow raygen shaders, but not the non-hit object APIs. So we have this special
> capdef specifically for the hitobject variant.
> Collection of capabilities for linear swept spheres.

`raytracing_motionblur`
> Capabilities needed for compute-shader rayquery and motion-blur

`raytracing_motionblur_anyhit_closesthit_intersection_miss`
> Collection of capabilities for raytracing + motion blur and the shader stages of anyhit, closesthit, intersection, and miss.

`raytracing_motionblur_raygen_closesthit_miss`
> Collection of capabilities for raytracing + motion blur and the shader stages of raygen, closesthit, and miss.

`raytracing_position`
> Collection of capabilities for raytracing + ray_tracing_position_fetch and the shader stages of anyhit and closesthit.

`raytracing_raygen_closesthit_miss`
> Collection of capabilities for raytracing with the shader stages of raygen, closesthit, and miss.

`raytracing_raygen_closesthit_miss_callable`
> Collection of capabilities for raytracing the shader stages of raygen, closesthit, miss, and callable.

`raytracing_stages`
> Collection of shader stages

`raytracingstages_compute`
> Collection of shader stages

`raytracingstages_compute_amplification_mesh`
> Collection of shader stages

`raytracingstages_compute_fragment`
> Collection of shader stages

`raytracingstages_compute_fragment_geometry_vertex`
> Collection of shader stages

`raytracingstages_fragment`
> Collection of shader stages

`ser`
> Capabilities needed for shader-execution-reordering

`ser_any_closesthit_intersection_miss`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, intersection, and miss.

`ser_anyhit_closesthit`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit and closesthit.

`ser_anyhit_closesthit_intersection`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of anyhit, closesthit, and intersection.

`ser_motion`
> Capabilities needed for shader-execution-reordering and motion-blur

`ser_motion_raygen`
> Collection of capabilities for raytracing raytracing + motion blur + shader execution reordering and the shader stage of raygen.

`ser_motion_raygen_closesthit_miss`
> Collection of capabilities for raytracing + motion blur + shader execution reordering and the shader stages of raygen, closesthit, and miss.

`ser_raygen`
> Collection of capabilities for raytracing + shader execution reordering and the shader stage of raygen.

`ser_raygen_closesthit_miss`
> Collection of capabilities for raytracing + shader execution reordering and the shader stages of raygen, closesthit, and miss.

`shader5_sm_4_0`
> Capabilities required to use sm_4_0 features apart of GL_ARB_gpu_shader5

`shader5_sm_5_0`
> Capabilities required to use sm_5_0 features apart of GL_ARB_gpu_shader5

`shaderclock`
> Capabilities needed for realtime clocks

`shaderinvocationgroup`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'

`shadermemorycontrol`
> (gfx targets) Capabilities needed to use memory barriers

`shadermemorycontrol_compute`
> (gfx targets) Capabilities required to use memory barriers that only work for raytracing & compute shader stages

`structuredbuffer`
> Capabilities required to use StructuredBuffer

`structuredbuffer_rw`
> Capabilities required to use RWStructuredBuffer

`subgroup_arithmetic`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_arithmetic'

`subgroup_ballot`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot'

`subgroup_ballot_activemask`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_ballot_activemask'

`subgroup_basic`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic'

`subgroup_basic_ballot`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_basic_ballot'

`subgroup_clustered`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_clustered'

`subgroup_partitioned`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_partitioned'

`subgroup_quad`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_quad'

`subgroup_rotate`
> Capabilities required to use GLSL-style subgroup rotate operations 'subgroup_rotate'

`subgroup_shuffle`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shuffle'

`subgroup_shufflerelative`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_shuffle_relative'

`subgroup_vote`
> Capabilities required to use GLSL-style subgroup operations 'subgroup_vote'

`subpass`
> Capabilities required to use Subpass-Input's

`tensor_addressing`
> Capabilities needed to use tensor addressing

`texture_gather`
> Capabilities required to use 'vertex/fragment/geometry shader only' texture gather operations

`texture_querylevels`
> Capabilities required to query texture level info

`texture_querylod`
> Capabilities required to query texture LOD info

`texture_shadowgrad`
> Capabilities required for shadow texture sampling with bias and gradients.
> New in HLSL SM6.8 but existed in older GLSL and SPIRV targets.

`texture_shadowlod`
> Capabilities required to query shadow texture lod info

`texture_size`
> Capabilities required to query texture sample info

`texture_sm_4_0`
> Capabilities required to use sm_4_0 texture operations

`texture_sm_4_0_fragment`
> Capabilities required to use 'fragment shader only' texture operations

`texture_sm_4_1`
> Capabilities required to use sm_4_1 texture operations

`texture_sm_4_1_clamp_fragment`
> Capabilities required to use 'fragment shader only' texture clamp operations

`texture_sm_4_1_compute_fragment`
> Capabilities required to use 'compute/fragment shader only' texture operations.
> We do not require 'compute'/'fragment' shader capabilities since this seems to be incorrect behavior despite what official documentation says.

`texture_sm_4_1_samplerless`
> Capabilities required to use sm_4_1 samplerless texture operations

`texture_sm_4_1_vertex_fragment_geometry`
> Capabilities required to use 'fragment/geometry shader only' texture clamp operations

`texturefootprint`
> Capabilities required to use basic TextureFootprint operations

`texturefootprintclamp`
> Capabilities required to use TextureFootprint clamp operations

`vk_mem_model`
> Capabilities needed to use vulkan memory model

Other
----------------------
*Capabilities which may be deprecated*

`DX_4_0`
> Use `sm_4_0` instead

`DX_4_1`
> Use `sm_4_1` instead

`DX_5_0`
> Use `sm_5_0` instead

`DX_5_1`
> Use `sm_5_1` instead

`DX_6_0`
> Use `sm_6_0` instead

`DX_6_1`
> Use `sm_6_1` instead

`DX_6_2`
> Use `sm_6_2` instead

`DX_6_3`
> Use `sm_6_3` instead

`DX_6_4`
> Use `sm_6_4` instead

`DX_6_5`
> Use `sm_6_5` instead

`DX_6_6`
> Use `sm_6_6` instead

`DX_6_7`
> Use `sm_6_7` instead

`DX_6_8`
> Use `sm_6_8` instead

`DX_6_9`
> Use `sm_6_9` instead

`GLSL_410_SPIRV_1_0`
> User should not use this capability

`GLSL_420_SPIRV_1_0`
> User should not use this capability

`GLSL_430_SPIRV_1_0`
> User should not use this capability

`GLSL_430_SPIRV_1_0_compute`
> User should not use this capability

`METAL_2_3`
> Use `metallib_2_3` instead

`METAL_2_4`
> Use `metallib_2_4` instead

`METAL_3_0`
> Use `metallib_3_0` instead

`METAL_3_1`
> Use `metallib_3_1` instead

`SPIRV_1_0`
> Use `spirv_1_0` instead

`SPIRV_1_1`
> Use `spirv_1_1` instead

`SPIRV_1_2`
> Use `spirv_1_2` instead

`SPIRV_1_3`
> Use `spirv_1_3` instead

`SPIRV_1_4`
> Use `spirv_1_4` instead

`SPIRV_1_5`
> Use `spirv_1_5` instead

`SPIRV_1_6`
> Use `spirv_1_6` instead

`SPV_KHR_variable_pointers`
> Represents the SPIRV-V Variable Pointers extension.

`all`
> User should not use this capability

`optix_coopvec`
> Represents capabilities required for optix cooperative vector support.

`optix_multilevel_traversal`
> Represents capabilities required for optix multi-level traversal support.
