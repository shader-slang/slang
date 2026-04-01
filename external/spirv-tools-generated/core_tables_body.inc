// Returns the name of an extension, as an index into kStrings
IndexRange ExtensionToIndexRange(Extension extension) {
  switch(extension) {
    case Extension::kSPV_ALTERA_arbitrary_precision_fixed_point: return IR(0, 43);
    case Extension::kSPV_ALTERA_arbitrary_precision_floating_point: return IR(43, 46);
    case Extension::kSPV_ALTERA_arbitrary_precision_integers: return IR(89, 40);
    case Extension::kSPV_ALTERA_blocking_pipes: return IR(129, 26);
    case Extension::kSPV_ALTERA_fpga_argument_interfaces: return IR(155, 36);
    case Extension::kSPV_ALTERA_fpga_buffer_location: return IR(191, 32);
    case Extension::kSPV_ALTERA_fpga_cluster_attributes: return IR(223, 35);
    case Extension::kSPV_ALTERA_fpga_dsp_control: return IR(258, 28);
    case Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes: return IR(286, 49);
    case Extension::kSPV_ALTERA_fpga_latency_control: return IR(335, 32);
    case Extension::kSPV_ALTERA_fpga_loop_controls: return IR(367, 30);
    case Extension::kSPV_ALTERA_fpga_memory_accesses: return IR(397, 32);
    case Extension::kSPV_ALTERA_fpga_memory_attributes: return IR(429, 34);
    case Extension::kSPV_ALTERA_fpga_reg: return IR(463, 20);
    case Extension::kSPV_ALTERA_global_variable_fpga_decorations: return IR(483, 44);
    case Extension::kSPV_ALTERA_io_pipes: return IR(527, 20);
    case Extension::kSPV_ALTERA_loop_fuse: return IR(547, 21);
    case Extension::kSPV_ALTERA_runtime_aligned: return IR(568, 27);
    case Extension::kSPV_ALTERA_task_sequence: return IR(595, 25);
    case Extension::kSPV_ALTERA_usm_storage_classes: return IR(620, 31);
    case Extension::kSPV_AMDX_shader_enqueue: return IR(651, 24);
    case Extension::kSPV_AMD_gcn_shader: return IR(675, 19);
    case Extension::kSPV_AMD_gpu_shader_half_float: return IR(694, 30);
    case Extension::kSPV_AMD_gpu_shader_half_float_fetch: return IR(724, 36);
    case Extension::kSPV_AMD_gpu_shader_int16: return IR(760, 25);
    case Extension::kSPV_AMD_shader_ballot: return IR(785, 22);
    case Extension::kSPV_AMD_shader_early_and_late_fragment_tests: return IR(807, 45);
    case Extension::kSPV_AMD_shader_explicit_vertex_parameter: return IR(852, 41);
    case Extension::kSPV_AMD_shader_fragment_mask: return IR(893, 29);
    case Extension::kSPV_AMD_shader_image_load_store_lod: return IR(922, 36);
    case Extension::kSPV_AMD_shader_trinary_minmax: return IR(958, 30);
    case Extension::kSPV_AMD_texture_gather_bias_lod: return IR(988, 32);
    case Extension::kSPV_ARM_cooperative_matrix_layouts: return IR(1020, 35);
    case Extension::kSPV_ARM_core_builtins: return IR(1055, 22);
    case Extension::kSPV_ARM_graph: return IR(1077, 14);
    case Extension::kSPV_ARM_tensors: return IR(1091, 16);
    case Extension::kSPV_EXT_arithmetic_fence: return IR(1107, 25);
    case Extension::kSPV_EXT_demote_to_helper_invocation: return IR(1132, 36);
    case Extension::kSPV_EXT_descriptor_heap: return IR(1168, 24);
    case Extension::kSPV_EXT_descriptor_indexing: return IR(1192, 28);
    case Extension::kSPV_EXT_float8: return IR(1220, 15);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(1235, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(1266, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(1302, 34);
    case Extension::kSPV_EXT_long_vector: return IR(1336, 20);
    case Extension::kSPV_EXT_mesh_shader: return IR(1356, 20);
    case Extension::kSPV_EXT_opacity_micromap: return IR(1376, 25);
    case Extension::kSPV_EXT_optnone: return IR(1401, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(1417, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(1449, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(1493, 30);
    case Extension::kSPV_EXT_shader_64bit_indexing: return IR(1523, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(1553, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(1587, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(1619, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(1655, 27);
    case Extension::kSPV_EXT_shader_invocation_reorder: return IR(1682, 34);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(1716, 30);
    case Extension::kSPV_EXT_shader_subgroup_partitioned: return IR(1746, 36);
    case Extension::kSPV_EXT_shader_tile_image: return IR(1782, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1808, 36);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1844, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1871, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1902, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1923, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(1945, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(1987, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(2032, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(2071, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(2101, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(2127, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(2152, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(2177, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(2200, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(2244, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(2270, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(2298, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(2321, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(2356, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(2387, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(2421, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(2448, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(2496, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(2527, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(2556, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(2587, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(2620, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(2639, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(2667, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(2695, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(2738, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2776, 26);
    case Extension::kSPV_INTEL_int4: return IR(2802, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2817, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2836, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2864, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2890, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2910, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2942, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2970, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2995, 33);
    case Extension::kSPV_INTEL_optnone: return IR(3028, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(3046, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(3072, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(3108, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3132, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3167, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3213, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3233, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3257, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3293, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3328, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3365, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3395, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3427, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3452, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3474, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(3495, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3512, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3537, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3572, 27);
    case Extension::kSPV_KHR_device_group: return IR(3599, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3620, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3642, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3665, 24);
    case Extension::kSPV_KHR_fma: return IR(3689, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3701, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3737, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3767, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3795, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3816, 30);
    case Extension::kSPV_KHR_multiview: return IR(3846, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3864, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3899, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3925, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(3957, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3979, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4007, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4028, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4050, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4068, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4088, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4123, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4160, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4194, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4216, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4237, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4268, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4305, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4329, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4367, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4389, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4418, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4453, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4478, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4504, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4532, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4573, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4611, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4635, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4673, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4707, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4733, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(4760, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(4786, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(4815, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(4850, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(4885, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4913, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(4932, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(4958, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4983, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5002, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5033, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5070, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5103, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5133, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5166, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5192, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5227, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5247, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5276, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5301, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5324, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5363, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5389, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(5416, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5438, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5476, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,189> kExtensionNames{{
    {IR(0, 43), static_cast<uint32_t>(kSPV_ALTERA_arbitrary_precision_fixed_point)},
    {IR(43, 46), static_cast<uint32_t>(kSPV_ALTERA_arbitrary_precision_floating_point)},
    {IR(89, 40), static_cast<uint32_t>(kSPV_ALTERA_arbitrary_precision_integers)},
    {IR(129, 26), static_cast<uint32_t>(kSPV_ALTERA_blocking_pipes)},
    {IR(155, 36), static_cast<uint32_t>(kSPV_ALTERA_fpga_argument_interfaces)},
    {IR(191, 32), static_cast<uint32_t>(kSPV_ALTERA_fpga_buffer_location)},
    {IR(223, 35), static_cast<uint32_t>(kSPV_ALTERA_fpga_cluster_attributes)},
    {IR(258, 28), static_cast<uint32_t>(kSPV_ALTERA_fpga_dsp_control)},
    {IR(286, 49), static_cast<uint32_t>(kSPV_ALTERA_fpga_invocation_pipelining_attributes)},
    {IR(335, 32), static_cast<uint32_t>(kSPV_ALTERA_fpga_latency_control)},
    {IR(367, 30), static_cast<uint32_t>(kSPV_ALTERA_fpga_loop_controls)},
    {IR(397, 32), static_cast<uint32_t>(kSPV_ALTERA_fpga_memory_accesses)},
    {IR(429, 34), static_cast<uint32_t>(kSPV_ALTERA_fpga_memory_attributes)},
    {IR(463, 20), static_cast<uint32_t>(kSPV_ALTERA_fpga_reg)},
    {IR(483, 44), static_cast<uint32_t>(kSPV_ALTERA_global_variable_fpga_decorations)},
    {IR(527, 20), static_cast<uint32_t>(kSPV_ALTERA_io_pipes)},
    {IR(547, 21), static_cast<uint32_t>(kSPV_ALTERA_loop_fuse)},
    {IR(568, 27), static_cast<uint32_t>(kSPV_ALTERA_runtime_aligned)},
    {IR(595, 25), static_cast<uint32_t>(kSPV_ALTERA_task_sequence)},
    {IR(620, 31), static_cast<uint32_t>(kSPV_ALTERA_usm_storage_classes)},
    {IR(651, 24), static_cast<uint32_t>(kSPV_AMDX_shader_enqueue)},
    {IR(675, 19), static_cast<uint32_t>(kSPV_AMD_gcn_shader)},
    {IR(694, 30), static_cast<uint32_t>(kSPV_AMD_gpu_shader_half_float)},
    {IR(724, 36), static_cast<uint32_t>(kSPV_AMD_gpu_shader_half_float_fetch)},
    {IR(760, 25), static_cast<uint32_t>(kSPV_AMD_gpu_shader_int16)},
    {IR(785, 22), static_cast<uint32_t>(kSPV_AMD_shader_ballot)},
    {IR(807, 45), static_cast<uint32_t>(kSPV_AMD_shader_early_and_late_fragment_tests)},
    {IR(852, 41), static_cast<uint32_t>(kSPV_AMD_shader_explicit_vertex_parameter)},
    {IR(893, 29), static_cast<uint32_t>(kSPV_AMD_shader_fragment_mask)},
    {IR(922, 36), static_cast<uint32_t>(kSPV_AMD_shader_image_load_store_lod)},
    {IR(958, 30), static_cast<uint32_t>(kSPV_AMD_shader_trinary_minmax)},
    {IR(988, 32), static_cast<uint32_t>(kSPV_AMD_texture_gather_bias_lod)},
    {IR(1020, 35), static_cast<uint32_t>(kSPV_ARM_cooperative_matrix_layouts)},
    {IR(1055, 22), static_cast<uint32_t>(kSPV_ARM_core_builtins)},
    {IR(1077, 14), static_cast<uint32_t>(kSPV_ARM_graph)},
    {IR(1091, 16), static_cast<uint32_t>(kSPV_ARM_tensors)},
    {IR(1107, 25), static_cast<uint32_t>(kSPV_EXT_arithmetic_fence)},
    {IR(1132, 36), static_cast<uint32_t>(kSPV_EXT_demote_to_helper_invocation)},
    {IR(1168, 24), static_cast<uint32_t>(kSPV_EXT_descriptor_heap)},
    {IR(1192, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(1220, 15), static_cast<uint32_t>(kSPV_EXT_float8)},
    {IR(1235, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(1266, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(1302, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(1336, 20), static_cast<uint32_t>(kSPV_EXT_long_vector)},
    {IR(1356, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(1376, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(1401, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(1417, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(1449, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(1493, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(1523, 30), static_cast<uint32_t>(kSPV_EXT_shader_64bit_indexing)},
    {IR(1553, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(1587, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(1619, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(1655, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(1682, 34), static_cast<uint32_t>(kSPV_EXT_shader_invocation_reorder)},
    {IR(1716, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(1746, 36), static_cast<uint32_t>(kSPV_EXT_shader_subgroup_partitioned)},
    {IR(1782, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1808, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1844, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1871, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1902, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1923, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(1945, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(1987, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(2032, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(2071, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(2101, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(2127, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(2152, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(2177, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(2200, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(2244, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(2270, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(2298, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(2321, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(2356, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(2387, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(2421, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(2448, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(2496, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(2527, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(2556, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(2587, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(2620, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(2639, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(2667, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(2695, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(2738, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2776, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2802, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2817, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2836, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2864, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2890, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2910, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2942, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2970, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2995, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(3028, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(3046, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(3072, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(3108, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3132, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3167, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3213, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3233, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3257, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3293, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3328, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3365, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3395, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3427, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3452, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3474, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3495, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3512, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3537, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3572, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3599, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3620, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3642, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3665, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3689, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3701, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3737, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3767, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3795, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3816, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3846, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3864, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3899, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3925, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3957, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(3979, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4007, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4028, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4050, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4068, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4088, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4123, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4160, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4194, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4216, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4237, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4268, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4305, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4329, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4367, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4389, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4418, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4453, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4478, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4504, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4532, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4573, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4611, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4635, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4673, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4707, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4733, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(4760, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(4786, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(4815, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(4850, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(4885, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4913, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4932, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(4958, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4983, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5002, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5033, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5070, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5103, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5133, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5166, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5192, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5227, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5247, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5276, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5301, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5324, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5363, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5389, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5416, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5438, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5476, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1209> kOperandsByValue{{
  {0,IR(0, 0),IR(5510, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5526, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5536, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5546, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5554, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5575, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(5586, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(5668, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(5684, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(5694, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(5707, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(5720, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(5729, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(5740, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(5830, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(5843, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(5889, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(5937, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(5952, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(5967, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(5977, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5991, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6001, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6012, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6024, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6051, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6066, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6077, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6087, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6104, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6118, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6132, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6144, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6162, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6181, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6202, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6210, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6221, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6243, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6256, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6271, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6318, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6334, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6347, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6368, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6379, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6405, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6417, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6431, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6457, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6470, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6483, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6493, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6506, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6561, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(6594, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(6627, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(6660, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(6693, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(6719, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(6732, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(6742, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(6789, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(6813, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(6833, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(6848, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(6879, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(6897, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(6915, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(6935, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(6963, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6989, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7008, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7035, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7060, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7082, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7117, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7164, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7180, 15)/* SamplerHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7213, 16)/* ResourceHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7229, 15)/* ViewportMaskNV */,IR(0, 0),IR(45, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7265, 20)/* SecondaryPositionNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7304, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7328, 18)/* PositionPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7366, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7388, 16)/* FullyCoveredEXT */,IR(0, 0),IR(50, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7428, 12)/* TaskCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7440, 17)/* PrimitiveCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7457, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7476, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7498, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7520, 15)/* LayerPerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7535, 16)/* MeshViewCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7551, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7569, 13)/* BaryCoordKHR */,IR(6, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(7617, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(7656, 12)/* FragSizeEXT */,IR(8, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(7702, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(7747, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(7772, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(7796, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(7824, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(7841, 12)/* LaunchIdKHR */,IR(10, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(7864, 14)/* LaunchSizeKHR */,IR(11, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(7891, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(7926, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(7967, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8004, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8047, 11)/* RayTminKHR */,IR(16, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8068, 11)/* RayTmaxKHR */,IR(17, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8089, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8134, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8167, 17)/* WorldToObjectKHR */,IR(20, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8200, 7)/* HitTNV */,IR(0, 0),IR(57, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8207, 11)/* HitKindKHR */,IR(21, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8228, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(58, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8268, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8325, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8392, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8429, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8468, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(61, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8488, 14)/* HitIsSphereNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8530, 11)/* HitIsLSSNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(8580, 20)/* HitSpherePositionNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(8600, 13)/* WarpsPerSMNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(8632, 10)/* SMCountNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(8642, 9)/* WarpIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(8651, 7)/* SMIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(8658, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(8676, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(8710, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(8743, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(8761, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(8775, 12)/* ClusterIDNV */,IR(0, 0),IR(65, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(8828, 12)/* CullMaskKHR */,IR(0, 0),IR(66, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(8855, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(5677, 7)/* Shader */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(5752, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(5761, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5565, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(8862, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5519, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(8870, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(8879, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(8893, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(8901, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(8909, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(8915, 13)/* Int64Atomics */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(8928, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(8939, 15)/* ImageReadWrite */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(8954, 12)/* ImageMipmap */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(8966, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(8972, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(8979, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(8993, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9008, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9022, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9028, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9050, 18)/* GeometryPointSize */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9068, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9088, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9112, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9146, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9179, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9213, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(5694, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(5707, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9246, 15)/* ImageCubeArray */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6033, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9278, 10)/* ImageRect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9288, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9300, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9315, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9320, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9336, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9352, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9359, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9369, 8)/* Image1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9261, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9377, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9391, 12)/* ImageBuffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9403, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9416, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9444, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9455, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9473, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9495, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9513, 16)/* GeometryStreams */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9529, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(9559, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(5903, 14)/* MultiViewport */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(9590, 17)/* SubgroupDispatch */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(9607, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(9620, 12)/* PipeStorage */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6284, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(9632, 20)/* GroupNonUniformVote */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(9652, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6539, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(9678, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(9701, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(9732, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(9757, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(5849, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(5917, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(9777, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6441, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(9795, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(9823, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(9851, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(9881, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(9892, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(9929, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(9969, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(9978, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10006, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10016, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(78, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(6766, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6300, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(6704, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10064, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10097, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(80, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10140, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(80, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10184, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10200, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10253, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(81, 1),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10305, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10327, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(6801, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(6823, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10348, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10378, 17)/* VariablePointers */,IR(0, 0),IR(82, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10395, 17)/* AtomicStorageOps */,IR(0, 0),IR(83, 1),IR(44, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10412, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10440, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10464, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(84, 1),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(10498, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(10519, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(10534, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(10552, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(10577, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(10593, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(10609, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(10632, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(10644, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(10663, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(85, 2),IR(50, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(5787, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(10695, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(10721, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(10742, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(6863, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(10764, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(87, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(10796, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(10819, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(10835, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(10857, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7100, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(10873, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(10894, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(10908, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7146, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(10923, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(10938, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(10952, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(88, 2),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(10979, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(10995, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(90, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11017, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(91, 2),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7195, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(93, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11046, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11062, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11091, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(70, 1),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(5861, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(94, 1),IR(70, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7244, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(95, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7285, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(96, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7346, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(73, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7404, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(5801, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11146, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(5815, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(7594, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11185, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(75, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(7683, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11260, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(77, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(11319, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(11356, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(11405, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(97, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(11480, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(74, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(11561, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(98, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(11642, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(99, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(11719, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(99, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(11794, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(99, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(11871, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(99, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(11946, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(100, 2),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12027, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(102, 2),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12114, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(104, 2),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8298, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(5774, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8245, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12201, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12240, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(5637, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(12335, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(75, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(12398, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(12423, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(12443, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(12476, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(8613, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(12514, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(12546, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(12599, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(12622, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(12651, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(61, 1),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(12677, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(61, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(12704, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(12722, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(12747, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(12767, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8359, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(61, 1),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(12789, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8502, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8541, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(12807, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(12827, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(12841, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(12864, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(12894, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(12925, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(12965, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13001, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13031, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(8787, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(61, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13059, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13078, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(13099, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(13126, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(13152, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(13183, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(13204, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(13227, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(13250, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(13272, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(13296, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(13305, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(13328, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(13351, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(13374, 19)/* VectorComputeINTEL */,IR(0, 0),IR(106, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(13393, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(13408, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(13424, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(13457, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(13495, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(13534, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(13559, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(13585, 27)/* FPGAMemoryAttributesALTERA */,IR(47, 1),IR(1, 0),IR(109, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(13638, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(13658, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(48, 1),IR(1, 0),IR(112, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(13723, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(49, 1),IR(1, 0),IR(114, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(13798, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(13828, 23)/* FPGALoopControlsALTERA */,IR(50, 1),IR(1, 0),IR(117, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(13873, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(13895, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(13921, 25)/* FPGAMemoryAccessesALTERA */,IR(51, 1),IR(1, 0),IR(120, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(13970, 28)/* FPGAClusterAttributesALTERA */,IR(52, 1),IR(1, 0),IR(122, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14025, 15)/* LoopFuseALTERA */,IR(53, 1),IR(1, 0),IR(124, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14054, 21)/* FPGADSPControlALTERA */,IR(54, 1),IR(1, 0),IR(126, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(14095, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(14121, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(55, 1),IR(1, 0),IR(129, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(14202, 25)/* FPGABufferLocationALTERA */,IR(56, 1),IR(1, 0),IR(131, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(14251, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(57, 1),IR(1, 0),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(14320, 24)/* USMStorageClassesALTERA */,IR(58, 1),IR(1, 0),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(14367, 30)/* RuntimeAlignedAttributeALTERA */,IR(59, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(14426, 14)/* IOPipesALTERA */,IR(60, 1),IR(1, 0),IR(139, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(14453, 20)/* BlockingPipesALTERA */,IR(61, 1),IR(1, 0),IR(141, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(14492, 14)/* FPGARegALTERA */,IR(62, 1),IR(1, 0),IR(143, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(14519, 19)/* DotProductInputAll */,IR(63, 1),IR(1, 0),IR(145, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(14560, 22)/* DotProductInput4x8Bit */,IR(64, 1),IR(107, 1),IR(145, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(14607, 28)/* DotProductInput4x8BitPacked */,IR(65, 1),IR(1, 0),IR(145, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(14666, 11)/* DotProduct */,IR(66, 1),IR(1, 0),IR(145, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(8840, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10043, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(14691, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(14715, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(148, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(14731, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(77, 1),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(14756, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(14771, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(14778, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(14798, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(14818, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(153, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(14838, 11)/* OptNoneEXT */,IR(67, 1),IR(1, 0),IR(154, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(14862, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(156, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(14882, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(157, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(14903, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(158, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(14927, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(14945, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(14964, 30)/* FPGAClusterAttributesV2ALTERA */,IR(68, 1),IR(108, 1),IR(122, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15023, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(109, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(15051, 19)/* TaskSequenceALTERA */,IR(69, 1),IR(1, 0),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(15088, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(15104, 25)/* FPGALatencyControlALTERA */,IR(70, 1),IR(1, 0),IR(164, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(15153, 29)/* FPGAArgumentInterfacesALTERA */,IR(71, 1),IR(1, 0),IR(166, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(15210, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(168, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(15240, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(72, 1),IR(1, 0),IR(169, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(15311, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(171, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(15339, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(15362, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(110, 1),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(15392, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(110, 1),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(15422, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(173, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(15460, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(174, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(15488, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(111, 2),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(15520, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(15541, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(113, 1),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(15563, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(176, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(15589, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(177, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(15616, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(15641, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(179, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(15660, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(180, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(15680, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(181, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(15700, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(114, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(15733, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(114, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(15766, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(90, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(15793, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(115, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(15825, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15837, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15845, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(15853, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15859, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15866, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15877, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15886, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15899, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15905, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15915, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15921, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(15936, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15961, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15977, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15991, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16004, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16016, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16033, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16045, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16060, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16073, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16088, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16106, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16123, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16144, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16164, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16184, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16200, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16216, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16236, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16260, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16266, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16271, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(16310, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(16321, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16330, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(16335, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(16342, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(16353, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(16360, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(16369, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16379, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16392, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(16405, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(16416, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(16426, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(16436, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(16446, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(16459, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(16473, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(16487, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(16501, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(16516, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(16532, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(16548, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(16564, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(16583, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(16604, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(16616, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(16628, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(16640, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(16655, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(16680, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16708, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(16716, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(16743, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(16770, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(16797, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(16829, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(16855, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(16859, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(16866, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(16870, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(16881, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(16892, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(16913, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(16924, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(16938, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(16959, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(15825, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15837, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15845, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15853, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15859, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15866, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15877, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(15886, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15899, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15905, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15915, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15961, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15977, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15991, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16004, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16016, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16033, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16045, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16060, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16073, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16088, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16106, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16123, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16144, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16164, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16184, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16260, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(16266, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(16271, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(16310, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(16321, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(16330, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(16335, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(16342, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(16353, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(16369, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16379, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16392, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(16977, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(16994, 7)/* SpecId */,IR(0, 0),IR(117, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(17001, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(17007, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(17019, 9)/* RowMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(17028, 9)/* ColMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(17037, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(17049, 13)/* MatrixStride */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(17062, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(17073, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(17084, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(17118, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(17126, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(17140, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(17145, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(17151, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(17160, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(17167, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(17177, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(17186, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(17194, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(17203, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(17212, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(17221, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(17233, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(17245, 8)/* Uniform */,IR(0, 0),IR(119, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(17279, 10)/* UniformId */,IR(0, 0),IR(119, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(17289, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(17309, 7)/* Stream */,IR(0, 0),IR(121, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(17316, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(17325, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(17335, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(17341, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(17349, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(17363, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(17370, 10)/* XfbBuffer */,IR(0, 0),IR(122, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(17380, 10)/* XfbStride */,IR(0, 0),IR(122, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(17436, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(17484, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(17534, 15)/* FPFastMathMode */,IR(0, 0),IR(123, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(17611, 18)/* LinkageAttributes */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(17629, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(17643, 21)/* InputAttachmentIndex */,IR(0, 0),IR(97, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(17664, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(17674, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(17708, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(17720, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(17736, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(115, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(17780, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(183, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(17793, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(183, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(17808, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(17826, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(17848, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(17870, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(17888, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(17920, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(17940, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(17963, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(17983, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(18008, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(18035, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(18060, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(18088, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(18105, 12)/* OffsetIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(18117, 19)/* OverrideCoverageNV */,IR(0, 0),IR(126, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(18136, 14)/* PassthroughNV */,IR(0, 0),IR(127, 1),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(18150, 19)/* ViewportRelativeNV */,IR(0, 0),IR(96, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(18169, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(18197, 16)/* PerPrimitiveEXT */,IR(73, 1),IR(128, 2),IR(184, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(18228, 10)/* PerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(18238, 10)/* PerTaskNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(18248, 13)/* PerVertexKHR */,IR(74, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(18273, 11)/* NonUniform */,IR(75, 1),IR(99, 1),IR(79, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(18298, 16)/* RestrictPointer */,IR(76, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(18333, 15)/* AliasedPointer */,IR(77, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(18366, 15)/* MemberOffsetNV */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(18381, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(18411, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(18442, 7)/* BankNV */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(18449, 18)/* BindlessSamplerNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(18467, 16)/* BindlessImageNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(18483, 15)/* BoundSamplerNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(18498, 13)/* BoundImageNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(18511, 14)/* SIMTCallINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(18525, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(135, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(18551, 13)/* ClobberINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(18564, 17)/* SideEffectsINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(18581, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(18608, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(18629, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(18656, 15)/* StackCallINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(18671, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(18697, 14)/* CounterBuffer */,IR(78, 1),IR(1, 0),IR(186, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(18735, 13)/* UserSemantic */,IR(79, 1),IR(1, 0),IR(186, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(18767, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(18782, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(18839, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(18863, 15)/* RegisterALTERA */,IR(80, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(18892, 13)/* MemoryALTERA */,IR(81, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(18917, 15)/* NumbanksALTERA */,IR(82, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(18946, 16)/* BankwidthALTERA */,IR(83, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(18977, 23)/* MaxPrivateCopiesALTERA */,IR(84, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(19022, 17)/* SinglepumpALTERA */,IR(85, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(19055, 17)/* DoublepumpALTERA */,IR(86, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(19088, 20)/* MaxReplicatesALTERA */,IR(87, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(19127, 21)/* SimpleDualPortALTERA */,IR(88, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(19168, 12)/* MergeALTERA */,IR(89, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(19233, 15)/* BankBitsALTERA */,IR(90, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(19262, 21)/* ForcePow2DepthALTERA */,IR(91, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(19303, 17)/* StridesizeALTERA */,IR(92, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(19336, 15)/* WordsizeALTERA */,IR(93, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(19365, 19)/* TrueDualPortALTERA */,IR(94, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(19402, 20)/* BurstCoalesceALTERA */,IR(95, 1),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(19441, 16)/* CacheSizeALTERA */,IR(96, 1),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(19472, 29)/* DontStaticallyCoalesceALTERA */,IR(97, 1),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(19529, 15)/* PrefetchALTERA */,IR(98, 1),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(19558, 18)/* StallEnableALTERA */,IR(99, 1),IR(108, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(19593, 26)/* FuseLoopsInFunctionALTERA */,IR(100, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(19644, 20)/* MathOpDSPModeALTERA */,IR(101, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(19683, 16)/* AliasScopeINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(19699, 13)/* NoAliasINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(19712, 25)/* InitiationIntervalALTERA */,IR(102, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(19761, 21)/* MaxConcurrencyALTERA */,IR(103, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(19802, 21)/* PipelineEnableALTERA */,IR(104, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(19843, 21)/* BufferLocationALTERA */,IR(105, 1),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(19884, 20)/* IOPipeStorageALTERA */,IR(106, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(19957, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(19988, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(20013, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(20048, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(20066, 16)/* StallFreeALTERA */,IR(107, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(20128, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(20154, 26)/* LatencyControlLabelALTERA */,IR(108, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(20205, 31)/* LatencyControlConstraintALTERA */,IR(109, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(20266, 28)/* ConduitKernelArgumentALTERA */,IR(110, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(20321, 32)/* RegisterMapKernelArgumentALTERA */,IR(111, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(20384, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(112, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(20451, 31)/* MMHostInterfaceDataWidthALTERA */,IR(113, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(20512, 29)/* MMHostInterfaceLatencyALTERA */,IR(114, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(20603, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(115, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(20672, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(116, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(20731, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(117, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(20796, 27)/* StableKernelArgumentALTERA */,IR(118, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(20888, 16)/* HostAccessINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(20951, 15)/* InitModeALTERA */,IR(119, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(20980, 29)/* ImplementInRegisterMapALTERA */,IR(120, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(21037, 17)/* ConditionalINTEL */,IR(0, 0),IR(113, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(21090, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(21149, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(21172, 3)/* 1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(21175, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(21178, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(21181, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(21186, 5)/* Rect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(21191, 7)/* Buffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(21198, 12)/* SubpassData */,IR(0, 0),IR(97, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(21210, 17)/* TileImageDataEXT */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(21227, 12)/* Invocations */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(21239, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(21252, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(21274, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(21295, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(21309, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(21324, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(21343, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(21359, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(21375, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(21394, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(21404, 4)/* Xfb */,IR(0, 0),IR(122, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(21408, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(21423, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(21436, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(21446, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(21461, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(21471, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(21485, 12)/* InputPoints */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(21497, 11)/* InputLines */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(21508, 20)/* InputLinesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(21528, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(21538, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(21562, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(21568, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(21577, 15)/* OutputVertices */,IR(0, 0),IR(154, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(21592, 13)/* OutputPoints */,IR(0, 0),IR(158, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(21605, 16)/* OutputLineStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(21621, 20)/* OutputTriangleStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(21641, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(21653, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(21668, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(21680, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6271, 13)/* SubgroupSize */,IR(0, 0),IR(161, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(21690, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(161, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(21712, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(161, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(21736, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(21748, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(21764, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(21798, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(162, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(21832, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(163, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(21868, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(21898, 18)/* PostDepthCoverage */,IR(0, 0),IR(164, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(10519, 15)/* DenormPreserve */,IR(0, 0),IR(165, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(10534, 18)/* DenormFlushToZero */,IR(0, 0),IR(166, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(10552, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(167, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(10577, 16)/* RoundingModeRTE */,IR(0, 0),IR(168, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(10593, 16)/* RoundingModeRTZ */,IR(0, 0),IR(169, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(21916, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(21950, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(21970, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(21999, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(22022, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(22037, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(22052, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(22073, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7164, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(22097, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(22118, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(22146, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(22172, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(22195, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(22222, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(22247, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(190, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(22269, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(22288, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(22308, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(22328, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(22348, 15)/* OutputLinesEXT */,IR(121, 1),IR(128, 2),IR(184, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(22377, 20)/* OutputPrimitivesEXT */,IR(122, 1),IR(128, 2),IR(184, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(22416, 24)/* DerivativeGroupQuadsKHR */,IR(123, 1),IR(172, 1),IR(75, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(22463, 25)/* DerivativeGroupLinearKHR */,IR(124, 1),IR(173, 1),IR(75, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(22512, 19)/* OutputTrianglesEXT */,IR(125, 1),IR(128, 2),IR(184, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(22549, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(174, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(22574, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(174, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(22601, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(175, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(22627, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(175, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(22655, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(176, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(22686, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(176, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(12841, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(177, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(22719, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(22746, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(22767, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(22788, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(22814, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(22841, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(179, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(22863, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(179, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(22879, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(179, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(22899, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(109, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(22921, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(109, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(22949, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(22973, 18)/* FPFastMathDefault */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(22991, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(109, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(23015, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(23041, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(23064, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(23086, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(23161, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(23188, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(23195, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(23215, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(5752, 9)/* Geometry */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(16360, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(23238, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5519, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(23248, 7)/* TaskNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(23255, 7)/* MeshNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(23262, 17)/* RayGenerationKHR */,IR(126, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(23295, 16)/* IntersectionKHR */,IR(127, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(23326, 10)/* AnyHitKHR */,IR(128, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(23345, 14)/* ClosestHitKHR */,IR(129, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(23372, 8)/* MissKHR */,IR(130, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(23387, 12)/* CallableKHR */,IR(131, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(23410, 8)/* TaskEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(23418, 8)/* MeshEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(23426, 9)/* Preserve */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(23435, 12)/* FlushToZero */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(23447, 12)/* BFloat16KHR */,IR(0, 0),IR(90, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(23459, 14)/* Float8E4M3EXT */,IR(0, 0),IR(115, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(23473, 14)/* Float8E5M2EXT */,IR(0, 0),IR(115, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(23487, 5)/* IEEE */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(23492, 4)/* ALT */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(23496, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(23503, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(23510, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(23514, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(23525, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(23530, 14)/* AllowContract */,IR(132, 1),IR(183, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(23567, 13)/* AllowReassoc */,IR(133, 1),IR(183, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(23598, 15)/* AllowTransform */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(23613, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23617, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(23621, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(23625, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23629, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(23645, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(23661, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(23679, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(23697, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(23704, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(23715, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(23720, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(14838, 11)/* OptNoneEXT */,IR(67, 1),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(23726, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(23731, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(23736, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(23742, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(23747, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(23755, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(23765, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(23773, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(23785, 21)/* RuntimeAlignedALTERA */,IR(134, 1),IR(186, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(23826, 7)/* Reduce */,IR(0, 0),IR(187, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(23833, 14)/* InclusiveScan */,IR(0, 0),IR(187, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(23847, 14)/* ExclusiveScan */,IR(0, 0),IR(187, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(23861, 16)/* ClusteredReduce */,IR(0, 0),IR(190, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(23877, 21)/* PartitionedReduceEXT */,IR(135, 1),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(23918, 28)/* PartitionedInclusiveScanEXT */,IR(136, 1),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(23973, 28)/* PartitionedExclusiveScanEXT */,IR(137, 1),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(24028, 10)/* NoneINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(24038, 10)/* ReadINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(24048, 11)/* WriteINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(24059, 15)/* ReadWriteINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(24074, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(24079, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(24083, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(24088, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(17363, 7)/* Offset */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(24100, 13)/* ConstOffsets */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(17160, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9352, 7)/* MinLod */,IR(0, 0),IR(193, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(24113, 19)/* MakeTexelAvailable */,IR(138, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(24154, 17)/* MakeTexelVisible */,IR(139, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(24191, 16)/* NonPrivateTexel */,IR(140, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(24226, 14)/* VolatileTexel */,IR(141, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(24257, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(24268, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(24279, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(24291, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(24299, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(24309, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(24320, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(24330, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(24341, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(24355, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(24369, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(24384, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(24395, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(24407, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(24419, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(24432, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(24446, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(24460, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(15853, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(24470, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(24481, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(24498, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(24514, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(24534, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(24554, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(24574, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(24593, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(24612, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(24631, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(24647, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(24663, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(24665, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(24667, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(24670, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(24673, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(24677, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(24682, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(24687, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(24692, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(24702, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(24712, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(24715, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(24719, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(24724, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(24730, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(24743, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(24748, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(24754, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(24760, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(24766, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(24771, 28)/* InitOnDeviceReprogramALTERA */,IR(142, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(24826, 24)/* InitOnDeviceResetALTERA */,IR(143, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(24873, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(24880, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(24891, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(24905, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(24917, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(24930, 7)/* Export */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(24937, 7)/* Import */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(24944, 12)/* LinkOnceODR */,IR(0, 0),IR(125, 1),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(24956, 14)/* UncachedINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(24970, 12)/* CachedINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(24982, 15)/* StreamingINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(24997, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(25022, 17)/* ConstCachedINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(25039, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(25046, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(25057, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(25076, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(25093, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(25107, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(25121, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(25139, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(25149, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(19712, 25)/* InitiationIntervalALTERA */,IR(102, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(19761, 21)/* MaxConcurrencyALTERA */,IR(103, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(25162, 22)/* DependencyArrayALTERA */,IR(144, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(19802, 21)/* PipelineEnableALTERA */,IR(104, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(25205, 19)/* LoopCoalesceALTERA */,IR(145, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(25242, 22)/* MaxInterleavingALTERA */,IR(146, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(25285, 27)/* SpeculatedIterationsALTERA */,IR(147, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(25338, 15)/* NoFusionALTERA */,IR(148, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(25367, 16)/* LoopCountALTERA */,IR(149, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(25398, 27)/* MaxReinvocationDelayALTERA */,IR(150, 1),IR(195, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(25451, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(25480, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(25509, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(25530, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(25556, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(25579, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(25602, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(25625, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(25648, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(25665, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(25682, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(25708, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(25734, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(25761, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(17194, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(25788, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(24279, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(25796, 21)/* MakePointerAvailable */,IR(151, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(25841, 19)/* MakePointerVisible */,IR(152, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(25882, 18)/* NonPrivatePointer */,IR(153, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(25921, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(142, 1),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(25941, 17)/* NoAliasINTELMask */,IR(0, 0),IR(142, 1),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(25958, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(25965, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(25973, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(25980, 7)/* Vulkan */,IR(154, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(25997, 8)/* Relaxed */,IR(155, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(26005, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(26013, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(26021, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(26036, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(26059, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(26073, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(26088, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(26104, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(26125, 20)/* AtomicCounterMemory */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(26145, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(26157, 13)/* OutputMemory */,IR(156, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(26186, 14)/* MakeAvailable */,IR(157, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(26217, 12)/* MakeVisible */,IR(158, 1),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(17194, 9)/* Volatile */,IR(0, 0),IR(194, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(26244, 10)/* AutoINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(26254, 5)/* WRAP */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(26259, 4)/* SAT */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(26263, 9)/* SAT_ZERO */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(26272, 8)/* SAT_SYM */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(26280, 25)/* PackedVectorFormat4x8Bit */,IR(159, 1),IR(1, 0),IR(145, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(26333, 4)/* TRN */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(26337, 9)/* TRN_ZERO */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(26346, 4)/* RND */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(26350, 9)/* RND_ZERO */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(26359, 8)/* RND_INF */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(26367, 12)/* RND_MIN_INF */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(26379, 9)/* RND_CONV */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(26388, 13)/* RND_CONV_ODD */,IR(0, 0),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(26401, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(26426, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(16708, 8)/* NoneKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(26449, 10)/* OpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(26459, 12)/* NoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(26471, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(26494, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(26518, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(26545, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(26573, 14)/* CullOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(26587, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(26603, 17)/* SkipTrianglesKHR */,IR(160, 1),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(26644, 13)/* SkipAABBsKHR */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(26657, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(26687, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(26728, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(26765, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(26802, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(26843, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(26885, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(26918, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(26951, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(26963, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(26969, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(26976, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(26991, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(26999, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(27006, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(27014, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(27022, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(27030, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(27035, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(27041, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(27052, 6)/* Rg32f */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(27058, 6)/* Rg16f */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(27064, 13)/* R11fG11fB10f */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(27077, 5)/* R16f */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(27082, 7)/* Rgba16 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(27089, 8)/* Rgb10A2 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(27097, 5)/* Rg16 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(27102, 4)/* Rg8 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(27106, 4)/* R16 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(27110, 3)/* R8 */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(27113, 12)/* Rgba16Snorm */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(27125, 10)/* Rg16Snorm */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(27135, 9)/* Rg8Snorm */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(27144, 9)/* R16Snorm */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(27153, 8)/* R8Snorm */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(27161, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(27169, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(27177, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(27184, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(27189, 6)/* Rg32i */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(27195, 6)/* Rg16i */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(27201, 5)/* Rg8i */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(27206, 5)/* R16i */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(27211, 4)/* R8i */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(27215, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(27224, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(27233, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(27241, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(27247, 10)/* Rgb10a2ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(27257, 7)/* Rg32ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(27264, 7)/* Rg16ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(27271, 6)/* Rg8ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(27277, 6)/* R16ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(27283, 5)/* R8ui */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(27288, 6)/* R64ui */,IR(0, 0),IR(202, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(27294, 5)/* R64i */,IR(0, 0),IR(202, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(27299, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(27311, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(27318, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(27328, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(27337, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(27348, 12)/* QueueFamily */,IR(161, 1),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(27375, 14)/* ShaderCallKHR */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(27389, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(27397, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(27409, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(15825, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15837, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15845, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(15853, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15859, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15866, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15877, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15886, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15899, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15905, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15915, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15921, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(15936, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15961, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15977, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15991, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16004, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16016, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16033, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16045, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16060, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16073, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16088, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16106, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16123, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16144, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16164, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16184, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16200, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16216, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16236, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(27438, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16260, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16266, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16271, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(16310, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(16321, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16330, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(16335, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(16342, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(16353, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(16360, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(16369, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16379, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16392, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(16405, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(27006, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(27464, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(27469, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(27474, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(27483, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(27494, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(27499, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(27514, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(27519, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(27526, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(27531, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(27536, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(27542, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(27546, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(27551, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(27567, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(17245, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(27573, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(27318, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(27580, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(27595, 8)/* Private */,IR(0, 0),IR(203, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(27603, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(27612, 8)/* Generic */,IR(0, 0),IR(205, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(27620, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(27633, 14)/* AtomicCounter */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(27647, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(27653, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(194, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(27667, 13)/* TileImageEXT */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(27680, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(27699, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(27715, 16)/* CallableDataKHR */,IR(162, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(27746, 24)/* IncomingCallableDataKHR */,IR(163, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(27793, 14)/* RayPayloadKHR */,IR(164, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(27820, 16)/* HitAttributeKHR */,IR(165, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(27851, 22)/* IncomingRayPayloadKHR */,IR(166, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(27894, 22)/* ShaderRecordBufferKHR */,IR(167, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(27937, 22)/* PhysicalStorageBuffer */,IR(168, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(27984, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(28005, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(54, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(28029, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(28051, 17)/* CodeSectionINTEL */,IR(0, 0),IR(206, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(28068, 17)/* DeviceOnlyALTERA */,IR(169, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(28101, 15)/* HostOnlyALTERA */,IR(170, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(24956, 14)/* UncachedINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(28130, 18)/* WriteThroughINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(28148, 15)/* WriteBackINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(24982, 15)/* StreamingINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(15956, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(28163, 11)/* TensorView */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(28174, 11)/* DecodeFunc */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(28185, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(17203, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(26951, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(26969, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(26976, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(28195, 8)/* NoneARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(28203, 15)/* NontemporalARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(28218, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(28238, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(28262, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(28284, 21)/* NonPrivateElementARM */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(7, 128);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(135, 285);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(420, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(428, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(431, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(433, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(452, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(462, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(466, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(481, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(485, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(491, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(494, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(497, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(501, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(509, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(512, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(528, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(537, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(540, 149);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(689, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(697, 98);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(795, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(812, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(814, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(817, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(819, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(828, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(832, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(836, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(842, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(851, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(858, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(862, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(879, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(905, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(925, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(927, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(930, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(932, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(934, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(937, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(942, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(962, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(977, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(986, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(990, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1005, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1006, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1010, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1011, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1019, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1022, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1034, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1036, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1039, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1041, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1046, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1048, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1090, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1097, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1100, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1101, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1109, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1112, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1114, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1134, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1144, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1148, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1162, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1191, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1195, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1198, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1203, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(862, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(977, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1010, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(485, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(962, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1019, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(814, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1203, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(135, 285);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1384> kOperandNames{{
  {IR(5510, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5536, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5526, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5546, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5554, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5575, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5586, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5610, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7569, 13), 94}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7582, 12), 94}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6915, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6935, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7617, 20), 95}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7637, 19), 95}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6963, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7060, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6989, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7008, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7035, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6719, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6693, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5694, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7476, 22), 89}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8775, 12), 133}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6457, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6431, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6470, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5707, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7498, 22), 90}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8828, 12), 134}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7824, 17), 101}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8228, 17), 115}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6789, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6732, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6221, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5991, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6077, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7702, 23), 97}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7656, 12), 96}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7082, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7668, 15), 96}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6012, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7388, 16), 85}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6162, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6256, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6243, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6210, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6087, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8530, 11), 122}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8488, 14), 121}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8710, 33), 130}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8676, 34), 129}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8207, 11), 114}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8218, 10), 114}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8658, 18), 128}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8761, 14), 132}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8392, 37), 118}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8325, 34), 117}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8580, 20), 123}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8743, 18), 131}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8200, 7), 113}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8268, 30), 116}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8429, 20), 119}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8449, 19), 119}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8089, 23), 110}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8112, 22), 110}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5729, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6417, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5830, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7725, 22), 97}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7841, 12), 102}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7853, 11), 102}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7864, 14), 103}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7878, 13), 103}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5843, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7520, 15), 91}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6144, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6181, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7535, 16), 92}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7551, 18), 93}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6347, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6334, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6104, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8004, 22), 107}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8026, 21), 107}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7967, 19), 106}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7986, 18), 106}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8134, 17), 111}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8151, 16), 111}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5977, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6001, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5684, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5668, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7328, 18), 83}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7440, 17), 87}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5740, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7457, 19), 88}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7772, 24), 99}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7747, 25), 98}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6742, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7796, 28), 100}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8468, 20), 120}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8068, 11), 109}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8079, 10), 109}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8047, 11), 108}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8058, 10), 108}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7117, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7213, 16), 79}, // 107 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8632, 10), 125}, // 108 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8651, 7), 127}, // 109 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6024, 9), 24}, // 110 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6066, 11), 26}, // 111 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6051, 15), 25}, // 112 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7180, 15), 78}, // 113 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7265, 20), 81}, // 114 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7304, 24), 82}, // 115 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7164, 16), 77}, // 116 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6833, 15), 64}, // 117 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6506, 15), 53}, // 118 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6521, 18), 53}, // 119 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6561, 15), 54}, // 120 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6576, 18), 54}, // 121 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6594, 15), 55}, // 122 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6609, 18), 55}, // 123 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6368, 11), 44}, // 124 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6627, 15), 56}, // 125 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6642, 18), 56}, // 126 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6379, 26), 45}, // 127 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6660, 15), 57}, // 128 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6675, 18), 57}, // 129 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6318, 16), 41}, // 130 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6271, 13), 40}, // 131 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7428, 12), 86}, // 132 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5967, 10), 19}, // 133 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5952, 15), 18}, // 134 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5937, 15), 17}, // 135 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6897, 18), 67}, // 136 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6879, 18), 66}, // 137 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6848, 15), 65}, // 138 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5720, 9), 11}, // 139 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6405, 12), 46}, // 140 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6813, 10), 63}, // 141 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5889, 14), 16}, // 142 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7229, 15), 80}, // 143 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7366, 22), 84}, // 144 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6483, 10), 51}, // 145 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8642, 9), 126}, // 146 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6493, 13), 52}, // 147 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8600, 13), 124}, // 148 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6202, 8), 35}, // 149 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6132, 12), 31}, // 150 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6118, 14), 30}, // 151 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7926, 21), 105}, // 152 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7947, 20), 105}, // 153 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7891, 18), 104}, // 154 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7909, 17), 104}, // 155 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8167, 17), 112}, // 156 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8184, 16), 112}, // 157 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5565, 10), 139}, // 158 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14251, 35), 367}, // 159 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14286, 34), 367}, // 160 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13723, 38), 355}, // 161 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13761, 37), 355}, // 162 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13658, 33), 354}, // 163 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13691, 32), 354}, // 164 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14945, 19), 392}, // 165 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13296, 9), 340}, // 166 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14862, 20), 388}, // 167 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13351, 23), 343}, // 168 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12767, 22), 315}, // 169 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14778, 20), 384}, // 170 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13305, 23), 341}, // 171 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14798, 20), 385}, // 172 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13328, 23), 342}, // 173 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9008, 14), 155}, // 174 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10395, 17), 231}, // 175 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14903, 24), 390}, // 176 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11017, 29), 265}, // 177 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10995, 22), 264}, // 178 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10979, 16), 263}, // 179 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15680, 20), 415}, // 180 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12704, 18), 312}, // 181 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14715, 16), 380}, // 182 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14453, 20), 371}, // 183 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14473, 19), 371}, // 184 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15641, 19), 413}, // 185 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5694, 13), 165}, // 186 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12335, 32), 300}, // 187 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12367, 31), 300}, // 188 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11185, 31), 279}, // 189 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11216, 30), 279}, // 190 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13001, 30), 327}, // 191 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10764, 32), 250}, // 192 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12894, 31), 324}, // 193 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10043, 21), 378}, // 194 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9978, 28), 213}, // 195 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12423, 20), 302}, // 196 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12925, 40), 325}, // 197 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12864, 30), 323}, // 198 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12965, 36), 326}, // 199 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12747, 20), 314}, // 200 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13031, 28), 328}, // 201 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6441, 16), 205}, // 202 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5707, 13), 166}, // 203 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14882, 21), 389}, // 204 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12546, 25), 307}, // 205 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12571, 28), 307}, // 206 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10534, 18), 237}, // 207 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10519, 15), 236}, // 208 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9455, 18), 184}, // 209 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7195, 18), 266}, // 210 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8979, 14), 153}, // 211 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6801, 12), 227}, // 212 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12599, 23), 308}, // 213 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14666, 11), 376}, // 214 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15766, 27), 418}, // 215 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15733, 33), 417}, // 216 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15700, 33), 416}, // 217 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15793, 32), 419}, // 218 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14560, 22), 374}, // 219 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14582, 25), 374}, // 220 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14607, 28), 375}, // 221 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14635, 31), 375}, // 222 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14519, 19), 373}, // 223 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14538, 22), 373}, // 224 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14677, 14), 376}, // 225 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6704, 15), 218}, // 226 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13408, 16), 346}, // 227 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14771, 7), 383}, // 228 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13638, 20), 353}, // 229 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15153, 29), 398}, // 230 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15182, 28), 398}, // 231 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14202, 25), 366}, // 232 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14227, 24), 366}, // 233 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13970, 28), 361}, // 234 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13998, 27), 361}, // 235 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14964, 30), 393}, // 236 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14994, 29), 393}, // 237 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14054, 21), 363}, // 238 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14075, 20), 363}, // 239 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14121, 41), 365}, // 240 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14162, 40), 365}, // 241 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13895, 26), 359}, // 242 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15023, 28), 394}, // 243 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15104, 25), 397}, // 244 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15129, 24), 397}, // 245 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13828, 23), 357}, // 246 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13851, 22), 357}, // 247 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13921, 25), 360}, // 248 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13946, 24), 360}, // 249 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13585, 27), 352}, // 250 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13612, 26), 352}, // 251 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14492, 14), 372}, // 252 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14506, 13), 372}, // 253 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15088, 16), 396}, // 254 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8893, 8), 144}, // 255 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8879, 14), 143}, // 256 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10819, 16), 252}, // 257 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8901, 8), 145}, // 258 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10016, 27), 215}, // 259 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10006, 10), 214}, // 260 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14756, 15), 382}, // 261 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13204, 23), 336}, // 262 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7594, 23), 278}, // 263 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11163, 22), 278}, // 264 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7683, 19), 280}, // 265 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7404, 24), 274}, // 266 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10857, 16), 254}, // 267 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12514, 32), 306}, // 268 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12443, 33), 303}, // 269 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12476, 38), 304}, // 270 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6766, 23), 216}, // 271 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13559, 26), 351}, // 272 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13250, 22), 338}, // 273 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15541, 22), 409}, // 274 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9300, 15), 171}, // 275 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5752, 9), 137}, // 276 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9050, 18), 158}, // 277 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11091, 28), 269}, // 278 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9513, 16), 187}, // 279 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15240, 36), 400}, // 280 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15276, 35), 400}, // 281 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15210, 30), 399}, // 282 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9969, 9), 212}, // 283 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6284, 16), 194}, // 284 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9652, 26), 196}, // 285 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6539, 22), 197}, // 286 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9732, 25), 200}, // 287 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11260, 30), 281}, // 288 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11290, 29), 281}, // 289 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9757, 20), 201}, // 290 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14731, 25), 381}, // 291 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9678, 23), 198}, // 292 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9701, 31), 199}, // 293 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9632, 20), 195}, // 294 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15563, 26), 410}, // 295 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8972, 7), 152}, // 296 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14426, 14), 370}, // 297 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14440, 13), 370}, // 298 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9369, 8), 177}, // 299 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8928, 11), 148}, // 300 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9391, 12), 180}, // 301 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9246, 15), 167}, // 302 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11146, 17), 276}, // 303 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10835, 22), 253}, // 304 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9068, 20), 159}, // 305 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9403, 13), 181}, // 306 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8954, 12), 150}, // 307 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9444, 11), 183}, // 308 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8939, 15), 149}, // 309 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10873, 21), 256}, // 310 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9278, 10), 169}, // 311 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13272, 24), 339}, // 312 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9320, 16), 173}, // 313 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11405, 36), 284}, // 314 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11441, 39), 284}, // 315 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11946, 39), 291}, // 316 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11985, 42), 291}, // 317 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9022, 6), 156}, // 318 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10952, 27), 262}, // 319 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10938, 14), 261}, // 320 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8909, 6), 146}, // 321 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8915, 13), 147}, // 322 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10894, 14), 257}, // 323 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9315, 5), 172}, // 324 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13227, 23), 337}, // 325 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9473, 22), 185}, // 326 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5519, 7), 141}, // 327 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13873, 22), 358}, // 328 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8862, 8), 140}, // 329 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8993, 15), 154}, // 330 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14818, 20), 386}, // 331 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12827, 14), 321}, // 332 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14025, 15), 362}, // 333 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14040, 14), 362}, // 334 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15616, 25), 412}, // 335 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8855, 7), 135}, // 336 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14095, 26), 364}, // 337 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5815, 15), 277}, // 338 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5801, 14), 275}, // 339 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9352, 7), 175}, // 340 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6823, 10), 228}, // 341 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5903, 14), 190}, // 342 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9607, 13), 192}, // 343 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14838, 11), 387}, // 344 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14849, 13), 387}, // 345 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7346, 20), 273}, // 346 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5637, 31), 299}, // 347 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12301, 34), 299}, // 348 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9620, 12), 193}, // 349 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8966, 6), 151}, // 350 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11046, 16), 267}, // 351 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12807, 20), 320}, // 352 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10923, 15), 260}, // 353 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12789, 18), 317}, // 354 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8840, 15), 377}, // 355 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10632, 12), 242}, // 356 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12722, 25), 313}, // 357 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10609, 23), 241}, // 358 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8787, 41), 329}, // 359 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8359, 33), 316}, // 360 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5787, 14), 245}, // 361 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8541, 39), 319}, // 362 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8245, 23), 296}, // 363 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5774, 13), 295}, // 364 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12622, 29), 309}, // 365 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8298, 27), 294}, // 366 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12398, 25), 301}, // 367 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8502, 28), 318}, // 368 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10663, 32), 244}, // 369 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15660, 20), 414}, // 370 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14691, 24), 379}, // 371 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13183, 21), 335}, // 372 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10577, 16), 239}, // 373 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10593, 16), 240}, // 374 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14367, 30), 369}, // 375 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14397, 29), 369}, // 376 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11356, 23), 283}, // 377 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11379, 26), 283}, // 378 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11062, 29), 268}, // 379 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10412, 28), 232}, // 380 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6033, 18), 168}, // 381 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9359, 10), 176}, // 382 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9377, 14), 179}, // 383 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9261, 17), 178}, // 384 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9146, 33), 162}, // 385 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11719, 36), 288}, // 386 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11755, 39), 288}, // 387 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9288, 12), 170}, // 388 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5677, 7), 136}, // 389 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12841, 23), 322}, // 390 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10908, 15), 258}, // 391 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7146, 18), 259}, // 392 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12677, 27), 311}, // 393 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12651, 26), 310}, // 394 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5849, 12), 202}, // 395 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11319, 17), 282}, // 396 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11336, 20), 282}, // 397 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8613, 19), 305}, // 398 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7285, 19), 272}, // 399 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5917, 20), 203}, // 400 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5861, 28), 270}, // 401 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11119, 27), 270}, // 402 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7244, 21), 271}, // 403 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11246, 14), 280}, // 404 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10552, 25), 238}, // 405 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9336, 16), 174}, // 406 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15520, 21), 408}, // 407 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14927, 18), 391}, // 408 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7100, 17), 255}, // 409 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10200, 25), 223}, // 410 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10440, 24), 233}, // 411 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9179, 34), 163}, // 412 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11794, 37), 289}, // 413 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11831, 40), 289}, // 414 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9213, 33), 164}, // 415 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11871, 36), 290}, // 416 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11907, 39), 290}, // 417 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9416, 28), 182}, // 418 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9088, 24), 160}, // 419 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9529, 30), 188}, // 420 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9559, 31), 189}, // 421 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10327, 21), 226}, // 422 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10305, 22), 225}, // 423 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10498, 21), 235}, // 424 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9892, 37), 210}, // 425 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9929, 40), 211}, // 426 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11561, 39), 286}, // 427 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11600, 42), 286}, // 428 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12114, 42), 293}, // 429 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12156, 45), 293}, // 430 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10288, 17), 224}, // 431 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10225, 28), 223}, // 432 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15339, 23), 402}, // 433 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15362, 30), 403}, // 434 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15392, 30), 404}, // 435 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13495, 39), 349}, // 436 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13424, 33), 347}, // 437 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13457, 38), 348}, // 438 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6300, 18), 217}, // 439 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13099, 27), 332}, // 440 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15311, 28), 401}, // 441 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9590, 17), 191}, // 442 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13126, 26), 333}, // 443 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13152, 31), 334}, // 444 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15422, 38), 405}, // 445 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13078, 21), 331}, // 446 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10184, 16), 222}, // 447 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15051, 19), 395}, // 448 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15070, 18), 395}, // 449 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13059, 19), 330}, // 450 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15589, 27), 411}, // 451 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9881, 11), 209}, // 452 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15460, 28), 406}, // 453 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5761, 13), 138}, // 454 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9028, 22), 157}, // 455 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10796, 23), 251}, // 456 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10742, 22), 248}, // 457 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10721, 21), 247}, // 458 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10695, 26), 246}, // 459 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9795, 28), 206}, // 460 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9823, 28), 207}, // 461 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9851, 30), 208}, // 462 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6863, 16), 249}, // 463 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9495, 18), 186}, // 464 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14320, 24), 368}, // 465 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14344, 23), 368}, // 466 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10253, 35), 224}, // 467 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10464, 34), 234}, // 468 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9112, 34), 161}, // 469 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11642, 37), 287}, // 470 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11679, 40), 287}, // 471 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9777, 18), 204}, // 472 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11480, 39), 285}, // 473 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11519, 42), 285}, // 474 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12027, 42), 292}, // 475 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12069, 45), 292}, // 476 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13798, 30), 356}, // 477 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10644, 19), 243}, // 478 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15488, 32), 407}, // 479 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13534, 25), 350}, // 480 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10378, 17), 230}, // 481 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10348, 30), 229}, // 482 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8870, 9), 142}, // 483 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13393, 15), 345}, // 484 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13374, 19), 344}, // 485 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12201, 18), 297}, // 486 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12240, 29), 298}, // 487 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12269, 32), 298}, // 488 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12219, 21), 297}, // 489 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10140, 44), 221}, // 490 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10097, 43), 220}, // 491 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10064, 33), 219}, // 492 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15837, 8), 421}, // 493 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15845, 8), 422}, // 494 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15853, 6), 423}, // 495 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15859, 7), 424}, // 496 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15866, 11), 425}, // 497 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15877, 9), 426}, // 498 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15886, 13), 427}, // 499 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15825, 12), 420}, // 500 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15899, 6), 428}, // 501 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15905, 10), 429}, // 502 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15915, 6), 430}, // 503 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15936, 20), 432}, // 504 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15921, 15), 431}, // 505 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16045, 15), 440}, // 506 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16060, 13), 441}, // 507 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16033, 12), 439}, // 508 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16123, 21), 445}, // 509 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16016, 17), 438}, // 510 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16200, 16), 449}, // 511 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16004, 12), 437}, // 512 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16184, 16), 448}, // 513 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15977, 14), 435}, // 514 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15961, 16), 434}, // 515 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15991, 13), 436}, // 516 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16144, 20), 446}, // 517 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16088, 18), 443}, // 518 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16073, 15), 442}, // 519 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16164, 20), 447}, // 520 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16106, 17), 444}, // 521 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16236, 24), 451}, // 522 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16216, 20), 450}, // 523 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15956, 5), 433}, // 524 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16321, 9), 456}, // 525 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16353, 7), 460}, // 526 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16260, 6), 452}, // 527 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16360, 9), 461}, // 528 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16271, 6), 454}, // 529 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16266, 5), 453}, // 530 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16310, 11), 455}, // 531 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16342, 11), 459}, // 532 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16330, 5), 457}, // 533 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16335, 7), 458}, // 534 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16405, 11), 465}, // 535 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16369, 10), 462}, // 536 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16392, 13), 464}, // 537 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16379, 13), 463}, // 538 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16416, 10), 466}, // 539 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16426, 10), 467}, // 540 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16436, 10), 468}, // 541 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16604, 12), 479}, // 542 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16616, 12), 480}, // 543 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16459, 14), 470}, // 544 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16473, 14), 471}, // 545 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16487, 14), 472}, // 546 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16446, 13), 469}, // 547 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16564, 19), 477}, // 548 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16516, 16), 474}, // 549 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16532, 16), 475}, // 550 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16548, 16), 476}, // 551 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16501, 15), 473}, // 552 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16583, 21), 478}, // 553 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16680, 28), 484}, // 554 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16640, 15), 482}, // 555 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16655, 25), 483}, // 556 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16628, 12), 481}, // 557 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16716, 27), 486}, // 558 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16743, 27), 487}, // 559 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16770, 27), 488}, // 560 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16797, 32), 489}, // 561 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16708, 8), 485}, // 562 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16829, 26), 490}, // 563 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16866, 4), 493}, // 564 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16859, 7), 492}, // 565 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16855, 4), 491}, // 566 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16870, 11), 494}, // 567 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16892, 21), 496}, // 568 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16881, 11), 495}, // 569 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16924, 14), 498}, // 570 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16938, 21), 499}, // 571 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16913, 11), 497}, // 572 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16959, 18), 500}, // 573 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15837, 8), 502}, // 574 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15845, 8), 503}, // 575 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15853, 6), 504}, // 576 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15859, 7), 505}, // 577 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15866, 11), 506}, // 578 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15877, 9), 507}, // 579 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15886, 13), 508}, // 580 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15825, 12), 501}, // 581 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15899, 6), 509}, // 582 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(15905, 10), 510}, // 583 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(15915, 6), 511}, // 584 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16045, 15), 519}, // 585 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16060, 13), 520}, // 586 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16033, 12), 518}, // 587 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16123, 21), 524}, // 588 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16016, 17), 517}, // 589 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16004, 12), 516}, // 590 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16184, 16), 527}, // 591 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15977, 14), 514}, // 592 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15961, 16), 513}, // 593 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15991, 13), 515}, // 594 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16144, 20), 525}, // 595 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16088, 18), 522}, // 596 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16073, 15), 521}, // 597 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16164, 20), 526}, // 598 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16106, 17), 523}, // 599 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15956, 5), 512}, // 600 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16321, 9), 532}, // 601 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16353, 7), 536}, // 602 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16260, 6), 528}, // 603 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16271, 6), 530}, // 604 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16266, 5), 529}, // 605 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16310, 11), 531}, // 606 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16342, 11), 535}, // 607 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16330, 5), 533}, // 608 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16335, 7), 534}, // 609 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16369, 10), 537}, // 610 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(16392, 13), 539}, // 611 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(16379, 13), 538}, // 612 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(19683, 16), 659}, // 613 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17186, 8), 559}, // 614 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(18333, 15), 614}, // 615 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(18348, 18), 614}, // 616 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17664, 10), 583}, // 617 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(17708, 12), 585}, // 618 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(17037, 12), 546}, // 619 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18088, 17), 602}, // 620 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19233, 15), 647}, // 621 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19248, 14), 647}, // 622 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18442, 7), 618}, // 623 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18946, 16), 640}, // 624 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18962, 15), 640}, // 625 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17341, 8), 572}, // 626 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(18467, 16), 620}, // 627 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18449, 18), 619}, // 628 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17001, 6), 542}, // 629 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(17848, 22), 592}, // 630 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17826, 22), 591}, // 631 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18498, 13), 622}, // 632 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18483, 15), 621}, // 633 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17007, 12), 543}, // 634 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(19843, 21), 664}, // 635 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19864, 20), 664}, // 636 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17118, 8), 551}, // 637 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(19402, 20), 652}, // 638 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19422, 19), 652}, // 639 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17084, 8), 550}, // 640 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(21090, 22), 687}, // 641 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21149, 23), 688}, // 642 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19441, 16), 653}, // 643 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19457, 15), 653}, // 644 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17151, 9), 555}, // 645 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(18551, 13), 625}, // 646 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17212, 9), 562}, // 647 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(17028, 9), 545}, // 648 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(17325, 10), 570}, // 649 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(21037, 17), 686}, // 650 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20266, 28), 674}, // 651 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20294, 27), 674}, // 652 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17203, 9), 561}, // 653 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(18697, 14), 632}, // 654 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(17349, 14), 573}, // 655 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(19472, 29), 654}, // 656 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19501, 28), 654}, // 657 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19055, 17), 643}, // 658 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19072, 16), 643}, // 659 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17870, 18), 593}, // 660 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(17534, 15), 579}, // 661 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(20128, 26), 671}, // 662 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17484, 15), 578}, // 663 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17140, 5), 553}, // 664 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(19262, 21), 648}, // 665 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19283, 20), 648}, // 666 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17436, 14), 577}, // 667 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(18608, 21), 628}, // 668 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18839, 24), 636}, // 669 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19957, 31), 666}, // 670 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18782, 26), 635}, // 671 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19593, 26), 657}, // 672 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19619, 25), 657}, // 673 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17073, 11), 549}, // 674 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(17062, 11), 548}, // 675 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(18671, 26), 631}, // 676 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18411, 31), 617}, // 677 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18381, 30), 616}, // 678 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18711, 24), 632}, // 679 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18748, 19), 633}, // 680 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20888, 16), 683}, // 681 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19884, 20), 665}, // 682 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19904, 19), 665}, // 683 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20980, 29), 685}, // 684 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21009, 28), 685}, // 685 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17335, 6), 571}, // 686 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(20951, 15), 684}, // 687 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20966, 14), 684}, // 688 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19712, 25), 661}, // 689 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19737, 24), 661}, // 690 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17643, 21), 582}, // 691 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(17167, 10), 557}, // 692 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(20205, 31), 673}, // 693 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20236, 30), 673}, // 694 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20154, 26), 672}, // 695 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20180, 25), 672}, // 696 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17611, 18), 580}, // 697 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(17316, 9), 569}, // 698 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(20384, 34), 676}, // 699 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20418, 33), 676}, // 700 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20451, 31), 677}, // 701 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20482, 30), 677}, // 702 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20512, 29), 678}, // 703 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20541, 28), 678}, // 704 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20672, 30), 680}, // 705 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20702, 29), 680}, // 706 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20603, 35), 679}, // 707 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20638, 34), 679}, // 708 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20731, 33), 681}, // 709 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20764, 32), 681}, // 710 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19644, 20), 658}, // 711 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19664, 19), 658}, // 712 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17049, 13), 547}, // 713 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17674, 14), 584}, // 714 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(17720, 16), 586}, // 715 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(19761, 21), 662}, // 716 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19782, 20), 662}, // 717 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18977, 23), 641}, // 718 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19000, 22), 641}, // 719 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19088, 20), 644}, // 720 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19108, 19), 644}, // 721 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20048, 18), 669}, // 722 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18366, 15), 615}, // 723 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18892, 13), 638}, // 724 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18905, 12), 638}, // 725 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19168, 12), 646}, // 726 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19180, 11), 646}, // 727 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19699, 13), 660}, // 728 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17629, 14), 581}, // 729 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(17126, 14), 552}, // 730 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(17780, 13), 588}, // 731 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(17793, 15), 589}, // 732 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(17920, 20), 595}, // 733 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17888, 32), 594}, // 734 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17233, 12), 564}, // 735 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(18273, 11), 612}, // 736 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18284, 14), 612}, // 737 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17221, 12), 563}, // 738 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(18917, 15), 639}, // 739 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18932, 14), 639}, // 740 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17363, 7), 574}, // 741 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(18105, 12), 603}, // 742 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18117, 19), 604}, // 743 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18136, 14), 605}, // 744 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17145, 6), 554}, // 745 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(18060, 28), 601}, // 746 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18035, 25), 600}, // 747 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17983, 25), 598}, // 748 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17963, 20), 597}, // 749 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18008, 27), 599}, // 750 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18197, 16), 608}, // 751 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18213, 15), 608}, // 752 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18238, 10), 610}, // 753 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18248, 13), 611}, // 754 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18261, 12), 611}, // 755 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18228, 10), 609}, // 756 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19802, 21), 663}, // 757 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19823, 20), 663}, // 758 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19529, 15), 655}, // 759 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19544, 14), 655}, // 760 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18525, 26), 624}, // 761 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18863, 15), 637}, // 762 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18878, 14), 637}, // 763 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20321, 32), 675}, // 764 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20353, 31), 675}, // 765 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16977, 17), 540}, // 766 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(17177, 9), 558}, // 767 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(18298, 16), 613}, // 768 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(18314, 19), 613}, // 769 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17019, 9), 544}, // 770 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(18511, 14), 623}, // 771 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17160, 7), 556}, // 772 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(17289, 20), 567}, // 773 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(17736, 44), 587}, // 774 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18169, 28), 607}, // 775 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18564, 17), 626}, // 776 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19127, 21), 645}, // 777 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19148, 20), 645}, // 778 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19988, 25), 667}, // 779 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19022, 17), 642}, // 780 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19039, 16), 642}, // 781 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16994, 7), 541}, // 782 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(20796, 27), 682}, // 783 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20823, 26), 682}, // 784 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18656, 15), 630}, // 785 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19558, 18), 656}, // 786 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19576, 17), 656}, // 787 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20066, 16), 670}, // 788 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20082, 15), 670}, // 789 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17309, 7), 568}, // 790 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(19303, 17), 649}, // 791 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19320, 16), 649}, // 792 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17940, 23), 596}, // 793 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19365, 19), 651}, // 794 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19384, 18), 651}, // 795 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17245, 8), 565}, // 796 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(17279, 10), 566}, // 797 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(18735, 13), 633}, // 798 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(18767, 15), 634}, // 799 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20013, 35), 668}, // 800 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18629, 27), 629}, // 801 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18581, 27), 627}, // 802 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18150, 19), 606}, // 803 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17194, 9), 560}, // 804 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(17808, 18), 590}, // 805 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19336, 15), 650}, // 806 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19351, 14), 650}, // 807 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17370, 10), 575}, // 808 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(17380, 10), 576}, // 809 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(21172, 3), 689}, // 810 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21175, 3), 690}, // 811 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21178, 3), 691}, // 812 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21191, 7), 694}, // 813 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21181, 5), 692}, // 814 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21186, 5), 693}, // 815 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21198, 12), 695}, // 816 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21210, 17), 696}, // 817 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22328, 20), 764}, // 818 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22022, 15), 749}, // 819 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21653, 15), 727}, // 820 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10534, 18), 741}, // 821 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10519, 15), 740}, // 822 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21423, 13), 710}, // 823 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21436, 10), 711}, // 824 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21408, 15), 709}, // 825 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21446, 15), 712}, // 826 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22463, 25), 768}, // 827 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22488, 24), 768}, // 828 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22416, 24), 767}, // 829 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22440, 23), 767}, // 830 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21970, 29), 747}, // 831 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21375, 19), 706}, // 832 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22973, 18), 788}, // 833 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21680, 10), 729}, // 834 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22788, 26), 780}, // 835 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22814, 27), 781}, // 836 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21668, 12), 728}, // 837 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21497, 11), 716}, // 838 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21508, 20), 717}, // 839 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21485, 12), 715}, // 840 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21538, 24), 719}, // 841 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21227, 12), 697}, // 842 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22037, 15), 750}, // 843 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21568, 9), 721}, // 844 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21461, 10), 713}, // 845 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21471, 14), 714}, // 846 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21748, 16), 734}, // 847 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21736, 12), 733}, // 848 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22052, 21), 751}, // 849 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22097, 21), 754}, // 850 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22863, 16), 783}, // 851 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22841, 22), 782}, // 852 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22949, 24), 787}, // 853 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23064, 22), 792}, // 854 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23086, 24), 793}, // 855 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23041, 23), 791}, // 856 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23161, 27), 794}, // 857 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22879, 20), 784}, // 858 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21764, 34), 735}, // 859 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21798, 34), 736}, // 860 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21832, 36), 737}, // 861 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21916, 34), 745}, // 862 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22899, 22), 785}, // 863 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21359, 16), 705}, // 864 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21343, 16), 704}, // 865 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21605, 16), 724}, // 866 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22348, 15), 765}, // 867 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22363, 14), 765}, // 868 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21592, 13), 723}, // 869 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22377, 20), 766}, // 870 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22397, 19), 766}, // 871 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21621, 20), 725}, // 872 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22512, 19), 769}, // 873 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22531, 18), 769}, // 874 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21577, 15), 722}, // 875 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21324, 19), 703}, // 876 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22549, 25), 770}, // 877 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22574, 27), 771}, // 878 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21394, 10), 707}, // 879 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21898, 18), 739}, // 880 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22269, 19), 761}, // 881 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21562, 6), 720}, // 882 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23015, 26), 790}, // 883 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22288, 20), 762}, // 884 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10577, 16), 743}, // 885 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22767, 21), 779}, // 886 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22746, 21), 778}, // 887 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10593, 16), 744}, // 888 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22601, 26), 772}, // 889 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22627, 28), 773}, // 890 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22921, 28), 786}, // 891 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(12841, 23), 776}, // 892 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7164, 16), 753}, // 893 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22655, 31), 774}, // 894 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22686, 33), 775}, // 895 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22719, 27), 777}, // 896 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22308, 20), 763}, // 897 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10552, 25), 742}, // 898 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21239, 13), 698}, // 899 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21252, 22), 699}, // 900 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21274, 21), 700}, // 901 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22073, 24), 752}, // 902 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22222, 25), 759}, // 903 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22146, 26), 756}, // 904 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22247, 22), 760}, // 905 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22172, 23), 757}, // 906 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21999, 23), 748}, // 907 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22195, 27), 758}, // 908 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22118, 28), 755}, // 909 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22991, 24), 789}, // 910 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6271, 13), 730}, // 911 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21868, 30), 738}, // 912 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21690, 22), 731}, // 913 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21712, 24), 732}, // 914 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21950, 20), 746}, // 915 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21528, 10), 718}, // 916 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21641, 12), 726}, // 917 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21309, 15), 702}, // 918 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21295, 14), 701}, // 919 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21404, 4), 708}, // 920 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23326, 10), 806}, // 921 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23336, 9), 806}, // 922 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23387, 12), 809}, // 923 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23399, 11), 809}, // 924 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23345, 14), 807}, // 925 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23359, 13), 807}, // 926 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(16360, 9), 799}, // 927 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23238, 10), 800}, // 928 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5752, 9), 798}, // 929 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23295, 16), 805}, // 930 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23311, 15), 805}, // 931 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5519, 7), 801}, // 932 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23418, 8), 811}, // 933 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23255, 7), 803}, // 934 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23372, 8), 808}, // 935 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23380, 7), 808}, // 936 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23262, 17), 804}, // 937 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23279, 16), 804}, // 938 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23410, 8), 810}, // 939 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23248, 7), 802}, // 940 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23195, 20), 796}, // 941 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23215, 23), 797}, // 942 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23188, 7), 795}, // 943 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23435, 12), 813}, // 944 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(23426, 9), 812}, // 945 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(23447, 12), 814}, // 946 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(23459, 14), 815}, // 947 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(23473, 14), 816}, // 948 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(23492, 4), 818}, // 949 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(23487, 5), 817}, // 950 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(23530, 14), 825}, // 951 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23544, 23), 825}, // 952 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23567, 13), 826}, // 953 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23580, 18), 826}, // 954 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23514, 11), 823}, // 955 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23598, 15), 827}, // 956 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23525, 5), 824}, // 957 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23510, 4), 822}, // 958 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(15956, 5), 819}, // 959 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23503, 7), 821}, // 960 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23496, 7), 820}, // 961 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23613, 4), 828}, // 962 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23625, 4), 831}, // 963 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23621, 4), 830}, // 964 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23617, 4), 829}, // 965 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23661, 18), 834}, // 966 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23679, 18), 835}, // 967 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23629, 16), 832}, // 968 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23645, 16), 833}, // 969 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23720, 6), 840}, // 970 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23704, 11), 838}, // 971 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23697, 7), 837}, // 972 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15956, 5), 836}, // 973 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14838, 11), 841}, // 974 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14849, 13), 841}, // 975 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23715, 5), 839}, // 976 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23736, 6), 844}, // 977 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23747, 8), 846}, // 978 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23755, 10), 847}, // 979 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23773, 12), 849}, // 980 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23765, 8), 848}, // 981 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23785, 21), 850}, // 982 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23806, 20), 850}, // 983 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23731, 5), 843}, // 984 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23742, 5), 845}, // 985 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23726, 5), 842}, // 986 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23861, 16), 854}, // 987 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23847, 14), 853}, // 988 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23833, 14), 852}, // 989 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23973, 28), 857}, // 990 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24001, 27), 857}, // 991 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23918, 28), 856}, // 992 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23946, 27), 856}, // 993 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23877, 21), 855}, // 994 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23898, 20), 855}, // 995 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23826, 7), 851}, // 996 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24028, 10), 858}, // 997 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24038, 10), 859}, // 998 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24059, 15), 861}, // 999 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24048, 11), 860}, // 1000 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24074, 5), 863}, // 1001 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(24088, 12), 866}, // 1002 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(24100, 13), 868}, // 1003 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(24083, 5), 865}, // 1004 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(24079, 4), 864}, // 1005 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(24113, 19), 871}, // 1006 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(24132, 22), 871}, // 1007 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(24154, 17), 872}, // 1008 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(24171, 20), 872}, // 1009 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9352, 7), 870}, // 1010 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(24191, 16), 873}, // 1011 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(24207, 19), 873}, // 1012 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(15956, 5), 862}, // 1013 None in SPV_OPERAND_TYPE_IMAGE
  {IR(24279, 12), 877}, // 1014 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(17363, 7), 867}, // 1015 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(24291, 8), 878}, // 1016 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(17160, 7), 869}, // 1017 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(24257, 11), 875}, // 1018 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(24226, 14), 874}, // 1019 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(24240, 17), 874}, // 1020 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(24268, 11), 876}, // 1021 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(15853, 6), 893}, // 1022 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24460, 10), 892}, // 1023 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24395, 12), 887}, // 1024 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24407, 12), 888}, // 1025 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24384, 11), 886}, // 1026 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24309, 11), 880}, // 1027 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24299, 10), 879}, // 1028 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24369, 15), 885}, // 1029 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24481, 17), 895}, // 1030 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24498, 16), 896}, // 1031 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24631, 16), 903}, // 1032 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24647, 16), 904}, // 1033 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24330, 11), 882}, // 1034 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24470, 11), 894}, // 1035 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24554, 20), 899}, // 1036 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24320, 10), 881}, // 1037 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24355, 14), 884}, // 1038 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24341, 14), 883}, // 1039 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24574, 19), 900}, // 1040 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24593, 19), 901}, // 1041 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24612, 19), 902}, // 1042 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24432, 14), 890}, // 1043 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24446, 14), 891}, // 1044 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24419, 13), 889}, // 1045 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24514, 20), 897}, // 1046 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24534, 20), 898}, // 1047 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24665, 2), 906}, // 1048 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24766, 5), 924}, // 1049 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24687, 5), 912}, // 1050 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24682, 5), 911}, // 1051 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24724, 6), 918}, // 1052 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24730, 13), 919}, // 1053 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24692, 10), 913}, // 1054 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24702, 10), 914}, // 1055 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24663, 2), 905}, // 1056 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24670, 3), 908}, // 1057 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24667, 3), 907}, // 1058 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24673, 4), 909}, // 1059 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24677, 5), 910}, // 1060 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24719, 5), 917}, // 1061 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24715, 4), 916}, // 1062 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24712, 3), 915}, // 1063 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24760, 6), 923}, // 1064 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24743, 5), 920}, // 1065 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24754, 6), 922}, // 1066 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24748, 6), 921}, // 1067 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24771, 28), 925}, // 1068 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24799, 27), 925}, // 1069 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24826, 24), 926}, // 1070 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24850, 23), 926}, // 1071 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24873, 7), 927}, // 1072 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24880, 11), 928}, // 1073 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24891, 14), 929}, // 1074 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24905, 12), 931}, // 1075 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(15956, 5), 930}, // 1076 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(24917, 13), 933}, // 1077 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(15956, 5), 932}, // 1078 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(24930, 7), 934}, // 1079 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24937, 7), 935}, // 1080 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24944, 12), 936}, // 1081 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24970, 12), 938}, // 1082 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25022, 17), 941}, // 1083 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24997, 25), 940}, // 1084 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24982, 15), 939}, // 1085 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24956, 14), 937}, // 1086 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25162, 22), 954}, // 1087 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25184, 21), 954}, // 1088 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25057, 19), 945}, // 1089 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25076, 17), 946}, // 1090 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25046, 11), 944}, // 1091 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19712, 25), 952}, // 1092 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19737, 24), 952}, // 1093 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25121, 18), 949}, // 1094 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25205, 19), 956}, // 1095 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25224, 18), 956}, // 1096 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25367, 16), 960}, // 1097 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25383, 15), 960}, // 1098 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19761, 21), 953}, // 1099 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19782, 20), 953}, // 1100 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25242, 22), 957}, // 1101 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25264, 21), 957}, // 1102 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25107, 14), 948}, // 1103 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25398, 27), 961}, // 1104 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25425, 26), 961}, // 1105 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25093, 14), 947}, // 1106 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25338, 15), 959}, // 1107 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25353, 14), 959}, // 1108 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(15956, 5), 942}, // 1109 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25149, 13), 951}, // 1110 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25139, 10), 950}, // 1111 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19802, 21), 955}, // 1112 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19823, 20), 955}, // 1113 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25285, 27), 958}, // 1114 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25312, 26), 958}, // 1115 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25039, 7), 943}, // 1116 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25734, 27), 975}, // 1117 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25682, 26), 973}, // 1118 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25602, 23), 969}, // 1119 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25556, 23), 967}, // 1120 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25451, 29), 963}, // 1121 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25648, 17), 971}, // 1122 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25761, 27), 976}, // 1123 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25708, 26), 974}, // 1124 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25625, 23), 970}, // 1125 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25579, 23), 968}, // 1126 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25480, 29), 964}, // 1127 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25665, 17), 972}, // 1128 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25509, 21), 965}, // 1129 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25530, 26), 966}, // 1130 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(15956, 5), 962}, // 1131 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25921, 20), 984}, // 1132 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25788, 8), 979}, // 1133 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25796, 21), 981}, // 1134 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25817, 24), 981}, // 1135 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25841, 19), 982}, // 1136 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25860, 22), 982}, // 1137 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25941, 17), 985}, // 1138 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25882, 18), 983}, // 1139 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25900, 21), 983}, // 1140 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15956, 5), 977}, // 1141 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(24279, 12), 980}, // 1142 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(17194, 9), 978}, // 1143 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25965, 8), 987}, // 1144 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25973, 7), 988}, // 1145 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25958, 7), 986}, // 1146 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25980, 7), 989}, // 1147 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25987, 10), 989}, // 1148 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26005, 8), 991}, // 1149 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26021, 15), 993}, // 1150 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26125, 20), 999}, // 1151 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26104, 21), 998}, // 1152 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26145, 12), 1000}, // 1153 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26186, 14), 1002}, // 1154 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26200, 17), 1002}, // 1155 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26217, 12), 1003}, // 1156 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26229, 15), 1003}, // 1157 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15956, 5), 990}, // 1158 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26157, 13), 1001}, // 1159 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26170, 16), 1001}, // 1160 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25997, 8), 990}, // 1161 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26013, 8), 992}, // 1162 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26036, 23), 994}, // 1163 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26073, 15), 996}, // 1164 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26059, 14), 995}, // 1165 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(17194, 9), 1004}, // 1166 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26088, 16), 997}, // 1167 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26244, 10), 1005}, // 1168 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(26259, 4), 1007}, // 1169 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26272, 8), 1009}, // 1170 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26263, 9), 1008}, // 1171 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26254, 5), 1006}, // 1172 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26280, 25), 1010}, // 1173 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(26305, 28), 1010}, // 1174 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(26346, 4), 1013}, // 1175 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26379, 9), 1017}, // 1176 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26388, 13), 1018}, // 1177 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26359, 8), 1015}, // 1178 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26367, 12), 1016}, // 1179 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26350, 9), 1014}, // 1180 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26333, 4), 1011}, // 1181 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26337, 9), 1012}, // 1182 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(15956, 5), 1019}, // 1183 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26401, 25), 1020}, // 1184 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26426, 23), 1021}, // 1185 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26518, 27), 1027}, // 1186 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26545, 28), 1028}, // 1187 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26587, 16), 1030}, // 1188 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26573, 14), 1029}, // 1189 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26657, 30), 1033}, // 1190 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26459, 12), 1024}, // 1191 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(16708, 8), 1022}, // 1192 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26449, 10), 1023}, // 1193 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26644, 13), 1032}, // 1194 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26620, 24), 1031}, // 1195 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26494, 24), 1026}, // 1196 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26603, 17), 1031}, // 1197 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26471, 23), 1025}, // 1198 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26728, 37), 1035}, // 1199 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(26687, 41), 1034}, // 1200 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(26843, 42), 1038}, // 1201 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26765, 37), 1036}, // 1202 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26802, 41), 1037}, // 1203 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26885, 33), 1039}, // 1204 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(26918, 33), 1040}, // 1205 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(26963, 6), 1043}, // 1206 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26951, 12), 1042}, // 1207 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(15956, 5), 1041}, // 1208 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26969, 7), 1044}, // 1209 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26976, 15), 1045}, // 1210 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26999, 7), 1047}, // 1211 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(26991, 8), 1046}, // 1212 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(27064, 13), 1056}, // 1213 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27106, 4), 1062}, // 1214 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27144, 9), 1067}, // 1215 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27077, 5), 1057}, // 1216 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27206, 5), 1076}, // 1217 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27277, 6), 1086}, // 1218 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27030, 5), 1051}, // 1219 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27184, 5), 1072}, // 1220 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27241, 6), 1081}, // 1221 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27294, 5), 1089}, // 1222 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27288, 6), 1088}, // 1223 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27110, 3), 1063}, // 1224 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27153, 8), 1068}, // 1225 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27211, 4), 1077}, // 1226 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27283, 5), 1087}, // 1227 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27097, 5), 1060}, // 1228 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27125, 10), 1065}, // 1229 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27058, 6), 1055}, // 1230 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27195, 6), 1074}, // 1231 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27264, 7), 1084}, // 1232 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27052, 6), 1054}, // 1233 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27189, 6), 1073}, // 1234 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27257, 7), 1083}, // 1235 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27102, 4), 1061}, // 1236 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27135, 9), 1066}, // 1237 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27201, 5), 1075}, // 1238 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27271, 6), 1085}, // 1239 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27089, 8), 1059}, // 1240 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27247, 10), 1082}, // 1241 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27082, 7), 1058}, // 1242 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27113, 12), 1064}, // 1243 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27022, 8), 1050}, // 1244 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27169, 8), 1070}, // 1245 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27224, 9), 1079}, // 1246 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27014, 8), 1049}, // 1247 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27161, 8), 1069}, // 1248 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27215, 9), 1078}, // 1249 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27035, 6), 1052}, // 1250 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27041, 11), 1053}, // 1251 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27177, 7), 1071}, // 1252 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27233, 8), 1080}, // 1253 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27006, 8), 1048}, // 1254 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27299, 12), 1090}, // 1255 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27311, 7), 1091}, // 1256 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27337, 11), 1094}, // 1257 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27348, 12), 1095}, // 1258 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27360, 15), 1095}, // 1259 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27375, 14), 1096}, // 1260 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27328, 9), 1093}, // 1261 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27318, 10), 1092}, // 1262 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27397, 12), 1099}, // 1263 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(27389, 8), 1098}, // 1264 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(15956, 5), 1097}, // 1265 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(27409, 29), 1100}, // 1266 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(15837, 8), 1102}, // 1267 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15845, 8), 1103}, // 1268 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15853, 6), 1104}, // 1269 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15859, 7), 1105}, // 1270 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15866, 11), 1106}, // 1271 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15877, 9), 1107}, // 1272 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15886, 13), 1108}, // 1273 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15825, 12), 1101}, // 1274 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15899, 6), 1109}, // 1275 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15905, 10), 1110}, // 1276 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15915, 6), 1111}, // 1277 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15936, 20), 1113}, // 1278 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15921, 15), 1112}, // 1279 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16045, 15), 1121}, // 1280 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16060, 13), 1122}, // 1281 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16033, 12), 1120}, // 1282 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16123, 21), 1126}, // 1283 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16016, 17), 1119}, // 1284 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16200, 16), 1130}, // 1285 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16004, 12), 1118}, // 1286 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16184, 16), 1129}, // 1287 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15977, 14), 1116}, // 1288 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15961, 16), 1115}, // 1289 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15991, 13), 1117}, // 1290 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16144, 20), 1127}, // 1291 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16088, 18), 1124}, // 1292 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16073, 15), 1123}, // 1293 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16164, 20), 1128}, // 1294 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16106, 17), 1125}, // 1295 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16236, 24), 1132}, // 1296 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16216, 20), 1131}, // 1297 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(27438, 26), 1133}, // 1298 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15956, 5), 1114}, // 1299 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16321, 9), 1138}, // 1300 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16353, 7), 1142}, // 1301 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16260, 6), 1134}, // 1302 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16360, 9), 1143}, // 1303 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16271, 6), 1136}, // 1304 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16266, 5), 1135}, // 1305 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16310, 11), 1137}, // 1306 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16342, 11), 1141}, // 1307 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16330, 5), 1139}, // 1308 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16335, 7), 1140}, // 1309 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16405, 11), 1147}, // 1310 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16369, 10), 1144}, // 1311 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16392, 13), 1146}, // 1312 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16379, 13), 1145}, // 1313 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(27499, 15), 1154}, // 1314 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27464, 5), 1149}, // 1315 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27469, 5), 1150}, // 1316 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27519, 7), 1156}, // 1317 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27494, 5), 1153}, // 1318 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27526, 5), 1157}, // 1319 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27474, 9), 1151}, // 1320 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27483, 11), 1152}, // 1321 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27546, 5), 1161}, // 1322 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27514, 5), 1155}, // 1323 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27536, 6), 1159}, // 1324 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27006, 8), 1148}, // 1325 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27531, 5), 1158}, // 1326 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27542, 4), 1160}, // 1327 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27633, 14), 1172}, // 1328 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27715, 16), 1178}, // 1329 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27731, 15), 1178}, // 1330 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28051, 17), 1188}, // 1331 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27580, 15), 1167}, // 1332 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28068, 17), 1189}, // 1333 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28085, 16), 1189}, // 1334 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27603, 9), 1169}, // 1335 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27612, 8), 1170}, // 1336 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27820, 16), 1181}, // 1337 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27836, 15), 1181}, // 1338 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28029, 22), 1187}, // 1339 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27984, 21), 1185}, // 1340 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28101, 15), 1190}, // 1341 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28116, 14), 1190}, // 1342 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27647, 6), 1173}, // 1343 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27746, 24), 1179}, // 1344 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27770, 23), 1179}, // 1345 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27851, 22), 1182}, // 1346 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27873, 21), 1182}, // 1347 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27567, 6), 1163}, // 1348 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27699, 16), 1177}, // 1349 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27573, 7), 1165}, // 1350 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27937, 22), 1184}, // 1351 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27959, 25), 1184}, // 1352 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27595, 8), 1168}, // 1353 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27620, 13), 1171}, // 1354 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27793, 14), 1180}, // 1355 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27807, 13), 1180}, // 1356 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27894, 22), 1183}, // 1357 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27916, 21), 1183}, // 1358 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27653, 14), 1174}, // 1359 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28005, 24), 1186}, // 1360 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27680, 19), 1176}, // 1361 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27667, 13), 1175}, // 1362 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(17245, 8), 1164}, // 1363 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27551, 16), 1162}, // 1364 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27318, 10), 1166}, // 1365 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24982, 15), 1194}, // 1366 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24956, 14), 1191}, // 1367 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28148, 15), 1193}, // 1368 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28130, 18), 1192}, // 1369 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28174, 11), 1197}, // 1370 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(15956, 5), 1195}, // 1371 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(28163, 11), 1196}, // 1372 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(26951, 12), 1200}, // 1373 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(17203, 9), 1199}, // 1374 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(26969, 7), 1201}, // 1375 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(26976, 15), 1202}, // 1376 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28185, 10), 1198}, // 1377 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28238, 24), 1206}, // 1378 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28262, 22), 1207}, // 1379 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28284, 21), 1208}, // 1380 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28195, 8), 1203}, // 1381 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28203, 15), 1204}, // 1382 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28218, 20), 1205}, // 1383 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 150);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(158, 335);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(493, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(501, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(504, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(506, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(525, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(535, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(539, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(554, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(558, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(564, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(567, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(570, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(574, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(582, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(585, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(601, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(610, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(613, 197);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(810, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(818, 103);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(921, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(944, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(946, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(949, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(951, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(962, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(966, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(970, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(977, 10);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(987, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(997, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1001, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1022, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1048, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1068, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1072, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1075, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1077, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1079, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1082, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1087, 30);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1117, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1132, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1144, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1149, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1168, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1169, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1173, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1175, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1183, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1186, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1199, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1201, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1204, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1206, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1211, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1213, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1255, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1263, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1266, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1267, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1275, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1278, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1280, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1300, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1310, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1314, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1328, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1366, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1370, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1373, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1378, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1001, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1132, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1173, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(558, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1117, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1183, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(946, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1378, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(158, 335);
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
static const std::array<InstructionDesc, 868> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(28305, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(28361, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(28367, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(28486, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(28493, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(28509, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(28514, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(28525, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(28532, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(28537, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(28547, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(28607, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(28679, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(28753, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(28796, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(28838, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(28849, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(28858, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(28867, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(28912, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(28922, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(28933, 11), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(29057, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(29067, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(29079, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(29096, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(29106, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(29123, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(29134, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(29176, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(29188, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(29201, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(29211, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(29227, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(29241, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(29251, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(29260, 19), IR(0, 0), IR(211, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(29279, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(29292, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(17203, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(29344, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(29440, 16), IR(0, 0), IR(213, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(29456, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(29469, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(29486, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(29504, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(29517, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(29580, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(27603, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(29629, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(29647, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(29659, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(29672, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(29681, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(29739, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(29744, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(29750, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(29761, 16), IR(0, 0), IR(214, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(29777, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(29789, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(29809, 15), IR(0, 0), IR(216, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(29824, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(29836, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(29859, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(29910, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(29919, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(29934, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(29950, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(30009, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(30029, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(30050, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(30070, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(30084, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(30103, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(30120, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(30136, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(30147, 10), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(30157, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(30202, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(30248, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(30271, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(30298, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(30325, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(30352, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(30379, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(30410, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(30441, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(30452, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(30464, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(30480, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(30490, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(27647, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(30501, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(30518, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(30534, 18), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(30552, 15), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(30567, 14), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(30581, 17), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(30598, 18), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(30616, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(30628, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(30640, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(30652, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(30664, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(30673, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(30682, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(30691, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(30705, 14), IR(0, 0), IR(211, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(30719, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(30734, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(30749, 14), IR(0, 0), IR(211, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(30763, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(30780, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(30797, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(30822, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(30830, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(30838, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(30846, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(30851, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(30856, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(30861, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(30866, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(30871, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(30876, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(30881, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(30886, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(30891, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(30896, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(30901, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(30906, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(30911, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(30916, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(30934, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(30952, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(30970, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(30988, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(31006, 13), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(31019, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(31023, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(31033, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(31044, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(31057, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(31070, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(31074, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(31078, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(31084, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(31090, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(31099, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(31108, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(31119, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(31133, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(31141, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(31151, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(31164, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(31180, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(31190, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(31201, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(31212, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(31219, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(31226, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(31236, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(31249, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(31262, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(31280, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(31298, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(31308, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(31318, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(31333, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(31348, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(31358, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(31370, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(31383, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(31398, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(31411, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(31426, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(31442, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(31460, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(31478, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(31498, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(31519, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(31542, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(31560, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(31581, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(31598, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(31608, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(31619, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(31630, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(31634, 15), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(31649, 17), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(31666, 17), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(31683, 11), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(31694, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(31703, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(31708, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(31713, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(31720, 9), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(31729, 9), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(31738, 11), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(31749, 11), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(31760, 11), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(31771, 13), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(31784, 11), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(31795, 13), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(31808, 17), IR(0, 0), IR(121, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(31825, 19), IR(0, 0), IR(121, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(31881, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(31896, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(31910, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(31921, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(31933, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(31948, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(31970, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(31996, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(32013, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(32030, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(32041, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(32052, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(32063, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(32074, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(32085, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(32096, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(32106, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(32115, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(32125, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(32159, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(32204, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(32219, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(32225, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(32232, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(32295, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(32302, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(32307, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(32314, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(32326, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(32338, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(32352, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(32365, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(32380, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(32396, 9), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(32405, 9), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(32414, 15), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(32462, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(32472, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(32482, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(32492, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(32502, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(32512, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(32522, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(32532, 10), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(32542, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(32551, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(32561, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(32578, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(32596, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(32619, 24), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(32643, 15), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(32658, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(32674, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(32691, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(32709, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(32727, 28), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(32755, 29), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(32784, 20), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(32804, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(32825, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(32839, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(32853, 30), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(32883, 32), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(32915, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(32938, 40), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(32978, 12), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(32990, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(33003, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(33019, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(33032, 19), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(33051, 26), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(33077, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(33093, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(33106, 29), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(33135, 29), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(33164, 33), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(33197, 33), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(33230, 33), IR(0, 0), IR(227, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(33263, 33), IR(0, 0), IR(227, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(33296, 37), IR(0, 0), IR(227, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(33333, 37), IR(0, 0), IR(227, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(33370, 17), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(33387, 18), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(33405, 22), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(33427, 26), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(33453, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(33460, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(33481, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(33497, 16), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(33513, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(33520, 16), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(33536, 20), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(33556, 26), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(33582, 35), IR(0, 0), IR(161, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(33617, 25), IR(0, 0), IR(161, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(33642, 17), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(33659, 23), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(33682, 19), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(33701, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(33717, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(33733, 11), IR(0, 0), IR(1, 0), IR(186, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(33744, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(33765, 19), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(33784, 19), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(33803, 24), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(33827, 25), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(33852, 30), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6539, 22), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(33882, 29), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(33911, 32), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(33943, 30), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(33973, 29), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(34002, 29), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(9678, 23), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(34031, 26), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(34057, 25), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(34082, 27), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(34109, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(34129, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(34149, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(34169, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(34189, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(34209, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(34229, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(34249, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(34269, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(34289, 20), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(34309, 26), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(34335, 25), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(34360, 26), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(34386, 26), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(34412, 25), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(34437, 26), IR(0, 0), IR(234, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(34463, 29), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(34492, 24), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(34516, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(34528, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(34537, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(34549, 8), IR(0, 0), IR(238, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(34557, 23), IR(0, 0), IR(153, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(34580, 23), IR(0, 0), IR(162, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(34603, 25), IR(0, 0), IR(163, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(34628, 14), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(34684, 14), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(34698, 15), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(34713, 19), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(34732, 17), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(34749, 19), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(9969, 9), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(34768, 14), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(34782, 18), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(34800, 12), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(34812, 13), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(34825, 20), IR(0, 0), IR(3, 1), IR(196, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(34845, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(34867, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(34886, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(34908, 30), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6300, 18), IR(0, 0), IR(242, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(34938, 27), IR(0, 0), IR(242, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(34965, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(34990, 33), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(35023, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(35045, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(35064, 7), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(35071, 15), IR(0, 0), IR(244, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(35086, 15), IR(0, 0), IR(244, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(35101, 20), IR(0, 0), IR(244, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(14731, 25), IR(0, 0), IR(245, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(35121, 26), IR(0, 0), IR(242, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(35147, 26), IR(0, 0), IR(1, 0), IR(197, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(35173, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(35198, 12), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(35210, 19), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(35229, 35), IR(0, 0), IR(246, 2), IR(198, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(35264, 22), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(35286, 16), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(35349, 5), IR(171, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(35362, 5), IR(172, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(35375, 6), IR(173, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(35390, 11), IR(174, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(35415, 11), IR(175, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(35440, 12), IR(176, 1), IR(248, 1), IR(145, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(35467, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(35492, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(35517, 26), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(35597, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(35624, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(35651, 30), IR(0, 0), IR(249, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(35681, 34), IR(0, 0), IR(249, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(35715, 31), IR(0, 0), IR(249, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(35746, 16), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(35762, 22), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(35784, 21), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(35805, 32), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(35837, 31), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(35868, 19), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(35887, 31), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(35918, 24), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(35942, 19), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(35961, 23), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(35984, 23), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(36007, 17), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(36024, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(36053, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(36082, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(36111, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(36140, 30), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(36170, 28), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(36198, 20), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(36218, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(36241, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(36264, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(36287, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(36310, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(36333, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(36356, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(36379, 23), IR(0, 0), IR(226, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(36402, 21), IR(0, 0), IR(255, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(36423, 17), IR(0, 0), IR(255, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(36440, 13), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(36453, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(36478, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(36502, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(36527, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(36556, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(36583, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(36606, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(36625, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(36648, 26), IR(0, 0), IR(170, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(36674, 26), IR(0, 0), IR(170, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(36700, 14), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(36714, 17), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(36731, 28), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(36759, 20), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(36779, 18), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(36797, 10), IR(0, 0), IR(171, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(36807, 10), IR(0, 0), IR(171, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(479, 14), IR(36817, 27), IR(0, 0), IR(257, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(493, 13), IR(36844, 36), IR(0, 0), IR(257, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(506, 7), IR(36880, 28), IR(0, 0), IR(257, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(36908, 28), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(36936, 28), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(36964, 33), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(36997, 30), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(493, 13), IR(37027, 26), IR(0, 0), IR(257, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(37053, 39), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(37092, 44), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(37136, 23), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(513, 12), IR(37159, 20), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(493, 13), IR(37179, 21), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(513, 12), IR(37200, 30), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(525, 6), IR(37230, 22), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(37252, 25), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(37277, 26), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(37303, 25), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(37328, 22), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(37350, 29), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(37379, 28), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(37407, 25), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(37432, 34), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(37466, 32), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(37498, 29), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(37527, 22), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(37549, 22), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(37571, 19), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(37590, 17), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(37607, 18), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(531, 3), IR(37625, 29), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(37654, 24), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(37678, 16), IR(0, 0), IR(131, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(534, 7), IR(37694, 23), IR(0, 0), IR(259, 1), IR(74, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(37717, 16), IR(177, 1), IR(260, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(541, 13), IR(37757, 29), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(554, 7), IR(37786, 42), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(37828, 39), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(561, 16), IR(37867, 32), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(37899, 27), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(577, 4), IR(37926, 17), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(37943, 18), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(37961, 28), IR(178, 1), IR(191, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(38016, 33), IR(0, 0), IR(51, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(38049, 35), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(38084, 38), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(581, 5), IR(38122, 24), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(586, 4), IR(38146, 25), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(296, 4), IR(38171, 28), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(506, 7), IR(38199, 23), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(38222, 29), IR(0, 0), IR(266, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(38251, 51), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(38302, 24), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(38326, 45), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(590, 4), IR(38371, 33), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(594, 14), IR(38404, 32), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(608, 15), IR(38436, 38), IR(0, 0), IR(266, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(38474, 17), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(38491, 25), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(531, 3), IR(38516, 30), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(513, 12), IR(38546, 21), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(493, 13), IR(38567, 27), IR(0, 0), IR(266, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(38594, 24), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(38618, 26), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(38644, 27), IR(0, 0), IR(266, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(38671, 26), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(38697, 23), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(38720, 30), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(38750, 29), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(38779, 26), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(38805, 35), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(38840, 31), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(38871, 34), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(38905, 33), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(38938, 30), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(38968, 29), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(38997, 29), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(39026, 23), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(39049, 22), IR(179, 1), IR(55, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(39092, 21), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(39113, 15), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(39128, 8), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(513, 12), IR(39136, 14), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(513, 12), IR(39150, 17), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(39167, 50), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(39217, 29), IR(180, 1), IR(269, 4), IR(200, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(39274, 18), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(39292, 35), IR(181, 1), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(39350, 24), IR(0, 0), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(39374, 23), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(39397, 45), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(39442, 40), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(39482, 20), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(39502, 18), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(39520, 19), IR(0, 0), IR(132, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(623, 5), IR(39539, 24), IR(0, 0), IR(273, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(628, 6), IR(39563, 24), IR(0, 0), IR(273, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(634, 5), IR(39587, 25), IR(0, 0), IR(273, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(39612, 26), IR(0, 0), IR(273, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(39638, 26), IR(0, 0), IR(273, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(39664, 28), IR(0, 0), IR(274, 3), IR(83, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(39692, 26), IR(0, 0), IR(274, 3), IR(83, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(639, 5), IR(39761, 26), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(644, 7), IR(39862, 30), IR(0, 0), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(651, 5), IR(39892, 31), IR(0, 0), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(39923, 32), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(39955, 19), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(656, 4), IR(39974, 17), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(39991, 21), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(40012, 27), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(40039, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(40063, 20), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(40083, 28), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(40111, 19), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(40130, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(40155, 22), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(12546, 25), IR(46, 1), IR(280, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(40177, 22), IR(0, 0), IR(280, 1), IR(84, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(40199, 20), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(40219, 27), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(40246, 29), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(40275, 18), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(40293, 20), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(40313, 18), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(40331, 20), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(40351, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(40376, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(40401, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(660, 7), IR(40482, 17), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(40499, 40), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(40539, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(40577, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(40615, 34), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(40649, 37), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(40686, 29), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(40715, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(40742, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(40769, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(40792, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(40815, 20), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(40835, 22), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(40857, 19), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13078, 21), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(40876, 25), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(40901, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(40924, 24), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(40948, 23), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(40971, 24), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(40995, 28), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(41023, 29), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(41052, 33), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(667, 5), IR(41085, 34), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(41119, 24), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(41143, 25), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(41168, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(41181, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(41194, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(41207, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(41220, 14), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(41234, 14), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(41248, 21), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(41269, 21), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(41290, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(41303, 13), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(41316, 15), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(41331, 15), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(41346, 29), IR(0, 0), IR(206, 1), IR(102, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(41375, 25), IR(0, 0), IR(206, 1), IR(102, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(41400, 15), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(672, 6), IR(13296, 9), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(41415, 13), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(41428, 14), IR(0, 0), IR(286, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(41442, 14), IR(0, 0), IR(286, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(41456, 14), IR(0, 0), IR(290, 1), IR(106, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(41470, 10), IR(0, 0), IR(290, 1), IR(106, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(41480, 15), IR(182, 1), IR(1, 0), IR(204, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(41516, 21), IR(183, 1), IR(1, 0), IR(204, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(41564, 14), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(41578, 18), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(41596, 23), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(41619, 23), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(41642, 23), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(41665, 23), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(41688, 22), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(41710, 22), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(41732, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(41778, 44), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(41822, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(41861, 37), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(41898, 22), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(41920, 22), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(41942, 60), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(42002, 53), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(42055, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(42102, 40), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(42142, 51), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(42193, 44), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(42237, 51), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(42288, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42344, 50), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42394, 52), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42446, 49), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(42495, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(42542, 50), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(42592, 51), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(42643, 56), IR(0, 0), IR(294, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(42699, 33), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(42732, 52), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(42784, 61), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(42845, 61), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(42906, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(42945, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(42983, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(43022, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(43060, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(43099, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(43137, 36), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(43173, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(43212, 43), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(43255, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(43293, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(43331, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(43369, 45), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(43414, 40), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(43454, 62), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(43516, 30), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(43546, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(43584, 36), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(43620, 33), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(43653, 35), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(43688, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(43727, 43), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(43770, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(43817, 54), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(43871, 34), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(43905, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(43952, 45), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(43997, 55), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(44052, 53), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(44105, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(44161, 54), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(44215, 58), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(44273, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(44329, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(44367, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(44413, 44), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(44457, 49), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(44506, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(44553, 70), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(44623, 68), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(44691, 69), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(44760, 68), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(44828, 66), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(44894, 67), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(44961, 36), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(44997, 48), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(45045, 59), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(45104, 58), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(45162, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(678, 9), IR(45218, 30), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(687, 10), IR(45248, 30), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(45278, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(45317, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(45363, 43), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(45406, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(45453, 45), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(45498, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(45544, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(45600, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(45638, 30), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(45668, 32), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(687, 10), IR(45700, 36), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(697, 13), IR(45736, 42), IR(0, 0), IR(294, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(45778, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(45817, 39), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(45856, 44), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(45900, 48), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(45948, 50), IR(0, 0), IR(294, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(45998, 43), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(46041, 48), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(46089, 43), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(46132, 31), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46163, 47), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46210, 45), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46255, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(46301, 56), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(46357, 38), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(46395, 35), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(46430, 44), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(46474, 46), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(46520, 41), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(46561, 36), IR(0, 0), IR(294, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(46597, 50), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(46647, 48), IR(0, 0), IR(292, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(46695, 35), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(13534, 25), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(46730, 16), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(46746, 19), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(710, 8), IR(46765, 29), IR(184, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(710, 8), IR(46822, 25), IR(185, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(710, 8), IR(46871, 32), IR(186, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(710, 8), IR(46934, 30), IR(187, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(718, 10), IR(46993, 24), IR(188, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(718, 10), IR(47040, 24), IR(189, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(718, 10), IR(47087, 24), IR(190, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(718, 10), IR(47134, 24), IR(191, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(728, 6), IR(47181, 23), IR(192, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(728, 6), IR(47226, 23), IR(193, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(728, 6), IR(47271, 23), IR(194, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(728, 6), IR(47316, 23), IR(195, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(728, 6), IR(47361, 23), IR(196, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(710, 8), IR(47406, 26), IR(197, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(710, 8), IR(47457, 26), IR(198, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(710, 8), IR(47508, 25), IR(199, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(718, 10), IR(47557, 26), IR(200, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(710, 8), IR(47608, 25), IR(201, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(710, 8), IR(47657, 23), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(710, 8), IR(47680, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(710, 8), IR(47704, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(710, 8), IR(47729, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(710, 8), IR(47754, 23), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(710, 8), IR(47777, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(710, 8), IR(47801, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(710, 8), IR(47826, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(710, 8), IR(47851, 23), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(710, 8), IR(47874, 23), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(710, 8), IR(47897, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(710, 8), IR(47923, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(710, 8), IR(47948, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(710, 8), IR(47973, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(710, 8), IR(47997, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(710, 8), IR(48023, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(710, 8), IR(48047, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(710, 8), IR(48073, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(710, 8), IR(48097, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(718, 10), IR(48123, 25), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(718, 10), IR(48148, 23), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(718, 10), IR(48171, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(718, 10), IR(48195, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(48219, 17), IR(0, 0), IR(298, 1), IR(116, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(734, 2), IR(48236, 21), IR(0, 0), IR(142, 1), IR(128, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(736, 3), IR(48257, 20), IR(0, 0), IR(142, 1), IR(128, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(48277, 24), IR(0, 0), IR(142, 1), IR(128, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(710, 8), IR(48301, 16), IR(202, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(710, 8), IR(48332, 17), IR(203, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(710, 8), IR(48365, 17), IR(204, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(710, 8), IR(48398, 15), IR(205, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(710, 8), IR(48427, 15), IR(206, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(710, 8), IR(48456, 18), IR(207, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(710, 8), IR(48491, 17), IR(208, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(710, 8), IR(48524, 17), IR(209, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(710, 8), IR(48557, 20), IR(210, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(710, 8), IR(48596, 15), IR(211, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(710, 8), IR(48625, 15), IR(212, 1), IR(196, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(48654, 30), IR(213, 1), IR(207, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(48713, 30), IR(214, 1), IR(207, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, true, true, IR(172, 4), IR(48772, 23), IR(215, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, true, true, IR(172, 4), IR(48817, 24), IR(216, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(14492, 14), IR(62, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(48864, 22), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(48886, 23), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(48909, 28), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(48937, 46), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(48983, 37), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(49020, 65), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(49085, 40), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(49125, 41), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(49166, 39), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(49205, 36), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(49241, 46), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(49287, 45), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(49332, 42), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(49374, 32), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(49406, 29), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(49435, 40), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(49475, 40), IR(0, 0), IR(200, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(49515, 14), IR(0, 0), IR(301, 4), IR(152, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(49529, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(739, 1), IR(49552, 25), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(739, 1), IR(49577, 32), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(739, 1), IR(49609, 36), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(49645, 33), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(49678, 20), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(49698, 20), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(49718, 26), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(49744, 24), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(14945, 19), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(740, 7), IR(49768, 25), IR(217, 1), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(49817, 24), IR(218, 1), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(49864, 22), IR(219, 1), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(49907, 26), IR(220, 1), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(49958, 23), IR(221, 1), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(50003, 27), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(747, 10), IR(50030, 25), IR(0, 0), IR(110, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(747, 10), IR(50055, 34), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(747, 10), IR(50089, 34), IR(0, 0), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(757, 9), IR(50123, 29), IR(0, 0), IR(110, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(747, 10), IR(50152, 26), IR(0, 0), IR(110, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(766, 7), IR(15422, 38), IR(0, 0), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(50240, 21), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(15488, 32), IR(0, 0), IR(315, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(50261, 26), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(773, 5), IR(50287, 27), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(778, 2), IR(50314, 27), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(780, 4), IR(50341, 24), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(784, 6), IR(50365, 30), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(790, 3), IR(50432, 30), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(50462, 27), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(50489, 13), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(50502, 13), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(50515, 19), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(50534, 18), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(50552, 19), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(50571, 19), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(50590, 18), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(50608, 19), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(50627, 18), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(793, 6), IR(50645, 18), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(799, 4), IR(50663, 19), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(50682, 26), IR(0, 0), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(50708, 28), IR(0, 0), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(50736, 33), IR(0, 0), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(50769, 19), IR(0, 0), IR(321, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(50788, 19), IR(0, 0), IR(323, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(50807, 19), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 921> kInstructionNames{{
  {IR(41168, 13), 597}, // 0 AbsISubINTEL
  {IR(41181, 13), 598}, // 1 AbsUSubINTEL
  {IR(29777, 12), 58}, // 2 AccessChain
  {IR(48236, 21), 783}, // 3 AliasDomainDeclINTEL
  {IR(48257, 20), 784}, // 4 AliasScopeDeclINTEL
  {IR(48277, 24), 785}, // 5 AliasScopeListDeclINTEL
  {IR(31074, 4), 143}, // 6 All
  {IR(36453, 25), 426}, // 7 AllocateNodePayloadsAMDX
  {IR(31070, 4), 142}, // 8 Any
  {IR(48023, 24), 774}, // 9 ArbitraryFloatACosINTEL
  {IR(48047, 26), 775}, // 10 ArbitraryFloatACosPiINTEL
  {IR(47973, 24), 772}, // 11 ArbitraryFloatASinINTEL
  {IR(47997, 26), 773}, // 12 ArbitraryFloatASinPiINTEL
  {IR(48123, 25), 778}, // 13 ArbitraryFloatATan2INTEL
  {IR(48073, 24), 776}, // 14 ArbitraryFloatATanINTEL
  {IR(48097, 26), 777}, // 15 ArbitraryFloatATanPiINTEL
  {IR(46993, 24), 745}, // 16 ArbitraryFloatAddALTERA
  {IR(47017, 23), 745}, // 17 ArbitraryFloatAddINTEL
  {IR(46822, 25), 742}, // 18 ArbitraryFloatCastALTERA
  {IR(46871, 32), 743}, // 19 ArbitraryFloatCastFromIntALTERA
  {IR(46903, 31), 743}, // 20 ArbitraryFloatCastFromIntINTEL
  {IR(46847, 24), 742}, // 21 ArbitraryFloatCastINTEL
  {IR(46934, 30), 744}, // 22 ArbitraryFloatCastToIntALTERA
  {IR(46964, 29), 744}, // 23 ArbitraryFloatCastToIntINTEL
  {IR(47508, 25), 756}, // 24 ArbitraryFloatCbrtALTERA
  {IR(47533, 24), 756}, // 25 ArbitraryFloatCbrtINTEL
  {IR(47874, 23), 768}, // 26 ArbitraryFloatCosINTEL
  {IR(47948, 25), 771}, // 27 ArbitraryFloatCosPiINTEL
  {IR(47134, 24), 748}, // 28 ArbitraryFloatDivALTERA
  {IR(47158, 23), 748}, // 29 ArbitraryFloatDivINTEL
  {IR(47361, 23), 753}, // 30 ArbitraryFloatEQALTERA
  {IR(47384, 22), 753}, // 31 ArbitraryFloatEQINTEL
  {IR(47801, 25), 765}, // 32 ArbitraryFloatExp10INTEL
  {IR(47777, 24), 764}, // 33 ArbitraryFloatExp2INTEL
  {IR(47754, 23), 763}, // 34 ArbitraryFloatExpINTEL
  {IR(47826, 25), 766}, // 35 ArbitraryFloatExpm1INTEL
  {IR(47226, 23), 750}, // 36 ArbitraryFloatGEALTERA
  {IR(47249, 22), 750}, // 37 ArbitraryFloatGEINTEL
  {IR(47181, 23), 749}, // 38 ArbitraryFloatGTALTERA
  {IR(47204, 22), 749}, // 39 ArbitraryFloatGTINTEL
  {IR(47557, 26), 757}, // 40 ArbitraryFloatHypotALTERA
  {IR(47583, 25), 757}, // 41 ArbitraryFloatHypotINTEL
  {IR(47316, 23), 752}, // 42 ArbitraryFloatLEALTERA
  {IR(47339, 22), 752}, // 43 ArbitraryFloatLEINTEL
  {IR(47271, 23), 751}, // 44 ArbitraryFloatLTALTERA
  {IR(47294, 22), 751}, // 45 ArbitraryFloatLTINTEL
  {IR(47704, 25), 761}, // 46 ArbitraryFloatLog10INTEL
  {IR(47729, 25), 762}, // 47 ArbitraryFloatLog1pINTEL
  {IR(47680, 24), 760}, // 48 ArbitraryFloatLog2INTEL
  {IR(47657, 23), 759}, // 49 ArbitraryFloatLogINTEL
  {IR(47087, 24), 747}, // 50 ArbitraryFloatMulALTERA
  {IR(47111, 23), 747}, // 51 ArbitraryFloatMulINTEL
  {IR(48148, 23), 779}, // 52 ArbitraryFloatPowINTEL
  {IR(48195, 24), 781}, // 53 ArbitraryFloatPowNINTEL
  {IR(48171, 24), 780}, // 54 ArbitraryFloatPowRINTEL
  {IR(47457, 26), 755}, // 55 ArbitraryFloatRSqrtALTERA
  {IR(47483, 25), 755}, // 56 ArbitraryFloatRSqrtINTEL
  {IR(47406, 26), 754}, // 57 ArbitraryFloatRecipALTERA
  {IR(47432, 25), 754}, // 58 ArbitraryFloatRecipINTEL
  {IR(47897, 26), 769}, // 59 ArbitraryFloatSinCosINTEL
  {IR(46765, 29), 741}, // 60 ArbitraryFloatSinCosPiALTERA
  {IR(46794, 28), 741}, // 61 ArbitraryFloatSinCosPiINTEL
  {IR(47851, 23), 767}, // 62 ArbitraryFloatSinINTEL
  {IR(47923, 25), 770}, // 63 ArbitraryFloatSinPiINTEL
  {IR(47608, 25), 758}, // 64 ArbitraryFloatSqrtALTERA
  {IR(47633, 24), 758}, // 65 ArbitraryFloatSqrtINTEL
  {IR(47040, 24), 746}, // 66 ArbitraryFloatSubALTERA
  {IR(47064, 23), 746}, // 67 ArbitraryFloatSubINTEL
  {IR(14945, 19), 829}, // 68 ArithmeticFenceEXT
  {IR(29824, 12), 61}, // 69 ArrayLength
  {IR(41415, 13), 613}, // 70 AsmCallINTEL
  {IR(13296, 9), 612}, // 71 AsmINTEL
  {IR(41400, 15), 611}, // 72 AsmTargetINTEL
  {IR(41456, 14), 616}, // 73 AssumeTrueKHR
  {IR(32096, 10), 220}, // 74 AtomicAnd
  {IR(31948, 22), 210}, // 75 AtomicCompareExchange
  {IR(31970, 26), 211}, // 76 AtomicCompareExchangeWeak
  {IR(31933, 15), 209}, // 77 AtomicExchange
  {IR(49515, 14), 819}, // 78 AtomicFAddEXT
  {IR(41442, 14), 615}, // 79 AtomicFMaxEXT
  {IR(41428, 14), 614}, // 80 AtomicFMinEXT
  {IR(33481, 16), 292}, // 81 AtomicFlagClear
  {IR(33460, 21), 291}, // 82 AtomicFlagTestAndSet
  {IR(32030, 11), 214}, // 83 AtomicIAdd
  {IR(32013, 17), 213}, // 84 AtomicIDecrement
  {IR(31996, 17), 212}, // 85 AtomicIIncrement
  {IR(32041, 11), 215}, // 86 AtomicISub
  {IR(31910, 11), 207}, // 87 AtomicLoad
  {IR(32106, 9), 221}, // 88 AtomicOr
  {IR(32074, 11), 218}, // 89 AtomicSMax
  {IR(32052, 11), 216}, // 90 AtomicSMin
  {IR(31921, 12), 208}, // 91 AtomicStore
  {IR(32085, 11), 219}, // 92 AtomicUMax
  {IR(32063, 11), 217}, // 93 AtomicUMin
  {IR(32115, 10), 222}, // 94 AtomicXor
  {IR(39664, 28), 543}, // 95 BeginInvocationInterlockEXT
  {IR(36007, 17), 407}, // 96 BitCastArrayQCOM
  {IR(31694, 9), 191}, // 97 BitCount
  {IR(31634, 15), 187}, // 98 BitFieldInsert
  {IR(31649, 17), 188}, // 99 BitFieldSExtract
  {IR(31666, 17), 189}, // 100 BitFieldUExtract
  {IR(31683, 11), 190}, // 101 BitReverse
  {IR(30822, 8), 114}, // 102 Bitcast
  {IR(31619, 11), 185}, // 103 BitwiseAnd
  {IR(50240, 21), 842}, // 104 BitwiseFunctionINTEL
  {IR(31598, 10), 183}, // 105 BitwiseOr
  {IR(31608, 11), 184}, // 106 BitwiseXor
  {IR(32225, 7), 227}, // 107 Branch
  {IR(32232, 18), 228}, // 108 BranchConditional
  {IR(36714, 17), 437}, // 109 BufferPointerEXT
  {IR(33093, 13), 277}, // 110 BuildNDRange
  {IR(28838, 11), 15}, // 111 Capability
  {IR(33051, 26), 275}, // 112 CaptureEventProfilingInfo
  {IR(34557, 23), 344}, // 113 ColorAttachmentReadEXT
  {IR(32643, 15), 255}, // 114 CommitReadPipe
  {IR(32658, 16), 256}, // 115 CommitWritePipe
  {IR(30084, 19), 72}, // 116 CompositeConstruct
  {IR(49645, 33), 824}, // 117 CompositeConstructContinuedINTEL
  {IR(36140, 30), 412}, // 118 CompositeConstructCoopMatQCOM
  {IR(35715, 31), 395}, // 119 CompositeConstructReplicateEXT
  {IR(30103, 17), 73}, // 120 CompositeExtract
  {IR(36170, 28), 413}, // 121 CompositeExtractCoopMatQCOM
  {IR(30120, 16), 74}, // 122 CompositeInsert
  {IR(50314, 27), 846}, // 123 ConditionalCapabilityINTEL
  {IR(50462, 27), 850}, // 124 ConditionalCopyObjectINTEL
  {IR(50287, 27), 845}, // 125 ConditionalEntryPointINTEL
  {IR(50261, 26), 844}, // 126 ConditionalExtensionINTEL
  {IR(17203, 9), 39}, // 127 Constant
  {IR(29344, 18), 40}, // 128 ConstantComposite
  {IR(49577, 32), 822}, // 129 ConstantCompositeContinuedINTEL
  {IR(35651, 30), 393}, // 130 ConstantCompositeReplicateEXT
  {IR(29292, 14), 38}, // 131 ConstantFalse
  {IR(41346, 29), 609}, // 132 ConstantFunctionPointerINTEL
  {IR(29456, 13), 42}, // 133 ConstantNull
  {IR(33536, 20), 296}, // 134 ConstantPipeStorage
  {IR(29440, 16), 41}, // 135 ConstantSampler
  {IR(36779, 18), 440}, // 136 ConstantSizeOfEXT
  {IR(36606, 19), 432}, // 137 ConstantStringAMDX
  {IR(29279, 13), 37}, // 138 ConstantTrue
  {IR(31881, 15), 205}, // 139 ControlBarrier
  {IR(49718, 26), 827}, // 140 ControlBarrierArriveINTEL
  {IR(49744, 24), 828}, // 141 ControlBarrierWaitINTEL
  {IR(49698, 20), 826}, // 142 ConvertBF16ToFINTEL
  {IR(49678, 20), 825}, // 143 ConvertFToBF16INTEL
  {IR(30628, 12), 100}, // 144 ConvertFToS
  {IR(30616, 12), 99}, // 145 ConvertFToU
  {IR(50682, 26), 862}, // 146 ConvertHandleToImageINTEL
  {IR(50736, 33), 864}, // 147 ConvertHandleToSampledImageINTEL
  {IR(50708, 28), 863}, // 148 ConvertHandleToSamplerINTEL
  {IR(40313, 18), 566}, // 149 ConvertImageToUNV
  {IR(30705, 14), 107}, // 150 ConvertPtrToU
  {IR(30640, 12), 101}, // 151 ConvertSToF
  {IR(40376, 25), 569}, // 152 ConvertSampledImageToUNV
  {IR(40331, 20), 567}, // 153 ConvertSamplerToUNV
  {IR(35229, 35), 379}, // 154 ConvertUToAccelerationStructureKHR
  {IR(30652, 12), 102}, // 155 ConvertUToF
  {IR(40275, 18), 564}, // 156 ConvertUToImageNV
  {IR(30749, 14), 110}, // 157 ConvertUToPtr
  {IR(40351, 25), 568}, // 158 ConvertUToSampledImageNV
  {IR(40293, 20), 565}, // 159 ConvertUToSamplerNV
  {IR(37899, 27), 482}, // 160 CooperativeMatrixConvertNV
  {IR(35624, 27), 392}, // 161 CooperativeMatrixLengthKHR
  {IR(39638, 26), 542}, // 162 CooperativeMatrixLengthNV
  {IR(35492, 25), 389}, // 163 CooperativeMatrixLoadKHR
  {IR(39563, 24), 539}, // 164 CooperativeMatrixLoadNV
  {IR(39862, 30), 546}, // 165 CooperativeMatrixLoadTensorNV
  {IR(35597, 27), 391}, // 166 CooperativeMatrixMulAddKHR
  {IR(39612, 26), 541}, // 167 CooperativeMatrixMulAddNV
  {IR(39923, 32), 548}, // 168 CooperativeMatrixPerElementOpNV
  {IR(39761, 26), 545}, // 169 CooperativeMatrixReduceNV
  {IR(35517, 26), 390}, // 170 CooperativeMatrixStoreKHR
  {IR(39587, 25), 540}, // 171 CooperativeMatrixStoreNV
  {IR(39892, 31), 547}, // 172 CooperativeMatrixStoreTensorNV
  {IR(40246, 29), 563}, // 173 CooperativeMatrixTransposeNV
  {IR(38122, 24), 489}, // 174 CooperativeVectorLoadNV
  {IR(37867, 32), 481}, // 175 CooperativeVectorMatrixMulAddNV
  {IR(37757, 29), 478}, // 176 CooperativeVectorMatrixMulNV
  {IR(37786, 42), 479}, // 177 CooperativeVectorOuterProductAccumulateNV
  {IR(37828, 39), 480}, // 178 CooperativeVectorReduceSumAccumulateNV
  {IR(38146, 25), 490}, // 179 CooperativeVectorStoreNV
  {IR(34516, 12), 340}, // 180 CopyLogical
  {IR(29750, 11), 56}, // 181 CopyMemory
  {IR(29761, 16), 57}, // 182 CopyMemorySized
  {IR(30136, 11), 75}, // 183 CopyObject
  {IR(33556, 26), 297}, // 184 CreatePipeFromPipeStorage
  {IR(39991, 21), 551}, // 185 CreateTensorLayoutNV
  {IR(40111, 19), 556}, // 186 CreateTensorViewNV
  {IR(33003, 16), 272}, // 187 CreateUserEvent
  {IR(48713, 30), 798}, // 188 CrossWorkgroupCastToPtrALTERA
  {IR(48743, 29), 798}, // 189 CrossWorkgroupCastToPtrINTEL
  {IR(31703, 5), 192}, // 190 DPdx
  {IR(31749, 11), 198}, // 191 DPdxCoarse
  {IR(31720, 9), 195}, // 192 DPdxFine
  {IR(31708, 5), 193}, // 193 DPdy
  {IR(31760, 11), 199}, // 194 DPdyCoarse
  {IR(31729, 9), 196}, // 195 DPdyFine
  {IR(29910, 9), 64}, // 196 Decorate
  {IR(33733, 11), 305}, // 197 DecorateId
  {IR(41480, 15), 618}, // 198 DecorateString
  {IR(41495, 21), 618}, // 199 DecorateStringGOOGLE
  {IR(29934, 16), 66}, // 200 DecorationGroup
  {IR(12546, 25), 559}, // 201 DemoteToHelperInvocation
  {IR(12571, 28), 559}, // 202 DemoteToHelperInvocationEXT
  {IR(34580, 23), 345}, // 203 DepthAttachmentReadEXT
  {IR(31019, 4), 137}, // 204 Dot
  {IR(37926, 17), 483}, // 205 EmitMeshTasksEXT
  {IR(31808, 17), 203}, // 206 EmitStreamVertex
  {IR(31784, 11), 201}, // 207 EmitVertex
  {IR(39692, 26), 544}, // 208 EndInvocationInterlockEXT
  {IR(31795, 13), 202}, // 209 EndPrimitive
  {IR(31825, 19), 204}, // 210 EndStreamPrimitive
  {IR(32839, 14), 265}, // 211 EnqueueKernel
  {IR(32825, 14), 264}, // 212 EnqueueMarker
  {IR(36478, 24), 427}, // 213 EnqueueNodePayloadsAMDX
  {IR(28753, 11), 13}, // 214 EntryPoint
  {IR(35210, 19), 378}, // 215 ExecuteCallableKHR
  {IR(39274, 18), 529}, // 216 ExecuteCallableNV
  {IR(28796, 14), 14}, // 217 ExecutionMode
  {IR(33717, 16), 304}, // 218 ExecutionModeId
  {IR(41470, 10), 617}, // 219 ExpectKHR
  {IR(28607, 8), 11}, // 220 ExtInst
  {IR(28547, 14), 10}, // 221 ExtInstImport
  {IR(35147, 26), 375}, // 222 ExtInstWithForwardRefsKHR
  {IR(28537, 10), 9}, // 223 Extension
  {IR(36198, 20), 414}, // 224 ExtractSubArrayQCOM
  {IR(30851, 5), 118}, // 225 FAdd
  {IR(30682, 9), 105}, // 226 FConvert
  {IR(30886, 5), 125}, // 227 FDiv
  {IR(50788, 19), 866}, // 228 FDot2MixAcc16VALVE
  {IR(50769, 19), 865}, // 229 FDot2MixAcc32VALVE
  {IR(50807, 19), 867}, // 230 FDot4MixAcc32VALVE
  {IR(30911, 5), 130}, // 231 FMod
  {IR(30871, 5), 122}, // 232 FMul
  {IR(30838, 8), 116}, // 233 FNegate
  {IR(31348, 10), 168}, // 234 FOrdEqual
  {IR(31426, 16), 174}, // 235 FOrdGreaterThan
  {IR(31498, 21), 178}, // 236 FOrdGreaterThanEqual
  {IR(31398, 13), 172}, // 237 FOrdLessThan
  {IR(31460, 18), 176}, // 238 FOrdLessThanEqual
  {IR(31370, 13), 170}, // 239 FOrdNotEqual
  {IR(14492, 14), 801}, // 240 FPGARegALTERA
  {IR(14506, 13), 801}, // 241 FPGARegINTEL
  {IR(30906, 5), 129}, // 242 FRem
  {IR(30861, 5), 120}, // 243 FSub
  {IR(31358, 12), 169}, // 244 FUnordEqual
  {IR(31442, 18), 175}, // 245 FUnordGreaterThan
  {IR(31519, 23), 179}, // 246 FUnordGreaterThanEqual
  {IR(31411, 15), 173}, // 247 FUnordLessThan
  {IR(31478, 20), 177}, // 248 FUnordLessThanEqual
  {IR(31383, 15), 171}, // 249 FUnordNotEqual
  {IR(38084, 38), 488}, // 250 FetchMicroTriangleVertexBarycentricNV
  {IR(38049, 35), 487}, // 251 FetchMicroTriangleVertexPositionNV
  {IR(36527, 29), 429}, // 252 FinishWritingNodePayloadAMDX
  {IR(48427, 15), 790}, // 253 FixedCosALTERA
  {IR(48442, 14), 790}, // 254 FixedCosINTEL
  {IR(48524, 17), 793}, // 255 FixedCosPiALTERA
  {IR(48541, 16), 793}, // 256 FixedCosPiINTEL
  {IR(48625, 15), 796}, // 257 FixedExpALTERA
  {IR(48640, 14), 796}, // 258 FixedExpINTEL
  {IR(48596, 15), 795}, // 259 FixedLogALTERA
  {IR(48611, 14), 795}, // 260 FixedLogINTEL
  {IR(48332, 17), 787}, // 261 FixedRecipALTERA
  {IR(48349, 16), 787}, // 262 FixedRecipINTEL
  {IR(48365, 17), 788}, // 263 FixedRsqrtALTERA
  {IR(48382, 16), 788}, // 264 FixedRsqrtINTEL
  {IR(48398, 15), 789}, // 265 FixedSinALTERA
  {IR(48456, 18), 791}, // 266 FixedSinCosALTERA
  {IR(48474, 17), 791}, // 267 FixedSinCosINTEL
  {IR(48557, 20), 794}, // 268 FixedSinCosPiALTERA
  {IR(48577, 19), 794}, // 269 FixedSinCosPiINTEL
  {IR(48413, 14), 789}, // 270 FixedSinINTEL
  {IR(48491, 17), 792}, // 271 FixedSinPiALTERA
  {IR(48508, 16), 792}, // 272 FixedSinPiINTEL
  {IR(48301, 16), 786}, // 273 FixedSqrtALTERA
  {IR(48317, 15), 786}, // 274 FixedSqrtINTEL
  {IR(35064, 7), 369}, // 275 FmaKHR
  {IR(36423, 17), 424}, // 276 FragmentFetchAMD
  {IR(36402, 21), 423}, // 277 FragmentMaskFetchAMD
  {IR(36807, 10), 442}, // 278 FreezeKHR
  {IR(27603, 9), 48}, // 279 Function
  {IR(29659, 13), 51}, // 280 FunctionCall
  {IR(29647, 12), 50}, // 281 FunctionEnd
  {IR(29629, 18), 49}, // 282 FunctionParameter
  {IR(41375, 25), 610}, // 283 FunctionPointerCallINTEL
  {IR(31713, 7), 194}, // 284 Fwidth
  {IR(31771, 13), 200}, // 285 FwidthCoarse
  {IR(31738, 11), 197}, // 286 FwidthFine
  {IR(30780, 17), 112}, // 287 GenericCastToPtr
  {IR(30797, 25), 113}, // 288 GenericCastToPtrExplicit
  {IR(29836, 23), 62}, // 289 GenericPtrMemSemantics
  {IR(33077, 16), 276}, // 290 GetDefaultQueue
  {IR(33582, 35), 298}, // 291 GetKernelLocalSizeForSubgroupCount
  {IR(33617, 25), 299}, // 292 GetKernelMaxNumSubgroups
  {IR(32883, 32), 267}, // 293 GetKernelNDrangeMaxSubGroupSize
  {IR(32853, 30), 266}, // 294 GetKernelNDrangeSubGroupCount
  {IR(32938, 40), 269}, // 295 GetKernelPreferredWorkGroupSizeMultiple
  {IR(32915, 23), 268}, // 296 GetKernelWorkGroupSize
  {IR(32709, 18), 259}, // 297 GetMaxPipePackets
  {IR(32691, 18), 258}, // 298 GetNumPipePackets
  {IR(9969, 9), 353}, // 299 GraphARM
  {IR(34732, 17), 351}, // 300 GraphConstantARM
  {IR(34800, 12), 356}, // 301 GraphEndARM
  {IR(34749, 19), 352}, // 302 GraphEntryPointARM
  {IR(34768, 14), 354}, // 303 GraphInputARM
  {IR(34782, 18), 355}, // 304 GraphSetOutputARM
  {IR(32396, 9), 238}, // 305 GroupAll
  {IR(32405, 9), 239}, // 306 GroupAny
  {IR(32365, 15), 236}, // 307 GroupAsyncCopy
  {IR(50515, 19), 853}, // 308 GroupBitwiseAndKHR
  {IR(50534, 18), 854}, // 309 GroupBitwiseOrKHR
  {IR(50552, 19), 855}, // 310 GroupBitwiseXorKHR
  {IR(32414, 15), 240}, // 311 GroupBroadcast
  {IR(32784, 20), 262}, // 312 GroupCommitReadPipe
  {IR(32804, 21), 263}, // 313 GroupCommitWritePipe
  {IR(29950, 14), 67}, // 314 GroupDecorate
  {IR(32472, 10), 242}, // 315 GroupFAdd
  {IR(36241, 23), 416}, // 316 GroupFAddNonUniformAMD
  {IR(32512, 10), 246}, // 317 GroupFMax
  {IR(36333, 23), 420}, // 318 GroupFMaxNonUniformAMD
  {IR(32482, 10), 243}, // 319 GroupFMin
  {IR(36264, 23), 417}, // 320 GroupFMinNonUniformAMD
  {IR(50502, 13), 852}, // 321 GroupFMulKHR
  {IR(32462, 10), 241}, // 322 GroupIAdd
  {IR(36218, 23), 415}, // 323 GroupIAddNonUniformAMD
  {IR(50489, 13), 851}, // 324 GroupIMulKHR
  {IR(50571, 19), 856}, // 325 GroupLogicalAndKHR
  {IR(50590, 18), 857}, // 326 GroupLogicalOrKHR
  {IR(50608, 19), 858}, // 327 GroupLogicalXorKHR
  {IR(30009, 20), 68}, // 328 GroupMemberDecorate
  {IR(33765, 19), 307}, // 329 GroupNonUniformAll
  {IR(33803, 24), 309}, // 330 GroupNonUniformAllEqual
  {IR(33784, 19), 308}, // 331 GroupNonUniformAny
  {IR(6539, 22), 312}, // 332 GroupNonUniformBallot
  {IR(33943, 30), 315}, // 333 GroupNonUniformBallotBitCount
  {IR(33911, 32), 314}, // 334 GroupNonUniformBallotBitExtract
  {IR(33973, 29), 316}, // 335 GroupNonUniformBallotFindLSB
  {IR(34002, 29), 317}, // 336 GroupNonUniformBallotFindMSB
  {IR(34309, 26), 332}, // 337 GroupNonUniformBitwiseAnd
  {IR(34335, 25), 333}, // 338 GroupNonUniformBitwiseOr
  {IR(34360, 26), 334}, // 339 GroupNonUniformBitwiseXor
  {IR(33827, 25), 310}, // 340 GroupNonUniformBroadcast
  {IR(33852, 30), 311}, // 341 GroupNonUniformBroadcastFirst
  {IR(33744, 21), 306}, // 342 GroupNonUniformElect
  {IR(34129, 20), 323}, // 343 GroupNonUniformFAdd
  {IR(34289, 20), 331}, // 344 GroupNonUniformFMax
  {IR(34229, 20), 328}, // 345 GroupNonUniformFMin
  {IR(34169, 20), 325}, // 346 GroupNonUniformFMul
  {IR(34109, 20), 322}, // 347 GroupNonUniformIAdd
  {IR(34149, 20), 324}, // 348 GroupNonUniformIMul
  {IR(33882, 29), 313}, // 349 GroupNonUniformInverseBallot
  {IR(34386, 26), 335}, // 350 GroupNonUniformLogicalAnd
  {IR(34412, 25), 336}, // 351 GroupNonUniformLogicalOr
  {IR(34437, 26), 337}, // 352 GroupNonUniformLogicalXor
  {IR(37961, 28), 485}, // 353 GroupNonUniformPartitionEXT
  {IR(37989, 27), 485}, // 354 GroupNonUniformPartitionNV
  {IR(36648, 26), 434}, // 355 GroupNonUniformQuadAllKHR
  {IR(36674, 26), 435}, // 356 GroupNonUniformQuadAnyKHR
  {IR(34463, 29), 338}, // 357 GroupNonUniformQuadBroadcast
  {IR(34492, 24), 339}, // 358 GroupNonUniformQuadSwap
  {IR(14731, 25), 373}, // 359 GroupNonUniformRotateKHR
  {IR(34249, 20), 329}, // 360 GroupNonUniformSMax
  {IR(34189, 20), 326}, // 361 GroupNonUniformSMin
  {IR(9678, 23), 318}, // 362 GroupNonUniformShuffle
  {IR(34082, 27), 321}, // 363 GroupNonUniformShuffleDown
  {IR(34057, 25), 320}, // 364 GroupNonUniformShuffleUp
  {IR(34031, 26), 319}, // 365 GroupNonUniformShuffleXor
  {IR(34269, 20), 330}, // 366 GroupNonUniformUMax
  {IR(34209, 20), 327}, // 367 GroupNonUniformUMin
  {IR(32727, 28), 260}, // 368 GroupReserveReadPipePackets
  {IR(32755, 29), 261}, // 369 GroupReserveWritePipePackets
  {IR(32532, 10), 248}, // 370 GroupSMax
  {IR(36379, 23), 422}, // 371 GroupSMaxNonUniformAMD
  {IR(32502, 10), 245}, // 372 GroupSMin
  {IR(36310, 23), 419}, // 373 GroupSMinNonUniformAMD
  {IR(32522, 10), 247}, // 374 GroupUMax
  {IR(36356, 23), 421}, // 375 GroupUMaxNonUniformAMD
  {IR(32492, 10), 244}, // 376 GroupUMin
  {IR(36287, 23), 418}, // 377 GroupUMinNonUniformAMD
  {IR(32380, 16), 237}, // 378 GroupWaitEvents
  {IR(38618, 26), 506}, // 379 HitObjectExecuteShaderEXT
  {IR(37252, 25), 458}, // 380 HitObjectExecuteShaderNV
  {IR(38671, 26), 508}, // 381 HitObjectGetAttributesEXT
  {IR(37303, 25), 460}, // 382 HitObjectGetAttributesNV
  {IR(39350, 24), 531}, // 383 HitObjectGetClusterIdNV
  {IR(38644, 27), 507}, // 384 HitObjectGetCurrentTimeEXT
  {IR(37277, 26), 459}, // 385 HitObjectGetCurrentTimeNV
  {IR(38750, 29), 511}, // 386 HitObjectGetGeometryIndexEXT
  {IR(37379, 28), 463}, // 387 HitObjectGetGeometryIndexNV
  {IR(38697, 23), 509}, // 388 HitObjectGetHitKindEXT
  {IR(37328, 22), 461}, // 389 HitObjectGetHitKindNV
  {IR(38805, 35), 513}, // 390 HitObjectGetInstanceCustomIndexEXT
  {IR(37432, 34), 465}, // 391 HitObjectGetInstanceCustomIndexNV
  {IR(38779, 26), 512}, // 392 HitObjectGetInstanceIdEXT
  {IR(37407, 25), 464}, // 393 HitObjectGetInstanceIdNV
  {IR(38251, 51), 494}, // 394 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(40742, 27), 579}, // 395 HitObjectGetLSSPositionsNV
  {IR(40769, 23), 580}, // 396 HitObjectGetLSSRadiiNV
  {IR(38871, 34), 515}, // 397 HitObjectGetObjectRayDirectionEXT
  {IR(36964, 33), 448}, // 398 HitObjectGetObjectRayDirectionNV
  {IR(38840, 31), 514}, // 399 HitObjectGetObjectRayOriginEXT
  {IR(36997, 30), 449}, // 400 HitObjectGetObjectRayOriginNV
  {IR(38968, 29), 518}, // 401 HitObjectGetObjectToWorldEXT
  {IR(36936, 28), 447}, // 402 HitObjectGetObjectToWorldNV
  {IR(38720, 30), 510}, // 403 HitObjectGetPrimitiveIndexEXT
  {IR(37350, 29), 462}, // 404 HitObjectGetPrimitiveIndexNV
  {IR(38302, 24), 495}, // 405 HitObjectGetRayFlagsEXT
  {IR(39026, 23), 520}, // 406 HitObjectGetRayTMaxEXT
  {IR(37527, 22), 468}, // 407 HitObjectGetRayTMaxNV
  {IR(39374, 23), 532}, // 408 HitObjectGetRayTMinEXT
  {IR(37549, 22), 469}, // 409 HitObjectGetRayTMinNV
  {IR(39397, 45), 533}, // 410 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(37092, 44), 452}, // 411 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(39442, 40), 534}, // 412 HitObjectGetShaderRecordBufferHandleEXT
  {IR(37053, 39), 451}, // 413 HitObjectGetShaderRecordBufferHandleNV
  {IR(40686, 29), 577}, // 414 HitObjectGetSpherePositionNV
  {IR(40715, 27), 578}, // 415 HitObjectGetSphereRadiusNV
  {IR(38905, 33), 516}, // 416 HitObjectGetWorldRayDirectionEXT
  {IR(37466, 32), 466}, // 417 HitObjectGetWorldRayDirectionNV
  {IR(38938, 30), 517}, // 418 HitObjectGetWorldRayOriginEXT
  {IR(37498, 29), 467}, // 419 HitObjectGetWorldRayOriginNV
  {IR(38997, 29), 519}, // 420 HitObjectGetWorldToObjectEXT
  {IR(36908, 28), 446}, // 421 HitObjectGetWorldToObjectNV
  {IR(39482, 20), 535}, // 422 HitObjectIsEmptyEXT
  {IR(37571, 19), 470}, // 423 HitObjectIsEmptyNV
  {IR(39502, 18), 536}, // 424 HitObjectIsHitEXT
  {IR(37590, 17), 471}, // 425 HitObjectIsHitNV
  {IR(40815, 20), 582}, // 426 HitObjectIsLSSHitNV
  {IR(39520, 19), 537}, // 427 HitObjectIsMissEXT
  {IR(37607, 18), 472}, // 428 HitObjectIsMissNV
  {IR(40792, 23), 581}, // 429 HitObjectIsSphereHitNV
  {IR(38594, 24), 505}, // 430 HitObjectRecordEmptyEXT
  {IR(37136, 23), 453}, // 431 HitObjectRecordEmptyNV
  {IR(38171, 28), 491}, // 432 HitObjectRecordFromQueryEXT
  {IR(36817, 27), 443}, // 433 HitObjectRecordHitMotionNV
  {IR(37179, 21), 455}, // 434 HitObjectRecordHitNV
  {IR(36844, 36), 444}, // 435 HitObjectRecordHitWithIndexMotionNV
  {IR(37200, 30), 456}, // 436 HitObjectRecordHitWithIndexNV
  {IR(38199, 23), 492}, // 437 HitObjectRecordMissEXT
  {IR(38222, 29), 493}, // 438 HitObjectRecordMissMotionEXT
  {IR(36880, 28), 445}, // 439 HitObjectRecordMissMotionNV
  {IR(37230, 22), 457}, // 440 HitObjectRecordMissNV
  {IR(38371, 33), 497}, // 441 HitObjectReorderExecuteShaderEXT
  {IR(38326, 45), 496}, // 442 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(38436, 38), 499}, // 443 HitObjectTraceMotionReorderExecuteEXT
  {IR(38546, 21), 503}, // 444 HitObjectTraceRayEXT
  {IR(38567, 27), 504}, // 445 HitObjectTraceRayMotionEXT
  {IR(37027, 26), 450}, // 446 HitObjectTraceRayMotionNV
  {IR(37159, 20), 454}, // 447 HitObjectTraceRayNV
  {IR(38404, 32), 498}, // 448 HitObjectTraceReorderExecuteEXT
  {IR(30846, 5), 117}, // 449 IAdd
  {IR(31023, 10), 138}, // 450 IAddCarry
  {IR(41194, 13), 599}, // 451 IAddSatINTEL
  {IR(41220, 14), 601}, // 452 IAverageINTEL
  {IR(41248, 21), 603}, // 453 IAverageRoundedINTEL
  {IR(31219, 7), 158}, // 454 IEqual
  {IR(30866, 5), 121}, // 455 IMul
  {IR(41316, 15), 607}, // 456 IMul32x16INTEL
  {IR(31226, 10), 159}, // 457 INotEqual
  {IR(30856, 5), 119}, // 458 ISub
  {IR(31033, 11), 139}, // 459 ISubBorrow
  {IR(41290, 13), 605}, // 460 ISubSatINTEL
  {IR(35264, 22), 380}, // 461 IgnoreIntersectionKHR
  {IR(39092, 21), 522}, // 462 IgnoreIntersectionNV
  {IR(27647, 6), 91}, // 463 Image
  {IR(36111, 29), 411}, // 464 ImageBlockMatchGatherSADQCOM
  {IR(36082, 29), 410}, // 465 ImageBlockMatchGatherSSDQCOM
  {IR(35984, 23), 406}, // 466 ImageBlockMatchSADQCOM
  {IR(35961, 23), 405}, // 467 ImageBlockMatchSSDQCOM
  {IR(36053, 29), 409}, // 468 ImageBlockMatchWindowSADQCOM
  {IR(36024, 29), 408}, // 469 ImageBlockMatchWindowSSDQCOM
  {IR(35942, 19), 404}, // 470 ImageBoxFilterQCOM
  {IR(30464, 16), 88}, // 471 ImageDrefGather
  {IR(30441, 11), 86}, // 472 ImageFetch
  {IR(30452, 12), 87}, // 473 ImageGather
  {IR(30501, 17), 92}, // 474 ImageQueryFormat
  {IR(30581, 17), 97}, // 475 ImageQueryLevels
  {IR(30567, 14), 96}, // 476 ImageQueryLod
  {IR(30518, 16), 93}, // 477 ImageQueryOrder
  {IR(30598, 18), 98}, // 478 ImageQuerySamples
  {IR(30552, 15), 95}, // 479 ImageQuerySize
  {IR(30534, 18), 94}, // 480 ImageQuerySizeLod
  {IR(30480, 10), 89}, // 481 ImageRead
  {IR(30298, 27), 81}, // 482 ImageSampleDrefExplicitLod
  {IR(30271, 27), 80}, // 483 ImageSampleDrefImplicitLod
  {IR(30248, 23), 79}, // 484 ImageSampleExplicitLod
  {IR(37694, 23), 476}, // 485 ImageSampleFootprintNV
  {IR(30202, 23), 78}, // 486 ImageSampleImplicitLod
  {IR(30410, 31), 85}, // 487 ImageSampleProjDrefExplicitLod
  {IR(30379, 31), 84}, // 488 ImageSampleProjDrefImplicitLod
  {IR(30352, 27), 83}, // 489 ImageSampleProjExplicitLod
  {IR(30325, 27), 82}, // 490 ImageSampleProjImplicitLod
  {IR(35918, 24), 403}, // 491 ImageSampleWeightedQCOM
  {IR(33405, 22), 288}, // 492 ImageSparseDrefGather
  {IR(33370, 17), 286}, // 493 ImageSparseFetch
  {IR(33387, 18), 287}, // 494 ImageSparseGather
  {IR(33497, 16), 293}, // 495 ImageSparseRead
  {IR(33197, 33), 281}, // 496 ImageSparseSampleDrefExplicitLod
  {IR(33164, 33), 280}, // 497 ImageSparseSampleDrefImplicitLod
  {IR(33135, 29), 279}, // 498 ImageSparseSampleExplicitLod
  {IR(33106, 29), 278}, // 499 ImageSparseSampleImplicitLod
  {IR(33333, 37), 285}, // 500 ImageSparseSampleProjDrefExplicitLod
  {IR(33296, 37), 284}, // 501 ImageSparseSampleProjDrefImplicitLod
  {IR(33263, 33), 283}, // 502 ImageSparseSampleProjExplicitLod
  {IR(33230, 33), 282}, // 503 ImageSparseSampleProjImplicitLod
  {IR(33427, 26), 289}, // 504 ImageSparseTexelsResident
  {IR(29681, 18), 53}, // 505 ImageTexelPointer
  {IR(30490, 11), 90}, // 506 ImageWrite
  {IR(29789, 20), 59}, // 507 InBoundsAccessChain
  {IR(29859, 23), 63}, // 508 InBoundsPtrAccessChain
  {IR(31090, 9), 146}, // 509 IsFinite
  {IR(40177, 22), 560}, // 510 IsHelperInvocationEXT
  {IR(31084, 6), 145}, // 511 IsInf
  {IR(31078, 6), 144}, // 512 IsNan
  {IR(36583, 23), 431}, // 513 IsNodePayloadValidAMDX
  {IR(31099, 9), 147}, // 514 IsNormal
  {IR(33019, 13), 273}, // 515 IsValidEvent
  {IR(32674, 17), 257}, // 516 IsValidReserveId
  {IR(32302, 5), 230}, // 517 Kill
  {IR(32219, 6), 226}, // 518 Label
  {IR(31119, 14), 149}, // 519 LessOrGreater
  {IR(32338, 14), 234}, // 520 LifetimeStart
  {IR(32352, 13), 235}, // 521 LifetimeStop
  {IR(28532, 5), 8}, // 522 Line
  {IR(29739, 5), 54}, // 523 Load
  {IR(31190, 11), 155}, // 524 LogicalAnd
  {IR(31151, 13), 152}, // 525 LogicalEqual
  {IR(31201, 11), 156}, // 526 LogicalNot
  {IR(31164, 16), 153}, // 527 LogicalNotEqual
  {IR(31180, 10), 154}, // 528 LogicalOr
  {IR(48219, 17), 782}, // 529 LoopControlINTEL
  {IR(32159, 10), 224}, // 530 LoopMerge
  {IR(50645, 18), 860}, // 531 MaskedGatherINTEL
  {IR(50663, 19), 861}, // 532 MaskedScatterINTEL
  {IR(30988, 18), 135}, // 533 MatrixTimesMatrix
  {IR(30934, 18), 132}, // 534 MatrixTimesScalar
  {IR(30970, 18), 134}, // 535 MatrixTimesVector
  {IR(29919, 15), 65}, // 536 MemberDecorate
  {IR(36759, 20), 439}, // 537 MemberDecorateIdEXT
  {IR(41516, 21), 619}, // 538 MemberDecorateString
  {IR(41537, 27), 619}, // 539 MemberDecorateStringGOOGLE
  {IR(28514, 11), 6}, // 540 MemberName
  {IR(31896, 14), 206}, // 541 MemoryBarrier
  {IR(28679, 12), 12}, // 542 MemoryModel
  {IR(33682, 19), 302}, // 543 MemoryNamedBarrier
  {IR(33701, 16), 303}, // 544 ModuleProcessed
  {IR(28509, 5), 5}, // 545 Name
  {IR(33659, 23), 301}, // 546 NamedBarrierInitialize
  {IR(33453, 7), 290}, // 547 NoLine
  {IR(36556, 27), 430}, // 548 NodePayloadArrayLengthAMDX
  {IR(28305, 4), 0}, // 549 Nop
  {IR(31630, 4), 186}, // 550 Not
  {IR(31133, 8), 150}, // 551 Ordered
  {IR(31006, 13), 136}, // 552 OuterProduct
  {IR(32125, 4), 223}, // 553 Phi
  {IR(36797, 10), 441}, // 554 PoisonKHR
  {IR(29809, 15), 60}, // 555 PtrAccessChain
  {IR(48654, 30), 797}, // 556 PtrCastToCrossWorkgroupALTERA
  {IR(48684, 29), 797}, // 557 PtrCastToCrossWorkgroupINTEL
  {IR(30763, 17), 111}, // 558 PtrCastToGeneric
  {IR(34549, 8), 343}, // 559 PtrDiff
  {IR(34528, 9), 341}, // 560 PtrEqual
  {IR(34537, 12), 342}, // 561 PtrNotEqual
  {IR(30691, 14), 106}, // 562 QuantizeToF16
  {IR(40482, 17), 571}, // 563 RawAccessChainNV
  {IR(35837, 31), 400}, // 564 RayQueryConfirmIntersectionKHR
  {IR(35805, 32), 399}, // 565 RayQueryGenerateIntersectionKHR
  {IR(39327, 23), 530}, // 566 RayQueryGetClusterIdNV
  {IR(49166, 39), 810}, // 567 RayQueryGetIntersectionBarycentricsKHR
  {IR(49241, 46), 812}, // 568 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(39292, 35), 530}, // 569 RayQueryGetIntersectionClusterIdNV
  {IR(49205, 36), 811}, // 570 RayQueryGetIntersectionFrontFaceKHR
  {IR(49085, 40), 808}, // 571 RayQueryGetIntersectionGeometryIndexKHR
  {IR(48937, 46), 805}, // 572 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(48983, 37), 806}, // 573 RayQueryGetIntersectionInstanceIdKHR
  {IR(49020, 65), 807}, // 574 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(40649, 37), 576}, // 575 RayQueryGetIntersectionLSSHitValueNV
  {IR(40577, 38), 574}, // 576 RayQueryGetIntersectionLSSPositionsNV
  {IR(40615, 34), 575}, // 577 RayQueryGetIntersectionLSSRadiiNV
  {IR(49287, 45), 813}, // 578 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(49332, 42), 814}, // 579 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(49435, 40), 817}, // 580 RayQueryGetIntersectionObjectToWorldKHR
  {IR(49125, 41), 809}, // 581 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(40499, 40), 572}, // 582 RayQueryGetIntersectionSpherePositionNV
  {IR(40539, 38), 573}, // 583 RayQueryGetIntersectionSphereRadiusNV
  {IR(48909, 28), 804}, // 584 RayQueryGetIntersectionTKHR
  {IR(39167, 50), 527}, // 585 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(35887, 31), 402}, // 586 RayQueryGetIntersectionTypeKHR
  {IR(49475, 40), 818}, // 587 RayQueryGetIntersectionWorldToObjectKHR
  {IR(48886, 23), 803}, // 588 RayQueryGetRayFlagsKHR
  {IR(48864, 22), 802}, // 589 RayQueryGetRayTMinKHR
  {IR(49374, 32), 815}, // 590 RayQueryGetWorldRayDirectionKHR
  {IR(49406, 29), 816}, // 591 RayQueryGetWorldRayOriginKHR
  {IR(35762, 22), 397}, // 592 RayQueryInitializeKHR
  {IR(40857, 19), 584}, // 593 RayQueryIsLSSHitNV
  {IR(40835, 22), 583}, // 594 RayQueryIsSphereHitNV
  {IR(35868, 19), 401}, // 595 RayQueryProceedKHR
  {IR(35784, 21), 398}, // 596 RayQueryTerminateKHR
  {IR(36440, 13), 425}, // 597 ReadClockKHR
  {IR(32542, 9), 249}, // 598 ReadPipe
  {IR(48772, 23), 799}, // 599 ReadPipeBlockingALTERA
  {IR(48795, 22), 799}, // 600 ReadPipeBlockingINTEL
  {IR(32990, 13), 271}, // 601 ReleaseEvent
  {IR(38491, 25), 501}, // 602 ReorderThreadWithHintEXT
  {IR(37654, 24), 474}, // 603 ReorderThreadWithHintNV
  {IR(38516, 30), 502}, // 604 ReorderThreadWithHitObjectEXT
  {IR(37625, 29), 473}, // 605 ReorderThreadWithHitObjectNV
  {IR(39049, 22), 521}, // 606 ReportIntersectionKHR
  {IR(39071, 21), 521}, // 607 ReportIntersectionNV
  {IR(32596, 23), 253}, // 608 ReserveReadPipePackets
  {IR(32619, 24), 254}, // 609 ReserveWritePipePackets
  {IR(32561, 17), 251}, // 610 ReservedReadPipe
  {IR(32578, 18), 252}, // 611 ReservedWritePipe
  {IR(46746, 19), 740}, // 612 RestoreMemoryINTEL
  {IR(32978, 12), 270}, // 613 RetainEvent
  {IR(32307, 7), 231}, // 614 Return
  {IR(32314, 12), 232}, // 615 ReturnValue
  {IR(50627, 18), 859}, // 616 RoundFToTF32INTEL
  {IR(30673, 9), 104}, // 617 SConvert
  {IR(30881, 5), 124}, // 618 SDiv
  {IR(35349, 5), 382}, // 619 SDot
  {IR(35390, 11), 385}, // 620 SDotAccSat
  {IR(35401, 14), 385}, // 621 SDotAccSatKHR
  {IR(35354, 8), 382}, // 622 SDotKHR
  {IR(31249, 13), 161}, // 623 SGreaterThan
  {IR(31280, 18), 163}, // 624 SGreaterThanEqual
  {IR(31308, 10), 165}, // 625 SLessThan
  {IR(31333, 15), 167}, // 626 SLessThanEqual
  {IR(30901, 5), 128}, // 627 SMod
  {IR(31057, 13), 141}, // 628 SMulExtended
  {IR(30830, 8), 115}, // 629 SNegate
  {IR(30896, 5), 127}, // 630 SRem
  {IR(35375, 6), 384}, // 631 SUDot
  {IR(35440, 12), 387}, // 632 SUDotAccSat
  {IR(35452, 15), 387}, // 633 SUDotAccSatKHR
  {IR(35381, 9), 384}, // 634 SUDotKHR
  {IR(30157, 13), 77}, // 635 SampledImage
  {IR(40401, 29), 570}, // 636 SamplerImageAddressingModeNV
  {IR(30719, 15), 108}, // 637 SatConvertSToU
  {IR(30734, 15), 109}, // 638 SatConvertUToS
  {IR(46730, 16), 739}, // 639 SaveMemoryINTEL
  {IR(31212, 7), 157}, // 640 Select
  {IR(32204, 15), 225}, // 641 SelectionMerge
  {IR(37943, 18), 484}, // 642 SetMeshOutputsEXT
  {IR(33032, 19), 274}, // 643 SetUserEventStatus
  {IR(31581, 17), 182}, // 644 ShiftLeftLogical
  {IR(31560, 21), 181}, // 645 ShiftRightArithmetic
  {IR(31542, 18), 180}, // 646 ShiftRightLogical
  {IR(31108, 11), 148}, // 647 SignBitSet
  {IR(33513, 7), 294}, // 648 SizeOf
  {IR(28486, 7), 3}, // 649 Source
  {IR(28367, 16), 2}, // 650 SourceContinued
  {IR(28493, 16), 4}, // 651 SourceExtension
  {IR(29504, 13), 45}, // 652 SpecConstant
  {IR(50365, 30), 848}, // 653 SpecConstantArchitectureINTEL
  {IR(50432, 30), 849}, // 654 SpecConstantCapabilitiesINTEL
  {IR(29517, 22), 46}, // 655 SpecConstantComposite
  {IR(49609, 36), 823}, // 656 SpecConstantCompositeContinuedINTEL
  {IR(35681, 34), 394}, // 657 SpecConstantCompositeReplicateEXT
  {IR(29486, 18), 44}, // 658 SpecConstantFalse
  {IR(29580, 15), 47}, // 659 SpecConstantOp
  {IR(36625, 23), 433}, // 660 SpecConstantStringAMDX
  {IR(50341, 24), 847}, // 661 SpecConstantTargetINTEL
  {IR(29469, 17), 43}, // 662 SpecConstantTrue
  {IR(34603, 25), 346}, // 663 StencilAttachmentReadEXT
  {IR(29744, 6), 55}, // 664 Store
  {IR(28525, 7), 7}, // 665 String
  {IR(50030, 25), 836}, // 666 Subgroup2DBlockLoadINTEL
  {IR(50055, 34), 837}, // 667 Subgroup2DBlockLoadTransformINTEL
  {IR(50089, 34), 838}, // 668 Subgroup2DBlockLoadTransposeINTEL
  {IR(50123, 29), 839}, // 669 Subgroup2DBlockPrefetchINTEL
  {IR(50152, 26), 840}, // 670 Subgroup2DBlockStoreINTEL
  {IR(35101, 20), 372}, // 671 SubgroupAllEqualKHR
  {IR(35071, 15), 370}, // 672 SubgroupAllKHR
  {IR(35086, 15), 371}, // 673 SubgroupAnyKHR
  {IR(45248, 30), 703}, // 674 SubgroupAvcBmeInitializeINTEL
  {IR(45218, 30), 702}, // 675 SubgroupAvcFmeInitializeINTEL
  {IR(43653, 35), 672}, // 676 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(43688, 39), 673}, // 677 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(44329, 38), 686}, // 678 SubgroupAvcImeConvertToMceResultINTEL
  {IR(43952, 45), 679}, // 679 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(44052, 53), 681}, // 680 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(44273, 56), 685}, // 681 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(44161, 54), 683}, // 682 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(43905, 47), 678}, // 683 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(43997, 55), 680}, // 684 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(44215, 58), 684}, // 685 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(44105, 56), 682}, // 686 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(44961, 36), 697}, // 687 SubgroupAvcImeGetBorderReachedINTEL
  {IR(44413, 44), 688}, // 688 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(44367, 46), 687}, // 689 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(44828, 66), 695}, // 690 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(44760, 68), 694}, // 691 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(44894, 67), 696}, // 692 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(44623, 68), 692}, // 693 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(44553, 70), 691}, // 694 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(44691, 69), 693}, // 695 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(44997, 48), 698}, // 696 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(45045, 59), 699}, // 697 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(45162, 56), 701}, // 698 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(45104, 58), 700}, // 699 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(43516, 30), 668}, // 700 SubgroupAvcImeInitializeINTEL
  {IR(43620, 33), 671}, // 701 SubgroupAvcImeRefWindowSizeINTEL
  {IR(43584, 36), 670}, // 702 SubgroupAvcImeSetDualReferenceINTEL
  {IR(43817, 54), 676}, // 703 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(43727, 43), 674}, // 704 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(43546, 38), 669}, // 705 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(43770, 47), 675}, // 706 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(43871, 34), 677}, // 707 SubgroupAvcImeSetWeightedSadINTEL
  {IR(44506, 47), 690}, // 708 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(44457, 49), 689}, // 709 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(42906, 39), 653}, // 710 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(42945, 38), 654}, // 711 SubgroupAvcMceConvertToImeResultINTEL
  {IR(42983, 39), 655}, // 712 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(43022, 38), 656}, // 713 SubgroupAvcMceConvertToRefResultINTEL
  {IR(43060, 39), 657}, // 714 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(43099, 38), 658}, // 715 SubgroupAvcMceConvertToSicResultINTEL
  {IR(43212, 43), 661}, // 716 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(42344, 50), 642}, // 717 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(41942, 60), 634}, // 718 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(42142, 51), 638}, // 719 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(42288, 56), 641}, // 720 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(42055, 47), 636}, // 721 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(42643, 56), 648}, // 722 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(42542, 50), 646}, // 723 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(42237, 51), 640}, // 724 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(42446, 49), 644}, // 725 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(42394, 52), 643}, // 726 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(42592, 51), 647}, // 727 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(43331, 38), 664}, // 728 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(43173, 39), 660}, // 729 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(43255, 38), 662}, // 730 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(43293, 38), 663}, // 731 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(43369, 45), 665}, // 732 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(43414, 40), 666}, // 733 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(43454, 62), 667}, // 734 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(43137, 36), 659}, // 735 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(42699, 33), 649}, // 736 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(42845, 61), 652}, // 737 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(42002, 53), 635}, // 738 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(42193, 44), 639}, // 739 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(42102, 40), 637}, // 740 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(42495, 47), 645}, // 741 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(42784, 61), 651}, // 742 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(42732, 52), 650}, // 743 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(45278, 39), 704}, // 744 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(45600, 38), 711}, // 745 SubgroupAvcRefConvertToMceResultINTEL
  {IR(45453, 45), 708}, // 746 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(45498, 46), 709}, // 747 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(45544, 56), 710}, // 748 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(45406, 47), 707}, // 749 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(45317, 46), 705}, // 750 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(45363, 43), 706}, // 751 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(45736, 42), 715}, // 752 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(45700, 36), 714}, // 753 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(45668, 32), 713}, // 754 SubgroupAvcSicConfigureSkcINTEL
  {IR(45817, 39), 717}, // 755 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(46357, 38), 729}, // 756 SubgroupAvcSicConvertToMceResultINTEL
  {IR(46132, 31), 724}, // 757 SubgroupAvcSicEvaluateIpeINTEL
  {IR(46210, 45), 726}, // 758 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(46255, 46), 727}, // 759 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(46301, 56), 728}, // 760 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46163, 47), 725}, // 761 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(46474, 46), 732}, // 762 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(46430, 44), 731}, // 763 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(46695, 35), 737}, // 764 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(46561, 36), 734}, // 765 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(46395, 35), 730}, // 766 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(45778, 39), 716}, // 767 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(46520, 41), 733}, // 768 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(46597, 50), 735}, // 769 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(46647, 48), 736}, // 770 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(45638, 30), 712}, // 771 SubgroupAvcSicInitializeINTEL
  {IR(45998, 43), 721}, // 772 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(46089, 43), 723}, // 773 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(45948, 50), 720}, // 774 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(45900, 48), 719}, // 775 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(45856, 44), 718}, // 776 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(46041, 48), 722}, // 777 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6300, 18), 363}, // 778 SubgroupBallotKHR
  {IR(50003, 27), 835}, // 779 SubgroupBlockPrefetchINTEL
  {IR(40948, 23), 589}, // 780 SubgroupBlockReadINTEL
  {IR(40971, 24), 590}, // 781 SubgroupBlockWriteINTEL
  {IR(34938, 27), 364}, // 782 SubgroupFirstInvocationKHR
  {IR(40995, 28), 591}, // 783 SubgroupImageBlockReadINTEL
  {IR(41023, 29), 592}, // 784 SubgroupImageBlockWriteINTEL
  {IR(41052, 33), 593}, // 785 SubgroupImageMediaBlockReadINTEL
  {IR(41085, 34), 594}, // 786 SubgroupImageMediaBlockWriteINTEL
  {IR(15422, 38), 841}, // 787 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(35121, 26), 374}, // 788 SubgroupReadInvocationKHR
  {IR(40876, 25), 586}, // 789 SubgroupShuffleDownINTEL
  {IR(13078, 21), 585}, // 790 SubgroupShuffleINTEL
  {IR(40901, 23), 587}, // 791 SubgroupShuffleUpINTEL
  {IR(40924, 24), 588}, // 792 SubgroupShuffleXorINTEL
  {IR(32295, 7), 229}, // 793 Switch
  {IR(49817, 24), 831}, // 794 TaskSequenceAsyncALTERA
  {IR(49841, 23), 831}, // 795 TaskSequenceAsyncINTEL
  {IR(49768, 25), 830}, // 796 TaskSequenceCreateALTERA
  {IR(49793, 24), 830}, // 797 TaskSequenceCreateINTEL
  {IR(49864, 22), 832}, // 798 TaskSequenceGetALTERA
  {IR(49886, 21), 832}, // 799 TaskSequenceGetINTEL
  {IR(49907, 26), 833}, // 800 TaskSequenceReleaseALTERA
  {IR(49933, 25), 833}, // 801 TaskSequenceReleaseINTEL
  {IR(40219, 27), 562}, // 802 TensorLayoutSetBlockSizeNV
  {IR(40083, 28), 555}, // 803 TensorLayoutSetClampValueNV
  {IR(40012, 27), 552}, // 804 TensorLayoutSetDimensionNV
  {IR(40039, 24), 553}, // 805 TensorLayoutSetStrideNV
  {IR(40063, 20), 554}, // 806 TensorLayoutSliceNV
  {IR(34713, 19), 350}, // 807 TensorQuerySizeARM
  {IR(34684, 14), 348}, // 808 TensorReadARM
  {IR(40199, 20), 561}, // 809 TensorViewSetClipNV
  {IR(40130, 25), 557}, // 810 TensorViewSetDimensionNV
  {IR(40155, 22), 558}, // 811 TensorViewSetStrideNV
  {IR(34698, 15), 349}, // 812 TensorWriteARM
  {IR(34825, 20), 358}, // 813 TerminateInvocation
  {IR(35286, 16), 381}, // 814 TerminateRayKHR
  {IR(39113, 15), 523}, // 815 TerminateRayNV
  {IR(39136, 14), 525}, // 816 TraceMotionNV
  {IR(39128, 8), 524}, // 817 TraceNV
  {IR(35198, 12), 377}, // 818 TraceRayKHR
  {IR(39150, 17), 526}, // 819 TraceRayMotionNV
  {IR(30147, 10), 76}, // 820 Transpose
  {IR(39217, 29), 528}, // 821 TypeAccelerationStructureKHR
  {IR(39246, 28), 528}, // 822 TypeAccelerationStructureNV
  {IR(29096, 10), 25}, // 823 TypeArray
  {IR(41861, 37), 631}, // 824 TypeAvcImeDualReferenceStreaminINTEL
  {IR(41596, 23), 622}, // 825 TypeAvcImePayloadINTEL
  {IR(41778, 44), 629}, // 826 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(41710, 22), 627}, // 827 TypeAvcImeResultINTEL
  {IR(41732, 46), 628}, // 828 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(41822, 39), 630}, // 829 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(41665, 23), 625}, // 830 TypeAvcMcePayloadINTEL
  {IR(41688, 22), 626}, // 831 TypeAvcMceResultINTEL
  {IR(41619, 23), 623}, // 832 TypeAvcRefPayloadINTEL
  {IR(41898, 22), 632}, // 833 TypeAvcRefResultINTEL
  {IR(41642, 23), 624}, // 834 TypeAvcSicPayloadINTEL
  {IR(41920, 22), 633}, // 835 TypeAvcSicResultINTEL
  {IR(28858, 9), 17}, // 836 TypeBool
  {IR(36700, 14), 436}, // 837 TypeBufferEXT
  {IR(49529, 23), 820}, // 838 TypeBufferSurfaceINTEL
  {IR(35467, 25), 388}, // 839 TypeCooperativeMatrixKHR
  {IR(39539, 24), 538}, // 840 TypeCooperativeMatrixNV
  {IR(37733, 24), 477}, // 841 TypeCooperativeVectorNV
  {IR(29211, 16), 32}, // 842 TypeDeviceEvent
  {IR(29201, 10), 31}, // 843 TypeEvent
  {IR(28912, 10), 19}, // 844 TypeFloat
  {IR(29260, 19), 36}, // 845 TypeForwardPointer
  {IR(29188, 13), 30}, // 846 TypeFunction
  {IR(34812, 13), 357}, // 847 TypeGraphARM
  {IR(38474, 17), 500}, // 848 TypeHitObjectEXT
  {IR(37678, 16), 475}, // 849 TypeHitObjectNV
  {IR(29057, 10), 22}, // 850 TypeImage
  {IR(28867, 8), 18}, // 851 TypeInt
  {IR(28933, 11), 21}, // 852 TypeMatrix
  {IR(33642, 17), 300}, // 853 TypeNamedBarrier
  {IR(36502, 25), 428}, // 854 TypeNodePayloadArrayAMDX
  {IR(29134, 11), 28}, // 855 TypeOpaque
  {IR(29251, 9), 35}, // 856 TypePipe
  {IR(33520, 16), 295}, // 857 TypePipeStorage
  {IR(29176, 12), 29}, // 858 TypePointer
  {IR(29241, 10), 34}, // 859 TypeQueue
  {IR(35746, 16), 396}, // 860 TypeRayQueryKHR
  {IR(29227, 14), 33}, // 861 TypeReserveId
  {IR(29106, 17), 26}, // 862 TypeRuntimeArray
  {IR(29079, 17), 24}, // 863 TypeSampledImage
  {IR(29067, 12), 23}, // 864 TypeSampler
  {IR(29123, 11), 27}, // 865 TypeStruct
  {IR(49552, 25), 821}, // 866 TypeStructContinuedINTEL
  {IR(49958, 23), 834}, // 867 TypeTaskSequenceALTERA
  {IR(49981, 22), 834}, // 868 TypeTaskSequenceINTEL
  {IR(34628, 14), 347}, // 869 TypeTensorARM
  {IR(39955, 19), 549}, // 870 TypeTensorLayoutNV
  {IR(39974, 17), 550}, // 871 TypeTensorViewNV
  {IR(34845, 22), 359}, // 872 TypeUntypedPointerKHR
  {IR(28922, 11), 20}, // 873 TypeVector
  {IR(37717, 16), 477}, // 874 TypeVectorIdEXT
  {IR(41578, 18), 621}, // 875 TypeVmeImageINTEL
  {IR(28849, 9), 16}, // 876 TypeVoid
  {IR(41207, 13), 600}, // 877 UAddSatINTEL
  {IR(41234, 14), 602}, // 878 UAverageINTEL
  {IR(41269, 21), 604}, // 879 UAverageRoundedINTEL
  {IR(30664, 9), 103}, // 880 UConvert
  {IR(41119, 24), 595}, // 881 UCountLeadingZerosINTEL
  {IR(41143, 25), 596}, // 882 UCountTrailingZerosINTEL
  {IR(30876, 5), 123}, // 883 UDiv
  {IR(35362, 5), 383}, // 884 UDot
  {IR(35415, 11), 386}, // 885 UDotAccSat
  {IR(35426, 14), 386}, // 886 UDotAccSatKHR
  {IR(35367, 8), 383}, // 887 UDotKHR
  {IR(31236, 13), 160}, // 888 UGreaterThan
  {IR(31262, 18), 162}, // 889 UGreaterThanEqual
  {IR(31298, 10), 164}, // 890 ULessThan
  {IR(31318, 15), 166}, // 891 ULessThanEqual
  {IR(30891, 5), 126}, // 892 UMod
  {IR(41331, 15), 608}, // 893 UMul32x16INTEL
  {IR(31044, 13), 140}, // 894 UMulExtended
  {IR(41303, 13), 606}, // 895 USubSatINTEL
  {IR(28361, 6), 1}, // 896 Undef
  {IR(31141, 10), 151}, // 897 Unordered
  {IR(32326, 12), 233}, // 898 Unreachable
  {IR(34886, 22), 361}, // 899 UntypedAccessChainKHR
  {IR(35023, 22), 367}, // 900 UntypedArrayLengthKHR
  {IR(35173, 25), 376}, // 901 UntypedGroupAsyncCopyKHR
  {IR(36731, 28), 438}, // 902 UntypedImageTexelPointerEXT
  {IR(34908, 30), 362}, // 903 UntypedInBoundsAccessChainKHR
  {IR(34990, 33), 366}, // 904 UntypedInBoundsPtrAccessChainKHR
  {IR(35045, 19), 368}, // 905 UntypedPrefetchKHR
  {IR(34965, 25), 365}, // 906 UntypedPtrAccessChainKHR
  {IR(34867, 19), 360}, // 907 UntypedVariableKHR
  {IR(15488, 32), 843}, // 908 UntypedVariableLengthArrayINTEL
  {IR(29672, 9), 52}, // 909 Variable
  {IR(13534, 25), 738}, // 910 VariableLengthArrayINTEL
  {IR(30029, 21), 69}, // 911 VectorExtractDynamic
  {IR(30050, 20), 70}, // 912 VectorInsertDynamic
  {IR(30070, 14), 71}, // 913 VectorShuffle
  {IR(30952, 18), 133}, // 914 VectorTimesMatrix
  {IR(30916, 18), 131}, // 915 VectorTimesScalar
  {IR(41564, 14), 620}, // 916 VmeImageINTEL
  {IR(38016, 33), 486}, // 917 WritePackedPrimitiveIndices4x8NV
  {IR(32551, 10), 250}, // 918 WritePipe
  {IR(48817, 24), 800}, // 919 WritePipeBlockingALTERA
  {IR(48841, 23), 800}, // 920 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 494> kExtInstByValue{{
    {0,IR(757, 9),IR(50826, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(757, 9),IR(50834, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(50847, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(50855, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(50869, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(803, 3),IR(50942, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(806, 3),IR(50991, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(809, 2),IR(51046, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(51065, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(51080, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(811, 6),IR(51096, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(51109, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(817, 9),IR(51127, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(826, 9),IR(51179, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(835, 10),IR(51198, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(845, 5),IR(51214, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(51235, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(51256, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(850, 6),IR(51274, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(856, 5),IR(51301, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(861, 5),IR(51336, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(835, 10),IR(51367, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(866, 8),IR(51387, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(874, 11),IR(51412, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(885, 5),IR(51426, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(890, 3),IR(51444, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(893, 2),IR(51475, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(51486, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(895, 3),IR(51499, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(898, 7),IR(51556, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(51575, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(51596, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(51609, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(905, 2),IR(51653, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(739, 1),IR(51668, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(907, 4),IR(51684, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(890, 3),IR(51698, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(51714, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(51720, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(51730, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(51736, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(51741, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(51746, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(51752, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(51758, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(51764, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(51769, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(51775, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(51783, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(51791, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(51795, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(51799, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(51803, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(51808, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(51813, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(51818, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(51823, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(51828, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(51833, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(51839, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(51845, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(51851, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(51857, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(51861, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(51865, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(51869, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(51874, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(51879, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(51884, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(51896, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(51908, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(51922, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(51927, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(51938, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(51943, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(51948, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(51953, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(51958, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(51963, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(51968, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(51975, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(51982, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(51989, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(51994, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(51999, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(52004, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(52015, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(52019, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(52025, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(52037, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(52043, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(52056, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(52069, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(52083, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(52097, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(52110, 15),IR(326, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(52125, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(52141, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(52157, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(52172, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(52187, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(52202, 17),IR(326, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(52219, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(52226, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(52235, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(52241, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(52251, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(52263, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(52271, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(52279, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(52288, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(52297, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(52306, 22),IR(327, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(52328, 20),IR(327, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(52348, 20),IR(327, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(52368, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(52373, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(52378, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5519, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(911, 5),IR(52385, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(916, 5),IR(52398, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(916, 5),IR(52420, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(554, 7),IR(52436, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(554, 7),IR(52461, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(916, 5),IR(52480, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(916, 5),IR(52504, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(916, 5),IR(52525, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(916, 5),IR(52546, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(916, 5),IR(52562, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(52580, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(52606, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(52631, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(52651, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(52676, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(52706, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(52729, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(52754, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(52780, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(52810, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(52836, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(8993, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(52856, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(52886, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(916, 5),IR(52914, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(554, 7),IR(52942, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(52965, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(53000, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(53038, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(53080, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(525, 6),IR(53125, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(525, 6),IR(53162, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(916, 5),IR(53202, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(916, 5),IR(53229, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(53256, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(53288, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(53328, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(53339, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(53365, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(53397, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(53431, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(50855, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(50869, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(50942, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(50991, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(51046, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(51065, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(51080, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(525, 6),IR(51096, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(375, 3),IR(51109, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(921, 9),IR(51127, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(930, 10),IR(51179, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(940, 9),IR(51198, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(51214, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(51235, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(51256, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(525, 6),IR(51274, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(667, 5),IR(51301, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(949, 5),IR(51336, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(954, 10),IR(51367, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(471, 8),IR(51387, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(954, 10),IR(51412, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(916, 5),IR(51426, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(51444, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(893, 2),IR(51475, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(51486, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(964, 3),IR(51499, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(967, 8),IR(51556, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(51575, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(975, 4),IR(51596, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(975, 4),IR(51609, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(51653, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(739, 1),IR(51668, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(577, 4),IR(51684, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(51698, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(506, 7),IR(53453, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(893, 2),IR(53473, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(53485, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(53509, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(667, 5),IR(53530, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(53540, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(53552, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(53573, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(53590, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(53606, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(757, 9),IR(53622, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(53636, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(53649, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(53661, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(53675, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(979, 15),IR(53698, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(994, 33),IR(53718, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1027, 19),IR(53737, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(50855, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1046, 4),IR(50869, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1050, 3),IR(50942, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1053, 3),IR(50991, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1056, 2),IR(51046, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(51065, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(51080, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(811, 6),IR(51096, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1058, 3),IR(51109, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1061, 9),IR(51127, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1070, 10),IR(51179, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1080, 10),IR(51198, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1090, 5),IR(51214, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(51235, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(51256, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(850, 6),IR(51274, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(856, 5),IR(51301, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(861, 5),IR(51336, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1080, 10),IR(51367, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1095, 8),IR(51387, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1103, 11),IR(51412, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(885, 5),IR(51426, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(890, 3),IR(51444, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(893, 2),IR(51475, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(51486, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(895, 3),IR(51499, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1114, 8),IR(51556, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(51575, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(51596, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(975, 4),IR(51609, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1122, 2),IR(51653, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(739, 1),IR(51668, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(907, 4),IR(51684, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(890, 3),IR(51698, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1124, 7),IR(53453, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(893, 2),IR(53473, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1131, 8),IR(54058, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(54075, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(54080, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(54086, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(54093, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(54098, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(54104, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(54111, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(54116, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(54122, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(54128, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(54135, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(54143, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(54148, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(54153, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(54162, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(54166, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(54171, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(54177, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(54182, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(54186, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(54190, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(54195, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(54201, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(54207, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(54212, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(54217, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(54223, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(54227, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(54232, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(54237, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(54242, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(54248, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(54254, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(54260, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(54266, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(54272, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(54279, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(54288, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(54292, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(54297, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(54303, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(54309, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(54314, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(54318, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(54325, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(54332, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(54337, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(54341, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(54351, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(54355, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(54360, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(54365, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(54375, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(54382, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(54387, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(54393, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(54399, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(54405, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(54409, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(54416, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(54421, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(54427, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(54432, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(54436, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(54441, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(54447, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(54454, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(54460, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(54469, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(54481, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(54490, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(54500, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(54511, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(54520, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(54530, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(54541, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(54551, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(54562, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(54573, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(54582, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(54592, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(54601, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(54612, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(54626, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(54637, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(54649, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(54662, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(54673, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(54685, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(54698, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(54710, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(54723, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(54736, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(54747, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(54759, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(54770, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(54777, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(54785, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(54797, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(54809, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(54813, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(54821, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(54826, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(54837, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(54842, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(54848, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(54857, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(54864, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(54874, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(54888, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(54900, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(54915, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(54921, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(54932, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(54942, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(54952, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(54959, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(54966, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(54974, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(54982, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(54990, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(54998, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(55002, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(55006, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(55015, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(55025, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(55035, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(55041, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(55047, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(55053, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(55059, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(55068, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(55075, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(55085, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(55095, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(55106, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(55117, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(55126, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(55134, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(55142, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(55150, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1139, 3),IR(55158, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(55165, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(55173, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1139, 3),IR(55184, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(55196, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1142, 4),IR(55208, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(55222, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1142, 4),IR(55235, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1139, 3),IR(55250, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(55263, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1142, 4),IR(55277, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(55293, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(55301, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(55310, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(55317, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(55326, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(55336, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(55343, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(55349, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(55360, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(55369, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(55378, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(55395, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(55412, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(55420, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(55442, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(55470, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(55489, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(55498, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(55521, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(55530, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(55539, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(55548, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(55557, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(55566, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(55575, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(55584, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(55593, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(55602, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(506, 7),IR(55609, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(747, 10),IR(55620, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(747, 10),IR(55627, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(747, 10),IR(55634, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(55651, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(55657, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(667, 5),IR(55664, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(55675, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(757, 9),IR(55682, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(55699, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(55705, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(55709, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(55717, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(55722, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(55726, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(55749, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(55761, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(55772, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(55784, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(55791, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(55803, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(55822, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(55842, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(55853, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(55865, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(55873, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(55881, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(55885, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(55889, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(55893, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(55899, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(55903, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(55915, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(55920, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(55924, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(55928, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(55932, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(55938, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(55942, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(55954, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(55961, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(55972, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(55978, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(55982, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(55989, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(55995, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(56003, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(56017, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(56028, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(56039, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(56050, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(56061, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(56076, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(56087, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(56094, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(56098, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(56106, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(56114, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(56120, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(56125, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(56135, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(56142, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(667, 5),IR(56150, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(56157, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(747, 10),IR(56162, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    {IR(50826, 8), 0}, // MIN_SAD in arm_motion_engine_100
    {IR(50834, 13), 1}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(50847, 8), 2}, // RAW_SAD in arm_motion_engine_100
    {IR(50869, 21), 4}, // DebugCompilationUnit in debuginfo
    {IR(51596, 13), 31}, // DebugDeclare in debuginfo
    {IR(51668, 16), 34}, // DebugExpression in debuginfo
    {IR(51412, 14), 23}, // DebugFunction in debuginfo
    {IR(51387, 25), 22}, // DebugFunctionDeclaration in debuginfo
    {IR(51367, 20), 21}, // DebugGlobalVariable in debuginfo
    {IR(50855, 14), 3}, // DebugInfoNone in debuginfo
    {IR(51499, 15), 28}, // DebugInlinedAt in debuginfo
    {IR(51575, 21), 30}, // DebugInlinedVariable in debuginfo
    {IR(51426, 18), 24}, // DebugLexicalBlock in debuginfo
    {IR(51444, 31), 25}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(51556, 19), 29}, // DebugLocalVariable in debuginfo
    {IR(51684, 14), 35}, // DebugMacroDef in debuginfo
    {IR(51698, 16), 36}, // DebugMacroUndef in debuginfo
    {IR(51486, 13), 27}, // DebugNoScope in debuginfo
    {IR(51653, 15), 33}, // DebugOperation in debuginfo
    {IR(51475, 11), 26}, // DebugScope in debuginfo
    {IR(51065, 15), 8}, // DebugTypeArray in debuginfo
    {IR(50942, 15), 5}, // DebugTypeBasic in debuginfo
    {IR(51179, 19), 13}, // DebugTypeComposite in debuginfo
    {IR(51127, 14), 12}, // DebugTypeEnum in debuginfo
    {IR(51109, 18), 11}, // DebugTypeFunction in debuginfo
    {IR(51214, 21), 15}, // DebugTypeInheritance in debuginfo
    {IR(51198, 16), 14}, // DebugTypeMember in debuginfo
    {IR(50991, 17), 6}, // DebugTypePointer in debuginfo
    {IR(51235, 21), 16}, // DebugTypePtrToMember in debuginfo
    {IR(51046, 19), 7}, // DebugTypeQualifier in debuginfo
    {IR(51256, 18), 17}, // DebugTypeTemplate in debuginfo
    {IR(51274, 27), 18}, // DebugTypeTemplateParameter in debuginfo
    {IR(51336, 31), 20}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(51301, 35), 19}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(51080, 16), 9}, // DebugTypeVector in debuginfo
    {IR(51096, 13), 10}, // DebugTypedef in debuginfo
    {IR(51609, 11), 32}, // DebugValue in debuginfo
    {IR(51808, 5), 53}, // Acos in glsl_std_450
    {IR(51839, 6), 59}, // Acosh in glsl_std_450
    {IR(51803, 5), 52}, // Asin in glsl_std_450
    {IR(51833, 6), 58}, // Asinh in glsl_std_450
    {IR(51813, 5), 54}, // Atan in glsl_std_450
    {IR(51851, 6), 61}, // Atan2 in glsl_std_450
    {IR(51845, 6), 60}, // Atanh in glsl_std_450
    {IR(51764, 5), 45}, // Ceil in glsl_std_450
    {IR(51795, 4), 50}, // Cos in glsl_std_450
    {IR(51823, 5), 56}, // Cosh in glsl_std_450
    {IR(52235, 6), 104}, // Cross in glsl_std_450
    {IR(51783, 8), 48}, // Degrees in glsl_std_450
    {IR(51896, 12), 69}, // Determinant in glsl_std_450
    {IR(52226, 9), 103}, // Distance in glsl_std_450
    {IR(51861, 4), 63}, // Exp in glsl_std_450
    {IR(51869, 5), 65}, // Exp2 in glsl_std_450
    {IR(51736, 5), 40}, // FAbs in glsl_std_450
    {IR(51968, 7), 79}, // FClamp in glsl_std_450
    {IR(51953, 5), 76}, // FMax in glsl_std_450
    {IR(51938, 5), 73}, // FMin in glsl_std_450
    {IR(51989, 5), 82}, // FMix in glsl_std_450
    {IR(51746, 6), 42}, // FSign in glsl_std_450
    {IR(52251, 12), 106}, // FaceForward in glsl_std_450
    {IR(52279, 9), 109}, // FindILsb in glsl_std_450
    {IR(52288, 9), 110}, // FindSMsb in glsl_std_450
    {IR(52297, 9), 111}, // FindUMsb in glsl_std_450
    {IR(51758, 6), 44}, // Floor in glsl_std_450
    {IR(52015, 4), 86}, // Fma in glsl_std_450
    {IR(51769, 6), 46}, // Fract in glsl_std_450
    {IR(52019, 6), 87}, // Frexp in glsl_std_450
    {IR(52025, 12), 88}, // FrexpStruct in glsl_std_450
    {IR(51994, 5), 83}, // IMix in glsl_std_450
    {IR(52306, 22), 112}, // InterpolateAtCentroid in glsl_std_450
    {IR(52348, 20), 114}, // InterpolateAtOffset in glsl_std_450
    {IR(52328, 20), 113}, // InterpolateAtSample in glsl_std_450
    {IR(51884, 12), 68}, // InverseSqrt in glsl_std_450
    {IR(52037, 6), 89}, // Ldexp in glsl_std_450
    {IR(52219, 7), 102}, // Length in glsl_std_450
    {IR(51865, 4), 64}, // Log in glsl_std_450
    {IR(51874, 5), 66}, // Log2 in glsl_std_450
    {IR(51908, 14), 70}, // MatrixInverse in glsl_std_450
    {IR(51922, 5), 71}, // Modf in glsl_std_450
    {IR(51927, 11), 72}, // ModfStruct in glsl_std_450
    {IR(52378, 7), 117}, // NClamp in glsl_std_450
    {IR(52373, 5), 116}, // NMax in glsl_std_450
    {IR(52368, 5), 115}, // NMin in glsl_std_450
    {IR(52241, 10), 105}, // Normalize in glsl_std_450
    {IR(52110, 15), 95}, // PackDouble2x32 in glsl_std_450
    {IR(52097, 13), 94}, // PackHalf2x16 in glsl_std_450
    {IR(52069, 14), 92}, // PackSnorm2x16 in glsl_std_450
    {IR(52043, 13), 90}, // PackSnorm4x8 in glsl_std_450
    {IR(52083, 14), 93}, // PackUnorm2x16 in glsl_std_450
    {IR(52056, 13), 91}, // PackUnorm4x8 in glsl_std_450
    {IR(51857, 4), 62}, // Pow in glsl_std_450
    {IR(51775, 8), 47}, // Radians in glsl_std_450
    {IR(52263, 8), 107}, // Reflect in glsl_std_450
    {IR(52271, 8), 108}, // Refract in glsl_std_450
    {IR(51714, 6), 37}, // Round in glsl_std_450
    {IR(51720, 10), 38}, // RoundEven in glsl_std_450
    {IR(51741, 5), 41}, // SAbs in glsl_std_450
    {IR(51982, 7), 81}, // SClamp in glsl_std_450
    {IR(51963, 5), 78}, // SMax in glsl_std_450
    {IR(51948, 5), 75}, // SMin in glsl_std_450
    {IR(51752, 6), 43}, // SSign in glsl_std_450
    {IR(51791, 4), 49}, // Sin in glsl_std_450
    {IR(51818, 5), 55}, // Sinh in glsl_std_450
    {IR(52004, 11), 85}, // SmoothStep in glsl_std_450
    {IR(51879, 5), 67}, // Sqrt in glsl_std_450
    {IR(51999, 5), 84}, // Step in glsl_std_450
    {IR(51799, 4), 51}, // Tan in glsl_std_450
    {IR(51828, 5), 57}, // Tanh in glsl_std_450
    {IR(51730, 6), 39}, // Trunc in glsl_std_450
    {IR(51975, 7), 80}, // UClamp in glsl_std_450
    {IR(51958, 5), 77}, // UMax in glsl_std_450
    {IR(51943, 5), 74}, // UMin in glsl_std_450
    {IR(52202, 17), 101}, // UnpackDouble2x32 in glsl_std_450
    {IR(52157, 15), 98}, // UnpackHalf2x16 in glsl_std_450
    {IR(52125, 16), 96}, // UnpackSnorm2x16 in glsl_std_450
    {IR(52172, 15), 99}, // UnpackSnorm4x8 in glsl_std_450
    {IR(52141, 16), 97}, // UnpackUnorm2x16 in glsl_std_450
    {IR(52187, 15), 100}, // UnpackUnorm4x8 in glsl_std_450
    {IR(52385, 13), 119}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(52480, 24), 124}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(52436, 25), 122}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(52461, 19), 123}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(52914, 28), 143}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(52942, 23), 144}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(52504, 21), 125}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(52546, 16), 127}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(52398, 22), 120}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(52525, 21), 126}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(53202, 27), 151}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(52420, 16), 121}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(53229, 27), 152}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(52562, 18), 128}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(53256, 32), 153}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(52810, 26), 138}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(52836, 20), 139}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(53080, 45), 148}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(53162, 40), 150}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(53038, 42), 147}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(53125, 37), 149}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5519, 7), 118}, // Kernel in nonsemantic_clspvreflection
    {IR(8993, 15), 140}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(53397, 34), 158}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(53365, 32), 157}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(53339, 26), 156}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(53328, 11), 155}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(53288, 40), 154}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(53000, 38), 146}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(52965, 35), 145}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(52856, 30), 141}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(52676, 30), 133}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(52651, 25), 132}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(52706, 23), 134}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(52754, 26), 136}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(52780, 30), 137}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(52729, 25), 135}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(52606, 25), 130}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(52886, 28), 142}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(52631, 20), 131}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(52580, 26), 129}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(53431, 22), 159}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(53552, 21), 200}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(50869, 21), 161}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(51596, 13), 188}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(53590, 16), 202}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(51668, 16), 191}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(51412, 14), 180}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(51387, 25), 179}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(53485, 24), 196}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(51367, 20), 178}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(53453, 20), 194}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(50855, 14), 160}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(51499, 15), 185}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(51575, 21), 187}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(51426, 18), 181}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(51444, 31), 182}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(53530, 10), 198}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(51556, 19), 186}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(51684, 14), 192}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(51698, 16), 193}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(53540, 12), 199}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(51486, 13), 184}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(51653, 15), 190}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(51475, 11), 183}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(53473, 12), 195}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(53509, 21), 197}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(53573, 17), 201}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(51065, 15), 165}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(50942, 15), 162}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(51179, 19), 170}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(51127, 14), 169}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(51109, 18), 168}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(51214, 21), 172}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(53606, 16), 203}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(51198, 16), 171}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(50991, 17), 163}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(51235, 21), 173}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(51046, 19), 164}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(51256, 18), 174}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(51274, 27), 175}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(51336, 31), 177}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(51301, 35), 176}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(51080, 16), 166}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(51096, 13), 167}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(51609, 11), 189}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(53622, 14), 204}, // Configuration in nonsemantic_vkspreflection
    {IR(53698, 20), 209}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(53718, 19), 210}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(53737, 21), 211}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(53661, 14), 207}, // PushConstants in nonsemantic_vkspreflection
    {IR(53675, 23), 208}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(53636, 13), 205}, // StartCounter in nonsemantic_vkspreflection
    {IR(53649, 12), 206}, // StopCounter in nonsemantic_vkspreflection
    {IR(50869, 21), 213}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(51596, 13), 240}, // DebugDeclare in opencl_debuginfo_100
    {IR(51668, 16), 243}, // DebugExpression in opencl_debuginfo_100
    {IR(51412, 14), 232}, // DebugFunction in opencl_debuginfo_100
    {IR(51387, 25), 231}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(51367, 20), 230}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(53453, 20), 246}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(50855, 14), 212}, // DebugInfoNone in opencl_debuginfo_100
    {IR(51499, 15), 237}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(51575, 21), 239}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(51426, 18), 233}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(51444, 31), 234}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(51556, 19), 238}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(51684, 14), 244}, // DebugMacroDef in opencl_debuginfo_100
    {IR(51698, 16), 245}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(54058, 17), 248}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(51486, 13), 236}, // DebugNoScope in opencl_debuginfo_100
    {IR(51653, 15), 242}, // DebugOperation in opencl_debuginfo_100
    {IR(51475, 11), 235}, // DebugScope in opencl_debuginfo_100
    {IR(53473, 12), 247}, // DebugSource in opencl_debuginfo_100
    {IR(51065, 15), 217}, // DebugTypeArray in opencl_debuginfo_100
    {IR(50942, 15), 214}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(51179, 19), 222}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(51127, 14), 221}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(51109, 18), 220}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(51214, 21), 224}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(51198, 16), 223}, // DebugTypeMember in opencl_debuginfo_100
    {IR(50991, 17), 215}, // DebugTypePointer in opencl_debuginfo_100
    {IR(51235, 21), 225}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(51046, 19), 216}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(51256, 18), 226}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(51274, 27), 227}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(51336, 31), 229}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(51301, 35), 228}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(51080, 16), 218}, // DebugTypeVector in opencl_debuginfo_100
    {IR(51096, 13), 219}, // DebugTypedef in opencl_debuginfo_100
    {IR(51609, 11), 241}, // DebugValue in opencl_debuginfo_100
    {IR(54075, 5), 249}, // acos in opencl_std_100
    {IR(54080, 6), 250}, // acosh in opencl_std_100
    {IR(54086, 7), 251}, // acospi in opencl_std_100
    {IR(54093, 5), 252}, // asin in opencl_std_100
    {IR(54098, 6), 253}, // asinh in opencl_std_100
    {IR(54104, 7), 254}, // asinpi in opencl_std_100
    {IR(54111, 5), 255}, // atan in opencl_std_100
    {IR(54116, 6), 256}, // atan2 in opencl_std_100
    {IR(54135, 8), 259}, // atan2pi in opencl_std_100
    {IR(54122, 6), 257}, // atanh in opencl_std_100
    {IR(54128, 7), 258}, // atanpi in opencl_std_100
    {IR(55326, 10), 405}, // bitselect in opencl_std_100
    {IR(54143, 5), 260}, // cbrt in opencl_std_100
    {IR(54148, 5), 261}, // ceil in opencl_std_100
    {IR(54998, 4), 370}, // clz in opencl_std_100
    {IR(54153, 9), 262}, // copysign in opencl_std_100
    {IR(54162, 4), 263}, // cos in opencl_std_100
    {IR(54166, 5), 264}, // cosh in opencl_std_100
    {IR(54171, 6), 265}, // cospi in opencl_std_100
    {IR(54842, 6), 353}, // cross in opencl_std_100
    {IR(55002, 4), 371}, // ctz in opencl_std_100
    {IR(54777, 8), 345}, // degrees in opencl_std_100
    {IR(54848, 9), 354}, // distance in opencl_std_100
    {IR(54182, 4), 267}, // erf in opencl_std_100
    {IR(54177, 5), 266}, // erfc in opencl_std_100
    {IR(54186, 4), 268}, // exp in opencl_std_100
    {IR(54195, 6), 270}, // exp10 in opencl_std_100
    {IR(54190, 5), 269}, // exp2 in opencl_std_100
    {IR(54201, 6), 271}, // expm1 in opencl_std_100
    {IR(54207, 5), 272}, // fabs in opencl_std_100
    {IR(54874, 14), 357}, // fast_distance in opencl_std_100
    {IR(54888, 12), 358}, // fast_length in opencl_std_100
    {IR(54900, 15), 359}, // fast_normalize in opencl_std_100
    {IR(54770, 7), 344}, // fclamp in opencl_std_100
    {IR(54212, 5), 273}, // fdim in opencl_std_100
    {IR(54217, 6), 274}, // floor in opencl_std_100
    {IR(54223, 4), 275}, // fma in opencl_std_100
    {IR(54227, 5), 276}, // fmax in opencl_std_100
    {IR(54785, 12), 346}, // fmax_common in opencl_std_100
    {IR(54232, 5), 277}, // fmin in opencl_std_100
    {IR(54797, 12), 347}, // fmin_common in opencl_std_100
    {IR(54237, 5), 278}, // fmod in opencl_std_100
    {IR(54242, 6), 279}, // fract in opencl_std_100
    {IR(54248, 6), 280}, // frexp in opencl_std_100
    {IR(54460, 9), 316}, // half_cos in opencl_std_100
    {IR(54469, 12), 317}, // half_divide in opencl_std_100
    {IR(54481, 9), 318}, // half_exp in opencl_std_100
    {IR(54500, 11), 320}, // half_exp10 in opencl_std_100
    {IR(54490, 10), 319}, // half_exp2 in opencl_std_100
    {IR(54511, 9), 321}, // half_log in opencl_std_100
    {IR(54530, 11), 323}, // half_log10 in opencl_std_100
    {IR(54520, 10), 322}, // half_log2 in opencl_std_100
    {IR(54541, 10), 324}, // half_powr in opencl_std_100
    {IR(54551, 11), 325}, // half_recip in opencl_std_100
    {IR(54562, 11), 326}, // half_rsqrt in opencl_std_100
    {IR(54573, 9), 327}, // half_sin in opencl_std_100
    {IR(54582, 10), 328}, // half_sqrt in opencl_std_100
    {IR(54592, 9), 329}, // half_tan in opencl_std_100
    {IR(54254, 6), 281}, // hypot in opencl_std_100
    {IR(54260, 6), 282}, // ilogb in opencl_std_100
    {IR(54266, 6), 283}, // ldexp in opencl_std_100
    {IR(54857, 7), 355}, // length in opencl_std_100
    {IR(54272, 7), 284}, // lgamma in opencl_std_100
    {IR(54279, 9), 285}, // lgamma_r in opencl_std_100
    {IR(54288, 4), 286}, // log in opencl_std_100
    {IR(54297, 6), 288}, // log10 in opencl_std_100
    {IR(54303, 6), 289}, // log1p in opencl_std_100
    {IR(54292, 5), 287}, // log2 in opencl_std_100
    {IR(54309, 5), 290}, // logb in opencl_std_100
    {IR(54314, 4), 291}, // mad in opencl_std_100
    {IR(54318, 7), 292}, // maxmag in opencl_std_100
    {IR(54325, 7), 293}, // minmag in opencl_std_100
    {IR(54809, 4), 348}, // mix in opencl_std_100
    {IR(54332, 5), 294}, // modf in opencl_std_100
    {IR(54337, 4), 295}, // nan in opencl_std_100
    {IR(54601, 11), 330}, // native_cos in opencl_std_100
    {IR(54612, 14), 331}, // native_divide in opencl_std_100
    {IR(54626, 11), 332}, // native_exp in opencl_std_100
    {IR(54649, 13), 334}, // native_exp10 in opencl_std_100
    {IR(54637, 12), 333}, // native_exp2 in opencl_std_100
    {IR(54662, 11), 335}, // native_log in opencl_std_100
    {IR(54685, 13), 337}, // native_log10 in opencl_std_100
    {IR(54673, 12), 336}, // native_log2 in opencl_std_100
    {IR(54698, 12), 338}, // native_powr in opencl_std_100
    {IR(54710, 13), 339}, // native_recip in opencl_std_100
    {IR(54723, 13), 340}, // native_rsqrt in opencl_std_100
    {IR(54736, 11), 341}, // native_sin in opencl_std_100
    {IR(54747, 12), 342}, // native_sqrt in opencl_std_100
    {IR(54759, 11), 343}, // native_tan in opencl_std_100
    {IR(54341, 10), 296}, // nextafter in opencl_std_100
    {IR(54864, 10), 356}, // normalize in opencl_std_100
    {IR(55117, 9), 385}, // popcount in opencl_std_100
    {IR(54351, 4), 297}, // pow in opencl_std_100
    {IR(54355, 5), 298}, // pown in opencl_std_100
    {IR(54360, 5), 299}, // powr in opencl_std_100
    {IR(55317, 9), 404}, // prefetch in opencl_std_100
    {IR(55310, 7), 403}, // printf in opencl_std_100
    {IR(54813, 8), 349}, // radians in opencl_std_100
    {IR(54365, 10), 300}, // remainder in opencl_std_100
    {IR(54375, 7), 301}, // remquo in opencl_std_100
    {IR(54382, 5), 302}, // rint in opencl_std_100
    {IR(54387, 6), 303}, // rootn in opencl_std_100
    {IR(55068, 7), 380}, // rotate in opencl_std_100
    {IR(54393, 6), 304}, // round in opencl_std_100
    {IR(54399, 6), 305}, // rsqrt in opencl_std_100
    {IR(54915, 6), 360}, // s_abs in opencl_std_100
    {IR(54921, 11), 361}, // s_abs_diff in opencl_std_100
    {IR(54932, 10), 362}, // s_add_sat in opencl_std_100
    {IR(54982, 8), 368}, // s_clamp in opencl_std_100
    {IR(54952, 7), 364}, // s_hadd in opencl_std_100
    {IR(55126, 8), 386}, // s_mad24 in opencl_std_100
    {IR(55006, 9), 372}, // s_mad_hi in opencl_std_100
    {IR(55025, 10), 374}, // s_mad_sat in opencl_std_100
    {IR(55035, 6), 375}, // s_max in opencl_std_100
    {IR(55047, 6), 377}, // s_min in opencl_std_100
    {IR(55142, 8), 388}, // s_mul24 in opencl_std_100
    {IR(55059, 9), 379}, // s_mul_hi in opencl_std_100
    {IR(54966, 8), 366}, // s_rhadd in opencl_std_100
    {IR(55075, 10), 381}, // s_sub_sat in opencl_std_100
    {IR(55106, 11), 384}, // s_upsample in opencl_std_100
    {IR(55336, 7), 406}, // select in opencl_std_100
    {IR(55293, 8), 401}, // shuffle in opencl_std_100
    {IR(55301, 9), 402}, // shuffle2 in opencl_std_100
    {IR(54837, 5), 352}, // sign in opencl_std_100
    {IR(54405, 4), 306}, // sin in opencl_std_100
    {IR(54409, 7), 307}, // sincos in opencl_std_100
    {IR(54416, 5), 308}, // sinh in opencl_std_100
    {IR(54421, 6), 309}, // sinpi in opencl_std_100
    {IR(54826, 11), 351}, // smoothstep in opencl_std_100
    {IR(54427, 5), 310}, // sqrt in opencl_std_100
    {IR(54821, 5), 350}, // step in opencl_std_100
    {IR(54432, 4), 311}, // tan in opencl_std_100
    {IR(54436, 5), 312}, // tanh in opencl_std_100
    {IR(54441, 6), 313}, // tanpi in opencl_std_100
    {IR(54447, 7), 314}, // tgamma in opencl_std_100
    {IR(54454, 6), 315}, // trunc in opencl_std_100
    {IR(55343, 6), 407}, // u_abs in opencl_std_100
    {IR(55349, 11), 408}, // u_abs_diff in opencl_std_100
    {IR(54942, 10), 363}, // u_add_sat in opencl_std_100
    {IR(54990, 8), 369}, // u_clamp in opencl_std_100
    {IR(54959, 7), 365}, // u_hadd in opencl_std_100
    {IR(55134, 8), 387}, // u_mad24 in opencl_std_100
    {IR(55369, 9), 410}, // u_mad_hi in opencl_std_100
    {IR(55015, 10), 373}, // u_mad_sat in opencl_std_100
    {IR(55041, 6), 376}, // u_max in opencl_std_100
    {IR(55053, 6), 378}, // u_min in opencl_std_100
    {IR(55150, 8), 389}, // u_mul24 in opencl_std_100
    {IR(55360, 9), 409}, // u_mul_hi in opencl_std_100
    {IR(54974, 8), 367}, // u_rhadd in opencl_std_100
    {IR(55085, 10), 382}, // u_sub_sat in opencl_std_100
    {IR(55095, 11), 383}, // u_upsample in opencl_std_100
    {IR(55173, 11), 392}, // vload_half in opencl_std_100
    {IR(55184, 12), 393}, // vload_halfn in opencl_std_100
    {IR(55250, 13), 398}, // vloada_halfn in opencl_std_100
    {IR(55158, 7), 390}, // vloadn in opencl_std_100
    {IR(55196, 12), 394}, // vstore_half in opencl_std_100
    {IR(55208, 14), 395}, // vstore_half_r in opencl_std_100
    {IR(55222, 13), 396}, // vstore_halfn in opencl_std_100
    {IR(55235, 15), 397}, // vstore_halfn_r in opencl_std_100
    {IR(55263, 14), 399}, // vstorea_halfn in opencl_std_100
    {IR(55277, 16), 400}, // vstorea_halfn_r in opencl_std_100
    {IR(55165, 8), 391}, // vstoren in opencl_std_100
    {IR(55395, 17), 412}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(55378, 17), 411}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(55412, 8), 413}, // TimeAMD in spv_amd_gcn_shader
    {IR(55489, 9), 417}, // MbcntAMD in spv_amd_shader_ballot
    {IR(55420, 22), 414}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(55442, 28), 415}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(55470, 19), 416}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(55498, 23), 418}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(55548, 9), 422}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55575, 9), 425}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55521, 9), 419}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(55566, 9), 424}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55593, 9), 427}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55539, 9), 421}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(55557, 9), 423}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55584, 9), 426}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55530, 9), 420}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(55899, 4), 459}, // ABS in tosa_001000_1
    {IR(55722, 4), 442}, // ADD in tosa_001000_1
    {IR(55602, 7), 428}, // ARGMAX in tosa_001000_1
    {IR(55726, 23), 443}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(55609, 11), 429}, // AVG_POOL2D in tosa_001000_1
    {IR(55749, 12), 444}, // BITWISE_AND in tosa_001000_1
    {IR(55903, 12), 460}, // BITWISE_NOT in tosa_001000_1
    {IR(55761, 11), 445}, // BITWISE_OR in tosa_001000_1
    {IR(55772, 12), 446}, // BITWISE_XOR in tosa_001000_1
    {IR(56157, 5), 492}, // CAST in tosa_001000_1
    {IR(55915, 5), 461}, // CEIL in tosa_001000_1
    {IR(55699, 6), 438}, // CLAMP in tosa_001000_1
    {IR(55920, 4), 462}, // CLZ in tosa_001000_1
    {IR(56087, 7), 482}, // CONCAT in tosa_001000_1
    {IR(55620, 7), 430}, // CONV2D in tosa_001000_1
    {IR(55627, 7), 431}, // CONV3D in tosa_001000_1
    {IR(55924, 4), 463}, // COS in tosa_001000_1
    {IR(55634, 17), 432}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(55989, 6), 473}, // EQUAL in tosa_001000_1
    {IR(55705, 4), 439}, // ERF in tosa_001000_1
    {IR(55928, 4), 464}, // EXP in tosa_001000_1
    {IR(55651, 6), 433}, // FFT2D in tosa_001000_1
    {IR(55932, 6), 465}, // FLOOR in tosa_001000_1
    {IR(56135, 7), 489}, // GATHER in tosa_001000_1
    {IR(55995, 8), 474}, // GREATER in tosa_001000_1
    {IR(56003, 14), 475}, // GREATER_EQUAL in tosa_001000_1
    {IR(55784, 7), 447}, // INTDIV in tosa_001000_1
    {IR(55938, 4), 466}, // LOG in tosa_001000_1
    {IR(55791, 12), 448}, // LOGICAL_AND in tosa_001000_1
    {IR(55803, 19), 449}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(55942, 12), 467}, // LOGICAL_NOT in tosa_001000_1
    {IR(55842, 11), 451}, // LOGICAL_OR in tosa_001000_1
    {IR(55822, 20), 450}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(55853, 12), 452}, // LOGICAL_XOR in tosa_001000_1
    {IR(55657, 7), 434}, // MATMUL in tosa_001000_1
    {IR(55865, 8), 453}, // MAXIMUM in tosa_001000_1
    {IR(55664, 11), 435}, // MAX_POOL2D in tosa_001000_1
    {IR(55873, 8), 454}, // MINIMUM in tosa_001000_1
    {IR(55881, 4), 455}, // MUL in tosa_001000_1
    {IR(55954, 7), 468}, // NEGATE in tosa_001000_1
    {IR(56094, 4), 483}, // PAD in tosa_001000_1
    {IR(55885, 4), 456}, // POW in tosa_001000_1
    {IR(55961, 11), 469}, // RECIPROCAL in tosa_001000_1
    {IR(56017, 11), 476}, // REDUCE_ALL in tosa_001000_1
    {IR(56028, 11), 477}, // REDUCE_ANY in tosa_001000_1
    {IR(56039, 11), 478}, // REDUCE_MAX in tosa_001000_1
    {IR(56050, 11), 479}, // REDUCE_MIN in tosa_001000_1
    {IR(56061, 15), 480}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(56076, 11), 481}, // REDUCE_SUM in tosa_001000_1
    {IR(56162, 8), 493}, // RESCALE in tosa_001000_1
    {IR(56098, 8), 484}, // RESHAPE in tosa_001000_1
    {IR(56150, 7), 491}, // RESIZE in tosa_001000_1
    {IR(56106, 8), 485}, // REVERSE in tosa_001000_1
    {IR(55675, 7), 436}, // RFFT2D in tosa_001000_1
    {IR(55972, 6), 470}, // RSQRT in tosa_001000_1
    {IR(56142, 8), 490}, // SCATTER in tosa_001000_1
    {IR(55982, 7), 472}, // SELECT in tosa_001000_1
    {IR(55709, 8), 440}, // SIGMOID in tosa_001000_1
    {IR(55978, 4), 471}, // SIN in tosa_001000_1
    {IR(56114, 6), 486}, // SLICE in tosa_001000_1
    {IR(55889, 4), 457}, // SUB in tosa_001000_1
    {IR(55893, 6), 458}, // TABLE in tosa_001000_1
    {IR(55717, 5), 441}, // TANH in tosa_001000_1
    {IR(56120, 5), 487}, // TILE in tosa_001000_1
    {IR(56125, 10), 488}, // TRANSPOSE in tosa_001000_1
    {IR(55682, 17), 437}, // TRANSPOSE_CONV2D in tosa_001000_1
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
  "SPV_ALTERA_arbitrary_precision_fixed_point\0" // IR(0, 43)
  "SPV_ALTERA_arbitrary_precision_floating_point\0" // IR(43, 46)
  "SPV_ALTERA_arbitrary_precision_integers\0" // IR(89, 40)
  "SPV_ALTERA_blocking_pipes\0" // IR(129, 26)
  "SPV_ALTERA_fpga_argument_interfaces\0" // IR(155, 36)
  "SPV_ALTERA_fpga_buffer_location\0" // IR(191, 32)
  "SPV_ALTERA_fpga_cluster_attributes\0" // IR(223, 35)
  "SPV_ALTERA_fpga_dsp_control\0" // IR(258, 28)
  "SPV_ALTERA_fpga_invocation_pipelining_attributes\0" // IR(286, 49)
  "SPV_ALTERA_fpga_latency_control\0" // IR(335, 32)
  "SPV_ALTERA_fpga_loop_controls\0" // IR(367, 30)
  "SPV_ALTERA_fpga_memory_accesses\0" // IR(397, 32)
  "SPV_ALTERA_fpga_memory_attributes\0" // IR(429, 34)
  "SPV_ALTERA_fpga_reg\0" // IR(463, 20)
  "SPV_ALTERA_global_variable_fpga_decorations\0" // IR(483, 44)
  "SPV_ALTERA_io_pipes\0" // IR(527, 20)
  "SPV_ALTERA_loop_fuse\0" // IR(547, 21)
  "SPV_ALTERA_runtime_aligned\0" // IR(568, 27)
  "SPV_ALTERA_task_sequence\0" // IR(595, 25)
  "SPV_ALTERA_usm_storage_classes\0" // IR(620, 31)
  "SPV_AMDX_shader_enqueue\0" // IR(651, 24)
  "SPV_AMD_gcn_shader\0" // IR(675, 19)
  "SPV_AMD_gpu_shader_half_float\0" // IR(694, 30)
  "SPV_AMD_gpu_shader_half_float_fetch\0" // IR(724, 36)
  "SPV_AMD_gpu_shader_int16\0" // IR(760, 25)
  "SPV_AMD_shader_ballot\0" // IR(785, 22)
  "SPV_AMD_shader_early_and_late_fragment_tests\0" // IR(807, 45)
  "SPV_AMD_shader_explicit_vertex_parameter\0" // IR(852, 41)
  "SPV_AMD_shader_fragment_mask\0" // IR(893, 29)
  "SPV_AMD_shader_image_load_store_lod\0" // IR(922, 36)
  "SPV_AMD_shader_trinary_minmax\0" // IR(958, 30)
  "SPV_AMD_texture_gather_bias_lod\0" // IR(988, 32)
  "SPV_ARM_cooperative_matrix_layouts\0" // IR(1020, 35)
  "SPV_ARM_core_builtins\0" // IR(1055, 22)
  "SPV_ARM_graph\0" // IR(1077, 14)
  "SPV_ARM_tensors\0" // IR(1091, 16)
  "SPV_EXT_arithmetic_fence\0" // IR(1107, 25)
  "SPV_EXT_demote_to_helper_invocation\0" // IR(1132, 36)
  "SPV_EXT_descriptor_heap\0" // IR(1168, 24)
  "SPV_EXT_descriptor_indexing\0" // IR(1192, 28)
  "SPV_EXT_float8\0" // IR(1220, 15)
  "SPV_EXT_fragment_fully_covered\0" // IR(1235, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(1266, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(1302, 34)
  "SPV_EXT_long_vector\0" // IR(1336, 20)
  "SPV_EXT_mesh_shader\0" // IR(1356, 20)
  "SPV_EXT_opacity_micromap\0" // IR(1376, 25)
  "SPV_EXT_optnone\0" // IR(1401, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(1417, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(1449, 44)
  "SPV_EXT_replicated_composites\0" // IR(1493, 30)
  "SPV_EXT_shader_64bit_indexing\0" // IR(1523, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(1553, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(1587, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(1619, 36)
  "SPV_EXT_shader_image_int64\0" // IR(1655, 27)
  "SPV_EXT_shader_invocation_reorder\0" // IR(1682, 34)
  "SPV_EXT_shader_stencil_export\0" // IR(1716, 30)
  "SPV_EXT_shader_subgroup_partitioned\0" // IR(1746, 36)
  "SPV_EXT_shader_tile_image\0" // IR(1782, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1808, 36)
  "SPV_GOOGLE_decorate_string\0" // IR(1844, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1871, 31)
  "SPV_GOOGLE_user_type\0" // IR(1902, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1923, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(1945, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(1987, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(2032, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(2071, 30)
  "SPV_INTEL_bindless_images\0" // IR(2101, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(2127, 25)
  "SPV_INTEL_cache_controls\0" // IR(2152, 25)
  "SPV_INTEL_debug_module\0" // IR(2177, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(2200, 44)
  "SPV_INTEL_float_controls2\0" // IR(2244, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(2270, 28)
  "SPV_INTEL_fp_max_error\0" // IR(2298, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(2321, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(2356, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(2387, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(2421, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(2448, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(2496, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(2527, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(2556, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(2587, 33)
  "SPV_INTEL_fpga_reg\0" // IR(2620, 19)
  "SPV_INTEL_function_pointers\0" // IR(2639, 28)
  "SPV_INTEL_function_variants\0" // IR(2667, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(2695, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(2738, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2776, 26)
  "SPV_INTEL_int4\0" // IR(2802, 15)
  "SPV_INTEL_io_pipes\0" // IR(2817, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2836, 28)
  "SPV_INTEL_long_composites\0" // IR(2864, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2890, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2910, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2942, 28)
  "SPV_INTEL_media_block_io\0" // IR(2970, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2995, 33)
  "SPV_INTEL_optnone\0" // IR(3028, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(3046, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(3072, 36)
  "SPV_INTEL_split_barrier\0" // IR(3108, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3132, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3167, 46)
  "SPV_INTEL_subgroups\0" // IR(3213, 20)
  "SPV_INTEL_task_sequence\0" // IR(3233, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3257, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3293, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3328, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3365, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3395, 32)
  "SPV_INTEL_vector_compute\0" // IR(3427, 25)
  "SPV_KHR_16bit_storage\0" // IR(3452, 22)
  "SPV_KHR_8bit_storage\0" // IR(3474, 21)
  "SPV_KHR_bfloat16\0" // IR(3495, 17)
  "SPV_KHR_bit_instructions\0" // IR(3512, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3537, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(3572, 27)
  "SPV_KHR_device_group\0" // IR(3599, 21)
  "SPV_KHR_expect_assume\0" // IR(3620, 22)
  "SPV_KHR_float_controls\0" // IR(3642, 23)
  "SPV_KHR_float_controls2\0" // IR(3665, 24)
  "SPV_KHR_fma\0" // IR(3689, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3701, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3737, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3767, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3795, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3816, 30)
  "SPV_KHR_multiview\0" // IR(3846, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3864, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3899, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3925, 32)
  "SPV_KHR_poison_freeze\0" // IR(3957, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(3979, 28)
  "SPV_KHR_quad_control\0" // IR(4007, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4028, 22)
  "SPV_KHR_ray_query\0" // IR(4050, 18)
  "SPV_KHR_ray_tracing\0" // IR(4068, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4088, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4123, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4160, 34)
  "SPV_KHR_shader_ballot\0" // IR(4194, 22)
  "SPV_KHR_shader_clock\0" // IR(4216, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4237, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4268, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4305, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4329, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4367, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4389, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4418, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4453, 25)
  "SPV_KHR_variable_pointers\0" // IR(4478, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4504, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4532, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4573, 38)
  "SPV_NV_bindless_texture\0" // IR(4611, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4635, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4673, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4707, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4733, 27)
  "SPV_NV_cooperative_vector\0" // IR(4760, 26)
  "SPV_NV_displacement_micromap\0" // IR(4786, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(4815, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(4850, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(4885, 28)
  "SPV_NV_mesh_shader\0" // IR(4913, 19)
  "SPV_NV_push_constant_bank\0" // IR(4932, 26)
  "SPV_NV_raw_access_chains\0" // IR(4958, 25)
  "SPV_NV_ray_tracing\0" // IR(4983, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5002, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5033, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5070, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5103, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5133, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5166, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5192, 35)
  "SPV_NV_shading_rate\0" // IR(5227, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5247, 29)
  "SPV_NV_tensor_addressing\0" // IR(5276, 25)
  "SPV_NV_viewport_array2\0" // IR(5301, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5324, 39)
  "SPV_QCOM_image_processing\0" // IR(5363, 26)
  "SPV_QCOM_image_processing2\0" // IR(5389, 27)
  "SPV_QCOM_tile_shading\0" // IR(5416, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5438, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5476, 34)
  "ReadOnly\0" // IR(5510, 9)
  "Kernel\0" // IR(5519, 7)
  "WriteOnly\0" // IR(5526, 10)
  "ReadWrite\0" // IR(5536, 10)
  "Logical\0" // IR(5546, 8)
  "Physical32\0" // IR(5554, 11)
  "Addresses\0" // IR(5565, 10)
  "Physical64\0" // IR(5575, 11)
  "PhysicalStorageBuffer64\0" // IR(5586, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(5610, 27)
  "PhysicalStorageBufferAddresses\0" // IR(5637, 31)
  "Position\0" // IR(5668, 9)
  "Shader\0" // IR(5677, 7)
  "PointSize\0" // IR(5684, 10)
  "ClipDistance\0" // IR(5694, 13)
  "CullDistance\0" // IR(5707, 13)
  "VertexId\0" // IR(5720, 9)
  "InstanceId\0" // IR(5729, 11)
  "PrimitiveId\0" // IR(5740, 12)
  "Geometry\0" // IR(5752, 9)
  "Tessellation\0" // IR(5761, 13)
  "RayTracingNV\0" // IR(5774, 13)
  "RayTracingKHR\0" // IR(5787, 14)
  "MeshShadingNV\0" // IR(5801, 14)
  "MeshShadingEXT\0" // IR(5815, 15)
  "InvocationId\0" // IR(5830, 13)
  "Layer\0" // IR(5843, 6)
  "ShaderLayer\0" // IR(5849, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(5861, 28)
  "ViewportIndex\0" // IR(5889, 14)
  "MultiViewport\0" // IR(5903, 14)
  "ShaderViewportIndex\0" // IR(5917, 20)
  "TessLevelOuter\0" // IR(5937, 15)
  "TessLevelInner\0" // IR(5952, 15)
  "TessCoord\0" // IR(5967, 10)
  "PatchVertices\0" // IR(5977, 14)
  "FragCoord\0" // IR(5991, 10)
  "PointCoord\0" // IR(6001, 11)
  "FrontFacing\0" // IR(6012, 12)
  "SampleId\0" // IR(6024, 9)
  "SampleRateShading\0" // IR(6033, 18)
  "SamplePosition\0" // IR(6051, 15)
  "SampleMask\0" // IR(6066, 11)
  "FragDepth\0" // IR(6077, 10)
  "HelperInvocation\0" // IR(6087, 17)
  "NumWorkgroups\0" // IR(6104, 14)
  "WorkgroupSize\0" // IR(6118, 14)
  "WorkgroupId\0" // IR(6132, 12)
  "LocalInvocationId\0" // IR(6144, 18)
  "GlobalInvocationId\0" // IR(6162, 19)
  "LocalInvocationIndex\0" // IR(6181, 21)
  "WorkDim\0" // IR(6202, 8)
  "GlobalSize\0" // IR(6210, 11)
  "EnqueuedWorkgroupSize\0" // IR(6221, 22)
  "GlobalOffset\0" // IR(6243, 13)
  "GlobalLinearId\0" // IR(6256, 15)
  "SubgroupSize\0" // IR(6271, 13)
  "GroupNonUniform\0" // IR(6284, 16)
  "SubgroupBallotKHR\0" // IR(6300, 18)
  "SubgroupMaxSize\0" // IR(6318, 16)
  "NumSubgroups\0" // IR(6334, 13)
  "NumEnqueuedSubgroups\0" // IR(6347, 21)
  "SubgroupId\0" // IR(6368, 11)
  "SubgroupLocalInvocationId\0" // IR(6379, 26)
  "VertexIndex\0" // IR(6405, 12)
  "InstanceIndex\0" // IR(6417, 14)
  "CoreIDARM\0" // IR(6431, 10)
  "CoreBuiltinsARM\0" // IR(6441, 16)
  "CoreCountARM\0" // IR(6457, 13)
  "CoreMaxIDARM\0" // IR(6470, 13)
  "WarpIDARM\0" // IR(6483, 10)
  "WarpMaxIDARM\0" // IR(6493, 13)
  "SubgroupEqMask\0" // IR(6506, 15)
  "SubgroupEqMaskKHR\0" // IR(6521, 18)
  "GroupNonUniformBallot\0" // IR(6539, 22)
  "SubgroupGeMask\0" // IR(6561, 15)
  "SubgroupGeMaskKHR\0" // IR(6576, 18)
  "SubgroupGtMask\0" // IR(6594, 15)
  "SubgroupGtMaskKHR\0" // IR(6609, 18)
  "SubgroupLeMask\0" // IR(6627, 15)
  "SubgroupLeMaskKHR\0" // IR(6642, 18)
  "SubgroupLtMask\0" // IR(6660, 15)
  "SubgroupLtMaskKHR\0" // IR(6675, 18)
  "BaseVertex\0" // IR(6693, 11)
  "DrawParameters\0" // IR(6704, 15)
  "BaseInstance\0" // IR(6719, 13)
  "DrawIndex\0" // IR(6732, 10)
  "PrimitiveShadingRateKHR\0" // IR(6742, 24)
  "FragmentShadingRateKHR\0" // IR(6766, 23)
  "DeviceIndex\0" // IR(6789, 12)
  "DeviceGroup\0" // IR(6801, 12)
  "ViewIndex\0" // IR(6813, 10)
  "MultiView\0" // IR(6823, 10)
  "ShadingRateKHR\0" // IR(6833, 15)
  "TileOffsetQCOM\0" // IR(6848, 15)
  "TileShadingQCOM\0" // IR(6863, 16)
  "TileDimensionQCOM\0" // IR(6879, 18)
  "TileApronSizeQCOM\0" // IR(6897, 18)
  "BaryCoordNoPerspAMD\0" // IR(6915, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(6935, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(6963, 26)
  "BaryCoordSmoothAMD\0" // IR(6989, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7008, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7035, 25)
  "BaryCoordPullModelAMD\0" // IR(7060, 22)
  "FragStencilRefEXT\0" // IR(7082, 18)
  "StencilExportEXT\0" // IR(7100, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7117, 29)
  "ShaderEnqueueAMDX\0" // IR(7146, 18)
  "ShaderIndexAMDX\0" // IR(7164, 16)
  "SamplerHeapEXT\0" // IR(7180, 15)
  "DescriptorHeapEXT\0" // IR(7195, 18)
  "ResourceHeapEXT\0" // IR(7213, 16)
  "ViewportMaskNV\0" // IR(7229, 15)
  "ShaderViewportMaskNV\0" // IR(7244, 21)
  "SecondaryPositionNV\0" // IR(7265, 20)
  "ShaderStereoViewNV\0" // IR(7285, 19)
  "SecondaryViewportMaskNV\0" // IR(7304, 24)
  "PositionPerViewNV\0" // IR(7328, 18)
  "PerViewAttributesNV\0" // IR(7346, 20)
  "ViewportMaskPerViewNV\0" // IR(7366, 22)
  "FullyCoveredEXT\0" // IR(7388, 16)
  "FragmentFullyCoveredEXT\0" // IR(7404, 24)
  "TaskCountNV\0" // IR(7428, 12)
  "PrimitiveCountNV\0" // IR(7440, 17)
  "PrimitiveIndicesNV\0" // IR(7457, 19)
  "ClipDistancePerViewNV\0" // IR(7476, 22)
  "CullDistancePerViewNV\0" // IR(7498, 22)
  "LayerPerViewNV\0" // IR(7520, 15)
  "MeshViewCountNV\0" // IR(7535, 16)
  "MeshViewIndicesNV\0" // IR(7551, 18)
  "BaryCoordKHR\0" // IR(7569, 13)
  "BaryCoordNV\0" // IR(7582, 12)
  "FragmentBarycentricKHR\0" // IR(7594, 23)
  "BaryCoordNoPerspKHR\0" // IR(7617, 20)
  "BaryCoordNoPerspNV\0" // IR(7637, 19)
  "FragSizeEXT\0" // IR(7656, 12)
  "FragmentSizeNV\0" // IR(7668, 15)
  "FragmentDensityEXT\0" // IR(7683, 19)
  "FragInvocationCountEXT\0" // IR(7702, 23)
  "InvocationsPerPixelNV\0" // IR(7725, 22)
  "PrimitivePointIndicesEXT\0" // IR(7747, 25)
  "PrimitiveLineIndicesEXT\0" // IR(7772, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(7796, 28)
  "CullPrimitiveEXT\0" // IR(7824, 17)
  "LaunchIdKHR\0" // IR(7841, 12)
  "LaunchIdNV\0" // IR(7853, 11)
  "LaunchSizeKHR\0" // IR(7864, 14)
  "LaunchSizeNV\0" // IR(7878, 13)
  "WorldRayOriginKHR\0" // IR(7891, 18)
  "WorldRayOriginNV\0" // IR(7909, 17)
  "WorldRayDirectionKHR\0" // IR(7926, 21)
  "WorldRayDirectionNV\0" // IR(7947, 20)
  "ObjectRayOriginKHR\0" // IR(7967, 19)
  "ObjectRayOriginNV\0" // IR(7986, 18)
  "ObjectRayDirectionKHR\0" // IR(8004, 22)
  "ObjectRayDirectionNV\0" // IR(8026, 21)
  "RayTminKHR\0" // IR(8047, 11)
  "RayTminNV\0" // IR(8058, 10)
  "RayTmaxKHR\0" // IR(8068, 11)
  "RayTmaxNV\0" // IR(8079, 10)
  "InstanceCustomIndexKHR\0" // IR(8089, 23)
  "InstanceCustomIndexNV\0" // IR(8112, 22)
  "ObjectToWorldKHR\0" // IR(8134, 17)
  "ObjectToWorldNV\0" // IR(8151, 16)
  "WorldToObjectKHR\0" // IR(8167, 17)
  "WorldToObjectNV\0" // IR(8184, 16)
  "HitTNV\0" // IR(8200, 7)
  "HitKindKHR\0" // IR(8207, 11)
  "HitKindNV\0" // IR(8218, 10)
  "CurrentRayTimeNV\0" // IR(8228, 17)
  "RayTracingMotionBlurNV\0" // IR(8245, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8268, 30)
  "RayTracingPositionFetchKHR\0" // IR(8298, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8325, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8359, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8392, 37)
  "IncomingRayFlagsKHR\0" // IR(8429, 20)
  "IncomingRayFlagsNV\0" // IR(8449, 19)
  "RayGeometryIndexKHR\0" // IR(8468, 20)
  "HitIsSphereNV\0" // IR(8488, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8502, 28)
  "HitIsLSSNV\0" // IR(8530, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8541, 39)
  "HitSpherePositionNV\0" // IR(8580, 20)
  "WarpsPerSMNV\0" // IR(8600, 13)
  "ShaderSMBuiltinsNV\0" // IR(8613, 19)
  "SMCountNV\0" // IR(8632, 10)
  "WarpIDNV\0" // IR(8642, 9)
  "SMIDNV\0" // IR(8651, 7)
  "HitLSSPositionsNV\0" // IR(8658, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(8676, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(8710, 33)
  "HitSphereRadiusNV\0" // IR(8743, 18)
  "HitLSSRadiiNV\0" // IR(8761, 14)
  "ClusterIDNV\0" // IR(8775, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(8787, 41)
  "CullMaskKHR\0" // IR(8828, 12)
  "RayCullMaskKHR\0" // IR(8840, 15)
  "Matrix\0" // IR(8855, 7)
  "Linkage\0" // IR(8862, 8)
  "Vector16\0" // IR(8870, 9)
  "Float16Buffer\0" // IR(8879, 14)
  "Float16\0" // IR(8893, 8)
  "Float64\0" // IR(8901, 8)
  "Int64\0" // IR(8909, 6)
  "Int64Atomics\0" // IR(8915, 13)
  "ImageBasic\0" // IR(8928, 11)
  "ImageReadWrite\0" // IR(8939, 15)
  "ImageMipmap\0" // IR(8954, 12)
  "Pipes\0" // IR(8966, 6)
  "Groups\0" // IR(8972, 7)
  "DeviceEnqueue\0" // IR(8979, 14)
  "LiteralSampler\0" // IR(8993, 15)
  "AtomicStorage\0" // IR(9008, 14)
  "Int16\0" // IR(9022, 6)
  "TessellationPointSize\0" // IR(9028, 22)
  "GeometryPointSize\0" // IR(9050, 18)
  "ImageGatherExtended\0" // IR(9068, 20)
  "StorageImageMultisample\0" // IR(9088, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9112, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9146, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9179, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9213, 33)
  "ImageCubeArray\0" // IR(9246, 15)
  "SampledCubeArray\0" // IR(9261, 17)
  "ImageRect\0" // IR(9278, 10)
  "SampledRect\0" // IR(9288, 12)
  "GenericPointer\0" // IR(9300, 15)
  "Int8\0" // IR(9315, 5)
  "InputAttachment\0" // IR(9320, 16)
  "SparseResidency\0" // IR(9336, 16)
  "MinLod\0" // IR(9352, 7)
  "Sampled1D\0" // IR(9359, 10)
  "Image1D\0" // IR(9369, 8)
  "SampledBuffer\0" // IR(9377, 14)
  "ImageBuffer\0" // IR(9391, 12)
  "ImageMSArray\0" // IR(9403, 13)
  "StorageImageExtendedFormats\0" // IR(9416, 28)
  "ImageQuery\0" // IR(9444, 11)
  "DerivativeControl\0" // IR(9455, 18)
  "InterpolationFunction\0" // IR(9473, 22)
  "TransformFeedback\0" // IR(9495, 18)
  "GeometryStreams\0" // IR(9513, 16)
  "StorageImageReadWithoutFormat\0" // IR(9529, 30)
  "StorageImageWriteWithoutFormat\0" // IR(9559, 31)
  "SubgroupDispatch\0" // IR(9590, 17)
  "NamedBarrier\0" // IR(9607, 13)
  "PipeStorage\0" // IR(9620, 12)
  "GroupNonUniformVote\0" // IR(9632, 20)
  "GroupNonUniformArithmetic\0" // IR(9652, 26)
  "GroupNonUniformShuffle\0" // IR(9678, 23)
  "GroupNonUniformShuffleRelative\0" // IR(9701, 31)
  "GroupNonUniformClustered\0" // IR(9732, 25)
  "GroupNonUniformQuad\0" // IR(9757, 20)
  "UniformDecoration\0" // IR(9777, 18)
  "TileImageColorReadAccessEXT\0" // IR(9795, 28)
  "TileImageDepthReadAccessEXT\0" // IR(9823, 28)
  "TileImageStencilReadAccessEXT\0" // IR(9851, 30)
  "TensorsARM\0" // IR(9881, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(9892, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(9929, 40)
  "GraphARM\0" // IR(9969, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(9978, 28)
  "Float8EXT\0" // IR(10006, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10016, 27)
  "CooperativeMatrixKHR\0" // IR(10043, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10064, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10097, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10140, 44)
  "SubgroupVoteKHR\0" // IR(10184, 16)
  "StorageBuffer16BitAccess\0" // IR(10200, 25)
  "StorageUniformBufferBlock16\0" // IR(10225, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10253, 35)
  "StorageUniform16\0" // IR(10288, 17)
  "StoragePushConstant16\0" // IR(10305, 22)
  "StorageInputOutput16\0" // IR(10327, 21)
  "VariablePointersStorageBuffer\0" // IR(10348, 30)
  "VariablePointers\0" // IR(10378, 17)
  "AtomicStorageOps\0" // IR(10395, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10412, 28)
  "StorageBuffer8BitAccess\0" // IR(10440, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10464, 34)
  "StoragePushConstant8\0" // IR(10498, 21)
  "DenormPreserve\0" // IR(10519, 15)
  "DenormFlushToZero\0" // IR(10534, 18)
  "SignedZeroInfNanPreserve\0" // IR(10552, 25)
  "RoundingModeRTE\0" // IR(10577, 16)
  "RoundingModeRTZ\0" // IR(10593, 16)
  "RayQueryProvisionalKHR\0" // IR(10609, 23)
  "RayQueryKHR\0" // IR(10632, 12)
  "UntypedPointersKHR\0" // IR(10644, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(10663, 32)
  "TextureSampleWeightedQCOM\0" // IR(10695, 26)
  "TextureBoxFilterQCOM\0" // IR(10721, 21)
  "TextureBlockMatchQCOM\0" // IR(10742, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(10764, 32)
  "TextureBlockMatch2QCOM\0" // IR(10796, 23)
  "Float16ImageAMD\0" // IR(10819, 16)
  "ImageGatherBiasLodAMD\0" // IR(10835, 22)
  "FragmentMaskAMD\0" // IR(10857, 16)
  "ImageReadWriteLodAMD\0" // IR(10873, 21)
  "Int64ImageEXT\0" // IR(10894, 14)
  "ShaderClockKHR\0" // IR(10908, 15)
  "QuadControlKHR\0" // IR(10923, 15)
  "Int4TypeINTEL\0" // IR(10938, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(10952, 27)
  "BFloat16TypeKHR\0" // IR(10979, 16)
  "BFloat16DotProductKHR\0" // IR(10995, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11017, 29)
  "PoisonFreezeKHR\0" // IR(11046, 16)
  "SampleMaskOverrideCoverageNV\0" // IR(11062, 29)
  "GeometryShaderPassthroughNV\0" // IR(11091, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11119, 27)
  "ImageFootprintNV\0" // IR(11146, 17)
  "FragmentBarycentricNV\0" // IR(11163, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11185, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11216, 30)
  "ShadingRateNV\0" // IR(11246, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11260, 30)
  "GroupNonUniformPartitionedNV\0" // IR(11290, 29)
  "ShaderNonUniform\0" // IR(11319, 17)
  "ShaderNonUniformEXT\0" // IR(11336, 20)
  "RuntimeDescriptorArray\0" // IR(11356, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(11379, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(11405, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(11441, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(11480, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(11519, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(11561, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(11600, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(11642, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(11679, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(11719, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(11755, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(11794, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(11831, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(11871, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(11907, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(11946, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(11985, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12027, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12069, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12114, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12156, 45)
  "VulkanMemoryModel\0" // IR(12201, 18)
  "VulkanMemoryModelKHR\0" // IR(12219, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12240, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12269, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12301, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(12335, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(12367, 31)
  "RayTracingProvisionalKHR\0" // IR(12398, 25)
  "CooperativeMatrixNV\0" // IR(12423, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(12443, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(12476, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(12514, 32)
  "DemoteToHelperInvocation\0" // IR(12546, 25)
  "DemoteToHelperInvocationEXT\0" // IR(12571, 28)
  "DisplacementMicromapNV\0" // IR(12599, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(12622, 29)
  "ShaderInvocationReorderNV\0" // IR(12651, 26)
  "ShaderInvocationReorderEXT\0" // IR(12677, 27)
  "BindlessTextureNV\0" // IR(12704, 18)
  "RayQueryPositionFetchKHR\0" // IR(12722, 25)
  "CooperativeVectorNV\0" // IR(12747, 20)
  "AtomicFloat16VectorNV\0" // IR(12767, 22)
  "RawAccessChainsNV\0" // IR(12789, 18)
  "PushConstantBanksNV\0" // IR(12807, 20)
  "LongVectorEXT\0" // IR(12827, 14)
  "Shader64BitIndexingEXT\0" // IR(12841, 23)
  "CooperativeMatrixReductionsNV\0" // IR(12864, 30)
  "CooperativeMatrixConversionsNV\0" // IR(12894, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(12925, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(12965, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13001, 30)
  "CooperativeVectorTrainingNV\0" // IR(13031, 28)
  "TensorAddressingNV\0" // IR(13059, 19)
  "SubgroupShuffleINTEL\0" // IR(13078, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(13099, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(13126, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(13152, 31)
  "RoundToInfinityINTEL\0" // IR(13183, 21)
  "FloatingPointModeINTEL\0" // IR(13204, 23)
  "IntegerFunctions2INTEL\0" // IR(13227, 23)
  "FunctionPointersINTEL\0" // IR(13250, 22)
  "IndirectReferencesINTEL\0" // IR(13272, 24)
  "AsmINTEL\0" // IR(13296, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(13305, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(13328, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(13351, 23)
  "VectorComputeINTEL\0" // IR(13374, 19)
  "VectorAnyINTEL\0" // IR(13393, 15)
  "ExpectAssumeKHR\0" // IR(13408, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(13424, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(13457, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(13495, 39)
  "VariableLengthArrayINTEL\0" // IR(13534, 25)
  "FunctionFloatControlINTEL\0" // IR(13559, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(13585, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(13612, 26)
  "FPFastMathModeINTEL\0" // IR(13638, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(13658, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(13691, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(13723, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(13761, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(13798, 30)
  "FPGALoopControlsALTERA\0" // IR(13828, 23)
  "FPGALoopControlsINTEL\0" // IR(13851, 22)
  "KernelAttributesINTEL\0" // IR(13873, 22)
  "FPGAKernelAttributesINTEL\0" // IR(13895, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(13921, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(13946, 24)
  "FPGAClusterAttributesALTERA\0" // IR(13970, 28)
  "FPGAClusterAttributesINTEL\0" // IR(13998, 27)
  "LoopFuseALTERA\0" // IR(14025, 15)
  "LoopFuseINTEL\0" // IR(14040, 14)
  "FPGADSPControlALTERA\0" // IR(14054, 21)
  "FPGADSPControlINTEL\0" // IR(14075, 20)
  "MemoryAccessAliasingINTEL\0" // IR(14095, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(14121, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(14162, 40)
  "FPGABufferLocationALTERA\0" // IR(14202, 25)
  "FPGABufferLocationINTEL\0" // IR(14227, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(14251, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(14286, 34)
  "USMStorageClassesALTERA\0" // IR(14320, 24)
  "USMStorageClassesINTEL\0" // IR(14344, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(14367, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(14397, 29)
  "IOPipesALTERA\0" // IR(14426, 14)
  "IOPipesINTEL\0" // IR(14440, 13)
  "BlockingPipesALTERA\0" // IR(14453, 20)
  "BlockingPipesINTEL\0" // IR(14473, 19)
  "FPGARegALTERA\0" // IR(14492, 14)
  "FPGARegINTEL\0" // IR(14506, 13)
  "DotProductInputAll\0" // IR(14519, 19)
  "DotProductInputAllKHR\0" // IR(14538, 22)
  "DotProductInput4x8Bit\0" // IR(14560, 22)
  "DotProductInput4x8BitKHR\0" // IR(14582, 25)
  "DotProductInput4x8BitPacked\0" // IR(14607, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(14635, 31)
  "DotProduct\0" // IR(14666, 11)
  "DotProductKHR\0" // IR(14677, 14)
  "ReplicatedCompositesEXT\0" // IR(14691, 24)
  "BitInstructions\0" // IR(14715, 16)
  "GroupNonUniformRotateKHR\0" // IR(14731, 25)
  "FloatControls2\0" // IR(14756, 15)
  "FMAKHR\0" // IR(14771, 7)
  "AtomicFloat32AddEXT\0" // IR(14778, 20)
  "AtomicFloat64AddEXT\0" // IR(14798, 20)
  "LongCompositesINTEL\0" // IR(14818, 20)
  "OptNoneEXT\0" // IR(14838, 11)
  "OptNoneINTEL\0" // IR(14849, 13)
  "AtomicFloat16AddEXT\0" // IR(14862, 20)
  "DebugInfoModuleINTEL\0" // IR(14882, 21)
  "BFloat16ConversionINTEL\0" // IR(14903, 24)
  "SplitBarrierINTEL\0" // IR(14927, 18)
  "ArithmeticFenceEXT\0" // IR(14945, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(14964, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(14994, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15023, 28)
  "TaskSequenceALTERA\0" // IR(15051, 19)
  "TaskSequenceINTEL\0" // IR(15070, 18)
  "FPMaxErrorINTEL\0" // IR(15088, 16)
  "FPGALatencyControlALTERA\0" // IR(15104, 25)
  "FPGALatencyControlINTEL\0" // IR(15129, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(15153, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(15182, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(15210, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(15240, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(15276, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(15311, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(15339, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(15362, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(15392, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(15422, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(15460, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(15488, 32)
  "SpecConditionalINTEL\0" // IR(15520, 21)
  "FunctionVariantsINTEL\0" // IR(15541, 22)
  "GroupUniformArithmeticKHR\0" // IR(15563, 26)
  "TensorFloat32RoundingINTEL\0" // IR(15589, 27)
  "MaskedGatherScatterINTEL\0" // IR(15616, 25)
  "CacheControlsINTEL\0" // IR(15641, 19)
  "RegisterLimitsINTEL\0" // IR(15660, 20)
  "BindlessImagesINTEL\0" // IR(15680, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(15700, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(15733, 33)
  "DotProductBFloat16AccVALVE\0" // IR(15766, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(15793, 32)
  "Unspecified\0" // IR(15825, 12)
  "Address\0" // IR(15837, 8)
  "Boolean\0" // IR(15845, 8)
  "Float\0" // IR(15853, 6)
  "Signed\0" // IR(15859, 7)
  "SignedChar\0" // IR(15866, 11)
  "Unsigned\0" // IR(15877, 9)
  "UnsignedChar\0" // IR(15886, 13)
  "Class\0" // IR(15899, 6)
  "Structure\0" // IR(15905, 10)
  "Union\0" // IR(15915, 6)
  "ImportedModule\0" // IR(15921, 15)
  "ImportedDeclaration\0" // IR(15936, 20)
  "None\0" // IR(15956, 5)
  "FlagIsProtected\0" // IR(15961, 16)
  "FlagIsPrivate\0" // IR(15977, 14)
  "FlagIsPublic\0" // IR(15991, 13)
  "FlagIsLocal\0" // IR(16004, 12)
  "FlagIsDefinition\0" // IR(16016, 17)
  "FlagFwdDecl\0" // IR(16033, 12)
  "FlagArtificial\0" // IR(16045, 15)
  "FlagExplicit\0" // IR(16060, 13)
  "FlagPrototyped\0" // IR(16073, 15)
  "FlagObjectPointer\0" // IR(16088, 18)
  "FlagStaticMember\0" // IR(16106, 17)
  "FlagIndirectVariable\0" // IR(16123, 21)
  "FlagLValueReference\0" // IR(16144, 20)
  "FlagRValueReference\0" // IR(16164, 20)
  "FlagIsOptimized\0" // IR(16184, 16)
  "FlagIsEnumClass\0" // IR(16200, 16)
  "FlagTypePassByValue\0" // IR(16216, 20)
  "FlagTypePassByReference\0" // IR(16236, 24)
  "Deref\0" // IR(16260, 6)
  "Plus\0" // IR(16266, 5)
  "Minus\0" // IR(16271, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(16277, 33)
  "PlusUconst\0" // IR(16310, 11)
  "BitPiece\0" // IR(16321, 9)
  "Swap\0" // IR(16330, 5)
  "Xderef\0" // IR(16335, 7)
  "StackValue\0" // IR(16342, 11)
  "Constu\0" // IR(16353, 7)
  "Fragment\0" // IR(16360, 9)
  "ConstType\0" // IR(16369, 10)
  "VolatileType\0" // IR(16379, 13)
  "RestrictType\0" // IR(16392, 13)
  "AtomicType\0" // IR(16405, 11)
  "Float16NV\0" // IR(16416, 10)
  "Float32NV\0" // IR(16426, 10)
  "Float64NV\0" // IR(16436, 10)
  "SignedInt8NV\0" // IR(16446, 13)
  "SignedInt16NV\0" // IR(16459, 14)
  "SignedInt32NV\0" // IR(16473, 14)
  "SignedInt64NV\0" // IR(16487, 14)
  "UnsignedInt8NV\0" // IR(16501, 15)
  "UnsignedInt16NV\0" // IR(16516, 16)
  "UnsignedInt32NV\0" // IR(16532, 16)
  "UnsignedInt64NV\0" // IR(16548, 16)
  "SignedInt8PackedNV\0" // IR(16564, 19)
  "UnsignedInt8PackedNV\0" // IR(16583, 21)
  "FloatE4M3NV\0" // IR(16604, 12)
  "FloatE5M2NV\0" // IR(16616, 12)
  "RowMajorKHR\0" // IR(16628, 12)
  "ColumnMajorKHR\0" // IR(16640, 15)
  "RowBlockedInterleavedARM\0" // IR(16655, 25)
  "ColumnBlockedInterleavedARM\0" // IR(16680, 28)
  "NoneKHR\0" // IR(16708, 8)
  "MatrixASignedComponentsKHR\0" // IR(16716, 27)
  "MatrixBSignedComponentsKHR\0" // IR(16743, 27)
  "MatrixCSignedComponentsKHR\0" // IR(16770, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(16797, 32)
  "SaturatingAccumulationKHR\0" // IR(16829, 26)
  "Row\0" // IR(16855, 4)
  "Column\0" // IR(16859, 7)
  "2x2\0" // IR(16866, 4)
  "MatrixAKHR\0" // IR(16870, 11)
  "MatrixBKHR\0" // IR(16881, 11)
  "MatrixAccumulatorKHR\0" // IR(16892, 21)
  "RowMajorNV\0" // IR(16913, 11)
  "ColumnMajorNV\0" // IR(16924, 14)
  "InferencingOptimalNV\0" // IR(16938, 21)
  "TrainingOptimalNV\0" // IR(16959, 18)
  "RelaxedPrecision\0" // IR(16977, 17)
  "SpecId\0" // IR(16994, 7)
  "Block\0" // IR(17001, 6)
  "BufferBlock\0" // IR(17007, 12)
  "RowMajor\0" // IR(17019, 9)
  "ColMajor\0" // IR(17028, 9)
  "ArrayStride\0" // IR(17037, 12)
  "MatrixStride\0" // IR(17049, 13)
  "GLSLShared\0" // IR(17062, 11)
  "GLSLPacked\0" // IR(17073, 11)
  "CPacked\0" // IR(17084, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(17092, 26)
  "BuiltIn\0" // IR(17118, 8)
  "NoPerspective\0" // IR(17126, 14)
  "Flat\0" // IR(17140, 5)
  "Patch\0" // IR(17145, 6)
  "Centroid\0" // IR(17151, 9)
  "Sample\0" // IR(17160, 7)
  "Invariant\0" // IR(17167, 10)
  "Restrict\0" // IR(17177, 9)
  "Aliased\0" // IR(17186, 8)
  "Volatile\0" // IR(17194, 9)
  "Constant\0" // IR(17203, 9)
  "Coherent\0" // IR(17212, 9)
  "NonWritable\0" // IR(17221, 12)
  "NonReadable\0" // IR(17233, 12)
  "Uniform\0" // IR(17245, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(17253, 26)
  "UniformId\0" // IR(17279, 10)
  "SaturatedConversion\0" // IR(17289, 20)
  "Stream\0" // IR(17309, 7)
  "Location\0" // IR(17316, 9)
  "Component\0" // IR(17325, 10)
  "Index\0" // IR(17335, 6)
  "Binding\0" // IR(17341, 8)
  "DescriptorSet\0" // IR(17349, 14)
  "Offset\0" // IR(17363, 7)
  "XfbBuffer\0" // IR(17370, 10)
  "XfbStride\0" // IR(17380, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(17390, 46)
  "FuncParamAttr\0" // IR(17436, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(17450, 34)
  "FPRoundingMode\0" // IR(17484, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(17499, 35)
  "FPFastMathMode\0" // IR(17534, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(17549, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(17581, 30)
  "LinkageAttributes\0" // IR(17611, 18)
  "NoContraction\0" // IR(17629, 14)
  "InputAttachmentIndex\0" // IR(17643, 21)
  "Alignment\0" // IR(17664, 10)
  "MaxByteOffset\0" // IR(17674, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(17688, 20)
  "AlignmentId\0" // IR(17708, 12)
  "MaxByteOffsetId\0" // IR(17720, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(17736, 44)
  "NoSignedWrap\0" // IR(17780, 13)
  "NoUnsignedWrap\0" // IR(17793, 15)
  "WeightTextureQCOM\0" // IR(17808, 18)
  "BlockMatchTextureQCOM\0" // IR(17826, 22)
  "BlockMatchSamplerQCOM\0" // IR(17848, 22)
  "ExplicitInterpAMD\0" // IR(17870, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(17888, 32)
  "NodeMaxPayloadsAMDX\0" // IR(17920, 20)
  "TrackFinishWritingAMDX\0" // IR(17940, 23)
  "PayloadNodeNameAMDX\0" // IR(17963, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(17983, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(18008, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(18035, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(18060, 28)
  "ArrayStrideIdEXT\0" // IR(18088, 17)
  "OffsetIdEXT\0" // IR(18105, 12)
  "OverrideCoverageNV\0" // IR(18117, 19)
  "PassthroughNV\0" // IR(18136, 14)
  "ViewportRelativeNV\0" // IR(18150, 19)
  "SecondaryViewportRelativeNV\0" // IR(18169, 28)
  "PerPrimitiveEXT\0" // IR(18197, 16)
  "PerPrimitiveNV\0" // IR(18213, 15)
  "PerViewNV\0" // IR(18228, 10)
  "PerTaskNV\0" // IR(18238, 10)
  "PerVertexKHR\0" // IR(18248, 13)
  "PerVertexNV\0" // IR(18261, 12)
  "NonUniform\0" // IR(18273, 11)
  "NonUniformEXT\0" // IR(18284, 14)
  "RestrictPointer\0" // IR(18298, 16)
  "RestrictPointerEXT\0" // IR(18314, 19)
  "AliasedPointer\0" // IR(18333, 15)
  "AliasedPointerEXT\0" // IR(18348, 18)
  "MemberOffsetNV\0" // IR(18366, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(18381, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(18411, 31)
  "BankNV\0" // IR(18442, 7)
  "BindlessSamplerNV\0" // IR(18449, 18)
  "BindlessImageNV\0" // IR(18467, 16)
  "BoundSamplerNV\0" // IR(18483, 15)
  "BoundImageNV\0" // IR(18498, 13)
  "SIMTCallINTEL\0" // IR(18511, 14)
  "ReferencedIndirectlyINTEL\0" // IR(18525, 26)
  "ClobberINTEL\0" // IR(18551, 13)
  "SideEffectsINTEL\0" // IR(18564, 17)
  "VectorComputeVariableINTEL\0" // IR(18581, 27)
  "FuncParamIOKindINTEL\0" // IR(18608, 21)
  "VectorComputeFunctionINTEL\0" // IR(18629, 27)
  "StackCallINTEL\0" // IR(18656, 15)
  "GlobalVariableOffsetINTEL\0" // IR(18671, 26)
  "CounterBuffer\0" // IR(18697, 14)
  "HlslCounterBufferGOOGLE\0" // IR(18711, 24)
  "UserSemantic\0" // IR(18735, 13)
  "HlslSemanticGOOGLE\0" // IR(18748, 19)
  "UserTypeGOOGLE\0" // IR(18767, 15)
  "FunctionRoundingModeINTEL\0" // IR(18782, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(18808, 31)
  "FunctionDenormModeINTEL\0" // IR(18839, 24)
  "RegisterALTERA\0" // IR(18863, 15)
  "RegisterINTEL\0" // IR(18878, 14)
  "MemoryALTERA\0" // IR(18892, 13)
  "MemoryINTEL\0" // IR(18905, 12)
  "NumbanksALTERA\0" // IR(18917, 15)
  "NumbanksINTEL\0" // IR(18932, 14)
  "BankwidthALTERA\0" // IR(18946, 16)
  "BankwidthINTEL\0" // IR(18962, 15)
  "MaxPrivateCopiesALTERA\0" // IR(18977, 23)
  "MaxPrivateCopiesINTEL\0" // IR(19000, 22)
  "SinglepumpALTERA\0" // IR(19022, 17)
  "SinglepumpINTEL\0" // IR(19039, 16)
  "DoublepumpALTERA\0" // IR(19055, 17)
  "DoublepumpINTEL\0" // IR(19072, 16)
  "MaxReplicatesALTERA\0" // IR(19088, 20)
  "MaxReplicatesINTEL\0" // IR(19108, 19)
  "SimpleDualPortALTERA\0" // IR(19127, 21)
  "SimpleDualPortINTEL\0" // IR(19148, 20)
  "MergeALTERA\0" // IR(19168, 12)
  "MergeINTEL\0" // IR(19180, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(19191, 42)
  "BankBitsALTERA\0" // IR(19233, 15)
  "BankBitsINTEL\0" // IR(19248, 14)
  "ForcePow2DepthALTERA\0" // IR(19262, 21)
  "ForcePow2DepthINTEL\0" // IR(19283, 20)
  "StridesizeALTERA\0" // IR(19303, 17)
  "StridesizeINTEL\0" // IR(19320, 16)
  "WordsizeALTERA\0" // IR(19336, 15)
  "WordsizeINTEL\0" // IR(19351, 14)
  "TrueDualPortALTERA\0" // IR(19365, 19)
  "TrueDualPortINTEL\0" // IR(19384, 18)
  "BurstCoalesceALTERA\0" // IR(19402, 20)
  "BurstCoalesceINTEL\0" // IR(19422, 19)
  "CacheSizeALTERA\0" // IR(19441, 16)
  "CacheSizeINTEL\0" // IR(19457, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(19472, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(19501, 28)
  "PrefetchALTERA\0" // IR(19529, 15)
  "PrefetchINTEL\0" // IR(19544, 14)
  "StallEnableALTERA\0" // IR(19558, 18)
  "StallEnableINTEL\0" // IR(19576, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(19593, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(19619, 25)
  "MathOpDSPModeALTERA\0" // IR(19644, 20)
  "MathOpDSPModeINTEL\0" // IR(19664, 19)
  "AliasScopeINTEL\0" // IR(19683, 16)
  "NoAliasINTEL\0" // IR(19699, 13)
  "InitiationIntervalALTERA\0" // IR(19712, 25)
  "InitiationIntervalINTEL\0" // IR(19737, 24)
  "MaxConcurrencyALTERA\0" // IR(19761, 21)
  "MaxConcurrencyINTEL\0" // IR(19782, 20)
  "PipelineEnableALTERA\0" // IR(19802, 21)
  "PipelineEnableINTEL\0" // IR(19823, 20)
  "BufferLocationALTERA\0" // IR(19843, 21)
  "BufferLocationINTEL\0" // IR(19864, 20)
  "IOPipeStorageALTERA\0" // IR(19884, 20)
  "IOPipeStorageINTEL\0" // IR(19904, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(19923, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(19957, 31)
  "SingleElementVectorINTEL\0" // IR(19988, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(20013, 35)
  "MediaBlockIOINTEL\0" // IR(20048, 18)
  "StallFreeALTERA\0" // IR(20066, 16)
  "StallFreeINTEL\0" // IR(20082, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(20097, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(20128, 26)
  "LatencyControlLabelALTERA\0" // IR(20154, 26)
  "LatencyControlLabelINTEL\0" // IR(20180, 25)
  "LatencyControlConstraintALTERA\0" // IR(20205, 31)
  "LatencyControlConstraintINTEL\0" // IR(20236, 30)
  "ConduitKernelArgumentALTERA\0" // IR(20266, 28)
  "ConduitKernelArgumentINTEL\0" // IR(20294, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(20321, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(20353, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(20384, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(20418, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(20451, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(20482, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(20512, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(20541, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(20569, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(20603, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(20638, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(20672, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(20702, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(20731, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(20764, 32)
  "StableKernelArgumentALTERA\0" // IR(20796, 27)
  "StableKernelArgumentINTEL\0" // IR(20823, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(20849, 39)
  "HostAccessINTEL\0" // IR(20888, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(20904, 47)
  "InitModeALTERA\0" // IR(20951, 15)
  "InitModeINTEL\0" // IR(20966, 14)
  "ImplementInRegisterMapALTERA\0" // IR(20980, 29)
  "ImplementInRegisterMapINTEL\0" // IR(21009, 28)
  "ConditionalINTEL\0" // IR(21037, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(21054, 36)
  "CacheControlLoadINTEL\0" // IR(21090, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(21112, 37)
  "CacheControlStoreINTEL\0" // IR(21149, 23)
  "1D\0" // IR(21172, 3)
  "2D\0" // IR(21175, 3)
  "3D\0" // IR(21178, 3)
  "Cube\0" // IR(21181, 5)
  "Rect\0" // IR(21186, 5)
  "Buffer\0" // IR(21191, 7)
  "SubpassData\0" // IR(21198, 12)
  "TileImageDataEXT\0" // IR(21210, 17)
  "Invocations\0" // IR(21227, 12)
  "SpacingEqual\0" // IR(21239, 13)
  "SpacingFractionalEven\0" // IR(21252, 22)
  "SpacingFractionalOdd\0" // IR(21274, 21)
  "VertexOrderCw\0" // IR(21295, 14)
  "VertexOrderCcw\0" // IR(21309, 15)
  "PixelCenterInteger\0" // IR(21324, 19)
  "OriginUpperLeft\0" // IR(21343, 16)
  "OriginLowerLeft\0" // IR(21359, 16)
  "EarlyFragmentTests\0" // IR(21375, 19)
  "PointMode\0" // IR(21394, 10)
  "Xfb\0" // IR(21404, 4)
  "DepthReplacing\0" // IR(21408, 15)
  "DepthGreater\0" // IR(21423, 13)
  "DepthLess\0" // IR(21436, 10)
  "DepthUnchanged\0" // IR(21446, 15)
  "LocalSize\0" // IR(21461, 10)
  "LocalSizeHint\0" // IR(21471, 14)
  "InputPoints\0" // IR(21485, 12)
  "InputLines\0" // IR(21497, 11)
  "InputLinesAdjacency\0" // IR(21508, 20)
  "Triangles\0" // IR(21528, 10)
  "InputTrianglesAdjacency\0" // IR(21538, 24)
  "Quads\0" // IR(21562, 6)
  "Isolines\0" // IR(21568, 9)
  "OutputVertices\0" // IR(21577, 15)
  "OutputPoints\0" // IR(21592, 13)
  "OutputLineStrip\0" // IR(21605, 16)
  "OutputTriangleStrip\0" // IR(21621, 20)
  "VecTypeHint\0" // IR(21641, 12)
  "ContractionOff\0" // IR(21653, 15)
  "Initializer\0" // IR(21668, 12)
  "Finalizer\0" // IR(21680, 10)
  "SubgroupsPerWorkgroup\0" // IR(21690, 22)
  "SubgroupsPerWorkgroupId\0" // IR(21712, 24)
  "LocalSizeId\0" // IR(21736, 12)
  "LocalSizeHintId\0" // IR(21748, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(21764, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(21798, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(21832, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(21868, 30)
  "PostDepthCoverage\0" // IR(21898, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(21916, 34)
  "TileShadingRateQCOM\0" // IR(21950, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(21970, 29)
  "StencilRefReplacingEXT\0" // IR(21999, 23)
  "CoalescingAMDX\0" // IR(22022, 15)
  "IsApiEntryAMDX\0" // IR(22037, 15)
  "MaxNodeRecursionAMDX\0" // IR(22052, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(22073, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(22097, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(22118, 28)
  "StencilRefGreaterFrontAMD\0" // IR(22146, 26)
  "StencilRefLessFrontAMD\0" // IR(22172, 23)
  "StencilRefUnchangedBackAMD\0" // IR(22195, 27)
  "StencilRefGreaterBackAMD\0" // IR(22222, 25)
  "StencilRefLessBackAMD\0" // IR(22247, 22)
  "QuadDerivativesKHR\0" // IR(22269, 19)
  "RequireFullQuadsKHR\0" // IR(22288, 20)
  "SharesInputWithAMDX\0" // IR(22308, 20)
  "ArithmeticPoisonKHR\0" // IR(22328, 20)
  "OutputLinesEXT\0" // IR(22348, 15)
  "OutputLinesNV\0" // IR(22363, 14)
  "OutputPrimitivesEXT\0" // IR(22377, 20)
  "OutputPrimitivesNV\0" // IR(22397, 19)
  "DerivativeGroupQuadsKHR\0" // IR(22416, 24)
  "DerivativeGroupQuadsNV\0" // IR(22440, 23)
  "DerivativeGroupLinearKHR\0" // IR(22463, 25)
  "DerivativeGroupLinearNV\0" // IR(22488, 24)
  "OutputTrianglesEXT\0" // IR(22512, 19)
  "OutputTrianglesNV\0" // IR(22531, 18)
  "PixelInterlockOrderedEXT\0" // IR(22549, 25)
  "PixelInterlockUnorderedEXT\0" // IR(22574, 27)
  "SampleInterlockOrderedEXT\0" // IR(22601, 26)
  "SampleInterlockUnorderedEXT\0" // IR(22627, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(22655, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(22686, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(22719, 27)
  "RoundingModeRTPINTEL\0" // IR(22746, 21)
  "RoundingModeRTNINTEL\0" // IR(22767, 21)
  "FloatingPointModeALTINTEL\0" // IR(22788, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(22814, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(22841, 22)
  "MaxWorkDimINTEL\0" // IR(22863, 16)
  "NoGlobalOffsetINTEL\0" // IR(22879, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(22899, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(22921, 28)
  "MaximallyReconvergesKHR\0" // IR(22949, 24)
  "FPFastMathDefault\0" // IR(22973, 18)
  "StreamingInterfaceINTEL\0" // IR(22991, 24)
  "RegisterMapInterfaceINTEL\0" // IR(23015, 26)
  "NamedBarrierCountINTEL\0" // IR(23041, 23)
  "MaximumRegistersINTEL\0" // IR(23064, 22)
  "MaximumRegistersIdINTEL\0" // IR(23086, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(23110, 51)
  "NamedMaximumRegistersINTEL\0" // IR(23161, 27)
  "Vertex\0" // IR(23188, 7)
  "TessellationControl\0" // IR(23195, 20)
  "TessellationEvaluation\0" // IR(23215, 23)
  "GLCompute\0" // IR(23238, 10)
  "TaskNV\0" // IR(23248, 7)
  "MeshNV\0" // IR(23255, 7)
  "RayGenerationKHR\0" // IR(23262, 17)
  "RayGenerationNV\0" // IR(23279, 16)
  "IntersectionKHR\0" // IR(23295, 16)
  "IntersectionNV\0" // IR(23311, 15)
  "AnyHitKHR\0" // IR(23326, 10)
  "AnyHitNV\0" // IR(23336, 9)
  "ClosestHitKHR\0" // IR(23345, 14)
  "ClosestHitNV\0" // IR(23359, 13)
  "MissKHR\0" // IR(23372, 8)
  "MissNV\0" // IR(23380, 7)
  "CallableKHR\0" // IR(23387, 12)
  "CallableNV\0" // IR(23399, 11)
  "TaskEXT\0" // IR(23410, 8)
  "MeshEXT\0" // IR(23418, 8)
  "Preserve\0" // IR(23426, 9)
  "FlushToZero\0" // IR(23435, 12)
  "BFloat16KHR\0" // IR(23447, 12)
  "Float8E4M3EXT\0" // IR(23459, 14)
  "Float8E5M2EXT\0" // IR(23473, 14)
  "IEEE\0" // IR(23487, 5)
  "ALT\0" // IR(23492, 4)
  "NotNaN\0" // IR(23496, 7)
  "NotInf\0" // IR(23503, 7)
  "NSZ\0" // IR(23510, 4)
  "AllowRecip\0" // IR(23514, 11)
  "Fast\0" // IR(23525, 5)
  "AllowContract\0" // IR(23530, 14)
  "AllowContractFastINTEL\0" // IR(23544, 23)
  "AllowReassoc\0" // IR(23567, 13)
  "AllowReassocINTEL\0" // IR(23580, 18)
  "AllowTransform\0" // IR(23598, 15)
  "RTE\0" // IR(23613, 4)
  "RTZ\0" // IR(23617, 4)
  "RTP\0" // IR(23621, 4)
  "RTN\0" // IR(23625, 4)
  "Vertical2Pixels\0" // IR(23629, 16)
  "Vertical4Pixels\0" // IR(23645, 16)
  "Horizontal2Pixels\0" // IR(23661, 18)
  "Horizontal4Pixels\0" // IR(23679, 18)
  "Inline\0" // IR(23697, 7)
  "DontInline\0" // IR(23704, 11)
  "Pure\0" // IR(23715, 5)
  "Const\0" // IR(23720, 6)
  "Zext\0" // IR(23726, 5)
  "Sext\0" // IR(23731, 5)
  "ByVal\0" // IR(23736, 6)
  "Sret\0" // IR(23742, 5)
  "NoAlias\0" // IR(23747, 8)
  "NoCapture\0" // IR(23755, 10)
  "NoWrite\0" // IR(23765, 8)
  "NoReadWrite\0" // IR(23773, 12)
  "RuntimeAlignedALTERA\0" // IR(23785, 21)
  "RuntimeAlignedINTEL\0" // IR(23806, 20)
  "Reduce\0" // IR(23826, 7)
  "InclusiveScan\0" // IR(23833, 14)
  "ExclusiveScan\0" // IR(23847, 14)
  "ClusteredReduce\0" // IR(23861, 16)
  "PartitionedReduceEXT\0" // IR(23877, 21)
  "PartitionedReduceNV\0" // IR(23898, 20)
  "PartitionedInclusiveScanEXT\0" // IR(23918, 28)
  "PartitionedInclusiveScanNV\0" // IR(23946, 27)
  "PartitionedExclusiveScanEXT\0" // IR(23973, 28)
  "PartitionedExclusiveScanNV\0" // IR(24001, 27)
  "NoneINTEL\0" // IR(24028, 10)
  "ReadINTEL\0" // IR(24038, 10)
  "WriteINTEL\0" // IR(24048, 11)
  "ReadWriteINTEL\0" // IR(24059, 15)
  "Bias\0" // IR(24074, 5)
  "Lod\0" // IR(24079, 4)
  "Grad\0" // IR(24083, 5)
  "ConstOffset\0" // IR(24088, 12)
  "ConstOffsets\0" // IR(24100, 13)
  "MakeTexelAvailable\0" // IR(24113, 19)
  "MakeTexelAvailableKHR\0" // IR(24132, 22)
  "MakeTexelVisible\0" // IR(24154, 17)
  "MakeTexelVisibleKHR\0" // IR(24171, 20)
  "NonPrivateTexel\0" // IR(24191, 16)
  "NonPrivateTexelKHR\0" // IR(24207, 19)
  "VolatileTexel\0" // IR(24226, 14)
  "VolatileTexelKHR\0" // IR(24240, 17)
  "SignExtend\0" // IR(24257, 11)
  "ZeroExtend\0" // IR(24268, 11)
  "Nontemporal\0" // IR(24279, 12)
  "Offsets\0" // IR(24291, 8)
  "SnormInt8\0" // IR(24299, 10)
  "SnormInt16\0" // IR(24309, 11)
  "UnormInt8\0" // IR(24320, 10)
  "UnormInt16\0" // IR(24330, 11)
  "UnormShort565\0" // IR(24341, 14)
  "UnormShort555\0" // IR(24355, 14)
  "UnormInt101010\0" // IR(24369, 15)
  "SignedInt8\0" // IR(24384, 11)
  "SignedInt16\0" // IR(24395, 12)
  "SignedInt32\0" // IR(24407, 12)
  "UnsignedInt8\0" // IR(24419, 13)
  "UnsignedInt16\0" // IR(24432, 14)
  "UnsignedInt32\0" // IR(24446, 14)
  "HalfFloat\0" // IR(24460, 10)
  "UnormInt24\0" // IR(24470, 11)
  "UnormInt101010_2\0" // IR(24481, 17)
  "UnormInt10X6EXT\0" // IR(24498, 16)
  "UnsignedIntRaw10EXT\0" // IR(24514, 20)
  "UnsignedIntRaw12EXT\0" // IR(24534, 20)
  "UnormInt2_101010EXT\0" // IR(24554, 20)
  "UnsignedInt10X6EXT\0" // IR(24574, 19)
  "UnsignedInt12X4EXT\0" // IR(24593, 19)
  "UnsignedInt14X2EXT\0" // IR(24612, 19)
  "UnormInt12X4EXT\0" // IR(24631, 16)
  "UnormInt14X2EXT\0" // IR(24647, 16)
  "R\0" // IR(24663, 2)
  "A\0" // IR(24665, 2)
  "RG\0" // IR(24667, 3)
  "RA\0" // IR(24670, 3)
  "RGB\0" // IR(24673, 4)
  "RGBA\0" // IR(24677, 5)
  "BGRA\0" // IR(24682, 5)
  "ARGB\0" // IR(24687, 5)
  "Intensity\0" // IR(24692, 10)
  "Luminance\0" // IR(24702, 10)
  "Rx\0" // IR(24712, 3)
  "RGx\0" // IR(24715, 4)
  "RGBx\0" // IR(24719, 5)
  "Depth\0" // IR(24724, 6)
  "DepthStencil\0" // IR(24730, 13)
  "sRGB\0" // IR(24743, 5)
  "sRGBx\0" // IR(24748, 6)
  "sRGBA\0" // IR(24754, 6)
  "sBGRA\0" // IR(24760, 6)
  "ABGR\0" // IR(24766, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(24771, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(24799, 27)
  "InitOnDeviceResetALTERA\0" // IR(24826, 24)
  "InitOnDeviceResetINTEL\0" // IR(24850, 23)
  "NoWait\0" // IR(24873, 7)
  "WaitKernel\0" // IR(24880, 11)
  "WaitWorkGroup\0" // IR(24891, 14)
  "CmdExecTime\0" // IR(24905, 12)
  "MayUsePrintf\0" // IR(24917, 13)
  "Export\0" // IR(24930, 7)
  "Import\0" // IR(24937, 7)
  "LinkOnceODR\0" // IR(24944, 12)
  "UncachedINTEL\0" // IR(24956, 14)
  "CachedINTEL\0" // IR(24970, 12)
  "StreamingINTEL\0" // IR(24982, 15)
  "InvalidateAfterReadINTEL\0" // IR(24997, 25)
  "ConstCachedINTEL\0" // IR(25022, 17)
  "Unroll\0" // IR(25039, 7)
  "DontUnroll\0" // IR(25046, 11)
  "DependencyInfinite\0" // IR(25057, 19)
  "DependencyLength\0" // IR(25076, 17)
  "MinIterations\0" // IR(25093, 14)
  "MaxIterations\0" // IR(25107, 14)
  "IterationMultiple\0" // IR(25121, 18)
  "PeelCount\0" // IR(25139, 10)
  "PartialCount\0" // IR(25149, 13)
  "DependencyArrayALTERA\0" // IR(25162, 22)
  "DependencyArrayINTEL\0" // IR(25184, 21)
  "LoopCoalesceALTERA\0" // IR(25205, 19)
  "LoopCoalesceINTEL\0" // IR(25224, 18)
  "MaxInterleavingALTERA\0" // IR(25242, 22)
  "MaxInterleavingINTEL\0" // IR(25264, 21)
  "SpeculatedIterationsALTERA\0" // IR(25285, 27)
  "SpeculatedIterationsINTEL\0" // IR(25312, 26)
  "NoFusionALTERA\0" // IR(25338, 15)
  "NoFusionINTEL\0" // IR(25353, 14)
  "LoopCountALTERA\0" // IR(25367, 16)
  "LoopCountINTEL\0" // IR(25383, 15)
  "MaxReinvocationDelayALTERA\0" // IR(25398, 27)
  "MaxReinvocationDelayINTEL\0" // IR(25425, 26)
  "MatrixASignedComponentsINTEL\0" // IR(25451, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(25480, 29)
  "MatrixCBFloat16INTEL\0" // IR(25509, 21)
  "MatrixResultBFloat16INTEL\0" // IR(25530, 26)
  "MatrixAPackedInt8INTEL\0" // IR(25556, 23)
  "MatrixBPackedInt8INTEL\0" // IR(25579, 23)
  "MatrixAPackedInt4INTEL\0" // IR(25602, 23)
  "MatrixBPackedInt4INTEL\0" // IR(25625, 23)
  "MatrixATF32INTEL\0" // IR(25648, 17)
  "MatrixBTF32INTEL\0" // IR(25665, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(25682, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(25708, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(25734, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(25761, 27)
  "Aligned\0" // IR(25788, 8)
  "MakePointerAvailable\0" // IR(25796, 21)
  "MakePointerAvailableKHR\0" // IR(25817, 24)
  "MakePointerVisible\0" // IR(25841, 19)
  "MakePointerVisibleKHR\0" // IR(25860, 22)
  "NonPrivatePointer\0" // IR(25882, 18)
  "NonPrivatePointerKHR\0" // IR(25900, 21)
  "AliasScopeINTELMask\0" // IR(25921, 20)
  "NoAliasINTELMask\0" // IR(25941, 17)
  "Simple\0" // IR(25958, 7)
  "GLSL450\0" // IR(25965, 8)
  "OpenCL\0" // IR(25973, 7)
  "Vulkan\0" // IR(25980, 7)
  "VulkanKHR\0" // IR(25987, 10)
  "Relaxed\0" // IR(25997, 8)
  "Acquire\0" // IR(26005, 8)
  "Release\0" // IR(26013, 8)
  "AcquireRelease\0" // IR(26021, 15)
  "SequentiallyConsistent\0" // IR(26036, 23)
  "UniformMemory\0" // IR(26059, 14)
  "SubgroupMemory\0" // IR(26073, 15)
  "WorkgroupMemory\0" // IR(26088, 16)
  "CrossWorkgroupMemory\0" // IR(26104, 21)
  "AtomicCounterMemory\0" // IR(26125, 20)
  "ImageMemory\0" // IR(26145, 12)
  "OutputMemory\0" // IR(26157, 13)
  "OutputMemoryKHR\0" // IR(26170, 16)
  "MakeAvailable\0" // IR(26186, 14)
  "MakeAvailableKHR\0" // IR(26200, 17)
  "MakeVisible\0" // IR(26217, 12)
  "MakeVisibleKHR\0" // IR(26229, 15)
  "AutoINTEL\0" // IR(26244, 10)
  "WRAP\0" // IR(26254, 5)
  "SAT\0" // IR(26259, 4)
  "SAT_ZERO\0" // IR(26263, 9)
  "SAT_SYM\0" // IR(26272, 8)
  "PackedVectorFormat4x8Bit\0" // IR(26280, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(26305, 28)
  "TRN\0" // IR(26333, 4)
  "TRN_ZERO\0" // IR(26337, 9)
  "RND\0" // IR(26346, 4)
  "RND_ZERO\0" // IR(26350, 9)
  "RND_INF\0" // IR(26359, 8)
  "RND_MIN_INF\0" // IR(26367, 12)
  "RND_CONV\0" // IR(26379, 9)
  "RND_CONV_ODD\0" // IR(26388, 13)
  "RobustnessPerComponentNV\0" // IR(26401, 25)
  "RobustnessPerElementNV\0" // IR(26426, 23)
  "OpaqueKHR\0" // IR(26449, 10)
  "NoOpaqueKHR\0" // IR(26459, 12)
  "TerminateOnFirstHitKHR\0" // IR(26471, 23)
  "SkipClosestHitShaderKHR\0" // IR(26494, 24)
  "CullBackFacingTrianglesKHR\0" // IR(26518, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(26545, 28)
  "CullOpaqueKHR\0" // IR(26573, 14)
  "CullNoOpaqueKHR\0" // IR(26587, 16)
  "SkipTrianglesKHR\0" // IR(26603, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(26620, 24)
  "SkipAABBsKHR\0" // IR(26644, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(26657, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(26687, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(26728, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(26765, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(26802, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(26843, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(26885, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(26918, 33)
  "ClampToEdge\0" // IR(26951, 12)
  "Clamp\0" // IR(26963, 6)
  "Repeat\0" // IR(26969, 7)
  "RepeatMirrored\0" // IR(26976, 15)
  "Nearest\0" // IR(26991, 8)
  "Linear\0" // IR(26999, 7)
  "Unknown\0" // IR(27006, 8)
  "Rgba32f\0" // IR(27014, 8)
  "Rgba16f\0" // IR(27022, 8)
  "R32f\0" // IR(27030, 5)
  "Rgba8\0" // IR(27035, 6)
  "Rgba8Snorm\0" // IR(27041, 11)
  "Rg32f\0" // IR(27052, 6)
  "Rg16f\0" // IR(27058, 6)
  "R11fG11fB10f\0" // IR(27064, 13)
  "R16f\0" // IR(27077, 5)
  "Rgba16\0" // IR(27082, 7)
  "Rgb10A2\0" // IR(27089, 8)
  "Rg16\0" // IR(27097, 5)
  "Rg8\0" // IR(27102, 4)
  "R16\0" // IR(27106, 4)
  "R8\0" // IR(27110, 3)
  "Rgba16Snorm\0" // IR(27113, 12)
  "Rg16Snorm\0" // IR(27125, 10)
  "Rg8Snorm\0" // IR(27135, 9)
  "R16Snorm\0" // IR(27144, 9)
  "R8Snorm\0" // IR(27153, 8)
  "Rgba32i\0" // IR(27161, 8)
  "Rgba16i\0" // IR(27169, 8)
  "Rgba8i\0" // IR(27177, 7)
  "R32i\0" // IR(27184, 5)
  "Rg32i\0" // IR(27189, 6)
  "Rg16i\0" // IR(27195, 6)
  "Rg8i\0" // IR(27201, 5)
  "R16i\0" // IR(27206, 5)
  "R8i\0" // IR(27211, 4)
  "Rgba32ui\0" // IR(27215, 9)
  "Rgba16ui\0" // IR(27224, 9)
  "Rgba8ui\0" // IR(27233, 8)
  "R32ui\0" // IR(27241, 6)
  "Rgb10a2ui\0" // IR(27247, 10)
  "Rg32ui\0" // IR(27257, 7)
  "Rg16ui\0" // IR(27264, 7)
  "Rg8ui\0" // IR(27271, 6)
  "R16ui\0" // IR(27277, 6)
  "R8ui\0" // IR(27283, 5)
  "R64ui\0" // IR(27288, 6)
  "R64i\0" // IR(27294, 5)
  "CrossDevice\0" // IR(27299, 12)
  "Device\0" // IR(27311, 7)
  "Workgroup\0" // IR(27318, 10)
  "Subgroup\0" // IR(27328, 9)
  "Invocation\0" // IR(27337, 11)
  "QueueFamily\0" // IR(27348, 12)
  "QueueFamilyKHR\0" // IR(27360, 15)
  "ShaderCallKHR\0" // IR(27375, 14)
  "Flatten\0" // IR(27389, 8)
  "DontFlatten\0" // IR(27397, 12)
  "IdentifierPossibleDuplicates\0" // IR(27409, 29)
  "FlagUnknownPhysicalLayout\0" // IR(27438, 26)
  "ESSL\0" // IR(27464, 5)
  "GLSL\0" // IR(27469, 5)
  "OpenCL_C\0" // IR(27474, 9)
  "OpenCL_CPP\0" // IR(27483, 11)
  "HLSL\0" // IR(27494, 5)
  "CPP_for_OpenCL\0" // IR(27499, 15)
  "SYCL\0" // IR(27514, 5)
  "HERO_C\0" // IR(27519, 7)
  "NZSL\0" // IR(27526, 5)
  "WGSL\0" // IR(27531, 5)
  "Slang\0" // IR(27536, 6)
  "Zig\0" // IR(27542, 4)
  "Rust\0" // IR(27546, 5)
  "UniformConstant\0" // IR(27551, 16)
  "Input\0" // IR(27567, 6)
  "Output\0" // IR(27573, 7)
  "CrossWorkgroup\0" // IR(27580, 15)
  "Private\0" // IR(27595, 8)
  "Function\0" // IR(27603, 9)
  "Generic\0" // IR(27612, 8)
  "PushConstant\0" // IR(27620, 13)
  "AtomicCounter\0" // IR(27633, 14)
  "Image\0" // IR(27647, 6)
  "StorageBuffer\0" // IR(27653, 14)
  "TileImageEXT\0" // IR(27667, 13)
  "TileAttachmentQCOM\0" // IR(27680, 19)
  "NodePayloadAMDX\0" // IR(27699, 16)
  "CallableDataKHR\0" // IR(27715, 16)
  "CallableDataNV\0" // IR(27731, 15)
  "IncomingCallableDataKHR\0" // IR(27746, 24)
  "IncomingCallableDataNV\0" // IR(27770, 23)
  "RayPayloadKHR\0" // IR(27793, 14)
  "RayPayloadNV\0" // IR(27807, 13)
  "HitAttributeKHR\0" // IR(27820, 16)
  "HitAttributeNV\0" // IR(27836, 15)
  "IncomingRayPayloadKHR\0" // IR(27851, 22)
  "IncomingRayPayloadNV\0" // IR(27873, 21)
  "ShaderRecordBufferKHR\0" // IR(27894, 22)
  "ShaderRecordBufferNV\0" // IR(27916, 21)
  "PhysicalStorageBuffer\0" // IR(27937, 22)
  "PhysicalStorageBufferEXT\0" // IR(27959, 25)
  "HitObjectAttributeNV\0" // IR(27984, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(28005, 24)
  "HitObjectAttributeEXT\0" // IR(28029, 22)
  "CodeSectionINTEL\0" // IR(28051, 17)
  "DeviceOnlyALTERA\0" // IR(28068, 17)
  "DeviceOnlyINTEL\0" // IR(28085, 16)
  "HostOnlyALTERA\0" // IR(28101, 15)
  "HostOnlyINTEL\0" // IR(28116, 14)
  "WriteThroughINTEL\0" // IR(28130, 18)
  "WriteBackINTEL\0" // IR(28148, 15)
  "TensorView\0" // IR(28163, 11)
  "DecodeFunc\0" // IR(28174, 11)
  "Undefined\0" // IR(28185, 10)
  "NoneARM\0" // IR(28195, 8)
  "NontemporalARM\0" // IR(28203, 15)
  "OutOfBoundsValueARM\0" // IR(28218, 20)
  "MakeElementAvailableARM\0" // IR(28238, 24)
  "MakeElementVisibleARM\0" // IR(28262, 22)
  "NonPrivateElementARM\0" // IR(28284, 21)
  "Nop\0" // IR(28305, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(28309, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(28334, 27)
  "Undef\0" // IR(28361, 6)
  "SourceContinued\0" // IR(28367, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(28383, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(28416, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(28445, 41)
  "Source\0" // IR(28486, 7)
  "SourceExtension\0" // IR(28493, 16)
  "Name\0" // IR(28509, 5)
  "MemberName\0" // IR(28514, 11)
  "String\0" // IR(28525, 7)
  "Line\0" // IR(28532, 5)
  "Extension\0" // IR(28537, 10)
  "ExtInstImport\0" // IR(28547, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(28561, 46)
  "ExtInst\0" // IR(28607, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(28615, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(28649, 30)
  "MemoryModel\0" // IR(28679, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(28691, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(28724, 29)
  "EntryPoint\0" // IR(28753, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(28764, 32)
  "ExecutionMode\0" // IR(28796, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(28810, 28)
  "Capability\0" // IR(28838, 11)
  "TypeVoid\0" // IR(28849, 9)
  "TypeBool\0" // IR(28858, 9)
  "TypeInt\0" // IR(28867, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(28875, 37)
  "TypeFloat\0" // IR(28912, 10)
  "TypeVector\0" // IR(28922, 11)
  "TypeMatrix\0" // IR(28933, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(28944, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(28976, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(29014, 43)
  "TypeImage\0" // IR(29057, 10)
  "TypeSampler\0" // IR(29067, 12)
  "TypeSampledImage\0" // IR(29079, 17)
  "TypeArray\0" // IR(29096, 10)
  "TypeRuntimeArray\0" // IR(29106, 17)
  "TypeStruct\0" // IR(29123, 11)
  "TypeOpaque\0" // IR(29134, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(29145, 31)
  "TypePointer\0" // IR(29176, 12)
  "TypeFunction\0" // IR(29188, 13)
  "TypeEvent\0" // IR(29201, 10)
  "TypeDeviceEvent\0" // IR(29211, 16)
  "TypeReserveId\0" // IR(29227, 14)
  "TypeQueue\0" // IR(29241, 10)
  "TypePipe\0" // IR(29251, 9)
  "TypeForwardPointer\0" // IR(29260, 19)
  "ConstantTrue\0" // IR(29279, 13)
  "ConstantFalse\0" // IR(29292, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(29306, 38)
  "ConstantComposite\0" // IR(29344, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(29362, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(29403, 37)
  "ConstantSampler\0" // IR(29440, 16)
  "ConstantNull\0" // IR(29456, 13)
  "SpecConstantTrue\0" // IR(29469, 17)
  "SpecConstantFalse\0" // IR(29486, 18)
  "SpecConstant\0" // IR(29504, 13)
  "SpecConstantComposite\0" // IR(29517, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(29539, 41)
  "SpecConstantOp\0" // IR(29580, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(29595, 34)
  "FunctionParameter\0" // IR(29629, 18)
  "FunctionEnd\0" // IR(29647, 12)
  "FunctionCall\0" // IR(29659, 13)
  "Variable\0" // IR(29672, 9)
  "ImageTexelPointer\0" // IR(29681, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(29699, 40)
  "Load\0" // IR(29739, 5)
  "Store\0" // IR(29744, 6)
  "CopyMemory\0" // IR(29750, 11)
  "CopyMemorySized\0" // IR(29761, 16)
  "AccessChain\0" // IR(29777, 12)
  "InBoundsAccessChain\0" // IR(29789, 20)
  "PtrAccessChain\0" // IR(29809, 15)
  "ArrayLength\0" // IR(29824, 12)
  "GenericPtrMemSemantics\0" // IR(29836, 23)
  "InBoundsPtrAccessChain\0" // IR(29859, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(29882, 28)
  "Decorate\0" // IR(29910, 9)
  "MemberDecorate\0" // IR(29919, 15)
  "DecorationGroup\0" // IR(29934, 16)
  "GroupDecorate\0" // IR(29950, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(29964, 45)
  "GroupMemberDecorate\0" // IR(30009, 20)
  "VectorExtractDynamic\0" // IR(30029, 21)
  "VectorInsertDynamic\0" // IR(30050, 20)
  "VectorShuffle\0" // IR(30070, 14)
  "CompositeConstruct\0" // IR(30084, 19)
  "CompositeExtract\0" // IR(30103, 17)
  "CompositeInsert\0" // IR(30120, 16)
  "CopyObject\0" // IR(30136, 11)
  "Transpose\0" // IR(30147, 10)
  "SampledImage\0" // IR(30157, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(30170, 32)
  "ImageSampleImplicitLod\0" // IR(30202, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(30225, 23)
  "ImageSampleExplicitLod\0" // IR(30248, 23)
  "ImageSampleDrefImplicitLod\0" // IR(30271, 27)
  "ImageSampleDrefExplicitLod\0" // IR(30298, 27)
  "ImageSampleProjImplicitLod\0" // IR(30325, 27)
  "ImageSampleProjExplicitLod\0" // IR(30352, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(30379, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(30410, 31)
  "ImageFetch\0" // IR(30441, 11)
  "ImageGather\0" // IR(30452, 12)
  "ImageDrefGather\0" // IR(30464, 16)
  "ImageRead\0" // IR(30480, 10)
  "ImageWrite\0" // IR(30490, 11)
  "ImageQueryFormat\0" // IR(30501, 17)
  "ImageQueryOrder\0" // IR(30518, 16)
  "ImageQuerySizeLod\0" // IR(30534, 18)
  "ImageQuerySize\0" // IR(30552, 15)
  "ImageQueryLod\0" // IR(30567, 14)
  "ImageQueryLevels\0" // IR(30581, 17)
  "ImageQuerySamples\0" // IR(30598, 18)
  "ConvertFToU\0" // IR(30616, 12)
  "ConvertFToS\0" // IR(30628, 12)
  "ConvertSToF\0" // IR(30640, 12)
  "ConvertUToF\0" // IR(30652, 12)
  "UConvert\0" // IR(30664, 9)
  "SConvert\0" // IR(30673, 9)
  "FConvert\0" // IR(30682, 9)
  "QuantizeToF16\0" // IR(30691, 14)
  "ConvertPtrToU\0" // IR(30705, 14)
  "SatConvertSToU\0" // IR(30719, 15)
  "SatConvertUToS\0" // IR(30734, 15)
  "ConvertUToPtr\0" // IR(30749, 14)
  "PtrCastToGeneric\0" // IR(30763, 17)
  "GenericCastToPtr\0" // IR(30780, 17)
  "GenericCastToPtrExplicit\0" // IR(30797, 25)
  "Bitcast\0" // IR(30822, 8)
  "SNegate\0" // IR(30830, 8)
  "FNegate\0" // IR(30838, 8)
  "IAdd\0" // IR(30846, 5)
  "FAdd\0" // IR(30851, 5)
  "ISub\0" // IR(30856, 5)
  "FSub\0" // IR(30861, 5)
  "IMul\0" // IR(30866, 5)
  "FMul\0" // IR(30871, 5)
  "UDiv\0" // IR(30876, 5)
  "SDiv\0" // IR(30881, 5)
  "FDiv\0" // IR(30886, 5)
  "UMod\0" // IR(30891, 5)
  "SRem\0" // IR(30896, 5)
  "SMod\0" // IR(30901, 5)
  "FRem\0" // IR(30906, 5)
  "FMod\0" // IR(30911, 5)
  "VectorTimesScalar\0" // IR(30916, 18)
  "MatrixTimesScalar\0" // IR(30934, 18)
  "VectorTimesMatrix\0" // IR(30952, 18)
  "MatrixTimesVector\0" // IR(30970, 18)
  "MatrixTimesMatrix\0" // IR(30988, 18)
  "OuterProduct\0" // IR(31006, 13)
  "Dot\0" // IR(31019, 4)
  "IAddCarry\0" // IR(31023, 10)
  "ISubBorrow\0" // IR(31033, 11)
  "UMulExtended\0" // IR(31044, 13)
  "SMulExtended\0" // IR(31057, 13)
  "Any\0" // IR(31070, 4)
  "All\0" // IR(31074, 4)
  "IsNan\0" // IR(31078, 6)
  "IsInf\0" // IR(31084, 6)
  "IsFinite\0" // IR(31090, 9)
  "IsNormal\0" // IR(31099, 9)
  "SignBitSet\0" // IR(31108, 11)
  "LessOrGreater\0" // IR(31119, 14)
  "Ordered\0" // IR(31133, 8)
  "Unordered\0" // IR(31141, 10)
  "LogicalEqual\0" // IR(31151, 13)
  "LogicalNotEqual\0" // IR(31164, 16)
  "LogicalOr\0" // IR(31180, 10)
  "LogicalAnd\0" // IR(31190, 11)
  "LogicalNot\0" // IR(31201, 11)
  "Select\0" // IR(31212, 7)
  "IEqual\0" // IR(31219, 7)
  "INotEqual\0" // IR(31226, 10)
  "UGreaterThan\0" // IR(31236, 13)
  "SGreaterThan\0" // IR(31249, 13)
  "UGreaterThanEqual\0" // IR(31262, 18)
  "SGreaterThanEqual\0" // IR(31280, 18)
  "ULessThan\0" // IR(31298, 10)
  "SLessThan\0" // IR(31308, 10)
  "ULessThanEqual\0" // IR(31318, 15)
  "SLessThanEqual\0" // IR(31333, 15)
  "FOrdEqual\0" // IR(31348, 10)
  "FUnordEqual\0" // IR(31358, 12)
  "FOrdNotEqual\0" // IR(31370, 13)
  "FUnordNotEqual\0" // IR(31383, 15)
  "FOrdLessThan\0" // IR(31398, 13)
  "FUnordLessThan\0" // IR(31411, 15)
  "FOrdGreaterThan\0" // IR(31426, 16)
  "FUnordGreaterThan\0" // IR(31442, 18)
  "FOrdLessThanEqual\0" // IR(31460, 18)
  "FUnordLessThanEqual\0" // IR(31478, 20)
  "FOrdGreaterThanEqual\0" // IR(31498, 21)
  "FUnordGreaterThanEqual\0" // IR(31519, 23)
  "ShiftRightLogical\0" // IR(31542, 18)
  "ShiftRightArithmetic\0" // IR(31560, 21)
  "ShiftLeftLogical\0" // IR(31581, 17)
  "BitwiseOr\0" // IR(31598, 10)
  "BitwiseXor\0" // IR(31608, 11)
  "BitwiseAnd\0" // IR(31619, 11)
  "Not\0" // IR(31630, 4)
  "BitFieldInsert\0" // IR(31634, 15)
  "BitFieldSExtract\0" // IR(31649, 17)
  "BitFieldUExtract\0" // IR(31666, 17)
  "BitReverse\0" // IR(31683, 11)
  "BitCount\0" // IR(31694, 9)
  "DPdx\0" // IR(31703, 5)
  "DPdy\0" // IR(31708, 5)
  "Fwidth\0" // IR(31713, 7)
  "DPdxFine\0" // IR(31720, 9)
  "DPdyFine\0" // IR(31729, 9)
  "FwidthFine\0" // IR(31738, 11)
  "DPdxCoarse\0" // IR(31749, 11)
  "DPdyCoarse\0" // IR(31760, 11)
  "FwidthCoarse\0" // IR(31771, 13)
  "EmitVertex\0" // IR(31784, 11)
  "EndPrimitive\0" // IR(31795, 13)
  "EmitStreamVertex\0" // IR(31808, 17)
  "EndStreamPrimitive\0" // IR(31825, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(31844, 37)
  "ControlBarrier\0" // IR(31881, 15)
  "MemoryBarrier\0" // IR(31896, 14)
  "AtomicLoad\0" // IR(31910, 11)
  "AtomicStore\0" // IR(31921, 12)
  "AtomicExchange\0" // IR(31933, 15)
  "AtomicCompareExchange\0" // IR(31948, 22)
  "AtomicCompareExchangeWeak\0" // IR(31970, 26)
  "AtomicIIncrement\0" // IR(31996, 17)
  "AtomicIDecrement\0" // IR(32013, 17)
  "AtomicIAdd\0" // IR(32030, 11)
  "AtomicISub\0" // IR(32041, 11)
  "AtomicSMin\0" // IR(32052, 11)
  "AtomicUMin\0" // IR(32063, 11)
  "AtomicSMax\0" // IR(32074, 11)
  "AtomicUMax\0" // IR(32085, 11)
  "AtomicAnd\0" // IR(32096, 10)
  "AtomicOr\0" // IR(32106, 9)
  "AtomicXor\0" // IR(32115, 10)
  "Phi\0" // IR(32125, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(32129, 30)
  "LoopMerge\0" // IR(32159, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(32169, 35)
  "SelectionMerge\0" // IR(32204, 15)
  "Label\0" // IR(32219, 6)
  "Branch\0" // IR(32225, 7)
  "BranchConditional\0" // IR(32232, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(32250, 45)
  "Switch\0" // IR(32295, 7)
  "Kill\0" // IR(32302, 5)
  "Return\0" // IR(32307, 7)
  "ReturnValue\0" // IR(32314, 12)
  "Unreachable\0" // IR(32326, 12)
  "LifetimeStart\0" // IR(32338, 14)
  "LifetimeStop\0" // IR(32352, 13)
  "GroupAsyncCopy\0" // IR(32365, 15)
  "GroupWaitEvents\0" // IR(32380, 16)
  "GroupAll\0" // IR(32396, 9)
  "GroupAny\0" // IR(32405, 9)
  "GroupBroadcast\0" // IR(32414, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(32429, 33)
  "GroupIAdd\0" // IR(32462, 10)
  "GroupFAdd\0" // IR(32472, 10)
  "GroupFMin\0" // IR(32482, 10)
  "GroupUMin\0" // IR(32492, 10)
  "GroupSMin\0" // IR(32502, 10)
  "GroupFMax\0" // IR(32512, 10)
  "GroupUMax\0" // IR(32522, 10)
  "GroupSMax\0" // IR(32532, 10)
  "ReadPipe\0" // IR(32542, 9)
  "WritePipe\0" // IR(32551, 10)
  "ReservedReadPipe\0" // IR(32561, 17)
  "ReservedWritePipe\0" // IR(32578, 18)
  "ReserveReadPipePackets\0" // IR(32596, 23)
  "ReserveWritePipePackets\0" // IR(32619, 24)
  "CommitReadPipe\0" // IR(32643, 15)
  "CommitWritePipe\0" // IR(32658, 16)
  "IsValidReserveId\0" // IR(32674, 17)
  "GetNumPipePackets\0" // IR(32691, 18)
  "GetMaxPipePackets\0" // IR(32709, 18)
  "GroupReserveReadPipePackets\0" // IR(32727, 28)
  "GroupReserveWritePipePackets\0" // IR(32755, 29)
  "GroupCommitReadPipe\0" // IR(32784, 20)
  "GroupCommitWritePipe\0" // IR(32804, 21)
  "EnqueueMarker\0" // IR(32825, 14)
  "EnqueueKernel\0" // IR(32839, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(32853, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(32883, 32)
  "GetKernelWorkGroupSize\0" // IR(32915, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(32938, 40)
  "RetainEvent\0" // IR(32978, 12)
  "ReleaseEvent\0" // IR(32990, 13)
  "CreateUserEvent\0" // IR(33003, 16)
  "IsValidEvent\0" // IR(33019, 13)
  "SetUserEventStatus\0" // IR(33032, 19)
  "CaptureEventProfilingInfo\0" // IR(33051, 26)
  "GetDefaultQueue\0" // IR(33077, 16)
  "BuildNDRange\0" // IR(33093, 13)
  "ImageSparseSampleImplicitLod\0" // IR(33106, 29)
  "ImageSparseSampleExplicitLod\0" // IR(33135, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(33164, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(33197, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(33230, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(33263, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(33296, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(33333, 37)
  "ImageSparseFetch\0" // IR(33370, 17)
  "ImageSparseGather\0" // IR(33387, 18)
  "ImageSparseDrefGather\0" // IR(33405, 22)
  "ImageSparseTexelsResident\0" // IR(33427, 26)
  "NoLine\0" // IR(33453, 7)
  "AtomicFlagTestAndSet\0" // IR(33460, 21)
  "AtomicFlagClear\0" // IR(33481, 16)
  "ImageSparseRead\0" // IR(33497, 16)
  "SizeOf\0" // IR(33513, 7)
  "TypePipeStorage\0" // IR(33520, 16)
  "ConstantPipeStorage\0" // IR(33536, 20)
  "CreatePipeFromPipeStorage\0" // IR(33556, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(33582, 35)
  "GetKernelMaxNumSubgroups\0" // IR(33617, 25)
  "TypeNamedBarrier\0" // IR(33642, 17)
  "NamedBarrierInitialize\0" // IR(33659, 23)
  "MemoryNamedBarrier\0" // IR(33682, 19)
  "ModuleProcessed\0" // IR(33701, 16)
  "ExecutionModeId\0" // IR(33717, 16)
  "DecorateId\0" // IR(33733, 11)
  "GroupNonUniformElect\0" // IR(33744, 21)
  "GroupNonUniformAll\0" // IR(33765, 19)
  "GroupNonUniformAny\0" // IR(33784, 19)
  "GroupNonUniformAllEqual\0" // IR(33803, 24)
  "GroupNonUniformBroadcast\0" // IR(33827, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(33852, 30)
  "GroupNonUniformInverseBallot\0" // IR(33882, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(33911, 32)
  "GroupNonUniformBallotBitCount\0" // IR(33943, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(33973, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(34002, 29)
  "GroupNonUniformShuffleXor\0" // IR(34031, 26)
  "GroupNonUniformShuffleUp\0" // IR(34057, 25)
  "GroupNonUniformShuffleDown\0" // IR(34082, 27)
  "GroupNonUniformIAdd\0" // IR(34109, 20)
  "GroupNonUniformFAdd\0" // IR(34129, 20)
  "GroupNonUniformIMul\0" // IR(34149, 20)
  "GroupNonUniformFMul\0" // IR(34169, 20)
  "GroupNonUniformSMin\0" // IR(34189, 20)
  "GroupNonUniformUMin\0" // IR(34209, 20)
  "GroupNonUniformFMin\0" // IR(34229, 20)
  "GroupNonUniformSMax\0" // IR(34249, 20)
  "GroupNonUniformUMax\0" // IR(34269, 20)
  "GroupNonUniformFMax\0" // IR(34289, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(34309, 26)
  "GroupNonUniformBitwiseOr\0" // IR(34335, 25)
  "GroupNonUniformBitwiseXor\0" // IR(34360, 26)
  "GroupNonUniformLogicalAnd\0" // IR(34386, 26)
  "GroupNonUniformLogicalOr\0" // IR(34412, 25)
  "GroupNonUniformLogicalXor\0" // IR(34437, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(34463, 29)
  "GroupNonUniformQuadSwap\0" // IR(34492, 24)
  "CopyLogical\0" // IR(34516, 12)
  "PtrEqual\0" // IR(34528, 9)
  "PtrNotEqual\0" // IR(34537, 12)
  "PtrDiff\0" // IR(34549, 8)
  "ColorAttachmentReadEXT\0" // IR(34557, 23)
  "DepthAttachmentReadEXT\0" // IR(34580, 23)
  "StencilAttachmentReadEXT\0" // IR(34603, 25)
  "TypeTensorARM\0" // IR(34628, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(34642, 42)
  "TensorReadARM\0" // IR(34684, 14)
  "TensorWriteARM\0" // IR(34698, 15)
  "TensorQuerySizeARM\0" // IR(34713, 19)
  "GraphConstantARM\0" // IR(34732, 17)
  "GraphEntryPointARM\0" // IR(34749, 19)
  "GraphInputARM\0" // IR(34768, 14)
  "GraphSetOutputARM\0" // IR(34782, 18)
  "GraphEndARM\0" // IR(34800, 12)
  "TypeGraphARM\0" // IR(34812, 13)
  "TerminateInvocation\0" // IR(34825, 20)
  "TypeUntypedPointerKHR\0" // IR(34845, 22)
  "UntypedVariableKHR\0" // IR(34867, 19)
  "UntypedAccessChainKHR\0" // IR(34886, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(34908, 30)
  "SubgroupFirstInvocationKHR\0" // IR(34938, 27)
  "UntypedPtrAccessChainKHR\0" // IR(34965, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(34990, 33)
  "UntypedArrayLengthKHR\0" // IR(35023, 22)
  "UntypedPrefetchKHR\0" // IR(35045, 19)
  "FmaKHR\0" // IR(35064, 7)
  "SubgroupAllKHR\0" // IR(35071, 15)
  "SubgroupAnyKHR\0" // IR(35086, 15)
  "SubgroupAllEqualKHR\0" // IR(35101, 20)
  "SubgroupReadInvocationKHR\0" // IR(35121, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(35147, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(35173, 25)
  "TraceRayKHR\0" // IR(35198, 12)
  "ExecuteCallableKHR\0" // IR(35210, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(35229, 35)
  "IgnoreIntersectionKHR\0" // IR(35264, 22)
  "TerminateRayKHR\0" // IR(35286, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(35302, 47)
  "SDot\0" // IR(35349, 5)
  "SDotKHR\0" // IR(35354, 8)
  "UDot\0" // IR(35362, 5)
  "UDotKHR\0" // IR(35367, 8)
  "SUDot\0" // IR(35375, 6)
  "SUDotKHR\0" // IR(35381, 9)
  "SDotAccSat\0" // IR(35390, 11)
  "SDotAccSatKHR\0" // IR(35401, 14)
  "UDotAccSat\0" // IR(35415, 11)
  "UDotAccSatKHR\0" // IR(35426, 14)
  "SUDotAccSat\0" // IR(35440, 12)
  "SUDotAccSatKHR\0" // IR(35452, 15)
  "TypeCooperativeMatrixKHR\0" // IR(35467, 25)
  "CooperativeMatrixLoadKHR\0" // IR(35492, 25)
  "CooperativeMatrixStoreKHR\0" // IR(35517, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(35543, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(35597, 27)
  "CooperativeMatrixLengthKHR\0" // IR(35624, 27)
  "ConstantCompositeReplicateEXT\0" // IR(35651, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(35681, 34)
  "CompositeConstructReplicateEXT\0" // IR(35715, 31)
  "TypeRayQueryKHR\0" // IR(35746, 16)
  "RayQueryInitializeKHR\0" // IR(35762, 22)
  "RayQueryTerminateKHR\0" // IR(35784, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(35805, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(35837, 31)
  "RayQueryProceedKHR\0" // IR(35868, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(35887, 31)
  "ImageSampleWeightedQCOM\0" // IR(35918, 24)
  "ImageBoxFilterQCOM\0" // IR(35942, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(35961, 23)
  "ImageBlockMatchSADQCOM\0" // IR(35984, 23)
  "BitCastArrayQCOM\0" // IR(36007, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(36024, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(36053, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(36082, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(36111, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(36140, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(36170, 28)
  "ExtractSubArrayQCOM\0" // IR(36198, 20)
  "GroupIAddNonUniformAMD\0" // IR(36218, 23)
  "GroupFAddNonUniformAMD\0" // IR(36241, 23)
  "GroupFMinNonUniformAMD\0" // IR(36264, 23)
  "GroupUMinNonUniformAMD\0" // IR(36287, 23)
  "GroupSMinNonUniformAMD\0" // IR(36310, 23)
  "GroupFMaxNonUniformAMD\0" // IR(36333, 23)
  "GroupUMaxNonUniformAMD\0" // IR(36356, 23)
  "GroupSMaxNonUniformAMD\0" // IR(36379, 23)
  "FragmentMaskFetchAMD\0" // IR(36402, 21)
  "FragmentFetchAMD\0" // IR(36423, 17)
  "ReadClockKHR\0" // IR(36440, 13)
  "AllocateNodePayloadsAMDX\0" // IR(36453, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(36478, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(36502, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(36527, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(36556, 27)
  "IsNodePayloadValidAMDX\0" // IR(36583, 23)
  "ConstantStringAMDX\0" // IR(36606, 19)
  "SpecConstantStringAMDX\0" // IR(36625, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(36648, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(36674, 26)
  "TypeBufferEXT\0" // IR(36700, 14)
  "BufferPointerEXT\0" // IR(36714, 17)
  "UntypedImageTexelPointerEXT\0" // IR(36731, 28)
  "MemberDecorateIdEXT\0" // IR(36759, 20)
  "ConstantSizeOfEXT\0" // IR(36779, 18)
  "PoisonKHR\0" // IR(36797, 10)
  "FreezeKHR\0" // IR(36807, 10)
  "HitObjectRecordHitMotionNV\0" // IR(36817, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(36844, 36)
  "HitObjectRecordMissMotionNV\0" // IR(36880, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(36908, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(36936, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(36964, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(36997, 30)
  "HitObjectTraceRayMotionNV\0" // IR(37027, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(37053, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(37092, 44)
  "HitObjectRecordEmptyNV\0" // IR(37136, 23)
  "HitObjectTraceRayNV\0" // IR(37159, 20)
  "HitObjectRecordHitNV\0" // IR(37179, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(37200, 30)
  "HitObjectRecordMissNV\0" // IR(37230, 22)
  "HitObjectExecuteShaderNV\0" // IR(37252, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(37277, 26)
  "HitObjectGetAttributesNV\0" // IR(37303, 25)
  "HitObjectGetHitKindNV\0" // IR(37328, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(37350, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(37379, 28)
  "HitObjectGetInstanceIdNV\0" // IR(37407, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(37432, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(37466, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(37498, 29)
  "HitObjectGetRayTMaxNV\0" // IR(37527, 22)
  "HitObjectGetRayTMinNV\0" // IR(37549, 22)
  "HitObjectIsEmptyNV\0" // IR(37571, 19)
  "HitObjectIsHitNV\0" // IR(37590, 17)
  "HitObjectIsMissNV\0" // IR(37607, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(37625, 29)
  "ReorderThreadWithHintNV\0" // IR(37654, 24)
  "TypeHitObjectNV\0" // IR(37678, 16)
  "ImageSampleFootprintNV\0" // IR(37694, 23)
  "TypeVectorIdEXT\0" // IR(37717, 16)
  "TypeCooperativeVectorNV\0" // IR(37733, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(37757, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(37786, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(37828, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(37867, 32)
  "CooperativeMatrixConvertNV\0" // IR(37899, 27)
  "EmitMeshTasksEXT\0" // IR(37926, 17)
  "SetMeshOutputsEXT\0" // IR(37943, 18)
  "GroupNonUniformPartitionEXT\0" // IR(37961, 28)
  "GroupNonUniformPartitionNV\0" // IR(37989, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(38016, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(38049, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(38084, 38)
  "CooperativeVectorLoadNV\0" // IR(38122, 24)
  "CooperativeVectorStoreNV\0" // IR(38146, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(38171, 28)
  "HitObjectRecordMissEXT\0" // IR(38199, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(38222, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(38251, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(38302, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(38326, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(38371, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(38404, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(38436, 38)
  "TypeHitObjectEXT\0" // IR(38474, 17)
  "ReorderThreadWithHintEXT\0" // IR(38491, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(38516, 30)
  "HitObjectTraceRayEXT\0" // IR(38546, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(38567, 27)
  "HitObjectRecordEmptyEXT\0" // IR(38594, 24)
  "HitObjectExecuteShaderEXT\0" // IR(38618, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(38644, 27)
  "HitObjectGetAttributesEXT\0" // IR(38671, 26)
  "HitObjectGetHitKindEXT\0" // IR(38697, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(38720, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(38750, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(38779, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(38805, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(38840, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(38871, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(38905, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(38938, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(38968, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(38997, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(39026, 23)
  "ReportIntersectionKHR\0" // IR(39049, 22)
  "ReportIntersectionNV\0" // IR(39071, 21)
  "IgnoreIntersectionNV\0" // IR(39092, 21)
  "TerminateRayNV\0" // IR(39113, 15)
  "TraceNV\0" // IR(39128, 8)
  "TraceMotionNV\0" // IR(39136, 14)
  "TraceRayMotionNV\0" // IR(39150, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(39167, 50)
  "TypeAccelerationStructureKHR\0" // IR(39217, 29)
  "TypeAccelerationStructureNV\0" // IR(39246, 28)
  "ExecuteCallableNV\0" // IR(39274, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(39292, 35)
  "RayQueryGetClusterIdNV\0" // IR(39327, 23)
  "HitObjectGetClusterIdNV\0" // IR(39350, 24)
  "HitObjectGetRayTMinEXT\0" // IR(39374, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(39397, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(39442, 40)
  "HitObjectIsEmptyEXT\0" // IR(39482, 20)
  "HitObjectIsHitEXT\0" // IR(39502, 18)
  "HitObjectIsMissEXT\0" // IR(39520, 19)
  "TypeCooperativeMatrixNV\0" // IR(39539, 24)
  "CooperativeMatrixLoadNV\0" // IR(39563, 24)
  "CooperativeMatrixStoreNV\0" // IR(39587, 25)
  "CooperativeMatrixMulAddNV\0" // IR(39612, 26)
  "CooperativeMatrixLengthNV\0" // IR(39638, 26)
  "BeginInvocationInterlockEXT\0" // IR(39664, 28)
  "EndInvocationInterlockEXT\0" // IR(39692, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(39718, 43)
  "CooperativeMatrixReduceNV\0" // IR(39761, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(39787, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(39818, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(39862, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(39892, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(39923, 32)
  "TypeTensorLayoutNV\0" // IR(39955, 19)
  "TypeTensorViewNV\0" // IR(39974, 17)
  "CreateTensorLayoutNV\0" // IR(39991, 21)
  "TensorLayoutSetDimensionNV\0" // IR(40012, 27)
  "TensorLayoutSetStrideNV\0" // IR(40039, 24)
  "TensorLayoutSliceNV\0" // IR(40063, 20)
  "TensorLayoutSetClampValueNV\0" // IR(40083, 28)
  "CreateTensorViewNV\0" // IR(40111, 19)
  "TensorViewSetDimensionNV\0" // IR(40130, 25)
  "TensorViewSetStrideNV\0" // IR(40155, 22)
  "IsHelperInvocationEXT\0" // IR(40177, 22)
  "TensorViewSetClipNV\0" // IR(40199, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(40219, 27)
  "CooperativeMatrixTransposeNV\0" // IR(40246, 29)
  "ConvertUToImageNV\0" // IR(40275, 18)
  "ConvertUToSamplerNV\0" // IR(40293, 20)
  "ConvertImageToUNV\0" // IR(40313, 18)
  "ConvertSamplerToUNV\0" // IR(40331, 20)
  "ConvertUToSampledImageNV\0" // IR(40351, 25)
  "ConvertSampledImageToUNV\0" // IR(40376, 25)
  "SamplerImageAddressingModeNV\0" // IR(40401, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(40430, 52)
  "RawAccessChainNV\0" // IR(40482, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(40499, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(40539, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(40577, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(40615, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(40649, 37)
  "HitObjectGetSpherePositionNV\0" // IR(40686, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(40715, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(40742, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(40769, 23)
  "HitObjectIsSphereHitNV\0" // IR(40792, 23)
  "HitObjectIsLSSHitNV\0" // IR(40815, 20)
  "RayQueryIsSphereHitNV\0" // IR(40835, 22)
  "RayQueryIsLSSHitNV\0" // IR(40857, 19)
  "SubgroupShuffleDownINTEL\0" // IR(40876, 25)
  "SubgroupShuffleUpINTEL\0" // IR(40901, 23)
  "SubgroupShuffleXorINTEL\0" // IR(40924, 24)
  "SubgroupBlockReadINTEL\0" // IR(40948, 23)
  "SubgroupBlockWriteINTEL\0" // IR(40971, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(40995, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(41023, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(41052, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(41085, 34)
  "UCountLeadingZerosINTEL\0" // IR(41119, 24)
  "UCountTrailingZerosINTEL\0" // IR(41143, 25)
  "AbsISubINTEL\0" // IR(41168, 13)
  "AbsUSubINTEL\0" // IR(41181, 13)
  "IAddSatINTEL\0" // IR(41194, 13)
  "UAddSatINTEL\0" // IR(41207, 13)
  "IAverageINTEL\0" // IR(41220, 14)
  "UAverageINTEL\0" // IR(41234, 14)
  "IAverageRoundedINTEL\0" // IR(41248, 21)
  "UAverageRoundedINTEL\0" // IR(41269, 21)
  "ISubSatINTEL\0" // IR(41290, 13)
  "USubSatINTEL\0" // IR(41303, 13)
  "IMul32x16INTEL\0" // IR(41316, 15)
  "UMul32x16INTEL\0" // IR(41331, 15)
  "ConstantFunctionPointerINTEL\0" // IR(41346, 29)
  "FunctionPointerCallINTEL\0" // IR(41375, 25)
  "AsmTargetINTEL\0" // IR(41400, 15)
  "AsmCallINTEL\0" // IR(41415, 13)
  "AtomicFMinEXT\0" // IR(41428, 14)
  "AtomicFMaxEXT\0" // IR(41442, 14)
  "AssumeTrueKHR\0" // IR(41456, 14)
  "ExpectKHR\0" // IR(41470, 10)
  "DecorateString\0" // IR(41480, 15)
  "DecorateStringGOOGLE\0" // IR(41495, 21)
  "MemberDecorateString\0" // IR(41516, 21)
  "MemberDecorateStringGOOGLE\0" // IR(41537, 27)
  "VmeImageINTEL\0" // IR(41564, 14)
  "TypeVmeImageINTEL\0" // IR(41578, 18)
  "TypeAvcImePayloadINTEL\0" // IR(41596, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(41619, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(41642, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(41665, 23)
  "TypeAvcMceResultINTEL\0" // IR(41688, 22)
  "TypeAvcImeResultINTEL\0" // IR(41710, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(41732, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(41778, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(41822, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(41861, 37)
  "TypeAvcRefResultINTEL\0" // IR(41898, 22)
  "TypeAvcSicResultINTEL\0" // IR(41920, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(41942, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(42002, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(42055, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(42102, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(42142, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(42193, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(42237, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(42288, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(42344, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(42394, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(42446, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(42495, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(42542, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(42592, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(42643, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(42699, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(42732, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(42784, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(42845, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(42906, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(42945, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(42983, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(43022, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(43060, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(43099, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(43137, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(43173, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(43212, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(43255, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(43293, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(43331, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(43369, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(43414, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(43454, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(43516, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(43546, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(43584, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(43620, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(43653, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(43688, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(43727, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(43770, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(43817, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(43871, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(43905, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(43952, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(43997, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(44052, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(44105, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(44161, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(44215, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(44273, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(44329, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(44367, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(44413, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(44457, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(44506, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(44553, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(44623, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(44691, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(44760, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(44828, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(44894, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(44961, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(44997, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(45045, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(45104, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(45162, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(45218, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(45248, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(45278, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(45317, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(45363, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(45406, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(45453, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(45498, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(45544, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(45600, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(45638, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(45668, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(45700, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(45736, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(45778, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(45817, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(45856, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(45900, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(45948, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(45998, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(46041, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(46089, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(46132, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(46163, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(46210, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(46255, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(46301, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(46357, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(46395, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(46430, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(46474, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(46520, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(46561, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(46597, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(46647, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(46695, 35)
  "SaveMemoryINTEL\0" // IR(46730, 16)
  "RestoreMemoryINTEL\0" // IR(46746, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(46765, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(46794, 28)
  "ArbitraryFloatCastALTERA\0" // IR(46822, 25)
  "ArbitraryFloatCastINTEL\0" // IR(46847, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(46871, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(46903, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(46934, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(46964, 29)
  "ArbitraryFloatAddALTERA\0" // IR(46993, 24)
  "ArbitraryFloatAddINTEL\0" // IR(47017, 23)
  "ArbitraryFloatSubALTERA\0" // IR(47040, 24)
  "ArbitraryFloatSubINTEL\0" // IR(47064, 23)
  "ArbitraryFloatMulALTERA\0" // IR(47087, 24)
  "ArbitraryFloatMulINTEL\0" // IR(47111, 23)
  "ArbitraryFloatDivALTERA\0" // IR(47134, 24)
  "ArbitraryFloatDivINTEL\0" // IR(47158, 23)
  "ArbitraryFloatGTALTERA\0" // IR(47181, 23)
  "ArbitraryFloatGTINTEL\0" // IR(47204, 22)
  "ArbitraryFloatGEALTERA\0" // IR(47226, 23)
  "ArbitraryFloatGEINTEL\0" // IR(47249, 22)
  "ArbitraryFloatLTALTERA\0" // IR(47271, 23)
  "ArbitraryFloatLTINTEL\0" // IR(47294, 22)
  "ArbitraryFloatLEALTERA\0" // IR(47316, 23)
  "ArbitraryFloatLEINTEL\0" // IR(47339, 22)
  "ArbitraryFloatEQALTERA\0" // IR(47361, 23)
  "ArbitraryFloatEQINTEL\0" // IR(47384, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(47406, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(47432, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(47457, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(47483, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(47508, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(47533, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(47557, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(47583, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(47608, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(47633, 24)
  "ArbitraryFloatLogINTEL\0" // IR(47657, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(47680, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(47704, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(47729, 25)
  "ArbitraryFloatExpINTEL\0" // IR(47754, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(47777, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(47801, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(47826, 25)
  "ArbitraryFloatSinINTEL\0" // IR(47851, 23)
  "ArbitraryFloatCosINTEL\0" // IR(47874, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(47897, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(47923, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(47948, 25)
  "ArbitraryFloatASinINTEL\0" // IR(47973, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(47997, 26)
  "ArbitraryFloatACosINTEL\0" // IR(48023, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(48047, 26)
  "ArbitraryFloatATanINTEL\0" // IR(48073, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(48097, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(48123, 25)
  "ArbitraryFloatPowINTEL\0" // IR(48148, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(48171, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(48195, 24)
  "LoopControlINTEL\0" // IR(48219, 17)
  "AliasDomainDeclINTEL\0" // IR(48236, 21)
  "AliasScopeDeclINTEL\0" // IR(48257, 20)
  "AliasScopeListDeclINTEL\0" // IR(48277, 24)
  "FixedSqrtALTERA\0" // IR(48301, 16)
  "FixedSqrtINTEL\0" // IR(48317, 15)
  "FixedRecipALTERA\0" // IR(48332, 17)
  "FixedRecipINTEL\0" // IR(48349, 16)
  "FixedRsqrtALTERA\0" // IR(48365, 17)
  "FixedRsqrtINTEL\0" // IR(48382, 16)
  "FixedSinALTERA\0" // IR(48398, 15)
  "FixedSinINTEL\0" // IR(48413, 14)
  "FixedCosALTERA\0" // IR(48427, 15)
  "FixedCosINTEL\0" // IR(48442, 14)
  "FixedSinCosALTERA\0" // IR(48456, 18)
  "FixedSinCosINTEL\0" // IR(48474, 17)
  "FixedSinPiALTERA\0" // IR(48491, 17)
  "FixedSinPiINTEL\0" // IR(48508, 16)
  "FixedCosPiALTERA\0" // IR(48524, 17)
  "FixedCosPiINTEL\0" // IR(48541, 16)
  "FixedSinCosPiALTERA\0" // IR(48557, 20)
  "FixedSinCosPiINTEL\0" // IR(48577, 19)
  "FixedLogALTERA\0" // IR(48596, 15)
  "FixedLogINTEL\0" // IR(48611, 14)
  "FixedExpALTERA\0" // IR(48625, 15)
  "FixedExpINTEL\0" // IR(48640, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(48654, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(48684, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(48713, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(48743, 29)
  "ReadPipeBlockingALTERA\0" // IR(48772, 23)
  "ReadPipeBlockingINTEL\0" // IR(48795, 22)
  "WritePipeBlockingALTERA\0" // IR(48817, 24)
  "WritePipeBlockingINTEL\0" // IR(48841, 23)
  "RayQueryGetRayTMinKHR\0" // IR(48864, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(48886, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(48909, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(48937, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(48983, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(49020, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(49085, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(49125, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(49166, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(49205, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(49241, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(49287, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(49332, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(49374, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(49406, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(49435, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(49475, 40)
  "AtomicFAddEXT\0" // IR(49515, 14)
  "TypeBufferSurfaceINTEL\0" // IR(49529, 23)
  "TypeStructContinuedINTEL\0" // IR(49552, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(49577, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(49609, 36)
  "CompositeConstructContinuedINTEL\0" // IR(49645, 33)
  "ConvertFToBF16INTEL\0" // IR(49678, 20)
  "ConvertBF16ToFINTEL\0" // IR(49698, 20)
  "ControlBarrierArriveINTEL\0" // IR(49718, 26)
  "ControlBarrierWaitINTEL\0" // IR(49744, 24)
  "TaskSequenceCreateALTERA\0" // IR(49768, 25)
  "TaskSequenceCreateINTEL\0" // IR(49793, 24)
  "TaskSequenceAsyncALTERA\0" // IR(49817, 24)
  "TaskSequenceAsyncINTEL\0" // IR(49841, 23)
  "TaskSequenceGetALTERA\0" // IR(49864, 22)
  "TaskSequenceGetINTEL\0" // IR(49886, 21)
  "TaskSequenceReleaseALTERA\0" // IR(49907, 26)
  "TaskSequenceReleaseINTEL\0" // IR(49933, 25)
  "TypeTaskSequenceALTERA\0" // IR(49958, 23)
  "TypeTaskSequenceINTEL\0" // IR(49981, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(50003, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(50030, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(50055, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(50089, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(50123, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(50152, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(50178, 62)
  "BitwiseFunctionINTEL\0" // IR(50240, 21)
  "ConditionalExtensionINTEL\0" // IR(50261, 26)
  "ConditionalEntryPointINTEL\0" // IR(50287, 27)
  "ConditionalCapabilityINTEL\0" // IR(50314, 27)
  "SpecConstantTargetINTEL\0" // IR(50341, 24)
  "SpecConstantArchitectureINTEL\0" // IR(50365, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(50395, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(50432, 30)
  "ConditionalCopyObjectINTEL\0" // IR(50462, 27)
  "GroupIMulKHR\0" // IR(50489, 13)
  "GroupFMulKHR\0" // IR(50502, 13)
  "GroupBitwiseAndKHR\0" // IR(50515, 19)
  "GroupBitwiseOrKHR\0" // IR(50534, 18)
  "GroupBitwiseXorKHR\0" // IR(50552, 19)
  "GroupLogicalAndKHR\0" // IR(50571, 19)
  "GroupLogicalOrKHR\0" // IR(50590, 18)
  "GroupLogicalXorKHR\0" // IR(50608, 19)
  "RoundFToTF32INTEL\0" // IR(50627, 18)
  "MaskedGatherINTEL\0" // IR(50645, 18)
  "MaskedScatterINTEL\0" // IR(50663, 19)
  "ConvertHandleToImageINTEL\0" // IR(50682, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(50708, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(50736, 33)
  "FDot2MixAcc32VALVE\0" // IR(50769, 19)
  "FDot2MixAcc16VALVE\0" // IR(50788, 19)
  "FDot4MixAcc32VALVE\0" // IR(50807, 19)
  "MIN_SAD\0" // IR(50826, 8)
  "MIN_SAD_COST\0" // IR(50834, 13)
  "RAW_SAD\0" // IR(50847, 8)
  "DebugInfoNone\0" // IR(50855, 14)
  "DebugCompilationUnit\0" // IR(50869, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(50890, 52)
  "DebugTypeBasic\0" // IR(50942, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(50957, 34)
  "DebugTypePointer\0" // IR(50991, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(51008, 38)
  "DebugTypeQualifier\0" // IR(51046, 19)
  "DebugTypeArray\0" // IR(51065, 15)
  "DebugTypeVector\0" // IR(51080, 16)
  "DebugTypedef\0" // IR(51096, 13)
  "DebugTypeFunction\0" // IR(51109, 18)
  "DebugTypeEnum\0" // IR(51127, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(51141, 38)
  "DebugTypeComposite\0" // IR(51179, 19)
  "DebugTypeMember\0" // IR(51198, 16)
  "DebugTypeInheritance\0" // IR(51214, 21)
  "DebugTypePtrToMember\0" // IR(51235, 21)
  "DebugTypeTemplate\0" // IR(51256, 18)
  "DebugTypeTemplateParameter\0" // IR(51274, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(51301, 35)
  "DebugTypeTemplateParameterPack\0" // IR(51336, 31)
  "DebugGlobalVariable\0" // IR(51367, 20)
  "DebugFunctionDeclaration\0" // IR(51387, 25)
  "DebugFunction\0" // IR(51412, 14)
  "DebugLexicalBlock\0" // IR(51426, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(51444, 31)
  "DebugScope\0" // IR(51475, 11)
  "DebugNoScope\0" // IR(51486, 13)
  "DebugInlinedAt\0" // IR(51499, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(51514, 42)
  "DebugLocalVariable\0" // IR(51556, 19)
  "DebugInlinedVariable\0" // IR(51575, 21)
  "DebugDeclare\0" // IR(51596, 13)
  "DebugValue\0" // IR(51609, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(51620, 33)
  "DebugOperation\0" // IR(51653, 15)
  "DebugExpression\0" // IR(51668, 16)
  "DebugMacroDef\0" // IR(51684, 14)
  "DebugMacroUndef\0" // IR(51698, 16)
  "Round\0" // IR(51714, 6)
  "RoundEven\0" // IR(51720, 10)
  "Trunc\0" // IR(51730, 6)
  "FAbs\0" // IR(51736, 5)
  "SAbs\0" // IR(51741, 5)
  "FSign\0" // IR(51746, 6)
  "SSign\0" // IR(51752, 6)
  "Floor\0" // IR(51758, 6)
  "Ceil\0" // IR(51764, 5)
  "Fract\0" // IR(51769, 6)
  "Radians\0" // IR(51775, 8)
  "Degrees\0" // IR(51783, 8)
  "Sin\0" // IR(51791, 4)
  "Cos\0" // IR(51795, 4)
  "Tan\0" // IR(51799, 4)
  "Asin\0" // IR(51803, 5)
  "Acos\0" // IR(51808, 5)
  "Atan\0" // IR(51813, 5)
  "Sinh\0" // IR(51818, 5)
  "Cosh\0" // IR(51823, 5)
  "Tanh\0" // IR(51828, 5)
  "Asinh\0" // IR(51833, 6)
  "Acosh\0" // IR(51839, 6)
  "Atanh\0" // IR(51845, 6)
  "Atan2\0" // IR(51851, 6)
  "Pow\0" // IR(51857, 4)
  "Exp\0" // IR(51861, 4)
  "Log\0" // IR(51865, 4)
  "Exp2\0" // IR(51869, 5)
  "Log2\0" // IR(51874, 5)
  "Sqrt\0" // IR(51879, 5)
  "InverseSqrt\0" // IR(51884, 12)
  "Determinant\0" // IR(51896, 12)
  "MatrixInverse\0" // IR(51908, 14)
  "Modf\0" // IR(51922, 5)
  "ModfStruct\0" // IR(51927, 11)
  "FMin\0" // IR(51938, 5)
  "UMin\0" // IR(51943, 5)
  "SMin\0" // IR(51948, 5)
  "FMax\0" // IR(51953, 5)
  "UMax\0" // IR(51958, 5)
  "SMax\0" // IR(51963, 5)
  "FClamp\0" // IR(51968, 7)
  "UClamp\0" // IR(51975, 7)
  "SClamp\0" // IR(51982, 7)
  "FMix\0" // IR(51989, 5)
  "IMix\0" // IR(51994, 5)
  "Step\0" // IR(51999, 5)
  "SmoothStep\0" // IR(52004, 11)
  "Fma\0" // IR(52015, 4)
  "Frexp\0" // IR(52019, 6)
  "FrexpStruct\0" // IR(52025, 12)
  "Ldexp\0" // IR(52037, 6)
  "PackSnorm4x8\0" // IR(52043, 13)
  "PackUnorm4x8\0" // IR(52056, 13)
  "PackSnorm2x16\0" // IR(52069, 14)
  "PackUnorm2x16\0" // IR(52083, 14)
  "PackHalf2x16\0" // IR(52097, 13)
  "PackDouble2x32\0" // IR(52110, 15)
  "UnpackSnorm2x16\0" // IR(52125, 16)
  "UnpackUnorm2x16\0" // IR(52141, 16)
  "UnpackHalf2x16\0" // IR(52157, 15)
  "UnpackSnorm4x8\0" // IR(52172, 15)
  "UnpackUnorm4x8\0" // IR(52187, 15)
  "UnpackDouble2x32\0" // IR(52202, 17)
  "Length\0" // IR(52219, 7)
  "Distance\0" // IR(52226, 9)
  "Cross\0" // IR(52235, 6)
  "Normalize\0" // IR(52241, 10)
  "FaceForward\0" // IR(52251, 12)
  "Reflect\0" // IR(52263, 8)
  "Refract\0" // IR(52271, 8)
  "FindILsb\0" // IR(52279, 9)
  "FindSMsb\0" // IR(52288, 9)
  "FindUMsb\0" // IR(52297, 9)
  "InterpolateAtCentroid\0" // IR(52306, 22)
  "InterpolateAtSample\0" // IR(52328, 20)
  "InterpolateAtOffset\0" // IR(52348, 20)
  "NMin\0" // IR(52368, 5)
  "NMax\0" // IR(52373, 5)
  "NClamp\0" // IR(52378, 7)
  "ArgumentInfo\0" // IR(52385, 13)
  "ArgumentStorageBuffer\0" // IR(52398, 22)
  "ArgumentUniform\0" // IR(52420, 16)
  "ArgumentPodStorageBuffer\0" // IR(52436, 25)
  "ArgumentPodUniform\0" // IR(52461, 19)
  "ArgumentPodPushConstant\0" // IR(52480, 24)
  "ArgumentSampledImage\0" // IR(52504, 21)
  "ArgumentStorageImage\0" // IR(52525, 21)
  "ArgumentSampler\0" // IR(52546, 16)
  "ArgumentWorkgroup\0" // IR(52562, 18)
  "SpecConstantWorkgroupSize\0" // IR(52580, 26)
  "SpecConstantGlobalOffset\0" // IR(52606, 25)
  "SpecConstantWorkDim\0" // IR(52631, 20)
  "PushConstantGlobalOffset\0" // IR(52651, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(52676, 30)
  "PushConstantGlobalSize\0" // IR(52706, 23)
  "PushConstantRegionOffset\0" // IR(52729, 25)
  "PushConstantNumWorkgroups\0" // IR(52754, 26)
  "PushConstantRegionGroupOffset\0" // IR(52780, 30)
  "ConstantDataStorageBuffer\0" // IR(52810, 26)
  "ConstantDataUniform\0" // IR(52836, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(52856, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(52886, 28)
  "ArgumentPointerPushConstant\0" // IR(52914, 28)
  "ArgumentPointerUniform\0" // IR(52942, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(52965, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(53000, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(53038, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(53080, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(53125, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(53162, 40)
  "ArgumentStorageTexelBuffer\0" // IR(53202, 27)
  "ArgumentUniformTexelBuffer\0" // IR(53229, 27)
  "ConstantDataPointerPushConstant\0" // IR(53256, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(53288, 40)
  "PrintfInfo\0" // IR(53328, 11)
  "PrintfBufferStorageBuffer\0" // IR(53339, 26)
  "PrintfBufferPointerPushConstant\0" // IR(53365, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(53397, 34)
  "WorkgroupVariableSize\0" // IR(53431, 22)
  "DebugImportedEntity\0" // IR(53453, 20)
  "DebugSource\0" // IR(53473, 12)
  "DebugFunctionDefinition\0" // IR(53485, 24)
  "DebugSourceContinued\0" // IR(53509, 21)
  "DebugLine\0" // IR(53530, 10)
  "DebugNoLine\0" // IR(53540, 12)
  "DebugBuildIdentifier\0" // IR(53552, 21)
  "DebugStoragePath\0" // IR(53573, 17)
  "DebugEntryPoint\0" // IR(53590, 16)
  "DebugTypeMatrix\0" // IR(53606, 16)
  "Configuration\0" // IR(53622, 14)
  "StartCounter\0" // IR(53636, 13)
  "StopCounter\0" // IR(53649, 12)
  "PushConstants\0" // IR(53661, 14)
  "SpecializationMapEntry\0" // IR(53675, 23)
  "DescriptorSetBuffer\0" // IR(53698, 20)
  "DescriptorSetImage\0" // IR(53718, 19)
  "DescriptorSetSampler\0" // IR(53737, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(53758, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(53821, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(53866, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(53915, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(53964, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(54008, 50)
  "DebugModuleINTEL\0" // IR(54058, 17)
  "acos\0" // IR(54075, 5)
  "acosh\0" // IR(54080, 6)
  "acospi\0" // IR(54086, 7)
  "asin\0" // IR(54093, 5)
  "asinh\0" // IR(54098, 6)
  "asinpi\0" // IR(54104, 7)
  "atan\0" // IR(54111, 5)
  "atan2\0" // IR(54116, 6)
  "atanh\0" // IR(54122, 6)
  "atanpi\0" // IR(54128, 7)
  "atan2pi\0" // IR(54135, 8)
  "cbrt\0" // IR(54143, 5)
  "ceil\0" // IR(54148, 5)
  "copysign\0" // IR(54153, 9)
  "cos\0" // IR(54162, 4)
  "cosh\0" // IR(54166, 5)
  "cospi\0" // IR(54171, 6)
  "erfc\0" // IR(54177, 5)
  "erf\0" // IR(54182, 4)
  "exp\0" // IR(54186, 4)
  "exp2\0" // IR(54190, 5)
  "exp10\0" // IR(54195, 6)
  "expm1\0" // IR(54201, 6)
  "fabs\0" // IR(54207, 5)
  "fdim\0" // IR(54212, 5)
  "floor\0" // IR(54217, 6)
  "fma\0" // IR(54223, 4)
  "fmax\0" // IR(54227, 5)
  "fmin\0" // IR(54232, 5)
  "fmod\0" // IR(54237, 5)
  "fract\0" // IR(54242, 6)
  "frexp\0" // IR(54248, 6)
  "hypot\0" // IR(54254, 6)
  "ilogb\0" // IR(54260, 6)
  "ldexp\0" // IR(54266, 6)
  "lgamma\0" // IR(54272, 7)
  "lgamma_r\0" // IR(54279, 9)
  "log\0" // IR(54288, 4)
  "log2\0" // IR(54292, 5)
  "log10\0" // IR(54297, 6)
  "log1p\0" // IR(54303, 6)
  "logb\0" // IR(54309, 5)
  "mad\0" // IR(54314, 4)
  "maxmag\0" // IR(54318, 7)
  "minmag\0" // IR(54325, 7)
  "modf\0" // IR(54332, 5)
  "nan\0" // IR(54337, 4)
  "nextafter\0" // IR(54341, 10)
  "pow\0" // IR(54351, 4)
  "pown\0" // IR(54355, 5)
  "powr\0" // IR(54360, 5)
  "remainder\0" // IR(54365, 10)
  "remquo\0" // IR(54375, 7)
  "rint\0" // IR(54382, 5)
  "rootn\0" // IR(54387, 6)
  "round\0" // IR(54393, 6)
  "rsqrt\0" // IR(54399, 6)
  "sin\0" // IR(54405, 4)
  "sincos\0" // IR(54409, 7)
  "sinh\0" // IR(54416, 5)
  "sinpi\0" // IR(54421, 6)
  "sqrt\0" // IR(54427, 5)
  "tan\0" // IR(54432, 4)
  "tanh\0" // IR(54436, 5)
  "tanpi\0" // IR(54441, 6)
  "tgamma\0" // IR(54447, 7)
  "trunc\0" // IR(54454, 6)
  "half_cos\0" // IR(54460, 9)
  "half_divide\0" // IR(54469, 12)
  "half_exp\0" // IR(54481, 9)
  "half_exp2\0" // IR(54490, 10)
  "half_exp10\0" // IR(54500, 11)
  "half_log\0" // IR(54511, 9)
  "half_log2\0" // IR(54520, 10)
  "half_log10\0" // IR(54530, 11)
  "half_powr\0" // IR(54541, 10)
  "half_recip\0" // IR(54551, 11)
  "half_rsqrt\0" // IR(54562, 11)
  "half_sin\0" // IR(54573, 9)
  "half_sqrt\0" // IR(54582, 10)
  "half_tan\0" // IR(54592, 9)
  "native_cos\0" // IR(54601, 11)
  "native_divide\0" // IR(54612, 14)
  "native_exp\0" // IR(54626, 11)
  "native_exp2\0" // IR(54637, 12)
  "native_exp10\0" // IR(54649, 13)
  "native_log\0" // IR(54662, 11)
  "native_log2\0" // IR(54673, 12)
  "native_log10\0" // IR(54685, 13)
  "native_powr\0" // IR(54698, 12)
  "native_recip\0" // IR(54710, 13)
  "native_rsqrt\0" // IR(54723, 13)
  "native_sin\0" // IR(54736, 11)
  "native_sqrt\0" // IR(54747, 12)
  "native_tan\0" // IR(54759, 11)
  "fclamp\0" // IR(54770, 7)
  "degrees\0" // IR(54777, 8)
  "fmax_common\0" // IR(54785, 12)
  "fmin_common\0" // IR(54797, 12)
  "mix\0" // IR(54809, 4)
  "radians\0" // IR(54813, 8)
  "step\0" // IR(54821, 5)
  "smoothstep\0" // IR(54826, 11)
  "sign\0" // IR(54837, 5)
  "cross\0" // IR(54842, 6)
  "distance\0" // IR(54848, 9)
  "length\0" // IR(54857, 7)
  "normalize\0" // IR(54864, 10)
  "fast_distance\0" // IR(54874, 14)
  "fast_length\0" // IR(54888, 12)
  "fast_normalize\0" // IR(54900, 15)
  "s_abs\0" // IR(54915, 6)
  "s_abs_diff\0" // IR(54921, 11)
  "s_add_sat\0" // IR(54932, 10)
  "u_add_sat\0" // IR(54942, 10)
  "s_hadd\0" // IR(54952, 7)
  "u_hadd\0" // IR(54959, 7)
  "s_rhadd\0" // IR(54966, 8)
  "u_rhadd\0" // IR(54974, 8)
  "s_clamp\0" // IR(54982, 8)
  "u_clamp\0" // IR(54990, 8)
  "clz\0" // IR(54998, 4)
  "ctz\0" // IR(55002, 4)
  "s_mad_hi\0" // IR(55006, 9)
  "u_mad_sat\0" // IR(55015, 10)
  "s_mad_sat\0" // IR(55025, 10)
  "s_max\0" // IR(55035, 6)
  "u_max\0" // IR(55041, 6)
  "s_min\0" // IR(55047, 6)
  "u_min\0" // IR(55053, 6)
  "s_mul_hi\0" // IR(55059, 9)
  "rotate\0" // IR(55068, 7)
  "s_sub_sat\0" // IR(55075, 10)
  "u_sub_sat\0" // IR(55085, 10)
  "u_upsample\0" // IR(55095, 11)
  "s_upsample\0" // IR(55106, 11)
  "popcount\0" // IR(55117, 9)
  "s_mad24\0" // IR(55126, 8)
  "u_mad24\0" // IR(55134, 8)
  "s_mul24\0" // IR(55142, 8)
  "u_mul24\0" // IR(55150, 8)
  "vloadn\0" // IR(55158, 7)
  "vstoren\0" // IR(55165, 8)
  "vload_half\0" // IR(55173, 11)
  "vload_halfn\0" // IR(55184, 12)
  "vstore_half\0" // IR(55196, 12)
  "vstore_half_r\0" // IR(55208, 14)
  "vstore_halfn\0" // IR(55222, 13)
  "vstore_halfn_r\0" // IR(55235, 15)
  "vloada_halfn\0" // IR(55250, 13)
  "vstorea_halfn\0" // IR(55263, 14)
  "vstorea_halfn_r\0" // IR(55277, 16)
  "shuffle\0" // IR(55293, 8)
  "shuffle2\0" // IR(55301, 9)
  "printf\0" // IR(55310, 7)
  "prefetch\0" // IR(55317, 9)
  "bitselect\0" // IR(55326, 10)
  "select\0" // IR(55336, 7)
  "u_abs\0" // IR(55343, 6)
  "u_abs_diff\0" // IR(55349, 11)
  "u_mul_hi\0" // IR(55360, 9)
  "u_mad_hi\0" // IR(55369, 9)
  "CubeFaceIndexAMD\0" // IR(55378, 17)
  "CubeFaceCoordAMD\0" // IR(55395, 17)
  "TimeAMD\0" // IR(55412, 8)
  "SwizzleInvocationsAMD\0" // IR(55420, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(55442, 28)
  "WriteInvocationAMD\0" // IR(55470, 19)
  "MbcntAMD\0" // IR(55489, 9)
  "InterpolateAtVertexAMD\0" // IR(55498, 23)
  "FMin3AMD\0" // IR(55521, 9)
  "UMin3AMD\0" // IR(55530, 9)
  "SMin3AMD\0" // IR(55539, 9)
  "FMax3AMD\0" // IR(55548, 9)
  "UMax3AMD\0" // IR(55557, 9)
  "SMax3AMD\0" // IR(55566, 9)
  "FMid3AMD\0" // IR(55575, 9)
  "UMid3AMD\0" // IR(55584, 9)
  "SMid3AMD\0" // IR(55593, 9)
  "ARGMAX\0" // IR(55602, 7)
  "AVG_POOL2D\0" // IR(55609, 11)
  "CONV2D\0" // IR(55620, 7)
  "CONV3D\0" // IR(55627, 7)
  "DEPTHWISE_CONV2D\0" // IR(55634, 17)
  "FFT2D\0" // IR(55651, 6)
  "MATMUL\0" // IR(55657, 7)
  "MAX_POOL2D\0" // IR(55664, 11)
  "RFFT2D\0" // IR(55675, 7)
  "TRANSPOSE_CONV2D\0" // IR(55682, 17)
  "CLAMP\0" // IR(55699, 6)
  "ERF\0" // IR(55705, 4)
  "SIGMOID\0" // IR(55709, 8)
  "TANH\0" // IR(55717, 5)
  "ADD\0" // IR(55722, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(55726, 23)
  "BITWISE_AND\0" // IR(55749, 12)
  "BITWISE_OR\0" // IR(55761, 11)
  "BITWISE_XOR\0" // IR(55772, 12)
  "INTDIV\0" // IR(55784, 7)
  "LOGICAL_AND\0" // IR(55791, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(55803, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(55822, 20)
  "LOGICAL_OR\0" // IR(55842, 11)
  "LOGICAL_XOR\0" // IR(55853, 12)
  "MAXIMUM\0" // IR(55865, 8)
  "MINIMUM\0" // IR(55873, 8)
  "MUL\0" // IR(55881, 4)
  "POW\0" // IR(55885, 4)
  "SUB\0" // IR(55889, 4)
  "TABLE\0" // IR(55893, 6)
  "ABS\0" // IR(55899, 4)
  "BITWISE_NOT\0" // IR(55903, 12)
  "CEIL\0" // IR(55915, 5)
  "CLZ\0" // IR(55920, 4)
  "COS\0" // IR(55924, 4)
  "EXP\0" // IR(55928, 4)
  "FLOOR\0" // IR(55932, 6)
  "LOG\0" // IR(55938, 4)
  "LOGICAL_NOT\0" // IR(55942, 12)
  "NEGATE\0" // IR(55954, 7)
  "RECIPROCAL\0" // IR(55961, 11)
  "RSQRT\0" // IR(55972, 6)
  "SIN\0" // IR(55978, 4)
  "SELECT\0" // IR(55982, 7)
  "EQUAL\0" // IR(55989, 6)
  "GREATER\0" // IR(55995, 8)
  "GREATER_EQUAL\0" // IR(56003, 14)
  "REDUCE_ALL\0" // IR(56017, 11)
  "REDUCE_ANY\0" // IR(56028, 11)
  "REDUCE_MAX\0" // IR(56039, 11)
  "REDUCE_MIN\0" // IR(56050, 11)
  "REDUCE_PRODUCT\0" // IR(56061, 15)
  "REDUCE_SUM\0" // IR(56076, 11)
  "CONCAT\0" // IR(56087, 7)
  "PAD\0" // IR(56094, 4)
  "RESHAPE\0" // IR(56098, 8)
  "REVERSE\0" // IR(56106, 8)
  "SLICE\0" // IR(56114, 6)
  "TILE\0" // IR(56120, 5)
  "TRANSPOSE\0" // IR(56125, 10)
  "GATHER\0" // IR(56135, 7)
  "SCATTER\0" // IR(56142, 8)
  "RESIZE\0" // IR(56150, 7)
  "CAST\0" // IR(56157, 5)
  "RESCALE\0" // IR(56162, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(5610, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6521, 18), // 1 SubgroupEqMaskKHR
  IR(6576, 18), // 2 SubgroupGeMaskKHR
  IR(6609, 18), // 3 SubgroupGtMaskKHR
  IR(6642, 18), // 4 SubgroupLeMaskKHR
  IR(6675, 18), // 5 SubgroupLtMaskKHR
  IR(7582, 12), // 6 BaryCoordNV
  IR(7637, 19), // 7 BaryCoordNoPerspNV
  IR(7668, 15), // 8 FragmentSizeNV
  IR(7725, 22), // 9 InvocationsPerPixelNV
  IR(7853, 11), // 10 LaunchIdNV
  IR(7878, 13), // 11 LaunchSizeNV
  IR(7909, 17), // 12 WorldRayOriginNV
  IR(7947, 20), // 13 WorldRayDirectionNV
  IR(7986, 18), // 14 ObjectRayOriginNV
  IR(8026, 21), // 15 ObjectRayDirectionNV
  IR(8058, 10), // 16 RayTminNV
  IR(8079, 10), // 17 RayTmaxNV
  IR(8112, 22), // 18 InstanceCustomIndexNV
  IR(8151, 16), // 19 ObjectToWorldNV
  IR(8184, 16), // 20 WorldToObjectNV
  IR(8218, 10), // 21 HitKindNV
  IR(8449, 19), // 22 IncomingRayFlagsNV
  IR(10225, 28), // 23 StorageUniformBufferBlock16
  IR(10288, 17), // 24 StorageUniform16
  IR(11119, 27), // 25 ShaderViewportIndexLayerNV
  IR(11163, 22), // 26 FragmentBarycentricNV
  IR(11216, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11246, 14), // 28 ShadingRateNV
  IR(11290, 29), // 29 GroupNonUniformPartitionedNV
  IR(11336, 20), // 30 ShaderNonUniformEXT
  IR(11379, 26), // 31 RuntimeDescriptorArrayEXT
  IR(11441, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(11519, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(11600, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(11679, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(11755, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(11831, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(11907, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(11985, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12069, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12156, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12219, 21), // 42 VulkanMemoryModelKHR
  IR(12269, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(12301, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(12367, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(12571, 28), // 46 DemoteToHelperInvocationEXT
  IR(13612, 26), // 47 FPGAMemoryAttributesINTEL
  IR(13691, 32), // 48 ArbitraryPrecisionIntegersINTEL
  IR(13761, 37), // 49 ArbitraryPrecisionFloatingPointINTEL
  IR(13851, 22), // 50 FPGALoopControlsINTEL
  IR(13946, 24), // 51 FPGAMemoryAccessesINTEL
  IR(13998, 27), // 52 FPGAClusterAttributesINTEL
  IR(14040, 14), // 53 LoopFuseINTEL
  IR(14075, 20), // 54 FPGADSPControlINTEL
  IR(14162, 40), // 55 FPGAInvocationPipeliningAttributesINTEL
  IR(14227, 24), // 56 FPGABufferLocationINTEL
  IR(14286, 34), // 57 ArbitraryPrecisionFixedPointINTEL
  IR(14344, 23), // 58 USMStorageClassesINTEL
  IR(14397, 29), // 59 RuntimeAlignedAttributeINTEL
  IR(14440, 13), // 60 IOPipesINTEL
  IR(14473, 19), // 61 BlockingPipesINTEL
  IR(14506, 13), // 62 FPGARegINTEL
  IR(14538, 22), // 63 DotProductInputAllKHR
  IR(14582, 25), // 64 DotProductInput4x8BitKHR
  IR(14635, 31), // 65 DotProductInput4x8BitPackedKHR
  IR(14677, 14), // 66 DotProductKHR
  IR(14849, 13), // 67 OptNoneINTEL
  IR(14994, 29), // 68 FPGAClusterAttributesV2INTEL
  IR(15070, 18), // 69 TaskSequenceINTEL
  IR(15129, 24), // 70 FPGALatencyControlINTEL
  IR(15182, 28), // 71 FPGAArgumentInterfacesINTEL
  IR(15276, 35), // 72 GlobalVariableFPGADecorationsINTEL
  IR(18213, 15), // 73 PerPrimitiveNV
  IR(18261, 12), // 74 PerVertexNV
  IR(18284, 14), // 75 NonUniformEXT
  IR(18314, 19), // 76 RestrictPointerEXT
  IR(18348, 18), // 77 AliasedPointerEXT
  IR(18711, 24), // 78 HlslCounterBufferGOOGLE
  IR(18748, 19), // 79 HlslSemanticGOOGLE
  IR(18878, 14), // 80 RegisterINTEL
  IR(18905, 12), // 81 MemoryINTEL
  IR(18932, 14), // 82 NumbanksINTEL
  IR(18962, 15), // 83 BankwidthINTEL
  IR(19000, 22), // 84 MaxPrivateCopiesINTEL
  IR(19039, 16), // 85 SinglepumpINTEL
  IR(19072, 16), // 86 DoublepumpINTEL
  IR(19108, 19), // 87 MaxReplicatesINTEL
  IR(19148, 20), // 88 SimpleDualPortINTEL
  IR(19180, 11), // 89 MergeINTEL
  IR(19248, 14), // 90 BankBitsINTEL
  IR(19283, 20), // 91 ForcePow2DepthINTEL
  IR(19320, 16), // 92 StridesizeINTEL
  IR(19351, 14), // 93 WordsizeINTEL
  IR(19384, 18), // 94 TrueDualPortINTEL
  IR(19422, 19), // 95 BurstCoalesceINTEL
  IR(19457, 15), // 96 CacheSizeINTEL
  IR(19501, 28), // 97 DontStaticallyCoalesceINTEL
  IR(19544, 14), // 98 PrefetchINTEL
  IR(19576, 17), // 99 StallEnableINTEL
  IR(19619, 25), // 100 FuseLoopsInFunctionINTEL
  IR(19664, 19), // 101 MathOpDSPModeINTEL
  IR(19737, 24), // 102 InitiationIntervalINTEL
  IR(19782, 20), // 103 MaxConcurrencyINTEL
  IR(19823, 20), // 104 PipelineEnableINTEL
  IR(19864, 20), // 105 BufferLocationINTEL
  IR(19904, 19), // 106 IOPipeStorageINTEL
  IR(20082, 15), // 107 StallFreeINTEL
  IR(20180, 25), // 108 LatencyControlLabelINTEL
  IR(20236, 30), // 109 LatencyControlConstraintINTEL
  IR(20294, 27), // 110 ConduitKernelArgumentINTEL
  IR(20353, 31), // 111 RegisterMapKernelArgumentINTEL
  IR(20418, 33), // 112 MMHostInterfaceAddressWidthINTEL
  IR(20482, 30), // 113 MMHostInterfaceDataWidthINTEL
  IR(20541, 28), // 114 MMHostInterfaceLatencyINTEL
  IR(20638, 34), // 115 MMHostInterfaceReadWriteModeINTEL
  IR(20702, 29), // 116 MMHostInterfaceMaxBurstINTEL
  IR(20764, 32), // 117 MMHostInterfaceWaitRequestINTEL
  IR(20823, 26), // 118 StableKernelArgumentINTEL
  IR(20966, 14), // 119 InitModeINTEL
  IR(21009, 28), // 120 ImplementInRegisterMapINTEL
  IR(22363, 14), // 121 OutputLinesNV
  IR(22397, 19), // 122 OutputPrimitivesNV
  IR(22440, 23), // 123 DerivativeGroupQuadsNV
  IR(22488, 24), // 124 DerivativeGroupLinearNV
  IR(22531, 18), // 125 OutputTrianglesNV
  IR(23279, 16), // 126 RayGenerationNV
  IR(23311, 15), // 127 IntersectionNV
  IR(23336, 9), // 128 AnyHitNV
  IR(23359, 13), // 129 ClosestHitNV
  IR(23380, 7), // 130 MissNV
  IR(23399, 11), // 131 CallableNV
  IR(23544, 23), // 132 AllowContractFastINTEL
  IR(23580, 18), // 133 AllowReassocINTEL
  IR(23806, 20), // 134 RuntimeAlignedINTEL
  IR(23898, 20), // 135 PartitionedReduceNV
  IR(23946, 27), // 136 PartitionedInclusiveScanNV
  IR(24001, 27), // 137 PartitionedExclusiveScanNV
  IR(24132, 22), // 138 MakeTexelAvailableKHR
  IR(24171, 20), // 139 MakeTexelVisibleKHR
  IR(24207, 19), // 140 NonPrivateTexelKHR
  IR(24240, 17), // 141 VolatileTexelKHR
  IR(24799, 27), // 142 InitOnDeviceReprogramINTEL
  IR(24850, 23), // 143 InitOnDeviceResetINTEL
  IR(25184, 21), // 144 DependencyArrayINTEL
  IR(25224, 18), // 145 LoopCoalesceINTEL
  IR(25264, 21), // 146 MaxInterleavingINTEL
  IR(25312, 26), // 147 SpeculatedIterationsINTEL
  IR(25353, 14), // 148 NoFusionINTEL
  IR(25383, 15), // 149 LoopCountINTEL
  IR(25425, 26), // 150 MaxReinvocationDelayINTEL
  IR(25817, 24), // 151 MakePointerAvailableKHR
  IR(25860, 22), // 152 MakePointerVisibleKHR
  IR(25900, 21), // 153 NonPrivatePointerKHR
  IR(25987, 10), // 154 VulkanKHR
  IR(15956, 5), // 155 None
  IR(26170, 16), // 156 OutputMemoryKHR
  IR(26200, 17), // 157 MakeAvailableKHR
  IR(26229, 15), // 158 MakeVisibleKHR
  IR(26305, 28), // 159 PackedVectorFormat4x8BitKHR
  IR(26620, 24), // 160 SkipBuiltinPrimitivesNV
  IR(27360, 15), // 161 QueueFamilyKHR
  IR(27731, 15), // 162 CallableDataNV
  IR(27770, 23), // 163 IncomingCallableDataNV
  IR(27807, 13), // 164 RayPayloadNV
  IR(27836, 15), // 165 HitAttributeNV
  IR(27873, 21), // 166 IncomingRayPayloadNV
  IR(27916, 21), // 167 ShaderRecordBufferNV
  IR(27959, 25), // 168 PhysicalStorageBufferEXT
  IR(28085, 16), // 169 DeviceOnlyINTEL
  IR(28116, 14), // 170 HostOnlyINTEL
  IR(35354, 8), // 171 SDotKHR
  IR(35367, 8), // 172 UDotKHR
  IR(35381, 9), // 173 SUDotKHR
  IR(35401, 14), // 174 SDotAccSatKHR
  IR(35426, 14), // 175 UDotAccSatKHR
  IR(35452, 15), // 176 SUDotAccSatKHR
  IR(37733, 24), // 177 TypeCooperativeVectorNV
  IR(37989, 27), // 178 GroupNonUniformPartitionNV
  IR(39071, 21), // 179 ReportIntersectionNV
  IR(39246, 28), // 180 TypeAccelerationStructureNV
  IR(39327, 23), // 181 RayQueryGetClusterIdNV
  IR(41495, 21), // 182 DecorateStringGOOGLE
  IR(41537, 27), // 183 MemberDecorateStringGOOGLE
  IR(46794, 28), // 184 ArbitraryFloatSinCosPiINTEL
  IR(46847, 24), // 185 ArbitraryFloatCastINTEL
  IR(46903, 31), // 186 ArbitraryFloatCastFromIntINTEL
  IR(46964, 29), // 187 ArbitraryFloatCastToIntINTEL
  IR(47017, 23), // 188 ArbitraryFloatAddINTEL
  IR(47064, 23), // 189 ArbitraryFloatSubINTEL
  IR(47111, 23), // 190 ArbitraryFloatMulINTEL
  IR(47158, 23), // 191 ArbitraryFloatDivINTEL
  IR(47204, 22), // 192 ArbitraryFloatGTINTEL
  IR(47249, 22), // 193 ArbitraryFloatGEINTEL
  IR(47294, 22), // 194 ArbitraryFloatLTINTEL
  IR(47339, 22), // 195 ArbitraryFloatLEINTEL
  IR(47384, 22), // 196 ArbitraryFloatEQINTEL
  IR(47432, 25), // 197 ArbitraryFloatRecipINTEL
  IR(47483, 25), // 198 ArbitraryFloatRSqrtINTEL
  IR(47533, 24), // 199 ArbitraryFloatCbrtINTEL
  IR(47583, 25), // 200 ArbitraryFloatHypotINTEL
  IR(47633, 24), // 201 ArbitraryFloatSqrtINTEL
  IR(48317, 15), // 202 FixedSqrtINTEL
  IR(48349, 16), // 203 FixedRecipINTEL
  IR(48382, 16), // 204 FixedRsqrtINTEL
  IR(48413, 14), // 205 FixedSinINTEL
  IR(48442, 14), // 206 FixedCosINTEL
  IR(48474, 17), // 207 FixedSinCosINTEL
  IR(48508, 16), // 208 FixedSinPiINTEL
  IR(48541, 16), // 209 FixedCosPiINTEL
  IR(48577, 19), // 210 FixedSinCosPiINTEL
  IR(48611, 14), // 211 FixedLogINTEL
  IR(48640, 14), // 212 FixedExpINTEL
  IR(48684, 29), // 213 PtrCastToCrossWorkgroupINTEL
  IR(48743, 29), // 214 CrossWorkgroupCastToPtrINTEL
  IR(48795, 22), // 215 ReadPipeBlockingINTEL
  IR(48841, 23), // 216 WritePipeBlockingINTEL
  IR(49793, 24), // 217 TaskSequenceCreateINTEL
  IR(49841, 23), // 218 TaskSequenceAsyncINTEL
  IR(49886, 21), // 219 TaskSequenceGetINTEL
  IR(49933, 25), // 220 TaskSequenceReleaseINTEL
  IR(49981, 22), // 221 TypeTaskSequenceINTEL
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
  spv::Capability::DescriptorHeapEXT, // 44
  spv::Capability::ShaderViewportMaskNV, // 45
  spv::Capability::MeshShadingNV, // 46
  spv::Capability::ShaderStereoViewNV, // 47
  spv::Capability::PerViewAttributesNV, // 48
  spv::Capability::MeshShadingNV, // 49
  spv::Capability::FragmentFullyCoveredEXT, // 50
  spv::Capability::MeshShadingNV, // 51
  spv::Capability::FragmentBarycentricKHR, // 52
  spv::Capability::FragmentDensityEXT, // 53
  spv::Capability::MeshShadingEXT, // 54
  spv::Capability::RayTracingNV, // 55
  spv::Capability::RayTracingKHR, // 56
  spv::Capability::RayTracingNV, // 57
  spv::Capability::RayTracingMotionBlurNV, // 58
  spv::Capability::RayTracingPositionFetchKHR, // 59
  spv::Capability::RayTracingDisplacementMicromapNV, // 60
  spv::Capability::RayTracingKHR, // 61
  spv::Capability::RayTracingSpheresGeometryNV, // 62
  spv::Capability::RayTracingLinearSweptSpheresGeometryNV, // 63
  spv::Capability::ShaderSMBuiltinsNV, // 64
  spv::Capability::RayTracingClusterAccelerationStructureNV, // 65
  spv::Capability::RayCullMaskKHR, // 66
  spv::Capability::Matrix, // 67
  spv::Capability::Int64, // 68
  spv::Capability::ImageBasic, // 69
  spv::Capability::Geometry, // 70
  spv::Capability::SampledCubeArray, // 71
  spv::Capability::SampledRect, // 72
  spv::Capability::Sampled1D, // 73
  spv::Capability::SampledBuffer, // 74
  spv::Capability::DeviceEnqueue, // 75
  spv::Capability::Pipes, // 76
  spv::Capability::GroupNonUniform, // 77
  spv::Capability::Float8EXT, // 78
  spv::Capability::CooperativeMatrixKHR, // 79
  spv::Capability::WorkgroupMemoryExplicitLayoutKHR, // 80
  spv::Capability::StorageBuffer16BitAccess, // 81
  spv::Capability::VariablePointersStorageBuffer, // 82
  spv::Capability::AtomicStorage, // 83
  spv::Capability::StorageBuffer8BitAccess, // 84
  spv::Capability::RayQueryKHR, // 85
  spv::Capability::RayTracingKHR, // 86
  spv::Capability::CooperativeMatrixKHR, // 87
  spv::Capability::Int4TypeINTEL, // 88
  spv::Capability::CooperativeMatrixKHR, // 89
  spv::Capability::BFloat16TypeKHR, // 90
  spv::Capability::BFloat16TypeKHR, // 91
  spv::Capability::CooperativeMatrixKHR, // 92
  spv::Capability::UntypedPointersKHR, // 93
  spv::Capability::MultiViewport, // 94
  spv::Capability::ShaderViewportIndexLayerEXT, // 95
  spv::Capability::ShaderViewportMaskNV, // 96
  spv::Capability::InputAttachment, // 97
  spv::Capability::ImageBuffer, // 98
  spv::Capability::ShaderNonUniform, // 99
  spv::Capability::InputAttachment, // 100
  spv::Capability::ShaderNonUniform, // 101
  spv::Capability::SampledBuffer, // 102
  spv::Capability::ShaderNonUniform, // 103
  spv::Capability::ImageBuffer, // 104
  spv::Capability::ShaderNonUniform, // 105
  spv::Capability::VectorAnyINTEL, // 106
  spv::Capability::Int8, // 107
  spv::Capability::FPGAClusterAttributesALTERA, // 108
  spv::Capability::FPGAKernelAttributesINTEL, // 109
  spv::Capability::Subgroup2DBlockIOINTEL, // 110
  spv::Capability::VariableLengthArrayINTEL, // 111
  spv::Capability::UntypedPointersKHR, // 112
  spv::Capability::SpecConditionalINTEL, // 113
  spv::Capability::Float16, // 114
  spv::Capability::Float8EXT, // 115
  spv::Capability::CooperativeMatrixLayoutsARM, // 116
  spv::Capability::Shader, // 117
  spv::Capability::Kernel, // 118
  spv::Capability::Shader, // 119
  spv::Capability::UniformDecoration, // 120
  spv::Capability::GeometryStreams, // 121
  spv::Capability::TransformFeedback, // 122
  spv::Capability::Kernel, // 123
  spv::Capability::FloatControls2, // 124
  spv::Capability::Linkage, // 125
  spv::Capability::SampleMaskOverrideCoverageNV, // 126
  spv::Capability::GeometryShaderPassthroughNV, // 127
  spv::Capability::MeshShadingNV, // 128
  spv::Capability::MeshShadingEXT, // 129
  spv::Capability::PushConstantBanksNV, // 130
  spv::Capability::ShaderInvocationReorderNV, // 131
  spv::Capability::ShaderInvocationReorderEXT, // 132
  spv::Capability::BindlessTextureNV, // 133
  spv::Capability::VectorComputeINTEL, // 134
  spv::Capability::IndirectReferencesINTEL, // 135
  spv::Capability::AsmINTEL, // 136
  spv::Capability::FunctionFloatControlINTEL, // 137
  spv::Capability::FPGAMemoryAttributesALTERA, // 138
  spv::Capability::FPGAMemoryAccessesALTERA, // 139
  spv::Capability::LoopFuseALTERA, // 140
  spv::Capability::FPGADSPControlALTERA, // 141
  spv::Capability::MemoryAccessAliasingINTEL, // 142
  spv::Capability::FPGAInvocationPipeliningAttributesALTERA, // 143
  spv::Capability::FPGABufferLocationALTERA, // 144
  spv::Capability::IOPipesALTERA, // 145
  spv::Capability::FPGAClusterAttributesV2ALTERA, // 146
  spv::Capability::FPMaxErrorINTEL, // 147
  spv::Capability::FPGALatencyControlALTERA, // 148
  spv::Capability::FPGAArgumentInterfacesALTERA, // 149
  spv::Capability::GlobalVariableHostAccessINTEL, // 150
  spv::Capability::GlobalVariableFPGADecorationsALTERA, // 151
  spv::Capability::CacheControlsINTEL, // 152
  spv::Capability::TileImageColorReadAccessEXT, // 153
  spv::Capability::Geometry, // 154
  spv::Capability::Tessellation, // 155
  spv::Capability::MeshShadingNV, // 156
  spv::Capability::MeshShadingEXT, // 157
  spv::Capability::Geometry, // 158
  spv::Capability::MeshShadingNV, // 159
  spv::Capability::MeshShadingEXT, // 160
  spv::Capability::SubgroupDispatch, // 161
  spv::Capability::TileImageDepthReadAccessEXT, // 162
  spv::Capability::TileImageStencilReadAccessEXT, // 163
  spv::Capability::SampleMaskPostDepthCoverage, // 164
  spv::Capability::DenormPreserve, // 165
  spv::Capability::DenormFlushToZero, // 166
  spv::Capability::SignedZeroInfNanPreserve, // 167
  spv::Capability::RoundingModeRTE, // 168
  spv::Capability::RoundingModeRTZ, // 169
  spv::Capability::QuadControlKHR, // 170
  spv::Capability::PoisonFreezeKHR, // 171
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 172
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 173
  spv::Capability::FragmentShaderPixelInterlockEXT, // 174
  spv::Capability::FragmentShaderSampleInterlockEXT, // 175
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 176
  spv::Capability::Shader64BitIndexingEXT, // 177
  spv::Capability::RoundToInfinityINTEL, // 178
  spv::Capability::KernelAttributesINTEL, // 179
  spv::Capability::FloatControls2, // 180
  spv::Capability::FPGAKernelAttributesv2INTEL, // 181
  spv::Capability::RegisterLimitsINTEL, // 182
  spv::Capability::FloatControls2, // 183
  spv::Capability::FPFastMathModeINTEL, // 184
  spv::Capability::OptNoneEXT, // 185
  spv::Capability::RuntimeAlignedAttributeALTERA, // 186
  spv::Capability::Kernel, // 187
  spv::Capability::GroupNonUniformArithmetic, // 188
  spv::Capability::GroupNonUniformBallot, // 189
  spv::Capability::GroupNonUniformClustered, // 190
  spv::Capability::GroupNonUniformPartitionedEXT, // 191
  spv::Capability::ImageGatherExtended, // 192
  spv::Capability::MinLod, // 193
  spv::Capability::VulkanMemoryModel, // 194
  spv::Capability::FPGALoopControlsALTERA, // 195
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 196
  spv::Capability::RawAccessChainsNV, // 197
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 198
  spv::Capability::RayTracingOpacityMicromapEXT, // 199
  spv::Capability::RayQueryKHR, // 200
  spv::Capability::StorageImageExtendedFormats, // 201
  spv::Capability::Int64ImageEXT, // 202
  spv::Capability::Shader, // 203
  spv::Capability::VectorComputeINTEL, // 204
  spv::Capability::GenericPointer, // 205
  spv::Capability::FunctionPointersINTEL, // 206
  spv::Capability::USMStorageClassesALTERA, // 207
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 208
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 209
  spv::Capability::TensorsARM, // 210
  spv::Capability::Addresses, // 211
  spv::Capability::PhysicalStorageBufferAddresses, // 212
  spv::Capability::LiteralSampler, // 213
  spv::Capability::Addresses, // 214
  spv::Capability::UntypedPointersKHR, // 215
  spv::Capability::Addresses, // 216
  spv::Capability::VariablePointers, // 217
  spv::Capability::VariablePointersStorageBuffer, // 218
  spv::Capability::PhysicalStorageBufferAddresses, // 219
  spv::Capability::Kernel, // 220
  spv::Capability::ImageQuery, // 221
  spv::Capability::ImageQuery, // 222
  spv::Capability::Shader, // 223
  spv::Capability::BitInstructions, // 224
  spv::Capability::DerivativeControl, // 225
  spv::Capability::Groups, // 226
  spv::Capability::SparseResidency, // 227
  spv::Capability::PipeStorage, // 228
  spv::Capability::NamedBarrier, // 229
  spv::Capability::GroupNonUniformVote, // 230
  spv::Capability::GroupNonUniformBallot, // 231
  spv::Capability::GroupNonUniformShuffle, // 232
  spv::Capability::GroupNonUniformShuffleRelative, // 233
  spv::Capability::GroupNonUniformArithmetic, // 234
  spv::Capability::GroupNonUniformClustered, // 235
  spv::Capability::GroupNonUniformPartitionedEXT, // 236
  spv::Capability::GroupNonUniformQuad, // 237
  spv::Capability::Addresses, // 238
  spv::Capability::VariablePointers, // 239
  spv::Capability::VariablePointersStorageBuffer, // 240
  spv::Capability::GraphARM, // 241
  spv::Capability::SubgroupBallotKHR, // 242
  spv::Capability::FMAKHR, // 243
  spv::Capability::SubgroupVoteKHR, // 244
  spv::Capability::GroupNonUniformRotateKHR, // 245
  spv::Capability::RayTracingKHR, // 246
  spv::Capability::RayQueryKHR, // 247
  spv::Capability::DotProduct, // 248
  spv::Capability::ReplicatedCompositesEXT, // 249
  spv::Capability::TextureSampleWeightedQCOM, // 250
  spv::Capability::TextureBoxFilterQCOM, // 251
  spv::Capability::TextureBlockMatchQCOM, // 252
  spv::Capability::CooperativeMatrixConversionQCOM, // 253
  spv::Capability::TextureBlockMatch2QCOM, // 254
  spv::Capability::FragmentMaskAMD, // 255
  spv::Capability::ShaderClockKHR, // 256
  spv::Capability::ShaderInvocationReorderNV, // 257
  spv::Capability::RayTracingMotionBlurNV, // 258
  spv::Capability::ImageFootprintNV, // 259
  spv::Capability::CooperativeVectorNV, // 260
  spv::Capability::LongVectorEXT, // 261
  spv::Capability::CooperativeVectorNV, // 262
  spv::Capability::CooperativeVectorTrainingNV, // 263
  spv::Capability::CooperativeMatrixConversionsNV, // 264
  spv::Capability::DisplacementMicromapNV, // 265
  spv::Capability::ShaderInvocationReorderEXT, // 266
  spv::Capability::RayTracingMotionBlurNV, // 267
  spv::Capability::RayQueryPositionFetchKHR, // 268
  spv::Capability::RayTracingNV, // 269
  spv::Capability::RayTracingKHR, // 270
  spv::Capability::RayQueryKHR, // 271
  spv::Capability::DisplacementMicromapNV, // 272
  spv::Capability::CooperativeMatrixNV, // 273
  spv::Capability::FragmentShaderSampleInterlockEXT, // 274
  spv::Capability::FragmentShaderPixelInterlockEXT, // 275
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 276
  spv::Capability::CooperativeMatrixReductionsNV, // 277
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 278
  spv::Capability::TensorAddressingNV, // 279
  spv::Capability::DemoteToHelperInvocation, // 280
  spv::Capability::SubgroupShuffleINTEL, // 281
  spv::Capability::SubgroupBufferBlockIOINTEL, // 282
  spv::Capability::SubgroupImageBlockIOINTEL, // 283
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 284
  spv::Capability::IntegerFunctions2INTEL, // 285
  spv::Capability::AtomicFloat16MinMaxEXT, // 286
  spv::Capability::AtomicFloat32MinMaxEXT, // 287
  spv::Capability::AtomicFloat64MinMaxEXT, // 288
  spv::Capability::AtomicFloat16VectorNV, // 289
  spv::Capability::ExpectAssumeKHR, // 290
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 291
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 292
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 293
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 294
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 295
  spv::Capability::VariableLengthArrayINTEL, // 296
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 297
  spv::Capability::UnstructuredLoopControlsINTEL, // 298
  spv::Capability::BlockingPipesALTERA, // 299
  spv::Capability::FPGARegALTERA, // 300
  spv::Capability::AtomicFloat16AddEXT, // 301
  spv::Capability::AtomicFloat32AddEXT, // 302
  spv::Capability::AtomicFloat64AddEXT, // 303
  spv::Capability::AtomicFloat16VectorNV, // 304
  spv::Capability::LongCompositesINTEL, // 305
  spv::Capability::BFloat16ConversionINTEL, // 306
  spv::Capability::SplitBarrierINTEL, // 307
  spv::Capability::ArithmeticFenceEXT, // 308
  spv::Capability::TaskSequenceALTERA, // 309
  spv::Capability::SubgroupBufferPrefetchINTEL, // 310
  spv::Capability::Subgroup2DBlockTransformINTEL, // 311
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 312
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 313
  spv::Capability::TernaryBitwiseFunctionINTEL, // 314
  spv::Capability::UntypedVariableLengthArrayINTEL, // 315
  spv::Capability::FunctionVariantsINTEL, // 316
  spv::Capability::GroupUniformArithmeticKHR, // 317
  spv::Capability::TensorFloat32RoundingINTEL, // 318
  spv::Capability::MaskedGatherScatterINTEL, // 319
  spv::Capability::BindlessImagesINTEL, // 320
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 321
  spv::Capability::DotProductBFloat16AccVALVE, // 322
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 323
  spv::Capability::DotProductBFloat16AccVALVE, // 324
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 325
  spv::Capability::Float64, // 326
  spv::Capability::InterpolationFunction, // 327
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
  spvtools::Extension::kSPV_EXT_descriptor_heap, // 66
  spvtools::Extension::kSPV_KHR_poison_freeze, // 67
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 68
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 69
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 70
  spvtools::Extension::kSPV_NV_viewport_array2, // 71
  spvtools::Extension::kSPV_NV_viewport_array2, // 72
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 73
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 74
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 75
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 76
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 77
  spvtools::Extension::kSPV_EXT_shader_subgroup_partitioned, // 78
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 79
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 80
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 81
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 82
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 83
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 84
  spvtools::Extension::kSPV_NV_displacement_micromap, // 85
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 86
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 87
  spvtools::Extension::kSPV_EXT_shader_invocation_reorder, // 88
  spvtools::Extension::kSPV_NV_bindless_texture, // 89
  spvtools::Extension::kSPV_NV_cooperative_vector, // 90
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 91
  spvtools::Extension::kSPV_NV_raw_access_chains, // 92
  spvtools::Extension::kSPV_NV_push_constant_bank, // 93
  spvtools::Extension::kSPV_EXT_long_vector, // 94
  spvtools::Extension::kSPV_EXT_shader_64bit_indexing, // 95
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 96
  spvtools::Extension::kSPV_NV_tensor_addressing, // 97
  spvtools::Extension::kSPV_INTEL_subgroups, // 98
  spvtools::Extension::kSPV_INTEL_media_block_io, // 99
  spvtools::Extension::kSPV_INTEL_float_controls2, // 100
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 101
  spvtools::Extension::kSPV_INTEL_function_pointers, // 102
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 103
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 104
  spvtools::Extension::kSPV_INTEL_vector_compute, // 105
  spvtools::Extension::kSPV_KHR_expect_assume, // 106
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 107
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 108
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 109
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 110
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 111
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 112
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 113
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 114
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 115
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 116
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 117
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 118
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 119
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 120
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 121
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 122
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 123
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 124
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 125
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 126
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 127
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 128
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 129
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 130
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 131
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 132
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 133
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 134
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 135
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 136
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 137
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 138
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 139
  spvtools::Extension::kSPV_INTEL_io_pipes, // 140
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 141
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 142
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 143
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 144
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 145
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 146
  spvtools::Extension::kSPV_EXT_replicated_composites, // 147
  spvtools::Extension::kSPV_KHR_bit_instructions, // 148
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 149
  spvtools::Extension::kSPV_KHR_float_controls2, // 150
  spvtools::Extension::kSPV_KHR_fma, // 151
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 152
  spvtools::Extension::kSPV_INTEL_long_composites, // 153
  spvtools::Extension::kSPV_EXT_optnone, // 154
  spvtools::Extension::kSPV_INTEL_optnone, // 155
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 156
  spvtools::Extension::kSPV_INTEL_debug_module, // 157
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 158
  spvtools::Extension::kSPV_INTEL_split_barrier, // 159
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 160
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 161
  spvtools::Extension::kSPV_INTEL_task_sequence, // 162
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 163
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 164
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 165
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 166
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 167
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 168
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 169
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 170
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 171
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 172
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 173
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 174
  spvtools::Extension::kSPV_INTEL_function_variants, // 175
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 176
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 177
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 178
  spvtools::Extension::kSPV_INTEL_cache_controls, // 179
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 180
  spvtools::Extension::kSPV_INTEL_bindless_images, // 181
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 182
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 183
  spvtools::Extension::kSPV_NV_mesh_shader, // 184
  spvtools::Extension::kSPV_EXT_mesh_shader, // 185
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 186
  spvtools::Extension::kSPV_GOOGLE_user_type, // 187
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 188
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 189
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 190
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 191
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 192
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 193
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 194
  spvtools::Extension::kSPV_KHR_variable_pointers, // 195
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 196
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 197
  spvtools::Extension::kSPV_KHR_ray_tracing, // 198
  spvtools::Extension::kSPV_KHR_ray_query, // 199
  spvtools::Extension::kSPV_NV_ray_tracing, // 200
  spvtools::Extension::kSPV_KHR_ray_tracing, // 201
  spvtools::Extension::kSPV_KHR_ray_query, // 202
  spvtools::Extension::kSPV_NV_displacement_micromap, // 203
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 204
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 205
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
  SPV_OPERAND_TYPE_ID, // 590
  SPV_OPERAND_TYPE_ID, // 591
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 592
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 593
  SPV_OPERAND_TYPE_ID, // 594
  SPV_OPERAND_TYPE_ID, // 595
  SPV_OPERAND_TYPE_ID, // 596
  SPV_OPERAND_TYPE_ID, // 597
  SPV_OPERAND_TYPE_ID, // 598
  SPV_OPERAND_TYPE_ID, // 599
  SPV_OPERAND_TYPE_ID, // 600
  SPV_OPERAND_TYPE_ID, // 601
  SPV_OPERAND_TYPE_ID, // 602
  SPV_OPERAND_TYPE_ID, // 603
  SPV_OPERAND_TYPE_ID, // 604
  SPV_OPERAND_TYPE_ID, // 605
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 606
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 607
  SPV_OPERAND_TYPE_ID, // 608
  SPV_OPERAND_TYPE_ID, // 609
  SPV_OPERAND_TYPE_ID, // 610
  SPV_OPERAND_TYPE_ID, // 611
  SPV_OPERAND_TYPE_ID, // 612
  SPV_OPERAND_TYPE_ID, // 613
  SPV_OPERAND_TYPE_ID, // 614
  SPV_OPERAND_TYPE_ID, // 615
  SPV_OPERAND_TYPE_ID, // 616
  SPV_OPERAND_TYPE_ID, // 617
  SPV_OPERAND_TYPE_ID, // 618
  SPV_OPERAND_TYPE_ID, // 619
  SPV_OPERAND_TYPE_ID, // 620
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 621
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 622
  SPV_OPERAND_TYPE_RESULT_ID, // 623
  SPV_OPERAND_TYPE_ID, // 624
  SPV_OPERAND_TYPE_SCOPE_ID, // 625
  SPV_OPERAND_TYPE_ID, // 626
  SPV_OPERAND_TYPE_ID, // 627
  SPV_OPERAND_TYPE_TYPE_ID, // 628
  SPV_OPERAND_TYPE_RESULT_ID, // 629
  SPV_OPERAND_TYPE_ID, // 630
  SPV_OPERAND_TYPE_ID, // 631
  SPV_OPERAND_TYPE_ID, // 632
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 633
  SPV_OPERAND_TYPE_ID, // 634
  SPV_OPERAND_TYPE_ID, // 635
  SPV_OPERAND_TYPE_ID, // 636
  SPV_OPERAND_TYPE_ID, // 637
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 638
  SPV_OPERAND_TYPE_TYPE_ID, // 639
  SPV_OPERAND_TYPE_RESULT_ID, // 640
  SPV_OPERAND_TYPE_ID, // 641
  SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE, // 642
  SPV_OPERAND_TYPE_ID, // 643
  SPV_OPERAND_TYPE_TYPE_ID, // 644
  SPV_OPERAND_TYPE_RESULT_ID, // 645
  SPV_OPERAND_TYPE_ID, // 646
  SPV_OPERAND_TYPE_ID, // 647
  SPV_OPERAND_TYPE_ID, // 648
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 649
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 650
  SPV_OPERAND_TYPE_ID, // 651
  SPV_OPERAND_TYPE_ID, // 652
  SPV_OPERAND_TYPE_ID, // 653
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 654
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 655
  SPV_OPERAND_TYPE_RESULT_ID, // 656
  SPV_OPERAND_TYPE_ID, // 657
  SPV_OPERAND_TYPE_ID, // 658
  SPV_OPERAND_TYPE_VARIABLE_ID, // 659
  SPV_OPERAND_TYPE_TYPE_ID, // 660
  SPV_OPERAND_TYPE_RESULT_ID, // 661
  SPV_OPERAND_TYPE_ID, // 662
  SPV_OPERAND_TYPE_ID, // 663
  SPV_OPERAND_TYPE_ID, // 664
  SPV_OPERAND_TYPE_ID, // 665
  SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS, // 666
  SPV_OPERAND_TYPE_ID, // 667
  SPV_OPERAND_TYPE_ID, // 668
  SPV_OPERAND_TYPE_ID, // 669
  SPV_OPERAND_TYPE_ID, // 670
  SPV_OPERAND_TYPE_ID, // 671
  SPV_OPERAND_TYPE_TYPE_ID, // 672
  SPV_OPERAND_TYPE_RESULT_ID, // 673
  SPV_OPERAND_TYPE_ID, // 674
  SPV_OPERAND_TYPE_ID, // 675
  SPV_OPERAND_TYPE_LITERAL_STRING, // 676
  SPV_OPERAND_TYPE_LITERAL_STRING, // 677
  SPV_OPERAND_TYPE_TYPE_ID, // 678
  SPV_OPERAND_TYPE_RESULT_ID, // 679
  SPV_OPERAND_TYPE_ID, // 680
  SPV_OPERAND_TYPE_ID, // 681
  SPV_OPERAND_TYPE_ID, // 682
  SPV_OPERAND_TYPE_ID, // 683
  SPV_OPERAND_TYPE_ID, // 684
  SPV_OPERAND_TYPE_ID, // 685
  SPV_OPERAND_TYPE_ID, // 686
  SPV_OPERAND_TYPE_TYPE_ID, // 687
  SPV_OPERAND_TYPE_RESULT_ID, // 688
  SPV_OPERAND_TYPE_ID, // 689
  SPV_OPERAND_TYPE_ID, // 690
  SPV_OPERAND_TYPE_ID, // 691
  SPV_OPERAND_TYPE_ID, // 692
  SPV_OPERAND_TYPE_ID, // 693
  SPV_OPERAND_TYPE_ID, // 694
  SPV_OPERAND_TYPE_ID, // 695
  SPV_OPERAND_TYPE_ID, // 696
  SPV_OPERAND_TYPE_TYPE_ID, // 697
  SPV_OPERAND_TYPE_RESULT_ID, // 698
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
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 713
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 714
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 715
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 716
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 717
  SPV_OPERAND_TYPE_TYPE_ID, // 718
  SPV_OPERAND_TYPE_RESULT_ID, // 719
  SPV_OPERAND_TYPE_ID, // 720
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 721
  SPV_OPERAND_TYPE_ID, // 722
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 723
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 724
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 725
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 726
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 727
  SPV_OPERAND_TYPE_TYPE_ID, // 728
  SPV_OPERAND_TYPE_RESULT_ID, // 729
  SPV_OPERAND_TYPE_ID, // 730
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 731
  SPV_OPERAND_TYPE_ID, // 732
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 733
  SPV_OPERAND_TYPE_RESULT_ID, // 734
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 735
  SPV_OPERAND_TYPE_RESULT_ID, // 736
  SPV_OPERAND_TYPE_ID, // 737
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 738
  SPV_OPERAND_TYPE_VARIABLE_ID, // 739
  SPV_OPERAND_TYPE_TYPE_ID, // 740
  SPV_OPERAND_TYPE_RESULT_ID, // 741
  SPV_OPERAND_TYPE_ID, // 742
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 743
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 744
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 745
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 746
  SPV_OPERAND_TYPE_ID, // 747
  SPV_OPERAND_TYPE_ID, // 748
  SPV_OPERAND_TYPE_ID, // 749
  SPV_OPERAND_TYPE_ID, // 750
  SPV_OPERAND_TYPE_ID, // 751
  SPV_OPERAND_TYPE_ID, // 752
  SPV_OPERAND_TYPE_ID, // 753
  SPV_OPERAND_TYPE_ID, // 754
  SPV_OPERAND_TYPE_ID, // 755
  SPV_OPERAND_TYPE_ID, // 756
  SPV_OPERAND_TYPE_ID, // 757
  SPV_OPERAND_TYPE_ID, // 758
  SPV_OPERAND_TYPE_ID, // 759
  SPV_OPERAND_TYPE_ID, // 760
  SPV_OPERAND_TYPE_ID, // 761
  SPV_OPERAND_TYPE_ID, // 762
  SPV_OPERAND_TYPE_ID, // 763
  SPV_OPERAND_TYPE_ID, // 764
  SPV_OPERAND_TYPE_ID, // 765
  SPV_OPERAND_TYPE_TYPE_ID, // 766
  SPV_OPERAND_TYPE_RESULT_ID, // 767
  SPV_OPERAND_TYPE_ID, // 768
  SPV_OPERAND_TYPE_ID, // 769
  SPV_OPERAND_TYPE_ID, // 770
  SPV_OPERAND_TYPE_ID, // 771
  SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS, // 772
  SPV_OPERAND_TYPE_ID, // 773
  SPV_OPERAND_TYPE_EXECUTION_MODEL, // 774
  SPV_OPERAND_TYPE_ID, // 775
  SPV_OPERAND_TYPE_LITERAL_STRING, // 776
  SPV_OPERAND_TYPE_VARIABLE_ID, // 777
  SPV_OPERAND_TYPE_ID, // 778
  SPV_OPERAND_TYPE_CAPABILITY, // 779
  SPV_OPERAND_TYPE_TYPE_ID, // 780
  SPV_OPERAND_TYPE_RESULT_ID, // 781
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 782
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 783
  SPV_OPERAND_TYPE_TYPE_ID, // 784
  SPV_OPERAND_TYPE_RESULT_ID, // 785
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 786
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 787
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 788
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 789
  SPV_OPERAND_TYPE_TYPE_ID, // 790
  SPV_OPERAND_TYPE_RESULT_ID, // 791
  SPV_OPERAND_TYPE_VARIABLE_CAPABILITY, // 792
  SPV_OPERAND_TYPE_TYPE_ID, // 793
  SPV_OPERAND_TYPE_RESULT_ID, // 794
  SPV_OPERAND_TYPE_ID, // 795
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 796
  SPV_OPERAND_TYPE_ID, // 797
  SPV_OPERAND_TYPE_ID, // 798
  SPV_OPERAND_TYPE_ID, // 799
  SPV_OPERAND_TYPE_ID, // 800
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 801
  SPV_OPERAND_TYPE_ID, // 802
  SPV_OPERAND_TYPE_ID, // 803
  SPV_OPERAND_TYPE_ID, // 804
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 805
  SPV_OPERAND_TYPE_ID, // 806
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 807
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 808
  SPV_OPERAND_TYPE_ID, // 809
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 810
  SPV_OPERAND_TYPE_ID, // 811
  SPV_OPERAND_TYPE_ID, // 812
  SPV_OPERAND_TYPE_ID, // 813
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 814
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 815
  SPV_OPERAND_TYPE_ID, // 816
  SPV_OPERAND_TYPE_ID, // 817
  SPV_OPERAND_TYPE_ID, // 818
  SPV_OPERAND_TYPE_ID, // 819
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 820
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 821
  SPV_OPERAND_TYPE_ID, // 822
  SPV_OPERAND_TYPE_ID, // 823
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 824
  SPV_OPERAND_TYPE_VARIABLE_ID, // 825
  SPV_OPERAND_TYPE_ID, // 826
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 827
  SPV_OPERAND_TYPE_ID, // 828
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 829
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 830
  SPV_OPERAND_TYPE_ID, // 831
  SPV_OPERAND_TYPE_ID, // 832
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 833
  SPV_OPERAND_TYPE_VARIABLE_ID, // 834
  SPV_OPERAND_TYPE_ID, // 835
  SPV_OPERAND_TYPE_ID, // 836
  SPV_OPERAND_TYPE_ID, // 837
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 838
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 839
  SPV_OPERAND_TYPE_ID, // 840
  SPV_OPERAND_TYPE_ID, // 841
  SPV_OPERAND_TYPE_ID, // 842
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 843
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 844
  SPV_OPERAND_TYPE_ID, // 845
  SPV_OPERAND_TYPE_ID, // 846
  SPV_OPERAND_TYPE_ID, // 847
  SPV_OPERAND_TYPE_ID, // 848
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 849
  SPV_OPERAND_TYPE_ID, // 850
  SPV_OPERAND_TYPE_ID, // 851
  SPV_OPERAND_TYPE_ID, // 852
  SPV_OPERAND_TYPE_ID, // 853
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 854
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 855
  SPV_OPERAND_TYPE_ID, // 856
  SPV_OPERAND_TYPE_ID, // 857
  SPV_OPERAND_TYPE_ID, // 858
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 859
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 860
  SPV_OPERAND_TYPE_ID, // 861
  SPV_OPERAND_TYPE_ID, // 862
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 863
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 864
  SPV_OPERAND_TYPE_VARIABLE_ID, // 865
  SPV_OPERAND_TYPE_ID, // 866
  SPV_OPERAND_TYPE_ID, // 867
  SPV_OPERAND_TYPE_ID, // 868
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 869
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 870
  SPV_OPERAND_TYPE_ID, // 871
  SPV_OPERAND_TYPE_ID, // 872
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 873
  SPV_OPERAND_TYPE_ID, // 874
  SPV_OPERAND_TYPE_ID, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 877
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 878
  SPV_OPERAND_TYPE_ID, // 879
  SPV_OPERAND_TYPE_ID, // 880
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 881
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 882
  SPV_OPERAND_TYPE_ID, // 883
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 884
  SPV_OPERAND_TYPE_ID, // 885
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 886
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 887
  SPV_OPERAND_TYPE_ID, // 888
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 889
  SPV_OPERAND_TYPE_ID, // 890
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 891
  SPV_OPERAND_TYPE_ID, // 892
  SPV_OPERAND_TYPE_ID, // 893
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 894
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 895
  SPV_OPERAND_TYPE_ID, // 896
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_ID, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 901
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 902
  SPV_OPERAND_TYPE_ID, // 903
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 904
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 905
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 906
  SPV_OPERAND_TYPE_ID, // 907
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 908
  SPV_OPERAND_TYPE_ID, // 909
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 910
  SPV_OPERAND_TYPE_ID, // 911
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 912
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 913
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 914
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 915
  SPV_OPERAND_TYPE_ID, // 916
  SPV_OPERAND_TYPE_ID, // 917
  SPV_OPERAND_TYPE_ID, // 918
  SPV_OPERAND_TYPE_ID, // 919
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 920
  SPV_OPERAND_TYPE_ID, // 921
  SPV_OPERAND_TYPE_ID, // 922
  SPV_OPERAND_TYPE_ID, // 923
  SPV_OPERAND_TYPE_ID, // 924
  SPV_OPERAND_TYPE_ID, // 925
  SPV_OPERAND_TYPE_ID, // 926
  SPV_OPERAND_TYPE_ID, // 927
  SPV_OPERAND_TYPE_ID, // 928
  SPV_OPERAND_TYPE_VARIABLE_ID, // 929
  SPV_OPERAND_TYPE_ID, // 930
  SPV_OPERAND_TYPE_ID, // 931
  SPV_OPERAND_TYPE_ID, // 932
  SPV_OPERAND_TYPE_ID, // 933
  SPV_OPERAND_TYPE_ID, // 934
  SPV_OPERAND_TYPE_ID, // 935
  SPV_OPERAND_TYPE_ID, // 936
  SPV_OPERAND_TYPE_ID, // 937
  SPV_OPERAND_TYPE_ID, // 938
  SPV_OPERAND_TYPE_VARIABLE_ID, // 939
  SPV_OPERAND_TYPE_ID, // 940
  SPV_OPERAND_TYPE_ID, // 941
  SPV_OPERAND_TYPE_ID, // 942
  SPV_OPERAND_TYPE_ID, // 943
  SPV_OPERAND_TYPE_ID, // 944
  SPV_OPERAND_TYPE_ID, // 945
  SPV_OPERAND_TYPE_ID, // 946
  SPV_OPERAND_TYPE_ID, // 947
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 948
  SPV_OPERAND_TYPE_ID, // 949
  SPV_OPERAND_TYPE_ID, // 950
  SPV_OPERAND_TYPE_ID, // 951
  SPV_OPERAND_TYPE_ID, // 952
  SPV_OPERAND_TYPE_VARIABLE_ID, // 953
  SPV_OPERAND_TYPE_ID, // 954
  SPV_OPERAND_TYPE_ID, // 955
  SPV_OPERAND_TYPE_ID, // 956
  SPV_OPERAND_TYPE_ID, // 957
  SPV_OPERAND_TYPE_ID, // 958
  SPV_OPERAND_TYPE_ID, // 959
  SPV_OPERAND_TYPE_ID, // 960
  SPV_OPERAND_TYPE_ID, // 961
  SPV_OPERAND_TYPE_ID, // 962
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 963
  SPV_OPERAND_TYPE_ID, // 964
  SPV_OPERAND_TYPE_ID, // 965
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 966
  SPV_OPERAND_TYPE_ID, // 967
  SPV_OPERAND_TYPE_ID, // 968
  SPV_OPERAND_TYPE_ID, // 969
  SPV_OPERAND_TYPE_ID, // 970
  SPV_OPERAND_TYPE_ID, // 971
  SPV_OPERAND_TYPE_ID, // 972
  SPV_OPERAND_TYPE_ID, // 973
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_ID, // 976
  SPV_OPERAND_TYPE_ID, // 977
  SPV_OPERAND_TYPE_VARIABLE_ID, // 978
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
  SPV_OPERAND_TYPE_ID, // 1013
  SPV_OPERAND_TYPE_ID, // 1014
  SPV_OPERAND_TYPE_ID, // 1015
  SPV_OPERAND_TYPE_ID, // 1016
  SPV_OPERAND_TYPE_ID, // 1017
  SPV_OPERAND_TYPE_ID, // 1018
  SPV_OPERAND_TYPE_ID, // 1019
  SPV_OPERAND_TYPE_ID, // 1020
  SPV_OPERAND_TYPE_ID, // 1021
  SPV_OPERAND_TYPE_ID, // 1022
  SPV_OPERAND_TYPE_ID, // 1023
  SPV_OPERAND_TYPE_ID, // 1024
  SPV_OPERAND_TYPE_ID, // 1025
  SPV_OPERAND_TYPE_ID, // 1026
  SPV_OPERAND_TYPE_ID, // 1027
  SPV_OPERAND_TYPE_ID, // 1028
  SPV_OPERAND_TYPE_ID, // 1029
  SPV_OPERAND_TYPE_ID, // 1030
  SPV_OPERAND_TYPE_ID, // 1031
  SPV_OPERAND_TYPE_ID, // 1032
  SPV_OPERAND_TYPE_ID, // 1033
  SPV_OPERAND_TYPE_ID, // 1034
  SPV_OPERAND_TYPE_ID, // 1035
  SPV_OPERAND_TYPE_ID, // 1036
  SPV_OPERAND_TYPE_ID, // 1037
  SPV_OPERAND_TYPE_ID, // 1038
  SPV_OPERAND_TYPE_ID, // 1039
  SPV_OPERAND_TYPE_ID, // 1040
  SPV_OPERAND_TYPE_ID, // 1041
  SPV_OPERAND_TYPE_ID, // 1042
  SPV_OPERAND_TYPE_ID, // 1043
  SPV_OPERAND_TYPE_ID, // 1044
  SPV_OPERAND_TYPE_ID, // 1045
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1046
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1047
  SPV_OPERAND_TYPE_ID, // 1048
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 1049
  SPV_OPERAND_TYPE_ID, // 1050
  SPV_OPERAND_TYPE_ID, // 1051
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 1052
  SPV_OPERAND_TYPE_ID, // 1053
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 1054
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1055
  SPV_OPERAND_TYPE_ID, // 1056
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 1057
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1058
  SPV_OPERAND_TYPE_ID, // 1059
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1060
  SPV_OPERAND_TYPE_ID, // 1061
  SPV_OPERAND_TYPE_ID, // 1062
  SPV_OPERAND_TYPE_ID, // 1063
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1064
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1065
  SPV_OPERAND_TYPE_ID, // 1066
  SPV_OPERAND_TYPE_ID, // 1067
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1068
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1069
  SPV_OPERAND_TYPE_ID, // 1070
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 1071
  SPV_OPERAND_TYPE_ID, // 1072
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1073
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1074
  SPV_OPERAND_TYPE_ID, // 1075
  SPV_OPERAND_TYPE_ID, // 1076
  SPV_OPERAND_TYPE_ID, // 1077
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1078
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1079
  SPV_OPERAND_TYPE_ID, // 1080
  SPV_OPERAND_TYPE_ID, // 1081
  SPV_OPERAND_TYPE_ID, // 1082
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1083
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1084
  SPV_OPERAND_TYPE_ID, // 1085
  SPV_OPERAND_TYPE_ID, // 1086
  SPV_OPERAND_TYPE_ID, // 1087
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1088
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1089
  SPV_OPERAND_TYPE_ID, // 1090
  SPV_OPERAND_TYPE_ID, // 1091
  SPV_OPERAND_TYPE_ID, // 1092
  SPV_OPERAND_TYPE_ID, // 1093
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1094
  SPV_OPERAND_TYPE_ID, // 1095
  SPV_OPERAND_TYPE_ID, // 1096
  SPV_OPERAND_TYPE_ID, // 1097
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1098
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1099
  SPV_OPERAND_TYPE_ID, // 1100
  SPV_OPERAND_TYPE_ID, // 1101
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1102
  SPV_OPERAND_TYPE_ID, // 1103
  SPV_OPERAND_TYPE_ID, // 1104
  SPV_OPERAND_TYPE_ID, // 1105
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1106
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1107
  SPV_OPERAND_TYPE_ID, // 1108
  SPV_OPERAND_TYPE_ID, // 1109
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1110
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1111
  SPV_OPERAND_TYPE_ID, // 1112
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1113
  SPV_OPERAND_TYPE_ID, // 1114
  SPV_OPERAND_TYPE_ID, // 1115
  SPV_OPERAND_TYPE_ID, // 1116
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1117
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1118
  SPV_OPERAND_TYPE_ID, // 1119
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1120
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1121
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1122
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1123
  SPV_OPERAND_TYPE_ID, // 1124
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1125
  SPV_OPERAND_TYPE_ID, // 1126
  SPV_OPERAND_TYPE_ID, // 1127
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1128
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1129
  SPV_OPERAND_TYPE_ID, // 1130
  SPV_OPERAND_TYPE_ID, // 1131
  SPV_OPERAND_TYPE_ID, // 1132
  SPV_OPERAND_TYPE_ID, // 1133
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1134
  SPV_OPERAND_TYPE_ID, // 1135
  SPV_OPERAND_TYPE_ID, // 1136
  SPV_OPERAND_TYPE_ID, // 1137
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1138
  SPV_OPERAND_TYPE_ID, // 1139
  SPV_OPERAND_TYPE_ID, // 1140
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1141
  SPV_OPERAND_TYPE_ID, // 1142
  SPV_OPERAND_TYPE_ID, // 1143
  SPV_OPERAND_TYPE_ID, // 1144
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1145
};
