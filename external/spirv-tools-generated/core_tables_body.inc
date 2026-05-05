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
    case Extension::kSPV_INTEL_predicated_io: return IR(3046, 24);
    case Extension::kSPV_INTEL_rounded_divide_sqrt: return IR(3070, 30);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(3100, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(3126, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(3162, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3186, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3221, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3267, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3287, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3311, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3347, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3382, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3419, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3449, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3481, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3506, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3528, 21);
    case Extension::kSPV_KHR_abort: return IR(3549, 14);
    case Extension::kSPV_KHR_bfloat16: return IR(3563, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3580, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3605, 35);
    case Extension::kSPV_KHR_constant_data: return IR(3640, 22);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3662, 27);
    case Extension::kSPV_KHR_device_group: return IR(3689, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3710, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3732, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3755, 24);
    case Extension::kSPV_KHR_fma: return IR(3779, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3791, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3827, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3857, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3885, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3906, 30);
    case Extension::kSPV_KHR_multiview: return IR(3936, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3954, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3989, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(4015, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(4047, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(4069, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4097, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4118, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4140, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4158, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4178, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4213, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4250, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4284, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4306, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4327, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4358, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4395, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4419, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4457, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4479, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4508, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4543, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4568, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4594, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4622, 41);
    case Extension::kSPV_MESA_fragment_coverage_mask: return IR(4663, 32);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4695, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4733, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4757, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4795, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4829, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4855, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(4882, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(4908, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(4937, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(4972, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(5007, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(5035, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(5054, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(5080, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(5105, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5124, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5155, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5192, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5225, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5255, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5288, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5314, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5349, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5369, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5398, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5423, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5446, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5485, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5511, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(5538, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5560, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5598, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,194> kExtensionNames{{
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
    {IR(3046, 24), static_cast<uint32_t>(kSPV_INTEL_predicated_io)},
    {IR(3070, 30), static_cast<uint32_t>(kSPV_INTEL_rounded_divide_sqrt)},
    {IR(3100, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(3126, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(3162, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3186, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3221, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3267, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3287, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3311, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3347, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3382, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3419, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3449, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3481, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3506, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3528, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3549, 14), static_cast<uint32_t>(kSPV_KHR_abort)},
    {IR(3563, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3580, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3605, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3640, 22), static_cast<uint32_t>(kSPV_KHR_constant_data)},
    {IR(3662, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3689, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3710, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3732, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3755, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3779, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3791, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3827, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3857, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3885, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3906, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3936, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3954, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3989, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(4015, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(4047, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(4069, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4097, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4118, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4140, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4158, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4178, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4213, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4250, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4284, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4306, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4327, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4358, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4395, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4419, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4457, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4479, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4508, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4543, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4568, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4594, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4622, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4663, 32), static_cast<uint32_t>(kSPV_MESA_fragment_coverage_mask)},
    {IR(4695, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4733, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4757, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4795, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4829, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4855, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(4882, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(4908, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(4937, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(4972, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(5007, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(5035, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(5054, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(5080, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(5105, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5124, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5155, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5192, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5225, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5255, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5288, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5314, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5349, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5369, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5398, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5423, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5446, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5485, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5511, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5538, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5560, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5598, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1216> kOperandsByValue{{
  {0,IR(0, 0),IR(5632, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5648, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5658, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5668, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5676, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5697, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(5708, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(5790, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(5806, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(5816, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(5829, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(5842, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(5851, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(5862, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(5952, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(5965, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(6011, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(6059, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(6074, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(6089, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(6099, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(6113, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6123, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6134, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6146, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6173, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6188, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6199, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6209, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6226, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6240, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6254, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6266, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6284, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6303, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6324, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6332, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6343, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6365, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6378, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6393, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6440, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6456, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6469, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6490, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6501, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6527, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6539, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4096,IR(0, 0),IR(6553, 25)/* FragmentCoverageMaskMESA */,IR(0, 0),IR(31, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6599, 10)/* CoreIDARM */,IR(0, 0),IR(32, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6625, 13)/* CoreCountARM */,IR(0, 0),IR(32, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6638, 13)/* CoreMaxIDARM */,IR(0, 0),IR(32, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6651, 10)/* WarpIDARM */,IR(0, 0),IR(32, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6661, 13)/* WarpMaxIDARM */,IR(0, 0),IR(32, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6674, 15)/* SubgroupEqMask */,IR(1, 1),IR(33, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6729, 15)/* SubgroupGeMask */,IR(2, 1),IR(33, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(6762, 15)/* SubgroupGtMask */,IR(3, 1),IR(33, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(6795, 15)/* SubgroupLeMask */,IR(4, 1),IR(33, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(6828, 15)/* SubgroupLtMask */,IR(5, 1),IR(33, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(6861, 11)/* BaseVertex */,IR(0, 0),IR(35, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(6887, 13)/* BaseInstance */,IR(0, 0),IR(35, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(6900, 10)/* DrawIndex */,IR(0, 0),IR(36, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(6910, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(39, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(6957, 12)/* DeviceIndex */,IR(0, 0),IR(40, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(6981, 10)/* ViewIndex */,IR(0, 0),IR(41, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(7001, 15)/* ShadingRateKHR */,IR(0, 0),IR(39, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(7016, 15)/* TileOffsetQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(7047, 18)/* TileDimensionQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(7065, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(7083, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(7103, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(7131, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(7157, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7176, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7203, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7228, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7250, 18)/* FragStencilRefEXT */,IR(0, 0),IR(43, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7285, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7332, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7348, 15)/* SamplerHeapEXT */,IR(0, 0),IR(45, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7381, 16)/* ResourceHeapEXT */,IR(0, 0),IR(45, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7397, 15)/* ViewportMaskNV */,IR(0, 0),IR(46, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7433, 20)/* SecondaryPositionNV */,IR(0, 0),IR(48, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7472, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(48, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7496, 18)/* PositionPerViewNV */,IR(0, 0),IR(49, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7534, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(49, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7556, 16)/* FullyCoveredEXT */,IR(0, 0),IR(51, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7596, 12)/* TaskCountNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7608, 17)/* PrimitiveCountNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7625, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7644, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7666, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7688, 15)/* LayerPerViewNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7703, 16)/* MeshViewCountNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7719, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7737, 13)/* BaryCoordKHR */,IR(6, 1),IR(53, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(7785, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(53, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(7824, 12)/* FragSizeEXT */,IR(8, 1),IR(54, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(7870, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(54, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(7915, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(55, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(7940, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(55, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(7964, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(55, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(7992, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(55, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(8009, 12)/* LaunchIdKHR */,IR(10, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(8032, 14)/* LaunchSizeKHR */,IR(11, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(8059, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(8094, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(8135, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8172, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8215, 11)/* RayTminKHR */,IR(16, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8236, 11)/* RayTmaxKHR */,IR(17, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8257, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8302, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8335, 17)/* WorldToObjectKHR */,IR(20, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8368, 7)/* HitTNV */,IR(0, 0),IR(58, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8375, 11)/* HitKindKHR */,IR(21, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8396, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(59, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8436, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8493, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8560, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8597, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8636, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(62, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8656, 14)/* HitIsSphereNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8698, 11)/* HitIsLSSNV */,IR(0, 0),IR(64, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(8748, 20)/* HitSpherePositionNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(8768, 13)/* WarpsPerSMNV */,IR(0, 0),IR(65, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(8800, 10)/* SMCountNV */,IR(0, 0),IR(65, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(8810, 9)/* WarpIDNV */,IR(0, 0),IR(65, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(8819, 7)/* SMIDNV */,IR(0, 0),IR(65, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(8826, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(64, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(8844, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(8878, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(8911, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(8929, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(64, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(8943, 12)/* ClusterIDNV */,IR(0, 0),IR(66, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(8996, 12)/* CullMaskKHR */,IR(0, 0),IR(67, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(9023, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(5799, 7)/* Shader */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(5874, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(5883, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5687, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(9030, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5641, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(9038, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(9047, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(9061, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(9069, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(9077, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(9083, 13)/* Int64Atomics */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(9096, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(9107, 15)/* ImageReadWrite */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(9122, 12)/* ImageMipmap */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(9134, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(9140, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(9147, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(9161, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9176, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9190, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9196, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9218, 18)/* GeometryPointSize */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9236, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9256, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9280, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9314, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9347, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9381, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(5816, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(5829, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9414, 15)/* ImageCubeArray */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6155, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9446, 10)/* ImageRect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9456, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9468, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9483, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9488, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9504, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9520, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9527, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9537, 8)/* Image1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9429, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9545, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9559, 12)/* ImageBuffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9571, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9584, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9612, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9623, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9641, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9663, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9681, 16)/* GeometryStreams */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9697, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(9727, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(6025, 14)/* MultiViewport */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(9758, 17)/* SubgroupDispatch */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(9775, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(9788, 12)/* PipeStorage */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6406, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(9800, 20)/* GroupNonUniformVote */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(9820, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6707, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(9846, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(9869, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(9900, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(9925, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(5971, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(6039, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(9945, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4097,IR(0, 0),IR(6578, 21)/* FragmentCoverageMESA */,IR(0, 0),IR(3, 1),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6609, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(9963, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(9991, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(10019, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(10049, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(10060, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(10097, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(10137, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(10146, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10174, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10184, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(79, 2),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(6934, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6422, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(6872, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10232, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10265, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10308, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10352, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10368, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10421, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(82, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10473, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10495, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(6969, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(6991, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10516, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10546, 17)/* VariablePointers */,IR(0, 0),IR(83, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10563, 17)/* AtomicStorageOps */,IR(0, 0),IR(84, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10580, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10608, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10632, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(85, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(10666, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(10687, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(10702, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(10720, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(10745, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(10761, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(10777, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(10800, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(10812, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(10831, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(86, 2),IR(51, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(5909, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(10863, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(10889, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(10910, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(7031, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(10932, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(88, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(10964, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(10987, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(11003, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(11025, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7268, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(11041, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(11062, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(11076, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7314, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(11091, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(11106, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(11120, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(89, 2),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(11147, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(11163, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(91, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11185, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(92, 2),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5120,IR(0, 0),IR(11214, 9)/* AbortKHR */,IR(0, 0),IR(1, 0),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7363, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(94, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5146,IR(0, 0),IR(11223, 16)/* ConstantDataKHR */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11239, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11255, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11284, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(71, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(5983, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(95, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7412, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(96, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7453, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(97, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7514, 20)/* PerViewAttributesNV */,IR(0, 0),IR(41, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7572, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(5923, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11339, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(5937, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(7762, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11378, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(7851, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11453, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(80, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(11512, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(11549, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(11598, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(98, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(11673, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(75, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(11754, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(99, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(11835, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(100, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(11912, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(100, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(11987, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(100, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(12064, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(100, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(12139, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(101, 2),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12220, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(103, 2),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12307, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(105, 2),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8466, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(5896, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8413, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12394, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12433, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(5759, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(12528, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(12591, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(12616, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(12636, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(12669, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(8781, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(12707, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(12739, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(12792, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(12815, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(12844, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(62, 1),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(12870, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(62, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(12897, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(12915, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(12940, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(12960, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8527, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(62, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(12982, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8670, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8709, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(13000, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(13020, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(13034, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(13057, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(13087, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(13118, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(13158, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13194, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13224, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(8955, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(62, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13252, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13271, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(13292, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(13319, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(13345, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(13376, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(13397, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(13420, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(13443, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(13465, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(13489, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(13498, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(13521, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(13544, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(13567, 19)/* VectorComputeINTEL */,IR(0, 0),IR(107, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(13586, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(13601, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(13617, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(13650, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(13688, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(13727, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(13752, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(13778, 27)/* FPGAMemoryAttributesALTERA */,IR(47, 1),IR(1, 0),IR(112, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(13831, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(13851, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(48, 1),IR(1, 0),IR(115, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(13916, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(49, 1),IR(1, 0),IR(117, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(13991, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(14021, 23)/* FPGALoopControlsALTERA */,IR(50, 1),IR(1, 0),IR(120, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(14066, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(14088, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(14114, 25)/* FPGAMemoryAccessesALTERA */,IR(51, 1),IR(1, 0),IR(123, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(14163, 28)/* FPGAClusterAttributesALTERA */,IR(52, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14218, 15)/* LoopFuseALTERA */,IR(53, 1),IR(1, 0),IR(127, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14247, 21)/* FPGADSPControlALTERA */,IR(54, 1),IR(1, 0),IR(129, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(14288, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(14314, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(55, 1),IR(1, 0),IR(132, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(14395, 25)/* FPGABufferLocationALTERA */,IR(56, 1),IR(1, 0),IR(134, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(14444, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(57, 1),IR(1, 0),IR(136, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(14513, 24)/* USMStorageClassesALTERA */,IR(58, 1),IR(1, 0),IR(138, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(14560, 30)/* RuntimeAlignedAttributeALTERA */,IR(59, 1),IR(1, 0),IR(140, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(14619, 14)/* IOPipesALTERA */,IR(60, 1),IR(1, 0),IR(142, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(14646, 20)/* BlockingPipesALTERA */,IR(61, 1),IR(1, 0),IR(144, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(14685, 14)/* FPGARegALTERA */,IR(62, 1),IR(1, 0),IR(146, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(14712, 19)/* DotProductInputAll */,IR(63, 1),IR(1, 0),IR(148, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(14753, 22)/* DotProductInput4x8Bit */,IR(64, 1),IR(108, 1),IR(148, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(14800, 28)/* DotProductInput4x8BitPacked */,IR(65, 1),IR(1, 0),IR(148, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(14859, 11)/* DotProduct */,IR(66, 1),IR(1, 0),IR(148, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(9008, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10211, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(14884, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(14908, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(14924, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(78, 1),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(14949, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(153, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(14964, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(154, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(14971, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(155, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(14991, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(155, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(15011, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(156, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(15031, 11)/* OptNoneEXT */,IR(67, 1),IR(1, 0),IR(157, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(15055, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(15075, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(15096, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(15120, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(15138, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(15157, 30)/* FPGAClusterAttributesV2ALTERA */,IR(68, 1),IR(109, 1),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15216, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(110, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(15244, 19)/* TaskSequenceALTERA */,IR(69, 1),IR(1, 0),IR(164, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(15281, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(15297, 25)/* FPGALatencyControlALTERA */,IR(70, 1),IR(1, 0),IR(167, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(15346, 29)/* FPGAArgumentInterfacesALTERA */,IR(71, 1),IR(1, 0),IR(169, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(15403, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(171, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(15433, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(72, 1),IR(1, 0),IR(172, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(15504, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(174, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(15532, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(15555, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(111, 1),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(15585, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(111, 1),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(15615, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(176, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(15653, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(177, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(15681, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(112, 2),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(15713, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(15734, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(114, 1),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6257,IR(0, 0),IR(15756, 18)/* PredicatedIOINTEL */,IR(0, 0),IR(1, 0),IR(179, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6265,IR(0, 0),IR(15774, 23)/* RoundedDivideSqrtINTEL */,IR(0, 0),IR(1, 0),IR(180, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(15797, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(181, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(15823, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(15850, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(183, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(15875, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(184, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(15894, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(15914, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(186, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(15934, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(115, 1),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(15967, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(115, 1),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(16000, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(91, 1),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(16027, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(116, 1),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(16059, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16071, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16079, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16087, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16093, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16100, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16111, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16120, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16133, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16139, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16149, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16155, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16170, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16195, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16211, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16225, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16238, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16250, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16267, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16279, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16294, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16307, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16322, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16340, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16357, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16378, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16398, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16418, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16434, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16450, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16470, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16494, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16500, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16505, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(16544, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(16555, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16564, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(16569, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(16576, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(16587, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(16594, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(16603, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16613, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16626, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(16639, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(16650, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(16660, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(16670, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(16680, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(16693, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(16707, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(16721, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(16735, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(16750, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(16766, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(16782, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(16798, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(16817, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(16838, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(16850, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(16862, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(16874, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(16889, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(16914, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16942, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(16950, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(16977, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(17004, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(17031, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(17063, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17089, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(17093, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(17100, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(17104, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(17115, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(17126, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(17147, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17158, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(17172, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(17193, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16059, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16071, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16079, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16087, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16093, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16100, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16111, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(16120, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16133, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16139, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16149, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16195, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16211, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16225, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16238, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16250, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16267, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16279, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16294, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16307, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16322, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16340, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16357, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16378, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16398, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16418, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16494, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(16500, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(16505, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(16544, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(16555, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(16564, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(16569, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(16576, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(16587, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(16603, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16613, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16626, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17211, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(17228, 7)/* SpecId */,IR(0, 0),IR(118, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(17235, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(17241, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(17253, 9)/* RowMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(17262, 9)/* ColMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(17271, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(17283, 13)/* MatrixStride */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(17296, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(17307, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(17318, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(17352, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(17360, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(17374, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(17379, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(17385, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(17394, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(17401, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(17411, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(17420, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(17428, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(17437, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(17446, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(17455, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(17467, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(17479, 8)/* Uniform */,IR(0, 0),IR(120, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(17513, 10)/* UniformId */,IR(0, 0),IR(120, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(17523, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(17543, 7)/* Stream */,IR(0, 0),IR(122, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(17550, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(17559, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(17569, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(17575, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(17583, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(17597, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(17604, 10)/* XfbBuffer */,IR(0, 0),IR(123, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(17614, 10)/* XfbStride */,IR(0, 0),IR(123, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(17670, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(17718, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(17768, 15)/* FPFastMathMode */,IR(0, 0),IR(124, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(17845, 18)/* LinkageAttributes */,IR(0, 0),IR(126, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(17863, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(17877, 21)/* InputAttachmentIndex */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(17898, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(17908, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(17942, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(17954, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(17970, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(18014, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(188, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(18027, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(188, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(18042, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(18060, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(18082, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(18104, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(18122, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(18154, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(18174, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(18197, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(18217, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(18242, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(18269, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(18294, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(18322, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(45, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(18339, 12)/* OffsetIdEXT */,IR(0, 0),IR(45, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5145,IR(0, 0),IR(18351, 14)/* UTFEncodedKHR */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(18365, 19)/* OverrideCoverageNV */,IR(0, 0),IR(128, 1),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(18384, 14)/* PassthroughNV */,IR(0, 0),IR(129, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(18398, 19)/* ViewportRelativeNV */,IR(0, 0),IR(97, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(18417, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(48, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(18445, 16)/* PerPrimitiveEXT */,IR(73, 1),IR(130, 2),IR(189, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(18476, 10)/* PerViewNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(18486, 10)/* PerTaskNV */,IR(0, 0),IR(52, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(18496, 13)/* PerVertexKHR */,IR(74, 1),IR(53, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(18521, 11)/* NonUniform */,IR(75, 1),IR(100, 1),IR(82, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(18546, 16)/* RestrictPointer */,IR(76, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(18581, 15)/* AliasedPointer */,IR(77, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(18614, 15)/* MemberOffsetNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(18629, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(18659, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(18690, 7)/* BankNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(18697, 18)/* BindlessSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(18715, 16)/* BindlessImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(18731, 15)/* BoundSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(18746, 13)/* BoundImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(18759, 14)/* SIMTCallINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(18773, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(137, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(18799, 13)/* ClobberINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(18812, 17)/* SideEffectsINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(18829, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(18856, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(18877, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(18904, 15)/* StackCallINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(18919, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(18945, 14)/* CounterBuffer */,IR(78, 1),IR(1, 0),IR(191, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(18983, 13)/* UserSemantic */,IR(79, 1),IR(1, 0),IR(191, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(19015, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(19030, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(19087, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(19111, 15)/* RegisterALTERA */,IR(80, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(19140, 13)/* MemoryALTERA */,IR(81, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(19165, 15)/* NumbanksALTERA */,IR(82, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(19194, 16)/* BankwidthALTERA */,IR(83, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(19225, 23)/* MaxPrivateCopiesALTERA */,IR(84, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(19270, 17)/* SinglepumpALTERA */,IR(85, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(19303, 17)/* DoublepumpALTERA */,IR(86, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(19336, 20)/* MaxReplicatesALTERA */,IR(87, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(19375, 21)/* SimpleDualPortALTERA */,IR(88, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(19416, 12)/* MergeALTERA */,IR(89, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(19481, 15)/* BankBitsALTERA */,IR(90, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(19510, 21)/* ForcePow2DepthALTERA */,IR(91, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(19551, 17)/* StridesizeALTERA */,IR(92, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(19584, 15)/* WordsizeALTERA */,IR(93, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(19613, 19)/* TrueDualPortALTERA */,IR(94, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(19650, 20)/* BurstCoalesceALTERA */,IR(95, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(19689, 16)/* CacheSizeALTERA */,IR(96, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(19720, 29)/* DontStaticallyCoalesceALTERA */,IR(97, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(19777, 15)/* PrefetchALTERA */,IR(98, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(19806, 18)/* StallEnableALTERA */,IR(99, 1),IR(109, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(19841, 26)/* FuseLoopsInFunctionALTERA */,IR(100, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(19892, 20)/* MathOpDSPModeALTERA */,IR(101, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(19931, 16)/* AliasScopeINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(19947, 13)/* NoAliasINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(19960, 25)/* InitiationIntervalALTERA */,IR(102, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(20009, 21)/* MaxConcurrencyALTERA */,IR(103, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(20050, 21)/* PipelineEnableALTERA */,IR(104, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(20091, 21)/* BufferLocationALTERA */,IR(105, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(20132, 20)/* IOPipeStorageALTERA */,IR(106, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(20205, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(20236, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(20261, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(20296, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(20314, 16)/* StallFreeALTERA */,IR(107, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(20376, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(20402, 26)/* LatencyControlLabelALTERA */,IR(108, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(20453, 31)/* LatencyControlConstraintALTERA */,IR(109, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(20514, 28)/* ConduitKernelArgumentALTERA */,IR(110, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(20569, 32)/* RegisterMapKernelArgumentALTERA */,IR(111, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(20632, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(112, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(20699, 31)/* MMHostInterfaceDataWidthALTERA */,IR(113, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(20760, 29)/* MMHostInterfaceLatencyALTERA */,IR(114, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(20851, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(115, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(20920, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(116, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(20979, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(117, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(21044, 27)/* StableKernelArgumentALTERA */,IR(118, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(21136, 16)/* HostAccessINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(21199, 15)/* InitModeALTERA */,IR(119, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(21228, 29)/* ImplementInRegisterMapALTERA */,IR(120, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(21285, 17)/* ConditionalINTEL */,IR(0, 0),IR(114, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(21338, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(21397, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(21420, 3)/* 1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(21423, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(21426, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(21429, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(21434, 5)/* Rect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(21439, 7)/* Buffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(21446, 12)/* SubpassData */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(21458, 17)/* TileImageDataEXT */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(21475, 12)/* Invocations */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(21487, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(21500, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(21522, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(21543, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(21557, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(21572, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(21591, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(21607, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(21623, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(21642, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(21652, 4)/* Xfb */,IR(0, 0),IR(123, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(21656, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(21671, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(21684, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(21694, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(21709, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(21719, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(21733, 12)/* InputPoints */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(21745, 11)/* InputLines */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(21756, 20)/* InputLinesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(21776, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(21786, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(21810, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(21816, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(21825, 15)/* OutputVertices */,IR(0, 0),IR(156, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(21840, 13)/* OutputPoints */,IR(0, 0),IR(160, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(21853, 16)/* OutputLineStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(21869, 20)/* OutputTriangleStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(21889, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(21901, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(21916, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(21928, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6393, 13)/* SubgroupSize */,IR(0, 0),IR(163, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(21938, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(163, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(21960, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(163, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(21984, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(21996, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(22012, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(22046, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(164, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(22080, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(22116, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(22146, 18)/* PostDepthCoverage */,IR(0, 0),IR(166, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(10687, 15)/* DenormPreserve */,IR(0, 0),IR(167, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(10702, 18)/* DenormFlushToZero */,IR(0, 0),IR(168, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(10720, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(169, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(10745, 16)/* RoundingModeRTE */,IR(0, 0),IR(170, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(10761, 16)/* RoundingModeRTZ */,IR(0, 0),IR(171, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(22164, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(22198, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(22218, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(22247, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(43, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(22270, 15)/* CoalescingAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(22285, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(22300, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(22321, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7332, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(22345, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(22366, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(22394, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(22420, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(22443, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(22470, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(22495, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(43, 1),IR(195, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(22517, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(22536, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(22556, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(22576, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(22596, 15)/* OutputLinesEXT */,IR(121, 1),IR(130, 2),IR(189, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(22625, 20)/* OutputPrimitivesEXT */,IR(122, 1),IR(130, 2),IR(189, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(22664, 24)/* DerivativeGroupQuadsKHR */,IR(123, 1),IR(174, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(22711, 25)/* DerivativeGroupLinearKHR */,IR(124, 1),IR(175, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(22760, 19)/* OutputTrianglesEXT */,IR(125, 1),IR(130, 2),IR(189, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(22797, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(176, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(22822, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(176, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(22849, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(177, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(22875, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(177, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(22903, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(178, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(22934, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(178, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(13034, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(179, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(22967, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(22994, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(23015, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(23036, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(23062, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(23089, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(181, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(23111, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(181, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(23127, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(181, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(23147, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(110, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(23169, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(110, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(23197, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(23221, 18)/* FPFastMathDefault */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(23239, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(110, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(23263, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(23289, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(23312, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(23334, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(23409, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(23436, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(23443, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(23463, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(5874, 9)/* Geometry */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(16594, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(23486, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5641, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(23496, 7)/* TaskNV */,IR(0, 0),IR(52, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(23503, 7)/* MeshNV */,IR(0, 0),IR(52, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(23510, 17)/* RayGenerationKHR */,IR(126, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(23543, 16)/* IntersectionKHR */,IR(127, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(23574, 10)/* AnyHitKHR */,IR(128, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(23593, 14)/* ClosestHitKHR */,IR(129, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(23620, 8)/* MissKHR */,IR(130, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(23635, 12)/* CallableKHR */,IR(131, 1),IR(56, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(23658, 8)/* TaskEXT */,IR(0, 0),IR(55, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(23666, 8)/* MeshEXT */,IR(0, 0),IR(55, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(23674, 9)/* Preserve */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(23683, 12)/* FlushToZero */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(23695, 12)/* BFloat16KHR */,IR(0, 0),IR(91, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(23707, 14)/* Float8E4M3EXT */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(23721, 14)/* Float8E5M2EXT */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(23735, 5)/* IEEE */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(23740, 4)/* ALT */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(23744, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(23751, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(23758, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(23762, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(23773, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(23778, 14)/* AllowContract */,IR(132, 1),IR(185, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(23815, 13)/* AllowReassoc */,IR(133, 1),IR(185, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(23846, 15)/* AllowTransform */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(23861, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23865, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(23869, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(23873, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23877, 16)/* Vertical2Pixels */,IR(0, 0),IR(39, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(23893, 16)/* Vertical4Pixels */,IR(0, 0),IR(39, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(23909, 18)/* Horizontal2Pixels */,IR(0, 0),IR(39, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(23927, 18)/* Horizontal4Pixels */,IR(0, 0),IR(39, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(23945, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(23952, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(23963, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(23968, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(15031, 11)/* OptNoneEXT */,IR(67, 1),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(23974, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(23979, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(23984, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(23990, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(23995, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(24003, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(24013, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(24021, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(24033, 21)/* RuntimeAlignedALTERA */,IR(134, 1),IR(188, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(24074, 7)/* Reduce */,IR(0, 0),IR(189, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(24081, 14)/* InclusiveScan */,IR(0, 0),IR(189, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(24095, 14)/* ExclusiveScan */,IR(0, 0),IR(189, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(24109, 16)/* ClusteredReduce */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(24125, 21)/* PartitionedReduceEXT */,IR(135, 1),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(24166, 28)/* PartitionedInclusiveScanEXT */,IR(136, 1),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(24221, 28)/* PartitionedExclusiveScanEXT */,IR(137, 1),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(24276, 10)/* NoneINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(24286, 10)/* ReadINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(24296, 11)/* WriteINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(24307, 15)/* ReadWriteINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(24322, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(24327, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(24331, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(24336, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(17597, 7)/* Offset */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(24348, 13)/* ConstOffsets */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(17394, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9520, 7)/* MinLod */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(24361, 19)/* MakeTexelAvailable */,IR(138, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(24402, 17)/* MakeTexelVisible */,IR(139, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(24439, 16)/* NonPrivateTexel */,IR(140, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(24474, 14)/* VolatileTexel */,IR(141, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(24505, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(24516, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(24527, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(24539, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(24547, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(24557, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(24568, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(24578, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(24589, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(24603, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(24617, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(24632, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(24643, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(24655, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(24667, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(24680, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(24694, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(24708, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(16087, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(24718, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(24729, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(24746, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(24762, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(24782, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(24802, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(24822, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(24841, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(24860, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(24879, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(24895, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(24911, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(24913, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(24915, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(24918, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(24921, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(24925, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(24930, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(24935, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(24940, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(24950, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(24960, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(24963, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(24967, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(24972, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(24978, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(24991, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(24996, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(25002, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(25008, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(25014, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(25019, 28)/* InitOnDeviceReprogramALTERA */,IR(142, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(25074, 24)/* InitOnDeviceResetALTERA */,IR(143, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(25121, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(25128, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(25139, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(25153, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(25165, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(25178, 7)/* Export */,IR(0, 0),IR(126, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(25185, 7)/* Import */,IR(0, 0),IR(126, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(25192, 12)/* LinkOnceODR */,IR(0, 0),IR(126, 1),IR(198, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(25204, 14)/* UncachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(25218, 12)/* CachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(25230, 15)/* StreamingINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(25245, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(25270, 17)/* ConstCachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(25287, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(25294, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(25305, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(25324, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(25341, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(25355, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(25369, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(25387, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(25397, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(19960, 25)/* InitiationIntervalALTERA */,IR(102, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(20009, 21)/* MaxConcurrencyALTERA */,IR(103, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(25410, 22)/* DependencyArrayALTERA */,IR(144, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(20050, 21)/* PipelineEnableALTERA */,IR(104, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(25453, 19)/* LoopCoalesceALTERA */,IR(145, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(25490, 22)/* MaxInterleavingALTERA */,IR(146, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(25533, 27)/* SpeculatedIterationsALTERA */,IR(147, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(25586, 15)/* NoFusionALTERA */,IR(148, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(25615, 16)/* LoopCountALTERA */,IR(149, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(25646, 27)/* MaxReinvocationDelayALTERA */,IR(150, 1),IR(197, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(25699, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(25728, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(25757, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(25778, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(25804, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(25827, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(25850, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(25873, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(25896, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(25913, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(25930, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(25956, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(25982, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(26009, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(17428, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(26036, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(24527, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(26044, 21)/* MakePointerAvailable */,IR(151, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(26089, 19)/* MakePointerVisible */,IR(152, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(26130, 18)/* NonPrivatePointer */,IR(153, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(26169, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(144, 1),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(26189, 17)/* NoAliasINTELMask */,IR(0, 0),IR(144, 1),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(26206, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(26213, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(26221, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(26228, 7)/* Vulkan */,IR(154, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(26245, 8)/* Relaxed */,IR(155, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(26253, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(26261, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(26269, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(26284, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(26307, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(26321, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(26336, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(26352, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(26373, 20)/* AtomicCounterMemory */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(26393, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(26405, 13)/* OutputMemory */,IR(156, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(26434, 14)/* MakeAvailable */,IR(157, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(26465, 12)/* MakeVisible */,IR(158, 1),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(17428, 9)/* Volatile */,IR(0, 0),IR(196, 1),IR(84, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(26492, 10)/* AutoINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(26502, 5)/* WRAP */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(26507, 4)/* SAT */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(26511, 9)/* SAT_ZERO */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(26520, 8)/* SAT_SYM */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(26528, 25)/* PackedVectorFormat4x8Bit */,IR(159, 1),IR(1, 0),IR(148, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(26581, 4)/* TRN */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(26585, 9)/* TRN_ZERO */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(26594, 4)/* RND */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(26598, 9)/* RND_ZERO */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(26607, 8)/* RND_INF */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(26615, 12)/* RND_MIN_INF */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(26627, 9)/* RND_CONV */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(26636, 13)/* RND_CONV_ODD */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(26649, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(26674, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(16942, 8)/* NoneKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(26697, 10)/* OpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(26707, 12)/* NoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(26719, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(26742, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(26766, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(26793, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(26821, 14)/* CullOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(26835, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(26851, 17)/* SkipTrianglesKHR */,IR(160, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(26892, 13)/* SkipAABBsKHR */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(26905, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(201, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(26935, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(26976, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27013, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27050, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(27091, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27133, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(27166, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(27199, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(27211, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(27217, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(27224, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(27239, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(27247, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(27254, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(27262, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(27270, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(27278, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(27283, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(27289, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(27300, 6)/* Rg32f */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(27306, 6)/* Rg16f */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(27312, 13)/* R11fG11fB10f */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(27325, 5)/* R16f */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(27330, 7)/* Rgba16 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(27337, 8)/* Rgb10A2 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(27345, 5)/* Rg16 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(27350, 4)/* Rg8 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(27354, 4)/* R16 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(27358, 3)/* R8 */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(27361, 12)/* Rgba16Snorm */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(27373, 10)/* Rg16Snorm */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(27383, 9)/* Rg8Snorm */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(27392, 9)/* R16Snorm */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(27401, 8)/* R8Snorm */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(27409, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(27417, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(27425, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(27432, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(27437, 6)/* Rg32i */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(27443, 6)/* Rg16i */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(27449, 5)/* Rg8i */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(27454, 5)/* R16i */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(27459, 4)/* R8i */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(27463, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(27472, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(27481, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(27489, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(27495, 10)/* Rgb10a2ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(27505, 7)/* Rg32ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(27512, 7)/* Rg16ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(27519, 6)/* Rg8ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(27525, 6)/* R16ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(27531, 5)/* R8ui */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(27536, 6)/* R64ui */,IR(0, 0),IR(204, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(27542, 5)/* R64i */,IR(0, 0),IR(204, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(27547, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(27559, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(27566, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(27576, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(27585, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(27596, 12)/* QueueFamily */,IR(161, 1),IR(196, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(27623, 14)/* ShaderCallKHR */,IR(0, 0),IR(62, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(27637, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(27645, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(27657, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(16059, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16071, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16079, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16087, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16093, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16100, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16111, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16120, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16133, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16139, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16149, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16155, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16170, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16195, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16211, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16225, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16238, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16250, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16267, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16279, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16294, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16307, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16322, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16340, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16357, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16378, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16398, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16418, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16434, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16450, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16470, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(27686, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16494, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16500, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16505, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(16544, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(16555, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16564, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(16569, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(16576, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(16587, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(16594, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(16603, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(16613, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(16626, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(16639, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(27254, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(27712, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(27717, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(27722, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(27731, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(27742, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(27747, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(27762, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(27767, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(27774, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(27779, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(27784, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(27790, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(27794, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(27799, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(27815, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(17479, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(27821, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(27566, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(27828, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(27843, 8)/* Private */,IR(0, 0),IR(205, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(27851, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(27860, 8)/* Generic */,IR(0, 0),IR(207, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(27868, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(27881, 14)/* AtomicCounter */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(27895, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(27901, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(199, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(27915, 13)/* TileImageEXT */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(27928, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(42, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(27947, 16)/* NodePayloadAMDX */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(27963, 16)/* CallableDataKHR */,IR(162, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(27994, 24)/* IncomingCallableDataKHR */,IR(163, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(28041, 14)/* RayPayloadKHR */,IR(164, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(28068, 16)/* HitAttributeKHR */,IR(165, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(28099, 22)/* IncomingRayPayloadKHR */,IR(166, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(28142, 22)/* ShaderRecordBufferKHR */,IR(167, 1),IR(56, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(28185, 22)/* PhysicalStorageBuffer */,IR(168, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(28232, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(28253, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(55, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(28277, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(28299, 17)/* CodeSectionINTEL */,IR(0, 0),IR(208, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(28316, 17)/* DeviceOnlyALTERA */,IR(169, 1),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(28349, 15)/* HostOnlyALTERA */,IR(170, 1),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(25204, 14)/* UncachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(28378, 18)/* WriteThroughINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(28396, 15)/* WriteBackINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(25230, 15)/* StreamingINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(16190, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(28411, 11)/* TensorView */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(28422, 11)/* DecodeFunc */,IR(0, 0),IR(211, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(28433, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(17437, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(27199, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(27217, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(27224, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(28443, 8)/* NoneARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(28451, 15)/* NontemporalARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(28466, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(28486, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(28510, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(28532, 21)/* NonPrivateElementARM */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(7, 129);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(136, 290);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(426, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(434, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(437, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(439, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(458, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(468, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(472, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(487, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(491, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(497, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(500, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(503, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(507, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(515, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(518, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(534, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(543, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(546, 150);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(696, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(704, 98);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(802, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(819, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(821, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(824, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(826, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(835, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(839, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(843, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(849, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(858, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(865, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(869, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(886, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(912, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(932, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(934, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(937, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(939, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(941, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(944, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(949, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(969, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(984, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(993, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(997, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1012, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1013, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1017, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1018, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1026, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1029, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1041, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1043, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1046, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1048, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1053, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1055, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1097, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1104, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1107, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1108, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1116, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1119, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1121, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1141, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1151, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1155, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1169, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1198, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1202, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1205, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1210, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(869, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(984, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1017, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(491, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(969, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1026, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(821, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1210, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(136, 290);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1391> kOperandNames{{
  {IR(5632, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5658, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5648, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5668, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5676, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5697, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5708, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5732, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7737, 13), 95}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7750, 12), 95}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7083, 20), 69}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7103, 28), 70}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7785, 20), 96}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7805, 19), 96}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7131, 26), 71}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7228, 22), 75}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7157, 19), 72}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7176, 27), 73}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7203, 25), 74}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6887, 13), 60}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6861, 11), 59}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5816, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7644, 22), 90}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8943, 12), 134}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6625, 13), 50}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6599, 10), 49}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6638, 13), 51}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5829, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7666, 22), 91}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8996, 12), 135}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7992, 17), 102}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8396, 17), 116}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6957, 12), 63}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6900, 10), 61}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6343, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6113, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6199, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7870, 23), 98}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7824, 12), 97}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7250, 18), 76}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6553, 25), 48}, // 40 FragmentCoverageMaskMESA in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7836, 15), 97}, // 41 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6134, 12), 23}, // 42 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7556, 16), 86}, // 43 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6284, 19), 33}, // 44 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6378, 15), 39}, // 45 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6365, 13), 38}, // 46 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6332, 11), 36}, // 47 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6209, 17), 28}, // 48 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8698, 11), 123}, // 49 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8656, 14), 122}, // 50 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8878, 33), 131}, // 51 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8844, 34), 130}, // 52 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8375, 11), 115}, // 53 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8386, 10), 115}, // 54 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8826, 18), 129}, // 55 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8929, 14), 133}, // 56 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8560, 37), 119}, // 57 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8493, 34), 118}, // 58 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8748, 20), 124}, // 59 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8911, 18), 132}, // 60 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8368, 7), 114}, // 61 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8436, 30), 117}, // 62 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8597, 20), 120}, // 63 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8617, 19), 120}, // 64 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8257, 23), 111}, // 65 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8280, 22), 111}, // 66 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5851, 11), 12}, // 67 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6539, 14), 47}, // 68 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5952, 13), 14}, // 69 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7893, 22), 98}, // 70 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8009, 12), 103}, // 71 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8021, 11), 103}, // 72 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8032, 14), 104}, // 73 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8046, 13), 104}, // 74 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5965, 6), 15}, // 75 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7688, 15), 92}, // 76 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6266, 18), 32}, // 77 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6303, 21), 34}, // 78 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7703, 16), 93}, // 79 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7719, 18), 94}, // 80 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6469, 21), 43}, // 81 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6456, 13), 42}, // 82 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6226, 14), 29}, // 83 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8172, 22), 108}, // 84 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8194, 21), 108}, // 85 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8135, 19), 107}, // 86 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8154, 18), 107}, // 87 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8302, 17), 112}, // 88 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8319, 16), 112}, // 89 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6099, 14), 20}, // 90 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6123, 11), 22}, // 91 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5806, 10), 8}, // 92 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5790, 9), 7}, // 93 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7496, 18), 84}, // 94 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7608, 17), 88}, // 95 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5862, 12), 13}, // 96 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7625, 19), 89}, // 97 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7940, 24), 100}, // 98 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7915, 25), 99}, // 99 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6910, 24), 62}, // 100 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7964, 28), 101}, // 101 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8636, 20), 121}, // 102 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8236, 11), 110}, // 103 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8247, 10), 110}, // 104 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8215, 11), 109}, // 105 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8226, 10), 109}, // 106 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7285, 29), 77}, // 107 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7381, 16), 80}, // 108 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8800, 10), 126}, // 109 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8819, 7), 128}, // 110 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6146, 9), 24}, // 111 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6188, 11), 26}, // 112 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6173, 15), 25}, // 113 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7348, 15), 79}, // 114 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7433, 20), 82}, // 115 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7472, 24), 83}, // 116 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7332, 16), 78}, // 117 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7001, 15), 65}, // 118 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6674, 15), 54}, // 119 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6689, 18), 54}, // 120 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6729, 15), 55}, // 121 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6744, 18), 55}, // 122 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6762, 15), 56}, // 123 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6777, 18), 56}, // 124 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6490, 11), 44}, // 125 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6795, 15), 57}, // 126 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6810, 18), 57}, // 127 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6501, 26), 45}, // 128 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6828, 15), 58}, // 129 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6843, 18), 58}, // 130 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6440, 16), 41}, // 131 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6393, 13), 40}, // 132 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7596, 12), 87}, // 133 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6089, 10), 19}, // 134 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6074, 15), 18}, // 135 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6059, 15), 17}, // 136 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7065, 18), 68}, // 137 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7047, 18), 67}, // 138 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7016, 15), 66}, // 139 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5842, 9), 11}, // 140 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6527, 12), 46}, // 141 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6981, 10), 64}, // 142 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6011, 14), 16}, // 143 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7397, 15), 81}, // 144 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7534, 22), 85}, // 145 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6651, 10), 52}, // 146 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8810, 9), 127}, // 147 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6661, 13), 53}, // 148 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8768, 13), 125}, // 149 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6324, 8), 35}, // 150 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6254, 12), 31}, // 151 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6240, 14), 30}, // 152 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8094, 21), 106}, // 153 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8115, 20), 106}, // 154 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8059, 18), 105}, // 155 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8077, 17), 105}, // 156 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8335, 17), 113}, // 157 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8352, 16), 113}, // 158 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(11214, 9), 268}, // 159 AbortKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5687, 10), 140}, // 160 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14444, 35), 371}, // 161 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14479, 34), 371}, // 162 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13916, 38), 359}, // 163 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13954, 37), 359}, // 164 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13851, 33), 358}, // 165 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13884, 32), 358}, // 166 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15138, 19), 396}, // 167 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13489, 9), 344}, // 168 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15055, 20), 392}, // 169 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13544, 23), 347}, // 170 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12960, 22), 319}, // 171 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14971, 20), 388}, // 172 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13498, 23), 345}, // 173 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14991, 20), 389}, // 174 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13521, 23), 346}, // 175 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9176, 14), 156}, // 176 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10563, 17), 233}, // 177 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15096, 24), 394}, // 178 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11185, 29), 267}, // 179 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11163, 22), 266}, // 180 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11147, 16), 265}, // 181 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15914, 20), 421}, // 182 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12897, 18), 316}, // 183 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14908, 16), 384}, // 184 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14646, 20), 375}, // 185 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14666, 19), 375}, // 186 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15875, 19), 419}, // 187 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5816, 13), 166}, // 188 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12528, 32), 304}, // 189 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12560, 31), 304}, // 190 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11378, 31), 283}, // 191 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11409, 30), 283}, // 192 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11223, 16), 270}, // 193 ConstantDataKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13194, 30), 331}, // 194 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10932, 32), 252}, // 195 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13087, 31), 328}, // 196 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10211, 21), 382}, // 197 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10146, 28), 215}, // 198 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12616, 20), 306}, // 199 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13118, 40), 329}, // 200 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13057, 30), 327}, // 201 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13158, 36), 330}, // 202 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12940, 20), 318}, // 203 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13224, 28), 332}, // 204 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6609, 16), 207}, // 205 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5829, 13), 167}, // 206 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15075, 21), 393}, // 207 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12739, 25), 311}, // 208 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12764, 28), 311}, // 209 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10702, 18), 239}, // 210 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10687, 15), 238}, // 211 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9623, 18), 185}, // 212 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7363, 18), 269}, // 213 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9147, 14), 154}, // 214 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6969, 12), 229}, // 215 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12792, 23), 312}, // 216 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14859, 11), 380}, // 217 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16000, 27), 424}, // 218 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15967, 33), 423}, // 219 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15934, 33), 422}, // 220 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16027, 32), 425}, // 221 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14753, 22), 378}, // 222 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14775, 25), 378}, // 223 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14800, 28), 379}, // 224 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14828, 31), 379}, // 225 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14712, 19), 377}, // 226 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14731, 22), 377}, // 227 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14870, 14), 380}, // 228 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6872, 15), 220}, // 229 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13601, 16), 350}, // 230 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14964, 7), 387}, // 231 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13831, 20), 357}, // 232 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15346, 29), 402}, // 233 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15375, 28), 402}, // 234 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14395, 25), 370}, // 235 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14420, 24), 370}, // 236 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14163, 28), 365}, // 237 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14191, 27), 365}, // 238 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15157, 30), 397}, // 239 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15187, 29), 397}, // 240 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14247, 21), 367}, // 241 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14268, 20), 367}, // 242 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14314, 41), 369}, // 243 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14355, 40), 369}, // 244 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14088, 26), 363}, // 245 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15216, 28), 398}, // 246 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15297, 25), 401}, // 247 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15322, 24), 401}, // 248 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14021, 23), 361}, // 249 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14044, 22), 361}, // 250 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14114, 25), 364}, // 251 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14139, 24), 364}, // 252 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13778, 27), 356}, // 253 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13805, 26), 356}, // 254 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14685, 14), 376}, // 255 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14699, 13), 376}, // 256 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15281, 16), 400}, // 257 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9061, 8), 145}, // 258 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9047, 14), 144}, // 259 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10987, 16), 254}, // 260 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9069, 8), 146}, // 261 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10184, 27), 217}, // 262 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10174, 10), 216}, // 263 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14949, 15), 386}, // 264 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13397, 23), 340}, // 265 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7762, 23), 282}, // 266 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11356, 22), 282}, // 267 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6578, 21), 206}, // 268 FragmentCoverageMESA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7851, 19), 284}, // 269 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7572, 24), 278}, // 270 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11025, 16), 256}, // 271 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12707, 32), 310}, // 272 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12636, 33), 307}, // 273 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12669, 38), 308}, // 274 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6934, 23), 218}, // 275 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13752, 26), 355}, // 276 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13443, 22), 342}, // 277 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15734, 22), 413}, // 278 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9468, 15), 172}, // 279 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5874, 9), 138}, // 280 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9218, 18), 159}, // 281 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11284, 28), 273}, // 282 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9681, 16), 188}, // 283 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15433, 36), 404}, // 284 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15469, 35), 404}, // 285 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15403, 30), 403}, // 286 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10137, 9), 214}, // 287 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6406, 16), 195}, // 288 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9820, 26), 197}, // 289 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6707, 22), 198}, // 290 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9900, 25), 201}, // 291 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11453, 30), 285}, // 292 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11483, 29), 285}, // 293 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9925, 20), 202}, // 294 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14924, 25), 385}, // 295 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9846, 23), 199}, // 296 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9869, 31), 200}, // 297 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9800, 20), 196}, // 298 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15797, 26), 416}, // 299 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9140, 7), 153}, // 300 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14619, 14), 374}, // 301 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14633, 13), 374}, // 302 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9537, 8), 178}, // 303 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9096, 11), 149}, // 304 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9559, 12), 181}, // 305 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9414, 15), 168}, // 306 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11339, 17), 280}, // 307 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11003, 22), 255}, // 308 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9236, 20), 160}, // 309 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9571, 13), 182}, // 310 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9122, 12), 151}, // 311 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9612, 11), 184}, // 312 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9107, 15), 150}, // 313 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11041, 21), 258}, // 314 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9446, 10), 170}, // 315 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13465, 24), 343}, // 316 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9488, 16), 174}, // 317 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11598, 36), 288}, // 318 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11634, 39), 288}, // 319 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12139, 39), 295}, // 320 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12178, 42), 295}, // 321 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9190, 6), 157}, // 322 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11120, 27), 264}, // 323 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11106, 14), 263}, // 324 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9077, 6), 147}, // 325 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9083, 13), 148}, // 326 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11062, 14), 259}, // 327 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9483, 5), 173}, // 328 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13420, 23), 341}, // 329 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9641, 22), 186}, // 330 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5641, 7), 142}, // 331 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14066, 22), 362}, // 332 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9030, 8), 141}, // 333 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9161, 15), 155}, // 334 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15011, 20), 390}, // 335 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13020, 14), 325}, // 336 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14218, 15), 366}, // 337 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14233, 14), 366}, // 338 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15850, 25), 418}, // 339 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9023, 7), 136}, // 340 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14288, 26), 368}, // 341 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5937, 15), 281}, // 342 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5923, 14), 279}, // 343 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9520, 7), 176}, // 344 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6991, 10), 230}, // 345 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6025, 14), 191}, // 346 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9775, 13), 193}, // 347 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15031, 11), 391}, // 348 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15042, 13), 391}, // 349 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7514, 20), 277}, // 350 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5759, 31), 303}, // 351 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12494, 34), 303}, // 352 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9788, 12), 194}, // 353 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9134, 6), 152}, // 354 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11239, 16), 271}, // 355 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15756, 18), 414}, // 356 PredicatedIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13000, 20), 324}, // 357 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11091, 15), 262}, // 358 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12982, 18), 321}, // 359 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9008, 15), 381}, // 360 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10800, 12), 244}, // 361 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12915, 25), 317}, // 362 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10777, 23), 243}, // 363 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8955, 41), 333}, // 364 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8527, 33), 320}, // 365 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5909, 14), 247}, // 366 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8709, 39), 323}, // 367 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8413, 23), 300}, // 368 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5896, 13), 299}, // 369 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12815, 29), 313}, // 370 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8466, 27), 298}, // 371 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12591, 25), 305}, // 372 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8670, 28), 322}, // 373 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10831, 32), 246}, // 374 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15894, 20), 420}, // 375 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14884, 24), 383}, // 376 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13376, 21), 339}, // 377 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15774, 23), 415}, // 378 RoundedDivideSqrtINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10745, 16), 241}, // 379 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10761, 16), 242}, // 380 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14560, 30), 373}, // 381 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14590, 29), 373}, // 382 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11549, 23), 287}, // 383 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11572, 26), 287}, // 384 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11255, 29), 272}, // 385 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10580, 28), 234}, // 386 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6155, 18), 169}, // 387 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9527, 10), 177}, // 388 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9545, 14), 180}, // 389 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9429, 17), 179}, // 390 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9314, 33), 163}, // 391 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11912, 36), 292}, // 392 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11948, 39), 292}, // 393 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9456, 12), 171}, // 394 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5799, 7), 137}, // 395 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13034, 23), 326}, // 396 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11076, 15), 260}, // 397 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7314, 18), 261}, // 398 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12870, 27), 315}, // 399 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12844, 26), 314}, // 400 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5971, 12), 203}, // 401 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11512, 17), 286}, // 402 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11529, 20), 286}, // 403 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8781, 19), 309}, // 404 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7453, 19), 276}, // 405 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6039, 20), 204}, // 406 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5983, 28), 274}, // 407 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11312, 27), 274}, // 408 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7412, 21), 275}, // 409 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11439, 14), 284}, // 410 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10720, 25), 240}, // 411 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9504, 16), 175}, // 412 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15713, 21), 412}, // 413 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15120, 18), 395}, // 414 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7268, 17), 257}, // 415 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10368, 25), 225}, // 416 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10608, 24), 235}, // 417 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9347, 34), 164}, // 418 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11987, 37), 293}, // 419 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12024, 40), 293}, // 420 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9381, 33), 165}, // 421 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12064, 36), 294}, // 422 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12100, 39), 294}, // 423 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9584, 28), 183}, // 424 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9256, 24), 161}, // 425 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9697, 30), 189}, // 426 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9727, 31), 190}, // 427 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10495, 21), 228}, // 428 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10473, 22), 227}, // 429 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10666, 21), 237}, // 430 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10060, 37), 212}, // 431 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10097, 40), 213}, // 432 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11754, 39), 290}, // 433 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11793, 42), 290}, // 434 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12307, 42), 297}, // 435 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12349, 45), 297}, // 436 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10456, 17), 226}, // 437 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10393, 28), 225}, // 438 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15532, 23), 406}, // 439 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15555, 30), 407}, // 440 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15585, 30), 408}, // 441 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13688, 39), 353}, // 442 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13617, 33), 351}, // 443 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13650, 38), 352}, // 444 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6422, 18), 219}, // 445 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13292, 27), 336}, // 446 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15504, 28), 405}, // 447 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9758, 17), 192}, // 448 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13319, 26), 337}, // 449 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13345, 31), 338}, // 450 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15615, 38), 409}, // 451 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13271, 21), 335}, // 452 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10352, 16), 224}, // 453 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15244, 19), 399}, // 454 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15263, 18), 399}, // 455 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13252, 19), 334}, // 456 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15823, 27), 417}, // 457 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10049, 11), 211}, // 458 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15653, 28), 410}, // 459 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5883, 13), 139}, // 460 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9196, 22), 158}, // 461 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10964, 23), 253}, // 462 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10910, 22), 250}, // 463 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10889, 21), 249}, // 464 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10863, 26), 248}, // 465 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9963, 28), 208}, // 466 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9991, 28), 209}, // 467 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10019, 30), 210}, // 468 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7031, 16), 251}, // 469 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9663, 18), 187}, // 470 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14513, 24), 372}, // 471 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14537, 23), 372}, // 472 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10421, 35), 226}, // 473 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10632, 34), 236}, // 474 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9280, 34), 162}, // 475 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11835, 37), 291}, // 476 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11872, 40), 291}, // 477 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9945, 18), 205}, // 478 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11673, 39), 289}, // 479 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11712, 42), 289}, // 480 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12220, 42), 296}, // 481 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12262, 45), 296}, // 482 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13991, 30), 360}, // 483 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10812, 19), 245}, // 484 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15681, 32), 411}, // 485 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13727, 25), 354}, // 486 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10546, 17), 232}, // 487 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10516, 30), 231}, // 488 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9038, 9), 143}, // 489 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13586, 15), 349}, // 490 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13567, 19), 348}, // 491 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12394, 18), 301}, // 492 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12433, 29), 302}, // 493 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12462, 32), 302}, // 494 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12412, 21), 301}, // 495 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10308, 44), 223}, // 496 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10265, 43), 222}, // 497 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10232, 33), 221}, // 498 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16071, 8), 427}, // 499 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16079, 8), 428}, // 500 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16087, 6), 429}, // 501 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16093, 7), 430}, // 502 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16100, 11), 431}, // 503 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16111, 9), 432}, // 504 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16120, 13), 433}, // 505 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16059, 12), 426}, // 506 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16133, 6), 434}, // 507 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16139, 10), 435}, // 508 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16149, 6), 436}, // 509 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16170, 20), 438}, // 510 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16155, 15), 437}, // 511 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16279, 15), 446}, // 512 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16294, 13), 447}, // 513 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16267, 12), 445}, // 514 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16357, 21), 451}, // 515 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16250, 17), 444}, // 516 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16434, 16), 455}, // 517 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16238, 12), 443}, // 518 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16418, 16), 454}, // 519 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16211, 14), 441}, // 520 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16195, 16), 440}, // 521 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16225, 13), 442}, // 522 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16378, 20), 452}, // 523 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16322, 18), 449}, // 524 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16307, 15), 448}, // 525 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16398, 20), 453}, // 526 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16340, 17), 450}, // 527 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16470, 24), 457}, // 528 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16450, 20), 456}, // 529 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16190, 5), 439}, // 530 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16555, 9), 462}, // 531 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16587, 7), 466}, // 532 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16494, 6), 458}, // 533 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16594, 9), 467}, // 534 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16505, 6), 460}, // 535 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16500, 5), 459}, // 536 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16544, 11), 461}, // 537 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16576, 11), 465}, // 538 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16564, 5), 463}, // 539 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16569, 7), 464}, // 540 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16639, 11), 471}, // 541 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16603, 10), 468}, // 542 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16626, 13), 470}, // 543 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16613, 13), 469}, // 544 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16650, 10), 472}, // 545 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16660, 10), 473}, // 546 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16670, 10), 474}, // 547 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16838, 12), 485}, // 548 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16850, 12), 486}, // 549 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16693, 14), 476}, // 550 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16707, 14), 477}, // 551 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16721, 14), 478}, // 552 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16680, 13), 475}, // 553 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16798, 19), 483}, // 554 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16750, 16), 480}, // 555 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16766, 16), 481}, // 556 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16782, 16), 482}, // 557 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16735, 15), 479}, // 558 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16817, 21), 484}, // 559 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16914, 28), 490}, // 560 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16874, 15), 488}, // 561 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16889, 25), 489}, // 562 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16862, 12), 487}, // 563 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16950, 27), 492}, // 564 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16977, 27), 493}, // 565 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17004, 27), 494}, // 566 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17031, 32), 495}, // 567 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16942, 8), 491}, // 568 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17063, 26), 496}, // 569 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17100, 4), 499}, // 570 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17093, 7), 498}, // 571 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17089, 4), 497}, // 572 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17104, 11), 500}, // 573 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17126, 21), 502}, // 574 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17115, 11), 501}, // 575 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17158, 14), 504}, // 576 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17172, 21), 505}, // 577 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17147, 11), 503}, // 578 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17193, 18), 506}, // 579 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16071, 8), 508}, // 580 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16079, 8), 509}, // 581 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16087, 6), 510}, // 582 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16093, 7), 511}, // 583 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16100, 11), 512}, // 584 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16111, 9), 513}, // 585 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16120, 13), 514}, // 586 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16059, 12), 507}, // 587 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16133, 6), 515}, // 588 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16139, 10), 516}, // 589 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16149, 6), 517}, // 590 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16279, 15), 525}, // 591 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16294, 13), 526}, // 592 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16267, 12), 524}, // 593 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16357, 21), 530}, // 594 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16250, 17), 523}, // 595 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16238, 12), 522}, // 596 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16418, 16), 533}, // 597 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16211, 14), 520}, // 598 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16195, 16), 519}, // 599 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16225, 13), 521}, // 600 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16378, 20), 531}, // 601 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16322, 18), 528}, // 602 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16307, 15), 527}, // 603 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16398, 20), 532}, // 604 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16340, 17), 529}, // 605 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16190, 5), 518}, // 606 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16555, 9), 538}, // 607 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16587, 7), 542}, // 608 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16494, 6), 534}, // 609 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16505, 6), 536}, // 610 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16500, 5), 535}, // 611 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16544, 11), 537}, // 612 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16576, 11), 541}, // 613 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16564, 5), 539}, // 614 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16569, 7), 540}, // 615 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16603, 10), 543}, // 616 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(16626, 13), 545}, // 617 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(16613, 13), 544}, // 618 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(19931, 16), 666}, // 619 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17420, 8), 565}, // 620 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(18581, 15), 621}, // 621 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(18596, 18), 621}, // 622 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17898, 10), 589}, // 623 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(17942, 12), 591}, // 624 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(17271, 12), 552}, // 625 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18322, 17), 608}, // 626 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19481, 15), 654}, // 627 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19496, 14), 654}, // 628 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18690, 7), 625}, // 629 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19194, 16), 647}, // 630 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19210, 15), 647}, // 631 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17575, 8), 578}, // 632 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(18715, 16), 627}, // 633 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18697, 18), 626}, // 634 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17235, 6), 548}, // 635 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(18082, 22), 598}, // 636 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18060, 22), 597}, // 637 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18746, 13), 629}, // 638 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18731, 15), 628}, // 639 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17241, 12), 549}, // 640 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(20091, 21), 671}, // 641 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20112, 20), 671}, // 642 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17352, 8), 557}, // 643 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(19650, 20), 659}, // 644 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19670, 19), 659}, // 645 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17318, 8), 556}, // 646 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(21338, 22), 694}, // 647 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21397, 23), 695}, // 648 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19689, 16), 660}, // 649 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19705, 15), 660}, // 650 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17385, 9), 561}, // 651 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(18799, 13), 632}, // 652 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17446, 9), 568}, // 653 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(17262, 9), 551}, // 654 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(17559, 10), 576}, // 655 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(21285, 17), 693}, // 656 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20514, 28), 681}, // 657 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20542, 27), 681}, // 658 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17437, 9), 567}, // 659 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(18945, 14), 639}, // 660 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(17583, 14), 579}, // 661 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(19720, 29), 661}, // 662 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19749, 28), 661}, // 663 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19303, 17), 650}, // 664 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19320, 16), 650}, // 665 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18104, 18), 599}, // 666 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(17768, 15), 585}, // 667 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(20376, 26), 678}, // 668 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17718, 15), 584}, // 669 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17374, 5), 559}, // 670 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(19510, 21), 655}, // 671 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19531, 20), 655}, // 672 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17670, 14), 583}, // 673 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(18856, 21), 635}, // 674 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19087, 24), 643}, // 675 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20205, 31), 673}, // 676 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19030, 26), 642}, // 677 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19841, 26), 664}, // 678 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19867, 25), 664}, // 679 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17307, 11), 555}, // 680 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(17296, 11), 554}, // 681 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(18919, 26), 638}, // 682 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18659, 31), 624}, // 683 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18629, 30), 623}, // 684 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18959, 24), 639}, // 685 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18996, 19), 640}, // 686 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21136, 16), 690}, // 687 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20132, 20), 672}, // 688 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20152, 19), 672}, // 689 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21228, 29), 692}, // 690 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21257, 28), 692}, // 691 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17569, 6), 577}, // 692 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(21199, 15), 691}, // 693 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21214, 14), 691}, // 694 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19960, 25), 668}, // 695 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19985, 24), 668}, // 696 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17877, 21), 588}, // 697 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(17401, 10), 563}, // 698 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(20453, 31), 680}, // 699 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20484, 30), 680}, // 700 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20402, 26), 679}, // 701 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20428, 25), 679}, // 702 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17845, 18), 586}, // 703 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(17550, 9), 575}, // 704 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(20632, 34), 683}, // 705 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20666, 33), 683}, // 706 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20699, 31), 684}, // 707 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20730, 30), 684}, // 708 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20760, 29), 685}, // 709 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20789, 28), 685}, // 710 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20920, 30), 687}, // 711 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20950, 29), 687}, // 712 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20851, 35), 686}, // 713 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20886, 34), 686}, // 714 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20979, 33), 688}, // 715 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21012, 32), 688}, // 716 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19892, 20), 665}, // 717 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19912, 19), 665}, // 718 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17283, 13), 553}, // 719 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17908, 14), 590}, // 720 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(17954, 16), 592}, // 721 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(20009, 21), 669}, // 722 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20030, 20), 669}, // 723 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19225, 23), 648}, // 724 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19248, 22), 648}, // 725 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19336, 20), 651}, // 726 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19356, 19), 651}, // 727 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20296, 18), 676}, // 728 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18614, 15), 622}, // 729 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19140, 13), 645}, // 730 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19153, 12), 645}, // 731 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19416, 12), 653}, // 732 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19428, 11), 653}, // 733 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19947, 13), 667}, // 734 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17863, 14), 587}, // 735 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(17360, 14), 558}, // 736 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(18014, 13), 594}, // 737 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18027, 15), 595}, // 738 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18154, 20), 601}, // 739 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18122, 32), 600}, // 740 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17467, 12), 570}, // 741 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(18521, 11), 619}, // 742 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18532, 14), 619}, // 743 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17455, 12), 569}, // 744 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(19165, 15), 646}, // 745 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19180, 14), 646}, // 746 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17597, 7), 580}, // 747 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(18339, 12), 609}, // 748 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18365, 19), 611}, // 749 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18384, 14), 612}, // 750 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17379, 6), 560}, // 751 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(18294, 28), 607}, // 752 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18269, 25), 606}, // 753 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18217, 25), 604}, // 754 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18197, 20), 603}, // 755 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18242, 27), 605}, // 756 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18445, 16), 615}, // 757 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18461, 15), 615}, // 758 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18486, 10), 617}, // 759 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18496, 13), 618}, // 760 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18509, 12), 618}, // 761 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18476, 10), 616}, // 762 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20050, 21), 670}, // 763 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20071, 20), 670}, // 764 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19777, 15), 662}, // 765 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19792, 14), 662}, // 766 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18773, 26), 631}, // 767 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19111, 15), 644}, // 768 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19126, 14), 644}, // 769 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20569, 32), 682}, // 770 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20601, 31), 682}, // 771 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17211, 17), 546}, // 772 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(17411, 9), 564}, // 773 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(18546, 16), 620}, // 774 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(18562, 19), 620}, // 775 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17253, 9), 550}, // 776 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(18759, 14), 630}, // 777 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17394, 7), 562}, // 778 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(17523, 20), 573}, // 779 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(17970, 44), 593}, // 780 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18417, 28), 614}, // 781 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18812, 17), 633}, // 782 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19375, 21), 652}, // 783 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19396, 20), 652}, // 784 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20236, 25), 674}, // 785 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19270, 17), 649}, // 786 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19287, 16), 649}, // 787 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17228, 7), 547}, // 788 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(21044, 27), 689}, // 789 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21071, 26), 689}, // 790 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18904, 15), 637}, // 791 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19806, 18), 663}, // 792 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19824, 17), 663}, // 793 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20314, 16), 677}, // 794 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20330, 15), 677}, // 795 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17543, 7), 574}, // 796 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(19551, 17), 656}, // 797 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19568, 16), 656}, // 798 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18174, 23), 602}, // 799 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19613, 19), 658}, // 800 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19632, 18), 658}, // 801 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18351, 14), 610}, // 802 UTFEncodedKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(17479, 8), 571}, // 803 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(17513, 10), 572}, // 804 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(18983, 13), 640}, // 805 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(19015, 15), 641}, // 806 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20261, 35), 675}, // 807 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18877, 27), 636}, // 808 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18829, 27), 634}, // 809 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18398, 19), 613}, // 810 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17428, 9), 566}, // 811 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(18042, 18), 596}, // 812 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19584, 15), 657}, // 813 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19599, 14), 657}, // 814 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17604, 10), 581}, // 815 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(17614, 10), 582}, // 816 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(21420, 3), 696}, // 817 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21423, 3), 697}, // 818 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21426, 3), 698}, // 819 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21439, 7), 701}, // 820 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21429, 5), 699}, // 821 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21434, 5), 700}, // 822 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21446, 12), 702}, // 823 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21458, 17), 703}, // 824 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22576, 20), 771}, // 825 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22270, 15), 756}, // 826 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21901, 15), 734}, // 827 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10702, 18), 748}, // 828 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10687, 15), 747}, // 829 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21671, 13), 717}, // 830 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21684, 10), 718}, // 831 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21656, 15), 716}, // 832 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21694, 15), 719}, // 833 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22711, 25), 775}, // 834 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22736, 24), 775}, // 835 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22664, 24), 774}, // 836 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22688, 23), 774}, // 837 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22218, 29), 754}, // 838 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21623, 19), 713}, // 839 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23221, 18), 795}, // 840 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21928, 10), 736}, // 841 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23036, 26), 787}, // 842 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23062, 27), 788}, // 843 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21916, 12), 735}, // 844 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21745, 11), 723}, // 845 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21756, 20), 724}, // 846 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21733, 12), 722}, // 847 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21786, 24), 726}, // 848 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21475, 12), 704}, // 849 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22285, 15), 757}, // 850 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21816, 9), 728}, // 851 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21709, 10), 720}, // 852 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21719, 14), 721}, // 853 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21996, 16), 741}, // 854 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21984, 12), 740}, // 855 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22300, 21), 758}, // 856 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22345, 21), 761}, // 857 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23111, 16), 790}, // 858 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23089, 22), 789}, // 859 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23197, 24), 794}, // 860 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23312, 22), 799}, // 861 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23334, 24), 800}, // 862 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23289, 23), 798}, // 863 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23409, 27), 801}, // 864 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23127, 20), 791}, // 865 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22012, 34), 742}, // 866 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22046, 34), 743}, // 867 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22080, 36), 744}, // 868 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22164, 34), 752}, // 869 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23147, 22), 792}, // 870 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21607, 16), 712}, // 871 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21591, 16), 711}, // 872 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21853, 16), 731}, // 873 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22596, 15), 772}, // 874 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22611, 14), 772}, // 875 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21840, 13), 730}, // 876 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22625, 20), 773}, // 877 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22645, 19), 773}, // 878 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21869, 20), 732}, // 879 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22760, 19), 776}, // 880 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22779, 18), 776}, // 881 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21825, 15), 729}, // 882 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21572, 19), 710}, // 883 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22797, 25), 777}, // 884 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22822, 27), 778}, // 885 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21642, 10), 714}, // 886 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22146, 18), 746}, // 887 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22517, 19), 768}, // 888 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21810, 6), 727}, // 889 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23263, 26), 797}, // 890 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22536, 20), 769}, // 891 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10745, 16), 750}, // 892 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23015, 21), 786}, // 893 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22994, 21), 785}, // 894 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10761, 16), 751}, // 895 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22849, 26), 779}, // 896 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22875, 28), 780}, // 897 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23169, 28), 793}, // 898 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(13034, 23), 783}, // 899 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7332, 16), 760}, // 900 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22903, 31), 781}, // 901 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22934, 33), 782}, // 902 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22967, 27), 784}, // 903 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22556, 20), 770}, // 904 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10720, 25), 749}, // 905 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21487, 13), 705}, // 906 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21500, 22), 706}, // 907 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21522, 21), 707}, // 908 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22321, 24), 759}, // 909 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22470, 25), 766}, // 910 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22394, 26), 763}, // 911 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22495, 22), 767}, // 912 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22420, 23), 764}, // 913 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22247, 23), 755}, // 914 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22443, 27), 765}, // 915 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22366, 28), 762}, // 916 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23239, 24), 796}, // 917 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6393, 13), 737}, // 918 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22116, 30), 745}, // 919 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21938, 22), 738}, // 920 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21960, 24), 739}, // 921 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22198, 20), 753}, // 922 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21776, 10), 725}, // 923 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21889, 12), 733}, // 924 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21557, 15), 709}, // 925 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21543, 14), 708}, // 926 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21652, 4), 715}, // 927 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23574, 10), 813}, // 928 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23584, 9), 813}, // 929 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23635, 12), 816}, // 930 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23647, 11), 816}, // 931 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23593, 14), 814}, // 932 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23607, 13), 814}, // 933 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(16594, 9), 806}, // 934 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23486, 10), 807}, // 935 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5874, 9), 805}, // 936 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23543, 16), 812}, // 937 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23559, 15), 812}, // 938 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5641, 7), 808}, // 939 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23666, 8), 818}, // 940 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23503, 7), 810}, // 941 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23620, 8), 815}, // 942 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23628, 7), 815}, // 943 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23510, 17), 811}, // 944 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23527, 16), 811}, // 945 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23658, 8), 817}, // 946 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23496, 7), 809}, // 947 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23443, 20), 803}, // 948 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23463, 23), 804}, // 949 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23436, 7), 802}, // 950 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23683, 12), 820}, // 951 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(23674, 9), 819}, // 952 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(23695, 12), 821}, // 953 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(23707, 14), 822}, // 954 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(23721, 14), 823}, // 955 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(23740, 4), 825}, // 956 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(23735, 5), 824}, // 957 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(23778, 14), 832}, // 958 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23792, 23), 832}, // 959 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23815, 13), 833}, // 960 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23828, 18), 833}, // 961 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23762, 11), 830}, // 962 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23846, 15), 834}, // 963 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23773, 5), 831}, // 964 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23758, 4), 829}, // 965 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(16190, 5), 826}, // 966 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23751, 7), 828}, // 967 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23744, 7), 827}, // 968 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23861, 4), 835}, // 969 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23873, 4), 838}, // 970 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23869, 4), 837}, // 971 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23865, 4), 836}, // 972 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23909, 18), 841}, // 973 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23927, 18), 842}, // 974 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23877, 16), 839}, // 975 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23893, 16), 840}, // 976 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23968, 6), 847}, // 977 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23952, 11), 845}, // 978 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23945, 7), 844}, // 979 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(16190, 5), 843}, // 980 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15031, 11), 848}, // 981 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15042, 13), 848}, // 982 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23963, 5), 846}, // 983 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23984, 6), 851}, // 984 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23995, 8), 853}, // 985 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24003, 10), 854}, // 986 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24021, 12), 856}, // 987 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24013, 8), 855}, // 988 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24033, 21), 857}, // 989 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24054, 20), 857}, // 990 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23979, 5), 850}, // 991 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23990, 5), 852}, // 992 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23974, 5), 849}, // 993 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24109, 16), 861}, // 994 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24095, 14), 860}, // 995 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24081, 14), 859}, // 996 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24221, 28), 864}, // 997 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24249, 27), 864}, // 998 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24166, 28), 863}, // 999 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24194, 27), 863}, // 1000 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24125, 21), 862}, // 1001 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24146, 20), 862}, // 1002 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24074, 7), 858}, // 1003 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24276, 10), 865}, // 1004 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24286, 10), 866}, // 1005 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24307, 15), 868}, // 1006 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24296, 11), 867}, // 1007 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24322, 5), 870}, // 1008 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(24336, 12), 873}, // 1009 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(24348, 13), 875}, // 1010 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(24331, 5), 872}, // 1011 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(24327, 4), 871}, // 1012 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(24361, 19), 878}, // 1013 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(24380, 22), 878}, // 1014 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(24402, 17), 879}, // 1015 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(24419, 20), 879}, // 1016 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9520, 7), 877}, // 1017 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(24439, 16), 880}, // 1018 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(24455, 19), 880}, // 1019 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(16190, 5), 869}, // 1020 None in SPV_OPERAND_TYPE_IMAGE
  {IR(24527, 12), 884}, // 1021 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(17597, 7), 874}, // 1022 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(24539, 8), 885}, // 1023 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(17394, 7), 876}, // 1024 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(24505, 11), 882}, // 1025 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(24474, 14), 881}, // 1026 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(24488, 17), 881}, // 1027 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(24516, 11), 883}, // 1028 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(16087, 6), 900}, // 1029 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24708, 10), 899}, // 1030 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24643, 12), 894}, // 1031 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24655, 12), 895}, // 1032 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24632, 11), 893}, // 1033 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24557, 11), 887}, // 1034 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24547, 10), 886}, // 1035 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24617, 15), 892}, // 1036 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24729, 17), 902}, // 1037 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24746, 16), 903}, // 1038 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24879, 16), 910}, // 1039 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24895, 16), 911}, // 1040 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24578, 11), 889}, // 1041 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24718, 11), 901}, // 1042 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24802, 20), 906}, // 1043 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24568, 10), 888}, // 1044 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24603, 14), 891}, // 1045 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24589, 14), 890}, // 1046 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24822, 19), 907}, // 1047 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24841, 19), 908}, // 1048 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24860, 19), 909}, // 1049 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24680, 14), 897}, // 1050 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24694, 14), 898}, // 1051 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24667, 13), 896}, // 1052 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24762, 20), 904}, // 1053 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24782, 20), 905}, // 1054 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24913, 2), 913}, // 1055 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25014, 5), 931}, // 1056 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24935, 5), 919}, // 1057 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24930, 5), 918}, // 1058 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24972, 6), 925}, // 1059 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24978, 13), 926}, // 1060 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24940, 10), 920}, // 1061 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24950, 10), 921}, // 1062 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24911, 2), 912}, // 1063 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24918, 3), 915}, // 1064 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24915, 3), 914}, // 1065 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24921, 4), 916}, // 1066 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24925, 5), 917}, // 1067 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24967, 5), 924}, // 1068 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24963, 4), 923}, // 1069 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24960, 3), 922}, // 1070 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25008, 6), 930}, // 1071 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24991, 5), 927}, // 1072 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25002, 6), 929}, // 1073 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24996, 6), 928}, // 1074 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25019, 28), 932}, // 1075 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25047, 27), 932}, // 1076 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25074, 24), 933}, // 1077 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25098, 23), 933}, // 1078 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25121, 7), 934}, // 1079 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25128, 11), 935}, // 1080 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25139, 14), 936}, // 1081 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25153, 12), 938}, // 1082 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(16190, 5), 937}, // 1083 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(25165, 13), 940}, // 1084 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(16190, 5), 939}, // 1085 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(25178, 7), 941}, // 1086 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25185, 7), 942}, // 1087 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25192, 12), 943}, // 1088 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25218, 12), 945}, // 1089 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25270, 17), 948}, // 1090 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25245, 25), 947}, // 1091 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25230, 15), 946}, // 1092 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25204, 14), 944}, // 1093 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25410, 22), 961}, // 1094 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25432, 21), 961}, // 1095 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25305, 19), 952}, // 1096 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25324, 17), 953}, // 1097 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25294, 11), 951}, // 1098 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19960, 25), 959}, // 1099 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19985, 24), 959}, // 1100 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25369, 18), 956}, // 1101 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25453, 19), 963}, // 1102 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25472, 18), 963}, // 1103 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25615, 16), 967}, // 1104 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25631, 15), 967}, // 1105 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20009, 21), 960}, // 1106 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20030, 20), 960}, // 1107 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25490, 22), 964}, // 1108 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25512, 21), 964}, // 1109 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25355, 14), 955}, // 1110 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25646, 27), 968}, // 1111 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25673, 26), 968}, // 1112 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25341, 14), 954}, // 1113 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25586, 15), 966}, // 1114 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25601, 14), 966}, // 1115 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(16190, 5), 949}, // 1116 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25397, 13), 958}, // 1117 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25387, 10), 957}, // 1118 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20050, 21), 962}, // 1119 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20071, 20), 962}, // 1120 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25533, 27), 965}, // 1121 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25560, 26), 965}, // 1122 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25287, 7), 950}, // 1123 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25982, 27), 982}, // 1124 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25930, 26), 980}, // 1125 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25850, 23), 976}, // 1126 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25804, 23), 974}, // 1127 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25699, 29), 970}, // 1128 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25896, 17), 978}, // 1129 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26009, 27), 983}, // 1130 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25956, 26), 981}, // 1131 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25873, 23), 977}, // 1132 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25827, 23), 975}, // 1133 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25728, 29), 971}, // 1134 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25913, 17), 979}, // 1135 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25757, 21), 972}, // 1136 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25778, 26), 973}, // 1137 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(16190, 5), 969}, // 1138 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26169, 20), 991}, // 1139 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26036, 8), 986}, // 1140 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26044, 21), 988}, // 1141 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26065, 24), 988}, // 1142 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26089, 19), 989}, // 1143 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26108, 22), 989}, // 1144 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26189, 17), 992}, // 1145 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26130, 18), 990}, // 1146 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26148, 21), 990}, // 1147 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(16190, 5), 984}, // 1148 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(24527, 12), 987}, // 1149 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(17428, 9), 985}, // 1150 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26213, 8), 994}, // 1151 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26221, 7), 995}, // 1152 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26206, 7), 993}, // 1153 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26228, 7), 996}, // 1154 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26235, 10), 996}, // 1155 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26253, 8), 998}, // 1156 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26269, 15), 1000}, // 1157 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26373, 20), 1006}, // 1158 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26352, 21), 1005}, // 1159 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26393, 12), 1007}, // 1160 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26434, 14), 1009}, // 1161 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26448, 17), 1009}, // 1162 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26465, 12), 1010}, // 1163 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26477, 15), 1010}, // 1164 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(16190, 5), 997}, // 1165 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26405, 13), 1008}, // 1166 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26418, 16), 1008}, // 1167 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26245, 8), 997}, // 1168 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26261, 8), 999}, // 1169 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26284, 23), 1001}, // 1170 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26321, 15), 1003}, // 1171 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26307, 14), 1002}, // 1172 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(17428, 9), 1011}, // 1173 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26336, 16), 1004}, // 1174 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26492, 10), 1012}, // 1175 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(26507, 4), 1014}, // 1176 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26520, 8), 1016}, // 1177 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26511, 9), 1015}, // 1178 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26502, 5), 1013}, // 1179 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(26528, 25), 1017}, // 1180 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(26553, 28), 1017}, // 1181 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(26594, 4), 1020}, // 1182 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26627, 9), 1024}, // 1183 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26636, 13), 1025}, // 1184 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26607, 8), 1022}, // 1185 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26615, 12), 1023}, // 1186 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26598, 9), 1021}, // 1187 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26581, 4), 1018}, // 1188 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(26585, 9), 1019}, // 1189 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(16190, 5), 1026}, // 1190 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26649, 25), 1027}, // 1191 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26674, 23), 1028}, // 1192 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(26766, 27), 1034}, // 1193 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26793, 28), 1035}, // 1194 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26835, 16), 1037}, // 1195 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26821, 14), 1036}, // 1196 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26905, 30), 1040}, // 1197 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26707, 12), 1031}, // 1198 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(16942, 8), 1029}, // 1199 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26697, 10), 1030}, // 1200 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26892, 13), 1039}, // 1201 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26868, 24), 1038}, // 1202 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26742, 24), 1033}, // 1203 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26851, 17), 1038}, // 1204 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26719, 23), 1032}, // 1205 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26976, 37), 1042}, // 1206 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(26935, 41), 1041}, // 1207 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27091, 42), 1045}, // 1208 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27013, 37), 1043}, // 1209 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27050, 41), 1044}, // 1210 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27133, 33), 1046}, // 1211 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(27166, 33), 1047}, // 1212 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(27211, 6), 1050}, // 1213 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27199, 12), 1049}, // 1214 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(16190, 5), 1048}, // 1215 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27217, 7), 1051}, // 1216 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27224, 15), 1052}, // 1217 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27247, 7), 1054}, // 1218 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(27239, 8), 1053}, // 1219 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(27312, 13), 1063}, // 1220 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27354, 4), 1069}, // 1221 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27392, 9), 1074}, // 1222 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27325, 5), 1064}, // 1223 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27454, 5), 1083}, // 1224 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27525, 6), 1093}, // 1225 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27278, 5), 1058}, // 1226 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27432, 5), 1079}, // 1227 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27489, 6), 1088}, // 1228 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27542, 5), 1096}, // 1229 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27536, 6), 1095}, // 1230 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27358, 3), 1070}, // 1231 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27401, 8), 1075}, // 1232 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27459, 4), 1084}, // 1233 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27531, 5), 1094}, // 1234 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27345, 5), 1067}, // 1235 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27373, 10), 1072}, // 1236 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27306, 6), 1062}, // 1237 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27443, 6), 1081}, // 1238 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27512, 7), 1091}, // 1239 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27300, 6), 1061}, // 1240 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27437, 6), 1080}, // 1241 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27505, 7), 1090}, // 1242 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27350, 4), 1068}, // 1243 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27383, 9), 1073}, // 1244 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27449, 5), 1082}, // 1245 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27519, 6), 1092}, // 1246 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27337, 8), 1066}, // 1247 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27495, 10), 1089}, // 1248 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27330, 7), 1065}, // 1249 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27361, 12), 1071}, // 1250 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27270, 8), 1057}, // 1251 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27417, 8), 1077}, // 1252 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27472, 9), 1086}, // 1253 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27262, 8), 1056}, // 1254 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27409, 8), 1076}, // 1255 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27463, 9), 1085}, // 1256 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27283, 6), 1059}, // 1257 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27289, 11), 1060}, // 1258 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27425, 7), 1078}, // 1259 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27481, 8), 1087}, // 1260 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27254, 8), 1055}, // 1261 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27547, 12), 1097}, // 1262 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27559, 7), 1098}, // 1263 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27585, 11), 1101}, // 1264 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27596, 12), 1102}, // 1265 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27608, 15), 1102}, // 1266 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27623, 14), 1103}, // 1267 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27576, 9), 1100}, // 1268 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27566, 10), 1099}, // 1269 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(27645, 12), 1106}, // 1270 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(27637, 8), 1105}, // 1271 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(16190, 5), 1104}, // 1272 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(27657, 29), 1107}, // 1273 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(16071, 8), 1109}, // 1274 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16079, 8), 1110}, // 1275 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16087, 6), 1111}, // 1276 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16093, 7), 1112}, // 1277 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16100, 11), 1113}, // 1278 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16111, 9), 1114}, // 1279 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16120, 13), 1115}, // 1280 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16059, 12), 1108}, // 1281 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16133, 6), 1116}, // 1282 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16139, 10), 1117}, // 1283 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16149, 6), 1118}, // 1284 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16170, 20), 1120}, // 1285 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16155, 15), 1119}, // 1286 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16279, 15), 1128}, // 1287 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16294, 13), 1129}, // 1288 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16267, 12), 1127}, // 1289 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16357, 21), 1133}, // 1290 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16250, 17), 1126}, // 1291 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16434, 16), 1137}, // 1292 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16238, 12), 1125}, // 1293 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16418, 16), 1136}, // 1294 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16211, 14), 1123}, // 1295 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16195, 16), 1122}, // 1296 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16225, 13), 1124}, // 1297 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16378, 20), 1134}, // 1298 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16322, 18), 1131}, // 1299 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16307, 15), 1130}, // 1300 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16398, 20), 1135}, // 1301 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16340, 17), 1132}, // 1302 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16470, 24), 1139}, // 1303 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16450, 20), 1138}, // 1304 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(27686, 26), 1140}, // 1305 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16190, 5), 1121}, // 1306 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16555, 9), 1145}, // 1307 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16587, 7), 1149}, // 1308 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16494, 6), 1141}, // 1309 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16594, 9), 1150}, // 1310 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16505, 6), 1143}, // 1311 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16500, 5), 1142}, // 1312 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16544, 11), 1144}, // 1313 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16576, 11), 1148}, // 1314 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16564, 5), 1146}, // 1315 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16569, 7), 1147}, // 1316 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16639, 11), 1154}, // 1317 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16603, 10), 1151}, // 1318 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16626, 13), 1153}, // 1319 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(16613, 13), 1152}, // 1320 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(27747, 15), 1161}, // 1321 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27712, 5), 1156}, // 1322 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27717, 5), 1157}, // 1323 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27767, 7), 1163}, // 1324 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27742, 5), 1160}, // 1325 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27774, 5), 1164}, // 1326 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27722, 9), 1158}, // 1327 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27731, 11), 1159}, // 1328 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27794, 5), 1168}, // 1329 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27762, 5), 1162}, // 1330 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27784, 6), 1166}, // 1331 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27254, 8), 1155}, // 1332 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27779, 5), 1165}, // 1333 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27790, 4), 1167}, // 1334 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27881, 14), 1179}, // 1335 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27963, 16), 1185}, // 1336 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27979, 15), 1185}, // 1337 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28299, 17), 1195}, // 1338 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27828, 15), 1174}, // 1339 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28316, 17), 1196}, // 1340 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28333, 16), 1196}, // 1341 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27851, 9), 1176}, // 1342 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27860, 8), 1177}, // 1343 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28068, 16), 1188}, // 1344 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28084, 15), 1188}, // 1345 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28277, 22), 1194}, // 1346 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28232, 21), 1192}, // 1347 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28349, 15), 1197}, // 1348 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28364, 14), 1197}, // 1349 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27895, 6), 1180}, // 1350 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27994, 24), 1186}, // 1351 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28018, 23), 1186}, // 1352 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28099, 22), 1189}, // 1353 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28121, 21), 1189}, // 1354 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27815, 6), 1170}, // 1355 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27947, 16), 1184}, // 1356 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27821, 7), 1172}, // 1357 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28185, 22), 1191}, // 1358 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28207, 25), 1191}, // 1359 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27843, 8), 1175}, // 1360 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27868, 13), 1178}, // 1361 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28041, 14), 1187}, // 1362 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28055, 13), 1187}, // 1363 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28142, 22), 1190}, // 1364 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28164, 21), 1190}, // 1365 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27901, 14), 1181}, // 1366 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28253, 24), 1193}, // 1367 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27928, 19), 1183}, // 1368 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27915, 13), 1182}, // 1369 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(17479, 8), 1171}, // 1370 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27799, 16), 1169}, // 1371 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27566, 10), 1173}, // 1372 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25230, 15), 1201}, // 1373 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25204, 14), 1198}, // 1374 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28396, 15), 1200}, // 1375 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28378, 18), 1199}, // 1376 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28422, 11), 1204}, // 1377 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(16190, 5), 1202}, // 1378 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(28411, 11), 1203}, // 1379 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(27199, 12), 1207}, // 1380 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(17437, 9), 1206}, // 1381 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27217, 7), 1208}, // 1382 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27224, 15), 1209}, // 1383 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28433, 10), 1205}, // 1384 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28486, 24), 1213}, // 1385 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28510, 22), 1214}, // 1386 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28532, 21), 1215}, // 1387 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28443, 8), 1210}, // 1388 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28451, 15), 1211}, // 1389 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(28466, 20), 1212}, // 1390 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 151);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(159, 340);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(499, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(507, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(510, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(512, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(531, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(541, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(545, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(560, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(564, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(570, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(573, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(576, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(580, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(588, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(591, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(607, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(616, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(619, 198);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(817, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(825, 103);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(928, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(951, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(953, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(956, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(958, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(969, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(973, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(977, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(984, 10);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(994, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(1004, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1008, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1029, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1055, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1075, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1079, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1082, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1084, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1086, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1089, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1094, 30);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1124, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1139, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1151, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1156, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1175, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1176, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1180, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1182, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1190, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1193, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1206, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1208, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1211, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1213, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1218, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1220, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1262, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1270, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1273, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1274, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1282, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1285, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1287, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1307, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1317, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1321, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1335, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1373, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1377, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1380, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1385, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1008, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1139, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1180, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(564, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1124, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1190, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(953, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1385, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(159, 340);
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
static const std::array<InstructionDesc, 873> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(28553, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(28609, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(28615, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(28734, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(28741, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(28757, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(28762, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(28773, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(28780, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(28785, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(28795, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(28855, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(28927, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(29001, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(29044, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(29086, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(29097, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(29106, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(29115, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(29160, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(29170, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(29181, 11), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(29305, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(29315, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(29327, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(29344, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(29354, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(29371, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(29382, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(29424, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(29436, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(29449, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(29459, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(29475, 14), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(29489, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(29499, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(29508, 19), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(29527, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(29540, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(17437, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(29592, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(29688, 16), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(29704, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(29717, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(29734, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(29752, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(29765, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(29828, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(27851, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(29877, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(29895, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(29907, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(29920, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(29929, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(29987, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(29992, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(29998, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(30009, 16), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(30025, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(30037, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(30057, 15), IR(0, 0), IR(218, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(30072, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(30084, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(30107, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(30158, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(30167, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(30182, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(30198, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(30257, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(30277, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(30298, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(30318, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(30332, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(30351, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(30368, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(30384, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(30395, 10), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(30405, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(30450, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(30496, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(30519, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(30546, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(30573, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(30600, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(30627, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(30658, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(30689, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(30700, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(30712, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(30728, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(30738, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(27895, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(30749, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(30766, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(30782, 18), IR(0, 0), IR(222, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(30800, 15), IR(0, 0), IR(222, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(30815, 14), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(30829, 17), IR(0, 0), IR(222, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(30846, 18), IR(0, 0), IR(222, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(30864, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(30876, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(30888, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(30900, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(30912, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(30921, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(30930, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(30939, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(30953, 14), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(30967, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(30982, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(30997, 14), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(31011, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(31028, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(31045, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(31070, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(31078, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(31086, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(31094, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(31099, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(31104, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(31109, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(31114, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(31119, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(31124, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(31129, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(31134, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(31139, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(31144, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(31149, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(31154, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(31159, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(31164, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(31182, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(31200, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(31218, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(31236, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(31254, 13), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(31267, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(31271, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(31281, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(31292, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(31305, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(31318, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(31322, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(31326, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(31332, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(31338, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(31347, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(31356, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(31367, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(31381, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(31389, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(31399, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(31412, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(31428, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(31438, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(31449, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(31460, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(31467, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(31474, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(31484, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(31497, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(31510, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(31528, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(31546, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(31556, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(31566, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(31581, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(31596, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(31606, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(31618, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(31631, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(31646, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(31659, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(31674, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(31690, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(31708, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(31726, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(31746, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(31767, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(31790, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(31808, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(31829, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(31846, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(31856, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(31867, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(31878, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(31882, 15), IR(0, 0), IR(225, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(31897, 17), IR(0, 0), IR(225, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(31914, 17), IR(0, 0), IR(225, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(31931, 11), IR(0, 0), IR(225, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(31942, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(31951, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(31956, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(31961, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(31968, 9), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(31977, 9), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(31986, 11), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(31997, 11), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(32008, 11), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(32019, 13), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(32032, 11), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(32043, 13), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(32056, 17), IR(0, 0), IR(122, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(32073, 19), IR(0, 0), IR(122, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(32129, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(32144, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(32158, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(32169, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(32181, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(32196, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(32218, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(32244, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(32261, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(32278, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(32289, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(32300, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(32311, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(32322, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(32333, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(32344, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(32354, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(32363, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(32373, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(32407, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(32452, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(32467, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(32473, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(32480, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(32543, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(32550, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(32555, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(32562, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(32574, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(32586, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(32600, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(32613, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(32628, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(32644, 9), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(32653, 9), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(32662, 15), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(32710, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(32720, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(32730, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(32740, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(32750, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(32760, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(32770, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(32780, 10), IR(0, 0), IR(228, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(32790, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(32799, 10), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(32809, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(32826, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(32844, 23), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(32867, 24), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(32891, 15), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(32906, 16), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(32922, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(32939, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(32957, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(32975, 28), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(33003, 29), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(33032, 20), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(33052, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(33073, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(33087, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(33101, 30), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(33131, 32), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(33163, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(33186, 40), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(33226, 12), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(33238, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(33251, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(33267, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(33280, 19), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(33299, 26), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(33325, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(33341, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(33354, 29), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(33383, 29), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(33412, 33), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(33445, 33), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(33478, 33), IR(0, 0), IR(229, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(33511, 33), IR(0, 0), IR(229, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(33544, 37), IR(0, 0), IR(229, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(33581, 37), IR(0, 0), IR(229, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(33618, 17), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(33635, 18), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(33653, 22), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(33675, 26), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(33701, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(33708, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(33729, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(33745, 16), IR(0, 0), IR(229, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(33761, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(33768, 16), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(33784, 20), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(33804, 26), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(33830, 35), IR(0, 0), IR(163, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(33865, 25), IR(0, 0), IR(163, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(33890, 17), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(33907, 23), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(33930, 19), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(33949, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(33965, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(33981, 11), IR(0, 0), IR(1, 0), IR(191, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(33992, 21), IR(0, 0), IR(78, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(34013, 19), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(34032, 19), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(34051, 24), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(34075, 25), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(34100, 30), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6707, 22), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(34130, 29), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(34159, 32), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(34191, 30), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(34221, 29), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(34250, 29), IR(0, 0), IR(233, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(9846, 23), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(34279, 26), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(34305, 25), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(34330, 27), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(34357, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(34377, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(34397, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(34417, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(34437, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(34457, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(34477, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(34497, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(34517, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(34537, 20), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(34557, 26), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(34583, 25), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(34608, 26), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(34634, 26), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(34660, 25), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(34685, 26), IR(0, 0), IR(236, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(34711, 29), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(34740, 24), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(34764, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(34776, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(34785, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(34797, 8), IR(0, 0), IR(240, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(34805, 23), IR(0, 0), IR(155, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(34828, 23), IR(0, 0), IR(164, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(34851, 25), IR(0, 0), IR(165, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(34876, 14), IR(0, 0), IR(212, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(34932, 14), IR(0, 0), IR(212, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(34946, 15), IR(0, 0), IR(212, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(34961, 19), IR(0, 0), IR(212, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(34980, 17), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(34997, 19), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(10137, 9), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(35016, 14), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(35030, 18), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(35048, 12), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(35060, 13), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(35073, 20), IR(0, 0), IR(3, 1), IR(201, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(35093, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(35115, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(35134, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(35156, 30), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6422, 18), IR(0, 0), IR(244, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(35186, 27), IR(0, 0), IR(244, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(35213, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(35238, 33), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(35271, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(35293, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(35312, 7), IR(0, 0), IR(245, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(35319, 15), IR(0, 0), IR(246, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(35334, 15), IR(0, 0), IR(246, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(35349, 20), IR(0, 0), IR(246, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(14924, 25), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(35369, 26), IR(0, 0), IR(244, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(35395, 26), IR(0, 0), IR(1, 0), IR(202, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(35421, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(35446, 12), IR(0, 0), IR(62, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(35458, 19), IR(0, 0), IR(62, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(35477, 35), IR(0, 0), IR(248, 2), IR(203, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(35512, 22), IR(0, 0), IR(62, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(35534, 16), IR(0, 0), IR(62, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(35597, 5), IR(171, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(35610, 5), IR(172, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(35623, 6), IR(173, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(35638, 11), IR(174, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(35663, 11), IR(175, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(35688, 12), IR(176, 1), IR(250, 1), IR(148, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(35715, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(35740, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(35765, 26), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(35845, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(35872, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(35899, 30), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(35929, 34), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(35963, 31), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(35994, 16), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(36010, 22), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(36032, 21), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(36053, 32), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(36085, 31), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(36116, 19), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(36135, 31), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(36166, 24), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(36190, 19), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(36209, 23), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(36232, 23), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(36255, 17), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(36272, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(36301, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(36330, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(36359, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(36388, 30), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(36418, 28), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(36446, 20), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(36466, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(36489, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(36512, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(36535, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(36558, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(36581, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(36604, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(36627, 23), IR(0, 0), IR(228, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(36650, 21), IR(0, 0), IR(257, 1), IR(59, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(36671, 17), IR(0, 0), IR(257, 1), IR(59, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(36688, 13), IR(0, 0), IR(258, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(36701, 25), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(36726, 24), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(36750, 25), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(36775, 29), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(36804, 27), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(36831, 23), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(36854, 19), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(36873, 23), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(36896, 26), IR(0, 0), IR(172, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(36922, 26), IR(0, 0), IR(172, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(36948, 14), IR(0, 0), IR(45, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(36962, 17), IR(0, 0), IR(45, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAbortKHR, false, false, IR(36, 2), IR(11214, 9), IR(0, 0), IR(259, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(36979, 28), IR(0, 0), IR(45, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(37007, 20), IR(0, 0), IR(45, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(37027, 18), IR(0, 0), IR(45, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantDataKHR, true, true, IR(479, 3), IR(11223, 16), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantDataKHR, true, true, IR(479, 3), IR(37045, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(37065, 10), IR(0, 0), IR(173, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(37075, 10), IR(0, 0), IR(173, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(482, 14), IR(37085, 27), IR(0, 0), IR(260, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(496, 13), IR(37112, 36), IR(0, 0), IR(260, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(509, 7), IR(37148, 28), IR(0, 0), IR(260, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(37176, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(37204, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(37232, 33), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(37265, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(496, 13), IR(37295, 26), IR(0, 0), IR(260, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(37321, 39), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(37360, 44), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(37404, 23), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(516, 12), IR(37427, 20), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(496, 13), IR(37447, 21), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(516, 12), IR(37468, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(528, 6), IR(37498, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(37520, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(37545, 26), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(37571, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(37596, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(37618, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(37647, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(37675, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(37700, 34), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(37734, 32), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(37766, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(37795, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(37817, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(37839, 19), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(37858, 17), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(37875, 18), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(534, 3), IR(37893, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(37922, 24), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(37946, 16), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(537, 7), IR(37962, 23), IR(0, 0), IR(262, 1), IR(77, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(37985, 16), IR(177, 1), IR(263, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(544, 13), IR(38025, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(557, 7), IR(38054, 42), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(38096, 39), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(564, 16), IR(38135, 32), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(38167, 27), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(580, 4), IR(38194, 17), IR(0, 0), IR(55, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(38211, 18), IR(0, 0), IR(55, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(38229, 28), IR(178, 1), IR(193, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(38284, 33), IR(0, 0), IR(52, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(38317, 35), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(38352, 38), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(584, 5), IR(38390, 24), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(589, 4), IR(38414, 25), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(593, 5), IR(38439, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(509, 7), IR(38467, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(38490, 29), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(38519, 51), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(38570, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(38594, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(598, 4), IR(38639, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(602, 14), IR(38672, 32), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(616, 15), IR(38704, 38), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(38742, 17), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(38759, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(534, 3), IR(38784, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(516, 12), IR(38814, 21), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(496, 13), IR(38835, 27), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(38862, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(38886, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(38912, 27), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(38939, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(38965, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(38988, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(39018, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(39047, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(39073, 35), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(39108, 31), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(39139, 34), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(39173, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(39206, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(39236, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(39265, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(39294, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(39317, 22), IR(179, 1), IR(56, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(39360, 21), IR(0, 0), IR(58, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(39381, 15), IR(0, 0), IR(58, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(39396, 8), IR(0, 0), IR(58, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(516, 12), IR(39404, 14), IR(0, 0), IR(59, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(516, 12), IR(39418, 17), IR(0, 0), IR(59, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(39435, 50), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(39485, 29), IR(180, 1), IR(272, 4), IR(205, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(39542, 18), IR(0, 0), IR(58, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(39560, 35), IR(181, 1), IR(66, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(39618, 24), IR(0, 0), IR(66, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(39642, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(39665, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(39710, 40), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(39750, 20), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(39770, 18), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(39788, 19), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(631, 5), IR(39807, 24), IR(0, 0), IR(276, 1), IR(85, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(636, 6), IR(39831, 24), IR(0, 0), IR(276, 1), IR(85, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(642, 5), IR(39855, 25), IR(0, 0), IR(276, 1), IR(85, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(39880, 26), IR(0, 0), IR(276, 1), IR(85, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(39906, 26), IR(0, 0), IR(276, 1), IR(85, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(39932, 28), IR(0, 0), IR(277, 3), IR(86, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(39960, 26), IR(0, 0), IR(277, 3), IR(86, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(647, 5), IR(40029, 26), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(652, 7), IR(40130, 30), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(659, 5), IR(40160, 31), IR(0, 0), IR(210, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(40191, 32), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(40223, 19), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(664, 4), IR(40242, 17), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(40259, 21), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(40280, 27), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(40307, 24), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(40331, 20), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(40351, 28), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(40379, 19), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(40398, 25), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(40423, 22), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(12739, 25), IR(46, 1), IR(283, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(40445, 22), IR(0, 0), IR(283, 1), IR(87, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(40467, 20), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(40487, 27), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(40514, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(40543, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(40561, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(40581, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(40599, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(40619, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(40644, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(40669, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(668, 7), IR(40750, 17), IR(0, 0), IR(199, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(40767, 40), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(40807, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(40845, 38), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(40883, 34), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(40917, 37), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(40954, 29), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(40983, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(41010, 27), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(41037, 23), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(41060, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(41083, 20), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(41103, 22), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(41125, 19), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13271, 21), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(41144, 25), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(41169, 23), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(41192, 24), IR(0, 0), IR(284, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(41216, 23), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(41239, 24), IR(0, 0), IR(285, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(41263, 28), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(41291, 29), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(41320, 33), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(675, 5), IR(41353, 34), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(41387, 24), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(41411, 25), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(41436, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(41449, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(41462, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(41475, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(41488, 14), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(41502, 14), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(41516, 21), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(41537, 21), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(41558, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(41571, 13), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(41584, 15), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(41599, 15), IR(0, 0), IR(288, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(41614, 29), IR(0, 0), IR(208, 1), IR(105, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(41643, 25), IR(0, 0), IR(208, 1), IR(105, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(41668, 15), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(680, 6), IR(13489, 9), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(41683, 13), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(41696, 14), IR(0, 0), IR(289, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(41710, 14), IR(0, 0), IR(289, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(41724, 14), IR(0, 0), IR(293, 1), IR(109, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(41738, 10), IR(0, 0), IR(293, 1), IR(109, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(41748, 15), IR(182, 1), IR(1, 0), IR(209, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(41784, 21), IR(183, 1), IR(1, 0), IR(209, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(41832, 14), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(41846, 18), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(41864, 23), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(41887, 23), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(41910, 23), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(41933, 23), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(41956, 22), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(41978, 22), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42000, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42046, 44), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(42090, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(42129, 37), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(42166, 22), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(42188, 22), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(42210, 60), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(42270, 53), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(42323, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(42370, 40), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(42410, 51), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(42461, 44), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(42505, 51), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(42556, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42612, 50), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42662, 52), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(42714, 49), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(42763, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(42810, 50), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(42860, 51), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(42911, 56), IR(0, 0), IR(297, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(42967, 33), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43000, 52), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43052, 61), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(43113, 61), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(43174, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(43213, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(43251, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(43290, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(43328, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(43367, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(43405, 36), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(43441, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(43480, 43), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(43523, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(43561, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(43599, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(43637, 45), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(43682, 40), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(43722, 62), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(43784, 30), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(43814, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(43852, 36), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(43888, 33), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(43921, 35), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(43956, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(43995, 43), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(44038, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(44085, 54), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(44139, 34), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(44173, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(44220, 45), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(44265, 55), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(44320, 53), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(44373, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(44429, 54), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(44483, 58), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(44541, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(44597, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(44635, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(44681, 44), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(44725, 49), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(44774, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(44821, 70), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(44891, 68), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(44959, 69), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(45028, 68), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(45096, 66), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(45162, 67), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(45229, 36), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(45265, 48), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(45313, 59), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(45372, 58), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(45430, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(686, 9), IR(45486, 30), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(695, 10), IR(45516, 30), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(45546, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(45585, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(45631, 43), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(45674, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(45721, 45), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(45766, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(45812, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(45868, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(45906, 30), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(45936, 32), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(695, 10), IR(45968, 36), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(705, 13), IR(46004, 42), IR(0, 0), IR(297, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(46046, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46085, 39), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(46124, 44), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(46168, 48), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(46216, 50), IR(0, 0), IR(297, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46266, 43), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(46309, 48), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(46357, 43), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(46400, 31), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46431, 47), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46478, 45), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46523, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(46569, 56), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(46625, 38), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(46663, 35), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(46698, 44), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(46742, 46), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(46788, 41), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(46829, 36), IR(0, 0), IR(297, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(46865, 50), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(46915, 48), IR(0, 0), IR(295, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(46963, 35), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(13727, 25), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(46998, 16), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(47014, 19), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(718, 8), IR(47033, 29), IR(184, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(718, 8), IR(47090, 25), IR(185, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(718, 8), IR(47139, 32), IR(186, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(718, 8), IR(47202, 30), IR(187, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(726, 10), IR(47261, 24), IR(188, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(726, 10), IR(47308, 24), IR(189, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(726, 10), IR(47355, 24), IR(190, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(726, 10), IR(47402, 24), IR(191, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(736, 6), IR(47449, 23), IR(192, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(736, 6), IR(47494, 23), IR(193, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(736, 6), IR(47539, 23), IR(194, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(736, 6), IR(47584, 23), IR(195, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(736, 6), IR(47629, 23), IR(196, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(718, 8), IR(47674, 26), IR(197, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(718, 8), IR(47725, 26), IR(198, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(718, 8), IR(47776, 25), IR(199, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(726, 10), IR(47825, 26), IR(200, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(718, 8), IR(47876, 25), IR(201, 1), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(718, 8), IR(47925, 23), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(718, 8), IR(47948, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(718, 8), IR(47972, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(718, 8), IR(47997, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(718, 8), IR(48022, 23), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(718, 8), IR(48045, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(718, 8), IR(48069, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(718, 8), IR(48094, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(718, 8), IR(48119, 23), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(718, 8), IR(48142, 23), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(718, 8), IR(48165, 26), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(718, 8), IR(48191, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(718, 8), IR(48216, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(718, 8), IR(48241, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(718, 8), IR(48265, 26), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(718, 8), IR(48291, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(718, 8), IR(48315, 26), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(718, 8), IR(48341, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(718, 8), IR(48365, 26), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(726, 10), IR(48391, 25), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(726, 10), IR(48416, 23), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(726, 10), IR(48439, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(726, 10), IR(48463, 24), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(48487, 17), IR(0, 0), IR(301, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(742, 2), IR(48504, 21), IR(0, 0), IR(144, 1), IR(131, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(744, 3), IR(48525, 20), IR(0, 0), IR(144, 1), IR(131, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(48545, 24), IR(0, 0), IR(144, 1), IR(131, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(718, 8), IR(48569, 16), IR(202, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(718, 8), IR(48600, 17), IR(203, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(718, 8), IR(48633, 17), IR(204, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(718, 8), IR(48666, 15), IR(205, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(718, 8), IR(48695, 15), IR(206, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(718, 8), IR(48724, 18), IR(207, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(718, 8), IR(48759, 17), IR(208, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(718, 8), IR(48792, 17), IR(209, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(718, 8), IR(48825, 20), IR(210, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(718, 8), IR(48864, 15), IR(211, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(718, 8), IR(48893, 15), IR(212, 1), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(48922, 30), IR(213, 1), IR(209, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(48981, 30), IR(214, 1), IR(209, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, true, true, IR(172, 4), IR(49040, 23), IR(215, 1), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, true, true, IR(172, 4), IR(49085, 24), IR(216, 1), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(14685, 14), IR(62, 1), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(49132, 22), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(49154, 23), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(49177, 28), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(49205, 46), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(49251, 37), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(49288, 65), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(49353, 40), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(49393, 41), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(49434, 39), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(49473, 36), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(49509, 46), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(49555, 45), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(49600, 42), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(49642, 32), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(49674, 29), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(49703, 40), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(49743, 40), IR(0, 0), IR(202, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(49783, 14), IR(0, 0), IR(304, 4), IR(155, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(49797, 23), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(747, 1), IR(49820, 25), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(49845, 32), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(49877, 36), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(49913, 33), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(49946, 20), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(49966, 20), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(49986, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(50012, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(15138, 19), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(748, 7), IR(50036, 25), IR(217, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(50085, 24), IR(218, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(50132, 22), IR(219, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(50175, 26), IR(220, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(50226, 23), IR(221, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(50271, 27), IR(0, 0), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(755, 10), IR(50298, 25), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(755, 10), IR(50323, 34), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(755, 10), IR(50357, 34), IR(0, 0), IR(315, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(765, 9), IR(50391, 29), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(755, 10), IR(50420, 26), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(774, 7), IR(15615, 38), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(50508, 21), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(15681, 32), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(50529, 26), IR(0, 0), IR(114, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(781, 5), IR(50555, 27), IR(0, 0), IR(114, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(786, 2), IR(50582, 27), IR(0, 0), IR(114, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(788, 4), IR(50609, 24), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(792, 6), IR(50633, 30), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(798, 3), IR(50700, 30), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(50730, 27), IR(0, 0), IR(114, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPredicatedLoadINTEL, true, true, IR(636, 6), IR(50757, 20), IR(0, 0), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPredicatedStoreINTEL, false, false, IR(589, 4), IR(50777, 21), IR(0, 0), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(50798, 13), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(50811, 13), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(50824, 19), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(50843, 18), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(50861, 19), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(50880, 19), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(50899, 18), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(50917, 19), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(50936, 18), IR(0, 0), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(801, 6), IR(50954, 18), IR(0, 0), IR(323, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(807, 4), IR(50972, 19), IR(0, 0), IR(323, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(50991, 26), IR(0, 0), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(51017, 28), IR(0, 0), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(51045, 33), IR(0, 0), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(51078, 19), IR(0, 0), IR(325, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(51097, 19), IR(0, 0), IR(327, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(51116, 19), IR(0, 0), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 926> kInstructionNames{{
  {IR(11214, 9), 438}, // 0 AbortKHR
  {IR(41436, 13), 600}, // 1 AbsISubINTEL
  {IR(41449, 13), 601}, // 2 AbsUSubINTEL
  {IR(30025, 12), 58}, // 3 AccessChain
  {IR(48504, 21), 786}, // 4 AliasDomainDeclINTEL
  {IR(48525, 20), 787}, // 5 AliasScopeDeclINTEL
  {IR(48545, 24), 788}, // 6 AliasScopeListDeclINTEL
  {IR(31322, 4), 143}, // 7 All
  {IR(36701, 25), 426}, // 8 AllocateNodePayloadsAMDX
  {IR(31318, 4), 142}, // 9 Any
  {IR(48291, 24), 777}, // 10 ArbitraryFloatACosINTEL
  {IR(48315, 26), 778}, // 11 ArbitraryFloatACosPiINTEL
  {IR(48241, 24), 775}, // 12 ArbitraryFloatASinINTEL
  {IR(48265, 26), 776}, // 13 ArbitraryFloatASinPiINTEL
  {IR(48391, 25), 781}, // 14 ArbitraryFloatATan2INTEL
  {IR(48341, 24), 779}, // 15 ArbitraryFloatATanINTEL
  {IR(48365, 26), 780}, // 16 ArbitraryFloatATanPiINTEL
  {IR(47261, 24), 748}, // 17 ArbitraryFloatAddALTERA
  {IR(47285, 23), 748}, // 18 ArbitraryFloatAddINTEL
  {IR(47090, 25), 745}, // 19 ArbitraryFloatCastALTERA
  {IR(47139, 32), 746}, // 20 ArbitraryFloatCastFromIntALTERA
  {IR(47171, 31), 746}, // 21 ArbitraryFloatCastFromIntINTEL
  {IR(47115, 24), 745}, // 22 ArbitraryFloatCastINTEL
  {IR(47202, 30), 747}, // 23 ArbitraryFloatCastToIntALTERA
  {IR(47232, 29), 747}, // 24 ArbitraryFloatCastToIntINTEL
  {IR(47776, 25), 759}, // 25 ArbitraryFloatCbrtALTERA
  {IR(47801, 24), 759}, // 26 ArbitraryFloatCbrtINTEL
  {IR(48142, 23), 771}, // 27 ArbitraryFloatCosINTEL
  {IR(48216, 25), 774}, // 28 ArbitraryFloatCosPiINTEL
  {IR(47402, 24), 751}, // 29 ArbitraryFloatDivALTERA
  {IR(47426, 23), 751}, // 30 ArbitraryFloatDivINTEL
  {IR(47629, 23), 756}, // 31 ArbitraryFloatEQALTERA
  {IR(47652, 22), 756}, // 32 ArbitraryFloatEQINTEL
  {IR(48069, 25), 768}, // 33 ArbitraryFloatExp10INTEL
  {IR(48045, 24), 767}, // 34 ArbitraryFloatExp2INTEL
  {IR(48022, 23), 766}, // 35 ArbitraryFloatExpINTEL
  {IR(48094, 25), 769}, // 36 ArbitraryFloatExpm1INTEL
  {IR(47494, 23), 753}, // 37 ArbitraryFloatGEALTERA
  {IR(47517, 22), 753}, // 38 ArbitraryFloatGEINTEL
  {IR(47449, 23), 752}, // 39 ArbitraryFloatGTALTERA
  {IR(47472, 22), 752}, // 40 ArbitraryFloatGTINTEL
  {IR(47825, 26), 760}, // 41 ArbitraryFloatHypotALTERA
  {IR(47851, 25), 760}, // 42 ArbitraryFloatHypotINTEL
  {IR(47584, 23), 755}, // 43 ArbitraryFloatLEALTERA
  {IR(47607, 22), 755}, // 44 ArbitraryFloatLEINTEL
  {IR(47539, 23), 754}, // 45 ArbitraryFloatLTALTERA
  {IR(47562, 22), 754}, // 46 ArbitraryFloatLTINTEL
  {IR(47972, 25), 764}, // 47 ArbitraryFloatLog10INTEL
  {IR(47997, 25), 765}, // 48 ArbitraryFloatLog1pINTEL
  {IR(47948, 24), 763}, // 49 ArbitraryFloatLog2INTEL
  {IR(47925, 23), 762}, // 50 ArbitraryFloatLogINTEL
  {IR(47355, 24), 750}, // 51 ArbitraryFloatMulALTERA
  {IR(47379, 23), 750}, // 52 ArbitraryFloatMulINTEL
  {IR(48416, 23), 782}, // 53 ArbitraryFloatPowINTEL
  {IR(48463, 24), 784}, // 54 ArbitraryFloatPowNINTEL
  {IR(48439, 24), 783}, // 55 ArbitraryFloatPowRINTEL
  {IR(47725, 26), 758}, // 56 ArbitraryFloatRSqrtALTERA
  {IR(47751, 25), 758}, // 57 ArbitraryFloatRSqrtINTEL
  {IR(47674, 26), 757}, // 58 ArbitraryFloatRecipALTERA
  {IR(47700, 25), 757}, // 59 ArbitraryFloatRecipINTEL
  {IR(48165, 26), 772}, // 60 ArbitraryFloatSinCosINTEL
  {IR(47033, 29), 744}, // 61 ArbitraryFloatSinCosPiALTERA
  {IR(47062, 28), 744}, // 62 ArbitraryFloatSinCosPiINTEL
  {IR(48119, 23), 770}, // 63 ArbitraryFloatSinINTEL
  {IR(48191, 25), 773}, // 64 ArbitraryFloatSinPiINTEL
  {IR(47876, 25), 761}, // 65 ArbitraryFloatSqrtALTERA
  {IR(47901, 24), 761}, // 66 ArbitraryFloatSqrtINTEL
  {IR(47308, 24), 749}, // 67 ArbitraryFloatSubALTERA
  {IR(47332, 23), 749}, // 68 ArbitraryFloatSubINTEL
  {IR(15138, 19), 832}, // 69 ArithmeticFenceEXT
  {IR(30072, 12), 61}, // 70 ArrayLength
  {IR(41683, 13), 616}, // 71 AsmCallINTEL
  {IR(13489, 9), 615}, // 72 AsmINTEL
  {IR(41668, 15), 614}, // 73 AsmTargetINTEL
  {IR(41724, 14), 619}, // 74 AssumeTrueKHR
  {IR(32344, 10), 220}, // 75 AtomicAnd
  {IR(32196, 22), 210}, // 76 AtomicCompareExchange
  {IR(32218, 26), 211}, // 77 AtomicCompareExchangeWeak
  {IR(32181, 15), 209}, // 78 AtomicExchange
  {IR(49783, 14), 822}, // 79 AtomicFAddEXT
  {IR(41710, 14), 618}, // 80 AtomicFMaxEXT
  {IR(41696, 14), 617}, // 81 AtomicFMinEXT
  {IR(33729, 16), 292}, // 82 AtomicFlagClear
  {IR(33708, 21), 291}, // 83 AtomicFlagTestAndSet
  {IR(32278, 11), 214}, // 84 AtomicIAdd
  {IR(32261, 17), 213}, // 85 AtomicIDecrement
  {IR(32244, 17), 212}, // 86 AtomicIIncrement
  {IR(32289, 11), 215}, // 87 AtomicISub
  {IR(32158, 11), 207}, // 88 AtomicLoad
  {IR(32354, 9), 221}, // 89 AtomicOr
  {IR(32322, 11), 218}, // 90 AtomicSMax
  {IR(32300, 11), 216}, // 91 AtomicSMin
  {IR(32169, 12), 208}, // 92 AtomicStore
  {IR(32333, 11), 219}, // 93 AtomicUMax
  {IR(32311, 11), 217}, // 94 AtomicUMin
  {IR(32363, 10), 222}, // 95 AtomicXor
  {IR(39932, 28), 546}, // 96 BeginInvocationInterlockEXT
  {IR(36255, 17), 407}, // 97 BitCastArrayQCOM
  {IR(31942, 9), 191}, // 98 BitCount
  {IR(31882, 15), 187}, // 99 BitFieldInsert
  {IR(31897, 17), 188}, // 100 BitFieldSExtract
  {IR(31914, 17), 189}, // 101 BitFieldUExtract
  {IR(31931, 11), 190}, // 102 BitReverse
  {IR(31070, 8), 114}, // 103 Bitcast
  {IR(31867, 11), 185}, // 104 BitwiseAnd
  {IR(50508, 21), 845}, // 105 BitwiseFunctionINTEL
  {IR(31846, 10), 183}, // 106 BitwiseOr
  {IR(31856, 11), 184}, // 107 BitwiseXor
  {IR(32473, 7), 227}, // 108 Branch
  {IR(32480, 18), 228}, // 109 BranchConditional
  {IR(36962, 17), 437}, // 110 BufferPointerEXT
  {IR(33341, 13), 277}, // 111 BuildNDRange
  {IR(29086, 11), 15}, // 112 Capability
  {IR(33299, 26), 275}, // 113 CaptureEventProfilingInfo
  {IR(34805, 23), 344}, // 114 ColorAttachmentReadEXT
  {IR(32891, 15), 255}, // 115 CommitReadPipe
  {IR(32906, 16), 256}, // 116 CommitWritePipe
  {IR(30332, 19), 72}, // 117 CompositeConstruct
  {IR(49913, 33), 827}, // 118 CompositeConstructContinuedINTEL
  {IR(36388, 30), 412}, // 119 CompositeConstructCoopMatQCOM
  {IR(35963, 31), 395}, // 120 CompositeConstructReplicateEXT
  {IR(30351, 17), 73}, // 121 CompositeExtract
  {IR(36418, 28), 413}, // 122 CompositeExtractCoopMatQCOM
  {IR(30368, 16), 74}, // 123 CompositeInsert
  {IR(50582, 27), 849}, // 124 ConditionalCapabilityINTEL
  {IR(50730, 27), 853}, // 125 ConditionalCopyObjectINTEL
  {IR(50555, 27), 848}, // 126 ConditionalEntryPointINTEL
  {IR(50529, 26), 847}, // 127 ConditionalExtensionINTEL
  {IR(17437, 9), 39}, // 128 Constant
  {IR(29592, 18), 40}, // 129 ConstantComposite
  {IR(49845, 32), 825}, // 130 ConstantCompositeContinuedINTEL
  {IR(35899, 30), 393}, // 131 ConstantCompositeReplicateEXT
  {IR(11223, 16), 442}, // 132 ConstantDataKHR
  {IR(29540, 14), 38}, // 133 ConstantFalse
  {IR(41614, 29), 612}, // 134 ConstantFunctionPointerINTEL
  {IR(29704, 13), 42}, // 135 ConstantNull
  {IR(33784, 20), 296}, // 136 ConstantPipeStorage
  {IR(29688, 16), 41}, // 137 ConstantSampler
  {IR(37027, 18), 441}, // 138 ConstantSizeOfEXT
  {IR(36854, 19), 432}, // 139 ConstantStringAMDX
  {IR(29527, 13), 37}, // 140 ConstantTrue
  {IR(32129, 15), 205}, // 141 ControlBarrier
  {IR(49986, 26), 830}, // 142 ControlBarrierArriveINTEL
  {IR(50012, 24), 831}, // 143 ControlBarrierWaitINTEL
  {IR(49966, 20), 829}, // 144 ConvertBF16ToFINTEL
  {IR(49946, 20), 828}, // 145 ConvertFToBF16INTEL
  {IR(30876, 12), 100}, // 146 ConvertFToS
  {IR(30864, 12), 99}, // 147 ConvertFToU
  {IR(50991, 26), 867}, // 148 ConvertHandleToImageINTEL
  {IR(51045, 33), 869}, // 149 ConvertHandleToSampledImageINTEL
  {IR(51017, 28), 868}, // 150 ConvertHandleToSamplerINTEL
  {IR(40581, 18), 569}, // 151 ConvertImageToUNV
  {IR(30953, 14), 107}, // 152 ConvertPtrToU
  {IR(30888, 12), 101}, // 153 ConvertSToF
  {IR(40644, 25), 572}, // 154 ConvertSampledImageToUNV
  {IR(40599, 20), 570}, // 155 ConvertSamplerToUNV
  {IR(35477, 35), 379}, // 156 ConvertUToAccelerationStructureKHR
  {IR(30900, 12), 102}, // 157 ConvertUToF
  {IR(40543, 18), 567}, // 158 ConvertUToImageNV
  {IR(30997, 14), 110}, // 159 ConvertUToPtr
  {IR(40619, 25), 571}, // 160 ConvertUToSampledImageNV
  {IR(40561, 20), 568}, // 161 ConvertUToSamplerNV
  {IR(38167, 27), 485}, // 162 CooperativeMatrixConvertNV
  {IR(35872, 27), 392}, // 163 CooperativeMatrixLengthKHR
  {IR(39906, 26), 545}, // 164 CooperativeMatrixLengthNV
  {IR(35740, 25), 389}, // 165 CooperativeMatrixLoadKHR
  {IR(39831, 24), 542}, // 166 CooperativeMatrixLoadNV
  {IR(40130, 30), 549}, // 167 CooperativeMatrixLoadTensorNV
  {IR(35845, 27), 391}, // 168 CooperativeMatrixMulAddKHR
  {IR(39880, 26), 544}, // 169 CooperativeMatrixMulAddNV
  {IR(40191, 32), 551}, // 170 CooperativeMatrixPerElementOpNV
  {IR(40029, 26), 548}, // 171 CooperativeMatrixReduceNV
  {IR(35765, 26), 390}, // 172 CooperativeMatrixStoreKHR
  {IR(39855, 25), 543}, // 173 CooperativeMatrixStoreNV
  {IR(40160, 31), 550}, // 174 CooperativeMatrixStoreTensorNV
  {IR(40514, 29), 566}, // 175 CooperativeMatrixTransposeNV
  {IR(38390, 24), 492}, // 176 CooperativeVectorLoadNV
  {IR(38135, 32), 484}, // 177 CooperativeVectorMatrixMulAddNV
  {IR(38025, 29), 481}, // 178 CooperativeVectorMatrixMulNV
  {IR(38054, 42), 482}, // 179 CooperativeVectorOuterProductAccumulateNV
  {IR(38096, 39), 483}, // 180 CooperativeVectorReduceSumAccumulateNV
  {IR(38414, 25), 493}, // 181 CooperativeVectorStoreNV
  {IR(34764, 12), 340}, // 182 CopyLogical
  {IR(29998, 11), 56}, // 183 CopyMemory
  {IR(30009, 16), 57}, // 184 CopyMemorySized
  {IR(30384, 11), 75}, // 185 CopyObject
  {IR(33804, 26), 297}, // 186 CreatePipeFromPipeStorage
  {IR(40259, 21), 554}, // 187 CreateTensorLayoutNV
  {IR(40379, 19), 559}, // 188 CreateTensorViewNV
  {IR(33251, 16), 272}, // 189 CreateUserEvent
  {IR(48981, 30), 801}, // 190 CrossWorkgroupCastToPtrALTERA
  {IR(49011, 29), 801}, // 191 CrossWorkgroupCastToPtrINTEL
  {IR(31951, 5), 192}, // 192 DPdx
  {IR(31997, 11), 198}, // 193 DPdxCoarse
  {IR(31968, 9), 195}, // 194 DPdxFine
  {IR(31956, 5), 193}, // 195 DPdy
  {IR(32008, 11), 199}, // 196 DPdyCoarse
  {IR(31977, 9), 196}, // 197 DPdyFine
  {IR(30158, 9), 64}, // 198 Decorate
  {IR(33981, 11), 305}, // 199 DecorateId
  {IR(41748, 15), 621}, // 200 DecorateString
  {IR(41763, 21), 621}, // 201 DecorateStringGOOGLE
  {IR(30182, 16), 66}, // 202 DecorationGroup
  {IR(12739, 25), 562}, // 203 DemoteToHelperInvocation
  {IR(12764, 28), 562}, // 204 DemoteToHelperInvocationEXT
  {IR(34828, 23), 345}, // 205 DepthAttachmentReadEXT
  {IR(31267, 4), 137}, // 206 Dot
  {IR(38194, 17), 486}, // 207 EmitMeshTasksEXT
  {IR(32056, 17), 203}, // 208 EmitStreamVertex
  {IR(32032, 11), 201}, // 209 EmitVertex
  {IR(39960, 26), 547}, // 210 EndInvocationInterlockEXT
  {IR(32043, 13), 202}, // 211 EndPrimitive
  {IR(32073, 19), 204}, // 212 EndStreamPrimitive
  {IR(33087, 14), 265}, // 213 EnqueueKernel
  {IR(33073, 14), 264}, // 214 EnqueueMarker
  {IR(36726, 24), 427}, // 215 EnqueueNodePayloadsAMDX
  {IR(29001, 11), 13}, // 216 EntryPoint
  {IR(35458, 19), 378}, // 217 ExecuteCallableKHR
  {IR(39542, 18), 532}, // 218 ExecuteCallableNV
  {IR(29044, 14), 14}, // 219 ExecutionMode
  {IR(33965, 16), 304}, // 220 ExecutionModeId
  {IR(41738, 10), 620}, // 221 ExpectKHR
  {IR(28855, 8), 11}, // 222 ExtInst
  {IR(28795, 14), 10}, // 223 ExtInstImport
  {IR(35395, 26), 375}, // 224 ExtInstWithForwardRefsKHR
  {IR(28785, 10), 9}, // 225 Extension
  {IR(36446, 20), 414}, // 226 ExtractSubArrayQCOM
  {IR(31099, 5), 118}, // 227 FAdd
  {IR(30930, 9), 105}, // 228 FConvert
  {IR(31134, 5), 125}, // 229 FDiv
  {IR(51097, 19), 871}, // 230 FDot2MixAcc16VALVE
  {IR(51078, 19), 870}, // 231 FDot2MixAcc32VALVE
  {IR(51116, 19), 872}, // 232 FDot4MixAcc32VALVE
  {IR(31159, 5), 130}, // 233 FMod
  {IR(31119, 5), 122}, // 234 FMul
  {IR(31086, 8), 116}, // 235 FNegate
  {IR(31596, 10), 168}, // 236 FOrdEqual
  {IR(31674, 16), 174}, // 237 FOrdGreaterThan
  {IR(31746, 21), 178}, // 238 FOrdGreaterThanEqual
  {IR(31646, 13), 172}, // 239 FOrdLessThan
  {IR(31708, 18), 176}, // 240 FOrdLessThanEqual
  {IR(31618, 13), 170}, // 241 FOrdNotEqual
  {IR(14685, 14), 804}, // 242 FPGARegALTERA
  {IR(14699, 13), 804}, // 243 FPGARegINTEL
  {IR(31154, 5), 129}, // 244 FRem
  {IR(31109, 5), 120}, // 245 FSub
  {IR(31606, 12), 169}, // 246 FUnordEqual
  {IR(31690, 18), 175}, // 247 FUnordGreaterThan
  {IR(31767, 23), 179}, // 248 FUnordGreaterThanEqual
  {IR(31659, 15), 173}, // 249 FUnordLessThan
  {IR(31726, 20), 177}, // 250 FUnordLessThanEqual
  {IR(31631, 15), 171}, // 251 FUnordNotEqual
  {IR(38352, 38), 491}, // 252 FetchMicroTriangleVertexBarycentricNV
  {IR(38317, 35), 490}, // 253 FetchMicroTriangleVertexPositionNV
  {IR(36775, 29), 429}, // 254 FinishWritingNodePayloadAMDX
  {IR(48695, 15), 793}, // 255 FixedCosALTERA
  {IR(48710, 14), 793}, // 256 FixedCosINTEL
  {IR(48792, 17), 796}, // 257 FixedCosPiALTERA
  {IR(48809, 16), 796}, // 258 FixedCosPiINTEL
  {IR(48893, 15), 799}, // 259 FixedExpALTERA
  {IR(48908, 14), 799}, // 260 FixedExpINTEL
  {IR(48864, 15), 798}, // 261 FixedLogALTERA
  {IR(48879, 14), 798}, // 262 FixedLogINTEL
  {IR(48600, 17), 790}, // 263 FixedRecipALTERA
  {IR(48617, 16), 790}, // 264 FixedRecipINTEL
  {IR(48633, 17), 791}, // 265 FixedRsqrtALTERA
  {IR(48650, 16), 791}, // 266 FixedRsqrtINTEL
  {IR(48666, 15), 792}, // 267 FixedSinALTERA
  {IR(48724, 18), 794}, // 268 FixedSinCosALTERA
  {IR(48742, 17), 794}, // 269 FixedSinCosINTEL
  {IR(48825, 20), 797}, // 270 FixedSinCosPiALTERA
  {IR(48845, 19), 797}, // 271 FixedSinCosPiINTEL
  {IR(48681, 14), 792}, // 272 FixedSinINTEL
  {IR(48759, 17), 795}, // 273 FixedSinPiALTERA
  {IR(48776, 16), 795}, // 274 FixedSinPiINTEL
  {IR(48569, 16), 789}, // 275 FixedSqrtALTERA
  {IR(48585, 15), 789}, // 276 FixedSqrtINTEL
  {IR(35312, 7), 369}, // 277 FmaKHR
  {IR(36671, 17), 424}, // 278 FragmentFetchAMD
  {IR(36650, 21), 423}, // 279 FragmentMaskFetchAMD
  {IR(37075, 10), 445}, // 280 FreezeKHR
  {IR(27851, 9), 48}, // 281 Function
  {IR(29907, 13), 51}, // 282 FunctionCall
  {IR(29895, 12), 50}, // 283 FunctionEnd
  {IR(29877, 18), 49}, // 284 FunctionParameter
  {IR(41643, 25), 613}, // 285 FunctionPointerCallINTEL
  {IR(31961, 7), 194}, // 286 Fwidth
  {IR(32019, 13), 200}, // 287 FwidthCoarse
  {IR(31986, 11), 197}, // 288 FwidthFine
  {IR(31028, 17), 112}, // 289 GenericCastToPtr
  {IR(31045, 25), 113}, // 290 GenericCastToPtrExplicit
  {IR(30084, 23), 62}, // 291 GenericPtrMemSemantics
  {IR(33325, 16), 276}, // 292 GetDefaultQueue
  {IR(33830, 35), 298}, // 293 GetKernelLocalSizeForSubgroupCount
  {IR(33865, 25), 299}, // 294 GetKernelMaxNumSubgroups
  {IR(33131, 32), 267}, // 295 GetKernelNDrangeMaxSubGroupSize
  {IR(33101, 30), 266}, // 296 GetKernelNDrangeSubGroupCount
  {IR(33186, 40), 269}, // 297 GetKernelPreferredWorkGroupSizeMultiple
  {IR(33163, 23), 268}, // 298 GetKernelWorkGroupSize
  {IR(32957, 18), 259}, // 299 GetMaxPipePackets
  {IR(32939, 18), 258}, // 300 GetNumPipePackets
  {IR(10137, 9), 353}, // 301 GraphARM
  {IR(34980, 17), 351}, // 302 GraphConstantARM
  {IR(35048, 12), 356}, // 303 GraphEndARM
  {IR(34997, 19), 352}, // 304 GraphEntryPointARM
  {IR(35016, 14), 354}, // 305 GraphInputARM
  {IR(35030, 18), 355}, // 306 GraphSetOutputARM
  {IR(32644, 9), 238}, // 307 GroupAll
  {IR(32653, 9), 239}, // 308 GroupAny
  {IR(32613, 15), 236}, // 309 GroupAsyncCopy
  {IR(50824, 19), 858}, // 310 GroupBitwiseAndKHR
  {IR(50843, 18), 859}, // 311 GroupBitwiseOrKHR
  {IR(50861, 19), 860}, // 312 GroupBitwiseXorKHR
  {IR(32662, 15), 240}, // 313 GroupBroadcast
  {IR(33032, 20), 262}, // 314 GroupCommitReadPipe
  {IR(33052, 21), 263}, // 315 GroupCommitWritePipe
  {IR(30198, 14), 67}, // 316 GroupDecorate
  {IR(32720, 10), 242}, // 317 GroupFAdd
  {IR(36489, 23), 416}, // 318 GroupFAddNonUniformAMD
  {IR(32760, 10), 246}, // 319 GroupFMax
  {IR(36581, 23), 420}, // 320 GroupFMaxNonUniformAMD
  {IR(32730, 10), 243}, // 321 GroupFMin
  {IR(36512, 23), 417}, // 322 GroupFMinNonUniformAMD
  {IR(50811, 13), 857}, // 323 GroupFMulKHR
  {IR(32710, 10), 241}, // 324 GroupIAdd
  {IR(36466, 23), 415}, // 325 GroupIAddNonUniformAMD
  {IR(50798, 13), 856}, // 326 GroupIMulKHR
  {IR(50880, 19), 861}, // 327 GroupLogicalAndKHR
  {IR(50899, 18), 862}, // 328 GroupLogicalOrKHR
  {IR(50917, 19), 863}, // 329 GroupLogicalXorKHR
  {IR(30257, 20), 68}, // 330 GroupMemberDecorate
  {IR(34013, 19), 307}, // 331 GroupNonUniformAll
  {IR(34051, 24), 309}, // 332 GroupNonUniformAllEqual
  {IR(34032, 19), 308}, // 333 GroupNonUniformAny
  {IR(6707, 22), 312}, // 334 GroupNonUniformBallot
  {IR(34191, 30), 315}, // 335 GroupNonUniformBallotBitCount
  {IR(34159, 32), 314}, // 336 GroupNonUniformBallotBitExtract
  {IR(34221, 29), 316}, // 337 GroupNonUniformBallotFindLSB
  {IR(34250, 29), 317}, // 338 GroupNonUniformBallotFindMSB
  {IR(34557, 26), 332}, // 339 GroupNonUniformBitwiseAnd
  {IR(34583, 25), 333}, // 340 GroupNonUniformBitwiseOr
  {IR(34608, 26), 334}, // 341 GroupNonUniformBitwiseXor
  {IR(34075, 25), 310}, // 342 GroupNonUniformBroadcast
  {IR(34100, 30), 311}, // 343 GroupNonUniformBroadcastFirst
  {IR(33992, 21), 306}, // 344 GroupNonUniformElect
  {IR(34377, 20), 323}, // 345 GroupNonUniformFAdd
  {IR(34537, 20), 331}, // 346 GroupNonUniformFMax
  {IR(34477, 20), 328}, // 347 GroupNonUniformFMin
  {IR(34417, 20), 325}, // 348 GroupNonUniformFMul
  {IR(34357, 20), 322}, // 349 GroupNonUniformIAdd
  {IR(34397, 20), 324}, // 350 GroupNonUniformIMul
  {IR(34130, 29), 313}, // 351 GroupNonUniformInverseBallot
  {IR(34634, 26), 335}, // 352 GroupNonUniformLogicalAnd
  {IR(34660, 25), 336}, // 353 GroupNonUniformLogicalOr
  {IR(34685, 26), 337}, // 354 GroupNonUniformLogicalXor
  {IR(38229, 28), 488}, // 355 GroupNonUniformPartitionEXT
  {IR(38257, 27), 488}, // 356 GroupNonUniformPartitionNV
  {IR(36896, 26), 434}, // 357 GroupNonUniformQuadAllKHR
  {IR(36922, 26), 435}, // 358 GroupNonUniformQuadAnyKHR
  {IR(34711, 29), 338}, // 359 GroupNonUniformQuadBroadcast
  {IR(34740, 24), 339}, // 360 GroupNonUniformQuadSwap
  {IR(14924, 25), 373}, // 361 GroupNonUniformRotateKHR
  {IR(34497, 20), 329}, // 362 GroupNonUniformSMax
  {IR(34437, 20), 326}, // 363 GroupNonUniformSMin
  {IR(9846, 23), 318}, // 364 GroupNonUniformShuffle
  {IR(34330, 27), 321}, // 365 GroupNonUniformShuffleDown
  {IR(34305, 25), 320}, // 366 GroupNonUniformShuffleUp
  {IR(34279, 26), 319}, // 367 GroupNonUniformShuffleXor
  {IR(34517, 20), 330}, // 368 GroupNonUniformUMax
  {IR(34457, 20), 327}, // 369 GroupNonUniformUMin
  {IR(32975, 28), 260}, // 370 GroupReserveReadPipePackets
  {IR(33003, 29), 261}, // 371 GroupReserveWritePipePackets
  {IR(32780, 10), 248}, // 372 GroupSMax
  {IR(36627, 23), 422}, // 373 GroupSMaxNonUniformAMD
  {IR(32750, 10), 245}, // 374 GroupSMin
  {IR(36558, 23), 419}, // 375 GroupSMinNonUniformAMD
  {IR(32770, 10), 247}, // 376 GroupUMax
  {IR(36604, 23), 421}, // 377 GroupUMaxNonUniformAMD
  {IR(32740, 10), 244}, // 378 GroupUMin
  {IR(36535, 23), 418}, // 379 GroupUMinNonUniformAMD
  {IR(32628, 16), 237}, // 380 GroupWaitEvents
  {IR(38886, 26), 509}, // 381 HitObjectExecuteShaderEXT
  {IR(37520, 25), 461}, // 382 HitObjectExecuteShaderNV
  {IR(38939, 26), 511}, // 383 HitObjectGetAttributesEXT
  {IR(37571, 25), 463}, // 384 HitObjectGetAttributesNV
  {IR(39618, 24), 534}, // 385 HitObjectGetClusterIdNV
  {IR(38912, 27), 510}, // 386 HitObjectGetCurrentTimeEXT
  {IR(37545, 26), 462}, // 387 HitObjectGetCurrentTimeNV
  {IR(39018, 29), 514}, // 388 HitObjectGetGeometryIndexEXT
  {IR(37647, 28), 466}, // 389 HitObjectGetGeometryIndexNV
  {IR(38965, 23), 512}, // 390 HitObjectGetHitKindEXT
  {IR(37596, 22), 464}, // 391 HitObjectGetHitKindNV
  {IR(39073, 35), 516}, // 392 HitObjectGetInstanceCustomIndexEXT
  {IR(37700, 34), 468}, // 393 HitObjectGetInstanceCustomIndexNV
  {IR(39047, 26), 515}, // 394 HitObjectGetInstanceIdEXT
  {IR(37675, 25), 467}, // 395 HitObjectGetInstanceIdNV
  {IR(38519, 51), 497}, // 396 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(41010, 27), 582}, // 397 HitObjectGetLSSPositionsNV
  {IR(41037, 23), 583}, // 398 HitObjectGetLSSRadiiNV
  {IR(39139, 34), 518}, // 399 HitObjectGetObjectRayDirectionEXT
  {IR(37232, 33), 451}, // 400 HitObjectGetObjectRayDirectionNV
  {IR(39108, 31), 517}, // 401 HitObjectGetObjectRayOriginEXT
  {IR(37265, 30), 452}, // 402 HitObjectGetObjectRayOriginNV
  {IR(39236, 29), 521}, // 403 HitObjectGetObjectToWorldEXT
  {IR(37204, 28), 450}, // 404 HitObjectGetObjectToWorldNV
  {IR(38988, 30), 513}, // 405 HitObjectGetPrimitiveIndexEXT
  {IR(37618, 29), 465}, // 406 HitObjectGetPrimitiveIndexNV
  {IR(38570, 24), 498}, // 407 HitObjectGetRayFlagsEXT
  {IR(39294, 23), 523}, // 408 HitObjectGetRayTMaxEXT
  {IR(37795, 22), 471}, // 409 HitObjectGetRayTMaxNV
  {IR(39642, 23), 535}, // 410 HitObjectGetRayTMinEXT
  {IR(37817, 22), 472}, // 411 HitObjectGetRayTMinNV
  {IR(39665, 45), 536}, // 412 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(37360, 44), 455}, // 413 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(39710, 40), 537}, // 414 HitObjectGetShaderRecordBufferHandleEXT
  {IR(37321, 39), 454}, // 415 HitObjectGetShaderRecordBufferHandleNV
  {IR(40954, 29), 580}, // 416 HitObjectGetSpherePositionNV
  {IR(40983, 27), 581}, // 417 HitObjectGetSphereRadiusNV
  {IR(39173, 33), 519}, // 418 HitObjectGetWorldRayDirectionEXT
  {IR(37734, 32), 469}, // 419 HitObjectGetWorldRayDirectionNV
  {IR(39206, 30), 520}, // 420 HitObjectGetWorldRayOriginEXT
  {IR(37766, 29), 470}, // 421 HitObjectGetWorldRayOriginNV
  {IR(39265, 29), 522}, // 422 HitObjectGetWorldToObjectEXT
  {IR(37176, 28), 449}, // 423 HitObjectGetWorldToObjectNV
  {IR(39750, 20), 538}, // 424 HitObjectIsEmptyEXT
  {IR(37839, 19), 473}, // 425 HitObjectIsEmptyNV
  {IR(39770, 18), 539}, // 426 HitObjectIsHitEXT
  {IR(37858, 17), 474}, // 427 HitObjectIsHitNV
  {IR(41083, 20), 585}, // 428 HitObjectIsLSSHitNV
  {IR(39788, 19), 540}, // 429 HitObjectIsMissEXT
  {IR(37875, 18), 475}, // 430 HitObjectIsMissNV
  {IR(41060, 23), 584}, // 431 HitObjectIsSphereHitNV
  {IR(38862, 24), 508}, // 432 HitObjectRecordEmptyEXT
  {IR(37404, 23), 456}, // 433 HitObjectRecordEmptyNV
  {IR(38439, 28), 494}, // 434 HitObjectRecordFromQueryEXT
  {IR(37085, 27), 446}, // 435 HitObjectRecordHitMotionNV
  {IR(37447, 21), 458}, // 436 HitObjectRecordHitNV
  {IR(37112, 36), 447}, // 437 HitObjectRecordHitWithIndexMotionNV
  {IR(37468, 30), 459}, // 438 HitObjectRecordHitWithIndexNV
  {IR(38467, 23), 495}, // 439 HitObjectRecordMissEXT
  {IR(38490, 29), 496}, // 440 HitObjectRecordMissMotionEXT
  {IR(37148, 28), 448}, // 441 HitObjectRecordMissMotionNV
  {IR(37498, 22), 460}, // 442 HitObjectRecordMissNV
  {IR(38639, 33), 500}, // 443 HitObjectReorderExecuteShaderEXT
  {IR(38594, 45), 499}, // 444 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(38704, 38), 502}, // 445 HitObjectTraceMotionReorderExecuteEXT
  {IR(38814, 21), 506}, // 446 HitObjectTraceRayEXT
  {IR(38835, 27), 507}, // 447 HitObjectTraceRayMotionEXT
  {IR(37295, 26), 453}, // 448 HitObjectTraceRayMotionNV
  {IR(37427, 20), 457}, // 449 HitObjectTraceRayNV
  {IR(38672, 32), 501}, // 450 HitObjectTraceReorderExecuteEXT
  {IR(31094, 5), 117}, // 451 IAdd
  {IR(31271, 10), 138}, // 452 IAddCarry
  {IR(41462, 13), 602}, // 453 IAddSatINTEL
  {IR(41488, 14), 604}, // 454 IAverageINTEL
  {IR(41516, 21), 606}, // 455 IAverageRoundedINTEL
  {IR(31467, 7), 158}, // 456 IEqual
  {IR(31114, 5), 121}, // 457 IMul
  {IR(41584, 15), 610}, // 458 IMul32x16INTEL
  {IR(31474, 10), 159}, // 459 INotEqual
  {IR(31104, 5), 119}, // 460 ISub
  {IR(31281, 11), 139}, // 461 ISubBorrow
  {IR(41558, 13), 608}, // 462 ISubSatINTEL
  {IR(35512, 22), 380}, // 463 IgnoreIntersectionKHR
  {IR(39360, 21), 525}, // 464 IgnoreIntersectionNV
  {IR(27895, 6), 91}, // 465 Image
  {IR(36359, 29), 411}, // 466 ImageBlockMatchGatherSADQCOM
  {IR(36330, 29), 410}, // 467 ImageBlockMatchGatherSSDQCOM
  {IR(36232, 23), 406}, // 468 ImageBlockMatchSADQCOM
  {IR(36209, 23), 405}, // 469 ImageBlockMatchSSDQCOM
  {IR(36301, 29), 409}, // 470 ImageBlockMatchWindowSADQCOM
  {IR(36272, 29), 408}, // 471 ImageBlockMatchWindowSSDQCOM
  {IR(36190, 19), 404}, // 472 ImageBoxFilterQCOM
  {IR(30712, 16), 88}, // 473 ImageDrefGather
  {IR(30689, 11), 86}, // 474 ImageFetch
  {IR(30700, 12), 87}, // 475 ImageGather
  {IR(30749, 17), 92}, // 476 ImageQueryFormat
  {IR(30829, 17), 97}, // 477 ImageQueryLevels
  {IR(30815, 14), 96}, // 478 ImageQueryLod
  {IR(30766, 16), 93}, // 479 ImageQueryOrder
  {IR(30846, 18), 98}, // 480 ImageQuerySamples
  {IR(30800, 15), 95}, // 481 ImageQuerySize
  {IR(30782, 18), 94}, // 482 ImageQuerySizeLod
  {IR(30728, 10), 89}, // 483 ImageRead
  {IR(30546, 27), 81}, // 484 ImageSampleDrefExplicitLod
  {IR(30519, 27), 80}, // 485 ImageSampleDrefImplicitLod
  {IR(30496, 23), 79}, // 486 ImageSampleExplicitLod
  {IR(37962, 23), 479}, // 487 ImageSampleFootprintNV
  {IR(30450, 23), 78}, // 488 ImageSampleImplicitLod
  {IR(30658, 31), 85}, // 489 ImageSampleProjDrefExplicitLod
  {IR(30627, 31), 84}, // 490 ImageSampleProjDrefImplicitLod
  {IR(30600, 27), 83}, // 491 ImageSampleProjExplicitLod
  {IR(30573, 27), 82}, // 492 ImageSampleProjImplicitLod
  {IR(36166, 24), 403}, // 493 ImageSampleWeightedQCOM
  {IR(33653, 22), 288}, // 494 ImageSparseDrefGather
  {IR(33618, 17), 286}, // 495 ImageSparseFetch
  {IR(33635, 18), 287}, // 496 ImageSparseGather
  {IR(33745, 16), 293}, // 497 ImageSparseRead
  {IR(33445, 33), 281}, // 498 ImageSparseSampleDrefExplicitLod
  {IR(33412, 33), 280}, // 499 ImageSparseSampleDrefImplicitLod
  {IR(33383, 29), 279}, // 500 ImageSparseSampleExplicitLod
  {IR(33354, 29), 278}, // 501 ImageSparseSampleImplicitLod
  {IR(33581, 37), 285}, // 502 ImageSparseSampleProjDrefExplicitLod
  {IR(33544, 37), 284}, // 503 ImageSparseSampleProjDrefImplicitLod
  {IR(33511, 33), 283}, // 504 ImageSparseSampleProjExplicitLod
  {IR(33478, 33), 282}, // 505 ImageSparseSampleProjImplicitLod
  {IR(33675, 26), 289}, // 506 ImageSparseTexelsResident
  {IR(29929, 18), 53}, // 507 ImageTexelPointer
  {IR(30738, 11), 90}, // 508 ImageWrite
  {IR(30037, 20), 59}, // 509 InBoundsAccessChain
  {IR(30107, 23), 63}, // 510 InBoundsPtrAccessChain
  {IR(31338, 9), 146}, // 511 IsFinite
  {IR(40445, 22), 563}, // 512 IsHelperInvocationEXT
  {IR(31332, 6), 145}, // 513 IsInf
  {IR(31326, 6), 144}, // 514 IsNan
  {IR(36831, 23), 431}, // 515 IsNodePayloadValidAMDX
  {IR(31347, 9), 147}, // 516 IsNormal
  {IR(33267, 13), 273}, // 517 IsValidEvent
  {IR(32922, 17), 257}, // 518 IsValidReserveId
  {IR(32550, 5), 230}, // 519 Kill
  {IR(32467, 6), 226}, // 520 Label
  {IR(31367, 14), 149}, // 521 LessOrGreater
  {IR(32586, 14), 234}, // 522 LifetimeStart
  {IR(32600, 13), 235}, // 523 LifetimeStop
  {IR(28780, 5), 8}, // 524 Line
  {IR(29987, 5), 54}, // 525 Load
  {IR(31438, 11), 155}, // 526 LogicalAnd
  {IR(31399, 13), 152}, // 527 LogicalEqual
  {IR(31449, 11), 156}, // 528 LogicalNot
  {IR(31412, 16), 153}, // 529 LogicalNotEqual
  {IR(31428, 10), 154}, // 530 LogicalOr
  {IR(48487, 17), 785}, // 531 LoopControlINTEL
  {IR(32407, 10), 224}, // 532 LoopMerge
  {IR(50954, 18), 865}, // 533 MaskedGatherINTEL
  {IR(50972, 19), 866}, // 534 MaskedScatterINTEL
  {IR(31236, 18), 135}, // 535 MatrixTimesMatrix
  {IR(31182, 18), 132}, // 536 MatrixTimesScalar
  {IR(31218, 18), 134}, // 537 MatrixTimesVector
  {IR(30167, 15), 65}, // 538 MemberDecorate
  {IR(37007, 20), 440}, // 539 MemberDecorateIdEXT
  {IR(41784, 21), 622}, // 540 MemberDecorateString
  {IR(41805, 27), 622}, // 541 MemberDecorateStringGOOGLE
  {IR(28762, 11), 6}, // 542 MemberName
  {IR(32144, 14), 206}, // 543 MemoryBarrier
  {IR(28927, 12), 12}, // 544 MemoryModel
  {IR(33930, 19), 302}, // 545 MemoryNamedBarrier
  {IR(33949, 16), 303}, // 546 ModuleProcessed
  {IR(28757, 5), 5}, // 547 Name
  {IR(33907, 23), 301}, // 548 NamedBarrierInitialize
  {IR(33701, 7), 290}, // 549 NoLine
  {IR(36804, 27), 430}, // 550 NodePayloadArrayLengthAMDX
  {IR(28553, 4), 0}, // 551 Nop
  {IR(31878, 4), 186}, // 552 Not
  {IR(31381, 8), 150}, // 553 Ordered
  {IR(31254, 13), 136}, // 554 OuterProduct
  {IR(32373, 4), 223}, // 555 Phi
  {IR(37065, 10), 444}, // 556 PoisonKHR
  {IR(50757, 20), 854}, // 557 PredicatedLoadINTEL
  {IR(50777, 21), 855}, // 558 PredicatedStoreINTEL
  {IR(30057, 15), 60}, // 559 PtrAccessChain
  {IR(48922, 30), 800}, // 560 PtrCastToCrossWorkgroupALTERA
  {IR(48952, 29), 800}, // 561 PtrCastToCrossWorkgroupINTEL
  {IR(31011, 17), 111}, // 562 PtrCastToGeneric
  {IR(34797, 8), 343}, // 563 PtrDiff
  {IR(34776, 9), 341}, // 564 PtrEqual
  {IR(34785, 12), 342}, // 565 PtrNotEqual
  {IR(30939, 14), 106}, // 566 QuantizeToF16
  {IR(40750, 17), 574}, // 567 RawAccessChainNV
  {IR(36085, 31), 400}, // 568 RayQueryConfirmIntersectionKHR
  {IR(36053, 32), 399}, // 569 RayQueryGenerateIntersectionKHR
  {IR(39595, 23), 533}, // 570 RayQueryGetClusterIdNV
  {IR(49434, 39), 813}, // 571 RayQueryGetIntersectionBarycentricsKHR
  {IR(49509, 46), 815}, // 572 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(39560, 35), 533}, // 573 RayQueryGetIntersectionClusterIdNV
  {IR(49473, 36), 814}, // 574 RayQueryGetIntersectionFrontFaceKHR
  {IR(49353, 40), 811}, // 575 RayQueryGetIntersectionGeometryIndexKHR
  {IR(49205, 46), 808}, // 576 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(49251, 37), 809}, // 577 RayQueryGetIntersectionInstanceIdKHR
  {IR(49288, 65), 810}, // 578 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(40917, 37), 579}, // 579 RayQueryGetIntersectionLSSHitValueNV
  {IR(40845, 38), 577}, // 580 RayQueryGetIntersectionLSSPositionsNV
  {IR(40883, 34), 578}, // 581 RayQueryGetIntersectionLSSRadiiNV
  {IR(49555, 45), 816}, // 582 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(49600, 42), 817}, // 583 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(49703, 40), 820}, // 584 RayQueryGetIntersectionObjectToWorldKHR
  {IR(49393, 41), 812}, // 585 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(40767, 40), 575}, // 586 RayQueryGetIntersectionSpherePositionNV
  {IR(40807, 38), 576}, // 587 RayQueryGetIntersectionSphereRadiusNV
  {IR(49177, 28), 807}, // 588 RayQueryGetIntersectionTKHR
  {IR(39435, 50), 530}, // 589 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(36135, 31), 402}, // 590 RayQueryGetIntersectionTypeKHR
  {IR(49743, 40), 821}, // 591 RayQueryGetIntersectionWorldToObjectKHR
  {IR(49154, 23), 806}, // 592 RayQueryGetRayFlagsKHR
  {IR(49132, 22), 805}, // 593 RayQueryGetRayTMinKHR
  {IR(49642, 32), 818}, // 594 RayQueryGetWorldRayDirectionKHR
  {IR(49674, 29), 819}, // 595 RayQueryGetWorldRayOriginKHR
  {IR(36010, 22), 397}, // 596 RayQueryInitializeKHR
  {IR(41125, 19), 587}, // 597 RayQueryIsLSSHitNV
  {IR(41103, 22), 586}, // 598 RayQueryIsSphereHitNV
  {IR(36116, 19), 401}, // 599 RayQueryProceedKHR
  {IR(36032, 21), 398}, // 600 RayQueryTerminateKHR
  {IR(36688, 13), 425}, // 601 ReadClockKHR
  {IR(32790, 9), 249}, // 602 ReadPipe
  {IR(49040, 23), 802}, // 603 ReadPipeBlockingALTERA
  {IR(49063, 22), 802}, // 604 ReadPipeBlockingINTEL
  {IR(33238, 13), 271}, // 605 ReleaseEvent
  {IR(38759, 25), 504}, // 606 ReorderThreadWithHintEXT
  {IR(37922, 24), 477}, // 607 ReorderThreadWithHintNV
  {IR(38784, 30), 505}, // 608 ReorderThreadWithHitObjectEXT
  {IR(37893, 29), 476}, // 609 ReorderThreadWithHitObjectNV
  {IR(39317, 22), 524}, // 610 ReportIntersectionKHR
  {IR(39339, 21), 524}, // 611 ReportIntersectionNV
  {IR(32844, 23), 253}, // 612 ReserveReadPipePackets
  {IR(32867, 24), 254}, // 613 ReserveWritePipePackets
  {IR(32809, 17), 251}, // 614 ReservedReadPipe
  {IR(32826, 18), 252}, // 615 ReservedWritePipe
  {IR(47014, 19), 743}, // 616 RestoreMemoryINTEL
  {IR(33226, 12), 270}, // 617 RetainEvent
  {IR(32555, 7), 231}, // 618 Return
  {IR(32562, 12), 232}, // 619 ReturnValue
  {IR(50936, 18), 864}, // 620 RoundFToTF32INTEL
  {IR(30921, 9), 104}, // 621 SConvert
  {IR(31129, 5), 124}, // 622 SDiv
  {IR(35597, 5), 382}, // 623 SDot
  {IR(35638, 11), 385}, // 624 SDotAccSat
  {IR(35649, 14), 385}, // 625 SDotAccSatKHR
  {IR(35602, 8), 382}, // 626 SDotKHR
  {IR(31497, 13), 161}, // 627 SGreaterThan
  {IR(31528, 18), 163}, // 628 SGreaterThanEqual
  {IR(31556, 10), 165}, // 629 SLessThan
  {IR(31581, 15), 167}, // 630 SLessThanEqual
  {IR(31149, 5), 128}, // 631 SMod
  {IR(31305, 13), 141}, // 632 SMulExtended
  {IR(31078, 8), 115}, // 633 SNegate
  {IR(31144, 5), 127}, // 634 SRem
  {IR(35623, 6), 384}, // 635 SUDot
  {IR(35688, 12), 387}, // 636 SUDotAccSat
  {IR(35700, 15), 387}, // 637 SUDotAccSatKHR
  {IR(35629, 9), 384}, // 638 SUDotKHR
  {IR(30405, 13), 77}, // 639 SampledImage
  {IR(40669, 29), 573}, // 640 SamplerImageAddressingModeNV
  {IR(30967, 15), 108}, // 641 SatConvertSToU
  {IR(30982, 15), 109}, // 642 SatConvertUToS
  {IR(46998, 16), 742}, // 643 SaveMemoryINTEL
  {IR(31460, 7), 157}, // 644 Select
  {IR(32452, 15), 225}, // 645 SelectionMerge
  {IR(38211, 18), 487}, // 646 SetMeshOutputsEXT
  {IR(33280, 19), 274}, // 647 SetUserEventStatus
  {IR(31829, 17), 182}, // 648 ShiftLeftLogical
  {IR(31808, 21), 181}, // 649 ShiftRightArithmetic
  {IR(31790, 18), 180}, // 650 ShiftRightLogical
  {IR(31356, 11), 148}, // 651 SignBitSet
  {IR(33761, 7), 294}, // 652 SizeOf
  {IR(28734, 7), 3}, // 653 Source
  {IR(28615, 16), 2}, // 654 SourceContinued
  {IR(28741, 16), 4}, // 655 SourceExtension
  {IR(29752, 13), 45}, // 656 SpecConstant
  {IR(50633, 30), 851}, // 657 SpecConstantArchitectureINTEL
  {IR(50700, 30), 852}, // 658 SpecConstantCapabilitiesINTEL
  {IR(29765, 22), 46}, // 659 SpecConstantComposite
  {IR(49877, 36), 826}, // 660 SpecConstantCompositeContinuedINTEL
  {IR(35929, 34), 394}, // 661 SpecConstantCompositeReplicateEXT
  {IR(37045, 20), 443}, // 662 SpecConstantDataKHR
  {IR(29734, 18), 44}, // 663 SpecConstantFalse
  {IR(29828, 15), 47}, // 664 SpecConstantOp
  {IR(36873, 23), 433}, // 665 SpecConstantStringAMDX
  {IR(50609, 24), 850}, // 666 SpecConstantTargetINTEL
  {IR(29717, 17), 43}, // 667 SpecConstantTrue
  {IR(34851, 25), 346}, // 668 StencilAttachmentReadEXT
  {IR(29992, 6), 55}, // 669 Store
  {IR(28773, 7), 7}, // 670 String
  {IR(50298, 25), 839}, // 671 Subgroup2DBlockLoadINTEL
  {IR(50323, 34), 840}, // 672 Subgroup2DBlockLoadTransformINTEL
  {IR(50357, 34), 841}, // 673 Subgroup2DBlockLoadTransposeINTEL
  {IR(50391, 29), 842}, // 674 Subgroup2DBlockPrefetchINTEL
  {IR(50420, 26), 843}, // 675 Subgroup2DBlockStoreINTEL
  {IR(35349, 20), 372}, // 676 SubgroupAllEqualKHR
  {IR(35319, 15), 370}, // 677 SubgroupAllKHR
  {IR(35334, 15), 371}, // 678 SubgroupAnyKHR
  {IR(45516, 30), 706}, // 679 SubgroupAvcBmeInitializeINTEL
  {IR(45486, 30), 705}, // 680 SubgroupAvcFmeInitializeINTEL
  {IR(43921, 35), 675}, // 681 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(43956, 39), 676}, // 682 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(44597, 38), 689}, // 683 SubgroupAvcImeConvertToMceResultINTEL
  {IR(44220, 45), 682}, // 684 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(44320, 53), 684}, // 685 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(44541, 56), 688}, // 686 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(44429, 54), 686}, // 687 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(44173, 47), 681}, // 688 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(44265, 55), 683}, // 689 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(44483, 58), 687}, // 690 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(44373, 56), 685}, // 691 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(45229, 36), 700}, // 692 SubgroupAvcImeGetBorderReachedINTEL
  {IR(44681, 44), 691}, // 693 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(44635, 46), 690}, // 694 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(45096, 66), 698}, // 695 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(45028, 68), 697}, // 696 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(45162, 67), 699}, // 697 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(44891, 68), 695}, // 698 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(44821, 70), 694}, // 699 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(44959, 69), 696}, // 700 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(45265, 48), 701}, // 701 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(45313, 59), 702}, // 702 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(45430, 56), 704}, // 703 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(45372, 58), 703}, // 704 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(43784, 30), 671}, // 705 SubgroupAvcImeInitializeINTEL
  {IR(43888, 33), 674}, // 706 SubgroupAvcImeRefWindowSizeINTEL
  {IR(43852, 36), 673}, // 707 SubgroupAvcImeSetDualReferenceINTEL
  {IR(44085, 54), 679}, // 708 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(43995, 43), 677}, // 709 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(43814, 38), 672}, // 710 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(44038, 47), 678}, // 711 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(44139, 34), 680}, // 712 SubgroupAvcImeSetWeightedSadINTEL
  {IR(44774, 47), 693}, // 713 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(44725, 49), 692}, // 714 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(43174, 39), 656}, // 715 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(43213, 38), 657}, // 716 SubgroupAvcMceConvertToImeResultINTEL
  {IR(43251, 39), 658}, // 717 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(43290, 38), 659}, // 718 SubgroupAvcMceConvertToRefResultINTEL
  {IR(43328, 39), 660}, // 719 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(43367, 38), 661}, // 720 SubgroupAvcMceConvertToSicResultINTEL
  {IR(43480, 43), 664}, // 721 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(42612, 50), 645}, // 722 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(42210, 60), 637}, // 723 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(42410, 51), 641}, // 724 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(42556, 56), 644}, // 725 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(42323, 47), 639}, // 726 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(42911, 56), 651}, // 727 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(42810, 50), 649}, // 728 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(42505, 51), 643}, // 729 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(42714, 49), 647}, // 730 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(42662, 52), 646}, // 731 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(42860, 51), 650}, // 732 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(43599, 38), 667}, // 733 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(43441, 39), 663}, // 734 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(43523, 38), 665}, // 735 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(43561, 38), 666}, // 736 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(43637, 45), 668}, // 737 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(43682, 40), 669}, // 738 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(43722, 62), 670}, // 739 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(43405, 36), 662}, // 740 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(42967, 33), 652}, // 741 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(43113, 61), 655}, // 742 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(42270, 53), 638}, // 743 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(42461, 44), 642}, // 744 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(42370, 40), 640}, // 745 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(42763, 47), 648}, // 746 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(43052, 61), 654}, // 747 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(43000, 52), 653}, // 748 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(45546, 39), 707}, // 749 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(45868, 38), 714}, // 750 SubgroupAvcRefConvertToMceResultINTEL
  {IR(45721, 45), 711}, // 751 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(45766, 46), 712}, // 752 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(45812, 56), 713}, // 753 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(45674, 47), 710}, // 754 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(45585, 46), 708}, // 755 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(45631, 43), 709}, // 756 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(46004, 42), 718}, // 757 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(45968, 36), 717}, // 758 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(45936, 32), 716}, // 759 SubgroupAvcSicConfigureSkcINTEL
  {IR(46085, 39), 720}, // 760 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(46625, 38), 732}, // 761 SubgroupAvcSicConvertToMceResultINTEL
  {IR(46400, 31), 727}, // 762 SubgroupAvcSicEvaluateIpeINTEL
  {IR(46478, 45), 729}, // 763 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(46523, 46), 730}, // 764 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(46569, 56), 731}, // 765 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46431, 47), 728}, // 766 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(46742, 46), 735}, // 767 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(46698, 44), 734}, // 768 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(46963, 35), 740}, // 769 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(46829, 36), 737}, // 770 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(46663, 35), 733}, // 771 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(46046, 39), 719}, // 772 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(46788, 41), 736}, // 773 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(46865, 50), 738}, // 774 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(46915, 48), 739}, // 775 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(45906, 30), 715}, // 776 SubgroupAvcSicInitializeINTEL
  {IR(46266, 43), 724}, // 777 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(46357, 43), 726}, // 778 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(46216, 50), 723}, // 779 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(46168, 48), 722}, // 780 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(46124, 44), 721}, // 781 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(46309, 48), 725}, // 782 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6422, 18), 363}, // 783 SubgroupBallotKHR
  {IR(50271, 27), 838}, // 784 SubgroupBlockPrefetchINTEL
  {IR(41216, 23), 592}, // 785 SubgroupBlockReadINTEL
  {IR(41239, 24), 593}, // 786 SubgroupBlockWriteINTEL
  {IR(35186, 27), 364}, // 787 SubgroupFirstInvocationKHR
  {IR(41263, 28), 594}, // 788 SubgroupImageBlockReadINTEL
  {IR(41291, 29), 595}, // 789 SubgroupImageBlockWriteINTEL
  {IR(41320, 33), 596}, // 790 SubgroupImageMediaBlockReadINTEL
  {IR(41353, 34), 597}, // 791 SubgroupImageMediaBlockWriteINTEL
  {IR(15615, 38), 844}, // 792 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(35369, 26), 374}, // 793 SubgroupReadInvocationKHR
  {IR(41144, 25), 589}, // 794 SubgroupShuffleDownINTEL
  {IR(13271, 21), 588}, // 795 SubgroupShuffleINTEL
  {IR(41169, 23), 590}, // 796 SubgroupShuffleUpINTEL
  {IR(41192, 24), 591}, // 797 SubgroupShuffleXorINTEL
  {IR(32543, 7), 229}, // 798 Switch
  {IR(50085, 24), 834}, // 799 TaskSequenceAsyncALTERA
  {IR(50109, 23), 834}, // 800 TaskSequenceAsyncINTEL
  {IR(50036, 25), 833}, // 801 TaskSequenceCreateALTERA
  {IR(50061, 24), 833}, // 802 TaskSequenceCreateINTEL
  {IR(50132, 22), 835}, // 803 TaskSequenceGetALTERA
  {IR(50154, 21), 835}, // 804 TaskSequenceGetINTEL
  {IR(50175, 26), 836}, // 805 TaskSequenceReleaseALTERA
  {IR(50201, 25), 836}, // 806 TaskSequenceReleaseINTEL
  {IR(40487, 27), 565}, // 807 TensorLayoutSetBlockSizeNV
  {IR(40351, 28), 558}, // 808 TensorLayoutSetClampValueNV
  {IR(40280, 27), 555}, // 809 TensorLayoutSetDimensionNV
  {IR(40307, 24), 556}, // 810 TensorLayoutSetStrideNV
  {IR(40331, 20), 557}, // 811 TensorLayoutSliceNV
  {IR(34961, 19), 350}, // 812 TensorQuerySizeARM
  {IR(34932, 14), 348}, // 813 TensorReadARM
  {IR(40467, 20), 564}, // 814 TensorViewSetClipNV
  {IR(40398, 25), 560}, // 815 TensorViewSetDimensionNV
  {IR(40423, 22), 561}, // 816 TensorViewSetStrideNV
  {IR(34946, 15), 349}, // 817 TensorWriteARM
  {IR(35073, 20), 358}, // 818 TerminateInvocation
  {IR(35534, 16), 381}, // 819 TerminateRayKHR
  {IR(39381, 15), 526}, // 820 TerminateRayNV
  {IR(39404, 14), 528}, // 821 TraceMotionNV
  {IR(39396, 8), 527}, // 822 TraceNV
  {IR(35446, 12), 377}, // 823 TraceRayKHR
  {IR(39418, 17), 529}, // 824 TraceRayMotionNV
  {IR(30395, 10), 76}, // 825 Transpose
  {IR(39485, 29), 531}, // 826 TypeAccelerationStructureKHR
  {IR(39514, 28), 531}, // 827 TypeAccelerationStructureNV
  {IR(29344, 10), 25}, // 828 TypeArray
  {IR(42129, 37), 634}, // 829 TypeAvcImeDualReferenceStreaminINTEL
  {IR(41864, 23), 625}, // 830 TypeAvcImePayloadINTEL
  {IR(42046, 44), 632}, // 831 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(41978, 22), 630}, // 832 TypeAvcImeResultINTEL
  {IR(42000, 46), 631}, // 833 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(42090, 39), 633}, // 834 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(41933, 23), 628}, // 835 TypeAvcMcePayloadINTEL
  {IR(41956, 22), 629}, // 836 TypeAvcMceResultINTEL
  {IR(41887, 23), 626}, // 837 TypeAvcRefPayloadINTEL
  {IR(42166, 22), 635}, // 838 TypeAvcRefResultINTEL
  {IR(41910, 23), 627}, // 839 TypeAvcSicPayloadINTEL
  {IR(42188, 22), 636}, // 840 TypeAvcSicResultINTEL
  {IR(29106, 9), 17}, // 841 TypeBool
  {IR(36948, 14), 436}, // 842 TypeBufferEXT
  {IR(49797, 23), 823}, // 843 TypeBufferSurfaceINTEL
  {IR(35715, 25), 388}, // 844 TypeCooperativeMatrixKHR
  {IR(39807, 24), 541}, // 845 TypeCooperativeMatrixNV
  {IR(38001, 24), 480}, // 846 TypeCooperativeVectorNV
  {IR(29459, 16), 32}, // 847 TypeDeviceEvent
  {IR(29449, 10), 31}, // 848 TypeEvent
  {IR(29160, 10), 19}, // 849 TypeFloat
  {IR(29508, 19), 36}, // 850 TypeForwardPointer
  {IR(29436, 13), 30}, // 851 TypeFunction
  {IR(35060, 13), 357}, // 852 TypeGraphARM
  {IR(38742, 17), 503}, // 853 TypeHitObjectEXT
  {IR(37946, 16), 478}, // 854 TypeHitObjectNV
  {IR(29305, 10), 22}, // 855 TypeImage
  {IR(29115, 8), 18}, // 856 TypeInt
  {IR(29181, 11), 21}, // 857 TypeMatrix
  {IR(33890, 17), 300}, // 858 TypeNamedBarrier
  {IR(36750, 25), 428}, // 859 TypeNodePayloadArrayAMDX
  {IR(29382, 11), 28}, // 860 TypeOpaque
  {IR(29499, 9), 35}, // 861 TypePipe
  {IR(33768, 16), 295}, // 862 TypePipeStorage
  {IR(29424, 12), 29}, // 863 TypePointer
  {IR(29489, 10), 34}, // 864 TypeQueue
  {IR(35994, 16), 396}, // 865 TypeRayQueryKHR
  {IR(29475, 14), 33}, // 866 TypeReserveId
  {IR(29354, 17), 26}, // 867 TypeRuntimeArray
  {IR(29327, 17), 24}, // 868 TypeSampledImage
  {IR(29315, 12), 23}, // 869 TypeSampler
  {IR(29371, 11), 27}, // 870 TypeStruct
  {IR(49820, 25), 824}, // 871 TypeStructContinuedINTEL
  {IR(50226, 23), 837}, // 872 TypeTaskSequenceALTERA
  {IR(50249, 22), 837}, // 873 TypeTaskSequenceINTEL
  {IR(34876, 14), 347}, // 874 TypeTensorARM
  {IR(40223, 19), 552}, // 875 TypeTensorLayoutNV
  {IR(40242, 17), 553}, // 876 TypeTensorViewNV
  {IR(35093, 22), 359}, // 877 TypeUntypedPointerKHR
  {IR(29170, 11), 20}, // 878 TypeVector
  {IR(37985, 16), 480}, // 879 TypeVectorIdEXT
  {IR(41846, 18), 624}, // 880 TypeVmeImageINTEL
  {IR(29097, 9), 16}, // 881 TypeVoid
  {IR(41475, 13), 603}, // 882 UAddSatINTEL
  {IR(41502, 14), 605}, // 883 UAverageINTEL
  {IR(41537, 21), 607}, // 884 UAverageRoundedINTEL
  {IR(30912, 9), 103}, // 885 UConvert
  {IR(41387, 24), 598}, // 886 UCountLeadingZerosINTEL
  {IR(41411, 25), 599}, // 887 UCountTrailingZerosINTEL
  {IR(31124, 5), 123}, // 888 UDiv
  {IR(35610, 5), 383}, // 889 UDot
  {IR(35663, 11), 386}, // 890 UDotAccSat
  {IR(35674, 14), 386}, // 891 UDotAccSatKHR
  {IR(35615, 8), 383}, // 892 UDotKHR
  {IR(31484, 13), 160}, // 893 UGreaterThan
  {IR(31510, 18), 162}, // 894 UGreaterThanEqual
  {IR(31546, 10), 164}, // 895 ULessThan
  {IR(31566, 15), 166}, // 896 ULessThanEqual
  {IR(31139, 5), 126}, // 897 UMod
  {IR(41599, 15), 611}, // 898 UMul32x16INTEL
  {IR(31292, 13), 140}, // 899 UMulExtended
  {IR(41571, 13), 609}, // 900 USubSatINTEL
  {IR(28609, 6), 1}, // 901 Undef
  {IR(31389, 10), 151}, // 902 Unordered
  {IR(32574, 12), 233}, // 903 Unreachable
  {IR(35134, 22), 361}, // 904 UntypedAccessChainKHR
  {IR(35271, 22), 367}, // 905 UntypedArrayLengthKHR
  {IR(35421, 25), 376}, // 906 UntypedGroupAsyncCopyKHR
  {IR(36979, 28), 439}, // 907 UntypedImageTexelPointerEXT
  {IR(35156, 30), 362}, // 908 UntypedInBoundsAccessChainKHR
  {IR(35238, 33), 366}, // 909 UntypedInBoundsPtrAccessChainKHR
  {IR(35293, 19), 368}, // 910 UntypedPrefetchKHR
  {IR(35213, 25), 365}, // 911 UntypedPtrAccessChainKHR
  {IR(35115, 19), 360}, // 912 UntypedVariableKHR
  {IR(15681, 32), 846}, // 913 UntypedVariableLengthArrayINTEL
  {IR(29920, 9), 52}, // 914 Variable
  {IR(13727, 25), 741}, // 915 VariableLengthArrayINTEL
  {IR(30277, 21), 69}, // 916 VectorExtractDynamic
  {IR(30298, 20), 70}, // 917 VectorInsertDynamic
  {IR(30318, 14), 71}, // 918 VectorShuffle
  {IR(31200, 18), 133}, // 919 VectorTimesMatrix
  {IR(31164, 18), 131}, // 920 VectorTimesScalar
  {IR(41832, 14), 623}, // 921 VmeImageINTEL
  {IR(38284, 33), 489}, // 922 WritePackedPrimitiveIndices4x8NV
  {IR(32799, 10), 250}, // 923 WritePipe
  {IR(49085, 24), 803}, // 924 WritePipeBlockingALTERA
  {IR(49109, 23), 803}, // 925 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 496> kExtInstByValue{{
    {0,IR(765, 9),IR(51135, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(765, 9),IR(51143, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(51156, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(51164, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(51178, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(811, 3),IR(51251, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(814, 3),IR(51300, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(817, 2),IR(51355, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(51374, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(51389, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(819, 6),IR(51405, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(51418, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(825, 9),IR(51436, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(834, 9),IR(51488, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(843, 10),IR(51507, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(853, 5),IR(51523, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(51544, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(51565, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(858, 6),IR(51583, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(864, 5),IR(51610, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(869, 5),IR(51645, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(843, 10),IR(51676, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(874, 8),IR(51696, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(882, 11),IR(51721, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(893, 5),IR(51735, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(898, 3),IR(51753, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(901, 2),IR(51784, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(51795, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(903, 3),IR(51808, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(906, 7),IR(51865, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(51884, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(51905, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(51918, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(913, 2),IR(51962, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(747, 1),IR(51977, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(915, 4),IR(51993, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(898, 3),IR(52007, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(52023, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(52029, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(52039, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(52045, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(52050, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(52055, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(52061, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(52067, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(52073, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(52078, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(52084, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(52092, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(52100, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(52104, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(52108, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(52112, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(52117, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(52122, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(52127, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(52132, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(52137, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(52142, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(52148, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(52154, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(52160, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(52166, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(52170, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(52174, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(52178, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(52183, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(52188, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(52193, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(52205, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(52217, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(52231, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(52236, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(52247, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(52252, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(52257, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(52262, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(52267, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(52272, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(52277, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(52284, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(52291, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(52298, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(52303, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(52308, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(52313, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(52324, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(52328, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(52334, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(52346, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(52352, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(52365, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(52378, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(52392, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(52406, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(52419, 15),IR(330, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(52434, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(52450, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(52466, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(52481, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(52496, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(52511, 17),IR(330, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(52528, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(52535, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(52544, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(52550, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(52560, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(52572, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(52580, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(52588, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(52597, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(52606, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(52615, 22),IR(331, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(52637, 20),IR(331, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(52657, 20),IR(331, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(52677, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(52682, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(52687, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5641, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(919, 5),IR(52694, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(593, 5),IR(52707, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(593, 5),IR(52729, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(557, 7),IR(52745, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(557, 7),IR(52770, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(593, 5),IR(52789, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(593, 5),IR(52813, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(593, 5),IR(52834, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(593, 5),IR(52855, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(593, 5),IR(52871, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(52889, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(52915, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(52940, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(52960, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(52985, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(53015, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(53038, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(53063, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(53089, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(53119, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(53145, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(9161, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(53165, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(53195, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(593, 5),IR(53223, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(557, 7),IR(53251, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(53274, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(53309, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(53347, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(53389, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(528, 6),IR(53434, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(528, 6),IR(53471, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(593, 5),IR(53511, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(593, 5),IR(53538, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(53565, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(53597, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(53637, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(53648, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(53674, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(53706, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(53740, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(51164, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {1,IR(296, 4),IR(51178, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {2,IR(593, 5),IR(51251, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {3,IR(33, 3),IR(51300, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {4,IR(36, 2),IR(51355, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {5,IR(168, 2),IR(51374, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {6,IR(36, 2),IR(51389, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {7,IR(528, 6),IR(51405, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo
    {8,IR(375, 3),IR(51418, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {9,IR(924, 9),IR(51436, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {10,IR(933, 10),IR(51488, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {11,IR(943, 9),IR(51507, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {12,IR(296, 4),IR(51523, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {13,IR(36, 2),IR(51544, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {14,IR(168, 2),IR(51565, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {15,IR(528, 6),IR(51583, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {16,IR(675, 5),IR(51610, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {17,IR(952, 5),IR(51645, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {18,IR(957, 10),IR(51676, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {19,IR(471, 8),IR(51696, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {20,IR(957, 10),IR(51721, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo
    {21,IR(593, 5),IR(51735, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {22,IR(33, 3),IR(51753, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {23,IR(901, 2),IR(51784, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo
    {24,IR(0, 0),IR(51795, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo
    {25,IR(967, 3),IR(51808, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {26,IR(970, 8),IR(51865, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {27,IR(36, 2),IR(51884, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {28,IR(978, 4),IR(51905, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo
    {29,IR(978, 4),IR(51918, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo
    {30,IR(168, 2),IR(51962, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo
    {31,IR(747, 1),IR(51977, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo
    {32,IR(580, 4),IR(51993, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {33,IR(33, 3),IR(52007, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {34,IR(509, 7),IR(53762, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {35,IR(901, 2),IR(53782, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo
    {101,IR(36, 2),IR(53794, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {102,IR(10, 1),IR(53818, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {103,IR(675, 5),IR(53839, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo
    {104,IR(0, 0),IR(53849, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo
    {105,IR(36, 2),IR(53861, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {106,IR(10, 1),IR(53882, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {107,IR(296, 4),IR(53899, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {108,IR(33, 3),IR(53915, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {109,IR(36, 2),IR(53931, 21),IR(1, 0)}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {110,IR(675, 5),IR(53952, 30),IR(1, 0)}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {1,IR(765, 9),IR(53982, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(53996, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(54009, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(54021, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(54035, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(982, 15),IR(54058, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(997, 33),IR(54078, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1030, 19),IR(54097, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(51164, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1049, 4),IR(51178, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1053, 3),IR(51251, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1056, 3),IR(51300, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1059, 2),IR(51355, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(51374, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(51389, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(819, 6),IR(51405, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1061, 3),IR(51418, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1064, 9),IR(51436, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1073, 10),IR(51488, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1083, 10),IR(51507, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1093, 5),IR(51523, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(51544, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(51565, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(858, 6),IR(51583, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(864, 5),IR(51610, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(869, 5),IR(51645, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1083, 10),IR(51676, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1098, 8),IR(51696, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1106, 11),IR(51721, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(893, 5),IR(51735, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(898, 3),IR(51753, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(901, 2),IR(51784, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(51795, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(903, 3),IR(51808, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1117, 8),IR(51865, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(51884, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(51905, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(978, 4),IR(51918, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1125, 2),IR(51962, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(747, 1),IR(51977, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(915, 4),IR(51993, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(898, 3),IR(52007, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1127, 7),IR(53762, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(901, 2),IR(53782, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1134, 8),IR(54418, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(54435, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(54440, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(54446, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(54453, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(54458, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(54464, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(54471, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(54476, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(54482, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(54488, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(54495, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(54503, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(54508, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(54513, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(54522, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(54526, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(54531, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(54537, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(54542, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(54546, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(54550, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(54555, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(54561, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(54567, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(54572, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(54577, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(54583, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(54587, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(54592, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(54597, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(54602, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(54608, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(54614, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(54620, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(54626, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(54632, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(54639, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(54648, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(54652, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(54657, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(54663, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(54669, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(54674, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(54678, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(54685, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(54692, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(54697, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(54701, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(54711, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(54715, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(54720, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(54725, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(54735, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(54742, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(54747, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(54753, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(54759, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(54765, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(54769, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(54776, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(54781, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(54787, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(54792, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(54796, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(54801, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(54807, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(54814, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(54820, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(54829, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(54841, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(54850, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(54860, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(54871, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(54880, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(54890, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(54901, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(54911, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(54922, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(54933, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(54942, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(54952, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(54961, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(54972, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(54986, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(54997, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(55009, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(55022, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(55033, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(55045, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(55058, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(55070, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(55083, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(55096, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(55107, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(55119, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(55130, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(55137, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(55145, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(55157, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(55169, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(55173, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(55181, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(55186, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(55197, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(55202, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(55208, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(55217, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(55224, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(55234, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(55248, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(55260, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(55275, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(55281, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(55292, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(55302, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(55312, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(55319, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(55326, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(55334, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(55342, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(55350, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(55358, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(55362, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(55366, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(55375, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(55385, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(55395, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(55401, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(55407, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(55413, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(55419, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(55428, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(55435, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(55445, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(55455, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(55466, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(55477, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(55486, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(55494, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(55502, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(55510, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1142, 3),IR(55518, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(55525, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(55533, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1142, 3),IR(55544, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(55556, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1145, 4),IR(55568, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(55582, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1145, 4),IR(55595, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1142, 3),IR(55610, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(55623, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1145, 4),IR(55637, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(55653, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(55661, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(55670, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(55677, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(55686, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(55696, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(55703, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(55709, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(55720, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(55729, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(55738, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(55755, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(55772, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(55780, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(55802, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(55830, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(55849, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(55858, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(55881, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(55890, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(55899, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(55908, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(55917, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(55926, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(55935, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(55944, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(55953, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(55962, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(509, 7),IR(55969, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(755, 10),IR(55980, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(755, 10),IR(55987, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(755, 10),IR(55994, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(56011, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(56017, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(675, 5),IR(56024, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(56035, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(765, 9),IR(56042, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(56059, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(56065, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(56069, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(56077, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(56082, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(56086, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(56109, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(56121, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(56132, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(56144, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(56151, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(56163, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(56182, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(56202, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(56213, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(56225, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(56233, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(56241, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(56245, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(56249, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(56253, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(56259, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(56263, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(56275, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(56280, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(56284, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(56288, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(56292, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(56298, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(56302, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(56314, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(56321, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(56332, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(56338, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(56342, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(56349, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(56355, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(56363, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(56377, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(56388, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(56399, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(56410, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(56421, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(56436, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(56447, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(56454, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(56458, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(56466, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(56474, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(56480, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(56485, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(56495, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(56502, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(675, 5),IR(56510, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(56517, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(755, 10),IR(56522, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
}};

IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(0, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(3, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(37, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(118, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(160, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(206, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(214, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(251, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(413, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(416, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(420, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(421, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(430, 66);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 496> kExtInstNames{{
    {IR(51135, 8), 0}, // MIN_SAD in arm_motion_engine_100
    {IR(51143, 13), 1}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(51156, 8), 2}, // RAW_SAD in arm_motion_engine_100
    {IR(51178, 21), 4}, // DebugCompilationUnit in debuginfo
    {IR(51905, 13), 31}, // DebugDeclare in debuginfo
    {IR(51977, 16), 34}, // DebugExpression in debuginfo
    {IR(51721, 14), 23}, // DebugFunction in debuginfo
    {IR(51696, 25), 22}, // DebugFunctionDeclaration in debuginfo
    {IR(51676, 20), 21}, // DebugGlobalVariable in debuginfo
    {IR(51164, 14), 3}, // DebugInfoNone in debuginfo
    {IR(51808, 15), 28}, // DebugInlinedAt in debuginfo
    {IR(51884, 21), 30}, // DebugInlinedVariable in debuginfo
    {IR(51735, 18), 24}, // DebugLexicalBlock in debuginfo
    {IR(51753, 31), 25}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(51865, 19), 29}, // DebugLocalVariable in debuginfo
    {IR(51993, 14), 35}, // DebugMacroDef in debuginfo
    {IR(52007, 16), 36}, // DebugMacroUndef in debuginfo
    {IR(51795, 13), 27}, // DebugNoScope in debuginfo
    {IR(51962, 15), 33}, // DebugOperation in debuginfo
    {IR(51784, 11), 26}, // DebugScope in debuginfo
    {IR(51374, 15), 8}, // DebugTypeArray in debuginfo
    {IR(51251, 15), 5}, // DebugTypeBasic in debuginfo
    {IR(51488, 19), 13}, // DebugTypeComposite in debuginfo
    {IR(51436, 14), 12}, // DebugTypeEnum in debuginfo
    {IR(51418, 18), 11}, // DebugTypeFunction in debuginfo
    {IR(51523, 21), 15}, // DebugTypeInheritance in debuginfo
    {IR(51507, 16), 14}, // DebugTypeMember in debuginfo
    {IR(51300, 17), 6}, // DebugTypePointer in debuginfo
    {IR(51544, 21), 16}, // DebugTypePtrToMember in debuginfo
    {IR(51355, 19), 7}, // DebugTypeQualifier in debuginfo
    {IR(51565, 18), 17}, // DebugTypeTemplate in debuginfo
    {IR(51583, 27), 18}, // DebugTypeTemplateParameter in debuginfo
    {IR(51645, 31), 20}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(51610, 35), 19}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(51389, 16), 9}, // DebugTypeVector in debuginfo
    {IR(51405, 13), 10}, // DebugTypedef in debuginfo
    {IR(51918, 11), 32}, // DebugValue in debuginfo
    {IR(52117, 5), 53}, // Acos in glsl_std_450
    {IR(52148, 6), 59}, // Acosh in glsl_std_450
    {IR(52112, 5), 52}, // Asin in glsl_std_450
    {IR(52142, 6), 58}, // Asinh in glsl_std_450
    {IR(52122, 5), 54}, // Atan in glsl_std_450
    {IR(52160, 6), 61}, // Atan2 in glsl_std_450
    {IR(52154, 6), 60}, // Atanh in glsl_std_450
    {IR(52073, 5), 45}, // Ceil in glsl_std_450
    {IR(52104, 4), 50}, // Cos in glsl_std_450
    {IR(52132, 5), 56}, // Cosh in glsl_std_450
    {IR(52544, 6), 104}, // Cross in glsl_std_450
    {IR(52092, 8), 48}, // Degrees in glsl_std_450
    {IR(52205, 12), 69}, // Determinant in glsl_std_450
    {IR(52535, 9), 103}, // Distance in glsl_std_450
    {IR(52170, 4), 63}, // Exp in glsl_std_450
    {IR(52178, 5), 65}, // Exp2 in glsl_std_450
    {IR(52045, 5), 40}, // FAbs in glsl_std_450
    {IR(52277, 7), 79}, // FClamp in glsl_std_450
    {IR(52262, 5), 76}, // FMax in glsl_std_450
    {IR(52247, 5), 73}, // FMin in glsl_std_450
    {IR(52298, 5), 82}, // FMix in glsl_std_450
    {IR(52055, 6), 42}, // FSign in glsl_std_450
    {IR(52560, 12), 106}, // FaceForward in glsl_std_450
    {IR(52588, 9), 109}, // FindILsb in glsl_std_450
    {IR(52597, 9), 110}, // FindSMsb in glsl_std_450
    {IR(52606, 9), 111}, // FindUMsb in glsl_std_450
    {IR(52067, 6), 44}, // Floor in glsl_std_450
    {IR(52324, 4), 86}, // Fma in glsl_std_450
    {IR(52078, 6), 46}, // Fract in glsl_std_450
    {IR(52328, 6), 87}, // Frexp in glsl_std_450
    {IR(52334, 12), 88}, // FrexpStruct in glsl_std_450
    {IR(52303, 5), 83}, // IMix in glsl_std_450
    {IR(52615, 22), 112}, // InterpolateAtCentroid in glsl_std_450
    {IR(52657, 20), 114}, // InterpolateAtOffset in glsl_std_450
    {IR(52637, 20), 113}, // InterpolateAtSample in glsl_std_450
    {IR(52193, 12), 68}, // InverseSqrt in glsl_std_450
    {IR(52346, 6), 89}, // Ldexp in glsl_std_450
    {IR(52528, 7), 102}, // Length in glsl_std_450
    {IR(52174, 4), 64}, // Log in glsl_std_450
    {IR(52183, 5), 66}, // Log2 in glsl_std_450
    {IR(52217, 14), 70}, // MatrixInverse in glsl_std_450
    {IR(52231, 5), 71}, // Modf in glsl_std_450
    {IR(52236, 11), 72}, // ModfStruct in glsl_std_450
    {IR(52687, 7), 117}, // NClamp in glsl_std_450
    {IR(52682, 5), 116}, // NMax in glsl_std_450
    {IR(52677, 5), 115}, // NMin in glsl_std_450
    {IR(52550, 10), 105}, // Normalize in glsl_std_450
    {IR(52419, 15), 95}, // PackDouble2x32 in glsl_std_450
    {IR(52406, 13), 94}, // PackHalf2x16 in glsl_std_450
    {IR(52378, 14), 92}, // PackSnorm2x16 in glsl_std_450
    {IR(52352, 13), 90}, // PackSnorm4x8 in glsl_std_450
    {IR(52392, 14), 93}, // PackUnorm2x16 in glsl_std_450
    {IR(52365, 13), 91}, // PackUnorm4x8 in glsl_std_450
    {IR(52166, 4), 62}, // Pow in glsl_std_450
    {IR(52084, 8), 47}, // Radians in glsl_std_450
    {IR(52572, 8), 107}, // Reflect in glsl_std_450
    {IR(52580, 8), 108}, // Refract in glsl_std_450
    {IR(52023, 6), 37}, // Round in glsl_std_450
    {IR(52029, 10), 38}, // RoundEven in glsl_std_450
    {IR(52050, 5), 41}, // SAbs in glsl_std_450
    {IR(52291, 7), 81}, // SClamp in glsl_std_450
    {IR(52272, 5), 78}, // SMax in glsl_std_450
    {IR(52257, 5), 75}, // SMin in glsl_std_450
    {IR(52061, 6), 43}, // SSign in glsl_std_450
    {IR(52100, 4), 49}, // Sin in glsl_std_450
    {IR(52127, 5), 55}, // Sinh in glsl_std_450
    {IR(52313, 11), 85}, // SmoothStep in glsl_std_450
    {IR(52188, 5), 67}, // Sqrt in glsl_std_450
    {IR(52308, 5), 84}, // Step in glsl_std_450
    {IR(52108, 4), 51}, // Tan in glsl_std_450
    {IR(52137, 5), 57}, // Tanh in glsl_std_450
    {IR(52039, 6), 39}, // Trunc in glsl_std_450
    {IR(52284, 7), 80}, // UClamp in glsl_std_450
    {IR(52267, 5), 77}, // UMax in glsl_std_450
    {IR(52252, 5), 74}, // UMin in glsl_std_450
    {IR(52511, 17), 101}, // UnpackDouble2x32 in glsl_std_450
    {IR(52466, 15), 98}, // UnpackHalf2x16 in glsl_std_450
    {IR(52434, 16), 96}, // UnpackSnorm2x16 in glsl_std_450
    {IR(52481, 15), 99}, // UnpackSnorm4x8 in glsl_std_450
    {IR(52450, 16), 97}, // UnpackUnorm2x16 in glsl_std_450
    {IR(52496, 15), 100}, // UnpackUnorm4x8 in glsl_std_450
    {IR(52694, 13), 119}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(52789, 24), 124}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(52745, 25), 122}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(52770, 19), 123}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(53223, 28), 143}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(53251, 23), 144}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(52813, 21), 125}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(52855, 16), 127}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(52707, 22), 120}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(52834, 21), 126}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(53511, 27), 151}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(52729, 16), 121}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(53538, 27), 152}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(52871, 18), 128}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(53565, 32), 153}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(53119, 26), 138}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(53145, 20), 139}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(53389, 45), 148}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(53471, 40), 150}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(53347, 42), 147}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(53434, 37), 149}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5641, 7), 118}, // Kernel in nonsemantic_clspvreflection
    {IR(9161, 15), 140}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(53706, 34), 158}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(53674, 32), 157}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(53648, 26), 156}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(53637, 11), 155}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(53597, 40), 154}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(53309, 38), 146}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(53274, 35), 145}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(53165, 30), 141}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(52985, 30), 133}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(52960, 25), 132}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(53015, 23), 134}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(53063, 26), 136}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(53089, 30), 137}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(53038, 25), 135}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(52915, 25), 130}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(53195, 28), 142}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(52940, 20), 131}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(52889, 26), 129}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(53740, 22), 159}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(53861, 21), 200}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {IR(51178, 21), 161}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {IR(51905, 13), 188}, // DebugDeclare in nonsemantic_shader_debuginfo
    {IR(53899, 16), 202}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {IR(51977, 16), 191}, // DebugExpression in nonsemantic_shader_debuginfo
    {IR(51721, 14), 180}, // DebugFunction in nonsemantic_shader_debuginfo
    {IR(51696, 25), 179}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {IR(53794, 24), 196}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {IR(51676, 20), 178}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {IR(53762, 20), 194}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {IR(51164, 14), 160}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {IR(51808, 15), 185}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {IR(51884, 21), 187}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {IR(51735, 18), 181}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {IR(51753, 31), 182}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {IR(53839, 10), 198}, // DebugLine in nonsemantic_shader_debuginfo
    {IR(51865, 19), 186}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {IR(51993, 14), 192}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {IR(52007, 16), 193}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {IR(53849, 12), 199}, // DebugNoLine in nonsemantic_shader_debuginfo
    {IR(51795, 13), 184}, // DebugNoScope in nonsemantic_shader_debuginfo
    {IR(51962, 15), 190}, // DebugOperation in nonsemantic_shader_debuginfo
    {IR(51784, 11), 183}, // DebugScope in nonsemantic_shader_debuginfo
    {IR(53782, 12), 195}, // DebugSource in nonsemantic_shader_debuginfo
    {IR(53818, 21), 197}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {IR(53882, 17), 201}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {IR(51374, 15), 165}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {IR(51251, 15), 162}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {IR(51488, 19), 170}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {IR(53952, 30), 205}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {IR(51436, 14), 169}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {IR(51418, 18), 168}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {IR(51523, 21), 172}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {IR(53915, 16), 203}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {IR(51507, 16), 171}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {IR(51300, 17), 163}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {IR(51544, 21), 173}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {IR(51355, 19), 164}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {IR(51565, 18), 174}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {IR(51583, 27), 175}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {IR(51645, 31), 177}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {IR(51610, 35), 176}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {IR(51389, 16), 166}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {IR(53931, 21), 204}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {IR(51405, 13), 167}, // DebugTypedef in nonsemantic_shader_debuginfo
    {IR(51918, 11), 189}, // DebugValue in nonsemantic_shader_debuginfo
    {IR(53982, 14), 206}, // Configuration in nonsemantic_vkspreflection
    {IR(54058, 20), 211}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(54078, 19), 212}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(54097, 21), 213}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(54021, 14), 209}, // PushConstants in nonsemantic_vkspreflection
    {IR(54035, 23), 210}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(53996, 13), 207}, // StartCounter in nonsemantic_vkspreflection
    {IR(54009, 12), 208}, // StopCounter in nonsemantic_vkspreflection
    {IR(51178, 21), 215}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(51905, 13), 242}, // DebugDeclare in opencl_debuginfo_100
    {IR(51977, 16), 245}, // DebugExpression in opencl_debuginfo_100
    {IR(51721, 14), 234}, // DebugFunction in opencl_debuginfo_100
    {IR(51696, 25), 233}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(51676, 20), 232}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(53762, 20), 248}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(51164, 14), 214}, // DebugInfoNone in opencl_debuginfo_100
    {IR(51808, 15), 239}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(51884, 21), 241}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(51735, 18), 235}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(51753, 31), 236}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(51865, 19), 240}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(51993, 14), 246}, // DebugMacroDef in opencl_debuginfo_100
    {IR(52007, 16), 247}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(54418, 17), 250}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(51795, 13), 238}, // DebugNoScope in opencl_debuginfo_100
    {IR(51962, 15), 244}, // DebugOperation in opencl_debuginfo_100
    {IR(51784, 11), 237}, // DebugScope in opencl_debuginfo_100
    {IR(53782, 12), 249}, // DebugSource in opencl_debuginfo_100
    {IR(51374, 15), 219}, // DebugTypeArray in opencl_debuginfo_100
    {IR(51251, 15), 216}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(51488, 19), 224}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(51436, 14), 223}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(51418, 18), 222}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(51523, 21), 226}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(51507, 16), 225}, // DebugTypeMember in opencl_debuginfo_100
    {IR(51300, 17), 217}, // DebugTypePointer in opencl_debuginfo_100
    {IR(51544, 21), 227}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(51355, 19), 218}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(51565, 18), 228}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(51583, 27), 229}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(51645, 31), 231}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(51610, 35), 230}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(51389, 16), 220}, // DebugTypeVector in opencl_debuginfo_100
    {IR(51405, 13), 221}, // DebugTypedef in opencl_debuginfo_100
    {IR(51918, 11), 243}, // DebugValue in opencl_debuginfo_100
    {IR(54435, 5), 251}, // acos in opencl_std_100
    {IR(54440, 6), 252}, // acosh in opencl_std_100
    {IR(54446, 7), 253}, // acospi in opencl_std_100
    {IR(54453, 5), 254}, // asin in opencl_std_100
    {IR(54458, 6), 255}, // asinh in opencl_std_100
    {IR(54464, 7), 256}, // asinpi in opencl_std_100
    {IR(54471, 5), 257}, // atan in opencl_std_100
    {IR(54476, 6), 258}, // atan2 in opencl_std_100
    {IR(54495, 8), 261}, // atan2pi in opencl_std_100
    {IR(54482, 6), 259}, // atanh in opencl_std_100
    {IR(54488, 7), 260}, // atanpi in opencl_std_100
    {IR(55686, 10), 407}, // bitselect in opencl_std_100
    {IR(54503, 5), 262}, // cbrt in opencl_std_100
    {IR(54508, 5), 263}, // ceil in opencl_std_100
    {IR(55358, 4), 372}, // clz in opencl_std_100
    {IR(54513, 9), 264}, // copysign in opencl_std_100
    {IR(54522, 4), 265}, // cos in opencl_std_100
    {IR(54526, 5), 266}, // cosh in opencl_std_100
    {IR(54531, 6), 267}, // cospi in opencl_std_100
    {IR(55202, 6), 355}, // cross in opencl_std_100
    {IR(55362, 4), 373}, // ctz in opencl_std_100
    {IR(55137, 8), 347}, // degrees in opencl_std_100
    {IR(55208, 9), 356}, // distance in opencl_std_100
    {IR(54542, 4), 269}, // erf in opencl_std_100
    {IR(54537, 5), 268}, // erfc in opencl_std_100
    {IR(54546, 4), 270}, // exp in opencl_std_100
    {IR(54555, 6), 272}, // exp10 in opencl_std_100
    {IR(54550, 5), 271}, // exp2 in opencl_std_100
    {IR(54561, 6), 273}, // expm1 in opencl_std_100
    {IR(54567, 5), 274}, // fabs in opencl_std_100
    {IR(55234, 14), 359}, // fast_distance in opencl_std_100
    {IR(55248, 12), 360}, // fast_length in opencl_std_100
    {IR(55260, 15), 361}, // fast_normalize in opencl_std_100
    {IR(55130, 7), 346}, // fclamp in opencl_std_100
    {IR(54572, 5), 275}, // fdim in opencl_std_100
    {IR(54577, 6), 276}, // floor in opencl_std_100
    {IR(54583, 4), 277}, // fma in opencl_std_100
    {IR(54587, 5), 278}, // fmax in opencl_std_100
    {IR(55145, 12), 348}, // fmax_common in opencl_std_100
    {IR(54592, 5), 279}, // fmin in opencl_std_100
    {IR(55157, 12), 349}, // fmin_common in opencl_std_100
    {IR(54597, 5), 280}, // fmod in opencl_std_100
    {IR(54602, 6), 281}, // fract in opencl_std_100
    {IR(54608, 6), 282}, // frexp in opencl_std_100
    {IR(54820, 9), 318}, // half_cos in opencl_std_100
    {IR(54829, 12), 319}, // half_divide in opencl_std_100
    {IR(54841, 9), 320}, // half_exp in opencl_std_100
    {IR(54860, 11), 322}, // half_exp10 in opencl_std_100
    {IR(54850, 10), 321}, // half_exp2 in opencl_std_100
    {IR(54871, 9), 323}, // half_log in opencl_std_100
    {IR(54890, 11), 325}, // half_log10 in opencl_std_100
    {IR(54880, 10), 324}, // half_log2 in opencl_std_100
    {IR(54901, 10), 326}, // half_powr in opencl_std_100
    {IR(54911, 11), 327}, // half_recip in opencl_std_100
    {IR(54922, 11), 328}, // half_rsqrt in opencl_std_100
    {IR(54933, 9), 329}, // half_sin in opencl_std_100
    {IR(54942, 10), 330}, // half_sqrt in opencl_std_100
    {IR(54952, 9), 331}, // half_tan in opencl_std_100
    {IR(54614, 6), 283}, // hypot in opencl_std_100
    {IR(54620, 6), 284}, // ilogb in opencl_std_100
    {IR(54626, 6), 285}, // ldexp in opencl_std_100
    {IR(55217, 7), 357}, // length in opencl_std_100
    {IR(54632, 7), 286}, // lgamma in opencl_std_100
    {IR(54639, 9), 287}, // lgamma_r in opencl_std_100
    {IR(54648, 4), 288}, // log in opencl_std_100
    {IR(54657, 6), 290}, // log10 in opencl_std_100
    {IR(54663, 6), 291}, // log1p in opencl_std_100
    {IR(54652, 5), 289}, // log2 in opencl_std_100
    {IR(54669, 5), 292}, // logb in opencl_std_100
    {IR(54674, 4), 293}, // mad in opencl_std_100
    {IR(54678, 7), 294}, // maxmag in opencl_std_100
    {IR(54685, 7), 295}, // minmag in opencl_std_100
    {IR(55169, 4), 350}, // mix in opencl_std_100
    {IR(54692, 5), 296}, // modf in opencl_std_100
    {IR(54697, 4), 297}, // nan in opencl_std_100
    {IR(54961, 11), 332}, // native_cos in opencl_std_100
    {IR(54972, 14), 333}, // native_divide in opencl_std_100
    {IR(54986, 11), 334}, // native_exp in opencl_std_100
    {IR(55009, 13), 336}, // native_exp10 in opencl_std_100
    {IR(54997, 12), 335}, // native_exp2 in opencl_std_100
    {IR(55022, 11), 337}, // native_log in opencl_std_100
    {IR(55045, 13), 339}, // native_log10 in opencl_std_100
    {IR(55033, 12), 338}, // native_log2 in opencl_std_100
    {IR(55058, 12), 340}, // native_powr in opencl_std_100
    {IR(55070, 13), 341}, // native_recip in opencl_std_100
    {IR(55083, 13), 342}, // native_rsqrt in opencl_std_100
    {IR(55096, 11), 343}, // native_sin in opencl_std_100
    {IR(55107, 12), 344}, // native_sqrt in opencl_std_100
    {IR(55119, 11), 345}, // native_tan in opencl_std_100
    {IR(54701, 10), 298}, // nextafter in opencl_std_100
    {IR(55224, 10), 358}, // normalize in opencl_std_100
    {IR(55477, 9), 387}, // popcount in opencl_std_100
    {IR(54711, 4), 299}, // pow in opencl_std_100
    {IR(54715, 5), 300}, // pown in opencl_std_100
    {IR(54720, 5), 301}, // powr in opencl_std_100
    {IR(55677, 9), 406}, // prefetch in opencl_std_100
    {IR(55670, 7), 405}, // printf in opencl_std_100
    {IR(55173, 8), 351}, // radians in opencl_std_100
    {IR(54725, 10), 302}, // remainder in opencl_std_100
    {IR(54735, 7), 303}, // remquo in opencl_std_100
    {IR(54742, 5), 304}, // rint in opencl_std_100
    {IR(54747, 6), 305}, // rootn in opencl_std_100
    {IR(55428, 7), 382}, // rotate in opencl_std_100
    {IR(54753, 6), 306}, // round in opencl_std_100
    {IR(54759, 6), 307}, // rsqrt in opencl_std_100
    {IR(55275, 6), 362}, // s_abs in opencl_std_100
    {IR(55281, 11), 363}, // s_abs_diff in opencl_std_100
    {IR(55292, 10), 364}, // s_add_sat in opencl_std_100
    {IR(55342, 8), 370}, // s_clamp in opencl_std_100
    {IR(55312, 7), 366}, // s_hadd in opencl_std_100
    {IR(55486, 8), 388}, // s_mad24 in opencl_std_100
    {IR(55366, 9), 374}, // s_mad_hi in opencl_std_100
    {IR(55385, 10), 376}, // s_mad_sat in opencl_std_100
    {IR(55395, 6), 377}, // s_max in opencl_std_100
    {IR(55407, 6), 379}, // s_min in opencl_std_100
    {IR(55502, 8), 390}, // s_mul24 in opencl_std_100
    {IR(55419, 9), 381}, // s_mul_hi in opencl_std_100
    {IR(55326, 8), 368}, // s_rhadd in opencl_std_100
    {IR(55435, 10), 383}, // s_sub_sat in opencl_std_100
    {IR(55466, 11), 386}, // s_upsample in opencl_std_100
    {IR(55696, 7), 408}, // select in opencl_std_100
    {IR(55653, 8), 403}, // shuffle in opencl_std_100
    {IR(55661, 9), 404}, // shuffle2 in opencl_std_100
    {IR(55197, 5), 354}, // sign in opencl_std_100
    {IR(54765, 4), 308}, // sin in opencl_std_100
    {IR(54769, 7), 309}, // sincos in opencl_std_100
    {IR(54776, 5), 310}, // sinh in opencl_std_100
    {IR(54781, 6), 311}, // sinpi in opencl_std_100
    {IR(55186, 11), 353}, // smoothstep in opencl_std_100
    {IR(54787, 5), 312}, // sqrt in opencl_std_100
    {IR(55181, 5), 352}, // step in opencl_std_100
    {IR(54792, 4), 313}, // tan in opencl_std_100
    {IR(54796, 5), 314}, // tanh in opencl_std_100
    {IR(54801, 6), 315}, // tanpi in opencl_std_100
    {IR(54807, 7), 316}, // tgamma in opencl_std_100
    {IR(54814, 6), 317}, // trunc in opencl_std_100
    {IR(55703, 6), 409}, // u_abs in opencl_std_100
    {IR(55709, 11), 410}, // u_abs_diff in opencl_std_100
    {IR(55302, 10), 365}, // u_add_sat in opencl_std_100
    {IR(55350, 8), 371}, // u_clamp in opencl_std_100
    {IR(55319, 7), 367}, // u_hadd in opencl_std_100
    {IR(55494, 8), 389}, // u_mad24 in opencl_std_100
    {IR(55729, 9), 412}, // u_mad_hi in opencl_std_100
    {IR(55375, 10), 375}, // u_mad_sat in opencl_std_100
    {IR(55401, 6), 378}, // u_max in opencl_std_100
    {IR(55413, 6), 380}, // u_min in opencl_std_100
    {IR(55510, 8), 391}, // u_mul24 in opencl_std_100
    {IR(55720, 9), 411}, // u_mul_hi in opencl_std_100
    {IR(55334, 8), 369}, // u_rhadd in opencl_std_100
    {IR(55445, 10), 384}, // u_sub_sat in opencl_std_100
    {IR(55455, 11), 385}, // u_upsample in opencl_std_100
    {IR(55533, 11), 394}, // vload_half in opencl_std_100
    {IR(55544, 12), 395}, // vload_halfn in opencl_std_100
    {IR(55610, 13), 400}, // vloada_halfn in opencl_std_100
    {IR(55518, 7), 392}, // vloadn in opencl_std_100
    {IR(55556, 12), 396}, // vstore_half in opencl_std_100
    {IR(55568, 14), 397}, // vstore_half_r in opencl_std_100
    {IR(55582, 13), 398}, // vstore_halfn in opencl_std_100
    {IR(55595, 15), 399}, // vstore_halfn_r in opencl_std_100
    {IR(55623, 14), 401}, // vstorea_halfn in opencl_std_100
    {IR(55637, 16), 402}, // vstorea_halfn_r in opencl_std_100
    {IR(55525, 8), 393}, // vstoren in opencl_std_100
    {IR(55755, 17), 414}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(55738, 17), 413}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(55772, 8), 415}, // TimeAMD in spv_amd_gcn_shader
    {IR(55849, 9), 419}, // MbcntAMD in spv_amd_shader_ballot
    {IR(55780, 22), 416}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(55802, 28), 417}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(55830, 19), 418}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(55858, 23), 420}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(55908, 9), 424}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55935, 9), 427}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55881, 9), 421}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(55926, 9), 426}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55953, 9), 429}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55899, 9), 423}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(55917, 9), 425}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(55944, 9), 428}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(55890, 9), 422}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56259, 4), 461}, // ABS in tosa_001000_1
    {IR(56082, 4), 444}, // ADD in tosa_001000_1
    {IR(55962, 7), 430}, // ARGMAX in tosa_001000_1
    {IR(56086, 23), 445}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(55969, 11), 431}, // AVG_POOL2D in tosa_001000_1
    {IR(56109, 12), 446}, // BITWISE_AND in tosa_001000_1
    {IR(56263, 12), 462}, // BITWISE_NOT in tosa_001000_1
    {IR(56121, 11), 447}, // BITWISE_OR in tosa_001000_1
    {IR(56132, 12), 448}, // BITWISE_XOR in tosa_001000_1
    {IR(56517, 5), 494}, // CAST in tosa_001000_1
    {IR(56275, 5), 463}, // CEIL in tosa_001000_1
    {IR(56059, 6), 440}, // CLAMP in tosa_001000_1
    {IR(56280, 4), 464}, // CLZ in tosa_001000_1
    {IR(56447, 7), 484}, // CONCAT in tosa_001000_1
    {IR(55980, 7), 432}, // CONV2D in tosa_001000_1
    {IR(55987, 7), 433}, // CONV3D in tosa_001000_1
    {IR(56284, 4), 465}, // COS in tosa_001000_1
    {IR(55994, 17), 434}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(56349, 6), 475}, // EQUAL in tosa_001000_1
    {IR(56065, 4), 441}, // ERF in tosa_001000_1
    {IR(56288, 4), 466}, // EXP in tosa_001000_1
    {IR(56011, 6), 435}, // FFT2D in tosa_001000_1
    {IR(56292, 6), 467}, // FLOOR in tosa_001000_1
    {IR(56495, 7), 491}, // GATHER in tosa_001000_1
    {IR(56355, 8), 476}, // GREATER in tosa_001000_1
    {IR(56363, 14), 477}, // GREATER_EQUAL in tosa_001000_1
    {IR(56144, 7), 449}, // INTDIV in tosa_001000_1
    {IR(56298, 4), 468}, // LOG in tosa_001000_1
    {IR(56151, 12), 450}, // LOGICAL_AND in tosa_001000_1
    {IR(56163, 19), 451}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(56302, 12), 469}, // LOGICAL_NOT in tosa_001000_1
    {IR(56202, 11), 453}, // LOGICAL_OR in tosa_001000_1
    {IR(56182, 20), 452}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(56213, 12), 454}, // LOGICAL_XOR in tosa_001000_1
    {IR(56017, 7), 436}, // MATMUL in tosa_001000_1
    {IR(56225, 8), 455}, // MAXIMUM in tosa_001000_1
    {IR(56024, 11), 437}, // MAX_POOL2D in tosa_001000_1
    {IR(56233, 8), 456}, // MINIMUM in tosa_001000_1
    {IR(56241, 4), 457}, // MUL in tosa_001000_1
    {IR(56314, 7), 470}, // NEGATE in tosa_001000_1
    {IR(56454, 4), 485}, // PAD in tosa_001000_1
    {IR(56245, 4), 458}, // POW in tosa_001000_1
    {IR(56321, 11), 471}, // RECIPROCAL in tosa_001000_1
    {IR(56377, 11), 478}, // REDUCE_ALL in tosa_001000_1
    {IR(56388, 11), 479}, // REDUCE_ANY in tosa_001000_1
    {IR(56399, 11), 480}, // REDUCE_MAX in tosa_001000_1
    {IR(56410, 11), 481}, // REDUCE_MIN in tosa_001000_1
    {IR(56421, 15), 482}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(56436, 11), 483}, // REDUCE_SUM in tosa_001000_1
    {IR(56522, 8), 495}, // RESCALE in tosa_001000_1
    {IR(56458, 8), 486}, // RESHAPE in tosa_001000_1
    {IR(56510, 7), 493}, // RESIZE in tosa_001000_1
    {IR(56466, 8), 487}, // REVERSE in tosa_001000_1
    {IR(56035, 7), 438}, // RFFT2D in tosa_001000_1
    {IR(56332, 6), 472}, // RSQRT in tosa_001000_1
    {IR(56502, 8), 492}, // SCATTER in tosa_001000_1
    {IR(56342, 7), 474}, // SELECT in tosa_001000_1
    {IR(56069, 8), 442}, // SIGMOID in tosa_001000_1
    {IR(56338, 4), 473}, // SIN in tosa_001000_1
    {IR(56474, 6), 488}, // SLICE in tosa_001000_1
    {IR(56249, 4), 459}, // SUB in tosa_001000_1
    {IR(56253, 6), 460}, // TABLE in tosa_001000_1
    {IR(56077, 5), 443}, // TANH in tosa_001000_1
    {IR(56480, 5), 489}, // TILE in tosa_001000_1
    {IR(56485, 10), 490}, // TRANSPOSE in tosa_001000_1
    {IR(56042, 17), 439}, // TRANSPOSE_CONV2D in tosa_001000_1
}};

IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(0, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(3, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(37, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(118, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(160, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(206, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(214, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(251, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(413, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(416, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(420, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(421, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(430, 66);
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
  "SPV_INTEL_predicated_io\0" // IR(3046, 24)
  "SPV_INTEL_rounded_divide_sqrt\0" // IR(3070, 30)
  "SPV_INTEL_runtime_aligned\0" // IR(3100, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(3126, 36)
  "SPV_INTEL_split_barrier\0" // IR(3162, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3186, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3221, 46)
  "SPV_INTEL_subgroups\0" // IR(3267, 20)
  "SPV_INTEL_task_sequence\0" // IR(3287, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3311, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3347, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3382, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3419, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3449, 32)
  "SPV_INTEL_vector_compute\0" // IR(3481, 25)
  "SPV_KHR_16bit_storage\0" // IR(3506, 22)
  "SPV_KHR_8bit_storage\0" // IR(3528, 21)
  "SPV_KHR_abort\0" // IR(3549, 14)
  "SPV_KHR_bfloat16\0" // IR(3563, 17)
  "SPV_KHR_bit_instructions\0" // IR(3580, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3605, 35)
  "SPV_KHR_constant_data\0" // IR(3640, 22)
  "SPV_KHR_cooperative_matrix\0" // IR(3662, 27)
  "SPV_KHR_device_group\0" // IR(3689, 21)
  "SPV_KHR_expect_assume\0" // IR(3710, 22)
  "SPV_KHR_float_controls\0" // IR(3732, 23)
  "SPV_KHR_float_controls2\0" // IR(3755, 24)
  "SPV_KHR_fma\0" // IR(3779, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3791, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3827, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3857, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3885, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3906, 30)
  "SPV_KHR_multiview\0" // IR(3936, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3954, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3989, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(4015, 32)
  "SPV_KHR_poison_freeze\0" // IR(4047, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(4069, 28)
  "SPV_KHR_quad_control\0" // IR(4097, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4118, 22)
  "SPV_KHR_ray_query\0" // IR(4140, 18)
  "SPV_KHR_ray_tracing\0" // IR(4158, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4178, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4213, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4250, 34)
  "SPV_KHR_shader_ballot\0" // IR(4284, 22)
  "SPV_KHR_shader_clock\0" // IR(4306, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4327, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4358, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4395, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4419, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4457, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4479, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4508, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4543, 25)
  "SPV_KHR_variable_pointers\0" // IR(4568, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4594, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4622, 41)
  "SPV_MESA_fragment_coverage_mask\0" // IR(4663, 32)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4695, 38)
  "SPV_NV_bindless_texture\0" // IR(4733, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4757, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4795, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4829, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4855, 27)
  "SPV_NV_cooperative_vector\0" // IR(4882, 26)
  "SPV_NV_displacement_micromap\0" // IR(4908, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(4937, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(4972, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(5007, 28)
  "SPV_NV_mesh_shader\0" // IR(5035, 19)
  "SPV_NV_push_constant_bank\0" // IR(5054, 26)
  "SPV_NV_raw_access_chains\0" // IR(5080, 25)
  "SPV_NV_ray_tracing\0" // IR(5105, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5124, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5155, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5192, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5225, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5255, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5288, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5314, 35)
  "SPV_NV_shading_rate\0" // IR(5349, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5369, 29)
  "SPV_NV_tensor_addressing\0" // IR(5398, 25)
  "SPV_NV_viewport_array2\0" // IR(5423, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5446, 39)
  "SPV_QCOM_image_processing\0" // IR(5485, 26)
  "SPV_QCOM_image_processing2\0" // IR(5511, 27)
  "SPV_QCOM_tile_shading\0" // IR(5538, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5560, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5598, 34)
  "ReadOnly\0" // IR(5632, 9)
  "Kernel\0" // IR(5641, 7)
  "WriteOnly\0" // IR(5648, 10)
  "ReadWrite\0" // IR(5658, 10)
  "Logical\0" // IR(5668, 8)
  "Physical32\0" // IR(5676, 11)
  "Addresses\0" // IR(5687, 10)
  "Physical64\0" // IR(5697, 11)
  "PhysicalStorageBuffer64\0" // IR(5708, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(5732, 27)
  "PhysicalStorageBufferAddresses\0" // IR(5759, 31)
  "Position\0" // IR(5790, 9)
  "Shader\0" // IR(5799, 7)
  "PointSize\0" // IR(5806, 10)
  "ClipDistance\0" // IR(5816, 13)
  "CullDistance\0" // IR(5829, 13)
  "VertexId\0" // IR(5842, 9)
  "InstanceId\0" // IR(5851, 11)
  "PrimitiveId\0" // IR(5862, 12)
  "Geometry\0" // IR(5874, 9)
  "Tessellation\0" // IR(5883, 13)
  "RayTracingNV\0" // IR(5896, 13)
  "RayTracingKHR\0" // IR(5909, 14)
  "MeshShadingNV\0" // IR(5923, 14)
  "MeshShadingEXT\0" // IR(5937, 15)
  "InvocationId\0" // IR(5952, 13)
  "Layer\0" // IR(5965, 6)
  "ShaderLayer\0" // IR(5971, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(5983, 28)
  "ViewportIndex\0" // IR(6011, 14)
  "MultiViewport\0" // IR(6025, 14)
  "ShaderViewportIndex\0" // IR(6039, 20)
  "TessLevelOuter\0" // IR(6059, 15)
  "TessLevelInner\0" // IR(6074, 15)
  "TessCoord\0" // IR(6089, 10)
  "PatchVertices\0" // IR(6099, 14)
  "FragCoord\0" // IR(6113, 10)
  "PointCoord\0" // IR(6123, 11)
  "FrontFacing\0" // IR(6134, 12)
  "SampleId\0" // IR(6146, 9)
  "SampleRateShading\0" // IR(6155, 18)
  "SamplePosition\0" // IR(6173, 15)
  "SampleMask\0" // IR(6188, 11)
  "FragDepth\0" // IR(6199, 10)
  "HelperInvocation\0" // IR(6209, 17)
  "NumWorkgroups\0" // IR(6226, 14)
  "WorkgroupSize\0" // IR(6240, 14)
  "WorkgroupId\0" // IR(6254, 12)
  "LocalInvocationId\0" // IR(6266, 18)
  "GlobalInvocationId\0" // IR(6284, 19)
  "LocalInvocationIndex\0" // IR(6303, 21)
  "WorkDim\0" // IR(6324, 8)
  "GlobalSize\0" // IR(6332, 11)
  "EnqueuedWorkgroupSize\0" // IR(6343, 22)
  "GlobalOffset\0" // IR(6365, 13)
  "GlobalLinearId\0" // IR(6378, 15)
  "SubgroupSize\0" // IR(6393, 13)
  "GroupNonUniform\0" // IR(6406, 16)
  "SubgroupBallotKHR\0" // IR(6422, 18)
  "SubgroupMaxSize\0" // IR(6440, 16)
  "NumSubgroups\0" // IR(6456, 13)
  "NumEnqueuedSubgroups\0" // IR(6469, 21)
  "SubgroupId\0" // IR(6490, 11)
  "SubgroupLocalInvocationId\0" // IR(6501, 26)
  "VertexIndex\0" // IR(6527, 12)
  "InstanceIndex\0" // IR(6539, 14)
  "FragmentCoverageMaskMESA\0" // IR(6553, 25)
  "FragmentCoverageMESA\0" // IR(6578, 21)
  "CoreIDARM\0" // IR(6599, 10)
  "CoreBuiltinsARM\0" // IR(6609, 16)
  "CoreCountARM\0" // IR(6625, 13)
  "CoreMaxIDARM\0" // IR(6638, 13)
  "WarpIDARM\0" // IR(6651, 10)
  "WarpMaxIDARM\0" // IR(6661, 13)
  "SubgroupEqMask\0" // IR(6674, 15)
  "SubgroupEqMaskKHR\0" // IR(6689, 18)
  "GroupNonUniformBallot\0" // IR(6707, 22)
  "SubgroupGeMask\0" // IR(6729, 15)
  "SubgroupGeMaskKHR\0" // IR(6744, 18)
  "SubgroupGtMask\0" // IR(6762, 15)
  "SubgroupGtMaskKHR\0" // IR(6777, 18)
  "SubgroupLeMask\0" // IR(6795, 15)
  "SubgroupLeMaskKHR\0" // IR(6810, 18)
  "SubgroupLtMask\0" // IR(6828, 15)
  "SubgroupLtMaskKHR\0" // IR(6843, 18)
  "BaseVertex\0" // IR(6861, 11)
  "DrawParameters\0" // IR(6872, 15)
  "BaseInstance\0" // IR(6887, 13)
  "DrawIndex\0" // IR(6900, 10)
  "PrimitiveShadingRateKHR\0" // IR(6910, 24)
  "FragmentShadingRateKHR\0" // IR(6934, 23)
  "DeviceIndex\0" // IR(6957, 12)
  "DeviceGroup\0" // IR(6969, 12)
  "ViewIndex\0" // IR(6981, 10)
  "MultiView\0" // IR(6991, 10)
  "ShadingRateKHR\0" // IR(7001, 15)
  "TileOffsetQCOM\0" // IR(7016, 15)
  "TileShadingQCOM\0" // IR(7031, 16)
  "TileDimensionQCOM\0" // IR(7047, 18)
  "TileApronSizeQCOM\0" // IR(7065, 18)
  "BaryCoordNoPerspAMD\0" // IR(7083, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(7103, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(7131, 26)
  "BaryCoordSmoothAMD\0" // IR(7157, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7176, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7203, 25)
  "BaryCoordPullModelAMD\0" // IR(7228, 22)
  "FragStencilRefEXT\0" // IR(7250, 18)
  "StencilExportEXT\0" // IR(7268, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7285, 29)
  "ShaderEnqueueAMDX\0" // IR(7314, 18)
  "ShaderIndexAMDX\0" // IR(7332, 16)
  "SamplerHeapEXT\0" // IR(7348, 15)
  "DescriptorHeapEXT\0" // IR(7363, 18)
  "ResourceHeapEXT\0" // IR(7381, 16)
  "ViewportMaskNV\0" // IR(7397, 15)
  "ShaderViewportMaskNV\0" // IR(7412, 21)
  "SecondaryPositionNV\0" // IR(7433, 20)
  "ShaderStereoViewNV\0" // IR(7453, 19)
  "SecondaryViewportMaskNV\0" // IR(7472, 24)
  "PositionPerViewNV\0" // IR(7496, 18)
  "PerViewAttributesNV\0" // IR(7514, 20)
  "ViewportMaskPerViewNV\0" // IR(7534, 22)
  "FullyCoveredEXT\0" // IR(7556, 16)
  "FragmentFullyCoveredEXT\0" // IR(7572, 24)
  "TaskCountNV\0" // IR(7596, 12)
  "PrimitiveCountNV\0" // IR(7608, 17)
  "PrimitiveIndicesNV\0" // IR(7625, 19)
  "ClipDistancePerViewNV\0" // IR(7644, 22)
  "CullDistancePerViewNV\0" // IR(7666, 22)
  "LayerPerViewNV\0" // IR(7688, 15)
  "MeshViewCountNV\0" // IR(7703, 16)
  "MeshViewIndicesNV\0" // IR(7719, 18)
  "BaryCoordKHR\0" // IR(7737, 13)
  "BaryCoordNV\0" // IR(7750, 12)
  "FragmentBarycentricKHR\0" // IR(7762, 23)
  "BaryCoordNoPerspKHR\0" // IR(7785, 20)
  "BaryCoordNoPerspNV\0" // IR(7805, 19)
  "FragSizeEXT\0" // IR(7824, 12)
  "FragmentSizeNV\0" // IR(7836, 15)
  "FragmentDensityEXT\0" // IR(7851, 19)
  "FragInvocationCountEXT\0" // IR(7870, 23)
  "InvocationsPerPixelNV\0" // IR(7893, 22)
  "PrimitivePointIndicesEXT\0" // IR(7915, 25)
  "PrimitiveLineIndicesEXT\0" // IR(7940, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(7964, 28)
  "CullPrimitiveEXT\0" // IR(7992, 17)
  "LaunchIdKHR\0" // IR(8009, 12)
  "LaunchIdNV\0" // IR(8021, 11)
  "LaunchSizeKHR\0" // IR(8032, 14)
  "LaunchSizeNV\0" // IR(8046, 13)
  "WorldRayOriginKHR\0" // IR(8059, 18)
  "WorldRayOriginNV\0" // IR(8077, 17)
  "WorldRayDirectionKHR\0" // IR(8094, 21)
  "WorldRayDirectionNV\0" // IR(8115, 20)
  "ObjectRayOriginKHR\0" // IR(8135, 19)
  "ObjectRayOriginNV\0" // IR(8154, 18)
  "ObjectRayDirectionKHR\0" // IR(8172, 22)
  "ObjectRayDirectionNV\0" // IR(8194, 21)
  "RayTminKHR\0" // IR(8215, 11)
  "RayTminNV\0" // IR(8226, 10)
  "RayTmaxKHR\0" // IR(8236, 11)
  "RayTmaxNV\0" // IR(8247, 10)
  "InstanceCustomIndexKHR\0" // IR(8257, 23)
  "InstanceCustomIndexNV\0" // IR(8280, 22)
  "ObjectToWorldKHR\0" // IR(8302, 17)
  "ObjectToWorldNV\0" // IR(8319, 16)
  "WorldToObjectKHR\0" // IR(8335, 17)
  "WorldToObjectNV\0" // IR(8352, 16)
  "HitTNV\0" // IR(8368, 7)
  "HitKindKHR\0" // IR(8375, 11)
  "HitKindNV\0" // IR(8386, 10)
  "CurrentRayTimeNV\0" // IR(8396, 17)
  "RayTracingMotionBlurNV\0" // IR(8413, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8436, 30)
  "RayTracingPositionFetchKHR\0" // IR(8466, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8493, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8527, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8560, 37)
  "IncomingRayFlagsKHR\0" // IR(8597, 20)
  "IncomingRayFlagsNV\0" // IR(8617, 19)
  "RayGeometryIndexKHR\0" // IR(8636, 20)
  "HitIsSphereNV\0" // IR(8656, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8670, 28)
  "HitIsLSSNV\0" // IR(8698, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8709, 39)
  "HitSpherePositionNV\0" // IR(8748, 20)
  "WarpsPerSMNV\0" // IR(8768, 13)
  "ShaderSMBuiltinsNV\0" // IR(8781, 19)
  "SMCountNV\0" // IR(8800, 10)
  "WarpIDNV\0" // IR(8810, 9)
  "SMIDNV\0" // IR(8819, 7)
  "HitLSSPositionsNV\0" // IR(8826, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(8844, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(8878, 33)
  "HitSphereRadiusNV\0" // IR(8911, 18)
  "HitLSSRadiiNV\0" // IR(8929, 14)
  "ClusterIDNV\0" // IR(8943, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(8955, 41)
  "CullMaskKHR\0" // IR(8996, 12)
  "RayCullMaskKHR\0" // IR(9008, 15)
  "Matrix\0" // IR(9023, 7)
  "Linkage\0" // IR(9030, 8)
  "Vector16\0" // IR(9038, 9)
  "Float16Buffer\0" // IR(9047, 14)
  "Float16\0" // IR(9061, 8)
  "Float64\0" // IR(9069, 8)
  "Int64\0" // IR(9077, 6)
  "Int64Atomics\0" // IR(9083, 13)
  "ImageBasic\0" // IR(9096, 11)
  "ImageReadWrite\0" // IR(9107, 15)
  "ImageMipmap\0" // IR(9122, 12)
  "Pipes\0" // IR(9134, 6)
  "Groups\0" // IR(9140, 7)
  "DeviceEnqueue\0" // IR(9147, 14)
  "LiteralSampler\0" // IR(9161, 15)
  "AtomicStorage\0" // IR(9176, 14)
  "Int16\0" // IR(9190, 6)
  "TessellationPointSize\0" // IR(9196, 22)
  "GeometryPointSize\0" // IR(9218, 18)
  "ImageGatherExtended\0" // IR(9236, 20)
  "StorageImageMultisample\0" // IR(9256, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9280, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9314, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9347, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9381, 33)
  "ImageCubeArray\0" // IR(9414, 15)
  "SampledCubeArray\0" // IR(9429, 17)
  "ImageRect\0" // IR(9446, 10)
  "SampledRect\0" // IR(9456, 12)
  "GenericPointer\0" // IR(9468, 15)
  "Int8\0" // IR(9483, 5)
  "InputAttachment\0" // IR(9488, 16)
  "SparseResidency\0" // IR(9504, 16)
  "MinLod\0" // IR(9520, 7)
  "Sampled1D\0" // IR(9527, 10)
  "Image1D\0" // IR(9537, 8)
  "SampledBuffer\0" // IR(9545, 14)
  "ImageBuffer\0" // IR(9559, 12)
  "ImageMSArray\0" // IR(9571, 13)
  "StorageImageExtendedFormats\0" // IR(9584, 28)
  "ImageQuery\0" // IR(9612, 11)
  "DerivativeControl\0" // IR(9623, 18)
  "InterpolationFunction\0" // IR(9641, 22)
  "TransformFeedback\0" // IR(9663, 18)
  "GeometryStreams\0" // IR(9681, 16)
  "StorageImageReadWithoutFormat\0" // IR(9697, 30)
  "StorageImageWriteWithoutFormat\0" // IR(9727, 31)
  "SubgroupDispatch\0" // IR(9758, 17)
  "NamedBarrier\0" // IR(9775, 13)
  "PipeStorage\0" // IR(9788, 12)
  "GroupNonUniformVote\0" // IR(9800, 20)
  "GroupNonUniformArithmetic\0" // IR(9820, 26)
  "GroupNonUniformShuffle\0" // IR(9846, 23)
  "GroupNonUniformShuffleRelative\0" // IR(9869, 31)
  "GroupNonUniformClustered\0" // IR(9900, 25)
  "GroupNonUniformQuad\0" // IR(9925, 20)
  "UniformDecoration\0" // IR(9945, 18)
  "TileImageColorReadAccessEXT\0" // IR(9963, 28)
  "TileImageDepthReadAccessEXT\0" // IR(9991, 28)
  "TileImageStencilReadAccessEXT\0" // IR(10019, 30)
  "TensorsARM\0" // IR(10049, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(10060, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(10097, 40)
  "GraphARM\0" // IR(10137, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(10146, 28)
  "Float8EXT\0" // IR(10174, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10184, 27)
  "CooperativeMatrixKHR\0" // IR(10211, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10232, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10265, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10308, 44)
  "SubgroupVoteKHR\0" // IR(10352, 16)
  "StorageBuffer16BitAccess\0" // IR(10368, 25)
  "StorageUniformBufferBlock16\0" // IR(10393, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10421, 35)
  "StorageUniform16\0" // IR(10456, 17)
  "StoragePushConstant16\0" // IR(10473, 22)
  "StorageInputOutput16\0" // IR(10495, 21)
  "VariablePointersStorageBuffer\0" // IR(10516, 30)
  "VariablePointers\0" // IR(10546, 17)
  "AtomicStorageOps\0" // IR(10563, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10580, 28)
  "StorageBuffer8BitAccess\0" // IR(10608, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10632, 34)
  "StoragePushConstant8\0" // IR(10666, 21)
  "DenormPreserve\0" // IR(10687, 15)
  "DenormFlushToZero\0" // IR(10702, 18)
  "SignedZeroInfNanPreserve\0" // IR(10720, 25)
  "RoundingModeRTE\0" // IR(10745, 16)
  "RoundingModeRTZ\0" // IR(10761, 16)
  "RayQueryProvisionalKHR\0" // IR(10777, 23)
  "RayQueryKHR\0" // IR(10800, 12)
  "UntypedPointersKHR\0" // IR(10812, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(10831, 32)
  "TextureSampleWeightedQCOM\0" // IR(10863, 26)
  "TextureBoxFilterQCOM\0" // IR(10889, 21)
  "TextureBlockMatchQCOM\0" // IR(10910, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(10932, 32)
  "TextureBlockMatch2QCOM\0" // IR(10964, 23)
  "Float16ImageAMD\0" // IR(10987, 16)
  "ImageGatherBiasLodAMD\0" // IR(11003, 22)
  "FragmentMaskAMD\0" // IR(11025, 16)
  "ImageReadWriteLodAMD\0" // IR(11041, 21)
  "Int64ImageEXT\0" // IR(11062, 14)
  "ShaderClockKHR\0" // IR(11076, 15)
  "QuadControlKHR\0" // IR(11091, 15)
  "Int4TypeINTEL\0" // IR(11106, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(11120, 27)
  "BFloat16TypeKHR\0" // IR(11147, 16)
  "BFloat16DotProductKHR\0" // IR(11163, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11185, 29)
  "AbortKHR\0" // IR(11214, 9)
  "ConstantDataKHR\0" // IR(11223, 16)
  "PoisonFreezeKHR\0" // IR(11239, 16)
  "SampleMaskOverrideCoverageNV\0" // IR(11255, 29)
  "GeometryShaderPassthroughNV\0" // IR(11284, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11312, 27)
  "ImageFootprintNV\0" // IR(11339, 17)
  "FragmentBarycentricNV\0" // IR(11356, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11378, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11409, 30)
  "ShadingRateNV\0" // IR(11439, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11453, 30)
  "GroupNonUniformPartitionedNV\0" // IR(11483, 29)
  "ShaderNonUniform\0" // IR(11512, 17)
  "ShaderNonUniformEXT\0" // IR(11529, 20)
  "RuntimeDescriptorArray\0" // IR(11549, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(11572, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(11598, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(11634, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(11673, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(11712, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(11754, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(11793, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(11835, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(11872, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(11912, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(11948, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(11987, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(12024, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(12064, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(12100, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(12139, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(12178, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12220, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12262, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12307, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12349, 45)
  "VulkanMemoryModel\0" // IR(12394, 18)
  "VulkanMemoryModelKHR\0" // IR(12412, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12433, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12462, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12494, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(12528, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(12560, 31)
  "RayTracingProvisionalKHR\0" // IR(12591, 25)
  "CooperativeMatrixNV\0" // IR(12616, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(12636, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(12669, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(12707, 32)
  "DemoteToHelperInvocation\0" // IR(12739, 25)
  "DemoteToHelperInvocationEXT\0" // IR(12764, 28)
  "DisplacementMicromapNV\0" // IR(12792, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(12815, 29)
  "ShaderInvocationReorderNV\0" // IR(12844, 26)
  "ShaderInvocationReorderEXT\0" // IR(12870, 27)
  "BindlessTextureNV\0" // IR(12897, 18)
  "RayQueryPositionFetchKHR\0" // IR(12915, 25)
  "CooperativeVectorNV\0" // IR(12940, 20)
  "AtomicFloat16VectorNV\0" // IR(12960, 22)
  "RawAccessChainsNV\0" // IR(12982, 18)
  "PushConstantBanksNV\0" // IR(13000, 20)
  "LongVectorEXT\0" // IR(13020, 14)
  "Shader64BitIndexingEXT\0" // IR(13034, 23)
  "CooperativeMatrixReductionsNV\0" // IR(13057, 30)
  "CooperativeMatrixConversionsNV\0" // IR(13087, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(13118, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(13158, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13194, 30)
  "CooperativeVectorTrainingNV\0" // IR(13224, 28)
  "TensorAddressingNV\0" // IR(13252, 19)
  "SubgroupShuffleINTEL\0" // IR(13271, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(13292, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(13319, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(13345, 31)
  "RoundToInfinityINTEL\0" // IR(13376, 21)
  "FloatingPointModeINTEL\0" // IR(13397, 23)
  "IntegerFunctions2INTEL\0" // IR(13420, 23)
  "FunctionPointersINTEL\0" // IR(13443, 22)
  "IndirectReferencesINTEL\0" // IR(13465, 24)
  "AsmINTEL\0" // IR(13489, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(13498, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(13521, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(13544, 23)
  "VectorComputeINTEL\0" // IR(13567, 19)
  "VectorAnyINTEL\0" // IR(13586, 15)
  "ExpectAssumeKHR\0" // IR(13601, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(13617, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(13650, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(13688, 39)
  "VariableLengthArrayINTEL\0" // IR(13727, 25)
  "FunctionFloatControlINTEL\0" // IR(13752, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(13778, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(13805, 26)
  "FPFastMathModeINTEL\0" // IR(13831, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(13851, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(13884, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(13916, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(13954, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(13991, 30)
  "FPGALoopControlsALTERA\0" // IR(14021, 23)
  "FPGALoopControlsINTEL\0" // IR(14044, 22)
  "KernelAttributesINTEL\0" // IR(14066, 22)
  "FPGAKernelAttributesINTEL\0" // IR(14088, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(14114, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(14139, 24)
  "FPGAClusterAttributesALTERA\0" // IR(14163, 28)
  "FPGAClusterAttributesINTEL\0" // IR(14191, 27)
  "LoopFuseALTERA\0" // IR(14218, 15)
  "LoopFuseINTEL\0" // IR(14233, 14)
  "FPGADSPControlALTERA\0" // IR(14247, 21)
  "FPGADSPControlINTEL\0" // IR(14268, 20)
  "MemoryAccessAliasingINTEL\0" // IR(14288, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(14314, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(14355, 40)
  "FPGABufferLocationALTERA\0" // IR(14395, 25)
  "FPGABufferLocationINTEL\0" // IR(14420, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(14444, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(14479, 34)
  "USMStorageClassesALTERA\0" // IR(14513, 24)
  "USMStorageClassesINTEL\0" // IR(14537, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(14560, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(14590, 29)
  "IOPipesALTERA\0" // IR(14619, 14)
  "IOPipesINTEL\0" // IR(14633, 13)
  "BlockingPipesALTERA\0" // IR(14646, 20)
  "BlockingPipesINTEL\0" // IR(14666, 19)
  "FPGARegALTERA\0" // IR(14685, 14)
  "FPGARegINTEL\0" // IR(14699, 13)
  "DotProductInputAll\0" // IR(14712, 19)
  "DotProductInputAllKHR\0" // IR(14731, 22)
  "DotProductInput4x8Bit\0" // IR(14753, 22)
  "DotProductInput4x8BitKHR\0" // IR(14775, 25)
  "DotProductInput4x8BitPacked\0" // IR(14800, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(14828, 31)
  "DotProduct\0" // IR(14859, 11)
  "DotProductKHR\0" // IR(14870, 14)
  "ReplicatedCompositesEXT\0" // IR(14884, 24)
  "BitInstructions\0" // IR(14908, 16)
  "GroupNonUniformRotateKHR\0" // IR(14924, 25)
  "FloatControls2\0" // IR(14949, 15)
  "FMAKHR\0" // IR(14964, 7)
  "AtomicFloat32AddEXT\0" // IR(14971, 20)
  "AtomicFloat64AddEXT\0" // IR(14991, 20)
  "LongCompositesINTEL\0" // IR(15011, 20)
  "OptNoneEXT\0" // IR(15031, 11)
  "OptNoneINTEL\0" // IR(15042, 13)
  "AtomicFloat16AddEXT\0" // IR(15055, 20)
  "DebugInfoModuleINTEL\0" // IR(15075, 21)
  "BFloat16ConversionINTEL\0" // IR(15096, 24)
  "SplitBarrierINTEL\0" // IR(15120, 18)
  "ArithmeticFenceEXT\0" // IR(15138, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(15157, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(15187, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15216, 28)
  "TaskSequenceALTERA\0" // IR(15244, 19)
  "TaskSequenceINTEL\0" // IR(15263, 18)
  "FPMaxErrorINTEL\0" // IR(15281, 16)
  "FPGALatencyControlALTERA\0" // IR(15297, 25)
  "FPGALatencyControlINTEL\0" // IR(15322, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(15346, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(15375, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(15403, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(15433, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(15469, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(15504, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(15532, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(15555, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(15585, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(15615, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(15653, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(15681, 32)
  "SpecConditionalINTEL\0" // IR(15713, 21)
  "FunctionVariantsINTEL\0" // IR(15734, 22)
  "PredicatedIOINTEL\0" // IR(15756, 18)
  "RoundedDivideSqrtINTEL\0" // IR(15774, 23)
  "GroupUniformArithmeticKHR\0" // IR(15797, 26)
  "TensorFloat32RoundingINTEL\0" // IR(15823, 27)
  "MaskedGatherScatterINTEL\0" // IR(15850, 25)
  "CacheControlsINTEL\0" // IR(15875, 19)
  "RegisterLimitsINTEL\0" // IR(15894, 20)
  "BindlessImagesINTEL\0" // IR(15914, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(15934, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(15967, 33)
  "DotProductBFloat16AccVALVE\0" // IR(16000, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(16027, 32)
  "Unspecified\0" // IR(16059, 12)
  "Address\0" // IR(16071, 8)
  "Boolean\0" // IR(16079, 8)
  "Float\0" // IR(16087, 6)
  "Signed\0" // IR(16093, 7)
  "SignedChar\0" // IR(16100, 11)
  "Unsigned\0" // IR(16111, 9)
  "UnsignedChar\0" // IR(16120, 13)
  "Class\0" // IR(16133, 6)
  "Structure\0" // IR(16139, 10)
  "Union\0" // IR(16149, 6)
  "ImportedModule\0" // IR(16155, 15)
  "ImportedDeclaration\0" // IR(16170, 20)
  "None\0" // IR(16190, 5)
  "FlagIsProtected\0" // IR(16195, 16)
  "FlagIsPrivate\0" // IR(16211, 14)
  "FlagIsPublic\0" // IR(16225, 13)
  "FlagIsLocal\0" // IR(16238, 12)
  "FlagIsDefinition\0" // IR(16250, 17)
  "FlagFwdDecl\0" // IR(16267, 12)
  "FlagArtificial\0" // IR(16279, 15)
  "FlagExplicit\0" // IR(16294, 13)
  "FlagPrototyped\0" // IR(16307, 15)
  "FlagObjectPointer\0" // IR(16322, 18)
  "FlagStaticMember\0" // IR(16340, 17)
  "FlagIndirectVariable\0" // IR(16357, 21)
  "FlagLValueReference\0" // IR(16378, 20)
  "FlagRValueReference\0" // IR(16398, 20)
  "FlagIsOptimized\0" // IR(16418, 16)
  "FlagIsEnumClass\0" // IR(16434, 16)
  "FlagTypePassByValue\0" // IR(16450, 20)
  "FlagTypePassByReference\0" // IR(16470, 24)
  "Deref\0" // IR(16494, 6)
  "Plus\0" // IR(16500, 5)
  "Minus\0" // IR(16505, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(16511, 33)
  "PlusUconst\0" // IR(16544, 11)
  "BitPiece\0" // IR(16555, 9)
  "Swap\0" // IR(16564, 5)
  "Xderef\0" // IR(16569, 7)
  "StackValue\0" // IR(16576, 11)
  "Constu\0" // IR(16587, 7)
  "Fragment\0" // IR(16594, 9)
  "ConstType\0" // IR(16603, 10)
  "VolatileType\0" // IR(16613, 13)
  "RestrictType\0" // IR(16626, 13)
  "AtomicType\0" // IR(16639, 11)
  "Float16NV\0" // IR(16650, 10)
  "Float32NV\0" // IR(16660, 10)
  "Float64NV\0" // IR(16670, 10)
  "SignedInt8NV\0" // IR(16680, 13)
  "SignedInt16NV\0" // IR(16693, 14)
  "SignedInt32NV\0" // IR(16707, 14)
  "SignedInt64NV\0" // IR(16721, 14)
  "UnsignedInt8NV\0" // IR(16735, 15)
  "UnsignedInt16NV\0" // IR(16750, 16)
  "UnsignedInt32NV\0" // IR(16766, 16)
  "UnsignedInt64NV\0" // IR(16782, 16)
  "SignedInt8PackedNV\0" // IR(16798, 19)
  "UnsignedInt8PackedNV\0" // IR(16817, 21)
  "FloatE4M3NV\0" // IR(16838, 12)
  "FloatE5M2NV\0" // IR(16850, 12)
  "RowMajorKHR\0" // IR(16862, 12)
  "ColumnMajorKHR\0" // IR(16874, 15)
  "RowBlockedInterleavedARM\0" // IR(16889, 25)
  "ColumnBlockedInterleavedARM\0" // IR(16914, 28)
  "NoneKHR\0" // IR(16942, 8)
  "MatrixASignedComponentsKHR\0" // IR(16950, 27)
  "MatrixBSignedComponentsKHR\0" // IR(16977, 27)
  "MatrixCSignedComponentsKHR\0" // IR(17004, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(17031, 32)
  "SaturatingAccumulationKHR\0" // IR(17063, 26)
  "Row\0" // IR(17089, 4)
  "Column\0" // IR(17093, 7)
  "2x2\0" // IR(17100, 4)
  "MatrixAKHR\0" // IR(17104, 11)
  "MatrixBKHR\0" // IR(17115, 11)
  "MatrixAccumulatorKHR\0" // IR(17126, 21)
  "RowMajorNV\0" // IR(17147, 11)
  "ColumnMajorNV\0" // IR(17158, 14)
  "InferencingOptimalNV\0" // IR(17172, 21)
  "TrainingOptimalNV\0" // IR(17193, 18)
  "RelaxedPrecision\0" // IR(17211, 17)
  "SpecId\0" // IR(17228, 7)
  "Block\0" // IR(17235, 6)
  "BufferBlock\0" // IR(17241, 12)
  "RowMajor\0" // IR(17253, 9)
  "ColMajor\0" // IR(17262, 9)
  "ArrayStride\0" // IR(17271, 12)
  "MatrixStride\0" // IR(17283, 13)
  "GLSLShared\0" // IR(17296, 11)
  "GLSLPacked\0" // IR(17307, 11)
  "CPacked\0" // IR(17318, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(17326, 26)
  "BuiltIn\0" // IR(17352, 8)
  "NoPerspective\0" // IR(17360, 14)
  "Flat\0" // IR(17374, 5)
  "Patch\0" // IR(17379, 6)
  "Centroid\0" // IR(17385, 9)
  "Sample\0" // IR(17394, 7)
  "Invariant\0" // IR(17401, 10)
  "Restrict\0" // IR(17411, 9)
  "Aliased\0" // IR(17420, 8)
  "Volatile\0" // IR(17428, 9)
  "Constant\0" // IR(17437, 9)
  "Coherent\0" // IR(17446, 9)
  "NonWritable\0" // IR(17455, 12)
  "NonReadable\0" // IR(17467, 12)
  "Uniform\0" // IR(17479, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(17487, 26)
  "UniformId\0" // IR(17513, 10)
  "SaturatedConversion\0" // IR(17523, 20)
  "Stream\0" // IR(17543, 7)
  "Location\0" // IR(17550, 9)
  "Component\0" // IR(17559, 10)
  "Index\0" // IR(17569, 6)
  "Binding\0" // IR(17575, 8)
  "DescriptorSet\0" // IR(17583, 14)
  "Offset\0" // IR(17597, 7)
  "XfbBuffer\0" // IR(17604, 10)
  "XfbStride\0" // IR(17614, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(17624, 46)
  "FuncParamAttr\0" // IR(17670, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(17684, 34)
  "FPRoundingMode\0" // IR(17718, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(17733, 35)
  "FPFastMathMode\0" // IR(17768, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(17783, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(17815, 30)
  "LinkageAttributes\0" // IR(17845, 18)
  "NoContraction\0" // IR(17863, 14)
  "InputAttachmentIndex\0" // IR(17877, 21)
  "Alignment\0" // IR(17898, 10)
  "MaxByteOffset\0" // IR(17908, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(17922, 20)
  "AlignmentId\0" // IR(17942, 12)
  "MaxByteOffsetId\0" // IR(17954, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(17970, 44)
  "NoSignedWrap\0" // IR(18014, 13)
  "NoUnsignedWrap\0" // IR(18027, 15)
  "WeightTextureQCOM\0" // IR(18042, 18)
  "BlockMatchTextureQCOM\0" // IR(18060, 22)
  "BlockMatchSamplerQCOM\0" // IR(18082, 22)
  "ExplicitInterpAMD\0" // IR(18104, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(18122, 32)
  "NodeMaxPayloadsAMDX\0" // IR(18154, 20)
  "TrackFinishWritingAMDX\0" // IR(18174, 23)
  "PayloadNodeNameAMDX\0" // IR(18197, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(18217, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(18242, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(18269, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(18294, 28)
  "ArrayStrideIdEXT\0" // IR(18322, 17)
  "OffsetIdEXT\0" // IR(18339, 12)
  "UTFEncodedKHR\0" // IR(18351, 14)
  "OverrideCoverageNV\0" // IR(18365, 19)
  "PassthroughNV\0" // IR(18384, 14)
  "ViewportRelativeNV\0" // IR(18398, 19)
  "SecondaryViewportRelativeNV\0" // IR(18417, 28)
  "PerPrimitiveEXT\0" // IR(18445, 16)
  "PerPrimitiveNV\0" // IR(18461, 15)
  "PerViewNV\0" // IR(18476, 10)
  "PerTaskNV\0" // IR(18486, 10)
  "PerVertexKHR\0" // IR(18496, 13)
  "PerVertexNV\0" // IR(18509, 12)
  "NonUniform\0" // IR(18521, 11)
  "NonUniformEXT\0" // IR(18532, 14)
  "RestrictPointer\0" // IR(18546, 16)
  "RestrictPointerEXT\0" // IR(18562, 19)
  "AliasedPointer\0" // IR(18581, 15)
  "AliasedPointerEXT\0" // IR(18596, 18)
  "MemberOffsetNV\0" // IR(18614, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(18629, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(18659, 31)
  "BankNV\0" // IR(18690, 7)
  "BindlessSamplerNV\0" // IR(18697, 18)
  "BindlessImageNV\0" // IR(18715, 16)
  "BoundSamplerNV\0" // IR(18731, 15)
  "BoundImageNV\0" // IR(18746, 13)
  "SIMTCallINTEL\0" // IR(18759, 14)
  "ReferencedIndirectlyINTEL\0" // IR(18773, 26)
  "ClobberINTEL\0" // IR(18799, 13)
  "SideEffectsINTEL\0" // IR(18812, 17)
  "VectorComputeVariableINTEL\0" // IR(18829, 27)
  "FuncParamIOKindINTEL\0" // IR(18856, 21)
  "VectorComputeFunctionINTEL\0" // IR(18877, 27)
  "StackCallINTEL\0" // IR(18904, 15)
  "GlobalVariableOffsetINTEL\0" // IR(18919, 26)
  "CounterBuffer\0" // IR(18945, 14)
  "HlslCounterBufferGOOGLE\0" // IR(18959, 24)
  "UserSemantic\0" // IR(18983, 13)
  "HlslSemanticGOOGLE\0" // IR(18996, 19)
  "UserTypeGOOGLE\0" // IR(19015, 15)
  "FunctionRoundingModeINTEL\0" // IR(19030, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(19056, 31)
  "FunctionDenormModeINTEL\0" // IR(19087, 24)
  "RegisterALTERA\0" // IR(19111, 15)
  "RegisterINTEL\0" // IR(19126, 14)
  "MemoryALTERA\0" // IR(19140, 13)
  "MemoryINTEL\0" // IR(19153, 12)
  "NumbanksALTERA\0" // IR(19165, 15)
  "NumbanksINTEL\0" // IR(19180, 14)
  "BankwidthALTERA\0" // IR(19194, 16)
  "BankwidthINTEL\0" // IR(19210, 15)
  "MaxPrivateCopiesALTERA\0" // IR(19225, 23)
  "MaxPrivateCopiesINTEL\0" // IR(19248, 22)
  "SinglepumpALTERA\0" // IR(19270, 17)
  "SinglepumpINTEL\0" // IR(19287, 16)
  "DoublepumpALTERA\0" // IR(19303, 17)
  "DoublepumpINTEL\0" // IR(19320, 16)
  "MaxReplicatesALTERA\0" // IR(19336, 20)
  "MaxReplicatesINTEL\0" // IR(19356, 19)
  "SimpleDualPortALTERA\0" // IR(19375, 21)
  "SimpleDualPortINTEL\0" // IR(19396, 20)
  "MergeALTERA\0" // IR(19416, 12)
  "MergeINTEL\0" // IR(19428, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(19439, 42)
  "BankBitsALTERA\0" // IR(19481, 15)
  "BankBitsINTEL\0" // IR(19496, 14)
  "ForcePow2DepthALTERA\0" // IR(19510, 21)
  "ForcePow2DepthINTEL\0" // IR(19531, 20)
  "StridesizeALTERA\0" // IR(19551, 17)
  "StridesizeINTEL\0" // IR(19568, 16)
  "WordsizeALTERA\0" // IR(19584, 15)
  "WordsizeINTEL\0" // IR(19599, 14)
  "TrueDualPortALTERA\0" // IR(19613, 19)
  "TrueDualPortINTEL\0" // IR(19632, 18)
  "BurstCoalesceALTERA\0" // IR(19650, 20)
  "BurstCoalesceINTEL\0" // IR(19670, 19)
  "CacheSizeALTERA\0" // IR(19689, 16)
  "CacheSizeINTEL\0" // IR(19705, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(19720, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(19749, 28)
  "PrefetchALTERA\0" // IR(19777, 15)
  "PrefetchINTEL\0" // IR(19792, 14)
  "StallEnableALTERA\0" // IR(19806, 18)
  "StallEnableINTEL\0" // IR(19824, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(19841, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(19867, 25)
  "MathOpDSPModeALTERA\0" // IR(19892, 20)
  "MathOpDSPModeINTEL\0" // IR(19912, 19)
  "AliasScopeINTEL\0" // IR(19931, 16)
  "NoAliasINTEL\0" // IR(19947, 13)
  "InitiationIntervalALTERA\0" // IR(19960, 25)
  "InitiationIntervalINTEL\0" // IR(19985, 24)
  "MaxConcurrencyALTERA\0" // IR(20009, 21)
  "MaxConcurrencyINTEL\0" // IR(20030, 20)
  "PipelineEnableALTERA\0" // IR(20050, 21)
  "PipelineEnableINTEL\0" // IR(20071, 20)
  "BufferLocationALTERA\0" // IR(20091, 21)
  "BufferLocationINTEL\0" // IR(20112, 20)
  "IOPipeStorageALTERA\0" // IR(20132, 20)
  "IOPipeStorageINTEL\0" // IR(20152, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(20171, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(20205, 31)
  "SingleElementVectorINTEL\0" // IR(20236, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(20261, 35)
  "MediaBlockIOINTEL\0" // IR(20296, 18)
  "StallFreeALTERA\0" // IR(20314, 16)
  "StallFreeINTEL\0" // IR(20330, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(20345, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(20376, 26)
  "LatencyControlLabelALTERA\0" // IR(20402, 26)
  "LatencyControlLabelINTEL\0" // IR(20428, 25)
  "LatencyControlConstraintALTERA\0" // IR(20453, 31)
  "LatencyControlConstraintINTEL\0" // IR(20484, 30)
  "ConduitKernelArgumentALTERA\0" // IR(20514, 28)
  "ConduitKernelArgumentINTEL\0" // IR(20542, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(20569, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(20601, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(20632, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(20666, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(20699, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(20730, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(20760, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(20789, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(20817, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(20851, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(20886, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(20920, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(20950, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(20979, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(21012, 32)
  "StableKernelArgumentALTERA\0" // IR(21044, 27)
  "StableKernelArgumentINTEL\0" // IR(21071, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(21097, 39)
  "HostAccessINTEL\0" // IR(21136, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(21152, 47)
  "InitModeALTERA\0" // IR(21199, 15)
  "InitModeINTEL\0" // IR(21214, 14)
  "ImplementInRegisterMapALTERA\0" // IR(21228, 29)
  "ImplementInRegisterMapINTEL\0" // IR(21257, 28)
  "ConditionalINTEL\0" // IR(21285, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(21302, 36)
  "CacheControlLoadINTEL\0" // IR(21338, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(21360, 37)
  "CacheControlStoreINTEL\0" // IR(21397, 23)
  "1D\0" // IR(21420, 3)
  "2D\0" // IR(21423, 3)
  "3D\0" // IR(21426, 3)
  "Cube\0" // IR(21429, 5)
  "Rect\0" // IR(21434, 5)
  "Buffer\0" // IR(21439, 7)
  "SubpassData\0" // IR(21446, 12)
  "TileImageDataEXT\0" // IR(21458, 17)
  "Invocations\0" // IR(21475, 12)
  "SpacingEqual\0" // IR(21487, 13)
  "SpacingFractionalEven\0" // IR(21500, 22)
  "SpacingFractionalOdd\0" // IR(21522, 21)
  "VertexOrderCw\0" // IR(21543, 14)
  "VertexOrderCcw\0" // IR(21557, 15)
  "PixelCenterInteger\0" // IR(21572, 19)
  "OriginUpperLeft\0" // IR(21591, 16)
  "OriginLowerLeft\0" // IR(21607, 16)
  "EarlyFragmentTests\0" // IR(21623, 19)
  "PointMode\0" // IR(21642, 10)
  "Xfb\0" // IR(21652, 4)
  "DepthReplacing\0" // IR(21656, 15)
  "DepthGreater\0" // IR(21671, 13)
  "DepthLess\0" // IR(21684, 10)
  "DepthUnchanged\0" // IR(21694, 15)
  "LocalSize\0" // IR(21709, 10)
  "LocalSizeHint\0" // IR(21719, 14)
  "InputPoints\0" // IR(21733, 12)
  "InputLines\0" // IR(21745, 11)
  "InputLinesAdjacency\0" // IR(21756, 20)
  "Triangles\0" // IR(21776, 10)
  "InputTrianglesAdjacency\0" // IR(21786, 24)
  "Quads\0" // IR(21810, 6)
  "Isolines\0" // IR(21816, 9)
  "OutputVertices\0" // IR(21825, 15)
  "OutputPoints\0" // IR(21840, 13)
  "OutputLineStrip\0" // IR(21853, 16)
  "OutputTriangleStrip\0" // IR(21869, 20)
  "VecTypeHint\0" // IR(21889, 12)
  "ContractionOff\0" // IR(21901, 15)
  "Initializer\0" // IR(21916, 12)
  "Finalizer\0" // IR(21928, 10)
  "SubgroupsPerWorkgroup\0" // IR(21938, 22)
  "SubgroupsPerWorkgroupId\0" // IR(21960, 24)
  "LocalSizeId\0" // IR(21984, 12)
  "LocalSizeHintId\0" // IR(21996, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(22012, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(22046, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(22080, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(22116, 30)
  "PostDepthCoverage\0" // IR(22146, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(22164, 34)
  "TileShadingRateQCOM\0" // IR(22198, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(22218, 29)
  "StencilRefReplacingEXT\0" // IR(22247, 23)
  "CoalescingAMDX\0" // IR(22270, 15)
  "IsApiEntryAMDX\0" // IR(22285, 15)
  "MaxNodeRecursionAMDX\0" // IR(22300, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(22321, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(22345, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(22366, 28)
  "StencilRefGreaterFrontAMD\0" // IR(22394, 26)
  "StencilRefLessFrontAMD\0" // IR(22420, 23)
  "StencilRefUnchangedBackAMD\0" // IR(22443, 27)
  "StencilRefGreaterBackAMD\0" // IR(22470, 25)
  "StencilRefLessBackAMD\0" // IR(22495, 22)
  "QuadDerivativesKHR\0" // IR(22517, 19)
  "RequireFullQuadsKHR\0" // IR(22536, 20)
  "SharesInputWithAMDX\0" // IR(22556, 20)
  "ArithmeticPoisonKHR\0" // IR(22576, 20)
  "OutputLinesEXT\0" // IR(22596, 15)
  "OutputLinesNV\0" // IR(22611, 14)
  "OutputPrimitivesEXT\0" // IR(22625, 20)
  "OutputPrimitivesNV\0" // IR(22645, 19)
  "DerivativeGroupQuadsKHR\0" // IR(22664, 24)
  "DerivativeGroupQuadsNV\0" // IR(22688, 23)
  "DerivativeGroupLinearKHR\0" // IR(22711, 25)
  "DerivativeGroupLinearNV\0" // IR(22736, 24)
  "OutputTrianglesEXT\0" // IR(22760, 19)
  "OutputTrianglesNV\0" // IR(22779, 18)
  "PixelInterlockOrderedEXT\0" // IR(22797, 25)
  "PixelInterlockUnorderedEXT\0" // IR(22822, 27)
  "SampleInterlockOrderedEXT\0" // IR(22849, 26)
  "SampleInterlockUnorderedEXT\0" // IR(22875, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(22903, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(22934, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(22967, 27)
  "RoundingModeRTPINTEL\0" // IR(22994, 21)
  "RoundingModeRTNINTEL\0" // IR(23015, 21)
  "FloatingPointModeALTINTEL\0" // IR(23036, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(23062, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(23089, 22)
  "MaxWorkDimINTEL\0" // IR(23111, 16)
  "NoGlobalOffsetINTEL\0" // IR(23127, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(23147, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(23169, 28)
  "MaximallyReconvergesKHR\0" // IR(23197, 24)
  "FPFastMathDefault\0" // IR(23221, 18)
  "StreamingInterfaceINTEL\0" // IR(23239, 24)
  "RegisterMapInterfaceINTEL\0" // IR(23263, 26)
  "NamedBarrierCountINTEL\0" // IR(23289, 23)
  "MaximumRegistersINTEL\0" // IR(23312, 22)
  "MaximumRegistersIdINTEL\0" // IR(23334, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(23358, 51)
  "NamedMaximumRegistersINTEL\0" // IR(23409, 27)
  "Vertex\0" // IR(23436, 7)
  "TessellationControl\0" // IR(23443, 20)
  "TessellationEvaluation\0" // IR(23463, 23)
  "GLCompute\0" // IR(23486, 10)
  "TaskNV\0" // IR(23496, 7)
  "MeshNV\0" // IR(23503, 7)
  "RayGenerationKHR\0" // IR(23510, 17)
  "RayGenerationNV\0" // IR(23527, 16)
  "IntersectionKHR\0" // IR(23543, 16)
  "IntersectionNV\0" // IR(23559, 15)
  "AnyHitKHR\0" // IR(23574, 10)
  "AnyHitNV\0" // IR(23584, 9)
  "ClosestHitKHR\0" // IR(23593, 14)
  "ClosestHitNV\0" // IR(23607, 13)
  "MissKHR\0" // IR(23620, 8)
  "MissNV\0" // IR(23628, 7)
  "CallableKHR\0" // IR(23635, 12)
  "CallableNV\0" // IR(23647, 11)
  "TaskEXT\0" // IR(23658, 8)
  "MeshEXT\0" // IR(23666, 8)
  "Preserve\0" // IR(23674, 9)
  "FlushToZero\0" // IR(23683, 12)
  "BFloat16KHR\0" // IR(23695, 12)
  "Float8E4M3EXT\0" // IR(23707, 14)
  "Float8E5M2EXT\0" // IR(23721, 14)
  "IEEE\0" // IR(23735, 5)
  "ALT\0" // IR(23740, 4)
  "NotNaN\0" // IR(23744, 7)
  "NotInf\0" // IR(23751, 7)
  "NSZ\0" // IR(23758, 4)
  "AllowRecip\0" // IR(23762, 11)
  "Fast\0" // IR(23773, 5)
  "AllowContract\0" // IR(23778, 14)
  "AllowContractFastINTEL\0" // IR(23792, 23)
  "AllowReassoc\0" // IR(23815, 13)
  "AllowReassocINTEL\0" // IR(23828, 18)
  "AllowTransform\0" // IR(23846, 15)
  "RTE\0" // IR(23861, 4)
  "RTZ\0" // IR(23865, 4)
  "RTP\0" // IR(23869, 4)
  "RTN\0" // IR(23873, 4)
  "Vertical2Pixels\0" // IR(23877, 16)
  "Vertical4Pixels\0" // IR(23893, 16)
  "Horizontal2Pixels\0" // IR(23909, 18)
  "Horizontal4Pixels\0" // IR(23927, 18)
  "Inline\0" // IR(23945, 7)
  "DontInline\0" // IR(23952, 11)
  "Pure\0" // IR(23963, 5)
  "Const\0" // IR(23968, 6)
  "Zext\0" // IR(23974, 5)
  "Sext\0" // IR(23979, 5)
  "ByVal\0" // IR(23984, 6)
  "Sret\0" // IR(23990, 5)
  "NoAlias\0" // IR(23995, 8)
  "NoCapture\0" // IR(24003, 10)
  "NoWrite\0" // IR(24013, 8)
  "NoReadWrite\0" // IR(24021, 12)
  "RuntimeAlignedALTERA\0" // IR(24033, 21)
  "RuntimeAlignedINTEL\0" // IR(24054, 20)
  "Reduce\0" // IR(24074, 7)
  "InclusiveScan\0" // IR(24081, 14)
  "ExclusiveScan\0" // IR(24095, 14)
  "ClusteredReduce\0" // IR(24109, 16)
  "PartitionedReduceEXT\0" // IR(24125, 21)
  "PartitionedReduceNV\0" // IR(24146, 20)
  "PartitionedInclusiveScanEXT\0" // IR(24166, 28)
  "PartitionedInclusiveScanNV\0" // IR(24194, 27)
  "PartitionedExclusiveScanEXT\0" // IR(24221, 28)
  "PartitionedExclusiveScanNV\0" // IR(24249, 27)
  "NoneINTEL\0" // IR(24276, 10)
  "ReadINTEL\0" // IR(24286, 10)
  "WriteINTEL\0" // IR(24296, 11)
  "ReadWriteINTEL\0" // IR(24307, 15)
  "Bias\0" // IR(24322, 5)
  "Lod\0" // IR(24327, 4)
  "Grad\0" // IR(24331, 5)
  "ConstOffset\0" // IR(24336, 12)
  "ConstOffsets\0" // IR(24348, 13)
  "MakeTexelAvailable\0" // IR(24361, 19)
  "MakeTexelAvailableKHR\0" // IR(24380, 22)
  "MakeTexelVisible\0" // IR(24402, 17)
  "MakeTexelVisibleKHR\0" // IR(24419, 20)
  "NonPrivateTexel\0" // IR(24439, 16)
  "NonPrivateTexelKHR\0" // IR(24455, 19)
  "VolatileTexel\0" // IR(24474, 14)
  "VolatileTexelKHR\0" // IR(24488, 17)
  "SignExtend\0" // IR(24505, 11)
  "ZeroExtend\0" // IR(24516, 11)
  "Nontemporal\0" // IR(24527, 12)
  "Offsets\0" // IR(24539, 8)
  "SnormInt8\0" // IR(24547, 10)
  "SnormInt16\0" // IR(24557, 11)
  "UnormInt8\0" // IR(24568, 10)
  "UnormInt16\0" // IR(24578, 11)
  "UnormShort565\0" // IR(24589, 14)
  "UnormShort555\0" // IR(24603, 14)
  "UnormInt101010\0" // IR(24617, 15)
  "SignedInt8\0" // IR(24632, 11)
  "SignedInt16\0" // IR(24643, 12)
  "SignedInt32\0" // IR(24655, 12)
  "UnsignedInt8\0" // IR(24667, 13)
  "UnsignedInt16\0" // IR(24680, 14)
  "UnsignedInt32\0" // IR(24694, 14)
  "HalfFloat\0" // IR(24708, 10)
  "UnormInt24\0" // IR(24718, 11)
  "UnormInt101010_2\0" // IR(24729, 17)
  "UnormInt10X6EXT\0" // IR(24746, 16)
  "UnsignedIntRaw10EXT\0" // IR(24762, 20)
  "UnsignedIntRaw12EXT\0" // IR(24782, 20)
  "UnormInt2_101010EXT\0" // IR(24802, 20)
  "UnsignedInt10X6EXT\0" // IR(24822, 19)
  "UnsignedInt12X4EXT\0" // IR(24841, 19)
  "UnsignedInt14X2EXT\0" // IR(24860, 19)
  "UnormInt12X4EXT\0" // IR(24879, 16)
  "UnormInt14X2EXT\0" // IR(24895, 16)
  "R\0" // IR(24911, 2)
  "A\0" // IR(24913, 2)
  "RG\0" // IR(24915, 3)
  "RA\0" // IR(24918, 3)
  "RGB\0" // IR(24921, 4)
  "RGBA\0" // IR(24925, 5)
  "BGRA\0" // IR(24930, 5)
  "ARGB\0" // IR(24935, 5)
  "Intensity\0" // IR(24940, 10)
  "Luminance\0" // IR(24950, 10)
  "Rx\0" // IR(24960, 3)
  "RGx\0" // IR(24963, 4)
  "RGBx\0" // IR(24967, 5)
  "Depth\0" // IR(24972, 6)
  "DepthStencil\0" // IR(24978, 13)
  "sRGB\0" // IR(24991, 5)
  "sRGBx\0" // IR(24996, 6)
  "sRGBA\0" // IR(25002, 6)
  "sBGRA\0" // IR(25008, 6)
  "ABGR\0" // IR(25014, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(25019, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(25047, 27)
  "InitOnDeviceResetALTERA\0" // IR(25074, 24)
  "InitOnDeviceResetINTEL\0" // IR(25098, 23)
  "NoWait\0" // IR(25121, 7)
  "WaitKernel\0" // IR(25128, 11)
  "WaitWorkGroup\0" // IR(25139, 14)
  "CmdExecTime\0" // IR(25153, 12)
  "MayUsePrintf\0" // IR(25165, 13)
  "Export\0" // IR(25178, 7)
  "Import\0" // IR(25185, 7)
  "LinkOnceODR\0" // IR(25192, 12)
  "UncachedINTEL\0" // IR(25204, 14)
  "CachedINTEL\0" // IR(25218, 12)
  "StreamingINTEL\0" // IR(25230, 15)
  "InvalidateAfterReadINTEL\0" // IR(25245, 25)
  "ConstCachedINTEL\0" // IR(25270, 17)
  "Unroll\0" // IR(25287, 7)
  "DontUnroll\0" // IR(25294, 11)
  "DependencyInfinite\0" // IR(25305, 19)
  "DependencyLength\0" // IR(25324, 17)
  "MinIterations\0" // IR(25341, 14)
  "MaxIterations\0" // IR(25355, 14)
  "IterationMultiple\0" // IR(25369, 18)
  "PeelCount\0" // IR(25387, 10)
  "PartialCount\0" // IR(25397, 13)
  "DependencyArrayALTERA\0" // IR(25410, 22)
  "DependencyArrayINTEL\0" // IR(25432, 21)
  "LoopCoalesceALTERA\0" // IR(25453, 19)
  "LoopCoalesceINTEL\0" // IR(25472, 18)
  "MaxInterleavingALTERA\0" // IR(25490, 22)
  "MaxInterleavingINTEL\0" // IR(25512, 21)
  "SpeculatedIterationsALTERA\0" // IR(25533, 27)
  "SpeculatedIterationsINTEL\0" // IR(25560, 26)
  "NoFusionALTERA\0" // IR(25586, 15)
  "NoFusionINTEL\0" // IR(25601, 14)
  "LoopCountALTERA\0" // IR(25615, 16)
  "LoopCountINTEL\0" // IR(25631, 15)
  "MaxReinvocationDelayALTERA\0" // IR(25646, 27)
  "MaxReinvocationDelayINTEL\0" // IR(25673, 26)
  "MatrixASignedComponentsINTEL\0" // IR(25699, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(25728, 29)
  "MatrixCBFloat16INTEL\0" // IR(25757, 21)
  "MatrixResultBFloat16INTEL\0" // IR(25778, 26)
  "MatrixAPackedInt8INTEL\0" // IR(25804, 23)
  "MatrixBPackedInt8INTEL\0" // IR(25827, 23)
  "MatrixAPackedInt4INTEL\0" // IR(25850, 23)
  "MatrixBPackedInt4INTEL\0" // IR(25873, 23)
  "MatrixATF32INTEL\0" // IR(25896, 17)
  "MatrixBTF32INTEL\0" // IR(25913, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(25930, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(25956, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(25982, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(26009, 27)
  "Aligned\0" // IR(26036, 8)
  "MakePointerAvailable\0" // IR(26044, 21)
  "MakePointerAvailableKHR\0" // IR(26065, 24)
  "MakePointerVisible\0" // IR(26089, 19)
  "MakePointerVisibleKHR\0" // IR(26108, 22)
  "NonPrivatePointer\0" // IR(26130, 18)
  "NonPrivatePointerKHR\0" // IR(26148, 21)
  "AliasScopeINTELMask\0" // IR(26169, 20)
  "NoAliasINTELMask\0" // IR(26189, 17)
  "Simple\0" // IR(26206, 7)
  "GLSL450\0" // IR(26213, 8)
  "OpenCL\0" // IR(26221, 7)
  "Vulkan\0" // IR(26228, 7)
  "VulkanKHR\0" // IR(26235, 10)
  "Relaxed\0" // IR(26245, 8)
  "Acquire\0" // IR(26253, 8)
  "Release\0" // IR(26261, 8)
  "AcquireRelease\0" // IR(26269, 15)
  "SequentiallyConsistent\0" // IR(26284, 23)
  "UniformMemory\0" // IR(26307, 14)
  "SubgroupMemory\0" // IR(26321, 15)
  "WorkgroupMemory\0" // IR(26336, 16)
  "CrossWorkgroupMemory\0" // IR(26352, 21)
  "AtomicCounterMemory\0" // IR(26373, 20)
  "ImageMemory\0" // IR(26393, 12)
  "OutputMemory\0" // IR(26405, 13)
  "OutputMemoryKHR\0" // IR(26418, 16)
  "MakeAvailable\0" // IR(26434, 14)
  "MakeAvailableKHR\0" // IR(26448, 17)
  "MakeVisible\0" // IR(26465, 12)
  "MakeVisibleKHR\0" // IR(26477, 15)
  "AutoINTEL\0" // IR(26492, 10)
  "WRAP\0" // IR(26502, 5)
  "SAT\0" // IR(26507, 4)
  "SAT_ZERO\0" // IR(26511, 9)
  "SAT_SYM\0" // IR(26520, 8)
  "PackedVectorFormat4x8Bit\0" // IR(26528, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(26553, 28)
  "TRN\0" // IR(26581, 4)
  "TRN_ZERO\0" // IR(26585, 9)
  "RND\0" // IR(26594, 4)
  "RND_ZERO\0" // IR(26598, 9)
  "RND_INF\0" // IR(26607, 8)
  "RND_MIN_INF\0" // IR(26615, 12)
  "RND_CONV\0" // IR(26627, 9)
  "RND_CONV_ODD\0" // IR(26636, 13)
  "RobustnessPerComponentNV\0" // IR(26649, 25)
  "RobustnessPerElementNV\0" // IR(26674, 23)
  "OpaqueKHR\0" // IR(26697, 10)
  "NoOpaqueKHR\0" // IR(26707, 12)
  "TerminateOnFirstHitKHR\0" // IR(26719, 23)
  "SkipClosestHitShaderKHR\0" // IR(26742, 24)
  "CullBackFacingTrianglesKHR\0" // IR(26766, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(26793, 28)
  "CullOpaqueKHR\0" // IR(26821, 14)
  "CullNoOpaqueKHR\0" // IR(26835, 16)
  "SkipTrianglesKHR\0" // IR(26851, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(26868, 24)
  "SkipAABBsKHR\0" // IR(26892, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(26905, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(26935, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(26976, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(27013, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(27050, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(27091, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(27133, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(27166, 33)
  "ClampToEdge\0" // IR(27199, 12)
  "Clamp\0" // IR(27211, 6)
  "Repeat\0" // IR(27217, 7)
  "RepeatMirrored\0" // IR(27224, 15)
  "Nearest\0" // IR(27239, 8)
  "Linear\0" // IR(27247, 7)
  "Unknown\0" // IR(27254, 8)
  "Rgba32f\0" // IR(27262, 8)
  "Rgba16f\0" // IR(27270, 8)
  "R32f\0" // IR(27278, 5)
  "Rgba8\0" // IR(27283, 6)
  "Rgba8Snorm\0" // IR(27289, 11)
  "Rg32f\0" // IR(27300, 6)
  "Rg16f\0" // IR(27306, 6)
  "R11fG11fB10f\0" // IR(27312, 13)
  "R16f\0" // IR(27325, 5)
  "Rgba16\0" // IR(27330, 7)
  "Rgb10A2\0" // IR(27337, 8)
  "Rg16\0" // IR(27345, 5)
  "Rg8\0" // IR(27350, 4)
  "R16\0" // IR(27354, 4)
  "R8\0" // IR(27358, 3)
  "Rgba16Snorm\0" // IR(27361, 12)
  "Rg16Snorm\0" // IR(27373, 10)
  "Rg8Snorm\0" // IR(27383, 9)
  "R16Snorm\0" // IR(27392, 9)
  "R8Snorm\0" // IR(27401, 8)
  "Rgba32i\0" // IR(27409, 8)
  "Rgba16i\0" // IR(27417, 8)
  "Rgba8i\0" // IR(27425, 7)
  "R32i\0" // IR(27432, 5)
  "Rg32i\0" // IR(27437, 6)
  "Rg16i\0" // IR(27443, 6)
  "Rg8i\0" // IR(27449, 5)
  "R16i\0" // IR(27454, 5)
  "R8i\0" // IR(27459, 4)
  "Rgba32ui\0" // IR(27463, 9)
  "Rgba16ui\0" // IR(27472, 9)
  "Rgba8ui\0" // IR(27481, 8)
  "R32ui\0" // IR(27489, 6)
  "Rgb10a2ui\0" // IR(27495, 10)
  "Rg32ui\0" // IR(27505, 7)
  "Rg16ui\0" // IR(27512, 7)
  "Rg8ui\0" // IR(27519, 6)
  "R16ui\0" // IR(27525, 6)
  "R8ui\0" // IR(27531, 5)
  "R64ui\0" // IR(27536, 6)
  "R64i\0" // IR(27542, 5)
  "CrossDevice\0" // IR(27547, 12)
  "Device\0" // IR(27559, 7)
  "Workgroup\0" // IR(27566, 10)
  "Subgroup\0" // IR(27576, 9)
  "Invocation\0" // IR(27585, 11)
  "QueueFamily\0" // IR(27596, 12)
  "QueueFamilyKHR\0" // IR(27608, 15)
  "ShaderCallKHR\0" // IR(27623, 14)
  "Flatten\0" // IR(27637, 8)
  "DontFlatten\0" // IR(27645, 12)
  "IdentifierPossibleDuplicates\0" // IR(27657, 29)
  "FlagUnknownPhysicalLayout\0" // IR(27686, 26)
  "ESSL\0" // IR(27712, 5)
  "GLSL\0" // IR(27717, 5)
  "OpenCL_C\0" // IR(27722, 9)
  "OpenCL_CPP\0" // IR(27731, 11)
  "HLSL\0" // IR(27742, 5)
  "CPP_for_OpenCL\0" // IR(27747, 15)
  "SYCL\0" // IR(27762, 5)
  "HERO_C\0" // IR(27767, 7)
  "NZSL\0" // IR(27774, 5)
  "WGSL\0" // IR(27779, 5)
  "Slang\0" // IR(27784, 6)
  "Zig\0" // IR(27790, 4)
  "Rust\0" // IR(27794, 5)
  "UniformConstant\0" // IR(27799, 16)
  "Input\0" // IR(27815, 6)
  "Output\0" // IR(27821, 7)
  "CrossWorkgroup\0" // IR(27828, 15)
  "Private\0" // IR(27843, 8)
  "Function\0" // IR(27851, 9)
  "Generic\0" // IR(27860, 8)
  "PushConstant\0" // IR(27868, 13)
  "AtomicCounter\0" // IR(27881, 14)
  "Image\0" // IR(27895, 6)
  "StorageBuffer\0" // IR(27901, 14)
  "TileImageEXT\0" // IR(27915, 13)
  "TileAttachmentQCOM\0" // IR(27928, 19)
  "NodePayloadAMDX\0" // IR(27947, 16)
  "CallableDataKHR\0" // IR(27963, 16)
  "CallableDataNV\0" // IR(27979, 15)
  "IncomingCallableDataKHR\0" // IR(27994, 24)
  "IncomingCallableDataNV\0" // IR(28018, 23)
  "RayPayloadKHR\0" // IR(28041, 14)
  "RayPayloadNV\0" // IR(28055, 13)
  "HitAttributeKHR\0" // IR(28068, 16)
  "HitAttributeNV\0" // IR(28084, 15)
  "IncomingRayPayloadKHR\0" // IR(28099, 22)
  "IncomingRayPayloadNV\0" // IR(28121, 21)
  "ShaderRecordBufferKHR\0" // IR(28142, 22)
  "ShaderRecordBufferNV\0" // IR(28164, 21)
  "PhysicalStorageBuffer\0" // IR(28185, 22)
  "PhysicalStorageBufferEXT\0" // IR(28207, 25)
  "HitObjectAttributeNV\0" // IR(28232, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(28253, 24)
  "HitObjectAttributeEXT\0" // IR(28277, 22)
  "CodeSectionINTEL\0" // IR(28299, 17)
  "DeviceOnlyALTERA\0" // IR(28316, 17)
  "DeviceOnlyINTEL\0" // IR(28333, 16)
  "HostOnlyALTERA\0" // IR(28349, 15)
  "HostOnlyINTEL\0" // IR(28364, 14)
  "WriteThroughINTEL\0" // IR(28378, 18)
  "WriteBackINTEL\0" // IR(28396, 15)
  "TensorView\0" // IR(28411, 11)
  "DecodeFunc\0" // IR(28422, 11)
  "Undefined\0" // IR(28433, 10)
  "NoneARM\0" // IR(28443, 8)
  "NontemporalARM\0" // IR(28451, 15)
  "OutOfBoundsValueARM\0" // IR(28466, 20)
  "MakeElementAvailableARM\0" // IR(28486, 24)
  "MakeElementVisibleARM\0" // IR(28510, 22)
  "NonPrivateElementARM\0" // IR(28532, 21)
  "Nop\0" // IR(28553, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(28557, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(28582, 27)
  "Undef\0" // IR(28609, 6)
  "SourceContinued\0" // IR(28615, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(28631, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(28664, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(28693, 41)
  "Source\0" // IR(28734, 7)
  "SourceExtension\0" // IR(28741, 16)
  "Name\0" // IR(28757, 5)
  "MemberName\0" // IR(28762, 11)
  "String\0" // IR(28773, 7)
  "Line\0" // IR(28780, 5)
  "Extension\0" // IR(28785, 10)
  "ExtInstImport\0" // IR(28795, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(28809, 46)
  "ExtInst\0" // IR(28855, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(28863, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(28897, 30)
  "MemoryModel\0" // IR(28927, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(28939, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(28972, 29)
  "EntryPoint\0" // IR(29001, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(29012, 32)
  "ExecutionMode\0" // IR(29044, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(29058, 28)
  "Capability\0" // IR(29086, 11)
  "TypeVoid\0" // IR(29097, 9)
  "TypeBool\0" // IR(29106, 9)
  "TypeInt\0" // IR(29115, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(29123, 37)
  "TypeFloat\0" // IR(29160, 10)
  "TypeVector\0" // IR(29170, 11)
  "TypeMatrix\0" // IR(29181, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(29192, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(29224, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(29262, 43)
  "TypeImage\0" // IR(29305, 10)
  "TypeSampler\0" // IR(29315, 12)
  "TypeSampledImage\0" // IR(29327, 17)
  "TypeArray\0" // IR(29344, 10)
  "TypeRuntimeArray\0" // IR(29354, 17)
  "TypeStruct\0" // IR(29371, 11)
  "TypeOpaque\0" // IR(29382, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(29393, 31)
  "TypePointer\0" // IR(29424, 12)
  "TypeFunction\0" // IR(29436, 13)
  "TypeEvent\0" // IR(29449, 10)
  "TypeDeviceEvent\0" // IR(29459, 16)
  "TypeReserveId\0" // IR(29475, 14)
  "TypeQueue\0" // IR(29489, 10)
  "TypePipe\0" // IR(29499, 9)
  "TypeForwardPointer\0" // IR(29508, 19)
  "ConstantTrue\0" // IR(29527, 13)
  "ConstantFalse\0" // IR(29540, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(29554, 38)
  "ConstantComposite\0" // IR(29592, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(29610, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(29651, 37)
  "ConstantSampler\0" // IR(29688, 16)
  "ConstantNull\0" // IR(29704, 13)
  "SpecConstantTrue\0" // IR(29717, 17)
  "SpecConstantFalse\0" // IR(29734, 18)
  "SpecConstant\0" // IR(29752, 13)
  "SpecConstantComposite\0" // IR(29765, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(29787, 41)
  "SpecConstantOp\0" // IR(29828, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(29843, 34)
  "FunctionParameter\0" // IR(29877, 18)
  "FunctionEnd\0" // IR(29895, 12)
  "FunctionCall\0" // IR(29907, 13)
  "Variable\0" // IR(29920, 9)
  "ImageTexelPointer\0" // IR(29929, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(29947, 40)
  "Load\0" // IR(29987, 5)
  "Store\0" // IR(29992, 6)
  "CopyMemory\0" // IR(29998, 11)
  "CopyMemorySized\0" // IR(30009, 16)
  "AccessChain\0" // IR(30025, 12)
  "InBoundsAccessChain\0" // IR(30037, 20)
  "PtrAccessChain\0" // IR(30057, 15)
  "ArrayLength\0" // IR(30072, 12)
  "GenericPtrMemSemantics\0" // IR(30084, 23)
  "InBoundsPtrAccessChain\0" // IR(30107, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(30130, 28)
  "Decorate\0" // IR(30158, 9)
  "MemberDecorate\0" // IR(30167, 15)
  "DecorationGroup\0" // IR(30182, 16)
  "GroupDecorate\0" // IR(30198, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(30212, 45)
  "GroupMemberDecorate\0" // IR(30257, 20)
  "VectorExtractDynamic\0" // IR(30277, 21)
  "VectorInsertDynamic\0" // IR(30298, 20)
  "VectorShuffle\0" // IR(30318, 14)
  "CompositeConstruct\0" // IR(30332, 19)
  "CompositeExtract\0" // IR(30351, 17)
  "CompositeInsert\0" // IR(30368, 16)
  "CopyObject\0" // IR(30384, 11)
  "Transpose\0" // IR(30395, 10)
  "SampledImage\0" // IR(30405, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(30418, 32)
  "ImageSampleImplicitLod\0" // IR(30450, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(30473, 23)
  "ImageSampleExplicitLod\0" // IR(30496, 23)
  "ImageSampleDrefImplicitLod\0" // IR(30519, 27)
  "ImageSampleDrefExplicitLod\0" // IR(30546, 27)
  "ImageSampleProjImplicitLod\0" // IR(30573, 27)
  "ImageSampleProjExplicitLod\0" // IR(30600, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(30627, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(30658, 31)
  "ImageFetch\0" // IR(30689, 11)
  "ImageGather\0" // IR(30700, 12)
  "ImageDrefGather\0" // IR(30712, 16)
  "ImageRead\0" // IR(30728, 10)
  "ImageWrite\0" // IR(30738, 11)
  "ImageQueryFormat\0" // IR(30749, 17)
  "ImageQueryOrder\0" // IR(30766, 16)
  "ImageQuerySizeLod\0" // IR(30782, 18)
  "ImageQuerySize\0" // IR(30800, 15)
  "ImageQueryLod\0" // IR(30815, 14)
  "ImageQueryLevels\0" // IR(30829, 17)
  "ImageQuerySamples\0" // IR(30846, 18)
  "ConvertFToU\0" // IR(30864, 12)
  "ConvertFToS\0" // IR(30876, 12)
  "ConvertSToF\0" // IR(30888, 12)
  "ConvertUToF\0" // IR(30900, 12)
  "UConvert\0" // IR(30912, 9)
  "SConvert\0" // IR(30921, 9)
  "FConvert\0" // IR(30930, 9)
  "QuantizeToF16\0" // IR(30939, 14)
  "ConvertPtrToU\0" // IR(30953, 14)
  "SatConvertSToU\0" // IR(30967, 15)
  "SatConvertUToS\0" // IR(30982, 15)
  "ConvertUToPtr\0" // IR(30997, 14)
  "PtrCastToGeneric\0" // IR(31011, 17)
  "GenericCastToPtr\0" // IR(31028, 17)
  "GenericCastToPtrExplicit\0" // IR(31045, 25)
  "Bitcast\0" // IR(31070, 8)
  "SNegate\0" // IR(31078, 8)
  "FNegate\0" // IR(31086, 8)
  "IAdd\0" // IR(31094, 5)
  "FAdd\0" // IR(31099, 5)
  "ISub\0" // IR(31104, 5)
  "FSub\0" // IR(31109, 5)
  "IMul\0" // IR(31114, 5)
  "FMul\0" // IR(31119, 5)
  "UDiv\0" // IR(31124, 5)
  "SDiv\0" // IR(31129, 5)
  "FDiv\0" // IR(31134, 5)
  "UMod\0" // IR(31139, 5)
  "SRem\0" // IR(31144, 5)
  "SMod\0" // IR(31149, 5)
  "FRem\0" // IR(31154, 5)
  "FMod\0" // IR(31159, 5)
  "VectorTimesScalar\0" // IR(31164, 18)
  "MatrixTimesScalar\0" // IR(31182, 18)
  "VectorTimesMatrix\0" // IR(31200, 18)
  "MatrixTimesVector\0" // IR(31218, 18)
  "MatrixTimesMatrix\0" // IR(31236, 18)
  "OuterProduct\0" // IR(31254, 13)
  "Dot\0" // IR(31267, 4)
  "IAddCarry\0" // IR(31271, 10)
  "ISubBorrow\0" // IR(31281, 11)
  "UMulExtended\0" // IR(31292, 13)
  "SMulExtended\0" // IR(31305, 13)
  "Any\0" // IR(31318, 4)
  "All\0" // IR(31322, 4)
  "IsNan\0" // IR(31326, 6)
  "IsInf\0" // IR(31332, 6)
  "IsFinite\0" // IR(31338, 9)
  "IsNormal\0" // IR(31347, 9)
  "SignBitSet\0" // IR(31356, 11)
  "LessOrGreater\0" // IR(31367, 14)
  "Ordered\0" // IR(31381, 8)
  "Unordered\0" // IR(31389, 10)
  "LogicalEqual\0" // IR(31399, 13)
  "LogicalNotEqual\0" // IR(31412, 16)
  "LogicalOr\0" // IR(31428, 10)
  "LogicalAnd\0" // IR(31438, 11)
  "LogicalNot\0" // IR(31449, 11)
  "Select\0" // IR(31460, 7)
  "IEqual\0" // IR(31467, 7)
  "INotEqual\0" // IR(31474, 10)
  "UGreaterThan\0" // IR(31484, 13)
  "SGreaterThan\0" // IR(31497, 13)
  "UGreaterThanEqual\0" // IR(31510, 18)
  "SGreaterThanEqual\0" // IR(31528, 18)
  "ULessThan\0" // IR(31546, 10)
  "SLessThan\0" // IR(31556, 10)
  "ULessThanEqual\0" // IR(31566, 15)
  "SLessThanEqual\0" // IR(31581, 15)
  "FOrdEqual\0" // IR(31596, 10)
  "FUnordEqual\0" // IR(31606, 12)
  "FOrdNotEqual\0" // IR(31618, 13)
  "FUnordNotEqual\0" // IR(31631, 15)
  "FOrdLessThan\0" // IR(31646, 13)
  "FUnordLessThan\0" // IR(31659, 15)
  "FOrdGreaterThan\0" // IR(31674, 16)
  "FUnordGreaterThan\0" // IR(31690, 18)
  "FOrdLessThanEqual\0" // IR(31708, 18)
  "FUnordLessThanEqual\0" // IR(31726, 20)
  "FOrdGreaterThanEqual\0" // IR(31746, 21)
  "FUnordGreaterThanEqual\0" // IR(31767, 23)
  "ShiftRightLogical\0" // IR(31790, 18)
  "ShiftRightArithmetic\0" // IR(31808, 21)
  "ShiftLeftLogical\0" // IR(31829, 17)
  "BitwiseOr\0" // IR(31846, 10)
  "BitwiseXor\0" // IR(31856, 11)
  "BitwiseAnd\0" // IR(31867, 11)
  "Not\0" // IR(31878, 4)
  "BitFieldInsert\0" // IR(31882, 15)
  "BitFieldSExtract\0" // IR(31897, 17)
  "BitFieldUExtract\0" // IR(31914, 17)
  "BitReverse\0" // IR(31931, 11)
  "BitCount\0" // IR(31942, 9)
  "DPdx\0" // IR(31951, 5)
  "DPdy\0" // IR(31956, 5)
  "Fwidth\0" // IR(31961, 7)
  "DPdxFine\0" // IR(31968, 9)
  "DPdyFine\0" // IR(31977, 9)
  "FwidthFine\0" // IR(31986, 11)
  "DPdxCoarse\0" // IR(31997, 11)
  "DPdyCoarse\0" // IR(32008, 11)
  "FwidthCoarse\0" // IR(32019, 13)
  "EmitVertex\0" // IR(32032, 11)
  "EndPrimitive\0" // IR(32043, 13)
  "EmitStreamVertex\0" // IR(32056, 17)
  "EndStreamPrimitive\0" // IR(32073, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(32092, 37)
  "ControlBarrier\0" // IR(32129, 15)
  "MemoryBarrier\0" // IR(32144, 14)
  "AtomicLoad\0" // IR(32158, 11)
  "AtomicStore\0" // IR(32169, 12)
  "AtomicExchange\0" // IR(32181, 15)
  "AtomicCompareExchange\0" // IR(32196, 22)
  "AtomicCompareExchangeWeak\0" // IR(32218, 26)
  "AtomicIIncrement\0" // IR(32244, 17)
  "AtomicIDecrement\0" // IR(32261, 17)
  "AtomicIAdd\0" // IR(32278, 11)
  "AtomicISub\0" // IR(32289, 11)
  "AtomicSMin\0" // IR(32300, 11)
  "AtomicUMin\0" // IR(32311, 11)
  "AtomicSMax\0" // IR(32322, 11)
  "AtomicUMax\0" // IR(32333, 11)
  "AtomicAnd\0" // IR(32344, 10)
  "AtomicOr\0" // IR(32354, 9)
  "AtomicXor\0" // IR(32363, 10)
  "Phi\0" // IR(32373, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(32377, 30)
  "LoopMerge\0" // IR(32407, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(32417, 35)
  "SelectionMerge\0" // IR(32452, 15)
  "Label\0" // IR(32467, 6)
  "Branch\0" // IR(32473, 7)
  "BranchConditional\0" // IR(32480, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(32498, 45)
  "Switch\0" // IR(32543, 7)
  "Kill\0" // IR(32550, 5)
  "Return\0" // IR(32555, 7)
  "ReturnValue\0" // IR(32562, 12)
  "Unreachable\0" // IR(32574, 12)
  "LifetimeStart\0" // IR(32586, 14)
  "LifetimeStop\0" // IR(32600, 13)
  "GroupAsyncCopy\0" // IR(32613, 15)
  "GroupWaitEvents\0" // IR(32628, 16)
  "GroupAll\0" // IR(32644, 9)
  "GroupAny\0" // IR(32653, 9)
  "GroupBroadcast\0" // IR(32662, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(32677, 33)
  "GroupIAdd\0" // IR(32710, 10)
  "GroupFAdd\0" // IR(32720, 10)
  "GroupFMin\0" // IR(32730, 10)
  "GroupUMin\0" // IR(32740, 10)
  "GroupSMin\0" // IR(32750, 10)
  "GroupFMax\0" // IR(32760, 10)
  "GroupUMax\0" // IR(32770, 10)
  "GroupSMax\0" // IR(32780, 10)
  "ReadPipe\0" // IR(32790, 9)
  "WritePipe\0" // IR(32799, 10)
  "ReservedReadPipe\0" // IR(32809, 17)
  "ReservedWritePipe\0" // IR(32826, 18)
  "ReserveReadPipePackets\0" // IR(32844, 23)
  "ReserveWritePipePackets\0" // IR(32867, 24)
  "CommitReadPipe\0" // IR(32891, 15)
  "CommitWritePipe\0" // IR(32906, 16)
  "IsValidReserveId\0" // IR(32922, 17)
  "GetNumPipePackets\0" // IR(32939, 18)
  "GetMaxPipePackets\0" // IR(32957, 18)
  "GroupReserveReadPipePackets\0" // IR(32975, 28)
  "GroupReserveWritePipePackets\0" // IR(33003, 29)
  "GroupCommitReadPipe\0" // IR(33032, 20)
  "GroupCommitWritePipe\0" // IR(33052, 21)
  "EnqueueMarker\0" // IR(33073, 14)
  "EnqueueKernel\0" // IR(33087, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(33101, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(33131, 32)
  "GetKernelWorkGroupSize\0" // IR(33163, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(33186, 40)
  "RetainEvent\0" // IR(33226, 12)
  "ReleaseEvent\0" // IR(33238, 13)
  "CreateUserEvent\0" // IR(33251, 16)
  "IsValidEvent\0" // IR(33267, 13)
  "SetUserEventStatus\0" // IR(33280, 19)
  "CaptureEventProfilingInfo\0" // IR(33299, 26)
  "GetDefaultQueue\0" // IR(33325, 16)
  "BuildNDRange\0" // IR(33341, 13)
  "ImageSparseSampleImplicitLod\0" // IR(33354, 29)
  "ImageSparseSampleExplicitLod\0" // IR(33383, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(33412, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(33445, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(33478, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(33511, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(33544, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(33581, 37)
  "ImageSparseFetch\0" // IR(33618, 17)
  "ImageSparseGather\0" // IR(33635, 18)
  "ImageSparseDrefGather\0" // IR(33653, 22)
  "ImageSparseTexelsResident\0" // IR(33675, 26)
  "NoLine\0" // IR(33701, 7)
  "AtomicFlagTestAndSet\0" // IR(33708, 21)
  "AtomicFlagClear\0" // IR(33729, 16)
  "ImageSparseRead\0" // IR(33745, 16)
  "SizeOf\0" // IR(33761, 7)
  "TypePipeStorage\0" // IR(33768, 16)
  "ConstantPipeStorage\0" // IR(33784, 20)
  "CreatePipeFromPipeStorage\0" // IR(33804, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(33830, 35)
  "GetKernelMaxNumSubgroups\0" // IR(33865, 25)
  "TypeNamedBarrier\0" // IR(33890, 17)
  "NamedBarrierInitialize\0" // IR(33907, 23)
  "MemoryNamedBarrier\0" // IR(33930, 19)
  "ModuleProcessed\0" // IR(33949, 16)
  "ExecutionModeId\0" // IR(33965, 16)
  "DecorateId\0" // IR(33981, 11)
  "GroupNonUniformElect\0" // IR(33992, 21)
  "GroupNonUniformAll\0" // IR(34013, 19)
  "GroupNonUniformAny\0" // IR(34032, 19)
  "GroupNonUniformAllEqual\0" // IR(34051, 24)
  "GroupNonUniformBroadcast\0" // IR(34075, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(34100, 30)
  "GroupNonUniformInverseBallot\0" // IR(34130, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(34159, 32)
  "GroupNonUniformBallotBitCount\0" // IR(34191, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(34221, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(34250, 29)
  "GroupNonUniformShuffleXor\0" // IR(34279, 26)
  "GroupNonUniformShuffleUp\0" // IR(34305, 25)
  "GroupNonUniformShuffleDown\0" // IR(34330, 27)
  "GroupNonUniformIAdd\0" // IR(34357, 20)
  "GroupNonUniformFAdd\0" // IR(34377, 20)
  "GroupNonUniformIMul\0" // IR(34397, 20)
  "GroupNonUniformFMul\0" // IR(34417, 20)
  "GroupNonUniformSMin\0" // IR(34437, 20)
  "GroupNonUniformUMin\0" // IR(34457, 20)
  "GroupNonUniformFMin\0" // IR(34477, 20)
  "GroupNonUniformSMax\0" // IR(34497, 20)
  "GroupNonUniformUMax\0" // IR(34517, 20)
  "GroupNonUniformFMax\0" // IR(34537, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(34557, 26)
  "GroupNonUniformBitwiseOr\0" // IR(34583, 25)
  "GroupNonUniformBitwiseXor\0" // IR(34608, 26)
  "GroupNonUniformLogicalAnd\0" // IR(34634, 26)
  "GroupNonUniformLogicalOr\0" // IR(34660, 25)
  "GroupNonUniformLogicalXor\0" // IR(34685, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(34711, 29)
  "GroupNonUniformQuadSwap\0" // IR(34740, 24)
  "CopyLogical\0" // IR(34764, 12)
  "PtrEqual\0" // IR(34776, 9)
  "PtrNotEqual\0" // IR(34785, 12)
  "PtrDiff\0" // IR(34797, 8)
  "ColorAttachmentReadEXT\0" // IR(34805, 23)
  "DepthAttachmentReadEXT\0" // IR(34828, 23)
  "StencilAttachmentReadEXT\0" // IR(34851, 25)
  "TypeTensorARM\0" // IR(34876, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(34890, 42)
  "TensorReadARM\0" // IR(34932, 14)
  "TensorWriteARM\0" // IR(34946, 15)
  "TensorQuerySizeARM\0" // IR(34961, 19)
  "GraphConstantARM\0" // IR(34980, 17)
  "GraphEntryPointARM\0" // IR(34997, 19)
  "GraphInputARM\0" // IR(35016, 14)
  "GraphSetOutputARM\0" // IR(35030, 18)
  "GraphEndARM\0" // IR(35048, 12)
  "TypeGraphARM\0" // IR(35060, 13)
  "TerminateInvocation\0" // IR(35073, 20)
  "TypeUntypedPointerKHR\0" // IR(35093, 22)
  "UntypedVariableKHR\0" // IR(35115, 19)
  "UntypedAccessChainKHR\0" // IR(35134, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(35156, 30)
  "SubgroupFirstInvocationKHR\0" // IR(35186, 27)
  "UntypedPtrAccessChainKHR\0" // IR(35213, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(35238, 33)
  "UntypedArrayLengthKHR\0" // IR(35271, 22)
  "UntypedPrefetchKHR\0" // IR(35293, 19)
  "FmaKHR\0" // IR(35312, 7)
  "SubgroupAllKHR\0" // IR(35319, 15)
  "SubgroupAnyKHR\0" // IR(35334, 15)
  "SubgroupAllEqualKHR\0" // IR(35349, 20)
  "SubgroupReadInvocationKHR\0" // IR(35369, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(35395, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(35421, 25)
  "TraceRayKHR\0" // IR(35446, 12)
  "ExecuteCallableKHR\0" // IR(35458, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(35477, 35)
  "IgnoreIntersectionKHR\0" // IR(35512, 22)
  "TerminateRayKHR\0" // IR(35534, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(35550, 47)
  "SDot\0" // IR(35597, 5)
  "SDotKHR\0" // IR(35602, 8)
  "UDot\0" // IR(35610, 5)
  "UDotKHR\0" // IR(35615, 8)
  "SUDot\0" // IR(35623, 6)
  "SUDotKHR\0" // IR(35629, 9)
  "SDotAccSat\0" // IR(35638, 11)
  "SDotAccSatKHR\0" // IR(35649, 14)
  "UDotAccSat\0" // IR(35663, 11)
  "UDotAccSatKHR\0" // IR(35674, 14)
  "SUDotAccSat\0" // IR(35688, 12)
  "SUDotAccSatKHR\0" // IR(35700, 15)
  "TypeCooperativeMatrixKHR\0" // IR(35715, 25)
  "CooperativeMatrixLoadKHR\0" // IR(35740, 25)
  "CooperativeMatrixStoreKHR\0" // IR(35765, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(35791, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(35845, 27)
  "CooperativeMatrixLengthKHR\0" // IR(35872, 27)
  "ConstantCompositeReplicateEXT\0" // IR(35899, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(35929, 34)
  "CompositeConstructReplicateEXT\0" // IR(35963, 31)
  "TypeRayQueryKHR\0" // IR(35994, 16)
  "RayQueryInitializeKHR\0" // IR(36010, 22)
  "RayQueryTerminateKHR\0" // IR(36032, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(36053, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(36085, 31)
  "RayQueryProceedKHR\0" // IR(36116, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(36135, 31)
  "ImageSampleWeightedQCOM\0" // IR(36166, 24)
  "ImageBoxFilterQCOM\0" // IR(36190, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(36209, 23)
  "ImageBlockMatchSADQCOM\0" // IR(36232, 23)
  "BitCastArrayQCOM\0" // IR(36255, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(36272, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(36301, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(36330, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(36359, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(36388, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(36418, 28)
  "ExtractSubArrayQCOM\0" // IR(36446, 20)
  "GroupIAddNonUniformAMD\0" // IR(36466, 23)
  "GroupFAddNonUniformAMD\0" // IR(36489, 23)
  "GroupFMinNonUniformAMD\0" // IR(36512, 23)
  "GroupUMinNonUniformAMD\0" // IR(36535, 23)
  "GroupSMinNonUniformAMD\0" // IR(36558, 23)
  "GroupFMaxNonUniformAMD\0" // IR(36581, 23)
  "GroupUMaxNonUniformAMD\0" // IR(36604, 23)
  "GroupSMaxNonUniformAMD\0" // IR(36627, 23)
  "FragmentMaskFetchAMD\0" // IR(36650, 21)
  "FragmentFetchAMD\0" // IR(36671, 17)
  "ReadClockKHR\0" // IR(36688, 13)
  "AllocateNodePayloadsAMDX\0" // IR(36701, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(36726, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(36750, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(36775, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(36804, 27)
  "IsNodePayloadValidAMDX\0" // IR(36831, 23)
  "ConstantStringAMDX\0" // IR(36854, 19)
  "SpecConstantStringAMDX\0" // IR(36873, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(36896, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(36922, 26)
  "TypeBufferEXT\0" // IR(36948, 14)
  "BufferPointerEXT\0" // IR(36962, 17)
  "UntypedImageTexelPointerEXT\0" // IR(36979, 28)
  "MemberDecorateIdEXT\0" // IR(37007, 20)
  "ConstantSizeOfEXT\0" // IR(37027, 18)
  "SpecConstantDataKHR\0" // IR(37045, 20)
  "PoisonKHR\0" // IR(37065, 10)
  "FreezeKHR\0" // IR(37075, 10)
  "HitObjectRecordHitMotionNV\0" // IR(37085, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(37112, 36)
  "HitObjectRecordMissMotionNV\0" // IR(37148, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(37176, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(37204, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(37232, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(37265, 30)
  "HitObjectTraceRayMotionNV\0" // IR(37295, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(37321, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(37360, 44)
  "HitObjectRecordEmptyNV\0" // IR(37404, 23)
  "HitObjectTraceRayNV\0" // IR(37427, 20)
  "HitObjectRecordHitNV\0" // IR(37447, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(37468, 30)
  "HitObjectRecordMissNV\0" // IR(37498, 22)
  "HitObjectExecuteShaderNV\0" // IR(37520, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(37545, 26)
  "HitObjectGetAttributesNV\0" // IR(37571, 25)
  "HitObjectGetHitKindNV\0" // IR(37596, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(37618, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(37647, 28)
  "HitObjectGetInstanceIdNV\0" // IR(37675, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(37700, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(37734, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(37766, 29)
  "HitObjectGetRayTMaxNV\0" // IR(37795, 22)
  "HitObjectGetRayTMinNV\0" // IR(37817, 22)
  "HitObjectIsEmptyNV\0" // IR(37839, 19)
  "HitObjectIsHitNV\0" // IR(37858, 17)
  "HitObjectIsMissNV\0" // IR(37875, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(37893, 29)
  "ReorderThreadWithHintNV\0" // IR(37922, 24)
  "TypeHitObjectNV\0" // IR(37946, 16)
  "ImageSampleFootprintNV\0" // IR(37962, 23)
  "TypeVectorIdEXT\0" // IR(37985, 16)
  "TypeCooperativeVectorNV\0" // IR(38001, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(38025, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(38054, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(38096, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(38135, 32)
  "CooperativeMatrixConvertNV\0" // IR(38167, 27)
  "EmitMeshTasksEXT\0" // IR(38194, 17)
  "SetMeshOutputsEXT\0" // IR(38211, 18)
  "GroupNonUniformPartitionEXT\0" // IR(38229, 28)
  "GroupNonUniformPartitionNV\0" // IR(38257, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(38284, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(38317, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(38352, 38)
  "CooperativeVectorLoadNV\0" // IR(38390, 24)
  "CooperativeVectorStoreNV\0" // IR(38414, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(38439, 28)
  "HitObjectRecordMissEXT\0" // IR(38467, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(38490, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(38519, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(38570, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(38594, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(38639, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(38672, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(38704, 38)
  "TypeHitObjectEXT\0" // IR(38742, 17)
  "ReorderThreadWithHintEXT\0" // IR(38759, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(38784, 30)
  "HitObjectTraceRayEXT\0" // IR(38814, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(38835, 27)
  "HitObjectRecordEmptyEXT\0" // IR(38862, 24)
  "HitObjectExecuteShaderEXT\0" // IR(38886, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(38912, 27)
  "HitObjectGetAttributesEXT\0" // IR(38939, 26)
  "HitObjectGetHitKindEXT\0" // IR(38965, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(38988, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(39018, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(39047, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(39073, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(39108, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(39139, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(39173, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(39206, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(39236, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(39265, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(39294, 23)
  "ReportIntersectionKHR\0" // IR(39317, 22)
  "ReportIntersectionNV\0" // IR(39339, 21)
  "IgnoreIntersectionNV\0" // IR(39360, 21)
  "TerminateRayNV\0" // IR(39381, 15)
  "TraceNV\0" // IR(39396, 8)
  "TraceMotionNV\0" // IR(39404, 14)
  "TraceRayMotionNV\0" // IR(39418, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(39435, 50)
  "TypeAccelerationStructureKHR\0" // IR(39485, 29)
  "TypeAccelerationStructureNV\0" // IR(39514, 28)
  "ExecuteCallableNV\0" // IR(39542, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(39560, 35)
  "RayQueryGetClusterIdNV\0" // IR(39595, 23)
  "HitObjectGetClusterIdNV\0" // IR(39618, 24)
  "HitObjectGetRayTMinEXT\0" // IR(39642, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(39665, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(39710, 40)
  "HitObjectIsEmptyEXT\0" // IR(39750, 20)
  "HitObjectIsHitEXT\0" // IR(39770, 18)
  "HitObjectIsMissEXT\0" // IR(39788, 19)
  "TypeCooperativeMatrixNV\0" // IR(39807, 24)
  "CooperativeMatrixLoadNV\0" // IR(39831, 24)
  "CooperativeMatrixStoreNV\0" // IR(39855, 25)
  "CooperativeMatrixMulAddNV\0" // IR(39880, 26)
  "CooperativeMatrixLengthNV\0" // IR(39906, 26)
  "BeginInvocationInterlockEXT\0" // IR(39932, 28)
  "EndInvocationInterlockEXT\0" // IR(39960, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(39986, 43)
  "CooperativeMatrixReduceNV\0" // IR(40029, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(40055, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(40086, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(40130, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(40160, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(40191, 32)
  "TypeTensorLayoutNV\0" // IR(40223, 19)
  "TypeTensorViewNV\0" // IR(40242, 17)
  "CreateTensorLayoutNV\0" // IR(40259, 21)
  "TensorLayoutSetDimensionNV\0" // IR(40280, 27)
  "TensorLayoutSetStrideNV\0" // IR(40307, 24)
  "TensorLayoutSliceNV\0" // IR(40331, 20)
  "TensorLayoutSetClampValueNV\0" // IR(40351, 28)
  "CreateTensorViewNV\0" // IR(40379, 19)
  "TensorViewSetDimensionNV\0" // IR(40398, 25)
  "TensorViewSetStrideNV\0" // IR(40423, 22)
  "IsHelperInvocationEXT\0" // IR(40445, 22)
  "TensorViewSetClipNV\0" // IR(40467, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(40487, 27)
  "CooperativeMatrixTransposeNV\0" // IR(40514, 29)
  "ConvertUToImageNV\0" // IR(40543, 18)
  "ConvertUToSamplerNV\0" // IR(40561, 20)
  "ConvertImageToUNV\0" // IR(40581, 18)
  "ConvertSamplerToUNV\0" // IR(40599, 20)
  "ConvertUToSampledImageNV\0" // IR(40619, 25)
  "ConvertSampledImageToUNV\0" // IR(40644, 25)
  "SamplerImageAddressingModeNV\0" // IR(40669, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(40698, 52)
  "RawAccessChainNV\0" // IR(40750, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(40767, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(40807, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(40845, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(40883, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(40917, 37)
  "HitObjectGetSpherePositionNV\0" // IR(40954, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(40983, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(41010, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(41037, 23)
  "HitObjectIsSphereHitNV\0" // IR(41060, 23)
  "HitObjectIsLSSHitNV\0" // IR(41083, 20)
  "RayQueryIsSphereHitNV\0" // IR(41103, 22)
  "RayQueryIsLSSHitNV\0" // IR(41125, 19)
  "SubgroupShuffleDownINTEL\0" // IR(41144, 25)
  "SubgroupShuffleUpINTEL\0" // IR(41169, 23)
  "SubgroupShuffleXorINTEL\0" // IR(41192, 24)
  "SubgroupBlockReadINTEL\0" // IR(41216, 23)
  "SubgroupBlockWriteINTEL\0" // IR(41239, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(41263, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(41291, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(41320, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(41353, 34)
  "UCountLeadingZerosINTEL\0" // IR(41387, 24)
  "UCountTrailingZerosINTEL\0" // IR(41411, 25)
  "AbsISubINTEL\0" // IR(41436, 13)
  "AbsUSubINTEL\0" // IR(41449, 13)
  "IAddSatINTEL\0" // IR(41462, 13)
  "UAddSatINTEL\0" // IR(41475, 13)
  "IAverageINTEL\0" // IR(41488, 14)
  "UAverageINTEL\0" // IR(41502, 14)
  "IAverageRoundedINTEL\0" // IR(41516, 21)
  "UAverageRoundedINTEL\0" // IR(41537, 21)
  "ISubSatINTEL\0" // IR(41558, 13)
  "USubSatINTEL\0" // IR(41571, 13)
  "IMul32x16INTEL\0" // IR(41584, 15)
  "UMul32x16INTEL\0" // IR(41599, 15)
  "ConstantFunctionPointerINTEL\0" // IR(41614, 29)
  "FunctionPointerCallINTEL\0" // IR(41643, 25)
  "AsmTargetINTEL\0" // IR(41668, 15)
  "AsmCallINTEL\0" // IR(41683, 13)
  "AtomicFMinEXT\0" // IR(41696, 14)
  "AtomicFMaxEXT\0" // IR(41710, 14)
  "AssumeTrueKHR\0" // IR(41724, 14)
  "ExpectKHR\0" // IR(41738, 10)
  "DecorateString\0" // IR(41748, 15)
  "DecorateStringGOOGLE\0" // IR(41763, 21)
  "MemberDecorateString\0" // IR(41784, 21)
  "MemberDecorateStringGOOGLE\0" // IR(41805, 27)
  "VmeImageINTEL\0" // IR(41832, 14)
  "TypeVmeImageINTEL\0" // IR(41846, 18)
  "TypeAvcImePayloadINTEL\0" // IR(41864, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(41887, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(41910, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(41933, 23)
  "TypeAvcMceResultINTEL\0" // IR(41956, 22)
  "TypeAvcImeResultINTEL\0" // IR(41978, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(42000, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(42046, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(42090, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(42129, 37)
  "TypeAvcRefResultINTEL\0" // IR(42166, 22)
  "TypeAvcSicResultINTEL\0" // IR(42188, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(42210, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(42270, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(42323, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(42370, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(42410, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(42461, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(42505, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(42556, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(42612, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(42662, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(42714, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(42763, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(42810, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(42860, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(42911, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(42967, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(43000, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(43052, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(43113, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(43174, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(43213, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(43251, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(43290, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(43328, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(43367, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(43405, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(43441, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(43480, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(43523, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(43561, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(43599, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(43637, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(43682, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(43722, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(43784, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(43814, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(43852, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(43888, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(43921, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(43956, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(43995, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(44038, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(44085, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(44139, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(44173, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(44220, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(44265, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(44320, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(44373, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(44429, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(44483, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(44541, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(44597, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(44635, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(44681, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(44725, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(44774, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(44821, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(44891, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(44959, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(45028, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(45096, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(45162, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(45229, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(45265, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(45313, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(45372, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(45430, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(45486, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(45516, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(45546, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(45585, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(45631, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(45674, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(45721, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(45766, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(45812, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(45868, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(45906, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(45936, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(45968, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(46004, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(46046, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(46085, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(46124, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(46168, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(46216, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(46266, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(46309, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(46357, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(46400, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(46431, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(46478, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(46523, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(46569, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(46625, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(46663, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(46698, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(46742, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(46788, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(46829, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(46865, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(46915, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(46963, 35)
  "SaveMemoryINTEL\0" // IR(46998, 16)
  "RestoreMemoryINTEL\0" // IR(47014, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(47033, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(47062, 28)
  "ArbitraryFloatCastALTERA\0" // IR(47090, 25)
  "ArbitraryFloatCastINTEL\0" // IR(47115, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(47139, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(47171, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(47202, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(47232, 29)
  "ArbitraryFloatAddALTERA\0" // IR(47261, 24)
  "ArbitraryFloatAddINTEL\0" // IR(47285, 23)
  "ArbitraryFloatSubALTERA\0" // IR(47308, 24)
  "ArbitraryFloatSubINTEL\0" // IR(47332, 23)
  "ArbitraryFloatMulALTERA\0" // IR(47355, 24)
  "ArbitraryFloatMulINTEL\0" // IR(47379, 23)
  "ArbitraryFloatDivALTERA\0" // IR(47402, 24)
  "ArbitraryFloatDivINTEL\0" // IR(47426, 23)
  "ArbitraryFloatGTALTERA\0" // IR(47449, 23)
  "ArbitraryFloatGTINTEL\0" // IR(47472, 22)
  "ArbitraryFloatGEALTERA\0" // IR(47494, 23)
  "ArbitraryFloatGEINTEL\0" // IR(47517, 22)
  "ArbitraryFloatLTALTERA\0" // IR(47539, 23)
  "ArbitraryFloatLTINTEL\0" // IR(47562, 22)
  "ArbitraryFloatLEALTERA\0" // IR(47584, 23)
  "ArbitraryFloatLEINTEL\0" // IR(47607, 22)
  "ArbitraryFloatEQALTERA\0" // IR(47629, 23)
  "ArbitraryFloatEQINTEL\0" // IR(47652, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(47674, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(47700, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(47725, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(47751, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(47776, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(47801, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(47825, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(47851, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(47876, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(47901, 24)
  "ArbitraryFloatLogINTEL\0" // IR(47925, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(47948, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(47972, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(47997, 25)
  "ArbitraryFloatExpINTEL\0" // IR(48022, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(48045, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(48069, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(48094, 25)
  "ArbitraryFloatSinINTEL\0" // IR(48119, 23)
  "ArbitraryFloatCosINTEL\0" // IR(48142, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(48165, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(48191, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(48216, 25)
  "ArbitraryFloatASinINTEL\0" // IR(48241, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(48265, 26)
  "ArbitraryFloatACosINTEL\0" // IR(48291, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(48315, 26)
  "ArbitraryFloatATanINTEL\0" // IR(48341, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(48365, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(48391, 25)
  "ArbitraryFloatPowINTEL\0" // IR(48416, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(48439, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(48463, 24)
  "LoopControlINTEL\0" // IR(48487, 17)
  "AliasDomainDeclINTEL\0" // IR(48504, 21)
  "AliasScopeDeclINTEL\0" // IR(48525, 20)
  "AliasScopeListDeclINTEL\0" // IR(48545, 24)
  "FixedSqrtALTERA\0" // IR(48569, 16)
  "FixedSqrtINTEL\0" // IR(48585, 15)
  "FixedRecipALTERA\0" // IR(48600, 17)
  "FixedRecipINTEL\0" // IR(48617, 16)
  "FixedRsqrtALTERA\0" // IR(48633, 17)
  "FixedRsqrtINTEL\0" // IR(48650, 16)
  "FixedSinALTERA\0" // IR(48666, 15)
  "FixedSinINTEL\0" // IR(48681, 14)
  "FixedCosALTERA\0" // IR(48695, 15)
  "FixedCosINTEL\0" // IR(48710, 14)
  "FixedSinCosALTERA\0" // IR(48724, 18)
  "FixedSinCosINTEL\0" // IR(48742, 17)
  "FixedSinPiALTERA\0" // IR(48759, 17)
  "FixedSinPiINTEL\0" // IR(48776, 16)
  "FixedCosPiALTERA\0" // IR(48792, 17)
  "FixedCosPiINTEL\0" // IR(48809, 16)
  "FixedSinCosPiALTERA\0" // IR(48825, 20)
  "FixedSinCosPiINTEL\0" // IR(48845, 19)
  "FixedLogALTERA\0" // IR(48864, 15)
  "FixedLogINTEL\0" // IR(48879, 14)
  "FixedExpALTERA\0" // IR(48893, 15)
  "FixedExpINTEL\0" // IR(48908, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(48922, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(48952, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(48981, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(49011, 29)
  "ReadPipeBlockingALTERA\0" // IR(49040, 23)
  "ReadPipeBlockingINTEL\0" // IR(49063, 22)
  "WritePipeBlockingALTERA\0" // IR(49085, 24)
  "WritePipeBlockingINTEL\0" // IR(49109, 23)
  "RayQueryGetRayTMinKHR\0" // IR(49132, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(49154, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(49177, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(49205, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(49251, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(49288, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(49353, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(49393, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(49434, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(49473, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(49509, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(49555, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(49600, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(49642, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(49674, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(49703, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(49743, 40)
  "AtomicFAddEXT\0" // IR(49783, 14)
  "TypeBufferSurfaceINTEL\0" // IR(49797, 23)
  "TypeStructContinuedINTEL\0" // IR(49820, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(49845, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(49877, 36)
  "CompositeConstructContinuedINTEL\0" // IR(49913, 33)
  "ConvertFToBF16INTEL\0" // IR(49946, 20)
  "ConvertBF16ToFINTEL\0" // IR(49966, 20)
  "ControlBarrierArriveINTEL\0" // IR(49986, 26)
  "ControlBarrierWaitINTEL\0" // IR(50012, 24)
  "TaskSequenceCreateALTERA\0" // IR(50036, 25)
  "TaskSequenceCreateINTEL\0" // IR(50061, 24)
  "TaskSequenceAsyncALTERA\0" // IR(50085, 24)
  "TaskSequenceAsyncINTEL\0" // IR(50109, 23)
  "TaskSequenceGetALTERA\0" // IR(50132, 22)
  "TaskSequenceGetINTEL\0" // IR(50154, 21)
  "TaskSequenceReleaseALTERA\0" // IR(50175, 26)
  "TaskSequenceReleaseINTEL\0" // IR(50201, 25)
  "TypeTaskSequenceALTERA\0" // IR(50226, 23)
  "TypeTaskSequenceINTEL\0" // IR(50249, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(50271, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(50298, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(50323, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(50357, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(50391, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(50420, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(50446, 62)
  "BitwiseFunctionINTEL\0" // IR(50508, 21)
  "ConditionalExtensionINTEL\0" // IR(50529, 26)
  "ConditionalEntryPointINTEL\0" // IR(50555, 27)
  "ConditionalCapabilityINTEL\0" // IR(50582, 27)
  "SpecConstantTargetINTEL\0" // IR(50609, 24)
  "SpecConstantArchitectureINTEL\0" // IR(50633, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(50663, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(50700, 30)
  "ConditionalCopyObjectINTEL\0" // IR(50730, 27)
  "PredicatedLoadINTEL\0" // IR(50757, 20)
  "PredicatedStoreINTEL\0" // IR(50777, 21)
  "GroupIMulKHR\0" // IR(50798, 13)
  "GroupFMulKHR\0" // IR(50811, 13)
  "GroupBitwiseAndKHR\0" // IR(50824, 19)
  "GroupBitwiseOrKHR\0" // IR(50843, 18)
  "GroupBitwiseXorKHR\0" // IR(50861, 19)
  "GroupLogicalAndKHR\0" // IR(50880, 19)
  "GroupLogicalOrKHR\0" // IR(50899, 18)
  "GroupLogicalXorKHR\0" // IR(50917, 19)
  "RoundFToTF32INTEL\0" // IR(50936, 18)
  "MaskedGatherINTEL\0" // IR(50954, 18)
  "MaskedScatterINTEL\0" // IR(50972, 19)
  "ConvertHandleToImageINTEL\0" // IR(50991, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(51017, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(51045, 33)
  "FDot2MixAcc32VALVE\0" // IR(51078, 19)
  "FDot2MixAcc16VALVE\0" // IR(51097, 19)
  "FDot4MixAcc32VALVE\0" // IR(51116, 19)
  "MIN_SAD\0" // IR(51135, 8)
  "MIN_SAD_COST\0" // IR(51143, 13)
  "RAW_SAD\0" // IR(51156, 8)
  "DebugInfoNone\0" // IR(51164, 14)
  "DebugCompilationUnit\0" // IR(51178, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(51199, 52)
  "DebugTypeBasic\0" // IR(51251, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(51266, 34)
  "DebugTypePointer\0" // IR(51300, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(51317, 38)
  "DebugTypeQualifier\0" // IR(51355, 19)
  "DebugTypeArray\0" // IR(51374, 15)
  "DebugTypeVector\0" // IR(51389, 16)
  "DebugTypedef\0" // IR(51405, 13)
  "DebugTypeFunction\0" // IR(51418, 18)
  "DebugTypeEnum\0" // IR(51436, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(51450, 38)
  "DebugTypeComposite\0" // IR(51488, 19)
  "DebugTypeMember\0" // IR(51507, 16)
  "DebugTypeInheritance\0" // IR(51523, 21)
  "DebugTypePtrToMember\0" // IR(51544, 21)
  "DebugTypeTemplate\0" // IR(51565, 18)
  "DebugTypeTemplateParameter\0" // IR(51583, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(51610, 35)
  "DebugTypeTemplateParameterPack\0" // IR(51645, 31)
  "DebugGlobalVariable\0" // IR(51676, 20)
  "DebugFunctionDeclaration\0" // IR(51696, 25)
  "DebugFunction\0" // IR(51721, 14)
  "DebugLexicalBlock\0" // IR(51735, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(51753, 31)
  "DebugScope\0" // IR(51784, 11)
  "DebugNoScope\0" // IR(51795, 13)
  "DebugInlinedAt\0" // IR(51808, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(51823, 42)
  "DebugLocalVariable\0" // IR(51865, 19)
  "DebugInlinedVariable\0" // IR(51884, 21)
  "DebugDeclare\0" // IR(51905, 13)
  "DebugValue\0" // IR(51918, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(51929, 33)
  "DebugOperation\0" // IR(51962, 15)
  "DebugExpression\0" // IR(51977, 16)
  "DebugMacroDef\0" // IR(51993, 14)
  "DebugMacroUndef\0" // IR(52007, 16)
  "Round\0" // IR(52023, 6)
  "RoundEven\0" // IR(52029, 10)
  "Trunc\0" // IR(52039, 6)
  "FAbs\0" // IR(52045, 5)
  "SAbs\0" // IR(52050, 5)
  "FSign\0" // IR(52055, 6)
  "SSign\0" // IR(52061, 6)
  "Floor\0" // IR(52067, 6)
  "Ceil\0" // IR(52073, 5)
  "Fract\0" // IR(52078, 6)
  "Radians\0" // IR(52084, 8)
  "Degrees\0" // IR(52092, 8)
  "Sin\0" // IR(52100, 4)
  "Cos\0" // IR(52104, 4)
  "Tan\0" // IR(52108, 4)
  "Asin\0" // IR(52112, 5)
  "Acos\0" // IR(52117, 5)
  "Atan\0" // IR(52122, 5)
  "Sinh\0" // IR(52127, 5)
  "Cosh\0" // IR(52132, 5)
  "Tanh\0" // IR(52137, 5)
  "Asinh\0" // IR(52142, 6)
  "Acosh\0" // IR(52148, 6)
  "Atanh\0" // IR(52154, 6)
  "Atan2\0" // IR(52160, 6)
  "Pow\0" // IR(52166, 4)
  "Exp\0" // IR(52170, 4)
  "Log\0" // IR(52174, 4)
  "Exp2\0" // IR(52178, 5)
  "Log2\0" // IR(52183, 5)
  "Sqrt\0" // IR(52188, 5)
  "InverseSqrt\0" // IR(52193, 12)
  "Determinant\0" // IR(52205, 12)
  "MatrixInverse\0" // IR(52217, 14)
  "Modf\0" // IR(52231, 5)
  "ModfStruct\0" // IR(52236, 11)
  "FMin\0" // IR(52247, 5)
  "UMin\0" // IR(52252, 5)
  "SMin\0" // IR(52257, 5)
  "FMax\0" // IR(52262, 5)
  "UMax\0" // IR(52267, 5)
  "SMax\0" // IR(52272, 5)
  "FClamp\0" // IR(52277, 7)
  "UClamp\0" // IR(52284, 7)
  "SClamp\0" // IR(52291, 7)
  "FMix\0" // IR(52298, 5)
  "IMix\0" // IR(52303, 5)
  "Step\0" // IR(52308, 5)
  "SmoothStep\0" // IR(52313, 11)
  "Fma\0" // IR(52324, 4)
  "Frexp\0" // IR(52328, 6)
  "FrexpStruct\0" // IR(52334, 12)
  "Ldexp\0" // IR(52346, 6)
  "PackSnorm4x8\0" // IR(52352, 13)
  "PackUnorm4x8\0" // IR(52365, 13)
  "PackSnorm2x16\0" // IR(52378, 14)
  "PackUnorm2x16\0" // IR(52392, 14)
  "PackHalf2x16\0" // IR(52406, 13)
  "PackDouble2x32\0" // IR(52419, 15)
  "UnpackSnorm2x16\0" // IR(52434, 16)
  "UnpackUnorm2x16\0" // IR(52450, 16)
  "UnpackHalf2x16\0" // IR(52466, 15)
  "UnpackSnorm4x8\0" // IR(52481, 15)
  "UnpackUnorm4x8\0" // IR(52496, 15)
  "UnpackDouble2x32\0" // IR(52511, 17)
  "Length\0" // IR(52528, 7)
  "Distance\0" // IR(52535, 9)
  "Cross\0" // IR(52544, 6)
  "Normalize\0" // IR(52550, 10)
  "FaceForward\0" // IR(52560, 12)
  "Reflect\0" // IR(52572, 8)
  "Refract\0" // IR(52580, 8)
  "FindILsb\0" // IR(52588, 9)
  "FindSMsb\0" // IR(52597, 9)
  "FindUMsb\0" // IR(52606, 9)
  "InterpolateAtCentroid\0" // IR(52615, 22)
  "InterpolateAtSample\0" // IR(52637, 20)
  "InterpolateAtOffset\0" // IR(52657, 20)
  "NMin\0" // IR(52677, 5)
  "NMax\0" // IR(52682, 5)
  "NClamp\0" // IR(52687, 7)
  "ArgumentInfo\0" // IR(52694, 13)
  "ArgumentStorageBuffer\0" // IR(52707, 22)
  "ArgumentUniform\0" // IR(52729, 16)
  "ArgumentPodStorageBuffer\0" // IR(52745, 25)
  "ArgumentPodUniform\0" // IR(52770, 19)
  "ArgumentPodPushConstant\0" // IR(52789, 24)
  "ArgumentSampledImage\0" // IR(52813, 21)
  "ArgumentStorageImage\0" // IR(52834, 21)
  "ArgumentSampler\0" // IR(52855, 16)
  "ArgumentWorkgroup\0" // IR(52871, 18)
  "SpecConstantWorkgroupSize\0" // IR(52889, 26)
  "SpecConstantGlobalOffset\0" // IR(52915, 25)
  "SpecConstantWorkDim\0" // IR(52940, 20)
  "PushConstantGlobalOffset\0" // IR(52960, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(52985, 30)
  "PushConstantGlobalSize\0" // IR(53015, 23)
  "PushConstantRegionOffset\0" // IR(53038, 25)
  "PushConstantNumWorkgroups\0" // IR(53063, 26)
  "PushConstantRegionGroupOffset\0" // IR(53089, 30)
  "ConstantDataStorageBuffer\0" // IR(53119, 26)
  "ConstantDataUniform\0" // IR(53145, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(53165, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(53195, 28)
  "ArgumentPointerPushConstant\0" // IR(53223, 28)
  "ArgumentPointerUniform\0" // IR(53251, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(53274, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(53309, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(53347, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(53389, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(53434, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(53471, 40)
  "ArgumentStorageTexelBuffer\0" // IR(53511, 27)
  "ArgumentUniformTexelBuffer\0" // IR(53538, 27)
  "ConstantDataPointerPushConstant\0" // IR(53565, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(53597, 40)
  "PrintfInfo\0" // IR(53637, 11)
  "PrintfBufferStorageBuffer\0" // IR(53648, 26)
  "PrintfBufferPointerPushConstant\0" // IR(53674, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(53706, 34)
  "WorkgroupVariableSize\0" // IR(53740, 22)
  "DebugImportedEntity\0" // IR(53762, 20)
  "DebugSource\0" // IR(53782, 12)
  "DebugFunctionDefinition\0" // IR(53794, 24)
  "DebugSourceContinued\0" // IR(53818, 21)
  "DebugLine\0" // IR(53839, 10)
  "DebugNoLine\0" // IR(53849, 12)
  "DebugBuildIdentifier\0" // IR(53861, 21)
  "DebugStoragePath\0" // IR(53882, 17)
  "DebugEntryPoint\0" // IR(53899, 16)
  "DebugTypeMatrix\0" // IR(53915, 16)
  "DebugTypeVectorIdEXT\0" // IR(53931, 21)
  "DebugTypeCooperativeMatrixKHR\0" // IR(53952, 30)
  "Configuration\0" // IR(53982, 14)
  "StartCounter\0" // IR(53996, 13)
  "StopCounter\0" // IR(54009, 12)
  "PushConstants\0" // IR(54021, 14)
  "SpecializationMapEntry\0" // IR(54035, 23)
  "DescriptorSetBuffer\0" // IR(54058, 20)
  "DescriptorSetImage\0" // IR(54078, 19)
  "DescriptorSetSampler\0" // IR(54097, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(54118, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(54181, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(54226, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(54275, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(54324, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(54368, 50)
  "DebugModuleINTEL\0" // IR(54418, 17)
  "acos\0" // IR(54435, 5)
  "acosh\0" // IR(54440, 6)
  "acospi\0" // IR(54446, 7)
  "asin\0" // IR(54453, 5)
  "asinh\0" // IR(54458, 6)
  "asinpi\0" // IR(54464, 7)
  "atan\0" // IR(54471, 5)
  "atan2\0" // IR(54476, 6)
  "atanh\0" // IR(54482, 6)
  "atanpi\0" // IR(54488, 7)
  "atan2pi\0" // IR(54495, 8)
  "cbrt\0" // IR(54503, 5)
  "ceil\0" // IR(54508, 5)
  "copysign\0" // IR(54513, 9)
  "cos\0" // IR(54522, 4)
  "cosh\0" // IR(54526, 5)
  "cospi\0" // IR(54531, 6)
  "erfc\0" // IR(54537, 5)
  "erf\0" // IR(54542, 4)
  "exp\0" // IR(54546, 4)
  "exp2\0" // IR(54550, 5)
  "exp10\0" // IR(54555, 6)
  "expm1\0" // IR(54561, 6)
  "fabs\0" // IR(54567, 5)
  "fdim\0" // IR(54572, 5)
  "floor\0" // IR(54577, 6)
  "fma\0" // IR(54583, 4)
  "fmax\0" // IR(54587, 5)
  "fmin\0" // IR(54592, 5)
  "fmod\0" // IR(54597, 5)
  "fract\0" // IR(54602, 6)
  "frexp\0" // IR(54608, 6)
  "hypot\0" // IR(54614, 6)
  "ilogb\0" // IR(54620, 6)
  "ldexp\0" // IR(54626, 6)
  "lgamma\0" // IR(54632, 7)
  "lgamma_r\0" // IR(54639, 9)
  "log\0" // IR(54648, 4)
  "log2\0" // IR(54652, 5)
  "log10\0" // IR(54657, 6)
  "log1p\0" // IR(54663, 6)
  "logb\0" // IR(54669, 5)
  "mad\0" // IR(54674, 4)
  "maxmag\0" // IR(54678, 7)
  "minmag\0" // IR(54685, 7)
  "modf\0" // IR(54692, 5)
  "nan\0" // IR(54697, 4)
  "nextafter\0" // IR(54701, 10)
  "pow\0" // IR(54711, 4)
  "pown\0" // IR(54715, 5)
  "powr\0" // IR(54720, 5)
  "remainder\0" // IR(54725, 10)
  "remquo\0" // IR(54735, 7)
  "rint\0" // IR(54742, 5)
  "rootn\0" // IR(54747, 6)
  "round\0" // IR(54753, 6)
  "rsqrt\0" // IR(54759, 6)
  "sin\0" // IR(54765, 4)
  "sincos\0" // IR(54769, 7)
  "sinh\0" // IR(54776, 5)
  "sinpi\0" // IR(54781, 6)
  "sqrt\0" // IR(54787, 5)
  "tan\0" // IR(54792, 4)
  "tanh\0" // IR(54796, 5)
  "tanpi\0" // IR(54801, 6)
  "tgamma\0" // IR(54807, 7)
  "trunc\0" // IR(54814, 6)
  "half_cos\0" // IR(54820, 9)
  "half_divide\0" // IR(54829, 12)
  "half_exp\0" // IR(54841, 9)
  "half_exp2\0" // IR(54850, 10)
  "half_exp10\0" // IR(54860, 11)
  "half_log\0" // IR(54871, 9)
  "half_log2\0" // IR(54880, 10)
  "half_log10\0" // IR(54890, 11)
  "half_powr\0" // IR(54901, 10)
  "half_recip\0" // IR(54911, 11)
  "half_rsqrt\0" // IR(54922, 11)
  "half_sin\0" // IR(54933, 9)
  "half_sqrt\0" // IR(54942, 10)
  "half_tan\0" // IR(54952, 9)
  "native_cos\0" // IR(54961, 11)
  "native_divide\0" // IR(54972, 14)
  "native_exp\0" // IR(54986, 11)
  "native_exp2\0" // IR(54997, 12)
  "native_exp10\0" // IR(55009, 13)
  "native_log\0" // IR(55022, 11)
  "native_log2\0" // IR(55033, 12)
  "native_log10\0" // IR(55045, 13)
  "native_powr\0" // IR(55058, 12)
  "native_recip\0" // IR(55070, 13)
  "native_rsqrt\0" // IR(55083, 13)
  "native_sin\0" // IR(55096, 11)
  "native_sqrt\0" // IR(55107, 12)
  "native_tan\0" // IR(55119, 11)
  "fclamp\0" // IR(55130, 7)
  "degrees\0" // IR(55137, 8)
  "fmax_common\0" // IR(55145, 12)
  "fmin_common\0" // IR(55157, 12)
  "mix\0" // IR(55169, 4)
  "radians\0" // IR(55173, 8)
  "step\0" // IR(55181, 5)
  "smoothstep\0" // IR(55186, 11)
  "sign\0" // IR(55197, 5)
  "cross\0" // IR(55202, 6)
  "distance\0" // IR(55208, 9)
  "length\0" // IR(55217, 7)
  "normalize\0" // IR(55224, 10)
  "fast_distance\0" // IR(55234, 14)
  "fast_length\0" // IR(55248, 12)
  "fast_normalize\0" // IR(55260, 15)
  "s_abs\0" // IR(55275, 6)
  "s_abs_diff\0" // IR(55281, 11)
  "s_add_sat\0" // IR(55292, 10)
  "u_add_sat\0" // IR(55302, 10)
  "s_hadd\0" // IR(55312, 7)
  "u_hadd\0" // IR(55319, 7)
  "s_rhadd\0" // IR(55326, 8)
  "u_rhadd\0" // IR(55334, 8)
  "s_clamp\0" // IR(55342, 8)
  "u_clamp\0" // IR(55350, 8)
  "clz\0" // IR(55358, 4)
  "ctz\0" // IR(55362, 4)
  "s_mad_hi\0" // IR(55366, 9)
  "u_mad_sat\0" // IR(55375, 10)
  "s_mad_sat\0" // IR(55385, 10)
  "s_max\0" // IR(55395, 6)
  "u_max\0" // IR(55401, 6)
  "s_min\0" // IR(55407, 6)
  "u_min\0" // IR(55413, 6)
  "s_mul_hi\0" // IR(55419, 9)
  "rotate\0" // IR(55428, 7)
  "s_sub_sat\0" // IR(55435, 10)
  "u_sub_sat\0" // IR(55445, 10)
  "u_upsample\0" // IR(55455, 11)
  "s_upsample\0" // IR(55466, 11)
  "popcount\0" // IR(55477, 9)
  "s_mad24\0" // IR(55486, 8)
  "u_mad24\0" // IR(55494, 8)
  "s_mul24\0" // IR(55502, 8)
  "u_mul24\0" // IR(55510, 8)
  "vloadn\0" // IR(55518, 7)
  "vstoren\0" // IR(55525, 8)
  "vload_half\0" // IR(55533, 11)
  "vload_halfn\0" // IR(55544, 12)
  "vstore_half\0" // IR(55556, 12)
  "vstore_half_r\0" // IR(55568, 14)
  "vstore_halfn\0" // IR(55582, 13)
  "vstore_halfn_r\0" // IR(55595, 15)
  "vloada_halfn\0" // IR(55610, 13)
  "vstorea_halfn\0" // IR(55623, 14)
  "vstorea_halfn_r\0" // IR(55637, 16)
  "shuffle\0" // IR(55653, 8)
  "shuffle2\0" // IR(55661, 9)
  "printf\0" // IR(55670, 7)
  "prefetch\0" // IR(55677, 9)
  "bitselect\0" // IR(55686, 10)
  "select\0" // IR(55696, 7)
  "u_abs\0" // IR(55703, 6)
  "u_abs_diff\0" // IR(55709, 11)
  "u_mul_hi\0" // IR(55720, 9)
  "u_mad_hi\0" // IR(55729, 9)
  "CubeFaceIndexAMD\0" // IR(55738, 17)
  "CubeFaceCoordAMD\0" // IR(55755, 17)
  "TimeAMD\0" // IR(55772, 8)
  "SwizzleInvocationsAMD\0" // IR(55780, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(55802, 28)
  "WriteInvocationAMD\0" // IR(55830, 19)
  "MbcntAMD\0" // IR(55849, 9)
  "InterpolateAtVertexAMD\0" // IR(55858, 23)
  "FMin3AMD\0" // IR(55881, 9)
  "UMin3AMD\0" // IR(55890, 9)
  "SMin3AMD\0" // IR(55899, 9)
  "FMax3AMD\0" // IR(55908, 9)
  "UMax3AMD\0" // IR(55917, 9)
  "SMax3AMD\0" // IR(55926, 9)
  "FMid3AMD\0" // IR(55935, 9)
  "UMid3AMD\0" // IR(55944, 9)
  "SMid3AMD\0" // IR(55953, 9)
  "ARGMAX\0" // IR(55962, 7)
  "AVG_POOL2D\0" // IR(55969, 11)
  "CONV2D\0" // IR(55980, 7)
  "CONV3D\0" // IR(55987, 7)
  "DEPTHWISE_CONV2D\0" // IR(55994, 17)
  "FFT2D\0" // IR(56011, 6)
  "MATMUL\0" // IR(56017, 7)
  "MAX_POOL2D\0" // IR(56024, 11)
  "RFFT2D\0" // IR(56035, 7)
  "TRANSPOSE_CONV2D\0" // IR(56042, 17)
  "CLAMP\0" // IR(56059, 6)
  "ERF\0" // IR(56065, 4)
  "SIGMOID\0" // IR(56069, 8)
  "TANH\0" // IR(56077, 5)
  "ADD\0" // IR(56082, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(56086, 23)
  "BITWISE_AND\0" // IR(56109, 12)
  "BITWISE_OR\0" // IR(56121, 11)
  "BITWISE_XOR\0" // IR(56132, 12)
  "INTDIV\0" // IR(56144, 7)
  "LOGICAL_AND\0" // IR(56151, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(56163, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(56182, 20)
  "LOGICAL_OR\0" // IR(56202, 11)
  "LOGICAL_XOR\0" // IR(56213, 12)
  "MAXIMUM\0" // IR(56225, 8)
  "MINIMUM\0" // IR(56233, 8)
  "MUL\0" // IR(56241, 4)
  "POW\0" // IR(56245, 4)
  "SUB\0" // IR(56249, 4)
  "TABLE\0" // IR(56253, 6)
  "ABS\0" // IR(56259, 4)
  "BITWISE_NOT\0" // IR(56263, 12)
  "CEIL\0" // IR(56275, 5)
  "CLZ\0" // IR(56280, 4)
  "COS\0" // IR(56284, 4)
  "EXP\0" // IR(56288, 4)
  "FLOOR\0" // IR(56292, 6)
  "LOG\0" // IR(56298, 4)
  "LOGICAL_NOT\0" // IR(56302, 12)
  "NEGATE\0" // IR(56314, 7)
  "RECIPROCAL\0" // IR(56321, 11)
  "RSQRT\0" // IR(56332, 6)
  "SIN\0" // IR(56338, 4)
  "SELECT\0" // IR(56342, 7)
  "EQUAL\0" // IR(56349, 6)
  "GREATER\0" // IR(56355, 8)
  "GREATER_EQUAL\0" // IR(56363, 14)
  "REDUCE_ALL\0" // IR(56377, 11)
  "REDUCE_ANY\0" // IR(56388, 11)
  "REDUCE_MAX\0" // IR(56399, 11)
  "REDUCE_MIN\0" // IR(56410, 11)
  "REDUCE_PRODUCT\0" // IR(56421, 15)
  "REDUCE_SUM\0" // IR(56436, 11)
  "CONCAT\0" // IR(56447, 7)
  "PAD\0" // IR(56454, 4)
  "RESHAPE\0" // IR(56458, 8)
  "REVERSE\0" // IR(56466, 8)
  "SLICE\0" // IR(56474, 6)
  "TILE\0" // IR(56480, 5)
  "TRANSPOSE\0" // IR(56485, 10)
  "GATHER\0" // IR(56495, 7)
  "SCATTER\0" // IR(56502, 8)
  "RESIZE\0" // IR(56510, 7)
  "CAST\0" // IR(56517, 5)
  "RESCALE\0" // IR(56522, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(5732, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6689, 18), // 1 SubgroupEqMaskKHR
  IR(6744, 18), // 2 SubgroupGeMaskKHR
  IR(6777, 18), // 3 SubgroupGtMaskKHR
  IR(6810, 18), // 4 SubgroupLeMaskKHR
  IR(6843, 18), // 5 SubgroupLtMaskKHR
  IR(7750, 12), // 6 BaryCoordNV
  IR(7805, 19), // 7 BaryCoordNoPerspNV
  IR(7836, 15), // 8 FragmentSizeNV
  IR(7893, 22), // 9 InvocationsPerPixelNV
  IR(8021, 11), // 10 LaunchIdNV
  IR(8046, 13), // 11 LaunchSizeNV
  IR(8077, 17), // 12 WorldRayOriginNV
  IR(8115, 20), // 13 WorldRayDirectionNV
  IR(8154, 18), // 14 ObjectRayOriginNV
  IR(8194, 21), // 15 ObjectRayDirectionNV
  IR(8226, 10), // 16 RayTminNV
  IR(8247, 10), // 17 RayTmaxNV
  IR(8280, 22), // 18 InstanceCustomIndexNV
  IR(8319, 16), // 19 ObjectToWorldNV
  IR(8352, 16), // 20 WorldToObjectNV
  IR(8386, 10), // 21 HitKindNV
  IR(8617, 19), // 22 IncomingRayFlagsNV
  IR(10393, 28), // 23 StorageUniformBufferBlock16
  IR(10456, 17), // 24 StorageUniform16
  IR(11312, 27), // 25 ShaderViewportIndexLayerNV
  IR(11356, 22), // 26 FragmentBarycentricNV
  IR(11409, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11439, 14), // 28 ShadingRateNV
  IR(11483, 29), // 29 GroupNonUniformPartitionedNV
  IR(11529, 20), // 30 ShaderNonUniformEXT
  IR(11572, 26), // 31 RuntimeDescriptorArrayEXT
  IR(11634, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(11712, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(11793, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(11872, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(11948, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(12024, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(12100, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(12178, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12262, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12349, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12412, 21), // 42 VulkanMemoryModelKHR
  IR(12462, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(12494, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(12560, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(12764, 28), // 46 DemoteToHelperInvocationEXT
  IR(13805, 26), // 47 FPGAMemoryAttributesINTEL
  IR(13884, 32), // 48 ArbitraryPrecisionIntegersINTEL
  IR(13954, 37), // 49 ArbitraryPrecisionFloatingPointINTEL
  IR(14044, 22), // 50 FPGALoopControlsINTEL
  IR(14139, 24), // 51 FPGAMemoryAccessesINTEL
  IR(14191, 27), // 52 FPGAClusterAttributesINTEL
  IR(14233, 14), // 53 LoopFuseINTEL
  IR(14268, 20), // 54 FPGADSPControlINTEL
  IR(14355, 40), // 55 FPGAInvocationPipeliningAttributesINTEL
  IR(14420, 24), // 56 FPGABufferLocationINTEL
  IR(14479, 34), // 57 ArbitraryPrecisionFixedPointINTEL
  IR(14537, 23), // 58 USMStorageClassesINTEL
  IR(14590, 29), // 59 RuntimeAlignedAttributeINTEL
  IR(14633, 13), // 60 IOPipesINTEL
  IR(14666, 19), // 61 BlockingPipesINTEL
  IR(14699, 13), // 62 FPGARegINTEL
  IR(14731, 22), // 63 DotProductInputAllKHR
  IR(14775, 25), // 64 DotProductInput4x8BitKHR
  IR(14828, 31), // 65 DotProductInput4x8BitPackedKHR
  IR(14870, 14), // 66 DotProductKHR
  IR(15042, 13), // 67 OptNoneINTEL
  IR(15187, 29), // 68 FPGAClusterAttributesV2INTEL
  IR(15263, 18), // 69 TaskSequenceINTEL
  IR(15322, 24), // 70 FPGALatencyControlINTEL
  IR(15375, 28), // 71 FPGAArgumentInterfacesINTEL
  IR(15469, 35), // 72 GlobalVariableFPGADecorationsINTEL
  IR(18461, 15), // 73 PerPrimitiveNV
  IR(18509, 12), // 74 PerVertexNV
  IR(18532, 14), // 75 NonUniformEXT
  IR(18562, 19), // 76 RestrictPointerEXT
  IR(18596, 18), // 77 AliasedPointerEXT
  IR(18959, 24), // 78 HlslCounterBufferGOOGLE
  IR(18996, 19), // 79 HlslSemanticGOOGLE
  IR(19126, 14), // 80 RegisterINTEL
  IR(19153, 12), // 81 MemoryINTEL
  IR(19180, 14), // 82 NumbanksINTEL
  IR(19210, 15), // 83 BankwidthINTEL
  IR(19248, 22), // 84 MaxPrivateCopiesINTEL
  IR(19287, 16), // 85 SinglepumpINTEL
  IR(19320, 16), // 86 DoublepumpINTEL
  IR(19356, 19), // 87 MaxReplicatesINTEL
  IR(19396, 20), // 88 SimpleDualPortINTEL
  IR(19428, 11), // 89 MergeINTEL
  IR(19496, 14), // 90 BankBitsINTEL
  IR(19531, 20), // 91 ForcePow2DepthINTEL
  IR(19568, 16), // 92 StridesizeINTEL
  IR(19599, 14), // 93 WordsizeINTEL
  IR(19632, 18), // 94 TrueDualPortINTEL
  IR(19670, 19), // 95 BurstCoalesceINTEL
  IR(19705, 15), // 96 CacheSizeINTEL
  IR(19749, 28), // 97 DontStaticallyCoalesceINTEL
  IR(19792, 14), // 98 PrefetchINTEL
  IR(19824, 17), // 99 StallEnableINTEL
  IR(19867, 25), // 100 FuseLoopsInFunctionINTEL
  IR(19912, 19), // 101 MathOpDSPModeINTEL
  IR(19985, 24), // 102 InitiationIntervalINTEL
  IR(20030, 20), // 103 MaxConcurrencyINTEL
  IR(20071, 20), // 104 PipelineEnableINTEL
  IR(20112, 20), // 105 BufferLocationINTEL
  IR(20152, 19), // 106 IOPipeStorageINTEL
  IR(20330, 15), // 107 StallFreeINTEL
  IR(20428, 25), // 108 LatencyControlLabelINTEL
  IR(20484, 30), // 109 LatencyControlConstraintINTEL
  IR(20542, 27), // 110 ConduitKernelArgumentINTEL
  IR(20601, 31), // 111 RegisterMapKernelArgumentINTEL
  IR(20666, 33), // 112 MMHostInterfaceAddressWidthINTEL
  IR(20730, 30), // 113 MMHostInterfaceDataWidthINTEL
  IR(20789, 28), // 114 MMHostInterfaceLatencyINTEL
  IR(20886, 34), // 115 MMHostInterfaceReadWriteModeINTEL
  IR(20950, 29), // 116 MMHostInterfaceMaxBurstINTEL
  IR(21012, 32), // 117 MMHostInterfaceWaitRequestINTEL
  IR(21071, 26), // 118 StableKernelArgumentINTEL
  IR(21214, 14), // 119 InitModeINTEL
  IR(21257, 28), // 120 ImplementInRegisterMapINTEL
  IR(22611, 14), // 121 OutputLinesNV
  IR(22645, 19), // 122 OutputPrimitivesNV
  IR(22688, 23), // 123 DerivativeGroupQuadsNV
  IR(22736, 24), // 124 DerivativeGroupLinearNV
  IR(22779, 18), // 125 OutputTrianglesNV
  IR(23527, 16), // 126 RayGenerationNV
  IR(23559, 15), // 127 IntersectionNV
  IR(23584, 9), // 128 AnyHitNV
  IR(23607, 13), // 129 ClosestHitNV
  IR(23628, 7), // 130 MissNV
  IR(23647, 11), // 131 CallableNV
  IR(23792, 23), // 132 AllowContractFastINTEL
  IR(23828, 18), // 133 AllowReassocINTEL
  IR(24054, 20), // 134 RuntimeAlignedINTEL
  IR(24146, 20), // 135 PartitionedReduceNV
  IR(24194, 27), // 136 PartitionedInclusiveScanNV
  IR(24249, 27), // 137 PartitionedExclusiveScanNV
  IR(24380, 22), // 138 MakeTexelAvailableKHR
  IR(24419, 20), // 139 MakeTexelVisibleKHR
  IR(24455, 19), // 140 NonPrivateTexelKHR
  IR(24488, 17), // 141 VolatileTexelKHR
  IR(25047, 27), // 142 InitOnDeviceReprogramINTEL
  IR(25098, 23), // 143 InitOnDeviceResetINTEL
  IR(25432, 21), // 144 DependencyArrayINTEL
  IR(25472, 18), // 145 LoopCoalesceINTEL
  IR(25512, 21), // 146 MaxInterleavingINTEL
  IR(25560, 26), // 147 SpeculatedIterationsINTEL
  IR(25601, 14), // 148 NoFusionINTEL
  IR(25631, 15), // 149 LoopCountINTEL
  IR(25673, 26), // 150 MaxReinvocationDelayINTEL
  IR(26065, 24), // 151 MakePointerAvailableKHR
  IR(26108, 22), // 152 MakePointerVisibleKHR
  IR(26148, 21), // 153 NonPrivatePointerKHR
  IR(26235, 10), // 154 VulkanKHR
  IR(16190, 5), // 155 None
  IR(26418, 16), // 156 OutputMemoryKHR
  IR(26448, 17), // 157 MakeAvailableKHR
  IR(26477, 15), // 158 MakeVisibleKHR
  IR(26553, 28), // 159 PackedVectorFormat4x8BitKHR
  IR(26868, 24), // 160 SkipBuiltinPrimitivesNV
  IR(27608, 15), // 161 QueueFamilyKHR
  IR(27979, 15), // 162 CallableDataNV
  IR(28018, 23), // 163 IncomingCallableDataNV
  IR(28055, 13), // 164 RayPayloadNV
  IR(28084, 15), // 165 HitAttributeNV
  IR(28121, 21), // 166 IncomingRayPayloadNV
  IR(28164, 21), // 167 ShaderRecordBufferNV
  IR(28207, 25), // 168 PhysicalStorageBufferEXT
  IR(28333, 16), // 169 DeviceOnlyINTEL
  IR(28364, 14), // 170 HostOnlyINTEL
  IR(35602, 8), // 171 SDotKHR
  IR(35615, 8), // 172 UDotKHR
  IR(35629, 9), // 173 SUDotKHR
  IR(35649, 14), // 174 SDotAccSatKHR
  IR(35674, 14), // 175 UDotAccSatKHR
  IR(35700, 15), // 176 SUDotAccSatKHR
  IR(38001, 24), // 177 TypeCooperativeVectorNV
  IR(38257, 27), // 178 GroupNonUniformPartitionNV
  IR(39339, 21), // 179 ReportIntersectionNV
  IR(39514, 28), // 180 TypeAccelerationStructureNV
  IR(39595, 23), // 181 RayQueryGetClusterIdNV
  IR(41763, 21), // 182 DecorateStringGOOGLE
  IR(41805, 27), // 183 MemberDecorateStringGOOGLE
  IR(47062, 28), // 184 ArbitraryFloatSinCosPiINTEL
  IR(47115, 24), // 185 ArbitraryFloatCastINTEL
  IR(47171, 31), // 186 ArbitraryFloatCastFromIntINTEL
  IR(47232, 29), // 187 ArbitraryFloatCastToIntINTEL
  IR(47285, 23), // 188 ArbitraryFloatAddINTEL
  IR(47332, 23), // 189 ArbitraryFloatSubINTEL
  IR(47379, 23), // 190 ArbitraryFloatMulINTEL
  IR(47426, 23), // 191 ArbitraryFloatDivINTEL
  IR(47472, 22), // 192 ArbitraryFloatGTINTEL
  IR(47517, 22), // 193 ArbitraryFloatGEINTEL
  IR(47562, 22), // 194 ArbitraryFloatLTINTEL
  IR(47607, 22), // 195 ArbitraryFloatLEINTEL
  IR(47652, 22), // 196 ArbitraryFloatEQINTEL
  IR(47700, 25), // 197 ArbitraryFloatRecipINTEL
  IR(47751, 25), // 198 ArbitraryFloatRSqrtINTEL
  IR(47801, 24), // 199 ArbitraryFloatCbrtINTEL
  IR(47851, 25), // 200 ArbitraryFloatHypotINTEL
  IR(47901, 24), // 201 ArbitraryFloatSqrtINTEL
  IR(48585, 15), // 202 FixedSqrtINTEL
  IR(48617, 16), // 203 FixedRecipINTEL
  IR(48650, 16), // 204 FixedRsqrtINTEL
  IR(48681, 14), // 205 FixedSinINTEL
  IR(48710, 14), // 206 FixedCosINTEL
  IR(48742, 17), // 207 FixedSinCosINTEL
  IR(48776, 16), // 208 FixedSinPiINTEL
  IR(48809, 16), // 209 FixedCosPiINTEL
  IR(48845, 19), // 210 FixedSinCosPiINTEL
  IR(48879, 14), // 211 FixedLogINTEL
  IR(48908, 14), // 212 FixedExpINTEL
  IR(48952, 29), // 213 PtrCastToCrossWorkgroupINTEL
  IR(49011, 29), // 214 CrossWorkgroupCastToPtrINTEL
  IR(49063, 22), // 215 ReadPipeBlockingINTEL
  IR(49109, 23), // 216 WritePipeBlockingINTEL
  IR(50061, 24), // 217 TaskSequenceCreateINTEL
  IR(50109, 23), // 218 TaskSequenceAsyncINTEL
  IR(50154, 21), // 219 TaskSequenceGetINTEL
  IR(50201, 25), // 220 TaskSequenceReleaseINTEL
  IR(50249, 22), // 221 TypeTaskSequenceINTEL
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
  spv::Capability::FragmentCoverageMESA, // 31
  spv::Capability::CoreBuiltinsARM, // 32
  spv::Capability::SubgroupBallotKHR, // 33
  spv::Capability::GroupNonUniformBallot, // 34
  spv::Capability::DrawParameters, // 35
  spv::Capability::DrawParameters, // 36
  spv::Capability::MeshShadingNV, // 37
  spv::Capability::MeshShadingEXT, // 38
  spv::Capability::FragmentShadingRateKHR, // 39
  spv::Capability::DeviceGroup, // 40
  spv::Capability::MultiView, // 41
  spv::Capability::TileShadingQCOM, // 42
  spv::Capability::StencilExportEXT, // 43
  spv::Capability::ShaderEnqueueAMDX, // 44
  spv::Capability::DescriptorHeapEXT, // 45
  spv::Capability::ShaderViewportMaskNV, // 46
  spv::Capability::MeshShadingNV, // 47
  spv::Capability::ShaderStereoViewNV, // 48
  spv::Capability::PerViewAttributesNV, // 49
  spv::Capability::MeshShadingNV, // 50
  spv::Capability::FragmentFullyCoveredEXT, // 51
  spv::Capability::MeshShadingNV, // 52
  spv::Capability::FragmentBarycentricKHR, // 53
  spv::Capability::FragmentDensityEXT, // 54
  spv::Capability::MeshShadingEXT, // 55
  spv::Capability::RayTracingNV, // 56
  spv::Capability::RayTracingKHR, // 57
  spv::Capability::RayTracingNV, // 58
  spv::Capability::RayTracingMotionBlurNV, // 59
  spv::Capability::RayTracingPositionFetchKHR, // 60
  spv::Capability::RayTracingDisplacementMicromapNV, // 61
  spv::Capability::RayTracingKHR, // 62
  spv::Capability::RayTracingSpheresGeometryNV, // 63
  spv::Capability::RayTracingLinearSweptSpheresGeometryNV, // 64
  spv::Capability::ShaderSMBuiltinsNV, // 65
  spv::Capability::RayTracingClusterAccelerationStructureNV, // 66
  spv::Capability::RayCullMaskKHR, // 67
  spv::Capability::Matrix, // 68
  spv::Capability::Int64, // 69
  spv::Capability::ImageBasic, // 70
  spv::Capability::Geometry, // 71
  spv::Capability::SampledCubeArray, // 72
  spv::Capability::SampledRect, // 73
  spv::Capability::Sampled1D, // 74
  spv::Capability::SampledBuffer, // 75
  spv::Capability::DeviceEnqueue, // 76
  spv::Capability::Pipes, // 77
  spv::Capability::GroupNonUniform, // 78
  spv::Capability::Float8EXT, // 79
  spv::Capability::CooperativeMatrixKHR, // 80
  spv::Capability::WorkgroupMemoryExplicitLayoutKHR, // 81
  spv::Capability::StorageBuffer16BitAccess, // 82
  spv::Capability::VariablePointersStorageBuffer, // 83
  spv::Capability::AtomicStorage, // 84
  spv::Capability::StorageBuffer8BitAccess, // 85
  spv::Capability::RayQueryKHR, // 86
  spv::Capability::RayTracingKHR, // 87
  spv::Capability::CooperativeMatrixKHR, // 88
  spv::Capability::Int4TypeINTEL, // 89
  spv::Capability::CooperativeMatrixKHR, // 90
  spv::Capability::BFloat16TypeKHR, // 91
  spv::Capability::BFloat16TypeKHR, // 92
  spv::Capability::CooperativeMatrixKHR, // 93
  spv::Capability::UntypedPointersKHR, // 94
  spv::Capability::MultiViewport, // 95
  spv::Capability::ShaderViewportIndexLayerEXT, // 96
  spv::Capability::ShaderViewportMaskNV, // 97
  spv::Capability::InputAttachment, // 98
  spv::Capability::ImageBuffer, // 99
  spv::Capability::ShaderNonUniform, // 100
  spv::Capability::InputAttachment, // 101
  spv::Capability::ShaderNonUniform, // 102
  spv::Capability::SampledBuffer, // 103
  spv::Capability::ShaderNonUniform, // 104
  spv::Capability::ImageBuffer, // 105
  spv::Capability::ShaderNonUniform, // 106
  spv::Capability::VectorAnyINTEL, // 107
  spv::Capability::Int8, // 108
  spv::Capability::FPGAClusterAttributesALTERA, // 109
  spv::Capability::FPGAKernelAttributesINTEL, // 110
  spv::Capability::Subgroup2DBlockIOINTEL, // 111
  spv::Capability::VariableLengthArrayINTEL, // 112
  spv::Capability::UntypedPointersKHR, // 113
  spv::Capability::SpecConditionalINTEL, // 114
  spv::Capability::Float16, // 115
  spv::Capability::Float8EXT, // 116
  spv::Capability::CooperativeMatrixLayoutsARM, // 117
  spv::Capability::Shader, // 118
  spv::Capability::Kernel, // 119
  spv::Capability::Shader, // 120
  spv::Capability::UniformDecoration, // 121
  spv::Capability::GeometryStreams, // 122
  spv::Capability::TransformFeedback, // 123
  spv::Capability::Kernel, // 124
  spv::Capability::FloatControls2, // 125
  spv::Capability::Linkage, // 126
  spv::Capability::ConstantDataKHR, // 127
  spv::Capability::SampleMaskOverrideCoverageNV, // 128
  spv::Capability::GeometryShaderPassthroughNV, // 129
  spv::Capability::MeshShadingNV, // 130
  spv::Capability::MeshShadingEXT, // 131
  spv::Capability::PushConstantBanksNV, // 132
  spv::Capability::ShaderInvocationReorderNV, // 133
  spv::Capability::ShaderInvocationReorderEXT, // 134
  spv::Capability::BindlessTextureNV, // 135
  spv::Capability::VectorComputeINTEL, // 136
  spv::Capability::IndirectReferencesINTEL, // 137
  spv::Capability::AsmINTEL, // 138
  spv::Capability::FunctionFloatControlINTEL, // 139
  spv::Capability::FPGAMemoryAttributesALTERA, // 140
  spv::Capability::FPGAMemoryAccessesALTERA, // 141
  spv::Capability::LoopFuseALTERA, // 142
  spv::Capability::FPGADSPControlALTERA, // 143
  spv::Capability::MemoryAccessAliasingINTEL, // 144
  spv::Capability::FPGAInvocationPipeliningAttributesALTERA, // 145
  spv::Capability::FPGABufferLocationALTERA, // 146
  spv::Capability::IOPipesALTERA, // 147
  spv::Capability::FPGAClusterAttributesV2ALTERA, // 148
  spv::Capability::FPMaxErrorINTEL, // 149
  spv::Capability::FPGALatencyControlALTERA, // 150
  spv::Capability::FPGAArgumentInterfacesALTERA, // 151
  spv::Capability::GlobalVariableHostAccessINTEL, // 152
  spv::Capability::GlobalVariableFPGADecorationsALTERA, // 153
  spv::Capability::CacheControlsINTEL, // 154
  spv::Capability::TileImageColorReadAccessEXT, // 155
  spv::Capability::Geometry, // 156
  spv::Capability::Tessellation, // 157
  spv::Capability::MeshShadingNV, // 158
  spv::Capability::MeshShadingEXT, // 159
  spv::Capability::Geometry, // 160
  spv::Capability::MeshShadingNV, // 161
  spv::Capability::MeshShadingEXT, // 162
  spv::Capability::SubgroupDispatch, // 163
  spv::Capability::TileImageDepthReadAccessEXT, // 164
  spv::Capability::TileImageStencilReadAccessEXT, // 165
  spv::Capability::SampleMaskPostDepthCoverage, // 166
  spv::Capability::DenormPreserve, // 167
  spv::Capability::DenormFlushToZero, // 168
  spv::Capability::SignedZeroInfNanPreserve, // 169
  spv::Capability::RoundingModeRTE, // 170
  spv::Capability::RoundingModeRTZ, // 171
  spv::Capability::QuadControlKHR, // 172
  spv::Capability::PoisonFreezeKHR, // 173
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 174
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 175
  spv::Capability::FragmentShaderPixelInterlockEXT, // 176
  spv::Capability::FragmentShaderSampleInterlockEXT, // 177
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 178
  spv::Capability::Shader64BitIndexingEXT, // 179
  spv::Capability::RoundToInfinityINTEL, // 180
  spv::Capability::KernelAttributesINTEL, // 181
  spv::Capability::FloatControls2, // 182
  spv::Capability::FPGAKernelAttributesv2INTEL, // 183
  spv::Capability::RegisterLimitsINTEL, // 184
  spv::Capability::FloatControls2, // 185
  spv::Capability::FPFastMathModeINTEL, // 186
  spv::Capability::OptNoneEXT, // 187
  spv::Capability::RuntimeAlignedAttributeALTERA, // 188
  spv::Capability::Kernel, // 189
  spv::Capability::GroupNonUniformArithmetic, // 190
  spv::Capability::GroupNonUniformBallot, // 191
  spv::Capability::GroupNonUniformClustered, // 192
  spv::Capability::GroupNonUniformPartitionedEXT, // 193
  spv::Capability::ImageGatherExtended, // 194
  spv::Capability::MinLod, // 195
  spv::Capability::VulkanMemoryModel, // 196
  spv::Capability::FPGALoopControlsALTERA, // 197
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 198
  spv::Capability::RawAccessChainsNV, // 199
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 200
  spv::Capability::RayTracingOpacityMicromapEXT, // 201
  spv::Capability::RayQueryKHR, // 202
  spv::Capability::StorageImageExtendedFormats, // 203
  spv::Capability::Int64ImageEXT, // 204
  spv::Capability::Shader, // 205
  spv::Capability::VectorComputeINTEL, // 206
  spv::Capability::GenericPointer, // 207
  spv::Capability::FunctionPointersINTEL, // 208
  spv::Capability::USMStorageClassesALTERA, // 209
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 210
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 211
  spv::Capability::TensorsARM, // 212
  spv::Capability::Addresses, // 213
  spv::Capability::PhysicalStorageBufferAddresses, // 214
  spv::Capability::LiteralSampler, // 215
  spv::Capability::Addresses, // 216
  spv::Capability::UntypedPointersKHR, // 217
  spv::Capability::Addresses, // 218
  spv::Capability::VariablePointers, // 219
  spv::Capability::VariablePointersStorageBuffer, // 220
  spv::Capability::PhysicalStorageBufferAddresses, // 221
  spv::Capability::Kernel, // 222
  spv::Capability::ImageQuery, // 223
  spv::Capability::ImageQuery, // 224
  spv::Capability::Shader, // 225
  spv::Capability::BitInstructions, // 226
  spv::Capability::DerivativeControl, // 227
  spv::Capability::Groups, // 228
  spv::Capability::SparseResidency, // 229
  spv::Capability::PipeStorage, // 230
  spv::Capability::NamedBarrier, // 231
  spv::Capability::GroupNonUniformVote, // 232
  spv::Capability::GroupNonUniformBallot, // 233
  spv::Capability::GroupNonUniformShuffle, // 234
  spv::Capability::GroupNonUniformShuffleRelative, // 235
  spv::Capability::GroupNonUniformArithmetic, // 236
  spv::Capability::GroupNonUniformClustered, // 237
  spv::Capability::GroupNonUniformPartitionedEXT, // 238
  spv::Capability::GroupNonUniformQuad, // 239
  spv::Capability::Addresses, // 240
  spv::Capability::VariablePointers, // 241
  spv::Capability::VariablePointersStorageBuffer, // 242
  spv::Capability::GraphARM, // 243
  spv::Capability::SubgroupBallotKHR, // 244
  spv::Capability::FMAKHR, // 245
  spv::Capability::SubgroupVoteKHR, // 246
  spv::Capability::GroupNonUniformRotateKHR, // 247
  spv::Capability::RayTracingKHR, // 248
  spv::Capability::RayQueryKHR, // 249
  spv::Capability::DotProduct, // 250
  spv::Capability::ReplicatedCompositesEXT, // 251
  spv::Capability::TextureSampleWeightedQCOM, // 252
  spv::Capability::TextureBoxFilterQCOM, // 253
  spv::Capability::TextureBlockMatchQCOM, // 254
  spv::Capability::CooperativeMatrixConversionQCOM, // 255
  spv::Capability::TextureBlockMatch2QCOM, // 256
  spv::Capability::FragmentMaskAMD, // 257
  spv::Capability::ShaderClockKHR, // 258
  spv::Capability::AbortKHR, // 259
  spv::Capability::ShaderInvocationReorderNV, // 260
  spv::Capability::RayTracingMotionBlurNV, // 261
  spv::Capability::ImageFootprintNV, // 262
  spv::Capability::CooperativeVectorNV, // 263
  spv::Capability::LongVectorEXT, // 264
  spv::Capability::CooperativeVectorNV, // 265
  spv::Capability::CooperativeVectorTrainingNV, // 266
  spv::Capability::CooperativeMatrixConversionsNV, // 267
  spv::Capability::DisplacementMicromapNV, // 268
  spv::Capability::ShaderInvocationReorderEXT, // 269
  spv::Capability::RayTracingMotionBlurNV, // 270
  spv::Capability::RayQueryPositionFetchKHR, // 271
  spv::Capability::RayTracingNV, // 272
  spv::Capability::RayTracingKHR, // 273
  spv::Capability::RayQueryKHR, // 274
  spv::Capability::DisplacementMicromapNV, // 275
  spv::Capability::CooperativeMatrixNV, // 276
  spv::Capability::FragmentShaderSampleInterlockEXT, // 277
  spv::Capability::FragmentShaderPixelInterlockEXT, // 278
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 279
  spv::Capability::CooperativeMatrixReductionsNV, // 280
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 281
  spv::Capability::TensorAddressingNV, // 282
  spv::Capability::DemoteToHelperInvocation, // 283
  spv::Capability::SubgroupShuffleINTEL, // 284
  spv::Capability::SubgroupBufferBlockIOINTEL, // 285
  spv::Capability::SubgroupImageBlockIOINTEL, // 286
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 287
  spv::Capability::IntegerFunctions2INTEL, // 288
  spv::Capability::AtomicFloat16MinMaxEXT, // 289
  spv::Capability::AtomicFloat32MinMaxEXT, // 290
  spv::Capability::AtomicFloat64MinMaxEXT, // 291
  spv::Capability::AtomicFloat16VectorNV, // 292
  spv::Capability::ExpectAssumeKHR, // 293
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 294
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 295
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 296
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 297
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 298
  spv::Capability::VariableLengthArrayINTEL, // 299
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 300
  spv::Capability::UnstructuredLoopControlsINTEL, // 301
  spv::Capability::BlockingPipesALTERA, // 302
  spv::Capability::FPGARegALTERA, // 303
  spv::Capability::AtomicFloat16AddEXT, // 304
  spv::Capability::AtomicFloat32AddEXT, // 305
  spv::Capability::AtomicFloat64AddEXT, // 306
  spv::Capability::AtomicFloat16VectorNV, // 307
  spv::Capability::LongCompositesINTEL, // 308
  spv::Capability::BFloat16ConversionINTEL, // 309
  spv::Capability::SplitBarrierINTEL, // 310
  spv::Capability::ArithmeticFenceEXT, // 311
  spv::Capability::TaskSequenceALTERA, // 312
  spv::Capability::SubgroupBufferPrefetchINTEL, // 313
  spv::Capability::Subgroup2DBlockTransformINTEL, // 314
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 315
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 316
  spv::Capability::TernaryBitwiseFunctionINTEL, // 317
  spv::Capability::UntypedVariableLengthArrayINTEL, // 318
  spv::Capability::FunctionVariantsINTEL, // 319
  spv::Capability::PredicatedIOINTEL, // 320
  spv::Capability::GroupUniformArithmeticKHR, // 321
  spv::Capability::TensorFloat32RoundingINTEL, // 322
  spv::Capability::MaskedGatherScatterINTEL, // 323
  spv::Capability::BindlessImagesINTEL, // 324
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 325
  spv::Capability::DotProductBFloat16AccVALVE, // 326
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 327
  spv::Capability::DotProductBFloat16AccVALVE, // 328
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 329
  spv::Capability::Float64, // 330
  spv::Capability::InterpolationFunction, // 331
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
  spvtools::Extension::kSPV_MESA_fragment_coverage_mask, // 34
  spvtools::Extension::kSPV_ARM_core_builtins, // 35
  spvtools::Extension::kSPV_EXT_shader_tile_image, // 36
  spvtools::Extension::kSPV_ARM_tensors, // 37
  spvtools::Extension::kSPV_ARM_graph, // 38
  spvtools::Extension::kSPV_ARM_cooperative_matrix_layouts, // 39
  spvtools::Extension::kSPV_EXT_float8, // 40
  spvtools::Extension::kSPV_KHR_workgroup_memory_explicit_layout, // 41
  spvtools::Extension::kSPV_KHR_subgroup_vote, // 42
  spvtools::Extension::kSPV_KHR_16bit_storage, // 43
  spvtools::Extension::kSPV_KHR_variable_pointers, // 44
  spvtools::Extension::kSPV_KHR_shader_atomic_counter_ops, // 45
  spvtools::Extension::kSPV_KHR_post_depth_coverage, // 46
  spvtools::Extension::kSPV_KHR_8bit_storage, // 47
  spvtools::Extension::kSPV_KHR_float_controls, // 48
  spvtools::Extension::kSPV_KHR_ray_query, // 49
  spvtools::Extension::kSPV_KHR_untyped_pointers, // 50
  spvtools::Extension::kSPV_KHR_ray_query, // 51
  spvtools::Extension::kSPV_KHR_ray_tracing, // 52
  spvtools::Extension::kSPV_QCOM_image_processing, // 53
  spvtools::Extension::kSPV_QCOM_tile_shading, // 54
  spvtools::Extension::kSPV_QCOM_cooperative_matrix_conversion, // 55
  spvtools::Extension::kSPV_QCOM_image_processing2, // 56
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 57
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 58
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 59
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 60
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 61
  spvtools::Extension::kSPV_KHR_shader_clock, // 62
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 63
  spvtools::Extension::kSPV_KHR_quad_control, // 64
  spvtools::Extension::kSPV_INTEL_int4, // 65
  spvtools::Extension::kSPV_KHR_bfloat16, // 66
  spvtools::Extension::kSPV_KHR_abort, // 67
  spvtools::Extension::kSPV_EXT_descriptor_heap, // 68
  spvtools::Extension::kSPV_KHR_constant_data, // 69
  spvtools::Extension::kSPV_KHR_poison_freeze, // 70
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 71
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 72
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 73
  spvtools::Extension::kSPV_NV_viewport_array2, // 74
  spvtools::Extension::kSPV_NV_viewport_array2, // 75
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 76
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 77
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 78
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 79
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 80
  spvtools::Extension::kSPV_EXT_shader_subgroup_partitioned, // 81
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 82
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 83
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 84
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 85
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 86
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 87
  spvtools::Extension::kSPV_NV_displacement_micromap, // 88
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 89
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 90
  spvtools::Extension::kSPV_EXT_shader_invocation_reorder, // 91
  spvtools::Extension::kSPV_NV_bindless_texture, // 92
  spvtools::Extension::kSPV_NV_cooperative_vector, // 93
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 94
  spvtools::Extension::kSPV_NV_raw_access_chains, // 95
  spvtools::Extension::kSPV_NV_push_constant_bank, // 96
  spvtools::Extension::kSPV_EXT_long_vector, // 97
  spvtools::Extension::kSPV_EXT_shader_64bit_indexing, // 98
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 99
  spvtools::Extension::kSPV_NV_tensor_addressing, // 100
  spvtools::Extension::kSPV_INTEL_subgroups, // 101
  spvtools::Extension::kSPV_INTEL_media_block_io, // 102
  spvtools::Extension::kSPV_INTEL_float_controls2, // 103
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 104
  spvtools::Extension::kSPV_INTEL_function_pointers, // 105
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 106
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 107
  spvtools::Extension::kSPV_INTEL_vector_compute, // 108
  spvtools::Extension::kSPV_KHR_expect_assume, // 109
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 110
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 111
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 112
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 113
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 114
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 115
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 116
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 117
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 118
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 119
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 120
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 121
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 122
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 123
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 124
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 125
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 126
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 127
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 128
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 129
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 130
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 131
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 132
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 133
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 134
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 135
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 136
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 137
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 138
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 139
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 140
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 141
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 142
  spvtools::Extension::kSPV_INTEL_io_pipes, // 143
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 144
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 145
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 146
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 147
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 148
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 149
  spvtools::Extension::kSPV_EXT_replicated_composites, // 150
  spvtools::Extension::kSPV_KHR_bit_instructions, // 151
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 152
  spvtools::Extension::kSPV_KHR_float_controls2, // 153
  spvtools::Extension::kSPV_KHR_fma, // 154
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 155
  spvtools::Extension::kSPV_INTEL_long_composites, // 156
  spvtools::Extension::kSPV_EXT_optnone, // 157
  spvtools::Extension::kSPV_INTEL_optnone, // 158
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 159
  spvtools::Extension::kSPV_INTEL_debug_module, // 160
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 161
  spvtools::Extension::kSPV_INTEL_split_barrier, // 162
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 163
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 164
  spvtools::Extension::kSPV_INTEL_task_sequence, // 165
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 166
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 167
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 168
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 169
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 170
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 171
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 172
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 173
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 174
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 175
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 176
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 177
  spvtools::Extension::kSPV_INTEL_function_variants, // 178
  spvtools::Extension::kSPV_INTEL_predicated_io, // 179
  spvtools::Extension::kSPV_INTEL_rounded_divide_sqrt, // 180
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 181
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 182
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 183
  spvtools::Extension::kSPV_INTEL_cache_controls, // 184
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 185
  spvtools::Extension::kSPV_INTEL_bindless_images, // 186
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 187
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 188
  spvtools::Extension::kSPV_NV_mesh_shader, // 189
  spvtools::Extension::kSPV_EXT_mesh_shader, // 190
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 191
  spvtools::Extension::kSPV_GOOGLE_user_type, // 192
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 193
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 194
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 195
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 196
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 197
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 198
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 199
  spvtools::Extension::kSPV_KHR_variable_pointers, // 200
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 201
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 202
  spvtools::Extension::kSPV_KHR_ray_tracing, // 203
  spvtools::Extension::kSPV_KHR_ray_query, // 204
  spvtools::Extension::kSPV_NV_ray_tracing, // 205
  spvtools::Extension::kSPV_KHR_ray_tracing, // 206
  spvtools::Extension::kSPV_KHR_ray_query, // 207
  spvtools::Extension::kSPV_NV_displacement_micromap, // 208
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 209
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 210
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
  SPV_OPERAND_TYPE_TYPE_ID, // 479
  SPV_OPERAND_TYPE_RESULT_ID, // 480
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 481
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
  SPV_OPERAND_TYPE_ID, // 532
  SPV_OPERAND_TYPE_ID, // 533
  SPV_OPERAND_TYPE_ID, // 534
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 535
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 536
  SPV_OPERAND_TYPE_TYPE_ID, // 537
  SPV_OPERAND_TYPE_RESULT_ID, // 538
  SPV_OPERAND_TYPE_ID, // 539
  SPV_OPERAND_TYPE_ID, // 540
  SPV_OPERAND_TYPE_ID, // 541
  SPV_OPERAND_TYPE_ID, // 542
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 543
  SPV_OPERAND_TYPE_TYPE_ID, // 544
  SPV_OPERAND_TYPE_RESULT_ID, // 545
  SPV_OPERAND_TYPE_ID, // 546
  SPV_OPERAND_TYPE_ID, // 547
  SPV_OPERAND_TYPE_ID, // 548
  SPV_OPERAND_TYPE_ID, // 549
  SPV_OPERAND_TYPE_ID, // 550
  SPV_OPERAND_TYPE_ID, // 551
  SPV_OPERAND_TYPE_ID, // 552
  SPV_OPERAND_TYPE_ID, // 553
  SPV_OPERAND_TYPE_ID, // 554
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 555
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 556
  SPV_OPERAND_TYPE_ID, // 557
  SPV_OPERAND_TYPE_ID, // 558
  SPV_OPERAND_TYPE_ID, // 559
  SPV_OPERAND_TYPE_ID, // 560
  SPV_OPERAND_TYPE_ID, // 561
  SPV_OPERAND_TYPE_ID, // 562
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 563
  SPV_OPERAND_TYPE_TYPE_ID, // 564
  SPV_OPERAND_TYPE_RESULT_ID, // 565
  SPV_OPERAND_TYPE_ID, // 566
  SPV_OPERAND_TYPE_ID, // 567
  SPV_OPERAND_TYPE_ID, // 568
  SPV_OPERAND_TYPE_ID, // 569
  SPV_OPERAND_TYPE_ID, // 570
  SPV_OPERAND_TYPE_ID, // 571
  SPV_OPERAND_TYPE_ID, // 572
  SPV_OPERAND_TYPE_ID, // 573
  SPV_OPERAND_TYPE_ID, // 574
  SPV_OPERAND_TYPE_ID, // 575
  SPV_OPERAND_TYPE_ID, // 576
  SPV_OPERAND_TYPE_ID, // 577
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 578
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 579
  SPV_OPERAND_TYPE_ID, // 580
  SPV_OPERAND_TYPE_ID, // 581
  SPV_OPERAND_TYPE_ID, // 582
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 583
  SPV_OPERAND_TYPE_TYPE_ID, // 584
  SPV_OPERAND_TYPE_RESULT_ID, // 585
  SPV_OPERAND_TYPE_ID, // 586
  SPV_OPERAND_TYPE_ID, // 587
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 588
  SPV_OPERAND_TYPE_ID, // 589
  SPV_OPERAND_TYPE_ID, // 590
  SPV_OPERAND_TYPE_ID, // 591
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 592
  SPV_OPERAND_TYPE_ID, // 593
  SPV_OPERAND_TYPE_ID, // 594
  SPV_OPERAND_TYPE_ID, // 595
  SPV_OPERAND_TYPE_ID, // 596
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 597
  SPV_OPERAND_TYPE_ID, // 598
  SPV_OPERAND_TYPE_ID, // 599
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 600
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 601
  SPV_OPERAND_TYPE_ID, // 602
  SPV_OPERAND_TYPE_ID, // 603
  SPV_OPERAND_TYPE_ID, // 604
  SPV_OPERAND_TYPE_ID, // 605
  SPV_OPERAND_TYPE_ID, // 606
  SPV_OPERAND_TYPE_ID, // 607
  SPV_OPERAND_TYPE_ID, // 608
  SPV_OPERAND_TYPE_ID, // 609
  SPV_OPERAND_TYPE_ID, // 610
  SPV_OPERAND_TYPE_ID, // 611
  SPV_OPERAND_TYPE_ID, // 612
  SPV_OPERAND_TYPE_ID, // 613
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 614
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 615
  SPV_OPERAND_TYPE_ID, // 616
  SPV_OPERAND_TYPE_ID, // 617
  SPV_OPERAND_TYPE_ID, // 618
  SPV_OPERAND_TYPE_ID, // 619
  SPV_OPERAND_TYPE_ID, // 620
  SPV_OPERAND_TYPE_ID, // 621
  SPV_OPERAND_TYPE_ID, // 622
  SPV_OPERAND_TYPE_ID, // 623
  SPV_OPERAND_TYPE_ID, // 624
  SPV_OPERAND_TYPE_ID, // 625
  SPV_OPERAND_TYPE_ID, // 626
  SPV_OPERAND_TYPE_ID, // 627
  SPV_OPERAND_TYPE_ID, // 628
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 629
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 630
  SPV_OPERAND_TYPE_RESULT_ID, // 631
  SPV_OPERAND_TYPE_ID, // 632
  SPV_OPERAND_TYPE_SCOPE_ID, // 633
  SPV_OPERAND_TYPE_ID, // 634
  SPV_OPERAND_TYPE_ID, // 635
  SPV_OPERAND_TYPE_TYPE_ID, // 636
  SPV_OPERAND_TYPE_RESULT_ID, // 637
  SPV_OPERAND_TYPE_ID, // 638
  SPV_OPERAND_TYPE_ID, // 639
  SPV_OPERAND_TYPE_ID, // 640
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 641
  SPV_OPERAND_TYPE_ID, // 642
  SPV_OPERAND_TYPE_ID, // 643
  SPV_OPERAND_TYPE_ID, // 644
  SPV_OPERAND_TYPE_ID, // 645
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 646
  SPV_OPERAND_TYPE_TYPE_ID, // 647
  SPV_OPERAND_TYPE_RESULT_ID, // 648
  SPV_OPERAND_TYPE_ID, // 649
  SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE, // 650
  SPV_OPERAND_TYPE_ID, // 651
  SPV_OPERAND_TYPE_TYPE_ID, // 652
  SPV_OPERAND_TYPE_RESULT_ID, // 653
  SPV_OPERAND_TYPE_ID, // 654
  SPV_OPERAND_TYPE_ID, // 655
  SPV_OPERAND_TYPE_ID, // 656
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 657
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 658
  SPV_OPERAND_TYPE_ID, // 659
  SPV_OPERAND_TYPE_ID, // 660
  SPV_OPERAND_TYPE_ID, // 661
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 662
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 663
  SPV_OPERAND_TYPE_RESULT_ID, // 664
  SPV_OPERAND_TYPE_ID, // 665
  SPV_OPERAND_TYPE_ID, // 666
  SPV_OPERAND_TYPE_VARIABLE_ID, // 667
  SPV_OPERAND_TYPE_TYPE_ID, // 668
  SPV_OPERAND_TYPE_RESULT_ID, // 669
  SPV_OPERAND_TYPE_ID, // 670
  SPV_OPERAND_TYPE_ID, // 671
  SPV_OPERAND_TYPE_ID, // 672
  SPV_OPERAND_TYPE_ID, // 673
  SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS, // 674
  SPV_OPERAND_TYPE_ID, // 675
  SPV_OPERAND_TYPE_ID, // 676
  SPV_OPERAND_TYPE_ID, // 677
  SPV_OPERAND_TYPE_ID, // 678
  SPV_OPERAND_TYPE_ID, // 679
  SPV_OPERAND_TYPE_TYPE_ID, // 680
  SPV_OPERAND_TYPE_RESULT_ID, // 681
  SPV_OPERAND_TYPE_ID, // 682
  SPV_OPERAND_TYPE_ID, // 683
  SPV_OPERAND_TYPE_LITERAL_STRING, // 684
  SPV_OPERAND_TYPE_LITERAL_STRING, // 685
  SPV_OPERAND_TYPE_TYPE_ID, // 686
  SPV_OPERAND_TYPE_RESULT_ID, // 687
  SPV_OPERAND_TYPE_ID, // 688
  SPV_OPERAND_TYPE_ID, // 689
  SPV_OPERAND_TYPE_ID, // 690
  SPV_OPERAND_TYPE_ID, // 691
  SPV_OPERAND_TYPE_ID, // 692
  SPV_OPERAND_TYPE_ID, // 693
  SPV_OPERAND_TYPE_ID, // 694
  SPV_OPERAND_TYPE_TYPE_ID, // 695
  SPV_OPERAND_TYPE_RESULT_ID, // 696
  SPV_OPERAND_TYPE_ID, // 697
  SPV_OPERAND_TYPE_ID, // 698
  SPV_OPERAND_TYPE_ID, // 699
  SPV_OPERAND_TYPE_ID, // 700
  SPV_OPERAND_TYPE_ID, // 701
  SPV_OPERAND_TYPE_ID, // 702
  SPV_OPERAND_TYPE_ID, // 703
  SPV_OPERAND_TYPE_ID, // 704
  SPV_OPERAND_TYPE_TYPE_ID, // 705
  SPV_OPERAND_TYPE_RESULT_ID, // 706
  SPV_OPERAND_TYPE_ID, // 707
  SPV_OPERAND_TYPE_ID, // 708
  SPV_OPERAND_TYPE_ID, // 709
  SPV_OPERAND_TYPE_ID, // 710
  SPV_OPERAND_TYPE_ID, // 711
  SPV_OPERAND_TYPE_ID, // 712
  SPV_OPERAND_TYPE_ID, // 713
  SPV_OPERAND_TYPE_ID, // 714
  SPV_OPERAND_TYPE_ID, // 715
  SPV_OPERAND_TYPE_ID, // 716
  SPV_OPERAND_TYPE_ID, // 717
  SPV_OPERAND_TYPE_TYPE_ID, // 718
  SPV_OPERAND_TYPE_RESULT_ID, // 719
  SPV_OPERAND_TYPE_ID, // 720
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 721
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 722
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 723
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 724
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 725
  SPV_OPERAND_TYPE_TYPE_ID, // 726
  SPV_OPERAND_TYPE_RESULT_ID, // 727
  SPV_OPERAND_TYPE_ID, // 728
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 729
  SPV_OPERAND_TYPE_ID, // 730
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 731
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 732
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 733
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 734
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 735
  SPV_OPERAND_TYPE_TYPE_ID, // 736
  SPV_OPERAND_TYPE_RESULT_ID, // 737
  SPV_OPERAND_TYPE_ID, // 738
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 739
  SPV_OPERAND_TYPE_ID, // 740
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 741
  SPV_OPERAND_TYPE_RESULT_ID, // 742
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 743
  SPV_OPERAND_TYPE_RESULT_ID, // 744
  SPV_OPERAND_TYPE_ID, // 745
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 746
  SPV_OPERAND_TYPE_VARIABLE_ID, // 747
  SPV_OPERAND_TYPE_TYPE_ID, // 748
  SPV_OPERAND_TYPE_RESULT_ID, // 749
  SPV_OPERAND_TYPE_ID, // 750
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 751
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 752
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 753
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 754
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
  SPV_OPERAND_TYPE_ID, // 766
  SPV_OPERAND_TYPE_ID, // 767
  SPV_OPERAND_TYPE_ID, // 768
  SPV_OPERAND_TYPE_ID, // 769
  SPV_OPERAND_TYPE_ID, // 770
  SPV_OPERAND_TYPE_ID, // 771
  SPV_OPERAND_TYPE_ID, // 772
  SPV_OPERAND_TYPE_ID, // 773
  SPV_OPERAND_TYPE_TYPE_ID, // 774
  SPV_OPERAND_TYPE_RESULT_ID, // 775
  SPV_OPERAND_TYPE_ID, // 776
  SPV_OPERAND_TYPE_ID, // 777
  SPV_OPERAND_TYPE_ID, // 778
  SPV_OPERAND_TYPE_ID, // 779
  SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS, // 780
  SPV_OPERAND_TYPE_ID, // 781
  SPV_OPERAND_TYPE_EXECUTION_MODEL, // 782
  SPV_OPERAND_TYPE_ID, // 783
  SPV_OPERAND_TYPE_LITERAL_STRING, // 784
  SPV_OPERAND_TYPE_VARIABLE_ID, // 785
  SPV_OPERAND_TYPE_ID, // 786
  SPV_OPERAND_TYPE_CAPABILITY, // 787
  SPV_OPERAND_TYPE_TYPE_ID, // 788
  SPV_OPERAND_TYPE_RESULT_ID, // 789
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 790
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 791
  SPV_OPERAND_TYPE_TYPE_ID, // 792
  SPV_OPERAND_TYPE_RESULT_ID, // 793
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 794
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 795
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 796
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 797
  SPV_OPERAND_TYPE_TYPE_ID, // 798
  SPV_OPERAND_TYPE_RESULT_ID, // 799
  SPV_OPERAND_TYPE_VARIABLE_CAPABILITY, // 800
  SPV_OPERAND_TYPE_TYPE_ID, // 801
  SPV_OPERAND_TYPE_RESULT_ID, // 802
  SPV_OPERAND_TYPE_ID, // 803
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 804
  SPV_OPERAND_TYPE_ID, // 805
  SPV_OPERAND_TYPE_ID, // 806
  SPV_OPERAND_TYPE_ID, // 807
  SPV_OPERAND_TYPE_ID, // 808
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 809
  SPV_OPERAND_TYPE_ID, // 810
  SPV_OPERAND_TYPE_ID, // 811
  SPV_OPERAND_TYPE_ID, // 812
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 813
  SPV_OPERAND_TYPE_ID, // 814
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 815
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 816
  SPV_OPERAND_TYPE_ID, // 817
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 818
  SPV_OPERAND_TYPE_ID, // 819
  SPV_OPERAND_TYPE_ID, // 820
  SPV_OPERAND_TYPE_ID, // 821
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 822
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 823
  SPV_OPERAND_TYPE_ID, // 824
  SPV_OPERAND_TYPE_ID, // 825
  SPV_OPERAND_TYPE_ID, // 826
  SPV_OPERAND_TYPE_ID, // 827
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 828
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 829
  SPV_OPERAND_TYPE_ID, // 830
  SPV_OPERAND_TYPE_ID, // 831
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 832
  SPV_OPERAND_TYPE_VARIABLE_ID, // 833
  SPV_OPERAND_TYPE_ID, // 834
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 835
  SPV_OPERAND_TYPE_ID, // 836
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 837
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 838
  SPV_OPERAND_TYPE_ID, // 839
  SPV_OPERAND_TYPE_ID, // 840
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 841
  SPV_OPERAND_TYPE_VARIABLE_ID, // 842
  SPV_OPERAND_TYPE_ID, // 843
  SPV_OPERAND_TYPE_ID, // 844
  SPV_OPERAND_TYPE_ID, // 845
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 846
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 847
  SPV_OPERAND_TYPE_ID, // 848
  SPV_OPERAND_TYPE_ID, // 849
  SPV_OPERAND_TYPE_ID, // 850
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 851
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 852
  SPV_OPERAND_TYPE_ID, // 853
  SPV_OPERAND_TYPE_ID, // 854
  SPV_OPERAND_TYPE_ID, // 855
  SPV_OPERAND_TYPE_ID, // 856
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 857
  SPV_OPERAND_TYPE_ID, // 858
  SPV_OPERAND_TYPE_ID, // 859
  SPV_OPERAND_TYPE_ID, // 860
  SPV_OPERAND_TYPE_ID, // 861
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 862
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 863
  SPV_OPERAND_TYPE_ID, // 864
  SPV_OPERAND_TYPE_ID, // 865
  SPV_OPERAND_TYPE_ID, // 866
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 867
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 868
  SPV_OPERAND_TYPE_ID, // 869
  SPV_OPERAND_TYPE_ID, // 870
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 871
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 872
  SPV_OPERAND_TYPE_VARIABLE_ID, // 873
  SPV_OPERAND_TYPE_ID, // 874
  SPV_OPERAND_TYPE_ID, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 877
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 878
  SPV_OPERAND_TYPE_ID, // 879
  SPV_OPERAND_TYPE_ID, // 880
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 881
  SPV_OPERAND_TYPE_ID, // 882
  SPV_OPERAND_TYPE_ID, // 883
  SPV_OPERAND_TYPE_ID, // 884
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 885
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 886
  SPV_OPERAND_TYPE_ID, // 887
  SPV_OPERAND_TYPE_ID, // 888
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 889
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 890
  SPV_OPERAND_TYPE_ID, // 891
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 892
  SPV_OPERAND_TYPE_ID, // 893
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 894
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 895
  SPV_OPERAND_TYPE_ID, // 896
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_ID, // 901
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 902
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 903
  SPV_OPERAND_TYPE_ID, // 904
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 905
  SPV_OPERAND_TYPE_ID, // 906
  SPV_OPERAND_TYPE_ID, // 907
  SPV_OPERAND_TYPE_ID, // 908
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 909
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 910
  SPV_OPERAND_TYPE_ID, // 911
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 912
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 913
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 914
  SPV_OPERAND_TYPE_ID, // 915
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 916
  SPV_OPERAND_TYPE_ID, // 917
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 918
  SPV_OPERAND_TYPE_ID, // 919
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 920
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 921
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 922
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 923
  SPV_OPERAND_TYPE_ID, // 924
  SPV_OPERAND_TYPE_ID, // 925
  SPV_OPERAND_TYPE_ID, // 926
  SPV_OPERAND_TYPE_ID, // 927
  SPV_OPERAND_TYPE_ID, // 928
  SPV_OPERAND_TYPE_ID, // 929
  SPV_OPERAND_TYPE_ID, // 930
  SPV_OPERAND_TYPE_ID, // 931
  SPV_OPERAND_TYPE_VARIABLE_ID, // 932
  SPV_OPERAND_TYPE_ID, // 933
  SPV_OPERAND_TYPE_ID, // 934
  SPV_OPERAND_TYPE_ID, // 935
  SPV_OPERAND_TYPE_ID, // 936
  SPV_OPERAND_TYPE_ID, // 937
  SPV_OPERAND_TYPE_ID, // 938
  SPV_OPERAND_TYPE_ID, // 939
  SPV_OPERAND_TYPE_ID, // 940
  SPV_OPERAND_TYPE_ID, // 941
  SPV_OPERAND_TYPE_VARIABLE_ID, // 942
  SPV_OPERAND_TYPE_ID, // 943
  SPV_OPERAND_TYPE_ID, // 944
  SPV_OPERAND_TYPE_ID, // 945
  SPV_OPERAND_TYPE_ID, // 946
  SPV_OPERAND_TYPE_ID, // 947
  SPV_OPERAND_TYPE_ID, // 948
  SPV_OPERAND_TYPE_ID, // 949
  SPV_OPERAND_TYPE_ID, // 950
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 951
  SPV_OPERAND_TYPE_ID, // 952
  SPV_OPERAND_TYPE_ID, // 953
  SPV_OPERAND_TYPE_ID, // 954
  SPV_OPERAND_TYPE_ID, // 955
  SPV_OPERAND_TYPE_VARIABLE_ID, // 956
  SPV_OPERAND_TYPE_ID, // 957
  SPV_OPERAND_TYPE_ID, // 958
  SPV_OPERAND_TYPE_ID, // 959
  SPV_OPERAND_TYPE_ID, // 960
  SPV_OPERAND_TYPE_ID, // 961
  SPV_OPERAND_TYPE_ID, // 962
  SPV_OPERAND_TYPE_ID, // 963
  SPV_OPERAND_TYPE_ID, // 964
  SPV_OPERAND_TYPE_ID, // 965
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 966
  SPV_OPERAND_TYPE_ID, // 967
  SPV_OPERAND_TYPE_ID, // 968
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 969
  SPV_OPERAND_TYPE_ID, // 970
  SPV_OPERAND_TYPE_ID, // 971
  SPV_OPERAND_TYPE_ID, // 972
  SPV_OPERAND_TYPE_ID, // 973
  SPV_OPERAND_TYPE_ID, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_ID, // 976
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 977
  SPV_OPERAND_TYPE_ID, // 978
  SPV_OPERAND_TYPE_ID, // 979
  SPV_OPERAND_TYPE_ID, // 980
  SPV_OPERAND_TYPE_VARIABLE_ID, // 981
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
  SPV_OPERAND_TYPE_ID, // 1046
  SPV_OPERAND_TYPE_ID, // 1047
  SPV_OPERAND_TYPE_ID, // 1048
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1049
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1050
  SPV_OPERAND_TYPE_ID, // 1051
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 1052
  SPV_OPERAND_TYPE_ID, // 1053
  SPV_OPERAND_TYPE_ID, // 1054
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 1055
  SPV_OPERAND_TYPE_ID, // 1056
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 1057
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1058
  SPV_OPERAND_TYPE_ID, // 1059
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 1060
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1061
  SPV_OPERAND_TYPE_ID, // 1062
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1063
  SPV_OPERAND_TYPE_ID, // 1064
  SPV_OPERAND_TYPE_ID, // 1065
  SPV_OPERAND_TYPE_ID, // 1066
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1067
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1068
  SPV_OPERAND_TYPE_ID, // 1069
  SPV_OPERAND_TYPE_ID, // 1070
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1071
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1072
  SPV_OPERAND_TYPE_ID, // 1073
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 1074
  SPV_OPERAND_TYPE_ID, // 1075
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1076
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1077
  SPV_OPERAND_TYPE_ID, // 1078
  SPV_OPERAND_TYPE_ID, // 1079
  SPV_OPERAND_TYPE_ID, // 1080
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1081
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1082
  SPV_OPERAND_TYPE_ID, // 1083
  SPV_OPERAND_TYPE_ID, // 1084
  SPV_OPERAND_TYPE_ID, // 1085
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1086
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1087
  SPV_OPERAND_TYPE_ID, // 1088
  SPV_OPERAND_TYPE_ID, // 1089
  SPV_OPERAND_TYPE_ID, // 1090
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1091
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1092
  SPV_OPERAND_TYPE_ID, // 1093
  SPV_OPERAND_TYPE_ID, // 1094
  SPV_OPERAND_TYPE_ID, // 1095
  SPV_OPERAND_TYPE_ID, // 1096
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1097
  SPV_OPERAND_TYPE_ID, // 1098
  SPV_OPERAND_TYPE_ID, // 1099
  SPV_OPERAND_TYPE_ID, // 1100
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1101
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1102
  SPV_OPERAND_TYPE_ID, // 1103
  SPV_OPERAND_TYPE_ID, // 1104
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1105
  SPV_OPERAND_TYPE_ID, // 1106
  SPV_OPERAND_TYPE_ID, // 1107
  SPV_OPERAND_TYPE_ID, // 1108
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1109
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1110
  SPV_OPERAND_TYPE_ID, // 1111
  SPV_OPERAND_TYPE_ID, // 1112
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1113
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1114
  SPV_OPERAND_TYPE_ID, // 1115
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1116
  SPV_OPERAND_TYPE_ID, // 1117
  SPV_OPERAND_TYPE_ID, // 1118
  SPV_OPERAND_TYPE_ID, // 1119
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1120
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1121
  SPV_OPERAND_TYPE_ID, // 1122
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1123
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1124
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1125
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1126
  SPV_OPERAND_TYPE_ID, // 1127
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1128
  SPV_OPERAND_TYPE_ID, // 1129
  SPV_OPERAND_TYPE_ID, // 1130
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1131
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1132
  SPV_OPERAND_TYPE_ID, // 1133
  SPV_OPERAND_TYPE_ID, // 1134
  SPV_OPERAND_TYPE_ID, // 1135
  SPV_OPERAND_TYPE_ID, // 1136
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1137
  SPV_OPERAND_TYPE_ID, // 1138
  SPV_OPERAND_TYPE_ID, // 1139
  SPV_OPERAND_TYPE_ID, // 1140
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1141
  SPV_OPERAND_TYPE_ID, // 1142
  SPV_OPERAND_TYPE_ID, // 1143
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1144
  SPV_OPERAND_TYPE_ID, // 1145
  SPV_OPERAND_TYPE_ID, // 1146
  SPV_OPERAND_TYPE_ID, // 1147
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1148
};
