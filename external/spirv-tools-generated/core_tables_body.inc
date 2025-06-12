// Returns the name of an extension, as an index into kStrings
IndexRange ExtensionToIndexRange(Extension extension) {
  switch(extension) {
    case Extension::kSPV_AMDX_shader_enqueue: return IR(0, 24);
    case Extension::kSPV_AMD_gcn_shader: return IR(24, 19);
    case Extension::kSPV_AMD_gpu_shader_half_float: return IR(43, 30);
    case Extension::kSPV_AMD_gpu_shader_half_float_fetch: return IR(73, 36);
    case Extension::kSPV_AMD_gpu_shader_int16: return IR(109, 25);
    case Extension::kSPV_AMD_shader_ballot: return IR(134, 22);
    case Extension::kSPV_AMD_shader_early_and_late_fragment_tests: return IR(156, 45);
    case Extension::kSPV_AMD_shader_explicit_vertex_parameter: return IR(201, 41);
    case Extension::kSPV_AMD_shader_fragment_mask: return IR(242, 29);
    case Extension::kSPV_AMD_shader_image_load_store_lod: return IR(271, 36);
    case Extension::kSPV_AMD_shader_trinary_minmax: return IR(307, 30);
    case Extension::kSPV_AMD_texture_gather_bias_lod: return IR(337, 32);
    case Extension::kSPV_ARM_cooperative_matrix_layouts: return IR(369, 35);
    case Extension::kSPV_ARM_core_builtins: return IR(404, 22);
    case Extension::kSPV_ARM_tensors: return IR(426, 16);
    case Extension::kSPV_EXT_arithmetic_fence: return IR(442, 25);
    case Extension::kSPV_EXT_demote_to_helper_invocation: return IR(467, 36);
    case Extension::kSPV_EXT_descriptor_indexing: return IR(503, 28);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(531, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(562, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(598, 34);
    case Extension::kSPV_EXT_mesh_shader: return IR(632, 20);
    case Extension::kSPV_EXT_opacity_micromap: return IR(652, 25);
    case Extension::kSPV_EXT_optnone: return IR(677, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(693, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(725, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(769, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(799, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(833, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(865, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(901, 27);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(928, 30);
    case Extension::kSPV_EXT_shader_tile_image: return IR(958, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(984, 36);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1020, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1047, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1078, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1099, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(1121, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(1163, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(1208, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(1247, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(1277, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(1303, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(1328, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(1353, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(1376, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(1420, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(1446, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(1474, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(1497, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(1532, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(1563, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(1597, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(1624, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(1672, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(1703, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(1732, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(1763, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(1796, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(1815, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(1843, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(1886, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(1924, 26);
    case Extension::kSPV_INTEL_int4: return IR(1950, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(1965, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(1984, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2012, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2038, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2058, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2090, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2118, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2143, 33);
    case Extension::kSPV_INTEL_optnone: return IR(2176, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(2194, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(2220, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(2256, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(2280, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(2315, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(2361, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(2381, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(2405, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(2441, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(2476, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(2513, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(2543, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(2575, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(2600, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(2622, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(2643, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(2660, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(2685, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(2720, 27);
    case Extension::kSPV_KHR_device_group: return IR(2747, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(2768, 22);
    case Extension::kSPV_KHR_float_controls: return IR(2790, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(2813, 24);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(2837, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(2873, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(2903, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(2931, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(2952, 30);
    case Extension::kSPV_KHR_multiview: return IR(2982, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3000, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3035, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3061, 32);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3093, 28);
    case Extension::kSPV_KHR_quad_control: return IR(3121, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(3142, 22);
    case Extension::kSPV_KHR_ray_query: return IR(3164, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(3182, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(3202, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(3237, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(3274, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(3308, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(3330, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(3351, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(3382, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(3419, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(3443, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(3481, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(3503, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(3532, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(3567, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(3592, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(3618, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(3646, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(3687, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(3725, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(3749, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(3787, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(3821, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(3847, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(3874, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(3900, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(3929, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(3964, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(3999, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4027, 19);
    case Extension::kSPV_NV_raw_access_chains: return IR(4046, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4071, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(4090, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(4121, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(4158, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(4191, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(4221, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(4254, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(4280, 35);
    case Extension::kSPV_NV_shading_rate: return IR(4315, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(4335, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(4364, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(4389, 23);
    case Extension::kSPV_QCOM_image_processing: return IR(4412, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(4438, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(4465, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(4487, 38);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,156> kExtensionNames{{
    {IR(0, 24), static_cast<uint32_t>(kSPV_AMDX_shader_enqueue)},
    {IR(24, 19), static_cast<uint32_t>(kSPV_AMD_gcn_shader)},
    {IR(43, 30), static_cast<uint32_t>(kSPV_AMD_gpu_shader_half_float)},
    {IR(73, 36), static_cast<uint32_t>(kSPV_AMD_gpu_shader_half_float_fetch)},
    {IR(109, 25), static_cast<uint32_t>(kSPV_AMD_gpu_shader_int16)},
    {IR(134, 22), static_cast<uint32_t>(kSPV_AMD_shader_ballot)},
    {IR(156, 45), static_cast<uint32_t>(kSPV_AMD_shader_early_and_late_fragment_tests)},
    {IR(201, 41), static_cast<uint32_t>(kSPV_AMD_shader_explicit_vertex_parameter)},
    {IR(242, 29), static_cast<uint32_t>(kSPV_AMD_shader_fragment_mask)},
    {IR(271, 36), static_cast<uint32_t>(kSPV_AMD_shader_image_load_store_lod)},
    {IR(307, 30), static_cast<uint32_t>(kSPV_AMD_shader_trinary_minmax)},
    {IR(337, 32), static_cast<uint32_t>(kSPV_AMD_texture_gather_bias_lod)},
    {IR(369, 35), static_cast<uint32_t>(kSPV_ARM_cooperative_matrix_layouts)},
    {IR(404, 22), static_cast<uint32_t>(kSPV_ARM_core_builtins)},
    {IR(426, 16), static_cast<uint32_t>(kSPV_ARM_tensors)},
    {IR(442, 25), static_cast<uint32_t>(kSPV_EXT_arithmetic_fence)},
    {IR(467, 36), static_cast<uint32_t>(kSPV_EXT_demote_to_helper_invocation)},
    {IR(503, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(531, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(562, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(598, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(632, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(652, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(677, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(693, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(725, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(769, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(799, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(833, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(865, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(901, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(928, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(958, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(984, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1020, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1047, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1078, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1099, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(1121, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(1163, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(1208, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(1247, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(1277, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(1303, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(1328, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(1353, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(1376, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(1420, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(1446, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(1474, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(1497, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(1532, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(1563, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(1597, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(1624, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(1672, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(1703, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(1732, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(1763, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(1796, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(1815, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(1843, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(1886, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(1924, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(1950, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(1965, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(1984, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2012, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2038, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2058, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2090, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2118, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2143, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(2176, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(2194, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(2220, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(2256, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(2280, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(2315, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(2361, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(2381, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(2405, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(2441, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(2476, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(2513, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(2543, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(2575, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(2600, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(2622, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(2643, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(2660, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(2685, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(2720, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(2747, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(2768, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(2790, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(2813, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(2837, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(2873, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(2903, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(2931, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(2952, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(2982, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3000, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3035, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3061, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3093, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(3121, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(3142, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(3164, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(3182, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(3202, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(3237, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(3274, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(3308, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(3330, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(3351, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(3382, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(3419, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(3443, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(3481, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(3503, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(3532, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(3567, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(3592, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(3618, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(3646, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(3687, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(3725, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(3749, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(3787, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(3821, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(3847, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(3874, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(3900, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(3929, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(3964, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(3999, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4027, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4046, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4071, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(4090, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(4121, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(4158, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(4191, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(4221, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(4254, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(4280, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(4315, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(4335, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(4364, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(4389, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(4412, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(4438, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(4465, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(4487, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
}};

// Operand descriptions, ordered by (operand kind, operand enum value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   aliases, an IndexRange into kAliasSpans
//   capabilities, an IndexRange into kCapabilitySpans
//   extensions, as an IndexRange into kExtensionSpans
//   version, first version of SPIR-V that has it
//   lastVersion, last version of SPIR-V that has it
static const std::array<OperandDesc, 1177> kOperandsByValue{{
  {0,IR(0, 0),IR(4525, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(4541, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(4551, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(4561, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(4569, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(4590, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(4601, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(4683, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(4699, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(4709, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(4722, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(4735, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(4744, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(4755, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(4845, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(4858, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(4904, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(4952, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(4967, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(4982, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(4992, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5006, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(5016, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(5027, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(5039, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(5066, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(5081, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(5092, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(5102, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(5119, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(5133, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(5147, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(5159, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(5177, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(5196, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(5217, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(5225, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(5236, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(5258, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(5271, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(5286, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(5333, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(5349, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(5362, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(5383, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(5394, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(5420, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(5432, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(5446, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(5472, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(5485, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(5498, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(5508, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(5521, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(5576, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(5609, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(5642, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(5675, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(5708, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(5734, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(5747, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(5757, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(5804, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(5828, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(5848, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(5863, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(5894, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(5912, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(5930, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(5950, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(5978, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6004, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(6023, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(6050, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(6075, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(6097, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(6132, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(6179, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(6195, 15)/* ViewportMaskNV */,IR(0, 0),IR(44, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(6231, 20)/* SecondaryPositionNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(6270, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(6294, 18)/* PositionPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(6332, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(6354, 16)/* FullyCoveredEXT */,IR(0, 0),IR(49, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(6394, 12)/* TaskCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(6406, 17)/* PrimitiveCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(6423, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(6442, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(6464, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(6486, 15)/* LayerPerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(6501, 16)/* MeshViewCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(6517, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(6535, 13)/* BaryCoordKHR */,IR(6, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(6583, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(6622, 12)/* FragSizeEXT */,IR(8, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(6668, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(6713, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(6738, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(6762, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(6790, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(6807, 12)/* LaunchIdKHR */,IR(10, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(6830, 14)/* LaunchSizeKHR */,IR(11, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(6857, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(6892, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(6933, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(6970, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(7013, 11)/* RayTminKHR */,IR(16, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(7034, 11)/* RayTmaxKHR */,IR(17, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(7055, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(7100, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(7133, 17)/* WorldToObjectKHR */,IR(20, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(7166, 7)/* HitTNV */,IR(0, 0),IR(56, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(7173, 11)/* HitKindKHR */,IR(21, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(7194, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(57, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(7234, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(58, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(7291, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(7358, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(7395, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(7434, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(60, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(7454, 14)/* HitIsSphereNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(7496, 11)/* HitIsLSSNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(7546, 20)/* HitSpherePositionNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(7566, 13)/* WarpsPerSMNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(7598, 10)/* SMCountNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(7608, 9)/* WarpIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(7617, 7)/* SMIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(7624, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(7642, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(7676, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(7709, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(7727, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(7741, 12)/* ClusterIDNV */,IR(0, 0),IR(64, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(7794, 12)/* CullMaskKHR */,IR(0, 0),IR(65, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(7821, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(4692, 7)/* Shader */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(4767, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(4776, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(4580, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(7828, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(4534, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(7836, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(7845, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(7859, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(7867, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(7875, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(7881, 13)/* Int64Atomics */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(7894, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(7905, 15)/* ImageReadWrite */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(7920, 12)/* ImageMipmap */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(7932, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(7938, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(7945, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(7959, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(7974, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(7988, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(7994, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(8016, 18)/* GeometryPointSize */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(8034, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(8054, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(8078, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(8112, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(8145, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(8179, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(4709, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(4722, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(8212, 15)/* ImageCubeArray */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(5048, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(8244, 10)/* ImageRect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(8254, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(8266, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(8281, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(8286, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(8302, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(8318, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(8325, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(8335, 8)/* Image1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(8227, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(8343, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(8357, 12)/* ImageBuffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(8369, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(8382, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(8410, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(8421, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(8439, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(8461, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(8479, 16)/* GeometryStreams */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(8495, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(8525, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(4918, 14)/* MultiViewport */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(8556, 17)/* SubgroupDispatch */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(8573, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(8586, 12)/* PipeStorage */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(5299, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(8598, 20)/* GroupNonUniformVote */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(8618, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(5554, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(8644, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(8667, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(8698, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(8723, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(4864, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(4932, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(8743, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(5456, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(8761, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(8789, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(8817, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(8847, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(8858, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(8895, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(8935, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(5781, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(5315, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(5719, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(8963, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(8996, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(77, 1),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(9039, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(77, 1),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(9083, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(9099, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(40, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(9152, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(78, 1),IR(40, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(9204, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(40, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(9226, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(40, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(5816, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(5838, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(9247, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(41, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(9277, 17)/* VariablePointers */,IR(0, 0),IR(79, 1),IR(41, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(9294, 17)/* AtomicStorageOps */,IR(0, 0),IR(80, 1),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(9311, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(43, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(9339, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(9363, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(81, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(9397, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(9418, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(9433, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(9451, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(9476, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(9492, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(9508, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(9531, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(9543, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(47, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(9562, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(82, 2),IR(48, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(4802, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(9594, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(9620, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(9641, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(5878, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(51, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(9663, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(9686, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(9702, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(9724, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(6115, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(9740, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(9761, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(9775, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(6161, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(9790, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(9805, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(9819, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(84, 2),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(9867, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(9883, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(86, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(9905, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(87, 2),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(9934, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(9963, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(69, 1),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(4876, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(89, 1),IR(65, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(6210, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(90, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(6251, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(91, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(6312, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(6370, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(4816, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(10018, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(4830, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(6560, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(10057, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(70, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(6649, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(10132, 29)/* GroupNonUniformPartitionedNV */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(10161, 17)/* ShaderNonUniform */,IR(29, 1),IR(3, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(10198, 23)/* RuntimeDescriptorArray */,IR(30, 1),IR(3, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(10247, 36)/* InputAttachmentArrayDynamicIndexing */,IR(31, 1),IR(92, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(10322, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(32, 1),IR(73, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(10403, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(93, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(10484, 37)/* UniformBufferArrayNonUniformIndexing */,IR(34, 1),IR(94, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(10561, 36)/* SampledImageArrayNonUniformIndexing */,IR(35, 1),IR(94, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(10636, 37)/* StorageBufferArrayNonUniformIndexing */,IR(36, 1),IR(94, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(10713, 36)/* StorageImageArrayNonUniformIndexing */,IR(37, 1),IR(94, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(10788, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(38, 1),IR(95, 2),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(10869, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(39, 1),IR(97, 2),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(10956, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(99, 2),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(7264, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(4789, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(7211, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(11043, 18)/* VulkanMemoryModel */,IR(41, 1),IR(1, 0),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(11082, 29)/* VulkanMemoryModelDeviceScope */,IR(42, 1),IR(1, 0),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(4652, 31)/* PhysicalStorageBufferAddresses */,IR(43, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(11177, 32)/* ComputeDerivativeGroupLinearKHR */,IR(44, 1),IR(3, 1),IR(70, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(11240, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(11265, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(11285, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(11318, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(7579, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(11356, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(11388, 25)/* DemoteToHelperInvocation */,IR(45, 1),IR(3, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(11441, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(11464, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(11493, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(60, 1),IR(81, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(11519, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(11537, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(11562, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(11582, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(84, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(7325, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(60, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(11604, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(7468, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(7507, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(11622, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(11652, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(11683, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(11723, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(11759, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(11789, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(7753, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(60, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(11817, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(11836, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(11857, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(11884, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(11910, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(11941, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(11962, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(11985, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(12008, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(12030, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(12054, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(12063, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(12086, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(12109, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(12132, 19)/* VectorComputeINTEL */,IR(0, 0),IR(101, 1),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(12151, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(12166, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(12182, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(12215, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(12253, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(12292, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(12317, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(12343, 26)/* FPGAMemoryAttributesINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(12369, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(12389, 32)/* ArbitraryPrecisionIntegersINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(12421, 37)/* ArbitraryPrecisionFloatingPointINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(12458, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(12488, 22)/* FPGALoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(12510, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(12532, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(12558, 24)/* FPGAMemoryAccessesINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(12582, 27)/* FPGAClusterAttributesINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(12609, 14)/* LoopFuseINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(12623, 20)/* FPGADSPControlINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(12643, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(12669, 40)/* FPGAInvocationPipeliningAttributesINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(12709, 24)/* FPGABufferLocationINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(12733, 34)/* ArbitraryPrecisionFixedPointINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(12767, 23)/* USMStorageClassesINTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(12790, 29)/* RuntimeAlignedAttributeINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(12819, 13)/* IOPipesINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(12832, 19)/* BlockingPipesINTEL */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(12851, 13)/* FPGARegINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(12864, 19)/* DotProductInputAll */,IR(46, 1),IR(1, 0),IR(119, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(12905, 22)/* DotProductInput4x8Bit */,IR(47, 1),IR(102, 1),IR(119, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(12952, 28)/* DotProductInput4x8BitPacked */,IR(48, 1),IR(1, 0),IR(119, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(13011, 11)/* DotProduct */,IR(49, 1),IR(1, 0),IR(119, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(7806, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(9846, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(13036, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(13060, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(13076, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(76, 1),IR(123, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(13101, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(13116, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(125, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(13136, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(125, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(13156, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(126, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(13176, 11)/* OptNoneEXT */,IR(50, 1),IR(1, 0),IR(127, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(13200, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(129, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(13220, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(13241, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(13265, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(13283, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(133, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(13302, 29)/* FPGAClusterAttributesV2INTEL */,IR(0, 0),IR(103, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(13331, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(104, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(13359, 18)/* TaskSequenceINTEL */,IR(0, 0),IR(1, 0),IR(134, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(13377, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(135, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(13393, 24)/* FPGALatencyControlINTEL */,IR(0, 0),IR(1, 0),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(13417, 28)/* FPGAArgumentInterfacesINTEL */,IR(0, 0),IR(1, 0),IR(137, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(13445, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(138, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(13475, 35)/* GlobalVariableFPGADecorationsINTEL */,IR(0, 0),IR(1, 0),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(13510, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(140, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(13538, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(13561, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(105, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(13591, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(105, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(13621, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(142, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(13659, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(13687, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(13713, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(145, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(13740, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(13765, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(13784, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(148, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(13804, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(13824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(13852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(13920, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(13935, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(13960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(13976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(13990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14199, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14215, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14235, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(14309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(14320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(14352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(14359, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14404, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(14415, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(14425, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(14435, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(14445, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(14458, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(14472, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(14486, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(14500, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(14515, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(14531, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(14547, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(14563, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(14582, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(14603, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(14615, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(14627, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14639, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(14654, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(14679, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(14707, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(14715, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(14742, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(14769, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(14796, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(14828, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(14854, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(14858, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(14865, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(14869, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(14880, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(14891, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(14912, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14923, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(14937, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(14958, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(13824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(13885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(13960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(13976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(13990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(14265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(14270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(14309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(14320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(14329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(14334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(14341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(14352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(14368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(14976, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(14993, 7)/* SpecId */,IR(0, 0),IR(106, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(15000, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(15006, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(15018, 9)/* RowMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(15027, 9)/* ColMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(15036, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(15048, 13)/* MatrixStride */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(15061, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(15072, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(15083, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(15117, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(15125, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(15139, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(15144, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(15150, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(15159, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(15166, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(15176, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(15185, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(15193, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(15202, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(15211, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(15220, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(15232, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(15244, 8)/* Uniform */,IR(0, 0),IR(108, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(15278, 10)/* UniformId */,IR(0, 0),IR(108, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(15288, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(15308, 7)/* Stream */,IR(0, 0),IR(110, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(15315, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(15324, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(15334, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(15340, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(15348, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(15362, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(15369, 10)/* XfbBuffer */,IR(0, 0),IR(111, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(15379, 10)/* XfbStride */,IR(0, 0),IR(111, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(15435, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(15483, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(15533, 15)/* FPFastMathMode */,IR(0, 0),IR(112, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(15610, 18)/* LinkageAttributes */,IR(0, 0),IR(114, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(15628, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(15642, 21)/* InputAttachmentIndex */,IR(0, 0),IR(92, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(15663, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(15673, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(15707, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(15719, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(15735, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(150, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(15748, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(150, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(15763, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(15781, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(15803, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(15825, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(15843, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(15875, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(15895, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(15918, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(15938, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(15963, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(15990, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(16015, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(16043, 19)/* OverrideCoverageNV */,IR(0, 0),IR(115, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(16062, 14)/* PassthroughNV */,IR(0, 0),IR(116, 1),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(16076, 19)/* ViewportRelativeNV */,IR(0, 0),IR(91, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(16095, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(16123, 16)/* PerPrimitiveEXT */,IR(51, 1),IR(117, 2),IR(151, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(16154, 10)/* PerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(16164, 10)/* PerTaskNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(16174, 13)/* PerVertexKHR */,IR(52, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(16199, 11)/* NonUniform */,IR(53, 1),IR(94, 1),IR(73, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(16224, 16)/* RestrictPointer */,IR(54, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(16259, 15)/* AliasedPointer */,IR(55, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(16292, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(16322, 18)/* BindlessSamplerNV */,IR(0, 0),IR(120, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(16340, 16)/* BindlessImageNV */,IR(0, 0),IR(120, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(16356, 15)/* BoundSamplerNV */,IR(0, 0),IR(120, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(16371, 13)/* BoundImageNV */,IR(0, 0),IR(120, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(16384, 14)/* SIMTCallINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(16398, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(122, 1),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(16424, 13)/* ClobberINTEL */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(16437, 17)/* SideEffectsINTEL */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(16454, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(16481, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(16502, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(16529, 15)/* StackCallINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(16544, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(16570, 14)/* CounterBuffer */,IR(56, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(16608, 13)/* UserSemantic */,IR(57, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(16640, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(154, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(16655, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(16712, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(16736, 14)/* RegisterINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(16750, 12)/* MemoryINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(16762, 14)/* NumbanksINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(16776, 15)/* BankwidthINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(16791, 22)/* MaxPrivateCopiesINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(16813, 16)/* SinglepumpINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(16829, 16)/* DoublepumpINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(16845, 19)/* MaxReplicatesINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(16864, 20)/* SimpleDualPortINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(16884, 11)/* MergeINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(16937, 14)/* BankBitsINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(16951, 20)/* ForcePow2DepthINTEL */,IR(0, 0),IR(125, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(16971, 16)/* StridesizeINTEL */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(16987, 14)/* WordsizeINTEL */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(17001, 18)/* TrueDualPortINTEL */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(17019, 19)/* BurstCoalesceINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(17038, 15)/* CacheSizeINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(17053, 28)/* DontStaticallyCoalesceINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(17081, 14)/* PrefetchINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(17095, 17)/* StallEnableINTEL */,IR(0, 0),IR(103, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(17112, 25)/* FuseLoopsInFunctionINTEL */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(17137, 19)/* MathOpDSPModeINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(17156, 16)/* AliasScopeINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(17172, 13)/* NoAliasINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(17185, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(17209, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(17229, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(17249, 20)/* BufferLocationINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(17269, 19)/* IOPipeStorageINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(17322, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(17353, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(17378, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(17413, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(17431, 15)/* StallFreeINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(17477, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(17503, 25)/* LatencyControlLabelINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(17528, 30)/* LatencyControlConstraintINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(17558, 27)/* ConduitKernelArgumentINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(17585, 31)/* RegisterMapKernelArgumentINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(17616, 33)/* MMHostInterfaceAddressWidthINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(17649, 30)/* MMHostInterfaceDataWidthINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(17679, 28)/* MMHostInterfaceLatencyINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(17741, 34)/* MMHostInterfaceReadWriteModeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(17775, 29)/* MMHostInterfaceMaxBurstINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(17804, 32)/* MMHostInterfaceWaitRequestINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(17836, 26)/* StableKernelArgumentINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(17901, 16)/* HostAccessINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(17964, 14)/* InitModeINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(17978, 28)/* ImplementInRegisterMapINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(18042, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(18101, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(18124, 3)/* 1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(18127, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(18130, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(18133, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(18138, 5)/* Rect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(18143, 7)/* Buffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(18150, 12)/* SubpassData */,IR(0, 0),IR(92, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(18162, 17)/* TileImageDataEXT */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(18179, 12)/* Invocations */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(18191, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(18204, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(18226, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(18247, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(18261, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(18276, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(18295, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(18311, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(18327, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(18346, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(18356, 4)/* Xfb */,IR(0, 0),IR(111, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(18360, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(18375, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(18388, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(18398, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(18413, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(18423, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(18437, 12)/* InputPoints */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(18449, 11)/* InputLines */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(18460, 20)/* InputLinesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(18480, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(18490, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(18514, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(18520, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(18529, 15)/* OutputVertices */,IR(0, 0),IR(141, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(18544, 13)/* OutputPoints */,IR(0, 0),IR(145, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(18557, 16)/* OutputLineStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(18573, 20)/* OutputTriangleStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(18593, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(18605, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(18620, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(18632, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(5286, 13)/* SubgroupSize */,IR(0, 0),IR(148, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(18642, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(148, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(18664, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(148, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(18688, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(18700, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(18716, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(18750, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(18784, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(18820, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(155, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(18850, 18)/* PostDepthCoverage */,IR(0, 0),IR(151, 1),IR(43, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(9418, 15)/* DenormPreserve */,IR(0, 0),IR(152, 1),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(9433, 18)/* DenormFlushToZero */,IR(0, 0),IR(153, 1),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(9451, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(154, 1),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(9476, 16)/* RoundingModeRTE */,IR(0, 0),IR(155, 1),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(9492, 16)/* RoundingModeRTZ */,IR(0, 0),IR(156, 1),IR(45, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(18868, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(18902, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(18922, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(156, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(18951, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(18974, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(18989, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(19004, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(19025, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(6179, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(19049, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(19070, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(19098, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(19124, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(19147, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(19174, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(19199, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(19221, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(19240, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(19260, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(19280, 15)/* OutputLinesEXT */,IR(58, 1),IR(117, 2),IR(151, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(19309, 20)/* OutputPrimitivesEXT */,IR(59, 1),IR(117, 2),IR(151, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(19348, 24)/* DerivativeGroupQuadsKHR */,IR(60, 1),IR(158, 2),IR(70, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(19395, 25)/* DerivativeGroupLinearKHR */,IR(61, 1),IR(160, 2),IR(70, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(19444, 19)/* OutputTrianglesEXT */,IR(62, 1),IR(117, 2),IR(151, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(19481, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(162, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(19506, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(162, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(19533, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(163, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(19559, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(163, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(19587, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(164, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(19618, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(164, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(19651, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(19678, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(19699, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(19720, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(19746, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(19773, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(166, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(19795, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(166, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(19811, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(166, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(19831, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(104, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(19853, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(104, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(19881, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(19905, 18)/* FPFastMathDefault */,IR(0, 0),IR(167, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(19923, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(104, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(19947, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(19973, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(19996, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(169, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(20018, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(169, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(20093, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(169, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(20120, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(20127, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(20147, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(4767, 9)/* Geometry */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(14359, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(20170, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(4534, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(20180, 7)/* TaskNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(20187, 7)/* MeshNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(20194, 17)/* RayGenerationKHR */,IR(63, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(20227, 16)/* IntersectionKHR */,IR(64, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(20258, 10)/* AnyHitKHR */,IR(65, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(20277, 14)/* ClosestHitKHR */,IR(66, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(20304, 8)/* MissKHR */,IR(67, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(20319, 12)/* CallableKHR */,IR(68, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(20342, 8)/* TaskEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(20350, 8)/* MeshEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(20358, 9)/* Preserve */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(20367, 12)/* FlushToZero */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(20379, 12)/* BFloat16KHR */,IR(0, 0),IR(86, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(20391, 5)/* IEEE */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(20396, 4)/* ALT */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(20400, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(20407, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(20414, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(20418, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(20429, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(20434, 14)/* AllowContract */,IR(69, 1),IR(170, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(20471, 13)/* AllowReassoc */,IR(70, 1),IR(170, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(20502, 15)/* AllowTransform */,IR(0, 0),IR(167, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(20517, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20521, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(20525, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(20529, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20533, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(20549, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(20565, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(20583, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(20601, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(20608, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(20619, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(20624, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(13176, 11)/* OptNoneEXT */,IR(50, 1),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(20630, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(20635, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(20640, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(20646, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(20651, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(20659, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(20669, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(20677, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(20689, 20)/* RuntimeAlignedINTEL */,IR(0, 0),IR(173, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(20709, 7)/* Reduce */,IR(0, 0),IR(174, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(20716, 14)/* InclusiveScan */,IR(0, 0),IR(174, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(20730, 14)/* ExclusiveScan */,IR(0, 0),IR(174, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(20744, 16)/* ClusteredReduce */,IR(0, 0),IR(177, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(20760, 20)/* PartitionedReduceNV */,IR(0, 0),IR(178, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(20780, 27)/* PartitionedInclusiveScanNV */,IR(0, 0),IR(178, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(20807, 27)/* PartitionedExclusiveScanNV */,IR(0, 0),IR(178, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(20834, 10)/* NoneINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(20844, 10)/* ReadINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(20854, 11)/* WriteINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(20865, 15)/* ReadWriteINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(20880, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(20885, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(20889, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(20894, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(15362, 7)/* Offset */,IR(0, 0),IR(179, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(20906, 13)/* ConstOffsets */,IR(0, 0),IR(179, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(15159, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(8318, 7)/* MinLod */,IR(0, 0),IR(180, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(20919, 19)/* MakeTexelAvailable */,IR(71, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(20960, 17)/* MakeTexelVisible */,IR(72, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(20997, 16)/* NonPrivateTexel */,IR(73, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(21032, 14)/* VolatileTexel */,IR(74, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(21063, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(21074, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(21085, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(21097, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(21105, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(21115, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(21126, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(21136, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(21147, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(21161, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(21175, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(21190, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(21201, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(21213, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(21225, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(21238, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(21252, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(21266, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(13852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(21276, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(21287, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(21304, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(21320, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(21340, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(21360, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(21380, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(21399, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(21418, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(21437, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(21453, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(21469, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(21471, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(21473, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(21476, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(21479, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(21483, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(21488, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(21493, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(21498, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(21508, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(21518, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(21521, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(21525, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(21530, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(21536, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(21549, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(21554, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(21560, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(21566, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(21572, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(21577, 27)/* InitOnDeviceReprogramINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(21604, 23)/* InitOnDeviceResetINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(21627, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(21634, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(21645, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(21659, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(21671, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(21684, 7)/* Export */,IR(0, 0),IR(114, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(21691, 7)/* Import */,IR(0, 0),IR(114, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(21698, 12)/* LinkOnceODR */,IR(0, 0),IR(114, 1),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(21710, 14)/* UncachedINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(21724, 12)/* CachedINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(21736, 15)/* StreamingINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(21751, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(21776, 17)/* ConstCachedINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(21793, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(21800, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(21811, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(21830, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(21847, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(21861, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(21875, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(21893, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(21903, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(17185, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(17209, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(21916, 21)/* DependencyArrayINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(17229, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(21937, 18)/* LoopCoalesceINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(21955, 21)/* MaxInterleavingINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(21976, 26)/* SpeculatedIterationsINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(22002, 14)/* NoFusionINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(22016, 15)/* LoopCountINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(22031, 26)/* MaxReinvocationDelayINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(22057, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(22086, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(22115, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(22136, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(22162, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(22185, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(22208, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(22231, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(22254, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(22271, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(22288, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(22314, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(22340, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(22367, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(15193, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(22394, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(21085, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(22402, 21)/* MakePointerAvailable */,IR(75, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(22447, 19)/* MakePointerVisible */,IR(76, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(22488, 18)/* NonPrivatePointer */,IR(77, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(22527, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(129, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(22547, 17)/* NoAliasINTELMask */,IR(0, 0),IR(129, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(22564, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(22571, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(22579, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(22586, 7)/* Vulkan */,IR(78, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(22603, 8)/* Relaxed */,IR(79, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(22611, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(22619, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(22627, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(22642, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(22665, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(22679, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(22694, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(22710, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(22731, 20)/* AtomicCounterMemory */,IR(0, 0),IR(80, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(22751, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(22763, 13)/* OutputMemory */,IR(80, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(22792, 14)/* MakeAvailable */,IR(81, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(22823, 12)/* MakeVisible */,IR(82, 1),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(15193, 9)/* Volatile */,IR(0, 0),IR(181, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(22850, 10)/* AutoINTEL */,IR(0, 0),IR(169, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(22860, 5)/* WRAP */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(22865, 4)/* SAT */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(22869, 9)/* SAT_ZERO */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(22878, 8)/* SAT_SYM */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(22886, 25)/* PackedVectorFormat4x8Bit */,IR(83, 1),IR(1, 0),IR(119, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(22939, 4)/* TRN */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(22943, 9)/* TRN_ZERO */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(22952, 4)/* RND */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(22956, 9)/* RND_ZERO */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(22965, 8)/* RND_INF */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(22973, 12)/* RND_MIN_INF */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(22985, 9)/* RND_CONV */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(22994, 13)/* RND_CONV_ODD */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(23007, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(23032, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(14707, 8)/* NoneKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(23055, 10)/* OpaqueKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(23065, 12)/* NoOpaqueKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(23077, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(23100, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(23124, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(23151, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(23179, 14)/* CullOpaqueKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(23193, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(82, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(23209, 17)/* SkipTrianglesKHR */,IR(84, 1),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(23250, 13)/* SkipAABBsKHR */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(23263, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(23293, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23334, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23371, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23408, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(23449, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23491, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(23524, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(23557, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(23569, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(23575, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(23582, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(23597, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(23605, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(23612, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(23620, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(23628, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(23636, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(23641, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(23647, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(23658, 6)/* Rg32f */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(23664, 6)/* Rg16f */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(23670, 13)/* R11fG11fB10f */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(23683, 5)/* R16f */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(23688, 7)/* Rgba16 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(23695, 8)/* Rgb10A2 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(23703, 5)/* Rg16 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(23708, 4)/* Rg8 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(23712, 4)/* R16 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(23716, 3)/* R8 */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(23719, 12)/* Rgba16Snorm */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(23731, 10)/* Rg16Snorm */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(23741, 9)/* Rg8Snorm */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(23750, 9)/* R16Snorm */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(23759, 8)/* R8Snorm */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(23767, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(23775, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(23783, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(23790, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(23795, 6)/* Rg32i */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(23801, 6)/* Rg16i */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(23807, 5)/* Rg8i */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(23812, 5)/* R16i */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(23817, 4)/* R8i */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(23821, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(23830, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(23839, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(23847, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(23853, 10)/* Rgb10a2ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(23863, 7)/* Rg32ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(23870, 7)/* Rg16ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(23877, 6)/* Rg8ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(23883, 6)/* R16ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(23889, 5)/* R8ui */,IR(0, 0),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(23894, 6)/* R64ui */,IR(0, 0),IR(189, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(23900, 5)/* R64i */,IR(0, 0),IR(189, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(23905, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(23917, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(23924, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(23934, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(23943, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(23954, 12)/* QueueFamily */,IR(85, 1),IR(181, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(23981, 14)/* ShaderCallKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(23995, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(24003, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24015, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(13824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(13852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(13920, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(13935, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(13960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(13976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(13990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14199, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14215, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14235, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(24044, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(14309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(14320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(14352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(14359, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14404, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(23612, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(24070, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(24075, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(24080, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(24089, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(24100, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(24105, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(24120, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(24125, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(24132, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(24137, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(24142, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(24148, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(24152, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(24157, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(24173, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(15244, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(24179, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(23924, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(24186, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(24201, 8)/* Private */,IR(0, 0),IR(190, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(24209, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(24218, 8)/* Generic */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(24226, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(24239, 14)/* AtomicCounter */,IR(0, 0),IR(80, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(24253, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(24259, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(161, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(24273, 13)/* TileImageEXT */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(24286, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(24305, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(24321, 16)/* CallableDataKHR */,IR(86, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(24352, 24)/* IncomingCallableDataKHR */,IR(87, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(24399, 14)/* RayPayloadKHR */,IR(88, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(24426, 16)/* HitAttributeKHR */,IR(89, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(24457, 22)/* IncomingRayPayloadKHR */,IR(90, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(24500, 22)/* ShaderRecordBufferKHR */,IR(91, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(24543, 22)/* PhysicalStorageBuffer */,IR(92, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(24590, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(24611, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(53, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(24635, 17)/* CodeSectionINTEL */,IR(0, 0),IR(193, 1),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(24652, 16)/* DeviceOnlyINTEL */,IR(0, 0),IR(194, 1),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(24668, 14)/* HostOnlyINTEL */,IR(0, 0),IR(194, 1),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(21710, 14)/* UncachedINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(24682, 18)/* WriteThroughINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(24700, 15)/* WriteBackINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(21736, 15)/* StreamingINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(13955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(24715, 11)/* TensorView */,IR(0, 0),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(24726, 11)/* DecodeFunc */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(24737, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(15202, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(23557, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(23575, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(23582, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(24747, 8)/* NoneARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(24755, 15)/* NontemporalARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(24770, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(24790, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(24814, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(24836, 21)/* NonPrivateElementARM */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

// Maps an operand kind to possible operands for that kind.
// The result is an IndexRange into kOperandsByValue, and the operands
// are sorted by value within that span.
// An optional variant of a kind maps to the details for the corresponding
// concrete operand kind.
IndexRange OperandByValueRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 4);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(7, 126);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(133, 267);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(400, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(408, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(411, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(413, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(432, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(442, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(446, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(461, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(465, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(471, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(474, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(477, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(481, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(489, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(492, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(508, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(517, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(520, 142);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(662, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(670, 96);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(766, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(783, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(785, 1);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(786, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(788, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(797, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(801, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(805, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(811, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(820, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(827, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(831, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(848, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(874, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(894, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(896, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(899, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(901, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(903, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(906, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(911, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(931, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(946, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(955, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(959, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(974, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(975, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(979, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(980, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(988, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(991, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1003, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1005, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1008, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1010, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1015, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1017, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1059, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1066, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1069, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1070, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1078, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1081, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1083, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1103, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1113, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1117, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1131, 28);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1159, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1163, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1166, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1171, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(831, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(946, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(979, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(465, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(931, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(988, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(785, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1171, 6);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1271> kOperandNames{{
  {IR(4525, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4551, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4541, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4561, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4569, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4590, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4601, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4625, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6535, 13), 92}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6548, 12), 92}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5930, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5950, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6583, 20), 93}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6603, 19), 93}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5978, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6075, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6004, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6023, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6050, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5734, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5708, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4709, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6442, 22), 87}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7741, 12), 131}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5472, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5446, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5485, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4722, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6464, 22), 88}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7794, 12), 132}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6790, 17), 99}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7194, 17), 113}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5804, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5747, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5236, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5006, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5092, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6668, 23), 95}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6622, 12), 94}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6097, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6634, 15), 94}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5027, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6354, 16), 83}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5177, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5271, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5258, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5225, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5102, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7496, 11), 120}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7454, 14), 119}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7676, 33), 128}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7642, 34), 127}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7173, 11), 112}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7184, 10), 112}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7624, 18), 126}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7727, 14), 130}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7358, 37), 116}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7291, 34), 115}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7546, 20), 121}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7709, 18), 129}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7166, 7), 111}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7234, 30), 114}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7395, 20), 117}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7415, 19), 117}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7055, 23), 108}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7078, 22), 108}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4744, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5432, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4845, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6691, 22), 95}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6807, 12), 100}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6819, 11), 100}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6830, 14), 101}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6844, 13), 101}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4858, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6486, 15), 89}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5159, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5196, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6501, 16), 90}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6517, 18), 91}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5362, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5349, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5119, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6970, 22), 105}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6992, 21), 105}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6933, 19), 104}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6952, 18), 104}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7100, 17), 109}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7117, 16), 109}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4992, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5016, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4699, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4683, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6294, 18), 81}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6406, 17), 85}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4755, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6423, 19), 86}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6738, 24), 97}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6713, 25), 96}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5757, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6762, 28), 98}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7434, 20), 118}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7034, 11), 107}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7045, 10), 107}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7013, 11), 106}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7024, 10), 106}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6132, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7598, 10), 123}, // 107 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7617, 7), 125}, // 108 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5039, 9), 24}, // 109 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5081, 11), 26}, // 110 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5066, 15), 25}, // 111 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6231, 20), 79}, // 112 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6270, 24), 80}, // 113 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6179, 16), 77}, // 114 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5848, 15), 64}, // 115 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5521, 15), 53}, // 116 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5536, 18), 53}, // 117 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5576, 15), 54}, // 118 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5591, 18), 54}, // 119 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5609, 15), 55}, // 120 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5624, 18), 55}, // 121 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5383, 11), 44}, // 122 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5642, 15), 56}, // 123 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5657, 18), 56}, // 124 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5394, 26), 45}, // 125 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5675, 15), 57}, // 126 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5690, 18), 57}, // 127 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5333, 16), 41}, // 128 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5286, 13), 40}, // 129 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6394, 12), 84}, // 130 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4982, 10), 19}, // 131 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4967, 15), 18}, // 132 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4952, 15), 17}, // 133 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5912, 18), 67}, // 134 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5894, 18), 66}, // 135 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5863, 15), 65}, // 136 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4735, 9), 11}, // 137 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5420, 12), 46}, // 138 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5828, 10), 63}, // 139 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4904, 14), 16}, // 140 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6195, 15), 78}, // 141 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6332, 22), 82}, // 142 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5498, 10), 51}, // 143 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7608, 9), 124}, // 144 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5508, 13), 52}, // 145 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7566, 13), 122}, // 146 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5217, 8), 35}, // 147 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5147, 12), 31}, // 148 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5133, 14), 30}, // 149 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6892, 21), 103}, // 150 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6913, 20), 103}, // 151 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6857, 18), 102}, // 152 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6875, 17), 102}, // 153 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7133, 17), 110}, // 154 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7150, 16), 110}, // 155 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4580, 10), 137}, // 156 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12733, 34), 355}, // 157 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12421, 37), 343}, // 158 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12389, 32), 342}, // 159 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13283, 19), 379}, // 160 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12054, 9), 328}, // 161 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13200, 20), 375}, // 162 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12109, 23), 331}, // 163 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11582, 22), 306}, // 164 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13116, 20), 371}, // 165 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12063, 23), 329}, // 166 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13136, 20), 372}, // 167 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12086, 23), 330}, // 168 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7974, 14), 153}, // 169 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9294, 17), 226}, // 170 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13241, 24), 377}, // 171 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9905, 29), 259}, // 172 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9883, 22), 258}, // 173 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9867, 16), 257}, // 174 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13804, 20), 399}, // 175 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11519, 18), 303}, // 176 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13060, 16), 368}, // 177 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12832, 19), 359}, // 178 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13765, 19), 397}, // 179 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4709, 13), 163}, // 180 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11177, 32), 292}, // 181 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11209, 31), 292}, // 182 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10057, 31), 271}, // 183 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10088, 30), 271}, // 184 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11759, 30), 315}, // 185 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11652, 31), 312}, // 186 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9846, 21), 366}, // 187 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8935, 28), 210}, // 188 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11265, 20), 294}, // 189 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11683, 40), 313}, // 190 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11622, 30), 311}, // 191 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11723, 36), 314}, // 192 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11562, 20), 305}, // 193 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11789, 28), 316}, // 194 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5456, 16), 203}, // 195 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4722, 13), 164}, // 196 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13220, 21), 376}, // 197 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11388, 25), 299}, // 198 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11413, 28), 299}, // 199 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9433, 18), 232}, // 200 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9418, 15), 231}, // 201 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8421, 18), 182}, // 202 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7945, 14), 151}, // 203 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5816, 12), 222}, // 204 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11441, 23), 300}, // 205 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13011, 11), 364}, // 206 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12905, 22), 362}, // 207 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12927, 25), 362}, // 208 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12952, 28), 363}, // 209 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12980, 31), 363}, // 210 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12864, 19), 361}, // 211 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12883, 22), 361}, // 212 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13022, 14), 364}, // 213 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5719, 15), 213}, // 214 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12166, 16), 334}, // 215 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12369, 20), 341}, // 216 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13417, 28), 385}, // 217 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12709, 24), 354}, // 218 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12582, 27), 349}, // 219 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13302, 29), 380}, // 220 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12623, 20), 351}, // 221 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12669, 40), 353}, // 222 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12532, 26), 347}, // 223 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13331, 28), 381}, // 224 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13393, 24), 384}, // 225 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12488, 22), 345}, // 226 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12558, 24), 348}, // 227 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12343, 26), 340}, // 228 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12851, 13), 360}, // 229 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13377, 16), 383}, // 230 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7859, 8), 142}, // 231 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7845, 14), 141}, // 232 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9686, 16), 246}, // 233 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7867, 8), 143}, // 234 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13101, 15), 370}, // 235 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11962, 23), 324}, // 236 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6560, 23), 270}, // 237 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10035, 22), 270}, // 238 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6649, 19), 272}, // 239 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6370, 24), 266}, // 240 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9724, 16), 248}, // 241 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11356, 32), 298}, // 242 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11285, 33), 295}, // 243 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11318, 38), 296}, // 244 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5781, 23), 211}, // 245 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12317, 26), 339}, // 246 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12008, 22), 326}, // 247 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8266, 15), 169}, // 248 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4767, 9), 135}, // 249 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8016, 18), 156}, // 250 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9963, 28), 261}, // 251 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8479, 16), 185}, // 252 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13475, 35), 387}, // 253 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13445, 30), 386}, // 254 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5299, 16), 192}, // 255 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8618, 26), 194}, // 256 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5554, 22), 195}, // 257 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8698, 25), 198}, // 258 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10132, 29), 273}, // 259 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8723, 20), 199}, // 260 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13076, 25), 369}, // 261 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8644, 23), 196}, // 262 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8667, 31), 197}, // 263 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8598, 20), 193}, // 264 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13687, 26), 394}, // 265 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7938, 7), 150}, // 266 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12819, 13), 358}, // 267 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8335, 8), 175}, // 268 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7894, 11), 146}, // 269 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8357, 12), 178}, // 270 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8212, 15), 165}, // 271 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10018, 17), 268}, // 272 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9702, 22), 247}, // 273 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8034, 20), 157}, // 274 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8369, 13), 179}, // 275 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7920, 12), 148}, // 276 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8410, 11), 181}, // 277 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7905, 15), 147}, // 278 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9740, 21), 250}, // 279 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8244, 10), 167}, // 280 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12030, 24), 327}, // 281 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8286, 16), 171}, // 282 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10247, 36), 276}, // 283 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10283, 39), 276}, // 284 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10788, 39), 283}, // 285 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10827, 42), 283}, // 286 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7988, 6), 154}, // 287 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9819, 27), 256}, // 288 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9805, 14), 255}, // 289 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7875, 6), 144}, // 290 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7881, 13), 145}, // 291 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9761, 14), 251}, // 292 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8281, 5), 170}, // 293 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11985, 23), 325}, // 294 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8439, 22), 183}, // 295 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4534, 7), 139}, // 296 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12510, 22), 346}, // 297 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7828, 8), 138}, // 298 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7959, 15), 152}, // 299 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13156, 20), 373}, // 300 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12609, 14), 350}, // 301 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13740, 25), 396}, // 302 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7821, 7), 133}, // 303 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12643, 26), 352}, // 304 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4830, 15), 269}, // 305 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4816, 14), 267}, // 306 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8318, 7), 173}, // 307 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5838, 10), 223}, // 308 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4918, 14), 188}, // 309 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8573, 13), 190}, // 310 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13176, 11), 374}, // 311 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13187, 13), 374}, // 312 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6312, 20), 265}, // 313 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4652, 31), 291}, // 314 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11143, 34), 291}, // 315 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8586, 12), 191}, // 316 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7932, 6), 149}, // 317 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9790, 15), 254}, // 318 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11604, 18), 308}, // 319 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7806, 15), 365}, // 320 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9531, 12), 237}, // 321 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11537, 25), 304}, // 322 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9508, 23), 236}, // 323 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7753, 41), 317}, // 324 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7325, 33), 307}, // 325 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4802, 14), 240}, // 326 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7507, 39), 310}, // 327 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7211, 23), 288}, // 328 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4789, 13), 287}, // 329 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11464, 29), 301}, // 330 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7264, 27), 286}, // 331 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11240, 25), 293}, // 332 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7468, 28), 309}, // 333 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9562, 32), 239}, // 334 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13784, 20), 398}, // 335 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13036, 24), 367}, // 336 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11941, 21), 323}, // 337 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9476, 16), 234}, // 338 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9492, 16), 235}, // 339 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12790, 29), 357}, // 340 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10198, 23), 275}, // 341 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10221, 26), 275}, // 342 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9934, 29), 260}, // 343 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9311, 28), 227}, // 344 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5048, 18), 166}, // 345 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8325, 10), 174}, // 346 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8343, 14), 177}, // 347 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8227, 17), 176}, // 348 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8112, 33), 160}, // 349 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10561, 36), 280}, // 350 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10597, 39), 280}, // 351 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8254, 12), 168}, // 352 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4692, 7), 134}, // 353 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9775, 15), 252}, // 354 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6161, 18), 253}, // 355 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11493, 26), 302}, // 356 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4864, 12), 200}, // 357 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10161, 17), 274}, // 358 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10178, 20), 274}, // 359 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7579, 19), 297}, // 360 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6251, 19), 264}, // 361 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4932, 20), 201}, // 362 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4876, 28), 262}, // 363 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9991, 27), 262}, // 364 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6210, 21), 263}, // 365 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10118, 14), 272}, // 366 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9451, 25), 233}, // 367 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8302, 16), 172}, // 368 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13265, 18), 378}, // 369 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6115, 17), 249}, // 370 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9099, 25), 218}, // 371 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9339, 24), 228}, // 372 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8145, 34), 161}, // 373 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10636, 37), 281}, // 374 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10673, 40), 281}, // 375 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8179, 33), 162}, // 376 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10713, 36), 282}, // 377 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10749, 39), 282}, // 378 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8382, 28), 180}, // 379 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8054, 24), 158}, // 380 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8495, 30), 186}, // 381 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8525, 31), 187}, // 382 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9226, 21), 221}, // 383 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9204, 22), 220}, // 384 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9397, 21), 230}, // 385 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8858, 37), 208}, // 386 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8895, 40), 209}, // 387 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10403, 39), 278}, // 388 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10442, 42), 278}, // 389 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10956, 42), 285}, // 390 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10998, 45), 285}, // 391 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9187, 17), 219}, // 392 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9124, 28), 218}, // 393 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13538, 23), 389}, // 394 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13561, 30), 390}, // 395 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13591, 30), 391}, // 396 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12253, 39), 337}, // 397 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12182, 33), 335}, // 398 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12215, 38), 336}, // 399 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5315, 18), 212}, // 400 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11857, 27), 320}, // 401 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13510, 28), 388}, // 402 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8556, 17), 189}, // 403 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11884, 26), 321}, // 404 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11910, 31), 322}, // 405 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13621, 38), 392}, // 406 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11836, 21), 319}, // 407 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9083, 16), 217}, // 408 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13359, 18), 382}, // 409 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11817, 19), 318}, // 410 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13713, 27), 395}, // 411 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8847, 11), 207}, // 412 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13659, 28), 393}, // 413 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4776, 13), 136}, // 414 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7994, 22), 155}, // 415 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9663, 23), 245}, // 416 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9641, 22), 243}, // 417 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9620, 21), 242}, // 418 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9594, 26), 241}, // 419 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8761, 28), 204}, // 420 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8789, 28), 205}, // 421 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8817, 30), 206}, // 422 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5878, 16), 244}, // 423 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8461, 18), 184}, // 424 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12767, 23), 356}, // 425 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9152, 35), 219}, // 426 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9363, 34), 229}, // 427 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8078, 34), 159}, // 428 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10484, 37), 279}, // 429 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10521, 40), 279}, // 430 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8743, 18), 202}, // 431 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10322, 39), 277}, // 432 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10361, 42), 277}, // 433 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10869, 42), 284}, // 434 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10911, 45), 284}, // 435 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12458, 30), 344}, // 436 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9543, 19), 238}, // 437 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12292, 25), 338}, // 438 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9277, 17), 225}, // 439 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9247, 30), 224}, // 440 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7836, 9), 140}, // 441 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12151, 15), 333}, // 442 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12132, 19), 332}, // 443 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11043, 18), 289}, // 444 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11082, 29), 290}, // 445 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11111, 32), 290}, // 446 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11061, 21), 289}, // 447 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9039, 44), 216}, // 448 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8996, 43), 215}, // 449 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8963, 33), 214}, // 450 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13836, 8), 401}, // 451 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13844, 8), 402}, // 452 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13852, 6), 403}, // 453 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13858, 7), 404}, // 454 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13865, 11), 405}, // 455 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13876, 9), 406}, // 456 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13885, 13), 407}, // 457 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13824, 12), 400}, // 458 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13898, 6), 408}, // 459 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13904, 10), 409}, // 460 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13914, 6), 410}, // 461 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13935, 20), 412}, // 462 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(13920, 15), 411}, // 463 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14044, 15), 420}, // 464 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14059, 13), 421}, // 465 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14032, 12), 419}, // 466 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14122, 21), 425}, // 467 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14015, 17), 418}, // 468 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14199, 16), 429}, // 469 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14003, 12), 417}, // 470 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14183, 16), 428}, // 471 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(13976, 14), 415}, // 472 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(13960, 16), 414}, // 473 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(13990, 13), 416}, // 474 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14143, 20), 426}, // 475 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14087, 18), 423}, // 476 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14072, 15), 422}, // 477 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14163, 20), 427}, // 478 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14105, 17), 424}, // 479 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14235, 24), 431}, // 480 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14215, 20), 430}, // 481 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(13955, 5), 413}, // 482 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14320, 9), 436}, // 483 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14352, 7), 440}, // 484 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14259, 6), 432}, // 485 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14359, 9), 441}, // 486 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14270, 6), 434}, // 487 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14265, 5), 433}, // 488 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14309, 11), 435}, // 489 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14341, 11), 439}, // 490 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14329, 5), 437}, // 491 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14334, 7), 438}, // 492 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14404, 11), 445}, // 493 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14368, 10), 442}, // 494 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14391, 13), 444}, // 495 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14378, 13), 443}, // 496 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14415, 10), 446}, // 497 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14425, 10), 447}, // 498 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14435, 10), 448}, // 499 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14603, 12), 459}, // 500 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14615, 12), 460}, // 501 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14458, 14), 450}, // 502 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14472, 14), 451}, // 503 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14486, 14), 452}, // 504 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14445, 13), 449}, // 505 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14563, 19), 457}, // 506 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14515, 16), 454}, // 507 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14531, 16), 455}, // 508 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14547, 16), 456}, // 509 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14500, 15), 453}, // 510 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14582, 21), 458}, // 511 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14679, 28), 464}, // 512 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14639, 15), 462}, // 513 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14654, 25), 463}, // 514 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14627, 12), 461}, // 515 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14715, 27), 466}, // 516 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14742, 27), 467}, // 517 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14769, 27), 468}, // 518 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14796, 32), 469}, // 519 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14707, 8), 465}, // 520 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14828, 26), 470}, // 521 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14865, 4), 473}, // 522 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14858, 7), 472}, // 523 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14854, 4), 471}, // 524 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14869, 11), 474}, // 525 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14891, 21), 476}, // 526 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14880, 11), 475}, // 527 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14923, 14), 478}, // 528 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14937, 21), 479}, // 529 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14912, 11), 477}, // 530 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14958, 18), 480}, // 531 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(13836, 8), 482}, // 532 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13844, 8), 483}, // 533 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13852, 6), 484}, // 534 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13858, 7), 485}, // 535 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13865, 11), 486}, // 536 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13876, 9), 487}, // 537 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13885, 13), 488}, // 538 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13824, 12), 481}, // 539 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13898, 6), 489}, // 540 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(13904, 10), 490}, // 541 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(13914, 6), 491}, // 542 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14044, 15), 499}, // 543 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14059, 13), 500}, // 544 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14032, 12), 498}, // 545 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14122, 21), 504}, // 546 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14015, 17), 497}, // 547 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14003, 12), 496}, // 548 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14183, 16), 507}, // 549 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(13976, 14), 494}, // 550 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(13960, 16), 493}, // 551 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(13990, 13), 495}, // 552 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14143, 20), 505}, // 553 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14087, 18), 502}, // 554 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14072, 15), 501}, // 555 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14163, 20), 506}, // 556 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14105, 17), 503}, // 557 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(13955, 5), 492}, // 558 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14320, 9), 512}, // 559 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14352, 7), 516}, // 560 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14259, 6), 508}, // 561 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14270, 6), 510}, // 562 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14265, 5), 509}, // 563 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14309, 11), 511}, // 564 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14341, 11), 515}, // 565 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14329, 5), 513}, // 566 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14334, 7), 514}, // 567 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14368, 10), 517}, // 568 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14391, 13), 519}, // 569 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14378, 13), 518}, // 570 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17156, 16), 633}, // 571 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15185, 8), 539}, // 572 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(16259, 15), 591}, // 573 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16274, 18), 591}, // 574 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15663, 10), 563}, // 575 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(15707, 12), 565}, // 576 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(15036, 12), 526}, // 577 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(16937, 14), 621}, // 578 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16776, 15), 614}, // 579 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15340, 8), 552}, // 580 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(16340, 16), 594}, // 581 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16322, 18), 593}, // 582 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15000, 6), 522}, // 583 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(15803, 22), 571}, // 584 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(15781, 22), 570}, // 585 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16371, 13), 596}, // 586 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16356, 15), 595}, // 587 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15006, 12), 523}, // 588 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(17249, 20), 638}, // 589 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15117, 8), 531}, // 590 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(17019, 19), 626}, // 591 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15083, 8), 530}, // 592 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18042, 22), 660}, // 593 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18101, 23), 661}, // 594 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17038, 15), 627}, // 595 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15150, 9), 535}, // 596 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(16424, 13), 599}, // 597 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15211, 9), 542}, // 598 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(15027, 9), 525}, // 599 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(15324, 10), 550}, // 600 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(17558, 27), 648}, // 601 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15202, 9), 541}, // 602 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(16570, 14), 606}, // 603 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15348, 14), 553}, // 604 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(17053, 28), 628}, // 605 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16829, 16), 617}, // 606 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15825, 18), 572}, // 607 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(15533, 15), 559}, // 608 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17477, 26), 645}, // 609 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15483, 15), 558}, // 610 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(15139, 5), 533}, // 611 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(16951, 20), 622}, // 612 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15435, 14), 557}, // 613 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(16481, 21), 602}, // 614 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16712, 24), 610}, // 615 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17322, 31), 640}, // 616 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16655, 26), 609}, // 617 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17112, 25), 631}, // 618 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15072, 11), 529}, // 619 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(15061, 11), 528}, // 620 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(16544, 26), 605}, // 621 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16292, 30), 592}, // 622 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16584, 24), 606}, // 623 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(16621, 19), 607}, // 624 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(17901, 16), 657}, // 625 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17269, 19), 639}, // 626 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17978, 28), 659}, // 627 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15334, 6), 551}, // 628 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(17964, 14), 658}, // 629 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17185, 24), 635}, // 630 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15642, 21), 562}, // 631 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(15166, 10), 537}, // 632 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(17528, 30), 647}, // 633 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17503, 25), 646}, // 634 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15610, 18), 560}, // 635 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(15315, 9), 549}, // 636 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(17616, 33), 650}, // 637 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17649, 30), 651}, // 638 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17679, 28), 652}, // 639 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17775, 29), 654}, // 640 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17741, 34), 653}, // 641 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17804, 32), 655}, // 642 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17137, 19), 632}, // 643 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15048, 13), 527}, // 644 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(15673, 14), 564}, // 645 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(15719, 16), 566}, // 646 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(17209, 20), 636}, // 647 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16791, 22), 615}, // 648 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16845, 19), 618}, // 649 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17413, 18), 643}, // 650 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16750, 12), 612}, // 651 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16884, 11), 620}, // 652 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17172, 13), 634}, // 653 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15628, 14), 561}, // 654 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(15125, 14), 532}, // 655 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(15735, 13), 567}, // 656 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(15748, 15), 568}, // 657 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(15875, 20), 574}, // 658 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15843, 32), 573}, // 659 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15232, 12), 544}, // 660 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(16199, 11), 589}, // 661 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(16210, 14), 589}, // 662 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15220, 12), 543}, // 663 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(16762, 14), 613}, // 664 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15362, 7), 554}, // 665 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(16043, 19), 581}, // 666 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16062, 14), 582}, // 667 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15144, 6), 534}, // 668 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(16015, 28), 580}, // 669 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15990, 25), 579}, // 670 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15938, 25), 577}, // 671 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15918, 20), 576}, // 672 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15963, 27), 578}, // 673 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16123, 16), 585}, // 674 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16139, 15), 585}, // 675 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16164, 10), 587}, // 676 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16174, 13), 588}, // 677 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(16187, 12), 588}, // 678 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16154, 10), 586}, // 679 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17229, 20), 637}, // 680 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17081, 14), 629}, // 681 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16398, 26), 598}, // 682 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16736, 14), 611}, // 683 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17585, 31), 649}, // 684 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(14976, 17), 520}, // 685 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(15176, 9), 538}, // 686 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(16224, 16), 590}, // 687 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16240, 19), 590}, // 688 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15018, 9), 524}, // 689 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(16384, 14), 597}, // 690 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15159, 7), 536}, // 691 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(15288, 20), 547}, // 692 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(16095, 28), 584}, // 693 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16437, 17), 600}, // 694 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16864, 20), 619}, // 695 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17353, 25), 641}, // 696 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16813, 16), 616}, // 697 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(14993, 7), 521}, // 698 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(17836, 26), 656}, // 699 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16529, 15), 604}, // 700 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17095, 17), 630}, // 701 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17431, 15), 644}, // 702 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15308, 7), 548}, // 703 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(16971, 16), 623}, // 704 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15895, 23), 575}, // 705 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17001, 18), 625}, // 706 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15244, 8), 545}, // 707 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(15278, 10), 546}, // 708 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(16608, 13), 607}, // 709 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(16640, 15), 608}, // 710 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(17378, 35), 642}, // 711 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16502, 27), 603}, // 712 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16454, 27), 601}, // 713 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16076, 19), 583}, // 714 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15193, 9), 540}, // 715 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(15763, 18), 569}, // 716 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16987, 14), 624}, // 717 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15369, 10), 555}, // 718 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15379, 10), 556}, // 719 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18124, 3), 662}, // 720 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18127, 3), 663}, // 721 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18130, 3), 664}, // 722 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18143, 7), 667}, // 723 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18133, 5), 665}, // 724 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18138, 5), 666}, // 725 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18150, 12), 668}, // 726 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18162, 17), 669}, // 727 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18974, 15), 722}, // 728 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18605, 15), 700}, // 729 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9433, 18), 714}, // 730 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9418, 15), 713}, // 731 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18375, 13), 683}, // 732 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18388, 10), 684}, // 733 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18360, 15), 682}, // 734 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18398, 15), 685}, // 735 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19395, 25), 740}, // 736 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19420, 24), 740}, // 737 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19348, 24), 739}, // 738 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19372, 23), 739}, // 739 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18922, 29), 720}, // 740 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18327, 19), 679}, // 741 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19905, 18), 759}, // 742 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18632, 10), 702}, // 743 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19720, 26), 751}, // 744 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19746, 27), 752}, // 745 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18620, 12), 701}, // 746 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18449, 11), 689}, // 747 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18460, 20), 690}, // 748 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18437, 12), 688}, // 749 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18490, 24), 692}, // 750 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18179, 12), 670}, // 751 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18989, 15), 723}, // 752 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18520, 9), 694}, // 753 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18413, 10), 686}, // 754 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18423, 14), 687}, // 755 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18700, 16), 707}, // 756 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18688, 12), 706}, // 757 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19004, 21), 724}, // 758 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19049, 21), 727}, // 759 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19795, 16), 754}, // 760 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19773, 22), 753}, // 761 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19881, 24), 758}, // 762 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19996, 22), 763}, // 763 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20018, 24), 764}, // 764 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19973, 23), 762}, // 765 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20093, 27), 765}, // 766 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19811, 20), 755}, // 767 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18716, 34), 708}, // 768 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18750, 34), 709}, // 769 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18784, 36), 710}, // 770 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18868, 34), 718}, // 771 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19831, 22), 756}, // 772 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18311, 16), 678}, // 773 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18295, 16), 677}, // 774 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18557, 16), 697}, // 775 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19280, 15), 737}, // 776 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19295, 14), 737}, // 777 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18544, 13), 696}, // 778 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19309, 20), 738}, // 779 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19329, 19), 738}, // 780 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18573, 20), 698}, // 781 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19444, 19), 741}, // 782 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19463, 18), 741}, // 783 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18529, 15), 695}, // 784 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18276, 19), 676}, // 785 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19481, 25), 742}, // 786 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19506, 27), 743}, // 787 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18346, 10), 680}, // 788 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18850, 18), 712}, // 789 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19221, 19), 734}, // 790 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18514, 6), 693}, // 791 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19947, 26), 761}, // 792 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19240, 20), 735}, // 793 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9476, 16), 716}, // 794 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19699, 21), 750}, // 795 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19678, 21), 749}, // 796 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9492, 16), 717}, // 797 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19533, 26), 744}, // 798 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19559, 28), 745}, // 799 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19853, 28), 757}, // 800 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6179, 16), 726}, // 801 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19587, 31), 746}, // 802 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19618, 33), 747}, // 803 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19651, 27), 748}, // 804 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19260, 20), 736}, // 805 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9451, 25), 715}, // 806 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18191, 13), 671}, // 807 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18204, 22), 672}, // 808 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18226, 21), 673}, // 809 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19025, 24), 725}, // 810 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19174, 25), 732}, // 811 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19098, 26), 729}, // 812 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19199, 22), 733}, // 813 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19124, 23), 730}, // 814 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18951, 23), 721}, // 815 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19147, 27), 731}, // 816 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19070, 28), 728}, // 817 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19923, 24), 760}, // 818 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(5286, 13), 703}, // 819 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18820, 30), 711}, // 820 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18642, 22), 704}, // 821 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18664, 24), 705}, // 822 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18902, 20), 719}, // 823 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18480, 10), 691}, // 824 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18593, 12), 699}, // 825 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18261, 15), 675}, // 826 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18247, 14), 674}, // 827 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18356, 4), 681}, // 828 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20258, 10), 777}, // 829 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20268, 9), 777}, // 830 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20319, 12), 780}, // 831 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20331, 11), 780}, // 832 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20277, 14), 778}, // 833 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20291, 13), 778}, // 834 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(14359, 9), 770}, // 835 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20170, 10), 771}, // 836 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4767, 9), 769}, // 837 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20227, 16), 776}, // 838 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20243, 15), 776}, // 839 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4534, 7), 772}, // 840 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20350, 8), 782}, // 841 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20187, 7), 774}, // 842 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20304, 8), 779}, // 843 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20312, 7), 779}, // 844 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20194, 17), 775}, // 845 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20211, 16), 775}, // 846 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20342, 8), 781}, // 847 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20180, 7), 773}, // 848 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20127, 20), 767}, // 849 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20147, 23), 768}, // 850 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20120, 7), 766}, // 851 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20367, 12), 784}, // 852 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20358, 9), 783}, // 853 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20379, 12), 785}, // 854 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(20396, 4), 787}, // 855 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20391, 5), 786}, // 856 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20434, 14), 794}, // 857 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20448, 23), 794}, // 858 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20471, 13), 795}, // 859 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20484, 18), 795}, // 860 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20418, 11), 792}, // 861 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20502, 15), 796}, // 862 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20429, 5), 793}, // 863 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20414, 4), 791}, // 864 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(13955, 5), 788}, // 865 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20407, 7), 790}, // 866 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20400, 7), 789}, // 867 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20517, 4), 797}, // 868 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20529, 4), 800}, // 869 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20525, 4), 799}, // 870 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20521, 4), 798}, // 871 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20565, 18), 803}, // 872 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20583, 18), 804}, // 873 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20533, 16), 801}, // 874 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20549, 16), 802}, // 875 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20624, 6), 809}, // 876 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20608, 11), 807}, // 877 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20601, 7), 806}, // 878 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13955, 5), 805}, // 879 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13176, 11), 810}, // 880 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13187, 13), 810}, // 881 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20619, 5), 808}, // 882 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20640, 6), 813}, // 883 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20651, 8), 815}, // 884 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20659, 10), 816}, // 885 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20677, 12), 818}, // 886 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20669, 8), 817}, // 887 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20689, 20), 819}, // 888 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20635, 5), 812}, // 889 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20646, 5), 814}, // 890 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20630, 5), 811}, // 891 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20744, 16), 823}, // 892 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20730, 14), 822}, // 893 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20716, 14), 821}, // 894 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20807, 27), 826}, // 895 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20780, 27), 825}, // 896 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20760, 20), 824}, // 897 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20709, 7), 820}, // 898 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20834, 10), 827}, // 899 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(20844, 10), 828}, // 900 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(20865, 15), 830}, // 901 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(20854, 11), 829}, // 902 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(20880, 5), 832}, // 903 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(20894, 12), 835}, // 904 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(20906, 13), 837}, // 905 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(20889, 5), 834}, // 906 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(20885, 4), 833}, // 907 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(20919, 19), 840}, // 908 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(20938, 22), 840}, // 909 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(20960, 17), 841}, // 910 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(20977, 20), 841}, // 911 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(8318, 7), 839}, // 912 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(20997, 16), 842}, // 913 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21013, 19), 842}, // 914 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(13955, 5), 831}, // 915 None in SPV_OPERAND_TYPE_IMAGE
  {IR(21085, 12), 846}, // 916 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(15362, 7), 836}, // 917 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(21097, 8), 847}, // 918 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(15159, 7), 838}, // 919 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(21063, 11), 844}, // 920 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(21032, 14), 843}, // 921 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21046, 17), 843}, // 922 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21074, 11), 845}, // 923 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(13852, 6), 862}, // 924 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21266, 10), 861}, // 925 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21201, 12), 856}, // 926 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21213, 12), 857}, // 927 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21190, 11), 855}, // 928 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21115, 11), 849}, // 929 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21105, 10), 848}, // 930 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21175, 15), 854}, // 931 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21287, 17), 864}, // 932 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21304, 16), 865}, // 933 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21437, 16), 872}, // 934 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21453, 16), 873}, // 935 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21136, 11), 851}, // 936 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21276, 11), 863}, // 937 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21360, 20), 868}, // 938 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21126, 10), 850}, // 939 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21161, 14), 853}, // 940 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21147, 14), 852}, // 941 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21380, 19), 869}, // 942 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21399, 19), 870}, // 943 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21418, 19), 871}, // 944 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21238, 14), 859}, // 945 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21252, 14), 860}, // 946 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21225, 13), 858}, // 947 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21320, 20), 866}, // 948 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21340, 20), 867}, // 949 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21471, 2), 875}, // 950 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21572, 5), 893}, // 951 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21493, 5), 881}, // 952 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21488, 5), 880}, // 953 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21530, 6), 887}, // 954 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21536, 13), 888}, // 955 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21498, 10), 882}, // 956 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21508, 10), 883}, // 957 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21469, 2), 874}, // 958 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21476, 3), 877}, // 959 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21473, 3), 876}, // 960 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21479, 4), 878}, // 961 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21483, 5), 879}, // 962 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21525, 5), 886}, // 963 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21521, 4), 885}, // 964 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21518, 3), 884}, // 965 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21566, 6), 892}, // 966 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21549, 5), 889}, // 967 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21560, 6), 891}, // 968 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21554, 6), 890}, // 969 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21577, 27), 894}, // 970 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21604, 23), 895}, // 971 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21627, 7), 896}, // 972 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21634, 11), 897}, // 973 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21645, 14), 898}, // 974 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21659, 12), 900}, // 975 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(13955, 5), 899}, // 976 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(21671, 13), 902}, // 977 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(13955, 5), 901}, // 978 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(21684, 7), 903}, // 979 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21691, 7), 904}, // 980 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21698, 12), 905}, // 981 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21724, 12), 907}, // 982 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21776, 17), 910}, // 983 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21751, 25), 909}, // 984 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21736, 15), 908}, // 985 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21710, 14), 906}, // 986 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21916, 21), 923}, // 987 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21811, 19), 914}, // 988 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21830, 17), 915}, // 989 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21800, 11), 913}, // 990 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17185, 24), 921}, // 991 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21875, 18), 918}, // 992 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21937, 18), 925}, // 993 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22016, 15), 929}, // 994 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17209, 20), 922}, // 995 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21955, 21), 926}, // 996 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21861, 14), 917}, // 997 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22031, 26), 930}, // 998 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21847, 14), 916}, // 999 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22002, 14), 928}, // 1000 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(13955, 5), 911}, // 1001 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21903, 13), 920}, // 1002 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21893, 10), 919}, // 1003 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17229, 20), 924}, // 1004 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21976, 26), 927}, // 1005 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21793, 7), 912}, // 1006 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22340, 27), 944}, // 1007 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22288, 26), 942}, // 1008 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22208, 23), 938}, // 1009 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22162, 23), 936}, // 1010 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22057, 29), 932}, // 1011 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22254, 17), 940}, // 1012 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22367, 27), 945}, // 1013 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22314, 26), 943}, // 1014 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22231, 23), 939}, // 1015 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22185, 23), 937}, // 1016 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22086, 29), 933}, // 1017 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22271, 17), 941}, // 1018 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22115, 21), 934}, // 1019 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22136, 26), 935}, // 1020 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(13955, 5), 931}, // 1021 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22527, 20), 953}, // 1022 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22394, 8), 948}, // 1023 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22402, 21), 950}, // 1024 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22423, 24), 950}, // 1025 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22447, 19), 951}, // 1026 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22466, 22), 951}, // 1027 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22547, 17), 954}, // 1028 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22488, 18), 952}, // 1029 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22506, 21), 952}, // 1030 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(13955, 5), 946}, // 1031 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(21085, 12), 949}, // 1032 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15193, 9), 947}, // 1033 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22571, 8), 956}, // 1034 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22579, 7), 957}, // 1035 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22564, 7), 955}, // 1036 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22586, 7), 958}, // 1037 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22593, 10), 958}, // 1038 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22611, 8), 960}, // 1039 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22627, 15), 962}, // 1040 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22731, 20), 968}, // 1041 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22710, 21), 967}, // 1042 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22751, 12), 969}, // 1043 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22792, 14), 971}, // 1044 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22806, 17), 971}, // 1045 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22823, 12), 972}, // 1046 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22835, 15), 972}, // 1047 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(13955, 5), 959}, // 1048 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22763, 13), 970}, // 1049 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22776, 16), 970}, // 1050 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22603, 8), 959}, // 1051 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22619, 8), 961}, // 1052 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22642, 23), 963}, // 1053 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22679, 15), 965}, // 1054 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22665, 14), 964}, // 1055 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15193, 9), 973}, // 1056 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22694, 16), 966}, // 1057 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22850, 10), 974}, // 1058 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(22865, 4), 976}, // 1059 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(22878, 8), 978}, // 1060 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(22869, 9), 977}, // 1061 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(22860, 5), 975}, // 1062 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(22886, 25), 979}, // 1063 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(22911, 28), 979}, // 1064 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(22952, 4), 982}, // 1065 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22985, 9), 986}, // 1066 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22994, 13), 987}, // 1067 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22965, 8), 984}, // 1068 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22973, 12), 985}, // 1069 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22956, 9), 983}, // 1070 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22939, 4), 980}, // 1071 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(22943, 9), 981}, // 1072 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(13955, 5), 988}, // 1073 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23007, 25), 989}, // 1074 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23032, 23), 990}, // 1075 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23124, 27), 996}, // 1076 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23151, 28), 997}, // 1077 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23193, 16), 999}, // 1078 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23179, 14), 998}, // 1079 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23263, 30), 1002}, // 1080 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23065, 12), 993}, // 1081 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(14707, 8), 991}, // 1082 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23055, 10), 992}, // 1083 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23250, 13), 1001}, // 1084 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23226, 24), 1000}, // 1085 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23100, 24), 995}, // 1086 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23209, 17), 1000}, // 1087 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23077, 23), 994}, // 1088 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23334, 37), 1004}, // 1089 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23293, 41), 1003}, // 1090 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23449, 42), 1007}, // 1091 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23371, 37), 1005}, // 1092 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23408, 41), 1006}, // 1093 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23491, 33), 1008}, // 1094 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23524, 33), 1009}, // 1095 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23569, 6), 1012}, // 1096 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23557, 12), 1011}, // 1097 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(13955, 5), 1010}, // 1098 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23575, 7), 1013}, // 1099 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23582, 15), 1014}, // 1100 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23605, 7), 1016}, // 1101 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23597, 8), 1015}, // 1102 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23670, 13), 1025}, // 1103 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23712, 4), 1031}, // 1104 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23750, 9), 1036}, // 1105 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23683, 5), 1026}, // 1106 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23812, 5), 1045}, // 1107 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23883, 6), 1055}, // 1108 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23636, 5), 1020}, // 1109 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23790, 5), 1041}, // 1110 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23847, 6), 1050}, // 1111 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23900, 5), 1058}, // 1112 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23894, 6), 1057}, // 1113 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23716, 3), 1032}, // 1114 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23759, 8), 1037}, // 1115 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23817, 4), 1046}, // 1116 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23889, 5), 1056}, // 1117 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23703, 5), 1029}, // 1118 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23731, 10), 1034}, // 1119 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23664, 6), 1024}, // 1120 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23801, 6), 1043}, // 1121 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23870, 7), 1053}, // 1122 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23658, 6), 1023}, // 1123 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23795, 6), 1042}, // 1124 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23863, 7), 1052}, // 1125 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23708, 4), 1030}, // 1126 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23741, 9), 1035}, // 1127 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23807, 5), 1044}, // 1128 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23877, 6), 1054}, // 1129 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23695, 8), 1028}, // 1130 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23853, 10), 1051}, // 1131 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23688, 7), 1027}, // 1132 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23719, 12), 1033}, // 1133 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23628, 8), 1019}, // 1134 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23775, 8), 1039}, // 1135 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23830, 9), 1048}, // 1136 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23620, 8), 1018}, // 1137 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23767, 8), 1038}, // 1138 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23821, 9), 1047}, // 1139 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23641, 6), 1021}, // 1140 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23647, 11), 1022}, // 1141 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23783, 7), 1040}, // 1142 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23839, 8), 1049}, // 1143 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23612, 8), 1017}, // 1144 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23905, 12), 1059}, // 1145 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23917, 7), 1060}, // 1146 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23943, 11), 1063}, // 1147 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23954, 12), 1064}, // 1148 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23966, 15), 1064}, // 1149 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23981, 14), 1065}, // 1150 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23934, 9), 1062}, // 1151 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(23924, 10), 1061}, // 1152 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24003, 12), 1068}, // 1153 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(23995, 8), 1067}, // 1154 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(13955, 5), 1066}, // 1155 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24015, 29), 1069}, // 1156 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(13836, 8), 1071}, // 1157 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13844, 8), 1072}, // 1158 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13852, 6), 1073}, // 1159 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13858, 7), 1074}, // 1160 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13865, 11), 1075}, // 1161 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13876, 9), 1076}, // 1162 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13885, 13), 1077}, // 1163 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13824, 12), 1070}, // 1164 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13898, 6), 1078}, // 1165 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13904, 10), 1079}, // 1166 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13914, 6), 1080}, // 1167 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13935, 20), 1082}, // 1168 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(13920, 15), 1081}, // 1169 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14044, 15), 1090}, // 1170 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14059, 13), 1091}, // 1171 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14032, 12), 1089}, // 1172 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14122, 21), 1095}, // 1173 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14015, 17), 1088}, // 1174 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14199, 16), 1099}, // 1175 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14003, 12), 1087}, // 1176 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14183, 16), 1098}, // 1177 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(13976, 14), 1085}, // 1178 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(13960, 16), 1084}, // 1179 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(13990, 13), 1086}, // 1180 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14143, 20), 1096}, // 1181 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14087, 18), 1093}, // 1182 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14072, 15), 1092}, // 1183 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14163, 20), 1097}, // 1184 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14105, 17), 1094}, // 1185 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14235, 24), 1101}, // 1186 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14215, 20), 1100}, // 1187 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(24044, 26), 1102}, // 1188 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(13955, 5), 1083}, // 1189 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14320, 9), 1107}, // 1190 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14352, 7), 1111}, // 1191 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14259, 6), 1103}, // 1192 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14359, 9), 1112}, // 1193 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14270, 6), 1105}, // 1194 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14265, 5), 1104}, // 1195 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14309, 11), 1106}, // 1196 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14341, 11), 1110}, // 1197 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14329, 5), 1108}, // 1198 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14334, 7), 1109}, // 1199 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14404, 11), 1116}, // 1200 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14368, 10), 1113}, // 1201 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14391, 13), 1115}, // 1202 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14378, 13), 1114}, // 1203 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(24105, 15), 1123}, // 1204 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24070, 5), 1118}, // 1205 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24075, 5), 1119}, // 1206 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24125, 7), 1125}, // 1207 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24100, 5), 1122}, // 1208 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24132, 5), 1126}, // 1209 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24080, 9), 1120}, // 1210 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24089, 11), 1121}, // 1211 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24152, 5), 1130}, // 1212 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24120, 5), 1124}, // 1213 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24142, 6), 1128}, // 1214 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(23612, 8), 1117}, // 1215 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24137, 5), 1127}, // 1216 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24148, 4), 1129}, // 1217 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24239, 14), 1141}, // 1218 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24321, 16), 1147}, // 1219 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24337, 15), 1147}, // 1220 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24635, 17), 1156}, // 1221 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24186, 15), 1136}, // 1222 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24652, 16), 1157}, // 1223 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24209, 9), 1138}, // 1224 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24218, 8), 1139}, // 1225 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24426, 16), 1150}, // 1226 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24442, 15), 1150}, // 1227 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24590, 21), 1154}, // 1228 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24668, 14), 1158}, // 1229 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24253, 6), 1142}, // 1230 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24352, 24), 1148}, // 1231 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24376, 23), 1148}, // 1232 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24457, 22), 1151}, // 1233 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24479, 21), 1151}, // 1234 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24173, 6), 1132}, // 1235 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24305, 16), 1146}, // 1236 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24179, 7), 1134}, // 1237 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24543, 22), 1153}, // 1238 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24565, 25), 1153}, // 1239 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24201, 8), 1137}, // 1240 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24226, 13), 1140}, // 1241 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24399, 14), 1149}, // 1242 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24413, 13), 1149}, // 1243 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24500, 22), 1152}, // 1244 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24522, 21), 1152}, // 1245 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24259, 14), 1143}, // 1246 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24611, 24), 1155}, // 1247 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24286, 19), 1145}, // 1248 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24273, 13), 1144}, // 1249 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(15244, 8), 1133}, // 1250 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24157, 16), 1131}, // 1251 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(23924, 10), 1135}, // 1252 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(21736, 15), 1162}, // 1253 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(21710, 14), 1159}, // 1254 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24700, 15), 1161}, // 1255 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24682, 18), 1160}, // 1256 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24726, 11), 1165}, // 1257 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(13955, 5), 1163}, // 1258 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(24715, 11), 1164}, // 1259 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(23557, 12), 1168}, // 1260 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(15202, 9), 1167}, // 1261 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23575, 7), 1169}, // 1262 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23582, 15), 1170}, // 1263 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(24737, 10), 1166}, // 1264 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(24790, 24), 1174}, // 1265 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24814, 22), 1175}, // 1266 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24836, 21), 1176}, // 1267 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24747, 8), 1171}, // 1268 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24755, 15), 1172}, // 1269 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24770, 20), 1173}, // 1270 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 148);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(156, 295);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(451, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(459, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(462, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(464, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(483, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(493, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(497, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(512, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(516, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(522, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(525, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(528, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(532, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(540, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(543, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(559, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(568, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(571, 149);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(720, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(728, 101);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(829, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(852, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(854, 1);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(855, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(857, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(868, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(872, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(876, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(883, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(892, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(899, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(903, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(924, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(950, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(970, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(972, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(975, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(977, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(979, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(982, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(987, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1007, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1022, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1034, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1039, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1058, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1059, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1063, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1065, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1073, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1076, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1089, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1091, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1094, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1096, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1101, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1103, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1145, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1153, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1156, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1157, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1165, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1168, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1170, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1190, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1200, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1204, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1218, 35);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1253, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1257, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1260, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1265, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(903, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1022, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1063, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(516, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1007, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1073, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(854, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1265, 6);
    default: break;
  }
  return IR(0,0);
}

// Instruction descriptions, ordered by opcode.
// The fields in order are:
//   opcode
//   a boolean indicating if the instruction produces a result ID
//   a boolean indicating if the instruction result ID has a type
//   operands, an IndexRange into kOperandSpans
//   opcode name (without the 'Op' prefix), a character-counting IndexRange into kStrings
//   aliases, an IndexRange into kAliasSpans
//   capabilities, an IndexRange into kCapabilitySpans
//   extensions, as an IndexRange into kExtensionSpans
//   version, first version of SPIR-V that has it
//   lastVersion, last version of SPIR-V that has it
static const std::array<InstructionDesc, 801> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(24857, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(24913, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(24919, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(25038, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(25045, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(25061, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(25066, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(25077, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(25084, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(25089, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(25099, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(25159, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(25231, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(25305, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(25348, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(25390, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(25401, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(25410, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(25419, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(25464, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(25474, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(25485, 11), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(25609, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(25619, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(25631, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(25648, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(25658, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(25675, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(25686, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(25728, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(25740, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(25753, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(25763, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(25779, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(25793, 10), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(25803, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(25812, 19), IR(0, 0), IR(198, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(25831, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(25844, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(15202, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(25896, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(25992, 16), IR(0, 0), IR(200, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(26008, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(26021, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(26038, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(26056, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(26069, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(26132, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(24209, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(26181, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(26199, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(26211, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(26224, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(26233, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(26291, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(26296, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(26302, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(26313, 16), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(26329, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(26341, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(26361, 15), IR(0, 0), IR(203, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(26376, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(26388, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(26411, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(26462, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(26471, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(26486, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(26502, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(26561, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(26581, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(26602, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(26622, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(26636, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(26655, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(26672, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(26688, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(26699, 10), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(26709, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(26754, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(26800, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(26823, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(26850, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(26877, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(26904, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(26931, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(26962, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(26993, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(27004, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(27016, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(27032, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(27042, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(24253, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(27053, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(27070, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(27086, 18), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(27104, 15), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(27119, 14), IR(0, 0), IR(209, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(27133, 17), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(27150, 18), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(27168, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(27180, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(27192, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(27204, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(27216, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(27225, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(27234, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(27243, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(27257, 14), IR(0, 0), IR(198, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(27271, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(27286, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(27301, 14), IR(0, 0), IR(198, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(27315, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(27332, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(27349, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(27374, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(27382, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(27390, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(27398, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(27403, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(27408, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(27413, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(27418, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(27423, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(27428, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(27433, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(27438, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(27443, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(27448, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(27453, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(27458, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(27463, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(27468, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(27486, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(27504, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(27522, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(27540, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(27558, 13), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(27571, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(27575, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(27585, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(27596, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(27609, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(27622, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(27626, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(27630, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(27636, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(27642, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(27651, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(27660, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(27671, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(27685, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(27693, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(27703, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(27716, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(27732, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(27742, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(27753, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(27764, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(27771, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(27778, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(27788, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(27801, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(27814, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(27832, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(27850, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(27860, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(27870, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(27885, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(27900, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(27910, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(27922, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(27935, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(27950, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(27963, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(27978, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(27994, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(28012, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(28030, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(28050, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(28071, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(28094, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(28112, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(28133, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(28150, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(28160, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(28171, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(28182, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(28186, 15), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(28201, 17), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(28218, 17), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(28235, 11), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(28246, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(28255, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(28260, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(28265, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(28272, 9), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(28281, 9), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(28290, 11), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(28301, 11), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(28312, 11), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(28323, 13), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(28336, 11), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(28347, 13), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(28360, 17), IR(0, 0), IR(110, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(28377, 19), IR(0, 0), IR(110, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(28433, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(28448, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(28462, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(28473, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(28485, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(28500, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(28522, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(28548, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(28565, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(28582, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(28593, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(28604, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(28615, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(28626, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(28637, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(28648, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(28658, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(28667, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(28677, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(28711, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(28756, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(28771, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(28777, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(28784, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(28847, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(28854, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(28859, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(28866, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(28878, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(28890, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(28904, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(28917, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(28932, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(28948, 9), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(28957, 9), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(28966, 15), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(29014, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(29024, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(29034, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(29044, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(29054, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(29064, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(29074, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(29084, 10), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(29094, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(29103, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(29113, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(29130, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(29148, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(29171, 24), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(29195, 15), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(29210, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(29226, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(29243, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(29261, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(29279, 28), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(29307, 29), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(29336, 20), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(29356, 21), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(29377, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(29391, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(29405, 30), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(29435, 32), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(29467, 23), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(29490, 40), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(29530, 12), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(29542, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(29555, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(29571, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(29584, 19), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(29603, 26), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(29629, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(29645, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(29658, 29), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(29687, 29), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(29716, 33), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(29749, 33), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(29782, 33), IR(0, 0), IR(214, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(29815, 33), IR(0, 0), IR(214, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(29848, 37), IR(0, 0), IR(214, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(29885, 37), IR(0, 0), IR(214, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(29922, 17), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(29939, 18), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(29957, 22), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(29979, 26), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(30005, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(30012, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(30033, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(30049, 16), IR(0, 0), IR(214, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(30065, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(30072, 16), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(30088, 20), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(30108, 26), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(30134, 35), IR(0, 0), IR(148, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(30169, 25), IR(0, 0), IR(148, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(30194, 17), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(30211, 23), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(30234, 19), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(30253, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(30269, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(30285, 11), IR(0, 0), IR(1, 0), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(30296, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(30317, 19), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(30336, 19), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(30355, 24), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(30379, 25), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(30404, 30), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(5554, 22), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(30434, 29), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(30463, 32), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(30495, 30), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(30525, 29), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(30554, 29), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(8644, 23), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(30583, 26), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(30609, 25), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(30634, 27), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(30661, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(30681, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(30701, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(30721, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(30741, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(30761, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(30781, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(30801, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(30821, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(30841, 20), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(30861, 26), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(30887, 25), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(30912, 26), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(30938, 26), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(30964, 25), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(30989, 26), IR(0, 0), IR(221, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(31015, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(31044, 24), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(31068, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(31080, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(31089, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(31101, 8), IR(0, 0), IR(225, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(31109, 23), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(31132, 23), IR(0, 0), IR(149, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(31155, 25), IR(0, 0), IR(150, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(31180, 14), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(31236, 14), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(31250, 15), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(31265, 19), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(31284, 20), IR(0, 0), IR(3, 1), IR(163, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(369, 2), IR(31304, 22), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(371, 5), IR(31326, 19), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(31345, 22), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(31367, 30), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(5315, 18), IR(0, 0), IR(229, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(31397, 27), IR(0, 0), IR(229, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(376, 6), IR(31424, 25), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(376, 6), IR(31449, 33), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(382, 5), IR(31482, 22), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(387, 5), IR(31504, 19), IR(0, 0), IR(228, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(31523, 15), IR(0, 0), IR(230, 1), IR(39, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(31538, 15), IR(0, 0), IR(230, 1), IR(39, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(31553, 20), IR(0, 0), IR(230, 1), IR(39, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(392, 6), IR(13076, 25), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(31573, 26), IR(0, 0), IR(229, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(398, 5), IR(31599, 26), IR(0, 0), IR(1, 0), IR(164, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpTraceRayKHR, false, false, IR(403, 11), IR(31625, 12), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(31637, 19), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(31656, 35), IR(0, 0), IR(232, 2), IR(165, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(31691, 22), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(31713, 16), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(414, 5), IR(31776, 5), IR(93, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(414, 5), IR(31789, 5), IR(94, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(414, 5), IR(31802, 6), IR(95, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(419, 6), IR(31817, 11), IR(96, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(419, 6), IR(31842, 11), IR(97, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(419, 6), IR(31867, 12), IR(98, 1), IR(234, 1), IR(119, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(425, 6), IR(31894, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(431, 6), IR(31919, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(437, 5), IR(31944, 26), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(442, 6), IR(32024, 27), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(32051, 27), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32078, 30), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32108, 34), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(32142, 31), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(32173, 16), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(448, 8), IR(32189, 22), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(32211, 21), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(32232, 32), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(32264, 31), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(32295, 19), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(32314, 31), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(32345, 24), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(32369, 19), IR(0, 0), IR(238, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(32388, 23), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(32411, 23), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(32434, 29), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(32463, 29), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(32492, 29), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(32521, 29), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(32550, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(32573, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(32596, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(32619, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(32642, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(32665, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(32688, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(32711, 23), IR(0, 0), IR(213, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(32734, 21), IR(0, 0), IR(241, 1), IR(55, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(32755, 17), IR(0, 0), IR(241, 1), IR(55, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(32772, 13), IR(0, 0), IR(242, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(32785, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(32810, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(32834, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(32859, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(32888, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(32915, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(32938, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(32957, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(32980, 26), IR(0, 0), IR(157, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(33006, 26), IR(0, 0), IR(157, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(456, 14), IR(33032, 27), IR(0, 0), IR(243, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(470, 13), IR(33059, 36), IR(0, 0), IR(243, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(483, 7), IR(33095, 28), IR(0, 0), IR(243, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(33123, 28), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(33151, 28), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(33179, 33), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(33212, 30), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(470, 13), IR(33242, 26), IR(0, 0), IR(243, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(33268, 39), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(33307, 44), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(33351, 23), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(490, 12), IR(33374, 20), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(470, 13), IR(33394, 21), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(490, 12), IR(33415, 30), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(502, 6), IR(33445, 22), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(33467, 25), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(33492, 26), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(33518, 25), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(33543, 22), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(33565, 29), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(33594, 28), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(33622, 25), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(33647, 34), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(33681, 32), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(33713, 29), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(33742, 22), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(33764, 22), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(33786, 19), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(33805, 17), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(33822, 18), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(508, 3), IR(33840, 29), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(33869, 24), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(33893, 16), IR(0, 0), IR(119, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(511, 7), IR(33909, 23), IR(0, 0), IR(245, 1), IR(69, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeCooperativeVectorNV, true, false, IR(89, 3), IR(33932, 24), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(518, 13), IR(33956, 29), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(531, 7), IR(33985, 42), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(34027, 39), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(538, 16), IR(34066, 32), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(34098, 27), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(554, 4), IR(34125, 17), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(34142, 18), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionNV, true, true, IR(160, 3), IR(34160, 27), IR(0, 0), IR(178, 1), IR(72, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(34187, 33), IR(0, 0), IR(50, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(34220, 35), IR(0, 0), IR(249, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(34255, 38), IR(0, 0), IR(249, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(558, 5), IR(34293, 24), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(563, 4), IR(34317, 25), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(34342, 22), IR(99, 1), IR(54, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(34385, 21), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(34406, 15), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(403, 11), IR(34421, 8), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(490, 12), IR(34429, 14), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(490, 12), IR(34443, 17), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(34460, 50), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(34510, 29), IR(100, 1), IR(251, 4), IR(167, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(34567, 18), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetClusterIdNV, true, true, IR(172, 4), IR(34585, 23), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(34608, 24), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(567, 5), IR(34632, 24), IR(0, 0), IR(255, 1), IR(76, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(572, 6), IR(34656, 24), IR(0, 0), IR(255, 1), IR(76, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(578, 5), IR(34680, 25), IR(0, 0), IR(255, 1), IR(76, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(34705, 26), IR(0, 0), IR(255, 1), IR(76, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(34731, 26), IR(0, 0), IR(255, 1), IR(76, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(34757, 28), IR(0, 0), IR(256, 3), IR(77, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(34785, 26), IR(0, 0), IR(256, 3), IR(77, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(583, 5), IR(34854, 26), IR(0, 0), IR(259, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(588, 7), IR(34955, 30), IR(0, 0), IR(195, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(595, 5), IR(34985, 31), IR(0, 0), IR(195, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(35016, 32), IR(0, 0), IR(260, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(35048, 19), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(600, 4), IR(35067, 17), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(35084, 21), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(35105, 27), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(35132, 24), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(35156, 20), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(35176, 28), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(35204, 19), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(35223, 25), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(35248, 22), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(11388, 25), IR(45, 1), IR(262, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(35270, 22), IR(0, 0), IR(262, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(35292, 20), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(35312, 27), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(35339, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(35368, 18), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(35386, 20), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(35406, 18), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(35424, 20), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(35444, 25), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(35469, 25), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(35494, 29), IR(0, 0), IR(120, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(604, 7), IR(35575, 17), IR(0, 0), IR(184, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(35592, 40), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(35632, 38), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(35670, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(35708, 34), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(35742, 37), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(35779, 29), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(35808, 27), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(35835, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(35862, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(35885, 23), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(35908, 20), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(35928, 22), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(35950, 19), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(11836, 21), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(35969, 25), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(35994, 23), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(36017, 24), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(36041, 23), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(36064, 24), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(36088, 28), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(36116, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(36145, 33), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(611, 5), IR(36178, 34), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(36212, 24), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(36236, 25), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(36261, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(36274, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(36287, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(36300, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(36313, 14), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(36327, 14), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(36341, 21), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(36362, 21), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(36383, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(36396, 13), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(36409, 15), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(36424, 15), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(36439, 29), IR(0, 0), IR(193, 1), IR(92, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(36468, 25), IR(0, 0), IR(193, 1), IR(92, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(36493, 15), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(616, 6), IR(12054, 9), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(36508, 13), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(36521, 14), IR(0, 0), IR(268, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(36535, 14), IR(0, 0), IR(268, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(36549, 14), IR(0, 0), IR(272, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(36563, 10), IR(0, 0), IR(272, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(36573, 15), IR(101, 1), IR(1, 0), IR(171, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(36609, 21), IR(102, 1), IR(1, 0), IR(171, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(36657, 14), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(36671, 18), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(36689, 23), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(36712, 23), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(36735, 23), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(36758, 23), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(36781, 22), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(36803, 22), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(36825, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(36871, 44), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(36915, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(36954, 37), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(36991, 22), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(37013, 22), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37035, 60), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37095, 53), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37148, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37195, 40), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37235, 51), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37286, 44), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(37330, 51), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(37381, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37437, 50), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37487, 52), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37539, 49), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(37588, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(37635, 50), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(37685, 51), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(37736, 56), IR(0, 0), IR(276, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(37792, 33), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(37825, 52), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(37877, 61), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(37938, 61), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(37999, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(38038, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(38076, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(38115, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(38153, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(38192, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(38230, 36), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(38266, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(38305, 43), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(38348, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(38386, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(38424, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(38462, 45), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(38507, 40), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(38547, 62), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(38609, 30), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(38639, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(38677, 36), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(38713, 33), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(38746, 35), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(38781, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(38820, 43), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(38863, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(38910, 54), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(38964, 34), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(38998, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(39045, 45), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(39090, 55), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(39145, 53), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(39198, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(39254, 54), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(39308, 58), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(39366, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(39422, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(39460, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(39506, 44), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(39550, 49), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(39599, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(39646, 70), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(39716, 68), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(39784, 69), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(39853, 68), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(39921, 66), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(39987, 67), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(40054, 36), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(40090, 48), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(40138, 59), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(40197, 58), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(40255, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(622, 9), IR(40311, 30), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(631, 10), IR(40341, 30), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(40371, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(40410, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(40456, 43), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(40499, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(40546, 45), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(40591, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(40637, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(40693, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(40731, 30), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(40761, 32), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(631, 10), IR(40793, 36), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(641, 13), IR(40829, 42), IR(0, 0), IR(276, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(40871, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(40910, 39), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(40949, 44), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(40993, 48), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(41041, 50), IR(0, 0), IR(276, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(41091, 43), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(41134, 48), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(41182, 43), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(41225, 31), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41256, 47), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41303, 45), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(41348, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(41394, 56), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(41450, 38), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(41488, 35), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(41523, 44), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(41567, 46), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(41613, 41), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(41654, 36), IR(0, 0), IR(276, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(41690, 50), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(41740, 48), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(41788, 35), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(12292, 25), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(41823, 16), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(41839, 19), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosPiINTEL, true, true, IR(654, 8), IR(41858, 28), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastINTEL, true, true, IR(654, 8), IR(41886, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntINTEL, true, true, IR(654, 8), IR(41910, 31), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntINTEL, true, true, IR(654, 8), IR(41941, 29), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddINTEL, true, true, IR(662, 10), IR(41970, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubINTEL, true, true, IR(662, 10), IR(41993, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulINTEL, true, true, IR(662, 10), IR(42016, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivINTEL, true, true, IR(662, 10), IR(42039, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTINTEL, true, true, IR(672, 6), IR(42062, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEINTEL, true, true, IR(672, 6), IR(42084, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTINTEL, true, true, IR(672, 6), IR(42106, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEINTEL, true, true, IR(672, 6), IR(42128, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQINTEL, true, true, IR(672, 6), IR(42150, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipINTEL, true, true, IR(654, 8), IR(42172, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtINTEL, true, true, IR(654, 8), IR(42197, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtINTEL, true, true, IR(654, 8), IR(42222, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotINTEL, true, true, IR(662, 10), IR(42246, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtINTEL, true, true, IR(654, 8), IR(42271, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(654, 8), IR(42295, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(654, 8), IR(42318, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(654, 8), IR(42342, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(654, 8), IR(42367, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(654, 8), IR(42392, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(654, 8), IR(42415, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(654, 8), IR(42439, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(654, 8), IR(42464, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(654, 8), IR(42489, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(654, 8), IR(42512, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(654, 8), IR(42535, 26), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(654, 8), IR(42561, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(654, 8), IR(42586, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(654, 8), IR(42611, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(654, 8), IR(42635, 26), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(654, 8), IR(42661, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(654, 8), IR(42685, 26), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(654, 8), IR(42711, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(654, 8), IR(42735, 26), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(662, 10), IR(42761, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(662, 10), IR(42786, 23), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(662, 10), IR(42809, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(662, 10), IR(42833, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(42857, 17), IR(0, 0), IR(280, 1), IR(103, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(678, 2), IR(42874, 21), IR(0, 0), IR(129, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(680, 3), IR(42895, 20), IR(0, 0), IR(129, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(42915, 24), IR(0, 0), IR(129, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtINTEL, true, true, IR(654, 8), IR(42939, 15), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipINTEL, true, true, IR(654, 8), IR(42954, 16), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtINTEL, true, true, IR(654, 8), IR(42970, 16), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinINTEL, true, true, IR(654, 8), IR(42986, 14), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosINTEL, true, true, IR(654, 8), IR(43000, 14), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosINTEL, true, true, IR(654, 8), IR(43014, 17), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiINTEL, true, true, IR(654, 8), IR(43031, 16), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiINTEL, true, true, IR(654, 8), IR(43047, 16), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiINTEL, true, true, IR(654, 8), IR(43063, 19), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogINTEL, true, true, IR(654, 8), IR(43082, 14), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpINTEL, true, true, IR(654, 8), IR(43096, 14), IR(0, 0), IR(183, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupINTEL, true, true, IR(160, 3), IR(43110, 29), IR(0, 0), IR(194, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrINTEL, true, true, IR(160, 3), IR(43139, 29), IR(0, 0), IR(194, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingINTEL, true, true, IR(172, 4), IR(43168, 22), IR(0, 0), IR(281, 1), IR(117, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipeBlockingINTEL, true, true, IR(172, 4), IR(43190, 23), IR(0, 0), IR(281, 1), IR(117, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpFPGARegINTEL, true, true, IR(160, 3), IR(12851, 13), IR(0, 0), IR(282, 1), IR(118, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(43213, 22), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(43235, 23), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(43258, 28), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(43286, 46), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(43332, 37), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(43369, 65), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(43434, 40), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(43474, 41), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(43515, 39), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(43554, 36), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(43590, 46), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(43636, 45), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(43681, 42), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(43723, 32), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(43755, 29), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(43784, 40), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(43824, 40), IR(0, 0), IR(187, 1), IR(46, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(43864, 14), IR(0, 0), IR(283, 4), IR(125, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(43878, 23), IR(0, 0), IR(121, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(683, 1), IR(43901, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(683, 1), IR(43926, 32), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(683, 1), IR(43958, 36), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(43994, 33), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(44027, 20), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(44047, 20), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(44067, 26), IR(0, 0), IR(289, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(44093, 24), IR(0, 0), IR(289, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(13283, 19), IR(0, 0), IR(290, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateINTEL, true, true, IR(684, 7), IR(44117, 24), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceAsyncINTEL, false, false, IR(168, 2), IR(44141, 23), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceGetINTEL, true, true, IR(160, 3), IR(44164, 21), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceReleaseINTEL, false, false, IR(10, 1), IR(44185, 25), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeTaskSequenceINTEL, true, false, IR(68, 1), IR(44210, 22), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(44232, 27), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(691, 10), IR(44259, 25), IR(0, 0), IR(105, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(691, 10), IR(44284, 34), IR(0, 0), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(691, 10), IR(44318, 34), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(701, 9), IR(44352, 29), IR(0, 0), IR(105, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(691, 10), IR(44381, 26), IR(0, 0), IR(105, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(710, 7), IR(13621, 38), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(44469, 21), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(44490, 13), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(44503, 13), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(44516, 19), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(44535, 18), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(44553, 19), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(44572, 19), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(44591, 18), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(44609, 19), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(44628, 18), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(717, 6), IR(44646, 18), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(723, 4), IR(44664, 19), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(44683, 26), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(44709, 28), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(44737, 33), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 812> kInstructionNames{{
  {IR(36261, 13), 541}, // 0 AbsISubINTEL
  {IR(36274, 13), 542}, // 1 AbsUSubINTEL
  {IR(26329, 12), 58}, // 2 AccessChain
  {IR(42874, 21), 727}, // 3 AliasDomainDeclINTEL
  {IR(42895, 20), 728}, // 4 AliasScopeDeclINTEL
  {IR(42915, 24), 729}, // 5 AliasScopeListDeclINTEL
  {IR(27626, 4), 143}, // 6 All
  {IR(32785, 25), 413}, // 7 AllocateNodePayloadsAMDX
  {IR(27622, 4), 142}, // 8 Any
  {IR(42661, 24), 718}, // 9 ArbitraryFloatACosINTEL
  {IR(42685, 26), 719}, // 10 ArbitraryFloatACosPiINTEL
  {IR(42611, 24), 716}, // 11 ArbitraryFloatASinINTEL
  {IR(42635, 26), 717}, // 12 ArbitraryFloatASinPiINTEL
  {IR(42761, 25), 722}, // 13 ArbitraryFloatATan2INTEL
  {IR(42711, 24), 720}, // 14 ArbitraryFloatATanINTEL
  {IR(42735, 26), 721}, // 15 ArbitraryFloatATanPiINTEL
  {IR(41970, 23), 689}, // 16 ArbitraryFloatAddINTEL
  {IR(41910, 31), 687}, // 17 ArbitraryFloatCastFromIntINTEL
  {IR(41886, 24), 686}, // 18 ArbitraryFloatCastINTEL
  {IR(41941, 29), 688}, // 19 ArbitraryFloatCastToIntINTEL
  {IR(42222, 24), 700}, // 20 ArbitraryFloatCbrtINTEL
  {IR(42512, 23), 712}, // 21 ArbitraryFloatCosINTEL
  {IR(42586, 25), 715}, // 22 ArbitraryFloatCosPiINTEL
  {IR(42039, 23), 692}, // 23 ArbitraryFloatDivINTEL
  {IR(42150, 22), 697}, // 24 ArbitraryFloatEQINTEL
  {IR(42439, 25), 709}, // 25 ArbitraryFloatExp10INTEL
  {IR(42415, 24), 708}, // 26 ArbitraryFloatExp2INTEL
  {IR(42392, 23), 707}, // 27 ArbitraryFloatExpINTEL
  {IR(42464, 25), 710}, // 28 ArbitraryFloatExpm1INTEL
  {IR(42084, 22), 694}, // 29 ArbitraryFloatGEINTEL
  {IR(42062, 22), 693}, // 30 ArbitraryFloatGTINTEL
  {IR(42246, 25), 701}, // 31 ArbitraryFloatHypotINTEL
  {IR(42128, 22), 696}, // 32 ArbitraryFloatLEINTEL
  {IR(42106, 22), 695}, // 33 ArbitraryFloatLTINTEL
  {IR(42342, 25), 705}, // 34 ArbitraryFloatLog10INTEL
  {IR(42367, 25), 706}, // 35 ArbitraryFloatLog1pINTEL
  {IR(42318, 24), 704}, // 36 ArbitraryFloatLog2INTEL
  {IR(42295, 23), 703}, // 37 ArbitraryFloatLogINTEL
  {IR(42016, 23), 691}, // 38 ArbitraryFloatMulINTEL
  {IR(42786, 23), 723}, // 39 ArbitraryFloatPowINTEL
  {IR(42833, 24), 725}, // 40 ArbitraryFloatPowNINTEL
  {IR(42809, 24), 724}, // 41 ArbitraryFloatPowRINTEL
  {IR(42197, 25), 699}, // 42 ArbitraryFloatRSqrtINTEL
  {IR(42172, 25), 698}, // 43 ArbitraryFloatRecipINTEL
  {IR(42535, 26), 713}, // 44 ArbitraryFloatSinCosINTEL
  {IR(41858, 28), 685}, // 45 ArbitraryFloatSinCosPiINTEL
  {IR(42489, 23), 711}, // 46 ArbitraryFloatSinINTEL
  {IR(42561, 25), 714}, // 47 ArbitraryFloatSinPiINTEL
  {IR(42271, 24), 702}, // 48 ArbitraryFloatSqrtINTEL
  {IR(41993, 23), 690}, // 49 ArbitraryFloatSubINTEL
  {IR(13283, 19), 773}, // 50 ArithmeticFenceEXT
  {IR(26376, 12), 61}, // 51 ArrayLength
  {IR(36508, 13), 557}, // 52 AsmCallINTEL
  {IR(12054, 9), 556}, // 53 AsmINTEL
  {IR(36493, 15), 555}, // 54 AsmTargetINTEL
  {IR(36549, 14), 560}, // 55 AssumeTrueKHR
  {IR(28648, 10), 220}, // 56 AtomicAnd
  {IR(28500, 22), 210}, // 57 AtomicCompareExchange
  {IR(28522, 26), 211}, // 58 AtomicCompareExchangeWeak
  {IR(28485, 15), 209}, // 59 AtomicExchange
  {IR(43864, 14), 763}, // 60 AtomicFAddEXT
  {IR(36535, 14), 559}, // 61 AtomicFMaxEXT
  {IR(36521, 14), 558}, // 62 AtomicFMinEXT
  {IR(30033, 16), 292}, // 63 AtomicFlagClear
  {IR(30012, 21), 291}, // 64 AtomicFlagTestAndSet
  {IR(28582, 11), 214}, // 65 AtomicIAdd
  {IR(28565, 17), 213}, // 66 AtomicIDecrement
  {IR(28548, 17), 212}, // 67 AtomicIIncrement
  {IR(28593, 11), 215}, // 68 AtomicISub
  {IR(28462, 11), 207}, // 69 AtomicLoad
  {IR(28658, 9), 221}, // 70 AtomicOr
  {IR(28626, 11), 218}, // 71 AtomicSMax
  {IR(28604, 11), 216}, // 72 AtomicSMin
  {IR(28473, 12), 208}, // 73 AtomicStore
  {IR(28637, 11), 219}, // 74 AtomicUMax
  {IR(28615, 11), 217}, // 75 AtomicUMin
  {IR(28667, 10), 222}, // 76 AtomicXor
  {IR(34757, 28), 487}, // 77 BeginInvocationInterlockEXT
  {IR(28246, 9), 191}, // 78 BitCount
  {IR(28186, 15), 187}, // 79 BitFieldInsert
  {IR(28201, 17), 188}, // 80 BitFieldSExtract
  {IR(28218, 17), 189}, // 81 BitFieldUExtract
  {IR(28235, 11), 190}, // 82 BitReverse
  {IR(27374, 8), 114}, // 83 Bitcast
  {IR(28171, 11), 185}, // 84 BitwiseAnd
  {IR(44469, 21), 786}, // 85 BitwiseFunctionINTEL
  {IR(28150, 10), 183}, // 86 BitwiseOr
  {IR(28160, 11), 184}, // 87 BitwiseXor
  {IR(28777, 7), 227}, // 88 Branch
  {IR(28784, 18), 228}, // 89 BranchConditional
  {IR(29645, 13), 277}, // 90 BuildNDRange
  {IR(25390, 11), 15}, // 91 Capability
  {IR(29603, 26), 275}, // 92 CaptureEventProfilingInfo
  {IR(31109, 23), 344}, // 93 ColorAttachmentReadEXT
  {IR(29195, 15), 255}, // 94 CommitReadPipe
  {IR(29210, 16), 256}, // 95 CommitWritePipe
  {IR(26636, 19), 72}, // 96 CompositeConstruct
  {IR(43994, 33), 768}, // 97 CompositeConstructContinuedINTEL
  {IR(32142, 31), 386}, // 98 CompositeConstructReplicateEXT
  {IR(26655, 17), 73}, // 99 CompositeExtract
  {IR(26672, 16), 74}, // 100 CompositeInsert
  {IR(15202, 9), 39}, // 101 Constant
  {IR(25896, 18), 40}, // 102 ConstantComposite
  {IR(43926, 32), 766}, // 103 ConstantCompositeContinuedINTEL
  {IR(32078, 30), 384}, // 104 ConstantCompositeReplicateEXT
  {IR(25844, 14), 38}, // 105 ConstantFalse
  {IR(36439, 29), 553}, // 106 ConstantFunctionPointerINTEL
  {IR(26008, 13), 42}, // 107 ConstantNull
  {IR(30088, 20), 296}, // 108 ConstantPipeStorage
  {IR(25992, 16), 41}, // 109 ConstantSampler
  {IR(32938, 19), 419}, // 110 ConstantStringAMDX
  {IR(25831, 13), 37}, // 111 ConstantTrue
  {IR(28433, 15), 205}, // 112 ControlBarrier
  {IR(44067, 26), 771}, // 113 ControlBarrierArriveINTEL
  {IR(44093, 24), 772}, // 114 ControlBarrierWaitINTEL
  {IR(44047, 20), 770}, // 115 ConvertBF16ToFINTEL
  {IR(44027, 20), 769}, // 116 ConvertFToBF16INTEL
  {IR(27180, 12), 100}, // 117 ConvertFToS
  {IR(27168, 12), 99}, // 118 ConvertFToU
  {IR(44683, 26), 798}, // 119 ConvertHandleToImageINTEL
  {IR(44737, 33), 800}, // 120 ConvertHandleToSampledImageINTEL
  {IR(44709, 28), 799}, // 121 ConvertHandleToSamplerINTEL
  {IR(35406, 18), 510}, // 122 ConvertImageToUNV
  {IR(27257, 14), 107}, // 123 ConvertPtrToU
  {IR(27192, 12), 101}, // 124 ConvertSToF
  {IR(35469, 25), 513}, // 125 ConvertSampledImageToUNV
  {IR(35424, 20), 511}, // 126 ConvertSamplerToUNV
  {IR(31656, 35), 370}, // 127 ConvertUToAccelerationStructureKHR
  {IR(27204, 12), 102}, // 128 ConvertUToF
  {IR(35368, 18), 508}, // 129 ConvertUToImageNV
  {IR(27301, 14), 110}, // 130 ConvertUToPtr
  {IR(35444, 25), 512}, // 131 ConvertUToSampledImageNV
  {IR(35386, 20), 509}, // 132 ConvertUToSamplerNV
  {IR(34098, 27), 462}, // 133 CooperativeMatrixConvertNV
  {IR(32051, 27), 383}, // 134 CooperativeMatrixLengthKHR
  {IR(34731, 26), 486}, // 135 CooperativeMatrixLengthNV
  {IR(31919, 25), 380}, // 136 CooperativeMatrixLoadKHR
  {IR(34656, 24), 483}, // 137 CooperativeMatrixLoadNV
  {IR(34955, 30), 490}, // 138 CooperativeMatrixLoadTensorNV
  {IR(32024, 27), 382}, // 139 CooperativeMatrixMulAddKHR
  {IR(34705, 26), 485}, // 140 CooperativeMatrixMulAddNV
  {IR(35016, 32), 492}, // 141 CooperativeMatrixPerElementOpNV
  {IR(34854, 26), 489}, // 142 CooperativeMatrixReduceNV
  {IR(31944, 26), 381}, // 143 CooperativeMatrixStoreKHR
  {IR(34680, 25), 484}, // 144 CooperativeMatrixStoreNV
  {IR(34985, 31), 491}, // 145 CooperativeMatrixStoreTensorNV
  {IR(35339, 29), 507}, // 146 CooperativeMatrixTransposeNV
  {IR(34293, 24), 469}, // 147 CooperativeVectorLoadNV
  {IR(34066, 32), 461}, // 148 CooperativeVectorMatrixMulAddNV
  {IR(33956, 29), 458}, // 149 CooperativeVectorMatrixMulNV
  {IR(33985, 42), 459}, // 150 CooperativeVectorOuterProductAccumulateNV
  {IR(34027, 39), 460}, // 151 CooperativeVectorReduceSumAccumulateNV
  {IR(34317, 25), 470}, // 152 CooperativeVectorStoreNV
  {IR(31068, 12), 340}, // 153 CopyLogical
  {IR(26302, 11), 56}, // 154 CopyMemory
  {IR(26313, 16), 57}, // 155 CopyMemorySized
  {IR(26688, 11), 75}, // 156 CopyObject
  {IR(30108, 26), 297}, // 157 CreatePipeFromPipeStorage
  {IR(35084, 21), 495}, // 158 CreateTensorLayoutNV
  {IR(35204, 19), 500}, // 159 CreateTensorViewNV
  {IR(29555, 16), 272}, // 160 CreateUserEvent
  {IR(43139, 29), 742}, // 161 CrossWorkgroupCastToPtrINTEL
  {IR(28255, 5), 192}, // 162 DPdx
  {IR(28301, 11), 198}, // 163 DPdxCoarse
  {IR(28272, 9), 195}, // 164 DPdxFine
  {IR(28260, 5), 193}, // 165 DPdy
  {IR(28312, 11), 199}, // 166 DPdyCoarse
  {IR(28281, 9), 196}, // 167 DPdyFine
  {IR(26462, 9), 64}, // 168 Decorate
  {IR(30285, 11), 305}, // 169 DecorateId
  {IR(36573, 15), 562}, // 170 DecorateString
  {IR(36588, 21), 562}, // 171 DecorateStringGOOGLE
  {IR(26486, 16), 66}, // 172 DecorationGroup
  {IR(11388, 25), 503}, // 173 DemoteToHelperInvocation
  {IR(11413, 28), 503}, // 174 DemoteToHelperInvocationEXT
  {IR(31132, 23), 345}, // 175 DepthAttachmentReadEXT
  {IR(27571, 4), 137}, // 176 Dot
  {IR(34125, 17), 463}, // 177 EmitMeshTasksEXT
  {IR(28360, 17), 203}, // 178 EmitStreamVertex
  {IR(28336, 11), 201}, // 179 EmitVertex
  {IR(34785, 26), 488}, // 180 EndInvocationInterlockEXT
  {IR(28347, 13), 202}, // 181 EndPrimitive
  {IR(28377, 19), 204}, // 182 EndStreamPrimitive
  {IR(29391, 14), 265}, // 183 EnqueueKernel
  {IR(29377, 14), 264}, // 184 EnqueueMarker
  {IR(32810, 24), 414}, // 185 EnqueueNodePayloadsAMDX
  {IR(25305, 11), 13}, // 186 EntryPoint
  {IR(31637, 19), 369}, // 187 ExecuteCallableKHR
  {IR(34567, 18), 479}, // 188 ExecuteCallableNV
  {IR(25348, 14), 14}, // 189 ExecutionMode
  {IR(30269, 16), 304}, // 190 ExecutionModeId
  {IR(36563, 10), 561}, // 191 ExpectKHR
  {IR(25159, 8), 11}, // 192 ExtInst
  {IR(25099, 14), 10}, // 193 ExtInstImport
  {IR(31599, 26), 367}, // 194 ExtInstWithForwardRefsKHR
  {IR(25089, 10), 9}, // 195 Extension
  {IR(27403, 5), 118}, // 196 FAdd
  {IR(27234, 9), 105}, // 197 FConvert
  {IR(27438, 5), 125}, // 198 FDiv
  {IR(27463, 5), 130}, // 199 FMod
  {IR(27423, 5), 122}, // 200 FMul
  {IR(27390, 8), 116}, // 201 FNegate
  {IR(27900, 10), 168}, // 202 FOrdEqual
  {IR(27978, 16), 174}, // 203 FOrdGreaterThan
  {IR(28050, 21), 178}, // 204 FOrdGreaterThanEqual
  {IR(27950, 13), 172}, // 205 FOrdLessThan
  {IR(28012, 18), 176}, // 206 FOrdLessThanEqual
  {IR(27922, 13), 170}, // 207 FOrdNotEqual
  {IR(12851, 13), 745}, // 208 FPGARegINTEL
  {IR(27458, 5), 129}, // 209 FRem
  {IR(27413, 5), 120}, // 210 FSub
  {IR(27910, 12), 169}, // 211 FUnordEqual
  {IR(27994, 18), 175}, // 212 FUnordGreaterThan
  {IR(28071, 23), 179}, // 213 FUnordGreaterThanEqual
  {IR(27963, 15), 173}, // 214 FUnordLessThan
  {IR(28030, 20), 177}, // 215 FUnordLessThanEqual
  {IR(27935, 15), 171}, // 216 FUnordNotEqual
  {IR(34255, 38), 468}, // 217 FetchMicroTriangleVertexBarycentricNV
  {IR(34220, 35), 467}, // 218 FetchMicroTriangleVertexPositionNV
  {IR(32859, 29), 416}, // 219 FinishWritingNodePayloadAMDX
  {IR(43000, 14), 734}, // 220 FixedCosINTEL
  {IR(43047, 16), 737}, // 221 FixedCosPiINTEL
  {IR(43096, 14), 740}, // 222 FixedExpINTEL
  {IR(43082, 14), 739}, // 223 FixedLogINTEL
  {IR(42954, 16), 731}, // 224 FixedRecipINTEL
  {IR(42970, 16), 732}, // 225 FixedRsqrtINTEL
  {IR(43014, 17), 735}, // 226 FixedSinCosINTEL
  {IR(43063, 19), 738}, // 227 FixedSinCosPiINTEL
  {IR(42986, 14), 733}, // 228 FixedSinINTEL
  {IR(43031, 16), 736}, // 229 FixedSinPiINTEL
  {IR(42939, 15), 730}, // 230 FixedSqrtINTEL
  {IR(32755, 17), 411}, // 231 FragmentFetchAMD
  {IR(32734, 21), 410}, // 232 FragmentMaskFetchAMD
  {IR(24209, 9), 48}, // 233 Function
  {IR(26211, 13), 51}, // 234 FunctionCall
  {IR(26199, 12), 50}, // 235 FunctionEnd
  {IR(26181, 18), 49}, // 236 FunctionParameter
  {IR(36468, 25), 554}, // 237 FunctionPointerCallINTEL
  {IR(28265, 7), 194}, // 238 Fwidth
  {IR(28323, 13), 200}, // 239 FwidthCoarse
  {IR(28290, 11), 197}, // 240 FwidthFine
  {IR(27332, 17), 112}, // 241 GenericCastToPtr
  {IR(27349, 25), 113}, // 242 GenericCastToPtrExplicit
  {IR(26388, 23), 62}, // 243 GenericPtrMemSemantics
  {IR(29629, 16), 276}, // 244 GetDefaultQueue
  {IR(30134, 35), 298}, // 245 GetKernelLocalSizeForSubgroupCount
  {IR(30169, 25), 299}, // 246 GetKernelMaxNumSubgroups
  {IR(29435, 32), 267}, // 247 GetKernelNDrangeMaxSubGroupSize
  {IR(29405, 30), 266}, // 248 GetKernelNDrangeSubGroupCount
  {IR(29490, 40), 269}, // 249 GetKernelPreferredWorkGroupSizeMultiple
  {IR(29467, 23), 268}, // 250 GetKernelWorkGroupSize
  {IR(29261, 18), 259}, // 251 GetMaxPipePackets
  {IR(29243, 18), 258}, // 252 GetNumPipePackets
  {IR(28948, 9), 238}, // 253 GroupAll
  {IR(28957, 9), 239}, // 254 GroupAny
  {IR(28917, 15), 236}, // 255 GroupAsyncCopy
  {IR(44516, 19), 789}, // 256 GroupBitwiseAndKHR
  {IR(44535, 18), 790}, // 257 GroupBitwiseOrKHR
  {IR(44553, 19), 791}, // 258 GroupBitwiseXorKHR
  {IR(28966, 15), 240}, // 259 GroupBroadcast
  {IR(29336, 20), 262}, // 260 GroupCommitReadPipe
  {IR(29356, 21), 263}, // 261 GroupCommitWritePipe
  {IR(26502, 14), 67}, // 262 GroupDecorate
  {IR(29024, 10), 242}, // 263 GroupFAdd
  {IR(32573, 23), 403}, // 264 GroupFAddNonUniformAMD
  {IR(29064, 10), 246}, // 265 GroupFMax
  {IR(32665, 23), 407}, // 266 GroupFMaxNonUniformAMD
  {IR(29034, 10), 243}, // 267 GroupFMin
  {IR(32596, 23), 404}, // 268 GroupFMinNonUniformAMD
  {IR(44503, 13), 788}, // 269 GroupFMulKHR
  {IR(29014, 10), 241}, // 270 GroupIAdd
  {IR(32550, 23), 402}, // 271 GroupIAddNonUniformAMD
  {IR(44490, 13), 787}, // 272 GroupIMulKHR
  {IR(44572, 19), 792}, // 273 GroupLogicalAndKHR
  {IR(44591, 18), 793}, // 274 GroupLogicalOrKHR
  {IR(44609, 19), 794}, // 275 GroupLogicalXorKHR
  {IR(26561, 20), 68}, // 276 GroupMemberDecorate
  {IR(30317, 19), 307}, // 277 GroupNonUniformAll
  {IR(30355, 24), 309}, // 278 GroupNonUniformAllEqual
  {IR(30336, 19), 308}, // 279 GroupNonUniformAny
  {IR(5554, 22), 312}, // 280 GroupNonUniformBallot
  {IR(30495, 30), 315}, // 281 GroupNonUniformBallotBitCount
  {IR(30463, 32), 314}, // 282 GroupNonUniformBallotBitExtract
  {IR(30525, 29), 316}, // 283 GroupNonUniformBallotFindLSB
  {IR(30554, 29), 317}, // 284 GroupNonUniformBallotFindMSB
  {IR(30861, 26), 332}, // 285 GroupNonUniformBitwiseAnd
  {IR(30887, 25), 333}, // 286 GroupNonUniformBitwiseOr
  {IR(30912, 26), 334}, // 287 GroupNonUniformBitwiseXor
  {IR(30379, 25), 310}, // 288 GroupNonUniformBroadcast
  {IR(30404, 30), 311}, // 289 GroupNonUniformBroadcastFirst
  {IR(30296, 21), 306}, // 290 GroupNonUniformElect
  {IR(30681, 20), 323}, // 291 GroupNonUniformFAdd
  {IR(30841, 20), 331}, // 292 GroupNonUniformFMax
  {IR(30781, 20), 328}, // 293 GroupNonUniformFMin
  {IR(30721, 20), 325}, // 294 GroupNonUniformFMul
  {IR(30661, 20), 322}, // 295 GroupNonUniformIAdd
  {IR(30701, 20), 324}, // 296 GroupNonUniformIMul
  {IR(30434, 29), 313}, // 297 GroupNonUniformInverseBallot
  {IR(30938, 26), 335}, // 298 GroupNonUniformLogicalAnd
  {IR(30964, 25), 336}, // 299 GroupNonUniformLogicalOr
  {IR(30989, 26), 337}, // 300 GroupNonUniformLogicalXor
  {IR(34160, 27), 465}, // 301 GroupNonUniformPartitionNV
  {IR(32980, 26), 421}, // 302 GroupNonUniformQuadAllKHR
  {IR(33006, 26), 422}, // 303 GroupNonUniformQuadAnyKHR
  {IR(31015, 29), 338}, // 304 GroupNonUniformQuadBroadcast
  {IR(31044, 24), 339}, // 305 GroupNonUniformQuadSwap
  {IR(13076, 25), 365}, // 306 GroupNonUniformRotateKHR
  {IR(30801, 20), 329}, // 307 GroupNonUniformSMax
  {IR(30741, 20), 326}, // 308 GroupNonUniformSMin
  {IR(8644, 23), 318}, // 309 GroupNonUniformShuffle
  {IR(30634, 27), 321}, // 310 GroupNonUniformShuffleDown
  {IR(30609, 25), 320}, // 311 GroupNonUniformShuffleUp
  {IR(30583, 26), 319}, // 312 GroupNonUniformShuffleXor
  {IR(30821, 20), 330}, // 313 GroupNonUniformUMax
  {IR(30761, 20), 327}, // 314 GroupNonUniformUMin
  {IR(29279, 28), 260}, // 315 GroupReserveReadPipePackets
  {IR(29307, 29), 261}, // 316 GroupReserveWritePipePackets
  {IR(29084, 10), 248}, // 317 GroupSMax
  {IR(32711, 23), 409}, // 318 GroupSMaxNonUniformAMD
  {IR(29054, 10), 245}, // 319 GroupSMin
  {IR(32642, 23), 406}, // 320 GroupSMinNonUniformAMD
  {IR(29074, 10), 247}, // 321 GroupUMax
  {IR(32688, 23), 408}, // 322 GroupUMaxNonUniformAMD
  {IR(29044, 10), 244}, // 323 GroupUMin
  {IR(32619, 23), 405}, // 324 GroupUMinNonUniformAMD
  {IR(28932, 16), 237}, // 325 GroupWaitEvents
  {IR(33467, 25), 438}, // 326 HitObjectExecuteShaderNV
  {IR(33518, 25), 440}, // 327 HitObjectGetAttributesNV
  {IR(34608, 24), 481}, // 328 HitObjectGetClusterIdNV
  {IR(33492, 26), 439}, // 329 HitObjectGetCurrentTimeNV
  {IR(33594, 28), 443}, // 330 HitObjectGetGeometryIndexNV
  {IR(33543, 22), 441}, // 331 HitObjectGetHitKindNV
  {IR(33647, 34), 445}, // 332 HitObjectGetInstanceCustomIndexNV
  {IR(33622, 25), 444}, // 333 HitObjectGetInstanceIdNV
  {IR(35835, 27), 523}, // 334 HitObjectGetLSSPositionsNV
  {IR(35862, 23), 524}, // 335 HitObjectGetLSSRadiiNV
  {IR(33179, 33), 428}, // 336 HitObjectGetObjectRayDirectionNV
  {IR(33212, 30), 429}, // 337 HitObjectGetObjectRayOriginNV
  {IR(33151, 28), 427}, // 338 HitObjectGetObjectToWorldNV
  {IR(33565, 29), 442}, // 339 HitObjectGetPrimitiveIndexNV
  {IR(33742, 22), 448}, // 340 HitObjectGetRayTMaxNV
  {IR(33764, 22), 449}, // 341 HitObjectGetRayTMinNV
  {IR(33307, 44), 432}, // 342 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(33268, 39), 431}, // 343 HitObjectGetShaderRecordBufferHandleNV
  {IR(35779, 29), 521}, // 344 HitObjectGetSpherePositionNV
  {IR(35808, 27), 522}, // 345 HitObjectGetSphereRadiusNV
  {IR(33681, 32), 446}, // 346 HitObjectGetWorldRayDirectionNV
  {IR(33713, 29), 447}, // 347 HitObjectGetWorldRayOriginNV
  {IR(33123, 28), 426}, // 348 HitObjectGetWorldToObjectNV
  {IR(33786, 19), 450}, // 349 HitObjectIsEmptyNV
  {IR(33805, 17), 451}, // 350 HitObjectIsHitNV
  {IR(35908, 20), 526}, // 351 HitObjectIsLSSHitNV
  {IR(33822, 18), 452}, // 352 HitObjectIsMissNV
  {IR(35885, 23), 525}, // 353 HitObjectIsSphereHitNV
  {IR(33351, 23), 433}, // 354 HitObjectRecordEmptyNV
  {IR(33032, 27), 423}, // 355 HitObjectRecordHitMotionNV
  {IR(33394, 21), 435}, // 356 HitObjectRecordHitNV
  {IR(33059, 36), 424}, // 357 HitObjectRecordHitWithIndexMotionNV
  {IR(33415, 30), 436}, // 358 HitObjectRecordHitWithIndexNV
  {IR(33095, 28), 425}, // 359 HitObjectRecordMissMotionNV
  {IR(33445, 22), 437}, // 360 HitObjectRecordMissNV
  {IR(33242, 26), 430}, // 361 HitObjectTraceRayMotionNV
  {IR(33374, 20), 434}, // 362 HitObjectTraceRayNV
  {IR(27398, 5), 117}, // 363 IAdd
  {IR(27575, 10), 138}, // 364 IAddCarry
  {IR(36287, 13), 543}, // 365 IAddSatINTEL
  {IR(36313, 14), 545}, // 366 IAverageINTEL
  {IR(36341, 21), 547}, // 367 IAverageRoundedINTEL
  {IR(27771, 7), 158}, // 368 IEqual
  {IR(27418, 5), 121}, // 369 IMul
  {IR(36409, 15), 551}, // 370 IMul32x16INTEL
  {IR(27778, 10), 159}, // 371 INotEqual
  {IR(27408, 5), 119}, // 372 ISub
  {IR(27585, 11), 139}, // 373 ISubBorrow
  {IR(36383, 13), 549}, // 374 ISubSatINTEL
  {IR(31691, 22), 371}, // 375 IgnoreIntersectionKHR
  {IR(34385, 21), 472}, // 376 IgnoreIntersectionNV
  {IR(24253, 6), 91}, // 377 Image
  {IR(32521, 29), 401}, // 378 ImageBlockMatchGatherSADQCOM
  {IR(32492, 29), 400}, // 379 ImageBlockMatchGatherSSDQCOM
  {IR(32411, 23), 397}, // 380 ImageBlockMatchSADQCOM
  {IR(32388, 23), 396}, // 381 ImageBlockMatchSSDQCOM
  {IR(32463, 29), 399}, // 382 ImageBlockMatchWindowSADQCOM
  {IR(32434, 29), 398}, // 383 ImageBlockMatchWindowSSDQCOM
  {IR(32369, 19), 395}, // 384 ImageBoxFilterQCOM
  {IR(27016, 16), 88}, // 385 ImageDrefGather
  {IR(26993, 11), 86}, // 386 ImageFetch
  {IR(27004, 12), 87}, // 387 ImageGather
  {IR(27053, 17), 92}, // 388 ImageQueryFormat
  {IR(27133, 17), 97}, // 389 ImageQueryLevels
  {IR(27119, 14), 96}, // 390 ImageQueryLod
  {IR(27070, 16), 93}, // 391 ImageQueryOrder
  {IR(27150, 18), 98}, // 392 ImageQuerySamples
  {IR(27104, 15), 95}, // 393 ImageQuerySize
  {IR(27086, 18), 94}, // 394 ImageQuerySizeLod
  {IR(27032, 10), 89}, // 395 ImageRead
  {IR(26850, 27), 81}, // 396 ImageSampleDrefExplicitLod
  {IR(26823, 27), 80}, // 397 ImageSampleDrefImplicitLod
  {IR(26800, 23), 79}, // 398 ImageSampleExplicitLod
  {IR(33909, 23), 456}, // 399 ImageSampleFootprintNV
  {IR(26754, 23), 78}, // 400 ImageSampleImplicitLod
  {IR(26962, 31), 85}, // 401 ImageSampleProjDrefExplicitLod
  {IR(26931, 31), 84}, // 402 ImageSampleProjDrefImplicitLod
  {IR(26904, 27), 83}, // 403 ImageSampleProjExplicitLod
  {IR(26877, 27), 82}, // 404 ImageSampleProjImplicitLod
  {IR(32345, 24), 394}, // 405 ImageSampleWeightedQCOM
  {IR(29957, 22), 288}, // 406 ImageSparseDrefGather
  {IR(29922, 17), 286}, // 407 ImageSparseFetch
  {IR(29939, 18), 287}, // 408 ImageSparseGather
  {IR(30049, 16), 293}, // 409 ImageSparseRead
  {IR(29749, 33), 281}, // 410 ImageSparseSampleDrefExplicitLod
  {IR(29716, 33), 280}, // 411 ImageSparseSampleDrefImplicitLod
  {IR(29687, 29), 279}, // 412 ImageSparseSampleExplicitLod
  {IR(29658, 29), 278}, // 413 ImageSparseSampleImplicitLod
  {IR(29885, 37), 285}, // 414 ImageSparseSampleProjDrefExplicitLod
  {IR(29848, 37), 284}, // 415 ImageSparseSampleProjDrefImplicitLod
  {IR(29815, 33), 283}, // 416 ImageSparseSampleProjExplicitLod
  {IR(29782, 33), 282}, // 417 ImageSparseSampleProjImplicitLod
  {IR(29979, 26), 289}, // 418 ImageSparseTexelsResident
  {IR(26233, 18), 53}, // 419 ImageTexelPointer
  {IR(27042, 11), 90}, // 420 ImageWrite
  {IR(26341, 20), 59}, // 421 InBoundsAccessChain
  {IR(26411, 23), 63}, // 422 InBoundsPtrAccessChain
  {IR(27642, 9), 146}, // 423 IsFinite
  {IR(35270, 22), 504}, // 424 IsHelperInvocationEXT
  {IR(27636, 6), 145}, // 425 IsInf
  {IR(27630, 6), 144}, // 426 IsNan
  {IR(32915, 23), 418}, // 427 IsNodePayloadValidAMDX
  {IR(27651, 9), 147}, // 428 IsNormal
  {IR(29571, 13), 273}, // 429 IsValidEvent
  {IR(29226, 17), 257}, // 430 IsValidReserveId
  {IR(28854, 5), 230}, // 431 Kill
  {IR(28771, 6), 226}, // 432 Label
  {IR(27671, 14), 149}, // 433 LessOrGreater
  {IR(28890, 14), 234}, // 434 LifetimeStart
  {IR(28904, 13), 235}, // 435 LifetimeStop
  {IR(25084, 5), 8}, // 436 Line
  {IR(26291, 5), 54}, // 437 Load
  {IR(27742, 11), 155}, // 438 LogicalAnd
  {IR(27703, 13), 152}, // 439 LogicalEqual
  {IR(27753, 11), 156}, // 440 LogicalNot
  {IR(27716, 16), 153}, // 441 LogicalNotEqual
  {IR(27732, 10), 154}, // 442 LogicalOr
  {IR(42857, 17), 726}, // 443 LoopControlINTEL
  {IR(28711, 10), 224}, // 444 LoopMerge
  {IR(44646, 18), 796}, // 445 MaskedGatherINTEL
  {IR(44664, 19), 797}, // 446 MaskedScatterINTEL
  {IR(27540, 18), 135}, // 447 MatrixTimesMatrix
  {IR(27486, 18), 132}, // 448 MatrixTimesScalar
  {IR(27522, 18), 134}, // 449 MatrixTimesVector
  {IR(26471, 15), 65}, // 450 MemberDecorate
  {IR(36609, 21), 563}, // 451 MemberDecorateString
  {IR(36630, 27), 563}, // 452 MemberDecorateStringGOOGLE
  {IR(25066, 11), 6}, // 453 MemberName
  {IR(28448, 14), 206}, // 454 MemoryBarrier
  {IR(25231, 12), 12}, // 455 MemoryModel
  {IR(30234, 19), 302}, // 456 MemoryNamedBarrier
  {IR(30253, 16), 303}, // 457 ModuleProcessed
  {IR(25061, 5), 5}, // 458 Name
  {IR(30211, 23), 301}, // 459 NamedBarrierInitialize
  {IR(30005, 7), 290}, // 460 NoLine
  {IR(32888, 27), 417}, // 461 NodePayloadArrayLengthAMDX
  {IR(24857, 4), 0}, // 462 Nop
  {IR(28182, 4), 186}, // 463 Not
  {IR(27685, 8), 150}, // 464 Ordered
  {IR(27558, 13), 136}, // 465 OuterProduct
  {IR(28677, 4), 223}, // 466 Phi
  {IR(26361, 15), 60}, // 467 PtrAccessChain
  {IR(43110, 29), 741}, // 468 PtrCastToCrossWorkgroupINTEL
  {IR(27315, 17), 111}, // 469 PtrCastToGeneric
  {IR(31101, 8), 343}, // 470 PtrDiff
  {IR(31080, 9), 341}, // 471 PtrEqual
  {IR(31089, 12), 342}, // 472 PtrNotEqual
  {IR(27243, 14), 106}, // 473 QuantizeToF16
  {IR(35575, 17), 515}, // 474 RawAccessChainNV
  {IR(32264, 31), 391}, // 475 RayQueryConfirmIntersectionKHR
  {IR(32232, 32), 390}, // 476 RayQueryGenerateIntersectionKHR
  {IR(34585, 23), 480}, // 477 RayQueryGetClusterIdNV
  {IR(43515, 39), 754}, // 478 RayQueryGetIntersectionBarycentricsKHR
  {IR(43590, 46), 756}, // 479 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(43554, 36), 755}, // 480 RayQueryGetIntersectionFrontFaceKHR
  {IR(43434, 40), 752}, // 481 RayQueryGetIntersectionGeometryIndexKHR
  {IR(43286, 46), 749}, // 482 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(43332, 37), 750}, // 483 RayQueryGetIntersectionInstanceIdKHR
  {IR(43369, 65), 751}, // 484 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(35742, 37), 520}, // 485 RayQueryGetIntersectionLSSHitValueNV
  {IR(35670, 38), 518}, // 486 RayQueryGetIntersectionLSSPositionsNV
  {IR(35708, 34), 519}, // 487 RayQueryGetIntersectionLSSRadiiNV
  {IR(43636, 45), 757}, // 488 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(43681, 42), 758}, // 489 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(43784, 40), 761}, // 490 RayQueryGetIntersectionObjectToWorldKHR
  {IR(43474, 41), 753}, // 491 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(35592, 40), 516}, // 492 RayQueryGetIntersectionSpherePositionNV
  {IR(35632, 38), 517}, // 493 RayQueryGetIntersectionSphereRadiusNV
  {IR(43258, 28), 748}, // 494 RayQueryGetIntersectionTKHR
  {IR(34460, 50), 477}, // 495 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(32314, 31), 393}, // 496 RayQueryGetIntersectionTypeKHR
  {IR(43824, 40), 762}, // 497 RayQueryGetIntersectionWorldToObjectKHR
  {IR(43235, 23), 747}, // 498 RayQueryGetRayFlagsKHR
  {IR(43213, 22), 746}, // 499 RayQueryGetRayTMinKHR
  {IR(43723, 32), 759}, // 500 RayQueryGetWorldRayDirectionKHR
  {IR(43755, 29), 760}, // 501 RayQueryGetWorldRayOriginKHR
  {IR(32189, 22), 388}, // 502 RayQueryInitializeKHR
  {IR(35950, 19), 528}, // 503 RayQueryIsLSSHitNV
  {IR(35928, 22), 527}, // 504 RayQueryIsSphereHitNV
  {IR(32295, 19), 392}, // 505 RayQueryProceedKHR
  {IR(32211, 21), 389}, // 506 RayQueryTerminateKHR
  {IR(32772, 13), 412}, // 507 ReadClockKHR
  {IR(29094, 9), 249}, // 508 ReadPipe
  {IR(43168, 22), 743}, // 509 ReadPipeBlockingINTEL
  {IR(29542, 13), 271}, // 510 ReleaseEvent
  {IR(33869, 24), 454}, // 511 ReorderThreadWithHintNV
  {IR(33840, 29), 453}, // 512 ReorderThreadWithHitObjectNV
  {IR(34342, 22), 471}, // 513 ReportIntersectionKHR
  {IR(34364, 21), 471}, // 514 ReportIntersectionNV
  {IR(29148, 23), 253}, // 515 ReserveReadPipePackets
  {IR(29171, 24), 254}, // 516 ReserveWritePipePackets
  {IR(29113, 17), 251}, // 517 ReservedReadPipe
  {IR(29130, 18), 252}, // 518 ReservedWritePipe
  {IR(41839, 19), 684}, // 519 RestoreMemoryINTEL
  {IR(29530, 12), 270}, // 520 RetainEvent
  {IR(28859, 7), 231}, // 521 Return
  {IR(28866, 12), 232}, // 522 ReturnValue
  {IR(44628, 18), 795}, // 523 RoundFToTF32INTEL
  {IR(27225, 9), 104}, // 524 SConvert
  {IR(27433, 5), 124}, // 525 SDiv
  {IR(31776, 5), 373}, // 526 SDot
  {IR(31817, 11), 376}, // 527 SDotAccSat
  {IR(31828, 14), 376}, // 528 SDotAccSatKHR
  {IR(31781, 8), 373}, // 529 SDotKHR
  {IR(27801, 13), 161}, // 530 SGreaterThan
  {IR(27832, 18), 163}, // 531 SGreaterThanEqual
  {IR(27860, 10), 165}, // 532 SLessThan
  {IR(27885, 15), 167}, // 533 SLessThanEqual
  {IR(27453, 5), 128}, // 534 SMod
  {IR(27609, 13), 141}, // 535 SMulExtended
  {IR(27382, 8), 115}, // 536 SNegate
  {IR(27448, 5), 127}, // 537 SRem
  {IR(31802, 6), 375}, // 538 SUDot
  {IR(31867, 12), 378}, // 539 SUDotAccSat
  {IR(31879, 15), 378}, // 540 SUDotAccSatKHR
  {IR(31808, 9), 375}, // 541 SUDotKHR
  {IR(26709, 13), 77}, // 542 SampledImage
  {IR(35494, 29), 514}, // 543 SamplerImageAddressingModeNV
  {IR(27271, 15), 108}, // 544 SatConvertSToU
  {IR(27286, 15), 109}, // 545 SatConvertUToS
  {IR(41823, 16), 683}, // 546 SaveMemoryINTEL
  {IR(27764, 7), 157}, // 547 Select
  {IR(28756, 15), 225}, // 548 SelectionMerge
  {IR(34142, 18), 464}, // 549 SetMeshOutputsEXT
  {IR(29584, 19), 274}, // 550 SetUserEventStatus
  {IR(28133, 17), 182}, // 551 ShiftLeftLogical
  {IR(28112, 21), 181}, // 552 ShiftRightArithmetic
  {IR(28094, 18), 180}, // 553 ShiftRightLogical
  {IR(27660, 11), 148}, // 554 SignBitSet
  {IR(30065, 7), 294}, // 555 SizeOf
  {IR(25038, 7), 3}, // 556 Source
  {IR(24919, 16), 2}, // 557 SourceContinued
  {IR(25045, 16), 4}, // 558 SourceExtension
  {IR(26056, 13), 45}, // 559 SpecConstant
  {IR(26069, 22), 46}, // 560 SpecConstantComposite
  {IR(43958, 36), 767}, // 561 SpecConstantCompositeContinuedINTEL
  {IR(32108, 34), 385}, // 562 SpecConstantCompositeReplicateEXT
  {IR(26038, 18), 44}, // 563 SpecConstantFalse
  {IR(26132, 15), 47}, // 564 SpecConstantOp
  {IR(32957, 23), 420}, // 565 SpecConstantStringAMDX
  {IR(26021, 17), 43}, // 566 SpecConstantTrue
  {IR(31155, 25), 346}, // 567 StencilAttachmentReadEXT
  {IR(26296, 6), 55}, // 568 Store
  {IR(25077, 7), 7}, // 569 String
  {IR(44259, 25), 780}, // 570 Subgroup2DBlockLoadINTEL
  {IR(44284, 34), 781}, // 571 Subgroup2DBlockLoadTransformINTEL
  {IR(44318, 34), 782}, // 572 Subgroup2DBlockLoadTransposeINTEL
  {IR(44352, 29), 783}, // 573 Subgroup2DBlockPrefetchINTEL
  {IR(44381, 26), 784}, // 574 Subgroup2DBlockStoreINTEL
  {IR(31553, 20), 364}, // 575 SubgroupAllEqualKHR
  {IR(31523, 15), 362}, // 576 SubgroupAllKHR
  {IR(31538, 15), 363}, // 577 SubgroupAnyKHR
  {IR(40341, 30), 647}, // 578 SubgroupAvcBmeInitializeINTEL
  {IR(40311, 30), 646}, // 579 SubgroupAvcFmeInitializeINTEL
  {IR(38746, 35), 616}, // 580 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(38781, 39), 617}, // 581 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(39422, 38), 630}, // 582 SubgroupAvcImeConvertToMceResultINTEL
  {IR(39045, 45), 623}, // 583 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(39145, 53), 625}, // 584 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(39366, 56), 629}, // 585 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(39254, 54), 627}, // 586 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(38998, 47), 622}, // 587 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(39090, 55), 624}, // 588 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(39308, 58), 628}, // 589 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(39198, 56), 626}, // 590 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(40054, 36), 641}, // 591 SubgroupAvcImeGetBorderReachedINTEL
  {IR(39506, 44), 632}, // 592 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(39460, 46), 631}, // 593 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(39921, 66), 639}, // 594 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(39853, 68), 638}, // 595 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(39987, 67), 640}, // 596 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(39716, 68), 636}, // 597 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(39646, 70), 635}, // 598 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(39784, 69), 637}, // 599 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(40090, 48), 642}, // 600 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(40138, 59), 643}, // 601 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(40255, 56), 645}, // 602 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(40197, 58), 644}, // 603 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(38609, 30), 612}, // 604 SubgroupAvcImeInitializeINTEL
  {IR(38713, 33), 615}, // 605 SubgroupAvcImeRefWindowSizeINTEL
  {IR(38677, 36), 614}, // 606 SubgroupAvcImeSetDualReferenceINTEL
  {IR(38910, 54), 620}, // 607 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(38820, 43), 618}, // 608 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(38639, 38), 613}, // 609 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(38863, 47), 619}, // 610 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(38964, 34), 621}, // 611 SubgroupAvcImeSetWeightedSadINTEL
  {IR(39599, 47), 634}, // 612 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(39550, 49), 633}, // 613 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(37999, 39), 597}, // 614 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(38038, 38), 598}, // 615 SubgroupAvcMceConvertToImeResultINTEL
  {IR(38076, 39), 599}, // 616 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(38115, 38), 600}, // 617 SubgroupAvcMceConvertToRefResultINTEL
  {IR(38153, 39), 601}, // 618 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(38192, 38), 602}, // 619 SubgroupAvcMceConvertToSicResultINTEL
  {IR(38305, 43), 605}, // 620 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(37437, 50), 586}, // 621 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(37035, 60), 578}, // 622 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(37235, 51), 582}, // 623 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(37381, 56), 585}, // 624 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(37148, 47), 580}, // 625 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(37736, 56), 592}, // 626 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(37635, 50), 590}, // 627 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(37330, 51), 584}, // 628 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(37539, 49), 588}, // 629 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(37487, 52), 587}, // 630 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(37685, 51), 591}, // 631 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(38424, 38), 608}, // 632 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(38266, 39), 604}, // 633 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(38348, 38), 606}, // 634 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(38386, 38), 607}, // 635 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(38462, 45), 609}, // 636 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(38507, 40), 610}, // 637 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(38547, 62), 611}, // 638 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(38230, 36), 603}, // 639 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(37792, 33), 593}, // 640 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(37938, 61), 596}, // 641 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(37095, 53), 579}, // 642 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(37286, 44), 583}, // 643 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(37195, 40), 581}, // 644 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(37588, 47), 589}, // 645 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(37877, 61), 595}, // 646 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(37825, 52), 594}, // 647 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(40371, 39), 648}, // 648 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(40693, 38), 655}, // 649 SubgroupAvcRefConvertToMceResultINTEL
  {IR(40546, 45), 652}, // 650 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(40591, 46), 653}, // 651 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(40637, 56), 654}, // 652 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(40499, 47), 651}, // 653 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(40410, 46), 649}, // 654 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(40456, 43), 650}, // 655 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(40829, 42), 659}, // 656 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(40793, 36), 658}, // 657 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(40761, 32), 657}, // 658 SubgroupAvcSicConfigureSkcINTEL
  {IR(40910, 39), 661}, // 659 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(41450, 38), 673}, // 660 SubgroupAvcSicConvertToMceResultINTEL
  {IR(41225, 31), 668}, // 661 SubgroupAvcSicEvaluateIpeINTEL
  {IR(41303, 45), 670}, // 662 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(41348, 46), 671}, // 663 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(41394, 56), 672}, // 664 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41256, 47), 669}, // 665 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(41567, 46), 676}, // 666 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(41523, 44), 675}, // 667 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(41788, 35), 681}, // 668 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(41654, 36), 678}, // 669 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(41488, 35), 674}, // 670 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(40871, 39), 660}, // 671 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(41613, 41), 677}, // 672 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(41690, 50), 679}, // 673 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(41740, 48), 680}, // 674 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(40731, 30), 656}, // 675 SubgroupAvcSicInitializeINTEL
  {IR(41091, 43), 665}, // 676 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(41182, 43), 667}, // 677 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(41041, 50), 664}, // 678 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(40993, 48), 663}, // 679 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(40949, 44), 662}, // 680 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(41134, 48), 666}, // 681 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(5315, 18), 356}, // 682 SubgroupBallotKHR
  {IR(44232, 27), 779}, // 683 SubgroupBlockPrefetchINTEL
  {IR(36041, 23), 533}, // 684 SubgroupBlockReadINTEL
  {IR(36064, 24), 534}, // 685 SubgroupBlockWriteINTEL
  {IR(31397, 27), 357}, // 686 SubgroupFirstInvocationKHR
  {IR(36088, 28), 535}, // 687 SubgroupImageBlockReadINTEL
  {IR(36116, 29), 536}, // 688 SubgroupImageBlockWriteINTEL
  {IR(36145, 33), 537}, // 689 SubgroupImageMediaBlockReadINTEL
  {IR(36178, 34), 538}, // 690 SubgroupImageMediaBlockWriteINTEL
  {IR(13621, 38), 785}, // 691 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(31573, 26), 366}, // 692 SubgroupReadInvocationKHR
  {IR(35969, 25), 530}, // 693 SubgroupShuffleDownINTEL
  {IR(11836, 21), 529}, // 694 SubgroupShuffleINTEL
  {IR(35994, 23), 531}, // 695 SubgroupShuffleUpINTEL
  {IR(36017, 24), 532}, // 696 SubgroupShuffleXorINTEL
  {IR(28847, 7), 229}, // 697 Switch
  {IR(44141, 23), 775}, // 698 TaskSequenceAsyncINTEL
  {IR(44117, 24), 774}, // 699 TaskSequenceCreateINTEL
  {IR(44164, 21), 776}, // 700 TaskSequenceGetINTEL
  {IR(44185, 25), 777}, // 701 TaskSequenceReleaseINTEL
  {IR(35312, 27), 506}, // 702 TensorLayoutSetBlockSizeNV
  {IR(35176, 28), 499}, // 703 TensorLayoutSetClampValueNV
  {IR(35105, 27), 496}, // 704 TensorLayoutSetDimensionNV
  {IR(35132, 24), 497}, // 705 TensorLayoutSetStrideNV
  {IR(35156, 20), 498}, // 706 TensorLayoutSliceNV
  {IR(31265, 19), 350}, // 707 TensorQuerySizeARM
  {IR(31236, 14), 348}, // 708 TensorReadARM
  {IR(35292, 20), 505}, // 709 TensorViewSetClipNV
  {IR(35223, 25), 501}, // 710 TensorViewSetDimensionNV
  {IR(35248, 22), 502}, // 711 TensorViewSetStrideNV
  {IR(31250, 15), 349}, // 712 TensorWriteARM
  {IR(31284, 20), 351}, // 713 TerminateInvocation
  {IR(31713, 16), 372}, // 714 TerminateRayKHR
  {IR(34406, 15), 473}, // 715 TerminateRayNV
  {IR(34429, 14), 475}, // 716 TraceMotionNV
  {IR(34421, 8), 474}, // 717 TraceNV
  {IR(31625, 12), 368}, // 718 TraceRayKHR
  {IR(34443, 17), 476}, // 719 TraceRayMotionNV
  {IR(26699, 10), 76}, // 720 Transpose
  {IR(34510, 29), 478}, // 721 TypeAccelerationStructureKHR
  {IR(34539, 28), 478}, // 722 TypeAccelerationStructureNV
  {IR(25648, 10), 25}, // 723 TypeArray
  {IR(36954, 37), 575}, // 724 TypeAvcImeDualReferenceStreaminINTEL
  {IR(36689, 23), 566}, // 725 TypeAvcImePayloadINTEL
  {IR(36871, 44), 573}, // 726 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(36803, 22), 571}, // 727 TypeAvcImeResultINTEL
  {IR(36825, 46), 572}, // 728 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(36915, 39), 574}, // 729 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(36758, 23), 569}, // 730 TypeAvcMcePayloadINTEL
  {IR(36781, 22), 570}, // 731 TypeAvcMceResultINTEL
  {IR(36712, 23), 567}, // 732 TypeAvcRefPayloadINTEL
  {IR(36991, 22), 576}, // 733 TypeAvcRefResultINTEL
  {IR(36735, 23), 568}, // 734 TypeAvcSicPayloadINTEL
  {IR(37013, 22), 577}, // 735 TypeAvcSicResultINTEL
  {IR(25410, 9), 17}, // 736 TypeBool
  {IR(43878, 23), 764}, // 737 TypeBufferSurfaceINTEL
  {IR(31894, 25), 379}, // 738 TypeCooperativeMatrixKHR
  {IR(34632, 24), 482}, // 739 TypeCooperativeMatrixNV
  {IR(33932, 24), 457}, // 740 TypeCooperativeVectorNV
  {IR(25763, 16), 32}, // 741 TypeDeviceEvent
  {IR(25753, 10), 31}, // 742 TypeEvent
  {IR(25464, 10), 19}, // 743 TypeFloat
  {IR(25812, 19), 36}, // 744 TypeForwardPointer
  {IR(25740, 13), 30}, // 745 TypeFunction
  {IR(33893, 16), 455}, // 746 TypeHitObjectNV
  {IR(25609, 10), 22}, // 747 TypeImage
  {IR(25419, 8), 18}, // 748 TypeInt
  {IR(25485, 11), 21}, // 749 TypeMatrix
  {IR(30194, 17), 300}, // 750 TypeNamedBarrier
  {IR(32834, 25), 415}, // 751 TypeNodePayloadArrayAMDX
  {IR(25686, 11), 28}, // 752 TypeOpaque
  {IR(25803, 9), 35}, // 753 TypePipe
  {IR(30072, 16), 295}, // 754 TypePipeStorage
  {IR(25728, 12), 29}, // 755 TypePointer
  {IR(25793, 10), 34}, // 756 TypeQueue
  {IR(32173, 16), 387}, // 757 TypeRayQueryKHR
  {IR(25779, 14), 33}, // 758 TypeReserveId
  {IR(25658, 17), 26}, // 759 TypeRuntimeArray
  {IR(25631, 17), 24}, // 760 TypeSampledImage
  {IR(25619, 12), 23}, // 761 TypeSampler
  {IR(25675, 11), 27}, // 762 TypeStruct
  {IR(43901, 25), 765}, // 763 TypeStructContinuedINTEL
  {IR(44210, 22), 778}, // 764 TypeTaskSequenceINTEL
  {IR(31180, 14), 347}, // 765 TypeTensorARM
  {IR(35048, 19), 493}, // 766 TypeTensorLayoutNV
  {IR(35067, 17), 494}, // 767 TypeTensorViewNV
  {IR(31304, 22), 352}, // 768 TypeUntypedPointerKHR
  {IR(25474, 11), 20}, // 769 TypeVector
  {IR(36671, 18), 565}, // 770 TypeVmeImageINTEL
  {IR(25401, 9), 16}, // 771 TypeVoid
  {IR(36300, 13), 544}, // 772 UAddSatINTEL
  {IR(36327, 14), 546}, // 773 UAverageINTEL
  {IR(36362, 21), 548}, // 774 UAverageRoundedINTEL
  {IR(27216, 9), 103}, // 775 UConvert
  {IR(36212, 24), 539}, // 776 UCountLeadingZerosINTEL
  {IR(36236, 25), 540}, // 777 UCountTrailingZerosINTEL
  {IR(27428, 5), 123}, // 778 UDiv
  {IR(31789, 5), 374}, // 779 UDot
  {IR(31842, 11), 377}, // 780 UDotAccSat
  {IR(31853, 14), 377}, // 781 UDotAccSatKHR
  {IR(31794, 8), 374}, // 782 UDotKHR
  {IR(27788, 13), 160}, // 783 UGreaterThan
  {IR(27814, 18), 162}, // 784 UGreaterThanEqual
  {IR(27850, 10), 164}, // 785 ULessThan
  {IR(27870, 15), 166}, // 786 ULessThanEqual
  {IR(27443, 5), 126}, // 787 UMod
  {IR(36424, 15), 552}, // 788 UMul32x16INTEL
  {IR(27596, 13), 140}, // 789 UMulExtended
  {IR(36396, 13), 550}, // 790 USubSatINTEL
  {IR(24913, 6), 1}, // 791 Undef
  {IR(27693, 10), 151}, // 792 Unordered
  {IR(28878, 12), 233}, // 793 Unreachable
  {IR(31345, 22), 354}, // 794 UntypedAccessChainKHR
  {IR(31482, 22), 360}, // 795 UntypedArrayLengthKHR
  {IR(31367, 30), 355}, // 796 UntypedInBoundsAccessChainKHR
  {IR(31449, 33), 359}, // 797 UntypedInBoundsPtrAccessChainKHR
  {IR(31504, 19), 361}, // 798 UntypedPrefetchKHR
  {IR(31424, 25), 358}, // 799 UntypedPtrAccessChainKHR
  {IR(31326, 19), 353}, // 800 UntypedVariableKHR
  {IR(26224, 9), 52}, // 801 Variable
  {IR(12292, 25), 682}, // 802 VariableLengthArrayINTEL
  {IR(26581, 21), 69}, // 803 VectorExtractDynamic
  {IR(26602, 20), 70}, // 804 VectorInsertDynamic
  {IR(26622, 14), 71}, // 805 VectorShuffle
  {IR(27504, 18), 133}, // 806 VectorTimesMatrix
  {IR(27468, 18), 131}, // 807 VectorTimesScalar
  {IR(36657, 14), 564}, // 808 VmeImageINTEL
  {IR(34187, 33), 466}, // 809 WritePackedPrimitiveIndices4x8NV
  {IR(29103, 10), 250}, // 810 WritePipe
  {IR(43190, 23), 744}, // 811 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 425> kExtInstByValue{{
    {0,IR(0, 0),IR(44770, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(44784, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(727, 3),IR(44857, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(730, 3),IR(44906, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(733, 2),IR(44961, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(44980, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(44995, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(735, 6),IR(45011, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(45024, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(741, 9),IR(45042, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(750, 9),IR(45094, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(759, 10),IR(45113, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(769, 5),IR(45129, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(45150, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(45171, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(774, 6),IR(45189, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(780, 5),IR(45216, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(785, 5),IR(45251, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(759, 10),IR(45282, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(790, 8),IR(45302, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(798, 11),IR(45327, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(809, 5),IR(45341, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(814, 3),IR(45359, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(817, 2),IR(45390, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(45401, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(819, 3),IR(45414, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(822, 7),IR(45471, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(45490, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(45511, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(829, 3),IR(45524, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(832, 2),IR(45568, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(683, 1),IR(45583, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(834, 4),IR(45599, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(814, 3),IR(45613, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(45629, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(45635, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(45645, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(45651, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(45656, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(45661, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(45667, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(45673, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(45679, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(45684, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(45690, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(45698, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(45706, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(45710, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(45714, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(45718, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(45723, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(45728, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(45733, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(45738, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(45743, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(45748, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(45754, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(45760, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(45766, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(45772, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(45776, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(45780, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(45784, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(45789, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(45794, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(45799, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(45811, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(45823, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(45837, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(45842, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(45853, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(45858, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(45863, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(45868, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(45873, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(45878, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(45883, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(45890, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(45897, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(45904, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(45909, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(45914, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(45919, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(45930, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(45934, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(45940, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(45952, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(45958, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(45971, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(45984, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(45998, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(46012, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(46025, 15),IR(301, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(46040, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(46056, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(46072, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(46087, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(46102, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(46117, 17),IR(301, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(46134, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(46141, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(46150, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(46156, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(46166, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(46178, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(46186, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(46194, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(46203, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(46212, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(46221, 22),IR(302, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(46243, 20),IR(302, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(46263, 20),IR(302, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(46283, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(46288, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(46293, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(387, 5),IR(4534, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(838, 5),IR(46300, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(843, 5),IR(46313, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(843, 5),IR(46335, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(531, 7),IR(46351, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(531, 7),IR(46376, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(843, 5),IR(46395, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(843, 5),IR(46419, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(843, 5),IR(46440, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(843, 5),IR(46461, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(843, 5),IR(46477, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(46495, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(46521, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(46546, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(46566, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(46591, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(46621, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(46644, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(46669, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(46695, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(46725, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(46751, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(7959, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(46771, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(46801, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(843, 5),IR(46829, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(531, 7),IR(46857, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(46880, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(46915, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(46953, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(46995, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(502, 6),IR(47040, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(502, 6),IR(47077, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(843, 5),IR(47117, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(843, 5),IR(47144, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(47171, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(47203, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(829, 3),IR(47243, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(47254, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(47280, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(47312, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(47346, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(44770, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(44784, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(44857, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(44906, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(44961, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(44980, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(44995, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(502, 6),IR(45011, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(829, 3),IR(45024, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(848, 9),IR(45042, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(857, 10),IR(45094, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(867, 9),IR(45113, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(45129, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(45150, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(45171, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(502, 6),IR(45189, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(611, 5),IR(45216, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(876, 5),IR(45251, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(881, 10),IR(45282, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(448, 8),IR(45302, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(881, 10),IR(45327, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(843, 5),IR(45341, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(45359, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(817, 2),IR(45390, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(45401, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(891, 3),IR(45414, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(894, 8),IR(45471, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(45490, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(902, 4),IR(45511, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(902, 4),IR(45524, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(45568, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(683, 1),IR(45583, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(554, 4),IR(45599, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(45613, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(483, 7),IR(47368, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(817, 2),IR(47388, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(47400, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(47424, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(611, 5),IR(47445, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(47455, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(47467, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(47488, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(47505, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(47521, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(701, 9),IR(47537, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(47551, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(47564, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(47576, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(47590, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(906, 15),IR(47613, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(921, 33),IR(47633, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(954, 19),IR(47652, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(44770, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(973, 4),IR(44784, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(977, 3),IR(44857, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(980, 3),IR(44906, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(983, 2),IR(44961, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(44980, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(44995, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(735, 6),IR(45011, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(985, 3),IR(45024, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(988, 9),IR(45042, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(997, 10),IR(45094, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1007, 10),IR(45113, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1017, 5),IR(45129, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(45150, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(45171, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(774, 6),IR(45189, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(780, 5),IR(45216, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(785, 5),IR(45251, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1007, 10),IR(45282, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1022, 8),IR(45302, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1030, 11),IR(45327, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(809, 5),IR(45341, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(814, 3),IR(45359, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(817, 2),IR(45390, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(45401, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(819, 3),IR(45414, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1041, 8),IR(45471, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(45490, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(45511, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(902, 4),IR(45524, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1049, 2),IR(45568, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(683, 1),IR(45583, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(834, 4),IR(45599, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(814, 3),IR(45613, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1051, 7),IR(47368, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(817, 2),IR(47388, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1058, 8),IR(47973, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(47990, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(47995, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(48001, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(48008, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(48013, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(48019, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(48026, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(48031, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(48037, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(48043, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(48050, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(48058, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(48063, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(48068, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(48077, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(48081, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(48086, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(48092, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(48097, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(48101, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(48105, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(48110, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(48116, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(48122, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(48127, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(48132, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(48138, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(48142, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(48147, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(48152, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(48157, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(48163, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(48169, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(48175, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(48181, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(48187, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(48194, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(48203, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(48207, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(48212, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(48218, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(48224, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(48229, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(48233, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(48240, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(48247, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(48252, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(48256, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(48266, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(48270, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(48275, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(48280, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(48290, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(48297, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(48302, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(48308, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(48314, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(48320, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(48324, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(48331, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(48336, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(48342, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(48347, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(48351, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(48356, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(48362, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(48369, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(48375, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(48384, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(48396, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(48405, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(48415, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(48426, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(48435, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(48445, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(48456, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(48466, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(48477, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(48488, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(48497, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(48507, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(48516, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(48527, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(48541, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(48552, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(48564, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(48577, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(48588, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(48600, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(48613, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(48625, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(48638, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(48651, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(48662, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(48674, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(48685, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(48692, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(48700, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(48712, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(48724, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(48728, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(48736, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(48741, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(48752, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(48757, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(48763, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(48772, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(48779, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(48789, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(48803, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(48815, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(48830, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(48836, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(48847, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(48857, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(48867, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(48874, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(48881, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(48889, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(48897, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(48905, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(48913, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(48917, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(48921, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(48930, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(48940, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(48950, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(48956, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(48962, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(48968, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(48974, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(48983, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(48990, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(49000, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(49010, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(49021, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(49032, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(49041, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(49049, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(49057, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(49065, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1066, 3),IR(49073, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(49080, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(49088, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1066, 3),IR(49099, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(49111, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1069, 4),IR(49123, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(49137, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1069, 4),IR(49150, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1066, 3),IR(49165, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(49178, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1069, 4),IR(49192, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(49208, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(49216, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(49225, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(49232, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(49241, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(49251, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(49258, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(49264, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(49275, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(49284, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(49293, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(49310, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(49327, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(49335, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(49357, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(49385, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(49404, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(49413, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(49436, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(49445, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(49454, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(49463, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(49472, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(49481, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(49490, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(49499, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(49508, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
}};

IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(0, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(34, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(115, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(157, 44);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(201, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(209, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(246, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(408, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(411, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(415, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(416, 9);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 425> kExtInstNames{{
    {IR(44784, 21), 1}, // DebugCompilationUnit in debuginfo
    {IR(45511, 13), 28}, // DebugDeclare in debuginfo
    {IR(45583, 16), 31}, // DebugExpression in debuginfo
    {IR(45327, 14), 20}, // DebugFunction in debuginfo
    {IR(45302, 25), 19}, // DebugFunctionDeclaration in debuginfo
    {IR(45282, 20), 18}, // DebugGlobalVariable in debuginfo
    {IR(44770, 14), 0}, // DebugInfoNone in debuginfo
    {IR(45414, 15), 25}, // DebugInlinedAt in debuginfo
    {IR(45490, 21), 27}, // DebugInlinedVariable in debuginfo
    {IR(45341, 18), 21}, // DebugLexicalBlock in debuginfo
    {IR(45359, 31), 22}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(45471, 19), 26}, // DebugLocalVariable in debuginfo
    {IR(45599, 14), 32}, // DebugMacroDef in debuginfo
    {IR(45613, 16), 33}, // DebugMacroUndef in debuginfo
    {IR(45401, 13), 24}, // DebugNoScope in debuginfo
    {IR(45568, 15), 30}, // DebugOperation in debuginfo
    {IR(45390, 11), 23}, // DebugScope in debuginfo
    {IR(44980, 15), 5}, // DebugTypeArray in debuginfo
    {IR(44857, 15), 2}, // DebugTypeBasic in debuginfo
    {IR(45094, 19), 10}, // DebugTypeComposite in debuginfo
    {IR(45042, 14), 9}, // DebugTypeEnum in debuginfo
    {IR(45024, 18), 8}, // DebugTypeFunction in debuginfo
    {IR(45129, 21), 12}, // DebugTypeInheritance in debuginfo
    {IR(45113, 16), 11}, // DebugTypeMember in debuginfo
    {IR(44906, 17), 3}, // DebugTypePointer in debuginfo
    {IR(45150, 21), 13}, // DebugTypePtrToMember in debuginfo
    {IR(44961, 19), 4}, // DebugTypeQualifier in debuginfo
    {IR(45171, 18), 14}, // DebugTypeTemplate in debuginfo
    {IR(45189, 27), 15}, // DebugTypeTemplateParameter in debuginfo
    {IR(45251, 31), 17}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(45216, 35), 16}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(44995, 16), 6}, // DebugTypeVector in debuginfo
    {IR(45011, 13), 7}, // DebugTypedef in debuginfo
    {IR(45524, 11), 29}, // DebugValue in debuginfo
    {IR(45723, 5), 50}, // Acos in glsl_std_450
    {IR(45754, 6), 56}, // Acosh in glsl_std_450
    {IR(45718, 5), 49}, // Asin in glsl_std_450
    {IR(45748, 6), 55}, // Asinh in glsl_std_450
    {IR(45728, 5), 51}, // Atan in glsl_std_450
    {IR(45766, 6), 58}, // Atan2 in glsl_std_450
    {IR(45760, 6), 57}, // Atanh in glsl_std_450
    {IR(45679, 5), 42}, // Ceil in glsl_std_450
    {IR(45710, 4), 47}, // Cos in glsl_std_450
    {IR(45738, 5), 53}, // Cosh in glsl_std_450
    {IR(46150, 6), 101}, // Cross in glsl_std_450
    {IR(45698, 8), 45}, // Degrees in glsl_std_450
    {IR(45811, 12), 66}, // Determinant in glsl_std_450
    {IR(46141, 9), 100}, // Distance in glsl_std_450
    {IR(45776, 4), 60}, // Exp in glsl_std_450
    {IR(45784, 5), 62}, // Exp2 in glsl_std_450
    {IR(45651, 5), 37}, // FAbs in glsl_std_450
    {IR(45883, 7), 76}, // FClamp in glsl_std_450
    {IR(45868, 5), 73}, // FMax in glsl_std_450
    {IR(45853, 5), 70}, // FMin in glsl_std_450
    {IR(45904, 5), 79}, // FMix in glsl_std_450
    {IR(45661, 6), 39}, // FSign in glsl_std_450
    {IR(46166, 12), 103}, // FaceForward in glsl_std_450
    {IR(46194, 9), 106}, // FindILsb in glsl_std_450
    {IR(46203, 9), 107}, // FindSMsb in glsl_std_450
    {IR(46212, 9), 108}, // FindUMsb in glsl_std_450
    {IR(45673, 6), 41}, // Floor in glsl_std_450
    {IR(45930, 4), 83}, // Fma in glsl_std_450
    {IR(45684, 6), 43}, // Fract in glsl_std_450
    {IR(45934, 6), 84}, // Frexp in glsl_std_450
    {IR(45940, 12), 85}, // FrexpStruct in glsl_std_450
    {IR(45909, 5), 80}, // IMix in glsl_std_450
    {IR(46221, 22), 109}, // InterpolateAtCentroid in glsl_std_450
    {IR(46263, 20), 111}, // InterpolateAtOffset in glsl_std_450
    {IR(46243, 20), 110}, // InterpolateAtSample in glsl_std_450
    {IR(45799, 12), 65}, // InverseSqrt in glsl_std_450
    {IR(45952, 6), 86}, // Ldexp in glsl_std_450
    {IR(46134, 7), 99}, // Length in glsl_std_450
    {IR(45780, 4), 61}, // Log in glsl_std_450
    {IR(45789, 5), 63}, // Log2 in glsl_std_450
    {IR(45823, 14), 67}, // MatrixInverse in glsl_std_450
    {IR(45837, 5), 68}, // Modf in glsl_std_450
    {IR(45842, 11), 69}, // ModfStruct in glsl_std_450
    {IR(46293, 7), 114}, // NClamp in glsl_std_450
    {IR(46288, 5), 113}, // NMax in glsl_std_450
    {IR(46283, 5), 112}, // NMin in glsl_std_450
    {IR(46156, 10), 102}, // Normalize in glsl_std_450
    {IR(46025, 15), 92}, // PackDouble2x32 in glsl_std_450
    {IR(46012, 13), 91}, // PackHalf2x16 in glsl_std_450
    {IR(45984, 14), 89}, // PackSnorm2x16 in glsl_std_450
    {IR(45958, 13), 87}, // PackSnorm4x8 in glsl_std_450
    {IR(45998, 14), 90}, // PackUnorm2x16 in glsl_std_450
    {IR(45971, 13), 88}, // PackUnorm4x8 in glsl_std_450
    {IR(45772, 4), 59}, // Pow in glsl_std_450
    {IR(45690, 8), 44}, // Radians in glsl_std_450
    {IR(46178, 8), 104}, // Reflect in glsl_std_450
    {IR(46186, 8), 105}, // Refract in glsl_std_450
    {IR(45629, 6), 34}, // Round in glsl_std_450
    {IR(45635, 10), 35}, // RoundEven in glsl_std_450
    {IR(45656, 5), 38}, // SAbs in glsl_std_450
    {IR(45897, 7), 78}, // SClamp in glsl_std_450
    {IR(45878, 5), 75}, // SMax in glsl_std_450
    {IR(45863, 5), 72}, // SMin in glsl_std_450
    {IR(45667, 6), 40}, // SSign in glsl_std_450
    {IR(45706, 4), 46}, // Sin in glsl_std_450
    {IR(45733, 5), 52}, // Sinh in glsl_std_450
    {IR(45919, 11), 82}, // SmoothStep in glsl_std_450
    {IR(45794, 5), 64}, // Sqrt in glsl_std_450
    {IR(45914, 5), 81}, // Step in glsl_std_450
    {IR(45714, 4), 48}, // Tan in glsl_std_450
    {IR(45743, 5), 54}, // Tanh in glsl_std_450
    {IR(45645, 6), 36}, // Trunc in glsl_std_450
    {IR(45890, 7), 77}, // UClamp in glsl_std_450
    {IR(45873, 5), 74}, // UMax in glsl_std_450
    {IR(45858, 5), 71}, // UMin in glsl_std_450
    {IR(46117, 17), 98}, // UnpackDouble2x32 in glsl_std_450
    {IR(46072, 15), 95}, // UnpackHalf2x16 in glsl_std_450
    {IR(46040, 16), 93}, // UnpackSnorm2x16 in glsl_std_450
    {IR(46087, 15), 96}, // UnpackSnorm4x8 in glsl_std_450
    {IR(46056, 16), 94}, // UnpackUnorm2x16 in glsl_std_450
    {IR(46102, 15), 97}, // UnpackUnorm4x8 in glsl_std_450
    {IR(46300, 13), 116}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(46395, 24), 121}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(46351, 25), 119}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(46376, 19), 120}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(46829, 28), 140}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(46857, 23), 141}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(46419, 21), 122}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(46461, 16), 124}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(46313, 22), 117}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(46440, 21), 123}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(47117, 27), 148}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(46335, 16), 118}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(47144, 27), 149}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(46477, 18), 125}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(47171, 32), 150}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(46725, 26), 135}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(46751, 20), 136}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(46995, 45), 145}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(47077, 40), 147}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(46953, 42), 144}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(47040, 37), 146}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(4534, 7), 115}, // Kernel in nonsemantic_clspvreflection
    {IR(7959, 15), 137}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(47312, 34), 155}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(47280, 32), 154}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(47254, 26), 153}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(47243, 11), 152}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(47203, 40), 151}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(46915, 38), 143}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(46880, 35), 142}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(46771, 30), 138}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(46591, 30), 130}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(46566, 25), 129}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(46621, 23), 131}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(46669, 26), 133}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(46695, 30), 134}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(46644, 25), 132}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(46521, 25), 127}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(46801, 28), 139}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(46546, 20), 128}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(46495, 26), 126}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(47346, 22), 156}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(47467, 21), 197}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(44784, 21), 158}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(45511, 13), 185}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(47505, 16), 199}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(45583, 16), 188}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(45327, 14), 177}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(45302, 25), 176}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(47400, 24), 193}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(45282, 20), 175}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(47368, 20), 191}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(44770, 14), 157}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(45414, 15), 182}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(45490, 21), 184}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(45341, 18), 178}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(45359, 31), 179}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(47445, 10), 195}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(45471, 19), 183}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(45599, 14), 189}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(45613, 16), 190}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(47455, 12), 196}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(45401, 13), 181}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(45568, 15), 187}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(45390, 11), 180}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(47388, 12), 192}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(47424, 21), 194}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(47488, 17), 198}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(44980, 15), 162}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(44857, 15), 159}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(45094, 19), 167}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(45042, 14), 166}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(45024, 18), 165}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(45129, 21), 169}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(47521, 16), 200}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(45113, 16), 168}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(44906, 17), 160}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(45150, 21), 170}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(44961, 19), 161}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(45171, 18), 171}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(45189, 27), 172}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45251, 31), 174}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(45216, 35), 173}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(44995, 16), 163}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(45011, 13), 164}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(45524, 11), 186}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(47537, 14), 201}, // Configuration in nonsemantic_vkspreflection
    {IR(47613, 20), 206}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(47633, 19), 207}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(47652, 21), 208}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(47576, 14), 204}, // PushConstants in nonsemantic_vkspreflection
    {IR(47590, 23), 205}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(47551, 13), 202}, // StartCounter in nonsemantic_vkspreflection
    {IR(47564, 12), 203}, // StopCounter in nonsemantic_vkspreflection
    {IR(44784, 21), 210}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(45511, 13), 237}, // DebugDeclare in opencl_debuginfo_100
    {IR(45583, 16), 240}, // DebugExpression in opencl_debuginfo_100
    {IR(45327, 14), 229}, // DebugFunction in opencl_debuginfo_100
    {IR(45302, 25), 228}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(45282, 20), 227}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(47368, 20), 243}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(44770, 14), 209}, // DebugInfoNone in opencl_debuginfo_100
    {IR(45414, 15), 234}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(45490, 21), 236}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(45341, 18), 230}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(45359, 31), 231}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(45471, 19), 235}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(45599, 14), 241}, // DebugMacroDef in opencl_debuginfo_100
    {IR(45613, 16), 242}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(47973, 17), 245}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(45401, 13), 233}, // DebugNoScope in opencl_debuginfo_100
    {IR(45568, 15), 239}, // DebugOperation in opencl_debuginfo_100
    {IR(45390, 11), 232}, // DebugScope in opencl_debuginfo_100
    {IR(47388, 12), 244}, // DebugSource in opencl_debuginfo_100
    {IR(44980, 15), 214}, // DebugTypeArray in opencl_debuginfo_100
    {IR(44857, 15), 211}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(45094, 19), 219}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(45042, 14), 218}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(45024, 18), 217}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(45129, 21), 221}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(45113, 16), 220}, // DebugTypeMember in opencl_debuginfo_100
    {IR(44906, 17), 212}, // DebugTypePointer in opencl_debuginfo_100
    {IR(45150, 21), 222}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(44961, 19), 213}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(45171, 18), 223}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(45189, 27), 224}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(45251, 31), 226}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(45216, 35), 225}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(44995, 16), 215}, // DebugTypeVector in opencl_debuginfo_100
    {IR(45011, 13), 216}, // DebugTypedef in opencl_debuginfo_100
    {IR(45524, 11), 238}, // DebugValue in opencl_debuginfo_100
    {IR(47990, 5), 246}, // acos in opencl_std_100
    {IR(47995, 6), 247}, // acosh in opencl_std_100
    {IR(48001, 7), 248}, // acospi in opencl_std_100
    {IR(48008, 5), 249}, // asin in opencl_std_100
    {IR(48013, 6), 250}, // asinh in opencl_std_100
    {IR(48019, 7), 251}, // asinpi in opencl_std_100
    {IR(48026, 5), 252}, // atan in opencl_std_100
    {IR(48031, 6), 253}, // atan2 in opencl_std_100
    {IR(48050, 8), 256}, // atan2pi in opencl_std_100
    {IR(48037, 6), 254}, // atanh in opencl_std_100
    {IR(48043, 7), 255}, // atanpi in opencl_std_100
    {IR(49241, 10), 402}, // bitselect in opencl_std_100
    {IR(48058, 5), 257}, // cbrt in opencl_std_100
    {IR(48063, 5), 258}, // ceil in opencl_std_100
    {IR(48913, 4), 367}, // clz in opencl_std_100
    {IR(48068, 9), 259}, // copysign in opencl_std_100
    {IR(48077, 4), 260}, // cos in opencl_std_100
    {IR(48081, 5), 261}, // cosh in opencl_std_100
    {IR(48086, 6), 262}, // cospi in opencl_std_100
    {IR(48757, 6), 350}, // cross in opencl_std_100
    {IR(48917, 4), 368}, // ctz in opencl_std_100
    {IR(48692, 8), 342}, // degrees in opencl_std_100
    {IR(48763, 9), 351}, // distance in opencl_std_100
    {IR(48097, 4), 264}, // erf in opencl_std_100
    {IR(48092, 5), 263}, // erfc in opencl_std_100
    {IR(48101, 4), 265}, // exp in opencl_std_100
    {IR(48110, 6), 267}, // exp10 in opencl_std_100
    {IR(48105, 5), 266}, // exp2 in opencl_std_100
    {IR(48116, 6), 268}, // expm1 in opencl_std_100
    {IR(48122, 5), 269}, // fabs in opencl_std_100
    {IR(48789, 14), 354}, // fast_distance in opencl_std_100
    {IR(48803, 12), 355}, // fast_length in opencl_std_100
    {IR(48815, 15), 356}, // fast_normalize in opencl_std_100
    {IR(48685, 7), 341}, // fclamp in opencl_std_100
    {IR(48127, 5), 270}, // fdim in opencl_std_100
    {IR(48132, 6), 271}, // floor in opencl_std_100
    {IR(48138, 4), 272}, // fma in opencl_std_100
    {IR(48142, 5), 273}, // fmax in opencl_std_100
    {IR(48700, 12), 343}, // fmax_common in opencl_std_100
    {IR(48147, 5), 274}, // fmin in opencl_std_100
    {IR(48712, 12), 344}, // fmin_common in opencl_std_100
    {IR(48152, 5), 275}, // fmod in opencl_std_100
    {IR(48157, 6), 276}, // fract in opencl_std_100
    {IR(48163, 6), 277}, // frexp in opencl_std_100
    {IR(48375, 9), 313}, // half_cos in opencl_std_100
    {IR(48384, 12), 314}, // half_divide in opencl_std_100
    {IR(48396, 9), 315}, // half_exp in opencl_std_100
    {IR(48415, 11), 317}, // half_exp10 in opencl_std_100
    {IR(48405, 10), 316}, // half_exp2 in opencl_std_100
    {IR(48426, 9), 318}, // half_log in opencl_std_100
    {IR(48445, 11), 320}, // half_log10 in opencl_std_100
    {IR(48435, 10), 319}, // half_log2 in opencl_std_100
    {IR(48456, 10), 321}, // half_powr in opencl_std_100
    {IR(48466, 11), 322}, // half_recip in opencl_std_100
    {IR(48477, 11), 323}, // half_rsqrt in opencl_std_100
    {IR(48488, 9), 324}, // half_sin in opencl_std_100
    {IR(48497, 10), 325}, // half_sqrt in opencl_std_100
    {IR(48507, 9), 326}, // half_tan in opencl_std_100
    {IR(48169, 6), 278}, // hypot in opencl_std_100
    {IR(48175, 6), 279}, // ilogb in opencl_std_100
    {IR(48181, 6), 280}, // ldexp in opencl_std_100
    {IR(48772, 7), 352}, // length in opencl_std_100
    {IR(48187, 7), 281}, // lgamma in opencl_std_100
    {IR(48194, 9), 282}, // lgamma_r in opencl_std_100
    {IR(48203, 4), 283}, // log in opencl_std_100
    {IR(48212, 6), 285}, // log10 in opencl_std_100
    {IR(48218, 6), 286}, // log1p in opencl_std_100
    {IR(48207, 5), 284}, // log2 in opencl_std_100
    {IR(48224, 5), 287}, // logb in opencl_std_100
    {IR(48229, 4), 288}, // mad in opencl_std_100
    {IR(48233, 7), 289}, // maxmag in opencl_std_100
    {IR(48240, 7), 290}, // minmag in opencl_std_100
    {IR(48724, 4), 345}, // mix in opencl_std_100
    {IR(48247, 5), 291}, // modf in opencl_std_100
    {IR(48252, 4), 292}, // nan in opencl_std_100
    {IR(48516, 11), 327}, // native_cos in opencl_std_100
    {IR(48527, 14), 328}, // native_divide in opencl_std_100
    {IR(48541, 11), 329}, // native_exp in opencl_std_100
    {IR(48564, 13), 331}, // native_exp10 in opencl_std_100
    {IR(48552, 12), 330}, // native_exp2 in opencl_std_100
    {IR(48577, 11), 332}, // native_log in opencl_std_100
    {IR(48600, 13), 334}, // native_log10 in opencl_std_100
    {IR(48588, 12), 333}, // native_log2 in opencl_std_100
    {IR(48613, 12), 335}, // native_powr in opencl_std_100
    {IR(48625, 13), 336}, // native_recip in opencl_std_100
    {IR(48638, 13), 337}, // native_rsqrt in opencl_std_100
    {IR(48651, 11), 338}, // native_sin in opencl_std_100
    {IR(48662, 12), 339}, // native_sqrt in opencl_std_100
    {IR(48674, 11), 340}, // native_tan in opencl_std_100
    {IR(48256, 10), 293}, // nextafter in opencl_std_100
    {IR(48779, 10), 353}, // normalize in opencl_std_100
    {IR(49032, 9), 382}, // popcount in opencl_std_100
    {IR(48266, 4), 294}, // pow in opencl_std_100
    {IR(48270, 5), 295}, // pown in opencl_std_100
    {IR(48275, 5), 296}, // powr in opencl_std_100
    {IR(49232, 9), 401}, // prefetch in opencl_std_100
    {IR(49225, 7), 400}, // printf in opencl_std_100
    {IR(48728, 8), 346}, // radians in opencl_std_100
    {IR(48280, 10), 297}, // remainder in opencl_std_100
    {IR(48290, 7), 298}, // remquo in opencl_std_100
    {IR(48297, 5), 299}, // rint in opencl_std_100
    {IR(48302, 6), 300}, // rootn in opencl_std_100
    {IR(48983, 7), 377}, // rotate in opencl_std_100
    {IR(48308, 6), 301}, // round in opencl_std_100
    {IR(48314, 6), 302}, // rsqrt in opencl_std_100
    {IR(48830, 6), 357}, // s_abs in opencl_std_100
    {IR(48836, 11), 358}, // s_abs_diff in opencl_std_100
    {IR(48847, 10), 359}, // s_add_sat in opencl_std_100
    {IR(48897, 8), 365}, // s_clamp in opencl_std_100
    {IR(48867, 7), 361}, // s_hadd in opencl_std_100
    {IR(49041, 8), 383}, // s_mad24 in opencl_std_100
    {IR(48921, 9), 369}, // s_mad_hi in opencl_std_100
    {IR(48940, 10), 371}, // s_mad_sat in opencl_std_100
    {IR(48950, 6), 372}, // s_max in opencl_std_100
    {IR(48962, 6), 374}, // s_min in opencl_std_100
    {IR(49057, 8), 385}, // s_mul24 in opencl_std_100
    {IR(48974, 9), 376}, // s_mul_hi in opencl_std_100
    {IR(48881, 8), 363}, // s_rhadd in opencl_std_100
    {IR(48990, 10), 378}, // s_sub_sat in opencl_std_100
    {IR(49021, 11), 381}, // s_upsample in opencl_std_100
    {IR(49251, 7), 403}, // select in opencl_std_100
    {IR(49208, 8), 398}, // shuffle in opencl_std_100
    {IR(49216, 9), 399}, // shuffle2 in opencl_std_100
    {IR(48752, 5), 349}, // sign in opencl_std_100
    {IR(48320, 4), 303}, // sin in opencl_std_100
    {IR(48324, 7), 304}, // sincos in opencl_std_100
    {IR(48331, 5), 305}, // sinh in opencl_std_100
    {IR(48336, 6), 306}, // sinpi in opencl_std_100
    {IR(48741, 11), 348}, // smoothstep in opencl_std_100
    {IR(48342, 5), 307}, // sqrt in opencl_std_100
    {IR(48736, 5), 347}, // step in opencl_std_100
    {IR(48347, 4), 308}, // tan in opencl_std_100
    {IR(48351, 5), 309}, // tanh in opencl_std_100
    {IR(48356, 6), 310}, // tanpi in opencl_std_100
    {IR(48362, 7), 311}, // tgamma in opencl_std_100
    {IR(48369, 6), 312}, // trunc in opencl_std_100
    {IR(49258, 6), 404}, // u_abs in opencl_std_100
    {IR(49264, 11), 405}, // u_abs_diff in opencl_std_100
    {IR(48857, 10), 360}, // u_add_sat in opencl_std_100
    {IR(48905, 8), 366}, // u_clamp in opencl_std_100
    {IR(48874, 7), 362}, // u_hadd in opencl_std_100
    {IR(49049, 8), 384}, // u_mad24 in opencl_std_100
    {IR(49284, 9), 407}, // u_mad_hi in opencl_std_100
    {IR(48930, 10), 370}, // u_mad_sat in opencl_std_100
    {IR(48956, 6), 373}, // u_max in opencl_std_100
    {IR(48968, 6), 375}, // u_min in opencl_std_100
    {IR(49065, 8), 386}, // u_mul24 in opencl_std_100
    {IR(49275, 9), 406}, // u_mul_hi in opencl_std_100
    {IR(48889, 8), 364}, // u_rhadd in opencl_std_100
    {IR(49000, 10), 379}, // u_sub_sat in opencl_std_100
    {IR(49010, 11), 380}, // u_upsample in opencl_std_100
    {IR(49088, 11), 389}, // vload_half in opencl_std_100
    {IR(49099, 12), 390}, // vload_halfn in opencl_std_100
    {IR(49165, 13), 395}, // vloada_halfn in opencl_std_100
    {IR(49073, 7), 387}, // vloadn in opencl_std_100
    {IR(49111, 12), 391}, // vstore_half in opencl_std_100
    {IR(49123, 14), 392}, // vstore_half_r in opencl_std_100
    {IR(49137, 13), 393}, // vstore_halfn in opencl_std_100
    {IR(49150, 15), 394}, // vstore_halfn_r in opencl_std_100
    {IR(49178, 14), 396}, // vstorea_halfn in opencl_std_100
    {IR(49192, 16), 397}, // vstorea_halfn_r in opencl_std_100
    {IR(49080, 8), 388}, // vstoren in opencl_std_100
    {IR(49310, 17), 409}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(49293, 17), 408}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(49327, 8), 410}, // TimeAMD in spv_amd_gcn_shader
    {IR(49404, 9), 414}, // MbcntAMD in spv_amd_shader_ballot
    {IR(49335, 22), 411}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(49357, 28), 412}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(49385, 19), 413}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(49413, 23), 415}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(49463, 9), 419}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49490, 9), 422}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49436, 9), 416}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(49481, 9), 421}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49508, 9), 424}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49454, 9), 418}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(49472, 9), 420}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49499, 9), 423}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49445, 9), 417}, // UMin3AMD in spv_amd_shader_trinary_minmax
}};

IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(0, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(34, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(115, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(157, 44);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(201, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(209, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(246, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(408, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(411, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(415, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(416, 9);
    default: break;
  }
  return IR(0,0);
}

// Array of characters, referenced by IndexRanges elsewhere.
// Each IndexRange denotes a string.
static const char kStrings[] =
  "SPV_AMDX_shader_enqueue\0" // IR(0, 24)
  "SPV_AMD_gcn_shader\0" // IR(24, 19)
  "SPV_AMD_gpu_shader_half_float\0" // IR(43, 30)
  "SPV_AMD_gpu_shader_half_float_fetch\0" // IR(73, 36)
  "SPV_AMD_gpu_shader_int16\0" // IR(109, 25)
  "SPV_AMD_shader_ballot\0" // IR(134, 22)
  "SPV_AMD_shader_early_and_late_fragment_tests\0" // IR(156, 45)
  "SPV_AMD_shader_explicit_vertex_parameter\0" // IR(201, 41)
  "SPV_AMD_shader_fragment_mask\0" // IR(242, 29)
  "SPV_AMD_shader_image_load_store_lod\0" // IR(271, 36)
  "SPV_AMD_shader_trinary_minmax\0" // IR(307, 30)
  "SPV_AMD_texture_gather_bias_lod\0" // IR(337, 32)
  "SPV_ARM_cooperative_matrix_layouts\0" // IR(369, 35)
  "SPV_ARM_core_builtins\0" // IR(404, 22)
  "SPV_ARM_tensors\0" // IR(426, 16)
  "SPV_EXT_arithmetic_fence\0" // IR(442, 25)
  "SPV_EXT_demote_to_helper_invocation\0" // IR(467, 36)
  "SPV_EXT_descriptor_indexing\0" // IR(503, 28)
  "SPV_EXT_fragment_fully_covered\0" // IR(531, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(562, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(598, 34)
  "SPV_EXT_mesh_shader\0" // IR(632, 20)
  "SPV_EXT_opacity_micromap\0" // IR(652, 25)
  "SPV_EXT_optnone\0" // IR(677, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(693, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(725, 44)
  "SPV_EXT_replicated_composites\0" // IR(769, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(799, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(833, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(865, 36)
  "SPV_EXT_shader_image_int64\0" // IR(901, 27)
  "SPV_EXT_shader_stencil_export\0" // IR(928, 30)
  "SPV_EXT_shader_tile_image\0" // IR(958, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(984, 36)
  "SPV_GOOGLE_decorate_string\0" // IR(1020, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1047, 31)
  "SPV_GOOGLE_user_type\0" // IR(1078, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1099, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(1121, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(1163, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(1208, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(1247, 30)
  "SPV_INTEL_bindless_images\0" // IR(1277, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(1303, 25)
  "SPV_INTEL_cache_controls\0" // IR(1328, 25)
  "SPV_INTEL_debug_module\0" // IR(1353, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(1376, 44)
  "SPV_INTEL_float_controls2\0" // IR(1420, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(1446, 28)
  "SPV_INTEL_fp_max_error\0" // IR(1474, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(1497, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(1532, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(1563, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(1597, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(1624, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(1672, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(1703, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(1732, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(1763, 33)
  "SPV_INTEL_fpga_reg\0" // IR(1796, 19)
  "SPV_INTEL_function_pointers\0" // IR(1815, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(1843, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(1886, 38)
  "SPV_INTEL_inline_assembly\0" // IR(1924, 26)
  "SPV_INTEL_int4\0" // IR(1950, 15)
  "SPV_INTEL_io_pipes\0" // IR(1965, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(1984, 28)
  "SPV_INTEL_long_composites\0" // IR(2012, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2038, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2058, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2090, 28)
  "SPV_INTEL_media_block_io\0" // IR(2118, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2143, 33)
  "SPV_INTEL_optnone\0" // IR(2176, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(2194, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(2220, 36)
  "SPV_INTEL_split_barrier\0" // IR(2256, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(2280, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(2315, 46)
  "SPV_INTEL_subgroups\0" // IR(2361, 20)
  "SPV_INTEL_task_sequence\0" // IR(2381, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(2405, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(2441, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(2476, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(2513, 30)
  "SPV_INTEL_variable_length_array\0" // IR(2543, 32)
  "SPV_INTEL_vector_compute\0" // IR(2575, 25)
  "SPV_KHR_16bit_storage\0" // IR(2600, 22)
  "SPV_KHR_8bit_storage\0" // IR(2622, 21)
  "SPV_KHR_bfloat16\0" // IR(2643, 17)
  "SPV_KHR_bit_instructions\0" // IR(2660, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(2685, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(2720, 27)
  "SPV_KHR_device_group\0" // IR(2747, 21)
  "SPV_KHR_expect_assume\0" // IR(2768, 22)
  "SPV_KHR_float_controls\0" // IR(2790, 23)
  "SPV_KHR_float_controls2\0" // IR(2813, 24)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(2837, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(2873, 30)
  "SPV_KHR_integer_dot_product\0" // IR(2903, 28)
  "SPV_KHR_linkonce_odr\0" // IR(2931, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(2952, 30)
  "SPV_KHR_multiview\0" // IR(2982, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3000, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3035, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3061, 32)
  "SPV_KHR_post_depth_coverage\0" // IR(3093, 28)
  "SPV_KHR_quad_control\0" // IR(3121, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(3142, 22)
  "SPV_KHR_ray_query\0" // IR(3164, 18)
  "SPV_KHR_ray_tracing\0" // IR(3182, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(3202, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(3237, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(3274, 34)
  "SPV_KHR_shader_ballot\0" // IR(3308, 22)
  "SPV_KHR_shader_clock\0" // IR(3330, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(3351, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(3382, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(3419, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(3443, 38)
  "SPV_KHR_subgroup_vote\0" // IR(3481, 22)
  "SPV_KHR_terminate_invocation\0" // IR(3503, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(3532, 35)
  "SPV_KHR_untyped_pointers\0" // IR(3567, 25)
  "SPV_KHR_variable_pointers\0" // IR(3592, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(3618, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(3646, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(3687, 38)
  "SPV_NV_bindless_texture\0" // IR(3725, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(3749, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(3787, 34)
  "SPV_NV_cooperative_matrix\0" // IR(3821, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(3847, 27)
  "SPV_NV_cooperative_vector\0" // IR(3874, 26)
  "SPV_NV_displacement_micromap\0" // IR(3900, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(3929, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(3964, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(3999, 28)
  "SPV_NV_mesh_shader\0" // IR(4027, 19)
  "SPV_NV_raw_access_chains\0" // IR(4046, 25)
  "SPV_NV_ray_tracing\0" // IR(4071, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(4090, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(4121, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(4158, 33)
  "SPV_NV_shader_image_footprint\0" // IR(4191, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(4221, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(4254, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(4280, 35)
  "SPV_NV_shading_rate\0" // IR(4315, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(4335, 29)
  "SPV_NV_tensor_addressing\0" // IR(4364, 25)
  "SPV_NV_viewport_array2\0" // IR(4389, 23)
  "SPV_QCOM_image_processing\0" // IR(4412, 26)
  "SPV_QCOM_image_processing2\0" // IR(4438, 27)
  "SPV_QCOM_tile_shading\0" // IR(4465, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(4487, 38)
  "ReadOnly\0" // IR(4525, 9)
  "Kernel\0" // IR(4534, 7)
  "WriteOnly\0" // IR(4541, 10)
  "ReadWrite\0" // IR(4551, 10)
  "Logical\0" // IR(4561, 8)
  "Physical32\0" // IR(4569, 11)
  "Addresses\0" // IR(4580, 10)
  "Physical64\0" // IR(4590, 11)
  "PhysicalStorageBuffer64\0" // IR(4601, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(4625, 27)
  "PhysicalStorageBufferAddresses\0" // IR(4652, 31)
  "Position\0" // IR(4683, 9)
  "Shader\0" // IR(4692, 7)
  "PointSize\0" // IR(4699, 10)
  "ClipDistance\0" // IR(4709, 13)
  "CullDistance\0" // IR(4722, 13)
  "VertexId\0" // IR(4735, 9)
  "InstanceId\0" // IR(4744, 11)
  "PrimitiveId\0" // IR(4755, 12)
  "Geometry\0" // IR(4767, 9)
  "Tessellation\0" // IR(4776, 13)
  "RayTracingNV\0" // IR(4789, 13)
  "RayTracingKHR\0" // IR(4802, 14)
  "MeshShadingNV\0" // IR(4816, 14)
  "MeshShadingEXT\0" // IR(4830, 15)
  "InvocationId\0" // IR(4845, 13)
  "Layer\0" // IR(4858, 6)
  "ShaderLayer\0" // IR(4864, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(4876, 28)
  "ViewportIndex\0" // IR(4904, 14)
  "MultiViewport\0" // IR(4918, 14)
  "ShaderViewportIndex\0" // IR(4932, 20)
  "TessLevelOuter\0" // IR(4952, 15)
  "TessLevelInner\0" // IR(4967, 15)
  "TessCoord\0" // IR(4982, 10)
  "PatchVertices\0" // IR(4992, 14)
  "FragCoord\0" // IR(5006, 10)
  "PointCoord\0" // IR(5016, 11)
  "FrontFacing\0" // IR(5027, 12)
  "SampleId\0" // IR(5039, 9)
  "SampleRateShading\0" // IR(5048, 18)
  "SamplePosition\0" // IR(5066, 15)
  "SampleMask\0" // IR(5081, 11)
  "FragDepth\0" // IR(5092, 10)
  "HelperInvocation\0" // IR(5102, 17)
  "NumWorkgroups\0" // IR(5119, 14)
  "WorkgroupSize\0" // IR(5133, 14)
  "WorkgroupId\0" // IR(5147, 12)
  "LocalInvocationId\0" // IR(5159, 18)
  "GlobalInvocationId\0" // IR(5177, 19)
  "LocalInvocationIndex\0" // IR(5196, 21)
  "WorkDim\0" // IR(5217, 8)
  "GlobalSize\0" // IR(5225, 11)
  "EnqueuedWorkgroupSize\0" // IR(5236, 22)
  "GlobalOffset\0" // IR(5258, 13)
  "GlobalLinearId\0" // IR(5271, 15)
  "SubgroupSize\0" // IR(5286, 13)
  "GroupNonUniform\0" // IR(5299, 16)
  "SubgroupBallotKHR\0" // IR(5315, 18)
  "SubgroupMaxSize\0" // IR(5333, 16)
  "NumSubgroups\0" // IR(5349, 13)
  "NumEnqueuedSubgroups\0" // IR(5362, 21)
  "SubgroupId\0" // IR(5383, 11)
  "SubgroupLocalInvocationId\0" // IR(5394, 26)
  "VertexIndex\0" // IR(5420, 12)
  "InstanceIndex\0" // IR(5432, 14)
  "CoreIDARM\0" // IR(5446, 10)
  "CoreBuiltinsARM\0" // IR(5456, 16)
  "CoreCountARM\0" // IR(5472, 13)
  "CoreMaxIDARM\0" // IR(5485, 13)
  "WarpIDARM\0" // IR(5498, 10)
  "WarpMaxIDARM\0" // IR(5508, 13)
  "SubgroupEqMask\0" // IR(5521, 15)
  "SubgroupEqMaskKHR\0" // IR(5536, 18)
  "GroupNonUniformBallot\0" // IR(5554, 22)
  "SubgroupGeMask\0" // IR(5576, 15)
  "SubgroupGeMaskKHR\0" // IR(5591, 18)
  "SubgroupGtMask\0" // IR(5609, 15)
  "SubgroupGtMaskKHR\0" // IR(5624, 18)
  "SubgroupLeMask\0" // IR(5642, 15)
  "SubgroupLeMaskKHR\0" // IR(5657, 18)
  "SubgroupLtMask\0" // IR(5675, 15)
  "SubgroupLtMaskKHR\0" // IR(5690, 18)
  "BaseVertex\0" // IR(5708, 11)
  "DrawParameters\0" // IR(5719, 15)
  "BaseInstance\0" // IR(5734, 13)
  "DrawIndex\0" // IR(5747, 10)
  "PrimitiveShadingRateKHR\0" // IR(5757, 24)
  "FragmentShadingRateKHR\0" // IR(5781, 23)
  "DeviceIndex\0" // IR(5804, 12)
  "DeviceGroup\0" // IR(5816, 12)
  "ViewIndex\0" // IR(5828, 10)
  "MultiView\0" // IR(5838, 10)
  "ShadingRateKHR\0" // IR(5848, 15)
  "TileOffsetQCOM\0" // IR(5863, 15)
  "TileShadingQCOM\0" // IR(5878, 16)
  "TileDimensionQCOM\0" // IR(5894, 18)
  "TileApronSizeQCOM\0" // IR(5912, 18)
  "BaryCoordNoPerspAMD\0" // IR(5930, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(5950, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(5978, 26)
  "BaryCoordSmoothAMD\0" // IR(6004, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(6023, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(6050, 25)
  "BaryCoordPullModelAMD\0" // IR(6075, 22)
  "FragStencilRefEXT\0" // IR(6097, 18)
  "StencilExportEXT\0" // IR(6115, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(6132, 29)
  "ShaderEnqueueAMDX\0" // IR(6161, 18)
  "ShaderIndexAMDX\0" // IR(6179, 16)
  "ViewportMaskNV\0" // IR(6195, 15)
  "ShaderViewportMaskNV\0" // IR(6210, 21)
  "SecondaryPositionNV\0" // IR(6231, 20)
  "ShaderStereoViewNV\0" // IR(6251, 19)
  "SecondaryViewportMaskNV\0" // IR(6270, 24)
  "PositionPerViewNV\0" // IR(6294, 18)
  "PerViewAttributesNV\0" // IR(6312, 20)
  "ViewportMaskPerViewNV\0" // IR(6332, 22)
  "FullyCoveredEXT\0" // IR(6354, 16)
  "FragmentFullyCoveredEXT\0" // IR(6370, 24)
  "TaskCountNV\0" // IR(6394, 12)
  "PrimitiveCountNV\0" // IR(6406, 17)
  "PrimitiveIndicesNV\0" // IR(6423, 19)
  "ClipDistancePerViewNV\0" // IR(6442, 22)
  "CullDistancePerViewNV\0" // IR(6464, 22)
  "LayerPerViewNV\0" // IR(6486, 15)
  "MeshViewCountNV\0" // IR(6501, 16)
  "MeshViewIndicesNV\0" // IR(6517, 18)
  "BaryCoordKHR\0" // IR(6535, 13)
  "BaryCoordNV\0" // IR(6548, 12)
  "FragmentBarycentricKHR\0" // IR(6560, 23)
  "BaryCoordNoPerspKHR\0" // IR(6583, 20)
  "BaryCoordNoPerspNV\0" // IR(6603, 19)
  "FragSizeEXT\0" // IR(6622, 12)
  "FragmentSizeNV\0" // IR(6634, 15)
  "FragmentDensityEXT\0" // IR(6649, 19)
  "FragInvocationCountEXT\0" // IR(6668, 23)
  "InvocationsPerPixelNV\0" // IR(6691, 22)
  "PrimitivePointIndicesEXT\0" // IR(6713, 25)
  "PrimitiveLineIndicesEXT\0" // IR(6738, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(6762, 28)
  "CullPrimitiveEXT\0" // IR(6790, 17)
  "LaunchIdKHR\0" // IR(6807, 12)
  "LaunchIdNV\0" // IR(6819, 11)
  "LaunchSizeKHR\0" // IR(6830, 14)
  "LaunchSizeNV\0" // IR(6844, 13)
  "WorldRayOriginKHR\0" // IR(6857, 18)
  "WorldRayOriginNV\0" // IR(6875, 17)
  "WorldRayDirectionKHR\0" // IR(6892, 21)
  "WorldRayDirectionNV\0" // IR(6913, 20)
  "ObjectRayOriginKHR\0" // IR(6933, 19)
  "ObjectRayOriginNV\0" // IR(6952, 18)
  "ObjectRayDirectionKHR\0" // IR(6970, 22)
  "ObjectRayDirectionNV\0" // IR(6992, 21)
  "RayTminKHR\0" // IR(7013, 11)
  "RayTminNV\0" // IR(7024, 10)
  "RayTmaxKHR\0" // IR(7034, 11)
  "RayTmaxNV\0" // IR(7045, 10)
  "InstanceCustomIndexKHR\0" // IR(7055, 23)
  "InstanceCustomIndexNV\0" // IR(7078, 22)
  "ObjectToWorldKHR\0" // IR(7100, 17)
  "ObjectToWorldNV\0" // IR(7117, 16)
  "WorldToObjectKHR\0" // IR(7133, 17)
  "WorldToObjectNV\0" // IR(7150, 16)
  "HitTNV\0" // IR(7166, 7)
  "HitKindKHR\0" // IR(7173, 11)
  "HitKindNV\0" // IR(7184, 10)
  "CurrentRayTimeNV\0" // IR(7194, 17)
  "RayTracingMotionBlurNV\0" // IR(7211, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(7234, 30)
  "RayTracingPositionFetchKHR\0" // IR(7264, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(7291, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(7325, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(7358, 37)
  "IncomingRayFlagsKHR\0" // IR(7395, 20)
  "IncomingRayFlagsNV\0" // IR(7415, 19)
  "RayGeometryIndexKHR\0" // IR(7434, 20)
  "HitIsSphereNV\0" // IR(7454, 14)
  "RayTracingSpheresGeometryNV\0" // IR(7468, 28)
  "HitIsLSSNV\0" // IR(7496, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(7507, 39)
  "HitSpherePositionNV\0" // IR(7546, 20)
  "WarpsPerSMNV\0" // IR(7566, 13)
  "ShaderSMBuiltinsNV\0" // IR(7579, 19)
  "SMCountNV\0" // IR(7598, 10)
  "WarpIDNV\0" // IR(7608, 9)
  "SMIDNV\0" // IR(7617, 7)
  "HitLSSPositionsNV\0" // IR(7624, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(7642, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(7676, 33)
  "HitSphereRadiusNV\0" // IR(7709, 18)
  "HitLSSRadiiNV\0" // IR(7727, 14)
  "ClusterIDNV\0" // IR(7741, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(7753, 41)
  "CullMaskKHR\0" // IR(7794, 12)
  "RayCullMaskKHR\0" // IR(7806, 15)
  "Matrix\0" // IR(7821, 7)
  "Linkage\0" // IR(7828, 8)
  "Vector16\0" // IR(7836, 9)
  "Float16Buffer\0" // IR(7845, 14)
  "Float16\0" // IR(7859, 8)
  "Float64\0" // IR(7867, 8)
  "Int64\0" // IR(7875, 6)
  "Int64Atomics\0" // IR(7881, 13)
  "ImageBasic\0" // IR(7894, 11)
  "ImageReadWrite\0" // IR(7905, 15)
  "ImageMipmap\0" // IR(7920, 12)
  "Pipes\0" // IR(7932, 6)
  "Groups\0" // IR(7938, 7)
  "DeviceEnqueue\0" // IR(7945, 14)
  "LiteralSampler\0" // IR(7959, 15)
  "AtomicStorage\0" // IR(7974, 14)
  "Int16\0" // IR(7988, 6)
  "TessellationPointSize\0" // IR(7994, 22)
  "GeometryPointSize\0" // IR(8016, 18)
  "ImageGatherExtended\0" // IR(8034, 20)
  "StorageImageMultisample\0" // IR(8054, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(8078, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(8112, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(8145, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(8179, 33)
  "ImageCubeArray\0" // IR(8212, 15)
  "SampledCubeArray\0" // IR(8227, 17)
  "ImageRect\0" // IR(8244, 10)
  "SampledRect\0" // IR(8254, 12)
  "GenericPointer\0" // IR(8266, 15)
  "Int8\0" // IR(8281, 5)
  "InputAttachment\0" // IR(8286, 16)
  "SparseResidency\0" // IR(8302, 16)
  "MinLod\0" // IR(8318, 7)
  "Sampled1D\0" // IR(8325, 10)
  "Image1D\0" // IR(8335, 8)
  "SampledBuffer\0" // IR(8343, 14)
  "ImageBuffer\0" // IR(8357, 12)
  "ImageMSArray\0" // IR(8369, 13)
  "StorageImageExtendedFormats\0" // IR(8382, 28)
  "ImageQuery\0" // IR(8410, 11)
  "DerivativeControl\0" // IR(8421, 18)
  "InterpolationFunction\0" // IR(8439, 22)
  "TransformFeedback\0" // IR(8461, 18)
  "GeometryStreams\0" // IR(8479, 16)
  "StorageImageReadWithoutFormat\0" // IR(8495, 30)
  "StorageImageWriteWithoutFormat\0" // IR(8525, 31)
  "SubgroupDispatch\0" // IR(8556, 17)
  "NamedBarrier\0" // IR(8573, 13)
  "PipeStorage\0" // IR(8586, 12)
  "GroupNonUniformVote\0" // IR(8598, 20)
  "GroupNonUniformArithmetic\0" // IR(8618, 26)
  "GroupNonUniformShuffle\0" // IR(8644, 23)
  "GroupNonUniformShuffleRelative\0" // IR(8667, 31)
  "GroupNonUniformClustered\0" // IR(8698, 25)
  "GroupNonUniformQuad\0" // IR(8723, 20)
  "UniformDecoration\0" // IR(8743, 18)
  "TileImageColorReadAccessEXT\0" // IR(8761, 28)
  "TileImageDepthReadAccessEXT\0" // IR(8789, 28)
  "TileImageStencilReadAccessEXT\0" // IR(8817, 30)
  "TensorsARM\0" // IR(8847, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(8858, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(8895, 40)
  "CooperativeMatrixLayoutsARM\0" // IR(8935, 28)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(8963, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(8996, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(9039, 44)
  "SubgroupVoteKHR\0" // IR(9083, 16)
  "StorageBuffer16BitAccess\0" // IR(9099, 25)
  "StorageUniformBufferBlock16\0" // IR(9124, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(9152, 35)
  "StorageUniform16\0" // IR(9187, 17)
  "StoragePushConstant16\0" // IR(9204, 22)
  "StorageInputOutput16\0" // IR(9226, 21)
  "VariablePointersStorageBuffer\0" // IR(9247, 30)
  "VariablePointers\0" // IR(9277, 17)
  "AtomicStorageOps\0" // IR(9294, 17)
  "SampleMaskPostDepthCoverage\0" // IR(9311, 28)
  "StorageBuffer8BitAccess\0" // IR(9339, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(9363, 34)
  "StoragePushConstant8\0" // IR(9397, 21)
  "DenormPreserve\0" // IR(9418, 15)
  "DenormFlushToZero\0" // IR(9433, 18)
  "SignedZeroInfNanPreserve\0" // IR(9451, 25)
  "RoundingModeRTE\0" // IR(9476, 16)
  "RoundingModeRTZ\0" // IR(9492, 16)
  "RayQueryProvisionalKHR\0" // IR(9508, 23)
  "RayQueryKHR\0" // IR(9531, 12)
  "UntypedPointersKHR\0" // IR(9543, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(9562, 32)
  "TextureSampleWeightedQCOM\0" // IR(9594, 26)
  "TextureBoxFilterQCOM\0" // IR(9620, 21)
  "TextureBlockMatchQCOM\0" // IR(9641, 22)
  "TextureBlockMatch2QCOM\0" // IR(9663, 23)
  "Float16ImageAMD\0" // IR(9686, 16)
  "ImageGatherBiasLodAMD\0" // IR(9702, 22)
  "FragmentMaskAMD\0" // IR(9724, 16)
  "ImageReadWriteLodAMD\0" // IR(9740, 21)
  "Int64ImageEXT\0" // IR(9761, 14)
  "ShaderClockKHR\0" // IR(9775, 15)
  "QuadControlKHR\0" // IR(9790, 15)
  "Int4TypeINTEL\0" // IR(9805, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(9819, 27)
  "CooperativeMatrixKHR\0" // IR(9846, 21)
  "BFloat16TypeKHR\0" // IR(9867, 16)
  "BFloat16DotProductKHR\0" // IR(9883, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(9905, 29)
  "SampleMaskOverrideCoverageNV\0" // IR(9934, 29)
  "GeometryShaderPassthroughNV\0" // IR(9963, 28)
  "ShaderViewportIndexLayerNV\0" // IR(9991, 27)
  "ImageFootprintNV\0" // IR(10018, 17)
  "FragmentBarycentricNV\0" // IR(10035, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(10057, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(10088, 30)
  "ShadingRateNV\0" // IR(10118, 14)
  "GroupNonUniformPartitionedNV\0" // IR(10132, 29)
  "ShaderNonUniform\0" // IR(10161, 17)
  "ShaderNonUniformEXT\0" // IR(10178, 20)
  "RuntimeDescriptorArray\0" // IR(10198, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(10221, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(10247, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(10283, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(10322, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(10361, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(10403, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(10442, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(10484, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(10521, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(10561, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(10597, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(10636, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(10673, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(10713, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(10749, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(10788, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(10827, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(10869, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(10911, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(10956, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(10998, 45)
  "VulkanMemoryModel\0" // IR(11043, 18)
  "VulkanMemoryModelKHR\0" // IR(11061, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(11082, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(11111, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(11143, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(11177, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(11209, 31)
  "RayTracingProvisionalKHR\0" // IR(11240, 25)
  "CooperativeMatrixNV\0" // IR(11265, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(11285, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(11318, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(11356, 32)
  "DemoteToHelperInvocation\0" // IR(11388, 25)
  "DemoteToHelperInvocationEXT\0" // IR(11413, 28)
  "DisplacementMicromapNV\0" // IR(11441, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(11464, 29)
  "ShaderInvocationReorderNV\0" // IR(11493, 26)
  "BindlessTextureNV\0" // IR(11519, 18)
  "RayQueryPositionFetchKHR\0" // IR(11537, 25)
  "CooperativeVectorNV\0" // IR(11562, 20)
  "AtomicFloat16VectorNV\0" // IR(11582, 22)
  "RawAccessChainsNV\0" // IR(11604, 18)
  "CooperativeMatrixReductionsNV\0" // IR(11622, 30)
  "CooperativeMatrixConversionsNV\0" // IR(11652, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(11683, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(11723, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(11759, 30)
  "CooperativeVectorTrainingNV\0" // IR(11789, 28)
  "TensorAddressingNV\0" // IR(11817, 19)
  "SubgroupShuffleINTEL\0" // IR(11836, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(11857, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(11884, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(11910, 31)
  "RoundToInfinityINTEL\0" // IR(11941, 21)
  "FloatingPointModeINTEL\0" // IR(11962, 23)
  "IntegerFunctions2INTEL\0" // IR(11985, 23)
  "FunctionPointersINTEL\0" // IR(12008, 22)
  "IndirectReferencesINTEL\0" // IR(12030, 24)
  "AsmINTEL\0" // IR(12054, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(12063, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(12086, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(12109, 23)
  "VectorComputeINTEL\0" // IR(12132, 19)
  "VectorAnyINTEL\0" // IR(12151, 15)
  "ExpectAssumeKHR\0" // IR(12166, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(12182, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(12215, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(12253, 39)
  "VariableLengthArrayINTEL\0" // IR(12292, 25)
  "FunctionFloatControlINTEL\0" // IR(12317, 26)
  "FPGAMemoryAttributesINTEL\0" // IR(12343, 26)
  "FPFastMathModeINTEL\0" // IR(12369, 20)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(12389, 32)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(12421, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(12458, 30)
  "FPGALoopControlsINTEL\0" // IR(12488, 22)
  "KernelAttributesINTEL\0" // IR(12510, 22)
  "FPGAKernelAttributesINTEL\0" // IR(12532, 26)
  "FPGAMemoryAccessesINTEL\0" // IR(12558, 24)
  "FPGAClusterAttributesINTEL\0" // IR(12582, 27)
  "LoopFuseINTEL\0" // IR(12609, 14)
  "FPGADSPControlINTEL\0" // IR(12623, 20)
  "MemoryAccessAliasingINTEL\0" // IR(12643, 26)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(12669, 40)
  "FPGABufferLocationINTEL\0" // IR(12709, 24)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(12733, 34)
  "USMStorageClassesINTEL\0" // IR(12767, 23)
  "RuntimeAlignedAttributeINTEL\0" // IR(12790, 29)
  "IOPipesINTEL\0" // IR(12819, 13)
  "BlockingPipesINTEL\0" // IR(12832, 19)
  "FPGARegINTEL\0" // IR(12851, 13)
  "DotProductInputAll\0" // IR(12864, 19)
  "DotProductInputAllKHR\0" // IR(12883, 22)
  "DotProductInput4x8Bit\0" // IR(12905, 22)
  "DotProductInput4x8BitKHR\0" // IR(12927, 25)
  "DotProductInput4x8BitPacked\0" // IR(12952, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(12980, 31)
  "DotProduct\0" // IR(13011, 11)
  "DotProductKHR\0" // IR(13022, 14)
  "ReplicatedCompositesEXT\0" // IR(13036, 24)
  "BitInstructions\0" // IR(13060, 16)
  "GroupNonUniformRotateKHR\0" // IR(13076, 25)
  "FloatControls2\0" // IR(13101, 15)
  "AtomicFloat32AddEXT\0" // IR(13116, 20)
  "AtomicFloat64AddEXT\0" // IR(13136, 20)
  "LongCompositesINTEL\0" // IR(13156, 20)
  "OptNoneEXT\0" // IR(13176, 11)
  "OptNoneINTEL\0" // IR(13187, 13)
  "AtomicFloat16AddEXT\0" // IR(13200, 20)
  "DebugInfoModuleINTEL\0" // IR(13220, 21)
  "BFloat16ConversionINTEL\0" // IR(13241, 24)
  "SplitBarrierINTEL\0" // IR(13265, 18)
  "ArithmeticFenceEXT\0" // IR(13283, 19)
  "FPGAClusterAttributesV2INTEL\0" // IR(13302, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(13331, 28)
  "TaskSequenceINTEL\0" // IR(13359, 18)
  "FPMaxErrorINTEL\0" // IR(13377, 16)
  "FPGALatencyControlINTEL\0" // IR(13393, 24)
  "FPGAArgumentInterfacesINTEL\0" // IR(13417, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(13445, 30)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(13475, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(13510, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(13538, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(13561, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(13591, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(13621, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(13659, 28)
  "GroupUniformArithmeticKHR\0" // IR(13687, 26)
  "TensorFloat32RoundingINTEL\0" // IR(13713, 27)
  "MaskedGatherScatterINTEL\0" // IR(13740, 25)
  "CacheControlsINTEL\0" // IR(13765, 19)
  "RegisterLimitsINTEL\0" // IR(13784, 20)
  "BindlessImagesINTEL\0" // IR(13804, 20)
  "Unspecified\0" // IR(13824, 12)
  "Address\0" // IR(13836, 8)
  "Boolean\0" // IR(13844, 8)
  "Float\0" // IR(13852, 6)
  "Signed\0" // IR(13858, 7)
  "SignedChar\0" // IR(13865, 11)
  "Unsigned\0" // IR(13876, 9)
  "UnsignedChar\0" // IR(13885, 13)
  "Class\0" // IR(13898, 6)
  "Structure\0" // IR(13904, 10)
  "Union\0" // IR(13914, 6)
  "ImportedModule\0" // IR(13920, 15)
  "ImportedDeclaration\0" // IR(13935, 20)
  "None\0" // IR(13955, 5)
  "FlagIsProtected\0" // IR(13960, 16)
  "FlagIsPrivate\0" // IR(13976, 14)
  "FlagIsPublic\0" // IR(13990, 13)
  "FlagIsLocal\0" // IR(14003, 12)
  "FlagIsDefinition\0" // IR(14015, 17)
  "FlagFwdDecl\0" // IR(14032, 12)
  "FlagArtificial\0" // IR(14044, 15)
  "FlagExplicit\0" // IR(14059, 13)
  "FlagPrototyped\0" // IR(14072, 15)
  "FlagObjectPointer\0" // IR(14087, 18)
  "FlagStaticMember\0" // IR(14105, 17)
  "FlagIndirectVariable\0" // IR(14122, 21)
  "FlagLValueReference\0" // IR(14143, 20)
  "FlagRValueReference\0" // IR(14163, 20)
  "FlagIsOptimized\0" // IR(14183, 16)
  "FlagIsEnumClass\0" // IR(14199, 16)
  "FlagTypePassByValue\0" // IR(14215, 20)
  "FlagTypePassByReference\0" // IR(14235, 24)
  "Deref\0" // IR(14259, 6)
  "Plus\0" // IR(14265, 5)
  "Minus\0" // IR(14270, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(14276, 33)
  "PlusUconst\0" // IR(14309, 11)
  "BitPiece\0" // IR(14320, 9)
  "Swap\0" // IR(14329, 5)
  "Xderef\0" // IR(14334, 7)
  "StackValue\0" // IR(14341, 11)
  "Constu\0" // IR(14352, 7)
  "Fragment\0" // IR(14359, 9)
  "ConstType\0" // IR(14368, 10)
  "VolatileType\0" // IR(14378, 13)
  "RestrictType\0" // IR(14391, 13)
  "AtomicType\0" // IR(14404, 11)
  "Float16NV\0" // IR(14415, 10)
  "Float32NV\0" // IR(14425, 10)
  "Float64NV\0" // IR(14435, 10)
  "SignedInt8NV\0" // IR(14445, 13)
  "SignedInt16NV\0" // IR(14458, 14)
  "SignedInt32NV\0" // IR(14472, 14)
  "SignedInt64NV\0" // IR(14486, 14)
  "UnsignedInt8NV\0" // IR(14500, 15)
  "UnsignedInt16NV\0" // IR(14515, 16)
  "UnsignedInt32NV\0" // IR(14531, 16)
  "UnsignedInt64NV\0" // IR(14547, 16)
  "SignedInt8PackedNV\0" // IR(14563, 19)
  "UnsignedInt8PackedNV\0" // IR(14582, 21)
  "FloatE4M3NV\0" // IR(14603, 12)
  "FloatE5M2NV\0" // IR(14615, 12)
  "RowMajorKHR\0" // IR(14627, 12)
  "ColumnMajorKHR\0" // IR(14639, 15)
  "RowBlockedInterleavedARM\0" // IR(14654, 25)
  "ColumnBlockedInterleavedARM\0" // IR(14679, 28)
  "NoneKHR\0" // IR(14707, 8)
  "MatrixASignedComponentsKHR\0" // IR(14715, 27)
  "MatrixBSignedComponentsKHR\0" // IR(14742, 27)
  "MatrixCSignedComponentsKHR\0" // IR(14769, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(14796, 32)
  "SaturatingAccumulationKHR\0" // IR(14828, 26)
  "Row\0" // IR(14854, 4)
  "Column\0" // IR(14858, 7)
  "2x2\0" // IR(14865, 4)
  "MatrixAKHR\0" // IR(14869, 11)
  "MatrixBKHR\0" // IR(14880, 11)
  "MatrixAccumulatorKHR\0" // IR(14891, 21)
  "RowMajorNV\0" // IR(14912, 11)
  "ColumnMajorNV\0" // IR(14923, 14)
  "InferencingOptimalNV\0" // IR(14937, 21)
  "TrainingOptimalNV\0" // IR(14958, 18)
  "RelaxedPrecision\0" // IR(14976, 17)
  "SpecId\0" // IR(14993, 7)
  "Block\0" // IR(15000, 6)
  "BufferBlock\0" // IR(15006, 12)
  "RowMajor\0" // IR(15018, 9)
  "ColMajor\0" // IR(15027, 9)
  "ArrayStride\0" // IR(15036, 12)
  "MatrixStride\0" // IR(15048, 13)
  "GLSLShared\0" // IR(15061, 11)
  "GLSLPacked\0" // IR(15072, 11)
  "CPacked\0" // IR(15083, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(15091, 26)
  "BuiltIn\0" // IR(15117, 8)
  "NoPerspective\0" // IR(15125, 14)
  "Flat\0" // IR(15139, 5)
  "Patch\0" // IR(15144, 6)
  "Centroid\0" // IR(15150, 9)
  "Sample\0" // IR(15159, 7)
  "Invariant\0" // IR(15166, 10)
  "Restrict\0" // IR(15176, 9)
  "Aliased\0" // IR(15185, 8)
  "Volatile\0" // IR(15193, 9)
  "Constant\0" // IR(15202, 9)
  "Coherent\0" // IR(15211, 9)
  "NonWritable\0" // IR(15220, 12)
  "NonReadable\0" // IR(15232, 12)
  "Uniform\0" // IR(15244, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(15252, 26)
  "UniformId\0" // IR(15278, 10)
  "SaturatedConversion\0" // IR(15288, 20)
  "Stream\0" // IR(15308, 7)
  "Location\0" // IR(15315, 9)
  "Component\0" // IR(15324, 10)
  "Index\0" // IR(15334, 6)
  "Binding\0" // IR(15340, 8)
  "DescriptorSet\0" // IR(15348, 14)
  "Offset\0" // IR(15362, 7)
  "XfbBuffer\0" // IR(15369, 10)
  "XfbStride\0" // IR(15379, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(15389, 46)
  "FuncParamAttr\0" // IR(15435, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(15449, 34)
  "FPRoundingMode\0" // IR(15483, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(15498, 35)
  "FPFastMathMode\0" // IR(15533, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(15548, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(15580, 30)
  "LinkageAttributes\0" // IR(15610, 18)
  "NoContraction\0" // IR(15628, 14)
  "InputAttachmentIndex\0" // IR(15642, 21)
  "Alignment\0" // IR(15663, 10)
  "MaxByteOffset\0" // IR(15673, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(15687, 20)
  "AlignmentId\0" // IR(15707, 12)
  "MaxByteOffsetId\0" // IR(15719, 16)
  "NoSignedWrap\0" // IR(15735, 13)
  "NoUnsignedWrap\0" // IR(15748, 15)
  "WeightTextureQCOM\0" // IR(15763, 18)
  "BlockMatchTextureQCOM\0" // IR(15781, 22)
  "BlockMatchSamplerQCOM\0" // IR(15803, 22)
  "ExplicitInterpAMD\0" // IR(15825, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(15843, 32)
  "NodeMaxPayloadsAMDX\0" // IR(15875, 20)
  "TrackFinishWritingAMDX\0" // IR(15895, 23)
  "PayloadNodeNameAMDX\0" // IR(15918, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(15938, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(15963, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(15990, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(16015, 28)
  "OverrideCoverageNV\0" // IR(16043, 19)
  "PassthroughNV\0" // IR(16062, 14)
  "ViewportRelativeNV\0" // IR(16076, 19)
  "SecondaryViewportRelativeNV\0" // IR(16095, 28)
  "PerPrimitiveEXT\0" // IR(16123, 16)
  "PerPrimitiveNV\0" // IR(16139, 15)
  "PerViewNV\0" // IR(16154, 10)
  "PerTaskNV\0" // IR(16164, 10)
  "PerVertexKHR\0" // IR(16174, 13)
  "PerVertexNV\0" // IR(16187, 12)
  "NonUniform\0" // IR(16199, 11)
  "NonUniformEXT\0" // IR(16210, 14)
  "RestrictPointer\0" // IR(16224, 16)
  "RestrictPointerEXT\0" // IR(16240, 19)
  "AliasedPointer\0" // IR(16259, 15)
  "AliasedPointerEXT\0" // IR(16274, 18)
  "HitObjectShaderRecordBufferNV\0" // IR(16292, 30)
  "BindlessSamplerNV\0" // IR(16322, 18)
  "BindlessImageNV\0" // IR(16340, 16)
  "BoundSamplerNV\0" // IR(16356, 15)
  "BoundImageNV\0" // IR(16371, 13)
  "SIMTCallINTEL\0" // IR(16384, 14)
  "ReferencedIndirectlyINTEL\0" // IR(16398, 26)
  "ClobberINTEL\0" // IR(16424, 13)
  "SideEffectsINTEL\0" // IR(16437, 17)
  "VectorComputeVariableINTEL\0" // IR(16454, 27)
  "FuncParamIOKindINTEL\0" // IR(16481, 21)
  "VectorComputeFunctionINTEL\0" // IR(16502, 27)
  "StackCallINTEL\0" // IR(16529, 15)
  "GlobalVariableOffsetINTEL\0" // IR(16544, 26)
  "CounterBuffer\0" // IR(16570, 14)
  "HlslCounterBufferGOOGLE\0" // IR(16584, 24)
  "UserSemantic\0" // IR(16608, 13)
  "HlslSemanticGOOGLE\0" // IR(16621, 19)
  "UserTypeGOOGLE\0" // IR(16640, 15)
  "FunctionRoundingModeINTEL\0" // IR(16655, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(16681, 31)
  "FunctionDenormModeINTEL\0" // IR(16712, 24)
  "RegisterINTEL\0" // IR(16736, 14)
  "MemoryINTEL\0" // IR(16750, 12)
  "NumbanksINTEL\0" // IR(16762, 14)
  "BankwidthINTEL\0" // IR(16776, 15)
  "MaxPrivateCopiesINTEL\0" // IR(16791, 22)
  "SinglepumpINTEL\0" // IR(16813, 16)
  "DoublepumpINTEL\0" // IR(16829, 16)
  "MaxReplicatesINTEL\0" // IR(16845, 19)
  "SimpleDualPortINTEL\0" // IR(16864, 20)
  "MergeINTEL\0" // IR(16884, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(16895, 42)
  "BankBitsINTEL\0" // IR(16937, 14)
  "ForcePow2DepthINTEL\0" // IR(16951, 20)
  "StridesizeINTEL\0" // IR(16971, 16)
  "WordsizeINTEL\0" // IR(16987, 14)
  "TrueDualPortINTEL\0" // IR(17001, 18)
  "BurstCoalesceINTEL\0" // IR(17019, 19)
  "CacheSizeINTEL\0" // IR(17038, 15)
  "DontStaticallyCoalesceINTEL\0" // IR(17053, 28)
  "PrefetchINTEL\0" // IR(17081, 14)
  "StallEnableINTEL\0" // IR(17095, 17)
  "FuseLoopsInFunctionINTEL\0" // IR(17112, 25)
  "MathOpDSPModeINTEL\0" // IR(17137, 19)
  "AliasScopeINTEL\0" // IR(17156, 16)
  "NoAliasINTEL\0" // IR(17172, 13)
  "InitiationIntervalINTEL\0" // IR(17185, 24)
  "MaxConcurrencyINTEL\0" // IR(17209, 20)
  "PipelineEnableINTEL\0" // IR(17229, 20)
  "BufferLocationINTEL\0" // IR(17249, 20)
  "IOPipeStorageINTEL\0" // IR(17269, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(17288, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(17322, 31)
  "SingleElementVectorINTEL\0" // IR(17353, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(17378, 35)
  "MediaBlockIOINTEL\0" // IR(17413, 18)
  "StallFreeINTEL\0" // IR(17431, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(17446, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(17477, 26)
  "LatencyControlLabelINTEL\0" // IR(17503, 25)
  "LatencyControlConstraintINTEL\0" // IR(17528, 30)
  "ConduitKernelArgumentINTEL\0" // IR(17558, 27)
  "RegisterMapKernelArgumentINTEL\0" // IR(17585, 31)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(17616, 33)
  "MMHostInterfaceDataWidthINTEL\0" // IR(17649, 30)
  "MMHostInterfaceLatencyINTEL\0" // IR(17679, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(17707, 34)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(17741, 34)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(17775, 29)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(17804, 32)
  "StableKernelArgumentINTEL\0" // IR(17836, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(17862, 39)
  "HostAccessINTEL\0" // IR(17901, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(17917, 47)
  "InitModeINTEL\0" // IR(17964, 14)
  "ImplementInRegisterMapINTEL\0" // IR(17978, 28)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(18006, 36)
  "CacheControlLoadINTEL\0" // IR(18042, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(18064, 37)
  "CacheControlStoreINTEL\0" // IR(18101, 23)
  "1D\0" // IR(18124, 3)
  "2D\0" // IR(18127, 3)
  "3D\0" // IR(18130, 3)
  "Cube\0" // IR(18133, 5)
  "Rect\0" // IR(18138, 5)
  "Buffer\0" // IR(18143, 7)
  "SubpassData\0" // IR(18150, 12)
  "TileImageDataEXT\0" // IR(18162, 17)
  "Invocations\0" // IR(18179, 12)
  "SpacingEqual\0" // IR(18191, 13)
  "SpacingFractionalEven\0" // IR(18204, 22)
  "SpacingFractionalOdd\0" // IR(18226, 21)
  "VertexOrderCw\0" // IR(18247, 14)
  "VertexOrderCcw\0" // IR(18261, 15)
  "PixelCenterInteger\0" // IR(18276, 19)
  "OriginUpperLeft\0" // IR(18295, 16)
  "OriginLowerLeft\0" // IR(18311, 16)
  "EarlyFragmentTests\0" // IR(18327, 19)
  "PointMode\0" // IR(18346, 10)
  "Xfb\0" // IR(18356, 4)
  "DepthReplacing\0" // IR(18360, 15)
  "DepthGreater\0" // IR(18375, 13)
  "DepthLess\0" // IR(18388, 10)
  "DepthUnchanged\0" // IR(18398, 15)
  "LocalSize\0" // IR(18413, 10)
  "LocalSizeHint\0" // IR(18423, 14)
  "InputPoints\0" // IR(18437, 12)
  "InputLines\0" // IR(18449, 11)
  "InputLinesAdjacency\0" // IR(18460, 20)
  "Triangles\0" // IR(18480, 10)
  "InputTrianglesAdjacency\0" // IR(18490, 24)
  "Quads\0" // IR(18514, 6)
  "Isolines\0" // IR(18520, 9)
  "OutputVertices\0" // IR(18529, 15)
  "OutputPoints\0" // IR(18544, 13)
  "OutputLineStrip\0" // IR(18557, 16)
  "OutputTriangleStrip\0" // IR(18573, 20)
  "VecTypeHint\0" // IR(18593, 12)
  "ContractionOff\0" // IR(18605, 15)
  "Initializer\0" // IR(18620, 12)
  "Finalizer\0" // IR(18632, 10)
  "SubgroupsPerWorkgroup\0" // IR(18642, 22)
  "SubgroupsPerWorkgroupId\0" // IR(18664, 24)
  "LocalSizeId\0" // IR(18688, 12)
  "LocalSizeHintId\0" // IR(18700, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(18716, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(18750, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(18784, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(18820, 30)
  "PostDepthCoverage\0" // IR(18850, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(18868, 34)
  "TileShadingRateQCOM\0" // IR(18902, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(18922, 29)
  "StencilRefReplacingEXT\0" // IR(18951, 23)
  "CoalescingAMDX\0" // IR(18974, 15)
  "IsApiEntryAMDX\0" // IR(18989, 15)
  "MaxNodeRecursionAMDX\0" // IR(19004, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(19025, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(19049, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(19070, 28)
  "StencilRefGreaterFrontAMD\0" // IR(19098, 26)
  "StencilRefLessFrontAMD\0" // IR(19124, 23)
  "StencilRefUnchangedBackAMD\0" // IR(19147, 27)
  "StencilRefGreaterBackAMD\0" // IR(19174, 25)
  "StencilRefLessBackAMD\0" // IR(19199, 22)
  "QuadDerivativesKHR\0" // IR(19221, 19)
  "RequireFullQuadsKHR\0" // IR(19240, 20)
  "SharesInputWithAMDX\0" // IR(19260, 20)
  "OutputLinesEXT\0" // IR(19280, 15)
  "OutputLinesNV\0" // IR(19295, 14)
  "OutputPrimitivesEXT\0" // IR(19309, 20)
  "OutputPrimitivesNV\0" // IR(19329, 19)
  "DerivativeGroupQuadsKHR\0" // IR(19348, 24)
  "DerivativeGroupQuadsNV\0" // IR(19372, 23)
  "DerivativeGroupLinearKHR\0" // IR(19395, 25)
  "DerivativeGroupLinearNV\0" // IR(19420, 24)
  "OutputTrianglesEXT\0" // IR(19444, 19)
  "OutputTrianglesNV\0" // IR(19463, 18)
  "PixelInterlockOrderedEXT\0" // IR(19481, 25)
  "PixelInterlockUnorderedEXT\0" // IR(19506, 27)
  "SampleInterlockOrderedEXT\0" // IR(19533, 26)
  "SampleInterlockUnorderedEXT\0" // IR(19559, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(19587, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(19618, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(19651, 27)
  "RoundingModeRTPINTEL\0" // IR(19678, 21)
  "RoundingModeRTNINTEL\0" // IR(19699, 21)
  "FloatingPointModeALTINTEL\0" // IR(19720, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(19746, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(19773, 22)
  "MaxWorkDimINTEL\0" // IR(19795, 16)
  "NoGlobalOffsetINTEL\0" // IR(19811, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(19831, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(19853, 28)
  "MaximallyReconvergesKHR\0" // IR(19881, 24)
  "FPFastMathDefault\0" // IR(19905, 18)
  "StreamingInterfaceINTEL\0" // IR(19923, 24)
  "RegisterMapInterfaceINTEL\0" // IR(19947, 26)
  "NamedBarrierCountINTEL\0" // IR(19973, 23)
  "MaximumRegistersINTEL\0" // IR(19996, 22)
  "MaximumRegistersIdINTEL\0" // IR(20018, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(20042, 51)
  "NamedMaximumRegistersINTEL\0" // IR(20093, 27)
  "Vertex\0" // IR(20120, 7)
  "TessellationControl\0" // IR(20127, 20)
  "TessellationEvaluation\0" // IR(20147, 23)
  "GLCompute\0" // IR(20170, 10)
  "TaskNV\0" // IR(20180, 7)
  "MeshNV\0" // IR(20187, 7)
  "RayGenerationKHR\0" // IR(20194, 17)
  "RayGenerationNV\0" // IR(20211, 16)
  "IntersectionKHR\0" // IR(20227, 16)
  "IntersectionNV\0" // IR(20243, 15)
  "AnyHitKHR\0" // IR(20258, 10)
  "AnyHitNV\0" // IR(20268, 9)
  "ClosestHitKHR\0" // IR(20277, 14)
  "ClosestHitNV\0" // IR(20291, 13)
  "MissKHR\0" // IR(20304, 8)
  "MissNV\0" // IR(20312, 7)
  "CallableKHR\0" // IR(20319, 12)
  "CallableNV\0" // IR(20331, 11)
  "TaskEXT\0" // IR(20342, 8)
  "MeshEXT\0" // IR(20350, 8)
  "Preserve\0" // IR(20358, 9)
  "FlushToZero\0" // IR(20367, 12)
  "BFloat16KHR\0" // IR(20379, 12)
  "IEEE\0" // IR(20391, 5)
  "ALT\0" // IR(20396, 4)
  "NotNaN\0" // IR(20400, 7)
  "NotInf\0" // IR(20407, 7)
  "NSZ\0" // IR(20414, 4)
  "AllowRecip\0" // IR(20418, 11)
  "Fast\0" // IR(20429, 5)
  "AllowContract\0" // IR(20434, 14)
  "AllowContractFastINTEL\0" // IR(20448, 23)
  "AllowReassoc\0" // IR(20471, 13)
  "AllowReassocINTEL\0" // IR(20484, 18)
  "AllowTransform\0" // IR(20502, 15)
  "RTE\0" // IR(20517, 4)
  "RTZ\0" // IR(20521, 4)
  "RTP\0" // IR(20525, 4)
  "RTN\0" // IR(20529, 4)
  "Vertical2Pixels\0" // IR(20533, 16)
  "Vertical4Pixels\0" // IR(20549, 16)
  "Horizontal2Pixels\0" // IR(20565, 18)
  "Horizontal4Pixels\0" // IR(20583, 18)
  "Inline\0" // IR(20601, 7)
  "DontInline\0" // IR(20608, 11)
  "Pure\0" // IR(20619, 5)
  "Const\0" // IR(20624, 6)
  "Zext\0" // IR(20630, 5)
  "Sext\0" // IR(20635, 5)
  "ByVal\0" // IR(20640, 6)
  "Sret\0" // IR(20646, 5)
  "NoAlias\0" // IR(20651, 8)
  "NoCapture\0" // IR(20659, 10)
  "NoWrite\0" // IR(20669, 8)
  "NoReadWrite\0" // IR(20677, 12)
  "RuntimeAlignedINTEL\0" // IR(20689, 20)
  "Reduce\0" // IR(20709, 7)
  "InclusiveScan\0" // IR(20716, 14)
  "ExclusiveScan\0" // IR(20730, 14)
  "ClusteredReduce\0" // IR(20744, 16)
  "PartitionedReduceNV\0" // IR(20760, 20)
  "PartitionedInclusiveScanNV\0" // IR(20780, 27)
  "PartitionedExclusiveScanNV\0" // IR(20807, 27)
  "NoneINTEL\0" // IR(20834, 10)
  "ReadINTEL\0" // IR(20844, 10)
  "WriteINTEL\0" // IR(20854, 11)
  "ReadWriteINTEL\0" // IR(20865, 15)
  "Bias\0" // IR(20880, 5)
  "Lod\0" // IR(20885, 4)
  "Grad\0" // IR(20889, 5)
  "ConstOffset\0" // IR(20894, 12)
  "ConstOffsets\0" // IR(20906, 13)
  "MakeTexelAvailable\0" // IR(20919, 19)
  "MakeTexelAvailableKHR\0" // IR(20938, 22)
  "MakeTexelVisible\0" // IR(20960, 17)
  "MakeTexelVisibleKHR\0" // IR(20977, 20)
  "NonPrivateTexel\0" // IR(20997, 16)
  "NonPrivateTexelKHR\0" // IR(21013, 19)
  "VolatileTexel\0" // IR(21032, 14)
  "VolatileTexelKHR\0" // IR(21046, 17)
  "SignExtend\0" // IR(21063, 11)
  "ZeroExtend\0" // IR(21074, 11)
  "Nontemporal\0" // IR(21085, 12)
  "Offsets\0" // IR(21097, 8)
  "SnormInt8\0" // IR(21105, 10)
  "SnormInt16\0" // IR(21115, 11)
  "UnormInt8\0" // IR(21126, 10)
  "UnormInt16\0" // IR(21136, 11)
  "UnormShort565\0" // IR(21147, 14)
  "UnormShort555\0" // IR(21161, 14)
  "UnormInt101010\0" // IR(21175, 15)
  "SignedInt8\0" // IR(21190, 11)
  "SignedInt16\0" // IR(21201, 12)
  "SignedInt32\0" // IR(21213, 12)
  "UnsignedInt8\0" // IR(21225, 13)
  "UnsignedInt16\0" // IR(21238, 14)
  "UnsignedInt32\0" // IR(21252, 14)
  "HalfFloat\0" // IR(21266, 10)
  "UnormInt24\0" // IR(21276, 11)
  "UnormInt101010_2\0" // IR(21287, 17)
  "UnormInt10X6EXT\0" // IR(21304, 16)
  "UnsignedIntRaw10EXT\0" // IR(21320, 20)
  "UnsignedIntRaw12EXT\0" // IR(21340, 20)
  "UnormInt2_101010EXT\0" // IR(21360, 20)
  "UnsignedInt10X6EXT\0" // IR(21380, 19)
  "UnsignedInt12X4EXT\0" // IR(21399, 19)
  "UnsignedInt14X2EXT\0" // IR(21418, 19)
  "UnormInt12X4EXT\0" // IR(21437, 16)
  "UnormInt14X2EXT\0" // IR(21453, 16)
  "R\0" // IR(21469, 2)
  "A\0" // IR(21471, 2)
  "RG\0" // IR(21473, 3)
  "RA\0" // IR(21476, 3)
  "RGB\0" // IR(21479, 4)
  "RGBA\0" // IR(21483, 5)
  "BGRA\0" // IR(21488, 5)
  "ARGB\0" // IR(21493, 5)
  "Intensity\0" // IR(21498, 10)
  "Luminance\0" // IR(21508, 10)
  "Rx\0" // IR(21518, 3)
  "RGx\0" // IR(21521, 4)
  "RGBx\0" // IR(21525, 5)
  "Depth\0" // IR(21530, 6)
  "DepthStencil\0" // IR(21536, 13)
  "sRGB\0" // IR(21549, 5)
  "sRGBx\0" // IR(21554, 6)
  "sRGBA\0" // IR(21560, 6)
  "sBGRA\0" // IR(21566, 6)
  "ABGR\0" // IR(21572, 5)
  "InitOnDeviceReprogramINTEL\0" // IR(21577, 27)
  "InitOnDeviceResetINTEL\0" // IR(21604, 23)
  "NoWait\0" // IR(21627, 7)
  "WaitKernel\0" // IR(21634, 11)
  "WaitWorkGroup\0" // IR(21645, 14)
  "CmdExecTime\0" // IR(21659, 12)
  "MayUsePrintf\0" // IR(21671, 13)
  "Export\0" // IR(21684, 7)
  "Import\0" // IR(21691, 7)
  "LinkOnceODR\0" // IR(21698, 12)
  "UncachedINTEL\0" // IR(21710, 14)
  "CachedINTEL\0" // IR(21724, 12)
  "StreamingINTEL\0" // IR(21736, 15)
  "InvalidateAfterReadINTEL\0" // IR(21751, 25)
  "ConstCachedINTEL\0" // IR(21776, 17)
  "Unroll\0" // IR(21793, 7)
  "DontUnroll\0" // IR(21800, 11)
  "DependencyInfinite\0" // IR(21811, 19)
  "DependencyLength\0" // IR(21830, 17)
  "MinIterations\0" // IR(21847, 14)
  "MaxIterations\0" // IR(21861, 14)
  "IterationMultiple\0" // IR(21875, 18)
  "PeelCount\0" // IR(21893, 10)
  "PartialCount\0" // IR(21903, 13)
  "DependencyArrayINTEL\0" // IR(21916, 21)
  "LoopCoalesceINTEL\0" // IR(21937, 18)
  "MaxInterleavingINTEL\0" // IR(21955, 21)
  "SpeculatedIterationsINTEL\0" // IR(21976, 26)
  "NoFusionINTEL\0" // IR(22002, 14)
  "LoopCountINTEL\0" // IR(22016, 15)
  "MaxReinvocationDelayINTEL\0" // IR(22031, 26)
  "MatrixASignedComponentsINTEL\0" // IR(22057, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(22086, 29)
  "MatrixCBFloat16INTEL\0" // IR(22115, 21)
  "MatrixResultBFloat16INTEL\0" // IR(22136, 26)
  "MatrixAPackedInt8INTEL\0" // IR(22162, 23)
  "MatrixBPackedInt8INTEL\0" // IR(22185, 23)
  "MatrixAPackedInt4INTEL\0" // IR(22208, 23)
  "MatrixBPackedInt4INTEL\0" // IR(22231, 23)
  "MatrixATF32INTEL\0" // IR(22254, 17)
  "MatrixBTF32INTEL\0" // IR(22271, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(22288, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(22314, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(22340, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(22367, 27)
  "Aligned\0" // IR(22394, 8)
  "MakePointerAvailable\0" // IR(22402, 21)
  "MakePointerAvailableKHR\0" // IR(22423, 24)
  "MakePointerVisible\0" // IR(22447, 19)
  "MakePointerVisibleKHR\0" // IR(22466, 22)
  "NonPrivatePointer\0" // IR(22488, 18)
  "NonPrivatePointerKHR\0" // IR(22506, 21)
  "AliasScopeINTELMask\0" // IR(22527, 20)
  "NoAliasINTELMask\0" // IR(22547, 17)
  "Simple\0" // IR(22564, 7)
  "GLSL450\0" // IR(22571, 8)
  "OpenCL\0" // IR(22579, 7)
  "Vulkan\0" // IR(22586, 7)
  "VulkanKHR\0" // IR(22593, 10)
  "Relaxed\0" // IR(22603, 8)
  "Acquire\0" // IR(22611, 8)
  "Release\0" // IR(22619, 8)
  "AcquireRelease\0" // IR(22627, 15)
  "SequentiallyConsistent\0" // IR(22642, 23)
  "UniformMemory\0" // IR(22665, 14)
  "SubgroupMemory\0" // IR(22679, 15)
  "WorkgroupMemory\0" // IR(22694, 16)
  "CrossWorkgroupMemory\0" // IR(22710, 21)
  "AtomicCounterMemory\0" // IR(22731, 20)
  "ImageMemory\0" // IR(22751, 12)
  "OutputMemory\0" // IR(22763, 13)
  "OutputMemoryKHR\0" // IR(22776, 16)
  "MakeAvailable\0" // IR(22792, 14)
  "MakeAvailableKHR\0" // IR(22806, 17)
  "MakeVisible\0" // IR(22823, 12)
  "MakeVisibleKHR\0" // IR(22835, 15)
  "AutoINTEL\0" // IR(22850, 10)
  "WRAP\0" // IR(22860, 5)
  "SAT\0" // IR(22865, 4)
  "SAT_ZERO\0" // IR(22869, 9)
  "SAT_SYM\0" // IR(22878, 8)
  "PackedVectorFormat4x8Bit\0" // IR(22886, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(22911, 28)
  "TRN\0" // IR(22939, 4)
  "TRN_ZERO\0" // IR(22943, 9)
  "RND\0" // IR(22952, 4)
  "RND_ZERO\0" // IR(22956, 9)
  "RND_INF\0" // IR(22965, 8)
  "RND_MIN_INF\0" // IR(22973, 12)
  "RND_CONV\0" // IR(22985, 9)
  "RND_CONV_ODD\0" // IR(22994, 13)
  "RobustnessPerComponentNV\0" // IR(23007, 25)
  "RobustnessPerElementNV\0" // IR(23032, 23)
  "OpaqueKHR\0" // IR(23055, 10)
  "NoOpaqueKHR\0" // IR(23065, 12)
  "TerminateOnFirstHitKHR\0" // IR(23077, 23)
  "SkipClosestHitShaderKHR\0" // IR(23100, 24)
  "CullBackFacingTrianglesKHR\0" // IR(23124, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(23151, 28)
  "CullOpaqueKHR\0" // IR(23179, 14)
  "CullNoOpaqueKHR\0" // IR(23193, 16)
  "SkipTrianglesKHR\0" // IR(23209, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(23226, 24)
  "SkipAABBsKHR\0" // IR(23250, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(23263, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(23293, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(23334, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(23371, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(23408, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(23449, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(23491, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(23524, 33)
  "ClampToEdge\0" // IR(23557, 12)
  "Clamp\0" // IR(23569, 6)
  "Repeat\0" // IR(23575, 7)
  "RepeatMirrored\0" // IR(23582, 15)
  "Nearest\0" // IR(23597, 8)
  "Linear\0" // IR(23605, 7)
  "Unknown\0" // IR(23612, 8)
  "Rgba32f\0" // IR(23620, 8)
  "Rgba16f\0" // IR(23628, 8)
  "R32f\0" // IR(23636, 5)
  "Rgba8\0" // IR(23641, 6)
  "Rgba8Snorm\0" // IR(23647, 11)
  "Rg32f\0" // IR(23658, 6)
  "Rg16f\0" // IR(23664, 6)
  "R11fG11fB10f\0" // IR(23670, 13)
  "R16f\0" // IR(23683, 5)
  "Rgba16\0" // IR(23688, 7)
  "Rgb10A2\0" // IR(23695, 8)
  "Rg16\0" // IR(23703, 5)
  "Rg8\0" // IR(23708, 4)
  "R16\0" // IR(23712, 4)
  "R8\0" // IR(23716, 3)
  "Rgba16Snorm\0" // IR(23719, 12)
  "Rg16Snorm\0" // IR(23731, 10)
  "Rg8Snorm\0" // IR(23741, 9)
  "R16Snorm\0" // IR(23750, 9)
  "R8Snorm\0" // IR(23759, 8)
  "Rgba32i\0" // IR(23767, 8)
  "Rgba16i\0" // IR(23775, 8)
  "Rgba8i\0" // IR(23783, 7)
  "R32i\0" // IR(23790, 5)
  "Rg32i\0" // IR(23795, 6)
  "Rg16i\0" // IR(23801, 6)
  "Rg8i\0" // IR(23807, 5)
  "R16i\0" // IR(23812, 5)
  "R8i\0" // IR(23817, 4)
  "Rgba32ui\0" // IR(23821, 9)
  "Rgba16ui\0" // IR(23830, 9)
  "Rgba8ui\0" // IR(23839, 8)
  "R32ui\0" // IR(23847, 6)
  "Rgb10a2ui\0" // IR(23853, 10)
  "Rg32ui\0" // IR(23863, 7)
  "Rg16ui\0" // IR(23870, 7)
  "Rg8ui\0" // IR(23877, 6)
  "R16ui\0" // IR(23883, 6)
  "R8ui\0" // IR(23889, 5)
  "R64ui\0" // IR(23894, 6)
  "R64i\0" // IR(23900, 5)
  "CrossDevice\0" // IR(23905, 12)
  "Device\0" // IR(23917, 7)
  "Workgroup\0" // IR(23924, 10)
  "Subgroup\0" // IR(23934, 9)
  "Invocation\0" // IR(23943, 11)
  "QueueFamily\0" // IR(23954, 12)
  "QueueFamilyKHR\0" // IR(23966, 15)
  "ShaderCallKHR\0" // IR(23981, 14)
  "Flatten\0" // IR(23995, 8)
  "DontFlatten\0" // IR(24003, 12)
  "IdentifierPossibleDuplicates\0" // IR(24015, 29)
  "FlagUnknownPhysicalLayout\0" // IR(24044, 26)
  "ESSL\0" // IR(24070, 5)
  "GLSL\0" // IR(24075, 5)
  "OpenCL_C\0" // IR(24080, 9)
  "OpenCL_CPP\0" // IR(24089, 11)
  "HLSL\0" // IR(24100, 5)
  "CPP_for_OpenCL\0" // IR(24105, 15)
  "SYCL\0" // IR(24120, 5)
  "HERO_C\0" // IR(24125, 7)
  "NZSL\0" // IR(24132, 5)
  "WGSL\0" // IR(24137, 5)
  "Slang\0" // IR(24142, 6)
  "Zig\0" // IR(24148, 4)
  "Rust\0" // IR(24152, 5)
  "UniformConstant\0" // IR(24157, 16)
  "Input\0" // IR(24173, 6)
  "Output\0" // IR(24179, 7)
  "CrossWorkgroup\0" // IR(24186, 15)
  "Private\0" // IR(24201, 8)
  "Function\0" // IR(24209, 9)
  "Generic\0" // IR(24218, 8)
  "PushConstant\0" // IR(24226, 13)
  "AtomicCounter\0" // IR(24239, 14)
  "Image\0" // IR(24253, 6)
  "StorageBuffer\0" // IR(24259, 14)
  "TileImageEXT\0" // IR(24273, 13)
  "TileAttachmentQCOM\0" // IR(24286, 19)
  "NodePayloadAMDX\0" // IR(24305, 16)
  "CallableDataKHR\0" // IR(24321, 16)
  "CallableDataNV\0" // IR(24337, 15)
  "IncomingCallableDataKHR\0" // IR(24352, 24)
  "IncomingCallableDataNV\0" // IR(24376, 23)
  "RayPayloadKHR\0" // IR(24399, 14)
  "RayPayloadNV\0" // IR(24413, 13)
  "HitAttributeKHR\0" // IR(24426, 16)
  "HitAttributeNV\0" // IR(24442, 15)
  "IncomingRayPayloadKHR\0" // IR(24457, 22)
  "IncomingRayPayloadNV\0" // IR(24479, 21)
  "ShaderRecordBufferKHR\0" // IR(24500, 22)
  "ShaderRecordBufferNV\0" // IR(24522, 21)
  "PhysicalStorageBuffer\0" // IR(24543, 22)
  "PhysicalStorageBufferEXT\0" // IR(24565, 25)
  "HitObjectAttributeNV\0" // IR(24590, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(24611, 24)
  "CodeSectionINTEL\0" // IR(24635, 17)
  "DeviceOnlyINTEL\0" // IR(24652, 16)
  "HostOnlyINTEL\0" // IR(24668, 14)
  "WriteThroughINTEL\0" // IR(24682, 18)
  "WriteBackINTEL\0" // IR(24700, 15)
  "TensorView\0" // IR(24715, 11)
  "DecodeFunc\0" // IR(24726, 11)
  "Undefined\0" // IR(24737, 10)
  "NoneARM\0" // IR(24747, 8)
  "NontemporalARM\0" // IR(24755, 15)
  "OutOfBoundsValueARM\0" // IR(24770, 20)
  "MakeElementAvailableARM\0" // IR(24790, 24)
  "MakeElementVisibleARM\0" // IR(24814, 22)
  "NonPrivateElementARM\0" // IR(24836, 21)
  "Nop\0" // IR(24857, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(24861, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(24886, 27)
  "Undef\0" // IR(24913, 6)
  "SourceContinued\0" // IR(24919, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(24935, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(24968, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(24997, 41)
  "Source\0" // IR(25038, 7)
  "SourceExtension\0" // IR(25045, 16)
  "Name\0" // IR(25061, 5)
  "MemberName\0" // IR(25066, 11)
  "String\0" // IR(25077, 7)
  "Line\0" // IR(25084, 5)
  "Extension\0" // IR(25089, 10)
  "ExtInstImport\0" // IR(25099, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(25113, 46)
  "ExtInst\0" // IR(25159, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(25167, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(25201, 30)
  "MemoryModel\0" // IR(25231, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(25243, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(25276, 29)
  "EntryPoint\0" // IR(25305, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(25316, 32)
  "ExecutionMode\0" // IR(25348, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(25362, 28)
  "Capability\0" // IR(25390, 11)
  "TypeVoid\0" // IR(25401, 9)
  "TypeBool\0" // IR(25410, 9)
  "TypeInt\0" // IR(25419, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(25427, 37)
  "TypeFloat\0" // IR(25464, 10)
  "TypeVector\0" // IR(25474, 11)
  "TypeMatrix\0" // IR(25485, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(25496, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(25528, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(25566, 43)
  "TypeImage\0" // IR(25609, 10)
  "TypeSampler\0" // IR(25619, 12)
  "TypeSampledImage\0" // IR(25631, 17)
  "TypeArray\0" // IR(25648, 10)
  "TypeRuntimeArray\0" // IR(25658, 17)
  "TypeStruct\0" // IR(25675, 11)
  "TypeOpaque\0" // IR(25686, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(25697, 31)
  "TypePointer\0" // IR(25728, 12)
  "TypeFunction\0" // IR(25740, 13)
  "TypeEvent\0" // IR(25753, 10)
  "TypeDeviceEvent\0" // IR(25763, 16)
  "TypeReserveId\0" // IR(25779, 14)
  "TypeQueue\0" // IR(25793, 10)
  "TypePipe\0" // IR(25803, 9)
  "TypeForwardPointer\0" // IR(25812, 19)
  "ConstantTrue\0" // IR(25831, 13)
  "ConstantFalse\0" // IR(25844, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(25858, 38)
  "ConstantComposite\0" // IR(25896, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(25914, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(25955, 37)
  "ConstantSampler\0" // IR(25992, 16)
  "ConstantNull\0" // IR(26008, 13)
  "SpecConstantTrue\0" // IR(26021, 17)
  "SpecConstantFalse\0" // IR(26038, 18)
  "SpecConstant\0" // IR(26056, 13)
  "SpecConstantComposite\0" // IR(26069, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(26091, 41)
  "SpecConstantOp\0" // IR(26132, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(26147, 34)
  "FunctionParameter\0" // IR(26181, 18)
  "FunctionEnd\0" // IR(26199, 12)
  "FunctionCall\0" // IR(26211, 13)
  "Variable\0" // IR(26224, 9)
  "ImageTexelPointer\0" // IR(26233, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(26251, 40)
  "Load\0" // IR(26291, 5)
  "Store\0" // IR(26296, 6)
  "CopyMemory\0" // IR(26302, 11)
  "CopyMemorySized\0" // IR(26313, 16)
  "AccessChain\0" // IR(26329, 12)
  "InBoundsAccessChain\0" // IR(26341, 20)
  "PtrAccessChain\0" // IR(26361, 15)
  "ArrayLength\0" // IR(26376, 12)
  "GenericPtrMemSemantics\0" // IR(26388, 23)
  "InBoundsPtrAccessChain\0" // IR(26411, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(26434, 28)
  "Decorate\0" // IR(26462, 9)
  "MemberDecorate\0" // IR(26471, 15)
  "DecorationGroup\0" // IR(26486, 16)
  "GroupDecorate\0" // IR(26502, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(26516, 45)
  "GroupMemberDecorate\0" // IR(26561, 20)
  "VectorExtractDynamic\0" // IR(26581, 21)
  "VectorInsertDynamic\0" // IR(26602, 20)
  "VectorShuffle\0" // IR(26622, 14)
  "CompositeConstruct\0" // IR(26636, 19)
  "CompositeExtract\0" // IR(26655, 17)
  "CompositeInsert\0" // IR(26672, 16)
  "CopyObject\0" // IR(26688, 11)
  "Transpose\0" // IR(26699, 10)
  "SampledImage\0" // IR(26709, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(26722, 32)
  "ImageSampleImplicitLod\0" // IR(26754, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(26777, 23)
  "ImageSampleExplicitLod\0" // IR(26800, 23)
  "ImageSampleDrefImplicitLod\0" // IR(26823, 27)
  "ImageSampleDrefExplicitLod\0" // IR(26850, 27)
  "ImageSampleProjImplicitLod\0" // IR(26877, 27)
  "ImageSampleProjExplicitLod\0" // IR(26904, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(26931, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(26962, 31)
  "ImageFetch\0" // IR(26993, 11)
  "ImageGather\0" // IR(27004, 12)
  "ImageDrefGather\0" // IR(27016, 16)
  "ImageRead\0" // IR(27032, 10)
  "ImageWrite\0" // IR(27042, 11)
  "ImageQueryFormat\0" // IR(27053, 17)
  "ImageQueryOrder\0" // IR(27070, 16)
  "ImageQuerySizeLod\0" // IR(27086, 18)
  "ImageQuerySize\0" // IR(27104, 15)
  "ImageQueryLod\0" // IR(27119, 14)
  "ImageQueryLevels\0" // IR(27133, 17)
  "ImageQuerySamples\0" // IR(27150, 18)
  "ConvertFToU\0" // IR(27168, 12)
  "ConvertFToS\0" // IR(27180, 12)
  "ConvertSToF\0" // IR(27192, 12)
  "ConvertUToF\0" // IR(27204, 12)
  "UConvert\0" // IR(27216, 9)
  "SConvert\0" // IR(27225, 9)
  "FConvert\0" // IR(27234, 9)
  "QuantizeToF16\0" // IR(27243, 14)
  "ConvertPtrToU\0" // IR(27257, 14)
  "SatConvertSToU\0" // IR(27271, 15)
  "SatConvertUToS\0" // IR(27286, 15)
  "ConvertUToPtr\0" // IR(27301, 14)
  "PtrCastToGeneric\0" // IR(27315, 17)
  "GenericCastToPtr\0" // IR(27332, 17)
  "GenericCastToPtrExplicit\0" // IR(27349, 25)
  "Bitcast\0" // IR(27374, 8)
  "SNegate\0" // IR(27382, 8)
  "FNegate\0" // IR(27390, 8)
  "IAdd\0" // IR(27398, 5)
  "FAdd\0" // IR(27403, 5)
  "ISub\0" // IR(27408, 5)
  "FSub\0" // IR(27413, 5)
  "IMul\0" // IR(27418, 5)
  "FMul\0" // IR(27423, 5)
  "UDiv\0" // IR(27428, 5)
  "SDiv\0" // IR(27433, 5)
  "FDiv\0" // IR(27438, 5)
  "UMod\0" // IR(27443, 5)
  "SRem\0" // IR(27448, 5)
  "SMod\0" // IR(27453, 5)
  "FRem\0" // IR(27458, 5)
  "FMod\0" // IR(27463, 5)
  "VectorTimesScalar\0" // IR(27468, 18)
  "MatrixTimesScalar\0" // IR(27486, 18)
  "VectorTimesMatrix\0" // IR(27504, 18)
  "MatrixTimesVector\0" // IR(27522, 18)
  "MatrixTimesMatrix\0" // IR(27540, 18)
  "OuterProduct\0" // IR(27558, 13)
  "Dot\0" // IR(27571, 4)
  "IAddCarry\0" // IR(27575, 10)
  "ISubBorrow\0" // IR(27585, 11)
  "UMulExtended\0" // IR(27596, 13)
  "SMulExtended\0" // IR(27609, 13)
  "Any\0" // IR(27622, 4)
  "All\0" // IR(27626, 4)
  "IsNan\0" // IR(27630, 6)
  "IsInf\0" // IR(27636, 6)
  "IsFinite\0" // IR(27642, 9)
  "IsNormal\0" // IR(27651, 9)
  "SignBitSet\0" // IR(27660, 11)
  "LessOrGreater\0" // IR(27671, 14)
  "Ordered\0" // IR(27685, 8)
  "Unordered\0" // IR(27693, 10)
  "LogicalEqual\0" // IR(27703, 13)
  "LogicalNotEqual\0" // IR(27716, 16)
  "LogicalOr\0" // IR(27732, 10)
  "LogicalAnd\0" // IR(27742, 11)
  "LogicalNot\0" // IR(27753, 11)
  "Select\0" // IR(27764, 7)
  "IEqual\0" // IR(27771, 7)
  "INotEqual\0" // IR(27778, 10)
  "UGreaterThan\0" // IR(27788, 13)
  "SGreaterThan\0" // IR(27801, 13)
  "UGreaterThanEqual\0" // IR(27814, 18)
  "SGreaterThanEqual\0" // IR(27832, 18)
  "ULessThan\0" // IR(27850, 10)
  "SLessThan\0" // IR(27860, 10)
  "ULessThanEqual\0" // IR(27870, 15)
  "SLessThanEqual\0" // IR(27885, 15)
  "FOrdEqual\0" // IR(27900, 10)
  "FUnordEqual\0" // IR(27910, 12)
  "FOrdNotEqual\0" // IR(27922, 13)
  "FUnordNotEqual\0" // IR(27935, 15)
  "FOrdLessThan\0" // IR(27950, 13)
  "FUnordLessThan\0" // IR(27963, 15)
  "FOrdGreaterThan\0" // IR(27978, 16)
  "FUnordGreaterThan\0" // IR(27994, 18)
  "FOrdLessThanEqual\0" // IR(28012, 18)
  "FUnordLessThanEqual\0" // IR(28030, 20)
  "FOrdGreaterThanEqual\0" // IR(28050, 21)
  "FUnordGreaterThanEqual\0" // IR(28071, 23)
  "ShiftRightLogical\0" // IR(28094, 18)
  "ShiftRightArithmetic\0" // IR(28112, 21)
  "ShiftLeftLogical\0" // IR(28133, 17)
  "BitwiseOr\0" // IR(28150, 10)
  "BitwiseXor\0" // IR(28160, 11)
  "BitwiseAnd\0" // IR(28171, 11)
  "Not\0" // IR(28182, 4)
  "BitFieldInsert\0" // IR(28186, 15)
  "BitFieldSExtract\0" // IR(28201, 17)
  "BitFieldUExtract\0" // IR(28218, 17)
  "BitReverse\0" // IR(28235, 11)
  "BitCount\0" // IR(28246, 9)
  "DPdx\0" // IR(28255, 5)
  "DPdy\0" // IR(28260, 5)
  "Fwidth\0" // IR(28265, 7)
  "DPdxFine\0" // IR(28272, 9)
  "DPdyFine\0" // IR(28281, 9)
  "FwidthFine\0" // IR(28290, 11)
  "DPdxCoarse\0" // IR(28301, 11)
  "DPdyCoarse\0" // IR(28312, 11)
  "FwidthCoarse\0" // IR(28323, 13)
  "EmitVertex\0" // IR(28336, 11)
  "EndPrimitive\0" // IR(28347, 13)
  "EmitStreamVertex\0" // IR(28360, 17)
  "EndStreamPrimitive\0" // IR(28377, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(28396, 37)
  "ControlBarrier\0" // IR(28433, 15)
  "MemoryBarrier\0" // IR(28448, 14)
  "AtomicLoad\0" // IR(28462, 11)
  "AtomicStore\0" // IR(28473, 12)
  "AtomicExchange\0" // IR(28485, 15)
  "AtomicCompareExchange\0" // IR(28500, 22)
  "AtomicCompareExchangeWeak\0" // IR(28522, 26)
  "AtomicIIncrement\0" // IR(28548, 17)
  "AtomicIDecrement\0" // IR(28565, 17)
  "AtomicIAdd\0" // IR(28582, 11)
  "AtomicISub\0" // IR(28593, 11)
  "AtomicSMin\0" // IR(28604, 11)
  "AtomicUMin\0" // IR(28615, 11)
  "AtomicSMax\0" // IR(28626, 11)
  "AtomicUMax\0" // IR(28637, 11)
  "AtomicAnd\0" // IR(28648, 10)
  "AtomicOr\0" // IR(28658, 9)
  "AtomicXor\0" // IR(28667, 10)
  "Phi\0" // IR(28677, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(28681, 30)
  "LoopMerge\0" // IR(28711, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(28721, 35)
  "SelectionMerge\0" // IR(28756, 15)
  "Label\0" // IR(28771, 6)
  "Branch\0" // IR(28777, 7)
  "BranchConditional\0" // IR(28784, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(28802, 45)
  "Switch\0" // IR(28847, 7)
  "Kill\0" // IR(28854, 5)
  "Return\0" // IR(28859, 7)
  "ReturnValue\0" // IR(28866, 12)
  "Unreachable\0" // IR(28878, 12)
  "LifetimeStart\0" // IR(28890, 14)
  "LifetimeStop\0" // IR(28904, 13)
  "GroupAsyncCopy\0" // IR(28917, 15)
  "GroupWaitEvents\0" // IR(28932, 16)
  "GroupAll\0" // IR(28948, 9)
  "GroupAny\0" // IR(28957, 9)
  "GroupBroadcast\0" // IR(28966, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(28981, 33)
  "GroupIAdd\0" // IR(29014, 10)
  "GroupFAdd\0" // IR(29024, 10)
  "GroupFMin\0" // IR(29034, 10)
  "GroupUMin\0" // IR(29044, 10)
  "GroupSMin\0" // IR(29054, 10)
  "GroupFMax\0" // IR(29064, 10)
  "GroupUMax\0" // IR(29074, 10)
  "GroupSMax\0" // IR(29084, 10)
  "ReadPipe\0" // IR(29094, 9)
  "WritePipe\0" // IR(29103, 10)
  "ReservedReadPipe\0" // IR(29113, 17)
  "ReservedWritePipe\0" // IR(29130, 18)
  "ReserveReadPipePackets\0" // IR(29148, 23)
  "ReserveWritePipePackets\0" // IR(29171, 24)
  "CommitReadPipe\0" // IR(29195, 15)
  "CommitWritePipe\0" // IR(29210, 16)
  "IsValidReserveId\0" // IR(29226, 17)
  "GetNumPipePackets\0" // IR(29243, 18)
  "GetMaxPipePackets\0" // IR(29261, 18)
  "GroupReserveReadPipePackets\0" // IR(29279, 28)
  "GroupReserveWritePipePackets\0" // IR(29307, 29)
  "GroupCommitReadPipe\0" // IR(29336, 20)
  "GroupCommitWritePipe\0" // IR(29356, 21)
  "EnqueueMarker\0" // IR(29377, 14)
  "EnqueueKernel\0" // IR(29391, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(29405, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(29435, 32)
  "GetKernelWorkGroupSize\0" // IR(29467, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(29490, 40)
  "RetainEvent\0" // IR(29530, 12)
  "ReleaseEvent\0" // IR(29542, 13)
  "CreateUserEvent\0" // IR(29555, 16)
  "IsValidEvent\0" // IR(29571, 13)
  "SetUserEventStatus\0" // IR(29584, 19)
  "CaptureEventProfilingInfo\0" // IR(29603, 26)
  "GetDefaultQueue\0" // IR(29629, 16)
  "BuildNDRange\0" // IR(29645, 13)
  "ImageSparseSampleImplicitLod\0" // IR(29658, 29)
  "ImageSparseSampleExplicitLod\0" // IR(29687, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(29716, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(29749, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(29782, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(29815, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(29848, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(29885, 37)
  "ImageSparseFetch\0" // IR(29922, 17)
  "ImageSparseGather\0" // IR(29939, 18)
  "ImageSparseDrefGather\0" // IR(29957, 22)
  "ImageSparseTexelsResident\0" // IR(29979, 26)
  "NoLine\0" // IR(30005, 7)
  "AtomicFlagTestAndSet\0" // IR(30012, 21)
  "AtomicFlagClear\0" // IR(30033, 16)
  "ImageSparseRead\0" // IR(30049, 16)
  "SizeOf\0" // IR(30065, 7)
  "TypePipeStorage\0" // IR(30072, 16)
  "ConstantPipeStorage\0" // IR(30088, 20)
  "CreatePipeFromPipeStorage\0" // IR(30108, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(30134, 35)
  "GetKernelMaxNumSubgroups\0" // IR(30169, 25)
  "TypeNamedBarrier\0" // IR(30194, 17)
  "NamedBarrierInitialize\0" // IR(30211, 23)
  "MemoryNamedBarrier\0" // IR(30234, 19)
  "ModuleProcessed\0" // IR(30253, 16)
  "ExecutionModeId\0" // IR(30269, 16)
  "DecorateId\0" // IR(30285, 11)
  "GroupNonUniformElect\0" // IR(30296, 21)
  "GroupNonUniformAll\0" // IR(30317, 19)
  "GroupNonUniformAny\0" // IR(30336, 19)
  "GroupNonUniformAllEqual\0" // IR(30355, 24)
  "GroupNonUniformBroadcast\0" // IR(30379, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(30404, 30)
  "GroupNonUniformInverseBallot\0" // IR(30434, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(30463, 32)
  "GroupNonUniformBallotBitCount\0" // IR(30495, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(30525, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(30554, 29)
  "GroupNonUniformShuffleXor\0" // IR(30583, 26)
  "GroupNonUniformShuffleUp\0" // IR(30609, 25)
  "GroupNonUniformShuffleDown\0" // IR(30634, 27)
  "GroupNonUniformIAdd\0" // IR(30661, 20)
  "GroupNonUniformFAdd\0" // IR(30681, 20)
  "GroupNonUniformIMul\0" // IR(30701, 20)
  "GroupNonUniformFMul\0" // IR(30721, 20)
  "GroupNonUniformSMin\0" // IR(30741, 20)
  "GroupNonUniformUMin\0" // IR(30761, 20)
  "GroupNonUniformFMin\0" // IR(30781, 20)
  "GroupNonUniformSMax\0" // IR(30801, 20)
  "GroupNonUniformUMax\0" // IR(30821, 20)
  "GroupNonUniformFMax\0" // IR(30841, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(30861, 26)
  "GroupNonUniformBitwiseOr\0" // IR(30887, 25)
  "GroupNonUniformBitwiseXor\0" // IR(30912, 26)
  "GroupNonUniformLogicalAnd\0" // IR(30938, 26)
  "GroupNonUniformLogicalOr\0" // IR(30964, 25)
  "GroupNonUniformLogicalXor\0" // IR(30989, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(31015, 29)
  "GroupNonUniformQuadSwap\0" // IR(31044, 24)
  "CopyLogical\0" // IR(31068, 12)
  "PtrEqual\0" // IR(31080, 9)
  "PtrNotEqual\0" // IR(31089, 12)
  "PtrDiff\0" // IR(31101, 8)
  "ColorAttachmentReadEXT\0" // IR(31109, 23)
  "DepthAttachmentReadEXT\0" // IR(31132, 23)
  "StencilAttachmentReadEXT\0" // IR(31155, 25)
  "TypeTensorARM\0" // IR(31180, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(31194, 42)
  "TensorReadARM\0" // IR(31236, 14)
  "TensorWriteARM\0" // IR(31250, 15)
  "TensorQuerySizeARM\0" // IR(31265, 19)
  "TerminateInvocation\0" // IR(31284, 20)
  "TypeUntypedPointerKHR\0" // IR(31304, 22)
  "UntypedVariableKHR\0" // IR(31326, 19)
  "UntypedAccessChainKHR\0" // IR(31345, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(31367, 30)
  "SubgroupFirstInvocationKHR\0" // IR(31397, 27)
  "UntypedPtrAccessChainKHR\0" // IR(31424, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(31449, 33)
  "UntypedArrayLengthKHR\0" // IR(31482, 22)
  "UntypedPrefetchKHR\0" // IR(31504, 19)
  "SubgroupAllKHR\0" // IR(31523, 15)
  "SubgroupAnyKHR\0" // IR(31538, 15)
  "SubgroupAllEqualKHR\0" // IR(31553, 20)
  "SubgroupReadInvocationKHR\0" // IR(31573, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(31599, 26)
  "TraceRayKHR\0" // IR(31625, 12)
  "ExecuteCallableKHR\0" // IR(31637, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(31656, 35)
  "IgnoreIntersectionKHR\0" // IR(31691, 22)
  "TerminateRayKHR\0" // IR(31713, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(31729, 47)
  "SDot\0" // IR(31776, 5)
  "SDotKHR\0" // IR(31781, 8)
  "UDot\0" // IR(31789, 5)
  "UDotKHR\0" // IR(31794, 8)
  "SUDot\0" // IR(31802, 6)
  "SUDotKHR\0" // IR(31808, 9)
  "SDotAccSat\0" // IR(31817, 11)
  "SDotAccSatKHR\0" // IR(31828, 14)
  "UDotAccSat\0" // IR(31842, 11)
  "UDotAccSatKHR\0" // IR(31853, 14)
  "SUDotAccSat\0" // IR(31867, 12)
  "SUDotAccSatKHR\0" // IR(31879, 15)
  "TypeCooperativeMatrixKHR\0" // IR(31894, 25)
  "CooperativeMatrixLoadKHR\0" // IR(31919, 25)
  "CooperativeMatrixStoreKHR\0" // IR(31944, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(31970, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(32024, 27)
  "CooperativeMatrixLengthKHR\0" // IR(32051, 27)
  "ConstantCompositeReplicateEXT\0" // IR(32078, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(32108, 34)
  "CompositeConstructReplicateEXT\0" // IR(32142, 31)
  "TypeRayQueryKHR\0" // IR(32173, 16)
  "RayQueryInitializeKHR\0" // IR(32189, 22)
  "RayQueryTerminateKHR\0" // IR(32211, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(32232, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(32264, 31)
  "RayQueryProceedKHR\0" // IR(32295, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(32314, 31)
  "ImageSampleWeightedQCOM\0" // IR(32345, 24)
  "ImageBoxFilterQCOM\0" // IR(32369, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(32388, 23)
  "ImageBlockMatchSADQCOM\0" // IR(32411, 23)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(32434, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(32463, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(32492, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(32521, 29)
  "GroupIAddNonUniformAMD\0" // IR(32550, 23)
  "GroupFAddNonUniformAMD\0" // IR(32573, 23)
  "GroupFMinNonUniformAMD\0" // IR(32596, 23)
  "GroupUMinNonUniformAMD\0" // IR(32619, 23)
  "GroupSMinNonUniformAMD\0" // IR(32642, 23)
  "GroupFMaxNonUniformAMD\0" // IR(32665, 23)
  "GroupUMaxNonUniformAMD\0" // IR(32688, 23)
  "GroupSMaxNonUniformAMD\0" // IR(32711, 23)
  "FragmentMaskFetchAMD\0" // IR(32734, 21)
  "FragmentFetchAMD\0" // IR(32755, 17)
  "ReadClockKHR\0" // IR(32772, 13)
  "AllocateNodePayloadsAMDX\0" // IR(32785, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(32810, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(32834, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(32859, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(32888, 27)
  "IsNodePayloadValidAMDX\0" // IR(32915, 23)
  "ConstantStringAMDX\0" // IR(32938, 19)
  "SpecConstantStringAMDX\0" // IR(32957, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(32980, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(33006, 26)
  "HitObjectRecordHitMotionNV\0" // IR(33032, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(33059, 36)
  "HitObjectRecordMissMotionNV\0" // IR(33095, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(33123, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(33151, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(33179, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(33212, 30)
  "HitObjectTraceRayMotionNV\0" // IR(33242, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(33268, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(33307, 44)
  "HitObjectRecordEmptyNV\0" // IR(33351, 23)
  "HitObjectTraceRayNV\0" // IR(33374, 20)
  "HitObjectRecordHitNV\0" // IR(33394, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(33415, 30)
  "HitObjectRecordMissNV\0" // IR(33445, 22)
  "HitObjectExecuteShaderNV\0" // IR(33467, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(33492, 26)
  "HitObjectGetAttributesNV\0" // IR(33518, 25)
  "HitObjectGetHitKindNV\0" // IR(33543, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(33565, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(33594, 28)
  "HitObjectGetInstanceIdNV\0" // IR(33622, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(33647, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(33681, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(33713, 29)
  "HitObjectGetRayTMaxNV\0" // IR(33742, 22)
  "HitObjectGetRayTMinNV\0" // IR(33764, 22)
  "HitObjectIsEmptyNV\0" // IR(33786, 19)
  "HitObjectIsHitNV\0" // IR(33805, 17)
  "HitObjectIsMissNV\0" // IR(33822, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(33840, 29)
  "ReorderThreadWithHintNV\0" // IR(33869, 24)
  "TypeHitObjectNV\0" // IR(33893, 16)
  "ImageSampleFootprintNV\0" // IR(33909, 23)
  "TypeCooperativeVectorNV\0" // IR(33932, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(33956, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(33985, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(34027, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(34066, 32)
  "CooperativeMatrixConvertNV\0" // IR(34098, 27)
  "EmitMeshTasksEXT\0" // IR(34125, 17)
  "SetMeshOutputsEXT\0" // IR(34142, 18)
  "GroupNonUniformPartitionNV\0" // IR(34160, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(34187, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(34220, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(34255, 38)
  "CooperativeVectorLoadNV\0" // IR(34293, 24)
  "CooperativeVectorStoreNV\0" // IR(34317, 25)
  "ReportIntersectionKHR\0" // IR(34342, 22)
  "ReportIntersectionNV\0" // IR(34364, 21)
  "IgnoreIntersectionNV\0" // IR(34385, 21)
  "TerminateRayNV\0" // IR(34406, 15)
  "TraceNV\0" // IR(34421, 8)
  "TraceMotionNV\0" // IR(34429, 14)
  "TraceRayMotionNV\0" // IR(34443, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(34460, 50)
  "TypeAccelerationStructureKHR\0" // IR(34510, 29)
  "TypeAccelerationStructureNV\0" // IR(34539, 28)
  "ExecuteCallableNV\0" // IR(34567, 18)
  "RayQueryGetClusterIdNV\0" // IR(34585, 23)
  "HitObjectGetClusterIdNV\0" // IR(34608, 24)
  "TypeCooperativeMatrixNV\0" // IR(34632, 24)
  "CooperativeMatrixLoadNV\0" // IR(34656, 24)
  "CooperativeMatrixStoreNV\0" // IR(34680, 25)
  "CooperativeMatrixMulAddNV\0" // IR(34705, 26)
  "CooperativeMatrixLengthNV\0" // IR(34731, 26)
  "BeginInvocationInterlockEXT\0" // IR(34757, 28)
  "EndInvocationInterlockEXT\0" // IR(34785, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(34811, 43)
  "CooperativeMatrixReduceNV\0" // IR(34854, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(34880, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(34911, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(34955, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(34985, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(35016, 32)
  "TypeTensorLayoutNV\0" // IR(35048, 19)
  "TypeTensorViewNV\0" // IR(35067, 17)
  "CreateTensorLayoutNV\0" // IR(35084, 21)
  "TensorLayoutSetDimensionNV\0" // IR(35105, 27)
  "TensorLayoutSetStrideNV\0" // IR(35132, 24)
  "TensorLayoutSliceNV\0" // IR(35156, 20)
  "TensorLayoutSetClampValueNV\0" // IR(35176, 28)
  "CreateTensorViewNV\0" // IR(35204, 19)
  "TensorViewSetDimensionNV\0" // IR(35223, 25)
  "TensorViewSetStrideNV\0" // IR(35248, 22)
  "IsHelperInvocationEXT\0" // IR(35270, 22)
  "TensorViewSetClipNV\0" // IR(35292, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(35312, 27)
  "CooperativeMatrixTransposeNV\0" // IR(35339, 29)
  "ConvertUToImageNV\0" // IR(35368, 18)
  "ConvertUToSamplerNV\0" // IR(35386, 20)
  "ConvertImageToUNV\0" // IR(35406, 18)
  "ConvertSamplerToUNV\0" // IR(35424, 20)
  "ConvertUToSampledImageNV\0" // IR(35444, 25)
  "ConvertSampledImageToUNV\0" // IR(35469, 25)
  "SamplerImageAddressingModeNV\0" // IR(35494, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(35523, 52)
  "RawAccessChainNV\0" // IR(35575, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(35592, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(35632, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(35670, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(35708, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(35742, 37)
  "HitObjectGetSpherePositionNV\0" // IR(35779, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(35808, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(35835, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(35862, 23)
  "HitObjectIsSphereHitNV\0" // IR(35885, 23)
  "HitObjectIsLSSHitNV\0" // IR(35908, 20)
  "RayQueryIsSphereHitNV\0" // IR(35928, 22)
  "RayQueryIsLSSHitNV\0" // IR(35950, 19)
  "SubgroupShuffleDownINTEL\0" // IR(35969, 25)
  "SubgroupShuffleUpINTEL\0" // IR(35994, 23)
  "SubgroupShuffleXorINTEL\0" // IR(36017, 24)
  "SubgroupBlockReadINTEL\0" // IR(36041, 23)
  "SubgroupBlockWriteINTEL\0" // IR(36064, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(36088, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(36116, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(36145, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(36178, 34)
  "UCountLeadingZerosINTEL\0" // IR(36212, 24)
  "UCountTrailingZerosINTEL\0" // IR(36236, 25)
  "AbsISubINTEL\0" // IR(36261, 13)
  "AbsUSubINTEL\0" // IR(36274, 13)
  "IAddSatINTEL\0" // IR(36287, 13)
  "UAddSatINTEL\0" // IR(36300, 13)
  "IAverageINTEL\0" // IR(36313, 14)
  "UAverageINTEL\0" // IR(36327, 14)
  "IAverageRoundedINTEL\0" // IR(36341, 21)
  "UAverageRoundedINTEL\0" // IR(36362, 21)
  "ISubSatINTEL\0" // IR(36383, 13)
  "USubSatINTEL\0" // IR(36396, 13)
  "IMul32x16INTEL\0" // IR(36409, 15)
  "UMul32x16INTEL\0" // IR(36424, 15)
  "ConstantFunctionPointerINTEL\0" // IR(36439, 29)
  "FunctionPointerCallINTEL\0" // IR(36468, 25)
  "AsmTargetINTEL\0" // IR(36493, 15)
  "AsmCallINTEL\0" // IR(36508, 13)
  "AtomicFMinEXT\0" // IR(36521, 14)
  "AtomicFMaxEXT\0" // IR(36535, 14)
  "AssumeTrueKHR\0" // IR(36549, 14)
  "ExpectKHR\0" // IR(36563, 10)
  "DecorateString\0" // IR(36573, 15)
  "DecorateStringGOOGLE\0" // IR(36588, 21)
  "MemberDecorateString\0" // IR(36609, 21)
  "MemberDecorateStringGOOGLE\0" // IR(36630, 27)
  "VmeImageINTEL\0" // IR(36657, 14)
  "TypeVmeImageINTEL\0" // IR(36671, 18)
  "TypeAvcImePayloadINTEL\0" // IR(36689, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(36712, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(36735, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(36758, 23)
  "TypeAvcMceResultINTEL\0" // IR(36781, 22)
  "TypeAvcImeResultINTEL\0" // IR(36803, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(36825, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(36871, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(36915, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(36954, 37)
  "TypeAvcRefResultINTEL\0" // IR(36991, 22)
  "TypeAvcSicResultINTEL\0" // IR(37013, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(37035, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(37095, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(37148, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(37195, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(37235, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(37286, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(37330, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(37381, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(37437, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(37487, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(37539, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(37588, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(37635, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(37685, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(37736, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(37792, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(37825, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(37877, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(37938, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(37999, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(38038, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(38076, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(38115, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(38153, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(38192, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(38230, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(38266, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(38305, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(38348, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(38386, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(38424, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(38462, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(38507, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(38547, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(38609, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(38639, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(38677, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(38713, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(38746, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(38781, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(38820, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(38863, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(38910, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(38964, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(38998, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(39045, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(39090, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(39145, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(39198, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(39254, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(39308, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(39366, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(39422, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(39460, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(39506, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(39550, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(39599, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(39646, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(39716, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(39784, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(39853, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(39921, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(39987, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(40054, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(40090, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(40138, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(40197, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(40255, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(40311, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(40341, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(40371, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(40410, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(40456, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(40499, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(40546, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(40591, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(40637, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(40693, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(40731, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(40761, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(40793, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(40829, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(40871, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(40910, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(40949, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(40993, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(41041, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(41091, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(41134, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(41182, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(41225, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(41256, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(41303, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(41348, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(41394, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(41450, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(41488, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(41523, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(41567, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(41613, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(41654, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(41690, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(41740, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(41788, 35)
  "SaveMemoryINTEL\0" // IR(41823, 16)
  "RestoreMemoryINTEL\0" // IR(41839, 19)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(41858, 28)
  "ArbitraryFloatCastINTEL\0" // IR(41886, 24)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(41910, 31)
  "ArbitraryFloatCastToIntINTEL\0" // IR(41941, 29)
  "ArbitraryFloatAddINTEL\0" // IR(41970, 23)
  "ArbitraryFloatSubINTEL\0" // IR(41993, 23)
  "ArbitraryFloatMulINTEL\0" // IR(42016, 23)
  "ArbitraryFloatDivINTEL\0" // IR(42039, 23)
  "ArbitraryFloatGTINTEL\0" // IR(42062, 22)
  "ArbitraryFloatGEINTEL\0" // IR(42084, 22)
  "ArbitraryFloatLTINTEL\0" // IR(42106, 22)
  "ArbitraryFloatLEINTEL\0" // IR(42128, 22)
  "ArbitraryFloatEQINTEL\0" // IR(42150, 22)
  "ArbitraryFloatRecipINTEL\0" // IR(42172, 25)
  "ArbitraryFloatRSqrtINTEL\0" // IR(42197, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(42222, 24)
  "ArbitraryFloatHypotINTEL\0" // IR(42246, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(42271, 24)
  "ArbitraryFloatLogINTEL\0" // IR(42295, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(42318, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(42342, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(42367, 25)
  "ArbitraryFloatExpINTEL\0" // IR(42392, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(42415, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(42439, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(42464, 25)
  "ArbitraryFloatSinINTEL\0" // IR(42489, 23)
  "ArbitraryFloatCosINTEL\0" // IR(42512, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(42535, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(42561, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(42586, 25)
  "ArbitraryFloatASinINTEL\0" // IR(42611, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(42635, 26)
  "ArbitraryFloatACosINTEL\0" // IR(42661, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(42685, 26)
  "ArbitraryFloatATanINTEL\0" // IR(42711, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(42735, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(42761, 25)
  "ArbitraryFloatPowINTEL\0" // IR(42786, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(42809, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(42833, 24)
  "LoopControlINTEL\0" // IR(42857, 17)
  "AliasDomainDeclINTEL\0" // IR(42874, 21)
  "AliasScopeDeclINTEL\0" // IR(42895, 20)
  "AliasScopeListDeclINTEL\0" // IR(42915, 24)
  "FixedSqrtINTEL\0" // IR(42939, 15)
  "FixedRecipINTEL\0" // IR(42954, 16)
  "FixedRsqrtINTEL\0" // IR(42970, 16)
  "FixedSinINTEL\0" // IR(42986, 14)
  "FixedCosINTEL\0" // IR(43000, 14)
  "FixedSinCosINTEL\0" // IR(43014, 17)
  "FixedSinPiINTEL\0" // IR(43031, 16)
  "FixedCosPiINTEL\0" // IR(43047, 16)
  "FixedSinCosPiINTEL\0" // IR(43063, 19)
  "FixedLogINTEL\0" // IR(43082, 14)
  "FixedExpINTEL\0" // IR(43096, 14)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(43110, 29)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(43139, 29)
  "ReadPipeBlockingINTEL\0" // IR(43168, 22)
  "WritePipeBlockingINTEL\0" // IR(43190, 23)
  "RayQueryGetRayTMinKHR\0" // IR(43213, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(43235, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(43258, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(43286, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(43332, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(43369, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(43434, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(43474, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(43515, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(43554, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(43590, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(43636, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(43681, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(43723, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(43755, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(43784, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(43824, 40)
  "AtomicFAddEXT\0" // IR(43864, 14)
  "TypeBufferSurfaceINTEL\0" // IR(43878, 23)
  "TypeStructContinuedINTEL\0" // IR(43901, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(43926, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(43958, 36)
  "CompositeConstructContinuedINTEL\0" // IR(43994, 33)
  "ConvertFToBF16INTEL\0" // IR(44027, 20)
  "ConvertBF16ToFINTEL\0" // IR(44047, 20)
  "ControlBarrierArriveINTEL\0" // IR(44067, 26)
  "ControlBarrierWaitINTEL\0" // IR(44093, 24)
  "TaskSequenceCreateINTEL\0" // IR(44117, 24)
  "TaskSequenceAsyncINTEL\0" // IR(44141, 23)
  "TaskSequenceGetINTEL\0" // IR(44164, 21)
  "TaskSequenceReleaseINTEL\0" // IR(44185, 25)
  "TypeTaskSequenceINTEL\0" // IR(44210, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(44232, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(44259, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(44284, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(44318, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(44352, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(44381, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(44407, 62)
  "BitwiseFunctionINTEL\0" // IR(44469, 21)
  "GroupIMulKHR\0" // IR(44490, 13)
  "GroupFMulKHR\0" // IR(44503, 13)
  "GroupBitwiseAndKHR\0" // IR(44516, 19)
  "GroupBitwiseOrKHR\0" // IR(44535, 18)
  "GroupBitwiseXorKHR\0" // IR(44553, 19)
  "GroupLogicalAndKHR\0" // IR(44572, 19)
  "GroupLogicalOrKHR\0" // IR(44591, 18)
  "GroupLogicalXorKHR\0" // IR(44609, 19)
  "RoundFToTF32INTEL\0" // IR(44628, 18)
  "MaskedGatherINTEL\0" // IR(44646, 18)
  "MaskedScatterINTEL\0" // IR(44664, 19)
  "ConvertHandleToImageINTEL\0" // IR(44683, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(44709, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(44737, 33)
  "DebugInfoNone\0" // IR(44770, 14)
  "DebugCompilationUnit\0" // IR(44784, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(44805, 52)
  "DebugTypeBasic\0" // IR(44857, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(44872, 34)
  "DebugTypePointer\0" // IR(44906, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(44923, 38)
  "DebugTypeQualifier\0" // IR(44961, 19)
  "DebugTypeArray\0" // IR(44980, 15)
  "DebugTypeVector\0" // IR(44995, 16)
  "DebugTypedef\0" // IR(45011, 13)
  "DebugTypeFunction\0" // IR(45024, 18)
  "DebugTypeEnum\0" // IR(45042, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(45056, 38)
  "DebugTypeComposite\0" // IR(45094, 19)
  "DebugTypeMember\0" // IR(45113, 16)
  "DebugTypeInheritance\0" // IR(45129, 21)
  "DebugTypePtrToMember\0" // IR(45150, 21)
  "DebugTypeTemplate\0" // IR(45171, 18)
  "DebugTypeTemplateParameter\0" // IR(45189, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(45216, 35)
  "DebugTypeTemplateParameterPack\0" // IR(45251, 31)
  "DebugGlobalVariable\0" // IR(45282, 20)
  "DebugFunctionDeclaration\0" // IR(45302, 25)
  "DebugFunction\0" // IR(45327, 14)
  "DebugLexicalBlock\0" // IR(45341, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(45359, 31)
  "DebugScope\0" // IR(45390, 11)
  "DebugNoScope\0" // IR(45401, 13)
  "DebugInlinedAt\0" // IR(45414, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(45429, 42)
  "DebugLocalVariable\0" // IR(45471, 19)
  "DebugInlinedVariable\0" // IR(45490, 21)
  "DebugDeclare\0" // IR(45511, 13)
  "DebugValue\0" // IR(45524, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(45535, 33)
  "DebugOperation\0" // IR(45568, 15)
  "DebugExpression\0" // IR(45583, 16)
  "DebugMacroDef\0" // IR(45599, 14)
  "DebugMacroUndef\0" // IR(45613, 16)
  "Round\0" // IR(45629, 6)
  "RoundEven\0" // IR(45635, 10)
  "Trunc\0" // IR(45645, 6)
  "FAbs\0" // IR(45651, 5)
  "SAbs\0" // IR(45656, 5)
  "FSign\0" // IR(45661, 6)
  "SSign\0" // IR(45667, 6)
  "Floor\0" // IR(45673, 6)
  "Ceil\0" // IR(45679, 5)
  "Fract\0" // IR(45684, 6)
  "Radians\0" // IR(45690, 8)
  "Degrees\0" // IR(45698, 8)
  "Sin\0" // IR(45706, 4)
  "Cos\0" // IR(45710, 4)
  "Tan\0" // IR(45714, 4)
  "Asin\0" // IR(45718, 5)
  "Acos\0" // IR(45723, 5)
  "Atan\0" // IR(45728, 5)
  "Sinh\0" // IR(45733, 5)
  "Cosh\0" // IR(45738, 5)
  "Tanh\0" // IR(45743, 5)
  "Asinh\0" // IR(45748, 6)
  "Acosh\0" // IR(45754, 6)
  "Atanh\0" // IR(45760, 6)
  "Atan2\0" // IR(45766, 6)
  "Pow\0" // IR(45772, 4)
  "Exp\0" // IR(45776, 4)
  "Log\0" // IR(45780, 4)
  "Exp2\0" // IR(45784, 5)
  "Log2\0" // IR(45789, 5)
  "Sqrt\0" // IR(45794, 5)
  "InverseSqrt\0" // IR(45799, 12)
  "Determinant\0" // IR(45811, 12)
  "MatrixInverse\0" // IR(45823, 14)
  "Modf\0" // IR(45837, 5)
  "ModfStruct\0" // IR(45842, 11)
  "FMin\0" // IR(45853, 5)
  "UMin\0" // IR(45858, 5)
  "SMin\0" // IR(45863, 5)
  "FMax\0" // IR(45868, 5)
  "UMax\0" // IR(45873, 5)
  "SMax\0" // IR(45878, 5)
  "FClamp\0" // IR(45883, 7)
  "UClamp\0" // IR(45890, 7)
  "SClamp\0" // IR(45897, 7)
  "FMix\0" // IR(45904, 5)
  "IMix\0" // IR(45909, 5)
  "Step\0" // IR(45914, 5)
  "SmoothStep\0" // IR(45919, 11)
  "Fma\0" // IR(45930, 4)
  "Frexp\0" // IR(45934, 6)
  "FrexpStruct\0" // IR(45940, 12)
  "Ldexp\0" // IR(45952, 6)
  "PackSnorm4x8\0" // IR(45958, 13)
  "PackUnorm4x8\0" // IR(45971, 13)
  "PackSnorm2x16\0" // IR(45984, 14)
  "PackUnorm2x16\0" // IR(45998, 14)
  "PackHalf2x16\0" // IR(46012, 13)
  "PackDouble2x32\0" // IR(46025, 15)
  "UnpackSnorm2x16\0" // IR(46040, 16)
  "UnpackUnorm2x16\0" // IR(46056, 16)
  "UnpackHalf2x16\0" // IR(46072, 15)
  "UnpackSnorm4x8\0" // IR(46087, 15)
  "UnpackUnorm4x8\0" // IR(46102, 15)
  "UnpackDouble2x32\0" // IR(46117, 17)
  "Length\0" // IR(46134, 7)
  "Distance\0" // IR(46141, 9)
  "Cross\0" // IR(46150, 6)
  "Normalize\0" // IR(46156, 10)
  "FaceForward\0" // IR(46166, 12)
  "Reflect\0" // IR(46178, 8)
  "Refract\0" // IR(46186, 8)
  "FindILsb\0" // IR(46194, 9)
  "FindSMsb\0" // IR(46203, 9)
  "FindUMsb\0" // IR(46212, 9)
  "InterpolateAtCentroid\0" // IR(46221, 22)
  "InterpolateAtSample\0" // IR(46243, 20)
  "InterpolateAtOffset\0" // IR(46263, 20)
  "NMin\0" // IR(46283, 5)
  "NMax\0" // IR(46288, 5)
  "NClamp\0" // IR(46293, 7)
  "ArgumentInfo\0" // IR(46300, 13)
  "ArgumentStorageBuffer\0" // IR(46313, 22)
  "ArgumentUniform\0" // IR(46335, 16)
  "ArgumentPodStorageBuffer\0" // IR(46351, 25)
  "ArgumentPodUniform\0" // IR(46376, 19)
  "ArgumentPodPushConstant\0" // IR(46395, 24)
  "ArgumentSampledImage\0" // IR(46419, 21)
  "ArgumentStorageImage\0" // IR(46440, 21)
  "ArgumentSampler\0" // IR(46461, 16)
  "ArgumentWorkgroup\0" // IR(46477, 18)
  "SpecConstantWorkgroupSize\0" // IR(46495, 26)
  "SpecConstantGlobalOffset\0" // IR(46521, 25)
  "SpecConstantWorkDim\0" // IR(46546, 20)
  "PushConstantGlobalOffset\0" // IR(46566, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(46591, 30)
  "PushConstantGlobalSize\0" // IR(46621, 23)
  "PushConstantRegionOffset\0" // IR(46644, 25)
  "PushConstantNumWorkgroups\0" // IR(46669, 26)
  "PushConstantRegionGroupOffset\0" // IR(46695, 30)
  "ConstantDataStorageBuffer\0" // IR(46725, 26)
  "ConstantDataUniform\0" // IR(46751, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(46771, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(46801, 28)
  "ArgumentPointerPushConstant\0" // IR(46829, 28)
  "ArgumentPointerUniform\0" // IR(46857, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(46880, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(46915, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(46953, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(46995, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(47040, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(47077, 40)
  "ArgumentStorageTexelBuffer\0" // IR(47117, 27)
  "ArgumentUniformTexelBuffer\0" // IR(47144, 27)
  "ConstantDataPointerPushConstant\0" // IR(47171, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(47203, 40)
  "PrintfInfo\0" // IR(47243, 11)
  "PrintfBufferStorageBuffer\0" // IR(47254, 26)
  "PrintfBufferPointerPushConstant\0" // IR(47280, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(47312, 34)
  "WorkgroupVariableSize\0" // IR(47346, 22)
  "DebugImportedEntity\0" // IR(47368, 20)
  "DebugSource\0" // IR(47388, 12)
  "DebugFunctionDefinition\0" // IR(47400, 24)
  "DebugSourceContinued\0" // IR(47424, 21)
  "DebugLine\0" // IR(47445, 10)
  "DebugNoLine\0" // IR(47455, 12)
  "DebugBuildIdentifier\0" // IR(47467, 21)
  "DebugStoragePath\0" // IR(47488, 17)
  "DebugEntryPoint\0" // IR(47505, 16)
  "DebugTypeMatrix\0" // IR(47521, 16)
  "Configuration\0" // IR(47537, 14)
  "StartCounter\0" // IR(47551, 13)
  "StopCounter\0" // IR(47564, 12)
  "PushConstants\0" // IR(47576, 14)
  "SpecializationMapEntry\0" // IR(47590, 23)
  "DescriptorSetBuffer\0" // IR(47613, 20)
  "DescriptorSetImage\0" // IR(47633, 19)
  "DescriptorSetSampler\0" // IR(47652, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(47673, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(47736, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(47781, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(47830, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(47879, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(47923, 50)
  "DebugModuleINTEL\0" // IR(47973, 17)
  "acos\0" // IR(47990, 5)
  "acosh\0" // IR(47995, 6)
  "acospi\0" // IR(48001, 7)
  "asin\0" // IR(48008, 5)
  "asinh\0" // IR(48013, 6)
  "asinpi\0" // IR(48019, 7)
  "atan\0" // IR(48026, 5)
  "atan2\0" // IR(48031, 6)
  "atanh\0" // IR(48037, 6)
  "atanpi\0" // IR(48043, 7)
  "atan2pi\0" // IR(48050, 8)
  "cbrt\0" // IR(48058, 5)
  "ceil\0" // IR(48063, 5)
  "copysign\0" // IR(48068, 9)
  "cos\0" // IR(48077, 4)
  "cosh\0" // IR(48081, 5)
  "cospi\0" // IR(48086, 6)
  "erfc\0" // IR(48092, 5)
  "erf\0" // IR(48097, 4)
  "exp\0" // IR(48101, 4)
  "exp2\0" // IR(48105, 5)
  "exp10\0" // IR(48110, 6)
  "expm1\0" // IR(48116, 6)
  "fabs\0" // IR(48122, 5)
  "fdim\0" // IR(48127, 5)
  "floor\0" // IR(48132, 6)
  "fma\0" // IR(48138, 4)
  "fmax\0" // IR(48142, 5)
  "fmin\0" // IR(48147, 5)
  "fmod\0" // IR(48152, 5)
  "fract\0" // IR(48157, 6)
  "frexp\0" // IR(48163, 6)
  "hypot\0" // IR(48169, 6)
  "ilogb\0" // IR(48175, 6)
  "ldexp\0" // IR(48181, 6)
  "lgamma\0" // IR(48187, 7)
  "lgamma_r\0" // IR(48194, 9)
  "log\0" // IR(48203, 4)
  "log2\0" // IR(48207, 5)
  "log10\0" // IR(48212, 6)
  "log1p\0" // IR(48218, 6)
  "logb\0" // IR(48224, 5)
  "mad\0" // IR(48229, 4)
  "maxmag\0" // IR(48233, 7)
  "minmag\0" // IR(48240, 7)
  "modf\0" // IR(48247, 5)
  "nan\0" // IR(48252, 4)
  "nextafter\0" // IR(48256, 10)
  "pow\0" // IR(48266, 4)
  "pown\0" // IR(48270, 5)
  "powr\0" // IR(48275, 5)
  "remainder\0" // IR(48280, 10)
  "remquo\0" // IR(48290, 7)
  "rint\0" // IR(48297, 5)
  "rootn\0" // IR(48302, 6)
  "round\0" // IR(48308, 6)
  "rsqrt\0" // IR(48314, 6)
  "sin\0" // IR(48320, 4)
  "sincos\0" // IR(48324, 7)
  "sinh\0" // IR(48331, 5)
  "sinpi\0" // IR(48336, 6)
  "sqrt\0" // IR(48342, 5)
  "tan\0" // IR(48347, 4)
  "tanh\0" // IR(48351, 5)
  "tanpi\0" // IR(48356, 6)
  "tgamma\0" // IR(48362, 7)
  "trunc\0" // IR(48369, 6)
  "half_cos\0" // IR(48375, 9)
  "half_divide\0" // IR(48384, 12)
  "half_exp\0" // IR(48396, 9)
  "half_exp2\0" // IR(48405, 10)
  "half_exp10\0" // IR(48415, 11)
  "half_log\0" // IR(48426, 9)
  "half_log2\0" // IR(48435, 10)
  "half_log10\0" // IR(48445, 11)
  "half_powr\0" // IR(48456, 10)
  "half_recip\0" // IR(48466, 11)
  "half_rsqrt\0" // IR(48477, 11)
  "half_sin\0" // IR(48488, 9)
  "half_sqrt\0" // IR(48497, 10)
  "half_tan\0" // IR(48507, 9)
  "native_cos\0" // IR(48516, 11)
  "native_divide\0" // IR(48527, 14)
  "native_exp\0" // IR(48541, 11)
  "native_exp2\0" // IR(48552, 12)
  "native_exp10\0" // IR(48564, 13)
  "native_log\0" // IR(48577, 11)
  "native_log2\0" // IR(48588, 12)
  "native_log10\0" // IR(48600, 13)
  "native_powr\0" // IR(48613, 12)
  "native_recip\0" // IR(48625, 13)
  "native_rsqrt\0" // IR(48638, 13)
  "native_sin\0" // IR(48651, 11)
  "native_sqrt\0" // IR(48662, 12)
  "native_tan\0" // IR(48674, 11)
  "fclamp\0" // IR(48685, 7)
  "degrees\0" // IR(48692, 8)
  "fmax_common\0" // IR(48700, 12)
  "fmin_common\0" // IR(48712, 12)
  "mix\0" // IR(48724, 4)
  "radians\0" // IR(48728, 8)
  "step\0" // IR(48736, 5)
  "smoothstep\0" // IR(48741, 11)
  "sign\0" // IR(48752, 5)
  "cross\0" // IR(48757, 6)
  "distance\0" // IR(48763, 9)
  "length\0" // IR(48772, 7)
  "normalize\0" // IR(48779, 10)
  "fast_distance\0" // IR(48789, 14)
  "fast_length\0" // IR(48803, 12)
  "fast_normalize\0" // IR(48815, 15)
  "s_abs\0" // IR(48830, 6)
  "s_abs_diff\0" // IR(48836, 11)
  "s_add_sat\0" // IR(48847, 10)
  "u_add_sat\0" // IR(48857, 10)
  "s_hadd\0" // IR(48867, 7)
  "u_hadd\0" // IR(48874, 7)
  "s_rhadd\0" // IR(48881, 8)
  "u_rhadd\0" // IR(48889, 8)
  "s_clamp\0" // IR(48897, 8)
  "u_clamp\0" // IR(48905, 8)
  "clz\0" // IR(48913, 4)
  "ctz\0" // IR(48917, 4)
  "s_mad_hi\0" // IR(48921, 9)
  "u_mad_sat\0" // IR(48930, 10)
  "s_mad_sat\0" // IR(48940, 10)
  "s_max\0" // IR(48950, 6)
  "u_max\0" // IR(48956, 6)
  "s_min\0" // IR(48962, 6)
  "u_min\0" // IR(48968, 6)
  "s_mul_hi\0" // IR(48974, 9)
  "rotate\0" // IR(48983, 7)
  "s_sub_sat\0" // IR(48990, 10)
  "u_sub_sat\0" // IR(49000, 10)
  "u_upsample\0" // IR(49010, 11)
  "s_upsample\0" // IR(49021, 11)
  "popcount\0" // IR(49032, 9)
  "s_mad24\0" // IR(49041, 8)
  "u_mad24\0" // IR(49049, 8)
  "s_mul24\0" // IR(49057, 8)
  "u_mul24\0" // IR(49065, 8)
  "vloadn\0" // IR(49073, 7)
  "vstoren\0" // IR(49080, 8)
  "vload_half\0" // IR(49088, 11)
  "vload_halfn\0" // IR(49099, 12)
  "vstore_half\0" // IR(49111, 12)
  "vstore_half_r\0" // IR(49123, 14)
  "vstore_halfn\0" // IR(49137, 13)
  "vstore_halfn_r\0" // IR(49150, 15)
  "vloada_halfn\0" // IR(49165, 13)
  "vstorea_halfn\0" // IR(49178, 14)
  "vstorea_halfn_r\0" // IR(49192, 16)
  "shuffle\0" // IR(49208, 8)
  "shuffle2\0" // IR(49216, 9)
  "printf\0" // IR(49225, 7)
  "prefetch\0" // IR(49232, 9)
  "bitselect\0" // IR(49241, 10)
  "select\0" // IR(49251, 7)
  "u_abs\0" // IR(49258, 6)
  "u_abs_diff\0" // IR(49264, 11)
  "u_mul_hi\0" // IR(49275, 9)
  "u_mad_hi\0" // IR(49284, 9)
  "CubeFaceIndexAMD\0" // IR(49293, 17)
  "CubeFaceCoordAMD\0" // IR(49310, 17)
  "TimeAMD\0" // IR(49327, 8)
  "SwizzleInvocationsAMD\0" // IR(49335, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(49357, 28)
  "WriteInvocationAMD\0" // IR(49385, 19)
  "MbcntAMD\0" // IR(49404, 9)
  "InterpolateAtVertexAMD\0" // IR(49413, 23)
  "FMin3AMD\0" // IR(49436, 9)
  "UMin3AMD\0" // IR(49445, 9)
  "SMin3AMD\0" // IR(49454, 9)
  "FMax3AMD\0" // IR(49463, 9)
  "UMax3AMD\0" // IR(49472, 9)
  "SMax3AMD\0" // IR(49481, 9)
  "FMid3AMD\0" // IR(49490, 9)
  "UMid3AMD\0" // IR(49499, 9)
  "SMid3AMD\0" // IR(49508, 9)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(4625, 27), // 0 PhysicalStorageBuffer64EXT
  IR(5536, 18), // 1 SubgroupEqMaskKHR
  IR(5591, 18), // 2 SubgroupGeMaskKHR
  IR(5624, 18), // 3 SubgroupGtMaskKHR
  IR(5657, 18), // 4 SubgroupLeMaskKHR
  IR(5690, 18), // 5 SubgroupLtMaskKHR
  IR(6548, 12), // 6 BaryCoordNV
  IR(6603, 19), // 7 BaryCoordNoPerspNV
  IR(6634, 15), // 8 FragmentSizeNV
  IR(6691, 22), // 9 InvocationsPerPixelNV
  IR(6819, 11), // 10 LaunchIdNV
  IR(6844, 13), // 11 LaunchSizeNV
  IR(6875, 17), // 12 WorldRayOriginNV
  IR(6913, 20), // 13 WorldRayDirectionNV
  IR(6952, 18), // 14 ObjectRayOriginNV
  IR(6992, 21), // 15 ObjectRayDirectionNV
  IR(7024, 10), // 16 RayTminNV
  IR(7045, 10), // 17 RayTmaxNV
  IR(7078, 22), // 18 InstanceCustomIndexNV
  IR(7117, 16), // 19 ObjectToWorldNV
  IR(7150, 16), // 20 WorldToObjectNV
  IR(7184, 10), // 21 HitKindNV
  IR(7415, 19), // 22 IncomingRayFlagsNV
  IR(9124, 28), // 23 StorageUniformBufferBlock16
  IR(9187, 17), // 24 StorageUniform16
  IR(9991, 27), // 25 ShaderViewportIndexLayerNV
  IR(10035, 22), // 26 FragmentBarycentricNV
  IR(10088, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(10118, 14), // 28 ShadingRateNV
  IR(10178, 20), // 29 ShaderNonUniformEXT
  IR(10221, 26), // 30 RuntimeDescriptorArrayEXT
  IR(10283, 39), // 31 InputAttachmentArrayDynamicIndexingEXT
  IR(10361, 42), // 32 UniformTexelBufferArrayDynamicIndexingEXT
  IR(10442, 42), // 33 StorageTexelBufferArrayDynamicIndexingEXT
  IR(10521, 40), // 34 UniformBufferArrayNonUniformIndexingEXT
  IR(10597, 39), // 35 SampledImageArrayNonUniformIndexingEXT
  IR(10673, 40), // 36 StorageBufferArrayNonUniformIndexingEXT
  IR(10749, 39), // 37 StorageImageArrayNonUniformIndexingEXT
  IR(10827, 42), // 38 InputAttachmentArrayNonUniformIndexingEXT
  IR(10911, 45), // 39 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(10998, 45), // 40 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(11061, 21), // 41 VulkanMemoryModelKHR
  IR(11111, 32), // 42 VulkanMemoryModelDeviceScopeKHR
  IR(11143, 34), // 43 PhysicalStorageBufferAddressesEXT
  IR(11209, 31), // 44 ComputeDerivativeGroupLinearNV
  IR(11413, 28), // 45 DemoteToHelperInvocationEXT
  IR(12883, 22), // 46 DotProductInputAllKHR
  IR(12927, 25), // 47 DotProductInput4x8BitKHR
  IR(12980, 31), // 48 DotProductInput4x8BitPackedKHR
  IR(13022, 14), // 49 DotProductKHR
  IR(13187, 13), // 50 OptNoneINTEL
  IR(16139, 15), // 51 PerPrimitiveNV
  IR(16187, 12), // 52 PerVertexNV
  IR(16210, 14), // 53 NonUniformEXT
  IR(16240, 19), // 54 RestrictPointerEXT
  IR(16274, 18), // 55 AliasedPointerEXT
  IR(16584, 24), // 56 HlslCounterBufferGOOGLE
  IR(16621, 19), // 57 HlslSemanticGOOGLE
  IR(19295, 14), // 58 OutputLinesNV
  IR(19329, 19), // 59 OutputPrimitivesNV
  IR(19372, 23), // 60 DerivativeGroupQuadsNV
  IR(19420, 24), // 61 DerivativeGroupLinearNV
  IR(19463, 18), // 62 OutputTrianglesNV
  IR(20211, 16), // 63 RayGenerationNV
  IR(20243, 15), // 64 IntersectionNV
  IR(20268, 9), // 65 AnyHitNV
  IR(20291, 13), // 66 ClosestHitNV
  IR(20312, 7), // 67 MissNV
  IR(20331, 11), // 68 CallableNV
  IR(20448, 23), // 69 AllowContractFastINTEL
  IR(20484, 18), // 70 AllowReassocINTEL
  IR(20938, 22), // 71 MakeTexelAvailableKHR
  IR(20977, 20), // 72 MakeTexelVisibleKHR
  IR(21013, 19), // 73 NonPrivateTexelKHR
  IR(21046, 17), // 74 VolatileTexelKHR
  IR(22423, 24), // 75 MakePointerAvailableKHR
  IR(22466, 22), // 76 MakePointerVisibleKHR
  IR(22506, 21), // 77 NonPrivatePointerKHR
  IR(22593, 10), // 78 VulkanKHR
  IR(13955, 5), // 79 None
  IR(22776, 16), // 80 OutputMemoryKHR
  IR(22806, 17), // 81 MakeAvailableKHR
  IR(22835, 15), // 82 MakeVisibleKHR
  IR(22911, 28), // 83 PackedVectorFormat4x8BitKHR
  IR(23226, 24), // 84 SkipBuiltinPrimitivesNV
  IR(23966, 15), // 85 QueueFamilyKHR
  IR(24337, 15), // 86 CallableDataNV
  IR(24376, 23), // 87 IncomingCallableDataNV
  IR(24413, 13), // 88 RayPayloadNV
  IR(24442, 15), // 89 HitAttributeNV
  IR(24479, 21), // 90 IncomingRayPayloadNV
  IR(24522, 21), // 91 ShaderRecordBufferNV
  IR(24565, 25), // 92 PhysicalStorageBufferEXT
  IR(31781, 8), // 93 SDotKHR
  IR(31794, 8), // 94 UDotKHR
  IR(31808, 9), // 95 SUDotKHR
  IR(31828, 14), // 96 SDotAccSatKHR
  IR(31853, 14), // 97 UDotAccSatKHR
  IR(31879, 15), // 98 SUDotAccSatKHR
  IR(34364, 21), // 99 ReportIntersectionNV
  IR(34539, 28), // 100 TypeAccelerationStructureNV
  IR(36588, 21), // 101 DecorateStringGOOGLE
  IR(36630, 27), // 102 MemberDecorateStringGOOGLE
};

// Array of capabilities, referenced by IndexRanges elsewhere.
// Contains all sequences of capabilities used in the grammar.
static const spv::Capability kCapabilitySpans[] = {
  spv::Capability::Kernel, // 0
  spv::Capability::Addresses, // 1
  spv::Capability::PhysicalStorageBufferAddresses, // 2
  spv::Capability::Shader, // 3
  spv::Capability::ClipDistance, // 4
  spv::Capability::CullDistance, // 5
  spv::Capability::Geometry, // 6
  spv::Capability::Tessellation, // 7
  spv::Capability::RayTracingNV, // 8
  spv::Capability::RayTracingKHR, // 9
  spv::Capability::MeshShadingNV, // 10
  spv::Capability::MeshShadingEXT, // 11
  spv::Capability::Geometry, // 12
  spv::Capability::Tessellation, // 13
  spv::Capability::Geometry, // 14
  spv::Capability::ShaderLayer, // 15
  spv::Capability::ShaderViewportIndexLayerEXT, // 16
  spv::Capability::MeshShadingNV, // 17
  spv::Capability::MeshShadingEXT, // 18
  spv::Capability::MultiViewport, // 19
  spv::Capability::ShaderViewportIndex, // 20
  spv::Capability::ShaderViewportIndexLayerEXT, // 21
  spv::Capability::MeshShadingNV, // 22
  spv::Capability::MeshShadingEXT, // 23
  spv::Capability::Tessellation, // 24
  spv::Capability::SampleRateShading, // 25
  spv::Capability::Kernel, // 26
  spv::Capability::GroupNonUniform, // 27
  spv::Capability::SubgroupBallotKHR, // 28
  spv::Capability::Kernel, // 29
  spv::Capability::GroupNonUniform, // 30
  spv::Capability::CoreBuiltinsARM, // 31
  spv::Capability::SubgroupBallotKHR, // 32
  spv::Capability::GroupNonUniformBallot, // 33
  spv::Capability::DrawParameters, // 34
  spv::Capability::DrawParameters, // 35
  spv::Capability::MeshShadingNV, // 36
  spv::Capability::MeshShadingEXT, // 37
  spv::Capability::FragmentShadingRateKHR, // 38
  spv::Capability::DeviceGroup, // 39
  spv::Capability::MultiView, // 40
  spv::Capability::TileShadingQCOM, // 41
  spv::Capability::StencilExportEXT, // 42
  spv::Capability::ShaderEnqueueAMDX, // 43
  spv::Capability::ShaderViewportMaskNV, // 44
  spv::Capability::MeshShadingNV, // 45
  spv::Capability::ShaderStereoViewNV, // 46
  spv::Capability::PerViewAttributesNV, // 47
  spv::Capability::MeshShadingNV, // 48
  spv::Capability::FragmentFullyCoveredEXT, // 49
  spv::Capability::MeshShadingNV, // 50
  spv::Capability::FragmentBarycentricKHR, // 51
  spv::Capability::FragmentDensityEXT, // 52
  spv::Capability::MeshShadingEXT, // 53
  spv::Capability::RayTracingNV, // 54
  spv::Capability::RayTracingKHR, // 55
  spv::Capability::RayTracingNV, // 56
  spv::Capability::RayTracingMotionBlurNV, // 57
  spv::Capability::RayTracingPositionFetchKHR, // 58
  spv::Capability::RayTracingDisplacementMicromapNV, // 59
  spv::Capability::RayTracingKHR, // 60
  spv::Capability::RayTracingSpheresGeometryNV, // 61
  spv::Capability::RayTracingLinearSweptSpheresGeometryNV, // 62
  spv::Capability::ShaderSMBuiltinsNV, // 63
  spv::Capability::RayTracingClusterAccelerationStructureNV, // 64
  spv::Capability::RayCullMaskKHR, // 65
  spv::Capability::Matrix, // 66
  spv::Capability::Int64, // 67
  spv::Capability::ImageBasic, // 68
  spv::Capability::Geometry, // 69
  spv::Capability::SampledCubeArray, // 70
  spv::Capability::SampledRect, // 71
  spv::Capability::Sampled1D, // 72
  spv::Capability::SampledBuffer, // 73
  spv::Capability::DeviceEnqueue, // 74
  spv::Capability::Pipes, // 75
  spv::Capability::GroupNonUniform, // 76
  spv::Capability::WorkgroupMemoryExplicitLayoutKHR, // 77
  spv::Capability::StorageBuffer16BitAccess, // 78
  spv::Capability::VariablePointersStorageBuffer, // 79
  spv::Capability::AtomicStorage, // 80
  spv::Capability::StorageBuffer8BitAccess, // 81
  spv::Capability::RayQueryKHR, // 82
  spv::Capability::RayTracingKHR, // 83
  spv::Capability::Int4TypeINTEL, // 84
  spv::Capability::CooperativeMatrixKHR, // 85
  spv::Capability::BFloat16TypeKHR, // 86
  spv::Capability::BFloat16TypeKHR, // 87
  spv::Capability::CooperativeMatrixKHR, // 88
  spv::Capability::MultiViewport, // 89
  spv::Capability::ShaderViewportIndexLayerEXT, // 90
  spv::Capability::ShaderViewportMaskNV, // 91
  spv::Capability::InputAttachment, // 92
  spv::Capability::ImageBuffer, // 93
  spv::Capability::ShaderNonUniform, // 94
  spv::Capability::InputAttachment, // 95
  spv::Capability::ShaderNonUniform, // 96
  spv::Capability::SampledBuffer, // 97
  spv::Capability::ShaderNonUniform, // 98
  spv::Capability::ImageBuffer, // 99
  spv::Capability::ShaderNonUniform, // 100
  spv::Capability::VectorAnyINTEL, // 101
  spv::Capability::Int8, // 102
  spv::Capability::FPGAClusterAttributesINTEL, // 103
  spv::Capability::FPGAKernelAttributesINTEL, // 104
  spv::Capability::Subgroup2DBlockIOINTEL, // 105
  spv::Capability::Shader, // 106
  spv::Capability::Kernel, // 107
  spv::Capability::Shader, // 108
  spv::Capability::UniformDecoration, // 109
  spv::Capability::GeometryStreams, // 110
  spv::Capability::TransformFeedback, // 111
  spv::Capability::Kernel, // 112
  spv::Capability::FloatControls2, // 113
  spv::Capability::Linkage, // 114
  spv::Capability::SampleMaskOverrideCoverageNV, // 115
  spv::Capability::GeometryShaderPassthroughNV, // 116
  spv::Capability::MeshShadingNV, // 117
  spv::Capability::MeshShadingEXT, // 118
  spv::Capability::ShaderInvocationReorderNV, // 119
  spv::Capability::BindlessTextureNV, // 120
  spv::Capability::VectorComputeINTEL, // 121
  spv::Capability::IndirectReferencesINTEL, // 122
  spv::Capability::AsmINTEL, // 123
  spv::Capability::FunctionFloatControlINTEL, // 124
  spv::Capability::FPGAMemoryAttributesINTEL, // 125
  spv::Capability::FPGAMemoryAccessesINTEL, // 126
  spv::Capability::LoopFuseINTEL, // 127
  spv::Capability::FPGADSPControlINTEL, // 128
  spv::Capability::MemoryAccessAliasingINTEL, // 129
  spv::Capability::FPGAInvocationPipeliningAttributesINTEL, // 130
  spv::Capability::FPGABufferLocationINTEL, // 131
  spv::Capability::IOPipesINTEL, // 132
  spv::Capability::FPGAClusterAttributesV2INTEL, // 133
  spv::Capability::FPMaxErrorINTEL, // 134
  spv::Capability::FPGALatencyControlINTEL, // 135
  spv::Capability::FPGAArgumentInterfacesINTEL, // 136
  spv::Capability::GlobalVariableHostAccessINTEL, // 137
  spv::Capability::GlobalVariableFPGADecorationsINTEL, // 138
  spv::Capability::CacheControlsINTEL, // 139
  spv::Capability::TileImageColorReadAccessEXT, // 140
  spv::Capability::Geometry, // 141
  spv::Capability::Tessellation, // 142
  spv::Capability::MeshShadingNV, // 143
  spv::Capability::MeshShadingEXT, // 144
  spv::Capability::Geometry, // 145
  spv::Capability::MeshShadingNV, // 146
  spv::Capability::MeshShadingEXT, // 147
  spv::Capability::SubgroupDispatch, // 148
  spv::Capability::TileImageDepthReadAccessEXT, // 149
  spv::Capability::TileImageStencilReadAccessEXT, // 150
  spv::Capability::SampleMaskPostDepthCoverage, // 151
  spv::Capability::DenormPreserve, // 152
  spv::Capability::DenormFlushToZero, // 153
  spv::Capability::SignedZeroInfNanPreserve, // 154
  spv::Capability::RoundingModeRTE, // 155
  spv::Capability::RoundingModeRTZ, // 156
  spv::Capability::QuadControlKHR, // 157
  spv::Capability::ComputeDerivativeGroupQuadsNV, // 158
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 159
  spv::Capability::ComputeDerivativeGroupLinearNV, // 160
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 161
  spv::Capability::FragmentShaderPixelInterlockEXT, // 162
  spv::Capability::FragmentShaderSampleInterlockEXT, // 163
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 164
  spv::Capability::RoundToInfinityINTEL, // 165
  spv::Capability::KernelAttributesINTEL, // 166
  spv::Capability::FloatControls2, // 167
  spv::Capability::FPGAKernelAttributesv2INTEL, // 168
  spv::Capability::RegisterLimitsINTEL, // 169
  spv::Capability::FloatControls2, // 170
  spv::Capability::FPFastMathModeINTEL, // 171
  spv::Capability::OptNoneEXT, // 172
  spv::Capability::RuntimeAlignedAttributeINTEL, // 173
  spv::Capability::Kernel, // 174
  spv::Capability::GroupNonUniformArithmetic, // 175
  spv::Capability::GroupNonUniformBallot, // 176
  spv::Capability::GroupNonUniformClustered, // 177
  spv::Capability::GroupNonUniformPartitionedNV, // 178
  spv::Capability::ImageGatherExtended, // 179
  spv::Capability::MinLod, // 180
  spv::Capability::VulkanMemoryModel, // 181
  spv::Capability::FPGALoopControlsINTEL, // 182
  spv::Capability::ArbitraryPrecisionFixedPointINTEL, // 183
  spv::Capability::RawAccessChainsNV, // 184
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 185
  spv::Capability::RayTracingOpacityMicromapEXT, // 186
  spv::Capability::RayQueryKHR, // 187
  spv::Capability::StorageImageExtendedFormats, // 188
  spv::Capability::Int64ImageEXT, // 189
  spv::Capability::Shader, // 190
  spv::Capability::VectorComputeINTEL, // 191
  spv::Capability::GenericPointer, // 192
  spv::Capability::FunctionPointersINTEL, // 193
  spv::Capability::USMStorageClassesINTEL, // 194
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 195
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 196
  spv::Capability::TensorsARM, // 197
  spv::Capability::Addresses, // 198
  spv::Capability::PhysicalStorageBufferAddresses, // 199
  spv::Capability::LiteralSampler, // 200
  spv::Capability::Addresses, // 201
  spv::Capability::UntypedPointersKHR, // 202
  spv::Capability::Addresses, // 203
  spv::Capability::VariablePointers, // 204
  spv::Capability::VariablePointersStorageBuffer, // 205
  spv::Capability::PhysicalStorageBufferAddresses, // 206
  spv::Capability::Kernel, // 207
  spv::Capability::ImageQuery, // 208
  spv::Capability::ImageQuery, // 209
  spv::Capability::Shader, // 210
  spv::Capability::BitInstructions, // 211
  spv::Capability::DerivativeControl, // 212
  spv::Capability::Groups, // 213
  spv::Capability::SparseResidency, // 214
  spv::Capability::PipeStorage, // 215
  spv::Capability::NamedBarrier, // 216
  spv::Capability::GroupNonUniformVote, // 217
  spv::Capability::GroupNonUniformBallot, // 218
  spv::Capability::GroupNonUniformShuffle, // 219
  spv::Capability::GroupNonUniformShuffleRelative, // 220
  spv::Capability::GroupNonUniformArithmetic, // 221
  spv::Capability::GroupNonUniformClustered, // 222
  spv::Capability::GroupNonUniformPartitionedNV, // 223
  spv::Capability::GroupNonUniformQuad, // 224
  spv::Capability::Addresses, // 225
  spv::Capability::VariablePointers, // 226
  spv::Capability::VariablePointersStorageBuffer, // 227
  spv::Capability::UntypedPointersKHR, // 228
  spv::Capability::SubgroupBallotKHR, // 229
  spv::Capability::SubgroupVoteKHR, // 230
  spv::Capability::GroupNonUniformRotateKHR, // 231
  spv::Capability::RayTracingKHR, // 232
  spv::Capability::RayQueryKHR, // 233
  spv::Capability::DotProduct, // 234
  spv::Capability::CooperativeMatrixKHR, // 235
  spv::Capability::ReplicatedCompositesEXT, // 236
  spv::Capability::TextureSampleWeightedQCOM, // 237
  spv::Capability::TextureBoxFilterQCOM, // 238
  spv::Capability::TextureBlockMatchQCOM, // 239
  spv::Capability::TextureBlockMatch2QCOM, // 240
  spv::Capability::FragmentMaskAMD, // 241
  spv::Capability::ShaderClockKHR, // 242
  spv::Capability::ShaderInvocationReorderNV, // 243
  spv::Capability::RayTracingMotionBlurNV, // 244
  spv::Capability::ImageFootprintNV, // 245
  spv::Capability::CooperativeVectorNV, // 246
  spv::Capability::CooperativeVectorTrainingNV, // 247
  spv::Capability::CooperativeMatrixConversionsNV, // 248
  spv::Capability::DisplacementMicromapNV, // 249
  spv::Capability::RayQueryPositionFetchKHR, // 250
  spv::Capability::RayTracingNV, // 251
  spv::Capability::RayTracingKHR, // 252
  spv::Capability::RayQueryKHR, // 253
  spv::Capability::DisplacementMicromapNV, // 254
  spv::Capability::CooperativeMatrixNV, // 255
  spv::Capability::FragmentShaderSampleInterlockEXT, // 256
  spv::Capability::FragmentShaderPixelInterlockEXT, // 257
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 258
  spv::Capability::CooperativeMatrixReductionsNV, // 259
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 260
  spv::Capability::TensorAddressingNV, // 261
  spv::Capability::DemoteToHelperInvocation, // 262
  spv::Capability::SubgroupShuffleINTEL, // 263
  spv::Capability::SubgroupBufferBlockIOINTEL, // 264
  spv::Capability::SubgroupImageBlockIOINTEL, // 265
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 266
  spv::Capability::IntegerFunctions2INTEL, // 267
  spv::Capability::AtomicFloat16MinMaxEXT, // 268
  spv::Capability::AtomicFloat32MinMaxEXT, // 269
  spv::Capability::AtomicFloat64MinMaxEXT, // 270
  spv::Capability::AtomicFloat16VectorNV, // 271
  spv::Capability::ExpectAssumeKHR, // 272
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 273
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 274
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 275
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 276
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 277
  spv::Capability::VariableLengthArrayINTEL, // 278
  spv::Capability::ArbitraryPrecisionFloatingPointINTEL, // 279
  spv::Capability::UnstructuredLoopControlsINTEL, // 280
  spv::Capability::BlockingPipesINTEL, // 281
  spv::Capability::FPGARegINTEL, // 282
  spv::Capability::AtomicFloat16AddEXT, // 283
  spv::Capability::AtomicFloat32AddEXT, // 284
  spv::Capability::AtomicFloat64AddEXT, // 285
  spv::Capability::AtomicFloat16VectorNV, // 286
  spv::Capability::LongCompositesINTEL, // 287
  spv::Capability::BFloat16ConversionINTEL, // 288
  spv::Capability::SplitBarrierINTEL, // 289
  spv::Capability::ArithmeticFenceEXT, // 290
  spv::Capability::TaskSequenceINTEL, // 291
  spv::Capability::SubgroupBufferPrefetchINTEL, // 292
  spv::Capability::Subgroup2DBlockTransformINTEL, // 293
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 294
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 295
  spv::Capability::TernaryBitwiseFunctionINTEL, // 296
  spv::Capability::GroupUniformArithmeticKHR, // 297
  spv::Capability::TensorFloat32RoundingINTEL, // 298
  spv::Capability::MaskedGatherScatterINTEL, // 299
  spv::Capability::BindlessImagesINTEL, // 300
  spv::Capability::Float64, // 301
  spv::Capability::InterpolationFunction, // 302
};

// Array of extensions, referenced by IndexRanges elsewhere.
// Contains all sequences of extensions used in the grammar.
static const spvtools::Extension kExtensionSpans[] = {
  spvtools::Extension::kSPV_EXT_physical_storage_buffer, // 0
  spvtools::Extension::kSPV_KHR_physical_storage_buffer, // 1
  spvtools::Extension::kSPV_KHR_shader_ballot, // 2
  spvtools::Extension::kSPV_KHR_shader_draw_parameters, // 3
  spvtools::Extension::kSPV_KHR_shader_draw_parameters, // 4
  spvtools::Extension::kSPV_NV_mesh_shader, // 5
  spvtools::Extension::kSPV_EXT_mesh_shader, // 6
  spvtools::Extension::kSPV_KHR_fragment_shading_rate, // 7
  spvtools::Extension::kSPV_KHR_device_group, // 8
  spvtools::Extension::kSPV_KHR_multiview, // 9
  spvtools::Extension::kSPV_AMD_shader_explicit_vertex_parameter, // 10
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 11
  spvtools::Extension::kSPV_NV_viewport_array2, // 12
  spvtools::Extension::kSPV_NV_mesh_shader, // 13
  spvtools::Extension::kSPV_NV_stereo_view_rendering, // 14
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 15
  spvtools::Extension::kSPV_NV_mesh_shader, // 16
  spvtools::Extension::kSPV_EXT_fragment_fully_covered, // 17
  spvtools::Extension::kSPV_NV_mesh_shader, // 18
  spvtools::Extension::kSPV_NV_fragment_shader_barycentric, // 19
  spvtools::Extension::kSPV_KHR_fragment_shader_barycentric, // 20
  spvtools::Extension::kSPV_EXT_fragment_invocation_density, // 21
  spvtools::Extension::kSPV_NV_shading_rate, // 22
  spvtools::Extension::kSPV_EXT_mesh_shader, // 23
  spvtools::Extension::kSPV_NV_ray_tracing, // 24
  spvtools::Extension::kSPV_KHR_ray_tracing, // 25
  spvtools::Extension::kSPV_NV_ray_tracing, // 26
  spvtools::Extension::kSPV_NV_ray_tracing_motion_blur, // 27
  spvtools::Extension::kSPV_KHR_ray_tracing, // 28
  spvtools::Extension::kSPV_NV_linear_swept_spheres, // 29
  spvtools::Extension::kSPV_NV_shader_sm_builtins, // 30
  spvtools::Extension::kSPV_NV_cluster_acceleration_structure, // 31
  spvtools::Extension::kSPV_KHR_ray_cull_mask, // 32
  spvtools::Extension::kSPV_AMD_shader_ballot, // 33
  spvtools::Extension::kSPV_ARM_core_builtins, // 34
  spvtools::Extension::kSPV_EXT_shader_tile_image, // 35
  spvtools::Extension::kSPV_ARM_tensors, // 36
  spvtools::Extension::kSPV_ARM_cooperative_matrix_layouts, // 37
  spvtools::Extension::kSPV_KHR_workgroup_memory_explicit_layout, // 38
  spvtools::Extension::kSPV_KHR_subgroup_vote, // 39
  spvtools::Extension::kSPV_KHR_16bit_storage, // 40
  spvtools::Extension::kSPV_KHR_variable_pointers, // 41
  spvtools::Extension::kSPV_KHR_shader_atomic_counter_ops, // 42
  spvtools::Extension::kSPV_KHR_post_depth_coverage, // 43
  spvtools::Extension::kSPV_KHR_8bit_storage, // 44
  spvtools::Extension::kSPV_KHR_float_controls, // 45
  spvtools::Extension::kSPV_KHR_ray_query, // 46
  spvtools::Extension::kSPV_KHR_untyped_pointers, // 47
  spvtools::Extension::kSPV_KHR_ray_query, // 48
  spvtools::Extension::kSPV_KHR_ray_tracing, // 49
  spvtools::Extension::kSPV_QCOM_image_processing, // 50
  spvtools::Extension::kSPV_QCOM_tile_shading, // 51
  spvtools::Extension::kSPV_QCOM_image_processing2, // 52
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 53
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 54
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 55
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 56
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 57
  spvtools::Extension::kSPV_KHR_shader_clock, // 58
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 59
  spvtools::Extension::kSPV_KHR_quad_control, // 60
  spvtools::Extension::kSPV_INTEL_int4, // 61
  spvtools::Extension::kSPV_KHR_bfloat16, // 62
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 63
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 64
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 65
  spvtools::Extension::kSPV_NV_viewport_array2, // 66
  spvtools::Extension::kSPV_NV_viewport_array2, // 67
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 68
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 69
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 70
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 71
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 72
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 73
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 74
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 75
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 76
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 77
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 78
  spvtools::Extension::kSPV_NV_displacement_micromap, // 79
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 80
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 81
  spvtools::Extension::kSPV_NV_bindless_texture, // 82
  spvtools::Extension::kSPV_NV_cooperative_vector, // 83
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 84
  spvtools::Extension::kSPV_NV_raw_access_chains, // 85
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 86
  spvtools::Extension::kSPV_NV_tensor_addressing, // 87
  spvtools::Extension::kSPV_INTEL_subgroups, // 88
  spvtools::Extension::kSPV_INTEL_media_block_io, // 89
  spvtools::Extension::kSPV_INTEL_float_controls2, // 90
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 91
  spvtools::Extension::kSPV_INTEL_function_pointers, // 92
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 93
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 94
  spvtools::Extension::kSPV_INTEL_vector_compute, // 95
  spvtools::Extension::kSPV_KHR_expect_assume, // 96
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 97
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 98
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 99
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 100
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 101
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 102
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 103
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 104
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 105
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 106
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 107
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 108
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 109
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 110
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 111
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 112
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 113
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 114
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 115
  spvtools::Extension::kSPV_INTEL_io_pipes, // 116
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 117
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 118
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 119
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 120
  spvtools::Extension::kSPV_EXT_replicated_composites, // 121
  spvtools::Extension::kSPV_KHR_bit_instructions, // 122
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 123
  spvtools::Extension::kSPV_KHR_float_controls2, // 124
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 125
  spvtools::Extension::kSPV_INTEL_long_composites, // 126
  spvtools::Extension::kSPV_EXT_optnone, // 127
  spvtools::Extension::kSPV_INTEL_optnone, // 128
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 129
  spvtools::Extension::kSPV_INTEL_debug_module, // 130
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 131
  spvtools::Extension::kSPV_INTEL_split_barrier, // 132
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 133
  spvtools::Extension::kSPV_INTEL_task_sequence, // 134
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 135
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 136
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 137
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 138
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 139
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 140
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 141
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 142
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 143
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 144
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 145
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 146
  spvtools::Extension::kSPV_INTEL_cache_controls, // 147
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 148
  spvtools::Extension::kSPV_INTEL_bindless_images, // 149
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 150
  spvtools::Extension::kSPV_NV_mesh_shader, // 151
  spvtools::Extension::kSPV_EXT_mesh_shader, // 152
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 153
  spvtools::Extension::kSPV_GOOGLE_user_type, // 154
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 155
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 156
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 157
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 158
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 159
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 160
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 161
  spvtools::Extension::kSPV_KHR_variable_pointers, // 162
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 163
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 164
  spvtools::Extension::kSPV_KHR_ray_tracing, // 165
  spvtools::Extension::kSPV_KHR_ray_query, // 166
  spvtools::Extension::kSPV_NV_ray_tracing, // 167
  spvtools::Extension::kSPV_KHR_ray_tracing, // 168
  spvtools::Extension::kSPV_KHR_ray_query, // 169
  spvtools::Extension::kSPV_NV_displacement_micromap, // 170
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 171
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 172
};

// Array of operand types, referenced by IndexRanges elsewhere.
// Contains all sequences of operand types used in the grammar.
static const spv_operand_type_t kOperandSpans[] = {
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 0
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 2
  SPV_OPERAND_TYPE_BUILT_IN, // 3
  SPV_OPERAND_TYPE_SCOPE_ID, // 4
  SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE, // 5
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 6
  SPV_OPERAND_TYPE_FP_FAST_MATH_MODE, // 7
  SPV_OPERAND_TYPE_LITERAL_STRING, // 8
  SPV_OPERAND_TYPE_LINKAGE_TYPE, // 9
  SPV_OPERAND_TYPE_ID, // 10
  SPV_OPERAND_TYPE_LITERAL_STRING, // 11
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 12
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 13
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 14
  SPV_OPERAND_TYPE_FPDENORM_MODE, // 15
  SPV_OPERAND_TYPE_LITERAL_STRING, // 16
  SPV_OPERAND_TYPE_LITERAL_STRING, // 17
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 18
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 19
  SPV_OPERAND_TYPE_FPOPERATION_MODE, // 20
  SPV_OPERAND_TYPE_LITERAL_FLOAT, // 21
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 22
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 23
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 24
  SPV_OPERAND_TYPE_ACCESS_QUALIFIER, // 25
  SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER, // 26
  SPV_OPERAND_TYPE_LITERAL_STRING, // 27
  SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER, // 28
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 29
  SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL, // 30
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 31
  SPV_OPERAND_TYPE_STORE_CACHE_CONTROL, // 32
  SPV_OPERAND_TYPE_ID, // 33
  SPV_OPERAND_TYPE_ID, // 34
  SPV_OPERAND_TYPE_ID, // 35
  SPV_OPERAND_TYPE_ID, // 36
  SPV_OPERAND_TYPE_ID, // 37
  SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS, // 38
  SPV_OPERAND_TYPE_TYPE_ID, // 39
  SPV_OPERAND_TYPE_RESULT_ID, // 40
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 41
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 42
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 43
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING, // 44
  SPV_OPERAND_TYPE_ID, // 45
  SPV_OPERAND_TYPE_LITERAL_STRING, // 46
  SPV_OPERAND_TYPE_ID, // 47
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 48
  SPV_OPERAND_TYPE_LITERAL_STRING, // 49
  SPV_OPERAND_TYPE_RESULT_ID, // 50
  SPV_OPERAND_TYPE_LITERAL_STRING, // 51
  SPV_OPERAND_TYPE_ID, // 52
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 53
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 54
  SPV_OPERAND_TYPE_TYPE_ID, // 55
  SPV_OPERAND_TYPE_RESULT_ID, // 56
  SPV_OPERAND_TYPE_ID, // 57
  SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER, // 58
  SPV_OPERAND_TYPE_ADDRESSING_MODEL, // 59
  SPV_OPERAND_TYPE_MEMORY_MODEL, // 60
  SPV_OPERAND_TYPE_EXECUTION_MODEL, // 61
  SPV_OPERAND_TYPE_ID, // 62
  SPV_OPERAND_TYPE_LITERAL_STRING, // 63
  SPV_OPERAND_TYPE_VARIABLE_ID, // 64
  SPV_OPERAND_TYPE_ID, // 65
  SPV_OPERAND_TYPE_EXECUTION_MODE, // 66
  SPV_OPERAND_TYPE_CAPABILITY, // 67
  SPV_OPERAND_TYPE_RESULT_ID, // 68
  SPV_OPERAND_TYPE_RESULT_ID, // 69
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 70
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 71
  SPV_OPERAND_TYPE_RESULT_ID, // 72
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 73
  SPV_OPERAND_TYPE_OPTIONAL_FPENCODING, // 74
  SPV_OPERAND_TYPE_RESULT_ID, // 75
  SPV_OPERAND_TYPE_ID, // 76
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 77
  SPV_OPERAND_TYPE_RESULT_ID, // 78
  SPV_OPERAND_TYPE_ID, // 79
  SPV_OPERAND_TYPE_DIMENSIONALITY, // 80
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 81
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 82
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 83
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 84
  SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT, // 85
  SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER, // 86
  SPV_OPERAND_TYPE_RESULT_ID, // 87
  SPV_OPERAND_TYPE_ID, // 88
  SPV_OPERAND_TYPE_RESULT_ID, // 89
  SPV_OPERAND_TYPE_ID, // 90
  SPV_OPERAND_TYPE_ID, // 91
  SPV_OPERAND_TYPE_RESULT_ID, // 92
  SPV_OPERAND_TYPE_VARIABLE_ID, // 93
  SPV_OPERAND_TYPE_RESULT_ID, // 94
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 95
  SPV_OPERAND_TYPE_ID, // 96
  SPV_OPERAND_TYPE_RESULT_ID, // 97
  SPV_OPERAND_TYPE_ID, // 98
  SPV_OPERAND_TYPE_VARIABLE_ID, // 99
  SPV_OPERAND_TYPE_RESULT_ID, // 100
  SPV_OPERAND_TYPE_ACCESS_QUALIFIER, // 101
  SPV_OPERAND_TYPE_ID, // 102
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 103
  SPV_OPERAND_TYPE_TYPE_ID, // 104
  SPV_OPERAND_TYPE_RESULT_ID, // 105
  SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER, // 106
  SPV_OPERAND_TYPE_TYPE_ID, // 107
  SPV_OPERAND_TYPE_RESULT_ID, // 108
  SPV_OPERAND_TYPE_VARIABLE_ID, // 109
  SPV_OPERAND_TYPE_TYPE_ID, // 110
  SPV_OPERAND_TYPE_RESULT_ID, // 111
  SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE, // 112
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 113
  SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE, // 114
  SPV_OPERAND_TYPE_TYPE_ID, // 115
  SPV_OPERAND_TYPE_RESULT_ID, // 116
  SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER, // 117
  SPV_OPERAND_TYPE_TYPE_ID, // 118
  SPV_OPERAND_TYPE_RESULT_ID, // 119
  SPV_OPERAND_TYPE_FUNCTION_CONTROL, // 120
  SPV_OPERAND_TYPE_ID, // 121
  SPV_OPERAND_TYPE_TYPE_ID, // 122
  SPV_OPERAND_TYPE_RESULT_ID, // 123
  SPV_OPERAND_TYPE_ID, // 124
  SPV_OPERAND_TYPE_VARIABLE_ID, // 125
  SPV_OPERAND_TYPE_TYPE_ID, // 126
  SPV_OPERAND_TYPE_RESULT_ID, // 127
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 128
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 129
  SPV_OPERAND_TYPE_TYPE_ID, // 130
  SPV_OPERAND_TYPE_RESULT_ID, // 131
  SPV_OPERAND_TYPE_ID, // 132
  SPV_OPERAND_TYPE_ID, // 133
  SPV_OPERAND_TYPE_ID, // 134
  SPV_OPERAND_TYPE_TYPE_ID, // 135
  SPV_OPERAND_TYPE_RESULT_ID, // 136
  SPV_OPERAND_TYPE_ID, // 137
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 138
  SPV_OPERAND_TYPE_ID, // 139
  SPV_OPERAND_TYPE_ID, // 140
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 141
  SPV_OPERAND_TYPE_ID, // 142
  SPV_OPERAND_TYPE_ID, // 143
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 144
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 145
  SPV_OPERAND_TYPE_ID, // 146
  SPV_OPERAND_TYPE_ID, // 147
  SPV_OPERAND_TYPE_ID, // 148
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 149
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 150
  SPV_OPERAND_TYPE_TYPE_ID, // 151
  SPV_OPERAND_TYPE_RESULT_ID, // 152
  SPV_OPERAND_TYPE_ID, // 153
  SPV_OPERAND_TYPE_ID, // 154
  SPV_OPERAND_TYPE_VARIABLE_ID, // 155
  SPV_OPERAND_TYPE_TYPE_ID, // 156
  SPV_OPERAND_TYPE_RESULT_ID, // 157
  SPV_OPERAND_TYPE_ID, // 158
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 159
  SPV_OPERAND_TYPE_TYPE_ID, // 160
  SPV_OPERAND_TYPE_RESULT_ID, // 161
  SPV_OPERAND_TYPE_ID, // 162
  SPV_OPERAND_TYPE_ID, // 163
  SPV_OPERAND_TYPE_DECORATION, // 164
  SPV_OPERAND_TYPE_ID, // 165
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 166
  SPV_OPERAND_TYPE_DECORATION, // 167
  SPV_OPERAND_TYPE_ID, // 168
  SPV_OPERAND_TYPE_VARIABLE_ID, // 169
  SPV_OPERAND_TYPE_ID, // 170
  SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER, // 171
  SPV_OPERAND_TYPE_TYPE_ID, // 172
  SPV_OPERAND_TYPE_RESULT_ID, // 173
  SPV_OPERAND_TYPE_ID, // 174
  SPV_OPERAND_TYPE_ID, // 175
  SPV_OPERAND_TYPE_TYPE_ID, // 176
  SPV_OPERAND_TYPE_RESULT_ID, // 177
  SPV_OPERAND_TYPE_ID, // 178
  SPV_OPERAND_TYPE_ID, // 179
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 180
  SPV_OPERAND_TYPE_TYPE_ID, // 181
  SPV_OPERAND_TYPE_RESULT_ID, // 182
  SPV_OPERAND_TYPE_ID, // 183
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 184
  SPV_OPERAND_TYPE_TYPE_ID, // 185
  SPV_OPERAND_TYPE_RESULT_ID, // 186
  SPV_OPERAND_TYPE_ID, // 187
  SPV_OPERAND_TYPE_ID, // 188
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 189
  SPV_OPERAND_TYPE_TYPE_ID, // 190
  SPV_OPERAND_TYPE_RESULT_ID, // 191
  SPV_OPERAND_TYPE_ID, // 192
  SPV_OPERAND_TYPE_ID, // 193
  SPV_OPERAND_TYPE_IMAGE, // 194
  SPV_OPERAND_TYPE_TYPE_ID, // 195
  SPV_OPERAND_TYPE_RESULT_ID, // 196
  SPV_OPERAND_TYPE_ID, // 197
  SPV_OPERAND_TYPE_ID, // 198
  SPV_OPERAND_TYPE_ID, // 199
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 200
  SPV_OPERAND_TYPE_TYPE_ID, // 201
  SPV_OPERAND_TYPE_RESULT_ID, // 202
  SPV_OPERAND_TYPE_ID, // 203
  SPV_OPERAND_TYPE_ID, // 204
  SPV_OPERAND_TYPE_ID, // 205
  SPV_OPERAND_TYPE_IMAGE, // 206
  SPV_OPERAND_TYPE_ID, // 207
  SPV_OPERAND_TYPE_ID, // 208
  SPV_OPERAND_TYPE_ID, // 209
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 210
  SPV_OPERAND_TYPE_TYPE_ID, // 211
  SPV_OPERAND_TYPE_RESULT_ID, // 212
  SPV_OPERAND_TYPE_ID, // 213
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 214
  SPV_OPERAND_TYPE_TYPE_ID, // 215
  SPV_OPERAND_TYPE_RESULT_ID, // 216
  SPV_OPERAND_TYPE_ID, // 217
  SPV_OPERAND_TYPE_ID, // 218
  SPV_OPERAND_TYPE_ID, // 219
  SPV_OPERAND_TYPE_ID, // 220
  SPV_OPERAND_TYPE_SCOPE_ID, // 221
  SPV_OPERAND_TYPE_SCOPE_ID, // 222
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 223
  SPV_OPERAND_TYPE_SCOPE_ID, // 224
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 225
  SPV_OPERAND_TYPE_TYPE_ID, // 226
  SPV_OPERAND_TYPE_RESULT_ID, // 227
  SPV_OPERAND_TYPE_ID, // 228
  SPV_OPERAND_TYPE_SCOPE_ID, // 229
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 230
  SPV_OPERAND_TYPE_ID, // 231
  SPV_OPERAND_TYPE_SCOPE_ID, // 232
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 233
  SPV_OPERAND_TYPE_ID, // 234
  SPV_OPERAND_TYPE_TYPE_ID, // 235
  SPV_OPERAND_TYPE_RESULT_ID, // 236
  SPV_OPERAND_TYPE_ID, // 237
  SPV_OPERAND_TYPE_SCOPE_ID, // 238
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 239
  SPV_OPERAND_TYPE_ID, // 240
  SPV_OPERAND_TYPE_TYPE_ID, // 241
  SPV_OPERAND_TYPE_RESULT_ID, // 242
  SPV_OPERAND_TYPE_ID, // 243
  SPV_OPERAND_TYPE_SCOPE_ID, // 244
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 245
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 246
  SPV_OPERAND_TYPE_ID, // 247
  SPV_OPERAND_TYPE_ID, // 248
  SPV_OPERAND_TYPE_ID, // 249
  SPV_OPERAND_TYPE_ID, // 250
  SPV_OPERAND_TYPE_LOOP_CONTROL, // 251
  SPV_OPERAND_TYPE_ID, // 252
  SPV_OPERAND_TYPE_SELECTION_CONTROL, // 253
  SPV_OPERAND_TYPE_ID, // 254
  SPV_OPERAND_TYPE_ID, // 255
  SPV_OPERAND_TYPE_ID, // 256
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 257
  SPV_OPERAND_TYPE_ID, // 258
  SPV_OPERAND_TYPE_ID, // 259
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID, // 260
  SPV_OPERAND_TYPE_ID, // 261
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 262
  SPV_OPERAND_TYPE_TYPE_ID, // 263
  SPV_OPERAND_TYPE_RESULT_ID, // 264
  SPV_OPERAND_TYPE_SCOPE_ID, // 265
  SPV_OPERAND_TYPE_ID, // 266
  SPV_OPERAND_TYPE_ID, // 267
  SPV_OPERAND_TYPE_ID, // 268
  SPV_OPERAND_TYPE_ID, // 269
  SPV_OPERAND_TYPE_ID, // 270
  SPV_OPERAND_TYPE_SCOPE_ID, // 271
  SPV_OPERAND_TYPE_ID, // 272
  SPV_OPERAND_TYPE_ID, // 273
  SPV_OPERAND_TYPE_TYPE_ID, // 274
  SPV_OPERAND_TYPE_RESULT_ID, // 275
  SPV_OPERAND_TYPE_SCOPE_ID, // 276
  SPV_OPERAND_TYPE_ID, // 277
  SPV_OPERAND_TYPE_TYPE_ID, // 278
  SPV_OPERAND_TYPE_RESULT_ID, // 279
  SPV_OPERAND_TYPE_SCOPE_ID, // 280
  SPV_OPERAND_TYPE_ID, // 281
  SPV_OPERAND_TYPE_ID, // 282
  SPV_OPERAND_TYPE_TYPE_ID, // 283
  SPV_OPERAND_TYPE_RESULT_ID, // 284
  SPV_OPERAND_TYPE_SCOPE_ID, // 285
  SPV_OPERAND_TYPE_GROUP_OPERATION, // 286
  SPV_OPERAND_TYPE_ID, // 287
  SPV_OPERAND_TYPE_TYPE_ID, // 288
  SPV_OPERAND_TYPE_RESULT_ID, // 289
  SPV_OPERAND_TYPE_ID, // 290
  SPV_OPERAND_TYPE_ID, // 291
  SPV_OPERAND_TYPE_ID, // 292
  SPV_OPERAND_TYPE_ID, // 293
  SPV_OPERAND_TYPE_ID, // 294
  SPV_OPERAND_TYPE_ID, // 295
  SPV_OPERAND_TYPE_ID, // 296
  SPV_OPERAND_TYPE_ID, // 297
  SPV_OPERAND_TYPE_ID, // 298
  SPV_OPERAND_TYPE_ID, // 299
  SPV_OPERAND_TYPE_TYPE_ID, // 300
  SPV_OPERAND_TYPE_RESULT_ID, // 301
  SPV_OPERAND_TYPE_SCOPE_ID, // 302
  SPV_OPERAND_TYPE_ID, // 303
  SPV_OPERAND_TYPE_ID, // 304
  SPV_OPERAND_TYPE_ID, // 305
  SPV_OPERAND_TYPE_ID, // 306
  SPV_OPERAND_TYPE_SCOPE_ID, // 307
  SPV_OPERAND_TYPE_ID, // 308
  SPV_OPERAND_TYPE_ID, // 309
  SPV_OPERAND_TYPE_ID, // 310
  SPV_OPERAND_TYPE_ID, // 311
  SPV_OPERAND_TYPE_TYPE_ID, // 312
  SPV_OPERAND_TYPE_RESULT_ID, // 313
  SPV_OPERAND_TYPE_ID, // 314
  SPV_OPERAND_TYPE_ID, // 315
  SPV_OPERAND_TYPE_ID, // 316
  SPV_OPERAND_TYPE_ID, // 317
  SPV_OPERAND_TYPE_ID, // 318
  SPV_OPERAND_TYPE_ID, // 319
  SPV_OPERAND_TYPE_ID, // 320
  SPV_OPERAND_TYPE_ID, // 321
  SPV_OPERAND_TYPE_ID, // 322
  SPV_OPERAND_TYPE_ID, // 323
  SPV_OPERAND_TYPE_VARIABLE_ID, // 324
  SPV_OPERAND_TYPE_TYPE_ID, // 325
  SPV_OPERAND_TYPE_RESULT_ID, // 326
  SPV_OPERAND_TYPE_ID, // 327
  SPV_OPERAND_TYPE_ID, // 328
  SPV_OPERAND_TYPE_ID, // 329
  SPV_OPERAND_TYPE_ID, // 330
  SPV_OPERAND_TYPE_ID, // 331
  SPV_OPERAND_TYPE_ID, // 332
  SPV_OPERAND_TYPE_SCOPE_ID, // 333
  SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, // 334
  SPV_OPERAND_TYPE_TYPE_ID, // 335
  SPV_OPERAND_TYPE_RESULT_ID, // 336
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 337
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 338
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 339
  SPV_OPERAND_TYPE_TYPE_ID, // 340
  SPV_OPERAND_TYPE_RESULT_ID, // 341
  SPV_OPERAND_TYPE_SCOPE_ID, // 342
  SPV_OPERAND_TYPE_TYPE_ID, // 343
  SPV_OPERAND_TYPE_RESULT_ID, // 344
  SPV_OPERAND_TYPE_SCOPE_ID, // 345
  SPV_OPERAND_TYPE_GROUP_OPERATION, // 346
  SPV_OPERAND_TYPE_ID, // 347
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 348
  SPV_OPERAND_TYPE_TYPE_ID, // 349
  SPV_OPERAND_TYPE_RESULT_ID, // 350
  SPV_OPERAND_TYPE_ID, // 351
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 352
  SPV_OPERAND_TYPE_TYPE_ID, // 353
  SPV_OPERAND_TYPE_RESULT_ID, // 354
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 355
  SPV_OPERAND_TYPE_RESULT_ID, // 356
  SPV_OPERAND_TYPE_ID, // 357
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 358
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 359
  SPV_OPERAND_TYPE_TYPE_ID, // 360
  SPV_OPERAND_TYPE_RESULT_ID, // 361
  SPV_OPERAND_TYPE_ID, // 362
  SPV_OPERAND_TYPE_ID, // 363
  SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS, // 364
  SPV_OPERAND_TYPE_ID, // 365
  SPV_OPERAND_TYPE_ID, // 366
  SPV_OPERAND_TYPE_ID, // 367
  SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS, // 368
  SPV_OPERAND_TYPE_RESULT_ID, // 369
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 370
  SPV_OPERAND_TYPE_TYPE_ID, // 371
  SPV_OPERAND_TYPE_RESULT_ID, // 372
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 373
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 374
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 375
  SPV_OPERAND_TYPE_TYPE_ID, // 376
  SPV_OPERAND_TYPE_RESULT_ID, // 377
  SPV_OPERAND_TYPE_ID, // 378
  SPV_OPERAND_TYPE_ID, // 379
  SPV_OPERAND_TYPE_ID, // 380
  SPV_OPERAND_TYPE_VARIABLE_ID, // 381
  SPV_OPERAND_TYPE_TYPE_ID, // 382
  SPV_OPERAND_TYPE_RESULT_ID, // 383
  SPV_OPERAND_TYPE_ID, // 384
  SPV_OPERAND_TYPE_ID, // 385
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 386
  SPV_OPERAND_TYPE_ID, // 387
  SPV_OPERAND_TYPE_ID, // 388
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 389
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 390
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 391
  SPV_OPERAND_TYPE_TYPE_ID, // 392
  SPV_OPERAND_TYPE_RESULT_ID, // 393
  SPV_OPERAND_TYPE_SCOPE_ID, // 394
  SPV_OPERAND_TYPE_ID, // 395
  SPV_OPERAND_TYPE_ID, // 396
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 397
  SPV_OPERAND_TYPE_TYPE_ID, // 398
  SPV_OPERAND_TYPE_RESULT_ID, // 399
  SPV_OPERAND_TYPE_ID, // 400
  SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER, // 401
  SPV_OPERAND_TYPE_VARIABLE_ID, // 402
  SPV_OPERAND_TYPE_ID, // 403
  SPV_OPERAND_TYPE_ID, // 404
  SPV_OPERAND_TYPE_ID, // 405
  SPV_OPERAND_TYPE_ID, // 406
  SPV_OPERAND_TYPE_ID, // 407
  SPV_OPERAND_TYPE_ID, // 408
  SPV_OPERAND_TYPE_ID, // 409
  SPV_OPERAND_TYPE_ID, // 410
  SPV_OPERAND_TYPE_ID, // 411
  SPV_OPERAND_TYPE_ID, // 412
  SPV_OPERAND_TYPE_ID, // 413
  SPV_OPERAND_TYPE_TYPE_ID, // 414
  SPV_OPERAND_TYPE_RESULT_ID, // 415
  SPV_OPERAND_TYPE_ID, // 416
  SPV_OPERAND_TYPE_ID, // 417
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 418
  SPV_OPERAND_TYPE_TYPE_ID, // 419
  SPV_OPERAND_TYPE_RESULT_ID, // 420
  SPV_OPERAND_TYPE_ID, // 421
  SPV_OPERAND_TYPE_ID, // 422
  SPV_OPERAND_TYPE_ID, // 423
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 424
  SPV_OPERAND_TYPE_RESULT_ID, // 425
  SPV_OPERAND_TYPE_ID, // 426
  SPV_OPERAND_TYPE_SCOPE_ID, // 427
  SPV_OPERAND_TYPE_ID, // 428
  SPV_OPERAND_TYPE_ID, // 429
  SPV_OPERAND_TYPE_ID, // 430
  SPV_OPERAND_TYPE_TYPE_ID, // 431
  SPV_OPERAND_TYPE_RESULT_ID, // 432
  SPV_OPERAND_TYPE_ID, // 433
  SPV_OPERAND_TYPE_ID, // 434
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 435
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 436
  SPV_OPERAND_TYPE_ID, // 437
  SPV_OPERAND_TYPE_ID, // 438
  SPV_OPERAND_TYPE_ID, // 439
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 440
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 441
  SPV_OPERAND_TYPE_TYPE_ID, // 442
  SPV_OPERAND_TYPE_RESULT_ID, // 443
  SPV_OPERAND_TYPE_ID, // 444
  SPV_OPERAND_TYPE_ID, // 445
  SPV_OPERAND_TYPE_ID, // 446
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 447
  SPV_OPERAND_TYPE_ID, // 448
  SPV_OPERAND_TYPE_ID, // 449
  SPV_OPERAND_TYPE_ID, // 450
  SPV_OPERAND_TYPE_ID, // 451
  SPV_OPERAND_TYPE_ID, // 452
  SPV_OPERAND_TYPE_ID, // 453
  SPV_OPERAND_TYPE_ID, // 454
  SPV_OPERAND_TYPE_ID, // 455
  SPV_OPERAND_TYPE_ID, // 456
  SPV_OPERAND_TYPE_ID, // 457
  SPV_OPERAND_TYPE_ID, // 458
  SPV_OPERAND_TYPE_ID, // 459
  SPV_OPERAND_TYPE_ID, // 460
  SPV_OPERAND_TYPE_ID, // 461
  SPV_OPERAND_TYPE_ID, // 462
  SPV_OPERAND_TYPE_ID, // 463
  SPV_OPERAND_TYPE_ID, // 464
  SPV_OPERAND_TYPE_ID, // 465
  SPV_OPERAND_TYPE_ID, // 466
  SPV_OPERAND_TYPE_ID, // 467
  SPV_OPERAND_TYPE_ID, // 468
  SPV_OPERAND_TYPE_ID, // 469
  SPV_OPERAND_TYPE_ID, // 470
  SPV_OPERAND_TYPE_ID, // 471
  SPV_OPERAND_TYPE_ID, // 472
  SPV_OPERAND_TYPE_ID, // 473
  SPV_OPERAND_TYPE_ID, // 474
  SPV_OPERAND_TYPE_ID, // 475
  SPV_OPERAND_TYPE_ID, // 476
  SPV_OPERAND_TYPE_ID, // 477
  SPV_OPERAND_TYPE_ID, // 478
  SPV_OPERAND_TYPE_ID, // 479
  SPV_OPERAND_TYPE_ID, // 480
  SPV_OPERAND_TYPE_ID, // 481
  SPV_OPERAND_TYPE_ID, // 482
  SPV_OPERAND_TYPE_ID, // 483
  SPV_OPERAND_TYPE_ID, // 484
  SPV_OPERAND_TYPE_ID, // 485
  SPV_OPERAND_TYPE_ID, // 486
  SPV_OPERAND_TYPE_ID, // 487
  SPV_OPERAND_TYPE_ID, // 488
  SPV_OPERAND_TYPE_ID, // 489
  SPV_OPERAND_TYPE_ID, // 490
  SPV_OPERAND_TYPE_ID, // 491
  SPV_OPERAND_TYPE_ID, // 492
  SPV_OPERAND_TYPE_ID, // 493
  SPV_OPERAND_TYPE_ID, // 494
  SPV_OPERAND_TYPE_ID, // 495
  SPV_OPERAND_TYPE_ID, // 496
  SPV_OPERAND_TYPE_ID, // 497
  SPV_OPERAND_TYPE_ID, // 498
  SPV_OPERAND_TYPE_ID, // 499
  SPV_OPERAND_TYPE_ID, // 500
  SPV_OPERAND_TYPE_ID, // 501
  SPV_OPERAND_TYPE_ID, // 502
  SPV_OPERAND_TYPE_ID, // 503
  SPV_OPERAND_TYPE_ID, // 504
  SPV_OPERAND_TYPE_ID, // 505
  SPV_OPERAND_TYPE_ID, // 506
  SPV_OPERAND_TYPE_ID, // 507
  SPV_OPERAND_TYPE_ID, // 508
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 509
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 510
  SPV_OPERAND_TYPE_TYPE_ID, // 511
  SPV_OPERAND_TYPE_RESULT_ID, // 512
  SPV_OPERAND_TYPE_ID, // 513
  SPV_OPERAND_TYPE_ID, // 514
  SPV_OPERAND_TYPE_ID, // 515
  SPV_OPERAND_TYPE_ID, // 516
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 517
  SPV_OPERAND_TYPE_TYPE_ID, // 518
  SPV_OPERAND_TYPE_RESULT_ID, // 519
  SPV_OPERAND_TYPE_ID, // 520
  SPV_OPERAND_TYPE_ID, // 521
  SPV_OPERAND_TYPE_ID, // 522
  SPV_OPERAND_TYPE_ID, // 523
  SPV_OPERAND_TYPE_ID, // 524
  SPV_OPERAND_TYPE_ID, // 525
  SPV_OPERAND_TYPE_ID, // 526
  SPV_OPERAND_TYPE_ID, // 527
  SPV_OPERAND_TYPE_ID, // 528
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 529
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 530
  SPV_OPERAND_TYPE_ID, // 531
  SPV_OPERAND_TYPE_ID, // 532
  SPV_OPERAND_TYPE_ID, // 533
  SPV_OPERAND_TYPE_ID, // 534
  SPV_OPERAND_TYPE_ID, // 535
  SPV_OPERAND_TYPE_ID, // 536
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 537
  SPV_OPERAND_TYPE_TYPE_ID, // 538
  SPV_OPERAND_TYPE_RESULT_ID, // 539
  SPV_OPERAND_TYPE_ID, // 540
  SPV_OPERAND_TYPE_ID, // 541
  SPV_OPERAND_TYPE_ID, // 542
  SPV_OPERAND_TYPE_ID, // 543
  SPV_OPERAND_TYPE_ID, // 544
  SPV_OPERAND_TYPE_ID, // 545
  SPV_OPERAND_TYPE_ID, // 546
  SPV_OPERAND_TYPE_ID, // 547
  SPV_OPERAND_TYPE_ID, // 548
  SPV_OPERAND_TYPE_ID, // 549
  SPV_OPERAND_TYPE_ID, // 550
  SPV_OPERAND_TYPE_ID, // 551
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 552
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 553
  SPV_OPERAND_TYPE_ID, // 554
  SPV_OPERAND_TYPE_ID, // 555
  SPV_OPERAND_TYPE_ID, // 556
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 557
  SPV_OPERAND_TYPE_TYPE_ID, // 558
  SPV_OPERAND_TYPE_RESULT_ID, // 559
  SPV_OPERAND_TYPE_ID, // 560
  SPV_OPERAND_TYPE_ID, // 561
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 562
  SPV_OPERAND_TYPE_ID, // 563
  SPV_OPERAND_TYPE_ID, // 564
  SPV_OPERAND_TYPE_ID, // 565
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 566
  SPV_OPERAND_TYPE_RESULT_ID, // 567
  SPV_OPERAND_TYPE_ID, // 568
  SPV_OPERAND_TYPE_SCOPE_ID, // 569
  SPV_OPERAND_TYPE_ID, // 570
  SPV_OPERAND_TYPE_ID, // 571
  SPV_OPERAND_TYPE_TYPE_ID, // 572
  SPV_OPERAND_TYPE_RESULT_ID, // 573
  SPV_OPERAND_TYPE_ID, // 574
  SPV_OPERAND_TYPE_ID, // 575
  SPV_OPERAND_TYPE_ID, // 576
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 577
  SPV_OPERAND_TYPE_ID, // 578
  SPV_OPERAND_TYPE_ID, // 579
  SPV_OPERAND_TYPE_ID, // 580
  SPV_OPERAND_TYPE_ID, // 581
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 582
  SPV_OPERAND_TYPE_TYPE_ID, // 583
  SPV_OPERAND_TYPE_RESULT_ID, // 584
  SPV_OPERAND_TYPE_ID, // 585
  SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE, // 586
  SPV_OPERAND_TYPE_ID, // 587
  SPV_OPERAND_TYPE_TYPE_ID, // 588
  SPV_OPERAND_TYPE_RESULT_ID, // 589
  SPV_OPERAND_TYPE_ID, // 590
  SPV_OPERAND_TYPE_ID, // 591
  SPV_OPERAND_TYPE_ID, // 592
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 593
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 594
  SPV_OPERAND_TYPE_ID, // 595
  SPV_OPERAND_TYPE_ID, // 596
  SPV_OPERAND_TYPE_ID, // 597
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 598
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 599
  SPV_OPERAND_TYPE_RESULT_ID, // 600
  SPV_OPERAND_TYPE_ID, // 601
  SPV_OPERAND_TYPE_ID, // 602
  SPV_OPERAND_TYPE_VARIABLE_ID, // 603
  SPV_OPERAND_TYPE_TYPE_ID, // 604
  SPV_OPERAND_TYPE_RESULT_ID, // 605
  SPV_OPERAND_TYPE_ID, // 606
  SPV_OPERAND_TYPE_ID, // 607
  SPV_OPERAND_TYPE_ID, // 608
  SPV_OPERAND_TYPE_ID, // 609
  SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS, // 610
  SPV_OPERAND_TYPE_ID, // 611
  SPV_OPERAND_TYPE_ID, // 612
  SPV_OPERAND_TYPE_ID, // 613
  SPV_OPERAND_TYPE_ID, // 614
  SPV_OPERAND_TYPE_ID, // 615
  SPV_OPERAND_TYPE_TYPE_ID, // 616
  SPV_OPERAND_TYPE_RESULT_ID, // 617
  SPV_OPERAND_TYPE_ID, // 618
  SPV_OPERAND_TYPE_ID, // 619
  SPV_OPERAND_TYPE_LITERAL_STRING, // 620
  SPV_OPERAND_TYPE_LITERAL_STRING, // 621
  SPV_OPERAND_TYPE_TYPE_ID, // 622
  SPV_OPERAND_TYPE_RESULT_ID, // 623
  SPV_OPERAND_TYPE_ID, // 624
  SPV_OPERAND_TYPE_ID, // 625
  SPV_OPERAND_TYPE_ID, // 626
  SPV_OPERAND_TYPE_ID, // 627
  SPV_OPERAND_TYPE_ID, // 628
  SPV_OPERAND_TYPE_ID, // 629
  SPV_OPERAND_TYPE_ID, // 630
  SPV_OPERAND_TYPE_TYPE_ID, // 631
  SPV_OPERAND_TYPE_RESULT_ID, // 632
  SPV_OPERAND_TYPE_ID, // 633
  SPV_OPERAND_TYPE_ID, // 634
  SPV_OPERAND_TYPE_ID, // 635
  SPV_OPERAND_TYPE_ID, // 636
  SPV_OPERAND_TYPE_ID, // 637
  SPV_OPERAND_TYPE_ID, // 638
  SPV_OPERAND_TYPE_ID, // 639
  SPV_OPERAND_TYPE_ID, // 640
  SPV_OPERAND_TYPE_TYPE_ID, // 641
  SPV_OPERAND_TYPE_RESULT_ID, // 642
  SPV_OPERAND_TYPE_ID, // 643
  SPV_OPERAND_TYPE_ID, // 644
  SPV_OPERAND_TYPE_ID, // 645
  SPV_OPERAND_TYPE_ID, // 646
  SPV_OPERAND_TYPE_ID, // 647
  SPV_OPERAND_TYPE_ID, // 648
  SPV_OPERAND_TYPE_ID, // 649
  SPV_OPERAND_TYPE_ID, // 650
  SPV_OPERAND_TYPE_ID, // 651
  SPV_OPERAND_TYPE_ID, // 652
  SPV_OPERAND_TYPE_ID, // 653
  SPV_OPERAND_TYPE_TYPE_ID, // 654
  SPV_OPERAND_TYPE_RESULT_ID, // 655
  SPV_OPERAND_TYPE_ID, // 656
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 657
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 658
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 659
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 660
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 661
  SPV_OPERAND_TYPE_TYPE_ID, // 662
  SPV_OPERAND_TYPE_RESULT_ID, // 663
  SPV_OPERAND_TYPE_ID, // 664
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 665
  SPV_OPERAND_TYPE_ID, // 666
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 667
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 668
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 669
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 670
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 671
  SPV_OPERAND_TYPE_TYPE_ID, // 672
  SPV_OPERAND_TYPE_RESULT_ID, // 673
  SPV_OPERAND_TYPE_ID, // 674
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 675
  SPV_OPERAND_TYPE_ID, // 676
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 677
  SPV_OPERAND_TYPE_RESULT_ID, // 678
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 679
  SPV_OPERAND_TYPE_RESULT_ID, // 680
  SPV_OPERAND_TYPE_ID, // 681
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 682
  SPV_OPERAND_TYPE_VARIABLE_ID, // 683
  SPV_OPERAND_TYPE_TYPE_ID, // 684
  SPV_OPERAND_TYPE_RESULT_ID, // 685
  SPV_OPERAND_TYPE_ID, // 686
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 687
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 688
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 689
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 690
  SPV_OPERAND_TYPE_ID, // 691
  SPV_OPERAND_TYPE_ID, // 692
  SPV_OPERAND_TYPE_ID, // 693
  SPV_OPERAND_TYPE_ID, // 694
  SPV_OPERAND_TYPE_ID, // 695
  SPV_OPERAND_TYPE_ID, // 696
  SPV_OPERAND_TYPE_ID, // 697
  SPV_OPERAND_TYPE_ID, // 698
  SPV_OPERAND_TYPE_ID, // 699
  SPV_OPERAND_TYPE_ID, // 700
  SPV_OPERAND_TYPE_ID, // 701
  SPV_OPERAND_TYPE_ID, // 702
  SPV_OPERAND_TYPE_ID, // 703
  SPV_OPERAND_TYPE_ID, // 704
  SPV_OPERAND_TYPE_ID, // 705
  SPV_OPERAND_TYPE_ID, // 706
  SPV_OPERAND_TYPE_ID, // 707
  SPV_OPERAND_TYPE_ID, // 708
  SPV_OPERAND_TYPE_ID, // 709
  SPV_OPERAND_TYPE_TYPE_ID, // 710
  SPV_OPERAND_TYPE_RESULT_ID, // 711
  SPV_OPERAND_TYPE_ID, // 712
  SPV_OPERAND_TYPE_ID, // 713
  SPV_OPERAND_TYPE_ID, // 714
  SPV_OPERAND_TYPE_ID, // 715
  SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS, // 716
  SPV_OPERAND_TYPE_TYPE_ID, // 717
  SPV_OPERAND_TYPE_RESULT_ID, // 718
  SPV_OPERAND_TYPE_ID, // 719
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 720
  SPV_OPERAND_TYPE_ID, // 721
  SPV_OPERAND_TYPE_ID, // 722
  SPV_OPERAND_TYPE_ID, // 723
  SPV_OPERAND_TYPE_ID, // 724
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 725
  SPV_OPERAND_TYPE_ID, // 726
  SPV_OPERAND_TYPE_ID, // 727
  SPV_OPERAND_TYPE_ID, // 728
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 729
  SPV_OPERAND_TYPE_ID, // 730
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 731
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 732
  SPV_OPERAND_TYPE_ID, // 733
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 734
  SPV_OPERAND_TYPE_ID, // 735
  SPV_OPERAND_TYPE_ID, // 736
  SPV_OPERAND_TYPE_ID, // 737
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 738
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 739
  SPV_OPERAND_TYPE_ID, // 740
  SPV_OPERAND_TYPE_ID, // 741
  SPV_OPERAND_TYPE_ID, // 742
  SPV_OPERAND_TYPE_ID, // 743
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 744
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 745
  SPV_OPERAND_TYPE_ID, // 746
  SPV_OPERAND_TYPE_ID, // 747
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 748
  SPV_OPERAND_TYPE_VARIABLE_ID, // 749
  SPV_OPERAND_TYPE_ID, // 750
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 751
  SPV_OPERAND_TYPE_ID, // 752
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 753
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 754
  SPV_OPERAND_TYPE_ID, // 755
  SPV_OPERAND_TYPE_ID, // 756
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 757
  SPV_OPERAND_TYPE_VARIABLE_ID, // 758
  SPV_OPERAND_TYPE_ID, // 759
  SPV_OPERAND_TYPE_ID, // 760
  SPV_OPERAND_TYPE_ID, // 761
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 762
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 763
  SPV_OPERAND_TYPE_ID, // 764
  SPV_OPERAND_TYPE_ID, // 765
  SPV_OPERAND_TYPE_ID, // 766
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 767
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 768
  SPV_OPERAND_TYPE_ID, // 769
  SPV_OPERAND_TYPE_ID, // 770
  SPV_OPERAND_TYPE_ID, // 771
  SPV_OPERAND_TYPE_ID, // 772
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 773
  SPV_OPERAND_TYPE_ID, // 774
  SPV_OPERAND_TYPE_ID, // 775
  SPV_OPERAND_TYPE_ID, // 776
  SPV_OPERAND_TYPE_ID, // 777
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 778
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 779
  SPV_OPERAND_TYPE_ID, // 780
  SPV_OPERAND_TYPE_ID, // 781
  SPV_OPERAND_TYPE_ID, // 782
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 783
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 784
  SPV_OPERAND_TYPE_ID, // 785
  SPV_OPERAND_TYPE_ID, // 786
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 787
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 788
  SPV_OPERAND_TYPE_VARIABLE_ID, // 789
  SPV_OPERAND_TYPE_ID, // 790
  SPV_OPERAND_TYPE_ID, // 791
  SPV_OPERAND_TYPE_ID, // 792
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 793
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 794
  SPV_OPERAND_TYPE_ID, // 795
  SPV_OPERAND_TYPE_ID, // 796
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 797
  SPV_OPERAND_TYPE_ID, // 798
  SPV_OPERAND_TYPE_ID, // 799
  SPV_OPERAND_TYPE_ID, // 800
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 801
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 802
  SPV_OPERAND_TYPE_ID, // 803
  SPV_OPERAND_TYPE_ID, // 804
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 805
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 806
  SPV_OPERAND_TYPE_ID, // 807
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 808
  SPV_OPERAND_TYPE_ID, // 809
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 810
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 811
  SPV_OPERAND_TYPE_ID, // 812
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 813
  SPV_OPERAND_TYPE_ID, // 814
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 815
  SPV_OPERAND_TYPE_ID, // 816
  SPV_OPERAND_TYPE_ID, // 817
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 818
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 819
  SPV_OPERAND_TYPE_ID, // 820
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 821
  SPV_OPERAND_TYPE_ID, // 822
  SPV_OPERAND_TYPE_ID, // 823
  SPV_OPERAND_TYPE_ID, // 824
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 825
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 826
  SPV_OPERAND_TYPE_ID, // 827
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 828
  SPV_OPERAND_TYPE_ID, // 829
  SPV_OPERAND_TYPE_ID, // 830
  SPV_OPERAND_TYPE_VARIABLE_ID, // 831
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 832
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 833
  SPV_OPERAND_TYPE_ID, // 834
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 835
  SPV_OPERAND_TYPE_ID, // 836
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 837
  SPV_OPERAND_TYPE_ID, // 838
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 839
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 840
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 841
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 842
  SPV_OPERAND_TYPE_ID, // 843
  SPV_OPERAND_TYPE_ID, // 844
  SPV_OPERAND_TYPE_ID, // 845
  SPV_OPERAND_TYPE_ID, // 846
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 847
  SPV_OPERAND_TYPE_ID, // 848
  SPV_OPERAND_TYPE_ID, // 849
  SPV_OPERAND_TYPE_ID, // 850
  SPV_OPERAND_TYPE_ID, // 851
  SPV_OPERAND_TYPE_ID, // 852
  SPV_OPERAND_TYPE_ID, // 853
  SPV_OPERAND_TYPE_ID, // 854
  SPV_OPERAND_TYPE_ID, // 855
  SPV_OPERAND_TYPE_VARIABLE_ID, // 856
  SPV_OPERAND_TYPE_ID, // 857
  SPV_OPERAND_TYPE_ID, // 858
  SPV_OPERAND_TYPE_ID, // 859
  SPV_OPERAND_TYPE_ID, // 860
  SPV_OPERAND_TYPE_ID, // 861
  SPV_OPERAND_TYPE_ID, // 862
  SPV_OPERAND_TYPE_ID, // 863
  SPV_OPERAND_TYPE_ID, // 864
  SPV_OPERAND_TYPE_ID, // 865
  SPV_OPERAND_TYPE_VARIABLE_ID, // 866
  SPV_OPERAND_TYPE_ID, // 867
  SPV_OPERAND_TYPE_ID, // 868
  SPV_OPERAND_TYPE_ID, // 869
  SPV_OPERAND_TYPE_ID, // 870
  SPV_OPERAND_TYPE_ID, // 871
  SPV_OPERAND_TYPE_ID, // 872
  SPV_OPERAND_TYPE_ID, // 873
  SPV_OPERAND_TYPE_ID, // 874
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_ID, // 877
  SPV_OPERAND_TYPE_ID, // 878
  SPV_OPERAND_TYPE_ID, // 879
  SPV_OPERAND_TYPE_VARIABLE_ID, // 880
  SPV_OPERAND_TYPE_ID, // 881
  SPV_OPERAND_TYPE_ID, // 882
  SPV_OPERAND_TYPE_ID, // 883
  SPV_OPERAND_TYPE_ID, // 884
  SPV_OPERAND_TYPE_ID, // 885
  SPV_OPERAND_TYPE_ID, // 886
  SPV_OPERAND_TYPE_ID, // 887
  SPV_OPERAND_TYPE_ID, // 888
  SPV_OPERAND_TYPE_ID, // 889
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 890
  SPV_OPERAND_TYPE_ID, // 891
  SPV_OPERAND_TYPE_ID, // 892
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 893
  SPV_OPERAND_TYPE_ID, // 894
  SPV_OPERAND_TYPE_ID, // 895
  SPV_OPERAND_TYPE_ID, // 896
  SPV_OPERAND_TYPE_ID, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_ID, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 901
  SPV_OPERAND_TYPE_ID, // 902
  SPV_OPERAND_TYPE_ID, // 903
  SPV_OPERAND_TYPE_ID, // 904
  SPV_OPERAND_TYPE_VARIABLE_ID, // 905
  SPV_OPERAND_TYPE_ID, // 906
  SPV_OPERAND_TYPE_ID, // 907
  SPV_OPERAND_TYPE_ID, // 908
  SPV_OPERAND_TYPE_ID, // 909
  SPV_OPERAND_TYPE_ID, // 910
  SPV_OPERAND_TYPE_ID, // 911
  SPV_OPERAND_TYPE_ID, // 912
  SPV_OPERAND_TYPE_ID, // 913
  SPV_OPERAND_TYPE_ID, // 914
  SPV_OPERAND_TYPE_ID, // 915
  SPV_OPERAND_TYPE_ID, // 916
  SPV_OPERAND_TYPE_ID, // 917
  SPV_OPERAND_TYPE_ID, // 918
  SPV_OPERAND_TYPE_ID, // 919
  SPV_OPERAND_TYPE_ID, // 920
  SPV_OPERAND_TYPE_ID, // 921
  SPV_OPERAND_TYPE_ID, // 922
  SPV_OPERAND_TYPE_ID, // 923
  SPV_OPERAND_TYPE_ID, // 924
  SPV_OPERAND_TYPE_ID, // 925
  SPV_OPERAND_TYPE_ID, // 926
  SPV_OPERAND_TYPE_ID, // 927
  SPV_OPERAND_TYPE_ID, // 928
  SPV_OPERAND_TYPE_ID, // 929
  SPV_OPERAND_TYPE_ID, // 930
  SPV_OPERAND_TYPE_ID, // 931
  SPV_OPERAND_TYPE_ID, // 932
  SPV_OPERAND_TYPE_ID, // 933
  SPV_OPERAND_TYPE_ID, // 934
  SPV_OPERAND_TYPE_ID, // 935
  SPV_OPERAND_TYPE_ID, // 936
  SPV_OPERAND_TYPE_ID, // 937
  SPV_OPERAND_TYPE_ID, // 938
  SPV_OPERAND_TYPE_ID, // 939
  SPV_OPERAND_TYPE_ID, // 940
  SPV_OPERAND_TYPE_ID, // 941
  SPV_OPERAND_TYPE_ID, // 942
  SPV_OPERAND_TYPE_ID, // 943
  SPV_OPERAND_TYPE_ID, // 944
  SPV_OPERAND_TYPE_ID, // 945
  SPV_OPERAND_TYPE_ID, // 946
  SPV_OPERAND_TYPE_ID, // 947
  SPV_OPERAND_TYPE_ID, // 948
  SPV_OPERAND_TYPE_ID, // 949
  SPV_OPERAND_TYPE_ID, // 950
  SPV_OPERAND_TYPE_ID, // 951
  SPV_OPERAND_TYPE_ID, // 952
  SPV_OPERAND_TYPE_ID, // 953
  SPV_OPERAND_TYPE_ID, // 954
  SPV_OPERAND_TYPE_ID, // 955
  SPV_OPERAND_TYPE_ID, // 956
  SPV_OPERAND_TYPE_ID, // 957
  SPV_OPERAND_TYPE_ID, // 958
  SPV_OPERAND_TYPE_ID, // 959
  SPV_OPERAND_TYPE_ID, // 960
  SPV_OPERAND_TYPE_ID, // 961
  SPV_OPERAND_TYPE_ID, // 962
  SPV_OPERAND_TYPE_ID, // 963
  SPV_OPERAND_TYPE_ID, // 964
  SPV_OPERAND_TYPE_ID, // 965
  SPV_OPERAND_TYPE_ID, // 966
  SPV_OPERAND_TYPE_ID, // 967
  SPV_OPERAND_TYPE_ID, // 968
  SPV_OPERAND_TYPE_ID, // 969
  SPV_OPERAND_TYPE_ID, // 970
  SPV_OPERAND_TYPE_ID, // 971
  SPV_OPERAND_TYPE_ID, // 972
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 973
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 976
  SPV_OPERAND_TYPE_ID, // 977
  SPV_OPERAND_TYPE_ID, // 978
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 979
  SPV_OPERAND_TYPE_ID, // 980
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 981
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 982
  SPV_OPERAND_TYPE_ID, // 983
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 984
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 985
  SPV_OPERAND_TYPE_ID, // 986
  SPV_OPERAND_TYPE_VARIABLE_ID, // 987
  SPV_OPERAND_TYPE_ID, // 988
  SPV_OPERAND_TYPE_ID, // 989
  SPV_OPERAND_TYPE_ID, // 990
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 991
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 992
  SPV_OPERAND_TYPE_ID, // 993
  SPV_OPERAND_TYPE_ID, // 994
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 995
  SPV_OPERAND_TYPE_VARIABLE_ID, // 996
  SPV_OPERAND_TYPE_ID, // 997
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 998
  SPV_OPERAND_TYPE_ID, // 999
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1000
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1001
  SPV_OPERAND_TYPE_ID, // 1002
  SPV_OPERAND_TYPE_ID, // 1003
  SPV_OPERAND_TYPE_ID, // 1004
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1005
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1006
  SPV_OPERAND_TYPE_ID, // 1007
  SPV_OPERAND_TYPE_ID, // 1008
  SPV_OPERAND_TYPE_ID, // 1009
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1010
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1011
  SPV_OPERAND_TYPE_ID, // 1012
  SPV_OPERAND_TYPE_ID, // 1013
  SPV_OPERAND_TYPE_ID, // 1014
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1015
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1016
  SPV_OPERAND_TYPE_ID, // 1017
  SPV_OPERAND_TYPE_ID, // 1018
  SPV_OPERAND_TYPE_ID, // 1019
  SPV_OPERAND_TYPE_ID, // 1020
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1021
  SPV_OPERAND_TYPE_ID, // 1022
  SPV_OPERAND_TYPE_ID, // 1023
  SPV_OPERAND_TYPE_ID, // 1024
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1025
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1026
  SPV_OPERAND_TYPE_ID, // 1027
  SPV_OPERAND_TYPE_ID, // 1028
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1029
  SPV_OPERAND_TYPE_ID, // 1030
  SPV_OPERAND_TYPE_ID, // 1031
  SPV_OPERAND_TYPE_ID, // 1032
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1033
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1034
  SPV_OPERAND_TYPE_ID, // 1035
  SPV_OPERAND_TYPE_ID, // 1036
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1037
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1038
  SPV_OPERAND_TYPE_ID, // 1039
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1040
  SPV_OPERAND_TYPE_ID, // 1041
  SPV_OPERAND_TYPE_ID, // 1042
  SPV_OPERAND_TYPE_ID, // 1043
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1044
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1045
  SPV_OPERAND_TYPE_ID, // 1046
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1047
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1048
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1049
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1050
  SPV_OPERAND_TYPE_ID, // 1051
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1052
  SPV_OPERAND_TYPE_ID, // 1053
  SPV_OPERAND_TYPE_ID, // 1054
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1055
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1056
  SPV_OPERAND_TYPE_ID, // 1057
  SPV_OPERAND_TYPE_ID, // 1058
  SPV_OPERAND_TYPE_ID, // 1059
  SPV_OPERAND_TYPE_ID, // 1060
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1061
  SPV_OPERAND_TYPE_ID, // 1062
  SPV_OPERAND_TYPE_ID, // 1063
  SPV_OPERAND_TYPE_ID, // 1064
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1065
  SPV_OPERAND_TYPE_ID, // 1066
  SPV_OPERAND_TYPE_ID, // 1067
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1068
  SPV_OPERAND_TYPE_ID, // 1069
  SPV_OPERAND_TYPE_ID, // 1070
  SPV_OPERAND_TYPE_ID, // 1071
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1072
};
