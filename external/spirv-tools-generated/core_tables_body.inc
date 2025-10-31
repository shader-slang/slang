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
    case Extension::kSPV_ARM_graph: return IR(426, 14);
    case Extension::kSPV_ARM_tensors: return IR(440, 16);
    case Extension::kSPV_EXT_arithmetic_fence: return IR(456, 25);
    case Extension::kSPV_EXT_demote_to_helper_invocation: return IR(481, 36);
    case Extension::kSPV_EXT_descriptor_indexing: return IR(517, 28);
    case Extension::kSPV_EXT_float8: return IR(545, 15);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(560, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(591, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(627, 34);
    case Extension::kSPV_EXT_mesh_shader: return IR(661, 20);
    case Extension::kSPV_EXT_opacity_micromap: return IR(681, 25);
    case Extension::kSPV_EXT_optnone: return IR(706, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(722, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(754, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(798, 30);
    case Extension::kSPV_EXT_shader_64bit_indexing: return IR(828, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(858, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(892, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(924, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(960, 27);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(987, 30);
    case Extension::kSPV_EXT_shader_tile_image: return IR(1017, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1043, 36);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1079, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1106, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1137, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1158, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(1180, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(1222, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(1267, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(1306, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(1336, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(1362, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(1387, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(1412, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(1435, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(1479, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(1505, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(1533, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(1556, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(1591, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(1622, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(1656, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(1683, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(1731, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(1762, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(1791, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(1822, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(1855, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(1874, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(1902, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(1930, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(1973, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2011, 26);
    case Extension::kSPV_INTEL_int4: return IR(2037, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2052, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2071, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2099, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2125, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2145, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2177, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2205, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2230, 33);
    case Extension::kSPV_INTEL_optnone: return IR(2263, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(2281, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(2307, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(2343, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(2367, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(2402, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(2448, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(2468, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(2492, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(2528, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(2563, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(2600, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(2630, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(2662, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(2687, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(2709, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(2730, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(2747, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(2772, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(2807, 27);
    case Extension::kSPV_KHR_device_group: return IR(2834, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(2855, 22);
    case Extension::kSPV_KHR_float_controls: return IR(2877, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(2900, 24);
    case Extension::kSPV_KHR_fma: return IR(2924, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(2936, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(2972, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3002, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3030, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3051, 30);
    case Extension::kSPV_KHR_multiview: return IR(3081, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3099, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3134, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3160, 32);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3192, 28);
    case Extension::kSPV_KHR_quad_control: return IR(3220, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(3241, 22);
    case Extension::kSPV_KHR_ray_query: return IR(3263, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(3281, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(3301, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(3336, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(3373, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(3407, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(3429, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(3450, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(3481, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(3518, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(3542, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(3580, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(3602, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(3631, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(3666, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(3691, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(3717, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(3745, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(3786, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(3824, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(3848, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(3886, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(3920, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(3946, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(3973, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(3999, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(4028, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(4063, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(4098, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4126, 19);
    case Extension::kSPV_NV_raw_access_chains: return IR(4145, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4170, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(4189, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(4220, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(4257, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(4290, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(4320, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(4353, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(4379, 35);
    case Extension::kSPV_NV_shading_rate: return IR(4414, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(4434, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(4463, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(4488, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(4511, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(4550, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(4576, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(4603, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(4625, 38);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,162> kExtensionNames{{
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
    {IR(426, 14), static_cast<uint32_t>(kSPV_ARM_graph)},
    {IR(440, 16), static_cast<uint32_t>(kSPV_ARM_tensors)},
    {IR(456, 25), static_cast<uint32_t>(kSPV_EXT_arithmetic_fence)},
    {IR(481, 36), static_cast<uint32_t>(kSPV_EXT_demote_to_helper_invocation)},
    {IR(517, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(545, 15), static_cast<uint32_t>(kSPV_EXT_float8)},
    {IR(560, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(591, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(627, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(661, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(681, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(706, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(722, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(754, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(798, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(828, 30), static_cast<uint32_t>(kSPV_EXT_shader_64bit_indexing)},
    {IR(858, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(892, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(924, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(960, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(987, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(1017, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1043, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1079, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1106, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1137, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1158, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(1180, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(1222, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(1267, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(1306, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(1336, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(1362, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(1387, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(1412, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(1435, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(1479, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(1505, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(1533, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(1556, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(1591, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(1622, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(1656, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(1683, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(1731, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(1762, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(1791, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(1822, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(1855, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(1874, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(1902, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(1930, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(1973, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2011, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2037, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2052, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2071, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2099, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2125, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2145, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2177, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2205, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2230, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(2263, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(2281, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(2307, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(2343, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(2367, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(2402, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(2448, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(2468, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(2492, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(2528, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(2563, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(2600, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(2630, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(2662, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(2687, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(2709, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(2730, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(2747, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(2772, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(2807, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(2834, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(2855, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(2877, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(2900, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(2924, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(2936, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(2972, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3002, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3030, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3051, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3081, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3099, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3134, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3160, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3192, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(3220, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(3241, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(3263, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(3281, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(3301, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(3336, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(3373, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(3407, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(3429, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(3450, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(3481, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(3518, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(3542, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(3580, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(3602, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(3631, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(3666, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(3691, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(3717, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(3745, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(3786, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(3824, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(3848, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(3886, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(3920, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(3946, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(3973, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(3999, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(4028, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(4063, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(4098, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4126, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4145, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4170, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(4189, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(4220, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(4257, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(4290, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(4320, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(4353, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(4379, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(4414, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(4434, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(4463, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(4488, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(4511, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(4550, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(4576, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(4603, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(4625, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
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
static const std::array<OperandDesc, 1191> kOperandsByValue{{
  {0,IR(0, 0),IR(4663, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(4679, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(4689, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(4699, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(4707, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(4728, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(4739, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(4821, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(4837, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(4847, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(4860, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(4873, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(4882, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(4893, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(4983, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(4996, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(5042, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(5090, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(5105, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(5120, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(5130, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5144, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(5154, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(5165, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(5177, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(5204, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(5219, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(5230, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(5240, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(5257, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(5271, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(5285, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(5297, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(5315, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(5334, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(5355, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(5363, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(5374, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(5396, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(5409, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(5424, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(5471, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(5487, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(5500, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(5521, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(5532, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(5558, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(5570, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(5584, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(5610, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(5623, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(5636, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(5646, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(5659, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(5714, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(5747, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(5780, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(5813, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(5846, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(5872, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(5885, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(5895, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(5942, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(5966, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(5986, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(6001, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(6032, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(6050, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(6068, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(6088, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(6116, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6142, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(6161, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(6188, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(6213, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(6235, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(6270, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(6317, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(6333, 15)/* ViewportMaskNV */,IR(0, 0),IR(44, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(6369, 20)/* SecondaryPositionNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(6408, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(6432, 18)/* PositionPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(6470, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(6492, 16)/* FullyCoveredEXT */,IR(0, 0),IR(49, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(6532, 12)/* TaskCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(6544, 17)/* PrimitiveCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(6561, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(6580, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(6602, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(6624, 15)/* LayerPerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(6639, 16)/* MeshViewCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(6655, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(6673, 13)/* BaryCoordKHR */,IR(6, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(6721, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(6760, 12)/* FragSizeEXT */,IR(8, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(6806, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(6851, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(6876, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(6900, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(6928, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(6945, 12)/* LaunchIdKHR */,IR(10, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(6968, 14)/* LaunchSizeKHR */,IR(11, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(6995, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(7030, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(7071, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(7108, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(7151, 11)/* RayTminKHR */,IR(16, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(7172, 11)/* RayTmaxKHR */,IR(17, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(7193, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(7238, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(7271, 17)/* WorldToObjectKHR */,IR(20, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(7304, 7)/* HitTNV */,IR(0, 0),IR(56, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(7311, 11)/* HitKindKHR */,IR(21, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(7332, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(57, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(7372, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(58, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(7429, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(7496, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(7533, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(7572, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(60, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(7592, 14)/* HitIsSphereNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(7634, 11)/* HitIsLSSNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(7684, 20)/* HitSpherePositionNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(7704, 13)/* WarpsPerSMNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(7736, 10)/* SMCountNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(7746, 9)/* WarpIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(7755, 7)/* SMIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(7762, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(7780, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(7814, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(7847, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(7865, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(7879, 12)/* ClusterIDNV */,IR(0, 0),IR(64, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(7932, 12)/* CullMaskKHR */,IR(0, 0),IR(65, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(7959, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(4830, 7)/* Shader */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(4905, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(4914, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(4718, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(7966, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(4672, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(7974, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(7983, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(7997, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(8005, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(8013, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(8019, 13)/* Int64Atomics */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(8032, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(8043, 15)/* ImageReadWrite */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(8058, 12)/* ImageMipmap */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(8070, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(8076, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(8083, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(8097, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(8112, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(8126, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(8132, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(8154, 18)/* GeometryPointSize */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(8172, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(8192, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(8216, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(8250, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(8283, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(8317, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(4847, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(4860, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(8350, 15)/* ImageCubeArray */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(5186, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(8382, 10)/* ImageRect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(8392, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(8404, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(8419, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(8424, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(8440, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(8456, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(8463, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(8473, 8)/* Image1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(8365, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(8481, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(8495, 12)/* ImageBuffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(8507, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(8520, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(8548, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(8559, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(8577, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(8599, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(8617, 16)/* GeometryStreams */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(8633, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(8663, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(5056, 14)/* MultiViewport */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(8694, 17)/* SubgroupDispatch */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(8711, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(8724, 12)/* PipeStorage */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(5437, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(8736, 20)/* GroupNonUniformVote */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(8756, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(5692, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(8782, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(8805, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(8836, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(8861, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(5002, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(5070, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(8881, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(5594, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(8899, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(8927, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(8955, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(8985, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(8996, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(9033, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(9073, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(9082, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(9110, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(9120, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(77, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(5919, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(5453, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(5857, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(9168, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(9201, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(9244, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(9288, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(9304, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(9357, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(80, 1),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(9409, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(9431, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(5954, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(5976, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(9452, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(9482, 17)/* VariablePointers */,IR(0, 0),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(9499, 17)/* AtomicStorageOps */,IR(0, 0),IR(82, 1),IR(44, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(9516, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(9544, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(9568, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(83, 1),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(9602, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(9623, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(9638, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(9656, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(9681, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(9697, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(9713, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(9736, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(9748, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(9767, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(84, 2),IR(50, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(4940, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(9799, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(9825, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(9846, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(6016, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(9868, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(86, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(9900, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(9923, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(9939, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(9961, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(6253, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(9977, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(9998, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(10012, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(6299, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(10027, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(10042, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(10056, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(87, 2),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(10083, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(10099, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(89, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(10121, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(90, 2),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(10150, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(10179, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(69, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(5014, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(92, 1),IR(68, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(6348, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(93, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(6389, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(94, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(6450, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(6508, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(4954, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(10234, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(4968, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(6698, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(10273, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(6787, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(10348, 29)/* GroupNonUniformPartitionedNV */,IR(0, 0),IR(1, 0),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(10377, 17)/* ShaderNonUniform */,IR(29, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(10414, 23)/* RuntimeDescriptorArray */,IR(30, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(10463, 36)/* InputAttachmentArrayDynamicIndexing */,IR(31, 1),IR(95, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(10538, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(32, 1),IR(73, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(10619, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(96, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(10700, 37)/* UniformBufferArrayNonUniformIndexing */,IR(34, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(10777, 36)/* SampledImageArrayNonUniformIndexing */,IR(35, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(10852, 37)/* StorageBufferArrayNonUniformIndexing */,IR(36, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(10929, 36)/* StorageImageArrayNonUniformIndexing */,IR(37, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(11004, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(38, 1),IR(98, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(11085, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(39, 1),IR(100, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(11172, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(102, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(7402, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(4927, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(7349, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(11259, 18)/* VulkanMemoryModel */,IR(41, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(11298, 29)/* VulkanMemoryModelDeviceScope */,IR(42, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(4790, 31)/* PhysicalStorageBufferAddresses */,IR(43, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(11393, 32)/* ComputeDerivativeGroupLinearKHR */,IR(44, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(11456, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(11481, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(11501, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(11534, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(7717, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(11572, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(11604, 25)/* DemoteToHelperInvocation */,IR(45, 1),IR(3, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(11657, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(11680, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(11709, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(60, 1),IR(84, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(11735, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(11753, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(11778, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(11798, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(7463, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(60, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(11820, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(7606, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(7645, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(11838, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(11861, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(11891, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(11922, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(11962, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(11998, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(12028, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(7891, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(60, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(12056, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(12075, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(12096, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(12123, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(12149, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(12180, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(12201, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(12224, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(12247, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(12269, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(12293, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(12302, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(12325, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(12348, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(12371, 19)/* VectorComputeINTEL */,IR(0, 0),IR(104, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(12390, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(12405, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(12421, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(12454, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(12492, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(12531, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(12556, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(12582, 26)/* FPGAMemoryAttributesINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(12608, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(12628, 32)/* ArbitraryPrecisionIntegersINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(12660, 37)/* ArbitraryPrecisionFloatingPointINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(12697, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(12727, 22)/* FPGALoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(12749, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(12771, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(12797, 24)/* FPGAMemoryAccessesINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(12821, 27)/* FPGAClusterAttributesINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(12848, 14)/* LoopFuseINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(12862, 20)/* FPGADSPControlINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(12882, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(12908, 40)/* FPGAInvocationPipeliningAttributesINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(12948, 24)/* FPGABufferLocationINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(12972, 34)/* ArbitraryPrecisionFixedPointINTEL */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(13006, 23)/* USMStorageClassesINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(13029, 29)/* RuntimeAlignedAttributeINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(13058, 13)/* IOPipesINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(13071, 19)/* BlockingPipesINTEL */,IR(0, 0),IR(1, 0),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(13090, 13)/* FPGARegINTEL */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(13103, 19)/* DotProductInputAll */,IR(46, 1),IR(1, 0),IR(123, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(13144, 22)/* DotProductInput4x8Bit */,IR(47, 1),IR(105, 1),IR(123, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(13191, 28)/* DotProductInput4x8BitPacked */,IR(48, 1),IR(1, 0),IR(123, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(13250, 11)/* DotProduct */,IR(49, 1),IR(1, 0),IR(123, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(7944, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(9147, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(13275, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(125, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(13299, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(126, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(13315, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(76, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(13340, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(13355, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(129, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(13362, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(13382, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(13402, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(13422, 11)/* OptNoneEXT */,IR(50, 1),IR(1, 0),IR(132, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(13446, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(134, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(13466, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(135, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(13487, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(13511, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(137, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(13529, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(138, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(13548, 29)/* FPGAClusterAttributesV2INTEL */,IR(0, 0),IR(106, 1),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(13577, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(107, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(13605, 18)/* TaskSequenceINTEL */,IR(0, 0),IR(1, 0),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(13623, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(140, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(13639, 24)/* FPGALatencyControlINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(13663, 28)/* FPGAArgumentInterfacesINTEL */,IR(0, 0),IR(1, 0),IR(142, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(13691, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(13721, 35)/* GlobalVariableFPGADecorationsINTEL */,IR(0, 0),IR(1, 0),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(13756, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(145, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(13784, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(13807, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(108, 1),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(13837, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(108, 1),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(13867, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(13905, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(148, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(13933, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(109, 2),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(13965, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(13986, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(111, 1),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(14008, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(14034, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(14061, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(14086, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(153, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(14105, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(154, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(14125, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(155, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(14145, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14157, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14165, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(14173, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14179, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14186, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14197, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14206, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14219, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14225, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14235, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14241, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14256, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14281, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14297, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14311, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14324, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14336, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14353, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14365, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14380, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14393, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14408, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14426, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14443, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14464, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14484, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14504, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14520, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14536, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14556, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14580, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14586, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14591, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(14630, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(14641, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14650, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14655, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14662, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(14673, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(14680, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14689, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14699, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14712, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14725, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(14736, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(14746, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(14756, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(14766, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(14779, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(14793, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(14807, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(14821, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(14836, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(14852, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(14868, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(14884, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(14903, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(14924, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(14936, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(14948, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14960, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(14975, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(15000, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(15028, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(15036, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(15063, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(15090, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(15117, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(15149, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(15175, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(15179, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(15186, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(15190, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(15201, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(15212, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(15233, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(15244, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(15258, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(15279, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(14145, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14157, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14165, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14173, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14179, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14186, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14197, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(14206, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14219, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14225, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14235, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14281, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14297, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14311, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14324, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14336, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14353, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14365, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14380, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14393, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14408, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14426, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14443, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14464, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14484, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14504, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14580, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(14586, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(14591, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(14630, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(14641, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(14650, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(14655, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(14662, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(14673, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(14689, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14699, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14712, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(15297, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(15314, 7)/* SpecId */,IR(0, 0),IR(112, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(15321, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(15327, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(15339, 9)/* RowMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(15348, 9)/* ColMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(15357, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(15369, 13)/* MatrixStride */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(15382, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(15393, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(15404, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(15438, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(15446, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(15460, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(15465, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(15471, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(15480, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(15487, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(15497, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(15506, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(15514, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(15523, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(15532, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(15541, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(15553, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(15565, 8)/* Uniform */,IR(0, 0),IR(114, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(15599, 10)/* UniformId */,IR(0, 0),IR(114, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(15609, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(15629, 7)/* Stream */,IR(0, 0),IR(116, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(15636, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(15645, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(15655, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(15661, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(15669, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(15683, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(15690, 10)/* XfbBuffer */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(15700, 10)/* XfbStride */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(15756, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(15804, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(15854, 15)/* FPFastMathMode */,IR(0, 0),IR(118, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(15931, 18)/* LinkageAttributes */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(15949, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(15963, 21)/* InputAttachmentIndex */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(15984, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(15994, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(16028, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(16040, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(16056, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(16100, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(16113, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(16128, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(16146, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(16168, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(16190, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(16208, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(16240, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(16260, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(16283, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(16303, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(16328, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(16355, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(16380, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(16408, 19)/* OverrideCoverageNV */,IR(0, 0),IR(122, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(16427, 14)/* PassthroughNV */,IR(0, 0),IR(123, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(16441, 19)/* ViewportRelativeNV */,IR(0, 0),IR(94, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(16460, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(16488, 16)/* PerPrimitiveEXT */,IR(51, 1),IR(124, 2),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(16519, 10)/* PerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(16529, 10)/* PerTaskNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(16539, 13)/* PerVertexKHR */,IR(52, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(16564, 11)/* NonUniform */,IR(53, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(16589, 16)/* RestrictPointer */,IR(54, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(16624, 15)/* AliasedPointer */,IR(55, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(16657, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(16687, 18)/* BindlessSamplerNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(16705, 16)/* BindlessImageNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(16721, 15)/* BoundSamplerNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(16736, 13)/* BoundImageNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(16749, 14)/* SIMTCallINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(16763, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(129, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(16789, 13)/* ClobberINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(16802, 17)/* SideEffectsINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(16819, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(16846, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(16867, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(16894, 15)/* StackCallINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(16909, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(16935, 14)/* CounterBuffer */,IR(56, 1),IR(1, 0),IR(159, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(16973, 13)/* UserSemantic */,IR(57, 1),IR(1, 0),IR(159, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(17005, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(17020, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(17077, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(17101, 14)/* RegisterINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(17115, 12)/* MemoryINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(17127, 14)/* NumbanksINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(17141, 15)/* BankwidthINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(17156, 22)/* MaxPrivateCopiesINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(17178, 16)/* SinglepumpINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(17194, 16)/* DoublepumpINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(17210, 19)/* MaxReplicatesINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(17229, 20)/* SimpleDualPortINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(17249, 11)/* MergeINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(17302, 14)/* BankBitsINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(17316, 20)/* ForcePow2DepthINTEL */,IR(0, 0),IR(132, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(17336, 16)/* StridesizeINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(17352, 14)/* WordsizeINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(17366, 18)/* TrueDualPortINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(17384, 19)/* BurstCoalesceINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(17403, 15)/* CacheSizeINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(17418, 28)/* DontStaticallyCoalesceINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(17446, 14)/* PrefetchINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(17460, 17)/* StallEnableINTEL */,IR(0, 0),IR(106, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(17477, 25)/* FuseLoopsInFunctionINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(17502, 19)/* MathOpDSPModeINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(17521, 16)/* AliasScopeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(17537, 13)/* NoAliasINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(17550, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(17574, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(17594, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(17614, 20)/* BufferLocationINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(17634, 19)/* IOPipeStorageINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(17687, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(17718, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(17743, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(17778, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(17796, 15)/* StallFreeINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(17842, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(17868, 25)/* LatencyControlLabelINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(17893, 30)/* LatencyControlConstraintINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(17923, 27)/* ConduitKernelArgumentINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(17950, 31)/* RegisterMapKernelArgumentINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(17981, 33)/* MMHostInterfaceAddressWidthINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(18014, 30)/* MMHostInterfaceDataWidthINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(18044, 28)/* MMHostInterfaceLatencyINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(18106, 34)/* MMHostInterfaceReadWriteModeINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(18140, 29)/* MMHostInterfaceMaxBurstINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(18169, 32)/* MMHostInterfaceWaitRequestINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(18201, 26)/* StableKernelArgumentINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(18266, 16)/* HostAccessINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(18329, 14)/* InitModeINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(18343, 28)/* ImplementInRegisterMapINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(18371, 17)/* ConditionalINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(18424, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(18483, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(18506, 3)/* 1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(18509, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(18512, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(18515, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(18520, 5)/* Rect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(18525, 7)/* Buffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(18532, 12)/* SubpassData */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(18544, 17)/* TileImageDataEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(18561, 12)/* Invocations */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(18573, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(18586, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(18608, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(18629, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(18643, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(18658, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(18677, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(18693, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(18709, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(18728, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(18738, 4)/* Xfb */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(18742, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(18757, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(18770, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(18780, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(18795, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(18805, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(18819, 12)/* InputPoints */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(18831, 11)/* InputLines */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(18842, 20)/* InputLinesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(18862, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(18872, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(18896, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(18902, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(18911, 15)/* OutputVertices */,IR(0, 0),IR(148, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(18926, 13)/* OutputPoints */,IR(0, 0),IR(152, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(18939, 16)/* OutputLineStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(18955, 20)/* OutputTriangleStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(18975, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(18987, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(19002, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(19014, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(5424, 13)/* SubgroupSize */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(19024, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(19046, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(19070, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(19082, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(19098, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(19132, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(19166, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(19202, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(19232, 18)/* PostDepthCoverage */,IR(0, 0),IR(158, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(9623, 15)/* DenormPreserve */,IR(0, 0),IR(159, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(9638, 18)/* DenormFlushToZero */,IR(0, 0),IR(160, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(9656, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(161, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(9681, 16)/* RoundingModeRTE */,IR(0, 0),IR(162, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(9697, 16)/* RoundingModeRTZ */,IR(0, 0),IR(163, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(19250, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(19284, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(19304, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(19333, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(19356, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(19371, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(19386, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(19407, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(6317, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(19431, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(19452, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(19480, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(19506, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(19529, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(19556, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(19581, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(19603, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(164, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(19622, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(164, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(19642, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(19662, 15)/* OutputLinesEXT */,IR(58, 1),IR(124, 2),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(19691, 20)/* OutputPrimitivesEXT */,IR(59, 1),IR(124, 2),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(19730, 24)/* DerivativeGroupQuadsKHR */,IR(60, 1),IR(165, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(19777, 25)/* DerivativeGroupLinearKHR */,IR(61, 1),IR(166, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(19826, 19)/* OutputTrianglesEXT */,IR(62, 1),IR(124, 2),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(19863, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(19888, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(19915, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(168, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(19941, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(168, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(19969, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(169, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(20000, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(169, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(11838, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(20033, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(20060, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(20081, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(20102, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(20128, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(20155, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(172, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(20177, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(172, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(20193, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(172, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(20213, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(107, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(20235, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(20263, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(20287, 18)/* FPFastMathDefault */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(20305, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(20329, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(174, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(20355, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(20378, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(20400, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(20475, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(20502, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(20509, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(20529, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(4905, 9)/* Geometry */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(14680, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(20552, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(4672, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(20562, 7)/* TaskNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(20569, 7)/* MeshNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(20576, 17)/* RayGenerationKHR */,IR(63, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(20609, 16)/* IntersectionKHR */,IR(64, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(20640, 10)/* AnyHitKHR */,IR(65, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(20659, 14)/* ClosestHitKHR */,IR(66, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(20686, 8)/* MissKHR */,IR(67, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(20701, 12)/* CallableKHR */,IR(68, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(20724, 8)/* TaskEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(20732, 8)/* MeshEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(20740, 9)/* Preserve */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(20749, 12)/* FlushToZero */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(20761, 12)/* BFloat16KHR */,IR(0, 0),IR(89, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(20773, 14)/* Float8E4M3EXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(20787, 14)/* Float8E5M2EXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(20801, 5)/* IEEE */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(20806, 4)/* ALT */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(20810, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(20817, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(20824, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(20828, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(20839, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(20844, 14)/* AllowContract */,IR(69, 1),IR(176, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(20881, 13)/* AllowReassoc */,IR(70, 1),IR(176, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(20912, 15)/* AllowTransform */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(20927, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20931, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(20935, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(20939, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20943, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(20959, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(20975, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(20993, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(21011, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(21018, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(21029, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(21034, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(13422, 11)/* OptNoneEXT */,IR(50, 1),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(21040, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(21045, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(21050, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(21056, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(21061, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(21069, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(21079, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(21087, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(21099, 20)/* RuntimeAlignedINTEL */,IR(0, 0),IR(179, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(21119, 7)/* Reduce */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(21126, 14)/* InclusiveScan */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(21140, 14)/* ExclusiveScan */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(21154, 16)/* ClusteredReduce */,IR(0, 0),IR(183, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(21170, 20)/* PartitionedReduceNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(21190, 27)/* PartitionedInclusiveScanNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(21217, 27)/* PartitionedExclusiveScanNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(21244, 10)/* NoneINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(21254, 10)/* ReadINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(21264, 11)/* WriteINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(21275, 15)/* ReadWriteINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(21290, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(21295, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(21299, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(21304, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(15683, 7)/* Offset */,IR(0, 0),IR(185, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(21316, 13)/* ConstOffsets */,IR(0, 0),IR(185, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(15480, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(8456, 7)/* MinLod */,IR(0, 0),IR(186, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(21329, 19)/* MakeTexelAvailable */,IR(71, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(21370, 17)/* MakeTexelVisible */,IR(72, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(21407, 16)/* NonPrivateTexel */,IR(73, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(21442, 14)/* VolatileTexel */,IR(74, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(21473, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(21484, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(21495, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(21507, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(21515, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(21525, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(21536, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(21546, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(21557, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(21571, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(21585, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(21600, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(21611, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(21623, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(21635, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(21648, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(21662, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(21676, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(14173, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(21686, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(21697, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(21714, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(21730, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(21750, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(21770, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(21790, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(21809, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(21828, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(21847, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(21863, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(21879, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(21881, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(21883, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(21886, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(21889, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(21893, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(21898, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(21903, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(21908, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(21918, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(21928, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(21931, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(21935, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(21940, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(21946, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(21959, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(21964, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(21970, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(21976, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(21982, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(21987, 27)/* InitOnDeviceReprogramINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(22014, 23)/* InitOnDeviceResetINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(22037, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(22044, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(22055, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(22069, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(22081, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(22094, 7)/* Export */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(22101, 7)/* Import */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(22108, 12)/* LinkOnceODR */,IR(0, 0),IR(120, 1),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(22120, 14)/* UncachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(22134, 12)/* CachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(22146, 15)/* StreamingINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(22161, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(22186, 17)/* ConstCachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(22203, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(22210, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(22221, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(22240, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(22257, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(22271, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(22285, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(22303, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(22313, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(17550, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(17574, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(22326, 21)/* DependencyArrayINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(17594, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(22347, 18)/* LoopCoalesceINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(22365, 21)/* MaxInterleavingINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(22386, 26)/* SpeculatedIterationsINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(22412, 14)/* NoFusionINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(22426, 15)/* LoopCountINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(22441, 26)/* MaxReinvocationDelayINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(22467, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(22496, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(22525, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(22546, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(22572, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(22595, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(22618, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(22641, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(22664, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(22681, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(22698, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(22724, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(22750, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(22777, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(15514, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(22804, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(21495, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(22812, 21)/* MakePointerAvailable */,IR(75, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(22857, 19)/* MakePointerVisible */,IR(76, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(22898, 18)/* NonPrivatePointer */,IR(77, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(22937, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(136, 1),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(22957, 17)/* NoAliasINTELMask */,IR(0, 0),IR(136, 1),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(22974, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(22981, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(22989, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(22996, 7)/* Vulkan */,IR(78, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(23013, 8)/* Relaxed */,IR(79, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(23021, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(23029, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(23037, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(23052, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(23075, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(23089, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(23104, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(23120, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(23141, 20)/* AtomicCounterMemory */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(23161, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(23173, 13)/* OutputMemory */,IR(80, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(23202, 14)/* MakeAvailable */,IR(81, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(23233, 12)/* MakeVisible */,IR(82, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(15514, 9)/* Volatile */,IR(0, 0),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(23260, 10)/* AutoINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(23270, 5)/* WRAP */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(23275, 4)/* SAT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(23279, 9)/* SAT_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(23288, 8)/* SAT_SYM */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(23296, 25)/* PackedVectorFormat4x8Bit */,IR(83, 1),IR(1, 0),IR(123, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(23349, 4)/* TRN */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(23353, 9)/* TRN_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(23362, 4)/* RND */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(23366, 9)/* RND_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(23375, 8)/* RND_INF */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(23383, 12)/* RND_MIN_INF */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(23395, 9)/* RND_CONV */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(23404, 13)/* RND_CONV_ODD */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(23417, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(23442, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(15028, 8)/* NoneKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(23465, 10)/* OpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(23475, 12)/* NoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(23487, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(23510, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(23534, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(23561, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(23589, 14)/* CullOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(23603, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(23619, 17)/* SkipTrianglesKHR */,IR(84, 1),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(23660, 13)/* SkipAABBsKHR */,IR(0, 0),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(23673, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(192, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(23703, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23744, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23781, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23818, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(23859, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23901, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(23934, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(23967, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(23979, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(23985, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(23992, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(24007, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(24015, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(24022, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(24030, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(24038, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(24046, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(24051, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(24057, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(24068, 6)/* Rg32f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(24074, 6)/* Rg16f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(24080, 13)/* R11fG11fB10f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(24093, 5)/* R16f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(24098, 7)/* Rgba16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(24105, 8)/* Rgb10A2 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(24113, 5)/* Rg16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(24118, 4)/* Rg8 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(24122, 4)/* R16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(24126, 3)/* R8 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(24129, 12)/* Rgba16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(24141, 10)/* Rg16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(24151, 9)/* Rg8Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(24160, 9)/* R16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(24169, 8)/* R8Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(24177, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(24185, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(24193, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(24200, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(24205, 6)/* Rg32i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(24211, 6)/* Rg16i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(24217, 5)/* Rg8i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(24222, 5)/* R16i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(24227, 4)/* R8i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(24231, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(24240, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(24249, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(24257, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(24263, 10)/* Rgb10a2ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(24273, 7)/* Rg32ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(24280, 7)/* Rg16ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(24287, 6)/* Rg8ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(24293, 6)/* R16ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(24299, 5)/* R8ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(24304, 6)/* R64ui */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(24310, 5)/* R64i */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(24315, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(24327, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(24334, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(24344, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(24353, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(24364, 12)/* QueueFamily */,IR(85, 1),IR(187, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(24391, 14)/* ShaderCallKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24405, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(24413, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24425, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(14145, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14157, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14165, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(14173, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14179, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14186, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14197, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14206, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14219, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14225, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14235, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14241, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14256, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14281, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14297, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14311, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14324, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14336, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14353, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14365, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14380, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14393, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14408, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14426, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14443, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14464, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14484, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14504, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14520, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14536, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14556, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(24454, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14580, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14586, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14591, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(14630, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(14641, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14650, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14655, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14662, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(14673, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(14680, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14689, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14699, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14712, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14725, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(24022, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(24480, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(24485, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(24490, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(24499, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(24510, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(24515, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(24530, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(24535, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(24542, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(24547, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(24552, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(24558, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(24562, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(24567, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(24583, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(15565, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(24589, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(24334, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(24596, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(24611, 8)/* Private */,IR(0, 0),IR(196, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(24619, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(24628, 8)/* Generic */,IR(0, 0),IR(198, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(24636, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(24649, 14)/* AtomicCounter */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(24663, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(24669, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(167, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(24683, 13)/* TileImageEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(24696, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(24715, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(24731, 16)/* CallableDataKHR */,IR(86, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(24762, 24)/* IncomingCallableDataKHR */,IR(87, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(24809, 14)/* RayPayloadKHR */,IR(88, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(24836, 16)/* HitAttributeKHR */,IR(89, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(24867, 22)/* IncomingRayPayloadKHR */,IR(90, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(24910, 22)/* ShaderRecordBufferKHR */,IR(91, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(24953, 22)/* PhysicalStorageBuffer */,IR(92, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(25000, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(25021, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(53, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(25045, 17)/* CodeSectionINTEL */,IR(0, 0),IR(199, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(25062, 16)/* DeviceOnlyINTEL */,IR(0, 0),IR(200, 1),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(25078, 14)/* HostOnlyINTEL */,IR(0, 0),IR(200, 1),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(22120, 14)/* UncachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(25092, 18)/* WriteThroughINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(25110, 15)/* WriteBackINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(22146, 15)/* StreamingINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(14276, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(25125, 11)/* TensorView */,IR(0, 0),IR(201, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(25136, 11)/* DecodeFunc */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(25147, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(15523, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(23967, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(23985, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(23992, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(25157, 8)/* NoneARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(25165, 15)/* NontemporalARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(25180, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(25200, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(25224, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(25246, 21)/* NonPrivateElementARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(133, 276);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(409, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(417, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(420, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(422, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(441, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(451, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(455, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(470, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(474, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(480, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(483, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(486, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(490, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(498, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(501, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(517, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(526, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(529, 144);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(673, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(681, 97);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(778, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(795, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(797, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(800, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(802, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(811, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(815, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(819, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(825, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(834, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(841, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(845, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(862, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(888, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(908, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(910, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(913, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(915, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(917, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(920, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(925, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(945, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(960, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(969, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(973, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(988, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(989, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(993, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(994, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1002, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1005, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1017, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1019, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1022, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1024, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1029, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1031, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1073, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1080, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1083, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1084, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1092, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1095, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1097, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1117, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1127, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1131, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1145, 28);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1173, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1177, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1180, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1185, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(845, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(960, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(993, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(474, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(945, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1002, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(797, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1185, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(133, 276);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1285> kOperandNames{{
  {IR(4663, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4689, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4679, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4699, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4707, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4728, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4739, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4763, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6673, 13), 92}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6686, 12), 92}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6068, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6088, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6721, 20), 93}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6741, 19), 93}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6116, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6213, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6142, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6161, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6188, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5872, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5846, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4847, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6580, 22), 87}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7879, 12), 131}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5610, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5584, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5623, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4860, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6602, 22), 88}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7932, 12), 132}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6928, 17), 99}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7332, 17), 113}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5942, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5885, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5374, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5144, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5230, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6806, 23), 95}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6760, 12), 94}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6235, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6772, 15), 94}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5165, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6492, 16), 83}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5315, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5409, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5396, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5363, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5240, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7634, 11), 120}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7592, 14), 119}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7814, 33), 128}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7780, 34), 127}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7311, 11), 112}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7322, 10), 112}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7762, 18), 126}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7865, 14), 130}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7496, 37), 116}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7429, 34), 115}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7684, 20), 121}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7847, 18), 129}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7304, 7), 111}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7372, 30), 114}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7533, 20), 117}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7553, 19), 117}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7193, 23), 108}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7216, 22), 108}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4882, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5570, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4983, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6829, 22), 95}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6945, 12), 100}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6957, 11), 100}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6968, 14), 101}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6982, 13), 101}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4996, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6624, 15), 89}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5297, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5334, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6639, 16), 90}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6655, 18), 91}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5500, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5487, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5257, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7108, 22), 105}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7130, 21), 105}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7071, 19), 104}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7090, 18), 104}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7238, 17), 109}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7255, 16), 109}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5130, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5154, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4837, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4821, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6432, 18), 81}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6544, 17), 85}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4893, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6561, 19), 86}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6876, 24), 97}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6851, 25), 96}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5895, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6900, 28), 98}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7572, 20), 118}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7172, 11), 107}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7183, 10), 107}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7151, 11), 106}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7162, 10), 106}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6270, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7736, 10), 123}, // 107 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7755, 7), 125}, // 108 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5177, 9), 24}, // 109 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5219, 11), 26}, // 110 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5204, 15), 25}, // 111 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6369, 20), 79}, // 112 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6408, 24), 80}, // 113 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6317, 16), 77}, // 114 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5986, 15), 64}, // 115 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5659, 15), 53}, // 116 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5674, 18), 53}, // 117 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5714, 15), 54}, // 118 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5729, 18), 54}, // 119 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5747, 15), 55}, // 120 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5762, 18), 55}, // 121 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5521, 11), 44}, // 122 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5780, 15), 56}, // 123 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5795, 18), 56}, // 124 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5532, 26), 45}, // 125 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5813, 15), 57}, // 126 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5828, 18), 57}, // 127 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5471, 16), 41}, // 128 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5424, 13), 40}, // 129 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6532, 12), 84}, // 130 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5120, 10), 19}, // 131 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5105, 15), 18}, // 132 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5090, 15), 17}, // 133 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6050, 18), 67}, // 134 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6032, 18), 66}, // 135 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6001, 15), 65}, // 136 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4873, 9), 11}, // 137 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5558, 12), 46}, // 138 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5966, 10), 63}, // 139 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5042, 14), 16}, // 140 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6333, 15), 78}, // 141 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6470, 22), 82}, // 142 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5636, 10), 51}, // 143 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7746, 9), 124}, // 144 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5646, 13), 52}, // 145 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7704, 13), 122}, // 146 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5355, 8), 35}, // 147 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5285, 12), 31}, // 148 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5271, 14), 30}, // 149 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7030, 21), 103}, // 150 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7051, 20), 103}, // 151 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6995, 18), 102}, // 152 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7013, 17), 102}, // 153 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7271, 17), 110}, // 154 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7288, 16), 110}, // 155 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4718, 10), 137}, // 156 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12972, 34), 360}, // 157 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12660, 37), 348}, // 158 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12628, 32), 347}, // 159 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13529, 19), 385}, // 160 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12293, 9), 333}, // 161 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13446, 20), 381}, // 162 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12348, 23), 336}, // 163 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11798, 22), 310}, // 164 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13362, 20), 377}, // 165 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12302, 23), 334}, // 166 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13382, 20), 378}, // 167 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12325, 23), 335}, // 168 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8112, 14), 153}, // 169 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9499, 17), 229}, // 170 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13487, 24), 383}, // 171 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10121, 29), 263}, // 172 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10099, 22), 262}, // 173 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10083, 16), 261}, // 174 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14125, 20), 408}, // 175 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11735, 18), 307}, // 176 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13299, 16), 373}, // 177 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13071, 19), 364}, // 178 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14086, 19), 406}, // 179 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4847, 13), 163}, // 180 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11393, 32), 296}, // 181 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11425, 31), 296}, // 182 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10273, 31), 275}, // 183 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10304, 30), 275}, // 184 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11998, 30), 320}, // 185 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9868, 32), 248}, // 186 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11891, 31), 317}, // 187 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9147, 21), 371}, // 188 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9082, 28), 211}, // 189 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11481, 20), 298}, // 190 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11922, 40), 318}, // 191 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11861, 30), 316}, // 192 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11962, 36), 319}, // 193 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11778, 20), 309}, // 194 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12028, 28), 321}, // 195 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5594, 16), 203}, // 196 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4860, 13), 164}, // 197 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13466, 21), 382}, // 198 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11604, 25), 303}, // 199 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11629, 28), 303}, // 200 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9638, 18), 235}, // 201 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9623, 15), 234}, // 202 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8559, 18), 182}, // 203 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8083, 14), 151}, // 204 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5954, 12), 225}, // 205 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11657, 23), 304}, // 206 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13250, 11), 369}, // 207 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13144, 22), 367}, // 208 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13166, 25), 367}, // 209 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13191, 28), 368}, // 210 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13219, 31), 368}, // 211 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13103, 19), 366}, // 212 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13122, 22), 366}, // 213 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13261, 14), 369}, // 214 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5857, 15), 216}, // 215 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12405, 16), 339}, // 216 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13355, 7), 376}, // 217 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12608, 20), 346}, // 218 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13663, 28), 391}, // 219 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12948, 24), 359}, // 220 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12821, 27), 354}, // 221 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13548, 29), 386}, // 222 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12862, 20), 356}, // 223 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12908, 40), 358}, // 224 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12771, 26), 352}, // 225 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13577, 28), 387}, // 226 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13639, 24), 390}, // 227 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12727, 22), 350}, // 228 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12797, 24), 353}, // 229 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12582, 26), 345}, // 230 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13090, 13), 365}, // 231 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13623, 16), 389}, // 232 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7997, 8), 142}, // 233 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7983, 14), 141}, // 234 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9923, 16), 250}, // 235 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8005, 8), 143}, // 236 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9120, 27), 213}, // 237 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9110, 10), 212}, // 238 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13340, 15), 375}, // 239 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12201, 23), 329}, // 240 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6698, 23), 274}, // 241 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10251, 22), 274}, // 242 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6787, 19), 276}, // 243 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6508, 24), 270}, // 244 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9961, 16), 252}, // 245 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11572, 32), 302}, // 246 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11501, 33), 299}, // 247 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11534, 38), 300}, // 248 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5919, 23), 214}, // 249 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12556, 26), 344}, // 250 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12247, 22), 331}, // 251 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13986, 22), 402}, // 252 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8404, 15), 169}, // 253 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4905, 9), 135}, // 254 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8154, 18), 156}, // 255 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10179, 28), 265}, // 256 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8617, 16), 185}, // 257 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13721, 35), 393}, // 258 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13691, 30), 392}, // 259 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9073, 9), 210}, // 260 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5437, 16), 192}, // 261 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8756, 26), 194}, // 262 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5692, 22), 195}, // 263 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8836, 25), 198}, // 264 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10348, 29), 277}, // 265 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8861, 20), 199}, // 266 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13315, 25), 374}, // 267 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8782, 23), 196}, // 268 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8805, 31), 197}, // 269 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8736, 20), 193}, // 270 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14008, 26), 403}, // 271 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8076, 7), 150}, // 272 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13058, 13), 363}, // 273 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8473, 8), 175}, // 274 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8032, 11), 146}, // 275 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8495, 12), 178}, // 276 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8350, 15), 165}, // 277 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10234, 17), 272}, // 278 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9939, 22), 251}, // 279 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8172, 20), 157}, // 280 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8507, 13), 179}, // 281 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8058, 12), 148}, // 282 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8548, 11), 181}, // 283 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8043, 15), 147}, // 284 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9977, 21), 254}, // 285 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8382, 10), 167}, // 286 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12269, 24), 332}, // 287 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8424, 16), 171}, // 288 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10463, 36), 280}, // 289 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10499, 39), 280}, // 290 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11004, 39), 287}, // 291 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11043, 42), 287}, // 292 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8126, 6), 154}, // 293 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10056, 27), 260}, // 294 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10042, 14), 259}, // 295 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8013, 6), 144}, // 296 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8019, 13), 145}, // 297 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9998, 14), 255}, // 298 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8419, 5), 170}, // 299 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12224, 23), 330}, // 300 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8577, 22), 183}, // 301 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4672, 7), 139}, // 302 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12749, 22), 351}, // 303 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7966, 8), 138}, // 304 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8097, 15), 152}, // 305 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13402, 20), 379}, // 306 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12848, 14), 355}, // 307 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14061, 25), 405}, // 308 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7959, 7), 133}, // 309 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12882, 26), 357}, // 310 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4968, 15), 273}, // 311 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4954, 14), 271}, // 312 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8456, 7), 173}, // 313 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5976, 10), 226}, // 314 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5056, 14), 188}, // 315 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8711, 13), 190}, // 316 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13422, 11), 380}, // 317 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13433, 13), 380}, // 318 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6450, 20), 269}, // 319 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4790, 31), 295}, // 320 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11359, 34), 295}, // 321 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8724, 12), 191}, // 322 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8070, 6), 149}, // 323 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10027, 15), 258}, // 324 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11820, 18), 312}, // 325 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7944, 15), 370}, // 326 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9736, 12), 240}, // 327 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11753, 25), 308}, // 328 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9713, 23), 239}, // 329 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7891, 41), 322}, // 330 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7463, 33), 311}, // 331 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4940, 14), 243}, // 332 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7645, 39), 314}, // 333 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7349, 23), 292}, // 334 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4927, 13), 291}, // 335 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11680, 29), 305}, // 336 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7402, 27), 290}, // 337 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11456, 25), 297}, // 338 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7606, 28), 313}, // 339 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9767, 32), 242}, // 340 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14105, 20), 407}, // 341 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13275, 24), 372}, // 342 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12180, 21), 328}, // 343 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9681, 16), 237}, // 344 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9697, 16), 238}, // 345 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13029, 29), 362}, // 346 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10414, 23), 279}, // 347 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10437, 26), 279}, // 348 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10150, 29), 264}, // 349 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9516, 28), 230}, // 350 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5186, 18), 166}, // 351 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8463, 10), 174}, // 352 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8481, 14), 177}, // 353 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8365, 17), 176}, // 354 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8250, 33), 160}, // 355 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10777, 36), 284}, // 356 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10813, 39), 284}, // 357 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8392, 12), 168}, // 358 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4830, 7), 134}, // 359 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11838, 23), 315}, // 360 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10012, 15), 256}, // 361 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6299, 18), 257}, // 362 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11709, 26), 306}, // 363 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5002, 12), 200}, // 364 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10377, 17), 278}, // 365 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10394, 20), 278}, // 366 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7717, 19), 301}, // 367 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6389, 19), 268}, // 368 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5070, 20), 201}, // 369 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5014, 28), 266}, // 370 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10207, 27), 266}, // 371 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6348, 21), 267}, // 372 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10334, 14), 276}, // 373 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9656, 25), 236}, // 374 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8440, 16), 172}, // 375 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13965, 21), 401}, // 376 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13511, 18), 384}, // 377 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6253, 17), 253}, // 378 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9304, 25), 221}, // 379 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9544, 24), 231}, // 380 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8283, 34), 161}, // 381 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10852, 37), 285}, // 382 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10889, 40), 285}, // 383 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8317, 33), 162}, // 384 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10929, 36), 286}, // 385 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10965, 39), 286}, // 386 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8520, 28), 180}, // 387 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8192, 24), 158}, // 388 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8633, 30), 186}, // 389 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8663, 31), 187}, // 390 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9431, 21), 224}, // 391 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9409, 22), 223}, // 392 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9602, 21), 233}, // 393 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8996, 37), 208}, // 394 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9033, 40), 209}, // 395 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10619, 39), 282}, // 396 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10658, 42), 282}, // 397 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11172, 42), 289}, // 398 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11214, 45), 289}, // 399 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9392, 17), 222}, // 400 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9329, 28), 221}, // 401 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13784, 23), 395}, // 402 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13807, 30), 396}, // 403 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13837, 30), 397}, // 404 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12492, 39), 342}, // 405 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12421, 33), 340}, // 406 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12454, 38), 341}, // 407 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5453, 18), 215}, // 408 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12096, 27), 325}, // 409 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13756, 28), 394}, // 410 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8694, 17), 189}, // 411 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12123, 26), 326}, // 412 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12149, 31), 327}, // 413 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13867, 38), 398}, // 414 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12075, 21), 324}, // 415 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9288, 16), 220}, // 416 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13605, 18), 388}, // 417 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12056, 19), 323}, // 418 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14034, 27), 404}, // 419 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8985, 11), 207}, // 420 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13905, 28), 399}, // 421 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4914, 13), 136}, // 422 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8132, 22), 155}, // 423 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9900, 23), 249}, // 424 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9846, 22), 246}, // 425 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9825, 21), 245}, // 426 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9799, 26), 244}, // 427 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8899, 28), 204}, // 428 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8927, 28), 205}, // 429 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8955, 30), 206}, // 430 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6016, 16), 247}, // 431 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8599, 18), 184}, // 432 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13006, 23), 361}, // 433 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9357, 35), 222}, // 434 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9568, 34), 232}, // 435 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8216, 34), 159}, // 436 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10700, 37), 283}, // 437 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10737, 40), 283}, // 438 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8881, 18), 202}, // 439 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10538, 39), 281}, // 440 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10577, 42), 281}, // 441 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11085, 42), 288}, // 442 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11127, 45), 288}, // 443 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12697, 30), 349}, // 444 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9748, 19), 241}, // 445 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13933, 32), 400}, // 446 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12531, 25), 343}, // 447 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9482, 17), 228}, // 448 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9452, 30), 227}, // 449 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7974, 9), 140}, // 450 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12390, 15), 338}, // 451 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12371, 19), 337}, // 452 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11259, 18), 293}, // 453 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11298, 29), 294}, // 454 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11327, 32), 294}, // 455 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11277, 21), 293}, // 456 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9244, 44), 219}, // 457 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9201, 43), 218}, // 458 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9168, 33), 217}, // 459 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14157, 8), 410}, // 460 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14165, 8), 411}, // 461 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14173, 6), 412}, // 462 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14179, 7), 413}, // 463 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14186, 11), 414}, // 464 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14197, 9), 415}, // 465 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14206, 13), 416}, // 466 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14145, 12), 409}, // 467 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14219, 6), 417}, // 468 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14225, 10), 418}, // 469 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14235, 6), 419}, // 470 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14256, 20), 421}, // 471 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14241, 15), 420}, // 472 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14365, 15), 429}, // 473 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14380, 13), 430}, // 474 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14353, 12), 428}, // 475 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14443, 21), 434}, // 476 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14336, 17), 427}, // 477 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14520, 16), 438}, // 478 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14324, 12), 426}, // 479 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14504, 16), 437}, // 480 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14297, 14), 424}, // 481 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14281, 16), 423}, // 482 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14311, 13), 425}, // 483 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14464, 20), 435}, // 484 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14408, 18), 432}, // 485 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14393, 15), 431}, // 486 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14484, 20), 436}, // 487 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14426, 17), 433}, // 488 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14556, 24), 440}, // 489 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14536, 20), 439}, // 490 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14276, 5), 422}, // 491 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14641, 9), 445}, // 492 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14673, 7), 449}, // 493 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14580, 6), 441}, // 494 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14680, 9), 450}, // 495 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14591, 6), 443}, // 496 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14586, 5), 442}, // 497 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14630, 11), 444}, // 498 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14662, 11), 448}, // 499 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14650, 5), 446}, // 500 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14655, 7), 447}, // 501 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14725, 11), 454}, // 502 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14689, 10), 451}, // 503 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14712, 13), 453}, // 504 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14699, 13), 452}, // 505 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14736, 10), 455}, // 506 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14746, 10), 456}, // 507 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14756, 10), 457}, // 508 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14924, 12), 468}, // 509 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14936, 12), 469}, // 510 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14779, 14), 459}, // 511 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14793, 14), 460}, // 512 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14807, 14), 461}, // 513 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14766, 13), 458}, // 514 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14884, 19), 466}, // 515 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14836, 16), 463}, // 516 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14852, 16), 464}, // 517 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14868, 16), 465}, // 518 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14821, 15), 462}, // 519 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14903, 21), 467}, // 520 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15000, 28), 473}, // 521 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14960, 15), 471}, // 522 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14975, 25), 472}, // 523 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14948, 12), 470}, // 524 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(15036, 27), 475}, // 525 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15063, 27), 476}, // 526 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15090, 27), 477}, // 527 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15117, 32), 478}, // 528 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15028, 8), 474}, // 529 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15149, 26), 479}, // 530 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15186, 4), 482}, // 531 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15179, 7), 481}, // 532 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15175, 4), 480}, // 533 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15190, 11), 483}, // 534 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15212, 21), 485}, // 535 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15201, 11), 484}, // 536 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15244, 14), 487}, // 537 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15258, 21), 488}, // 538 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15233, 11), 486}, // 539 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15279, 18), 489}, // 540 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14157, 8), 491}, // 541 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14165, 8), 492}, // 542 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14173, 6), 493}, // 543 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14179, 7), 494}, // 544 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14186, 11), 495}, // 545 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14197, 9), 496}, // 546 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14206, 13), 497}, // 547 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14145, 12), 490}, // 548 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14219, 6), 498}, // 549 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14225, 10), 499}, // 550 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14235, 6), 500}, // 551 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14365, 15), 508}, // 552 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14380, 13), 509}, // 553 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14353, 12), 507}, // 554 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14443, 21), 513}, // 555 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14336, 17), 506}, // 556 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14324, 12), 505}, // 557 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14504, 16), 516}, // 558 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14297, 14), 503}, // 559 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14281, 16), 502}, // 560 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14311, 13), 504}, // 561 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14464, 20), 514}, // 562 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14408, 18), 511}, // 563 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14393, 15), 510}, // 564 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14484, 20), 515}, // 565 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14426, 17), 512}, // 566 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14276, 5), 501}, // 567 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14641, 9), 521}, // 568 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14673, 7), 525}, // 569 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14580, 6), 517}, // 570 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14591, 6), 519}, // 571 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14586, 5), 518}, // 572 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14630, 11), 520}, // 573 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14662, 11), 524}, // 574 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14650, 5), 522}, // 575 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14655, 7), 523}, // 576 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14689, 10), 526}, // 577 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14712, 13), 528}, // 578 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14699, 13), 527}, // 579 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17521, 16), 643}, // 580 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15506, 8), 548}, // 581 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(16624, 15), 601}, // 582 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16639, 18), 601}, // 583 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15984, 10), 572}, // 584 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(16028, 12), 574}, // 585 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(15357, 12), 535}, // 586 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17302, 14), 631}, // 587 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17141, 15), 624}, // 588 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15661, 8), 561}, // 589 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(16705, 16), 604}, // 590 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16687, 18), 603}, // 591 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15321, 6), 531}, // 592 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(16168, 22), 581}, // 593 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16146, 22), 580}, // 594 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16736, 13), 606}, // 595 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16721, 15), 605}, // 596 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15327, 12), 532}, // 597 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(17614, 20), 648}, // 598 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15438, 8), 540}, // 599 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(17384, 19), 636}, // 600 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15404, 8), 539}, // 601 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18424, 22), 671}, // 602 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18483, 23), 672}, // 603 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17403, 15), 637}, // 604 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15471, 9), 544}, // 605 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(16789, 13), 609}, // 606 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15532, 9), 551}, // 607 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(15348, 9), 534}, // 608 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(15645, 10), 559}, // 609 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(18371, 17), 670}, // 610 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17923, 27), 658}, // 611 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15523, 9), 550}, // 612 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(16935, 14), 616}, // 613 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15669, 14), 562}, // 614 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(17418, 28), 638}, // 615 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17194, 16), 627}, // 616 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16190, 18), 582}, // 617 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(15854, 15), 568}, // 618 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17842, 26), 655}, // 619 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15804, 15), 567}, // 620 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(15460, 5), 542}, // 621 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(17316, 20), 632}, // 622 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15756, 14), 566}, // 623 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(16846, 21), 612}, // 624 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17077, 24), 620}, // 625 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17687, 31), 650}, // 626 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17020, 26), 619}, // 627 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17477, 25), 641}, // 628 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15393, 11), 538}, // 629 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(15382, 11), 537}, // 630 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(16909, 26), 615}, // 631 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16657, 30), 602}, // 632 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16949, 24), 616}, // 633 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(16986, 19), 617}, // 634 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18266, 16), 667}, // 635 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17634, 19), 649}, // 636 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18343, 28), 669}, // 637 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15655, 6), 560}, // 638 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(18329, 14), 668}, // 639 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17550, 24), 645}, // 640 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15963, 21), 571}, // 641 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(15487, 10), 546}, // 642 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(17893, 30), 657}, // 643 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17868, 25), 656}, // 644 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15931, 18), 569}, // 645 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(15636, 9), 558}, // 646 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(17981, 33), 660}, // 647 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18014, 30), 661}, // 648 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18044, 28), 662}, // 649 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18140, 29), 664}, // 650 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18106, 34), 663}, // 651 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18169, 32), 665}, // 652 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17502, 19), 642}, // 653 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15369, 13), 536}, // 654 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(15994, 14), 573}, // 655 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(16040, 16), 575}, // 656 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(17574, 20), 646}, // 657 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17156, 22), 625}, // 658 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17210, 19), 628}, // 659 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17778, 18), 653}, // 660 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17115, 12), 622}, // 661 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17249, 11), 630}, // 662 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17537, 13), 644}, // 663 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15949, 14), 570}, // 664 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(15446, 14), 541}, // 665 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(16100, 13), 577}, // 666 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(16113, 15), 578}, // 667 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(16240, 20), 584}, // 668 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16208, 32), 583}, // 669 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15553, 12), 553}, // 670 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(16564, 11), 599}, // 671 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(16575, 14), 599}, // 672 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15541, 12), 552}, // 673 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(17127, 14), 623}, // 674 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15683, 7), 563}, // 675 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(16408, 19), 591}, // 676 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16427, 14), 592}, // 677 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15465, 6), 543}, // 678 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(16380, 28), 590}, // 679 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16355, 25), 589}, // 680 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16303, 25), 587}, // 681 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16283, 20), 586}, // 682 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16328, 27), 588}, // 683 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16488, 16), 595}, // 684 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16504, 15), 595}, // 685 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16529, 10), 597}, // 686 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16539, 13), 598}, // 687 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(16552, 12), 598}, // 688 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16519, 10), 596}, // 689 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17594, 20), 647}, // 690 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17446, 14), 639}, // 691 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16763, 26), 608}, // 692 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17101, 14), 621}, // 693 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17950, 31), 659}, // 694 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15297, 17), 529}, // 695 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(15497, 9), 547}, // 696 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(16589, 16), 600}, // 697 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16605, 19), 600}, // 698 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15339, 9), 533}, // 699 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(16749, 14), 607}, // 700 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15480, 7), 545}, // 701 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(15609, 20), 556}, // 702 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(16056, 44), 576}, // 703 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16460, 28), 594}, // 704 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16802, 17), 610}, // 705 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17229, 20), 629}, // 706 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17718, 25), 651}, // 707 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17178, 16), 626}, // 708 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15314, 7), 530}, // 709 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(18201, 26), 666}, // 710 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16894, 15), 614}, // 711 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17460, 17), 640}, // 712 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17796, 15), 654}, // 713 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15629, 7), 557}, // 714 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(17336, 16), 633}, // 715 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16260, 23), 585}, // 716 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17366, 18), 635}, // 717 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15565, 8), 554}, // 718 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(15599, 10), 555}, // 719 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(16973, 13), 617}, // 720 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(17005, 15), 618}, // 721 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(17743, 35), 652}, // 722 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16867, 27), 613}, // 723 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16819, 27), 611}, // 724 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16441, 19), 593}, // 725 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15514, 9), 549}, // 726 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(16128, 18), 579}, // 727 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17352, 14), 634}, // 728 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15690, 10), 564}, // 729 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15700, 10), 565}, // 730 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18506, 3), 673}, // 731 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18509, 3), 674}, // 732 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18512, 3), 675}, // 733 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18525, 7), 678}, // 734 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18515, 5), 676}, // 735 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18520, 5), 677}, // 736 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18532, 12), 679}, // 737 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18544, 17), 680}, // 738 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(19356, 15), 733}, // 739 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18987, 15), 711}, // 740 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9638, 18), 725}, // 741 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9623, 15), 724}, // 742 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18757, 13), 694}, // 743 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18770, 10), 695}, // 744 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18742, 15), 693}, // 745 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18780, 15), 696}, // 746 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19777, 25), 751}, // 747 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19802, 24), 751}, // 748 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19730, 24), 750}, // 749 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19754, 23), 750}, // 750 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19304, 29), 731}, // 751 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18709, 19), 690}, // 752 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20287, 18), 771}, // 753 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19014, 10), 713}, // 754 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20102, 26), 763}, // 755 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20128, 27), 764}, // 756 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19002, 12), 712}, // 757 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18831, 11), 700}, // 758 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18842, 20), 701}, // 759 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18819, 12), 699}, // 760 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18872, 24), 703}, // 761 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18561, 12), 681}, // 762 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19371, 15), 734}, // 763 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18902, 9), 705}, // 764 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18795, 10), 697}, // 765 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18805, 14), 698}, // 766 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19082, 16), 718}, // 767 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19070, 12), 717}, // 768 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19386, 21), 735}, // 769 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19431, 21), 738}, // 770 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20177, 16), 766}, // 771 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20155, 22), 765}, // 772 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20263, 24), 770}, // 773 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20378, 22), 775}, // 774 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20400, 24), 776}, // 775 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20355, 23), 774}, // 776 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20475, 27), 777}, // 777 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20193, 20), 767}, // 778 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19098, 34), 719}, // 779 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19132, 34), 720}, // 780 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19166, 36), 721}, // 781 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19250, 34), 729}, // 782 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20213, 22), 768}, // 783 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18693, 16), 689}, // 784 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18677, 16), 688}, // 785 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18939, 16), 708}, // 786 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19662, 15), 748}, // 787 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19677, 14), 748}, // 788 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18926, 13), 707}, // 789 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19691, 20), 749}, // 790 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19711, 19), 749}, // 791 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18955, 20), 709}, // 792 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19826, 19), 752}, // 793 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19845, 18), 752}, // 794 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18911, 15), 706}, // 795 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18658, 19), 687}, // 796 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19863, 25), 753}, // 797 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19888, 27), 754}, // 798 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18728, 10), 691}, // 799 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19232, 18), 723}, // 800 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19603, 19), 745}, // 801 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18896, 6), 704}, // 802 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20329, 26), 773}, // 803 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19622, 20), 746}, // 804 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9681, 16), 727}, // 805 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20081, 21), 762}, // 806 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20060, 21), 761}, // 807 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9697, 16), 728}, // 808 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19915, 26), 755}, // 809 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19941, 28), 756}, // 810 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20235, 28), 769}, // 811 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11838, 23), 759}, // 812 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6317, 16), 737}, // 813 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19969, 31), 757}, // 814 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20000, 33), 758}, // 815 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20033, 27), 760}, // 816 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19642, 20), 747}, // 817 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9656, 25), 726}, // 818 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18573, 13), 682}, // 819 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18586, 22), 683}, // 820 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18608, 21), 684}, // 821 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19407, 24), 736}, // 822 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19556, 25), 743}, // 823 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19480, 26), 740}, // 824 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19581, 22), 744}, // 825 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19506, 23), 741}, // 826 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19333, 23), 732}, // 827 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19529, 27), 742}, // 828 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19452, 28), 739}, // 829 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20305, 24), 772}, // 830 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(5424, 13), 714}, // 831 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19202, 30), 722}, // 832 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19024, 22), 715}, // 833 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19046, 24), 716}, // 834 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19284, 20), 730}, // 835 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18862, 10), 702}, // 836 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18975, 12), 710}, // 837 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18643, 15), 686}, // 838 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18629, 14), 685}, // 839 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18738, 4), 692}, // 840 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20640, 10), 789}, // 841 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20650, 9), 789}, // 842 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20701, 12), 792}, // 843 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20713, 11), 792}, // 844 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20659, 14), 790}, // 845 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20673, 13), 790}, // 846 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(14680, 9), 782}, // 847 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20552, 10), 783}, // 848 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4905, 9), 781}, // 849 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20609, 16), 788}, // 850 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20625, 15), 788}, // 851 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4672, 7), 784}, // 852 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20732, 8), 794}, // 853 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20569, 7), 786}, // 854 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20686, 8), 791}, // 855 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20694, 7), 791}, // 856 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20576, 17), 787}, // 857 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20593, 16), 787}, // 858 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20724, 8), 793}, // 859 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20562, 7), 785}, // 860 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20509, 20), 779}, // 861 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20529, 23), 780}, // 862 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20502, 7), 778}, // 863 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20749, 12), 796}, // 864 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20740, 9), 795}, // 865 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20761, 12), 797}, // 866 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(20773, 14), 798}, // 867 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20787, 14), 799}, // 868 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20806, 4), 801}, // 869 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20801, 5), 800}, // 870 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20844, 14), 808}, // 871 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20858, 23), 808}, // 872 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20881, 13), 809}, // 873 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20894, 18), 809}, // 874 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20828, 11), 806}, // 875 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20912, 15), 810}, // 876 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20839, 5), 807}, // 877 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20824, 4), 805}, // 878 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(14276, 5), 802}, // 879 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20817, 7), 804}, // 880 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20810, 7), 803}, // 881 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20927, 4), 811}, // 882 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20939, 4), 814}, // 883 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20935, 4), 813}, // 884 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20931, 4), 812}, // 885 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20975, 18), 817}, // 886 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20993, 18), 818}, // 887 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20943, 16), 815}, // 888 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20959, 16), 816}, // 889 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(21034, 6), 823}, // 890 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(21018, 11), 821}, // 891 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(21011, 7), 820}, // 892 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14276, 5), 819}, // 893 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13422, 11), 824}, // 894 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13433, 13), 824}, // 895 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(21029, 5), 822}, // 896 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(21050, 6), 827}, // 897 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21061, 8), 829}, // 898 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21069, 10), 830}, // 899 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21087, 12), 832}, // 900 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21079, 8), 831}, // 901 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21099, 20), 833}, // 902 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21045, 5), 826}, // 903 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21056, 5), 828}, // 904 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21040, 5), 825}, // 905 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21154, 16), 837}, // 906 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21140, 14), 836}, // 907 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21126, 14), 835}, // 908 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21217, 27), 840}, // 909 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21190, 27), 839}, // 910 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21170, 20), 838}, // 911 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21119, 7), 834}, // 912 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21244, 10), 841}, // 913 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21254, 10), 842}, // 914 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21275, 15), 844}, // 915 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21264, 11), 843}, // 916 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21290, 5), 846}, // 917 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(21304, 12), 849}, // 918 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(21316, 13), 851}, // 919 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(21299, 5), 848}, // 920 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(21295, 4), 847}, // 921 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(21329, 19), 854}, // 922 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(21348, 22), 854}, // 923 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21370, 17), 855}, // 924 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(21387, 20), 855}, // 925 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(8456, 7), 853}, // 926 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(21407, 16), 856}, // 927 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21423, 19), 856}, // 928 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(14276, 5), 845}, // 929 None in SPV_OPERAND_TYPE_IMAGE
  {IR(21495, 12), 860}, // 930 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(15683, 7), 850}, // 931 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(21507, 8), 861}, // 932 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(15480, 7), 852}, // 933 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(21473, 11), 858}, // 934 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(21442, 14), 857}, // 935 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21456, 17), 857}, // 936 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21484, 11), 859}, // 937 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(14173, 6), 876}, // 938 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21676, 10), 875}, // 939 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21611, 12), 870}, // 940 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21623, 12), 871}, // 941 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21600, 11), 869}, // 942 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21525, 11), 863}, // 943 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21515, 10), 862}, // 944 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21585, 15), 868}, // 945 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21697, 17), 878}, // 946 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21714, 16), 879}, // 947 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21847, 16), 886}, // 948 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21863, 16), 887}, // 949 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21546, 11), 865}, // 950 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21686, 11), 877}, // 951 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21770, 20), 882}, // 952 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21536, 10), 864}, // 953 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21571, 14), 867}, // 954 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21557, 14), 866}, // 955 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21790, 19), 883}, // 956 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21809, 19), 884}, // 957 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21828, 19), 885}, // 958 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21648, 14), 873}, // 959 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21662, 14), 874}, // 960 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21635, 13), 872}, // 961 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21730, 20), 880}, // 962 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21750, 20), 881}, // 963 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21881, 2), 889}, // 964 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21982, 5), 907}, // 965 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21903, 5), 895}, // 966 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21898, 5), 894}, // 967 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21940, 6), 901}, // 968 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21946, 13), 902}, // 969 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21908, 10), 896}, // 970 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21918, 10), 897}, // 971 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21879, 2), 888}, // 972 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21886, 3), 891}, // 973 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21883, 3), 890}, // 974 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21889, 4), 892}, // 975 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21893, 5), 893}, // 976 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21935, 5), 900}, // 977 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21931, 4), 899}, // 978 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21928, 3), 898}, // 979 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21976, 6), 906}, // 980 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21959, 5), 903}, // 981 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21970, 6), 905}, // 982 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21964, 6), 904}, // 983 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21987, 27), 908}, // 984 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(22014, 23), 909}, // 985 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(22037, 7), 910}, // 986 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(22044, 11), 911}, // 987 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(22055, 14), 912}, // 988 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(22069, 12), 914}, // 989 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(14276, 5), 913}, // 990 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(22081, 13), 916}, // 991 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(14276, 5), 915}, // 992 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(22094, 7), 917}, // 993 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(22101, 7), 918}, // 994 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(22108, 12), 919}, // 995 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(22134, 12), 921}, // 996 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22186, 17), 924}, // 997 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22161, 25), 923}, // 998 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22146, 15), 922}, // 999 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22120, 14), 920}, // 1000 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22326, 21), 937}, // 1001 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22221, 19), 928}, // 1002 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22240, 17), 929}, // 1003 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22210, 11), 927}, // 1004 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17550, 24), 935}, // 1005 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22285, 18), 932}, // 1006 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22347, 18), 939}, // 1007 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22426, 15), 943}, // 1008 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17574, 20), 936}, // 1009 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22365, 21), 940}, // 1010 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22271, 14), 931}, // 1011 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22441, 26), 944}, // 1012 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22257, 14), 930}, // 1013 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22412, 14), 942}, // 1014 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(14276, 5), 925}, // 1015 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22313, 13), 934}, // 1016 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22303, 10), 933}, // 1017 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17594, 20), 938}, // 1018 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22386, 26), 941}, // 1019 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22203, 7), 926}, // 1020 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22750, 27), 958}, // 1021 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22698, 26), 956}, // 1022 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22618, 23), 952}, // 1023 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22572, 23), 950}, // 1024 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22467, 29), 946}, // 1025 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22664, 17), 954}, // 1026 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22777, 27), 959}, // 1027 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22724, 26), 957}, // 1028 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22641, 23), 953}, // 1029 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22595, 23), 951}, // 1030 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22496, 29), 947}, // 1031 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22681, 17), 955}, // 1032 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22525, 21), 948}, // 1033 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22546, 26), 949}, // 1034 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(14276, 5), 945}, // 1035 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22937, 20), 967}, // 1036 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22804, 8), 962}, // 1037 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22812, 21), 964}, // 1038 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22833, 24), 964}, // 1039 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22857, 19), 965}, // 1040 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22876, 22), 965}, // 1041 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22957, 17), 968}, // 1042 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22898, 18), 966}, // 1043 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22916, 21), 966}, // 1044 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(14276, 5), 960}, // 1045 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(21495, 12), 963}, // 1046 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15514, 9), 961}, // 1047 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22981, 8), 970}, // 1048 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22989, 7), 971}, // 1049 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22974, 7), 969}, // 1050 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22996, 7), 972}, // 1051 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(23003, 10), 972}, // 1052 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(23021, 8), 974}, // 1053 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23037, 15), 976}, // 1054 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23141, 20), 982}, // 1055 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23120, 21), 981}, // 1056 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23161, 12), 983}, // 1057 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23202, 14), 985}, // 1058 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23216, 17), 985}, // 1059 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23233, 12), 986}, // 1060 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23245, 15), 986}, // 1061 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(14276, 5), 973}, // 1062 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23173, 13), 984}, // 1063 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23186, 16), 984}, // 1064 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23013, 8), 973}, // 1065 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23029, 8), 975}, // 1066 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23052, 23), 977}, // 1067 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23089, 15), 979}, // 1068 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23075, 14), 978}, // 1069 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15514, 9), 987}, // 1070 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23104, 16), 980}, // 1071 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23260, 10), 988}, // 1072 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(23275, 4), 990}, // 1073 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23288, 8), 992}, // 1074 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23279, 9), 991}, // 1075 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23270, 5), 989}, // 1076 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23296, 25), 993}, // 1077 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23321, 28), 993}, // 1078 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23362, 4), 996}, // 1079 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23395, 9), 1000}, // 1080 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23404, 13), 1001}, // 1081 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23375, 8), 998}, // 1082 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23383, 12), 999}, // 1083 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23366, 9), 997}, // 1084 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23349, 4), 994}, // 1085 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23353, 9), 995}, // 1086 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(14276, 5), 1002}, // 1087 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23417, 25), 1003}, // 1088 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23442, 23), 1004}, // 1089 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23534, 27), 1010}, // 1090 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23561, 28), 1011}, // 1091 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23603, 16), 1013}, // 1092 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23589, 14), 1012}, // 1093 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23673, 30), 1016}, // 1094 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23475, 12), 1007}, // 1095 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(15028, 8), 1005}, // 1096 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23465, 10), 1006}, // 1097 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23660, 13), 1015}, // 1098 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23636, 24), 1014}, // 1099 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23510, 24), 1009}, // 1100 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23619, 17), 1014}, // 1101 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23487, 23), 1008}, // 1102 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23744, 37), 1018}, // 1103 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23703, 41), 1017}, // 1104 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23859, 42), 1021}, // 1105 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23781, 37), 1019}, // 1106 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23818, 41), 1020}, // 1107 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23901, 33), 1022}, // 1108 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23934, 33), 1023}, // 1109 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23979, 6), 1026}, // 1110 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23967, 12), 1025}, // 1111 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(14276, 5), 1024}, // 1112 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23985, 7), 1027}, // 1113 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23992, 15), 1028}, // 1114 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(24015, 7), 1030}, // 1115 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(24007, 8), 1029}, // 1116 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(24080, 13), 1039}, // 1117 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24122, 4), 1045}, // 1118 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24160, 9), 1050}, // 1119 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24093, 5), 1040}, // 1120 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24222, 5), 1059}, // 1121 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24293, 6), 1069}, // 1122 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24046, 5), 1034}, // 1123 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24200, 5), 1055}, // 1124 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24257, 6), 1064}, // 1125 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24310, 5), 1072}, // 1126 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24304, 6), 1071}, // 1127 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24126, 3), 1046}, // 1128 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24169, 8), 1051}, // 1129 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24227, 4), 1060}, // 1130 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24299, 5), 1070}, // 1131 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24113, 5), 1043}, // 1132 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24141, 10), 1048}, // 1133 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24074, 6), 1038}, // 1134 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24211, 6), 1057}, // 1135 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24280, 7), 1067}, // 1136 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24068, 6), 1037}, // 1137 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24205, 6), 1056}, // 1138 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24273, 7), 1066}, // 1139 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24118, 4), 1044}, // 1140 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24151, 9), 1049}, // 1141 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24217, 5), 1058}, // 1142 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24287, 6), 1068}, // 1143 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24105, 8), 1042}, // 1144 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24263, 10), 1065}, // 1145 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24098, 7), 1041}, // 1146 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24129, 12), 1047}, // 1147 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24038, 8), 1033}, // 1148 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24185, 8), 1053}, // 1149 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24240, 9), 1062}, // 1150 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24030, 8), 1032}, // 1151 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24177, 8), 1052}, // 1152 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24231, 9), 1061}, // 1153 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24051, 6), 1035}, // 1154 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24057, 11), 1036}, // 1155 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24193, 7), 1054}, // 1156 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24249, 8), 1063}, // 1157 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24022, 8), 1031}, // 1158 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24315, 12), 1073}, // 1159 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24327, 7), 1074}, // 1160 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24353, 11), 1077}, // 1161 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24364, 12), 1078}, // 1162 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24376, 15), 1078}, // 1163 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24391, 14), 1079}, // 1164 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24344, 9), 1076}, // 1165 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24334, 10), 1075}, // 1166 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24413, 12), 1082}, // 1167 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24405, 8), 1081}, // 1168 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(14276, 5), 1080}, // 1169 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24425, 29), 1083}, // 1170 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(14157, 8), 1085}, // 1171 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14165, 8), 1086}, // 1172 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14173, 6), 1087}, // 1173 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14179, 7), 1088}, // 1174 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14186, 11), 1089}, // 1175 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14197, 9), 1090}, // 1176 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14206, 13), 1091}, // 1177 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14145, 12), 1084}, // 1178 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14219, 6), 1092}, // 1179 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14225, 10), 1093}, // 1180 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14235, 6), 1094}, // 1181 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14256, 20), 1096}, // 1182 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14241, 15), 1095}, // 1183 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14365, 15), 1104}, // 1184 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14380, 13), 1105}, // 1185 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14353, 12), 1103}, // 1186 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14443, 21), 1109}, // 1187 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14336, 17), 1102}, // 1188 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14520, 16), 1113}, // 1189 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14324, 12), 1101}, // 1190 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14504, 16), 1112}, // 1191 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14297, 14), 1099}, // 1192 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14281, 16), 1098}, // 1193 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14311, 13), 1100}, // 1194 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14464, 20), 1110}, // 1195 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14408, 18), 1107}, // 1196 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14393, 15), 1106}, // 1197 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14484, 20), 1111}, // 1198 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14426, 17), 1108}, // 1199 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14556, 24), 1115}, // 1200 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14536, 20), 1114}, // 1201 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(24454, 26), 1116}, // 1202 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14276, 5), 1097}, // 1203 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14641, 9), 1121}, // 1204 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14673, 7), 1125}, // 1205 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14580, 6), 1117}, // 1206 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14680, 9), 1126}, // 1207 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14591, 6), 1119}, // 1208 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14586, 5), 1118}, // 1209 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14630, 11), 1120}, // 1210 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14662, 11), 1124}, // 1211 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14650, 5), 1122}, // 1212 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14655, 7), 1123}, // 1213 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14725, 11), 1130}, // 1214 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14689, 10), 1127}, // 1215 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14712, 13), 1129}, // 1216 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14699, 13), 1128}, // 1217 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(24515, 15), 1137}, // 1218 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24480, 5), 1132}, // 1219 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24485, 5), 1133}, // 1220 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24535, 7), 1139}, // 1221 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24510, 5), 1136}, // 1222 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24542, 5), 1140}, // 1223 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24490, 9), 1134}, // 1224 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24499, 11), 1135}, // 1225 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24562, 5), 1144}, // 1226 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24530, 5), 1138}, // 1227 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24552, 6), 1142}, // 1228 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24022, 8), 1131}, // 1229 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24547, 5), 1141}, // 1230 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24558, 4), 1143}, // 1231 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24649, 14), 1155}, // 1232 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24731, 16), 1161}, // 1233 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24747, 15), 1161}, // 1234 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25045, 17), 1170}, // 1235 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24596, 15), 1150}, // 1236 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25062, 16), 1171}, // 1237 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24619, 9), 1152}, // 1238 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24628, 8), 1153}, // 1239 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24836, 16), 1164}, // 1240 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24852, 15), 1164}, // 1241 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25000, 21), 1168}, // 1242 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25078, 14), 1172}, // 1243 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24663, 6), 1156}, // 1244 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24762, 24), 1162}, // 1245 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24786, 23), 1162}, // 1246 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24867, 22), 1165}, // 1247 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24889, 21), 1165}, // 1248 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24583, 6), 1146}, // 1249 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24715, 16), 1160}, // 1250 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24589, 7), 1148}, // 1251 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24953, 22), 1167}, // 1252 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24975, 25), 1167}, // 1253 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24611, 8), 1151}, // 1254 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24636, 13), 1154}, // 1255 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24809, 14), 1163}, // 1256 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24823, 13), 1163}, // 1257 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24910, 22), 1166}, // 1258 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24932, 21), 1166}, // 1259 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24669, 14), 1157}, // 1260 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25021, 24), 1169}, // 1261 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24696, 19), 1159}, // 1262 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24683, 13), 1158}, // 1263 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(15565, 8), 1147}, // 1264 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24567, 16), 1145}, // 1265 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24334, 10), 1149}, // 1266 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(22146, 15), 1176}, // 1267 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(22120, 14), 1173}, // 1268 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25110, 15), 1175}, // 1269 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25092, 18), 1174}, // 1270 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25136, 11), 1179}, // 1271 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(14276, 5), 1177}, // 1272 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(25125, 11), 1178}, // 1273 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(23967, 12), 1182}, // 1274 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(15523, 9), 1181}, // 1275 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23985, 7), 1183}, // 1276 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23992, 15), 1184}, // 1277 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(25147, 10), 1180}, // 1278 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(25200, 24), 1188}, // 1279 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25224, 22), 1189}, // 1280 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25246, 21), 1190}, // 1281 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25157, 8), 1185}, // 1282 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25165, 15), 1186}, // 1283 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25180, 20), 1187}, // 1284 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 148);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(156, 304);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(460, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(468, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(471, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(473, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(492, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(502, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(506, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(521, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(525, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(531, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(534, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(537, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(541, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(549, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(552, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(568, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(577, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(580, 151);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(731, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(739, 102);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(841, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(864, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(866, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(869, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(871, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(882, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(886, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(890, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(897, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(906, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(913, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(917, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(938, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(964, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(984, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(986, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(989, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(991, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(993, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(996, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1001, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1021, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1036, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1048, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1053, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1072, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1073, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1077, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1079, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1087, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1090, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1103, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1105, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1108, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1110, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1115, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1117, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1159, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1167, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1170, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1171, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1179, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1182, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1184, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1204, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1214, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1218, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1232, 35);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1267, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1271, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1274, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1279, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(917, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1036, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1077, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(525, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1021, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1087, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(866, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1279, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(156, 304);
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
static const std::array<InstructionDesc, 822> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(25267, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(25323, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(25329, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(25448, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(25455, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(25471, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(25476, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(25487, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(25494, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(25499, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(25509, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(25569, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(25641, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(25715, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(25758, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(25800, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(25811, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(25820, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(25829, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(25874, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(25884, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(25895, 11), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(26019, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(26029, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(26041, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(26058, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(26068, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(26085, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(26096, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(26138, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(26150, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(26163, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(26173, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(26189, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(26203, 10), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(26213, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(26222, 19), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(26241, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(26254, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(15523, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(26306, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(26402, 16), IR(0, 0), IR(206, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(26418, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(26431, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(26448, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(26466, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(26479, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(26542, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(24619, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(26591, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(26609, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(26621, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(26634, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(26643, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(26701, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(26706, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(26712, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(26723, 16), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(26739, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(26751, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(26771, 15), IR(0, 0), IR(209, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(26786, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(26798, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(26821, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(26872, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(26881, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(26896, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(26912, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(26971, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(26991, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(27012, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(27032, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(27046, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(27065, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(27082, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(27098, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(27109, 10), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(27119, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(27164, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(27210, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(27233, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(27260, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(27287, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(27314, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(27341, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(27372, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(27403, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(27414, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(27426, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(27442, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(27452, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(24663, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(27463, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(27480, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(27496, 18), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(27514, 15), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(27529, 14), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(27543, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(27560, 18), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(27578, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(27590, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(27602, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(27614, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(27626, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(27635, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(27644, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(27653, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(27667, 14), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(27681, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(27696, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(27711, 14), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(27725, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(27742, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(27759, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(27784, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(27792, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(27800, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(27808, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(27813, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(27818, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(27823, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(27828, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(27833, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(27838, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(27843, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(27848, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(27853, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(27858, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(27863, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(27868, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(27873, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(27878, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(27896, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(27914, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(27932, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(27950, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(27968, 13), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(27981, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(27985, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(27995, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(28006, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(28019, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(28032, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(28036, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(28040, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(28046, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(28052, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(28061, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(28070, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(28081, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(28095, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(28103, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(28113, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(28126, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(28142, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(28152, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(28163, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(28174, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(28181, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(28188, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(28198, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(28211, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(28224, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(28242, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(28260, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(28270, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(28280, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(28295, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(28310, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(28320, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(28332, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(28345, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(28360, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(28373, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(28388, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(28404, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(28422, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(28440, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(28460, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(28481, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(28504, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(28522, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(28543, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(28560, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(28570, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(28581, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(28592, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(28596, 15), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(28611, 17), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(28628, 17), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(28645, 11), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(28656, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(28665, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(28670, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(28675, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(28682, 9), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(28691, 9), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(28700, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(28711, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(28722, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(28733, 13), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(28746, 11), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(28757, 13), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(28770, 17), IR(0, 0), IR(116, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(28787, 19), IR(0, 0), IR(116, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(28843, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(28858, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(28872, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(28883, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(28895, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(28910, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(28932, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(28958, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(28975, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(28992, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(29003, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(29014, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(29025, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(29036, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(29047, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(29058, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(29068, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(29077, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(29087, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(29121, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(29166, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(29181, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(29187, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(29194, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(29257, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(29264, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(29269, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(29276, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(29288, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(29300, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(29314, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(29327, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(29342, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(29358, 9), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(29367, 9), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(29376, 15), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(29424, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(29434, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(29444, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(29454, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(29464, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(29474, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(29484, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(29494, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(29504, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(29513, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(29523, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(29540, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(29558, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(29581, 24), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(29605, 15), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(29620, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(29636, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(29653, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(29671, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(29689, 28), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(29717, 29), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(29746, 20), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(29766, 21), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(29787, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(29801, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(29815, 30), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(29845, 32), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(29877, 23), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(29900, 40), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(29940, 12), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(29952, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(29965, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(29981, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(29994, 19), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(30013, 26), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(30039, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(30055, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(30068, 29), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(30097, 29), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(30126, 33), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(30159, 33), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(30192, 33), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(30225, 33), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(30258, 37), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(30295, 37), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(30332, 17), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(30349, 18), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(30367, 22), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(30389, 26), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(30415, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(30422, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(30443, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(30459, 16), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(30475, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(30482, 16), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(30498, 20), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(30518, 26), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(30544, 35), IR(0, 0), IR(155, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(30579, 25), IR(0, 0), IR(155, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(30604, 17), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(30621, 23), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(30644, 19), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(30663, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(30679, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(30695, 11), IR(0, 0), IR(1, 0), IR(159, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(30706, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(30727, 19), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(30746, 19), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(30765, 24), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(30789, 25), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(30814, 30), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(5692, 22), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(30844, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(30873, 32), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(30905, 30), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(30935, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(30964, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(8782, 23), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(30993, 26), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(31019, 25), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(31044, 27), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(31071, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(31091, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(31111, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(31131, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(31151, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(31171, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(31191, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(31211, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(31231, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(31251, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(31271, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(31297, 25), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(31322, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(31348, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(31374, 25), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(31399, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(31425, 29), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(31454, 24), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(31478, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(31490, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(31499, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(31511, 8), IR(0, 0), IR(231, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(31519, 23), IR(0, 0), IR(147, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(31542, 23), IR(0, 0), IR(156, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(31565, 25), IR(0, 0), IR(157, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(31590, 14), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(31646, 14), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(31660, 15), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(31675, 19), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(31694, 17), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(31711, 19), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(9073, 9), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(31730, 14), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(31744, 18), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(31762, 12), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(31774, 13), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(31787, 20), IR(0, 0), IR(3, 1), IR(169, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(31807, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(31829, 19), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(31848, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(31870, 30), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(5453, 18), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(31900, 27), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(31927, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(31952, 33), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(31985, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(32007, 19), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(32026, 7), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(32033, 15), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(32048, 15), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(32063, 20), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(13315, 25), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(32083, 26), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(32109, 26), IR(0, 0), IR(1, 0), IR(170, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(32135, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(32160, 12), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(32172, 19), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(32191, 35), IR(0, 0), IR(240, 2), IR(171, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(32226, 22), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(32248, 16), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(32311, 5), IR(93, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(32324, 5), IR(94, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(32337, 6), IR(95, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(32352, 11), IR(96, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(32377, 11), IR(97, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(32402, 12), IR(98, 1), IR(242, 1), IR(123, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(32429, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(32454, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(32479, 26), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(32559, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(32586, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32613, 30), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32643, 34), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(32677, 31), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(32708, 16), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(32724, 22), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(32746, 21), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(32767, 32), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(32799, 31), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(32830, 19), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(32849, 31), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(32880, 24), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(32904, 19), IR(0, 0), IR(245, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(32923, 23), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(32946, 23), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(32969, 17), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(32986, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(33015, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(33044, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(33073, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(33102, 30), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(33132, 28), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(33160, 20), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(33180, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(33203, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(33226, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(33249, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(33272, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(33295, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(33318, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(33341, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(33364, 21), IR(0, 0), IR(249, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(33385, 17), IR(0, 0), IR(249, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(33402, 13), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(33415, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(33440, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(33464, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(33489, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(33518, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(33545, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(33568, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(33587, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(33610, 26), IR(0, 0), IR(164, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(33636, 26), IR(0, 0), IR(164, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(479, 14), IR(33662, 27), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(493, 13), IR(33689, 36), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(506, 7), IR(33725, 28), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(33753, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(33781, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(33809, 33), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(33842, 30), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(493, 13), IR(33872, 26), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(33898, 39), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(33937, 44), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(33981, 23), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(513, 12), IR(34004, 20), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(493, 13), IR(34024, 21), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(513, 12), IR(34045, 30), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(525, 6), IR(34075, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(34097, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(34122, 26), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(34148, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(34173, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(34195, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(34224, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(34252, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(34277, 34), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(34311, 32), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(34343, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(34372, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(34394, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(34416, 19), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(34435, 17), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(34452, 18), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(531, 3), IR(34470, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(34499, 24), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(34523, 16), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(534, 7), IR(34539, 23), IR(0, 0), IR(253, 1), IR(72, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeCooperativeVectorNV, true, false, IR(89, 3), IR(34562, 24), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(541, 13), IR(34586, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(554, 7), IR(34615, 42), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(34657, 39), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(561, 16), IR(34696, 32), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(34728, 27), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(577, 4), IR(34755, 17), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(34772, 18), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionNV, true, true, IR(160, 3), IR(34790, 27), IR(0, 0), IR(184, 1), IR(75, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(34817, 33), IR(0, 0), IR(50, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(34850, 35), IR(0, 0), IR(257, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(34885, 38), IR(0, 0), IR(257, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(581, 5), IR(34923, 24), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(586, 4), IR(34947, 25), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(34972, 22), IR(99, 1), IR(54, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(35015, 21), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(35036, 15), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(35051, 8), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(513, 12), IR(35059, 14), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(513, 12), IR(35073, 17), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(35090, 50), IR(0, 0), IR(258, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(35140, 29), IR(100, 1), IR(259, 4), IR(173, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(35197, 18), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(35215, 35), IR(101, 1), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(35273, 24), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(590, 5), IR(35297, 24), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(595, 6), IR(35321, 24), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(601, 5), IR(35345, 25), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(35370, 26), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(35396, 26), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(35422, 28), IR(0, 0), IR(264, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(35450, 26), IR(0, 0), IR(264, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(606, 5), IR(35519, 26), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(611, 7), IR(35620, 30), IR(0, 0), IR(201, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(618, 5), IR(35650, 31), IR(0, 0), IR(201, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(35681, 32), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(35713, 19), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(623, 4), IR(35732, 17), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(35749, 21), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(35770, 27), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(35797, 24), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(35821, 20), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(35841, 28), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(35869, 19), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(35888, 25), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(35913, 22), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(11604, 25), IR(45, 1), IR(270, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(35935, 22), IR(0, 0), IR(270, 1), IR(81, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(35957, 20), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(35977, 27), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(36004, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(36033, 18), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(36051, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(36071, 18), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(36089, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(36109, 25), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(36134, 25), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(36159, 29), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(627, 7), IR(36240, 17), IR(0, 0), IR(190, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(36257, 40), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(36297, 38), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(36335, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(36373, 34), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(36407, 37), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(36444, 29), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(36473, 27), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(36500, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(36527, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(36550, 23), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(36573, 20), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(36593, 22), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(36615, 19), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(12075, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(36634, 25), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(36659, 23), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(36682, 24), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(36706, 23), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(36729, 24), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(36753, 28), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(36781, 29), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(36810, 33), IR(0, 0), IR(274, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(634, 5), IR(36843, 34), IR(0, 0), IR(274, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(36877, 24), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(36901, 25), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(36926, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(36939, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(36952, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(36965, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(36978, 14), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(36992, 14), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(37006, 21), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(37027, 21), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(37048, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(37061, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(37074, 15), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(37089, 15), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(37104, 29), IR(0, 0), IR(199, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(37133, 25), IR(0, 0), IR(199, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(37158, 15), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(639, 6), IR(12293, 9), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(37173, 13), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(37186, 14), IR(0, 0), IR(276, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(37200, 14), IR(0, 0), IR(276, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(37214, 14), IR(0, 0), IR(280, 1), IR(100, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(37228, 10), IR(0, 0), IR(280, 1), IR(100, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(37238, 15), IR(102, 1), IR(1, 0), IR(177, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(37274, 21), IR(103, 1), IR(1, 0), IR(177, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(37322, 14), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(37336, 18), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(37354, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(37377, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(37400, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(37423, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(37446, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(37468, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37490, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37536, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(37580, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(37619, 37), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(37656, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(37678, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37700, 60), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37760, 53), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37813, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37860, 40), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37900, 51), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37951, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(37995, 51), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(38046, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(38102, 50), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(38152, 52), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(38204, 49), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(38253, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(38300, 50), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(38350, 51), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(38401, 56), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(38457, 33), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38490, 52), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38542, 61), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(38603, 61), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(38664, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(38703, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(38741, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(38780, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(38818, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(38857, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(38895, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(38931, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(38970, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(39013, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(39051, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(39089, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(39127, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(39172, 40), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(39212, 62), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(39274, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(39304, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(39342, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(39378, 33), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(39411, 35), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(39446, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(39485, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(39528, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(39575, 54), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(39629, 34), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(39663, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(39710, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(39755, 55), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(39810, 53), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(39863, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(39919, 54), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(39973, 58), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(40031, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(40087, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(40125, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(40171, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(40215, 49), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(40264, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(40311, 70), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(40381, 68), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(40449, 69), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(40518, 68), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(40586, 66), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(40652, 67), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(40719, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(40755, 48), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(40803, 59), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(40862, 58), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(40920, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(645, 9), IR(40976, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(654, 10), IR(41006, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(41036, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(41075, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(41121, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41164, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41211, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(41256, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(41302, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(41358, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(41396, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(41426, 32), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(654, 10), IR(41458, 36), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(664, 13), IR(41494, 42), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(41536, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(41575, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(41614, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(41658, 48), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(41706, 50), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(41756, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(41799, 48), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(41847, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(41890, 31), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41921, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41968, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(42013, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(42059, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(42115, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(42153, 35), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(42188, 44), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(42232, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(42278, 41), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(42319, 36), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(42355, 50), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(42405, 48), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(42453, 35), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(12531, 25), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(42488, 16), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(42504, 19), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiINTEL, true, true, IR(677, 8), IR(42523, 28), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastINTEL, true, true, IR(677, 8), IR(42551, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntINTEL, true, true, IR(677, 8), IR(42575, 31), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntINTEL, true, true, IR(677, 8), IR(42606, 29), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddINTEL, true, true, IR(685, 10), IR(42635, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubINTEL, true, true, IR(685, 10), IR(42658, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulINTEL, true, true, IR(685, 10), IR(42681, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivINTEL, true, true, IR(685, 10), IR(42704, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTINTEL, true, true, IR(695, 6), IR(42727, 22), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEINTEL, true, true, IR(695, 6), IR(42749, 22), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTINTEL, true, true, IR(695, 6), IR(42771, 22), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEINTEL, true, true, IR(695, 6), IR(42793, 22), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQINTEL, true, true, IR(695, 6), IR(42815, 22), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipINTEL, true, true, IR(677, 8), IR(42837, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtINTEL, true, true, IR(677, 8), IR(42862, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtINTEL, true, true, IR(677, 8), IR(42887, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotINTEL, true, true, IR(685, 10), IR(42911, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtINTEL, true, true, IR(677, 8), IR(42936, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(677, 8), IR(42960, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(677, 8), IR(42983, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(677, 8), IR(43007, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(677, 8), IR(43032, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(677, 8), IR(43057, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(677, 8), IR(43080, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(677, 8), IR(43104, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(677, 8), IR(43129, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(677, 8), IR(43154, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(677, 8), IR(43177, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(677, 8), IR(43200, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(677, 8), IR(43226, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(677, 8), IR(43251, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(677, 8), IR(43276, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(677, 8), IR(43300, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(677, 8), IR(43326, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(677, 8), IR(43350, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(677, 8), IR(43376, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(677, 8), IR(43400, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(685, 10), IR(43426, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(685, 10), IR(43451, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(685, 10), IR(43474, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(685, 10), IR(43498, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(43522, 17), IR(0, 0), IR(288, 1), IR(107, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(701, 2), IR(43539, 21), IR(0, 0), IR(136, 1), IR(114, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(703, 3), IR(43560, 20), IR(0, 0), IR(136, 1), IR(114, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(43580, 24), IR(0, 0), IR(136, 1), IR(114, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtINTEL, true, true, IR(677, 8), IR(43604, 15), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipINTEL, true, true, IR(677, 8), IR(43619, 16), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtINTEL, true, true, IR(677, 8), IR(43635, 16), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinINTEL, true, true, IR(677, 8), IR(43651, 14), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosINTEL, true, true, IR(677, 8), IR(43665, 14), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosINTEL, true, true, IR(677, 8), IR(43679, 17), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiINTEL, true, true, IR(677, 8), IR(43696, 16), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiINTEL, true, true, IR(677, 8), IR(43712, 16), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiINTEL, true, true, IR(677, 8), IR(43728, 19), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogINTEL, true, true, IR(677, 8), IR(43747, 14), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpINTEL, true, true, IR(677, 8), IR(43761, 14), IR(0, 0), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupINTEL, true, true, IR(160, 3), IR(43775, 29), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrINTEL, true, true, IR(160, 3), IR(43804, 29), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingINTEL, true, true, IR(172, 4), IR(43833, 22), IR(0, 0), IR(289, 1), IR(121, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipeBlockingINTEL, true, true, IR(172, 4), IR(43855, 23), IR(0, 0), IR(289, 1), IR(121, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpFPGARegINTEL, true, true, IR(160, 3), IR(13090, 13), IR(0, 0), IR(290, 1), IR(122, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(43878, 22), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(43900, 23), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(43923, 28), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(43951, 46), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(43997, 37), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(44034, 65), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(44099, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(44139, 41), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(44180, 39), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(44219, 36), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(44255, 46), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(44301, 45), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(44346, 42), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(44388, 32), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(44420, 29), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(44449, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(44489, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(44529, 14), IR(0, 0), IR(291, 4), IR(130, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(44543, 23), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(706, 1), IR(44566, 25), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(44591, 32), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(44623, 36), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(44659, 33), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(44692, 20), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(44712, 20), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(44732, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(44758, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(13529, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateINTEL, true, true, IR(707, 7), IR(44782, 24), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceAsyncINTEL, false, false, IR(168, 2), IR(44806, 23), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceGetINTEL, true, true, IR(160, 3), IR(44829, 21), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceReleaseINTEL, false, false, IR(10, 1), IR(44850, 25), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeTaskSequenceINTEL, true, false, IR(68, 1), IR(44875, 22), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(44897, 27), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(714, 10), IR(44924, 25), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(714, 10), IR(44949, 34), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(714, 10), IR(44983, 34), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(724, 9), IR(45017, 29), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(714, 10), IR(45046, 26), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(733, 7), IR(13867, 38), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(45134, 21), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(13933, 32), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(45155, 26), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(740, 5), IR(45181, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(745, 2), IR(45208, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(747, 4), IR(45235, 24), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(751, 6), IR(45259, 30), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(757, 3), IR(45326, 30), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(45356, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(45383, 13), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(45396, 13), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(45409, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(45428, 18), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(45446, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(45465, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(45484, 18), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(45502, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(45521, 18), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(760, 6), IR(45539, 18), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(766, 4), IR(45557, 19), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(45576, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(45602, 28), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(45630, 33), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 834> kInstructionNames{{
  {IR(36926, 13), 554}, // 0 AbsISubINTEL
  {IR(36939, 13), 555}, // 1 AbsUSubINTEL
  {IR(26739, 12), 58}, // 2 AccessChain
  {IR(43539, 21), 740}, // 3 AliasDomainDeclINTEL
  {IR(43560, 20), 741}, // 4 AliasScopeDeclINTEL
  {IR(43580, 24), 742}, // 5 AliasScopeListDeclINTEL
  {IR(28036, 4), 143}, // 6 All
  {IR(33415, 25), 426}, // 7 AllocateNodePayloadsAMDX
  {IR(28032, 4), 142}, // 8 Any
  {IR(43326, 24), 731}, // 9 ArbitraryFloatACosINTEL
  {IR(43350, 26), 732}, // 10 ArbitraryFloatACosPiINTEL
  {IR(43276, 24), 729}, // 11 ArbitraryFloatASinINTEL
  {IR(43300, 26), 730}, // 12 ArbitraryFloatASinPiINTEL
  {IR(43426, 25), 735}, // 13 ArbitraryFloatATan2INTEL
  {IR(43376, 24), 733}, // 14 ArbitraryFloatATanINTEL
  {IR(43400, 26), 734}, // 15 ArbitraryFloatATanPiINTEL
  {IR(42635, 23), 702}, // 16 ArbitraryFloatAddINTEL
  {IR(42575, 31), 700}, // 17 ArbitraryFloatCastFromIntINTEL
  {IR(42551, 24), 699}, // 18 ArbitraryFloatCastINTEL
  {IR(42606, 29), 701}, // 19 ArbitraryFloatCastToIntINTEL
  {IR(42887, 24), 713}, // 20 ArbitraryFloatCbrtINTEL
  {IR(43177, 23), 725}, // 21 ArbitraryFloatCosINTEL
  {IR(43251, 25), 728}, // 22 ArbitraryFloatCosPiINTEL
  {IR(42704, 23), 705}, // 23 ArbitraryFloatDivINTEL
  {IR(42815, 22), 710}, // 24 ArbitraryFloatEQINTEL
  {IR(43104, 25), 722}, // 25 ArbitraryFloatExp10INTEL
  {IR(43080, 24), 721}, // 26 ArbitraryFloatExp2INTEL
  {IR(43057, 23), 720}, // 27 ArbitraryFloatExpINTEL
  {IR(43129, 25), 723}, // 28 ArbitraryFloatExpm1INTEL
  {IR(42749, 22), 707}, // 29 ArbitraryFloatGEINTEL
  {IR(42727, 22), 706}, // 30 ArbitraryFloatGTINTEL
  {IR(42911, 25), 714}, // 31 ArbitraryFloatHypotINTEL
  {IR(42793, 22), 709}, // 32 ArbitraryFloatLEINTEL
  {IR(42771, 22), 708}, // 33 ArbitraryFloatLTINTEL
  {IR(43007, 25), 718}, // 34 ArbitraryFloatLog10INTEL
  {IR(43032, 25), 719}, // 35 ArbitraryFloatLog1pINTEL
  {IR(42983, 24), 717}, // 36 ArbitraryFloatLog2INTEL
  {IR(42960, 23), 716}, // 37 ArbitraryFloatLogINTEL
  {IR(42681, 23), 704}, // 38 ArbitraryFloatMulINTEL
  {IR(43451, 23), 736}, // 39 ArbitraryFloatPowINTEL
  {IR(43498, 24), 738}, // 40 ArbitraryFloatPowNINTEL
  {IR(43474, 24), 737}, // 41 ArbitraryFloatPowRINTEL
  {IR(42862, 25), 712}, // 42 ArbitraryFloatRSqrtINTEL
  {IR(42837, 25), 711}, // 43 ArbitraryFloatRecipINTEL
  {IR(43200, 26), 726}, // 44 ArbitraryFloatSinCosINTEL
  {IR(42523, 28), 698}, // 45 ArbitraryFloatSinCosPiINTEL
  {IR(43154, 23), 724}, // 46 ArbitraryFloatSinINTEL
  {IR(43226, 25), 727}, // 47 ArbitraryFloatSinPiINTEL
  {IR(42936, 24), 715}, // 48 ArbitraryFloatSqrtINTEL
  {IR(42658, 23), 703}, // 49 ArbitraryFloatSubINTEL
  {IR(13529, 19), 786}, // 50 ArithmeticFenceEXT
  {IR(26786, 12), 61}, // 51 ArrayLength
  {IR(37173, 13), 570}, // 52 AsmCallINTEL
  {IR(12293, 9), 569}, // 53 AsmINTEL
  {IR(37158, 15), 568}, // 54 AsmTargetINTEL
  {IR(37214, 14), 573}, // 55 AssumeTrueKHR
  {IR(29058, 10), 220}, // 56 AtomicAnd
  {IR(28910, 22), 210}, // 57 AtomicCompareExchange
  {IR(28932, 26), 211}, // 58 AtomicCompareExchangeWeak
  {IR(28895, 15), 209}, // 59 AtomicExchange
  {IR(44529, 14), 776}, // 60 AtomicFAddEXT
  {IR(37200, 14), 572}, // 61 AtomicFMaxEXT
  {IR(37186, 14), 571}, // 62 AtomicFMinEXT
  {IR(30443, 16), 292}, // 63 AtomicFlagClear
  {IR(30422, 21), 291}, // 64 AtomicFlagTestAndSet
  {IR(28992, 11), 214}, // 65 AtomicIAdd
  {IR(28975, 17), 213}, // 66 AtomicIDecrement
  {IR(28958, 17), 212}, // 67 AtomicIIncrement
  {IR(29003, 11), 215}, // 68 AtomicISub
  {IR(28872, 11), 207}, // 69 AtomicLoad
  {IR(29068, 9), 221}, // 70 AtomicOr
  {IR(29036, 11), 218}, // 71 AtomicSMax
  {IR(29014, 11), 216}, // 72 AtomicSMin
  {IR(28883, 12), 208}, // 73 AtomicStore
  {IR(29047, 11), 219}, // 74 AtomicUMax
  {IR(29025, 11), 217}, // 75 AtomicUMin
  {IR(29077, 10), 222}, // 76 AtomicXor
  {IR(35422, 28), 500}, // 77 BeginInvocationInterlockEXT
  {IR(32969, 17), 407}, // 78 BitCastArrayQCOM
  {IR(28656, 9), 191}, // 79 BitCount
  {IR(28596, 15), 187}, // 80 BitFieldInsert
  {IR(28611, 17), 188}, // 81 BitFieldSExtract
  {IR(28628, 17), 189}, // 82 BitFieldUExtract
  {IR(28645, 11), 190}, // 83 BitReverse
  {IR(27784, 8), 114}, // 84 Bitcast
  {IR(28581, 11), 185}, // 85 BitwiseAnd
  {IR(45134, 21), 799}, // 86 BitwiseFunctionINTEL
  {IR(28560, 10), 183}, // 87 BitwiseOr
  {IR(28570, 11), 184}, // 88 BitwiseXor
  {IR(29187, 7), 227}, // 89 Branch
  {IR(29194, 18), 228}, // 90 BranchConditional
  {IR(30055, 13), 277}, // 91 BuildNDRange
  {IR(25800, 11), 15}, // 92 Capability
  {IR(30013, 26), 275}, // 93 CaptureEventProfilingInfo
  {IR(31519, 23), 344}, // 94 ColorAttachmentReadEXT
  {IR(29605, 15), 255}, // 95 CommitReadPipe
  {IR(29620, 16), 256}, // 96 CommitWritePipe
  {IR(27046, 19), 72}, // 97 CompositeConstruct
  {IR(44659, 33), 781}, // 98 CompositeConstructContinuedINTEL
  {IR(33102, 30), 412}, // 99 CompositeConstructCoopMatQCOM
  {IR(32677, 31), 395}, // 100 CompositeConstructReplicateEXT
  {IR(27065, 17), 73}, // 101 CompositeExtract
  {IR(33132, 28), 413}, // 102 CompositeExtractCoopMatQCOM
  {IR(27082, 16), 74}, // 103 CompositeInsert
  {IR(45208, 27), 803}, // 104 ConditionalCapabilityINTEL
  {IR(45356, 27), 807}, // 105 ConditionalCopyObjectINTEL
  {IR(45181, 27), 802}, // 106 ConditionalEntryPointINTEL
  {IR(45155, 26), 801}, // 107 ConditionalExtensionINTEL
  {IR(15523, 9), 39}, // 108 Constant
  {IR(26306, 18), 40}, // 109 ConstantComposite
  {IR(44591, 32), 779}, // 110 ConstantCompositeContinuedINTEL
  {IR(32613, 30), 393}, // 111 ConstantCompositeReplicateEXT
  {IR(26254, 14), 38}, // 112 ConstantFalse
  {IR(37104, 29), 566}, // 113 ConstantFunctionPointerINTEL
  {IR(26418, 13), 42}, // 114 ConstantNull
  {IR(30498, 20), 296}, // 115 ConstantPipeStorage
  {IR(26402, 16), 41}, // 116 ConstantSampler
  {IR(33568, 19), 432}, // 117 ConstantStringAMDX
  {IR(26241, 13), 37}, // 118 ConstantTrue
  {IR(28843, 15), 205}, // 119 ControlBarrier
  {IR(44732, 26), 784}, // 120 ControlBarrierArriveINTEL
  {IR(44758, 24), 785}, // 121 ControlBarrierWaitINTEL
  {IR(44712, 20), 783}, // 122 ConvertBF16ToFINTEL
  {IR(44692, 20), 782}, // 123 ConvertFToBF16INTEL
  {IR(27590, 12), 100}, // 124 ConvertFToS
  {IR(27578, 12), 99}, // 125 ConvertFToU
  {IR(45576, 26), 819}, // 126 ConvertHandleToImageINTEL
  {IR(45630, 33), 821}, // 127 ConvertHandleToSampledImageINTEL
  {IR(45602, 28), 820}, // 128 ConvertHandleToSamplerINTEL
  {IR(36071, 18), 523}, // 129 ConvertImageToUNV
  {IR(27667, 14), 107}, // 130 ConvertPtrToU
  {IR(27602, 12), 101}, // 131 ConvertSToF
  {IR(36134, 25), 526}, // 132 ConvertSampledImageToUNV
  {IR(36089, 20), 524}, // 133 ConvertSamplerToUNV
  {IR(32191, 35), 379}, // 134 ConvertUToAccelerationStructureKHR
  {IR(27614, 12), 102}, // 135 ConvertUToF
  {IR(36033, 18), 521}, // 136 ConvertUToImageNV
  {IR(27711, 14), 110}, // 137 ConvertUToPtr
  {IR(36109, 25), 525}, // 138 ConvertUToSampledImageNV
  {IR(36051, 20), 522}, // 139 ConvertUToSamplerNV
  {IR(34728, 27), 475}, // 140 CooperativeMatrixConvertNV
  {IR(32586, 27), 392}, // 141 CooperativeMatrixLengthKHR
  {IR(35396, 26), 499}, // 142 CooperativeMatrixLengthNV
  {IR(32454, 25), 389}, // 143 CooperativeMatrixLoadKHR
  {IR(35321, 24), 496}, // 144 CooperativeMatrixLoadNV
  {IR(35620, 30), 503}, // 145 CooperativeMatrixLoadTensorNV
  {IR(32559, 27), 391}, // 146 CooperativeMatrixMulAddKHR
  {IR(35370, 26), 498}, // 147 CooperativeMatrixMulAddNV
  {IR(35681, 32), 505}, // 148 CooperativeMatrixPerElementOpNV
  {IR(35519, 26), 502}, // 149 CooperativeMatrixReduceNV
  {IR(32479, 26), 390}, // 150 CooperativeMatrixStoreKHR
  {IR(35345, 25), 497}, // 151 CooperativeMatrixStoreNV
  {IR(35650, 31), 504}, // 152 CooperativeMatrixStoreTensorNV
  {IR(36004, 29), 520}, // 153 CooperativeMatrixTransposeNV
  {IR(34923, 24), 482}, // 154 CooperativeVectorLoadNV
  {IR(34696, 32), 474}, // 155 CooperativeVectorMatrixMulAddNV
  {IR(34586, 29), 471}, // 156 CooperativeVectorMatrixMulNV
  {IR(34615, 42), 472}, // 157 CooperativeVectorOuterProductAccumulateNV
  {IR(34657, 39), 473}, // 158 CooperativeVectorReduceSumAccumulateNV
  {IR(34947, 25), 483}, // 159 CooperativeVectorStoreNV
  {IR(31478, 12), 340}, // 160 CopyLogical
  {IR(26712, 11), 56}, // 161 CopyMemory
  {IR(26723, 16), 57}, // 162 CopyMemorySized
  {IR(27098, 11), 75}, // 163 CopyObject
  {IR(30518, 26), 297}, // 164 CreatePipeFromPipeStorage
  {IR(35749, 21), 508}, // 165 CreateTensorLayoutNV
  {IR(35869, 19), 513}, // 166 CreateTensorViewNV
  {IR(29965, 16), 272}, // 167 CreateUserEvent
  {IR(43804, 29), 755}, // 168 CrossWorkgroupCastToPtrINTEL
  {IR(28665, 5), 192}, // 169 DPdx
  {IR(28711, 11), 198}, // 170 DPdxCoarse
  {IR(28682, 9), 195}, // 171 DPdxFine
  {IR(28670, 5), 193}, // 172 DPdy
  {IR(28722, 11), 199}, // 173 DPdyCoarse
  {IR(28691, 9), 196}, // 174 DPdyFine
  {IR(26872, 9), 64}, // 175 Decorate
  {IR(30695, 11), 305}, // 176 DecorateId
  {IR(37238, 15), 575}, // 177 DecorateString
  {IR(37253, 21), 575}, // 178 DecorateStringGOOGLE
  {IR(26896, 16), 66}, // 179 DecorationGroup
  {IR(11604, 25), 516}, // 180 DemoteToHelperInvocation
  {IR(11629, 28), 516}, // 181 DemoteToHelperInvocationEXT
  {IR(31542, 23), 345}, // 182 DepthAttachmentReadEXT
  {IR(27981, 4), 137}, // 183 Dot
  {IR(34755, 17), 476}, // 184 EmitMeshTasksEXT
  {IR(28770, 17), 203}, // 185 EmitStreamVertex
  {IR(28746, 11), 201}, // 186 EmitVertex
  {IR(35450, 26), 501}, // 187 EndInvocationInterlockEXT
  {IR(28757, 13), 202}, // 188 EndPrimitive
  {IR(28787, 19), 204}, // 189 EndStreamPrimitive
  {IR(29801, 14), 265}, // 190 EnqueueKernel
  {IR(29787, 14), 264}, // 191 EnqueueMarker
  {IR(33440, 24), 427}, // 192 EnqueueNodePayloadsAMDX
  {IR(25715, 11), 13}, // 193 EntryPoint
  {IR(32172, 19), 378}, // 194 ExecuteCallableKHR
  {IR(35197, 18), 492}, // 195 ExecuteCallableNV
  {IR(25758, 14), 14}, // 196 ExecutionMode
  {IR(30679, 16), 304}, // 197 ExecutionModeId
  {IR(37228, 10), 574}, // 198 ExpectKHR
  {IR(25569, 8), 11}, // 199 ExtInst
  {IR(25509, 14), 10}, // 200 ExtInstImport
  {IR(32109, 26), 375}, // 201 ExtInstWithForwardRefsKHR
  {IR(25499, 10), 9}, // 202 Extension
  {IR(33160, 20), 414}, // 203 ExtractSubArrayQCOM
  {IR(27813, 5), 118}, // 204 FAdd
  {IR(27644, 9), 105}, // 205 FConvert
  {IR(27848, 5), 125}, // 206 FDiv
  {IR(27873, 5), 130}, // 207 FMod
  {IR(27833, 5), 122}, // 208 FMul
  {IR(27800, 8), 116}, // 209 FNegate
  {IR(28310, 10), 168}, // 210 FOrdEqual
  {IR(28388, 16), 174}, // 211 FOrdGreaterThan
  {IR(28460, 21), 178}, // 212 FOrdGreaterThanEqual
  {IR(28360, 13), 172}, // 213 FOrdLessThan
  {IR(28422, 18), 176}, // 214 FOrdLessThanEqual
  {IR(28332, 13), 170}, // 215 FOrdNotEqual
  {IR(13090, 13), 758}, // 216 FPGARegINTEL
  {IR(27868, 5), 129}, // 217 FRem
  {IR(27823, 5), 120}, // 218 FSub
  {IR(28320, 12), 169}, // 219 FUnordEqual
  {IR(28404, 18), 175}, // 220 FUnordGreaterThan
  {IR(28481, 23), 179}, // 221 FUnordGreaterThanEqual
  {IR(28373, 15), 173}, // 222 FUnordLessThan
  {IR(28440, 20), 177}, // 223 FUnordLessThanEqual
  {IR(28345, 15), 171}, // 224 FUnordNotEqual
  {IR(34885, 38), 481}, // 225 FetchMicroTriangleVertexBarycentricNV
  {IR(34850, 35), 480}, // 226 FetchMicroTriangleVertexPositionNV
  {IR(33489, 29), 429}, // 227 FinishWritingNodePayloadAMDX
  {IR(43665, 14), 747}, // 228 FixedCosINTEL
  {IR(43712, 16), 750}, // 229 FixedCosPiINTEL
  {IR(43761, 14), 753}, // 230 FixedExpINTEL
  {IR(43747, 14), 752}, // 231 FixedLogINTEL
  {IR(43619, 16), 744}, // 232 FixedRecipINTEL
  {IR(43635, 16), 745}, // 233 FixedRsqrtINTEL
  {IR(43679, 17), 748}, // 234 FixedSinCosINTEL
  {IR(43728, 19), 751}, // 235 FixedSinCosPiINTEL
  {IR(43651, 14), 746}, // 236 FixedSinINTEL
  {IR(43696, 16), 749}, // 237 FixedSinPiINTEL
  {IR(43604, 15), 743}, // 238 FixedSqrtINTEL
  {IR(32026, 7), 369}, // 239 FmaKHR
  {IR(33385, 17), 424}, // 240 FragmentFetchAMD
  {IR(33364, 21), 423}, // 241 FragmentMaskFetchAMD
  {IR(24619, 9), 48}, // 242 Function
  {IR(26621, 13), 51}, // 243 FunctionCall
  {IR(26609, 12), 50}, // 244 FunctionEnd
  {IR(26591, 18), 49}, // 245 FunctionParameter
  {IR(37133, 25), 567}, // 246 FunctionPointerCallINTEL
  {IR(28675, 7), 194}, // 247 Fwidth
  {IR(28733, 13), 200}, // 248 FwidthCoarse
  {IR(28700, 11), 197}, // 249 FwidthFine
  {IR(27742, 17), 112}, // 250 GenericCastToPtr
  {IR(27759, 25), 113}, // 251 GenericCastToPtrExplicit
  {IR(26798, 23), 62}, // 252 GenericPtrMemSemantics
  {IR(30039, 16), 276}, // 253 GetDefaultQueue
  {IR(30544, 35), 298}, // 254 GetKernelLocalSizeForSubgroupCount
  {IR(30579, 25), 299}, // 255 GetKernelMaxNumSubgroups
  {IR(29845, 32), 267}, // 256 GetKernelNDrangeMaxSubGroupSize
  {IR(29815, 30), 266}, // 257 GetKernelNDrangeSubGroupCount
  {IR(29900, 40), 269}, // 258 GetKernelPreferredWorkGroupSizeMultiple
  {IR(29877, 23), 268}, // 259 GetKernelWorkGroupSize
  {IR(29671, 18), 259}, // 260 GetMaxPipePackets
  {IR(29653, 18), 258}, // 261 GetNumPipePackets
  {IR(9073, 9), 353}, // 262 GraphARM
  {IR(31694, 17), 351}, // 263 GraphConstantARM
  {IR(31762, 12), 356}, // 264 GraphEndARM
  {IR(31711, 19), 352}, // 265 GraphEntryPointARM
  {IR(31730, 14), 354}, // 266 GraphInputARM
  {IR(31744, 18), 355}, // 267 GraphSetOutputARM
  {IR(29358, 9), 238}, // 268 GroupAll
  {IR(29367, 9), 239}, // 269 GroupAny
  {IR(29327, 15), 236}, // 270 GroupAsyncCopy
  {IR(45409, 19), 810}, // 271 GroupBitwiseAndKHR
  {IR(45428, 18), 811}, // 272 GroupBitwiseOrKHR
  {IR(45446, 19), 812}, // 273 GroupBitwiseXorKHR
  {IR(29376, 15), 240}, // 274 GroupBroadcast
  {IR(29746, 20), 262}, // 275 GroupCommitReadPipe
  {IR(29766, 21), 263}, // 276 GroupCommitWritePipe
  {IR(26912, 14), 67}, // 277 GroupDecorate
  {IR(29434, 10), 242}, // 278 GroupFAdd
  {IR(33203, 23), 416}, // 279 GroupFAddNonUniformAMD
  {IR(29474, 10), 246}, // 280 GroupFMax
  {IR(33295, 23), 420}, // 281 GroupFMaxNonUniformAMD
  {IR(29444, 10), 243}, // 282 GroupFMin
  {IR(33226, 23), 417}, // 283 GroupFMinNonUniformAMD
  {IR(45396, 13), 809}, // 284 GroupFMulKHR
  {IR(29424, 10), 241}, // 285 GroupIAdd
  {IR(33180, 23), 415}, // 286 GroupIAddNonUniformAMD
  {IR(45383, 13), 808}, // 287 GroupIMulKHR
  {IR(45465, 19), 813}, // 288 GroupLogicalAndKHR
  {IR(45484, 18), 814}, // 289 GroupLogicalOrKHR
  {IR(45502, 19), 815}, // 290 GroupLogicalXorKHR
  {IR(26971, 20), 68}, // 291 GroupMemberDecorate
  {IR(30727, 19), 307}, // 292 GroupNonUniformAll
  {IR(30765, 24), 309}, // 293 GroupNonUniformAllEqual
  {IR(30746, 19), 308}, // 294 GroupNonUniformAny
  {IR(5692, 22), 312}, // 295 GroupNonUniformBallot
  {IR(30905, 30), 315}, // 296 GroupNonUniformBallotBitCount
  {IR(30873, 32), 314}, // 297 GroupNonUniformBallotBitExtract
  {IR(30935, 29), 316}, // 298 GroupNonUniformBallotFindLSB
  {IR(30964, 29), 317}, // 299 GroupNonUniformBallotFindMSB
  {IR(31271, 26), 332}, // 300 GroupNonUniformBitwiseAnd
  {IR(31297, 25), 333}, // 301 GroupNonUniformBitwiseOr
  {IR(31322, 26), 334}, // 302 GroupNonUniformBitwiseXor
  {IR(30789, 25), 310}, // 303 GroupNonUniformBroadcast
  {IR(30814, 30), 311}, // 304 GroupNonUniformBroadcastFirst
  {IR(30706, 21), 306}, // 305 GroupNonUniformElect
  {IR(31091, 20), 323}, // 306 GroupNonUniformFAdd
  {IR(31251, 20), 331}, // 307 GroupNonUniformFMax
  {IR(31191, 20), 328}, // 308 GroupNonUniformFMin
  {IR(31131, 20), 325}, // 309 GroupNonUniformFMul
  {IR(31071, 20), 322}, // 310 GroupNonUniformIAdd
  {IR(31111, 20), 324}, // 311 GroupNonUniformIMul
  {IR(30844, 29), 313}, // 312 GroupNonUniformInverseBallot
  {IR(31348, 26), 335}, // 313 GroupNonUniformLogicalAnd
  {IR(31374, 25), 336}, // 314 GroupNonUniformLogicalOr
  {IR(31399, 26), 337}, // 315 GroupNonUniformLogicalXor
  {IR(34790, 27), 478}, // 316 GroupNonUniformPartitionNV
  {IR(33610, 26), 434}, // 317 GroupNonUniformQuadAllKHR
  {IR(33636, 26), 435}, // 318 GroupNonUniformQuadAnyKHR
  {IR(31425, 29), 338}, // 319 GroupNonUniformQuadBroadcast
  {IR(31454, 24), 339}, // 320 GroupNonUniformQuadSwap
  {IR(13315, 25), 373}, // 321 GroupNonUniformRotateKHR
  {IR(31211, 20), 329}, // 322 GroupNonUniformSMax
  {IR(31151, 20), 326}, // 323 GroupNonUniformSMin
  {IR(8782, 23), 318}, // 324 GroupNonUniformShuffle
  {IR(31044, 27), 321}, // 325 GroupNonUniformShuffleDown
  {IR(31019, 25), 320}, // 326 GroupNonUniformShuffleUp
  {IR(30993, 26), 319}, // 327 GroupNonUniformShuffleXor
  {IR(31231, 20), 330}, // 328 GroupNonUniformUMax
  {IR(31171, 20), 327}, // 329 GroupNonUniformUMin
  {IR(29689, 28), 260}, // 330 GroupReserveReadPipePackets
  {IR(29717, 29), 261}, // 331 GroupReserveWritePipePackets
  {IR(29494, 10), 248}, // 332 GroupSMax
  {IR(33341, 23), 422}, // 333 GroupSMaxNonUniformAMD
  {IR(29464, 10), 245}, // 334 GroupSMin
  {IR(33272, 23), 419}, // 335 GroupSMinNonUniformAMD
  {IR(29484, 10), 247}, // 336 GroupUMax
  {IR(33318, 23), 421}, // 337 GroupUMaxNonUniformAMD
  {IR(29454, 10), 244}, // 338 GroupUMin
  {IR(33249, 23), 418}, // 339 GroupUMinNonUniformAMD
  {IR(29342, 16), 237}, // 340 GroupWaitEvents
  {IR(34097, 25), 451}, // 341 HitObjectExecuteShaderNV
  {IR(34148, 25), 453}, // 342 HitObjectGetAttributesNV
  {IR(35273, 24), 494}, // 343 HitObjectGetClusterIdNV
  {IR(34122, 26), 452}, // 344 HitObjectGetCurrentTimeNV
  {IR(34224, 28), 456}, // 345 HitObjectGetGeometryIndexNV
  {IR(34173, 22), 454}, // 346 HitObjectGetHitKindNV
  {IR(34277, 34), 458}, // 347 HitObjectGetInstanceCustomIndexNV
  {IR(34252, 25), 457}, // 348 HitObjectGetInstanceIdNV
  {IR(36500, 27), 536}, // 349 HitObjectGetLSSPositionsNV
  {IR(36527, 23), 537}, // 350 HitObjectGetLSSRadiiNV
  {IR(33809, 33), 441}, // 351 HitObjectGetObjectRayDirectionNV
  {IR(33842, 30), 442}, // 352 HitObjectGetObjectRayOriginNV
  {IR(33781, 28), 440}, // 353 HitObjectGetObjectToWorldNV
  {IR(34195, 29), 455}, // 354 HitObjectGetPrimitiveIndexNV
  {IR(34372, 22), 461}, // 355 HitObjectGetRayTMaxNV
  {IR(34394, 22), 462}, // 356 HitObjectGetRayTMinNV
  {IR(33937, 44), 445}, // 357 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(33898, 39), 444}, // 358 HitObjectGetShaderRecordBufferHandleNV
  {IR(36444, 29), 534}, // 359 HitObjectGetSpherePositionNV
  {IR(36473, 27), 535}, // 360 HitObjectGetSphereRadiusNV
  {IR(34311, 32), 459}, // 361 HitObjectGetWorldRayDirectionNV
  {IR(34343, 29), 460}, // 362 HitObjectGetWorldRayOriginNV
  {IR(33753, 28), 439}, // 363 HitObjectGetWorldToObjectNV
  {IR(34416, 19), 463}, // 364 HitObjectIsEmptyNV
  {IR(34435, 17), 464}, // 365 HitObjectIsHitNV
  {IR(36573, 20), 539}, // 366 HitObjectIsLSSHitNV
  {IR(34452, 18), 465}, // 367 HitObjectIsMissNV
  {IR(36550, 23), 538}, // 368 HitObjectIsSphereHitNV
  {IR(33981, 23), 446}, // 369 HitObjectRecordEmptyNV
  {IR(33662, 27), 436}, // 370 HitObjectRecordHitMotionNV
  {IR(34024, 21), 448}, // 371 HitObjectRecordHitNV
  {IR(33689, 36), 437}, // 372 HitObjectRecordHitWithIndexMotionNV
  {IR(34045, 30), 449}, // 373 HitObjectRecordHitWithIndexNV
  {IR(33725, 28), 438}, // 374 HitObjectRecordMissMotionNV
  {IR(34075, 22), 450}, // 375 HitObjectRecordMissNV
  {IR(33872, 26), 443}, // 376 HitObjectTraceRayMotionNV
  {IR(34004, 20), 447}, // 377 HitObjectTraceRayNV
  {IR(27808, 5), 117}, // 378 IAdd
  {IR(27985, 10), 138}, // 379 IAddCarry
  {IR(36952, 13), 556}, // 380 IAddSatINTEL
  {IR(36978, 14), 558}, // 381 IAverageINTEL
  {IR(37006, 21), 560}, // 382 IAverageRoundedINTEL
  {IR(28181, 7), 158}, // 383 IEqual
  {IR(27828, 5), 121}, // 384 IMul
  {IR(37074, 15), 564}, // 385 IMul32x16INTEL
  {IR(28188, 10), 159}, // 386 INotEqual
  {IR(27818, 5), 119}, // 387 ISub
  {IR(27995, 11), 139}, // 388 ISubBorrow
  {IR(37048, 13), 562}, // 389 ISubSatINTEL
  {IR(32226, 22), 380}, // 390 IgnoreIntersectionKHR
  {IR(35015, 21), 485}, // 391 IgnoreIntersectionNV
  {IR(24663, 6), 91}, // 392 Image
  {IR(33073, 29), 411}, // 393 ImageBlockMatchGatherSADQCOM
  {IR(33044, 29), 410}, // 394 ImageBlockMatchGatherSSDQCOM
  {IR(32946, 23), 406}, // 395 ImageBlockMatchSADQCOM
  {IR(32923, 23), 405}, // 396 ImageBlockMatchSSDQCOM
  {IR(33015, 29), 409}, // 397 ImageBlockMatchWindowSADQCOM
  {IR(32986, 29), 408}, // 398 ImageBlockMatchWindowSSDQCOM
  {IR(32904, 19), 404}, // 399 ImageBoxFilterQCOM
  {IR(27426, 16), 88}, // 400 ImageDrefGather
  {IR(27403, 11), 86}, // 401 ImageFetch
  {IR(27414, 12), 87}, // 402 ImageGather
  {IR(27463, 17), 92}, // 403 ImageQueryFormat
  {IR(27543, 17), 97}, // 404 ImageQueryLevels
  {IR(27529, 14), 96}, // 405 ImageQueryLod
  {IR(27480, 16), 93}, // 406 ImageQueryOrder
  {IR(27560, 18), 98}, // 407 ImageQuerySamples
  {IR(27514, 15), 95}, // 408 ImageQuerySize
  {IR(27496, 18), 94}, // 409 ImageQuerySizeLod
  {IR(27442, 10), 89}, // 410 ImageRead
  {IR(27260, 27), 81}, // 411 ImageSampleDrefExplicitLod
  {IR(27233, 27), 80}, // 412 ImageSampleDrefImplicitLod
  {IR(27210, 23), 79}, // 413 ImageSampleExplicitLod
  {IR(34539, 23), 469}, // 414 ImageSampleFootprintNV
  {IR(27164, 23), 78}, // 415 ImageSampleImplicitLod
  {IR(27372, 31), 85}, // 416 ImageSampleProjDrefExplicitLod
  {IR(27341, 31), 84}, // 417 ImageSampleProjDrefImplicitLod
  {IR(27314, 27), 83}, // 418 ImageSampleProjExplicitLod
  {IR(27287, 27), 82}, // 419 ImageSampleProjImplicitLod
  {IR(32880, 24), 403}, // 420 ImageSampleWeightedQCOM
  {IR(30367, 22), 288}, // 421 ImageSparseDrefGather
  {IR(30332, 17), 286}, // 422 ImageSparseFetch
  {IR(30349, 18), 287}, // 423 ImageSparseGather
  {IR(30459, 16), 293}, // 424 ImageSparseRead
  {IR(30159, 33), 281}, // 425 ImageSparseSampleDrefExplicitLod
  {IR(30126, 33), 280}, // 426 ImageSparseSampleDrefImplicitLod
  {IR(30097, 29), 279}, // 427 ImageSparseSampleExplicitLod
  {IR(30068, 29), 278}, // 428 ImageSparseSampleImplicitLod
  {IR(30295, 37), 285}, // 429 ImageSparseSampleProjDrefExplicitLod
  {IR(30258, 37), 284}, // 430 ImageSparseSampleProjDrefImplicitLod
  {IR(30225, 33), 283}, // 431 ImageSparseSampleProjExplicitLod
  {IR(30192, 33), 282}, // 432 ImageSparseSampleProjImplicitLod
  {IR(30389, 26), 289}, // 433 ImageSparseTexelsResident
  {IR(26643, 18), 53}, // 434 ImageTexelPointer
  {IR(27452, 11), 90}, // 435 ImageWrite
  {IR(26751, 20), 59}, // 436 InBoundsAccessChain
  {IR(26821, 23), 63}, // 437 InBoundsPtrAccessChain
  {IR(28052, 9), 146}, // 438 IsFinite
  {IR(35935, 22), 517}, // 439 IsHelperInvocationEXT
  {IR(28046, 6), 145}, // 440 IsInf
  {IR(28040, 6), 144}, // 441 IsNan
  {IR(33545, 23), 431}, // 442 IsNodePayloadValidAMDX
  {IR(28061, 9), 147}, // 443 IsNormal
  {IR(29981, 13), 273}, // 444 IsValidEvent
  {IR(29636, 17), 257}, // 445 IsValidReserveId
  {IR(29264, 5), 230}, // 446 Kill
  {IR(29181, 6), 226}, // 447 Label
  {IR(28081, 14), 149}, // 448 LessOrGreater
  {IR(29300, 14), 234}, // 449 LifetimeStart
  {IR(29314, 13), 235}, // 450 LifetimeStop
  {IR(25494, 5), 8}, // 451 Line
  {IR(26701, 5), 54}, // 452 Load
  {IR(28152, 11), 155}, // 453 LogicalAnd
  {IR(28113, 13), 152}, // 454 LogicalEqual
  {IR(28163, 11), 156}, // 455 LogicalNot
  {IR(28126, 16), 153}, // 456 LogicalNotEqual
  {IR(28142, 10), 154}, // 457 LogicalOr
  {IR(43522, 17), 739}, // 458 LoopControlINTEL
  {IR(29121, 10), 224}, // 459 LoopMerge
  {IR(45539, 18), 817}, // 460 MaskedGatherINTEL
  {IR(45557, 19), 818}, // 461 MaskedScatterINTEL
  {IR(27950, 18), 135}, // 462 MatrixTimesMatrix
  {IR(27896, 18), 132}, // 463 MatrixTimesScalar
  {IR(27932, 18), 134}, // 464 MatrixTimesVector
  {IR(26881, 15), 65}, // 465 MemberDecorate
  {IR(37274, 21), 576}, // 466 MemberDecorateString
  {IR(37295, 27), 576}, // 467 MemberDecorateStringGOOGLE
  {IR(25476, 11), 6}, // 468 MemberName
  {IR(28858, 14), 206}, // 469 MemoryBarrier
  {IR(25641, 12), 12}, // 470 MemoryModel
  {IR(30644, 19), 302}, // 471 MemoryNamedBarrier
  {IR(30663, 16), 303}, // 472 ModuleProcessed
  {IR(25471, 5), 5}, // 473 Name
  {IR(30621, 23), 301}, // 474 NamedBarrierInitialize
  {IR(30415, 7), 290}, // 475 NoLine
  {IR(33518, 27), 430}, // 476 NodePayloadArrayLengthAMDX
  {IR(25267, 4), 0}, // 477 Nop
  {IR(28592, 4), 186}, // 478 Not
  {IR(28095, 8), 150}, // 479 Ordered
  {IR(27968, 13), 136}, // 480 OuterProduct
  {IR(29087, 4), 223}, // 481 Phi
  {IR(26771, 15), 60}, // 482 PtrAccessChain
  {IR(43775, 29), 754}, // 483 PtrCastToCrossWorkgroupINTEL
  {IR(27725, 17), 111}, // 484 PtrCastToGeneric
  {IR(31511, 8), 343}, // 485 PtrDiff
  {IR(31490, 9), 341}, // 486 PtrEqual
  {IR(31499, 12), 342}, // 487 PtrNotEqual
  {IR(27653, 14), 106}, // 488 QuantizeToF16
  {IR(36240, 17), 528}, // 489 RawAccessChainNV
  {IR(32799, 31), 400}, // 490 RayQueryConfirmIntersectionKHR
  {IR(32767, 32), 399}, // 491 RayQueryGenerateIntersectionKHR
  {IR(35250, 23), 493}, // 492 RayQueryGetClusterIdNV
  {IR(44180, 39), 767}, // 493 RayQueryGetIntersectionBarycentricsKHR
  {IR(44255, 46), 769}, // 494 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(35215, 35), 493}, // 495 RayQueryGetIntersectionClusterIdNV
  {IR(44219, 36), 768}, // 496 RayQueryGetIntersectionFrontFaceKHR
  {IR(44099, 40), 765}, // 497 RayQueryGetIntersectionGeometryIndexKHR
  {IR(43951, 46), 762}, // 498 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(43997, 37), 763}, // 499 RayQueryGetIntersectionInstanceIdKHR
  {IR(44034, 65), 764}, // 500 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(36407, 37), 533}, // 501 RayQueryGetIntersectionLSSHitValueNV
  {IR(36335, 38), 531}, // 502 RayQueryGetIntersectionLSSPositionsNV
  {IR(36373, 34), 532}, // 503 RayQueryGetIntersectionLSSRadiiNV
  {IR(44301, 45), 770}, // 504 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(44346, 42), 771}, // 505 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(44449, 40), 774}, // 506 RayQueryGetIntersectionObjectToWorldKHR
  {IR(44139, 41), 766}, // 507 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(36257, 40), 529}, // 508 RayQueryGetIntersectionSpherePositionNV
  {IR(36297, 38), 530}, // 509 RayQueryGetIntersectionSphereRadiusNV
  {IR(43923, 28), 761}, // 510 RayQueryGetIntersectionTKHR
  {IR(35090, 50), 490}, // 511 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(32849, 31), 402}, // 512 RayQueryGetIntersectionTypeKHR
  {IR(44489, 40), 775}, // 513 RayQueryGetIntersectionWorldToObjectKHR
  {IR(43900, 23), 760}, // 514 RayQueryGetRayFlagsKHR
  {IR(43878, 22), 759}, // 515 RayQueryGetRayTMinKHR
  {IR(44388, 32), 772}, // 516 RayQueryGetWorldRayDirectionKHR
  {IR(44420, 29), 773}, // 517 RayQueryGetWorldRayOriginKHR
  {IR(32724, 22), 397}, // 518 RayQueryInitializeKHR
  {IR(36615, 19), 541}, // 519 RayQueryIsLSSHitNV
  {IR(36593, 22), 540}, // 520 RayQueryIsSphereHitNV
  {IR(32830, 19), 401}, // 521 RayQueryProceedKHR
  {IR(32746, 21), 398}, // 522 RayQueryTerminateKHR
  {IR(33402, 13), 425}, // 523 ReadClockKHR
  {IR(29504, 9), 249}, // 524 ReadPipe
  {IR(43833, 22), 756}, // 525 ReadPipeBlockingINTEL
  {IR(29952, 13), 271}, // 526 ReleaseEvent
  {IR(34499, 24), 467}, // 527 ReorderThreadWithHintNV
  {IR(34470, 29), 466}, // 528 ReorderThreadWithHitObjectNV
  {IR(34972, 22), 484}, // 529 ReportIntersectionKHR
  {IR(34994, 21), 484}, // 530 ReportIntersectionNV
  {IR(29558, 23), 253}, // 531 ReserveReadPipePackets
  {IR(29581, 24), 254}, // 532 ReserveWritePipePackets
  {IR(29523, 17), 251}, // 533 ReservedReadPipe
  {IR(29540, 18), 252}, // 534 ReservedWritePipe
  {IR(42504, 19), 697}, // 535 RestoreMemoryINTEL
  {IR(29940, 12), 270}, // 536 RetainEvent
  {IR(29269, 7), 231}, // 537 Return
  {IR(29276, 12), 232}, // 538 ReturnValue
  {IR(45521, 18), 816}, // 539 RoundFToTF32INTEL
  {IR(27635, 9), 104}, // 540 SConvert
  {IR(27843, 5), 124}, // 541 SDiv
  {IR(32311, 5), 382}, // 542 SDot
  {IR(32352, 11), 385}, // 543 SDotAccSat
  {IR(32363, 14), 385}, // 544 SDotAccSatKHR
  {IR(32316, 8), 382}, // 545 SDotKHR
  {IR(28211, 13), 161}, // 546 SGreaterThan
  {IR(28242, 18), 163}, // 547 SGreaterThanEqual
  {IR(28270, 10), 165}, // 548 SLessThan
  {IR(28295, 15), 167}, // 549 SLessThanEqual
  {IR(27863, 5), 128}, // 550 SMod
  {IR(28019, 13), 141}, // 551 SMulExtended
  {IR(27792, 8), 115}, // 552 SNegate
  {IR(27858, 5), 127}, // 553 SRem
  {IR(32337, 6), 384}, // 554 SUDot
  {IR(32402, 12), 387}, // 555 SUDotAccSat
  {IR(32414, 15), 387}, // 556 SUDotAccSatKHR
  {IR(32343, 9), 384}, // 557 SUDotKHR
  {IR(27119, 13), 77}, // 558 SampledImage
  {IR(36159, 29), 527}, // 559 SamplerImageAddressingModeNV
  {IR(27681, 15), 108}, // 560 SatConvertSToU
  {IR(27696, 15), 109}, // 561 SatConvertUToS
  {IR(42488, 16), 696}, // 562 SaveMemoryINTEL
  {IR(28174, 7), 157}, // 563 Select
  {IR(29166, 15), 225}, // 564 SelectionMerge
  {IR(34772, 18), 477}, // 565 SetMeshOutputsEXT
  {IR(29994, 19), 274}, // 566 SetUserEventStatus
  {IR(28543, 17), 182}, // 567 ShiftLeftLogical
  {IR(28522, 21), 181}, // 568 ShiftRightArithmetic
  {IR(28504, 18), 180}, // 569 ShiftRightLogical
  {IR(28070, 11), 148}, // 570 SignBitSet
  {IR(30475, 7), 294}, // 571 SizeOf
  {IR(25448, 7), 3}, // 572 Source
  {IR(25329, 16), 2}, // 573 SourceContinued
  {IR(25455, 16), 4}, // 574 SourceExtension
  {IR(26466, 13), 45}, // 575 SpecConstant
  {IR(45259, 30), 805}, // 576 SpecConstantArchitectureINTEL
  {IR(45326, 30), 806}, // 577 SpecConstantCapabilitiesINTEL
  {IR(26479, 22), 46}, // 578 SpecConstantComposite
  {IR(44623, 36), 780}, // 579 SpecConstantCompositeContinuedINTEL
  {IR(32643, 34), 394}, // 580 SpecConstantCompositeReplicateEXT
  {IR(26448, 18), 44}, // 581 SpecConstantFalse
  {IR(26542, 15), 47}, // 582 SpecConstantOp
  {IR(33587, 23), 433}, // 583 SpecConstantStringAMDX
  {IR(45235, 24), 804}, // 584 SpecConstantTargetINTEL
  {IR(26431, 17), 43}, // 585 SpecConstantTrue
  {IR(31565, 25), 346}, // 586 StencilAttachmentReadEXT
  {IR(26706, 6), 55}, // 587 Store
  {IR(25487, 7), 7}, // 588 String
  {IR(44924, 25), 793}, // 589 Subgroup2DBlockLoadINTEL
  {IR(44949, 34), 794}, // 590 Subgroup2DBlockLoadTransformINTEL
  {IR(44983, 34), 795}, // 591 Subgroup2DBlockLoadTransposeINTEL
  {IR(45017, 29), 796}, // 592 Subgroup2DBlockPrefetchINTEL
  {IR(45046, 26), 797}, // 593 Subgroup2DBlockStoreINTEL
  {IR(32063, 20), 372}, // 594 SubgroupAllEqualKHR
  {IR(32033, 15), 370}, // 595 SubgroupAllKHR
  {IR(32048, 15), 371}, // 596 SubgroupAnyKHR
  {IR(41006, 30), 660}, // 597 SubgroupAvcBmeInitializeINTEL
  {IR(40976, 30), 659}, // 598 SubgroupAvcFmeInitializeINTEL
  {IR(39411, 35), 629}, // 599 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(39446, 39), 630}, // 600 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(40087, 38), 643}, // 601 SubgroupAvcImeConvertToMceResultINTEL
  {IR(39710, 45), 636}, // 602 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(39810, 53), 638}, // 603 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(40031, 56), 642}, // 604 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(39919, 54), 640}, // 605 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(39663, 47), 635}, // 606 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(39755, 55), 637}, // 607 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(39973, 58), 641}, // 608 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(39863, 56), 639}, // 609 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(40719, 36), 654}, // 610 SubgroupAvcImeGetBorderReachedINTEL
  {IR(40171, 44), 645}, // 611 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(40125, 46), 644}, // 612 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(40586, 66), 652}, // 613 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(40518, 68), 651}, // 614 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(40652, 67), 653}, // 615 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(40381, 68), 649}, // 616 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(40311, 70), 648}, // 617 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(40449, 69), 650}, // 618 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(40755, 48), 655}, // 619 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(40803, 59), 656}, // 620 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(40920, 56), 658}, // 621 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(40862, 58), 657}, // 622 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(39274, 30), 625}, // 623 SubgroupAvcImeInitializeINTEL
  {IR(39378, 33), 628}, // 624 SubgroupAvcImeRefWindowSizeINTEL
  {IR(39342, 36), 627}, // 625 SubgroupAvcImeSetDualReferenceINTEL
  {IR(39575, 54), 633}, // 626 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(39485, 43), 631}, // 627 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(39304, 38), 626}, // 628 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(39528, 47), 632}, // 629 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(39629, 34), 634}, // 630 SubgroupAvcImeSetWeightedSadINTEL
  {IR(40264, 47), 647}, // 631 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(40215, 49), 646}, // 632 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(38664, 39), 610}, // 633 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(38703, 38), 611}, // 634 SubgroupAvcMceConvertToImeResultINTEL
  {IR(38741, 39), 612}, // 635 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(38780, 38), 613}, // 636 SubgroupAvcMceConvertToRefResultINTEL
  {IR(38818, 39), 614}, // 637 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(38857, 38), 615}, // 638 SubgroupAvcMceConvertToSicResultINTEL
  {IR(38970, 43), 618}, // 639 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(38102, 50), 599}, // 640 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(37700, 60), 591}, // 641 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(37900, 51), 595}, // 642 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(38046, 56), 598}, // 643 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(37813, 47), 593}, // 644 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(38401, 56), 605}, // 645 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(38300, 50), 603}, // 646 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(37995, 51), 597}, // 647 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(38204, 49), 601}, // 648 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(38152, 52), 600}, // 649 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(38350, 51), 604}, // 650 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(39089, 38), 621}, // 651 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(38931, 39), 617}, // 652 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(39013, 38), 619}, // 653 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(39051, 38), 620}, // 654 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(39127, 45), 622}, // 655 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(39172, 40), 623}, // 656 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(39212, 62), 624}, // 657 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(38895, 36), 616}, // 658 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(38457, 33), 606}, // 659 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(38603, 61), 609}, // 660 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(37760, 53), 592}, // 661 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(37951, 44), 596}, // 662 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(37860, 40), 594}, // 663 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(38253, 47), 602}, // 664 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(38542, 61), 608}, // 665 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(38490, 52), 607}, // 666 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(41036, 39), 661}, // 667 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(41358, 38), 668}, // 668 SubgroupAvcRefConvertToMceResultINTEL
  {IR(41211, 45), 665}, // 669 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(41256, 46), 666}, // 670 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(41302, 56), 667}, // 671 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41164, 47), 664}, // 672 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(41075, 46), 662}, // 673 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(41121, 43), 663}, // 674 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(41494, 42), 672}, // 675 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(41458, 36), 671}, // 676 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(41426, 32), 670}, // 677 SubgroupAvcSicConfigureSkcINTEL
  {IR(41575, 39), 674}, // 678 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(42115, 38), 686}, // 679 SubgroupAvcSicConvertToMceResultINTEL
  {IR(41890, 31), 681}, // 680 SubgroupAvcSicEvaluateIpeINTEL
  {IR(41968, 45), 683}, // 681 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(42013, 46), 684}, // 682 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(42059, 56), 685}, // 683 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41921, 47), 682}, // 684 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(42232, 46), 689}, // 685 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(42188, 44), 688}, // 686 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(42453, 35), 694}, // 687 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(42319, 36), 691}, // 688 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(42153, 35), 687}, // 689 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(41536, 39), 673}, // 690 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(42278, 41), 690}, // 691 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(42355, 50), 692}, // 692 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(42405, 48), 693}, // 693 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(41396, 30), 669}, // 694 SubgroupAvcSicInitializeINTEL
  {IR(41756, 43), 678}, // 695 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(41847, 43), 680}, // 696 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(41706, 50), 677}, // 697 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(41658, 48), 676}, // 698 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(41614, 44), 675}, // 699 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(41799, 48), 679}, // 700 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(5453, 18), 363}, // 701 SubgroupBallotKHR
  {IR(44897, 27), 792}, // 702 SubgroupBlockPrefetchINTEL
  {IR(36706, 23), 546}, // 703 SubgroupBlockReadINTEL
  {IR(36729, 24), 547}, // 704 SubgroupBlockWriteINTEL
  {IR(31900, 27), 364}, // 705 SubgroupFirstInvocationKHR
  {IR(36753, 28), 548}, // 706 SubgroupImageBlockReadINTEL
  {IR(36781, 29), 549}, // 707 SubgroupImageBlockWriteINTEL
  {IR(36810, 33), 550}, // 708 SubgroupImageMediaBlockReadINTEL
  {IR(36843, 34), 551}, // 709 SubgroupImageMediaBlockWriteINTEL
  {IR(13867, 38), 798}, // 710 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(32083, 26), 374}, // 711 SubgroupReadInvocationKHR
  {IR(36634, 25), 543}, // 712 SubgroupShuffleDownINTEL
  {IR(12075, 21), 542}, // 713 SubgroupShuffleINTEL
  {IR(36659, 23), 544}, // 714 SubgroupShuffleUpINTEL
  {IR(36682, 24), 545}, // 715 SubgroupShuffleXorINTEL
  {IR(29257, 7), 229}, // 716 Switch
  {IR(44806, 23), 788}, // 717 TaskSequenceAsyncINTEL
  {IR(44782, 24), 787}, // 718 TaskSequenceCreateINTEL
  {IR(44829, 21), 789}, // 719 TaskSequenceGetINTEL
  {IR(44850, 25), 790}, // 720 TaskSequenceReleaseINTEL
  {IR(35977, 27), 519}, // 721 TensorLayoutSetBlockSizeNV
  {IR(35841, 28), 512}, // 722 TensorLayoutSetClampValueNV
  {IR(35770, 27), 509}, // 723 TensorLayoutSetDimensionNV
  {IR(35797, 24), 510}, // 724 TensorLayoutSetStrideNV
  {IR(35821, 20), 511}, // 725 TensorLayoutSliceNV
  {IR(31675, 19), 350}, // 726 TensorQuerySizeARM
  {IR(31646, 14), 348}, // 727 TensorReadARM
  {IR(35957, 20), 518}, // 728 TensorViewSetClipNV
  {IR(35888, 25), 514}, // 729 TensorViewSetDimensionNV
  {IR(35913, 22), 515}, // 730 TensorViewSetStrideNV
  {IR(31660, 15), 349}, // 731 TensorWriteARM
  {IR(31787, 20), 358}, // 732 TerminateInvocation
  {IR(32248, 16), 381}, // 733 TerminateRayKHR
  {IR(35036, 15), 486}, // 734 TerminateRayNV
  {IR(35059, 14), 488}, // 735 TraceMotionNV
  {IR(35051, 8), 487}, // 736 TraceNV
  {IR(32160, 12), 377}, // 737 TraceRayKHR
  {IR(35073, 17), 489}, // 738 TraceRayMotionNV
  {IR(27109, 10), 76}, // 739 Transpose
  {IR(35140, 29), 491}, // 740 TypeAccelerationStructureKHR
  {IR(35169, 28), 491}, // 741 TypeAccelerationStructureNV
  {IR(26058, 10), 25}, // 742 TypeArray
  {IR(37619, 37), 588}, // 743 TypeAvcImeDualReferenceStreaminINTEL
  {IR(37354, 23), 579}, // 744 TypeAvcImePayloadINTEL
  {IR(37536, 44), 586}, // 745 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(37468, 22), 584}, // 746 TypeAvcImeResultINTEL
  {IR(37490, 46), 585}, // 747 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(37580, 39), 587}, // 748 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(37423, 23), 582}, // 749 TypeAvcMcePayloadINTEL
  {IR(37446, 22), 583}, // 750 TypeAvcMceResultINTEL
  {IR(37377, 23), 580}, // 751 TypeAvcRefPayloadINTEL
  {IR(37656, 22), 589}, // 752 TypeAvcRefResultINTEL
  {IR(37400, 23), 581}, // 753 TypeAvcSicPayloadINTEL
  {IR(37678, 22), 590}, // 754 TypeAvcSicResultINTEL
  {IR(25820, 9), 17}, // 755 TypeBool
  {IR(44543, 23), 777}, // 756 TypeBufferSurfaceINTEL
  {IR(32429, 25), 388}, // 757 TypeCooperativeMatrixKHR
  {IR(35297, 24), 495}, // 758 TypeCooperativeMatrixNV
  {IR(34562, 24), 470}, // 759 TypeCooperativeVectorNV
  {IR(26173, 16), 32}, // 760 TypeDeviceEvent
  {IR(26163, 10), 31}, // 761 TypeEvent
  {IR(25874, 10), 19}, // 762 TypeFloat
  {IR(26222, 19), 36}, // 763 TypeForwardPointer
  {IR(26150, 13), 30}, // 764 TypeFunction
  {IR(31774, 13), 357}, // 765 TypeGraphARM
  {IR(34523, 16), 468}, // 766 TypeHitObjectNV
  {IR(26019, 10), 22}, // 767 TypeImage
  {IR(25829, 8), 18}, // 768 TypeInt
  {IR(25895, 11), 21}, // 769 TypeMatrix
  {IR(30604, 17), 300}, // 770 TypeNamedBarrier
  {IR(33464, 25), 428}, // 771 TypeNodePayloadArrayAMDX
  {IR(26096, 11), 28}, // 772 TypeOpaque
  {IR(26213, 9), 35}, // 773 TypePipe
  {IR(30482, 16), 295}, // 774 TypePipeStorage
  {IR(26138, 12), 29}, // 775 TypePointer
  {IR(26203, 10), 34}, // 776 TypeQueue
  {IR(32708, 16), 396}, // 777 TypeRayQueryKHR
  {IR(26189, 14), 33}, // 778 TypeReserveId
  {IR(26068, 17), 26}, // 779 TypeRuntimeArray
  {IR(26041, 17), 24}, // 780 TypeSampledImage
  {IR(26029, 12), 23}, // 781 TypeSampler
  {IR(26085, 11), 27}, // 782 TypeStruct
  {IR(44566, 25), 778}, // 783 TypeStructContinuedINTEL
  {IR(44875, 22), 791}, // 784 TypeTaskSequenceINTEL
  {IR(31590, 14), 347}, // 785 TypeTensorARM
  {IR(35713, 19), 506}, // 786 TypeTensorLayoutNV
  {IR(35732, 17), 507}, // 787 TypeTensorViewNV
  {IR(31807, 22), 359}, // 788 TypeUntypedPointerKHR
  {IR(25884, 11), 20}, // 789 TypeVector
  {IR(37336, 18), 578}, // 790 TypeVmeImageINTEL
  {IR(25811, 9), 16}, // 791 TypeVoid
  {IR(36965, 13), 557}, // 792 UAddSatINTEL
  {IR(36992, 14), 559}, // 793 UAverageINTEL
  {IR(37027, 21), 561}, // 794 UAverageRoundedINTEL
  {IR(27626, 9), 103}, // 795 UConvert
  {IR(36877, 24), 552}, // 796 UCountLeadingZerosINTEL
  {IR(36901, 25), 553}, // 797 UCountTrailingZerosINTEL
  {IR(27838, 5), 123}, // 798 UDiv
  {IR(32324, 5), 383}, // 799 UDot
  {IR(32377, 11), 386}, // 800 UDotAccSat
  {IR(32388, 14), 386}, // 801 UDotAccSatKHR
  {IR(32329, 8), 383}, // 802 UDotKHR
  {IR(28198, 13), 160}, // 803 UGreaterThan
  {IR(28224, 18), 162}, // 804 UGreaterThanEqual
  {IR(28260, 10), 164}, // 805 ULessThan
  {IR(28280, 15), 166}, // 806 ULessThanEqual
  {IR(27853, 5), 126}, // 807 UMod
  {IR(37089, 15), 565}, // 808 UMul32x16INTEL
  {IR(28006, 13), 140}, // 809 UMulExtended
  {IR(37061, 13), 563}, // 810 USubSatINTEL
  {IR(25323, 6), 1}, // 811 Undef
  {IR(28103, 10), 151}, // 812 Unordered
  {IR(29288, 12), 233}, // 813 Unreachable
  {IR(31848, 22), 361}, // 814 UntypedAccessChainKHR
  {IR(31985, 22), 367}, // 815 UntypedArrayLengthKHR
  {IR(32135, 25), 376}, // 816 UntypedGroupAsyncCopyKHR
  {IR(31870, 30), 362}, // 817 UntypedInBoundsAccessChainKHR
  {IR(31952, 33), 366}, // 818 UntypedInBoundsPtrAccessChainKHR
  {IR(32007, 19), 368}, // 819 UntypedPrefetchKHR
  {IR(31927, 25), 365}, // 820 UntypedPtrAccessChainKHR
  {IR(31829, 19), 360}, // 821 UntypedVariableKHR
  {IR(13933, 32), 800}, // 822 UntypedVariableLengthArrayINTEL
  {IR(26634, 9), 52}, // 823 Variable
  {IR(12531, 25), 695}, // 824 VariableLengthArrayINTEL
  {IR(26991, 21), 69}, // 825 VectorExtractDynamic
  {IR(27012, 20), 70}, // 826 VectorInsertDynamic
  {IR(27032, 14), 71}, // 827 VectorShuffle
  {IR(27914, 18), 133}, // 828 VectorTimesMatrix
  {IR(27878, 18), 131}, // 829 VectorTimesScalar
  {IR(37322, 14), 577}, // 830 VmeImageINTEL
  {IR(34817, 33), 479}, // 831 WritePackedPrimitiveIndices4x8NV
  {IR(29513, 10), 250}, // 832 WritePipe
  {IR(43855, 23), 757}, // 833 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 494> kExtInstByValue{{
    {0,IR(724, 9),IR(45663, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(724, 9),IR(45671, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(45684, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(45692, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(45706, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(770, 3),IR(45779, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(773, 3),IR(45828, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(776, 2),IR(45883, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(45902, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(45917, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(778, 6),IR(45933, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(45946, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(784, 9),IR(45964, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(793, 9),IR(46016, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(802, 10),IR(46035, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(812, 5),IR(46051, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(46072, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(46093, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(817, 6),IR(46111, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(823, 5),IR(46138, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(828, 5),IR(46173, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(802, 10),IR(46204, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(833, 8),IR(46224, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(841, 11),IR(46249, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(852, 5),IR(46263, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(857, 3),IR(46281, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(860, 2),IR(46312, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(46323, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(862, 3),IR(46336, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(865, 7),IR(46393, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(46412, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(46433, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(46446, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(872, 2),IR(46490, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(706, 1),IR(46505, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(874, 4),IR(46521, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(857, 3),IR(46535, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(46551, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(46557, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(46567, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(46573, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(46578, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(46583, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(46589, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(46595, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(46601, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(46606, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(46612, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(46620, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(46628, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(46632, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(46636, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(46640, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(46645, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(46650, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(46655, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(46660, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(46665, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(46670, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(46676, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(46682, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(46688, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(46694, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(46698, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(46702, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(46706, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(46711, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(46716, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(46721, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(46733, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(46745, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(46759, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(46764, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(46775, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(46780, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(46785, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(46790, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(46795, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(46800, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(46805, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(46812, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(46819, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(46826, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(46831, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(46836, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(46841, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(46852, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(46856, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(46862, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(46874, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(46880, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(46893, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(46906, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(46920, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(46934, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(46947, 15),IR(311, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(46962, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(46978, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(46994, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(47009, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(47024, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(47039, 17),IR(311, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(47056, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(47063, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(47072, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(47078, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(47088, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(47100, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(47108, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(47116, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(47125, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(47134, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(47143, 22),IR(312, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(47165, 20),IR(312, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(47185, 20),IR(312, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(47205, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(47210, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(47215, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(4672, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(878, 5),IR(47222, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(883, 5),IR(47235, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(883, 5),IR(47257, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(554, 7),IR(47273, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(554, 7),IR(47298, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(883, 5),IR(47317, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(883, 5),IR(47341, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(883, 5),IR(47362, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(883, 5),IR(47383, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(883, 5),IR(47399, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(47417, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(47443, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(47468, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(47488, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(47513, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(47543, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(47566, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(47591, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(47617, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(47647, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(47673, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(8097, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(47693, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(47723, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(883, 5),IR(47751, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(554, 7),IR(47779, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(47802, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(47837, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(47875, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(47917, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(525, 6),IR(47962, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(525, 6),IR(47999, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(883, 5),IR(48039, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(883, 5),IR(48066, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(48093, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(48125, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(48165, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(48176, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(48202, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(48234, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(48268, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(45692, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(45706, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(45779, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(45828, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(45883, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(45902, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(45917, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(525, 6),IR(45933, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(375, 3),IR(45946, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(888, 9),IR(45964, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(897, 10),IR(46016, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(907, 9),IR(46035, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(46051, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(46072, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(46093, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(525, 6),IR(46111, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(634, 5),IR(46138, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(916, 5),IR(46173, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(921, 10),IR(46204, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(471, 8),IR(46224, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(921, 10),IR(46249, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(883, 5),IR(46263, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(46281, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(860, 2),IR(46312, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(46323, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(931, 3),IR(46336, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(934, 8),IR(46393, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(46412, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(942, 4),IR(46433, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(942, 4),IR(46446, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(46490, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(706, 1),IR(46505, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(577, 4),IR(46521, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(46535, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(506, 7),IR(48290, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(860, 2),IR(48310, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(48322, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(48346, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(634, 5),IR(48367, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(48377, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(48389, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(48410, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(48427, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(48443, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(724, 9),IR(48459, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(48473, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(48486, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(48498, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(48512, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(946, 15),IR(48535, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(961, 33),IR(48555, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(994, 19),IR(48574, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(45692, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1013, 4),IR(45706, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1017, 3),IR(45779, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1020, 3),IR(45828, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1023, 2),IR(45883, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(45902, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(45917, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(778, 6),IR(45933, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1025, 3),IR(45946, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1028, 9),IR(45964, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1037, 10),IR(46016, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1047, 10),IR(46035, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1057, 5),IR(46051, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(46072, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(46093, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(817, 6),IR(46111, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(823, 5),IR(46138, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(828, 5),IR(46173, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1047, 10),IR(46204, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1062, 8),IR(46224, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1070, 11),IR(46249, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(852, 5),IR(46263, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(857, 3),IR(46281, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(860, 2),IR(46312, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(46323, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(862, 3),IR(46336, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1081, 8),IR(46393, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(46412, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(46433, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(942, 4),IR(46446, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1089, 2),IR(46490, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(706, 1),IR(46505, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(874, 4),IR(46521, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(857, 3),IR(46535, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1091, 7),IR(48290, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(860, 2),IR(48310, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1098, 8),IR(48895, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(48912, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(48917, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(48923, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(48930, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(48935, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(48941, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(48948, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(48953, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(48959, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(48965, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(48972, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(48980, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(48985, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(48990, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(48999, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(49003, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(49008, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(49014, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(49019, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(49023, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(49027, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(49032, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(49038, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(49044, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(49049, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(49054, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(49060, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(49064, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(49069, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(49074, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(49079, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(49085, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(49091, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(49097, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(49103, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(49109, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(49116, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(49125, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(49129, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(49134, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(49140, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(49146, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(49151, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(49155, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(49162, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(49169, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(49174, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(49178, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(49188, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(49192, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(49197, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(49202, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(49212, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(49219, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(49224, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(49230, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(49236, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(49242, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(49246, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(49253, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(49258, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(49264, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(49269, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(49273, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(49278, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(49284, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(49291, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(49297, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(49306, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(49318, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(49327, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(49337, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(49348, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(49357, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(49367, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(49378, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(49388, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(49399, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(49410, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(49419, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(49429, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(49438, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(49449, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(49463, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(49474, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(49486, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(49499, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(49510, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(49522, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(49535, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(49547, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(49560, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(49573, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(49584, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(49596, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(49607, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(49614, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(49622, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(49634, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(49646, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(49650, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(49658, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(49663, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(49674, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(49679, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(49685, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(49694, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(49701, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(49711, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(49725, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(49737, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(49752, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(49758, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(49769, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(49779, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(49789, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(49796, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(49803, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(49811, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(49819, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(49827, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(49835, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(49839, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(49843, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(49852, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(49862, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(49872, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(49878, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(49884, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(49890, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(49896, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(49905, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(49912, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(49922, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(49932, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(49943, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(49954, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(49963, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(49971, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(49979, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(49987, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1106, 3),IR(49995, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(50002, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(50010, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1106, 3),IR(50021, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(50033, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1109, 4),IR(50045, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(50059, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1109, 4),IR(50072, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1106, 3),IR(50087, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(50100, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1109, 4),IR(50114, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(50130, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(50138, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(50147, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(50154, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(50163, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(50173, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(50180, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(50186, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(50197, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(50206, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(50215, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(50232, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(50249, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(50257, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(50279, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(50307, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(50326, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(50335, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(50358, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(50367, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(50376, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(50385, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(50394, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(50403, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(50412, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(50421, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(50430, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(50439, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(506, 7),IR(50446, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(714, 10),IR(50457, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(714, 10),IR(50464, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(714, 10),IR(50471, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(50488, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(50494, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(634, 5),IR(50501, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(50512, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(724, 9),IR(50519, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(50536, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(50542, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(50546, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(50554, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(50559, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(50563, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(50586, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(50598, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(50609, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(50621, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(50628, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(50640, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(50659, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(50679, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(50690, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(50702, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(50710, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(50718, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(50722, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(50726, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(50730, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(50736, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(50740, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(50752, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(50757, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(50761, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(50765, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(50769, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(50775, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(50779, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(50791, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(50798, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(50809, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(50815, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(50819, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(50826, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(50832, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(50840, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(50854, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(50865, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(50876, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(50887, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(50898, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(50913, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(50924, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(50931, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(50935, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(50943, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(50951, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(50957, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(50962, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(50972, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(50979, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(634, 5),IR(50987, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(50994, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(714, 10),IR(50999, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
}};

IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(0, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(3, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(37, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(118, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(160, 44);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(204, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(212, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(249, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(411, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(414, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(418, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(419, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(428, 66);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 494> kExtInstNames{{
    {IR(45663, 8), 0}, // MIN_SAD in arm_motion_engine_100
    {IR(45671, 13), 1}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(45684, 8), 2}, // RAW_SAD in arm_motion_engine_100
    {IR(45706, 21), 4}, // DebugCompilationUnit in debuginfo
    {IR(46433, 13), 31}, // DebugDeclare in debuginfo
    {IR(46505, 16), 34}, // DebugExpression in debuginfo
    {IR(46249, 14), 23}, // DebugFunction in debuginfo
    {IR(46224, 25), 22}, // DebugFunctionDeclaration in debuginfo
    {IR(46204, 20), 21}, // DebugGlobalVariable in debuginfo
    {IR(45692, 14), 3}, // DebugInfoNone in debuginfo
    {IR(46336, 15), 28}, // DebugInlinedAt in debuginfo
    {IR(46412, 21), 30}, // DebugInlinedVariable in debuginfo
    {IR(46263, 18), 24}, // DebugLexicalBlock in debuginfo
    {IR(46281, 31), 25}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(46393, 19), 29}, // DebugLocalVariable in debuginfo
    {IR(46521, 14), 35}, // DebugMacroDef in debuginfo
    {IR(46535, 16), 36}, // DebugMacroUndef in debuginfo
    {IR(46323, 13), 27}, // DebugNoScope in debuginfo
    {IR(46490, 15), 33}, // DebugOperation in debuginfo
    {IR(46312, 11), 26}, // DebugScope in debuginfo
    {IR(45902, 15), 8}, // DebugTypeArray in debuginfo
    {IR(45779, 15), 5}, // DebugTypeBasic in debuginfo
    {IR(46016, 19), 13}, // DebugTypeComposite in debuginfo
    {IR(45964, 14), 12}, // DebugTypeEnum in debuginfo
    {IR(45946, 18), 11}, // DebugTypeFunction in debuginfo
    {IR(46051, 21), 15}, // DebugTypeInheritance in debuginfo
    {IR(46035, 16), 14}, // DebugTypeMember in debuginfo
    {IR(45828, 17), 6}, // DebugTypePointer in debuginfo
    {IR(46072, 21), 16}, // DebugTypePtrToMember in debuginfo
    {IR(45883, 19), 7}, // DebugTypeQualifier in debuginfo
    {IR(46093, 18), 17}, // DebugTypeTemplate in debuginfo
    {IR(46111, 27), 18}, // DebugTypeTemplateParameter in debuginfo
    {IR(46173, 31), 20}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(46138, 35), 19}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(45917, 16), 9}, // DebugTypeVector in debuginfo
    {IR(45933, 13), 10}, // DebugTypedef in debuginfo
    {IR(46446, 11), 32}, // DebugValue in debuginfo
    {IR(46645, 5), 53}, // Acos in glsl_std_450
    {IR(46676, 6), 59}, // Acosh in glsl_std_450
    {IR(46640, 5), 52}, // Asin in glsl_std_450
    {IR(46670, 6), 58}, // Asinh in glsl_std_450
    {IR(46650, 5), 54}, // Atan in glsl_std_450
    {IR(46688, 6), 61}, // Atan2 in glsl_std_450
    {IR(46682, 6), 60}, // Atanh in glsl_std_450
    {IR(46601, 5), 45}, // Ceil in glsl_std_450
    {IR(46632, 4), 50}, // Cos in glsl_std_450
    {IR(46660, 5), 56}, // Cosh in glsl_std_450
    {IR(47072, 6), 104}, // Cross in glsl_std_450
    {IR(46620, 8), 48}, // Degrees in glsl_std_450
    {IR(46733, 12), 69}, // Determinant in glsl_std_450
    {IR(47063, 9), 103}, // Distance in glsl_std_450
    {IR(46698, 4), 63}, // Exp in glsl_std_450
    {IR(46706, 5), 65}, // Exp2 in glsl_std_450
    {IR(46573, 5), 40}, // FAbs in glsl_std_450
    {IR(46805, 7), 79}, // FClamp in glsl_std_450
    {IR(46790, 5), 76}, // FMax in glsl_std_450
    {IR(46775, 5), 73}, // FMin in glsl_std_450
    {IR(46826, 5), 82}, // FMix in glsl_std_450
    {IR(46583, 6), 42}, // FSign in glsl_std_450
    {IR(47088, 12), 106}, // FaceForward in glsl_std_450
    {IR(47116, 9), 109}, // FindILsb in glsl_std_450
    {IR(47125, 9), 110}, // FindSMsb in glsl_std_450
    {IR(47134, 9), 111}, // FindUMsb in glsl_std_450
    {IR(46595, 6), 44}, // Floor in glsl_std_450
    {IR(46852, 4), 86}, // Fma in glsl_std_450
    {IR(46606, 6), 46}, // Fract in glsl_std_450
    {IR(46856, 6), 87}, // Frexp in glsl_std_450
    {IR(46862, 12), 88}, // FrexpStruct in glsl_std_450
    {IR(46831, 5), 83}, // IMix in glsl_std_450
    {IR(47143, 22), 112}, // InterpolateAtCentroid in glsl_std_450
    {IR(47185, 20), 114}, // InterpolateAtOffset in glsl_std_450
    {IR(47165, 20), 113}, // InterpolateAtSample in glsl_std_450
    {IR(46721, 12), 68}, // InverseSqrt in glsl_std_450
    {IR(46874, 6), 89}, // Ldexp in glsl_std_450
    {IR(47056, 7), 102}, // Length in glsl_std_450
    {IR(46702, 4), 64}, // Log in glsl_std_450
    {IR(46711, 5), 66}, // Log2 in glsl_std_450
    {IR(46745, 14), 70}, // MatrixInverse in glsl_std_450
    {IR(46759, 5), 71}, // Modf in glsl_std_450
    {IR(46764, 11), 72}, // ModfStruct in glsl_std_450
    {IR(47215, 7), 117}, // NClamp in glsl_std_450
    {IR(47210, 5), 116}, // NMax in glsl_std_450
    {IR(47205, 5), 115}, // NMin in glsl_std_450
    {IR(47078, 10), 105}, // Normalize in glsl_std_450
    {IR(46947, 15), 95}, // PackDouble2x32 in glsl_std_450
    {IR(46934, 13), 94}, // PackHalf2x16 in glsl_std_450
    {IR(46906, 14), 92}, // PackSnorm2x16 in glsl_std_450
    {IR(46880, 13), 90}, // PackSnorm4x8 in glsl_std_450
    {IR(46920, 14), 93}, // PackUnorm2x16 in glsl_std_450
    {IR(46893, 13), 91}, // PackUnorm4x8 in glsl_std_450
    {IR(46694, 4), 62}, // Pow in glsl_std_450
    {IR(46612, 8), 47}, // Radians in glsl_std_450
    {IR(47100, 8), 107}, // Reflect in glsl_std_450
    {IR(47108, 8), 108}, // Refract in glsl_std_450
    {IR(46551, 6), 37}, // Round in glsl_std_450
    {IR(46557, 10), 38}, // RoundEven in glsl_std_450
    {IR(46578, 5), 41}, // SAbs in glsl_std_450
    {IR(46819, 7), 81}, // SClamp in glsl_std_450
    {IR(46800, 5), 78}, // SMax in glsl_std_450
    {IR(46785, 5), 75}, // SMin in glsl_std_450
    {IR(46589, 6), 43}, // SSign in glsl_std_450
    {IR(46628, 4), 49}, // Sin in glsl_std_450
    {IR(46655, 5), 55}, // Sinh in glsl_std_450
    {IR(46841, 11), 85}, // SmoothStep in glsl_std_450
    {IR(46716, 5), 67}, // Sqrt in glsl_std_450
    {IR(46836, 5), 84}, // Step in glsl_std_450
    {IR(46636, 4), 51}, // Tan in glsl_std_450
    {IR(46665, 5), 57}, // Tanh in glsl_std_450
    {IR(46567, 6), 39}, // Trunc in glsl_std_450
    {IR(46812, 7), 80}, // UClamp in glsl_std_450
    {IR(46795, 5), 77}, // UMax in glsl_std_450
    {IR(46780, 5), 74}, // UMin in glsl_std_450
    {IR(47039, 17), 101}, // UnpackDouble2x32 in glsl_std_450
    {IR(46994, 15), 98}, // UnpackHalf2x16 in glsl_std_450
    {IR(46962, 16), 96}, // UnpackSnorm2x16 in glsl_std_450
    {IR(47009, 15), 99}, // UnpackSnorm4x8 in glsl_std_450
    {IR(46978, 16), 97}, // UnpackUnorm2x16 in glsl_std_450
    {IR(47024, 15), 100}, // UnpackUnorm4x8 in glsl_std_450
    {IR(47222, 13), 119}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(47317, 24), 124}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(47273, 25), 122}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(47298, 19), 123}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(47751, 28), 143}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(47779, 23), 144}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(47341, 21), 125}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(47383, 16), 127}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(47235, 22), 120}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(47362, 21), 126}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(48039, 27), 151}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(47257, 16), 121}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(48066, 27), 152}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(47399, 18), 128}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(48093, 32), 153}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(47647, 26), 138}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(47673, 20), 139}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(47917, 45), 148}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(47999, 40), 150}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(47875, 42), 147}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(47962, 37), 149}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(4672, 7), 118}, // Kernel in nonsemantic_clspvreflection
    {IR(8097, 15), 140}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(48234, 34), 158}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(48202, 32), 157}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(48176, 26), 156}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(48165, 11), 155}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(48125, 40), 154}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(47837, 38), 146}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(47802, 35), 145}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(47693, 30), 141}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(47513, 30), 133}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(47488, 25), 132}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(47543, 23), 134}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(47591, 26), 136}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(47617, 30), 137}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(47566, 25), 135}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(47443, 25), 130}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(47723, 28), 142}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(47468, 20), 131}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(47417, 26), 129}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(48268, 22), 159}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(48389, 21), 200}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(45706, 21), 161}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(46433, 13), 188}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(48427, 16), 202}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(46505, 16), 191}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(46249, 14), 180}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(46224, 25), 179}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(48322, 24), 196}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(46204, 20), 178}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(48290, 20), 194}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(45692, 14), 160}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(46336, 15), 185}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(46412, 21), 187}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(46263, 18), 181}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(46281, 31), 182}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(48367, 10), 198}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(46393, 19), 186}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(46521, 14), 192}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(46535, 16), 193}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(48377, 12), 199}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(46323, 13), 184}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(46490, 15), 190}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(46312, 11), 183}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(48310, 12), 195}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(48346, 21), 197}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(48410, 17), 201}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(45902, 15), 165}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(45779, 15), 162}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(46016, 19), 170}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(45964, 14), 169}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(45946, 18), 168}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(46051, 21), 172}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(48443, 16), 203}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(46035, 16), 171}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(45828, 17), 163}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(46072, 21), 173}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(45883, 19), 164}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(46093, 18), 174}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(46111, 27), 175}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(46173, 31), 177}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(46138, 35), 176}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45917, 16), 166}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(45933, 13), 167}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(46446, 11), 189}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(48459, 14), 204}, // Configuration in nonsemantic_vkspreflection
    {IR(48535, 20), 209}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(48555, 19), 210}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(48574, 21), 211}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(48498, 14), 207}, // PushConstants in nonsemantic_vkspreflection
    {IR(48512, 23), 208}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(48473, 13), 205}, // StartCounter in nonsemantic_vkspreflection
    {IR(48486, 12), 206}, // StopCounter in nonsemantic_vkspreflection
    {IR(45706, 21), 213}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(46433, 13), 240}, // DebugDeclare in opencl_debuginfo_100
    {IR(46505, 16), 243}, // DebugExpression in opencl_debuginfo_100
    {IR(46249, 14), 232}, // DebugFunction in opencl_debuginfo_100
    {IR(46224, 25), 231}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(46204, 20), 230}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(48290, 20), 246}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(45692, 14), 212}, // DebugInfoNone in opencl_debuginfo_100
    {IR(46336, 15), 237}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(46412, 21), 239}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(46263, 18), 233}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(46281, 31), 234}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(46393, 19), 238}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(46521, 14), 244}, // DebugMacroDef in opencl_debuginfo_100
    {IR(46535, 16), 245}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(48895, 17), 248}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(46323, 13), 236}, // DebugNoScope in opencl_debuginfo_100
    {IR(46490, 15), 242}, // DebugOperation in opencl_debuginfo_100
    {IR(46312, 11), 235}, // DebugScope in opencl_debuginfo_100
    {IR(48310, 12), 247}, // DebugSource in opencl_debuginfo_100
    {IR(45902, 15), 217}, // DebugTypeArray in opencl_debuginfo_100
    {IR(45779, 15), 214}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(46016, 19), 222}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(45964, 14), 221}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(45946, 18), 220}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(46051, 21), 224}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(46035, 16), 223}, // DebugTypeMember in opencl_debuginfo_100
    {IR(45828, 17), 215}, // DebugTypePointer in opencl_debuginfo_100
    {IR(46072, 21), 225}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(45883, 19), 216}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(46093, 18), 226}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(46111, 27), 227}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(46173, 31), 229}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(46138, 35), 228}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(45917, 16), 218}, // DebugTypeVector in opencl_debuginfo_100
    {IR(45933, 13), 219}, // DebugTypedef in opencl_debuginfo_100
    {IR(46446, 11), 241}, // DebugValue in opencl_debuginfo_100
    {IR(48912, 5), 249}, // acos in opencl_std_100
    {IR(48917, 6), 250}, // acosh in opencl_std_100
    {IR(48923, 7), 251}, // acospi in opencl_std_100
    {IR(48930, 5), 252}, // asin in opencl_std_100
    {IR(48935, 6), 253}, // asinh in opencl_std_100
    {IR(48941, 7), 254}, // asinpi in opencl_std_100
    {IR(48948, 5), 255}, // atan in opencl_std_100
    {IR(48953, 6), 256}, // atan2 in opencl_std_100
    {IR(48972, 8), 259}, // atan2pi in opencl_std_100
    {IR(48959, 6), 257}, // atanh in opencl_std_100
    {IR(48965, 7), 258}, // atanpi in opencl_std_100
    {IR(50163, 10), 405}, // bitselect in opencl_std_100
    {IR(48980, 5), 260}, // cbrt in opencl_std_100
    {IR(48985, 5), 261}, // ceil in opencl_std_100
    {IR(49835, 4), 370}, // clz in opencl_std_100
    {IR(48990, 9), 262}, // copysign in opencl_std_100
    {IR(48999, 4), 263}, // cos in opencl_std_100
    {IR(49003, 5), 264}, // cosh in opencl_std_100
    {IR(49008, 6), 265}, // cospi in opencl_std_100
    {IR(49679, 6), 353}, // cross in opencl_std_100
    {IR(49839, 4), 371}, // ctz in opencl_std_100
    {IR(49614, 8), 345}, // degrees in opencl_std_100
    {IR(49685, 9), 354}, // distance in opencl_std_100
    {IR(49019, 4), 267}, // erf in opencl_std_100
    {IR(49014, 5), 266}, // erfc in opencl_std_100
    {IR(49023, 4), 268}, // exp in opencl_std_100
    {IR(49032, 6), 270}, // exp10 in opencl_std_100
    {IR(49027, 5), 269}, // exp2 in opencl_std_100
    {IR(49038, 6), 271}, // expm1 in opencl_std_100
    {IR(49044, 5), 272}, // fabs in opencl_std_100
    {IR(49711, 14), 357}, // fast_distance in opencl_std_100
    {IR(49725, 12), 358}, // fast_length in opencl_std_100
    {IR(49737, 15), 359}, // fast_normalize in opencl_std_100
    {IR(49607, 7), 344}, // fclamp in opencl_std_100
    {IR(49049, 5), 273}, // fdim in opencl_std_100
    {IR(49054, 6), 274}, // floor in opencl_std_100
    {IR(49060, 4), 275}, // fma in opencl_std_100
    {IR(49064, 5), 276}, // fmax in opencl_std_100
    {IR(49622, 12), 346}, // fmax_common in opencl_std_100
    {IR(49069, 5), 277}, // fmin in opencl_std_100
    {IR(49634, 12), 347}, // fmin_common in opencl_std_100
    {IR(49074, 5), 278}, // fmod in opencl_std_100
    {IR(49079, 6), 279}, // fract in opencl_std_100
    {IR(49085, 6), 280}, // frexp in opencl_std_100
    {IR(49297, 9), 316}, // half_cos in opencl_std_100
    {IR(49306, 12), 317}, // half_divide in opencl_std_100
    {IR(49318, 9), 318}, // half_exp in opencl_std_100
    {IR(49337, 11), 320}, // half_exp10 in opencl_std_100
    {IR(49327, 10), 319}, // half_exp2 in opencl_std_100
    {IR(49348, 9), 321}, // half_log in opencl_std_100
    {IR(49367, 11), 323}, // half_log10 in opencl_std_100
    {IR(49357, 10), 322}, // half_log2 in opencl_std_100
    {IR(49378, 10), 324}, // half_powr in opencl_std_100
    {IR(49388, 11), 325}, // half_recip in opencl_std_100
    {IR(49399, 11), 326}, // half_rsqrt in opencl_std_100
    {IR(49410, 9), 327}, // half_sin in opencl_std_100
    {IR(49419, 10), 328}, // half_sqrt in opencl_std_100
    {IR(49429, 9), 329}, // half_tan in opencl_std_100
    {IR(49091, 6), 281}, // hypot in opencl_std_100
    {IR(49097, 6), 282}, // ilogb in opencl_std_100
    {IR(49103, 6), 283}, // ldexp in opencl_std_100
    {IR(49694, 7), 355}, // length in opencl_std_100
    {IR(49109, 7), 284}, // lgamma in opencl_std_100
    {IR(49116, 9), 285}, // lgamma_r in opencl_std_100
    {IR(49125, 4), 286}, // log in opencl_std_100
    {IR(49134, 6), 288}, // log10 in opencl_std_100
    {IR(49140, 6), 289}, // log1p in opencl_std_100
    {IR(49129, 5), 287}, // log2 in opencl_std_100
    {IR(49146, 5), 290}, // logb in opencl_std_100
    {IR(49151, 4), 291}, // mad in opencl_std_100
    {IR(49155, 7), 292}, // maxmag in opencl_std_100
    {IR(49162, 7), 293}, // minmag in opencl_std_100
    {IR(49646, 4), 348}, // mix in opencl_std_100
    {IR(49169, 5), 294}, // modf in opencl_std_100
    {IR(49174, 4), 295}, // nan in opencl_std_100
    {IR(49438, 11), 330}, // native_cos in opencl_std_100
    {IR(49449, 14), 331}, // native_divide in opencl_std_100
    {IR(49463, 11), 332}, // native_exp in opencl_std_100
    {IR(49486, 13), 334}, // native_exp10 in opencl_std_100
    {IR(49474, 12), 333}, // native_exp2 in opencl_std_100
    {IR(49499, 11), 335}, // native_log in opencl_std_100
    {IR(49522, 13), 337}, // native_log10 in opencl_std_100
    {IR(49510, 12), 336}, // native_log2 in opencl_std_100
    {IR(49535, 12), 338}, // native_powr in opencl_std_100
    {IR(49547, 13), 339}, // native_recip in opencl_std_100
    {IR(49560, 13), 340}, // native_rsqrt in opencl_std_100
    {IR(49573, 11), 341}, // native_sin in opencl_std_100
    {IR(49584, 12), 342}, // native_sqrt in opencl_std_100
    {IR(49596, 11), 343}, // native_tan in opencl_std_100
    {IR(49178, 10), 296}, // nextafter in opencl_std_100
    {IR(49701, 10), 356}, // normalize in opencl_std_100
    {IR(49954, 9), 385}, // popcount in opencl_std_100
    {IR(49188, 4), 297}, // pow in opencl_std_100
    {IR(49192, 5), 298}, // pown in opencl_std_100
    {IR(49197, 5), 299}, // powr in opencl_std_100
    {IR(50154, 9), 404}, // prefetch in opencl_std_100
    {IR(50147, 7), 403}, // printf in opencl_std_100
    {IR(49650, 8), 349}, // radians in opencl_std_100
    {IR(49202, 10), 300}, // remainder in opencl_std_100
    {IR(49212, 7), 301}, // remquo in opencl_std_100
    {IR(49219, 5), 302}, // rint in opencl_std_100
    {IR(49224, 6), 303}, // rootn in opencl_std_100
    {IR(49905, 7), 380}, // rotate in opencl_std_100
    {IR(49230, 6), 304}, // round in opencl_std_100
    {IR(49236, 6), 305}, // rsqrt in opencl_std_100
    {IR(49752, 6), 360}, // s_abs in opencl_std_100
    {IR(49758, 11), 361}, // s_abs_diff in opencl_std_100
    {IR(49769, 10), 362}, // s_add_sat in opencl_std_100
    {IR(49819, 8), 368}, // s_clamp in opencl_std_100
    {IR(49789, 7), 364}, // s_hadd in opencl_std_100
    {IR(49963, 8), 386}, // s_mad24 in opencl_std_100
    {IR(49843, 9), 372}, // s_mad_hi in opencl_std_100
    {IR(49862, 10), 374}, // s_mad_sat in opencl_std_100
    {IR(49872, 6), 375}, // s_max in opencl_std_100
    {IR(49884, 6), 377}, // s_min in opencl_std_100
    {IR(49979, 8), 388}, // s_mul24 in opencl_std_100
    {IR(49896, 9), 379}, // s_mul_hi in opencl_std_100
    {IR(49803, 8), 366}, // s_rhadd in opencl_std_100
    {IR(49912, 10), 381}, // s_sub_sat in opencl_std_100
    {IR(49943, 11), 384}, // s_upsample in opencl_std_100
    {IR(50173, 7), 406}, // select in opencl_std_100
    {IR(50130, 8), 401}, // shuffle in opencl_std_100
    {IR(50138, 9), 402}, // shuffle2 in opencl_std_100
    {IR(49674, 5), 352}, // sign in opencl_std_100
    {IR(49242, 4), 306}, // sin in opencl_std_100
    {IR(49246, 7), 307}, // sincos in opencl_std_100
    {IR(49253, 5), 308}, // sinh in opencl_std_100
    {IR(49258, 6), 309}, // sinpi in opencl_std_100
    {IR(49663, 11), 351}, // smoothstep in opencl_std_100
    {IR(49264, 5), 310}, // sqrt in opencl_std_100
    {IR(49658, 5), 350}, // step in opencl_std_100
    {IR(49269, 4), 311}, // tan in opencl_std_100
    {IR(49273, 5), 312}, // tanh in opencl_std_100
    {IR(49278, 6), 313}, // tanpi in opencl_std_100
    {IR(49284, 7), 314}, // tgamma in opencl_std_100
    {IR(49291, 6), 315}, // trunc in opencl_std_100
    {IR(50180, 6), 407}, // u_abs in opencl_std_100
    {IR(50186, 11), 408}, // u_abs_diff in opencl_std_100
    {IR(49779, 10), 363}, // u_add_sat in opencl_std_100
    {IR(49827, 8), 369}, // u_clamp in opencl_std_100
    {IR(49796, 7), 365}, // u_hadd in opencl_std_100
    {IR(49971, 8), 387}, // u_mad24 in opencl_std_100
    {IR(50206, 9), 410}, // u_mad_hi in opencl_std_100
    {IR(49852, 10), 373}, // u_mad_sat in opencl_std_100
    {IR(49878, 6), 376}, // u_max in opencl_std_100
    {IR(49890, 6), 378}, // u_min in opencl_std_100
    {IR(49987, 8), 389}, // u_mul24 in opencl_std_100
    {IR(50197, 9), 409}, // u_mul_hi in opencl_std_100
    {IR(49811, 8), 367}, // u_rhadd in opencl_std_100
    {IR(49922, 10), 382}, // u_sub_sat in opencl_std_100
    {IR(49932, 11), 383}, // u_upsample in opencl_std_100
    {IR(50010, 11), 392}, // vload_half in opencl_std_100
    {IR(50021, 12), 393}, // vload_halfn in opencl_std_100
    {IR(50087, 13), 398}, // vloada_halfn in opencl_std_100
    {IR(49995, 7), 390}, // vloadn in opencl_std_100
    {IR(50033, 12), 394}, // vstore_half in opencl_std_100
    {IR(50045, 14), 395}, // vstore_half_r in opencl_std_100
    {IR(50059, 13), 396}, // vstore_halfn in opencl_std_100
    {IR(50072, 15), 397}, // vstore_halfn_r in opencl_std_100
    {IR(50100, 14), 399}, // vstorea_halfn in opencl_std_100
    {IR(50114, 16), 400}, // vstorea_halfn_r in opencl_std_100
    {IR(50002, 8), 391}, // vstoren in opencl_std_100
    {IR(50232, 17), 412}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(50215, 17), 411}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(50249, 8), 413}, // TimeAMD in spv_amd_gcn_shader
    {IR(50326, 9), 417}, // MbcntAMD in spv_amd_shader_ballot
    {IR(50257, 22), 414}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(50279, 28), 415}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(50307, 19), 416}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(50335, 23), 418}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(50385, 9), 422}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50412, 9), 425}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50358, 9), 419}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50403, 9), 424}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50430, 9), 427}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50376, 9), 421}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50394, 9), 423}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50421, 9), 426}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50367, 9), 420}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50736, 4), 459}, // ABS in tosa_001000_1
    {IR(50559, 4), 442}, // ADD in tosa_001000_1
    {IR(50439, 7), 428}, // ARGMAX in tosa_001000_1
    {IR(50563, 23), 443}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(50446, 11), 429}, // AVG_POOL2D in tosa_001000_1
    {IR(50586, 12), 444}, // BITWISE_AND in tosa_001000_1
    {IR(50740, 12), 460}, // BITWISE_NOT in tosa_001000_1
    {IR(50598, 11), 445}, // BITWISE_OR in tosa_001000_1
    {IR(50609, 12), 446}, // BITWISE_XOR in tosa_001000_1
    {IR(50994, 5), 492}, // CAST in tosa_001000_1
    {IR(50752, 5), 461}, // CEIL in tosa_001000_1
    {IR(50536, 6), 438}, // CLAMP in tosa_001000_1
    {IR(50757, 4), 462}, // CLZ in tosa_001000_1
    {IR(50924, 7), 482}, // CONCAT in tosa_001000_1
    {IR(50457, 7), 430}, // CONV2D in tosa_001000_1
    {IR(50464, 7), 431}, // CONV3D in tosa_001000_1
    {IR(50761, 4), 463}, // COS in tosa_001000_1
    {IR(50471, 17), 432}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(50826, 6), 473}, // EQUAL in tosa_001000_1
    {IR(50542, 4), 439}, // ERF in tosa_001000_1
    {IR(50765, 4), 464}, // EXP in tosa_001000_1
    {IR(50488, 6), 433}, // FFT2D in tosa_001000_1
    {IR(50769, 6), 465}, // FLOOR in tosa_001000_1
    {IR(50972, 7), 489}, // GATHER in tosa_001000_1
    {IR(50832, 8), 474}, // GREATER in tosa_001000_1
    {IR(50840, 14), 475}, // GREATER_EQUAL in tosa_001000_1
    {IR(50621, 7), 447}, // INTDIV in tosa_001000_1
    {IR(50775, 4), 466}, // LOG in tosa_001000_1
    {IR(50628, 12), 448}, // LOGICAL_AND in tosa_001000_1
    {IR(50640, 19), 449}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(50779, 12), 467}, // LOGICAL_NOT in tosa_001000_1
    {IR(50679, 11), 451}, // LOGICAL_OR in tosa_001000_1
    {IR(50659, 20), 450}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(50690, 12), 452}, // LOGICAL_XOR in tosa_001000_1
    {IR(50494, 7), 434}, // MATMUL in tosa_001000_1
    {IR(50702, 8), 453}, // MAXIMUM in tosa_001000_1
    {IR(50501, 11), 435}, // MAX_POOL2D in tosa_001000_1
    {IR(50710, 8), 454}, // MINIMUM in tosa_001000_1
    {IR(50718, 4), 455}, // MUL in tosa_001000_1
    {IR(50791, 7), 468}, // NEGATE in tosa_001000_1
    {IR(50931, 4), 483}, // PAD in tosa_001000_1
    {IR(50722, 4), 456}, // POW in tosa_001000_1
    {IR(50798, 11), 469}, // RECIPROCAL in tosa_001000_1
    {IR(50854, 11), 476}, // REDUCE_ALL in tosa_001000_1
    {IR(50865, 11), 477}, // REDUCE_ANY in tosa_001000_1
    {IR(50876, 11), 478}, // REDUCE_MAX in tosa_001000_1
    {IR(50887, 11), 479}, // REDUCE_MIN in tosa_001000_1
    {IR(50898, 15), 480}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(50913, 11), 481}, // REDUCE_SUM in tosa_001000_1
    {IR(50999, 8), 493}, // RESCALE in tosa_001000_1
    {IR(50935, 8), 484}, // RESHAPE in tosa_001000_1
    {IR(50987, 7), 491}, // RESIZE in tosa_001000_1
    {IR(50943, 8), 485}, // REVERSE in tosa_001000_1
    {IR(50512, 7), 436}, // RFFT2D in tosa_001000_1
    {IR(50809, 6), 470}, // RSQRT in tosa_001000_1
    {IR(50979, 8), 490}, // SCATTER in tosa_001000_1
    {IR(50819, 7), 472}, // SELECT in tosa_001000_1
    {IR(50546, 8), 440}, // SIGMOID in tosa_001000_1
    {IR(50815, 4), 471}, // SIN in tosa_001000_1
    {IR(50951, 6), 486}, // SLICE in tosa_001000_1
    {IR(50726, 4), 457}, // SUB in tosa_001000_1
    {IR(50730, 6), 458}, // TABLE in tosa_001000_1
    {IR(50554, 5), 441}, // TANH in tosa_001000_1
    {IR(50957, 5), 487}, // TILE in tosa_001000_1
    {IR(50962, 10), 488}, // TRANSPOSE in tosa_001000_1
    {IR(50519, 17), 437}, // TRANSPOSE_CONV2D in tosa_001000_1
}};

IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(0, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(3, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(37, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(118, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(160, 44);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(204, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(212, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(249, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(411, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(414, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(418, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(419, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(428, 66);
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
  "SPV_ARM_graph\0" // IR(426, 14)
  "SPV_ARM_tensors\0" // IR(440, 16)
  "SPV_EXT_arithmetic_fence\0" // IR(456, 25)
  "SPV_EXT_demote_to_helper_invocation\0" // IR(481, 36)
  "SPV_EXT_descriptor_indexing\0" // IR(517, 28)
  "SPV_EXT_float8\0" // IR(545, 15)
  "SPV_EXT_fragment_fully_covered\0" // IR(560, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(591, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(627, 34)
  "SPV_EXT_mesh_shader\0" // IR(661, 20)
  "SPV_EXT_opacity_micromap\0" // IR(681, 25)
  "SPV_EXT_optnone\0" // IR(706, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(722, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(754, 44)
  "SPV_EXT_replicated_composites\0" // IR(798, 30)
  "SPV_EXT_shader_64bit_indexing\0" // IR(828, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(858, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(892, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(924, 36)
  "SPV_EXT_shader_image_int64\0" // IR(960, 27)
  "SPV_EXT_shader_stencil_export\0" // IR(987, 30)
  "SPV_EXT_shader_tile_image\0" // IR(1017, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1043, 36)
  "SPV_GOOGLE_decorate_string\0" // IR(1079, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1106, 31)
  "SPV_GOOGLE_user_type\0" // IR(1137, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1158, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(1180, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(1222, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(1267, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(1306, 30)
  "SPV_INTEL_bindless_images\0" // IR(1336, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(1362, 25)
  "SPV_INTEL_cache_controls\0" // IR(1387, 25)
  "SPV_INTEL_debug_module\0" // IR(1412, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(1435, 44)
  "SPV_INTEL_float_controls2\0" // IR(1479, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(1505, 28)
  "SPV_INTEL_fp_max_error\0" // IR(1533, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(1556, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(1591, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(1622, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(1656, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(1683, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(1731, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(1762, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(1791, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(1822, 33)
  "SPV_INTEL_fpga_reg\0" // IR(1855, 19)
  "SPV_INTEL_function_pointers\0" // IR(1874, 28)
  "SPV_INTEL_function_variants\0" // IR(1902, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(1930, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(1973, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2011, 26)
  "SPV_INTEL_int4\0" // IR(2037, 15)
  "SPV_INTEL_io_pipes\0" // IR(2052, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2071, 28)
  "SPV_INTEL_long_composites\0" // IR(2099, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2125, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2145, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2177, 28)
  "SPV_INTEL_media_block_io\0" // IR(2205, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2230, 33)
  "SPV_INTEL_optnone\0" // IR(2263, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(2281, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(2307, 36)
  "SPV_INTEL_split_barrier\0" // IR(2343, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(2367, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(2402, 46)
  "SPV_INTEL_subgroups\0" // IR(2448, 20)
  "SPV_INTEL_task_sequence\0" // IR(2468, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(2492, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(2528, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(2563, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(2600, 30)
  "SPV_INTEL_variable_length_array\0" // IR(2630, 32)
  "SPV_INTEL_vector_compute\0" // IR(2662, 25)
  "SPV_KHR_16bit_storage\0" // IR(2687, 22)
  "SPV_KHR_8bit_storage\0" // IR(2709, 21)
  "SPV_KHR_bfloat16\0" // IR(2730, 17)
  "SPV_KHR_bit_instructions\0" // IR(2747, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(2772, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(2807, 27)
  "SPV_KHR_device_group\0" // IR(2834, 21)
  "SPV_KHR_expect_assume\0" // IR(2855, 22)
  "SPV_KHR_float_controls\0" // IR(2877, 23)
  "SPV_KHR_float_controls2\0" // IR(2900, 24)
  "SPV_KHR_fma\0" // IR(2924, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(2936, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(2972, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3002, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3030, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3051, 30)
  "SPV_KHR_multiview\0" // IR(3081, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3099, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3134, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3160, 32)
  "SPV_KHR_post_depth_coverage\0" // IR(3192, 28)
  "SPV_KHR_quad_control\0" // IR(3220, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(3241, 22)
  "SPV_KHR_ray_query\0" // IR(3263, 18)
  "SPV_KHR_ray_tracing\0" // IR(3281, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(3301, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(3336, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(3373, 34)
  "SPV_KHR_shader_ballot\0" // IR(3407, 22)
  "SPV_KHR_shader_clock\0" // IR(3429, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(3450, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(3481, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(3518, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(3542, 38)
  "SPV_KHR_subgroup_vote\0" // IR(3580, 22)
  "SPV_KHR_terminate_invocation\0" // IR(3602, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(3631, 35)
  "SPV_KHR_untyped_pointers\0" // IR(3666, 25)
  "SPV_KHR_variable_pointers\0" // IR(3691, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(3717, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(3745, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(3786, 38)
  "SPV_NV_bindless_texture\0" // IR(3824, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(3848, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(3886, 34)
  "SPV_NV_cooperative_matrix\0" // IR(3920, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(3946, 27)
  "SPV_NV_cooperative_vector\0" // IR(3973, 26)
  "SPV_NV_displacement_micromap\0" // IR(3999, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(4028, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(4063, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(4098, 28)
  "SPV_NV_mesh_shader\0" // IR(4126, 19)
  "SPV_NV_raw_access_chains\0" // IR(4145, 25)
  "SPV_NV_ray_tracing\0" // IR(4170, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(4189, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(4220, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(4257, 33)
  "SPV_NV_shader_image_footprint\0" // IR(4290, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(4320, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(4353, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(4379, 35)
  "SPV_NV_shading_rate\0" // IR(4414, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(4434, 29)
  "SPV_NV_tensor_addressing\0" // IR(4463, 25)
  "SPV_NV_viewport_array2\0" // IR(4488, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(4511, 39)
  "SPV_QCOM_image_processing\0" // IR(4550, 26)
  "SPV_QCOM_image_processing2\0" // IR(4576, 27)
  "SPV_QCOM_tile_shading\0" // IR(4603, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(4625, 38)
  "ReadOnly\0" // IR(4663, 9)
  "Kernel\0" // IR(4672, 7)
  "WriteOnly\0" // IR(4679, 10)
  "ReadWrite\0" // IR(4689, 10)
  "Logical\0" // IR(4699, 8)
  "Physical32\0" // IR(4707, 11)
  "Addresses\0" // IR(4718, 10)
  "Physical64\0" // IR(4728, 11)
  "PhysicalStorageBuffer64\0" // IR(4739, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(4763, 27)
  "PhysicalStorageBufferAddresses\0" // IR(4790, 31)
  "Position\0" // IR(4821, 9)
  "Shader\0" // IR(4830, 7)
  "PointSize\0" // IR(4837, 10)
  "ClipDistance\0" // IR(4847, 13)
  "CullDistance\0" // IR(4860, 13)
  "VertexId\0" // IR(4873, 9)
  "InstanceId\0" // IR(4882, 11)
  "PrimitiveId\0" // IR(4893, 12)
  "Geometry\0" // IR(4905, 9)
  "Tessellation\0" // IR(4914, 13)
  "RayTracingNV\0" // IR(4927, 13)
  "RayTracingKHR\0" // IR(4940, 14)
  "MeshShadingNV\0" // IR(4954, 14)
  "MeshShadingEXT\0" // IR(4968, 15)
  "InvocationId\0" // IR(4983, 13)
  "Layer\0" // IR(4996, 6)
  "ShaderLayer\0" // IR(5002, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(5014, 28)
  "ViewportIndex\0" // IR(5042, 14)
  "MultiViewport\0" // IR(5056, 14)
  "ShaderViewportIndex\0" // IR(5070, 20)
  "TessLevelOuter\0" // IR(5090, 15)
  "TessLevelInner\0" // IR(5105, 15)
  "TessCoord\0" // IR(5120, 10)
  "PatchVertices\0" // IR(5130, 14)
  "FragCoord\0" // IR(5144, 10)
  "PointCoord\0" // IR(5154, 11)
  "FrontFacing\0" // IR(5165, 12)
  "SampleId\0" // IR(5177, 9)
  "SampleRateShading\0" // IR(5186, 18)
  "SamplePosition\0" // IR(5204, 15)
  "SampleMask\0" // IR(5219, 11)
  "FragDepth\0" // IR(5230, 10)
  "HelperInvocation\0" // IR(5240, 17)
  "NumWorkgroups\0" // IR(5257, 14)
  "WorkgroupSize\0" // IR(5271, 14)
  "WorkgroupId\0" // IR(5285, 12)
  "LocalInvocationId\0" // IR(5297, 18)
  "GlobalInvocationId\0" // IR(5315, 19)
  "LocalInvocationIndex\0" // IR(5334, 21)
  "WorkDim\0" // IR(5355, 8)
  "GlobalSize\0" // IR(5363, 11)
  "EnqueuedWorkgroupSize\0" // IR(5374, 22)
  "GlobalOffset\0" // IR(5396, 13)
  "GlobalLinearId\0" // IR(5409, 15)
  "SubgroupSize\0" // IR(5424, 13)
  "GroupNonUniform\0" // IR(5437, 16)
  "SubgroupBallotKHR\0" // IR(5453, 18)
  "SubgroupMaxSize\0" // IR(5471, 16)
  "NumSubgroups\0" // IR(5487, 13)
  "NumEnqueuedSubgroups\0" // IR(5500, 21)
  "SubgroupId\0" // IR(5521, 11)
  "SubgroupLocalInvocationId\0" // IR(5532, 26)
  "VertexIndex\0" // IR(5558, 12)
  "InstanceIndex\0" // IR(5570, 14)
  "CoreIDARM\0" // IR(5584, 10)
  "CoreBuiltinsARM\0" // IR(5594, 16)
  "CoreCountARM\0" // IR(5610, 13)
  "CoreMaxIDARM\0" // IR(5623, 13)
  "WarpIDARM\0" // IR(5636, 10)
  "WarpMaxIDARM\0" // IR(5646, 13)
  "SubgroupEqMask\0" // IR(5659, 15)
  "SubgroupEqMaskKHR\0" // IR(5674, 18)
  "GroupNonUniformBallot\0" // IR(5692, 22)
  "SubgroupGeMask\0" // IR(5714, 15)
  "SubgroupGeMaskKHR\0" // IR(5729, 18)
  "SubgroupGtMask\0" // IR(5747, 15)
  "SubgroupGtMaskKHR\0" // IR(5762, 18)
  "SubgroupLeMask\0" // IR(5780, 15)
  "SubgroupLeMaskKHR\0" // IR(5795, 18)
  "SubgroupLtMask\0" // IR(5813, 15)
  "SubgroupLtMaskKHR\0" // IR(5828, 18)
  "BaseVertex\0" // IR(5846, 11)
  "DrawParameters\0" // IR(5857, 15)
  "BaseInstance\0" // IR(5872, 13)
  "DrawIndex\0" // IR(5885, 10)
  "PrimitiveShadingRateKHR\0" // IR(5895, 24)
  "FragmentShadingRateKHR\0" // IR(5919, 23)
  "DeviceIndex\0" // IR(5942, 12)
  "DeviceGroup\0" // IR(5954, 12)
  "ViewIndex\0" // IR(5966, 10)
  "MultiView\0" // IR(5976, 10)
  "ShadingRateKHR\0" // IR(5986, 15)
  "TileOffsetQCOM\0" // IR(6001, 15)
  "TileShadingQCOM\0" // IR(6016, 16)
  "TileDimensionQCOM\0" // IR(6032, 18)
  "TileApronSizeQCOM\0" // IR(6050, 18)
  "BaryCoordNoPerspAMD\0" // IR(6068, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(6088, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(6116, 26)
  "BaryCoordSmoothAMD\0" // IR(6142, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(6161, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(6188, 25)
  "BaryCoordPullModelAMD\0" // IR(6213, 22)
  "FragStencilRefEXT\0" // IR(6235, 18)
  "StencilExportEXT\0" // IR(6253, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(6270, 29)
  "ShaderEnqueueAMDX\0" // IR(6299, 18)
  "ShaderIndexAMDX\0" // IR(6317, 16)
  "ViewportMaskNV\0" // IR(6333, 15)
  "ShaderViewportMaskNV\0" // IR(6348, 21)
  "SecondaryPositionNV\0" // IR(6369, 20)
  "ShaderStereoViewNV\0" // IR(6389, 19)
  "SecondaryViewportMaskNV\0" // IR(6408, 24)
  "PositionPerViewNV\0" // IR(6432, 18)
  "PerViewAttributesNV\0" // IR(6450, 20)
  "ViewportMaskPerViewNV\0" // IR(6470, 22)
  "FullyCoveredEXT\0" // IR(6492, 16)
  "FragmentFullyCoveredEXT\0" // IR(6508, 24)
  "TaskCountNV\0" // IR(6532, 12)
  "PrimitiveCountNV\0" // IR(6544, 17)
  "PrimitiveIndicesNV\0" // IR(6561, 19)
  "ClipDistancePerViewNV\0" // IR(6580, 22)
  "CullDistancePerViewNV\0" // IR(6602, 22)
  "LayerPerViewNV\0" // IR(6624, 15)
  "MeshViewCountNV\0" // IR(6639, 16)
  "MeshViewIndicesNV\0" // IR(6655, 18)
  "BaryCoordKHR\0" // IR(6673, 13)
  "BaryCoordNV\0" // IR(6686, 12)
  "FragmentBarycentricKHR\0" // IR(6698, 23)
  "BaryCoordNoPerspKHR\0" // IR(6721, 20)
  "BaryCoordNoPerspNV\0" // IR(6741, 19)
  "FragSizeEXT\0" // IR(6760, 12)
  "FragmentSizeNV\0" // IR(6772, 15)
  "FragmentDensityEXT\0" // IR(6787, 19)
  "FragInvocationCountEXT\0" // IR(6806, 23)
  "InvocationsPerPixelNV\0" // IR(6829, 22)
  "PrimitivePointIndicesEXT\0" // IR(6851, 25)
  "PrimitiveLineIndicesEXT\0" // IR(6876, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(6900, 28)
  "CullPrimitiveEXT\0" // IR(6928, 17)
  "LaunchIdKHR\0" // IR(6945, 12)
  "LaunchIdNV\0" // IR(6957, 11)
  "LaunchSizeKHR\0" // IR(6968, 14)
  "LaunchSizeNV\0" // IR(6982, 13)
  "WorldRayOriginKHR\0" // IR(6995, 18)
  "WorldRayOriginNV\0" // IR(7013, 17)
  "WorldRayDirectionKHR\0" // IR(7030, 21)
  "WorldRayDirectionNV\0" // IR(7051, 20)
  "ObjectRayOriginKHR\0" // IR(7071, 19)
  "ObjectRayOriginNV\0" // IR(7090, 18)
  "ObjectRayDirectionKHR\0" // IR(7108, 22)
  "ObjectRayDirectionNV\0" // IR(7130, 21)
  "RayTminKHR\0" // IR(7151, 11)
  "RayTminNV\0" // IR(7162, 10)
  "RayTmaxKHR\0" // IR(7172, 11)
  "RayTmaxNV\0" // IR(7183, 10)
  "InstanceCustomIndexKHR\0" // IR(7193, 23)
  "InstanceCustomIndexNV\0" // IR(7216, 22)
  "ObjectToWorldKHR\0" // IR(7238, 17)
  "ObjectToWorldNV\0" // IR(7255, 16)
  "WorldToObjectKHR\0" // IR(7271, 17)
  "WorldToObjectNV\0" // IR(7288, 16)
  "HitTNV\0" // IR(7304, 7)
  "HitKindKHR\0" // IR(7311, 11)
  "HitKindNV\0" // IR(7322, 10)
  "CurrentRayTimeNV\0" // IR(7332, 17)
  "RayTracingMotionBlurNV\0" // IR(7349, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(7372, 30)
  "RayTracingPositionFetchKHR\0" // IR(7402, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(7429, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(7463, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(7496, 37)
  "IncomingRayFlagsKHR\0" // IR(7533, 20)
  "IncomingRayFlagsNV\0" // IR(7553, 19)
  "RayGeometryIndexKHR\0" // IR(7572, 20)
  "HitIsSphereNV\0" // IR(7592, 14)
  "RayTracingSpheresGeometryNV\0" // IR(7606, 28)
  "HitIsLSSNV\0" // IR(7634, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(7645, 39)
  "HitSpherePositionNV\0" // IR(7684, 20)
  "WarpsPerSMNV\0" // IR(7704, 13)
  "ShaderSMBuiltinsNV\0" // IR(7717, 19)
  "SMCountNV\0" // IR(7736, 10)
  "WarpIDNV\0" // IR(7746, 9)
  "SMIDNV\0" // IR(7755, 7)
  "HitLSSPositionsNV\0" // IR(7762, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(7780, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(7814, 33)
  "HitSphereRadiusNV\0" // IR(7847, 18)
  "HitLSSRadiiNV\0" // IR(7865, 14)
  "ClusterIDNV\0" // IR(7879, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(7891, 41)
  "CullMaskKHR\0" // IR(7932, 12)
  "RayCullMaskKHR\0" // IR(7944, 15)
  "Matrix\0" // IR(7959, 7)
  "Linkage\0" // IR(7966, 8)
  "Vector16\0" // IR(7974, 9)
  "Float16Buffer\0" // IR(7983, 14)
  "Float16\0" // IR(7997, 8)
  "Float64\0" // IR(8005, 8)
  "Int64\0" // IR(8013, 6)
  "Int64Atomics\0" // IR(8019, 13)
  "ImageBasic\0" // IR(8032, 11)
  "ImageReadWrite\0" // IR(8043, 15)
  "ImageMipmap\0" // IR(8058, 12)
  "Pipes\0" // IR(8070, 6)
  "Groups\0" // IR(8076, 7)
  "DeviceEnqueue\0" // IR(8083, 14)
  "LiteralSampler\0" // IR(8097, 15)
  "AtomicStorage\0" // IR(8112, 14)
  "Int16\0" // IR(8126, 6)
  "TessellationPointSize\0" // IR(8132, 22)
  "GeometryPointSize\0" // IR(8154, 18)
  "ImageGatherExtended\0" // IR(8172, 20)
  "StorageImageMultisample\0" // IR(8192, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(8216, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(8250, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(8283, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(8317, 33)
  "ImageCubeArray\0" // IR(8350, 15)
  "SampledCubeArray\0" // IR(8365, 17)
  "ImageRect\0" // IR(8382, 10)
  "SampledRect\0" // IR(8392, 12)
  "GenericPointer\0" // IR(8404, 15)
  "Int8\0" // IR(8419, 5)
  "InputAttachment\0" // IR(8424, 16)
  "SparseResidency\0" // IR(8440, 16)
  "MinLod\0" // IR(8456, 7)
  "Sampled1D\0" // IR(8463, 10)
  "Image1D\0" // IR(8473, 8)
  "SampledBuffer\0" // IR(8481, 14)
  "ImageBuffer\0" // IR(8495, 12)
  "ImageMSArray\0" // IR(8507, 13)
  "StorageImageExtendedFormats\0" // IR(8520, 28)
  "ImageQuery\0" // IR(8548, 11)
  "DerivativeControl\0" // IR(8559, 18)
  "InterpolationFunction\0" // IR(8577, 22)
  "TransformFeedback\0" // IR(8599, 18)
  "GeometryStreams\0" // IR(8617, 16)
  "StorageImageReadWithoutFormat\0" // IR(8633, 30)
  "StorageImageWriteWithoutFormat\0" // IR(8663, 31)
  "SubgroupDispatch\0" // IR(8694, 17)
  "NamedBarrier\0" // IR(8711, 13)
  "PipeStorage\0" // IR(8724, 12)
  "GroupNonUniformVote\0" // IR(8736, 20)
  "GroupNonUniformArithmetic\0" // IR(8756, 26)
  "GroupNonUniformShuffle\0" // IR(8782, 23)
  "GroupNonUniformShuffleRelative\0" // IR(8805, 31)
  "GroupNonUniformClustered\0" // IR(8836, 25)
  "GroupNonUniformQuad\0" // IR(8861, 20)
  "UniformDecoration\0" // IR(8881, 18)
  "TileImageColorReadAccessEXT\0" // IR(8899, 28)
  "TileImageDepthReadAccessEXT\0" // IR(8927, 28)
  "TileImageStencilReadAccessEXT\0" // IR(8955, 30)
  "TensorsARM\0" // IR(8985, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(8996, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(9033, 40)
  "GraphARM\0" // IR(9073, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(9082, 28)
  "Float8EXT\0" // IR(9110, 10)
  "Float8CooperativeMatrixEXT\0" // IR(9120, 27)
  "CooperativeMatrixKHR\0" // IR(9147, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(9168, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(9201, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(9244, 44)
  "SubgroupVoteKHR\0" // IR(9288, 16)
  "StorageBuffer16BitAccess\0" // IR(9304, 25)
  "StorageUniformBufferBlock16\0" // IR(9329, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(9357, 35)
  "StorageUniform16\0" // IR(9392, 17)
  "StoragePushConstant16\0" // IR(9409, 22)
  "StorageInputOutput16\0" // IR(9431, 21)
  "VariablePointersStorageBuffer\0" // IR(9452, 30)
  "VariablePointers\0" // IR(9482, 17)
  "AtomicStorageOps\0" // IR(9499, 17)
  "SampleMaskPostDepthCoverage\0" // IR(9516, 28)
  "StorageBuffer8BitAccess\0" // IR(9544, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(9568, 34)
  "StoragePushConstant8\0" // IR(9602, 21)
  "DenormPreserve\0" // IR(9623, 15)
  "DenormFlushToZero\0" // IR(9638, 18)
  "SignedZeroInfNanPreserve\0" // IR(9656, 25)
  "RoundingModeRTE\0" // IR(9681, 16)
  "RoundingModeRTZ\0" // IR(9697, 16)
  "RayQueryProvisionalKHR\0" // IR(9713, 23)
  "RayQueryKHR\0" // IR(9736, 12)
  "UntypedPointersKHR\0" // IR(9748, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(9767, 32)
  "TextureSampleWeightedQCOM\0" // IR(9799, 26)
  "TextureBoxFilterQCOM\0" // IR(9825, 21)
  "TextureBlockMatchQCOM\0" // IR(9846, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(9868, 32)
  "TextureBlockMatch2QCOM\0" // IR(9900, 23)
  "Float16ImageAMD\0" // IR(9923, 16)
  "ImageGatherBiasLodAMD\0" // IR(9939, 22)
  "FragmentMaskAMD\0" // IR(9961, 16)
  "ImageReadWriteLodAMD\0" // IR(9977, 21)
  "Int64ImageEXT\0" // IR(9998, 14)
  "ShaderClockKHR\0" // IR(10012, 15)
  "QuadControlKHR\0" // IR(10027, 15)
  "Int4TypeINTEL\0" // IR(10042, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(10056, 27)
  "BFloat16TypeKHR\0" // IR(10083, 16)
  "BFloat16DotProductKHR\0" // IR(10099, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(10121, 29)
  "SampleMaskOverrideCoverageNV\0" // IR(10150, 29)
  "GeometryShaderPassthroughNV\0" // IR(10179, 28)
  "ShaderViewportIndexLayerNV\0" // IR(10207, 27)
  "ImageFootprintNV\0" // IR(10234, 17)
  "FragmentBarycentricNV\0" // IR(10251, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(10273, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(10304, 30)
  "ShadingRateNV\0" // IR(10334, 14)
  "GroupNonUniformPartitionedNV\0" // IR(10348, 29)
  "ShaderNonUniform\0" // IR(10377, 17)
  "ShaderNonUniformEXT\0" // IR(10394, 20)
  "RuntimeDescriptorArray\0" // IR(10414, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(10437, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(10463, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(10499, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(10538, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(10577, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(10619, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(10658, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(10700, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(10737, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(10777, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(10813, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(10852, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(10889, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(10929, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(10965, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(11004, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(11043, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(11085, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(11127, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(11172, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(11214, 45)
  "VulkanMemoryModel\0" // IR(11259, 18)
  "VulkanMemoryModelKHR\0" // IR(11277, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(11298, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(11327, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(11359, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(11393, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(11425, 31)
  "RayTracingProvisionalKHR\0" // IR(11456, 25)
  "CooperativeMatrixNV\0" // IR(11481, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(11501, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(11534, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(11572, 32)
  "DemoteToHelperInvocation\0" // IR(11604, 25)
  "DemoteToHelperInvocationEXT\0" // IR(11629, 28)
  "DisplacementMicromapNV\0" // IR(11657, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(11680, 29)
  "ShaderInvocationReorderNV\0" // IR(11709, 26)
  "BindlessTextureNV\0" // IR(11735, 18)
  "RayQueryPositionFetchKHR\0" // IR(11753, 25)
  "CooperativeVectorNV\0" // IR(11778, 20)
  "AtomicFloat16VectorNV\0" // IR(11798, 22)
  "RawAccessChainsNV\0" // IR(11820, 18)
  "Shader64BitIndexingEXT\0" // IR(11838, 23)
  "CooperativeMatrixReductionsNV\0" // IR(11861, 30)
  "CooperativeMatrixConversionsNV\0" // IR(11891, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(11922, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(11962, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(11998, 30)
  "CooperativeVectorTrainingNV\0" // IR(12028, 28)
  "TensorAddressingNV\0" // IR(12056, 19)
  "SubgroupShuffleINTEL\0" // IR(12075, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(12096, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(12123, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(12149, 31)
  "RoundToInfinityINTEL\0" // IR(12180, 21)
  "FloatingPointModeINTEL\0" // IR(12201, 23)
  "IntegerFunctions2INTEL\0" // IR(12224, 23)
  "FunctionPointersINTEL\0" // IR(12247, 22)
  "IndirectReferencesINTEL\0" // IR(12269, 24)
  "AsmINTEL\0" // IR(12293, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(12302, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(12325, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(12348, 23)
  "VectorComputeINTEL\0" // IR(12371, 19)
  "VectorAnyINTEL\0" // IR(12390, 15)
  "ExpectAssumeKHR\0" // IR(12405, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(12421, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(12454, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(12492, 39)
  "VariableLengthArrayINTEL\0" // IR(12531, 25)
  "FunctionFloatControlINTEL\0" // IR(12556, 26)
  "FPGAMemoryAttributesINTEL\0" // IR(12582, 26)
  "FPFastMathModeINTEL\0" // IR(12608, 20)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(12628, 32)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(12660, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(12697, 30)
  "FPGALoopControlsINTEL\0" // IR(12727, 22)
  "KernelAttributesINTEL\0" // IR(12749, 22)
  "FPGAKernelAttributesINTEL\0" // IR(12771, 26)
  "FPGAMemoryAccessesINTEL\0" // IR(12797, 24)
  "FPGAClusterAttributesINTEL\0" // IR(12821, 27)
  "LoopFuseINTEL\0" // IR(12848, 14)
  "FPGADSPControlINTEL\0" // IR(12862, 20)
  "MemoryAccessAliasingINTEL\0" // IR(12882, 26)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(12908, 40)
  "FPGABufferLocationINTEL\0" // IR(12948, 24)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(12972, 34)
  "USMStorageClassesINTEL\0" // IR(13006, 23)
  "RuntimeAlignedAttributeINTEL\0" // IR(13029, 29)
  "IOPipesINTEL\0" // IR(13058, 13)
  "BlockingPipesINTEL\0" // IR(13071, 19)
  "FPGARegINTEL\0" // IR(13090, 13)
  "DotProductInputAll\0" // IR(13103, 19)
  "DotProductInputAllKHR\0" // IR(13122, 22)
  "DotProductInput4x8Bit\0" // IR(13144, 22)
  "DotProductInput4x8BitKHR\0" // IR(13166, 25)
  "DotProductInput4x8BitPacked\0" // IR(13191, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(13219, 31)
  "DotProduct\0" // IR(13250, 11)
  "DotProductKHR\0" // IR(13261, 14)
  "ReplicatedCompositesEXT\0" // IR(13275, 24)
  "BitInstructions\0" // IR(13299, 16)
  "GroupNonUniformRotateKHR\0" // IR(13315, 25)
  "FloatControls2\0" // IR(13340, 15)
  "FMAKHR\0" // IR(13355, 7)
  "AtomicFloat32AddEXT\0" // IR(13362, 20)
  "AtomicFloat64AddEXT\0" // IR(13382, 20)
  "LongCompositesINTEL\0" // IR(13402, 20)
  "OptNoneEXT\0" // IR(13422, 11)
  "OptNoneINTEL\0" // IR(13433, 13)
  "AtomicFloat16AddEXT\0" // IR(13446, 20)
  "DebugInfoModuleINTEL\0" // IR(13466, 21)
  "BFloat16ConversionINTEL\0" // IR(13487, 24)
  "SplitBarrierINTEL\0" // IR(13511, 18)
  "ArithmeticFenceEXT\0" // IR(13529, 19)
  "FPGAClusterAttributesV2INTEL\0" // IR(13548, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(13577, 28)
  "TaskSequenceINTEL\0" // IR(13605, 18)
  "FPMaxErrorINTEL\0" // IR(13623, 16)
  "FPGALatencyControlINTEL\0" // IR(13639, 24)
  "FPGAArgumentInterfacesINTEL\0" // IR(13663, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(13691, 30)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(13721, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(13756, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(13784, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(13807, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(13837, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(13867, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(13905, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(13933, 32)
  "SpecConditionalINTEL\0" // IR(13965, 21)
  "FunctionVariantsINTEL\0" // IR(13986, 22)
  "GroupUniformArithmeticKHR\0" // IR(14008, 26)
  "TensorFloat32RoundingINTEL\0" // IR(14034, 27)
  "MaskedGatherScatterINTEL\0" // IR(14061, 25)
  "CacheControlsINTEL\0" // IR(14086, 19)
  "RegisterLimitsINTEL\0" // IR(14105, 20)
  "BindlessImagesINTEL\0" // IR(14125, 20)
  "Unspecified\0" // IR(14145, 12)
  "Address\0" // IR(14157, 8)
  "Boolean\0" // IR(14165, 8)
  "Float\0" // IR(14173, 6)
  "Signed\0" // IR(14179, 7)
  "SignedChar\0" // IR(14186, 11)
  "Unsigned\0" // IR(14197, 9)
  "UnsignedChar\0" // IR(14206, 13)
  "Class\0" // IR(14219, 6)
  "Structure\0" // IR(14225, 10)
  "Union\0" // IR(14235, 6)
  "ImportedModule\0" // IR(14241, 15)
  "ImportedDeclaration\0" // IR(14256, 20)
  "None\0" // IR(14276, 5)
  "FlagIsProtected\0" // IR(14281, 16)
  "FlagIsPrivate\0" // IR(14297, 14)
  "FlagIsPublic\0" // IR(14311, 13)
  "FlagIsLocal\0" // IR(14324, 12)
  "FlagIsDefinition\0" // IR(14336, 17)
  "FlagFwdDecl\0" // IR(14353, 12)
  "FlagArtificial\0" // IR(14365, 15)
  "FlagExplicit\0" // IR(14380, 13)
  "FlagPrototyped\0" // IR(14393, 15)
  "FlagObjectPointer\0" // IR(14408, 18)
  "FlagStaticMember\0" // IR(14426, 17)
  "FlagIndirectVariable\0" // IR(14443, 21)
  "FlagLValueReference\0" // IR(14464, 20)
  "FlagRValueReference\0" // IR(14484, 20)
  "FlagIsOptimized\0" // IR(14504, 16)
  "FlagIsEnumClass\0" // IR(14520, 16)
  "FlagTypePassByValue\0" // IR(14536, 20)
  "FlagTypePassByReference\0" // IR(14556, 24)
  "Deref\0" // IR(14580, 6)
  "Plus\0" // IR(14586, 5)
  "Minus\0" // IR(14591, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(14597, 33)
  "PlusUconst\0" // IR(14630, 11)
  "BitPiece\0" // IR(14641, 9)
  "Swap\0" // IR(14650, 5)
  "Xderef\0" // IR(14655, 7)
  "StackValue\0" // IR(14662, 11)
  "Constu\0" // IR(14673, 7)
  "Fragment\0" // IR(14680, 9)
  "ConstType\0" // IR(14689, 10)
  "VolatileType\0" // IR(14699, 13)
  "RestrictType\0" // IR(14712, 13)
  "AtomicType\0" // IR(14725, 11)
  "Float16NV\0" // IR(14736, 10)
  "Float32NV\0" // IR(14746, 10)
  "Float64NV\0" // IR(14756, 10)
  "SignedInt8NV\0" // IR(14766, 13)
  "SignedInt16NV\0" // IR(14779, 14)
  "SignedInt32NV\0" // IR(14793, 14)
  "SignedInt64NV\0" // IR(14807, 14)
  "UnsignedInt8NV\0" // IR(14821, 15)
  "UnsignedInt16NV\0" // IR(14836, 16)
  "UnsignedInt32NV\0" // IR(14852, 16)
  "UnsignedInt64NV\0" // IR(14868, 16)
  "SignedInt8PackedNV\0" // IR(14884, 19)
  "UnsignedInt8PackedNV\0" // IR(14903, 21)
  "FloatE4M3NV\0" // IR(14924, 12)
  "FloatE5M2NV\0" // IR(14936, 12)
  "RowMajorKHR\0" // IR(14948, 12)
  "ColumnMajorKHR\0" // IR(14960, 15)
  "RowBlockedInterleavedARM\0" // IR(14975, 25)
  "ColumnBlockedInterleavedARM\0" // IR(15000, 28)
  "NoneKHR\0" // IR(15028, 8)
  "MatrixASignedComponentsKHR\0" // IR(15036, 27)
  "MatrixBSignedComponentsKHR\0" // IR(15063, 27)
  "MatrixCSignedComponentsKHR\0" // IR(15090, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(15117, 32)
  "SaturatingAccumulationKHR\0" // IR(15149, 26)
  "Row\0" // IR(15175, 4)
  "Column\0" // IR(15179, 7)
  "2x2\0" // IR(15186, 4)
  "MatrixAKHR\0" // IR(15190, 11)
  "MatrixBKHR\0" // IR(15201, 11)
  "MatrixAccumulatorKHR\0" // IR(15212, 21)
  "RowMajorNV\0" // IR(15233, 11)
  "ColumnMajorNV\0" // IR(15244, 14)
  "InferencingOptimalNV\0" // IR(15258, 21)
  "TrainingOptimalNV\0" // IR(15279, 18)
  "RelaxedPrecision\0" // IR(15297, 17)
  "SpecId\0" // IR(15314, 7)
  "Block\0" // IR(15321, 6)
  "BufferBlock\0" // IR(15327, 12)
  "RowMajor\0" // IR(15339, 9)
  "ColMajor\0" // IR(15348, 9)
  "ArrayStride\0" // IR(15357, 12)
  "MatrixStride\0" // IR(15369, 13)
  "GLSLShared\0" // IR(15382, 11)
  "GLSLPacked\0" // IR(15393, 11)
  "CPacked\0" // IR(15404, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(15412, 26)
  "BuiltIn\0" // IR(15438, 8)
  "NoPerspective\0" // IR(15446, 14)
  "Flat\0" // IR(15460, 5)
  "Patch\0" // IR(15465, 6)
  "Centroid\0" // IR(15471, 9)
  "Sample\0" // IR(15480, 7)
  "Invariant\0" // IR(15487, 10)
  "Restrict\0" // IR(15497, 9)
  "Aliased\0" // IR(15506, 8)
  "Volatile\0" // IR(15514, 9)
  "Constant\0" // IR(15523, 9)
  "Coherent\0" // IR(15532, 9)
  "NonWritable\0" // IR(15541, 12)
  "NonReadable\0" // IR(15553, 12)
  "Uniform\0" // IR(15565, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(15573, 26)
  "UniformId\0" // IR(15599, 10)
  "SaturatedConversion\0" // IR(15609, 20)
  "Stream\0" // IR(15629, 7)
  "Location\0" // IR(15636, 9)
  "Component\0" // IR(15645, 10)
  "Index\0" // IR(15655, 6)
  "Binding\0" // IR(15661, 8)
  "DescriptorSet\0" // IR(15669, 14)
  "Offset\0" // IR(15683, 7)
  "XfbBuffer\0" // IR(15690, 10)
  "XfbStride\0" // IR(15700, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(15710, 46)
  "FuncParamAttr\0" // IR(15756, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(15770, 34)
  "FPRoundingMode\0" // IR(15804, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(15819, 35)
  "FPFastMathMode\0" // IR(15854, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(15869, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(15901, 30)
  "LinkageAttributes\0" // IR(15931, 18)
  "NoContraction\0" // IR(15949, 14)
  "InputAttachmentIndex\0" // IR(15963, 21)
  "Alignment\0" // IR(15984, 10)
  "MaxByteOffset\0" // IR(15994, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(16008, 20)
  "AlignmentId\0" // IR(16028, 12)
  "MaxByteOffsetId\0" // IR(16040, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(16056, 44)
  "NoSignedWrap\0" // IR(16100, 13)
  "NoUnsignedWrap\0" // IR(16113, 15)
  "WeightTextureQCOM\0" // IR(16128, 18)
  "BlockMatchTextureQCOM\0" // IR(16146, 22)
  "BlockMatchSamplerQCOM\0" // IR(16168, 22)
  "ExplicitInterpAMD\0" // IR(16190, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(16208, 32)
  "NodeMaxPayloadsAMDX\0" // IR(16240, 20)
  "TrackFinishWritingAMDX\0" // IR(16260, 23)
  "PayloadNodeNameAMDX\0" // IR(16283, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(16303, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(16328, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(16355, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(16380, 28)
  "OverrideCoverageNV\0" // IR(16408, 19)
  "PassthroughNV\0" // IR(16427, 14)
  "ViewportRelativeNV\0" // IR(16441, 19)
  "SecondaryViewportRelativeNV\0" // IR(16460, 28)
  "PerPrimitiveEXT\0" // IR(16488, 16)
  "PerPrimitiveNV\0" // IR(16504, 15)
  "PerViewNV\0" // IR(16519, 10)
  "PerTaskNV\0" // IR(16529, 10)
  "PerVertexKHR\0" // IR(16539, 13)
  "PerVertexNV\0" // IR(16552, 12)
  "NonUniform\0" // IR(16564, 11)
  "NonUniformEXT\0" // IR(16575, 14)
  "RestrictPointer\0" // IR(16589, 16)
  "RestrictPointerEXT\0" // IR(16605, 19)
  "AliasedPointer\0" // IR(16624, 15)
  "AliasedPointerEXT\0" // IR(16639, 18)
  "HitObjectShaderRecordBufferNV\0" // IR(16657, 30)
  "BindlessSamplerNV\0" // IR(16687, 18)
  "BindlessImageNV\0" // IR(16705, 16)
  "BoundSamplerNV\0" // IR(16721, 15)
  "BoundImageNV\0" // IR(16736, 13)
  "SIMTCallINTEL\0" // IR(16749, 14)
  "ReferencedIndirectlyINTEL\0" // IR(16763, 26)
  "ClobberINTEL\0" // IR(16789, 13)
  "SideEffectsINTEL\0" // IR(16802, 17)
  "VectorComputeVariableINTEL\0" // IR(16819, 27)
  "FuncParamIOKindINTEL\0" // IR(16846, 21)
  "VectorComputeFunctionINTEL\0" // IR(16867, 27)
  "StackCallINTEL\0" // IR(16894, 15)
  "GlobalVariableOffsetINTEL\0" // IR(16909, 26)
  "CounterBuffer\0" // IR(16935, 14)
  "HlslCounterBufferGOOGLE\0" // IR(16949, 24)
  "UserSemantic\0" // IR(16973, 13)
  "HlslSemanticGOOGLE\0" // IR(16986, 19)
  "UserTypeGOOGLE\0" // IR(17005, 15)
  "FunctionRoundingModeINTEL\0" // IR(17020, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(17046, 31)
  "FunctionDenormModeINTEL\0" // IR(17077, 24)
  "RegisterINTEL\0" // IR(17101, 14)
  "MemoryINTEL\0" // IR(17115, 12)
  "NumbanksINTEL\0" // IR(17127, 14)
  "BankwidthINTEL\0" // IR(17141, 15)
  "MaxPrivateCopiesINTEL\0" // IR(17156, 22)
  "SinglepumpINTEL\0" // IR(17178, 16)
  "DoublepumpINTEL\0" // IR(17194, 16)
  "MaxReplicatesINTEL\0" // IR(17210, 19)
  "SimpleDualPortINTEL\0" // IR(17229, 20)
  "MergeINTEL\0" // IR(17249, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(17260, 42)
  "BankBitsINTEL\0" // IR(17302, 14)
  "ForcePow2DepthINTEL\0" // IR(17316, 20)
  "StridesizeINTEL\0" // IR(17336, 16)
  "WordsizeINTEL\0" // IR(17352, 14)
  "TrueDualPortINTEL\0" // IR(17366, 18)
  "BurstCoalesceINTEL\0" // IR(17384, 19)
  "CacheSizeINTEL\0" // IR(17403, 15)
  "DontStaticallyCoalesceINTEL\0" // IR(17418, 28)
  "PrefetchINTEL\0" // IR(17446, 14)
  "StallEnableINTEL\0" // IR(17460, 17)
  "FuseLoopsInFunctionINTEL\0" // IR(17477, 25)
  "MathOpDSPModeINTEL\0" // IR(17502, 19)
  "AliasScopeINTEL\0" // IR(17521, 16)
  "NoAliasINTEL\0" // IR(17537, 13)
  "InitiationIntervalINTEL\0" // IR(17550, 24)
  "MaxConcurrencyINTEL\0" // IR(17574, 20)
  "PipelineEnableINTEL\0" // IR(17594, 20)
  "BufferLocationINTEL\0" // IR(17614, 20)
  "IOPipeStorageINTEL\0" // IR(17634, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(17653, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(17687, 31)
  "SingleElementVectorINTEL\0" // IR(17718, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(17743, 35)
  "MediaBlockIOINTEL\0" // IR(17778, 18)
  "StallFreeINTEL\0" // IR(17796, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(17811, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(17842, 26)
  "LatencyControlLabelINTEL\0" // IR(17868, 25)
  "LatencyControlConstraintINTEL\0" // IR(17893, 30)
  "ConduitKernelArgumentINTEL\0" // IR(17923, 27)
  "RegisterMapKernelArgumentINTEL\0" // IR(17950, 31)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(17981, 33)
  "MMHostInterfaceDataWidthINTEL\0" // IR(18014, 30)
  "MMHostInterfaceLatencyINTEL\0" // IR(18044, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(18072, 34)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(18106, 34)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(18140, 29)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(18169, 32)
  "StableKernelArgumentINTEL\0" // IR(18201, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(18227, 39)
  "HostAccessINTEL\0" // IR(18266, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(18282, 47)
  "InitModeINTEL\0" // IR(18329, 14)
  "ImplementInRegisterMapINTEL\0" // IR(18343, 28)
  "ConditionalINTEL\0" // IR(18371, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(18388, 36)
  "CacheControlLoadINTEL\0" // IR(18424, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(18446, 37)
  "CacheControlStoreINTEL\0" // IR(18483, 23)
  "1D\0" // IR(18506, 3)
  "2D\0" // IR(18509, 3)
  "3D\0" // IR(18512, 3)
  "Cube\0" // IR(18515, 5)
  "Rect\0" // IR(18520, 5)
  "Buffer\0" // IR(18525, 7)
  "SubpassData\0" // IR(18532, 12)
  "TileImageDataEXT\0" // IR(18544, 17)
  "Invocations\0" // IR(18561, 12)
  "SpacingEqual\0" // IR(18573, 13)
  "SpacingFractionalEven\0" // IR(18586, 22)
  "SpacingFractionalOdd\0" // IR(18608, 21)
  "VertexOrderCw\0" // IR(18629, 14)
  "VertexOrderCcw\0" // IR(18643, 15)
  "PixelCenterInteger\0" // IR(18658, 19)
  "OriginUpperLeft\0" // IR(18677, 16)
  "OriginLowerLeft\0" // IR(18693, 16)
  "EarlyFragmentTests\0" // IR(18709, 19)
  "PointMode\0" // IR(18728, 10)
  "Xfb\0" // IR(18738, 4)
  "DepthReplacing\0" // IR(18742, 15)
  "DepthGreater\0" // IR(18757, 13)
  "DepthLess\0" // IR(18770, 10)
  "DepthUnchanged\0" // IR(18780, 15)
  "LocalSize\0" // IR(18795, 10)
  "LocalSizeHint\0" // IR(18805, 14)
  "InputPoints\0" // IR(18819, 12)
  "InputLines\0" // IR(18831, 11)
  "InputLinesAdjacency\0" // IR(18842, 20)
  "Triangles\0" // IR(18862, 10)
  "InputTrianglesAdjacency\0" // IR(18872, 24)
  "Quads\0" // IR(18896, 6)
  "Isolines\0" // IR(18902, 9)
  "OutputVertices\0" // IR(18911, 15)
  "OutputPoints\0" // IR(18926, 13)
  "OutputLineStrip\0" // IR(18939, 16)
  "OutputTriangleStrip\0" // IR(18955, 20)
  "VecTypeHint\0" // IR(18975, 12)
  "ContractionOff\0" // IR(18987, 15)
  "Initializer\0" // IR(19002, 12)
  "Finalizer\0" // IR(19014, 10)
  "SubgroupsPerWorkgroup\0" // IR(19024, 22)
  "SubgroupsPerWorkgroupId\0" // IR(19046, 24)
  "LocalSizeId\0" // IR(19070, 12)
  "LocalSizeHintId\0" // IR(19082, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(19098, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(19132, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(19166, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(19202, 30)
  "PostDepthCoverage\0" // IR(19232, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(19250, 34)
  "TileShadingRateQCOM\0" // IR(19284, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(19304, 29)
  "StencilRefReplacingEXT\0" // IR(19333, 23)
  "CoalescingAMDX\0" // IR(19356, 15)
  "IsApiEntryAMDX\0" // IR(19371, 15)
  "MaxNodeRecursionAMDX\0" // IR(19386, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(19407, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(19431, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(19452, 28)
  "StencilRefGreaterFrontAMD\0" // IR(19480, 26)
  "StencilRefLessFrontAMD\0" // IR(19506, 23)
  "StencilRefUnchangedBackAMD\0" // IR(19529, 27)
  "StencilRefGreaterBackAMD\0" // IR(19556, 25)
  "StencilRefLessBackAMD\0" // IR(19581, 22)
  "QuadDerivativesKHR\0" // IR(19603, 19)
  "RequireFullQuadsKHR\0" // IR(19622, 20)
  "SharesInputWithAMDX\0" // IR(19642, 20)
  "OutputLinesEXT\0" // IR(19662, 15)
  "OutputLinesNV\0" // IR(19677, 14)
  "OutputPrimitivesEXT\0" // IR(19691, 20)
  "OutputPrimitivesNV\0" // IR(19711, 19)
  "DerivativeGroupQuadsKHR\0" // IR(19730, 24)
  "DerivativeGroupQuadsNV\0" // IR(19754, 23)
  "DerivativeGroupLinearKHR\0" // IR(19777, 25)
  "DerivativeGroupLinearNV\0" // IR(19802, 24)
  "OutputTrianglesEXT\0" // IR(19826, 19)
  "OutputTrianglesNV\0" // IR(19845, 18)
  "PixelInterlockOrderedEXT\0" // IR(19863, 25)
  "PixelInterlockUnorderedEXT\0" // IR(19888, 27)
  "SampleInterlockOrderedEXT\0" // IR(19915, 26)
  "SampleInterlockUnorderedEXT\0" // IR(19941, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(19969, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(20000, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(20033, 27)
  "RoundingModeRTPINTEL\0" // IR(20060, 21)
  "RoundingModeRTNINTEL\0" // IR(20081, 21)
  "FloatingPointModeALTINTEL\0" // IR(20102, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(20128, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(20155, 22)
  "MaxWorkDimINTEL\0" // IR(20177, 16)
  "NoGlobalOffsetINTEL\0" // IR(20193, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(20213, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(20235, 28)
  "MaximallyReconvergesKHR\0" // IR(20263, 24)
  "FPFastMathDefault\0" // IR(20287, 18)
  "StreamingInterfaceINTEL\0" // IR(20305, 24)
  "RegisterMapInterfaceINTEL\0" // IR(20329, 26)
  "NamedBarrierCountINTEL\0" // IR(20355, 23)
  "MaximumRegistersINTEL\0" // IR(20378, 22)
  "MaximumRegistersIdINTEL\0" // IR(20400, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(20424, 51)
  "NamedMaximumRegistersINTEL\0" // IR(20475, 27)
  "Vertex\0" // IR(20502, 7)
  "TessellationControl\0" // IR(20509, 20)
  "TessellationEvaluation\0" // IR(20529, 23)
  "GLCompute\0" // IR(20552, 10)
  "TaskNV\0" // IR(20562, 7)
  "MeshNV\0" // IR(20569, 7)
  "RayGenerationKHR\0" // IR(20576, 17)
  "RayGenerationNV\0" // IR(20593, 16)
  "IntersectionKHR\0" // IR(20609, 16)
  "IntersectionNV\0" // IR(20625, 15)
  "AnyHitKHR\0" // IR(20640, 10)
  "AnyHitNV\0" // IR(20650, 9)
  "ClosestHitKHR\0" // IR(20659, 14)
  "ClosestHitNV\0" // IR(20673, 13)
  "MissKHR\0" // IR(20686, 8)
  "MissNV\0" // IR(20694, 7)
  "CallableKHR\0" // IR(20701, 12)
  "CallableNV\0" // IR(20713, 11)
  "TaskEXT\0" // IR(20724, 8)
  "MeshEXT\0" // IR(20732, 8)
  "Preserve\0" // IR(20740, 9)
  "FlushToZero\0" // IR(20749, 12)
  "BFloat16KHR\0" // IR(20761, 12)
  "Float8E4M3EXT\0" // IR(20773, 14)
  "Float8E5M2EXT\0" // IR(20787, 14)
  "IEEE\0" // IR(20801, 5)
  "ALT\0" // IR(20806, 4)
  "NotNaN\0" // IR(20810, 7)
  "NotInf\0" // IR(20817, 7)
  "NSZ\0" // IR(20824, 4)
  "AllowRecip\0" // IR(20828, 11)
  "Fast\0" // IR(20839, 5)
  "AllowContract\0" // IR(20844, 14)
  "AllowContractFastINTEL\0" // IR(20858, 23)
  "AllowReassoc\0" // IR(20881, 13)
  "AllowReassocINTEL\0" // IR(20894, 18)
  "AllowTransform\0" // IR(20912, 15)
  "RTE\0" // IR(20927, 4)
  "RTZ\0" // IR(20931, 4)
  "RTP\0" // IR(20935, 4)
  "RTN\0" // IR(20939, 4)
  "Vertical2Pixels\0" // IR(20943, 16)
  "Vertical4Pixels\0" // IR(20959, 16)
  "Horizontal2Pixels\0" // IR(20975, 18)
  "Horizontal4Pixels\0" // IR(20993, 18)
  "Inline\0" // IR(21011, 7)
  "DontInline\0" // IR(21018, 11)
  "Pure\0" // IR(21029, 5)
  "Const\0" // IR(21034, 6)
  "Zext\0" // IR(21040, 5)
  "Sext\0" // IR(21045, 5)
  "ByVal\0" // IR(21050, 6)
  "Sret\0" // IR(21056, 5)
  "NoAlias\0" // IR(21061, 8)
  "NoCapture\0" // IR(21069, 10)
  "NoWrite\0" // IR(21079, 8)
  "NoReadWrite\0" // IR(21087, 12)
  "RuntimeAlignedINTEL\0" // IR(21099, 20)
  "Reduce\0" // IR(21119, 7)
  "InclusiveScan\0" // IR(21126, 14)
  "ExclusiveScan\0" // IR(21140, 14)
  "ClusteredReduce\0" // IR(21154, 16)
  "PartitionedReduceNV\0" // IR(21170, 20)
  "PartitionedInclusiveScanNV\0" // IR(21190, 27)
  "PartitionedExclusiveScanNV\0" // IR(21217, 27)
  "NoneINTEL\0" // IR(21244, 10)
  "ReadINTEL\0" // IR(21254, 10)
  "WriteINTEL\0" // IR(21264, 11)
  "ReadWriteINTEL\0" // IR(21275, 15)
  "Bias\0" // IR(21290, 5)
  "Lod\0" // IR(21295, 4)
  "Grad\0" // IR(21299, 5)
  "ConstOffset\0" // IR(21304, 12)
  "ConstOffsets\0" // IR(21316, 13)
  "MakeTexelAvailable\0" // IR(21329, 19)
  "MakeTexelAvailableKHR\0" // IR(21348, 22)
  "MakeTexelVisible\0" // IR(21370, 17)
  "MakeTexelVisibleKHR\0" // IR(21387, 20)
  "NonPrivateTexel\0" // IR(21407, 16)
  "NonPrivateTexelKHR\0" // IR(21423, 19)
  "VolatileTexel\0" // IR(21442, 14)
  "VolatileTexelKHR\0" // IR(21456, 17)
  "SignExtend\0" // IR(21473, 11)
  "ZeroExtend\0" // IR(21484, 11)
  "Nontemporal\0" // IR(21495, 12)
  "Offsets\0" // IR(21507, 8)
  "SnormInt8\0" // IR(21515, 10)
  "SnormInt16\0" // IR(21525, 11)
  "UnormInt8\0" // IR(21536, 10)
  "UnormInt16\0" // IR(21546, 11)
  "UnormShort565\0" // IR(21557, 14)
  "UnormShort555\0" // IR(21571, 14)
  "UnormInt101010\0" // IR(21585, 15)
  "SignedInt8\0" // IR(21600, 11)
  "SignedInt16\0" // IR(21611, 12)
  "SignedInt32\0" // IR(21623, 12)
  "UnsignedInt8\0" // IR(21635, 13)
  "UnsignedInt16\0" // IR(21648, 14)
  "UnsignedInt32\0" // IR(21662, 14)
  "HalfFloat\0" // IR(21676, 10)
  "UnormInt24\0" // IR(21686, 11)
  "UnormInt101010_2\0" // IR(21697, 17)
  "UnormInt10X6EXT\0" // IR(21714, 16)
  "UnsignedIntRaw10EXT\0" // IR(21730, 20)
  "UnsignedIntRaw12EXT\0" // IR(21750, 20)
  "UnormInt2_101010EXT\0" // IR(21770, 20)
  "UnsignedInt10X6EXT\0" // IR(21790, 19)
  "UnsignedInt12X4EXT\0" // IR(21809, 19)
  "UnsignedInt14X2EXT\0" // IR(21828, 19)
  "UnormInt12X4EXT\0" // IR(21847, 16)
  "UnormInt14X2EXT\0" // IR(21863, 16)
  "R\0" // IR(21879, 2)
  "A\0" // IR(21881, 2)
  "RG\0" // IR(21883, 3)
  "RA\0" // IR(21886, 3)
  "RGB\0" // IR(21889, 4)
  "RGBA\0" // IR(21893, 5)
  "BGRA\0" // IR(21898, 5)
  "ARGB\0" // IR(21903, 5)
  "Intensity\0" // IR(21908, 10)
  "Luminance\0" // IR(21918, 10)
  "Rx\0" // IR(21928, 3)
  "RGx\0" // IR(21931, 4)
  "RGBx\0" // IR(21935, 5)
  "Depth\0" // IR(21940, 6)
  "DepthStencil\0" // IR(21946, 13)
  "sRGB\0" // IR(21959, 5)
  "sRGBx\0" // IR(21964, 6)
  "sRGBA\0" // IR(21970, 6)
  "sBGRA\0" // IR(21976, 6)
  "ABGR\0" // IR(21982, 5)
  "InitOnDeviceReprogramINTEL\0" // IR(21987, 27)
  "InitOnDeviceResetINTEL\0" // IR(22014, 23)
  "NoWait\0" // IR(22037, 7)
  "WaitKernel\0" // IR(22044, 11)
  "WaitWorkGroup\0" // IR(22055, 14)
  "CmdExecTime\0" // IR(22069, 12)
  "MayUsePrintf\0" // IR(22081, 13)
  "Export\0" // IR(22094, 7)
  "Import\0" // IR(22101, 7)
  "LinkOnceODR\0" // IR(22108, 12)
  "UncachedINTEL\0" // IR(22120, 14)
  "CachedINTEL\0" // IR(22134, 12)
  "StreamingINTEL\0" // IR(22146, 15)
  "InvalidateAfterReadINTEL\0" // IR(22161, 25)
  "ConstCachedINTEL\0" // IR(22186, 17)
  "Unroll\0" // IR(22203, 7)
  "DontUnroll\0" // IR(22210, 11)
  "DependencyInfinite\0" // IR(22221, 19)
  "DependencyLength\0" // IR(22240, 17)
  "MinIterations\0" // IR(22257, 14)
  "MaxIterations\0" // IR(22271, 14)
  "IterationMultiple\0" // IR(22285, 18)
  "PeelCount\0" // IR(22303, 10)
  "PartialCount\0" // IR(22313, 13)
  "DependencyArrayINTEL\0" // IR(22326, 21)
  "LoopCoalesceINTEL\0" // IR(22347, 18)
  "MaxInterleavingINTEL\0" // IR(22365, 21)
  "SpeculatedIterationsINTEL\0" // IR(22386, 26)
  "NoFusionINTEL\0" // IR(22412, 14)
  "LoopCountINTEL\0" // IR(22426, 15)
  "MaxReinvocationDelayINTEL\0" // IR(22441, 26)
  "MatrixASignedComponentsINTEL\0" // IR(22467, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(22496, 29)
  "MatrixCBFloat16INTEL\0" // IR(22525, 21)
  "MatrixResultBFloat16INTEL\0" // IR(22546, 26)
  "MatrixAPackedInt8INTEL\0" // IR(22572, 23)
  "MatrixBPackedInt8INTEL\0" // IR(22595, 23)
  "MatrixAPackedInt4INTEL\0" // IR(22618, 23)
  "MatrixBPackedInt4INTEL\0" // IR(22641, 23)
  "MatrixATF32INTEL\0" // IR(22664, 17)
  "MatrixBTF32INTEL\0" // IR(22681, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(22698, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(22724, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(22750, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(22777, 27)
  "Aligned\0" // IR(22804, 8)
  "MakePointerAvailable\0" // IR(22812, 21)
  "MakePointerAvailableKHR\0" // IR(22833, 24)
  "MakePointerVisible\0" // IR(22857, 19)
  "MakePointerVisibleKHR\0" // IR(22876, 22)
  "NonPrivatePointer\0" // IR(22898, 18)
  "NonPrivatePointerKHR\0" // IR(22916, 21)
  "AliasScopeINTELMask\0" // IR(22937, 20)
  "NoAliasINTELMask\0" // IR(22957, 17)
  "Simple\0" // IR(22974, 7)
  "GLSL450\0" // IR(22981, 8)
  "OpenCL\0" // IR(22989, 7)
  "Vulkan\0" // IR(22996, 7)
  "VulkanKHR\0" // IR(23003, 10)
  "Relaxed\0" // IR(23013, 8)
  "Acquire\0" // IR(23021, 8)
  "Release\0" // IR(23029, 8)
  "AcquireRelease\0" // IR(23037, 15)
  "SequentiallyConsistent\0" // IR(23052, 23)
  "UniformMemory\0" // IR(23075, 14)
  "SubgroupMemory\0" // IR(23089, 15)
  "WorkgroupMemory\0" // IR(23104, 16)
  "CrossWorkgroupMemory\0" // IR(23120, 21)
  "AtomicCounterMemory\0" // IR(23141, 20)
  "ImageMemory\0" // IR(23161, 12)
  "OutputMemory\0" // IR(23173, 13)
  "OutputMemoryKHR\0" // IR(23186, 16)
  "MakeAvailable\0" // IR(23202, 14)
  "MakeAvailableKHR\0" // IR(23216, 17)
  "MakeVisible\0" // IR(23233, 12)
  "MakeVisibleKHR\0" // IR(23245, 15)
  "AutoINTEL\0" // IR(23260, 10)
  "WRAP\0" // IR(23270, 5)
  "SAT\0" // IR(23275, 4)
  "SAT_ZERO\0" // IR(23279, 9)
  "SAT_SYM\0" // IR(23288, 8)
  "PackedVectorFormat4x8Bit\0" // IR(23296, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(23321, 28)
  "TRN\0" // IR(23349, 4)
  "TRN_ZERO\0" // IR(23353, 9)
  "RND\0" // IR(23362, 4)
  "RND_ZERO\0" // IR(23366, 9)
  "RND_INF\0" // IR(23375, 8)
  "RND_MIN_INF\0" // IR(23383, 12)
  "RND_CONV\0" // IR(23395, 9)
  "RND_CONV_ODD\0" // IR(23404, 13)
  "RobustnessPerComponentNV\0" // IR(23417, 25)
  "RobustnessPerElementNV\0" // IR(23442, 23)
  "OpaqueKHR\0" // IR(23465, 10)
  "NoOpaqueKHR\0" // IR(23475, 12)
  "TerminateOnFirstHitKHR\0" // IR(23487, 23)
  "SkipClosestHitShaderKHR\0" // IR(23510, 24)
  "CullBackFacingTrianglesKHR\0" // IR(23534, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(23561, 28)
  "CullOpaqueKHR\0" // IR(23589, 14)
  "CullNoOpaqueKHR\0" // IR(23603, 16)
  "SkipTrianglesKHR\0" // IR(23619, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(23636, 24)
  "SkipAABBsKHR\0" // IR(23660, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(23673, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(23703, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(23744, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(23781, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(23818, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(23859, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(23901, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(23934, 33)
  "ClampToEdge\0" // IR(23967, 12)
  "Clamp\0" // IR(23979, 6)
  "Repeat\0" // IR(23985, 7)
  "RepeatMirrored\0" // IR(23992, 15)
  "Nearest\0" // IR(24007, 8)
  "Linear\0" // IR(24015, 7)
  "Unknown\0" // IR(24022, 8)
  "Rgba32f\0" // IR(24030, 8)
  "Rgba16f\0" // IR(24038, 8)
  "R32f\0" // IR(24046, 5)
  "Rgba8\0" // IR(24051, 6)
  "Rgba8Snorm\0" // IR(24057, 11)
  "Rg32f\0" // IR(24068, 6)
  "Rg16f\0" // IR(24074, 6)
  "R11fG11fB10f\0" // IR(24080, 13)
  "R16f\0" // IR(24093, 5)
  "Rgba16\0" // IR(24098, 7)
  "Rgb10A2\0" // IR(24105, 8)
  "Rg16\0" // IR(24113, 5)
  "Rg8\0" // IR(24118, 4)
  "R16\0" // IR(24122, 4)
  "R8\0" // IR(24126, 3)
  "Rgba16Snorm\0" // IR(24129, 12)
  "Rg16Snorm\0" // IR(24141, 10)
  "Rg8Snorm\0" // IR(24151, 9)
  "R16Snorm\0" // IR(24160, 9)
  "R8Snorm\0" // IR(24169, 8)
  "Rgba32i\0" // IR(24177, 8)
  "Rgba16i\0" // IR(24185, 8)
  "Rgba8i\0" // IR(24193, 7)
  "R32i\0" // IR(24200, 5)
  "Rg32i\0" // IR(24205, 6)
  "Rg16i\0" // IR(24211, 6)
  "Rg8i\0" // IR(24217, 5)
  "R16i\0" // IR(24222, 5)
  "R8i\0" // IR(24227, 4)
  "Rgba32ui\0" // IR(24231, 9)
  "Rgba16ui\0" // IR(24240, 9)
  "Rgba8ui\0" // IR(24249, 8)
  "R32ui\0" // IR(24257, 6)
  "Rgb10a2ui\0" // IR(24263, 10)
  "Rg32ui\0" // IR(24273, 7)
  "Rg16ui\0" // IR(24280, 7)
  "Rg8ui\0" // IR(24287, 6)
  "R16ui\0" // IR(24293, 6)
  "R8ui\0" // IR(24299, 5)
  "R64ui\0" // IR(24304, 6)
  "R64i\0" // IR(24310, 5)
  "CrossDevice\0" // IR(24315, 12)
  "Device\0" // IR(24327, 7)
  "Workgroup\0" // IR(24334, 10)
  "Subgroup\0" // IR(24344, 9)
  "Invocation\0" // IR(24353, 11)
  "QueueFamily\0" // IR(24364, 12)
  "QueueFamilyKHR\0" // IR(24376, 15)
  "ShaderCallKHR\0" // IR(24391, 14)
  "Flatten\0" // IR(24405, 8)
  "DontFlatten\0" // IR(24413, 12)
  "IdentifierPossibleDuplicates\0" // IR(24425, 29)
  "FlagUnknownPhysicalLayout\0" // IR(24454, 26)
  "ESSL\0" // IR(24480, 5)
  "GLSL\0" // IR(24485, 5)
  "OpenCL_C\0" // IR(24490, 9)
  "OpenCL_CPP\0" // IR(24499, 11)
  "HLSL\0" // IR(24510, 5)
  "CPP_for_OpenCL\0" // IR(24515, 15)
  "SYCL\0" // IR(24530, 5)
  "HERO_C\0" // IR(24535, 7)
  "NZSL\0" // IR(24542, 5)
  "WGSL\0" // IR(24547, 5)
  "Slang\0" // IR(24552, 6)
  "Zig\0" // IR(24558, 4)
  "Rust\0" // IR(24562, 5)
  "UniformConstant\0" // IR(24567, 16)
  "Input\0" // IR(24583, 6)
  "Output\0" // IR(24589, 7)
  "CrossWorkgroup\0" // IR(24596, 15)
  "Private\0" // IR(24611, 8)
  "Function\0" // IR(24619, 9)
  "Generic\0" // IR(24628, 8)
  "PushConstant\0" // IR(24636, 13)
  "AtomicCounter\0" // IR(24649, 14)
  "Image\0" // IR(24663, 6)
  "StorageBuffer\0" // IR(24669, 14)
  "TileImageEXT\0" // IR(24683, 13)
  "TileAttachmentQCOM\0" // IR(24696, 19)
  "NodePayloadAMDX\0" // IR(24715, 16)
  "CallableDataKHR\0" // IR(24731, 16)
  "CallableDataNV\0" // IR(24747, 15)
  "IncomingCallableDataKHR\0" // IR(24762, 24)
  "IncomingCallableDataNV\0" // IR(24786, 23)
  "RayPayloadKHR\0" // IR(24809, 14)
  "RayPayloadNV\0" // IR(24823, 13)
  "HitAttributeKHR\0" // IR(24836, 16)
  "HitAttributeNV\0" // IR(24852, 15)
  "IncomingRayPayloadKHR\0" // IR(24867, 22)
  "IncomingRayPayloadNV\0" // IR(24889, 21)
  "ShaderRecordBufferKHR\0" // IR(24910, 22)
  "ShaderRecordBufferNV\0" // IR(24932, 21)
  "PhysicalStorageBuffer\0" // IR(24953, 22)
  "PhysicalStorageBufferEXT\0" // IR(24975, 25)
  "HitObjectAttributeNV\0" // IR(25000, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(25021, 24)
  "CodeSectionINTEL\0" // IR(25045, 17)
  "DeviceOnlyINTEL\0" // IR(25062, 16)
  "HostOnlyINTEL\0" // IR(25078, 14)
  "WriteThroughINTEL\0" // IR(25092, 18)
  "WriteBackINTEL\0" // IR(25110, 15)
  "TensorView\0" // IR(25125, 11)
  "DecodeFunc\0" // IR(25136, 11)
  "Undefined\0" // IR(25147, 10)
  "NoneARM\0" // IR(25157, 8)
  "NontemporalARM\0" // IR(25165, 15)
  "OutOfBoundsValueARM\0" // IR(25180, 20)
  "MakeElementAvailableARM\0" // IR(25200, 24)
  "MakeElementVisibleARM\0" // IR(25224, 22)
  "NonPrivateElementARM\0" // IR(25246, 21)
  "Nop\0" // IR(25267, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(25271, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(25296, 27)
  "Undef\0" // IR(25323, 6)
  "SourceContinued\0" // IR(25329, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(25345, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(25378, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(25407, 41)
  "Source\0" // IR(25448, 7)
  "SourceExtension\0" // IR(25455, 16)
  "Name\0" // IR(25471, 5)
  "MemberName\0" // IR(25476, 11)
  "String\0" // IR(25487, 7)
  "Line\0" // IR(25494, 5)
  "Extension\0" // IR(25499, 10)
  "ExtInstImport\0" // IR(25509, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(25523, 46)
  "ExtInst\0" // IR(25569, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(25577, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(25611, 30)
  "MemoryModel\0" // IR(25641, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(25653, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(25686, 29)
  "EntryPoint\0" // IR(25715, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(25726, 32)
  "ExecutionMode\0" // IR(25758, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(25772, 28)
  "Capability\0" // IR(25800, 11)
  "TypeVoid\0" // IR(25811, 9)
  "TypeBool\0" // IR(25820, 9)
  "TypeInt\0" // IR(25829, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(25837, 37)
  "TypeFloat\0" // IR(25874, 10)
  "TypeVector\0" // IR(25884, 11)
  "TypeMatrix\0" // IR(25895, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(25906, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(25938, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(25976, 43)
  "TypeImage\0" // IR(26019, 10)
  "TypeSampler\0" // IR(26029, 12)
  "TypeSampledImage\0" // IR(26041, 17)
  "TypeArray\0" // IR(26058, 10)
  "TypeRuntimeArray\0" // IR(26068, 17)
  "TypeStruct\0" // IR(26085, 11)
  "TypeOpaque\0" // IR(26096, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(26107, 31)
  "TypePointer\0" // IR(26138, 12)
  "TypeFunction\0" // IR(26150, 13)
  "TypeEvent\0" // IR(26163, 10)
  "TypeDeviceEvent\0" // IR(26173, 16)
  "TypeReserveId\0" // IR(26189, 14)
  "TypeQueue\0" // IR(26203, 10)
  "TypePipe\0" // IR(26213, 9)
  "TypeForwardPointer\0" // IR(26222, 19)
  "ConstantTrue\0" // IR(26241, 13)
  "ConstantFalse\0" // IR(26254, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(26268, 38)
  "ConstantComposite\0" // IR(26306, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(26324, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(26365, 37)
  "ConstantSampler\0" // IR(26402, 16)
  "ConstantNull\0" // IR(26418, 13)
  "SpecConstantTrue\0" // IR(26431, 17)
  "SpecConstantFalse\0" // IR(26448, 18)
  "SpecConstant\0" // IR(26466, 13)
  "SpecConstantComposite\0" // IR(26479, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(26501, 41)
  "SpecConstantOp\0" // IR(26542, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(26557, 34)
  "FunctionParameter\0" // IR(26591, 18)
  "FunctionEnd\0" // IR(26609, 12)
  "FunctionCall\0" // IR(26621, 13)
  "Variable\0" // IR(26634, 9)
  "ImageTexelPointer\0" // IR(26643, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(26661, 40)
  "Load\0" // IR(26701, 5)
  "Store\0" // IR(26706, 6)
  "CopyMemory\0" // IR(26712, 11)
  "CopyMemorySized\0" // IR(26723, 16)
  "AccessChain\0" // IR(26739, 12)
  "InBoundsAccessChain\0" // IR(26751, 20)
  "PtrAccessChain\0" // IR(26771, 15)
  "ArrayLength\0" // IR(26786, 12)
  "GenericPtrMemSemantics\0" // IR(26798, 23)
  "InBoundsPtrAccessChain\0" // IR(26821, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(26844, 28)
  "Decorate\0" // IR(26872, 9)
  "MemberDecorate\0" // IR(26881, 15)
  "DecorationGroup\0" // IR(26896, 16)
  "GroupDecorate\0" // IR(26912, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(26926, 45)
  "GroupMemberDecorate\0" // IR(26971, 20)
  "VectorExtractDynamic\0" // IR(26991, 21)
  "VectorInsertDynamic\0" // IR(27012, 20)
  "VectorShuffle\0" // IR(27032, 14)
  "CompositeConstruct\0" // IR(27046, 19)
  "CompositeExtract\0" // IR(27065, 17)
  "CompositeInsert\0" // IR(27082, 16)
  "CopyObject\0" // IR(27098, 11)
  "Transpose\0" // IR(27109, 10)
  "SampledImage\0" // IR(27119, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(27132, 32)
  "ImageSampleImplicitLod\0" // IR(27164, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(27187, 23)
  "ImageSampleExplicitLod\0" // IR(27210, 23)
  "ImageSampleDrefImplicitLod\0" // IR(27233, 27)
  "ImageSampleDrefExplicitLod\0" // IR(27260, 27)
  "ImageSampleProjImplicitLod\0" // IR(27287, 27)
  "ImageSampleProjExplicitLod\0" // IR(27314, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(27341, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(27372, 31)
  "ImageFetch\0" // IR(27403, 11)
  "ImageGather\0" // IR(27414, 12)
  "ImageDrefGather\0" // IR(27426, 16)
  "ImageRead\0" // IR(27442, 10)
  "ImageWrite\0" // IR(27452, 11)
  "ImageQueryFormat\0" // IR(27463, 17)
  "ImageQueryOrder\0" // IR(27480, 16)
  "ImageQuerySizeLod\0" // IR(27496, 18)
  "ImageQuerySize\0" // IR(27514, 15)
  "ImageQueryLod\0" // IR(27529, 14)
  "ImageQueryLevels\0" // IR(27543, 17)
  "ImageQuerySamples\0" // IR(27560, 18)
  "ConvertFToU\0" // IR(27578, 12)
  "ConvertFToS\0" // IR(27590, 12)
  "ConvertSToF\0" // IR(27602, 12)
  "ConvertUToF\0" // IR(27614, 12)
  "UConvert\0" // IR(27626, 9)
  "SConvert\0" // IR(27635, 9)
  "FConvert\0" // IR(27644, 9)
  "QuantizeToF16\0" // IR(27653, 14)
  "ConvertPtrToU\0" // IR(27667, 14)
  "SatConvertSToU\0" // IR(27681, 15)
  "SatConvertUToS\0" // IR(27696, 15)
  "ConvertUToPtr\0" // IR(27711, 14)
  "PtrCastToGeneric\0" // IR(27725, 17)
  "GenericCastToPtr\0" // IR(27742, 17)
  "GenericCastToPtrExplicit\0" // IR(27759, 25)
  "Bitcast\0" // IR(27784, 8)
  "SNegate\0" // IR(27792, 8)
  "FNegate\0" // IR(27800, 8)
  "IAdd\0" // IR(27808, 5)
  "FAdd\0" // IR(27813, 5)
  "ISub\0" // IR(27818, 5)
  "FSub\0" // IR(27823, 5)
  "IMul\0" // IR(27828, 5)
  "FMul\0" // IR(27833, 5)
  "UDiv\0" // IR(27838, 5)
  "SDiv\0" // IR(27843, 5)
  "FDiv\0" // IR(27848, 5)
  "UMod\0" // IR(27853, 5)
  "SRem\0" // IR(27858, 5)
  "SMod\0" // IR(27863, 5)
  "FRem\0" // IR(27868, 5)
  "FMod\0" // IR(27873, 5)
  "VectorTimesScalar\0" // IR(27878, 18)
  "MatrixTimesScalar\0" // IR(27896, 18)
  "VectorTimesMatrix\0" // IR(27914, 18)
  "MatrixTimesVector\0" // IR(27932, 18)
  "MatrixTimesMatrix\0" // IR(27950, 18)
  "OuterProduct\0" // IR(27968, 13)
  "Dot\0" // IR(27981, 4)
  "IAddCarry\0" // IR(27985, 10)
  "ISubBorrow\0" // IR(27995, 11)
  "UMulExtended\0" // IR(28006, 13)
  "SMulExtended\0" // IR(28019, 13)
  "Any\0" // IR(28032, 4)
  "All\0" // IR(28036, 4)
  "IsNan\0" // IR(28040, 6)
  "IsInf\0" // IR(28046, 6)
  "IsFinite\0" // IR(28052, 9)
  "IsNormal\0" // IR(28061, 9)
  "SignBitSet\0" // IR(28070, 11)
  "LessOrGreater\0" // IR(28081, 14)
  "Ordered\0" // IR(28095, 8)
  "Unordered\0" // IR(28103, 10)
  "LogicalEqual\0" // IR(28113, 13)
  "LogicalNotEqual\0" // IR(28126, 16)
  "LogicalOr\0" // IR(28142, 10)
  "LogicalAnd\0" // IR(28152, 11)
  "LogicalNot\0" // IR(28163, 11)
  "Select\0" // IR(28174, 7)
  "IEqual\0" // IR(28181, 7)
  "INotEqual\0" // IR(28188, 10)
  "UGreaterThan\0" // IR(28198, 13)
  "SGreaterThan\0" // IR(28211, 13)
  "UGreaterThanEqual\0" // IR(28224, 18)
  "SGreaterThanEqual\0" // IR(28242, 18)
  "ULessThan\0" // IR(28260, 10)
  "SLessThan\0" // IR(28270, 10)
  "ULessThanEqual\0" // IR(28280, 15)
  "SLessThanEqual\0" // IR(28295, 15)
  "FOrdEqual\0" // IR(28310, 10)
  "FUnordEqual\0" // IR(28320, 12)
  "FOrdNotEqual\0" // IR(28332, 13)
  "FUnordNotEqual\0" // IR(28345, 15)
  "FOrdLessThan\0" // IR(28360, 13)
  "FUnordLessThan\0" // IR(28373, 15)
  "FOrdGreaterThan\0" // IR(28388, 16)
  "FUnordGreaterThan\0" // IR(28404, 18)
  "FOrdLessThanEqual\0" // IR(28422, 18)
  "FUnordLessThanEqual\0" // IR(28440, 20)
  "FOrdGreaterThanEqual\0" // IR(28460, 21)
  "FUnordGreaterThanEqual\0" // IR(28481, 23)
  "ShiftRightLogical\0" // IR(28504, 18)
  "ShiftRightArithmetic\0" // IR(28522, 21)
  "ShiftLeftLogical\0" // IR(28543, 17)
  "BitwiseOr\0" // IR(28560, 10)
  "BitwiseXor\0" // IR(28570, 11)
  "BitwiseAnd\0" // IR(28581, 11)
  "Not\0" // IR(28592, 4)
  "BitFieldInsert\0" // IR(28596, 15)
  "BitFieldSExtract\0" // IR(28611, 17)
  "BitFieldUExtract\0" // IR(28628, 17)
  "BitReverse\0" // IR(28645, 11)
  "BitCount\0" // IR(28656, 9)
  "DPdx\0" // IR(28665, 5)
  "DPdy\0" // IR(28670, 5)
  "Fwidth\0" // IR(28675, 7)
  "DPdxFine\0" // IR(28682, 9)
  "DPdyFine\0" // IR(28691, 9)
  "FwidthFine\0" // IR(28700, 11)
  "DPdxCoarse\0" // IR(28711, 11)
  "DPdyCoarse\0" // IR(28722, 11)
  "FwidthCoarse\0" // IR(28733, 13)
  "EmitVertex\0" // IR(28746, 11)
  "EndPrimitive\0" // IR(28757, 13)
  "EmitStreamVertex\0" // IR(28770, 17)
  "EndStreamPrimitive\0" // IR(28787, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(28806, 37)
  "ControlBarrier\0" // IR(28843, 15)
  "MemoryBarrier\0" // IR(28858, 14)
  "AtomicLoad\0" // IR(28872, 11)
  "AtomicStore\0" // IR(28883, 12)
  "AtomicExchange\0" // IR(28895, 15)
  "AtomicCompareExchange\0" // IR(28910, 22)
  "AtomicCompareExchangeWeak\0" // IR(28932, 26)
  "AtomicIIncrement\0" // IR(28958, 17)
  "AtomicIDecrement\0" // IR(28975, 17)
  "AtomicIAdd\0" // IR(28992, 11)
  "AtomicISub\0" // IR(29003, 11)
  "AtomicSMin\0" // IR(29014, 11)
  "AtomicUMin\0" // IR(29025, 11)
  "AtomicSMax\0" // IR(29036, 11)
  "AtomicUMax\0" // IR(29047, 11)
  "AtomicAnd\0" // IR(29058, 10)
  "AtomicOr\0" // IR(29068, 9)
  "AtomicXor\0" // IR(29077, 10)
  "Phi\0" // IR(29087, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(29091, 30)
  "LoopMerge\0" // IR(29121, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(29131, 35)
  "SelectionMerge\0" // IR(29166, 15)
  "Label\0" // IR(29181, 6)
  "Branch\0" // IR(29187, 7)
  "BranchConditional\0" // IR(29194, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(29212, 45)
  "Switch\0" // IR(29257, 7)
  "Kill\0" // IR(29264, 5)
  "Return\0" // IR(29269, 7)
  "ReturnValue\0" // IR(29276, 12)
  "Unreachable\0" // IR(29288, 12)
  "LifetimeStart\0" // IR(29300, 14)
  "LifetimeStop\0" // IR(29314, 13)
  "GroupAsyncCopy\0" // IR(29327, 15)
  "GroupWaitEvents\0" // IR(29342, 16)
  "GroupAll\0" // IR(29358, 9)
  "GroupAny\0" // IR(29367, 9)
  "GroupBroadcast\0" // IR(29376, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(29391, 33)
  "GroupIAdd\0" // IR(29424, 10)
  "GroupFAdd\0" // IR(29434, 10)
  "GroupFMin\0" // IR(29444, 10)
  "GroupUMin\0" // IR(29454, 10)
  "GroupSMin\0" // IR(29464, 10)
  "GroupFMax\0" // IR(29474, 10)
  "GroupUMax\0" // IR(29484, 10)
  "GroupSMax\0" // IR(29494, 10)
  "ReadPipe\0" // IR(29504, 9)
  "WritePipe\0" // IR(29513, 10)
  "ReservedReadPipe\0" // IR(29523, 17)
  "ReservedWritePipe\0" // IR(29540, 18)
  "ReserveReadPipePackets\0" // IR(29558, 23)
  "ReserveWritePipePackets\0" // IR(29581, 24)
  "CommitReadPipe\0" // IR(29605, 15)
  "CommitWritePipe\0" // IR(29620, 16)
  "IsValidReserveId\0" // IR(29636, 17)
  "GetNumPipePackets\0" // IR(29653, 18)
  "GetMaxPipePackets\0" // IR(29671, 18)
  "GroupReserveReadPipePackets\0" // IR(29689, 28)
  "GroupReserveWritePipePackets\0" // IR(29717, 29)
  "GroupCommitReadPipe\0" // IR(29746, 20)
  "GroupCommitWritePipe\0" // IR(29766, 21)
  "EnqueueMarker\0" // IR(29787, 14)
  "EnqueueKernel\0" // IR(29801, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(29815, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(29845, 32)
  "GetKernelWorkGroupSize\0" // IR(29877, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(29900, 40)
  "RetainEvent\0" // IR(29940, 12)
  "ReleaseEvent\0" // IR(29952, 13)
  "CreateUserEvent\0" // IR(29965, 16)
  "IsValidEvent\0" // IR(29981, 13)
  "SetUserEventStatus\0" // IR(29994, 19)
  "CaptureEventProfilingInfo\0" // IR(30013, 26)
  "GetDefaultQueue\0" // IR(30039, 16)
  "BuildNDRange\0" // IR(30055, 13)
  "ImageSparseSampleImplicitLod\0" // IR(30068, 29)
  "ImageSparseSampleExplicitLod\0" // IR(30097, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(30126, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(30159, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(30192, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(30225, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(30258, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(30295, 37)
  "ImageSparseFetch\0" // IR(30332, 17)
  "ImageSparseGather\0" // IR(30349, 18)
  "ImageSparseDrefGather\0" // IR(30367, 22)
  "ImageSparseTexelsResident\0" // IR(30389, 26)
  "NoLine\0" // IR(30415, 7)
  "AtomicFlagTestAndSet\0" // IR(30422, 21)
  "AtomicFlagClear\0" // IR(30443, 16)
  "ImageSparseRead\0" // IR(30459, 16)
  "SizeOf\0" // IR(30475, 7)
  "TypePipeStorage\0" // IR(30482, 16)
  "ConstantPipeStorage\0" // IR(30498, 20)
  "CreatePipeFromPipeStorage\0" // IR(30518, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(30544, 35)
  "GetKernelMaxNumSubgroups\0" // IR(30579, 25)
  "TypeNamedBarrier\0" // IR(30604, 17)
  "NamedBarrierInitialize\0" // IR(30621, 23)
  "MemoryNamedBarrier\0" // IR(30644, 19)
  "ModuleProcessed\0" // IR(30663, 16)
  "ExecutionModeId\0" // IR(30679, 16)
  "DecorateId\0" // IR(30695, 11)
  "GroupNonUniformElect\0" // IR(30706, 21)
  "GroupNonUniformAll\0" // IR(30727, 19)
  "GroupNonUniformAny\0" // IR(30746, 19)
  "GroupNonUniformAllEqual\0" // IR(30765, 24)
  "GroupNonUniformBroadcast\0" // IR(30789, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(30814, 30)
  "GroupNonUniformInverseBallot\0" // IR(30844, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(30873, 32)
  "GroupNonUniformBallotBitCount\0" // IR(30905, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(30935, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(30964, 29)
  "GroupNonUniformShuffleXor\0" // IR(30993, 26)
  "GroupNonUniformShuffleUp\0" // IR(31019, 25)
  "GroupNonUniformShuffleDown\0" // IR(31044, 27)
  "GroupNonUniformIAdd\0" // IR(31071, 20)
  "GroupNonUniformFAdd\0" // IR(31091, 20)
  "GroupNonUniformIMul\0" // IR(31111, 20)
  "GroupNonUniformFMul\0" // IR(31131, 20)
  "GroupNonUniformSMin\0" // IR(31151, 20)
  "GroupNonUniformUMin\0" // IR(31171, 20)
  "GroupNonUniformFMin\0" // IR(31191, 20)
  "GroupNonUniformSMax\0" // IR(31211, 20)
  "GroupNonUniformUMax\0" // IR(31231, 20)
  "GroupNonUniformFMax\0" // IR(31251, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(31271, 26)
  "GroupNonUniformBitwiseOr\0" // IR(31297, 25)
  "GroupNonUniformBitwiseXor\0" // IR(31322, 26)
  "GroupNonUniformLogicalAnd\0" // IR(31348, 26)
  "GroupNonUniformLogicalOr\0" // IR(31374, 25)
  "GroupNonUniformLogicalXor\0" // IR(31399, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(31425, 29)
  "GroupNonUniformQuadSwap\0" // IR(31454, 24)
  "CopyLogical\0" // IR(31478, 12)
  "PtrEqual\0" // IR(31490, 9)
  "PtrNotEqual\0" // IR(31499, 12)
  "PtrDiff\0" // IR(31511, 8)
  "ColorAttachmentReadEXT\0" // IR(31519, 23)
  "DepthAttachmentReadEXT\0" // IR(31542, 23)
  "StencilAttachmentReadEXT\0" // IR(31565, 25)
  "TypeTensorARM\0" // IR(31590, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(31604, 42)
  "TensorReadARM\0" // IR(31646, 14)
  "TensorWriteARM\0" // IR(31660, 15)
  "TensorQuerySizeARM\0" // IR(31675, 19)
  "GraphConstantARM\0" // IR(31694, 17)
  "GraphEntryPointARM\0" // IR(31711, 19)
  "GraphInputARM\0" // IR(31730, 14)
  "GraphSetOutputARM\0" // IR(31744, 18)
  "GraphEndARM\0" // IR(31762, 12)
  "TypeGraphARM\0" // IR(31774, 13)
  "TerminateInvocation\0" // IR(31787, 20)
  "TypeUntypedPointerKHR\0" // IR(31807, 22)
  "UntypedVariableKHR\0" // IR(31829, 19)
  "UntypedAccessChainKHR\0" // IR(31848, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(31870, 30)
  "SubgroupFirstInvocationKHR\0" // IR(31900, 27)
  "UntypedPtrAccessChainKHR\0" // IR(31927, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(31952, 33)
  "UntypedArrayLengthKHR\0" // IR(31985, 22)
  "UntypedPrefetchKHR\0" // IR(32007, 19)
  "FmaKHR\0" // IR(32026, 7)
  "SubgroupAllKHR\0" // IR(32033, 15)
  "SubgroupAnyKHR\0" // IR(32048, 15)
  "SubgroupAllEqualKHR\0" // IR(32063, 20)
  "SubgroupReadInvocationKHR\0" // IR(32083, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(32109, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(32135, 25)
  "TraceRayKHR\0" // IR(32160, 12)
  "ExecuteCallableKHR\0" // IR(32172, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(32191, 35)
  "IgnoreIntersectionKHR\0" // IR(32226, 22)
  "TerminateRayKHR\0" // IR(32248, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(32264, 47)
  "SDot\0" // IR(32311, 5)
  "SDotKHR\0" // IR(32316, 8)
  "UDot\0" // IR(32324, 5)
  "UDotKHR\0" // IR(32329, 8)
  "SUDot\0" // IR(32337, 6)
  "SUDotKHR\0" // IR(32343, 9)
  "SDotAccSat\0" // IR(32352, 11)
  "SDotAccSatKHR\0" // IR(32363, 14)
  "UDotAccSat\0" // IR(32377, 11)
  "UDotAccSatKHR\0" // IR(32388, 14)
  "SUDotAccSat\0" // IR(32402, 12)
  "SUDotAccSatKHR\0" // IR(32414, 15)
  "TypeCooperativeMatrixKHR\0" // IR(32429, 25)
  "CooperativeMatrixLoadKHR\0" // IR(32454, 25)
  "CooperativeMatrixStoreKHR\0" // IR(32479, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(32505, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(32559, 27)
  "CooperativeMatrixLengthKHR\0" // IR(32586, 27)
  "ConstantCompositeReplicateEXT\0" // IR(32613, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(32643, 34)
  "CompositeConstructReplicateEXT\0" // IR(32677, 31)
  "TypeRayQueryKHR\0" // IR(32708, 16)
  "RayQueryInitializeKHR\0" // IR(32724, 22)
  "RayQueryTerminateKHR\0" // IR(32746, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(32767, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(32799, 31)
  "RayQueryProceedKHR\0" // IR(32830, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(32849, 31)
  "ImageSampleWeightedQCOM\0" // IR(32880, 24)
  "ImageBoxFilterQCOM\0" // IR(32904, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(32923, 23)
  "ImageBlockMatchSADQCOM\0" // IR(32946, 23)
  "BitCastArrayQCOM\0" // IR(32969, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(32986, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(33015, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(33044, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(33073, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(33102, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(33132, 28)
  "ExtractSubArrayQCOM\0" // IR(33160, 20)
  "GroupIAddNonUniformAMD\0" // IR(33180, 23)
  "GroupFAddNonUniformAMD\0" // IR(33203, 23)
  "GroupFMinNonUniformAMD\0" // IR(33226, 23)
  "GroupUMinNonUniformAMD\0" // IR(33249, 23)
  "GroupSMinNonUniformAMD\0" // IR(33272, 23)
  "GroupFMaxNonUniformAMD\0" // IR(33295, 23)
  "GroupUMaxNonUniformAMD\0" // IR(33318, 23)
  "GroupSMaxNonUniformAMD\0" // IR(33341, 23)
  "FragmentMaskFetchAMD\0" // IR(33364, 21)
  "FragmentFetchAMD\0" // IR(33385, 17)
  "ReadClockKHR\0" // IR(33402, 13)
  "AllocateNodePayloadsAMDX\0" // IR(33415, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(33440, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(33464, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(33489, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(33518, 27)
  "IsNodePayloadValidAMDX\0" // IR(33545, 23)
  "ConstantStringAMDX\0" // IR(33568, 19)
  "SpecConstantStringAMDX\0" // IR(33587, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(33610, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(33636, 26)
  "HitObjectRecordHitMotionNV\0" // IR(33662, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(33689, 36)
  "HitObjectRecordMissMotionNV\0" // IR(33725, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(33753, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(33781, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(33809, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(33842, 30)
  "HitObjectTraceRayMotionNV\0" // IR(33872, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(33898, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(33937, 44)
  "HitObjectRecordEmptyNV\0" // IR(33981, 23)
  "HitObjectTraceRayNV\0" // IR(34004, 20)
  "HitObjectRecordHitNV\0" // IR(34024, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(34045, 30)
  "HitObjectRecordMissNV\0" // IR(34075, 22)
  "HitObjectExecuteShaderNV\0" // IR(34097, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(34122, 26)
  "HitObjectGetAttributesNV\0" // IR(34148, 25)
  "HitObjectGetHitKindNV\0" // IR(34173, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(34195, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(34224, 28)
  "HitObjectGetInstanceIdNV\0" // IR(34252, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(34277, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(34311, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(34343, 29)
  "HitObjectGetRayTMaxNV\0" // IR(34372, 22)
  "HitObjectGetRayTMinNV\0" // IR(34394, 22)
  "HitObjectIsEmptyNV\0" // IR(34416, 19)
  "HitObjectIsHitNV\0" // IR(34435, 17)
  "HitObjectIsMissNV\0" // IR(34452, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(34470, 29)
  "ReorderThreadWithHintNV\0" // IR(34499, 24)
  "TypeHitObjectNV\0" // IR(34523, 16)
  "ImageSampleFootprintNV\0" // IR(34539, 23)
  "TypeCooperativeVectorNV\0" // IR(34562, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(34586, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(34615, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(34657, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(34696, 32)
  "CooperativeMatrixConvertNV\0" // IR(34728, 27)
  "EmitMeshTasksEXT\0" // IR(34755, 17)
  "SetMeshOutputsEXT\0" // IR(34772, 18)
  "GroupNonUniformPartitionNV\0" // IR(34790, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(34817, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(34850, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(34885, 38)
  "CooperativeVectorLoadNV\0" // IR(34923, 24)
  "CooperativeVectorStoreNV\0" // IR(34947, 25)
  "ReportIntersectionKHR\0" // IR(34972, 22)
  "ReportIntersectionNV\0" // IR(34994, 21)
  "IgnoreIntersectionNV\0" // IR(35015, 21)
  "TerminateRayNV\0" // IR(35036, 15)
  "TraceNV\0" // IR(35051, 8)
  "TraceMotionNV\0" // IR(35059, 14)
  "TraceRayMotionNV\0" // IR(35073, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(35090, 50)
  "TypeAccelerationStructureKHR\0" // IR(35140, 29)
  "TypeAccelerationStructureNV\0" // IR(35169, 28)
  "ExecuteCallableNV\0" // IR(35197, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(35215, 35)
  "RayQueryGetClusterIdNV\0" // IR(35250, 23)
  "HitObjectGetClusterIdNV\0" // IR(35273, 24)
  "TypeCooperativeMatrixNV\0" // IR(35297, 24)
  "CooperativeMatrixLoadNV\0" // IR(35321, 24)
  "CooperativeMatrixStoreNV\0" // IR(35345, 25)
  "CooperativeMatrixMulAddNV\0" // IR(35370, 26)
  "CooperativeMatrixLengthNV\0" // IR(35396, 26)
  "BeginInvocationInterlockEXT\0" // IR(35422, 28)
  "EndInvocationInterlockEXT\0" // IR(35450, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(35476, 43)
  "CooperativeMatrixReduceNV\0" // IR(35519, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(35545, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(35576, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(35620, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(35650, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(35681, 32)
  "TypeTensorLayoutNV\0" // IR(35713, 19)
  "TypeTensorViewNV\0" // IR(35732, 17)
  "CreateTensorLayoutNV\0" // IR(35749, 21)
  "TensorLayoutSetDimensionNV\0" // IR(35770, 27)
  "TensorLayoutSetStrideNV\0" // IR(35797, 24)
  "TensorLayoutSliceNV\0" // IR(35821, 20)
  "TensorLayoutSetClampValueNV\0" // IR(35841, 28)
  "CreateTensorViewNV\0" // IR(35869, 19)
  "TensorViewSetDimensionNV\0" // IR(35888, 25)
  "TensorViewSetStrideNV\0" // IR(35913, 22)
  "IsHelperInvocationEXT\0" // IR(35935, 22)
  "TensorViewSetClipNV\0" // IR(35957, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(35977, 27)
  "CooperativeMatrixTransposeNV\0" // IR(36004, 29)
  "ConvertUToImageNV\0" // IR(36033, 18)
  "ConvertUToSamplerNV\0" // IR(36051, 20)
  "ConvertImageToUNV\0" // IR(36071, 18)
  "ConvertSamplerToUNV\0" // IR(36089, 20)
  "ConvertUToSampledImageNV\0" // IR(36109, 25)
  "ConvertSampledImageToUNV\0" // IR(36134, 25)
  "SamplerImageAddressingModeNV\0" // IR(36159, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(36188, 52)
  "RawAccessChainNV\0" // IR(36240, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(36257, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(36297, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(36335, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(36373, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(36407, 37)
  "HitObjectGetSpherePositionNV\0" // IR(36444, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(36473, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(36500, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(36527, 23)
  "HitObjectIsSphereHitNV\0" // IR(36550, 23)
  "HitObjectIsLSSHitNV\0" // IR(36573, 20)
  "RayQueryIsSphereHitNV\0" // IR(36593, 22)
  "RayQueryIsLSSHitNV\0" // IR(36615, 19)
  "SubgroupShuffleDownINTEL\0" // IR(36634, 25)
  "SubgroupShuffleUpINTEL\0" // IR(36659, 23)
  "SubgroupShuffleXorINTEL\0" // IR(36682, 24)
  "SubgroupBlockReadINTEL\0" // IR(36706, 23)
  "SubgroupBlockWriteINTEL\0" // IR(36729, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(36753, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(36781, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(36810, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(36843, 34)
  "UCountLeadingZerosINTEL\0" // IR(36877, 24)
  "UCountTrailingZerosINTEL\0" // IR(36901, 25)
  "AbsISubINTEL\0" // IR(36926, 13)
  "AbsUSubINTEL\0" // IR(36939, 13)
  "IAddSatINTEL\0" // IR(36952, 13)
  "UAddSatINTEL\0" // IR(36965, 13)
  "IAverageINTEL\0" // IR(36978, 14)
  "UAverageINTEL\0" // IR(36992, 14)
  "IAverageRoundedINTEL\0" // IR(37006, 21)
  "UAverageRoundedINTEL\0" // IR(37027, 21)
  "ISubSatINTEL\0" // IR(37048, 13)
  "USubSatINTEL\0" // IR(37061, 13)
  "IMul32x16INTEL\0" // IR(37074, 15)
  "UMul32x16INTEL\0" // IR(37089, 15)
  "ConstantFunctionPointerINTEL\0" // IR(37104, 29)
  "FunctionPointerCallINTEL\0" // IR(37133, 25)
  "AsmTargetINTEL\0" // IR(37158, 15)
  "AsmCallINTEL\0" // IR(37173, 13)
  "AtomicFMinEXT\0" // IR(37186, 14)
  "AtomicFMaxEXT\0" // IR(37200, 14)
  "AssumeTrueKHR\0" // IR(37214, 14)
  "ExpectKHR\0" // IR(37228, 10)
  "DecorateString\0" // IR(37238, 15)
  "DecorateStringGOOGLE\0" // IR(37253, 21)
  "MemberDecorateString\0" // IR(37274, 21)
  "MemberDecorateStringGOOGLE\0" // IR(37295, 27)
  "VmeImageINTEL\0" // IR(37322, 14)
  "TypeVmeImageINTEL\0" // IR(37336, 18)
  "TypeAvcImePayloadINTEL\0" // IR(37354, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(37377, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(37400, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(37423, 23)
  "TypeAvcMceResultINTEL\0" // IR(37446, 22)
  "TypeAvcImeResultINTEL\0" // IR(37468, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(37490, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(37536, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(37580, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(37619, 37)
  "TypeAvcRefResultINTEL\0" // IR(37656, 22)
  "TypeAvcSicResultINTEL\0" // IR(37678, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(37700, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(37760, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(37813, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(37860, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(37900, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(37951, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(37995, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(38046, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(38102, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(38152, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(38204, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(38253, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(38300, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(38350, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(38401, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(38457, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(38490, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(38542, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(38603, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(38664, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(38703, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(38741, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(38780, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(38818, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(38857, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(38895, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(38931, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(38970, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(39013, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(39051, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(39089, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(39127, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(39172, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(39212, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(39274, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(39304, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(39342, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(39378, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(39411, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(39446, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(39485, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(39528, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(39575, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(39629, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(39663, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(39710, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(39755, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(39810, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(39863, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(39919, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(39973, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(40031, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(40087, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(40125, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(40171, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(40215, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(40264, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(40311, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(40381, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(40449, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(40518, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(40586, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(40652, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(40719, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(40755, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(40803, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(40862, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(40920, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(40976, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(41006, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(41036, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(41075, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(41121, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(41164, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(41211, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(41256, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(41302, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(41358, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(41396, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(41426, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(41458, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(41494, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(41536, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(41575, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(41614, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(41658, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(41706, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(41756, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(41799, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(41847, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(41890, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(41921, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(41968, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(42013, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(42059, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(42115, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(42153, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(42188, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(42232, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(42278, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(42319, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(42355, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(42405, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(42453, 35)
  "SaveMemoryINTEL\0" // IR(42488, 16)
  "RestoreMemoryINTEL\0" // IR(42504, 19)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(42523, 28)
  "ArbitraryFloatCastINTEL\0" // IR(42551, 24)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(42575, 31)
  "ArbitraryFloatCastToIntINTEL\0" // IR(42606, 29)
  "ArbitraryFloatAddINTEL\0" // IR(42635, 23)
  "ArbitraryFloatSubINTEL\0" // IR(42658, 23)
  "ArbitraryFloatMulINTEL\0" // IR(42681, 23)
  "ArbitraryFloatDivINTEL\0" // IR(42704, 23)
  "ArbitraryFloatGTINTEL\0" // IR(42727, 22)
  "ArbitraryFloatGEINTEL\0" // IR(42749, 22)
  "ArbitraryFloatLTINTEL\0" // IR(42771, 22)
  "ArbitraryFloatLEINTEL\0" // IR(42793, 22)
  "ArbitraryFloatEQINTEL\0" // IR(42815, 22)
  "ArbitraryFloatRecipINTEL\0" // IR(42837, 25)
  "ArbitraryFloatRSqrtINTEL\0" // IR(42862, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(42887, 24)
  "ArbitraryFloatHypotINTEL\0" // IR(42911, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(42936, 24)
  "ArbitraryFloatLogINTEL\0" // IR(42960, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(42983, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(43007, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(43032, 25)
  "ArbitraryFloatExpINTEL\0" // IR(43057, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(43080, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(43104, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(43129, 25)
  "ArbitraryFloatSinINTEL\0" // IR(43154, 23)
  "ArbitraryFloatCosINTEL\0" // IR(43177, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(43200, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(43226, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(43251, 25)
  "ArbitraryFloatASinINTEL\0" // IR(43276, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(43300, 26)
  "ArbitraryFloatACosINTEL\0" // IR(43326, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(43350, 26)
  "ArbitraryFloatATanINTEL\0" // IR(43376, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(43400, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(43426, 25)
  "ArbitraryFloatPowINTEL\0" // IR(43451, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(43474, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(43498, 24)
  "LoopControlINTEL\0" // IR(43522, 17)
  "AliasDomainDeclINTEL\0" // IR(43539, 21)
  "AliasScopeDeclINTEL\0" // IR(43560, 20)
  "AliasScopeListDeclINTEL\0" // IR(43580, 24)
  "FixedSqrtINTEL\0" // IR(43604, 15)
  "FixedRecipINTEL\0" // IR(43619, 16)
  "FixedRsqrtINTEL\0" // IR(43635, 16)
  "FixedSinINTEL\0" // IR(43651, 14)
  "FixedCosINTEL\0" // IR(43665, 14)
  "FixedSinCosINTEL\0" // IR(43679, 17)
  "FixedSinPiINTEL\0" // IR(43696, 16)
  "FixedCosPiINTEL\0" // IR(43712, 16)
  "FixedSinCosPiINTEL\0" // IR(43728, 19)
  "FixedLogINTEL\0" // IR(43747, 14)
  "FixedExpINTEL\0" // IR(43761, 14)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(43775, 29)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(43804, 29)
  "ReadPipeBlockingINTEL\0" // IR(43833, 22)
  "WritePipeBlockingINTEL\0" // IR(43855, 23)
  "RayQueryGetRayTMinKHR\0" // IR(43878, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(43900, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(43923, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(43951, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(43997, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(44034, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(44099, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(44139, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(44180, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(44219, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(44255, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(44301, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(44346, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(44388, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(44420, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(44449, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(44489, 40)
  "AtomicFAddEXT\0" // IR(44529, 14)
  "TypeBufferSurfaceINTEL\0" // IR(44543, 23)
  "TypeStructContinuedINTEL\0" // IR(44566, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(44591, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(44623, 36)
  "CompositeConstructContinuedINTEL\0" // IR(44659, 33)
  "ConvertFToBF16INTEL\0" // IR(44692, 20)
  "ConvertBF16ToFINTEL\0" // IR(44712, 20)
  "ControlBarrierArriveINTEL\0" // IR(44732, 26)
  "ControlBarrierWaitINTEL\0" // IR(44758, 24)
  "TaskSequenceCreateINTEL\0" // IR(44782, 24)
  "TaskSequenceAsyncINTEL\0" // IR(44806, 23)
  "TaskSequenceGetINTEL\0" // IR(44829, 21)
  "TaskSequenceReleaseINTEL\0" // IR(44850, 25)
  "TypeTaskSequenceINTEL\0" // IR(44875, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(44897, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(44924, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(44949, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(44983, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(45017, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(45046, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(45072, 62)
  "BitwiseFunctionINTEL\0" // IR(45134, 21)
  "ConditionalExtensionINTEL\0" // IR(45155, 26)
  "ConditionalEntryPointINTEL\0" // IR(45181, 27)
  "ConditionalCapabilityINTEL\0" // IR(45208, 27)
  "SpecConstantTargetINTEL\0" // IR(45235, 24)
  "SpecConstantArchitectureINTEL\0" // IR(45259, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(45289, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(45326, 30)
  "ConditionalCopyObjectINTEL\0" // IR(45356, 27)
  "GroupIMulKHR\0" // IR(45383, 13)
  "GroupFMulKHR\0" // IR(45396, 13)
  "GroupBitwiseAndKHR\0" // IR(45409, 19)
  "GroupBitwiseOrKHR\0" // IR(45428, 18)
  "GroupBitwiseXorKHR\0" // IR(45446, 19)
  "GroupLogicalAndKHR\0" // IR(45465, 19)
  "GroupLogicalOrKHR\0" // IR(45484, 18)
  "GroupLogicalXorKHR\0" // IR(45502, 19)
  "RoundFToTF32INTEL\0" // IR(45521, 18)
  "MaskedGatherINTEL\0" // IR(45539, 18)
  "MaskedScatterINTEL\0" // IR(45557, 19)
  "ConvertHandleToImageINTEL\0" // IR(45576, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(45602, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(45630, 33)
  "MIN_SAD\0" // IR(45663, 8)
  "MIN_SAD_COST\0" // IR(45671, 13)
  "RAW_SAD\0" // IR(45684, 8)
  "DebugInfoNone\0" // IR(45692, 14)
  "DebugCompilationUnit\0" // IR(45706, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(45727, 52)
  "DebugTypeBasic\0" // IR(45779, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(45794, 34)
  "DebugTypePointer\0" // IR(45828, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(45845, 38)
  "DebugTypeQualifier\0" // IR(45883, 19)
  "DebugTypeArray\0" // IR(45902, 15)
  "DebugTypeVector\0" // IR(45917, 16)
  "DebugTypedef\0" // IR(45933, 13)
  "DebugTypeFunction\0" // IR(45946, 18)
  "DebugTypeEnum\0" // IR(45964, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(45978, 38)
  "DebugTypeComposite\0" // IR(46016, 19)
  "DebugTypeMember\0" // IR(46035, 16)
  "DebugTypeInheritance\0" // IR(46051, 21)
  "DebugTypePtrToMember\0" // IR(46072, 21)
  "DebugTypeTemplate\0" // IR(46093, 18)
  "DebugTypeTemplateParameter\0" // IR(46111, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(46138, 35)
  "DebugTypeTemplateParameterPack\0" // IR(46173, 31)
  "DebugGlobalVariable\0" // IR(46204, 20)
  "DebugFunctionDeclaration\0" // IR(46224, 25)
  "DebugFunction\0" // IR(46249, 14)
  "DebugLexicalBlock\0" // IR(46263, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(46281, 31)
  "DebugScope\0" // IR(46312, 11)
  "DebugNoScope\0" // IR(46323, 13)
  "DebugInlinedAt\0" // IR(46336, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(46351, 42)
  "DebugLocalVariable\0" // IR(46393, 19)
  "DebugInlinedVariable\0" // IR(46412, 21)
  "DebugDeclare\0" // IR(46433, 13)
  "DebugValue\0" // IR(46446, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(46457, 33)
  "DebugOperation\0" // IR(46490, 15)
  "DebugExpression\0" // IR(46505, 16)
  "DebugMacroDef\0" // IR(46521, 14)
  "DebugMacroUndef\0" // IR(46535, 16)
  "Round\0" // IR(46551, 6)
  "RoundEven\0" // IR(46557, 10)
  "Trunc\0" // IR(46567, 6)
  "FAbs\0" // IR(46573, 5)
  "SAbs\0" // IR(46578, 5)
  "FSign\0" // IR(46583, 6)
  "SSign\0" // IR(46589, 6)
  "Floor\0" // IR(46595, 6)
  "Ceil\0" // IR(46601, 5)
  "Fract\0" // IR(46606, 6)
  "Radians\0" // IR(46612, 8)
  "Degrees\0" // IR(46620, 8)
  "Sin\0" // IR(46628, 4)
  "Cos\0" // IR(46632, 4)
  "Tan\0" // IR(46636, 4)
  "Asin\0" // IR(46640, 5)
  "Acos\0" // IR(46645, 5)
  "Atan\0" // IR(46650, 5)
  "Sinh\0" // IR(46655, 5)
  "Cosh\0" // IR(46660, 5)
  "Tanh\0" // IR(46665, 5)
  "Asinh\0" // IR(46670, 6)
  "Acosh\0" // IR(46676, 6)
  "Atanh\0" // IR(46682, 6)
  "Atan2\0" // IR(46688, 6)
  "Pow\0" // IR(46694, 4)
  "Exp\0" // IR(46698, 4)
  "Log\0" // IR(46702, 4)
  "Exp2\0" // IR(46706, 5)
  "Log2\0" // IR(46711, 5)
  "Sqrt\0" // IR(46716, 5)
  "InverseSqrt\0" // IR(46721, 12)
  "Determinant\0" // IR(46733, 12)
  "MatrixInverse\0" // IR(46745, 14)
  "Modf\0" // IR(46759, 5)
  "ModfStruct\0" // IR(46764, 11)
  "FMin\0" // IR(46775, 5)
  "UMin\0" // IR(46780, 5)
  "SMin\0" // IR(46785, 5)
  "FMax\0" // IR(46790, 5)
  "UMax\0" // IR(46795, 5)
  "SMax\0" // IR(46800, 5)
  "FClamp\0" // IR(46805, 7)
  "UClamp\0" // IR(46812, 7)
  "SClamp\0" // IR(46819, 7)
  "FMix\0" // IR(46826, 5)
  "IMix\0" // IR(46831, 5)
  "Step\0" // IR(46836, 5)
  "SmoothStep\0" // IR(46841, 11)
  "Fma\0" // IR(46852, 4)
  "Frexp\0" // IR(46856, 6)
  "FrexpStruct\0" // IR(46862, 12)
  "Ldexp\0" // IR(46874, 6)
  "PackSnorm4x8\0" // IR(46880, 13)
  "PackUnorm4x8\0" // IR(46893, 13)
  "PackSnorm2x16\0" // IR(46906, 14)
  "PackUnorm2x16\0" // IR(46920, 14)
  "PackHalf2x16\0" // IR(46934, 13)
  "PackDouble2x32\0" // IR(46947, 15)
  "UnpackSnorm2x16\0" // IR(46962, 16)
  "UnpackUnorm2x16\0" // IR(46978, 16)
  "UnpackHalf2x16\0" // IR(46994, 15)
  "UnpackSnorm4x8\0" // IR(47009, 15)
  "UnpackUnorm4x8\0" // IR(47024, 15)
  "UnpackDouble2x32\0" // IR(47039, 17)
  "Length\0" // IR(47056, 7)
  "Distance\0" // IR(47063, 9)
  "Cross\0" // IR(47072, 6)
  "Normalize\0" // IR(47078, 10)
  "FaceForward\0" // IR(47088, 12)
  "Reflect\0" // IR(47100, 8)
  "Refract\0" // IR(47108, 8)
  "FindILsb\0" // IR(47116, 9)
  "FindSMsb\0" // IR(47125, 9)
  "FindUMsb\0" // IR(47134, 9)
  "InterpolateAtCentroid\0" // IR(47143, 22)
  "InterpolateAtSample\0" // IR(47165, 20)
  "InterpolateAtOffset\0" // IR(47185, 20)
  "NMin\0" // IR(47205, 5)
  "NMax\0" // IR(47210, 5)
  "NClamp\0" // IR(47215, 7)
  "ArgumentInfo\0" // IR(47222, 13)
  "ArgumentStorageBuffer\0" // IR(47235, 22)
  "ArgumentUniform\0" // IR(47257, 16)
  "ArgumentPodStorageBuffer\0" // IR(47273, 25)
  "ArgumentPodUniform\0" // IR(47298, 19)
  "ArgumentPodPushConstant\0" // IR(47317, 24)
  "ArgumentSampledImage\0" // IR(47341, 21)
  "ArgumentStorageImage\0" // IR(47362, 21)
  "ArgumentSampler\0" // IR(47383, 16)
  "ArgumentWorkgroup\0" // IR(47399, 18)
  "SpecConstantWorkgroupSize\0" // IR(47417, 26)
  "SpecConstantGlobalOffset\0" // IR(47443, 25)
  "SpecConstantWorkDim\0" // IR(47468, 20)
  "PushConstantGlobalOffset\0" // IR(47488, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(47513, 30)
  "PushConstantGlobalSize\0" // IR(47543, 23)
  "PushConstantRegionOffset\0" // IR(47566, 25)
  "PushConstantNumWorkgroups\0" // IR(47591, 26)
  "PushConstantRegionGroupOffset\0" // IR(47617, 30)
  "ConstantDataStorageBuffer\0" // IR(47647, 26)
  "ConstantDataUniform\0" // IR(47673, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(47693, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(47723, 28)
  "ArgumentPointerPushConstant\0" // IR(47751, 28)
  "ArgumentPointerUniform\0" // IR(47779, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(47802, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(47837, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(47875, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(47917, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(47962, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(47999, 40)
  "ArgumentStorageTexelBuffer\0" // IR(48039, 27)
  "ArgumentUniformTexelBuffer\0" // IR(48066, 27)
  "ConstantDataPointerPushConstant\0" // IR(48093, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(48125, 40)
  "PrintfInfo\0" // IR(48165, 11)
  "PrintfBufferStorageBuffer\0" // IR(48176, 26)
  "PrintfBufferPointerPushConstant\0" // IR(48202, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(48234, 34)
  "WorkgroupVariableSize\0" // IR(48268, 22)
  "DebugImportedEntity\0" // IR(48290, 20)
  "DebugSource\0" // IR(48310, 12)
  "DebugFunctionDefinition\0" // IR(48322, 24)
  "DebugSourceContinued\0" // IR(48346, 21)
  "DebugLine\0" // IR(48367, 10)
  "DebugNoLine\0" // IR(48377, 12)
  "DebugBuildIdentifier\0" // IR(48389, 21)
  "DebugStoragePath\0" // IR(48410, 17)
  "DebugEntryPoint\0" // IR(48427, 16)
  "DebugTypeMatrix\0" // IR(48443, 16)
  "Configuration\0" // IR(48459, 14)
  "StartCounter\0" // IR(48473, 13)
  "StopCounter\0" // IR(48486, 12)
  "PushConstants\0" // IR(48498, 14)
  "SpecializationMapEntry\0" // IR(48512, 23)
  "DescriptorSetBuffer\0" // IR(48535, 20)
  "DescriptorSetImage\0" // IR(48555, 19)
  "DescriptorSetSampler\0" // IR(48574, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(48595, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(48658, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(48703, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(48752, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(48801, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(48845, 50)
  "DebugModuleINTEL\0" // IR(48895, 17)
  "acos\0" // IR(48912, 5)
  "acosh\0" // IR(48917, 6)
  "acospi\0" // IR(48923, 7)
  "asin\0" // IR(48930, 5)
  "asinh\0" // IR(48935, 6)
  "asinpi\0" // IR(48941, 7)
  "atan\0" // IR(48948, 5)
  "atan2\0" // IR(48953, 6)
  "atanh\0" // IR(48959, 6)
  "atanpi\0" // IR(48965, 7)
  "atan2pi\0" // IR(48972, 8)
  "cbrt\0" // IR(48980, 5)
  "ceil\0" // IR(48985, 5)
  "copysign\0" // IR(48990, 9)
  "cos\0" // IR(48999, 4)
  "cosh\0" // IR(49003, 5)
  "cospi\0" // IR(49008, 6)
  "erfc\0" // IR(49014, 5)
  "erf\0" // IR(49019, 4)
  "exp\0" // IR(49023, 4)
  "exp2\0" // IR(49027, 5)
  "exp10\0" // IR(49032, 6)
  "expm1\0" // IR(49038, 6)
  "fabs\0" // IR(49044, 5)
  "fdim\0" // IR(49049, 5)
  "floor\0" // IR(49054, 6)
  "fma\0" // IR(49060, 4)
  "fmax\0" // IR(49064, 5)
  "fmin\0" // IR(49069, 5)
  "fmod\0" // IR(49074, 5)
  "fract\0" // IR(49079, 6)
  "frexp\0" // IR(49085, 6)
  "hypot\0" // IR(49091, 6)
  "ilogb\0" // IR(49097, 6)
  "ldexp\0" // IR(49103, 6)
  "lgamma\0" // IR(49109, 7)
  "lgamma_r\0" // IR(49116, 9)
  "log\0" // IR(49125, 4)
  "log2\0" // IR(49129, 5)
  "log10\0" // IR(49134, 6)
  "log1p\0" // IR(49140, 6)
  "logb\0" // IR(49146, 5)
  "mad\0" // IR(49151, 4)
  "maxmag\0" // IR(49155, 7)
  "minmag\0" // IR(49162, 7)
  "modf\0" // IR(49169, 5)
  "nan\0" // IR(49174, 4)
  "nextafter\0" // IR(49178, 10)
  "pow\0" // IR(49188, 4)
  "pown\0" // IR(49192, 5)
  "powr\0" // IR(49197, 5)
  "remainder\0" // IR(49202, 10)
  "remquo\0" // IR(49212, 7)
  "rint\0" // IR(49219, 5)
  "rootn\0" // IR(49224, 6)
  "round\0" // IR(49230, 6)
  "rsqrt\0" // IR(49236, 6)
  "sin\0" // IR(49242, 4)
  "sincos\0" // IR(49246, 7)
  "sinh\0" // IR(49253, 5)
  "sinpi\0" // IR(49258, 6)
  "sqrt\0" // IR(49264, 5)
  "tan\0" // IR(49269, 4)
  "tanh\0" // IR(49273, 5)
  "tanpi\0" // IR(49278, 6)
  "tgamma\0" // IR(49284, 7)
  "trunc\0" // IR(49291, 6)
  "half_cos\0" // IR(49297, 9)
  "half_divide\0" // IR(49306, 12)
  "half_exp\0" // IR(49318, 9)
  "half_exp2\0" // IR(49327, 10)
  "half_exp10\0" // IR(49337, 11)
  "half_log\0" // IR(49348, 9)
  "half_log2\0" // IR(49357, 10)
  "half_log10\0" // IR(49367, 11)
  "half_powr\0" // IR(49378, 10)
  "half_recip\0" // IR(49388, 11)
  "half_rsqrt\0" // IR(49399, 11)
  "half_sin\0" // IR(49410, 9)
  "half_sqrt\0" // IR(49419, 10)
  "half_tan\0" // IR(49429, 9)
  "native_cos\0" // IR(49438, 11)
  "native_divide\0" // IR(49449, 14)
  "native_exp\0" // IR(49463, 11)
  "native_exp2\0" // IR(49474, 12)
  "native_exp10\0" // IR(49486, 13)
  "native_log\0" // IR(49499, 11)
  "native_log2\0" // IR(49510, 12)
  "native_log10\0" // IR(49522, 13)
  "native_powr\0" // IR(49535, 12)
  "native_recip\0" // IR(49547, 13)
  "native_rsqrt\0" // IR(49560, 13)
  "native_sin\0" // IR(49573, 11)
  "native_sqrt\0" // IR(49584, 12)
  "native_tan\0" // IR(49596, 11)
  "fclamp\0" // IR(49607, 7)
  "degrees\0" // IR(49614, 8)
  "fmax_common\0" // IR(49622, 12)
  "fmin_common\0" // IR(49634, 12)
  "mix\0" // IR(49646, 4)
  "radians\0" // IR(49650, 8)
  "step\0" // IR(49658, 5)
  "smoothstep\0" // IR(49663, 11)
  "sign\0" // IR(49674, 5)
  "cross\0" // IR(49679, 6)
  "distance\0" // IR(49685, 9)
  "length\0" // IR(49694, 7)
  "normalize\0" // IR(49701, 10)
  "fast_distance\0" // IR(49711, 14)
  "fast_length\0" // IR(49725, 12)
  "fast_normalize\0" // IR(49737, 15)
  "s_abs\0" // IR(49752, 6)
  "s_abs_diff\0" // IR(49758, 11)
  "s_add_sat\0" // IR(49769, 10)
  "u_add_sat\0" // IR(49779, 10)
  "s_hadd\0" // IR(49789, 7)
  "u_hadd\0" // IR(49796, 7)
  "s_rhadd\0" // IR(49803, 8)
  "u_rhadd\0" // IR(49811, 8)
  "s_clamp\0" // IR(49819, 8)
  "u_clamp\0" // IR(49827, 8)
  "clz\0" // IR(49835, 4)
  "ctz\0" // IR(49839, 4)
  "s_mad_hi\0" // IR(49843, 9)
  "u_mad_sat\0" // IR(49852, 10)
  "s_mad_sat\0" // IR(49862, 10)
  "s_max\0" // IR(49872, 6)
  "u_max\0" // IR(49878, 6)
  "s_min\0" // IR(49884, 6)
  "u_min\0" // IR(49890, 6)
  "s_mul_hi\0" // IR(49896, 9)
  "rotate\0" // IR(49905, 7)
  "s_sub_sat\0" // IR(49912, 10)
  "u_sub_sat\0" // IR(49922, 10)
  "u_upsample\0" // IR(49932, 11)
  "s_upsample\0" // IR(49943, 11)
  "popcount\0" // IR(49954, 9)
  "s_mad24\0" // IR(49963, 8)
  "u_mad24\0" // IR(49971, 8)
  "s_mul24\0" // IR(49979, 8)
  "u_mul24\0" // IR(49987, 8)
  "vloadn\0" // IR(49995, 7)
  "vstoren\0" // IR(50002, 8)
  "vload_half\0" // IR(50010, 11)
  "vload_halfn\0" // IR(50021, 12)
  "vstore_half\0" // IR(50033, 12)
  "vstore_half_r\0" // IR(50045, 14)
  "vstore_halfn\0" // IR(50059, 13)
  "vstore_halfn_r\0" // IR(50072, 15)
  "vloada_halfn\0" // IR(50087, 13)
  "vstorea_halfn\0" // IR(50100, 14)
  "vstorea_halfn_r\0" // IR(50114, 16)
  "shuffle\0" // IR(50130, 8)
  "shuffle2\0" // IR(50138, 9)
  "printf\0" // IR(50147, 7)
  "prefetch\0" // IR(50154, 9)
  "bitselect\0" // IR(50163, 10)
  "select\0" // IR(50173, 7)
  "u_abs\0" // IR(50180, 6)
  "u_abs_diff\0" // IR(50186, 11)
  "u_mul_hi\0" // IR(50197, 9)
  "u_mad_hi\0" // IR(50206, 9)
  "CubeFaceIndexAMD\0" // IR(50215, 17)
  "CubeFaceCoordAMD\0" // IR(50232, 17)
  "TimeAMD\0" // IR(50249, 8)
  "SwizzleInvocationsAMD\0" // IR(50257, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(50279, 28)
  "WriteInvocationAMD\0" // IR(50307, 19)
  "MbcntAMD\0" // IR(50326, 9)
  "InterpolateAtVertexAMD\0" // IR(50335, 23)
  "FMin3AMD\0" // IR(50358, 9)
  "UMin3AMD\0" // IR(50367, 9)
  "SMin3AMD\0" // IR(50376, 9)
  "FMax3AMD\0" // IR(50385, 9)
  "UMax3AMD\0" // IR(50394, 9)
  "SMax3AMD\0" // IR(50403, 9)
  "FMid3AMD\0" // IR(50412, 9)
  "UMid3AMD\0" // IR(50421, 9)
  "SMid3AMD\0" // IR(50430, 9)
  "ARGMAX\0" // IR(50439, 7)
  "AVG_POOL2D\0" // IR(50446, 11)
  "CONV2D\0" // IR(50457, 7)
  "CONV3D\0" // IR(50464, 7)
  "DEPTHWISE_CONV2D\0" // IR(50471, 17)
  "FFT2D\0" // IR(50488, 6)
  "MATMUL\0" // IR(50494, 7)
  "MAX_POOL2D\0" // IR(50501, 11)
  "RFFT2D\0" // IR(50512, 7)
  "TRANSPOSE_CONV2D\0" // IR(50519, 17)
  "CLAMP\0" // IR(50536, 6)
  "ERF\0" // IR(50542, 4)
  "SIGMOID\0" // IR(50546, 8)
  "TANH\0" // IR(50554, 5)
  "ADD\0" // IR(50559, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(50563, 23)
  "BITWISE_AND\0" // IR(50586, 12)
  "BITWISE_OR\0" // IR(50598, 11)
  "BITWISE_XOR\0" // IR(50609, 12)
  "INTDIV\0" // IR(50621, 7)
  "LOGICAL_AND\0" // IR(50628, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(50640, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(50659, 20)
  "LOGICAL_OR\0" // IR(50679, 11)
  "LOGICAL_XOR\0" // IR(50690, 12)
  "MAXIMUM\0" // IR(50702, 8)
  "MINIMUM\0" // IR(50710, 8)
  "MUL\0" // IR(50718, 4)
  "POW\0" // IR(50722, 4)
  "SUB\0" // IR(50726, 4)
  "TABLE\0" // IR(50730, 6)
  "ABS\0" // IR(50736, 4)
  "BITWISE_NOT\0" // IR(50740, 12)
  "CEIL\0" // IR(50752, 5)
  "CLZ\0" // IR(50757, 4)
  "COS\0" // IR(50761, 4)
  "EXP\0" // IR(50765, 4)
  "FLOOR\0" // IR(50769, 6)
  "LOG\0" // IR(50775, 4)
  "LOGICAL_NOT\0" // IR(50779, 12)
  "NEGATE\0" // IR(50791, 7)
  "RECIPROCAL\0" // IR(50798, 11)
  "RSQRT\0" // IR(50809, 6)
  "SIN\0" // IR(50815, 4)
  "SELECT\0" // IR(50819, 7)
  "EQUAL\0" // IR(50826, 6)
  "GREATER\0" // IR(50832, 8)
  "GREATER_EQUAL\0" // IR(50840, 14)
  "REDUCE_ALL\0" // IR(50854, 11)
  "REDUCE_ANY\0" // IR(50865, 11)
  "REDUCE_MAX\0" // IR(50876, 11)
  "REDUCE_MIN\0" // IR(50887, 11)
  "REDUCE_PRODUCT\0" // IR(50898, 15)
  "REDUCE_SUM\0" // IR(50913, 11)
  "CONCAT\0" // IR(50924, 7)
  "PAD\0" // IR(50931, 4)
  "RESHAPE\0" // IR(50935, 8)
  "REVERSE\0" // IR(50943, 8)
  "SLICE\0" // IR(50951, 6)
  "TILE\0" // IR(50957, 5)
  "TRANSPOSE\0" // IR(50962, 10)
  "GATHER\0" // IR(50972, 7)
  "SCATTER\0" // IR(50979, 8)
  "RESIZE\0" // IR(50987, 7)
  "CAST\0" // IR(50994, 5)
  "RESCALE\0" // IR(50999, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(4763, 27), // 0 PhysicalStorageBuffer64EXT
  IR(5674, 18), // 1 SubgroupEqMaskKHR
  IR(5729, 18), // 2 SubgroupGeMaskKHR
  IR(5762, 18), // 3 SubgroupGtMaskKHR
  IR(5795, 18), // 4 SubgroupLeMaskKHR
  IR(5828, 18), // 5 SubgroupLtMaskKHR
  IR(6686, 12), // 6 BaryCoordNV
  IR(6741, 19), // 7 BaryCoordNoPerspNV
  IR(6772, 15), // 8 FragmentSizeNV
  IR(6829, 22), // 9 InvocationsPerPixelNV
  IR(6957, 11), // 10 LaunchIdNV
  IR(6982, 13), // 11 LaunchSizeNV
  IR(7013, 17), // 12 WorldRayOriginNV
  IR(7051, 20), // 13 WorldRayDirectionNV
  IR(7090, 18), // 14 ObjectRayOriginNV
  IR(7130, 21), // 15 ObjectRayDirectionNV
  IR(7162, 10), // 16 RayTminNV
  IR(7183, 10), // 17 RayTmaxNV
  IR(7216, 22), // 18 InstanceCustomIndexNV
  IR(7255, 16), // 19 ObjectToWorldNV
  IR(7288, 16), // 20 WorldToObjectNV
  IR(7322, 10), // 21 HitKindNV
  IR(7553, 19), // 22 IncomingRayFlagsNV
  IR(9329, 28), // 23 StorageUniformBufferBlock16
  IR(9392, 17), // 24 StorageUniform16
  IR(10207, 27), // 25 ShaderViewportIndexLayerNV
  IR(10251, 22), // 26 FragmentBarycentricNV
  IR(10304, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(10334, 14), // 28 ShadingRateNV
  IR(10394, 20), // 29 ShaderNonUniformEXT
  IR(10437, 26), // 30 RuntimeDescriptorArrayEXT
  IR(10499, 39), // 31 InputAttachmentArrayDynamicIndexingEXT
  IR(10577, 42), // 32 UniformTexelBufferArrayDynamicIndexingEXT
  IR(10658, 42), // 33 StorageTexelBufferArrayDynamicIndexingEXT
  IR(10737, 40), // 34 UniformBufferArrayNonUniformIndexingEXT
  IR(10813, 39), // 35 SampledImageArrayNonUniformIndexingEXT
  IR(10889, 40), // 36 StorageBufferArrayNonUniformIndexingEXT
  IR(10965, 39), // 37 StorageImageArrayNonUniformIndexingEXT
  IR(11043, 42), // 38 InputAttachmentArrayNonUniformIndexingEXT
  IR(11127, 45), // 39 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(11214, 45), // 40 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(11277, 21), // 41 VulkanMemoryModelKHR
  IR(11327, 32), // 42 VulkanMemoryModelDeviceScopeKHR
  IR(11359, 34), // 43 PhysicalStorageBufferAddressesEXT
  IR(11425, 31), // 44 ComputeDerivativeGroupLinearNV
  IR(11629, 28), // 45 DemoteToHelperInvocationEXT
  IR(13122, 22), // 46 DotProductInputAllKHR
  IR(13166, 25), // 47 DotProductInput4x8BitKHR
  IR(13219, 31), // 48 DotProductInput4x8BitPackedKHR
  IR(13261, 14), // 49 DotProductKHR
  IR(13433, 13), // 50 OptNoneINTEL
  IR(16504, 15), // 51 PerPrimitiveNV
  IR(16552, 12), // 52 PerVertexNV
  IR(16575, 14), // 53 NonUniformEXT
  IR(16605, 19), // 54 RestrictPointerEXT
  IR(16639, 18), // 55 AliasedPointerEXT
  IR(16949, 24), // 56 HlslCounterBufferGOOGLE
  IR(16986, 19), // 57 HlslSemanticGOOGLE
  IR(19677, 14), // 58 OutputLinesNV
  IR(19711, 19), // 59 OutputPrimitivesNV
  IR(19754, 23), // 60 DerivativeGroupQuadsNV
  IR(19802, 24), // 61 DerivativeGroupLinearNV
  IR(19845, 18), // 62 OutputTrianglesNV
  IR(20593, 16), // 63 RayGenerationNV
  IR(20625, 15), // 64 IntersectionNV
  IR(20650, 9), // 65 AnyHitNV
  IR(20673, 13), // 66 ClosestHitNV
  IR(20694, 7), // 67 MissNV
  IR(20713, 11), // 68 CallableNV
  IR(20858, 23), // 69 AllowContractFastINTEL
  IR(20894, 18), // 70 AllowReassocINTEL
  IR(21348, 22), // 71 MakeTexelAvailableKHR
  IR(21387, 20), // 72 MakeTexelVisibleKHR
  IR(21423, 19), // 73 NonPrivateTexelKHR
  IR(21456, 17), // 74 VolatileTexelKHR
  IR(22833, 24), // 75 MakePointerAvailableKHR
  IR(22876, 22), // 76 MakePointerVisibleKHR
  IR(22916, 21), // 77 NonPrivatePointerKHR
  IR(23003, 10), // 78 VulkanKHR
  IR(14276, 5), // 79 None
  IR(23186, 16), // 80 OutputMemoryKHR
  IR(23216, 17), // 81 MakeAvailableKHR
  IR(23245, 15), // 82 MakeVisibleKHR
  IR(23321, 28), // 83 PackedVectorFormat4x8BitKHR
  IR(23636, 24), // 84 SkipBuiltinPrimitivesNV
  IR(24376, 15), // 85 QueueFamilyKHR
  IR(24747, 15), // 86 CallableDataNV
  IR(24786, 23), // 87 IncomingCallableDataNV
  IR(24823, 13), // 88 RayPayloadNV
  IR(24852, 15), // 89 HitAttributeNV
  IR(24889, 21), // 90 IncomingRayPayloadNV
  IR(24932, 21), // 91 ShaderRecordBufferNV
  IR(24975, 25), // 92 PhysicalStorageBufferEXT
  IR(32316, 8), // 93 SDotKHR
  IR(32329, 8), // 94 UDotKHR
  IR(32343, 9), // 95 SUDotKHR
  IR(32363, 14), // 96 SDotAccSatKHR
  IR(32388, 14), // 97 UDotAccSatKHR
  IR(32414, 15), // 98 SUDotAccSatKHR
  IR(34994, 21), // 99 ReportIntersectionNV
  IR(35169, 28), // 100 TypeAccelerationStructureNV
  IR(35250, 23), // 101 RayQueryGetClusterIdNV
  IR(37253, 21), // 102 DecorateStringGOOGLE
  IR(37295, 27), // 103 MemberDecorateStringGOOGLE
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
  spv::Capability::Float8EXT, // 77
  spv::Capability::CooperativeMatrixKHR, // 78
  spv::Capability::WorkgroupMemoryExplicitLayoutKHR, // 79
  spv::Capability::StorageBuffer16BitAccess, // 80
  spv::Capability::VariablePointersStorageBuffer, // 81
  spv::Capability::AtomicStorage, // 82
  spv::Capability::StorageBuffer8BitAccess, // 83
  spv::Capability::RayQueryKHR, // 84
  spv::Capability::RayTracingKHR, // 85
  spv::Capability::CooperativeMatrixKHR, // 86
  spv::Capability::Int4TypeINTEL, // 87
  spv::Capability::CooperativeMatrixKHR, // 88
  spv::Capability::BFloat16TypeKHR, // 89
  spv::Capability::BFloat16TypeKHR, // 90
  spv::Capability::CooperativeMatrixKHR, // 91
  spv::Capability::MultiViewport, // 92
  spv::Capability::ShaderViewportIndexLayerEXT, // 93
  spv::Capability::ShaderViewportMaskNV, // 94
  spv::Capability::InputAttachment, // 95
  spv::Capability::ImageBuffer, // 96
  spv::Capability::ShaderNonUniform, // 97
  spv::Capability::InputAttachment, // 98
  spv::Capability::ShaderNonUniform, // 99
  spv::Capability::SampledBuffer, // 100
  spv::Capability::ShaderNonUniform, // 101
  spv::Capability::ImageBuffer, // 102
  spv::Capability::ShaderNonUniform, // 103
  spv::Capability::VectorAnyINTEL, // 104
  spv::Capability::Int8, // 105
  spv::Capability::FPGAClusterAttributesINTEL, // 106
  spv::Capability::FPGAKernelAttributesINTEL, // 107
  spv::Capability::Subgroup2DBlockIOINTEL, // 108
  spv::Capability::VariableLengthArrayINTEL, // 109
  spv::Capability::UntypedPointersKHR, // 110
  spv::Capability::SpecConditionalINTEL, // 111
  spv::Capability::Shader, // 112
  spv::Capability::Kernel, // 113
  spv::Capability::Shader, // 114
  spv::Capability::UniformDecoration, // 115
  spv::Capability::GeometryStreams, // 116
  spv::Capability::TransformFeedback, // 117
  spv::Capability::Kernel, // 118
  spv::Capability::FloatControls2, // 119
  spv::Capability::Linkage, // 120
  spv::Capability::Float8EXT, // 121
  spv::Capability::SampleMaskOverrideCoverageNV, // 122
  spv::Capability::GeometryShaderPassthroughNV, // 123
  spv::Capability::MeshShadingNV, // 124
  spv::Capability::MeshShadingEXT, // 125
  spv::Capability::ShaderInvocationReorderNV, // 126
  spv::Capability::BindlessTextureNV, // 127
  spv::Capability::VectorComputeINTEL, // 128
  spv::Capability::IndirectReferencesINTEL, // 129
  spv::Capability::AsmINTEL, // 130
  spv::Capability::FunctionFloatControlINTEL, // 131
  spv::Capability::FPGAMemoryAttributesINTEL, // 132
  spv::Capability::FPGAMemoryAccessesINTEL, // 133
  spv::Capability::LoopFuseINTEL, // 134
  spv::Capability::FPGADSPControlINTEL, // 135
  spv::Capability::MemoryAccessAliasingINTEL, // 136
  spv::Capability::FPGAInvocationPipeliningAttributesINTEL, // 137
  spv::Capability::FPGABufferLocationINTEL, // 138
  spv::Capability::IOPipesINTEL, // 139
  spv::Capability::FPGAClusterAttributesV2INTEL, // 140
  spv::Capability::FPMaxErrorINTEL, // 141
  spv::Capability::FPGALatencyControlINTEL, // 142
  spv::Capability::FPGAArgumentInterfacesINTEL, // 143
  spv::Capability::GlobalVariableHostAccessINTEL, // 144
  spv::Capability::GlobalVariableFPGADecorationsINTEL, // 145
  spv::Capability::CacheControlsINTEL, // 146
  spv::Capability::TileImageColorReadAccessEXT, // 147
  spv::Capability::Geometry, // 148
  spv::Capability::Tessellation, // 149
  spv::Capability::MeshShadingNV, // 150
  spv::Capability::MeshShadingEXT, // 151
  spv::Capability::Geometry, // 152
  spv::Capability::MeshShadingNV, // 153
  spv::Capability::MeshShadingEXT, // 154
  spv::Capability::SubgroupDispatch, // 155
  spv::Capability::TileImageDepthReadAccessEXT, // 156
  spv::Capability::TileImageStencilReadAccessEXT, // 157
  spv::Capability::SampleMaskPostDepthCoverage, // 158
  spv::Capability::DenormPreserve, // 159
  spv::Capability::DenormFlushToZero, // 160
  spv::Capability::SignedZeroInfNanPreserve, // 161
  spv::Capability::RoundingModeRTE, // 162
  spv::Capability::RoundingModeRTZ, // 163
  spv::Capability::QuadControlKHR, // 164
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 165
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 166
  spv::Capability::FragmentShaderPixelInterlockEXT, // 167
  spv::Capability::FragmentShaderSampleInterlockEXT, // 168
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 169
  spv::Capability::Shader64BitIndexingEXT, // 170
  spv::Capability::RoundToInfinityINTEL, // 171
  spv::Capability::KernelAttributesINTEL, // 172
  spv::Capability::FloatControls2, // 173
  spv::Capability::FPGAKernelAttributesv2INTEL, // 174
  spv::Capability::RegisterLimitsINTEL, // 175
  spv::Capability::FloatControls2, // 176
  spv::Capability::FPFastMathModeINTEL, // 177
  spv::Capability::OptNoneEXT, // 178
  spv::Capability::RuntimeAlignedAttributeINTEL, // 179
  spv::Capability::Kernel, // 180
  spv::Capability::GroupNonUniformArithmetic, // 181
  spv::Capability::GroupNonUniformBallot, // 182
  spv::Capability::GroupNonUniformClustered, // 183
  spv::Capability::GroupNonUniformPartitionedNV, // 184
  spv::Capability::ImageGatherExtended, // 185
  spv::Capability::MinLod, // 186
  spv::Capability::VulkanMemoryModel, // 187
  spv::Capability::FPGALoopControlsINTEL, // 188
  spv::Capability::ArbitraryPrecisionFixedPointINTEL, // 189
  spv::Capability::RawAccessChainsNV, // 190
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 191
  spv::Capability::RayTracingOpacityMicromapEXT, // 192
  spv::Capability::RayQueryKHR, // 193
  spv::Capability::StorageImageExtendedFormats, // 194
  spv::Capability::Int64ImageEXT, // 195
  spv::Capability::Shader, // 196
  spv::Capability::VectorComputeINTEL, // 197
  spv::Capability::GenericPointer, // 198
  spv::Capability::FunctionPointersINTEL, // 199
  spv::Capability::USMStorageClassesINTEL, // 200
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 201
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 202
  spv::Capability::TensorsARM, // 203
  spv::Capability::Addresses, // 204
  spv::Capability::PhysicalStorageBufferAddresses, // 205
  spv::Capability::LiteralSampler, // 206
  spv::Capability::Addresses, // 207
  spv::Capability::UntypedPointersKHR, // 208
  spv::Capability::Addresses, // 209
  spv::Capability::VariablePointers, // 210
  spv::Capability::VariablePointersStorageBuffer, // 211
  spv::Capability::PhysicalStorageBufferAddresses, // 212
  spv::Capability::Kernel, // 213
  spv::Capability::ImageQuery, // 214
  spv::Capability::ImageQuery, // 215
  spv::Capability::Shader, // 216
  spv::Capability::BitInstructions, // 217
  spv::Capability::DerivativeControl, // 218
  spv::Capability::Groups, // 219
  spv::Capability::SparseResidency, // 220
  spv::Capability::PipeStorage, // 221
  spv::Capability::NamedBarrier, // 222
  spv::Capability::GroupNonUniformVote, // 223
  spv::Capability::GroupNonUniformBallot, // 224
  spv::Capability::GroupNonUniformShuffle, // 225
  spv::Capability::GroupNonUniformShuffleRelative, // 226
  spv::Capability::GroupNonUniformArithmetic, // 227
  spv::Capability::GroupNonUniformClustered, // 228
  spv::Capability::GroupNonUniformPartitionedNV, // 229
  spv::Capability::GroupNonUniformQuad, // 230
  spv::Capability::Addresses, // 231
  spv::Capability::VariablePointers, // 232
  spv::Capability::VariablePointersStorageBuffer, // 233
  spv::Capability::GraphARM, // 234
  spv::Capability::UntypedPointersKHR, // 235
  spv::Capability::SubgroupBallotKHR, // 236
  spv::Capability::FMAKHR, // 237
  spv::Capability::SubgroupVoteKHR, // 238
  spv::Capability::GroupNonUniformRotateKHR, // 239
  spv::Capability::RayTracingKHR, // 240
  spv::Capability::RayQueryKHR, // 241
  spv::Capability::DotProduct, // 242
  spv::Capability::ReplicatedCompositesEXT, // 243
  spv::Capability::TextureSampleWeightedQCOM, // 244
  spv::Capability::TextureBoxFilterQCOM, // 245
  spv::Capability::TextureBlockMatchQCOM, // 246
  spv::Capability::CooperativeMatrixConversionQCOM, // 247
  spv::Capability::TextureBlockMatch2QCOM, // 248
  spv::Capability::FragmentMaskAMD, // 249
  spv::Capability::ShaderClockKHR, // 250
  spv::Capability::ShaderInvocationReorderNV, // 251
  spv::Capability::RayTracingMotionBlurNV, // 252
  spv::Capability::ImageFootprintNV, // 253
  spv::Capability::CooperativeVectorNV, // 254
  spv::Capability::CooperativeVectorTrainingNV, // 255
  spv::Capability::CooperativeMatrixConversionsNV, // 256
  spv::Capability::DisplacementMicromapNV, // 257
  spv::Capability::RayQueryPositionFetchKHR, // 258
  spv::Capability::RayTracingNV, // 259
  spv::Capability::RayTracingKHR, // 260
  spv::Capability::RayQueryKHR, // 261
  spv::Capability::DisplacementMicromapNV, // 262
  spv::Capability::CooperativeMatrixNV, // 263
  spv::Capability::FragmentShaderSampleInterlockEXT, // 264
  spv::Capability::FragmentShaderPixelInterlockEXT, // 265
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 266
  spv::Capability::CooperativeMatrixReductionsNV, // 267
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 268
  spv::Capability::TensorAddressingNV, // 269
  spv::Capability::DemoteToHelperInvocation, // 270
  spv::Capability::SubgroupShuffleINTEL, // 271
  spv::Capability::SubgroupBufferBlockIOINTEL, // 272
  spv::Capability::SubgroupImageBlockIOINTEL, // 273
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 274
  spv::Capability::IntegerFunctions2INTEL, // 275
  spv::Capability::AtomicFloat16MinMaxEXT, // 276
  spv::Capability::AtomicFloat32MinMaxEXT, // 277
  spv::Capability::AtomicFloat64MinMaxEXT, // 278
  spv::Capability::AtomicFloat16VectorNV, // 279
  spv::Capability::ExpectAssumeKHR, // 280
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 281
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 282
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 283
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 284
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 285
  spv::Capability::VariableLengthArrayINTEL, // 286
  spv::Capability::ArbitraryPrecisionFloatingPointINTEL, // 287
  spv::Capability::UnstructuredLoopControlsINTEL, // 288
  spv::Capability::BlockingPipesINTEL, // 289
  spv::Capability::FPGARegINTEL, // 290
  spv::Capability::AtomicFloat16AddEXT, // 291
  spv::Capability::AtomicFloat32AddEXT, // 292
  spv::Capability::AtomicFloat64AddEXT, // 293
  spv::Capability::AtomicFloat16VectorNV, // 294
  spv::Capability::LongCompositesINTEL, // 295
  spv::Capability::BFloat16ConversionINTEL, // 296
  spv::Capability::SplitBarrierINTEL, // 297
  spv::Capability::ArithmeticFenceEXT, // 298
  spv::Capability::TaskSequenceINTEL, // 299
  spv::Capability::SubgroupBufferPrefetchINTEL, // 300
  spv::Capability::Subgroup2DBlockTransformINTEL, // 301
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 302
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 303
  spv::Capability::TernaryBitwiseFunctionINTEL, // 304
  spv::Capability::UntypedVariableLengthArrayINTEL, // 305
  spv::Capability::FunctionVariantsINTEL, // 306
  spv::Capability::GroupUniformArithmeticKHR, // 307
  spv::Capability::TensorFloat32RoundingINTEL, // 308
  spv::Capability::MaskedGatherScatterINTEL, // 309
  spv::Capability::BindlessImagesINTEL, // 310
  spv::Capability::Float64, // 311
  spv::Capability::InterpolationFunction, // 312
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
  spvtools::Extension::kSPV_ARM_graph, // 37
  spvtools::Extension::kSPV_ARM_cooperative_matrix_layouts, // 38
  spvtools::Extension::kSPV_EXT_float8, // 39
  spvtools::Extension::kSPV_KHR_workgroup_memory_explicit_layout, // 40
  spvtools::Extension::kSPV_KHR_subgroup_vote, // 41
  spvtools::Extension::kSPV_KHR_16bit_storage, // 42
  spvtools::Extension::kSPV_KHR_variable_pointers, // 43
  spvtools::Extension::kSPV_KHR_shader_atomic_counter_ops, // 44
  spvtools::Extension::kSPV_KHR_post_depth_coverage, // 45
  spvtools::Extension::kSPV_KHR_8bit_storage, // 46
  spvtools::Extension::kSPV_KHR_float_controls, // 47
  spvtools::Extension::kSPV_KHR_ray_query, // 48
  spvtools::Extension::kSPV_KHR_untyped_pointers, // 49
  spvtools::Extension::kSPV_KHR_ray_query, // 50
  spvtools::Extension::kSPV_KHR_ray_tracing, // 51
  spvtools::Extension::kSPV_QCOM_image_processing, // 52
  spvtools::Extension::kSPV_QCOM_tile_shading, // 53
  spvtools::Extension::kSPV_QCOM_cooperative_matrix_conversion, // 54
  spvtools::Extension::kSPV_QCOM_image_processing2, // 55
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 56
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 57
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 58
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 59
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 60
  spvtools::Extension::kSPV_KHR_shader_clock, // 61
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 62
  spvtools::Extension::kSPV_KHR_quad_control, // 63
  spvtools::Extension::kSPV_INTEL_int4, // 64
  spvtools::Extension::kSPV_KHR_bfloat16, // 65
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 66
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 67
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 68
  spvtools::Extension::kSPV_NV_viewport_array2, // 69
  spvtools::Extension::kSPV_NV_viewport_array2, // 70
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 71
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 72
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 73
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 74
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 75
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 76
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 77
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 78
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 79
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 80
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 81
  spvtools::Extension::kSPV_NV_displacement_micromap, // 82
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 83
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 84
  spvtools::Extension::kSPV_NV_bindless_texture, // 85
  spvtools::Extension::kSPV_NV_cooperative_vector, // 86
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 87
  spvtools::Extension::kSPV_NV_raw_access_chains, // 88
  spvtools::Extension::kSPV_EXT_shader_64bit_indexing, // 89
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 90
  spvtools::Extension::kSPV_NV_tensor_addressing, // 91
  spvtools::Extension::kSPV_INTEL_subgroups, // 92
  spvtools::Extension::kSPV_INTEL_media_block_io, // 93
  spvtools::Extension::kSPV_INTEL_float_controls2, // 94
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 95
  spvtools::Extension::kSPV_INTEL_function_pointers, // 96
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 97
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 98
  spvtools::Extension::kSPV_INTEL_vector_compute, // 99
  spvtools::Extension::kSPV_KHR_expect_assume, // 100
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 101
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 102
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 103
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 104
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 105
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 106
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 107
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 108
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 109
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 110
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 111
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 112
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 113
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 114
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 115
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 116
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 117
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 118
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 119
  spvtools::Extension::kSPV_INTEL_io_pipes, // 120
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 121
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 122
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 123
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 124
  spvtools::Extension::kSPV_EXT_replicated_composites, // 125
  spvtools::Extension::kSPV_KHR_bit_instructions, // 126
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 127
  spvtools::Extension::kSPV_KHR_float_controls2, // 128
  spvtools::Extension::kSPV_KHR_fma, // 129
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 130
  spvtools::Extension::kSPV_INTEL_long_composites, // 131
  spvtools::Extension::kSPV_EXT_optnone, // 132
  spvtools::Extension::kSPV_INTEL_optnone, // 133
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 134
  spvtools::Extension::kSPV_INTEL_debug_module, // 135
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 136
  spvtools::Extension::kSPV_INTEL_split_barrier, // 137
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 138
  spvtools::Extension::kSPV_INTEL_task_sequence, // 139
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 140
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 141
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 142
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 143
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 144
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 145
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 146
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 147
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 148
  spvtools::Extension::kSPV_INTEL_function_variants, // 149
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 150
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 151
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 152
  spvtools::Extension::kSPV_INTEL_cache_controls, // 153
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 154
  spvtools::Extension::kSPV_INTEL_bindless_images, // 155
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 156
  spvtools::Extension::kSPV_NV_mesh_shader, // 157
  spvtools::Extension::kSPV_EXT_mesh_shader, // 158
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 159
  spvtools::Extension::kSPV_GOOGLE_user_type, // 160
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 161
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 162
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 163
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 164
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 165
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 166
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 167
  spvtools::Extension::kSPV_KHR_variable_pointers, // 168
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 169
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 170
  spvtools::Extension::kSPV_KHR_ray_tracing, // 171
  spvtools::Extension::kSPV_KHR_ray_query, // 172
  spvtools::Extension::kSPV_NV_ray_tracing, // 173
  spvtools::Extension::kSPV_KHR_ray_tracing, // 174
  spvtools::Extension::kSPV_KHR_ray_query, // 175
  spvtools::Extension::kSPV_NV_displacement_micromap, // 176
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 177
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 178
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
  SPV_OPERAND_TYPE_TYPE_ID, // 369
  SPV_OPERAND_TYPE_RESULT_ID, // 370
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 371
  SPV_OPERAND_TYPE_ID, // 372
  SPV_OPERAND_TYPE_LITERAL_STRING, // 373
  SPV_OPERAND_TYPE_VARIABLE_ID, // 374
  SPV_OPERAND_TYPE_ID, // 375
  SPV_OPERAND_TYPE_ID, // 376
  SPV_OPERAND_TYPE_VARIABLE_ID, // 377
  SPV_OPERAND_TYPE_RESULT_ID, // 378
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 379
  SPV_OPERAND_TYPE_VARIABLE_ID, // 380
  SPV_OPERAND_TYPE_RESULT_ID, // 381
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 382
  SPV_OPERAND_TYPE_TYPE_ID, // 383
  SPV_OPERAND_TYPE_RESULT_ID, // 384
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 385
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 386
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 387
  SPV_OPERAND_TYPE_TYPE_ID, // 388
  SPV_OPERAND_TYPE_RESULT_ID, // 389
  SPV_OPERAND_TYPE_ID, // 390
  SPV_OPERAND_TYPE_ID, // 391
  SPV_OPERAND_TYPE_ID, // 392
  SPV_OPERAND_TYPE_VARIABLE_ID, // 393
  SPV_OPERAND_TYPE_TYPE_ID, // 394
  SPV_OPERAND_TYPE_RESULT_ID, // 395
  SPV_OPERAND_TYPE_ID, // 396
  SPV_OPERAND_TYPE_ID, // 397
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 398
  SPV_OPERAND_TYPE_ID, // 399
  SPV_OPERAND_TYPE_ID, // 400
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 401
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 402
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 403
  SPV_OPERAND_TYPE_TYPE_ID, // 404
  SPV_OPERAND_TYPE_RESULT_ID, // 405
  SPV_OPERAND_TYPE_SCOPE_ID, // 406
  SPV_OPERAND_TYPE_ID, // 407
  SPV_OPERAND_TYPE_ID, // 408
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 409
  SPV_OPERAND_TYPE_TYPE_ID, // 410
  SPV_OPERAND_TYPE_RESULT_ID, // 411
  SPV_OPERAND_TYPE_ID, // 412
  SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER, // 413
  SPV_OPERAND_TYPE_VARIABLE_ID, // 414
  SPV_OPERAND_TYPE_TYPE_ID, // 415
  SPV_OPERAND_TYPE_RESULT_ID, // 416
  SPV_OPERAND_TYPE_ID, // 417
  SPV_OPERAND_TYPE_ID, // 418
  SPV_OPERAND_TYPE_ID, // 419
  SPV_OPERAND_TYPE_ID, // 420
  SPV_OPERAND_TYPE_ID, // 421
  SPV_OPERAND_TYPE_ID, // 422
  SPV_OPERAND_TYPE_ID, // 423
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 424
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 425
  SPV_OPERAND_TYPE_ID, // 426
  SPV_OPERAND_TYPE_ID, // 427
  SPV_OPERAND_TYPE_ID, // 428
  SPV_OPERAND_TYPE_ID, // 429
  SPV_OPERAND_TYPE_ID, // 430
  SPV_OPERAND_TYPE_ID, // 431
  SPV_OPERAND_TYPE_ID, // 432
  SPV_OPERAND_TYPE_ID, // 433
  SPV_OPERAND_TYPE_ID, // 434
  SPV_OPERAND_TYPE_ID, // 435
  SPV_OPERAND_TYPE_ID, // 436
  SPV_OPERAND_TYPE_TYPE_ID, // 437
  SPV_OPERAND_TYPE_RESULT_ID, // 438
  SPV_OPERAND_TYPE_ID, // 439
  SPV_OPERAND_TYPE_ID, // 440
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 441
  SPV_OPERAND_TYPE_TYPE_ID, // 442
  SPV_OPERAND_TYPE_RESULT_ID, // 443
  SPV_OPERAND_TYPE_ID, // 444
  SPV_OPERAND_TYPE_ID, // 445
  SPV_OPERAND_TYPE_ID, // 446
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 447
  SPV_OPERAND_TYPE_RESULT_ID, // 448
  SPV_OPERAND_TYPE_ID, // 449
  SPV_OPERAND_TYPE_SCOPE_ID, // 450
  SPV_OPERAND_TYPE_ID, // 451
  SPV_OPERAND_TYPE_ID, // 452
  SPV_OPERAND_TYPE_ID, // 453
  SPV_OPERAND_TYPE_TYPE_ID, // 454
  SPV_OPERAND_TYPE_RESULT_ID, // 455
  SPV_OPERAND_TYPE_ID, // 456
  SPV_OPERAND_TYPE_ID, // 457
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 458
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 459
  SPV_OPERAND_TYPE_ID, // 460
  SPV_OPERAND_TYPE_ID, // 461
  SPV_OPERAND_TYPE_ID, // 462
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 463
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 464
  SPV_OPERAND_TYPE_TYPE_ID, // 465
  SPV_OPERAND_TYPE_RESULT_ID, // 466
  SPV_OPERAND_TYPE_ID, // 467
  SPV_OPERAND_TYPE_ID, // 468
  SPV_OPERAND_TYPE_ID, // 469
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 470
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
  SPV_OPERAND_TYPE_ID, // 509
  SPV_OPERAND_TYPE_ID, // 510
  SPV_OPERAND_TYPE_ID, // 511
  SPV_OPERAND_TYPE_ID, // 512
  SPV_OPERAND_TYPE_ID, // 513
  SPV_OPERAND_TYPE_ID, // 514
  SPV_OPERAND_TYPE_ID, // 515
  SPV_OPERAND_TYPE_ID, // 516
  SPV_OPERAND_TYPE_ID, // 517
  SPV_OPERAND_TYPE_ID, // 518
  SPV_OPERAND_TYPE_ID, // 519
  SPV_OPERAND_TYPE_ID, // 520
  SPV_OPERAND_TYPE_ID, // 521
  SPV_OPERAND_TYPE_ID, // 522
  SPV_OPERAND_TYPE_ID, // 523
  SPV_OPERAND_TYPE_ID, // 524
  SPV_OPERAND_TYPE_ID, // 525
  SPV_OPERAND_TYPE_ID, // 526
  SPV_OPERAND_TYPE_ID, // 527
  SPV_OPERAND_TYPE_ID, // 528
  SPV_OPERAND_TYPE_ID, // 529
  SPV_OPERAND_TYPE_ID, // 530
  SPV_OPERAND_TYPE_ID, // 531
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 532
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 533
  SPV_OPERAND_TYPE_TYPE_ID, // 534
  SPV_OPERAND_TYPE_RESULT_ID, // 535
  SPV_OPERAND_TYPE_ID, // 536
  SPV_OPERAND_TYPE_ID, // 537
  SPV_OPERAND_TYPE_ID, // 538
  SPV_OPERAND_TYPE_ID, // 539
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 540
  SPV_OPERAND_TYPE_TYPE_ID, // 541
  SPV_OPERAND_TYPE_RESULT_ID, // 542
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
  SPV_OPERAND_TYPE_ID, // 557
  SPV_OPERAND_TYPE_ID, // 558
  SPV_OPERAND_TYPE_ID, // 559
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 560
  SPV_OPERAND_TYPE_TYPE_ID, // 561
  SPV_OPERAND_TYPE_RESULT_ID, // 562
  SPV_OPERAND_TYPE_ID, // 563
  SPV_OPERAND_TYPE_ID, // 564
  SPV_OPERAND_TYPE_ID, // 565
  SPV_OPERAND_TYPE_ID, // 566
  SPV_OPERAND_TYPE_ID, // 567
  SPV_OPERAND_TYPE_ID, // 568
  SPV_OPERAND_TYPE_ID, // 569
  SPV_OPERAND_TYPE_ID, // 570
  SPV_OPERAND_TYPE_ID, // 571
  SPV_OPERAND_TYPE_ID, // 572
  SPV_OPERAND_TYPE_ID, // 573
  SPV_OPERAND_TYPE_ID, // 574
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 575
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 576
  SPV_OPERAND_TYPE_ID, // 577
  SPV_OPERAND_TYPE_ID, // 578
  SPV_OPERAND_TYPE_ID, // 579
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 580
  SPV_OPERAND_TYPE_TYPE_ID, // 581
  SPV_OPERAND_TYPE_RESULT_ID, // 582
  SPV_OPERAND_TYPE_ID, // 583
  SPV_OPERAND_TYPE_ID, // 584
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 585
  SPV_OPERAND_TYPE_ID, // 586
  SPV_OPERAND_TYPE_ID, // 587
  SPV_OPERAND_TYPE_ID, // 588
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 589
  SPV_OPERAND_TYPE_RESULT_ID, // 590
  SPV_OPERAND_TYPE_ID, // 591
  SPV_OPERAND_TYPE_SCOPE_ID, // 592
  SPV_OPERAND_TYPE_ID, // 593
  SPV_OPERAND_TYPE_ID, // 594
  SPV_OPERAND_TYPE_TYPE_ID, // 595
  SPV_OPERAND_TYPE_RESULT_ID, // 596
  SPV_OPERAND_TYPE_ID, // 597
  SPV_OPERAND_TYPE_ID, // 598
  SPV_OPERAND_TYPE_ID, // 599
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 600
  SPV_OPERAND_TYPE_ID, // 601
  SPV_OPERAND_TYPE_ID, // 602
  SPV_OPERAND_TYPE_ID, // 603
  SPV_OPERAND_TYPE_ID, // 604
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 605
  SPV_OPERAND_TYPE_TYPE_ID, // 606
  SPV_OPERAND_TYPE_RESULT_ID, // 607
  SPV_OPERAND_TYPE_ID, // 608
  SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE, // 609
  SPV_OPERAND_TYPE_ID, // 610
  SPV_OPERAND_TYPE_TYPE_ID, // 611
  SPV_OPERAND_TYPE_RESULT_ID, // 612
  SPV_OPERAND_TYPE_ID, // 613
  SPV_OPERAND_TYPE_ID, // 614
  SPV_OPERAND_TYPE_ID, // 615
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 616
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 617
  SPV_OPERAND_TYPE_ID, // 618
  SPV_OPERAND_TYPE_ID, // 619
  SPV_OPERAND_TYPE_ID, // 620
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 621
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 622
  SPV_OPERAND_TYPE_RESULT_ID, // 623
  SPV_OPERAND_TYPE_ID, // 624
  SPV_OPERAND_TYPE_ID, // 625
  SPV_OPERAND_TYPE_VARIABLE_ID, // 626
  SPV_OPERAND_TYPE_TYPE_ID, // 627
  SPV_OPERAND_TYPE_RESULT_ID, // 628
  SPV_OPERAND_TYPE_ID, // 629
  SPV_OPERAND_TYPE_ID, // 630
  SPV_OPERAND_TYPE_ID, // 631
  SPV_OPERAND_TYPE_ID, // 632
  SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS, // 633
  SPV_OPERAND_TYPE_ID, // 634
  SPV_OPERAND_TYPE_ID, // 635
  SPV_OPERAND_TYPE_ID, // 636
  SPV_OPERAND_TYPE_ID, // 637
  SPV_OPERAND_TYPE_ID, // 638
  SPV_OPERAND_TYPE_TYPE_ID, // 639
  SPV_OPERAND_TYPE_RESULT_ID, // 640
  SPV_OPERAND_TYPE_ID, // 641
  SPV_OPERAND_TYPE_ID, // 642
  SPV_OPERAND_TYPE_LITERAL_STRING, // 643
  SPV_OPERAND_TYPE_LITERAL_STRING, // 644
  SPV_OPERAND_TYPE_TYPE_ID, // 645
  SPV_OPERAND_TYPE_RESULT_ID, // 646
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
  SPV_OPERAND_TYPE_ID, // 657
  SPV_OPERAND_TYPE_ID, // 658
  SPV_OPERAND_TYPE_ID, // 659
  SPV_OPERAND_TYPE_ID, // 660
  SPV_OPERAND_TYPE_ID, // 661
  SPV_OPERAND_TYPE_ID, // 662
  SPV_OPERAND_TYPE_ID, // 663
  SPV_OPERAND_TYPE_TYPE_ID, // 664
  SPV_OPERAND_TYPE_RESULT_ID, // 665
  SPV_OPERAND_TYPE_ID, // 666
  SPV_OPERAND_TYPE_ID, // 667
  SPV_OPERAND_TYPE_ID, // 668
  SPV_OPERAND_TYPE_ID, // 669
  SPV_OPERAND_TYPE_ID, // 670
  SPV_OPERAND_TYPE_ID, // 671
  SPV_OPERAND_TYPE_ID, // 672
  SPV_OPERAND_TYPE_ID, // 673
  SPV_OPERAND_TYPE_ID, // 674
  SPV_OPERAND_TYPE_ID, // 675
  SPV_OPERAND_TYPE_ID, // 676
  SPV_OPERAND_TYPE_TYPE_ID, // 677
  SPV_OPERAND_TYPE_RESULT_ID, // 678
  SPV_OPERAND_TYPE_ID, // 679
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 680
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 681
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 682
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 683
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 684
  SPV_OPERAND_TYPE_TYPE_ID, // 685
  SPV_OPERAND_TYPE_RESULT_ID, // 686
  SPV_OPERAND_TYPE_ID, // 687
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 688
  SPV_OPERAND_TYPE_ID, // 689
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 690
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 691
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 692
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 693
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 694
  SPV_OPERAND_TYPE_TYPE_ID, // 695
  SPV_OPERAND_TYPE_RESULT_ID, // 696
  SPV_OPERAND_TYPE_ID, // 697
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 698
  SPV_OPERAND_TYPE_ID, // 699
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 700
  SPV_OPERAND_TYPE_RESULT_ID, // 701
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 702
  SPV_OPERAND_TYPE_RESULT_ID, // 703
  SPV_OPERAND_TYPE_ID, // 704
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 705
  SPV_OPERAND_TYPE_VARIABLE_ID, // 706
  SPV_OPERAND_TYPE_TYPE_ID, // 707
  SPV_OPERAND_TYPE_RESULT_ID, // 708
  SPV_OPERAND_TYPE_ID, // 709
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 710
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 711
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 712
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 713
  SPV_OPERAND_TYPE_ID, // 714
  SPV_OPERAND_TYPE_ID, // 715
  SPV_OPERAND_TYPE_ID, // 716
  SPV_OPERAND_TYPE_ID, // 717
  SPV_OPERAND_TYPE_ID, // 718
  SPV_OPERAND_TYPE_ID, // 719
  SPV_OPERAND_TYPE_ID, // 720
  SPV_OPERAND_TYPE_ID, // 721
  SPV_OPERAND_TYPE_ID, // 722
  SPV_OPERAND_TYPE_ID, // 723
  SPV_OPERAND_TYPE_ID, // 724
  SPV_OPERAND_TYPE_ID, // 725
  SPV_OPERAND_TYPE_ID, // 726
  SPV_OPERAND_TYPE_ID, // 727
  SPV_OPERAND_TYPE_ID, // 728
  SPV_OPERAND_TYPE_ID, // 729
  SPV_OPERAND_TYPE_ID, // 730
  SPV_OPERAND_TYPE_ID, // 731
  SPV_OPERAND_TYPE_ID, // 732
  SPV_OPERAND_TYPE_TYPE_ID, // 733
  SPV_OPERAND_TYPE_RESULT_ID, // 734
  SPV_OPERAND_TYPE_ID, // 735
  SPV_OPERAND_TYPE_ID, // 736
  SPV_OPERAND_TYPE_ID, // 737
  SPV_OPERAND_TYPE_ID, // 738
  SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS, // 739
  SPV_OPERAND_TYPE_ID, // 740
  SPV_OPERAND_TYPE_EXECUTION_MODEL, // 741
  SPV_OPERAND_TYPE_ID, // 742
  SPV_OPERAND_TYPE_LITERAL_STRING, // 743
  SPV_OPERAND_TYPE_VARIABLE_ID, // 744
  SPV_OPERAND_TYPE_ID, // 745
  SPV_OPERAND_TYPE_CAPABILITY, // 746
  SPV_OPERAND_TYPE_TYPE_ID, // 747
  SPV_OPERAND_TYPE_RESULT_ID, // 748
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 749
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 750
  SPV_OPERAND_TYPE_TYPE_ID, // 751
  SPV_OPERAND_TYPE_RESULT_ID, // 752
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 753
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 754
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 755
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 756
  SPV_OPERAND_TYPE_TYPE_ID, // 757
  SPV_OPERAND_TYPE_RESULT_ID, // 758
  SPV_OPERAND_TYPE_VARIABLE_CAPABILITY, // 759
  SPV_OPERAND_TYPE_TYPE_ID, // 760
  SPV_OPERAND_TYPE_RESULT_ID, // 761
  SPV_OPERAND_TYPE_ID, // 762
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 763
  SPV_OPERAND_TYPE_ID, // 764
  SPV_OPERAND_TYPE_ID, // 765
  SPV_OPERAND_TYPE_ID, // 766
  SPV_OPERAND_TYPE_ID, // 767
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 768
  SPV_OPERAND_TYPE_ID, // 769
  SPV_OPERAND_TYPE_ID, // 770
  SPV_OPERAND_TYPE_ID, // 771
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 772
  SPV_OPERAND_TYPE_ID, // 773
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 774
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 775
  SPV_OPERAND_TYPE_ID, // 776
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 777
  SPV_OPERAND_TYPE_ID, // 778
  SPV_OPERAND_TYPE_ID, // 779
  SPV_OPERAND_TYPE_ID, // 780
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 781
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 782
  SPV_OPERAND_TYPE_ID, // 783
  SPV_OPERAND_TYPE_ID, // 784
  SPV_OPERAND_TYPE_ID, // 785
  SPV_OPERAND_TYPE_ID, // 786
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 787
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 788
  SPV_OPERAND_TYPE_ID, // 789
  SPV_OPERAND_TYPE_ID, // 790
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 791
  SPV_OPERAND_TYPE_VARIABLE_ID, // 792
  SPV_OPERAND_TYPE_ID, // 793
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 794
  SPV_OPERAND_TYPE_ID, // 795
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 796
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 797
  SPV_OPERAND_TYPE_ID, // 798
  SPV_OPERAND_TYPE_ID, // 799
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 800
  SPV_OPERAND_TYPE_VARIABLE_ID, // 801
  SPV_OPERAND_TYPE_ID, // 802
  SPV_OPERAND_TYPE_ID, // 803
  SPV_OPERAND_TYPE_ID, // 804
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 805
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 806
  SPV_OPERAND_TYPE_ID, // 807
  SPV_OPERAND_TYPE_ID, // 808
  SPV_OPERAND_TYPE_ID, // 809
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 810
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 811
  SPV_OPERAND_TYPE_ID, // 812
  SPV_OPERAND_TYPE_ID, // 813
  SPV_OPERAND_TYPE_ID, // 814
  SPV_OPERAND_TYPE_ID, // 815
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 816
  SPV_OPERAND_TYPE_ID, // 817
  SPV_OPERAND_TYPE_ID, // 818
  SPV_OPERAND_TYPE_ID, // 819
  SPV_OPERAND_TYPE_ID, // 820
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 821
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 822
  SPV_OPERAND_TYPE_ID, // 823
  SPV_OPERAND_TYPE_ID, // 824
  SPV_OPERAND_TYPE_ID, // 825
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 826
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 827
  SPV_OPERAND_TYPE_ID, // 828
  SPV_OPERAND_TYPE_ID, // 829
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 830
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 831
  SPV_OPERAND_TYPE_VARIABLE_ID, // 832
  SPV_OPERAND_TYPE_ID, // 833
  SPV_OPERAND_TYPE_ID, // 834
  SPV_OPERAND_TYPE_ID, // 835
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 836
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 837
  SPV_OPERAND_TYPE_ID, // 838
  SPV_OPERAND_TYPE_ID, // 839
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 840
  SPV_OPERAND_TYPE_ID, // 841
  SPV_OPERAND_TYPE_ID, // 842
  SPV_OPERAND_TYPE_ID, // 843
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 844
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 845
  SPV_OPERAND_TYPE_ID, // 846
  SPV_OPERAND_TYPE_ID, // 847
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 848
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 849
  SPV_OPERAND_TYPE_ID, // 850
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 851
  SPV_OPERAND_TYPE_ID, // 852
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 853
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 854
  SPV_OPERAND_TYPE_ID, // 855
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 856
  SPV_OPERAND_TYPE_ID, // 857
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 858
  SPV_OPERAND_TYPE_ID, // 859
  SPV_OPERAND_TYPE_ID, // 860
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 861
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 862
  SPV_OPERAND_TYPE_ID, // 863
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 864
  SPV_OPERAND_TYPE_ID, // 865
  SPV_OPERAND_TYPE_ID, // 866
  SPV_OPERAND_TYPE_ID, // 867
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 868
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 869
  SPV_OPERAND_TYPE_ID, // 870
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 871
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 872
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 873
  SPV_OPERAND_TYPE_ID, // 874
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 877
  SPV_OPERAND_TYPE_ID, // 878
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 879
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 880
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 881
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 882
  SPV_OPERAND_TYPE_ID, // 883
  SPV_OPERAND_TYPE_ID, // 884
  SPV_OPERAND_TYPE_ID, // 885
  SPV_OPERAND_TYPE_ID, // 886
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 887
  SPV_OPERAND_TYPE_ID, // 888
  SPV_OPERAND_TYPE_ID, // 889
  SPV_OPERAND_TYPE_ID, // 890
  SPV_OPERAND_TYPE_ID, // 891
  SPV_OPERAND_TYPE_ID, // 892
  SPV_OPERAND_TYPE_ID, // 893
  SPV_OPERAND_TYPE_ID, // 894
  SPV_OPERAND_TYPE_ID, // 895
  SPV_OPERAND_TYPE_VARIABLE_ID, // 896
  SPV_OPERAND_TYPE_ID, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_ID, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_ID, // 901
  SPV_OPERAND_TYPE_ID, // 902
  SPV_OPERAND_TYPE_ID, // 903
  SPV_OPERAND_TYPE_ID, // 904
  SPV_OPERAND_TYPE_ID, // 905
  SPV_OPERAND_TYPE_VARIABLE_ID, // 906
  SPV_OPERAND_TYPE_ID, // 907
  SPV_OPERAND_TYPE_ID, // 908
  SPV_OPERAND_TYPE_ID, // 909
  SPV_OPERAND_TYPE_ID, // 910
  SPV_OPERAND_TYPE_ID, // 911
  SPV_OPERAND_TYPE_ID, // 912
  SPV_OPERAND_TYPE_ID, // 913
  SPV_OPERAND_TYPE_ID, // 914
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 915
  SPV_OPERAND_TYPE_ID, // 916
  SPV_OPERAND_TYPE_ID, // 917
  SPV_OPERAND_TYPE_ID, // 918
  SPV_OPERAND_TYPE_ID, // 919
  SPV_OPERAND_TYPE_VARIABLE_ID, // 920
  SPV_OPERAND_TYPE_ID, // 921
  SPV_OPERAND_TYPE_ID, // 922
  SPV_OPERAND_TYPE_ID, // 923
  SPV_OPERAND_TYPE_ID, // 924
  SPV_OPERAND_TYPE_ID, // 925
  SPV_OPERAND_TYPE_ID, // 926
  SPV_OPERAND_TYPE_ID, // 927
  SPV_OPERAND_TYPE_ID, // 928
  SPV_OPERAND_TYPE_ID, // 929
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 930
  SPV_OPERAND_TYPE_ID, // 931
  SPV_OPERAND_TYPE_ID, // 932
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 933
  SPV_OPERAND_TYPE_ID, // 934
  SPV_OPERAND_TYPE_ID, // 935
  SPV_OPERAND_TYPE_ID, // 936
  SPV_OPERAND_TYPE_ID, // 937
  SPV_OPERAND_TYPE_ID, // 938
  SPV_OPERAND_TYPE_ID, // 939
  SPV_OPERAND_TYPE_ID, // 940
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 941
  SPV_OPERAND_TYPE_ID, // 942
  SPV_OPERAND_TYPE_ID, // 943
  SPV_OPERAND_TYPE_ID, // 944
  SPV_OPERAND_TYPE_VARIABLE_ID, // 945
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
  SPV_OPERAND_TYPE_ID, // 973
  SPV_OPERAND_TYPE_ID, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_ID, // 976
  SPV_OPERAND_TYPE_ID, // 977
  SPV_OPERAND_TYPE_ID, // 978
  SPV_OPERAND_TYPE_ID, // 979
  SPV_OPERAND_TYPE_ID, // 980
  SPV_OPERAND_TYPE_ID, // 981
  SPV_OPERAND_TYPE_ID, // 982
  SPV_OPERAND_TYPE_ID, // 983
  SPV_OPERAND_TYPE_ID, // 984
  SPV_OPERAND_TYPE_ID, // 985
  SPV_OPERAND_TYPE_ID, // 986
  SPV_OPERAND_TYPE_ID, // 987
  SPV_OPERAND_TYPE_ID, // 988
  SPV_OPERAND_TYPE_ID, // 989
  SPV_OPERAND_TYPE_ID, // 990
  SPV_OPERAND_TYPE_ID, // 991
  SPV_OPERAND_TYPE_ID, // 992
  SPV_OPERAND_TYPE_ID, // 993
  SPV_OPERAND_TYPE_ID, // 994
  SPV_OPERAND_TYPE_ID, // 995
  SPV_OPERAND_TYPE_ID, // 996
  SPV_OPERAND_TYPE_ID, // 997
  SPV_OPERAND_TYPE_ID, // 998
  SPV_OPERAND_TYPE_ID, // 999
  SPV_OPERAND_TYPE_ID, // 1000
  SPV_OPERAND_TYPE_ID, // 1001
  SPV_OPERAND_TYPE_ID, // 1002
  SPV_OPERAND_TYPE_ID, // 1003
  SPV_OPERAND_TYPE_ID, // 1004
  SPV_OPERAND_TYPE_ID, // 1005
  SPV_OPERAND_TYPE_ID, // 1006
  SPV_OPERAND_TYPE_ID, // 1007
  SPV_OPERAND_TYPE_ID, // 1008
  SPV_OPERAND_TYPE_ID, // 1009
  SPV_OPERAND_TYPE_ID, // 1010
  SPV_OPERAND_TYPE_ID, // 1011
  SPV_OPERAND_TYPE_ID, // 1012
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1013
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1014
  SPV_OPERAND_TYPE_ID, // 1015
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 1016
  SPV_OPERAND_TYPE_ID, // 1017
  SPV_OPERAND_TYPE_ID, // 1018
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 1019
  SPV_OPERAND_TYPE_ID, // 1020
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 1021
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1022
  SPV_OPERAND_TYPE_ID, // 1023
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 1024
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1025
  SPV_OPERAND_TYPE_ID, // 1026
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1027
  SPV_OPERAND_TYPE_ID, // 1028
  SPV_OPERAND_TYPE_ID, // 1029
  SPV_OPERAND_TYPE_ID, // 1030
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1031
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1032
  SPV_OPERAND_TYPE_ID, // 1033
  SPV_OPERAND_TYPE_ID, // 1034
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1035
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1036
  SPV_OPERAND_TYPE_ID, // 1037
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 1038
  SPV_OPERAND_TYPE_ID, // 1039
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1040
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1041
  SPV_OPERAND_TYPE_ID, // 1042
  SPV_OPERAND_TYPE_ID, // 1043
  SPV_OPERAND_TYPE_ID, // 1044
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1045
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1046
  SPV_OPERAND_TYPE_ID, // 1047
  SPV_OPERAND_TYPE_ID, // 1048
  SPV_OPERAND_TYPE_ID, // 1049
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1050
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1051
  SPV_OPERAND_TYPE_ID, // 1052
  SPV_OPERAND_TYPE_ID, // 1053
  SPV_OPERAND_TYPE_ID, // 1054
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1055
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1056
  SPV_OPERAND_TYPE_ID, // 1057
  SPV_OPERAND_TYPE_ID, // 1058
  SPV_OPERAND_TYPE_ID, // 1059
  SPV_OPERAND_TYPE_ID, // 1060
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1061
  SPV_OPERAND_TYPE_ID, // 1062
  SPV_OPERAND_TYPE_ID, // 1063
  SPV_OPERAND_TYPE_ID, // 1064
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1065
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1066
  SPV_OPERAND_TYPE_ID, // 1067
  SPV_OPERAND_TYPE_ID, // 1068
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1069
  SPV_OPERAND_TYPE_ID, // 1070
  SPV_OPERAND_TYPE_ID, // 1071
  SPV_OPERAND_TYPE_ID, // 1072
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1073
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1074
  SPV_OPERAND_TYPE_ID, // 1075
  SPV_OPERAND_TYPE_ID, // 1076
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1077
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1078
  SPV_OPERAND_TYPE_ID, // 1079
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1080
  SPV_OPERAND_TYPE_ID, // 1081
  SPV_OPERAND_TYPE_ID, // 1082
  SPV_OPERAND_TYPE_ID, // 1083
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1084
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1085
  SPV_OPERAND_TYPE_ID, // 1086
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1087
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1088
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1089
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1090
  SPV_OPERAND_TYPE_ID, // 1091
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1092
  SPV_OPERAND_TYPE_ID, // 1093
  SPV_OPERAND_TYPE_ID, // 1094
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1095
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1096
  SPV_OPERAND_TYPE_ID, // 1097
  SPV_OPERAND_TYPE_ID, // 1098
  SPV_OPERAND_TYPE_ID, // 1099
  SPV_OPERAND_TYPE_ID, // 1100
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1101
  SPV_OPERAND_TYPE_ID, // 1102
  SPV_OPERAND_TYPE_ID, // 1103
  SPV_OPERAND_TYPE_ID, // 1104
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1105
  SPV_OPERAND_TYPE_ID, // 1106
  SPV_OPERAND_TYPE_ID, // 1107
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1108
  SPV_OPERAND_TYPE_ID, // 1109
  SPV_OPERAND_TYPE_ID, // 1110
  SPV_OPERAND_TYPE_ID, // 1111
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1112
};
