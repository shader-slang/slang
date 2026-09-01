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
    case Extension::kSPV_AMD_weak_linkage: return IR(1020, 21);
    case Extension::kSPV_ARM_cooperative_matrix_layouts: return IR(1041, 35);
    case Extension::kSPV_ARM_core_builtins: return IR(1076, 22);
    case Extension::kSPV_ARM_graph: return IR(1098, 14);
    case Extension::kSPV_ARM_tensors: return IR(1112, 16);
    case Extension::kSPV_EXT_arithmetic_fence: return IR(1128, 25);
    case Extension::kSPV_EXT_cooperative_matrix_maintenance1: return IR(1153, 40);
    case Extension::kSPV_EXT_demote_to_helper_invocation: return IR(1193, 36);
    case Extension::kSPV_EXT_descriptor_heap: return IR(1229, 24);
    case Extension::kSPV_EXT_descriptor_indexing: return IR(1253, 28);
    case Extension::kSPV_EXT_float8: return IR(1281, 15);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(1296, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(1327, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(1363, 34);
    case Extension::kSPV_EXT_long_vector: return IR(1397, 20);
    case Extension::kSPV_EXT_mesh_shader: return IR(1417, 20);
    case Extension::kSPV_EXT_ocp_microscaling_types: return IR(1437, 31);
    case Extension::kSPV_EXT_opacity_micromap: return IR(1468, 25);
    case Extension::kSPV_EXT_optnone: return IR(1493, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(1509, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(1541, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(1585, 30);
    case Extension::kSPV_EXT_shader_64bit_indexing: return IR(1615, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(1645, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(1679, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(1711, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(1747, 27);
    case Extension::kSPV_EXT_shader_invocation_reorder: return IR(1774, 34);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(1808, 30);
    case Extension::kSPV_EXT_shader_subgroup_partitioned: return IR(1838, 36);
    case Extension::kSPV_EXT_shader_tile_image: return IR(1874, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1900, 36);
    case Extension::kSPV_EXT_split_barrier: return IR(1936, 22);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1958, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1985, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(2016, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(2037, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(2059, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(2101, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(2146, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(2185, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(2215, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(2241, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(2266, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(2291, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(2314, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(2358, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(2384, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(2412, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(2435, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(2470, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(2501, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(2535, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(2562, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(2610, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(2641, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(2670, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(2701, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(2734, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(2753, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(2781, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(2809, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(2852, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2890, 26);
    case Extension::kSPV_INTEL_int4: return IR(2916, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2931, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2950, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2978, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(3004, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(3024, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(3056, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(3084, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(3109, 33);
    case Extension::kSPV_INTEL_optnone: return IR(3142, 18);
    case Extension::kSPV_INTEL_predicated_io: return IR(3160, 24);
    case Extension::kSPV_INTEL_rounded_divide_sqrt: return IR(3184, 30);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(3214, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(3240, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(3276, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3300, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3335, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3381, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3401, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3425, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3461, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3496, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3533, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3563, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3595, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3620, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3642, 21);
    case Extension::kSPV_KHR_abort: return IR(3663, 14);
    case Extension::kSPV_KHR_bfloat16: return IR(3677, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3694, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3719, 35);
    case Extension::kSPV_KHR_constant_data: return IR(3754, 22);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3776, 27);
    case Extension::kSPV_KHR_device_group: return IR(3803, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3824, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3846, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3869, 24);
    case Extension::kSPV_KHR_fma: return IR(3893, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3905, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3941, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3971, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3999, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(4020, 30);
    case Extension::kSPV_KHR_multiview: return IR(4050, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(4068, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(4103, 26);
    case Extension::kSPV_KHR_opacity_micromap: return IR(4129, 25);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(4154, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(4186, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(4208, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4236, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4257, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4279, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4297, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4317, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4352, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4389, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4423, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4445, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4466, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4497, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4534, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4558, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4596, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4618, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4647, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4682, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4707, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4733, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4761, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4802, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4840, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4864, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4902, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4936, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4962, 27);
    case Extension::kSPV_NV_cooperative_matrix_decode_vector: return IR(4989, 40);
    case Extension::kSPV_NV_cooperative_vector: return IR(5029, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(5055, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(5084, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(5119, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(5154, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(5182, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(5201, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(5227, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(5252, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5271, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5302, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5339, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5372, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5402, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5435, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5461, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5496, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5516, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5545, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5570, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5593, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5632, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5658, 27);
    case Extension::kSPV_QCOM_image_processing3: return IR(5685, 27);
    case Extension::kSPV_QCOM_multiple_wait_queues: return IR(5712, 30);
    case Extension::kSPV_QCOM_tile_shading: return IR(5742, 22);
    case Extension::kSPV_SAMSUNG_intrinsic: return IR(5764, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5786, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5824, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,202> kExtensionNames{{
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
    {IR(1020, 21), static_cast<uint32_t>(kSPV_AMD_weak_linkage)},
    {IR(1041, 35), static_cast<uint32_t>(kSPV_ARM_cooperative_matrix_layouts)},
    {IR(1076, 22), static_cast<uint32_t>(kSPV_ARM_core_builtins)},
    {IR(1098, 14), static_cast<uint32_t>(kSPV_ARM_graph)},
    {IR(1112, 16), static_cast<uint32_t>(kSPV_ARM_tensors)},
    {IR(1128, 25), static_cast<uint32_t>(kSPV_EXT_arithmetic_fence)},
    {IR(1153, 40), static_cast<uint32_t>(kSPV_EXT_cooperative_matrix_maintenance1)},
    {IR(1193, 36), static_cast<uint32_t>(kSPV_EXT_demote_to_helper_invocation)},
    {IR(1229, 24), static_cast<uint32_t>(kSPV_EXT_descriptor_heap)},
    {IR(1253, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(1281, 15), static_cast<uint32_t>(kSPV_EXT_float8)},
    {IR(1296, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(1327, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(1363, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(1397, 20), static_cast<uint32_t>(kSPV_EXT_long_vector)},
    {IR(1417, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(1437, 31), static_cast<uint32_t>(kSPV_EXT_ocp_microscaling_types)},
    {IR(1468, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(1493, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(1509, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(1541, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(1585, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(1615, 30), static_cast<uint32_t>(kSPV_EXT_shader_64bit_indexing)},
    {IR(1645, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(1679, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(1711, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(1747, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(1774, 34), static_cast<uint32_t>(kSPV_EXT_shader_invocation_reorder)},
    {IR(1808, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(1838, 36), static_cast<uint32_t>(kSPV_EXT_shader_subgroup_partitioned)},
    {IR(1874, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1900, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1936, 22), static_cast<uint32_t>(kSPV_EXT_split_barrier)},
    {IR(1958, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1985, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(2016, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(2037, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(2059, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(2101, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(2146, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(2185, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(2215, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(2241, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(2266, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(2291, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(2314, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(2358, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(2384, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(2412, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(2435, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(2470, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(2501, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(2535, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(2562, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(2610, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(2641, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(2670, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(2701, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(2734, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(2753, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(2781, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(2809, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(2852, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2890, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2916, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2931, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2950, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2978, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(3004, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(3024, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(3056, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(3084, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(3109, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(3142, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(3160, 24), static_cast<uint32_t>(kSPV_INTEL_predicated_io)},
    {IR(3184, 30), static_cast<uint32_t>(kSPV_INTEL_rounded_divide_sqrt)},
    {IR(3214, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(3240, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(3276, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3300, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3335, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3381, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3401, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3425, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3461, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3496, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3533, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3563, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3595, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3620, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3642, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3663, 14), static_cast<uint32_t>(kSPV_KHR_abort)},
    {IR(3677, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3694, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3719, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3754, 22), static_cast<uint32_t>(kSPV_KHR_constant_data)},
    {IR(3776, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3803, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3824, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3846, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3869, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3893, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3905, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3941, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3971, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3999, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(4020, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(4050, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(4068, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(4103, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(4129, 25), static_cast<uint32_t>(kSPV_KHR_opacity_micromap)},
    {IR(4154, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(4186, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(4208, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4236, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4257, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4279, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4297, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4317, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4352, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4389, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4423, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4445, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4466, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4497, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4534, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4558, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4596, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4618, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4647, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4682, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4707, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4733, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4761, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4802, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4840, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4864, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4902, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4936, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4962, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(4989, 40), static_cast<uint32_t>(kSPV_NV_cooperative_matrix_decode_vector)},
    {IR(5029, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(5055, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(5084, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(5119, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(5154, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(5182, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(5201, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(5227, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(5252, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5271, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5302, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5339, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5372, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5402, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5435, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5461, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5496, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5516, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5545, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5570, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5593, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5632, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5658, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5685, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing3)},
    {IR(5712, 30), static_cast<uint32_t>(kSPV_QCOM_multiple_wait_queues)},
    {IR(5742, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5764, 22), static_cast<uint32_t>(kSPV_SAMSUNG_intrinsic)},
    {IR(5786, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5824, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1245> kOperandsByValue{{
  {0,IR(0, 0),IR(5858, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5874, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5884, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5894, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5902, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5923, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(5934, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(6016, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(6032, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(6042, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(6055, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(6068, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(6077, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(6088, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(6178, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(6191, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(6237, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(6285, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(6300, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(6315, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(6325, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(6339, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6349, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6360, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6372, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6399, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6414, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6425, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6435, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6452, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6466, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6480, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6492, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6510, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6529, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6550, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6558, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6569, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6591, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6604, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6619, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6666, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6682, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6695, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6716, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6727, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6753, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6765, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6779, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6805, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6818, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6831, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6841, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6854, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6909, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(6942, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(6975, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(7008, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(7041, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(7067, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(7080, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(7090, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(7137, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(7161, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(7181, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(7196, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(7227, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(7245, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(7263, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(7283, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(7311, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(7337, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7356, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7383, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7408, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7430, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7465, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7512, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7528, 15)/* SamplerHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7561, 16)/* ResourceHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7577, 15)/* ViewportMaskNV */,IR(0, 0),IR(45, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7613, 20)/* SecondaryPositionNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7652, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7676, 18)/* PositionPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7714, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7736, 16)/* FullyCoveredEXT */,IR(0, 0),IR(50, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7776, 12)/* TaskCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7788, 17)/* PrimitiveCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7805, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7824, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7846, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7868, 15)/* LayerPerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7883, 16)/* MeshViewCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7899, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7917, 13)/* BaryCoordKHR */,IR(6, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(7965, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(8004, 12)/* FragSizeEXT */,IR(8, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(8050, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(8095, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(8120, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(8144, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(8172, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(8189, 12)/* LaunchIdKHR */,IR(10, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(8212, 14)/* LaunchSizeKHR */,IR(11, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(8239, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(8274, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(8315, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8352, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8395, 11)/* RayTminKHR */,IR(16, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8416, 11)/* RayTmaxKHR */,IR(17, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8437, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8482, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8515, 17)/* WorldToObjectKHR */,IR(20, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8548, 7)/* HitTNV */,IR(0, 0),IR(57, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8555, 11)/* HitKindKHR */,IR(21, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8576, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(58, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8616, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8673, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8740, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8777, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8816, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(61, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8836, 14)/* HitIsSphereNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8878, 11)/* HitIsLSSNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(8928, 20)/* HitSpherePositionNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(8948, 13)/* WarpsPerSMNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(8980, 10)/* SMCountNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(8990, 9)/* WarpIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(8999, 7)/* SMIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(9006, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(9024, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(9058, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(9091, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(9109, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(9123, 12)/* ClusterIDNV */,IR(0, 0),IR(65, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(9176, 12)/* CullMaskKHR */,IR(0, 0),IR(66, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(9203, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(6025, 7)/* Shader */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(6100, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(6109, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5913, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(9210, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5867, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(9218, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(9227, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(9241, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(9249, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(9257, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(9263, 13)/* Int64Atomics */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(9276, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(9287, 15)/* ImageReadWrite */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(9302, 12)/* ImageMipmap */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(9314, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(9320, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(9327, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(9341, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9356, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9370, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9376, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9398, 18)/* GeometryPointSize */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9416, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9436, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9460, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9494, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9527, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9561, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(6042, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(6055, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9594, 15)/* ImageCubeArray */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6381, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9626, 10)/* ImageRect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9636, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9648, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9663, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9668, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9684, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9700, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9707, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9717, 8)/* Image1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9609, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9725, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9739, 12)/* ImageBuffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9751, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9764, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9792, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9803, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9821, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9843, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9861, 16)/* GeometryStreams */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9877, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(9907, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(6251, 14)/* MultiViewport */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(9938, 17)/* SubgroupDispatch */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(9955, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(9968, 12)/* PipeStorage */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6632, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(9980, 20)/* GroupNonUniformVote */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(10000, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6887, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(10026, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(10049, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(10080, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(10105, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(6197, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(6265, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(10125, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6789, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(10143, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(10171, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(10199, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(10229, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(10240, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(10277, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(10317, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(10326, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10354, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10364, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(78, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4228,IR(0, 0),IR(10412, 10)/* Float6EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4229,IR(0, 0),IR(10422, 10)/* Float4EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4230,IR(0, 0),IR(10432, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4231,IR(0, 0),IR(10454, 10)/* MXInt8EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4232,IR(0, 0),IR(10464, 18)/* BitcastExtractEXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(7114, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6648, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(7052, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10482, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10515, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(80, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10558, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(80, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10602, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10618, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10671, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10723, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10745, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(7149, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(7171, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10766, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10796, 17)/* VariablePointers */,IR(0, 0),IR(82, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10813, 17)/* AtomicStorageOps */,IR(0, 0),IR(83, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10830, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10858, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10882, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(84, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(10916, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(10937, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(10952, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(10970, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(10995, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(11011, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(11027, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(11050, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(11062, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(11081, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(85, 2),IR(51, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(6135, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(11113, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(11139, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(11160, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(7211, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(11182, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(87, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(11214, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4539,IR(0, 0),IR(11237, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4543,IR(0, 0),IR(11260, 22)/* ImageGatherLinearQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4544,IR(0, 0),IR(11282, 29)/* ImageGatherExtendedModesQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(11311, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(11327, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(11349, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7448, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(11365, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(11386, 14)/* Int64ImageEXT */,IR(0, 0),IR(68, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(11400, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7494, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(11415, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(11430, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(11444, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(88, 2),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(11471, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(11487, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(90, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11509, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(91, 2),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5120,IR(0, 0),IR(11538, 9)/* AbortKHR */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7543, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(93, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5146,IR(0, 0),IR(11547, 16)/* ConstantDataKHR */,IR(0, 0),IR(1, 0),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11563, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5181,IR(0, 0),IR(11579, 15)/* WeakLinkageAMD */,IR(0, 0),IR(94, 1),IR(73, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11594, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11623, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(70, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(6209, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(95, 1),IR(76, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7592, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(96, 1),IR(78, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7633, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(97, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7694, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7752, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(6149, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11678, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(6163, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(7942, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11717, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(8031, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11792, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(11851, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(11888, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(11937, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(98, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(12012, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(74, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(12093, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(99, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(12174, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(12251, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(12326, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(12403, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(12478, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(101, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12559, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(103, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12646, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(105, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8646, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(6122, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8593, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12733, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12772, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(5985, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(12867, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(12930, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(12955, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(12975, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(13008, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(8961, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(13046, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(13078, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(90, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(13131, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(13154, 29)/* RayTracingOpacityMicromapKHR */,IR(47, 1),IR(3, 1),IR(92, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(13212, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(61, 1),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(13238, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(61, 1),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(13265, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(13283, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(13308, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(13328, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8707, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(61, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(13350, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8850, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8889, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(13368, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(13388, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(13402, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5429,IR(0, 0),IR(13425, 32)/* CooperativeMatrixConversionsEXT */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(13457, 31)/* CooperativeMatrixReductionsEXT */,IR(48, 1),IR(1, 0),IR(104, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(13518, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(13549, 41)/* CooperativeMatrixPerElementOperationsEXT */,IR(49, 1),IR(1, 0),IR(104, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(13630, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13666, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13696, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(9135, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(61, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5438,IR(0, 0),IR(13724, 34)/* CooperativeMatrixGetCoordinateEXT */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13758, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5447,IR(0, 0),IR(13777, 32)/* CooperativeMatrixDecodeVectorNV */,IR(0, 0),IR(107, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13809, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(13830, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(13857, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(13883, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(13914, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(13935, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(13958, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(13981, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(14003, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(14027, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(14036, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(14059, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(14082, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(14105, 19)/* VectorComputeINTEL */,IR(0, 0),IR(108, 1),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(14124, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(14139, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(14155, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(14188, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(14226, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(14265, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(14290, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(14316, 27)/* FPGAMemoryAttributesALTERA */,IR(50, 1),IR(1, 0),IR(120, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(14369, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(14389, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(51, 1),IR(1, 0),IR(123, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(14454, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(52, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(14529, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(14559, 23)/* FPGALoopControlsALTERA */,IR(53, 1),IR(1, 0),IR(128, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(14604, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(14626, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(14652, 25)/* FPGAMemoryAccessesALTERA */,IR(54, 1),IR(1, 0),IR(131, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(14701, 28)/* FPGAClusterAttributesALTERA */,IR(55, 1),IR(1, 0),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14756, 15)/* LoopFuseALTERA */,IR(56, 1),IR(1, 0),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14785, 21)/* FPGADSPControlALTERA */,IR(57, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(14826, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(14852, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(58, 1),IR(1, 0),IR(140, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(14933, 25)/* FPGABufferLocationALTERA */,IR(59, 1),IR(1, 0),IR(142, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(14982, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(60, 1),IR(1, 0),IR(144, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(15051, 24)/* USMStorageClassesALTERA */,IR(61, 1),IR(1, 0),IR(146, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(15098, 30)/* RuntimeAlignedAttributeALTERA */,IR(62, 1),IR(1, 0),IR(148, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(15157, 14)/* IOPipesALTERA */,IR(63, 1),IR(1, 0),IR(150, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(15184, 20)/* BlockingPipesALTERA */,IR(64, 1),IR(1, 0),IR(152, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(15223, 14)/* FPGARegALTERA */,IR(65, 1),IR(1, 0),IR(154, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(15250, 19)/* DotProductInputAll */,IR(66, 1),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(15291, 22)/* DotProductInput4x8Bit */,IR(67, 1),IR(109, 1),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(15338, 28)/* DotProductInput4x8BitPacked */,IR(68, 1),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(15397, 11)/* DotProduct */,IR(69, 1),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(9188, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10391, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(157, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(15422, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(158, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(15446, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(15462, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(77, 1),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(15487, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(15502, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6032,IR(0, 0),IR(15509, 42)/* RayTracingOpacityMicromapExecutionModeKHR */,IR(0, 0),IR(3, 1),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(15551, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(164, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(15571, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(164, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(15591, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(15611, 11)/* OptNoneEXT */,IR(70, 1),IR(1, 0),IR(166, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(15635, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(168, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(15655, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(169, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(15676, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(170, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(15700, 16)/* SplitBarrierEXT */,IR(71, 1),IR(1, 0),IR(171, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(15734, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(173, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(15753, 30)/* FPGAClusterAttributesV2ALTERA */,IR(72, 1),IR(110, 1),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15812, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(111, 1),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(15840, 19)/* TaskSequenceALTERA */,IR(73, 1),IR(1, 0),IR(174, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(15877, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(176, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(15893, 25)/* FPGALatencyControlALTERA */,IR(74, 1),IR(1, 0),IR(177, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(15942, 29)/* FPGAArgumentInterfacesALTERA */,IR(75, 1),IR(1, 0),IR(179, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(15999, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(181, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(16029, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(76, 1),IR(1, 0),IR(182, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(16100, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(184, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(16128, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(16151, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(112, 1),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(16181, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(112, 1),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(16211, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(186, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(16249, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(16277, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(113, 2),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(16309, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(16330, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(115, 1),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6257,IR(0, 0),IR(16352, 18)/* PredicatedIOINTEL */,IR(0, 0),IR(1, 0),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6265,IR(0, 0),IR(16370, 23)/* RoundedDivideSqrtINTEL */,IR(0, 0),IR(1, 0),IR(190, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(16393, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(191, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(16419, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(16446, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(16471, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(16490, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(195, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(16510, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(196, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(16530, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(116, 1),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(16563, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(116, 1),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(16596, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(90, 1),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(16623, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(117, 1),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7041,IR(0, 0),IR(16655, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(1, 0),IR(198, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(16672, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16684, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16692, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16700, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16706, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16713, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16724, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16733, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16746, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16752, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16762, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16768, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16783, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16808, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16824, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16838, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16851, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16863, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16880, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16892, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16907, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16920, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16935, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16953, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16970, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16991, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17011, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17031, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17047, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17063, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17083, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17107, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17113, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17118, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(17157, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(17168, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17177, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17182, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17189, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(17200, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(17207, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17216, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17226, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17239, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17252, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17263, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(17273, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(17283, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(17293, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(17306, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(17320, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(17334, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(17348, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(17363, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(17379, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(17395, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(17411, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(17430, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(17451, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(17463, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(17475, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17487, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(17502, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(17527, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(17555, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17563, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(17590, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(17617, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(17644, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(17676, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17702, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(17706, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(17713, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(17717, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(17728, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(17739, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(17760, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17771, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(17785, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(17806, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16672, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16684, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16692, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16700, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16706, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16713, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16724, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(16733, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16746, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16752, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16762, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16808, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16824, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16838, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16851, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16863, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16880, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16892, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16907, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16920, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16935, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16953, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16970, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16991, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17011, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17031, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17107, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(17113, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(17118, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(17157, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(17168, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(17177, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(17182, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(17189, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(17200, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(17216, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17226, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17239, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17824, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(17841, 7)/* SpecId */,IR(0, 0),IR(119, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(17848, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(17854, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(17866, 9)/* RowMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(17875, 9)/* ColMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(17884, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(17896, 13)/* MatrixStride */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(17909, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(17920, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(17931, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(17965, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(17973, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(17987, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(17992, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(17998, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(18007, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(18014, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(18024, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(18033, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(18041, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(18050, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(18059, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(18068, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(18080, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(18092, 8)/* Uniform */,IR(0, 0),IR(121, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(18126, 10)/* UniformId */,IR(0, 0),IR(121, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(18136, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(18156, 7)/* Stream */,IR(0, 0),IR(123, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(18163, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(18172, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(18182, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(18188, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(18196, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(18210, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(18217, 10)/* XfbBuffer */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(18227, 10)/* XfbStride */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(18283, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(18331, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(18381, 15)/* FPFastMathMode */,IR(0, 0),IR(125, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(18458, 18)/* LinkageAttributes */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(18476, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(18490, 21)/* InputAttachmentIndex */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(18511, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(18521, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(18555, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(18567, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(18583, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(18627, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(199, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(18640, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(199, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(18655, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(18673, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(18695, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(18717, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(18735, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(18767, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(18787, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(18810, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(18830, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(18855, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(18882, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(18907, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(18935, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(18952, 12)/* OffsetIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5145,IR(0, 0),IR(18964, 14)/* UTFEncodedKHR */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(18978, 19)/* OverrideCoverageNV */,IR(0, 0),IR(128, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(18997, 14)/* PassthroughNV */,IR(0, 0),IR(129, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(19011, 19)/* ViewportRelativeNV */,IR(0, 0),IR(97, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(19030, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(19058, 16)/* PerPrimitiveEXT */,IR(77, 1),IR(130, 2),IR(200, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(19089, 10)/* PerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(19099, 10)/* PerTaskNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(19109, 13)/* PerVertexKHR */,IR(78, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(19134, 11)/* NonUniform */,IR(79, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(19159, 16)/* RestrictPointer */,IR(80, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(19194, 15)/* AliasedPointer */,IR(81, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(19227, 15)/* MemberOffsetNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(19242, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(19272, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(19303, 7)/* BankNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(19310, 18)/* BindlessSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(19328, 16)/* BindlessImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(19344, 15)/* BoundSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(19359, 13)/* BoundImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5440,IR(0, 0),IR(19372, 30)/* CooperativeMatrixTransposeEXT */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(19402, 14)/* SIMTCallINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(19416, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(138, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(19442, 13)/* ClobberINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(19455, 17)/* SideEffectsINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(19472, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(19499, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(19520, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(19547, 15)/* StackCallINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(19562, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(19588, 14)/* CounterBuffer */,IR(82, 1),IR(1, 0),IR(202, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(19626, 13)/* UserSemantic */,IR(83, 1),IR(1, 0),IR(202, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(19658, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(203, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(19673, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(19730, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(19754, 15)/* RegisterALTERA */,IR(84, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(19783, 13)/* MemoryALTERA */,IR(85, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(19808, 15)/* NumbanksALTERA */,IR(86, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(19837, 16)/* BankwidthALTERA */,IR(87, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(19868, 23)/* MaxPrivateCopiesALTERA */,IR(88, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(19913, 17)/* SinglepumpALTERA */,IR(89, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(19946, 17)/* DoublepumpALTERA */,IR(90, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(19979, 20)/* MaxReplicatesALTERA */,IR(91, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(20018, 21)/* SimpleDualPortALTERA */,IR(92, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(20059, 12)/* MergeALTERA */,IR(93, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(20124, 15)/* BankBitsALTERA */,IR(94, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(20153, 21)/* ForcePow2DepthALTERA */,IR(95, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(20194, 17)/* StridesizeALTERA */,IR(96, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(20227, 15)/* WordsizeALTERA */,IR(97, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(20256, 19)/* TrueDualPortALTERA */,IR(98, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(20293, 20)/* BurstCoalesceALTERA */,IR(99, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(20332, 16)/* CacheSizeALTERA */,IR(100, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(20363, 29)/* DontStaticallyCoalesceALTERA */,IR(101, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(20420, 15)/* PrefetchALTERA */,IR(102, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(20449, 18)/* StallEnableALTERA */,IR(103, 1),IR(110, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(20484, 26)/* FuseLoopsInFunctionALTERA */,IR(104, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(20535, 20)/* MathOpDSPModeALTERA */,IR(105, 1),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(20574, 16)/* AliasScopeINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(20590, 13)/* NoAliasINTEL */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(20603, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(20652, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(20693, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(20734, 21)/* BufferLocationALTERA */,IR(109, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(20775, 20)/* IOPipeStorageALTERA */,IR(110, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(20848, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(20879, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(20904, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(20939, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(20957, 16)/* StallFreeALTERA */,IR(111, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(21019, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(21045, 26)/* LatencyControlLabelALTERA */,IR(112, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(21096, 31)/* LatencyControlConstraintALTERA */,IR(113, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(21157, 28)/* ConduitKernelArgumentALTERA */,IR(114, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(21212, 32)/* RegisterMapKernelArgumentALTERA */,IR(115, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(21275, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(116, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(21342, 31)/* MMHostInterfaceDataWidthALTERA */,IR(117, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(21403, 29)/* MMHostInterfaceLatencyALTERA */,IR(118, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(21494, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(119, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(21563, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(120, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(21622, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(121, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(21687, 27)/* StableKernelArgumentALTERA */,IR(122, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(21779, 16)/* HostAccessINTEL */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(21842, 15)/* InitModeALTERA */,IR(123, 1),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(21871, 29)/* ImplementInRegisterMapALTERA */,IR(124, 1),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(21928, 17)/* ConditionalINTEL */,IR(0, 0),IR(115, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(21981, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(22040, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7040,IR(0, 1),IR(16655, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(22063, 3)/* 1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(22066, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(22069, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(22072, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(22077, 5)/* Rect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(22082, 7)/* Buffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(22089, 12)/* SubpassData */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(22101, 17)/* TileImageDataEXT */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(22118, 12)/* Invocations */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(22130, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(22143, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(22165, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(22186, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(22200, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(22215, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(22234, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(22250, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(22266, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(22285, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(22295, 4)/* Xfb */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(22299, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(22314, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(22327, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(22337, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(22352, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(22362, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(22376, 12)/* InputPoints */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(22388, 11)/* InputLines */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(22399, 20)/* InputLinesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(22419, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(22429, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(22453, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(22459, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(22468, 15)/* OutputVertices */,IR(0, 0),IR(158, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(22483, 13)/* OutputPoints */,IR(0, 0),IR(162, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(22496, 16)/* OutputLineStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(22512, 20)/* OutputTriangleStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(22532, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(22544, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(22559, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(22571, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6619, 13)/* SubgroupSize */,IR(0, 0),IR(165, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(22581, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(165, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(22603, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(165, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(22627, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(22639, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(22655, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(22689, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(166, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(22723, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(167, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(22759, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(204, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(22789, 18)/* PostDepthCoverage */,IR(0, 0),IR(168, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(10937, 15)/* DenormPreserve */,IR(0, 0),IR(169, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(10952, 18)/* DenormFlushToZero */,IR(0, 0),IR(170, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(10970, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(171, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(10995, 16)/* RoundingModeRTE */,IR(0, 0),IR(172, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(11011, 16)/* RoundingModeRTZ */,IR(0, 0),IR(173, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(22807, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(22841, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(22861, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(205, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(22890, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(22913, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(22928, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(22943, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(22964, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7512, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(22988, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(23009, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(23037, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(23063, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(23086, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(23113, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(23138, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(206, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(23160, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(174, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(23179, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(174, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(23199, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(23219, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(23239, 15)/* OutputLinesEXT */,IR(125, 1),IR(130, 2),IR(200, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(23268, 20)/* OutputPrimitivesEXT */,IR(126, 1),IR(130, 2),IR(200, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(23307, 24)/* DerivativeGroupQuadsKHR */,IR(127, 1),IR(176, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(23354, 25)/* DerivativeGroupLinearKHR */,IR(128, 1),IR(177, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(23403, 19)/* OutputTrianglesEXT */,IR(129, 1),IR(130, 2),IR(200, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(23440, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(178, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(23465, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(178, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(23492, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(179, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(23518, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(179, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(23546, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(180, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(23577, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(180, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(13402, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(23610, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(23637, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(23658, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(23679, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(23705, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(182, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(23732, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(183, 1),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(23754, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(183, 1),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(23770, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(183, 1),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(23790, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(111, 1),IR(130, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(23812, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(23840, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(208, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(23864, 18)/* FPFastMathDefault */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6031,IR(10, 1),IR(23882, 21)/* OpacityMicromapIdKHR */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(23903, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(23927, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(23953, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(23976, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(23998, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(24073, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(24100, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(24107, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(24127, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(6100, 9)/* Geometry */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(17207, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(24150, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5867, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(24160, 7)/* TaskNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(24167, 7)/* MeshNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(24174, 17)/* RayGenerationKHR */,IR(130, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(24207, 16)/* IntersectionKHR */,IR(131, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(24238, 10)/* AnyHitKHR */,IR(132, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(24257, 14)/* ClosestHitKHR */,IR(133, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(24284, 8)/* MissKHR */,IR(134, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(24299, 12)/* CallableKHR */,IR(135, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(24322, 8)/* TaskEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(24330, 8)/* MeshEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(24338, 9)/* Preserve */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(24347, 12)/* FlushToZero */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(24359, 12)/* BFloat16KHR */,IR(0, 0),IR(90, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(24371, 14)/* Float8E4M3EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(24385, 14)/* Float8E5M2EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4223,IR(0, 0),IR(24399, 14)/* Float6E2M3EXT */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4224,IR(0, 0),IR(24413, 14)/* Float6E3M2EXT */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4225,IR(0, 0),IR(24427, 14)/* Float4E2M1EXT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4226,IR(0, 0),IR(10432, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4227,IR(0, 0),IR(10454, 10)/* MXInt8EXT */,IR(0, 0),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(24441, 5)/* IEEE */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(24446, 4)/* ALT */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(24450, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(24457, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(24464, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(24468, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(24479, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(24484, 14)/* AllowContract */,IR(136, 1),IR(192, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(24521, 13)/* AllowReassoc */,IR(137, 1),IR(192, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(24552, 15)/* AllowTransform */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(24567, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24571, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(24575, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(24579, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24583, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(24599, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(24615, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(24633, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(24651, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(24658, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(24669, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(24674, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(15611, 11)/* OptNoneEXT */,IR(70, 1),IR(194, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(24680, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(24685, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(24690, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(24696, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(24701, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(24709, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(24719, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(24727, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(24739, 21)/* RuntimeAlignedALTERA */,IR(138, 1),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(24780, 14)/* Gather4x1QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {1,IR(0, 0),IR(24794, 12)/* GatherDQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {2,IR(0, 0),IR(24806, 13)/* GatherH2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {3,IR(0, 0),IR(24819, 13)/* GatherV2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {0,IR(0, 0),IR(24832, 7)/* Reduce */,IR(0, 0),IR(196, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(24839, 14)/* InclusiveScan */,IR(0, 0),IR(196, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(24853, 14)/* ExclusiveScan */,IR(0, 0),IR(196, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(24867, 16)/* ClusteredReduce */,IR(0, 0),IR(199, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(24883, 21)/* PartitionedReduceEXT */,IR(139, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(24924, 28)/* PartitionedInclusiveScanEXT */,IR(140, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(24979, 28)/* PartitionedExclusiveScanEXT */,IR(141, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(25034, 10)/* NoneINTEL */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(25044, 10)/* ReadINTEL */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(25054, 11)/* WriteINTEL */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(25065, 15)/* ReadWriteINTEL */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(25080, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(25085, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(25089, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(25094, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(18210, 7)/* Offset */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(25106, 13)/* ConstOffsets */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(18007, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9700, 7)/* MinLod */,IR(0, 0),IR(202, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(25119, 19)/* MakeTexelAvailable */,IR(142, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(25160, 17)/* MakeTexelVisible */,IR(143, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(25197, 16)/* NonPrivateTexel */,IR(144, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(25232, 14)/* VolatileTexel */,IR(145, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(25263, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(25274, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(25285, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(25297, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(25305, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(25315, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(25326, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(25336, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(25347, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(25361, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(25375, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(25390, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(25401, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(25413, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(25425, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(25438, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(25452, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(25466, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(16700, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(25476, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(25487, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(25504, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(25520, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(25540, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(25560, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(25580, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(25599, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(25618, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(25637, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(25653, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(25669, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(25671, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(25673, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(25676, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(25679, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(25683, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(25688, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(25693, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(25698, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(25708, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(25718, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(25721, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(25725, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(25730, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(25736, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(25749, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(25754, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(25760, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(25766, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(25772, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(25777, 28)/* InitOnDeviceReprogramALTERA */,IR(146, 1),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(25832, 24)/* InitOnDeviceResetALTERA */,IR(147, 1),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(25879, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(25886, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(25897, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(25911, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(25923, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(25936, 7)/* Export */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(25943, 7)/* Import */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(25950, 12)/* LinkOnceODR */,IR(0, 0),IR(94, 1),IR(209, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {3,IR(0, 0),IR(25962, 8)/* WeakAMD */,IR(0, 0),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(25970, 14)/* UncachedINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(25984, 12)/* CachedINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(25996, 15)/* StreamingINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(26011, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(26036, 17)/* ConstCachedINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(26053, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(26060, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(26071, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(26090, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(26107, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(26121, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(26135, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(26153, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(26163, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(20603, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(20652, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(26176, 22)/* DependencyArrayALTERA */,IR(148, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(20693, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(26219, 19)/* LoopCoalesceALTERA */,IR(149, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(26256, 22)/* MaxInterleavingALTERA */,IR(150, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(26299, 27)/* SpeculatedIterationsALTERA */,IR(151, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(26352, 15)/* NoFusionALTERA */,IR(152, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(26381, 16)/* LoopCountALTERA */,IR(153, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(26412, 27)/* MaxReinvocationDelayALTERA */,IR(154, 1),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {268435456,IR(0, 1),IR(11237, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(26465, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(26494, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(26523, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(26544, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(26570, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(26593, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(26616, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(26639, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(26662, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(26679, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(26696, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(26722, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(26748, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(26775, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(18041, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(26802, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(25285, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(26810, 21)/* MakePointerAvailable */,IR(155, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(26855, 19)/* MakePointerVisible */,IR(156, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(26896, 18)/* NonPrivatePointer */,IR(157, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(26935, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(145, 1),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(26955, 17)/* NoAliasINTELMask */,IR(0, 0),IR(145, 1),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(26972, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(26979, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(26987, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(26994, 7)/* Vulkan */,IR(158, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(27011, 8)/* Relaxed */,IR(159, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(27019, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(27027, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(27035, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(27050, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(27073, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(27087, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(27102, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(27118, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(27139, 20)/* AtomicCounterMemory */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(27159, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(27171, 13)/* OutputMemory */,IR(160, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(27200, 14)/* MakeAvailable */,IR(161, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(27231, 12)/* MakeVisible */,IR(162, 1),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(18041, 9)/* Volatile */,IR(0, 0),IR(203, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(27258, 10)/* AutoINTEL */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(27268, 5)/* WRAP */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(27273, 4)/* SAT */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(27277, 9)/* SAT_ZERO */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(27286, 8)/* SAT_SYM */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(27294, 25)/* PackedVectorFormat4x8Bit */,IR(163, 1),IR(1, 0),IR(156, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(27347, 4)/* TRN */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(27351, 9)/* TRN_ZERO */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(27360, 4)/* RND */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(27364, 9)/* RND_ZERO */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(27373, 8)/* RND_INF */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(27381, 12)/* RND_MIN_INF */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(27393, 9)/* RND_CONV */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(27402, 13)/* RND_CONV_ODD */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(27415, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(27440, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(17555, 8)/* NoneKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(27463, 10)/* OpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(27473, 12)/* NoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(27485, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(27508, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(27532, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(27559, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(27587, 14)/* CullOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(27601, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(27617, 17)/* SkipTrianglesKHR */,IR(164, 1),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(27658, 13)/* SkipAABBsKHR */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(27671, 30)/* ForceOpacityMicromap2StateKHR */,IR(165, 1),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(27731, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27772, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27809, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27846, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(27887, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27929, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(27962, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(27995, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(28007, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(28013, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(28020, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(28035, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(28043, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(28050, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(28058, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(28066, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(28074, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(28079, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(28085, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(28096, 6)/* Rg32f */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(28102, 6)/* Rg16f */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(28108, 13)/* R11fG11fB10f */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(28121, 5)/* R16f */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(28126, 7)/* Rgba16 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(28133, 8)/* Rgb10A2 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(28141, 5)/* Rg16 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(28146, 4)/* Rg8 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(28150, 4)/* R16 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(28154, 3)/* R8 */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(28157, 12)/* Rgba16Snorm */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(28169, 10)/* Rg16Snorm */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(28179, 9)/* Rg8Snorm */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(28188, 9)/* R16Snorm */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(28197, 8)/* R8Snorm */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(28205, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(28213, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(28221, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(28228, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(28233, 6)/* Rg32i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(28239, 6)/* Rg16i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(28245, 5)/* Rg8i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(28250, 5)/* R16i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(28255, 4)/* R8i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(28259, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(28268, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(28277, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(28285, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(28291, 10)/* Rgb10a2ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(28301, 7)/* Rg32ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(28308, 7)/* Rg16ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(28315, 6)/* Rg8ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(28321, 6)/* R16ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(28327, 5)/* R8ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(28332, 6)/* R64ui */,IR(0, 0),IR(212, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(28338, 5)/* R64i */,IR(0, 0),IR(212, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(28343, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(28355, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(28362, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(28372, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(28381, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(28392, 12)/* QueueFamily */,IR(166, 1),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(28419, 14)/* ShaderCallKHR */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28433, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(28441, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28453, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(16672, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16684, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16692, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16700, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16706, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16713, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16724, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16733, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16746, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16752, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16762, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16768, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16783, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16808, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16824, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16838, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16851, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16863, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16880, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16892, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16907, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16920, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16935, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16953, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16970, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16991, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17011, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17031, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17047, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17063, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17083, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(28482, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17107, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17113, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17118, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(17157, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(17168, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17177, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17182, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17189, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(17200, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(17207, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17216, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17226, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17239, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17252, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(28050, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(28508, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(28513, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(28518, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(28527, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(28538, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(28543, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(28558, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(28563, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(28570, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(28575, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(28580, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(28586, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(28590, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {14,IR(0, 0),IR(28595, 5)/* Pred */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {15,IR(0, 0),IR(28600, 9)/* ApilaJai */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(28609, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(28625, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(18092, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(28631, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(28362, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(28638, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(28653, 8)/* Private */,IR(0, 0),IR(213, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(28661, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(28670, 8)/* Generic */,IR(0, 0),IR(215, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(28678, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(28691, 14)/* AtomicCounter */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(28705, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(28711, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(210, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(28725, 13)/* TileImageEXT */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(28738, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(28757, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(28773, 16)/* CallableDataKHR */,IR(167, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(28804, 24)/* IncomingCallableDataKHR */,IR(168, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(28851, 14)/* RayPayloadKHR */,IR(169, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(28878, 16)/* HitAttributeKHR */,IR(170, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(28909, 22)/* IncomingRayPayloadKHR */,IR(171, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(28952, 22)/* ShaderRecordBufferKHR */,IR(172, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(28995, 22)/* PhysicalStorageBuffer */,IR(173, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(29042, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(29063, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(54, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(29087, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(29109, 17)/* CodeSectionINTEL */,IR(0, 0),IR(216, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(29126, 17)/* DeviceOnlyALTERA */,IR(174, 1),IR(217, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(29159, 15)/* HostOnlyALTERA */,IR(175, 1),IR(217, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(25970, 14)/* UncachedINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(29188, 18)/* WriteThroughINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(29206, 15)/* WriteBackINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(25996, 15)/* StreamingINTEL */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(16803, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(29221, 11)/* TensorView */,IR(0, 0),IR(218, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(29232, 11)/* DecodeFunc */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {4,IR(10, 1),IR(29243, 17)/* DecodeVectorFunc */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(29260, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(18050, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(27995, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(28013, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(28020, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(29270, 8)/* NoneARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(29278, 15)/* NontemporalARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(29293, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(29313, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(29337, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(29359, 21)/* NonPrivateElementARM */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(135, 303);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(438, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(446, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(449, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(451, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(470, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(480, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(484, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(499, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(503, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(509, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(512, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(515, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(519, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(527, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(530, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(546, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(555, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(558, 152);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(710, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(718, 99);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(817, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(834, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(836, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(844, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(846, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(855, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(859, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(863, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(869, 9);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(878, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(882, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(889, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(893, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(910, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(936, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(956, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(958, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(961, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(963, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(965, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(969, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(974, 21);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(995, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1010, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1019, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1023, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1038, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1039, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1043, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1044, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1052, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1055, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1067, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1069, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1072, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1074, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1079, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1081, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1123, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1130, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1133, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1134, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1142, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1145, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1147, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1167, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1177, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1181, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1197, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1226, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1230, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1234, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1239, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(893, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1010, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1043, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(503, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(995, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1052, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(836, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1239, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(135, 303);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1425> kOperandNames{{
  {IR(5858, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5884, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5874, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5894, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5902, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5923, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5934, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5958, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7917, 13), 94}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7930, 12), 94}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7263, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7283, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7965, 20), 95}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7985, 19), 95}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7311, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7408, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7337, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7356, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7383, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7067, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7041, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6042, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7824, 22), 89}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9123, 12), 133}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6805, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6779, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6818, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6055, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7846, 22), 90}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9176, 12), 134}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8172, 17), 101}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8576, 17), 115}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7137, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7080, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6569, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6339, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6425, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8050, 23), 97}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8004, 12), 96}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7430, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8016, 15), 96}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6360, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7736, 16), 85}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6510, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6604, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6591, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6558, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6435, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8878, 11), 122}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8836, 14), 121}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9058, 33), 130}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9024, 34), 129}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8555, 11), 114}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8566, 10), 114}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9006, 18), 128}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9109, 14), 132}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8740, 37), 118}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8673, 34), 117}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8928, 20), 123}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9091, 18), 131}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8548, 7), 113}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8616, 30), 116}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8777, 20), 119}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8797, 19), 119}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8437, 23), 110}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8460, 22), 110}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6077, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6765, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6178, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8073, 22), 97}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8189, 12), 102}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8201, 11), 102}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8212, 14), 103}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8226, 13), 103}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6191, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7868, 15), 91}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6492, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6529, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7883, 16), 92}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7899, 18), 93}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6695, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6682, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6452, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8352, 22), 107}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8374, 21), 107}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8315, 19), 106}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8334, 18), 106}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8482, 17), 111}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8499, 16), 111}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6325, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6349, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6032, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6016, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7676, 18), 83}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7788, 17), 87}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6088, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7805, 19), 88}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8120, 24), 99}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8095, 25), 98}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7090, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8144, 28), 100}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8816, 20), 120}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8416, 11), 109}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8427, 10), 109}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8395, 11), 108}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8406, 10), 108}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7465, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7561, 16), 79}, // 107 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8980, 10), 125}, // 108 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8999, 7), 127}, // 109 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6372, 9), 24}, // 110 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6414, 11), 26}, // 111 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6399, 15), 25}, // 112 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7528, 15), 78}, // 113 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7613, 20), 81}, // 114 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7652, 24), 82}, // 115 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7512, 16), 77}, // 116 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7181, 15), 64}, // 117 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6854, 15), 53}, // 118 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6869, 18), 53}, // 119 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6909, 15), 54}, // 120 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6924, 18), 54}, // 121 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6942, 15), 55}, // 122 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6957, 18), 55}, // 123 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6716, 11), 44}, // 124 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6975, 15), 56}, // 125 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6990, 18), 56}, // 126 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6727, 26), 45}, // 127 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7008, 15), 57}, // 128 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7023, 18), 57}, // 129 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6666, 16), 41}, // 130 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6619, 13), 40}, // 131 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7776, 12), 86}, // 132 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6315, 10), 19}, // 133 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6300, 15), 18}, // 134 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6285, 15), 17}, // 135 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7245, 18), 67}, // 136 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7227, 18), 66}, // 137 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7196, 15), 65}, // 138 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6068, 9), 11}, // 139 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6753, 12), 46}, // 140 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7161, 10), 63}, // 141 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6237, 14), 16}, // 142 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7577, 15), 80}, // 143 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7714, 22), 84}, // 144 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6831, 10), 51}, // 145 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8990, 9), 126}, // 146 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6841, 13), 52}, // 147 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8948, 13), 124}, // 148 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6550, 8), 35}, // 149 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6480, 12), 31}, // 150 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6466, 14), 30}, // 151 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8274, 21), 105}, // 152 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8295, 20), 105}, // 153 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8239, 18), 104}, // 154 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8257, 17), 104}, // 155 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8515, 17), 112}, // 156 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8532, 16), 112}, // 157 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(11538, 9), 274}, // 158 AbortKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5913, 10), 139}, // 159 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14982, 35), 381}, // 160 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15017, 34), 381}, // 161 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14454, 38), 369}, // 162 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14492, 37), 369}, // 163 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14389, 33), 368}, // 164 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14422, 32), 368}, // 165 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15734, 19), 407}, // 166 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14027, 9), 354}, // 167 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15635, 20), 403}, // 168 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14082, 23), 357}, // 169 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13328, 22), 326}, // 170 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15551, 20), 399}, // 171 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14036, 23), 355}, // 172 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15571, 20), 400}, // 173 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14059, 23), 356}, // 174 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9356, 14), 155}, // 175 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10813, 17), 236}, // 176 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15676, 24), 405}, // 177 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11509, 29), 273}, // 178 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11487, 22), 272}, // 179 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11471, 16), 271}, // 180 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16510, 20), 432}, // 181 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13265, 18), 323}, // 182 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15446, 16), 394}, // 183 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10464, 18), 220}, // 184 BitcastExtractEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15184, 20), 385}, // 185 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15204, 19), 385}, // 186 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16471, 19), 430}, // 187 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6042, 13), 165}, // 188 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12867, 32), 311}, // 189 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12899, 31), 311}, // 190 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11717, 31), 290}, // 191 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11748, 30), 290}, // 192 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11547, 16), 276}, // 193 ConstantDataKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13666, 30), 339}, // 194 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11182, 32), 255}, // 195 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13425, 32), 334}, // 196 CooperativeMatrixConversionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13518, 31), 336}, // 197 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13777, 32), 344}, // 198 CooperativeMatrixDecodeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13724, 34), 342}, // 199 CooperativeMatrixGetCoordinateEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10391, 21), 392}, // 200 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10326, 28), 213}, // 201 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12955, 20), 313}, // 202 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13549, 41), 337}, // 203 CooperativeMatrixPerElementOperationsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13590, 40), 337}, // 204 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13457, 31), 335}, // 205 CooperativeMatrixReductionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13488, 30), 335}, // 206 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13630, 36), 338}, // 207 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13308, 20), 325}, // 208 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13696, 28), 340}, // 209 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6789, 16), 205}, // 210 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6055, 13), 166}, // 211 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15655, 21), 404}, // 212 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13078, 25), 318}, // 213 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13103, 28), 318}, // 214 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10952, 18), 242}, // 215 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10937, 15), 241}, // 216 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9803, 18), 184}, // 217 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7543, 18), 275}, // 218 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9327, 14), 153}, // 219 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7149, 12), 232}, // 220 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13131, 23), 319}, // 221 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15397, 11), 390}, // 222 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16596, 27), 435}, // 223 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16563, 33), 434}, // 224 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16530, 33), 433}, // 225 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16623, 32), 436}, // 226 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15291, 22), 388}, // 227 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15313, 25), 388}, // 228 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15338, 28), 389}, // 229 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15366, 31), 389}, // 230 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15250, 19), 387}, // 231 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15269, 22), 387}, // 232 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15408, 14), 390}, // 233 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7052, 15), 223}, // 234 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14139, 16), 360}, // 235 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15502, 7), 397}, // 236 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14369, 20), 367}, // 237 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15942, 29), 413}, // 238 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15971, 28), 413}, // 239 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14933, 25), 380}, // 240 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14958, 24), 380}, // 241 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14701, 28), 375}, // 242 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14729, 27), 375}, // 243 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15753, 30), 408}, // 244 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15783, 29), 408}, // 245 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14785, 21), 377}, // 246 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14806, 20), 377}, // 247 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14852, 41), 379}, // 248 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14893, 40), 379}, // 249 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14626, 26), 373}, // 250 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15812, 28), 409}, // 251 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15893, 25), 412}, // 252 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15918, 24), 412}, // 253 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14559, 23), 371}, // 254 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14582, 22), 371}, // 255 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14652, 25), 374}, // 256 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14677, 24), 374}, // 257 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14316, 27), 366}, // 258 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14343, 26), 366}, // 259 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15223, 14), 386}, // 260 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15237, 13), 386}, // 261 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15877, 16), 411}, // 262 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9241, 8), 144}, // 263 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9227, 14), 143}, // 264 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11311, 16), 260}, // 265 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10422, 10), 217}, // 266 Float4EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9249, 8), 145}, // 267 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10412, 10), 216}, // 268 Float6EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10364, 27), 215}, // 269 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10354, 10), 214}, // 270 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10432, 22), 218}, // 271 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15487, 15), 396}, // 272 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13935, 23), 350}, // 273 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7942, 23), 289}, // 274 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11695, 22), 289}, // 275 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8031, 19), 291}, // 276 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7752, 24), 285}, // 277 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11349, 16), 262}, // 278 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13046, 32), 317}, // 279 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12975, 33), 314}, // 280 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13008, 38), 315}, // 281 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7114, 23), 221}, // 282 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14290, 26), 365}, // 283 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13981, 22), 352}, // 284 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16330, 22), 424}, // 285 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9648, 15), 171}, // 286 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6100, 9), 137}, // 287 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9398, 18), 158}, // 288 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11623, 28), 280}, // 289 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9861, 16), 187}, // 290 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16029, 36), 415}, // 291 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16065, 35), 415}, // 292 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15999, 30), 414}, // 293 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10317, 9), 212}, // 294 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6632, 16), 194}, // 295 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10000, 26), 196}, // 296 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6887, 22), 197}, // 297 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10080, 25), 200}, // 298 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11792, 30), 292}, // 299 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11822, 29), 292}, // 300 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10105, 20), 201}, // 301 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15462, 25), 395}, // 302 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10026, 23), 198}, // 303 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10049, 31), 199}, // 304 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9980, 20), 195}, // 305 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16393, 26), 427}, // 306 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9320, 7), 152}, // 307 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15157, 14), 384}, // 308 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15171, 13), 384}, // 309 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9717, 8), 177}, // 310 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9276, 11), 148}, // 311 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9739, 12), 180}, // 312 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9594, 15), 167}, // 313 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11678, 17), 287}, // 314 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11327, 22), 261}, // 315 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9416, 20), 159}, // 316 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11282, 29), 259}, // 317 ImageGatherExtendedModesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11260, 22), 258}, // 318 ImageGatherLinearQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9751, 13), 181}, // 319 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9302, 12), 150}, // 320 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9792, 11), 183}, // 321 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9287, 15), 149}, // 322 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11365, 21), 264}, // 323 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9626, 10), 169}, // 324 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14003, 24), 353}, // 325 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9668, 16), 173}, // 326 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11937, 36), 295}, // 327 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11973, 39), 295}, // 328 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12478, 39), 302}, // 329 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12517, 42), 302}, // 330 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9370, 6), 156}, // 331 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11444, 27), 270}, // 332 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11430, 14), 269}, // 333 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9257, 6), 146}, // 334 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9263, 13), 147}, // 335 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11386, 14), 265}, // 336 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9663, 5), 172}, // 337 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13958, 23), 351}, // 338 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9821, 22), 185}, // 339 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16655, 17), 437}, // 340 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5867, 7), 141}, // 341 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14604, 22), 372}, // 342 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9210, 8), 140}, // 343 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9341, 15), 154}, // 344 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15591, 20), 401}, // 345 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13388, 14), 332}, // 346 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14756, 15), 376}, // 347 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14771, 14), 376}, // 348 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10454, 10), 219}, // 349 MXInt8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16446, 25), 429}, // 350 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9203, 7), 135}, // 351 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14826, 26), 378}, // 352 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6163, 15), 288}, // 353 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6149, 14), 286}, // 354 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9700, 7), 175}, // 355 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7171, 10), 233}, // 356 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6251, 14), 190}, // 357 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11237, 23), 257}, // 358 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9955, 13), 192}, // 359 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15611, 11), 402}, // 360 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15622, 13), 402}, // 361 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7694, 20), 284}, // 362 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5985, 31), 310}, // 363 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12833, 34), 310}, // 364 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9968, 12), 193}, // 365 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9314, 6), 151}, // 366 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11563, 16), 277}, // 367 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16352, 18), 425}, // 368 PredicatedIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13368, 20), 331}, // 369 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11415, 15), 268}, // 370 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13350, 18), 328}, // 371 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9188, 15), 391}, // 372 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11050, 12), 247}, // 373 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13283, 25), 324}, // 374 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11027, 23), 246}, // 375 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9135, 41), 341}, // 376 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8707, 33), 327}, // 377 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6135, 14), 250}, // 378 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8889, 39), 330}, // 379 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8593, 23), 307}, // 380 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6122, 13), 306}, // 381 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13183, 29), 320}, // 382 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15509, 42), 398}, // 383 RayTracingOpacityMicromapExecutionModeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13154, 29), 320}, // 384 RayTracingOpacityMicromapKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8646, 27), 305}, // 385 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12930, 25), 312}, // 386 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8850, 28), 329}, // 387 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11081, 32), 249}, // 388 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16490, 20), 431}, // 389 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15422, 24), 393}, // 390 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13914, 21), 349}, // 391 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16370, 23), 426}, // 392 RoundedDivideSqrtINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10995, 16), 244}, // 393 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11011, 16), 245}, // 394 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15098, 30), 383}, // 395 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15128, 29), 383}, // 396 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11888, 23), 294}, // 397 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11911, 26), 294}, // 398 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11594, 29), 279}, // 399 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10830, 28), 237}, // 400 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6381, 18), 168}, // 401 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9707, 10), 176}, // 402 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9725, 14), 179}, // 403 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9609, 17), 178}, // 404 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9494, 33), 162}, // 405 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12251, 36), 299}, // 406 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12287, 39), 299}, // 407 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9636, 12), 170}, // 408 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6025, 7), 136}, // 409 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13402, 23), 333}, // 410 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11400, 15), 266}, // 411 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7494, 18), 267}, // 412 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13238, 27), 322}, // 413 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13212, 26), 321}, // 414 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6197, 12), 202}, // 415 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11851, 17), 293}, // 416 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11868, 20), 293}, // 417 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8961, 19), 316}, // 418 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7633, 19), 283}, // 419 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6265, 20), 203}, // 420 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6209, 28), 281}, // 421 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11651, 27), 281}, // 422 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7592, 21), 282}, // 423 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11778, 14), 291}, // 424 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10970, 25), 243}, // 425 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9684, 16), 174}, // 426 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16309, 21), 423}, // 427 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15700, 16), 406}, // 428 SplitBarrierEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15716, 18), 406}, // 429 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7448, 17), 263}, // 430 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10618, 25), 228}, // 431 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10858, 24), 238}, // 432 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9527, 34), 163}, // 433 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12326, 37), 300}, // 434 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12363, 40), 300}, // 435 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9561, 33), 164}, // 436 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12403, 36), 301}, // 437 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12439, 39), 301}, // 438 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9764, 28), 182}, // 439 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9436, 24), 160}, // 440 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9877, 30), 188}, // 441 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9907, 31), 189}, // 442 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10745, 21), 231}, // 443 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10723, 22), 230}, // 444 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10916, 21), 240}, // 445 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10240, 37), 210}, // 446 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10277, 40), 211}, // 447 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12093, 39), 297}, // 448 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12132, 42), 297}, // 449 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12646, 42), 304}, // 450 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12688, 45), 304}, // 451 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10706, 17), 229}, // 452 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10643, 28), 228}, // 453 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16128, 23), 417}, // 454 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16151, 30), 418}, // 455 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16181, 30), 419}, // 456 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14226, 39), 363}, // 457 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14155, 33), 361}, // 458 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14188, 38), 362}, // 459 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6648, 18), 222}, // 460 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13830, 27), 346}, // 461 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16100, 28), 416}, // 462 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9938, 17), 191}, // 463 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13857, 26), 347}, // 464 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13883, 31), 348}, // 465 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16211, 38), 420}, // 466 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13809, 21), 345}, // 467 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10602, 16), 227}, // 468 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15840, 19), 410}, // 469 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15859, 18), 410}, // 470 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13758, 19), 343}, // 471 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16419, 27), 428}, // 472 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10229, 11), 209}, // 473 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16249, 28), 421}, // 474 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6109, 13), 138}, // 475 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9376, 22), 157}, // 476 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11214, 23), 256}, // 477 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11160, 22), 253}, // 478 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11139, 21), 252}, // 479 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11113, 26), 251}, // 480 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10143, 28), 206}, // 481 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10171, 28), 207}, // 482 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10199, 30), 208}, // 483 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7211, 16), 254}, // 484 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9843, 18), 186}, // 485 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15051, 24), 382}, // 486 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15075, 23), 382}, // 487 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10671, 35), 229}, // 488 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10882, 34), 239}, // 489 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9460, 34), 161}, // 490 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12174, 37), 298}, // 491 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12211, 40), 298}, // 492 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10125, 18), 204}, // 493 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12012, 39), 296}, // 494 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12051, 42), 296}, // 495 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12559, 42), 303}, // 496 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12601, 45), 303}, // 497 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14529, 30), 370}, // 498 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11062, 19), 248}, // 499 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16277, 32), 422}, // 500 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14265, 25), 364}, // 501 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10796, 17), 235}, // 502 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10766, 30), 234}, // 503 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9218, 9), 142}, // 504 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14124, 15), 359}, // 505 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14105, 19), 358}, // 506 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12733, 18), 308}, // 507 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12772, 29), 309}, // 508 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12801, 32), 309}, // 509 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12751, 21), 308}, // 510 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11579, 15), 278}, // 511 WeakLinkageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10558, 44), 226}, // 512 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10515, 43), 225}, // 513 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10482, 33), 224}, // 514 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16684, 8), 439}, // 515 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16692, 8), 440}, // 516 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16700, 6), 441}, // 517 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16706, 7), 442}, // 518 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16713, 11), 443}, // 519 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16724, 9), 444}, // 520 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16733, 13), 445}, // 521 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16672, 12), 438}, // 522 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16746, 6), 446}, // 523 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16752, 10), 447}, // 524 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16762, 6), 448}, // 525 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16783, 20), 450}, // 526 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16768, 15), 449}, // 527 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16892, 15), 458}, // 528 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16907, 13), 459}, // 529 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16880, 12), 457}, // 530 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16970, 21), 463}, // 531 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16863, 17), 456}, // 532 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17047, 16), 467}, // 533 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16851, 12), 455}, // 534 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17031, 16), 466}, // 535 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16824, 14), 453}, // 536 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16808, 16), 452}, // 537 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16838, 13), 454}, // 538 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16991, 20), 464}, // 539 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16935, 18), 461}, // 540 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16920, 15), 460}, // 541 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17011, 20), 465}, // 542 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16953, 17), 462}, // 543 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17083, 24), 469}, // 544 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17063, 20), 468}, // 545 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16803, 5), 451}, // 546 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17168, 9), 474}, // 547 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17200, 7), 478}, // 548 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17107, 6), 470}, // 549 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17207, 9), 479}, // 550 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17118, 6), 472}, // 551 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17113, 5), 471}, // 552 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17157, 11), 473}, // 553 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17189, 11), 477}, // 554 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17177, 5), 475}, // 555 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17182, 7), 476}, // 556 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17252, 11), 483}, // 557 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17216, 10), 480}, // 558 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17239, 13), 482}, // 559 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17226, 13), 481}, // 560 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17263, 10), 484}, // 561 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17273, 10), 485}, // 562 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17283, 10), 486}, // 563 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17451, 12), 497}, // 564 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17463, 12), 498}, // 565 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17306, 14), 488}, // 566 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17320, 14), 489}, // 567 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17334, 14), 490}, // 568 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17293, 13), 487}, // 569 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17411, 19), 495}, // 570 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17363, 16), 492}, // 571 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17379, 16), 493}, // 572 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17395, 16), 494}, // 573 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17348, 15), 491}, // 574 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17430, 21), 496}, // 575 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17527, 28), 502}, // 576 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17487, 15), 500}, // 577 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17502, 25), 501}, // 578 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17475, 12), 499}, // 579 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17563, 27), 504}, // 580 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17590, 27), 505}, // 581 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17617, 27), 506}, // 582 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17644, 32), 507}, // 583 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17555, 8), 503}, // 584 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17676, 26), 508}, // 585 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17713, 4), 511}, // 586 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17706, 7), 510}, // 587 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17702, 4), 509}, // 588 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17717, 11), 512}, // 589 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17739, 21), 514}, // 590 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17728, 11), 513}, // 591 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17771, 14), 516}, // 592 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17785, 21), 517}, // 593 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17760, 11), 515}, // 594 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17806, 18), 518}, // 595 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16684, 8), 520}, // 596 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16692, 8), 521}, // 597 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16700, 6), 522}, // 598 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16706, 7), 523}, // 599 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16713, 11), 524}, // 600 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16724, 9), 525}, // 601 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16733, 13), 526}, // 602 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16672, 12), 519}, // 603 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16746, 6), 527}, // 604 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16752, 10), 528}, // 605 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16762, 6), 529}, // 606 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16892, 15), 537}, // 607 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16907, 13), 538}, // 608 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16880, 12), 536}, // 609 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16970, 21), 542}, // 610 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16863, 17), 535}, // 611 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16851, 12), 534}, // 612 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17031, 16), 545}, // 613 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16824, 14), 532}, // 614 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16808, 16), 531}, // 615 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16838, 13), 533}, // 616 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16991, 20), 543}, // 617 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16935, 18), 540}, // 618 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16920, 15), 539}, // 619 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17011, 20), 544}, // 620 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16953, 17), 541}, // 621 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16803, 5), 530}, // 622 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17168, 9), 550}, // 623 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17200, 7), 554}, // 624 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17107, 6), 546}, // 625 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17118, 6), 548}, // 626 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17113, 5), 547}, // 627 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17157, 11), 549}, // 628 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17189, 11), 553}, // 629 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17177, 5), 551}, // 630 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17182, 7), 552}, // 631 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17216, 10), 555}, // 632 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17239, 13), 557}, // 633 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17226, 13), 556}, // 634 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(20574, 16), 679}, // 635 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18033, 8), 577}, // 636 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(19194, 15), 633}, // 637 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19209, 18), 633}, // 638 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18511, 10), 601}, // 639 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(18555, 12), 603}, // 640 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(17884, 12), 564}, // 641 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18935, 17), 620}, // 642 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(20124, 15), 667}, // 643 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20139, 14), 667}, // 644 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19303, 7), 637}, // 645 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19837, 16), 660}, // 646 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19853, 15), 660}, // 647 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18188, 8), 590}, // 648 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(19328, 16), 639}, // 649 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19310, 18), 638}, // 650 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17848, 6), 560}, // 651 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(18695, 22), 610}, // 652 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18673, 22), 609}, // 653 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19359, 13), 641}, // 654 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19344, 15), 640}, // 655 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17854, 12), 561}, // 656 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(20734, 21), 684}, // 657 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20755, 20), 684}, // 658 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17965, 8), 569}, // 659 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(20293, 20), 672}, // 660 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20313, 19), 672}, // 661 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17931, 8), 568}, // 662 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(21981, 22), 707}, // 663 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(22040, 23), 708}, // 664 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20332, 16), 673}, // 665 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20348, 15), 673}, // 666 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17998, 9), 573}, // 667 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(19442, 13), 645}, // 668 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18059, 9), 580}, // 669 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(17875, 9), 563}, // 670 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(18172, 10), 588}, // 671 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(21928, 17), 706}, // 672 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21157, 28), 694}, // 673 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21185, 27), 694}, // 674 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18050, 9), 579}, // 675 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(19372, 30), 642}, // 676 CooperativeMatrixTransposeEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19588, 14), 652}, // 677 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18196, 14), 591}, // 678 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(20363, 29), 674}, // 679 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20392, 28), 674}, // 680 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19946, 17), 663}, // 681 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19963, 16), 663}, // 682 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18717, 18), 611}, // 683 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(18381, 15), 597}, // 684 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(21019, 26), 691}, // 685 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18331, 15), 596}, // 686 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17987, 5), 571}, // 687 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(20153, 21), 668}, // 688 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20174, 20), 668}, // 689 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18283, 14), 595}, // 690 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(19499, 21), 648}, // 691 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19730, 24), 656}, // 692 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20848, 31), 686}, // 693 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19673, 26), 655}, // 694 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20484, 26), 677}, // 695 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20510, 25), 677}, // 696 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17920, 11), 567}, // 697 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(17909, 11), 566}, // 698 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(19562, 26), 651}, // 699 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19272, 31), 636}, // 700 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19242, 30), 635}, // 701 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19602, 24), 652}, // 702 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(19639, 19), 653}, // 703 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21779, 16), 703}, // 704 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20775, 20), 685}, // 705 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20795, 19), 685}, // 706 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21871, 29), 705}, // 707 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21900, 28), 705}, // 708 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18182, 6), 589}, // 709 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(21842, 15), 704}, // 710 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21857, 14), 704}, // 711 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20603, 25), 681}, // 712 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20628, 24), 681}, // 713 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18490, 21), 600}, // 714 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(16655, 17), 709}, // 715 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_DECORATION
  {IR(18014, 10), 575}, // 716 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(21096, 31), 693}, // 717 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21127, 30), 693}, // 718 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21045, 26), 692}, // 719 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21071, 25), 692}, // 720 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18458, 18), 598}, // 721 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(18163, 9), 587}, // 722 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(21275, 34), 696}, // 723 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21309, 33), 696}, // 724 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21342, 31), 697}, // 725 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21373, 30), 697}, // 726 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21403, 29), 698}, // 727 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21432, 28), 698}, // 728 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21563, 30), 700}, // 729 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21593, 29), 700}, // 730 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21494, 35), 699}, // 731 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21529, 34), 699}, // 732 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21622, 33), 701}, // 733 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21655, 32), 701}, // 734 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20535, 20), 678}, // 735 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20555, 19), 678}, // 736 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17896, 13), 565}, // 737 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18521, 14), 602}, // 738 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(18567, 16), 604}, // 739 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(20652, 21), 682}, // 740 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20673, 20), 682}, // 741 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19868, 23), 661}, // 742 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19891, 22), 661}, // 743 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19979, 20), 664}, // 744 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19999, 19), 664}, // 745 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20939, 18), 689}, // 746 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19227, 15), 634}, // 747 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19783, 13), 658}, // 748 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19796, 12), 658}, // 749 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20059, 12), 666}, // 750 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20071, 11), 666}, // 751 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20590, 13), 680}, // 752 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18476, 14), 599}, // 753 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(17973, 14), 570}, // 754 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(18627, 13), 606}, // 755 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18640, 15), 607}, // 756 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18767, 20), 613}, // 757 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18735, 32), 612}, // 758 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18080, 12), 582}, // 759 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(19134, 11), 631}, // 760 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(19145, 14), 631}, // 761 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18068, 12), 581}, // 762 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(19808, 15), 659}, // 763 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19823, 14), 659}, // 764 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18210, 7), 592}, // 765 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(18952, 12), 621}, // 766 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18978, 19), 623}, // 767 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18997, 14), 624}, // 768 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17992, 6), 572}, // 769 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(18907, 28), 619}, // 770 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18882, 25), 618}, // 771 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18830, 25), 616}, // 772 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18810, 20), 615}, // 773 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18855, 27), 617}, // 774 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19058, 16), 627}, // 775 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19074, 15), 627}, // 776 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19099, 10), 629}, // 777 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19109, 13), 630}, // 778 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(19122, 12), 630}, // 779 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19089, 10), 628}, // 780 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20693, 21), 683}, // 781 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20714, 20), 683}, // 782 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20420, 15), 675}, // 783 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20435, 14), 675}, // 784 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19416, 26), 644}, // 785 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19754, 15), 657}, // 786 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19769, 14), 657}, // 787 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21212, 32), 695}, // 788 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21244, 31), 695}, // 789 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17824, 17), 558}, // 790 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(18024, 9), 576}, // 791 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(19159, 16), 632}, // 792 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19175, 19), 632}, // 793 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17866, 9), 562}, // 794 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(19402, 14), 643}, // 795 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18007, 7), 574}, // 796 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(18136, 20), 585}, // 797 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(18583, 44), 605}, // 798 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19030, 28), 626}, // 799 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19455, 17), 646}, // 800 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20018, 21), 665}, // 801 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20039, 20), 665}, // 802 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20879, 25), 687}, // 803 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19913, 17), 662}, // 804 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19930, 16), 662}, // 805 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17841, 7), 559}, // 806 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(21687, 27), 702}, // 807 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21714, 26), 702}, // 808 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19547, 15), 650}, // 809 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20449, 18), 676}, // 810 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20467, 17), 676}, // 811 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20957, 16), 690}, // 812 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20973, 15), 690}, // 813 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18156, 7), 586}, // 814 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(20194, 17), 669}, // 815 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20211, 16), 669}, // 816 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18787, 23), 614}, // 817 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(20256, 19), 671}, // 818 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20275, 18), 671}, // 819 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18964, 14), 622}, // 820 UTFEncodedKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18092, 8), 583}, // 821 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18126, 10), 584}, // 822 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(19626, 13), 653}, // 823 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(19658, 15), 654}, // 824 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20904, 35), 688}, // 825 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19520, 27), 649}, // 826 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19472, 27), 647}, // 827 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19011, 19), 625}, // 828 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18041, 9), 578}, // 829 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(18655, 18), 608}, // 830 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(20227, 15), 670}, // 831 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20242, 14), 670}, // 832 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18217, 10), 593}, // 833 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18227, 10), 594}, // 834 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(22063, 3), 710}, // 835 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22066, 3), 711}, // 836 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22069, 3), 712}, // 837 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22082, 7), 715}, // 838 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22072, 5), 713}, // 839 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22077, 5), 714}, // 840 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22089, 12), 716}, // 841 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22101, 17), 717}, // 842 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(23219, 20), 785}, // 843 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22913, 15), 770}, // 844 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22544, 15), 748}, // 845 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10952, 18), 762}, // 846 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10937, 15), 761}, // 847 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22314, 13), 731}, // 848 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22327, 10), 732}, // 849 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22299, 15), 730}, // 850 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22337, 15), 733}, // 851 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23354, 25), 789}, // 852 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23379, 24), 789}, // 853 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23307, 24), 788}, // 854 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23331, 23), 788}, // 855 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22861, 29), 768}, // 856 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22266, 19), 727}, // 857 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23864, 18), 809}, // 858 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22571, 10), 750}, // 859 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23679, 26), 801}, // 860 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23705, 27), 802}, // 861 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22559, 12), 749}, // 862 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22388, 11), 737}, // 863 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22399, 20), 738}, // 864 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22376, 12), 736}, // 865 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22429, 24), 740}, // 866 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22118, 12), 718}, // 867 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22928, 15), 771}, // 868 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22459, 9), 742}, // 869 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22352, 10), 734}, // 870 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22362, 14), 735}, // 871 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22639, 16), 755}, // 872 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22627, 12), 754}, // 873 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22943, 21), 772}, // 874 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22988, 21), 775}, // 875 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23754, 16), 804}, // 876 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23732, 22), 803}, // 877 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23840, 24), 808}, // 878 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23976, 22), 814}, // 879 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23998, 24), 815}, // 880 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23953, 23), 813}, // 881 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24073, 27), 816}, // 882 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23770, 20), 805}, // 883 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22655, 34), 756}, // 884 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22689, 34), 757}, // 885 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22723, 36), 758}, // 886 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22807, 34), 766}, // 887 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23790, 22), 806}, // 888 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23882, 21), 810}, // 889 OpacityMicromapIdKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22250, 16), 726}, // 890 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22234, 16), 725}, // 891 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22496, 16), 745}, // 892 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23239, 15), 786}, // 893 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23254, 14), 786}, // 894 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22483, 13), 744}, // 895 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23268, 20), 787}, // 896 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23288, 19), 787}, // 897 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22512, 20), 746}, // 898 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23403, 19), 790}, // 899 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23422, 18), 790}, // 900 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22468, 15), 743}, // 901 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22215, 19), 724}, // 902 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23440, 25), 791}, // 903 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23465, 27), 792}, // 904 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22285, 10), 728}, // 905 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22789, 18), 760}, // 906 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23160, 19), 782}, // 907 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22453, 6), 741}, // 908 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23927, 26), 812}, // 909 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23179, 20), 783}, // 910 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10995, 16), 764}, // 911 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23658, 21), 800}, // 912 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23637, 21), 799}, // 913 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11011, 16), 765}, // 914 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23492, 26), 793}, // 915 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23518, 28), 794}, // 916 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23812, 28), 807}, // 917 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(13402, 23), 797}, // 918 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7512, 16), 774}, // 919 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23546, 31), 795}, // 920 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23577, 33), 796}, // 921 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23610, 27), 798}, // 922 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23199, 20), 784}, // 923 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10970, 25), 763}, // 924 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22130, 13), 719}, // 925 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22143, 22), 720}, // 926 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22165, 21), 721}, // 927 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22964, 24), 773}, // 928 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23113, 25), 780}, // 929 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23037, 26), 777}, // 930 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23138, 22), 781}, // 931 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23063, 23), 778}, // 932 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22890, 23), 769}, // 933 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23086, 27), 779}, // 934 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23009, 28), 776}, // 935 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23903, 24), 811}, // 936 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6619, 13), 751}, // 937 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22759, 30), 759}, // 938 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22581, 22), 752}, // 939 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22603, 24), 753}, // 940 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22841, 20), 767}, // 941 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22419, 10), 739}, // 942 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22532, 12), 747}, // 943 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22200, 15), 723}, // 944 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22186, 14), 722}, // 945 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22295, 4), 729}, // 946 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24238, 10), 828}, // 947 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24248, 9), 828}, // 948 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24299, 12), 831}, // 949 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24311, 11), 831}, // 950 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24257, 14), 829}, // 951 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24271, 13), 829}, // 952 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(17207, 9), 821}, // 953 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24150, 10), 822}, // 954 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(6100, 9), 820}, // 955 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24207, 16), 827}, // 956 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24223, 15), 827}, // 957 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5867, 7), 823}, // 958 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24330, 8), 833}, // 959 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24167, 7), 825}, // 960 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24284, 8), 830}, // 961 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24292, 7), 830}, // 962 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24174, 17), 826}, // 963 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24191, 16), 826}, // 964 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24322, 8), 832}, // 965 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24160, 7), 824}, // 966 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24107, 20), 818}, // 967 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24127, 23), 819}, // 968 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24100, 7), 817}, // 969 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24347, 12), 835}, // 970 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24338, 9), 834}, // 971 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24359, 12), 836}, // 972 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(24427, 14), 841}, // 973 Float4E2M1EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24399, 14), 839}, // 974 Float6E2M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24413, 14), 840}, // 975 Float6E3M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24371, 14), 837}, // 976 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24385, 14), 838}, // 977 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10432, 22), 842}, // 978 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10454, 10), 843}, // 979 MXInt8EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24446, 4), 845}, // 980 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24441, 5), 844}, // 981 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24484, 14), 852}, // 982 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24498, 23), 852}, // 983 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24521, 13), 853}, // 984 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24534, 18), 853}, // 985 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24468, 11), 850}, // 986 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24552, 15), 854}, // 987 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24479, 5), 851}, // 988 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24464, 4), 849}, // 989 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(16803, 5), 846}, // 990 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24457, 7), 848}, // 991 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24450, 7), 847}, // 992 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24567, 4), 855}, // 993 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24579, 4), 858}, // 994 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24575, 4), 857}, // 995 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24571, 4), 856}, // 996 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24615, 18), 861}, // 997 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24633, 18), 862}, // 998 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24583, 16), 859}, // 999 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24599, 16), 860}, // 1000 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24674, 6), 867}, // 1001 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24658, 11), 865}, // 1002 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24651, 7), 864}, // 1003 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(16803, 5), 863}, // 1004 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15611, 11), 868}, // 1005 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15622, 13), 868}, // 1006 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24669, 5), 866}, // 1007 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24690, 6), 871}, // 1008 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24701, 8), 873}, // 1009 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24709, 10), 874}, // 1010 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24727, 12), 876}, // 1011 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24719, 8), 875}, // 1012 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24739, 21), 877}, // 1013 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24760, 20), 877}, // 1014 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24685, 5), 870}, // 1015 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24696, 5), 872}, // 1016 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24680, 5), 869}, // 1017 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24780, 14), 878}, // 1018 Gather4x1QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24794, 12), 879}, // 1019 GatherDQCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24806, 13), 880}, // 1020 GatherH2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24819, 13), 881}, // 1021 GatherV2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24867, 16), 885}, // 1022 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24853, 14), 884}, // 1023 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24839, 14), 883}, // 1024 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24979, 28), 888}, // 1025 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25007, 27), 888}, // 1026 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24924, 28), 887}, // 1027 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24952, 27), 887}, // 1028 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24883, 21), 886}, // 1029 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24904, 20), 886}, // 1030 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24832, 7), 882}, // 1031 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25034, 10), 889}, // 1032 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25044, 10), 890}, // 1033 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25065, 15), 892}, // 1034 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25054, 11), 891}, // 1035 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25080, 5), 894}, // 1036 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(25094, 12), 897}, // 1037 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(25106, 13), 899}, // 1038 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(25089, 5), 896}, // 1039 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(25085, 4), 895}, // 1040 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(25119, 19), 902}, // 1041 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(25138, 22), 902}, // 1042 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25160, 17), 903}, // 1043 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(25177, 20), 903}, // 1044 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9700, 7), 901}, // 1045 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(25197, 16), 904}, // 1046 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25213, 19), 904}, // 1047 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(16803, 5), 893}, // 1048 None in SPV_OPERAND_TYPE_IMAGE
  {IR(25285, 12), 908}, // 1049 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(18210, 7), 898}, // 1050 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(25297, 8), 909}, // 1051 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(18007, 7), 900}, // 1052 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(25263, 11), 906}, // 1053 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(25232, 14), 905}, // 1054 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25246, 17), 905}, // 1055 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25274, 11), 907}, // 1056 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(16700, 6), 924}, // 1057 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25466, 10), 923}, // 1058 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25401, 12), 918}, // 1059 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25413, 12), 919}, // 1060 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25390, 11), 917}, // 1061 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25315, 11), 911}, // 1062 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25305, 10), 910}, // 1063 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25375, 15), 916}, // 1064 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25487, 17), 926}, // 1065 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25504, 16), 927}, // 1066 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25637, 16), 934}, // 1067 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25653, 16), 935}, // 1068 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25336, 11), 913}, // 1069 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25476, 11), 925}, // 1070 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25560, 20), 930}, // 1071 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25326, 10), 912}, // 1072 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25361, 14), 915}, // 1073 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25347, 14), 914}, // 1074 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25580, 19), 931}, // 1075 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25599, 19), 932}, // 1076 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25618, 19), 933}, // 1077 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25438, 14), 921}, // 1078 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25452, 14), 922}, // 1079 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25425, 13), 920}, // 1080 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25520, 20), 928}, // 1081 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25540, 20), 929}, // 1082 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25671, 2), 937}, // 1083 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25772, 5), 955}, // 1084 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25693, 5), 943}, // 1085 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25688, 5), 942}, // 1086 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25730, 6), 949}, // 1087 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25736, 13), 950}, // 1088 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25698, 10), 944}, // 1089 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25708, 10), 945}, // 1090 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25669, 2), 936}, // 1091 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25676, 3), 939}, // 1092 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25673, 3), 938}, // 1093 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25679, 4), 940}, // 1094 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25683, 5), 941}, // 1095 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25725, 5), 948}, // 1096 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25721, 4), 947}, // 1097 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25718, 3), 946}, // 1098 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25766, 6), 954}, // 1099 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25749, 5), 951}, // 1100 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25760, 6), 953}, // 1101 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25754, 6), 952}, // 1102 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25777, 28), 956}, // 1103 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25805, 27), 956}, // 1104 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25832, 24), 957}, // 1105 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25856, 23), 957}, // 1106 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25879, 7), 958}, // 1107 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25886, 11), 959}, // 1108 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25897, 14), 960}, // 1109 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25911, 12), 962}, // 1110 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(16803, 5), 961}, // 1111 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(25923, 13), 964}, // 1112 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(16803, 5), 963}, // 1113 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(25936, 7), 965}, // 1114 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25943, 7), 966}, // 1115 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25950, 12), 967}, // 1116 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25962, 8), 968}, // 1117 WeakAMD in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25984, 12), 970}, // 1118 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26036, 17), 973}, // 1119 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26011, 25), 972}, // 1120 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25996, 15), 971}, // 1121 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25970, 14), 969}, // 1122 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26176, 22), 986}, // 1123 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26198, 21), 986}, // 1124 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26071, 19), 977}, // 1125 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26090, 17), 978}, // 1126 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26060, 11), 976}, // 1127 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20603, 25), 984}, // 1128 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20628, 24), 984}, // 1129 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26135, 18), 981}, // 1130 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26219, 19), 988}, // 1131 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26238, 18), 988}, // 1132 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26381, 16), 992}, // 1133 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26397, 15), 992}, // 1134 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20652, 21), 985}, // 1135 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20673, 20), 985}, // 1136 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26256, 22), 989}, // 1137 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26278, 21), 989}, // 1138 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26121, 14), 980}, // 1139 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26412, 27), 993}, // 1140 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26439, 26), 993}, // 1141 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26107, 14), 979}, // 1142 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(11237, 23), 994}, // 1143 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26352, 15), 991}, // 1144 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26367, 14), 991}, // 1145 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(16803, 5), 974}, // 1146 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26163, 13), 983}, // 1147 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26153, 10), 982}, // 1148 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20693, 21), 987}, // 1149 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20714, 20), 987}, // 1150 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26299, 27), 990}, // 1151 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26326, 26), 990}, // 1152 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26053, 7), 975}, // 1153 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26748, 27), 1008}, // 1154 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26696, 26), 1006}, // 1155 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26616, 23), 1002}, // 1156 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26570, 23), 1000}, // 1157 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26465, 29), 996}, // 1158 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26662, 17), 1004}, // 1159 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26775, 27), 1009}, // 1160 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26722, 26), 1007}, // 1161 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26639, 23), 1003}, // 1162 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26593, 23), 1001}, // 1163 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26494, 29), 997}, // 1164 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26679, 17), 1005}, // 1165 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26523, 21), 998}, // 1166 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26544, 26), 999}, // 1167 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(16803, 5), 995}, // 1168 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26935, 20), 1017}, // 1169 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26802, 8), 1012}, // 1170 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26810, 21), 1014}, // 1171 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26831, 24), 1014}, // 1172 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26855, 19), 1015}, // 1173 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26874, 22), 1015}, // 1174 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26955, 17), 1018}, // 1175 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26896, 18), 1016}, // 1176 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26914, 21), 1016}, // 1177 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(16803, 5), 1010}, // 1178 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25285, 12), 1013}, // 1179 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(18041, 9), 1011}, // 1180 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26979, 8), 1020}, // 1181 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26987, 7), 1021}, // 1182 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26972, 7), 1019}, // 1183 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26994, 7), 1022}, // 1184 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27001, 10), 1022}, // 1185 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27019, 8), 1024}, // 1186 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27035, 15), 1026}, // 1187 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27139, 20), 1032}, // 1188 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27118, 21), 1031}, // 1189 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27159, 12), 1033}, // 1190 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27200, 14), 1035}, // 1191 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27214, 17), 1035}, // 1192 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27231, 12), 1036}, // 1193 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27243, 15), 1036}, // 1194 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(16803, 5), 1023}, // 1195 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27171, 13), 1034}, // 1196 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27184, 16), 1034}, // 1197 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27011, 8), 1023}, // 1198 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27027, 8), 1025}, // 1199 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27050, 23), 1027}, // 1200 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27087, 15), 1029}, // 1201 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27073, 14), 1028}, // 1202 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(18041, 9), 1037}, // 1203 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27102, 16), 1030}, // 1204 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27258, 10), 1038}, // 1205 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(27273, 4), 1040}, // 1206 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27286, 8), 1042}, // 1207 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27277, 9), 1041}, // 1208 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27268, 5), 1039}, // 1209 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27294, 25), 1043}, // 1210 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27319, 28), 1043}, // 1211 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27360, 4), 1046}, // 1212 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27393, 9), 1050}, // 1213 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27402, 13), 1051}, // 1214 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27373, 8), 1048}, // 1215 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27381, 12), 1049}, // 1216 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27364, 9), 1047}, // 1217 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27347, 4), 1044}, // 1218 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27351, 9), 1045}, // 1219 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(16803, 5), 1052}, // 1220 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27415, 25), 1053}, // 1221 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27440, 23), 1054}, // 1222 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27532, 27), 1060}, // 1223 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27559, 28), 1061}, // 1224 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27601, 16), 1063}, // 1225 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27587, 14), 1062}, // 1226 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27701, 30), 1066}, // 1227 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27671, 30), 1066}, // 1228 ForceOpacityMicromap2StateKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27473, 12), 1057}, // 1229 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(17555, 8), 1055}, // 1230 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27463, 10), 1056}, // 1231 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27658, 13), 1065}, // 1232 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27634, 24), 1064}, // 1233 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27508, 24), 1059}, // 1234 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27617, 17), 1064}, // 1235 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27485, 23), 1058}, // 1236 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27772, 37), 1068}, // 1237 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27731, 41), 1067}, // 1238 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27887, 42), 1071}, // 1239 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27809, 37), 1069}, // 1240 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27846, 41), 1070}, // 1241 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27929, 33), 1072}, // 1242 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(27962, 33), 1073}, // 1243 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(28007, 6), 1076}, // 1244 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27995, 12), 1075}, // 1245 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(16803, 5), 1074}, // 1246 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28013, 7), 1077}, // 1247 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28020, 15), 1078}, // 1248 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28043, 7), 1080}, // 1249 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28035, 8), 1079}, // 1250 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28108, 13), 1089}, // 1251 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28150, 4), 1095}, // 1252 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28188, 9), 1100}, // 1253 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28121, 5), 1090}, // 1254 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28250, 5), 1109}, // 1255 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28321, 6), 1119}, // 1256 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28074, 5), 1084}, // 1257 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28228, 5), 1105}, // 1258 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28285, 6), 1114}, // 1259 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28338, 5), 1122}, // 1260 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28332, 6), 1121}, // 1261 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28154, 3), 1096}, // 1262 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28197, 8), 1101}, // 1263 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28255, 4), 1110}, // 1264 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28327, 5), 1120}, // 1265 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28141, 5), 1093}, // 1266 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28169, 10), 1098}, // 1267 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28102, 6), 1088}, // 1268 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28239, 6), 1107}, // 1269 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28308, 7), 1117}, // 1270 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28096, 6), 1087}, // 1271 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28233, 6), 1106}, // 1272 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28301, 7), 1116}, // 1273 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28146, 4), 1094}, // 1274 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28179, 9), 1099}, // 1275 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28245, 5), 1108}, // 1276 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28315, 6), 1118}, // 1277 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28133, 8), 1092}, // 1278 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28291, 10), 1115}, // 1279 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28126, 7), 1091}, // 1280 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28157, 12), 1097}, // 1281 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28066, 8), 1083}, // 1282 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28213, 8), 1103}, // 1283 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28268, 9), 1112}, // 1284 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28058, 8), 1082}, // 1285 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28205, 8), 1102}, // 1286 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28259, 9), 1111}, // 1287 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28079, 6), 1085}, // 1288 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28085, 11), 1086}, // 1289 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28221, 7), 1104}, // 1290 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28277, 8), 1113}, // 1291 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28050, 8), 1081}, // 1292 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28343, 12), 1123}, // 1293 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28355, 7), 1124}, // 1294 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28381, 11), 1127}, // 1295 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28392, 12), 1128}, // 1296 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28404, 15), 1128}, // 1297 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28419, 14), 1129}, // 1298 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28372, 9), 1126}, // 1299 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28362, 10), 1125}, // 1300 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28441, 12), 1132}, // 1301 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28433, 8), 1131}, // 1302 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(16803, 5), 1130}, // 1303 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28453, 29), 1133}, // 1304 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(16684, 8), 1135}, // 1305 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16692, 8), 1136}, // 1306 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16700, 6), 1137}, // 1307 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16706, 7), 1138}, // 1308 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16713, 11), 1139}, // 1309 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16724, 9), 1140}, // 1310 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16733, 13), 1141}, // 1311 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16672, 12), 1134}, // 1312 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16746, 6), 1142}, // 1313 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16752, 10), 1143}, // 1314 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16762, 6), 1144}, // 1315 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16783, 20), 1146}, // 1316 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16768, 15), 1145}, // 1317 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16892, 15), 1154}, // 1318 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16907, 13), 1155}, // 1319 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16880, 12), 1153}, // 1320 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16970, 21), 1159}, // 1321 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16863, 17), 1152}, // 1322 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17047, 16), 1163}, // 1323 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16851, 12), 1151}, // 1324 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17031, 16), 1162}, // 1325 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16824, 14), 1149}, // 1326 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16808, 16), 1148}, // 1327 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16838, 13), 1150}, // 1328 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16991, 20), 1160}, // 1329 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16935, 18), 1157}, // 1330 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16920, 15), 1156}, // 1331 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17011, 20), 1161}, // 1332 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16953, 17), 1158}, // 1333 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17083, 24), 1165}, // 1334 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17063, 20), 1164}, // 1335 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(28482, 26), 1166}, // 1336 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16803, 5), 1147}, // 1337 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17168, 9), 1171}, // 1338 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17200, 7), 1175}, // 1339 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17107, 6), 1167}, // 1340 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17207, 9), 1176}, // 1341 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17118, 6), 1169}, // 1342 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17113, 5), 1168}, // 1343 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17157, 11), 1170}, // 1344 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17189, 11), 1174}, // 1345 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17177, 5), 1172}, // 1346 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17182, 7), 1173}, // 1347 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17252, 11), 1180}, // 1348 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17216, 10), 1177}, // 1349 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17239, 13), 1179}, // 1350 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17226, 13), 1178}, // 1351 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(28600, 9), 1196}, // 1352 ApilaJai in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28543, 15), 1187}, // 1353 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28508, 5), 1182}, // 1354 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28513, 5), 1183}, // 1355 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28563, 7), 1189}, // 1356 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28538, 5), 1186}, // 1357 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28570, 5), 1190}, // 1358 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28518, 9), 1184}, // 1359 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28527, 11), 1185}, // 1360 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28595, 5), 1195}, // 1361 Pred in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28590, 5), 1194}, // 1362 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28558, 5), 1188}, // 1363 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28580, 6), 1192}, // 1364 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28050, 8), 1181}, // 1365 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28575, 5), 1191}, // 1366 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28586, 4), 1193}, // 1367 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28691, 14), 1207}, // 1368 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28773, 16), 1213}, // 1369 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28789, 15), 1213}, // 1370 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29109, 17), 1223}, // 1371 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28638, 15), 1202}, // 1372 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29126, 17), 1224}, // 1373 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29143, 16), 1224}, // 1374 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28661, 9), 1204}, // 1375 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28670, 8), 1205}, // 1376 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28878, 16), 1216}, // 1377 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28894, 15), 1216}, // 1378 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29087, 22), 1222}, // 1379 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29042, 21), 1220}, // 1380 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29159, 15), 1225}, // 1381 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29174, 14), 1225}, // 1382 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28705, 6), 1208}, // 1383 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28804, 24), 1214}, // 1384 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28828, 23), 1214}, // 1385 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28909, 22), 1217}, // 1386 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28931, 21), 1217}, // 1387 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28625, 6), 1198}, // 1388 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28757, 16), 1212}, // 1389 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28631, 7), 1200}, // 1390 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28995, 22), 1219}, // 1391 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29017, 25), 1219}, // 1392 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28653, 8), 1203}, // 1393 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28678, 13), 1206}, // 1394 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28851, 14), 1215}, // 1395 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28865, 13), 1215}, // 1396 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28952, 22), 1218}, // 1397 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28974, 21), 1218}, // 1398 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28711, 14), 1209}, // 1399 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29063, 24), 1221}, // 1400 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28738, 19), 1211}, // 1401 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28725, 13), 1210}, // 1402 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(18092, 8), 1199}, // 1403 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28609, 16), 1197}, // 1404 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28362, 10), 1201}, // 1405 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25996, 15), 1229}, // 1406 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25970, 14), 1226}, // 1407 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29206, 15), 1228}, // 1408 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29188, 18), 1227}, // 1409 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29232, 11), 1232}, // 1410 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29243, 17), 1233}, // 1411 DecodeVectorFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(16803, 5), 1230}, // 1412 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29221, 11), 1231}, // 1413 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(27995, 12), 1236}, // 1414 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(18050, 9), 1235}, // 1415 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28013, 7), 1237}, // 1416 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28020, 15), 1238}, // 1417 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29260, 10), 1234}, // 1418 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29313, 24), 1242}, // 1419 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29337, 22), 1243}, // 1420 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29359, 21), 1244}, // 1421 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29270, 8), 1239}, // 1422 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29278, 15), 1240}, // 1423 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29293, 20), 1241}, // 1424 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 150);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(158, 357);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(515, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(523, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(526, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(528, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(547, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(557, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(561, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(576, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(580, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(586, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(589, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(592, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(596, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(604, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(607, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(623, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(632, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(635, 200);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(835, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(843, 104);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(947, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(970, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(972, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(980, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(982, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(993, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(997, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(1001, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(1008, 10);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(1018, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(1022, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(1032, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1036, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1057, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1083, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1103, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1107, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1110, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1112, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1114, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1118, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1123, 31);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1154, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1169, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1181, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1186, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1205, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1206, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1210, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1212, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1220, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1223, 14);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1237, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1239, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1242, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1244, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1249, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1251, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1293, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1301, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1304, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1305, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1313, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1316, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1318, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1338, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1348, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1352, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1368, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1406, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1410, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1414, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1419, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1036, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1169, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1210, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(580, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1154, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1220, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(972, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1419, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(158, 357);
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
static const std::array<InstructionDesc, 876> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(29380, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(29436, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(29442, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(29561, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(29568, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(29584, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(29589, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(29600, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(29607, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(29612, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(29622, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(29682, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(29754, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(29828, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(29871, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(29913, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(29924, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(29933, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(29942, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(29987, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(29997, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(30008, 11), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(30132, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(30142, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(30154, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(30171, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(30181, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(30198, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(30209, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(30251, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(30263, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(30276, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(30286, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(30302, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(30316, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(30326, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(30335, 19), IR(0, 0), IR(221, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(30354, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(30367, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(18050, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(30419, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(30515, 16), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(30531, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(30544, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(30561, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(30579, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(30592, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(30655, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(28661, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(30704, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(30722, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(30734, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(30747, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(30756, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(30814, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(30819, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(30825, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(30836, 16), IR(0, 0), IR(224, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(30852, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(30864, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(30884, 15), IR(0, 0), IR(226, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(30899, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(30911, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(30934, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(30985, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(30994, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(31009, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(31025, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(31084, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(31104, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(31125, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(31145, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(31159, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(31178, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(31195, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(31211, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(31222, 10), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(31232, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(31277, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(31323, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(31346, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(31373, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(31400, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(31427, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(31454, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(31485, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(31516, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(31527, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(31539, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(31555, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(31565, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(28705, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(31576, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(31593, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(31609, 18), IR(0, 0), IR(230, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(31627, 15), IR(0, 0), IR(230, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(31642, 14), IR(0, 0), IR(232, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(31656, 17), IR(0, 0), IR(230, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(31673, 18), IR(0, 0), IR(230, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(31691, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(31703, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(31715, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(31727, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(31739, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(31748, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(31757, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(31766, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(31780, 14), IR(0, 0), IR(221, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(31794, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(31809, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(31824, 14), IR(0, 0), IR(221, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(31838, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(31855, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(31872, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(31897, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(31905, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(31913, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(31921, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(31926, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(31931, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(31936, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(31941, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(31946, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(31951, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(31956, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(31961, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(31966, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(31971, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(31976, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(31981, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(31986, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(31991, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(32009, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(32027, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(32045, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(32063, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(32081, 13), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(32094, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(32098, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(32108, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(32119, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(32132, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(32145, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(32149, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(32153, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(32159, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(32165, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(32174, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(32183, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(32194, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(32208, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(32216, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(32226, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(32239, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(32255, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(32265, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(32276, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(32287, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(32294, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(32301, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(32311, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(32324, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(32337, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(32355, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(32373, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(32383, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(32393, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(32408, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(32423, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(32433, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(32445, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(32458, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(32473, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(32486, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(32501, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(32517, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(32535, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(32553, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(32573, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(32594, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(32617, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(32635, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(32656, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(32673, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(32683, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(32694, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(32705, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(32709, 15), IR(0, 0), IR(233, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(32724, 17), IR(0, 0), IR(233, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(32741, 17), IR(0, 0), IR(233, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(32758, 11), IR(0, 0), IR(233, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(32769, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(32778, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(32783, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(32788, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(32795, 9), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(32804, 9), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(32813, 11), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(32824, 11), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(32835, 11), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(32846, 13), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(32859, 11), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(32870, 13), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(32883, 17), IR(0, 0), IR(123, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(32900, 19), IR(0, 0), IR(123, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(32956, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(32971, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(32985, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(32996, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(33008, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(33023, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(33045, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(33071, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(33088, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(33105, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(33116, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(33127, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(33138, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(33149, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(33160, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(33171, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(33181, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(33190, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(33200, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(33234, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(33279, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(33294, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(33300, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(33307, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(33370, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(33377, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(33382, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(33389, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(33401, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(33413, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(33427, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(33440, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(33455, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(33471, 9), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(33480, 9), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(33489, 15), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(33537, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(33547, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(33557, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(33567, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(33577, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(33587, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(33597, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(33607, 10), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(33617, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(33626, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(33636, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(33653, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(33671, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(33694, 24), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(33718, 15), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(33733, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(33749, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(33766, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(33784, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(33802, 28), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(33830, 29), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(33859, 20), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(33879, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(33900, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(33914, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(33928, 30), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(33958, 32), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(33990, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(34013, 40), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(34053, 12), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(34065, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(34078, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(34094, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(34107, 19), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(34126, 26), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(34152, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(34168, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(34181, 29), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(34210, 29), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(34239, 33), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(34272, 33), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(34305, 33), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(34338, 33), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(34371, 37), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(34408, 37), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(34445, 17), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(34462, 18), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(34480, 22), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(34502, 26), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(34528, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(34535, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(34556, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(34572, 16), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(34588, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(34595, 16), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(34611, 20), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(34631, 26), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(34657, 35), IR(0, 0), IR(165, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(34692, 25), IR(0, 0), IR(165, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(34717, 17), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(34734, 23), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(34757, 19), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(34776, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(34792, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(34808, 11), IR(0, 0), IR(1, 0), IR(202, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(34819, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(34840, 19), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(34859, 19), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(34878, 24), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(34902, 25), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(34927, 30), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6887, 22), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(34957, 29), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(34986, 32), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(35018, 30), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(35048, 29), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(35077, 29), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(10026, 23), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(35106, 26), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(35132, 25), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(35157, 27), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(35184, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(35204, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(35224, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(35244, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(35264, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(35284, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(35304, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(35324, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(35344, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(35364, 20), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(35384, 26), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(35410, 25), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(35435, 26), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(35461, 26), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(35487, 25), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(35512, 26), IR(0, 0), IR(244, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(35538, 29), IR(0, 0), IR(247, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(35567, 24), IR(0, 0), IR(247, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(35591, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(35603, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(35612, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(35624, 8), IR(0, 0), IR(248, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(35632, 23), IR(0, 0), IR(157, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(35655, 23), IR(0, 0), IR(166, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(35678, 25), IR(0, 0), IR(167, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(35703, 14), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(35759, 14), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(35773, 15), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(35788, 19), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(35807, 17), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(35824, 19), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(10317, 9), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(35843, 14), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(35857, 18), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(35875, 12), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(35887, 13), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBitcastExtractEXT, true, true, IR(172, 4), IR(10464, 18), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(35900, 20), IR(0, 0), IR(3, 1), IR(212, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(35920, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(35942, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(35961, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(35983, 30), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6648, 18), IR(0, 0), IR(253, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(36013, 27), IR(0, 0), IR(253, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(36040, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(36065, 33), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(36098, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(36120, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(36139, 7), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(36146, 15), IR(0, 0), IR(255, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(36161, 15), IR(0, 0), IR(255, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(36176, 20), IR(0, 0), IR(255, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(15462, 25), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(36196, 26), IR(0, 0), IR(253, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(36222, 26), IR(0, 0), IR(1, 0), IR(213, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(36248, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(36273, 12), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(36285, 19), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(36304, 35), IR(0, 0), IR(257, 2), IR(214, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(36339, 22), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(36361, 16), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(36424, 5), IR(176, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(36437, 5), IR(177, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(36450, 6), IR(178, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(36465, 11), IR(179, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(36490, 11), IR(180, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(36515, 12), IR(181, 1), IR(259, 1), IR(156, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(36542, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(36567, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(36592, 26), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(36672, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(36699, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36726, 30), IR(0, 0), IR(260, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36756, 34), IR(0, 0), IR(260, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(36790, 31), IR(0, 0), IR(260, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(36821, 16), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(36837, 22), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(36859, 21), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(36880, 32), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(36912, 31), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(36943, 19), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(36962, 31), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(36993, 24), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(37017, 19), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(37036, 23), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(37059, 23), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(37082, 17), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(37099, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(37128, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(37157, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(37186, 29), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(37215, 30), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(37245, 28), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(37273, 20), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpImageGatherQCOM, true, true, IR(479, 7), IR(37293, 16), IR(0, 0), IR(266, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(37309, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(37332, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(37355, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(37378, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(37401, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(37424, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(37447, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(37470, 23), IR(0, 0), IR(236, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(37493, 21), IR(0, 0), IR(268, 1), IR(61, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(37514, 17), IR(0, 0), IR(268, 1), IR(61, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(37531, 13), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(37544, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(37569, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(37593, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(37618, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(37647, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(37674, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(37697, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(37716, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(37739, 26), IR(0, 0), IR(174, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(37765, 26), IR(0, 0), IR(174, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(37791, 14), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(37805, 17), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAbortKHR, false, false, IR(36, 2), IR(11538, 9), IR(0, 0), IR(270, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(37822, 28), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(37850, 20), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(37870, 18), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantDataKHR, true, true, IR(486, 3), IR(11547, 16), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantDataKHR, true, true, IR(486, 3), IR(37888, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(37908, 10), IR(0, 0), IR(175, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(37918, 10), IR(0, 0), IR(175, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(489, 14), IR(37928, 27), IR(0, 0), IR(271, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(503, 13), IR(37955, 36), IR(0, 0), IR(271, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(516, 7), IR(37991, 28), IR(0, 0), IR(271, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(38019, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(38047, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(38075, 33), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(38108, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(503, 13), IR(38138, 26), IR(0, 0), IR(271, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(38164, 39), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(38203, 44), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(38247, 23), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(523, 12), IR(38270, 20), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(503, 13), IR(38290, 21), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(523, 12), IR(38311, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(535, 6), IR(38341, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(38363, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(38388, 26), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(38414, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(38439, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(38461, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(38490, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(38518, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(38543, 34), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(38577, 32), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(38609, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(38638, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(38660, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(38682, 19), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(38701, 17), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(38718, 18), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(541, 3), IR(38736, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(38765, 24), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(38789, 16), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(479, 7), IR(38805, 23), IR(0, 0), IR(273, 1), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(38828, 16), IR(182, 1), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(544, 13), IR(38868, 29), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(557, 7), IR(38897, 42), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(38939, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(564, 16), IR(38978, 32), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertUseEXT, true, true, IR(160, 3), IR(39010, 31), IR(183, 1), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(580, 4), IR(39068, 17), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(39085, 18), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(39103, 28), IR(184, 1), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(39158, 33), IR(0, 0), IR(51, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(39191, 35), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(39226, 38), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(584, 5), IR(39264, 24), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(589, 4), IR(39288, 25), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(593, 5), IR(39313, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(516, 7), IR(39341, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(39364, 29), IR(0, 0), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(39393, 51), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(39444, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(39468, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(598, 4), IR(39513, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(602, 14), IR(39546, 32), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(616, 15), IR(39578, 38), IR(0, 0), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(39616, 17), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(39633, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(541, 3), IR(39658, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(523, 12), IR(39688, 21), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(503, 13), IR(39709, 27), IR(0, 0), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(39736, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(39760, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(39786, 27), IR(0, 0), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(39813, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(39839, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(39862, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(39892, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(39921, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(39947, 35), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(39982, 31), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(40013, 34), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(40047, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(40080, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(40110, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(40139, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(40168, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(40191, 22), IR(185, 1), IR(55, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(40234, 21), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(40255, 15), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(40270, 8), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(523, 12), IR(40278, 14), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(523, 12), IR(40292, 17), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(40309, 50), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(40359, 29), IR(186, 1), IR(284, 4), IR(216, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(40416, 18), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(40434, 35), IR(187, 1), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(40492, 24), IR(0, 0), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(40516, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(40539, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(40584, 40), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(40624, 20), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(40644, 18), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(40662, 19), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(631, 5), IR(40681, 24), IR(0, 0), IR(288, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(636, 6), IR(40705, 24), IR(0, 0), IR(288, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(642, 5), IR(40729, 25), IR(0, 0), IR(288, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(40754, 26), IR(0, 0), IR(288, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(40780, 26), IR(0, 0), IR(288, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixGetCoordinateEXT, true, true, IR(172, 4), IR(13724, 34), IR(0, 0), IR(289, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(40806, 28), IR(0, 0), IR(290, 3), IR(89, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(40834, 26), IR(0, 0), IR(290, 3), IR(89, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceEXT, true, true, IR(647, 5), IR(40903, 27), IR(188, 1), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(652, 7), IR(41031, 30), IR(0, 0), IR(218, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(659, 5), IR(41061, 31), IR(0, 0), IR(218, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpEXT, true, true, IR(151, 5), IR(41092, 33), IR(189, 1), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(41157, 19), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(664, 4), IR(41176, 17), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(41193, 21), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(41214, 27), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(41241, 24), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(41265, 20), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(41285, 28), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(41313, 19), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(41332, 25), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(41357, 22), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(13078, 25), IR(46, 1), IR(296, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(41379, 22), IR(0, 0), IR(296, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(41401, 20), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(41421, 27), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(41448, 29), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(41477, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(41495, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(41515, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(41533, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(41553, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(41578, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(41603, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(668, 7), IR(41684, 17), IR(0, 0), IR(207, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(41701, 40), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(41741, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(41779, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(41817, 34), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(41851, 37), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(41888, 29), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(41917, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(41944, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(41971, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(41994, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(42017, 20), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(42037, 22), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(42059, 19), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13809, 21), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(42078, 25), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(42103, 23), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(42126, 24), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(42150, 23), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(42173, 24), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(42197, 28), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(42225, 29), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(42254, 33), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(675, 5), IR(42287, 34), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(42321, 24), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(42345, 25), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(42370, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(42383, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(42396, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(42409, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(42422, 14), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(42436, 14), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(42450, 21), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(42471, 21), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(42492, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(42505, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(42518, 15), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(42533, 15), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(42548, 29), IR(0, 0), IR(216, 1), IR(113, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(42577, 25), IR(0, 0), IR(216, 1), IR(113, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(42602, 15), IR(0, 0), IR(139, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(680, 6), IR(14027, 9), IR(0, 0), IR(139, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(42617, 13), IR(0, 0), IR(139, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(42630, 14), IR(0, 0), IR(303, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(42644, 14), IR(0, 0), IR(303, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(42658, 14), IR(0, 0), IR(307, 1), IR(117, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(42672, 10), IR(0, 0), IR(307, 1), IR(117, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(42682, 15), IR(190, 1), IR(1, 0), IR(220, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(42718, 21), IR(191, 1), IR(1, 0), IR(220, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(42766, 14), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(42780, 18), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(42798, 23), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(42821, 23), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(42844, 23), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(42867, 23), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(42890, 22), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(42912, 22), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42934, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42980, 44), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(43024, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(43063, 37), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(43100, 22), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(43122, 22), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43144, 60), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43204, 53), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43257, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43304, 40), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43344, 51), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43395, 44), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(43439, 51), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(43490, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43546, 50), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43596, 52), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43648, 49), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(43697, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(43744, 50), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(43794, 51), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(43845, 56), IR(0, 0), IR(311, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(43901, 33), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43934, 52), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43986, 61), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44047, 61), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(44108, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(44147, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(44185, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(44224, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(44262, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(44301, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(44339, 36), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(44375, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(44414, 43), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(44457, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(44495, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(44533, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(44571, 45), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(44616, 40), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44656, 62), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(44718, 30), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(44748, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(44786, 36), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(44822, 33), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(44855, 35), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(44890, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(44929, 43), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(44972, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(45019, 54), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(45073, 34), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(45107, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(45154, 45), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(45199, 55), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(45254, 53), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(45307, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(45363, 54), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(45417, 58), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(45475, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(45531, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(45569, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(45615, 44), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45659, 49), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45708, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(45755, 70), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(45825, 68), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(45893, 69), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(45962, 68), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(46030, 66), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(46096, 67), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(46163, 36), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(46199, 48), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(46247, 59), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(46306, 58), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(46364, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(686, 9), IR(46420, 30), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(695, 10), IR(46450, 30), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46480, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(46519, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46565, 43), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46608, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46655, 45), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46700, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(46746, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(46802, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(46840, 30), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(46870, 32), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(695, 10), IR(46902, 36), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(705, 13), IR(46938, 42), IR(0, 0), IR(311, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(46980, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(47019, 39), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(47058, 44), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(47102, 48), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(47150, 50), IR(0, 0), IR(311, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(47200, 43), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(47243, 48), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(47291, 43), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(47334, 31), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(47365, 47), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(47412, 45), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(47457, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(47503, 56), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(47559, 38), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(47597, 35), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(47632, 44), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(47676, 46), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(47722, 41), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(47763, 36), IR(0, 0), IR(311, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(47799, 50), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(47849, 48), IR(0, 0), IR(309, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(47897, 35), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(14265, 25), IR(0, 0), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(47932, 16), IR(0, 0), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(47948, 19), IR(0, 0), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(718, 8), IR(47967, 29), IR(192, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(718, 8), IR(48024, 25), IR(193, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(718, 8), IR(48073, 32), IR(194, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(718, 8), IR(48136, 30), IR(195, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(726, 10), IR(48195, 24), IR(196, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(726, 10), IR(48242, 24), IR(197, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(726, 10), IR(48289, 24), IR(198, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(726, 10), IR(48336, 24), IR(199, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(736, 6), IR(48383, 23), IR(200, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(736, 6), IR(48428, 23), IR(201, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(736, 6), IR(48473, 23), IR(202, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(736, 6), IR(48518, 23), IR(203, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(736, 6), IR(48563, 23), IR(204, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(718, 8), IR(48608, 26), IR(205, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(718, 8), IR(48659, 26), IR(206, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(718, 8), IR(48710, 25), IR(207, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(726, 10), IR(48759, 26), IR(208, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(718, 8), IR(48810, 25), IR(209, 1), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(718, 8), IR(48859, 23), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(718, 8), IR(48882, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(718, 8), IR(48906, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(718, 8), IR(48931, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(718, 8), IR(48956, 23), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(718, 8), IR(48979, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(718, 8), IR(49003, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(718, 8), IR(49028, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(718, 8), IR(49053, 23), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(718, 8), IR(49076, 23), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(718, 8), IR(49099, 26), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(718, 8), IR(49125, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(718, 8), IR(49150, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(718, 8), IR(49175, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(718, 8), IR(49199, 26), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(718, 8), IR(49225, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(718, 8), IR(49249, 26), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(718, 8), IR(49275, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(718, 8), IR(49299, 26), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(726, 10), IR(49325, 25), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(726, 10), IR(49350, 23), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(726, 10), IR(49373, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(726, 10), IR(49397, 24), IR(0, 0), IR(314, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(49421, 17), IR(0, 0), IR(315, 1), IR(127, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(742, 2), IR(49438, 21), IR(0, 0), IR(145, 1), IR(139, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(744, 3), IR(49459, 20), IR(0, 0), IR(145, 1), IR(139, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(49479, 24), IR(0, 0), IR(145, 1), IR(139, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(718, 8), IR(49503, 16), IR(210, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(718, 8), IR(49534, 17), IR(211, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(718, 8), IR(49567, 17), IR(212, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(718, 8), IR(49600, 15), IR(213, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(718, 8), IR(49629, 15), IR(214, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(718, 8), IR(49658, 18), IR(215, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(718, 8), IR(49693, 17), IR(216, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(718, 8), IR(49726, 17), IR(217, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(718, 8), IR(49759, 20), IR(218, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(718, 8), IR(49798, 15), IR(219, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(718, 8), IR(49827, 15), IR(220, 1), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(49856, 30), IR(221, 1), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(49915, 30), IR(222, 1), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, false, false, IR(296, 4), IR(49974, 23), IR(223, 1), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, false, false, IR(296, 4), IR(50019, 24), IR(224, 1), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(15223, 14), IR(65, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(50066, 22), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(50088, 23), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(50111, 28), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(50139, 46), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(50185, 37), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(50222, 65), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(50287, 40), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(50327, 41), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(50368, 39), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(50407, 36), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(50443, 46), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(50489, 45), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(50534, 42), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(50576, 32), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(50608, 29), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(50637, 40), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(50677, 40), IR(0, 0), IR(210, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(50717, 14), IR(0, 0), IR(318, 4), IR(164, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(50731, 23), IR(0, 0), IR(137, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(747, 1), IR(50754, 25), IR(0, 0), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(50779, 32), IR(0, 0), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(50811, 36), IR(0, 0), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, false, false, IR(747, 1), IR(50847, 33), IR(0, 0), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(50880, 20), IR(0, 0), IR(323, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(50900, 20), IR(0, 0), IR(323, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveEXT, false, false, IR(221, 3), IR(50920, 24), IR(225, 1), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitEXT, false, false, IR(221, 3), IR(50970, 22), IR(226, 1), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(15734, 19), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(748, 7), IR(51016, 25), IR(227, 1), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(51065, 24), IR(228, 1), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(51112, 22), IR(229, 1), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(51155, 26), IR(230, 1), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(51206, 23), IR(231, 1), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(51251, 27), IR(0, 0), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(755, 10), IR(51278, 25), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(755, 10), IR(51303, 34), IR(0, 0), IR(328, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(755, 10), IR(51337, 34), IR(0, 0), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(765, 9), IR(51371, 29), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(755, 10), IR(51400, 26), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(774, 7), IR(16211, 38), IR(0, 0), IR(330, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(51488, 21), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(16277, 32), IR(0, 0), IR(332, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(51509, 26), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(781, 5), IR(51535, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(786, 2), IR(51562, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(788, 4), IR(51589, 24), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(792, 6), IR(51613, 30), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(798, 3), IR(51680, 30), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(51710, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPredicatedLoadINTEL, true, true, IR(636, 6), IR(51737, 20), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPredicatedStoreINTEL, false, false, IR(589, 4), IR(51757, 21), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(51778, 13), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(51791, 13), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(51804, 19), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(51823, 18), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(51841, 19), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(51860, 19), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(51879, 18), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(51897, 19), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(51916, 18), IR(0, 0), IR(336, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(801, 6), IR(51934, 18), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(807, 4), IR(51952, 19), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(51971, 26), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(51997, 28), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(52025, 33), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(52058, 19), IR(0, 0), IR(339, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(52077, 19), IR(0, 0), IR(341, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(52096, 19), IR(0, 0), IR(343, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 934> kInstructionNames{{
  {IR(11538, 9), 440}, // 0 AbortKHR
  {IR(42370, 13), 603}, // 1 AbsISubINTEL
  {IR(42383, 13), 604}, // 2 AbsUSubINTEL
  {IR(30852, 12), 58}, // 3 AccessChain
  {IR(49438, 21), 789}, // 4 AliasDomainDeclINTEL
  {IR(49459, 20), 790}, // 5 AliasScopeDeclINTEL
  {IR(49479, 24), 791}, // 6 AliasScopeListDeclINTEL
  {IR(32149, 4), 143}, // 7 All
  {IR(37544, 25), 428}, // 8 AllocateNodePayloadsAMDX
  {IR(32145, 4), 142}, // 9 Any
  {IR(49225, 24), 780}, // 10 ArbitraryFloatACosINTEL
  {IR(49249, 26), 781}, // 11 ArbitraryFloatACosPiINTEL
  {IR(49175, 24), 778}, // 12 ArbitraryFloatASinINTEL
  {IR(49199, 26), 779}, // 13 ArbitraryFloatASinPiINTEL
  {IR(49325, 25), 784}, // 14 ArbitraryFloatATan2INTEL
  {IR(49275, 24), 782}, // 15 ArbitraryFloatATanINTEL
  {IR(49299, 26), 783}, // 16 ArbitraryFloatATanPiINTEL
  {IR(48195, 24), 751}, // 17 ArbitraryFloatAddALTERA
  {IR(48219, 23), 751}, // 18 ArbitraryFloatAddINTEL
  {IR(48024, 25), 748}, // 19 ArbitraryFloatCastALTERA
  {IR(48073, 32), 749}, // 20 ArbitraryFloatCastFromIntALTERA
  {IR(48105, 31), 749}, // 21 ArbitraryFloatCastFromIntINTEL
  {IR(48049, 24), 748}, // 22 ArbitraryFloatCastINTEL
  {IR(48136, 30), 750}, // 23 ArbitraryFloatCastToIntALTERA
  {IR(48166, 29), 750}, // 24 ArbitraryFloatCastToIntINTEL
  {IR(48710, 25), 762}, // 25 ArbitraryFloatCbrtALTERA
  {IR(48735, 24), 762}, // 26 ArbitraryFloatCbrtINTEL
  {IR(49076, 23), 774}, // 27 ArbitraryFloatCosINTEL
  {IR(49150, 25), 777}, // 28 ArbitraryFloatCosPiINTEL
  {IR(48336, 24), 754}, // 29 ArbitraryFloatDivALTERA
  {IR(48360, 23), 754}, // 30 ArbitraryFloatDivINTEL
  {IR(48563, 23), 759}, // 31 ArbitraryFloatEQALTERA
  {IR(48586, 22), 759}, // 32 ArbitraryFloatEQINTEL
  {IR(49003, 25), 771}, // 33 ArbitraryFloatExp10INTEL
  {IR(48979, 24), 770}, // 34 ArbitraryFloatExp2INTEL
  {IR(48956, 23), 769}, // 35 ArbitraryFloatExpINTEL
  {IR(49028, 25), 772}, // 36 ArbitraryFloatExpm1INTEL
  {IR(48428, 23), 756}, // 37 ArbitraryFloatGEALTERA
  {IR(48451, 22), 756}, // 38 ArbitraryFloatGEINTEL
  {IR(48383, 23), 755}, // 39 ArbitraryFloatGTALTERA
  {IR(48406, 22), 755}, // 40 ArbitraryFloatGTINTEL
  {IR(48759, 26), 763}, // 41 ArbitraryFloatHypotALTERA
  {IR(48785, 25), 763}, // 42 ArbitraryFloatHypotINTEL
  {IR(48518, 23), 758}, // 43 ArbitraryFloatLEALTERA
  {IR(48541, 22), 758}, // 44 ArbitraryFloatLEINTEL
  {IR(48473, 23), 757}, // 45 ArbitraryFloatLTALTERA
  {IR(48496, 22), 757}, // 46 ArbitraryFloatLTINTEL
  {IR(48906, 25), 767}, // 47 ArbitraryFloatLog10INTEL
  {IR(48931, 25), 768}, // 48 ArbitraryFloatLog1pINTEL
  {IR(48882, 24), 766}, // 49 ArbitraryFloatLog2INTEL
  {IR(48859, 23), 765}, // 50 ArbitraryFloatLogINTEL
  {IR(48289, 24), 753}, // 51 ArbitraryFloatMulALTERA
  {IR(48313, 23), 753}, // 52 ArbitraryFloatMulINTEL
  {IR(49350, 23), 785}, // 53 ArbitraryFloatPowINTEL
  {IR(49397, 24), 787}, // 54 ArbitraryFloatPowNINTEL
  {IR(49373, 24), 786}, // 55 ArbitraryFloatPowRINTEL
  {IR(48659, 26), 761}, // 56 ArbitraryFloatRSqrtALTERA
  {IR(48685, 25), 761}, // 57 ArbitraryFloatRSqrtINTEL
  {IR(48608, 26), 760}, // 58 ArbitraryFloatRecipALTERA
  {IR(48634, 25), 760}, // 59 ArbitraryFloatRecipINTEL
  {IR(49099, 26), 775}, // 60 ArbitraryFloatSinCosINTEL
  {IR(47967, 29), 747}, // 61 ArbitraryFloatSinCosPiALTERA
  {IR(47996, 28), 747}, // 62 ArbitraryFloatSinCosPiINTEL
  {IR(49053, 23), 773}, // 63 ArbitraryFloatSinINTEL
  {IR(49125, 25), 776}, // 64 ArbitraryFloatSinPiINTEL
  {IR(48810, 25), 764}, // 65 ArbitraryFloatSqrtALTERA
  {IR(48835, 24), 764}, // 66 ArbitraryFloatSqrtINTEL
  {IR(48242, 24), 752}, // 67 ArbitraryFloatSubALTERA
  {IR(48266, 23), 752}, // 68 ArbitraryFloatSubINTEL
  {IR(15734, 19), 835}, // 69 ArithmeticFenceEXT
  {IR(30899, 12), 61}, // 70 ArrayLength
  {IR(42617, 13), 619}, // 71 AsmCallINTEL
  {IR(14027, 9), 618}, // 72 AsmINTEL
  {IR(42602, 15), 617}, // 73 AsmTargetINTEL
  {IR(42658, 14), 622}, // 74 AssumeTrueKHR
  {IR(33171, 10), 220}, // 75 AtomicAnd
  {IR(33023, 22), 210}, // 76 AtomicCompareExchange
  {IR(33045, 26), 211}, // 77 AtomicCompareExchangeWeak
  {IR(33008, 15), 209}, // 78 AtomicExchange
  {IR(50717, 14), 825}, // 79 AtomicFAddEXT
  {IR(42644, 14), 621}, // 80 AtomicFMaxEXT
  {IR(42630, 14), 620}, // 81 AtomicFMinEXT
  {IR(34556, 16), 292}, // 82 AtomicFlagClear
  {IR(34535, 21), 291}, // 83 AtomicFlagTestAndSet
  {IR(33105, 11), 214}, // 84 AtomicIAdd
  {IR(33088, 17), 213}, // 85 AtomicIDecrement
  {IR(33071, 17), 212}, // 86 AtomicIIncrement
  {IR(33116, 11), 215}, // 87 AtomicISub
  {IR(32985, 11), 207}, // 88 AtomicLoad
  {IR(33181, 9), 221}, // 89 AtomicOr
  {IR(33149, 11), 218}, // 90 AtomicSMax
  {IR(33127, 11), 216}, // 91 AtomicSMin
  {IR(32996, 12), 208}, // 92 AtomicStore
  {IR(33160, 11), 219}, // 93 AtomicUMax
  {IR(33138, 11), 217}, // 94 AtomicUMin
  {IR(33190, 10), 222}, // 95 AtomicXor
  {IR(40806, 28), 549}, // 96 BeginInvocationInterlockEXT
  {IR(37082, 17), 408}, // 97 BitCastArrayQCOM
  {IR(32769, 9), 191}, // 98 BitCount
  {IR(32709, 15), 187}, // 99 BitFieldInsert
  {IR(32724, 17), 188}, // 100 BitFieldSExtract
  {IR(32741, 17), 189}, // 101 BitFieldUExtract
  {IR(32758, 11), 190}, // 102 BitReverse
  {IR(31897, 8), 114}, // 103 Bitcast
  {IR(10464, 18), 358}, // 104 BitcastExtractEXT
  {IR(32694, 11), 185}, // 105 BitwiseAnd
  {IR(51488, 21), 848}, // 106 BitwiseFunctionINTEL
  {IR(32673, 10), 183}, // 107 BitwiseOr
  {IR(32683, 11), 184}, // 108 BitwiseXor
  {IR(33300, 7), 227}, // 109 Branch
  {IR(33307, 18), 228}, // 110 BranchConditional
  {IR(37805, 17), 439}, // 111 BufferPointerEXT
  {IR(34168, 13), 277}, // 112 BuildNDRange
  {IR(29913, 11), 15}, // 113 Capability
  {IR(34126, 26), 275}, // 114 CaptureEventProfilingInfo
  {IR(35632, 23), 344}, // 115 ColorAttachmentReadEXT
  {IR(33718, 15), 255}, // 116 CommitReadPipe
  {IR(33733, 16), 256}, // 117 CommitWritePipe
  {IR(31159, 19), 72}, // 118 CompositeConstruct
  {IR(50847, 33), 830}, // 119 CompositeConstructContinuedINTEL
  {IR(37215, 30), 413}, // 120 CompositeConstructCoopMatQCOM
  {IR(36790, 31), 396}, // 121 CompositeConstructReplicateEXT
  {IR(31178, 17), 73}, // 122 CompositeExtract
  {IR(37245, 28), 414}, // 123 CompositeExtractCoopMatQCOM
  {IR(31195, 16), 74}, // 124 CompositeInsert
  {IR(51562, 27), 852}, // 125 ConditionalCapabilityINTEL
  {IR(51710, 27), 856}, // 126 ConditionalCopyObjectINTEL
  {IR(51535, 27), 851}, // 127 ConditionalEntryPointINTEL
  {IR(51509, 26), 850}, // 128 ConditionalExtensionINTEL
  {IR(18050, 9), 39}, // 129 Constant
  {IR(30419, 18), 40}, // 130 ConstantComposite
  {IR(50779, 32), 828}, // 131 ConstantCompositeContinuedINTEL
  {IR(36726, 30), 394}, // 132 ConstantCompositeReplicateEXT
  {IR(11547, 16), 444}, // 133 ConstantDataKHR
  {IR(30367, 14), 38}, // 134 ConstantFalse
  {IR(42548, 29), 615}, // 135 ConstantFunctionPointerINTEL
  {IR(30531, 13), 42}, // 136 ConstantNull
  {IR(34611, 20), 296}, // 137 ConstantPipeStorage
  {IR(30515, 16), 41}, // 138 ConstantSampler
  {IR(37870, 18), 443}, // 139 ConstantSizeOfEXT
  {IR(37697, 19), 434}, // 140 ConstantStringAMDX
  {IR(30354, 13), 37}, // 141 ConstantTrue
  {IR(32956, 15), 205}, // 142 ControlBarrier
  {IR(50920, 24), 833}, // 143 ControlBarrierArriveEXT
  {IR(50944, 26), 833}, // 144 ControlBarrierArriveINTEL
  {IR(50970, 22), 834}, // 145 ControlBarrierWaitEXT
  {IR(50992, 24), 834}, // 146 ControlBarrierWaitINTEL
  {IR(50900, 20), 832}, // 147 ConvertBF16ToFINTEL
  {IR(50880, 20), 831}, // 148 ConvertFToBF16INTEL
  {IR(31703, 12), 100}, // 149 ConvertFToS
  {IR(31691, 12), 99}, // 150 ConvertFToU
  {IR(51971, 26), 870}, // 151 ConvertHandleToImageINTEL
  {IR(52025, 33), 872}, // 152 ConvertHandleToSampledImageINTEL
  {IR(51997, 28), 871}, // 153 ConvertHandleToSamplerINTEL
  {IR(41515, 18), 572}, // 154 ConvertImageToUNV
  {IR(31780, 14), 107}, // 155 ConvertPtrToU
  {IR(31715, 12), 101}, // 156 ConvertSToF
  {IR(41578, 25), 575}, // 157 ConvertSampledImageToUNV
  {IR(41533, 20), 573}, // 158 ConvertSamplerToUNV
  {IR(36304, 35), 380}, // 159 ConvertUToAccelerationStructureKHR
  {IR(31727, 12), 102}, // 160 ConvertUToF
  {IR(41477, 18), 570}, // 161 ConvertUToImageNV
  {IR(31824, 14), 110}, // 162 ConvertUToPtr
  {IR(41553, 25), 574}, // 163 ConvertUToSampledImageNV
  {IR(41495, 20), 571}, // 164 ConvertUToSamplerNV
  {IR(39041, 27), 487}, // 165 CooperativeMatrixConvertNV
  {IR(39010, 31), 487}, // 166 CooperativeMatrixConvertUseEXT
  {IR(13724, 34), 548}, // 167 CooperativeMatrixGetCoordinateEXT
  {IR(36699, 27), 393}, // 168 CooperativeMatrixLengthKHR
  {IR(40780, 26), 547}, // 169 CooperativeMatrixLengthNV
  {IR(36567, 25), 390}, // 170 CooperativeMatrixLoadKHR
  {IR(40705, 24), 544}, // 171 CooperativeMatrixLoadNV
  {IR(41031, 30), 552}, // 172 CooperativeMatrixLoadTensorNV
  {IR(36672, 27), 392}, // 173 CooperativeMatrixMulAddKHR
  {IR(40754, 26), 546}, // 174 CooperativeMatrixMulAddNV
  {IR(41092, 33), 554}, // 175 CooperativeMatrixPerElementOpEXT
  {IR(41125, 32), 554}, // 176 CooperativeMatrixPerElementOpNV
  {IR(40903, 27), 551}, // 177 CooperativeMatrixReduceEXT
  {IR(40930, 26), 551}, // 178 CooperativeMatrixReduceNV
  {IR(36592, 26), 391}, // 179 CooperativeMatrixStoreKHR
  {IR(40729, 25), 545}, // 180 CooperativeMatrixStoreNV
  {IR(41061, 31), 553}, // 181 CooperativeMatrixStoreTensorNV
  {IR(41448, 29), 569}, // 182 CooperativeMatrixTransposeNV
  {IR(39264, 24), 494}, // 183 CooperativeVectorLoadNV
  {IR(38978, 32), 486}, // 184 CooperativeVectorMatrixMulAddNV
  {IR(38868, 29), 483}, // 185 CooperativeVectorMatrixMulNV
  {IR(38897, 42), 484}, // 186 CooperativeVectorOuterProductAccumulateNV
  {IR(38939, 39), 485}, // 187 CooperativeVectorReduceSumAccumulateNV
  {IR(39288, 25), 495}, // 188 CooperativeVectorStoreNV
  {IR(35591, 12), 340}, // 189 CopyLogical
  {IR(30825, 11), 56}, // 190 CopyMemory
  {IR(30836, 16), 57}, // 191 CopyMemorySized
  {IR(31211, 11), 75}, // 192 CopyObject
  {IR(34631, 26), 297}, // 193 CreatePipeFromPipeStorage
  {IR(41193, 21), 557}, // 194 CreateTensorLayoutNV
  {IR(41313, 19), 562}, // 195 CreateTensorViewNV
  {IR(34078, 16), 272}, // 196 CreateUserEvent
  {IR(49915, 30), 804}, // 197 CrossWorkgroupCastToPtrALTERA
  {IR(49945, 29), 804}, // 198 CrossWorkgroupCastToPtrINTEL
  {IR(32778, 5), 192}, // 199 DPdx
  {IR(32824, 11), 198}, // 200 DPdxCoarse
  {IR(32795, 9), 195}, // 201 DPdxFine
  {IR(32783, 5), 193}, // 202 DPdy
  {IR(32835, 11), 199}, // 203 DPdyCoarse
  {IR(32804, 9), 196}, // 204 DPdyFine
  {IR(30985, 9), 64}, // 205 Decorate
  {IR(34808, 11), 305}, // 206 DecorateId
  {IR(42682, 15), 624}, // 207 DecorateString
  {IR(42697, 21), 624}, // 208 DecorateStringGOOGLE
  {IR(31009, 16), 66}, // 209 DecorationGroup
  {IR(13078, 25), 565}, // 210 DemoteToHelperInvocation
  {IR(13103, 28), 565}, // 211 DemoteToHelperInvocationEXT
  {IR(35655, 23), 345}, // 212 DepthAttachmentReadEXT
  {IR(32094, 4), 137}, // 213 Dot
  {IR(39068, 17), 488}, // 214 EmitMeshTasksEXT
  {IR(32883, 17), 203}, // 215 EmitStreamVertex
  {IR(32859, 11), 201}, // 216 EmitVertex
  {IR(40834, 26), 550}, // 217 EndInvocationInterlockEXT
  {IR(32870, 13), 202}, // 218 EndPrimitive
  {IR(32900, 19), 204}, // 219 EndStreamPrimitive
  {IR(33914, 14), 265}, // 220 EnqueueKernel
  {IR(33900, 14), 264}, // 221 EnqueueMarker
  {IR(37569, 24), 429}, // 222 EnqueueNodePayloadsAMDX
  {IR(29828, 11), 13}, // 223 EntryPoint
  {IR(36285, 19), 379}, // 224 ExecuteCallableKHR
  {IR(40416, 18), 534}, // 225 ExecuteCallableNV
  {IR(29871, 14), 14}, // 226 ExecutionMode
  {IR(34792, 16), 304}, // 227 ExecutionModeId
  {IR(42672, 10), 623}, // 228 ExpectKHR
  {IR(29682, 8), 11}, // 229 ExtInst
  {IR(29622, 14), 10}, // 230 ExtInstImport
  {IR(36222, 26), 376}, // 231 ExtInstWithForwardRefsKHR
  {IR(29612, 10), 9}, // 232 Extension
  {IR(37273, 20), 415}, // 233 ExtractSubArrayQCOM
  {IR(31926, 5), 118}, // 234 FAdd
  {IR(31757, 9), 105}, // 235 FConvert
  {IR(31961, 5), 125}, // 236 FDiv
  {IR(52077, 19), 874}, // 237 FDot2MixAcc16VALVE
  {IR(52058, 19), 873}, // 238 FDot2MixAcc32VALVE
  {IR(52096, 19), 875}, // 239 FDot4MixAcc32VALVE
  {IR(31986, 5), 130}, // 240 FMod
  {IR(31946, 5), 122}, // 241 FMul
  {IR(31913, 8), 116}, // 242 FNegate
  {IR(32423, 10), 168}, // 243 FOrdEqual
  {IR(32501, 16), 174}, // 244 FOrdGreaterThan
  {IR(32573, 21), 178}, // 245 FOrdGreaterThanEqual
  {IR(32473, 13), 172}, // 246 FOrdLessThan
  {IR(32535, 18), 176}, // 247 FOrdLessThanEqual
  {IR(32445, 13), 170}, // 248 FOrdNotEqual
  {IR(15223, 14), 807}, // 249 FPGARegALTERA
  {IR(15237, 13), 807}, // 250 FPGARegINTEL
  {IR(31981, 5), 129}, // 251 FRem
  {IR(31936, 5), 120}, // 252 FSub
  {IR(32433, 12), 169}, // 253 FUnordEqual
  {IR(32517, 18), 175}, // 254 FUnordGreaterThan
  {IR(32594, 23), 179}, // 255 FUnordGreaterThanEqual
  {IR(32486, 15), 173}, // 256 FUnordLessThan
  {IR(32553, 20), 177}, // 257 FUnordLessThanEqual
  {IR(32458, 15), 171}, // 258 FUnordNotEqual
  {IR(39226, 38), 493}, // 259 FetchMicroTriangleVertexBarycentricNV
  {IR(39191, 35), 492}, // 260 FetchMicroTriangleVertexPositionNV
  {IR(37618, 29), 431}, // 261 FinishWritingNodePayloadAMDX
  {IR(49629, 15), 796}, // 262 FixedCosALTERA
  {IR(49644, 14), 796}, // 263 FixedCosINTEL
  {IR(49726, 17), 799}, // 264 FixedCosPiALTERA
  {IR(49743, 16), 799}, // 265 FixedCosPiINTEL
  {IR(49827, 15), 802}, // 266 FixedExpALTERA
  {IR(49842, 14), 802}, // 267 FixedExpINTEL
  {IR(49798, 15), 801}, // 268 FixedLogALTERA
  {IR(49813, 14), 801}, // 269 FixedLogINTEL
  {IR(49534, 17), 793}, // 270 FixedRecipALTERA
  {IR(49551, 16), 793}, // 271 FixedRecipINTEL
  {IR(49567, 17), 794}, // 272 FixedRsqrtALTERA
  {IR(49584, 16), 794}, // 273 FixedRsqrtINTEL
  {IR(49600, 15), 795}, // 274 FixedSinALTERA
  {IR(49658, 18), 797}, // 275 FixedSinCosALTERA
  {IR(49676, 17), 797}, // 276 FixedSinCosINTEL
  {IR(49759, 20), 800}, // 277 FixedSinCosPiALTERA
  {IR(49779, 19), 800}, // 278 FixedSinCosPiINTEL
  {IR(49615, 14), 795}, // 279 FixedSinINTEL
  {IR(49693, 17), 798}, // 280 FixedSinPiALTERA
  {IR(49710, 16), 798}, // 281 FixedSinPiINTEL
  {IR(49503, 16), 792}, // 282 FixedSqrtALTERA
  {IR(49519, 15), 792}, // 283 FixedSqrtINTEL
  {IR(36139, 7), 370}, // 284 FmaKHR
  {IR(37514, 17), 426}, // 285 FragmentFetchAMD
  {IR(37493, 21), 425}, // 286 FragmentMaskFetchAMD
  {IR(37918, 10), 447}, // 287 FreezeKHR
  {IR(28661, 9), 48}, // 288 Function
  {IR(30734, 13), 51}, // 289 FunctionCall
  {IR(30722, 12), 50}, // 290 FunctionEnd
  {IR(30704, 18), 49}, // 291 FunctionParameter
  {IR(42577, 25), 616}, // 292 FunctionPointerCallINTEL
  {IR(32788, 7), 194}, // 293 Fwidth
  {IR(32846, 13), 200}, // 294 FwidthCoarse
  {IR(32813, 11), 197}, // 295 FwidthFine
  {IR(31855, 17), 112}, // 296 GenericCastToPtr
  {IR(31872, 25), 113}, // 297 GenericCastToPtrExplicit
  {IR(30911, 23), 62}, // 298 GenericPtrMemSemantics
  {IR(34152, 16), 276}, // 299 GetDefaultQueue
  {IR(34657, 35), 298}, // 300 GetKernelLocalSizeForSubgroupCount
  {IR(34692, 25), 299}, // 301 GetKernelMaxNumSubgroups
  {IR(33958, 32), 267}, // 302 GetKernelNDrangeMaxSubGroupSize
  {IR(33928, 30), 266}, // 303 GetKernelNDrangeSubGroupCount
  {IR(34013, 40), 269}, // 304 GetKernelPreferredWorkGroupSizeMultiple
  {IR(33990, 23), 268}, // 305 GetKernelWorkGroupSize
  {IR(33784, 18), 259}, // 306 GetMaxPipePackets
  {IR(33766, 18), 258}, // 307 GetNumPipePackets
  {IR(10317, 9), 353}, // 308 GraphARM
  {IR(35807, 17), 351}, // 309 GraphConstantARM
  {IR(35875, 12), 356}, // 310 GraphEndARM
  {IR(35824, 19), 352}, // 311 GraphEntryPointARM
  {IR(35843, 14), 354}, // 312 GraphInputARM
  {IR(35857, 18), 355}, // 313 GraphSetOutputARM
  {IR(33471, 9), 238}, // 314 GroupAll
  {IR(33480, 9), 239}, // 315 GroupAny
  {IR(33440, 15), 236}, // 316 GroupAsyncCopy
  {IR(51804, 19), 861}, // 317 GroupBitwiseAndKHR
  {IR(51823, 18), 862}, // 318 GroupBitwiseOrKHR
  {IR(51841, 19), 863}, // 319 GroupBitwiseXorKHR
  {IR(33489, 15), 240}, // 320 GroupBroadcast
  {IR(33859, 20), 262}, // 321 GroupCommitReadPipe
  {IR(33879, 21), 263}, // 322 GroupCommitWritePipe
  {IR(31025, 14), 67}, // 323 GroupDecorate
  {IR(33547, 10), 242}, // 324 GroupFAdd
  {IR(37332, 23), 418}, // 325 GroupFAddNonUniformAMD
  {IR(33587, 10), 246}, // 326 GroupFMax
  {IR(37424, 23), 422}, // 327 GroupFMaxNonUniformAMD
  {IR(33557, 10), 243}, // 328 GroupFMin
  {IR(37355, 23), 419}, // 329 GroupFMinNonUniformAMD
  {IR(51791, 13), 860}, // 330 GroupFMulKHR
  {IR(33537, 10), 241}, // 331 GroupIAdd
  {IR(37309, 23), 417}, // 332 GroupIAddNonUniformAMD
  {IR(51778, 13), 859}, // 333 GroupIMulKHR
  {IR(51860, 19), 864}, // 334 GroupLogicalAndKHR
  {IR(51879, 18), 865}, // 335 GroupLogicalOrKHR
  {IR(51897, 19), 866}, // 336 GroupLogicalXorKHR
  {IR(31084, 20), 68}, // 337 GroupMemberDecorate
  {IR(34840, 19), 307}, // 338 GroupNonUniformAll
  {IR(34878, 24), 309}, // 339 GroupNonUniformAllEqual
  {IR(34859, 19), 308}, // 340 GroupNonUniformAny
  {IR(6887, 22), 312}, // 341 GroupNonUniformBallot
  {IR(35018, 30), 315}, // 342 GroupNonUniformBallotBitCount
  {IR(34986, 32), 314}, // 343 GroupNonUniformBallotBitExtract
  {IR(35048, 29), 316}, // 344 GroupNonUniformBallotFindLSB
  {IR(35077, 29), 317}, // 345 GroupNonUniformBallotFindMSB
  {IR(35384, 26), 332}, // 346 GroupNonUniformBitwiseAnd
  {IR(35410, 25), 333}, // 347 GroupNonUniformBitwiseOr
  {IR(35435, 26), 334}, // 348 GroupNonUniformBitwiseXor
  {IR(34902, 25), 310}, // 349 GroupNonUniformBroadcast
  {IR(34927, 30), 311}, // 350 GroupNonUniformBroadcastFirst
  {IR(34819, 21), 306}, // 351 GroupNonUniformElect
  {IR(35204, 20), 323}, // 352 GroupNonUniformFAdd
  {IR(35364, 20), 331}, // 353 GroupNonUniformFMax
  {IR(35304, 20), 328}, // 354 GroupNonUniformFMin
  {IR(35244, 20), 325}, // 355 GroupNonUniformFMul
  {IR(35184, 20), 322}, // 356 GroupNonUniformIAdd
  {IR(35224, 20), 324}, // 357 GroupNonUniformIMul
  {IR(34957, 29), 313}, // 358 GroupNonUniformInverseBallot
  {IR(35461, 26), 335}, // 359 GroupNonUniformLogicalAnd
  {IR(35487, 25), 336}, // 360 GroupNonUniformLogicalOr
  {IR(35512, 26), 337}, // 361 GroupNonUniformLogicalXor
  {IR(39103, 28), 490}, // 362 GroupNonUniformPartitionEXT
  {IR(39131, 27), 490}, // 363 GroupNonUniformPartitionNV
  {IR(37739, 26), 436}, // 364 GroupNonUniformQuadAllKHR
  {IR(37765, 26), 437}, // 365 GroupNonUniformQuadAnyKHR
  {IR(35538, 29), 338}, // 366 GroupNonUniformQuadBroadcast
  {IR(35567, 24), 339}, // 367 GroupNonUniformQuadSwap
  {IR(15462, 25), 374}, // 368 GroupNonUniformRotateKHR
  {IR(35324, 20), 329}, // 369 GroupNonUniformSMax
  {IR(35264, 20), 326}, // 370 GroupNonUniformSMin
  {IR(10026, 23), 318}, // 371 GroupNonUniformShuffle
  {IR(35157, 27), 321}, // 372 GroupNonUniformShuffleDown
  {IR(35132, 25), 320}, // 373 GroupNonUniformShuffleUp
  {IR(35106, 26), 319}, // 374 GroupNonUniformShuffleXor
  {IR(35344, 20), 330}, // 375 GroupNonUniformUMax
  {IR(35284, 20), 327}, // 376 GroupNonUniformUMin
  {IR(33802, 28), 260}, // 377 GroupReserveReadPipePackets
  {IR(33830, 29), 261}, // 378 GroupReserveWritePipePackets
  {IR(33607, 10), 248}, // 379 GroupSMax
  {IR(37470, 23), 424}, // 380 GroupSMaxNonUniformAMD
  {IR(33577, 10), 245}, // 381 GroupSMin
  {IR(37401, 23), 421}, // 382 GroupSMinNonUniformAMD
  {IR(33597, 10), 247}, // 383 GroupUMax
  {IR(37447, 23), 423}, // 384 GroupUMaxNonUniformAMD
  {IR(33567, 10), 244}, // 385 GroupUMin
  {IR(37378, 23), 420}, // 386 GroupUMinNonUniformAMD
  {IR(33455, 16), 237}, // 387 GroupWaitEvents
  {IR(39760, 26), 511}, // 388 HitObjectExecuteShaderEXT
  {IR(38363, 25), 463}, // 389 HitObjectExecuteShaderNV
  {IR(39813, 26), 513}, // 390 HitObjectGetAttributesEXT
  {IR(38414, 25), 465}, // 391 HitObjectGetAttributesNV
  {IR(40492, 24), 536}, // 392 HitObjectGetClusterIdNV
  {IR(39786, 27), 512}, // 393 HitObjectGetCurrentTimeEXT
  {IR(38388, 26), 464}, // 394 HitObjectGetCurrentTimeNV
  {IR(39892, 29), 516}, // 395 HitObjectGetGeometryIndexEXT
  {IR(38490, 28), 468}, // 396 HitObjectGetGeometryIndexNV
  {IR(39839, 23), 514}, // 397 HitObjectGetHitKindEXT
  {IR(38439, 22), 466}, // 398 HitObjectGetHitKindNV
  {IR(39947, 35), 518}, // 399 HitObjectGetInstanceCustomIndexEXT
  {IR(38543, 34), 470}, // 400 HitObjectGetInstanceCustomIndexNV
  {IR(39921, 26), 517}, // 401 HitObjectGetInstanceIdEXT
  {IR(38518, 25), 469}, // 402 HitObjectGetInstanceIdNV
  {IR(39393, 51), 499}, // 403 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(41944, 27), 585}, // 404 HitObjectGetLSSPositionsNV
  {IR(41971, 23), 586}, // 405 HitObjectGetLSSRadiiNV
  {IR(40013, 34), 520}, // 406 HitObjectGetObjectRayDirectionEXT
  {IR(38075, 33), 453}, // 407 HitObjectGetObjectRayDirectionNV
  {IR(39982, 31), 519}, // 408 HitObjectGetObjectRayOriginEXT
  {IR(38108, 30), 454}, // 409 HitObjectGetObjectRayOriginNV
  {IR(40110, 29), 523}, // 410 HitObjectGetObjectToWorldEXT
  {IR(38047, 28), 452}, // 411 HitObjectGetObjectToWorldNV
  {IR(39862, 30), 515}, // 412 HitObjectGetPrimitiveIndexEXT
  {IR(38461, 29), 467}, // 413 HitObjectGetPrimitiveIndexNV
  {IR(39444, 24), 500}, // 414 HitObjectGetRayFlagsEXT
  {IR(40168, 23), 525}, // 415 HitObjectGetRayTMaxEXT
  {IR(38638, 22), 473}, // 416 HitObjectGetRayTMaxNV
  {IR(40516, 23), 537}, // 417 HitObjectGetRayTMinEXT
  {IR(38660, 22), 474}, // 418 HitObjectGetRayTMinNV
  {IR(40539, 45), 538}, // 419 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(38203, 44), 457}, // 420 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(40584, 40), 539}, // 421 HitObjectGetShaderRecordBufferHandleEXT
  {IR(38164, 39), 456}, // 422 HitObjectGetShaderRecordBufferHandleNV
  {IR(41888, 29), 583}, // 423 HitObjectGetSpherePositionNV
  {IR(41917, 27), 584}, // 424 HitObjectGetSphereRadiusNV
  {IR(40047, 33), 521}, // 425 HitObjectGetWorldRayDirectionEXT
  {IR(38577, 32), 471}, // 426 HitObjectGetWorldRayDirectionNV
  {IR(40080, 30), 522}, // 427 HitObjectGetWorldRayOriginEXT
  {IR(38609, 29), 472}, // 428 HitObjectGetWorldRayOriginNV
  {IR(40139, 29), 524}, // 429 HitObjectGetWorldToObjectEXT
  {IR(38019, 28), 451}, // 430 HitObjectGetWorldToObjectNV
  {IR(40624, 20), 540}, // 431 HitObjectIsEmptyEXT
  {IR(38682, 19), 475}, // 432 HitObjectIsEmptyNV
  {IR(40644, 18), 541}, // 433 HitObjectIsHitEXT
  {IR(38701, 17), 476}, // 434 HitObjectIsHitNV
  {IR(42017, 20), 588}, // 435 HitObjectIsLSSHitNV
  {IR(40662, 19), 542}, // 436 HitObjectIsMissEXT
  {IR(38718, 18), 477}, // 437 HitObjectIsMissNV
  {IR(41994, 23), 587}, // 438 HitObjectIsSphereHitNV
  {IR(39736, 24), 510}, // 439 HitObjectRecordEmptyEXT
  {IR(38247, 23), 458}, // 440 HitObjectRecordEmptyNV
  {IR(39313, 28), 496}, // 441 HitObjectRecordFromQueryEXT
  {IR(37928, 27), 448}, // 442 HitObjectRecordHitMotionNV
  {IR(38290, 21), 460}, // 443 HitObjectRecordHitNV
  {IR(37955, 36), 449}, // 444 HitObjectRecordHitWithIndexMotionNV
  {IR(38311, 30), 461}, // 445 HitObjectRecordHitWithIndexNV
  {IR(39341, 23), 497}, // 446 HitObjectRecordMissEXT
  {IR(39364, 29), 498}, // 447 HitObjectRecordMissMotionEXT
  {IR(37991, 28), 450}, // 448 HitObjectRecordMissMotionNV
  {IR(38341, 22), 462}, // 449 HitObjectRecordMissNV
  {IR(39513, 33), 502}, // 450 HitObjectReorderExecuteShaderEXT
  {IR(39468, 45), 501}, // 451 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(39578, 38), 504}, // 452 HitObjectTraceMotionReorderExecuteEXT
  {IR(39688, 21), 508}, // 453 HitObjectTraceRayEXT
  {IR(39709, 27), 509}, // 454 HitObjectTraceRayMotionEXT
  {IR(38138, 26), 455}, // 455 HitObjectTraceRayMotionNV
  {IR(38270, 20), 459}, // 456 HitObjectTraceRayNV
  {IR(39546, 32), 503}, // 457 HitObjectTraceReorderExecuteEXT
  {IR(31921, 5), 117}, // 458 IAdd
  {IR(32098, 10), 138}, // 459 IAddCarry
  {IR(42396, 13), 605}, // 460 IAddSatINTEL
  {IR(42422, 14), 607}, // 461 IAverageINTEL
  {IR(42450, 21), 609}, // 462 IAverageRoundedINTEL
  {IR(32294, 7), 158}, // 463 IEqual
  {IR(31941, 5), 121}, // 464 IMul
  {IR(42518, 15), 613}, // 465 IMul32x16INTEL
  {IR(32301, 10), 159}, // 466 INotEqual
  {IR(31931, 5), 119}, // 467 ISub
  {IR(32108, 11), 139}, // 468 ISubBorrow
  {IR(42492, 13), 611}, // 469 ISubSatINTEL
  {IR(36339, 22), 381}, // 470 IgnoreIntersectionKHR
  {IR(40234, 21), 527}, // 471 IgnoreIntersectionNV
  {IR(28705, 6), 91}, // 472 Image
  {IR(37186, 29), 412}, // 473 ImageBlockMatchGatherSADQCOM
  {IR(37157, 29), 411}, // 474 ImageBlockMatchGatherSSDQCOM
  {IR(37059, 23), 407}, // 475 ImageBlockMatchSADQCOM
  {IR(37036, 23), 406}, // 476 ImageBlockMatchSSDQCOM
  {IR(37128, 29), 410}, // 477 ImageBlockMatchWindowSADQCOM
  {IR(37099, 29), 409}, // 478 ImageBlockMatchWindowSSDQCOM
  {IR(37017, 19), 405}, // 479 ImageBoxFilterQCOM
  {IR(31539, 16), 88}, // 480 ImageDrefGather
  {IR(31516, 11), 86}, // 481 ImageFetch
  {IR(31527, 12), 87}, // 482 ImageGather
  {IR(37293, 16), 416}, // 483 ImageGatherQCOM
  {IR(31576, 17), 92}, // 484 ImageQueryFormat
  {IR(31656, 17), 97}, // 485 ImageQueryLevels
  {IR(31642, 14), 96}, // 486 ImageQueryLod
  {IR(31593, 16), 93}, // 487 ImageQueryOrder
  {IR(31673, 18), 98}, // 488 ImageQuerySamples
  {IR(31627, 15), 95}, // 489 ImageQuerySize
  {IR(31609, 18), 94}, // 490 ImageQuerySizeLod
  {IR(31555, 10), 89}, // 491 ImageRead
  {IR(31373, 27), 81}, // 492 ImageSampleDrefExplicitLod
  {IR(31346, 27), 80}, // 493 ImageSampleDrefImplicitLod
  {IR(31323, 23), 79}, // 494 ImageSampleExplicitLod
  {IR(38805, 23), 481}, // 495 ImageSampleFootprintNV
  {IR(31277, 23), 78}, // 496 ImageSampleImplicitLod
  {IR(31485, 31), 85}, // 497 ImageSampleProjDrefExplicitLod
  {IR(31454, 31), 84}, // 498 ImageSampleProjDrefImplicitLod
  {IR(31427, 27), 83}, // 499 ImageSampleProjExplicitLod
  {IR(31400, 27), 82}, // 500 ImageSampleProjImplicitLod
  {IR(36993, 24), 404}, // 501 ImageSampleWeightedQCOM
  {IR(34480, 22), 288}, // 502 ImageSparseDrefGather
  {IR(34445, 17), 286}, // 503 ImageSparseFetch
  {IR(34462, 18), 287}, // 504 ImageSparseGather
  {IR(34572, 16), 293}, // 505 ImageSparseRead
  {IR(34272, 33), 281}, // 506 ImageSparseSampleDrefExplicitLod
  {IR(34239, 33), 280}, // 507 ImageSparseSampleDrefImplicitLod
  {IR(34210, 29), 279}, // 508 ImageSparseSampleExplicitLod
  {IR(34181, 29), 278}, // 509 ImageSparseSampleImplicitLod
  {IR(34408, 37), 285}, // 510 ImageSparseSampleProjDrefExplicitLod
  {IR(34371, 37), 284}, // 511 ImageSparseSampleProjDrefImplicitLod
  {IR(34338, 33), 283}, // 512 ImageSparseSampleProjExplicitLod
  {IR(34305, 33), 282}, // 513 ImageSparseSampleProjImplicitLod
  {IR(34502, 26), 289}, // 514 ImageSparseTexelsResident
  {IR(30756, 18), 53}, // 515 ImageTexelPointer
  {IR(31565, 11), 90}, // 516 ImageWrite
  {IR(30864, 20), 59}, // 517 InBoundsAccessChain
  {IR(30934, 23), 63}, // 518 InBoundsPtrAccessChain
  {IR(32165, 9), 146}, // 519 IsFinite
  {IR(41379, 22), 566}, // 520 IsHelperInvocationEXT
  {IR(32159, 6), 145}, // 521 IsInf
  {IR(32153, 6), 144}, // 522 IsNan
  {IR(37674, 23), 433}, // 523 IsNodePayloadValidAMDX
  {IR(32174, 9), 147}, // 524 IsNormal
  {IR(34094, 13), 273}, // 525 IsValidEvent
  {IR(33749, 17), 257}, // 526 IsValidReserveId
  {IR(33377, 5), 230}, // 527 Kill
  {IR(33294, 6), 226}, // 528 Label
  {IR(32194, 14), 149}, // 529 LessOrGreater
  {IR(33413, 14), 234}, // 530 LifetimeStart
  {IR(33427, 13), 235}, // 531 LifetimeStop
  {IR(29607, 5), 8}, // 532 Line
  {IR(30814, 5), 54}, // 533 Load
  {IR(32265, 11), 155}, // 534 LogicalAnd
  {IR(32226, 13), 152}, // 535 LogicalEqual
  {IR(32276, 11), 156}, // 536 LogicalNot
  {IR(32239, 16), 153}, // 537 LogicalNotEqual
  {IR(32255, 10), 154}, // 538 LogicalOr
  {IR(49421, 17), 788}, // 539 LoopControlINTEL
  {IR(33234, 10), 224}, // 540 LoopMerge
  {IR(51934, 18), 868}, // 541 MaskedGatherINTEL
  {IR(51952, 19), 869}, // 542 MaskedScatterINTEL
  {IR(32063, 18), 135}, // 543 MatrixTimesMatrix
  {IR(32009, 18), 132}, // 544 MatrixTimesScalar
  {IR(32045, 18), 134}, // 545 MatrixTimesVector
  {IR(30994, 15), 65}, // 546 MemberDecorate
  {IR(37850, 20), 442}, // 547 MemberDecorateIdEXT
  {IR(42718, 21), 625}, // 548 MemberDecorateString
  {IR(42739, 27), 625}, // 549 MemberDecorateStringGOOGLE
  {IR(29589, 11), 6}, // 550 MemberName
  {IR(32971, 14), 206}, // 551 MemoryBarrier
  {IR(29754, 12), 12}, // 552 MemoryModel
  {IR(34757, 19), 302}, // 553 MemoryNamedBarrier
  {IR(34776, 16), 303}, // 554 ModuleProcessed
  {IR(29584, 5), 5}, // 555 Name
  {IR(34734, 23), 301}, // 556 NamedBarrierInitialize
  {IR(34528, 7), 290}, // 557 NoLine
  {IR(37647, 27), 432}, // 558 NodePayloadArrayLengthAMDX
  {IR(29380, 4), 0}, // 559 Nop
  {IR(32705, 4), 186}, // 560 Not
  {IR(32208, 8), 150}, // 561 Ordered
  {IR(32081, 13), 136}, // 562 OuterProduct
  {IR(33200, 4), 223}, // 563 Phi
  {IR(37908, 10), 446}, // 564 PoisonKHR
  {IR(51737, 20), 857}, // 565 PredicatedLoadINTEL
  {IR(51757, 21), 858}, // 566 PredicatedStoreINTEL
  {IR(30884, 15), 60}, // 567 PtrAccessChain
  {IR(49856, 30), 803}, // 568 PtrCastToCrossWorkgroupALTERA
  {IR(49886, 29), 803}, // 569 PtrCastToCrossWorkgroupINTEL
  {IR(31838, 17), 111}, // 570 PtrCastToGeneric
  {IR(35624, 8), 343}, // 571 PtrDiff
  {IR(35603, 9), 341}, // 572 PtrEqual
  {IR(35612, 12), 342}, // 573 PtrNotEqual
  {IR(31766, 14), 106}, // 574 QuantizeToF16
  {IR(41684, 17), 577}, // 575 RawAccessChainNV
  {IR(36912, 31), 401}, // 576 RayQueryConfirmIntersectionKHR
  {IR(36880, 32), 400}, // 577 RayQueryGenerateIntersectionKHR
  {IR(40469, 23), 535}, // 578 RayQueryGetClusterIdNV
  {IR(50368, 39), 816}, // 579 RayQueryGetIntersectionBarycentricsKHR
  {IR(50443, 46), 818}, // 580 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(40434, 35), 535}, // 581 RayQueryGetIntersectionClusterIdNV
  {IR(50407, 36), 817}, // 582 RayQueryGetIntersectionFrontFaceKHR
  {IR(50287, 40), 814}, // 583 RayQueryGetIntersectionGeometryIndexKHR
  {IR(50139, 46), 811}, // 584 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(50185, 37), 812}, // 585 RayQueryGetIntersectionInstanceIdKHR
  {IR(50222, 65), 813}, // 586 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(41851, 37), 582}, // 587 RayQueryGetIntersectionLSSHitValueNV
  {IR(41779, 38), 580}, // 588 RayQueryGetIntersectionLSSPositionsNV
  {IR(41817, 34), 581}, // 589 RayQueryGetIntersectionLSSRadiiNV
  {IR(50489, 45), 819}, // 590 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(50534, 42), 820}, // 591 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(50637, 40), 823}, // 592 RayQueryGetIntersectionObjectToWorldKHR
  {IR(50327, 41), 815}, // 593 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(41701, 40), 578}, // 594 RayQueryGetIntersectionSpherePositionNV
  {IR(41741, 38), 579}, // 595 RayQueryGetIntersectionSphereRadiusNV
  {IR(50111, 28), 810}, // 596 RayQueryGetIntersectionTKHR
  {IR(40309, 50), 532}, // 597 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(36962, 31), 403}, // 598 RayQueryGetIntersectionTypeKHR
  {IR(50677, 40), 824}, // 599 RayQueryGetIntersectionWorldToObjectKHR
  {IR(50088, 23), 809}, // 600 RayQueryGetRayFlagsKHR
  {IR(50066, 22), 808}, // 601 RayQueryGetRayTMinKHR
  {IR(50576, 32), 821}, // 602 RayQueryGetWorldRayDirectionKHR
  {IR(50608, 29), 822}, // 603 RayQueryGetWorldRayOriginKHR
  {IR(36837, 22), 398}, // 604 RayQueryInitializeKHR
  {IR(42059, 19), 590}, // 605 RayQueryIsLSSHitNV
  {IR(42037, 22), 589}, // 606 RayQueryIsSphereHitNV
  {IR(36943, 19), 402}, // 607 RayQueryProceedKHR
  {IR(36859, 21), 399}, // 608 RayQueryTerminateKHR
  {IR(37531, 13), 427}, // 609 ReadClockKHR
  {IR(33617, 9), 249}, // 610 ReadPipe
  {IR(49974, 23), 805}, // 611 ReadPipeBlockingALTERA
  {IR(49997, 22), 805}, // 612 ReadPipeBlockingINTEL
  {IR(34065, 13), 271}, // 613 ReleaseEvent
  {IR(39633, 25), 506}, // 614 ReorderThreadWithHintEXT
  {IR(38765, 24), 479}, // 615 ReorderThreadWithHintNV
  {IR(39658, 30), 507}, // 616 ReorderThreadWithHitObjectEXT
  {IR(38736, 29), 478}, // 617 ReorderThreadWithHitObjectNV
  {IR(40191, 22), 526}, // 618 ReportIntersectionKHR
  {IR(40213, 21), 526}, // 619 ReportIntersectionNV
  {IR(33671, 23), 253}, // 620 ReserveReadPipePackets
  {IR(33694, 24), 254}, // 621 ReserveWritePipePackets
  {IR(33636, 17), 251}, // 622 ReservedReadPipe
  {IR(33653, 18), 252}, // 623 ReservedWritePipe
  {IR(47948, 19), 746}, // 624 RestoreMemoryINTEL
  {IR(34053, 12), 270}, // 625 RetainEvent
  {IR(33382, 7), 231}, // 626 Return
  {IR(33389, 12), 232}, // 627 ReturnValue
  {IR(51916, 18), 867}, // 628 RoundFToTF32INTEL
  {IR(31748, 9), 104}, // 629 SConvert
  {IR(31956, 5), 124}, // 630 SDiv
  {IR(36424, 5), 383}, // 631 SDot
  {IR(36465, 11), 386}, // 632 SDotAccSat
  {IR(36476, 14), 386}, // 633 SDotAccSatKHR
  {IR(36429, 8), 383}, // 634 SDotKHR
  {IR(32324, 13), 161}, // 635 SGreaterThan
  {IR(32355, 18), 163}, // 636 SGreaterThanEqual
  {IR(32383, 10), 165}, // 637 SLessThan
  {IR(32408, 15), 167}, // 638 SLessThanEqual
  {IR(31976, 5), 128}, // 639 SMod
  {IR(32132, 13), 141}, // 640 SMulExtended
  {IR(31905, 8), 115}, // 641 SNegate
  {IR(31971, 5), 127}, // 642 SRem
  {IR(36450, 6), 385}, // 643 SUDot
  {IR(36515, 12), 388}, // 644 SUDotAccSat
  {IR(36527, 15), 388}, // 645 SUDotAccSatKHR
  {IR(36456, 9), 385}, // 646 SUDotKHR
  {IR(31232, 13), 77}, // 647 SampledImage
  {IR(41603, 29), 576}, // 648 SamplerImageAddressingModeNV
  {IR(31794, 15), 108}, // 649 SatConvertSToU
  {IR(31809, 15), 109}, // 650 SatConvertUToS
  {IR(47932, 16), 745}, // 651 SaveMemoryINTEL
  {IR(32287, 7), 157}, // 652 Select
  {IR(33279, 15), 225}, // 653 SelectionMerge
  {IR(39085, 18), 489}, // 654 SetMeshOutputsEXT
  {IR(34107, 19), 274}, // 655 SetUserEventStatus
  {IR(32656, 17), 182}, // 656 ShiftLeftLogical
  {IR(32635, 21), 181}, // 657 ShiftRightArithmetic
  {IR(32617, 18), 180}, // 658 ShiftRightLogical
  {IR(32183, 11), 148}, // 659 SignBitSet
  {IR(34588, 7), 294}, // 660 SizeOf
  {IR(29561, 7), 3}, // 661 Source
  {IR(29442, 16), 2}, // 662 SourceContinued
  {IR(29568, 16), 4}, // 663 SourceExtension
  {IR(30579, 13), 45}, // 664 SpecConstant
  {IR(51613, 30), 854}, // 665 SpecConstantArchitectureINTEL
  {IR(51680, 30), 855}, // 666 SpecConstantCapabilitiesINTEL
  {IR(30592, 22), 46}, // 667 SpecConstantComposite
  {IR(50811, 36), 829}, // 668 SpecConstantCompositeContinuedINTEL
  {IR(36756, 34), 395}, // 669 SpecConstantCompositeReplicateEXT
  {IR(37888, 20), 445}, // 670 SpecConstantDataKHR
  {IR(30561, 18), 44}, // 671 SpecConstantFalse
  {IR(30655, 15), 47}, // 672 SpecConstantOp
  {IR(37716, 23), 435}, // 673 SpecConstantStringAMDX
  {IR(51589, 24), 853}, // 674 SpecConstantTargetINTEL
  {IR(30544, 17), 43}, // 675 SpecConstantTrue
  {IR(35678, 25), 346}, // 676 StencilAttachmentReadEXT
  {IR(30819, 6), 55}, // 677 Store
  {IR(29600, 7), 7}, // 678 String
  {IR(51278, 25), 842}, // 679 Subgroup2DBlockLoadINTEL
  {IR(51303, 34), 843}, // 680 Subgroup2DBlockLoadTransformINTEL
  {IR(51337, 34), 844}, // 681 Subgroup2DBlockLoadTransposeINTEL
  {IR(51371, 29), 845}, // 682 Subgroup2DBlockPrefetchINTEL
  {IR(51400, 26), 846}, // 683 Subgroup2DBlockStoreINTEL
  {IR(36176, 20), 373}, // 684 SubgroupAllEqualKHR
  {IR(36146, 15), 371}, // 685 SubgroupAllKHR
  {IR(36161, 15), 372}, // 686 SubgroupAnyKHR
  {IR(46450, 30), 709}, // 687 SubgroupAvcBmeInitializeINTEL
  {IR(46420, 30), 708}, // 688 SubgroupAvcFmeInitializeINTEL
  {IR(44855, 35), 678}, // 689 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(44890, 39), 679}, // 690 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(45531, 38), 692}, // 691 SubgroupAvcImeConvertToMceResultINTEL
  {IR(45154, 45), 685}, // 692 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(45254, 53), 687}, // 693 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(45475, 56), 691}, // 694 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(45363, 54), 689}, // 695 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(45107, 47), 684}, // 696 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(45199, 55), 686}, // 697 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(45417, 58), 690}, // 698 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(45307, 56), 688}, // 699 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(46163, 36), 703}, // 700 SubgroupAvcImeGetBorderReachedINTEL
  {IR(45615, 44), 694}, // 701 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(45569, 46), 693}, // 702 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(46030, 66), 701}, // 703 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(45962, 68), 700}, // 704 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(46096, 67), 702}, // 705 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(45825, 68), 698}, // 706 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(45755, 70), 697}, // 707 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(45893, 69), 699}, // 708 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(46199, 48), 704}, // 709 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(46247, 59), 705}, // 710 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(46364, 56), 707}, // 711 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(46306, 58), 706}, // 712 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(44718, 30), 674}, // 713 SubgroupAvcImeInitializeINTEL
  {IR(44822, 33), 677}, // 714 SubgroupAvcImeRefWindowSizeINTEL
  {IR(44786, 36), 676}, // 715 SubgroupAvcImeSetDualReferenceINTEL
  {IR(45019, 54), 682}, // 716 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(44929, 43), 680}, // 717 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(44748, 38), 675}, // 718 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(44972, 47), 681}, // 719 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(45073, 34), 683}, // 720 SubgroupAvcImeSetWeightedSadINTEL
  {IR(45708, 47), 696}, // 721 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(45659, 49), 695}, // 722 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(44108, 39), 659}, // 723 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(44147, 38), 660}, // 724 SubgroupAvcMceConvertToImeResultINTEL
  {IR(44185, 39), 661}, // 725 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(44224, 38), 662}, // 726 SubgroupAvcMceConvertToRefResultINTEL
  {IR(44262, 39), 663}, // 727 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(44301, 38), 664}, // 728 SubgroupAvcMceConvertToSicResultINTEL
  {IR(44414, 43), 667}, // 729 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(43546, 50), 648}, // 730 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(43144, 60), 640}, // 731 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(43344, 51), 644}, // 732 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(43490, 56), 647}, // 733 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(43257, 47), 642}, // 734 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(43845, 56), 654}, // 735 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(43744, 50), 652}, // 736 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(43439, 51), 646}, // 737 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(43648, 49), 650}, // 738 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(43596, 52), 649}, // 739 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(43794, 51), 653}, // 740 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(44533, 38), 670}, // 741 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(44375, 39), 666}, // 742 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(44457, 38), 668}, // 743 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(44495, 38), 669}, // 744 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(44571, 45), 671}, // 745 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(44616, 40), 672}, // 746 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(44656, 62), 673}, // 747 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(44339, 36), 665}, // 748 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(43901, 33), 655}, // 749 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(44047, 61), 658}, // 750 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(43204, 53), 641}, // 751 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(43395, 44), 645}, // 752 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(43304, 40), 643}, // 753 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(43697, 47), 651}, // 754 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(43986, 61), 657}, // 755 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(43934, 52), 656}, // 756 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(46480, 39), 710}, // 757 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(46802, 38), 717}, // 758 SubgroupAvcRefConvertToMceResultINTEL
  {IR(46655, 45), 714}, // 759 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(46700, 46), 715}, // 760 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(46746, 56), 716}, // 761 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46608, 47), 713}, // 762 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(46519, 46), 711}, // 763 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(46565, 43), 712}, // 764 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(46938, 42), 721}, // 765 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(46902, 36), 720}, // 766 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(46870, 32), 719}, // 767 SubgroupAvcSicConfigureSkcINTEL
  {IR(47019, 39), 723}, // 768 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(47559, 38), 735}, // 769 SubgroupAvcSicConvertToMceResultINTEL
  {IR(47334, 31), 730}, // 770 SubgroupAvcSicEvaluateIpeINTEL
  {IR(47412, 45), 732}, // 771 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(47457, 46), 733}, // 772 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(47503, 56), 734}, // 773 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(47365, 47), 731}, // 774 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(47676, 46), 738}, // 775 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(47632, 44), 737}, // 776 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(47897, 35), 743}, // 777 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(47763, 36), 740}, // 778 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(47597, 35), 736}, // 779 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(46980, 39), 722}, // 780 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(47722, 41), 739}, // 781 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(47799, 50), 741}, // 782 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(47849, 48), 742}, // 783 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(46840, 30), 718}, // 784 SubgroupAvcSicInitializeINTEL
  {IR(47200, 43), 727}, // 785 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(47291, 43), 729}, // 786 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(47150, 50), 726}, // 787 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(47102, 48), 725}, // 788 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(47058, 44), 724}, // 789 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(47243, 48), 728}, // 790 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6648, 18), 364}, // 791 SubgroupBallotKHR
  {IR(51251, 27), 841}, // 792 SubgroupBlockPrefetchINTEL
  {IR(42150, 23), 595}, // 793 SubgroupBlockReadINTEL
  {IR(42173, 24), 596}, // 794 SubgroupBlockWriteINTEL
  {IR(36013, 27), 365}, // 795 SubgroupFirstInvocationKHR
  {IR(42197, 28), 597}, // 796 SubgroupImageBlockReadINTEL
  {IR(42225, 29), 598}, // 797 SubgroupImageBlockWriteINTEL
  {IR(42254, 33), 599}, // 798 SubgroupImageMediaBlockReadINTEL
  {IR(42287, 34), 600}, // 799 SubgroupImageMediaBlockWriteINTEL
  {IR(16211, 38), 847}, // 800 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(36196, 26), 375}, // 801 SubgroupReadInvocationKHR
  {IR(42078, 25), 592}, // 802 SubgroupShuffleDownINTEL
  {IR(13809, 21), 591}, // 803 SubgroupShuffleINTEL
  {IR(42103, 23), 593}, // 804 SubgroupShuffleUpINTEL
  {IR(42126, 24), 594}, // 805 SubgroupShuffleXorINTEL
  {IR(33370, 7), 229}, // 806 Switch
  {IR(51065, 24), 837}, // 807 TaskSequenceAsyncALTERA
  {IR(51089, 23), 837}, // 808 TaskSequenceAsyncINTEL
  {IR(51016, 25), 836}, // 809 TaskSequenceCreateALTERA
  {IR(51041, 24), 836}, // 810 TaskSequenceCreateINTEL
  {IR(51112, 22), 838}, // 811 TaskSequenceGetALTERA
  {IR(51134, 21), 838}, // 812 TaskSequenceGetINTEL
  {IR(51155, 26), 839}, // 813 TaskSequenceReleaseALTERA
  {IR(51181, 25), 839}, // 814 TaskSequenceReleaseINTEL
  {IR(41421, 27), 568}, // 815 TensorLayoutSetBlockSizeNV
  {IR(41285, 28), 561}, // 816 TensorLayoutSetClampValueNV
  {IR(41214, 27), 558}, // 817 TensorLayoutSetDimensionNV
  {IR(41241, 24), 559}, // 818 TensorLayoutSetStrideNV
  {IR(41265, 20), 560}, // 819 TensorLayoutSliceNV
  {IR(35788, 19), 350}, // 820 TensorQuerySizeARM
  {IR(35759, 14), 348}, // 821 TensorReadARM
  {IR(41401, 20), 567}, // 822 TensorViewSetClipNV
  {IR(41332, 25), 563}, // 823 TensorViewSetDimensionNV
  {IR(41357, 22), 564}, // 824 TensorViewSetStrideNV
  {IR(35773, 15), 349}, // 825 TensorWriteARM
  {IR(35900, 20), 359}, // 826 TerminateInvocation
  {IR(36361, 16), 382}, // 827 TerminateRayKHR
  {IR(40255, 15), 528}, // 828 TerminateRayNV
  {IR(40278, 14), 530}, // 829 TraceMotionNV
  {IR(40270, 8), 529}, // 830 TraceNV
  {IR(36273, 12), 378}, // 831 TraceRayKHR
  {IR(40292, 17), 531}, // 832 TraceRayMotionNV
  {IR(31222, 10), 76}, // 833 Transpose
  {IR(40359, 29), 533}, // 834 TypeAccelerationStructureKHR
  {IR(40388, 28), 533}, // 835 TypeAccelerationStructureNV
  {IR(30171, 10), 25}, // 836 TypeArray
  {IR(43063, 37), 637}, // 837 TypeAvcImeDualReferenceStreaminINTEL
  {IR(42798, 23), 628}, // 838 TypeAvcImePayloadINTEL
  {IR(42980, 44), 635}, // 839 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(42912, 22), 633}, // 840 TypeAvcImeResultINTEL
  {IR(42934, 46), 634}, // 841 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(43024, 39), 636}, // 842 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(42867, 23), 631}, // 843 TypeAvcMcePayloadINTEL
  {IR(42890, 22), 632}, // 844 TypeAvcMceResultINTEL
  {IR(42821, 23), 629}, // 845 TypeAvcRefPayloadINTEL
  {IR(43100, 22), 638}, // 846 TypeAvcRefResultINTEL
  {IR(42844, 23), 630}, // 847 TypeAvcSicPayloadINTEL
  {IR(43122, 22), 639}, // 848 TypeAvcSicResultINTEL
  {IR(29933, 9), 17}, // 849 TypeBool
  {IR(37791, 14), 438}, // 850 TypeBufferEXT
  {IR(50731, 23), 826}, // 851 TypeBufferSurfaceINTEL
  {IR(36542, 25), 389}, // 852 TypeCooperativeMatrixKHR
  {IR(40681, 24), 543}, // 853 TypeCooperativeMatrixNV
  {IR(38844, 24), 482}, // 854 TypeCooperativeVectorNV
  {IR(30286, 16), 32}, // 855 TypeDeviceEvent
  {IR(30276, 10), 31}, // 856 TypeEvent
  {IR(29987, 10), 19}, // 857 TypeFloat
  {IR(30335, 19), 36}, // 858 TypeForwardPointer
  {IR(30263, 13), 30}, // 859 TypeFunction
  {IR(35887, 13), 357}, // 860 TypeGraphARM
  {IR(39616, 17), 505}, // 861 TypeHitObjectEXT
  {IR(38789, 16), 480}, // 862 TypeHitObjectNV
  {IR(30132, 10), 22}, // 863 TypeImage
  {IR(29942, 8), 18}, // 864 TypeInt
  {IR(30008, 11), 21}, // 865 TypeMatrix
  {IR(34717, 17), 300}, // 866 TypeNamedBarrier
  {IR(37593, 25), 430}, // 867 TypeNodePayloadArrayAMDX
  {IR(30209, 11), 28}, // 868 TypeOpaque
  {IR(30326, 9), 35}, // 869 TypePipe
  {IR(34595, 16), 295}, // 870 TypePipeStorage
  {IR(30251, 12), 29}, // 871 TypePointer
  {IR(30316, 10), 34}, // 872 TypeQueue
  {IR(36821, 16), 397}, // 873 TypeRayQueryKHR
  {IR(30302, 14), 33}, // 874 TypeReserveId
  {IR(30181, 17), 26}, // 875 TypeRuntimeArray
  {IR(30154, 17), 24}, // 876 TypeSampledImage
  {IR(30142, 12), 23}, // 877 TypeSampler
  {IR(30198, 11), 27}, // 878 TypeStruct
  {IR(50754, 25), 827}, // 879 TypeStructContinuedINTEL
  {IR(51206, 23), 840}, // 880 TypeTaskSequenceALTERA
  {IR(51229, 22), 840}, // 881 TypeTaskSequenceINTEL
  {IR(35703, 14), 347}, // 882 TypeTensorARM
  {IR(41157, 19), 555}, // 883 TypeTensorLayoutNV
  {IR(41176, 17), 556}, // 884 TypeTensorViewNV
  {IR(35920, 22), 360}, // 885 TypeUntypedPointerKHR
  {IR(29997, 11), 20}, // 886 TypeVector
  {IR(38828, 16), 482}, // 887 TypeVectorIdEXT
  {IR(42780, 18), 627}, // 888 TypeVmeImageINTEL
  {IR(29924, 9), 16}, // 889 TypeVoid
  {IR(42409, 13), 606}, // 890 UAddSatINTEL
  {IR(42436, 14), 608}, // 891 UAverageINTEL
  {IR(42471, 21), 610}, // 892 UAverageRoundedINTEL
  {IR(31739, 9), 103}, // 893 UConvert
  {IR(42321, 24), 601}, // 894 UCountLeadingZerosINTEL
  {IR(42345, 25), 602}, // 895 UCountTrailingZerosINTEL
  {IR(31951, 5), 123}, // 896 UDiv
  {IR(36437, 5), 384}, // 897 UDot
  {IR(36490, 11), 387}, // 898 UDotAccSat
  {IR(36501, 14), 387}, // 899 UDotAccSatKHR
  {IR(36442, 8), 384}, // 900 UDotKHR
  {IR(32311, 13), 160}, // 901 UGreaterThan
  {IR(32337, 18), 162}, // 902 UGreaterThanEqual
  {IR(32373, 10), 164}, // 903 ULessThan
  {IR(32393, 15), 166}, // 904 ULessThanEqual
  {IR(31966, 5), 126}, // 905 UMod
  {IR(42533, 15), 614}, // 906 UMul32x16INTEL
  {IR(32119, 13), 140}, // 907 UMulExtended
  {IR(42505, 13), 612}, // 908 USubSatINTEL
  {IR(29436, 6), 1}, // 909 Undef
  {IR(32216, 10), 151}, // 910 Unordered
  {IR(33401, 12), 233}, // 911 Unreachable
  {IR(35961, 22), 362}, // 912 UntypedAccessChainKHR
  {IR(36098, 22), 368}, // 913 UntypedArrayLengthKHR
  {IR(36248, 25), 377}, // 914 UntypedGroupAsyncCopyKHR
  {IR(37822, 28), 441}, // 915 UntypedImageTexelPointerEXT
  {IR(35983, 30), 363}, // 916 UntypedInBoundsAccessChainKHR
  {IR(36065, 33), 367}, // 917 UntypedInBoundsPtrAccessChainKHR
  {IR(36120, 19), 369}, // 918 UntypedPrefetchKHR
  {IR(36040, 25), 366}, // 919 UntypedPtrAccessChainKHR
  {IR(35942, 19), 361}, // 920 UntypedVariableKHR
  {IR(16277, 32), 849}, // 921 UntypedVariableLengthArrayINTEL
  {IR(30747, 9), 52}, // 922 Variable
  {IR(14265, 25), 744}, // 923 VariableLengthArrayINTEL
  {IR(31104, 21), 69}, // 924 VectorExtractDynamic
  {IR(31125, 20), 70}, // 925 VectorInsertDynamic
  {IR(31145, 14), 71}, // 926 VectorShuffle
  {IR(32027, 18), 133}, // 927 VectorTimesMatrix
  {IR(31991, 18), 131}, // 928 VectorTimesScalar
  {IR(42766, 14), 626}, // 929 VmeImageINTEL
  {IR(39158, 33), 491}, // 930 WritePackedPrimitiveIndices4x8NV
  {IR(33626, 10), 250}, // 931 WritePipe
  {IR(50019, 24), 806}, // 932 WritePipeBlockingALTERA
  {IR(50043, 23), 806}, // 933 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 500> kExtInstByValue{{
    {0,IR(811, 2),IR(52115, 5),IR(1, 0)}, // CALL in arm_experimental_ml_operations
    {0,IR(765, 9),IR(52120, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(765, 9),IR(52128, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(52141, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(52149, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(52163, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(813, 3),IR(52236, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(816, 3),IR(52285, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(819, 2),IR(52340, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(52359, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(52374, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(821, 6),IR(52390, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(52403, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(827, 9),IR(52421, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(836, 9),IR(52473, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(845, 10),IR(52492, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(855, 5),IR(52508, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(52529, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(52550, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(860, 6),IR(52568, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(866, 5),IR(52595, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(871, 5),IR(52630, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(845, 10),IR(52661, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(876, 8),IR(52681, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(884, 11),IR(52706, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(895, 5),IR(52720, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(900, 3),IR(52738, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(903, 2),IR(52769, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(52780, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(905, 3),IR(52793, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(908, 7),IR(52850, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(52869, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(52890, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(52903, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(915, 2),IR(52947, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(747, 1),IR(52962, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(917, 4),IR(52978, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(900, 3),IR(52992, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(53008, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(53014, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(53024, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(53030, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(53035, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(53040, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(53046, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(53052, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(53058, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(53063, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(53069, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(53077, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(53085, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(53089, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(53093, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(53097, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(53102, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(53107, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(53112, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(53117, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(53122, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(53127, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(53133, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(53139, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(53145, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(53151, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(53155, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(53159, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(53163, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(53168, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(53173, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(53178, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(53190, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(53202, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(53216, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(53221, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(53232, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(53237, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(53242, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(53247, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(53252, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(53257, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(53262, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(53269, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(53276, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(53283, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(53288, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(53293, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(53298, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(53309, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(53313, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(53319, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(53331, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(53337, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(53350, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(53363, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(53377, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(53391, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(53404, 15),IR(344, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(53419, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(53435, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(53451, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(53466, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(53481, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(53496, 17),IR(344, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(53513, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(53520, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(53529, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(53535, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(53545, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(53557, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(53565, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(53573, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(53582, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(53591, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(53600, 22),IR(345, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(53622, 20),IR(345, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(53642, 20),IR(345, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(53662, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(53667, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(53672, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5867, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(921, 5),IR(53679, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(593, 5),IR(53692, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(593, 5),IR(53714, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(557, 7),IR(53730, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(557, 7),IR(53755, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(593, 5),IR(53774, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(593, 5),IR(53798, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(593, 5),IR(53819, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(593, 5),IR(53840, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(593, 5),IR(53856, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(53874, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(53900, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(53925, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(53945, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(53970, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(54000, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(54023, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(54048, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(54074, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(54104, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(54130, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(9341, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(54150, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(54180, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(593, 5),IR(54208, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(557, 7),IR(54236, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(54259, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(54294, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(54332, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(54374, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(535, 6),IR(54419, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(535, 6),IR(54456, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(593, 5),IR(54496, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(593, 5),IR(54523, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(54550, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(54582, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(54622, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(54633, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(54659, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(54691, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(54725, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {1,IR(36, 2),IR(54747, 11),IR(1, 0)}, // DebugGraph in nonsemantic_graph_debuginfo
    {2,IR(375, 3),IR(52947, 15),IR(1, 0)}, // DebugOperation in nonsemantic_graph_debuginfo
    {3,IR(926, 3),IR(54758, 12),IR(1, 0)}, // DebugTensor in nonsemantic_graph_debuginfo
    {0,IR(0, 0),IR(52149, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {1,IR(296, 4),IR(52163, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {2,IR(593, 5),IR(52236, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {3,IR(33, 3),IR(52285, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {4,IR(36, 2),IR(52340, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {5,IR(168, 2),IR(52359, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {6,IR(36, 2),IR(52374, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {7,IR(535, 6),IR(52390, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo
    {8,IR(375, 3),IR(52403, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {9,IR(929, 9),IR(52421, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {10,IR(938, 10),IR(52473, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {11,IR(948, 9),IR(52492, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {12,IR(296, 4),IR(52508, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {13,IR(36, 2),IR(52529, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {14,IR(168, 2),IR(52550, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {15,IR(535, 6),IR(52568, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {16,IR(675, 5),IR(52595, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {17,IR(957, 5),IR(52630, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {18,IR(962, 10),IR(52661, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {19,IR(471, 8),IR(52681, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {20,IR(962, 10),IR(52706, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo
    {21,IR(593, 5),IR(52720, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {22,IR(33, 3),IR(52738, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {23,IR(903, 2),IR(52769, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo
    {24,IR(0, 0),IR(52780, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo
    {25,IR(926, 3),IR(52793, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {26,IR(972, 8),IR(52850, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {27,IR(36, 2),IR(52869, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {28,IR(980, 4),IR(52890, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo
    {29,IR(980, 4),IR(52903, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo
    {30,IR(168, 2),IR(52947, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo
    {31,IR(747, 1),IR(52962, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo
    {32,IR(580, 4),IR(52978, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {33,IR(33, 3),IR(52992, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {34,IR(516, 7),IR(54770, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {35,IR(903, 2),IR(54790, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo
    {101,IR(36, 2),IR(54802, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {102,IR(10, 1),IR(54826, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {103,IR(675, 5),IR(54847, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo
    {104,IR(0, 0),IR(54857, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo
    {105,IR(36, 2),IR(54869, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {106,IR(10, 1),IR(54890, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {107,IR(296, 4),IR(54907, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {108,IR(33, 3),IR(54923, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {109,IR(36, 2),IR(54939, 21),IR(1, 0)}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {110,IR(675, 5),IR(54960, 30),IR(1, 0)}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {1,IR(765, 9),IR(54990, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(55004, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(55017, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(55029, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(55043, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(984, 15),IR(55066, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(999, 33),IR(55086, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1032, 19),IR(55105, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(52149, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1051, 4),IR(52163, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1055, 3),IR(52236, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1058, 3),IR(52285, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1061, 2),IR(52340, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(52359, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(52374, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(821, 6),IR(52390, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1063, 3),IR(52403, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1066, 9),IR(52421, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1075, 10),IR(52473, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1085, 10),IR(52492, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1095, 5),IR(52508, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(52529, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(52550, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(860, 6),IR(52568, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(866, 5),IR(52595, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(871, 5),IR(52630, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1085, 10),IR(52661, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1100, 8),IR(52681, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1108, 11),IR(52706, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(895, 5),IR(52720, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(900, 3),IR(52738, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(903, 2),IR(52769, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(52780, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(905, 3),IR(52793, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1119, 8),IR(52850, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(52869, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(52890, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(980, 4),IR(52903, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1127, 2),IR(52947, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(747, 1),IR(52962, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(917, 4),IR(52978, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(900, 3),IR(52992, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1129, 7),IR(54770, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(903, 2),IR(54790, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1136, 8),IR(55426, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(55443, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(55448, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(55454, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(55461, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(55466, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(55472, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(55479, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(55484, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(55490, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(55496, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(55503, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(55511, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(55516, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(55521, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(55530, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(55534, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(55539, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(55545, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(55550, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(55554, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(55558, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(55563, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(55569, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(55575, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(55580, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(55585, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(55591, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(55595, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(55600, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(55605, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(55610, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(55616, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(55622, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(55628, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(55634, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(55640, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(55647, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(55656, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(55660, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(55665, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(55671, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(55677, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(55682, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(55686, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(55693, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(55700, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(55705, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(55709, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(55719, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(55723, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(55728, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(55733, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(55743, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(55750, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(55755, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(55761, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(55767, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(55773, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(55777, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(55784, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(55789, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(55795, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(55800, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(55804, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(55809, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(55815, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(55822, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(55828, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(55837, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(55849, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(55858, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(55868, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(55879, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(55888, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(55898, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(55909, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(55919, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(55930, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(55941, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(55950, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(55960, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(55969, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(55980, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(55994, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(56005, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(56017, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(56030, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(56041, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(56053, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(56066, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(56078, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(56091, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(56104, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(56115, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(56127, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(56138, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(56145, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(56153, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(56165, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(56177, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(56181, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(56189, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(56194, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(56205, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(56210, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(56216, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(56225, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(56232, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(56242, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(56256, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(56268, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(56283, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(56289, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(56300, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(56310, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(56320, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(56327, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(56334, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(56342, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(56350, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(56358, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(56366, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(56370, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(56374, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(56383, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(56393, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(56403, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(56409, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(56415, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(56421, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(56427, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(56436, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(56443, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(56453, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(56463, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(56474, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(56485, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(56494, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(56502, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(56510, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(56518, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1144, 3),IR(56526, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(56533, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(56541, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1144, 3),IR(56552, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(56564, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1147, 4),IR(56576, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(56590, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1147, 4),IR(56603, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1144, 3),IR(56618, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(56631, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1147, 4),IR(56645, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(56661, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(56669, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(56678, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(56685, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(56694, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(56704, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(56711, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(56717, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(56728, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(56737, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(56746, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(56763, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(56780, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(56788, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(56810, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(56838, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(56857, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(56866, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(56889, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(56898, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(56907, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(56916, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(56925, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(56934, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(56943, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(56952, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(56961, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(56970, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(516, 7),IR(56977, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(755, 10),IR(56988, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(755, 10),IR(56995, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(755, 10),IR(57002, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(57019, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(57025, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(675, 5),IR(57032, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(57043, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(765, 9),IR(57050, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(57067, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(57073, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(57077, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(57085, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(57090, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(57094, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(57117, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(57129, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(57140, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(57152, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(57159, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(57171, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(57190, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(57210, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(57221, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(57233, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(57241, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(57249, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(57253, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(57257, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(57261, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(57267, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(57271, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(57283, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(57288, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(57292, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(57296, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(57300, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(57306, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(57310, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(57322, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(57329, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(57340, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(57346, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(57350, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(57357, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(57363, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(57371, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(57385, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(57396, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(57407, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(57418, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(57429, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(57444, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(57455, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(57462, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(57466, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(57474, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(57482, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(57488, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(57493, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(57503, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(57510, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(675, 5),IR(57518, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(57525, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(755, 10),IR(57530, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
}};

IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_EXPERIMENTAL_ML_OPERATIONS: return IR(0, 1);
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(1, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(4, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(38, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(119, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_GRAPH_DEBUGINFO: return IR(161, 3);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(164, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(210, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(218, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(255, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(417, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(420, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(424, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(425, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(434, 66);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 500> kExtInstNames{{
    {IR(52115, 5), 0}, // CALL in arm_experimental_ml_operations
    {IR(52120, 8), 1}, // MIN_SAD in arm_motion_engine_100
    {IR(52128, 13), 2}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(52141, 8), 3}, // RAW_SAD in arm_motion_engine_100
    {IR(52163, 21), 5}, // DebugCompilationUnit in debuginfo
    {IR(52890, 13), 32}, // DebugDeclare in debuginfo
    {IR(52962, 16), 35}, // DebugExpression in debuginfo
    {IR(52706, 14), 24}, // DebugFunction in debuginfo
    {IR(52681, 25), 23}, // DebugFunctionDeclaration in debuginfo
    {IR(52661, 20), 22}, // DebugGlobalVariable in debuginfo
    {IR(52149, 14), 4}, // DebugInfoNone in debuginfo
    {IR(52793, 15), 29}, // DebugInlinedAt in debuginfo
    {IR(52869, 21), 31}, // DebugInlinedVariable in debuginfo
    {IR(52720, 18), 25}, // DebugLexicalBlock in debuginfo
    {IR(52738, 31), 26}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(52850, 19), 30}, // DebugLocalVariable in debuginfo
    {IR(52978, 14), 36}, // DebugMacroDef in debuginfo
    {IR(52992, 16), 37}, // DebugMacroUndef in debuginfo
    {IR(52780, 13), 28}, // DebugNoScope in debuginfo
    {IR(52947, 15), 34}, // DebugOperation in debuginfo
    {IR(52769, 11), 27}, // DebugScope in debuginfo
    {IR(52359, 15), 9}, // DebugTypeArray in debuginfo
    {IR(52236, 15), 6}, // DebugTypeBasic in debuginfo
    {IR(52473, 19), 14}, // DebugTypeComposite in debuginfo
    {IR(52421, 14), 13}, // DebugTypeEnum in debuginfo
    {IR(52403, 18), 12}, // DebugTypeFunction in debuginfo
    {IR(52508, 21), 16}, // DebugTypeInheritance in debuginfo
    {IR(52492, 16), 15}, // DebugTypeMember in debuginfo
    {IR(52285, 17), 7}, // DebugTypePointer in debuginfo
    {IR(52529, 21), 17}, // DebugTypePtrToMember in debuginfo
    {IR(52340, 19), 8}, // DebugTypeQualifier in debuginfo
    {IR(52550, 18), 18}, // DebugTypeTemplate in debuginfo
    {IR(52568, 27), 19}, // DebugTypeTemplateParameter in debuginfo
    {IR(52630, 31), 21}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(52595, 35), 20}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(52374, 16), 10}, // DebugTypeVector in debuginfo
    {IR(52390, 13), 11}, // DebugTypedef in debuginfo
    {IR(52903, 11), 33}, // DebugValue in debuginfo
    {IR(53102, 5), 54}, // Acos in glsl_std_450
    {IR(53133, 6), 60}, // Acosh in glsl_std_450
    {IR(53097, 5), 53}, // Asin in glsl_std_450
    {IR(53127, 6), 59}, // Asinh in glsl_std_450
    {IR(53107, 5), 55}, // Atan in glsl_std_450
    {IR(53145, 6), 62}, // Atan2 in glsl_std_450
    {IR(53139, 6), 61}, // Atanh in glsl_std_450
    {IR(53058, 5), 46}, // Ceil in glsl_std_450
    {IR(53089, 4), 51}, // Cos in glsl_std_450
    {IR(53117, 5), 57}, // Cosh in glsl_std_450
    {IR(53529, 6), 105}, // Cross in glsl_std_450
    {IR(53077, 8), 49}, // Degrees in glsl_std_450
    {IR(53190, 12), 70}, // Determinant in glsl_std_450
    {IR(53520, 9), 104}, // Distance in glsl_std_450
    {IR(53155, 4), 64}, // Exp in glsl_std_450
    {IR(53163, 5), 66}, // Exp2 in glsl_std_450
    {IR(53030, 5), 41}, // FAbs in glsl_std_450
    {IR(53262, 7), 80}, // FClamp in glsl_std_450
    {IR(53247, 5), 77}, // FMax in glsl_std_450
    {IR(53232, 5), 74}, // FMin in glsl_std_450
    {IR(53283, 5), 83}, // FMix in glsl_std_450
    {IR(53040, 6), 43}, // FSign in glsl_std_450
    {IR(53545, 12), 107}, // FaceForward in glsl_std_450
    {IR(53573, 9), 110}, // FindILsb in glsl_std_450
    {IR(53582, 9), 111}, // FindSMsb in glsl_std_450
    {IR(53591, 9), 112}, // FindUMsb in glsl_std_450
    {IR(53052, 6), 45}, // Floor in glsl_std_450
    {IR(53309, 4), 87}, // Fma in glsl_std_450
    {IR(53063, 6), 47}, // Fract in glsl_std_450
    {IR(53313, 6), 88}, // Frexp in glsl_std_450
    {IR(53319, 12), 89}, // FrexpStruct in glsl_std_450
    {IR(53288, 5), 84}, // IMix in glsl_std_450
    {IR(53600, 22), 113}, // InterpolateAtCentroid in glsl_std_450
    {IR(53642, 20), 115}, // InterpolateAtOffset in glsl_std_450
    {IR(53622, 20), 114}, // InterpolateAtSample in glsl_std_450
    {IR(53178, 12), 69}, // InverseSqrt in glsl_std_450
    {IR(53331, 6), 90}, // Ldexp in glsl_std_450
    {IR(53513, 7), 103}, // Length in glsl_std_450
    {IR(53159, 4), 65}, // Log in glsl_std_450
    {IR(53168, 5), 67}, // Log2 in glsl_std_450
    {IR(53202, 14), 71}, // MatrixInverse in glsl_std_450
    {IR(53216, 5), 72}, // Modf in glsl_std_450
    {IR(53221, 11), 73}, // ModfStruct in glsl_std_450
    {IR(53672, 7), 118}, // NClamp in glsl_std_450
    {IR(53667, 5), 117}, // NMax in glsl_std_450
    {IR(53662, 5), 116}, // NMin in glsl_std_450
    {IR(53535, 10), 106}, // Normalize in glsl_std_450
    {IR(53404, 15), 96}, // PackDouble2x32 in glsl_std_450
    {IR(53391, 13), 95}, // PackHalf2x16 in glsl_std_450
    {IR(53363, 14), 93}, // PackSnorm2x16 in glsl_std_450
    {IR(53337, 13), 91}, // PackSnorm4x8 in glsl_std_450
    {IR(53377, 14), 94}, // PackUnorm2x16 in glsl_std_450
    {IR(53350, 13), 92}, // PackUnorm4x8 in glsl_std_450
    {IR(53151, 4), 63}, // Pow in glsl_std_450
    {IR(53069, 8), 48}, // Radians in glsl_std_450
    {IR(53557, 8), 108}, // Reflect in glsl_std_450
    {IR(53565, 8), 109}, // Refract in glsl_std_450
    {IR(53008, 6), 38}, // Round in glsl_std_450
    {IR(53014, 10), 39}, // RoundEven in glsl_std_450
    {IR(53035, 5), 42}, // SAbs in glsl_std_450
    {IR(53276, 7), 82}, // SClamp in glsl_std_450
    {IR(53257, 5), 79}, // SMax in glsl_std_450
    {IR(53242, 5), 76}, // SMin in glsl_std_450
    {IR(53046, 6), 44}, // SSign in glsl_std_450
    {IR(53085, 4), 50}, // Sin in glsl_std_450
    {IR(53112, 5), 56}, // Sinh in glsl_std_450
    {IR(53298, 11), 86}, // SmoothStep in glsl_std_450
    {IR(53173, 5), 68}, // Sqrt in glsl_std_450
    {IR(53293, 5), 85}, // Step in glsl_std_450
    {IR(53093, 4), 52}, // Tan in glsl_std_450
    {IR(53122, 5), 58}, // Tanh in glsl_std_450
    {IR(53024, 6), 40}, // Trunc in glsl_std_450
    {IR(53269, 7), 81}, // UClamp in glsl_std_450
    {IR(53252, 5), 78}, // UMax in glsl_std_450
    {IR(53237, 5), 75}, // UMin in glsl_std_450
    {IR(53496, 17), 102}, // UnpackDouble2x32 in glsl_std_450
    {IR(53451, 15), 99}, // UnpackHalf2x16 in glsl_std_450
    {IR(53419, 16), 97}, // UnpackSnorm2x16 in glsl_std_450
    {IR(53466, 15), 100}, // UnpackSnorm4x8 in glsl_std_450
    {IR(53435, 16), 98}, // UnpackUnorm2x16 in glsl_std_450
    {IR(53481, 15), 101}, // UnpackUnorm4x8 in glsl_std_450
    {IR(53679, 13), 120}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(53774, 24), 125}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(53730, 25), 123}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(53755, 19), 124}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(54208, 28), 144}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(54236, 23), 145}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(53798, 21), 126}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(53840, 16), 128}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(53692, 22), 121}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(53819, 21), 127}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(54496, 27), 152}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(53714, 16), 122}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(54523, 27), 153}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(53856, 18), 129}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(54550, 32), 154}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(54104, 26), 139}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(54130, 20), 140}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(54374, 45), 149}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(54456, 40), 151}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(54332, 42), 148}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(54419, 37), 150}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5867, 7), 119}, // Kernel in nonsemantic_clspvreflection
    {IR(9341, 15), 141}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(54691, 34), 159}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(54659, 32), 158}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(54633, 26), 157}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(54622, 11), 156}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(54582, 40), 155}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(54294, 38), 147}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(54259, 35), 146}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(54150, 30), 142}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(53970, 30), 134}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(53945, 25), 133}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54000, 23), 135}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(54048, 26), 137}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(54074, 30), 138}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(54023, 25), 136}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(53900, 25), 131}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54180, 28), 143}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(53925, 20), 132}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(53874, 26), 130}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(54725, 22), 160}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(54747, 11), 161}, // DebugGraph in nonsemantic_graph_debuginfo
    {IR(52947, 15), 162}, // DebugOperation in nonsemantic_graph_debuginfo
    {IR(54758, 12), 163}, // DebugTensor in nonsemantic_graph_debuginfo
    {IR(54869, 21), 204}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {IR(52163, 21), 165}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {IR(52890, 13), 192}, // DebugDeclare in nonsemantic_shader_debuginfo
    {IR(54907, 16), 206}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {IR(52962, 16), 195}, // DebugExpression in nonsemantic_shader_debuginfo
    {IR(52706, 14), 184}, // DebugFunction in nonsemantic_shader_debuginfo
    {IR(52681, 25), 183}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {IR(54802, 24), 200}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {IR(52661, 20), 182}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {IR(54770, 20), 198}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {IR(52149, 14), 164}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {IR(52793, 15), 189}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {IR(52869, 21), 191}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {IR(52720, 18), 185}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {IR(52738, 31), 186}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {IR(54847, 10), 202}, // DebugLine in nonsemantic_shader_debuginfo
    {IR(52850, 19), 190}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {IR(52978, 14), 196}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {IR(52992, 16), 197}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {IR(54857, 12), 203}, // DebugNoLine in nonsemantic_shader_debuginfo
    {IR(52780, 13), 188}, // DebugNoScope in nonsemantic_shader_debuginfo
    {IR(52947, 15), 194}, // DebugOperation in nonsemantic_shader_debuginfo
    {IR(52769, 11), 187}, // DebugScope in nonsemantic_shader_debuginfo
    {IR(54790, 12), 199}, // DebugSource in nonsemantic_shader_debuginfo
    {IR(54826, 21), 201}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {IR(54890, 17), 205}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {IR(52359, 15), 169}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {IR(52236, 15), 166}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {IR(52473, 19), 174}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {IR(54960, 30), 209}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {IR(52421, 14), 173}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {IR(52403, 18), 172}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {IR(52508, 21), 176}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {IR(54923, 16), 207}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {IR(52492, 16), 175}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {IR(52285, 17), 167}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {IR(52529, 21), 177}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {IR(52340, 19), 168}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {IR(52550, 18), 178}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {IR(52568, 27), 179}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52630, 31), 181}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {IR(52595, 35), 180}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52374, 16), 170}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {IR(54939, 21), 208}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {IR(52390, 13), 171}, // DebugTypedef in nonsemantic_shader_debuginfo
    {IR(52903, 11), 193}, // DebugValue in nonsemantic_shader_debuginfo
    {IR(54990, 14), 210}, // Configuration in nonsemantic_vkspreflection
    {IR(55066, 20), 215}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(55086, 19), 216}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(55105, 21), 217}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(55029, 14), 213}, // PushConstants in nonsemantic_vkspreflection
    {IR(55043, 23), 214}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(55004, 13), 211}, // StartCounter in nonsemantic_vkspreflection
    {IR(55017, 12), 212}, // StopCounter in nonsemantic_vkspreflection
    {IR(52163, 21), 219}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(52890, 13), 246}, // DebugDeclare in opencl_debuginfo_100
    {IR(52962, 16), 249}, // DebugExpression in opencl_debuginfo_100
    {IR(52706, 14), 238}, // DebugFunction in opencl_debuginfo_100
    {IR(52681, 25), 237}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(52661, 20), 236}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(54770, 20), 252}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(52149, 14), 218}, // DebugInfoNone in opencl_debuginfo_100
    {IR(52793, 15), 243}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(52869, 21), 245}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(52720, 18), 239}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(52738, 31), 240}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(52850, 19), 244}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(52978, 14), 250}, // DebugMacroDef in opencl_debuginfo_100
    {IR(52992, 16), 251}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(55426, 17), 254}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(52780, 13), 242}, // DebugNoScope in opencl_debuginfo_100
    {IR(52947, 15), 248}, // DebugOperation in opencl_debuginfo_100
    {IR(52769, 11), 241}, // DebugScope in opencl_debuginfo_100
    {IR(54790, 12), 253}, // DebugSource in opencl_debuginfo_100
    {IR(52359, 15), 223}, // DebugTypeArray in opencl_debuginfo_100
    {IR(52236, 15), 220}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(52473, 19), 228}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(52421, 14), 227}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(52403, 18), 226}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(52508, 21), 230}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(52492, 16), 229}, // DebugTypeMember in opencl_debuginfo_100
    {IR(52285, 17), 221}, // DebugTypePointer in opencl_debuginfo_100
    {IR(52529, 21), 231}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(52340, 19), 222}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(52550, 18), 232}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(52568, 27), 233}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(52630, 31), 235}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(52595, 35), 234}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(52374, 16), 224}, // DebugTypeVector in opencl_debuginfo_100
    {IR(52390, 13), 225}, // DebugTypedef in opencl_debuginfo_100
    {IR(52903, 11), 247}, // DebugValue in opencl_debuginfo_100
    {IR(55443, 5), 255}, // acos in opencl_std_100
    {IR(55448, 6), 256}, // acosh in opencl_std_100
    {IR(55454, 7), 257}, // acospi in opencl_std_100
    {IR(55461, 5), 258}, // asin in opencl_std_100
    {IR(55466, 6), 259}, // asinh in opencl_std_100
    {IR(55472, 7), 260}, // asinpi in opencl_std_100
    {IR(55479, 5), 261}, // atan in opencl_std_100
    {IR(55484, 6), 262}, // atan2 in opencl_std_100
    {IR(55503, 8), 265}, // atan2pi in opencl_std_100
    {IR(55490, 6), 263}, // atanh in opencl_std_100
    {IR(55496, 7), 264}, // atanpi in opencl_std_100
    {IR(56694, 10), 411}, // bitselect in opencl_std_100
    {IR(55511, 5), 266}, // cbrt in opencl_std_100
    {IR(55516, 5), 267}, // ceil in opencl_std_100
    {IR(56366, 4), 376}, // clz in opencl_std_100
    {IR(55521, 9), 268}, // copysign in opencl_std_100
    {IR(55530, 4), 269}, // cos in opencl_std_100
    {IR(55534, 5), 270}, // cosh in opencl_std_100
    {IR(55539, 6), 271}, // cospi in opencl_std_100
    {IR(56210, 6), 359}, // cross in opencl_std_100
    {IR(56370, 4), 377}, // ctz in opencl_std_100
    {IR(56145, 8), 351}, // degrees in opencl_std_100
    {IR(56216, 9), 360}, // distance in opencl_std_100
    {IR(55550, 4), 273}, // erf in opencl_std_100
    {IR(55545, 5), 272}, // erfc in opencl_std_100
    {IR(55554, 4), 274}, // exp in opencl_std_100
    {IR(55563, 6), 276}, // exp10 in opencl_std_100
    {IR(55558, 5), 275}, // exp2 in opencl_std_100
    {IR(55569, 6), 277}, // expm1 in opencl_std_100
    {IR(55575, 5), 278}, // fabs in opencl_std_100
    {IR(56242, 14), 363}, // fast_distance in opencl_std_100
    {IR(56256, 12), 364}, // fast_length in opencl_std_100
    {IR(56268, 15), 365}, // fast_normalize in opencl_std_100
    {IR(56138, 7), 350}, // fclamp in opencl_std_100
    {IR(55580, 5), 279}, // fdim in opencl_std_100
    {IR(55585, 6), 280}, // floor in opencl_std_100
    {IR(55591, 4), 281}, // fma in opencl_std_100
    {IR(55595, 5), 282}, // fmax in opencl_std_100
    {IR(56153, 12), 352}, // fmax_common in opencl_std_100
    {IR(55600, 5), 283}, // fmin in opencl_std_100
    {IR(56165, 12), 353}, // fmin_common in opencl_std_100
    {IR(55605, 5), 284}, // fmod in opencl_std_100
    {IR(55610, 6), 285}, // fract in opencl_std_100
    {IR(55616, 6), 286}, // frexp in opencl_std_100
    {IR(55828, 9), 322}, // half_cos in opencl_std_100
    {IR(55837, 12), 323}, // half_divide in opencl_std_100
    {IR(55849, 9), 324}, // half_exp in opencl_std_100
    {IR(55868, 11), 326}, // half_exp10 in opencl_std_100
    {IR(55858, 10), 325}, // half_exp2 in opencl_std_100
    {IR(55879, 9), 327}, // half_log in opencl_std_100
    {IR(55898, 11), 329}, // half_log10 in opencl_std_100
    {IR(55888, 10), 328}, // half_log2 in opencl_std_100
    {IR(55909, 10), 330}, // half_powr in opencl_std_100
    {IR(55919, 11), 331}, // half_recip in opencl_std_100
    {IR(55930, 11), 332}, // half_rsqrt in opencl_std_100
    {IR(55941, 9), 333}, // half_sin in opencl_std_100
    {IR(55950, 10), 334}, // half_sqrt in opencl_std_100
    {IR(55960, 9), 335}, // half_tan in opencl_std_100
    {IR(55622, 6), 287}, // hypot in opencl_std_100
    {IR(55628, 6), 288}, // ilogb in opencl_std_100
    {IR(55634, 6), 289}, // ldexp in opencl_std_100
    {IR(56225, 7), 361}, // length in opencl_std_100
    {IR(55640, 7), 290}, // lgamma in opencl_std_100
    {IR(55647, 9), 291}, // lgamma_r in opencl_std_100
    {IR(55656, 4), 292}, // log in opencl_std_100
    {IR(55665, 6), 294}, // log10 in opencl_std_100
    {IR(55671, 6), 295}, // log1p in opencl_std_100
    {IR(55660, 5), 293}, // log2 in opencl_std_100
    {IR(55677, 5), 296}, // logb in opencl_std_100
    {IR(55682, 4), 297}, // mad in opencl_std_100
    {IR(55686, 7), 298}, // maxmag in opencl_std_100
    {IR(55693, 7), 299}, // minmag in opencl_std_100
    {IR(56177, 4), 354}, // mix in opencl_std_100
    {IR(55700, 5), 300}, // modf in opencl_std_100
    {IR(55705, 4), 301}, // nan in opencl_std_100
    {IR(55969, 11), 336}, // native_cos in opencl_std_100
    {IR(55980, 14), 337}, // native_divide in opencl_std_100
    {IR(55994, 11), 338}, // native_exp in opencl_std_100
    {IR(56017, 13), 340}, // native_exp10 in opencl_std_100
    {IR(56005, 12), 339}, // native_exp2 in opencl_std_100
    {IR(56030, 11), 341}, // native_log in opencl_std_100
    {IR(56053, 13), 343}, // native_log10 in opencl_std_100
    {IR(56041, 12), 342}, // native_log2 in opencl_std_100
    {IR(56066, 12), 344}, // native_powr in opencl_std_100
    {IR(56078, 13), 345}, // native_recip in opencl_std_100
    {IR(56091, 13), 346}, // native_rsqrt in opencl_std_100
    {IR(56104, 11), 347}, // native_sin in opencl_std_100
    {IR(56115, 12), 348}, // native_sqrt in opencl_std_100
    {IR(56127, 11), 349}, // native_tan in opencl_std_100
    {IR(55709, 10), 302}, // nextafter in opencl_std_100
    {IR(56232, 10), 362}, // normalize in opencl_std_100
    {IR(56485, 9), 391}, // popcount in opencl_std_100
    {IR(55719, 4), 303}, // pow in opencl_std_100
    {IR(55723, 5), 304}, // pown in opencl_std_100
    {IR(55728, 5), 305}, // powr in opencl_std_100
    {IR(56685, 9), 410}, // prefetch in opencl_std_100
    {IR(56678, 7), 409}, // printf in opencl_std_100
    {IR(56181, 8), 355}, // radians in opencl_std_100
    {IR(55733, 10), 306}, // remainder in opencl_std_100
    {IR(55743, 7), 307}, // remquo in opencl_std_100
    {IR(55750, 5), 308}, // rint in opencl_std_100
    {IR(55755, 6), 309}, // rootn in opencl_std_100
    {IR(56436, 7), 386}, // rotate in opencl_std_100
    {IR(55761, 6), 310}, // round in opencl_std_100
    {IR(55767, 6), 311}, // rsqrt in opencl_std_100
    {IR(56283, 6), 366}, // s_abs in opencl_std_100
    {IR(56289, 11), 367}, // s_abs_diff in opencl_std_100
    {IR(56300, 10), 368}, // s_add_sat in opencl_std_100
    {IR(56350, 8), 374}, // s_clamp in opencl_std_100
    {IR(56320, 7), 370}, // s_hadd in opencl_std_100
    {IR(56494, 8), 392}, // s_mad24 in opencl_std_100
    {IR(56374, 9), 378}, // s_mad_hi in opencl_std_100
    {IR(56393, 10), 380}, // s_mad_sat in opencl_std_100
    {IR(56403, 6), 381}, // s_max in opencl_std_100
    {IR(56415, 6), 383}, // s_min in opencl_std_100
    {IR(56510, 8), 394}, // s_mul24 in opencl_std_100
    {IR(56427, 9), 385}, // s_mul_hi in opencl_std_100
    {IR(56334, 8), 372}, // s_rhadd in opencl_std_100
    {IR(56443, 10), 387}, // s_sub_sat in opencl_std_100
    {IR(56474, 11), 390}, // s_upsample in opencl_std_100
    {IR(56704, 7), 412}, // select in opencl_std_100
    {IR(56661, 8), 407}, // shuffle in opencl_std_100
    {IR(56669, 9), 408}, // shuffle2 in opencl_std_100
    {IR(56205, 5), 358}, // sign in opencl_std_100
    {IR(55773, 4), 312}, // sin in opencl_std_100
    {IR(55777, 7), 313}, // sincos in opencl_std_100
    {IR(55784, 5), 314}, // sinh in opencl_std_100
    {IR(55789, 6), 315}, // sinpi in opencl_std_100
    {IR(56194, 11), 357}, // smoothstep in opencl_std_100
    {IR(55795, 5), 316}, // sqrt in opencl_std_100
    {IR(56189, 5), 356}, // step in opencl_std_100
    {IR(55800, 4), 317}, // tan in opencl_std_100
    {IR(55804, 5), 318}, // tanh in opencl_std_100
    {IR(55809, 6), 319}, // tanpi in opencl_std_100
    {IR(55815, 7), 320}, // tgamma in opencl_std_100
    {IR(55822, 6), 321}, // trunc in opencl_std_100
    {IR(56711, 6), 413}, // u_abs in opencl_std_100
    {IR(56717, 11), 414}, // u_abs_diff in opencl_std_100
    {IR(56310, 10), 369}, // u_add_sat in opencl_std_100
    {IR(56358, 8), 375}, // u_clamp in opencl_std_100
    {IR(56327, 7), 371}, // u_hadd in opencl_std_100
    {IR(56502, 8), 393}, // u_mad24 in opencl_std_100
    {IR(56737, 9), 416}, // u_mad_hi in opencl_std_100
    {IR(56383, 10), 379}, // u_mad_sat in opencl_std_100
    {IR(56409, 6), 382}, // u_max in opencl_std_100
    {IR(56421, 6), 384}, // u_min in opencl_std_100
    {IR(56518, 8), 395}, // u_mul24 in opencl_std_100
    {IR(56728, 9), 415}, // u_mul_hi in opencl_std_100
    {IR(56342, 8), 373}, // u_rhadd in opencl_std_100
    {IR(56453, 10), 388}, // u_sub_sat in opencl_std_100
    {IR(56463, 11), 389}, // u_upsample in opencl_std_100
    {IR(56541, 11), 398}, // vload_half in opencl_std_100
    {IR(56552, 12), 399}, // vload_halfn in opencl_std_100
    {IR(56618, 13), 404}, // vloada_halfn in opencl_std_100
    {IR(56526, 7), 396}, // vloadn in opencl_std_100
    {IR(56564, 12), 400}, // vstore_half in opencl_std_100
    {IR(56576, 14), 401}, // vstore_half_r in opencl_std_100
    {IR(56590, 13), 402}, // vstore_halfn in opencl_std_100
    {IR(56603, 15), 403}, // vstore_halfn_r in opencl_std_100
    {IR(56631, 14), 405}, // vstorea_halfn in opencl_std_100
    {IR(56645, 16), 406}, // vstorea_halfn_r in opencl_std_100
    {IR(56533, 8), 397}, // vstoren in opencl_std_100
    {IR(56763, 17), 418}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(56746, 17), 417}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(56780, 8), 419}, // TimeAMD in spv_amd_gcn_shader
    {IR(56857, 9), 423}, // MbcntAMD in spv_amd_shader_ballot
    {IR(56788, 22), 420}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(56810, 28), 421}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(56838, 19), 422}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(56866, 23), 424}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(56916, 9), 428}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56943, 9), 431}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56889, 9), 425}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56934, 9), 430}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56961, 9), 433}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56907, 9), 427}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56925, 9), 429}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56952, 9), 432}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56898, 9), 426}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57267, 4), 465}, // ABS in tosa_001000_1
    {IR(57090, 4), 448}, // ADD in tosa_001000_1
    {IR(56970, 7), 434}, // ARGMAX in tosa_001000_1
    {IR(57094, 23), 449}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(56977, 11), 435}, // AVG_POOL2D in tosa_001000_1
    {IR(57117, 12), 450}, // BITWISE_AND in tosa_001000_1
    {IR(57271, 12), 466}, // BITWISE_NOT in tosa_001000_1
    {IR(57129, 11), 451}, // BITWISE_OR in tosa_001000_1
    {IR(57140, 12), 452}, // BITWISE_XOR in tosa_001000_1
    {IR(57525, 5), 498}, // CAST in tosa_001000_1
    {IR(57283, 5), 467}, // CEIL in tosa_001000_1
    {IR(57067, 6), 444}, // CLAMP in tosa_001000_1
    {IR(57288, 4), 468}, // CLZ in tosa_001000_1
    {IR(57455, 7), 488}, // CONCAT in tosa_001000_1
    {IR(56988, 7), 436}, // CONV2D in tosa_001000_1
    {IR(56995, 7), 437}, // CONV3D in tosa_001000_1
    {IR(57292, 4), 469}, // COS in tosa_001000_1
    {IR(57002, 17), 438}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(57357, 6), 479}, // EQUAL in tosa_001000_1
    {IR(57073, 4), 445}, // ERF in tosa_001000_1
    {IR(57296, 4), 470}, // EXP in tosa_001000_1
    {IR(57019, 6), 439}, // FFT2D in tosa_001000_1
    {IR(57300, 6), 471}, // FLOOR in tosa_001000_1
    {IR(57503, 7), 495}, // GATHER in tosa_001000_1
    {IR(57363, 8), 480}, // GREATER in tosa_001000_1
    {IR(57371, 14), 481}, // GREATER_EQUAL in tosa_001000_1
    {IR(57152, 7), 453}, // INTDIV in tosa_001000_1
    {IR(57306, 4), 472}, // LOG in tosa_001000_1
    {IR(57159, 12), 454}, // LOGICAL_AND in tosa_001000_1
    {IR(57171, 19), 455}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(57310, 12), 473}, // LOGICAL_NOT in tosa_001000_1
    {IR(57210, 11), 457}, // LOGICAL_OR in tosa_001000_1
    {IR(57190, 20), 456}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(57221, 12), 458}, // LOGICAL_XOR in tosa_001000_1
    {IR(57025, 7), 440}, // MATMUL in tosa_001000_1
    {IR(57233, 8), 459}, // MAXIMUM in tosa_001000_1
    {IR(57032, 11), 441}, // MAX_POOL2D in tosa_001000_1
    {IR(57241, 8), 460}, // MINIMUM in tosa_001000_1
    {IR(57249, 4), 461}, // MUL in tosa_001000_1
    {IR(57322, 7), 474}, // NEGATE in tosa_001000_1
    {IR(57462, 4), 489}, // PAD in tosa_001000_1
    {IR(57253, 4), 462}, // POW in tosa_001000_1
    {IR(57329, 11), 475}, // RECIPROCAL in tosa_001000_1
    {IR(57385, 11), 482}, // REDUCE_ALL in tosa_001000_1
    {IR(57396, 11), 483}, // REDUCE_ANY in tosa_001000_1
    {IR(57407, 11), 484}, // REDUCE_MAX in tosa_001000_1
    {IR(57418, 11), 485}, // REDUCE_MIN in tosa_001000_1
    {IR(57429, 15), 486}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(57444, 11), 487}, // REDUCE_SUM in tosa_001000_1
    {IR(57530, 8), 499}, // RESCALE in tosa_001000_1
    {IR(57466, 8), 490}, // RESHAPE in tosa_001000_1
    {IR(57518, 7), 497}, // RESIZE in tosa_001000_1
    {IR(57474, 8), 491}, // REVERSE in tosa_001000_1
    {IR(57043, 7), 442}, // RFFT2D in tosa_001000_1
    {IR(57340, 6), 476}, // RSQRT in tosa_001000_1
    {IR(57510, 8), 496}, // SCATTER in tosa_001000_1
    {IR(57350, 7), 478}, // SELECT in tosa_001000_1
    {IR(57077, 8), 446}, // SIGMOID in tosa_001000_1
    {IR(57346, 4), 477}, // SIN in tosa_001000_1
    {IR(57482, 6), 492}, // SLICE in tosa_001000_1
    {IR(57257, 4), 463}, // SUB in tosa_001000_1
    {IR(57261, 6), 464}, // TABLE in tosa_001000_1
    {IR(57085, 5), 447}, // TANH in tosa_001000_1
    {IR(57488, 5), 493}, // TILE in tosa_001000_1
    {IR(57493, 10), 494}, // TRANSPOSE in tosa_001000_1
    {IR(57050, 17), 443}, // TRANSPOSE_CONV2D in tosa_001000_1
}};

IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_EXPERIMENTAL_ML_OPERATIONS: return IR(0, 1);
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(1, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(4, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(38, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(119, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_GRAPH_DEBUGINFO: return IR(161, 3);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(164, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(210, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(218, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(255, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(417, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(420, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(424, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(425, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(434, 66);
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
  "SPV_AMD_weak_linkage\0" // IR(1020, 21)
  "SPV_ARM_cooperative_matrix_layouts\0" // IR(1041, 35)
  "SPV_ARM_core_builtins\0" // IR(1076, 22)
  "SPV_ARM_graph\0" // IR(1098, 14)
  "SPV_ARM_tensors\0" // IR(1112, 16)
  "SPV_EXT_arithmetic_fence\0" // IR(1128, 25)
  "SPV_EXT_cooperative_matrix_maintenance1\0" // IR(1153, 40)
  "SPV_EXT_demote_to_helper_invocation\0" // IR(1193, 36)
  "SPV_EXT_descriptor_heap\0" // IR(1229, 24)
  "SPV_EXT_descriptor_indexing\0" // IR(1253, 28)
  "SPV_EXT_float8\0" // IR(1281, 15)
  "SPV_EXT_fragment_fully_covered\0" // IR(1296, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(1327, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(1363, 34)
  "SPV_EXT_long_vector\0" // IR(1397, 20)
  "SPV_EXT_mesh_shader\0" // IR(1417, 20)
  "SPV_EXT_ocp_microscaling_types\0" // IR(1437, 31)
  "SPV_EXT_opacity_micromap\0" // IR(1468, 25)
  "SPV_EXT_optnone\0" // IR(1493, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(1509, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(1541, 44)
  "SPV_EXT_replicated_composites\0" // IR(1585, 30)
  "SPV_EXT_shader_64bit_indexing\0" // IR(1615, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(1645, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(1679, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(1711, 36)
  "SPV_EXT_shader_image_int64\0" // IR(1747, 27)
  "SPV_EXT_shader_invocation_reorder\0" // IR(1774, 34)
  "SPV_EXT_shader_stencil_export\0" // IR(1808, 30)
  "SPV_EXT_shader_subgroup_partitioned\0" // IR(1838, 36)
  "SPV_EXT_shader_tile_image\0" // IR(1874, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1900, 36)
  "SPV_EXT_split_barrier\0" // IR(1936, 22)
  "SPV_GOOGLE_decorate_string\0" // IR(1958, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1985, 31)
  "SPV_GOOGLE_user_type\0" // IR(2016, 21)
  "SPV_INTEL_2d_block_io\0" // IR(2037, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(2059, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(2101, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(2146, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(2185, 30)
  "SPV_INTEL_bindless_images\0" // IR(2215, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(2241, 25)
  "SPV_INTEL_cache_controls\0" // IR(2266, 25)
  "SPV_INTEL_debug_module\0" // IR(2291, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(2314, 44)
  "SPV_INTEL_float_controls2\0" // IR(2358, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(2384, 28)
  "SPV_INTEL_fp_max_error\0" // IR(2412, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(2435, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(2470, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(2501, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(2535, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(2562, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(2610, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(2641, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(2670, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(2701, 33)
  "SPV_INTEL_fpga_reg\0" // IR(2734, 19)
  "SPV_INTEL_function_pointers\0" // IR(2753, 28)
  "SPV_INTEL_function_variants\0" // IR(2781, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(2809, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(2852, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2890, 26)
  "SPV_INTEL_int4\0" // IR(2916, 15)
  "SPV_INTEL_io_pipes\0" // IR(2931, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2950, 28)
  "SPV_INTEL_long_composites\0" // IR(2978, 26)
  "SPV_INTEL_loop_fuse\0" // IR(3004, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(3024, 32)
  "SPV_INTEL_maximum_registers\0" // IR(3056, 28)
  "SPV_INTEL_media_block_io\0" // IR(3084, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(3109, 33)
  "SPV_INTEL_optnone\0" // IR(3142, 18)
  "SPV_INTEL_predicated_io\0" // IR(3160, 24)
  "SPV_INTEL_rounded_divide_sqrt\0" // IR(3184, 30)
  "SPV_INTEL_runtime_aligned\0" // IR(3214, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(3240, 36)
  "SPV_INTEL_split_barrier\0" // IR(3276, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3300, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3335, 46)
  "SPV_INTEL_subgroups\0" // IR(3381, 20)
  "SPV_INTEL_task_sequence\0" // IR(3401, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3425, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3461, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3496, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3533, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3563, 32)
  "SPV_INTEL_vector_compute\0" // IR(3595, 25)
  "SPV_KHR_16bit_storage\0" // IR(3620, 22)
  "SPV_KHR_8bit_storage\0" // IR(3642, 21)
  "SPV_KHR_abort\0" // IR(3663, 14)
  "SPV_KHR_bfloat16\0" // IR(3677, 17)
  "SPV_KHR_bit_instructions\0" // IR(3694, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3719, 35)
  "SPV_KHR_constant_data\0" // IR(3754, 22)
  "SPV_KHR_cooperative_matrix\0" // IR(3776, 27)
  "SPV_KHR_device_group\0" // IR(3803, 21)
  "SPV_KHR_expect_assume\0" // IR(3824, 22)
  "SPV_KHR_float_controls\0" // IR(3846, 23)
  "SPV_KHR_float_controls2\0" // IR(3869, 24)
  "SPV_KHR_fma\0" // IR(3893, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3905, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3941, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3971, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3999, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(4020, 30)
  "SPV_KHR_multiview\0" // IR(4050, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(4068, 35)
  "SPV_KHR_non_semantic_info\0" // IR(4103, 26)
  "SPV_KHR_opacity_micromap\0" // IR(4129, 25)
  "SPV_KHR_physical_storage_buffer\0" // IR(4154, 32)
  "SPV_KHR_poison_freeze\0" // IR(4186, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(4208, 28)
  "SPV_KHR_quad_control\0" // IR(4236, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4257, 22)
  "SPV_KHR_ray_query\0" // IR(4279, 18)
  "SPV_KHR_ray_tracing\0" // IR(4297, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4317, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4352, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4389, 34)
  "SPV_KHR_shader_ballot\0" // IR(4423, 22)
  "SPV_KHR_shader_clock\0" // IR(4445, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4466, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4497, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4534, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4558, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4596, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4618, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4647, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4682, 25)
  "SPV_KHR_variable_pointers\0" // IR(4707, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4733, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4761, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4802, 38)
  "SPV_NV_bindless_texture\0" // IR(4840, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4864, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4902, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4936, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4962, 27)
  "SPV_NV_cooperative_matrix_decode_vector\0" // IR(4989, 40)
  "SPV_NV_cooperative_vector\0" // IR(5029, 26)
  "SPV_NV_displacement_micromap\0" // IR(5055, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(5084, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(5119, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(5154, 28)
  "SPV_NV_mesh_shader\0" // IR(5182, 19)
  "SPV_NV_push_constant_bank\0" // IR(5201, 26)
  "SPV_NV_raw_access_chains\0" // IR(5227, 25)
  "SPV_NV_ray_tracing\0" // IR(5252, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5271, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5302, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5339, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5372, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5402, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5435, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5461, 35)
  "SPV_NV_shading_rate\0" // IR(5496, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5516, 29)
  "SPV_NV_tensor_addressing\0" // IR(5545, 25)
  "SPV_NV_viewport_array2\0" // IR(5570, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5593, 39)
  "SPV_QCOM_image_processing\0" // IR(5632, 26)
  "SPV_QCOM_image_processing2\0" // IR(5658, 27)
  "SPV_QCOM_image_processing3\0" // IR(5685, 27)
  "SPV_QCOM_multiple_wait_queues\0" // IR(5712, 30)
  "SPV_QCOM_tile_shading\0" // IR(5742, 22)
  "SPV_SAMSUNG_intrinsic\0" // IR(5764, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5786, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5824, 34)
  "ReadOnly\0" // IR(5858, 9)
  "Kernel\0" // IR(5867, 7)
  "WriteOnly\0" // IR(5874, 10)
  "ReadWrite\0" // IR(5884, 10)
  "Logical\0" // IR(5894, 8)
  "Physical32\0" // IR(5902, 11)
  "Addresses\0" // IR(5913, 10)
  "Physical64\0" // IR(5923, 11)
  "PhysicalStorageBuffer64\0" // IR(5934, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(5958, 27)
  "PhysicalStorageBufferAddresses\0" // IR(5985, 31)
  "Position\0" // IR(6016, 9)
  "Shader\0" // IR(6025, 7)
  "PointSize\0" // IR(6032, 10)
  "ClipDistance\0" // IR(6042, 13)
  "CullDistance\0" // IR(6055, 13)
  "VertexId\0" // IR(6068, 9)
  "InstanceId\0" // IR(6077, 11)
  "PrimitiveId\0" // IR(6088, 12)
  "Geometry\0" // IR(6100, 9)
  "Tessellation\0" // IR(6109, 13)
  "RayTracingNV\0" // IR(6122, 13)
  "RayTracingKHR\0" // IR(6135, 14)
  "MeshShadingNV\0" // IR(6149, 14)
  "MeshShadingEXT\0" // IR(6163, 15)
  "InvocationId\0" // IR(6178, 13)
  "Layer\0" // IR(6191, 6)
  "ShaderLayer\0" // IR(6197, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(6209, 28)
  "ViewportIndex\0" // IR(6237, 14)
  "MultiViewport\0" // IR(6251, 14)
  "ShaderViewportIndex\0" // IR(6265, 20)
  "TessLevelOuter\0" // IR(6285, 15)
  "TessLevelInner\0" // IR(6300, 15)
  "TessCoord\0" // IR(6315, 10)
  "PatchVertices\0" // IR(6325, 14)
  "FragCoord\0" // IR(6339, 10)
  "PointCoord\0" // IR(6349, 11)
  "FrontFacing\0" // IR(6360, 12)
  "SampleId\0" // IR(6372, 9)
  "SampleRateShading\0" // IR(6381, 18)
  "SamplePosition\0" // IR(6399, 15)
  "SampleMask\0" // IR(6414, 11)
  "FragDepth\0" // IR(6425, 10)
  "HelperInvocation\0" // IR(6435, 17)
  "NumWorkgroups\0" // IR(6452, 14)
  "WorkgroupSize\0" // IR(6466, 14)
  "WorkgroupId\0" // IR(6480, 12)
  "LocalInvocationId\0" // IR(6492, 18)
  "GlobalInvocationId\0" // IR(6510, 19)
  "LocalInvocationIndex\0" // IR(6529, 21)
  "WorkDim\0" // IR(6550, 8)
  "GlobalSize\0" // IR(6558, 11)
  "EnqueuedWorkgroupSize\0" // IR(6569, 22)
  "GlobalOffset\0" // IR(6591, 13)
  "GlobalLinearId\0" // IR(6604, 15)
  "SubgroupSize\0" // IR(6619, 13)
  "GroupNonUniform\0" // IR(6632, 16)
  "SubgroupBallotKHR\0" // IR(6648, 18)
  "SubgroupMaxSize\0" // IR(6666, 16)
  "NumSubgroups\0" // IR(6682, 13)
  "NumEnqueuedSubgroups\0" // IR(6695, 21)
  "SubgroupId\0" // IR(6716, 11)
  "SubgroupLocalInvocationId\0" // IR(6727, 26)
  "VertexIndex\0" // IR(6753, 12)
  "InstanceIndex\0" // IR(6765, 14)
  "CoreIDARM\0" // IR(6779, 10)
  "CoreBuiltinsARM\0" // IR(6789, 16)
  "CoreCountARM\0" // IR(6805, 13)
  "CoreMaxIDARM\0" // IR(6818, 13)
  "WarpIDARM\0" // IR(6831, 10)
  "WarpMaxIDARM\0" // IR(6841, 13)
  "SubgroupEqMask\0" // IR(6854, 15)
  "SubgroupEqMaskKHR\0" // IR(6869, 18)
  "GroupNonUniformBallot\0" // IR(6887, 22)
  "SubgroupGeMask\0" // IR(6909, 15)
  "SubgroupGeMaskKHR\0" // IR(6924, 18)
  "SubgroupGtMask\0" // IR(6942, 15)
  "SubgroupGtMaskKHR\0" // IR(6957, 18)
  "SubgroupLeMask\0" // IR(6975, 15)
  "SubgroupLeMaskKHR\0" // IR(6990, 18)
  "SubgroupLtMask\0" // IR(7008, 15)
  "SubgroupLtMaskKHR\0" // IR(7023, 18)
  "BaseVertex\0" // IR(7041, 11)
  "DrawParameters\0" // IR(7052, 15)
  "BaseInstance\0" // IR(7067, 13)
  "DrawIndex\0" // IR(7080, 10)
  "PrimitiveShadingRateKHR\0" // IR(7090, 24)
  "FragmentShadingRateKHR\0" // IR(7114, 23)
  "DeviceIndex\0" // IR(7137, 12)
  "DeviceGroup\0" // IR(7149, 12)
  "ViewIndex\0" // IR(7161, 10)
  "MultiView\0" // IR(7171, 10)
  "ShadingRateKHR\0" // IR(7181, 15)
  "TileOffsetQCOM\0" // IR(7196, 15)
  "TileShadingQCOM\0" // IR(7211, 16)
  "TileDimensionQCOM\0" // IR(7227, 18)
  "TileApronSizeQCOM\0" // IR(7245, 18)
  "BaryCoordNoPerspAMD\0" // IR(7263, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(7283, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(7311, 26)
  "BaryCoordSmoothAMD\0" // IR(7337, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7356, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7383, 25)
  "BaryCoordPullModelAMD\0" // IR(7408, 22)
  "FragStencilRefEXT\0" // IR(7430, 18)
  "StencilExportEXT\0" // IR(7448, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7465, 29)
  "ShaderEnqueueAMDX\0" // IR(7494, 18)
  "ShaderIndexAMDX\0" // IR(7512, 16)
  "SamplerHeapEXT\0" // IR(7528, 15)
  "DescriptorHeapEXT\0" // IR(7543, 18)
  "ResourceHeapEXT\0" // IR(7561, 16)
  "ViewportMaskNV\0" // IR(7577, 15)
  "ShaderViewportMaskNV\0" // IR(7592, 21)
  "SecondaryPositionNV\0" // IR(7613, 20)
  "ShaderStereoViewNV\0" // IR(7633, 19)
  "SecondaryViewportMaskNV\0" // IR(7652, 24)
  "PositionPerViewNV\0" // IR(7676, 18)
  "PerViewAttributesNV\0" // IR(7694, 20)
  "ViewportMaskPerViewNV\0" // IR(7714, 22)
  "FullyCoveredEXT\0" // IR(7736, 16)
  "FragmentFullyCoveredEXT\0" // IR(7752, 24)
  "TaskCountNV\0" // IR(7776, 12)
  "PrimitiveCountNV\0" // IR(7788, 17)
  "PrimitiveIndicesNV\0" // IR(7805, 19)
  "ClipDistancePerViewNV\0" // IR(7824, 22)
  "CullDistancePerViewNV\0" // IR(7846, 22)
  "LayerPerViewNV\0" // IR(7868, 15)
  "MeshViewCountNV\0" // IR(7883, 16)
  "MeshViewIndicesNV\0" // IR(7899, 18)
  "BaryCoordKHR\0" // IR(7917, 13)
  "BaryCoordNV\0" // IR(7930, 12)
  "FragmentBarycentricKHR\0" // IR(7942, 23)
  "BaryCoordNoPerspKHR\0" // IR(7965, 20)
  "BaryCoordNoPerspNV\0" // IR(7985, 19)
  "FragSizeEXT\0" // IR(8004, 12)
  "FragmentSizeNV\0" // IR(8016, 15)
  "FragmentDensityEXT\0" // IR(8031, 19)
  "FragInvocationCountEXT\0" // IR(8050, 23)
  "InvocationsPerPixelNV\0" // IR(8073, 22)
  "PrimitivePointIndicesEXT\0" // IR(8095, 25)
  "PrimitiveLineIndicesEXT\0" // IR(8120, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(8144, 28)
  "CullPrimitiveEXT\0" // IR(8172, 17)
  "LaunchIdKHR\0" // IR(8189, 12)
  "LaunchIdNV\0" // IR(8201, 11)
  "LaunchSizeKHR\0" // IR(8212, 14)
  "LaunchSizeNV\0" // IR(8226, 13)
  "WorldRayOriginKHR\0" // IR(8239, 18)
  "WorldRayOriginNV\0" // IR(8257, 17)
  "WorldRayDirectionKHR\0" // IR(8274, 21)
  "WorldRayDirectionNV\0" // IR(8295, 20)
  "ObjectRayOriginKHR\0" // IR(8315, 19)
  "ObjectRayOriginNV\0" // IR(8334, 18)
  "ObjectRayDirectionKHR\0" // IR(8352, 22)
  "ObjectRayDirectionNV\0" // IR(8374, 21)
  "RayTminKHR\0" // IR(8395, 11)
  "RayTminNV\0" // IR(8406, 10)
  "RayTmaxKHR\0" // IR(8416, 11)
  "RayTmaxNV\0" // IR(8427, 10)
  "InstanceCustomIndexKHR\0" // IR(8437, 23)
  "InstanceCustomIndexNV\0" // IR(8460, 22)
  "ObjectToWorldKHR\0" // IR(8482, 17)
  "ObjectToWorldNV\0" // IR(8499, 16)
  "WorldToObjectKHR\0" // IR(8515, 17)
  "WorldToObjectNV\0" // IR(8532, 16)
  "HitTNV\0" // IR(8548, 7)
  "HitKindKHR\0" // IR(8555, 11)
  "HitKindNV\0" // IR(8566, 10)
  "CurrentRayTimeNV\0" // IR(8576, 17)
  "RayTracingMotionBlurNV\0" // IR(8593, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8616, 30)
  "RayTracingPositionFetchKHR\0" // IR(8646, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8673, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8707, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8740, 37)
  "IncomingRayFlagsKHR\0" // IR(8777, 20)
  "IncomingRayFlagsNV\0" // IR(8797, 19)
  "RayGeometryIndexKHR\0" // IR(8816, 20)
  "HitIsSphereNV\0" // IR(8836, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8850, 28)
  "HitIsLSSNV\0" // IR(8878, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8889, 39)
  "HitSpherePositionNV\0" // IR(8928, 20)
  "WarpsPerSMNV\0" // IR(8948, 13)
  "ShaderSMBuiltinsNV\0" // IR(8961, 19)
  "SMCountNV\0" // IR(8980, 10)
  "WarpIDNV\0" // IR(8990, 9)
  "SMIDNV\0" // IR(8999, 7)
  "HitLSSPositionsNV\0" // IR(9006, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(9024, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(9058, 33)
  "HitSphereRadiusNV\0" // IR(9091, 18)
  "HitLSSRadiiNV\0" // IR(9109, 14)
  "ClusterIDNV\0" // IR(9123, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(9135, 41)
  "CullMaskKHR\0" // IR(9176, 12)
  "RayCullMaskKHR\0" // IR(9188, 15)
  "Matrix\0" // IR(9203, 7)
  "Linkage\0" // IR(9210, 8)
  "Vector16\0" // IR(9218, 9)
  "Float16Buffer\0" // IR(9227, 14)
  "Float16\0" // IR(9241, 8)
  "Float64\0" // IR(9249, 8)
  "Int64\0" // IR(9257, 6)
  "Int64Atomics\0" // IR(9263, 13)
  "ImageBasic\0" // IR(9276, 11)
  "ImageReadWrite\0" // IR(9287, 15)
  "ImageMipmap\0" // IR(9302, 12)
  "Pipes\0" // IR(9314, 6)
  "Groups\0" // IR(9320, 7)
  "DeviceEnqueue\0" // IR(9327, 14)
  "LiteralSampler\0" // IR(9341, 15)
  "AtomicStorage\0" // IR(9356, 14)
  "Int16\0" // IR(9370, 6)
  "TessellationPointSize\0" // IR(9376, 22)
  "GeometryPointSize\0" // IR(9398, 18)
  "ImageGatherExtended\0" // IR(9416, 20)
  "StorageImageMultisample\0" // IR(9436, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9460, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9494, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9527, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9561, 33)
  "ImageCubeArray\0" // IR(9594, 15)
  "SampledCubeArray\0" // IR(9609, 17)
  "ImageRect\0" // IR(9626, 10)
  "SampledRect\0" // IR(9636, 12)
  "GenericPointer\0" // IR(9648, 15)
  "Int8\0" // IR(9663, 5)
  "InputAttachment\0" // IR(9668, 16)
  "SparseResidency\0" // IR(9684, 16)
  "MinLod\0" // IR(9700, 7)
  "Sampled1D\0" // IR(9707, 10)
  "Image1D\0" // IR(9717, 8)
  "SampledBuffer\0" // IR(9725, 14)
  "ImageBuffer\0" // IR(9739, 12)
  "ImageMSArray\0" // IR(9751, 13)
  "StorageImageExtendedFormats\0" // IR(9764, 28)
  "ImageQuery\0" // IR(9792, 11)
  "DerivativeControl\0" // IR(9803, 18)
  "InterpolationFunction\0" // IR(9821, 22)
  "TransformFeedback\0" // IR(9843, 18)
  "GeometryStreams\0" // IR(9861, 16)
  "StorageImageReadWithoutFormat\0" // IR(9877, 30)
  "StorageImageWriteWithoutFormat\0" // IR(9907, 31)
  "SubgroupDispatch\0" // IR(9938, 17)
  "NamedBarrier\0" // IR(9955, 13)
  "PipeStorage\0" // IR(9968, 12)
  "GroupNonUniformVote\0" // IR(9980, 20)
  "GroupNonUniformArithmetic\0" // IR(10000, 26)
  "GroupNonUniformShuffle\0" // IR(10026, 23)
  "GroupNonUniformShuffleRelative\0" // IR(10049, 31)
  "GroupNonUniformClustered\0" // IR(10080, 25)
  "GroupNonUniformQuad\0" // IR(10105, 20)
  "UniformDecoration\0" // IR(10125, 18)
  "TileImageColorReadAccessEXT\0" // IR(10143, 28)
  "TileImageDepthReadAccessEXT\0" // IR(10171, 28)
  "TileImageStencilReadAccessEXT\0" // IR(10199, 30)
  "TensorsARM\0" // IR(10229, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(10240, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(10277, 40)
  "GraphARM\0" // IR(10317, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(10326, 28)
  "Float8EXT\0" // IR(10354, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10364, 27)
  "CooperativeMatrixKHR\0" // IR(10391, 21)
  "Float6EXT\0" // IR(10412, 10)
  "Float4EXT\0" // IR(10422, 10)
  "Float8UnsignedE8M0EXT\0" // IR(10432, 22)
  "MXInt8EXT\0" // IR(10454, 10)
  "BitcastExtractEXT\0" // IR(10464, 18)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10482, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10515, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10558, 44)
  "SubgroupVoteKHR\0" // IR(10602, 16)
  "StorageBuffer16BitAccess\0" // IR(10618, 25)
  "StorageUniformBufferBlock16\0" // IR(10643, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10671, 35)
  "StorageUniform16\0" // IR(10706, 17)
  "StoragePushConstant16\0" // IR(10723, 22)
  "StorageInputOutput16\0" // IR(10745, 21)
  "VariablePointersStorageBuffer\0" // IR(10766, 30)
  "VariablePointers\0" // IR(10796, 17)
  "AtomicStorageOps\0" // IR(10813, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10830, 28)
  "StorageBuffer8BitAccess\0" // IR(10858, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10882, 34)
  "StoragePushConstant8\0" // IR(10916, 21)
  "DenormPreserve\0" // IR(10937, 15)
  "DenormFlushToZero\0" // IR(10952, 18)
  "SignedZeroInfNanPreserve\0" // IR(10970, 25)
  "RoundingModeRTE\0" // IR(10995, 16)
  "RoundingModeRTZ\0" // IR(11011, 16)
  "RayQueryProvisionalKHR\0" // IR(11027, 23)
  "RayQueryKHR\0" // IR(11050, 12)
  "UntypedPointersKHR\0" // IR(11062, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(11081, 32)
  "TextureSampleWeightedQCOM\0" // IR(11113, 26)
  "TextureBoxFilterQCOM\0" // IR(11139, 21)
  "TextureBlockMatchQCOM\0" // IR(11160, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(11182, 32)
  "TextureBlockMatch2QCOM\0" // IR(11214, 23)
  "MultipleWaitQueuesQCOM\0" // IR(11237, 23)
  "ImageGatherLinearQCOM\0" // IR(11260, 22)
  "ImageGatherExtendedModesQCOM\0" // IR(11282, 29)
  "Float16ImageAMD\0" // IR(11311, 16)
  "ImageGatherBiasLodAMD\0" // IR(11327, 22)
  "FragmentMaskAMD\0" // IR(11349, 16)
  "ImageReadWriteLodAMD\0" // IR(11365, 21)
  "Int64ImageEXT\0" // IR(11386, 14)
  "ShaderClockKHR\0" // IR(11400, 15)
  "QuadControlKHR\0" // IR(11415, 15)
  "Int4TypeINTEL\0" // IR(11430, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(11444, 27)
  "BFloat16TypeKHR\0" // IR(11471, 16)
  "BFloat16DotProductKHR\0" // IR(11487, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11509, 29)
  "AbortKHR\0" // IR(11538, 9)
  "ConstantDataKHR\0" // IR(11547, 16)
  "PoisonFreezeKHR\0" // IR(11563, 16)
  "WeakLinkageAMD\0" // IR(11579, 15)
  "SampleMaskOverrideCoverageNV\0" // IR(11594, 29)
  "GeometryShaderPassthroughNV\0" // IR(11623, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11651, 27)
  "ImageFootprintNV\0" // IR(11678, 17)
  "FragmentBarycentricNV\0" // IR(11695, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11717, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11748, 30)
  "ShadingRateNV\0" // IR(11778, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11792, 30)
  "GroupNonUniformPartitionedNV\0" // IR(11822, 29)
  "ShaderNonUniform\0" // IR(11851, 17)
  "ShaderNonUniformEXT\0" // IR(11868, 20)
  "RuntimeDescriptorArray\0" // IR(11888, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(11911, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(11937, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(11973, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(12012, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(12051, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(12093, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(12132, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(12174, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(12211, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(12251, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(12287, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(12326, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(12363, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(12403, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(12439, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(12478, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(12517, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12559, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12601, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12646, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12688, 45)
  "VulkanMemoryModel\0" // IR(12733, 18)
  "VulkanMemoryModelKHR\0" // IR(12751, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12772, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12801, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12833, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(12867, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(12899, 31)
  "RayTracingProvisionalKHR\0" // IR(12930, 25)
  "CooperativeMatrixNV\0" // IR(12955, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(12975, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(13008, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(13046, 32)
  "DemoteToHelperInvocation\0" // IR(13078, 25)
  "DemoteToHelperInvocationEXT\0" // IR(13103, 28)
  "DisplacementMicromapNV\0" // IR(13131, 23)
  "RayTracingOpacityMicromapKHR\0" // IR(13154, 29)
  "RayTracingOpacityMicromapEXT\0" // IR(13183, 29)
  "ShaderInvocationReorderNV\0" // IR(13212, 26)
  "ShaderInvocationReorderEXT\0" // IR(13238, 27)
  "BindlessTextureNV\0" // IR(13265, 18)
  "RayQueryPositionFetchKHR\0" // IR(13283, 25)
  "CooperativeVectorNV\0" // IR(13308, 20)
  "AtomicFloat16VectorNV\0" // IR(13328, 22)
  "RawAccessChainsNV\0" // IR(13350, 18)
  "PushConstantBanksNV\0" // IR(13368, 20)
  "LongVectorEXT\0" // IR(13388, 14)
  "Shader64BitIndexingEXT\0" // IR(13402, 23)
  "CooperativeMatrixConversionsEXT\0" // IR(13425, 32)
  "CooperativeMatrixReductionsEXT\0" // IR(13457, 31)
  "CooperativeMatrixReductionsNV\0" // IR(13488, 30)
  "CooperativeMatrixConversionsNV\0" // IR(13518, 31)
  "CooperativeMatrixPerElementOperationsEXT\0" // IR(13549, 41)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(13590, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(13630, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13666, 30)
  "CooperativeVectorTrainingNV\0" // IR(13696, 28)
  "CooperativeMatrixGetCoordinateEXT\0" // IR(13724, 34)
  "TensorAddressingNV\0" // IR(13758, 19)
  "CooperativeMatrixDecodeVectorNV\0" // IR(13777, 32)
  "SubgroupShuffleINTEL\0" // IR(13809, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(13830, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(13857, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(13883, 31)
  "RoundToInfinityINTEL\0" // IR(13914, 21)
  "FloatingPointModeINTEL\0" // IR(13935, 23)
  "IntegerFunctions2INTEL\0" // IR(13958, 23)
  "FunctionPointersINTEL\0" // IR(13981, 22)
  "IndirectReferencesINTEL\0" // IR(14003, 24)
  "AsmINTEL\0" // IR(14027, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(14036, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(14059, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(14082, 23)
  "VectorComputeINTEL\0" // IR(14105, 19)
  "VectorAnyINTEL\0" // IR(14124, 15)
  "ExpectAssumeKHR\0" // IR(14139, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(14155, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(14188, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(14226, 39)
  "VariableLengthArrayINTEL\0" // IR(14265, 25)
  "FunctionFloatControlINTEL\0" // IR(14290, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(14316, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(14343, 26)
  "FPFastMathModeINTEL\0" // IR(14369, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(14389, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(14422, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(14454, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(14492, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(14529, 30)
  "FPGALoopControlsALTERA\0" // IR(14559, 23)
  "FPGALoopControlsINTEL\0" // IR(14582, 22)
  "KernelAttributesINTEL\0" // IR(14604, 22)
  "FPGAKernelAttributesINTEL\0" // IR(14626, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(14652, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(14677, 24)
  "FPGAClusterAttributesALTERA\0" // IR(14701, 28)
  "FPGAClusterAttributesINTEL\0" // IR(14729, 27)
  "LoopFuseALTERA\0" // IR(14756, 15)
  "LoopFuseINTEL\0" // IR(14771, 14)
  "FPGADSPControlALTERA\0" // IR(14785, 21)
  "FPGADSPControlINTEL\0" // IR(14806, 20)
  "MemoryAccessAliasingINTEL\0" // IR(14826, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(14852, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(14893, 40)
  "FPGABufferLocationALTERA\0" // IR(14933, 25)
  "FPGABufferLocationINTEL\0" // IR(14958, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(14982, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(15017, 34)
  "USMStorageClassesALTERA\0" // IR(15051, 24)
  "USMStorageClassesINTEL\0" // IR(15075, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(15098, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(15128, 29)
  "IOPipesALTERA\0" // IR(15157, 14)
  "IOPipesINTEL\0" // IR(15171, 13)
  "BlockingPipesALTERA\0" // IR(15184, 20)
  "BlockingPipesINTEL\0" // IR(15204, 19)
  "FPGARegALTERA\0" // IR(15223, 14)
  "FPGARegINTEL\0" // IR(15237, 13)
  "DotProductInputAll\0" // IR(15250, 19)
  "DotProductInputAllKHR\0" // IR(15269, 22)
  "DotProductInput4x8Bit\0" // IR(15291, 22)
  "DotProductInput4x8BitKHR\0" // IR(15313, 25)
  "DotProductInput4x8BitPacked\0" // IR(15338, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(15366, 31)
  "DotProduct\0" // IR(15397, 11)
  "DotProductKHR\0" // IR(15408, 14)
  "ReplicatedCompositesEXT\0" // IR(15422, 24)
  "BitInstructions\0" // IR(15446, 16)
  "GroupNonUniformRotateKHR\0" // IR(15462, 25)
  "FloatControls2\0" // IR(15487, 15)
  "FMAKHR\0" // IR(15502, 7)
  "RayTracingOpacityMicromapExecutionModeKHR\0" // IR(15509, 42)
  "AtomicFloat32AddEXT\0" // IR(15551, 20)
  "AtomicFloat64AddEXT\0" // IR(15571, 20)
  "LongCompositesINTEL\0" // IR(15591, 20)
  "OptNoneEXT\0" // IR(15611, 11)
  "OptNoneINTEL\0" // IR(15622, 13)
  "AtomicFloat16AddEXT\0" // IR(15635, 20)
  "DebugInfoModuleINTEL\0" // IR(15655, 21)
  "BFloat16ConversionINTEL\0" // IR(15676, 24)
  "SplitBarrierEXT\0" // IR(15700, 16)
  "SplitBarrierINTEL\0" // IR(15716, 18)
  "ArithmeticFenceEXT\0" // IR(15734, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(15753, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(15783, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15812, 28)
  "TaskSequenceALTERA\0" // IR(15840, 19)
  "TaskSequenceINTEL\0" // IR(15859, 18)
  "FPMaxErrorINTEL\0" // IR(15877, 16)
  "FPGALatencyControlALTERA\0" // IR(15893, 25)
  "FPGALatencyControlINTEL\0" // IR(15918, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(15942, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(15971, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(15999, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(16029, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(16065, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(16100, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(16128, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(16151, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(16181, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(16211, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(16249, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(16277, 32)
  "SpecConditionalINTEL\0" // IR(16309, 21)
  "FunctionVariantsINTEL\0" // IR(16330, 22)
  "PredicatedIOINTEL\0" // IR(16352, 18)
  "RoundedDivideSqrtINTEL\0" // IR(16370, 23)
  "GroupUniformArithmeticKHR\0" // IR(16393, 26)
  "TensorFloat32RoundingINTEL\0" // IR(16419, 27)
  "MaskedGatherScatterINTEL\0" // IR(16446, 25)
  "CacheControlsINTEL\0" // IR(16471, 19)
  "RegisterLimitsINTEL\0" // IR(16490, 20)
  "BindlessImagesINTEL\0" // IR(16510, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(16530, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(16563, 33)
  "DotProductBFloat16AccVALVE\0" // IR(16596, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(16623, 32)
  "IntrinsicSAMSUNG\0" // IR(16655, 17)
  "Unspecified\0" // IR(16672, 12)
  "Address\0" // IR(16684, 8)
  "Boolean\0" // IR(16692, 8)
  "Float\0" // IR(16700, 6)
  "Signed\0" // IR(16706, 7)
  "SignedChar\0" // IR(16713, 11)
  "Unsigned\0" // IR(16724, 9)
  "UnsignedChar\0" // IR(16733, 13)
  "Class\0" // IR(16746, 6)
  "Structure\0" // IR(16752, 10)
  "Union\0" // IR(16762, 6)
  "ImportedModule\0" // IR(16768, 15)
  "ImportedDeclaration\0" // IR(16783, 20)
  "None\0" // IR(16803, 5)
  "FlagIsProtected\0" // IR(16808, 16)
  "FlagIsPrivate\0" // IR(16824, 14)
  "FlagIsPublic\0" // IR(16838, 13)
  "FlagIsLocal\0" // IR(16851, 12)
  "FlagIsDefinition\0" // IR(16863, 17)
  "FlagFwdDecl\0" // IR(16880, 12)
  "FlagArtificial\0" // IR(16892, 15)
  "FlagExplicit\0" // IR(16907, 13)
  "FlagPrototyped\0" // IR(16920, 15)
  "FlagObjectPointer\0" // IR(16935, 18)
  "FlagStaticMember\0" // IR(16953, 17)
  "FlagIndirectVariable\0" // IR(16970, 21)
  "FlagLValueReference\0" // IR(16991, 20)
  "FlagRValueReference\0" // IR(17011, 20)
  "FlagIsOptimized\0" // IR(17031, 16)
  "FlagIsEnumClass\0" // IR(17047, 16)
  "FlagTypePassByValue\0" // IR(17063, 20)
  "FlagTypePassByReference\0" // IR(17083, 24)
  "Deref\0" // IR(17107, 6)
  "Plus\0" // IR(17113, 5)
  "Minus\0" // IR(17118, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(17124, 33)
  "PlusUconst\0" // IR(17157, 11)
  "BitPiece\0" // IR(17168, 9)
  "Swap\0" // IR(17177, 5)
  "Xderef\0" // IR(17182, 7)
  "StackValue\0" // IR(17189, 11)
  "Constu\0" // IR(17200, 7)
  "Fragment\0" // IR(17207, 9)
  "ConstType\0" // IR(17216, 10)
  "VolatileType\0" // IR(17226, 13)
  "RestrictType\0" // IR(17239, 13)
  "AtomicType\0" // IR(17252, 11)
  "Float16NV\0" // IR(17263, 10)
  "Float32NV\0" // IR(17273, 10)
  "Float64NV\0" // IR(17283, 10)
  "SignedInt8NV\0" // IR(17293, 13)
  "SignedInt16NV\0" // IR(17306, 14)
  "SignedInt32NV\0" // IR(17320, 14)
  "SignedInt64NV\0" // IR(17334, 14)
  "UnsignedInt8NV\0" // IR(17348, 15)
  "UnsignedInt16NV\0" // IR(17363, 16)
  "UnsignedInt32NV\0" // IR(17379, 16)
  "UnsignedInt64NV\0" // IR(17395, 16)
  "SignedInt8PackedNV\0" // IR(17411, 19)
  "UnsignedInt8PackedNV\0" // IR(17430, 21)
  "FloatE4M3NV\0" // IR(17451, 12)
  "FloatE5M2NV\0" // IR(17463, 12)
  "RowMajorKHR\0" // IR(17475, 12)
  "ColumnMajorKHR\0" // IR(17487, 15)
  "RowBlockedInterleavedARM\0" // IR(17502, 25)
  "ColumnBlockedInterleavedARM\0" // IR(17527, 28)
  "NoneKHR\0" // IR(17555, 8)
  "MatrixASignedComponentsKHR\0" // IR(17563, 27)
  "MatrixBSignedComponentsKHR\0" // IR(17590, 27)
  "MatrixCSignedComponentsKHR\0" // IR(17617, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(17644, 32)
  "SaturatingAccumulationKHR\0" // IR(17676, 26)
  "Row\0" // IR(17702, 4)
  "Column\0" // IR(17706, 7)
  "2x2\0" // IR(17713, 4)
  "MatrixAKHR\0" // IR(17717, 11)
  "MatrixBKHR\0" // IR(17728, 11)
  "MatrixAccumulatorKHR\0" // IR(17739, 21)
  "RowMajorNV\0" // IR(17760, 11)
  "ColumnMajorNV\0" // IR(17771, 14)
  "InferencingOptimalNV\0" // IR(17785, 21)
  "TrainingOptimalNV\0" // IR(17806, 18)
  "RelaxedPrecision\0" // IR(17824, 17)
  "SpecId\0" // IR(17841, 7)
  "Block\0" // IR(17848, 6)
  "BufferBlock\0" // IR(17854, 12)
  "RowMajor\0" // IR(17866, 9)
  "ColMajor\0" // IR(17875, 9)
  "ArrayStride\0" // IR(17884, 12)
  "MatrixStride\0" // IR(17896, 13)
  "GLSLShared\0" // IR(17909, 11)
  "GLSLPacked\0" // IR(17920, 11)
  "CPacked\0" // IR(17931, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(17939, 26)
  "BuiltIn\0" // IR(17965, 8)
  "NoPerspective\0" // IR(17973, 14)
  "Flat\0" // IR(17987, 5)
  "Patch\0" // IR(17992, 6)
  "Centroid\0" // IR(17998, 9)
  "Sample\0" // IR(18007, 7)
  "Invariant\0" // IR(18014, 10)
  "Restrict\0" // IR(18024, 9)
  "Aliased\0" // IR(18033, 8)
  "Volatile\0" // IR(18041, 9)
  "Constant\0" // IR(18050, 9)
  "Coherent\0" // IR(18059, 9)
  "NonWritable\0" // IR(18068, 12)
  "NonReadable\0" // IR(18080, 12)
  "Uniform\0" // IR(18092, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(18100, 26)
  "UniformId\0" // IR(18126, 10)
  "SaturatedConversion\0" // IR(18136, 20)
  "Stream\0" // IR(18156, 7)
  "Location\0" // IR(18163, 9)
  "Component\0" // IR(18172, 10)
  "Index\0" // IR(18182, 6)
  "Binding\0" // IR(18188, 8)
  "DescriptorSet\0" // IR(18196, 14)
  "Offset\0" // IR(18210, 7)
  "XfbBuffer\0" // IR(18217, 10)
  "XfbStride\0" // IR(18227, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(18237, 46)
  "FuncParamAttr\0" // IR(18283, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(18297, 34)
  "FPRoundingMode\0" // IR(18331, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(18346, 35)
  "FPFastMathMode\0" // IR(18381, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(18396, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(18428, 30)
  "LinkageAttributes\0" // IR(18458, 18)
  "NoContraction\0" // IR(18476, 14)
  "InputAttachmentIndex\0" // IR(18490, 21)
  "Alignment\0" // IR(18511, 10)
  "MaxByteOffset\0" // IR(18521, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(18535, 20)
  "AlignmentId\0" // IR(18555, 12)
  "MaxByteOffsetId\0" // IR(18567, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(18583, 44)
  "NoSignedWrap\0" // IR(18627, 13)
  "NoUnsignedWrap\0" // IR(18640, 15)
  "WeightTextureQCOM\0" // IR(18655, 18)
  "BlockMatchTextureQCOM\0" // IR(18673, 22)
  "BlockMatchSamplerQCOM\0" // IR(18695, 22)
  "ExplicitInterpAMD\0" // IR(18717, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(18735, 32)
  "NodeMaxPayloadsAMDX\0" // IR(18767, 20)
  "TrackFinishWritingAMDX\0" // IR(18787, 23)
  "PayloadNodeNameAMDX\0" // IR(18810, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(18830, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(18855, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(18882, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(18907, 28)
  "ArrayStrideIdEXT\0" // IR(18935, 17)
  "OffsetIdEXT\0" // IR(18952, 12)
  "UTFEncodedKHR\0" // IR(18964, 14)
  "OverrideCoverageNV\0" // IR(18978, 19)
  "PassthroughNV\0" // IR(18997, 14)
  "ViewportRelativeNV\0" // IR(19011, 19)
  "SecondaryViewportRelativeNV\0" // IR(19030, 28)
  "PerPrimitiveEXT\0" // IR(19058, 16)
  "PerPrimitiveNV\0" // IR(19074, 15)
  "PerViewNV\0" // IR(19089, 10)
  "PerTaskNV\0" // IR(19099, 10)
  "PerVertexKHR\0" // IR(19109, 13)
  "PerVertexNV\0" // IR(19122, 12)
  "NonUniform\0" // IR(19134, 11)
  "NonUniformEXT\0" // IR(19145, 14)
  "RestrictPointer\0" // IR(19159, 16)
  "RestrictPointerEXT\0" // IR(19175, 19)
  "AliasedPointer\0" // IR(19194, 15)
  "AliasedPointerEXT\0" // IR(19209, 18)
  "MemberOffsetNV\0" // IR(19227, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(19242, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(19272, 31)
  "BankNV\0" // IR(19303, 7)
  "BindlessSamplerNV\0" // IR(19310, 18)
  "BindlessImageNV\0" // IR(19328, 16)
  "BoundSamplerNV\0" // IR(19344, 15)
  "BoundImageNV\0" // IR(19359, 13)
  "CooperativeMatrixTransposeEXT\0" // IR(19372, 30)
  "SIMTCallINTEL\0" // IR(19402, 14)
  "ReferencedIndirectlyINTEL\0" // IR(19416, 26)
  "ClobberINTEL\0" // IR(19442, 13)
  "SideEffectsINTEL\0" // IR(19455, 17)
  "VectorComputeVariableINTEL\0" // IR(19472, 27)
  "FuncParamIOKindINTEL\0" // IR(19499, 21)
  "VectorComputeFunctionINTEL\0" // IR(19520, 27)
  "StackCallINTEL\0" // IR(19547, 15)
  "GlobalVariableOffsetINTEL\0" // IR(19562, 26)
  "CounterBuffer\0" // IR(19588, 14)
  "HlslCounterBufferGOOGLE\0" // IR(19602, 24)
  "UserSemantic\0" // IR(19626, 13)
  "HlslSemanticGOOGLE\0" // IR(19639, 19)
  "UserTypeGOOGLE\0" // IR(19658, 15)
  "FunctionRoundingModeINTEL\0" // IR(19673, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(19699, 31)
  "FunctionDenormModeINTEL\0" // IR(19730, 24)
  "RegisterALTERA\0" // IR(19754, 15)
  "RegisterINTEL\0" // IR(19769, 14)
  "MemoryALTERA\0" // IR(19783, 13)
  "MemoryINTEL\0" // IR(19796, 12)
  "NumbanksALTERA\0" // IR(19808, 15)
  "NumbanksINTEL\0" // IR(19823, 14)
  "BankwidthALTERA\0" // IR(19837, 16)
  "BankwidthINTEL\0" // IR(19853, 15)
  "MaxPrivateCopiesALTERA\0" // IR(19868, 23)
  "MaxPrivateCopiesINTEL\0" // IR(19891, 22)
  "SinglepumpALTERA\0" // IR(19913, 17)
  "SinglepumpINTEL\0" // IR(19930, 16)
  "DoublepumpALTERA\0" // IR(19946, 17)
  "DoublepumpINTEL\0" // IR(19963, 16)
  "MaxReplicatesALTERA\0" // IR(19979, 20)
  "MaxReplicatesINTEL\0" // IR(19999, 19)
  "SimpleDualPortALTERA\0" // IR(20018, 21)
  "SimpleDualPortINTEL\0" // IR(20039, 20)
  "MergeALTERA\0" // IR(20059, 12)
  "MergeINTEL\0" // IR(20071, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(20082, 42)
  "BankBitsALTERA\0" // IR(20124, 15)
  "BankBitsINTEL\0" // IR(20139, 14)
  "ForcePow2DepthALTERA\0" // IR(20153, 21)
  "ForcePow2DepthINTEL\0" // IR(20174, 20)
  "StridesizeALTERA\0" // IR(20194, 17)
  "StridesizeINTEL\0" // IR(20211, 16)
  "WordsizeALTERA\0" // IR(20227, 15)
  "WordsizeINTEL\0" // IR(20242, 14)
  "TrueDualPortALTERA\0" // IR(20256, 19)
  "TrueDualPortINTEL\0" // IR(20275, 18)
  "BurstCoalesceALTERA\0" // IR(20293, 20)
  "BurstCoalesceINTEL\0" // IR(20313, 19)
  "CacheSizeALTERA\0" // IR(20332, 16)
  "CacheSizeINTEL\0" // IR(20348, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(20363, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(20392, 28)
  "PrefetchALTERA\0" // IR(20420, 15)
  "PrefetchINTEL\0" // IR(20435, 14)
  "StallEnableALTERA\0" // IR(20449, 18)
  "StallEnableINTEL\0" // IR(20467, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(20484, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(20510, 25)
  "MathOpDSPModeALTERA\0" // IR(20535, 20)
  "MathOpDSPModeINTEL\0" // IR(20555, 19)
  "AliasScopeINTEL\0" // IR(20574, 16)
  "NoAliasINTEL\0" // IR(20590, 13)
  "InitiationIntervalALTERA\0" // IR(20603, 25)
  "InitiationIntervalINTEL\0" // IR(20628, 24)
  "MaxConcurrencyALTERA\0" // IR(20652, 21)
  "MaxConcurrencyINTEL\0" // IR(20673, 20)
  "PipelineEnableALTERA\0" // IR(20693, 21)
  "PipelineEnableINTEL\0" // IR(20714, 20)
  "BufferLocationALTERA\0" // IR(20734, 21)
  "BufferLocationINTEL\0" // IR(20755, 20)
  "IOPipeStorageALTERA\0" // IR(20775, 20)
  "IOPipeStorageINTEL\0" // IR(20795, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(20814, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(20848, 31)
  "SingleElementVectorINTEL\0" // IR(20879, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(20904, 35)
  "MediaBlockIOINTEL\0" // IR(20939, 18)
  "StallFreeALTERA\0" // IR(20957, 16)
  "StallFreeINTEL\0" // IR(20973, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(20988, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(21019, 26)
  "LatencyControlLabelALTERA\0" // IR(21045, 26)
  "LatencyControlLabelINTEL\0" // IR(21071, 25)
  "LatencyControlConstraintALTERA\0" // IR(21096, 31)
  "LatencyControlConstraintINTEL\0" // IR(21127, 30)
  "ConduitKernelArgumentALTERA\0" // IR(21157, 28)
  "ConduitKernelArgumentINTEL\0" // IR(21185, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(21212, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(21244, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(21275, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(21309, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(21342, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(21373, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(21403, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(21432, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(21460, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(21494, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(21529, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(21563, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(21593, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(21622, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(21655, 32)
  "StableKernelArgumentALTERA\0" // IR(21687, 27)
  "StableKernelArgumentINTEL\0" // IR(21714, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(21740, 39)
  "HostAccessINTEL\0" // IR(21779, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(21795, 47)
  "InitModeALTERA\0" // IR(21842, 15)
  "InitModeINTEL\0" // IR(21857, 14)
  "ImplementInRegisterMapALTERA\0" // IR(21871, 29)
  "ImplementInRegisterMapINTEL\0" // IR(21900, 28)
  "ConditionalINTEL\0" // IR(21928, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(21945, 36)
  "CacheControlLoadINTEL\0" // IR(21981, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(22003, 37)
  "CacheControlStoreINTEL\0" // IR(22040, 23)
  "1D\0" // IR(22063, 3)
  "2D\0" // IR(22066, 3)
  "3D\0" // IR(22069, 3)
  "Cube\0" // IR(22072, 5)
  "Rect\0" // IR(22077, 5)
  "Buffer\0" // IR(22082, 7)
  "SubpassData\0" // IR(22089, 12)
  "TileImageDataEXT\0" // IR(22101, 17)
  "Invocations\0" // IR(22118, 12)
  "SpacingEqual\0" // IR(22130, 13)
  "SpacingFractionalEven\0" // IR(22143, 22)
  "SpacingFractionalOdd\0" // IR(22165, 21)
  "VertexOrderCw\0" // IR(22186, 14)
  "VertexOrderCcw\0" // IR(22200, 15)
  "PixelCenterInteger\0" // IR(22215, 19)
  "OriginUpperLeft\0" // IR(22234, 16)
  "OriginLowerLeft\0" // IR(22250, 16)
  "EarlyFragmentTests\0" // IR(22266, 19)
  "PointMode\0" // IR(22285, 10)
  "Xfb\0" // IR(22295, 4)
  "DepthReplacing\0" // IR(22299, 15)
  "DepthGreater\0" // IR(22314, 13)
  "DepthLess\0" // IR(22327, 10)
  "DepthUnchanged\0" // IR(22337, 15)
  "LocalSize\0" // IR(22352, 10)
  "LocalSizeHint\0" // IR(22362, 14)
  "InputPoints\0" // IR(22376, 12)
  "InputLines\0" // IR(22388, 11)
  "InputLinesAdjacency\0" // IR(22399, 20)
  "Triangles\0" // IR(22419, 10)
  "InputTrianglesAdjacency\0" // IR(22429, 24)
  "Quads\0" // IR(22453, 6)
  "Isolines\0" // IR(22459, 9)
  "OutputVertices\0" // IR(22468, 15)
  "OutputPoints\0" // IR(22483, 13)
  "OutputLineStrip\0" // IR(22496, 16)
  "OutputTriangleStrip\0" // IR(22512, 20)
  "VecTypeHint\0" // IR(22532, 12)
  "ContractionOff\0" // IR(22544, 15)
  "Initializer\0" // IR(22559, 12)
  "Finalizer\0" // IR(22571, 10)
  "SubgroupsPerWorkgroup\0" // IR(22581, 22)
  "SubgroupsPerWorkgroupId\0" // IR(22603, 24)
  "LocalSizeId\0" // IR(22627, 12)
  "LocalSizeHintId\0" // IR(22639, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(22655, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(22689, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(22723, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(22759, 30)
  "PostDepthCoverage\0" // IR(22789, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(22807, 34)
  "TileShadingRateQCOM\0" // IR(22841, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(22861, 29)
  "StencilRefReplacingEXT\0" // IR(22890, 23)
  "CoalescingAMDX\0" // IR(22913, 15)
  "IsApiEntryAMDX\0" // IR(22928, 15)
  "MaxNodeRecursionAMDX\0" // IR(22943, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(22964, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(22988, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(23009, 28)
  "StencilRefGreaterFrontAMD\0" // IR(23037, 26)
  "StencilRefLessFrontAMD\0" // IR(23063, 23)
  "StencilRefUnchangedBackAMD\0" // IR(23086, 27)
  "StencilRefGreaterBackAMD\0" // IR(23113, 25)
  "StencilRefLessBackAMD\0" // IR(23138, 22)
  "QuadDerivativesKHR\0" // IR(23160, 19)
  "RequireFullQuadsKHR\0" // IR(23179, 20)
  "SharesInputWithAMDX\0" // IR(23199, 20)
  "ArithmeticPoisonKHR\0" // IR(23219, 20)
  "OutputLinesEXT\0" // IR(23239, 15)
  "OutputLinesNV\0" // IR(23254, 14)
  "OutputPrimitivesEXT\0" // IR(23268, 20)
  "OutputPrimitivesNV\0" // IR(23288, 19)
  "DerivativeGroupQuadsKHR\0" // IR(23307, 24)
  "DerivativeGroupQuadsNV\0" // IR(23331, 23)
  "DerivativeGroupLinearKHR\0" // IR(23354, 25)
  "DerivativeGroupLinearNV\0" // IR(23379, 24)
  "OutputTrianglesEXT\0" // IR(23403, 19)
  "OutputTrianglesNV\0" // IR(23422, 18)
  "PixelInterlockOrderedEXT\0" // IR(23440, 25)
  "PixelInterlockUnorderedEXT\0" // IR(23465, 27)
  "SampleInterlockOrderedEXT\0" // IR(23492, 26)
  "SampleInterlockUnorderedEXT\0" // IR(23518, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(23546, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(23577, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(23610, 27)
  "RoundingModeRTPINTEL\0" // IR(23637, 21)
  "RoundingModeRTNINTEL\0" // IR(23658, 21)
  "FloatingPointModeALTINTEL\0" // IR(23679, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(23705, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(23732, 22)
  "MaxWorkDimINTEL\0" // IR(23754, 16)
  "NoGlobalOffsetINTEL\0" // IR(23770, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(23790, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(23812, 28)
  "MaximallyReconvergesKHR\0" // IR(23840, 24)
  "FPFastMathDefault\0" // IR(23864, 18)
  "OpacityMicromapIdKHR\0" // IR(23882, 21)
  "StreamingInterfaceINTEL\0" // IR(23903, 24)
  "RegisterMapInterfaceINTEL\0" // IR(23927, 26)
  "NamedBarrierCountINTEL\0" // IR(23953, 23)
  "MaximumRegistersINTEL\0" // IR(23976, 22)
  "MaximumRegistersIdINTEL\0" // IR(23998, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(24022, 51)
  "NamedMaximumRegistersINTEL\0" // IR(24073, 27)
  "Vertex\0" // IR(24100, 7)
  "TessellationControl\0" // IR(24107, 20)
  "TessellationEvaluation\0" // IR(24127, 23)
  "GLCompute\0" // IR(24150, 10)
  "TaskNV\0" // IR(24160, 7)
  "MeshNV\0" // IR(24167, 7)
  "RayGenerationKHR\0" // IR(24174, 17)
  "RayGenerationNV\0" // IR(24191, 16)
  "IntersectionKHR\0" // IR(24207, 16)
  "IntersectionNV\0" // IR(24223, 15)
  "AnyHitKHR\0" // IR(24238, 10)
  "AnyHitNV\0" // IR(24248, 9)
  "ClosestHitKHR\0" // IR(24257, 14)
  "ClosestHitNV\0" // IR(24271, 13)
  "MissKHR\0" // IR(24284, 8)
  "MissNV\0" // IR(24292, 7)
  "CallableKHR\0" // IR(24299, 12)
  "CallableNV\0" // IR(24311, 11)
  "TaskEXT\0" // IR(24322, 8)
  "MeshEXT\0" // IR(24330, 8)
  "Preserve\0" // IR(24338, 9)
  "FlushToZero\0" // IR(24347, 12)
  "BFloat16KHR\0" // IR(24359, 12)
  "Float8E4M3EXT\0" // IR(24371, 14)
  "Float8E5M2EXT\0" // IR(24385, 14)
  "Float6E2M3EXT\0" // IR(24399, 14)
  "Float6E3M2EXT\0" // IR(24413, 14)
  "Float4E2M1EXT\0" // IR(24427, 14)
  "IEEE\0" // IR(24441, 5)
  "ALT\0" // IR(24446, 4)
  "NotNaN\0" // IR(24450, 7)
  "NotInf\0" // IR(24457, 7)
  "NSZ\0" // IR(24464, 4)
  "AllowRecip\0" // IR(24468, 11)
  "Fast\0" // IR(24479, 5)
  "AllowContract\0" // IR(24484, 14)
  "AllowContractFastINTEL\0" // IR(24498, 23)
  "AllowReassoc\0" // IR(24521, 13)
  "AllowReassocINTEL\0" // IR(24534, 18)
  "AllowTransform\0" // IR(24552, 15)
  "RTE\0" // IR(24567, 4)
  "RTZ\0" // IR(24571, 4)
  "RTP\0" // IR(24575, 4)
  "RTN\0" // IR(24579, 4)
  "Vertical2Pixels\0" // IR(24583, 16)
  "Vertical4Pixels\0" // IR(24599, 16)
  "Horizontal2Pixels\0" // IR(24615, 18)
  "Horizontal4Pixels\0" // IR(24633, 18)
  "Inline\0" // IR(24651, 7)
  "DontInline\0" // IR(24658, 11)
  "Pure\0" // IR(24669, 5)
  "Const\0" // IR(24674, 6)
  "Zext\0" // IR(24680, 5)
  "Sext\0" // IR(24685, 5)
  "ByVal\0" // IR(24690, 6)
  "Sret\0" // IR(24696, 5)
  "NoAlias\0" // IR(24701, 8)
  "NoCapture\0" // IR(24709, 10)
  "NoWrite\0" // IR(24719, 8)
  "NoReadWrite\0" // IR(24727, 12)
  "RuntimeAlignedALTERA\0" // IR(24739, 21)
  "RuntimeAlignedINTEL\0" // IR(24760, 20)
  "Gather4x1QCOM\0" // IR(24780, 14)
  "GatherDQCOM\0" // IR(24794, 12)
  "GatherH2QCOM\0" // IR(24806, 13)
  "GatherV2QCOM\0" // IR(24819, 13)
  "Reduce\0" // IR(24832, 7)
  "InclusiveScan\0" // IR(24839, 14)
  "ExclusiveScan\0" // IR(24853, 14)
  "ClusteredReduce\0" // IR(24867, 16)
  "PartitionedReduceEXT\0" // IR(24883, 21)
  "PartitionedReduceNV\0" // IR(24904, 20)
  "PartitionedInclusiveScanEXT\0" // IR(24924, 28)
  "PartitionedInclusiveScanNV\0" // IR(24952, 27)
  "PartitionedExclusiveScanEXT\0" // IR(24979, 28)
  "PartitionedExclusiveScanNV\0" // IR(25007, 27)
  "NoneINTEL\0" // IR(25034, 10)
  "ReadINTEL\0" // IR(25044, 10)
  "WriteINTEL\0" // IR(25054, 11)
  "ReadWriteINTEL\0" // IR(25065, 15)
  "Bias\0" // IR(25080, 5)
  "Lod\0" // IR(25085, 4)
  "Grad\0" // IR(25089, 5)
  "ConstOffset\0" // IR(25094, 12)
  "ConstOffsets\0" // IR(25106, 13)
  "MakeTexelAvailable\0" // IR(25119, 19)
  "MakeTexelAvailableKHR\0" // IR(25138, 22)
  "MakeTexelVisible\0" // IR(25160, 17)
  "MakeTexelVisibleKHR\0" // IR(25177, 20)
  "NonPrivateTexel\0" // IR(25197, 16)
  "NonPrivateTexelKHR\0" // IR(25213, 19)
  "VolatileTexel\0" // IR(25232, 14)
  "VolatileTexelKHR\0" // IR(25246, 17)
  "SignExtend\0" // IR(25263, 11)
  "ZeroExtend\0" // IR(25274, 11)
  "Nontemporal\0" // IR(25285, 12)
  "Offsets\0" // IR(25297, 8)
  "SnormInt8\0" // IR(25305, 10)
  "SnormInt16\0" // IR(25315, 11)
  "UnormInt8\0" // IR(25326, 10)
  "UnormInt16\0" // IR(25336, 11)
  "UnormShort565\0" // IR(25347, 14)
  "UnormShort555\0" // IR(25361, 14)
  "UnormInt101010\0" // IR(25375, 15)
  "SignedInt8\0" // IR(25390, 11)
  "SignedInt16\0" // IR(25401, 12)
  "SignedInt32\0" // IR(25413, 12)
  "UnsignedInt8\0" // IR(25425, 13)
  "UnsignedInt16\0" // IR(25438, 14)
  "UnsignedInt32\0" // IR(25452, 14)
  "HalfFloat\0" // IR(25466, 10)
  "UnormInt24\0" // IR(25476, 11)
  "UnormInt101010_2\0" // IR(25487, 17)
  "UnormInt10X6EXT\0" // IR(25504, 16)
  "UnsignedIntRaw10EXT\0" // IR(25520, 20)
  "UnsignedIntRaw12EXT\0" // IR(25540, 20)
  "UnormInt2_101010EXT\0" // IR(25560, 20)
  "UnsignedInt10X6EXT\0" // IR(25580, 19)
  "UnsignedInt12X4EXT\0" // IR(25599, 19)
  "UnsignedInt14X2EXT\0" // IR(25618, 19)
  "UnormInt12X4EXT\0" // IR(25637, 16)
  "UnormInt14X2EXT\0" // IR(25653, 16)
  "R\0" // IR(25669, 2)
  "A\0" // IR(25671, 2)
  "RG\0" // IR(25673, 3)
  "RA\0" // IR(25676, 3)
  "RGB\0" // IR(25679, 4)
  "RGBA\0" // IR(25683, 5)
  "BGRA\0" // IR(25688, 5)
  "ARGB\0" // IR(25693, 5)
  "Intensity\0" // IR(25698, 10)
  "Luminance\0" // IR(25708, 10)
  "Rx\0" // IR(25718, 3)
  "RGx\0" // IR(25721, 4)
  "RGBx\0" // IR(25725, 5)
  "Depth\0" // IR(25730, 6)
  "DepthStencil\0" // IR(25736, 13)
  "sRGB\0" // IR(25749, 5)
  "sRGBx\0" // IR(25754, 6)
  "sRGBA\0" // IR(25760, 6)
  "sBGRA\0" // IR(25766, 6)
  "ABGR\0" // IR(25772, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(25777, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(25805, 27)
  "InitOnDeviceResetALTERA\0" // IR(25832, 24)
  "InitOnDeviceResetINTEL\0" // IR(25856, 23)
  "NoWait\0" // IR(25879, 7)
  "WaitKernel\0" // IR(25886, 11)
  "WaitWorkGroup\0" // IR(25897, 14)
  "CmdExecTime\0" // IR(25911, 12)
  "MayUsePrintf\0" // IR(25923, 13)
  "Export\0" // IR(25936, 7)
  "Import\0" // IR(25943, 7)
  "LinkOnceODR\0" // IR(25950, 12)
  "WeakAMD\0" // IR(25962, 8)
  "UncachedINTEL\0" // IR(25970, 14)
  "CachedINTEL\0" // IR(25984, 12)
  "StreamingINTEL\0" // IR(25996, 15)
  "InvalidateAfterReadINTEL\0" // IR(26011, 25)
  "ConstCachedINTEL\0" // IR(26036, 17)
  "Unroll\0" // IR(26053, 7)
  "DontUnroll\0" // IR(26060, 11)
  "DependencyInfinite\0" // IR(26071, 19)
  "DependencyLength\0" // IR(26090, 17)
  "MinIterations\0" // IR(26107, 14)
  "MaxIterations\0" // IR(26121, 14)
  "IterationMultiple\0" // IR(26135, 18)
  "PeelCount\0" // IR(26153, 10)
  "PartialCount\0" // IR(26163, 13)
  "DependencyArrayALTERA\0" // IR(26176, 22)
  "DependencyArrayINTEL\0" // IR(26198, 21)
  "LoopCoalesceALTERA\0" // IR(26219, 19)
  "LoopCoalesceINTEL\0" // IR(26238, 18)
  "MaxInterleavingALTERA\0" // IR(26256, 22)
  "MaxInterleavingINTEL\0" // IR(26278, 21)
  "SpeculatedIterationsALTERA\0" // IR(26299, 27)
  "SpeculatedIterationsINTEL\0" // IR(26326, 26)
  "NoFusionALTERA\0" // IR(26352, 15)
  "NoFusionINTEL\0" // IR(26367, 14)
  "LoopCountALTERA\0" // IR(26381, 16)
  "LoopCountINTEL\0" // IR(26397, 15)
  "MaxReinvocationDelayALTERA\0" // IR(26412, 27)
  "MaxReinvocationDelayINTEL\0" // IR(26439, 26)
  "MatrixASignedComponentsINTEL\0" // IR(26465, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(26494, 29)
  "MatrixCBFloat16INTEL\0" // IR(26523, 21)
  "MatrixResultBFloat16INTEL\0" // IR(26544, 26)
  "MatrixAPackedInt8INTEL\0" // IR(26570, 23)
  "MatrixBPackedInt8INTEL\0" // IR(26593, 23)
  "MatrixAPackedInt4INTEL\0" // IR(26616, 23)
  "MatrixBPackedInt4INTEL\0" // IR(26639, 23)
  "MatrixATF32INTEL\0" // IR(26662, 17)
  "MatrixBTF32INTEL\0" // IR(26679, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(26696, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(26722, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(26748, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(26775, 27)
  "Aligned\0" // IR(26802, 8)
  "MakePointerAvailable\0" // IR(26810, 21)
  "MakePointerAvailableKHR\0" // IR(26831, 24)
  "MakePointerVisible\0" // IR(26855, 19)
  "MakePointerVisibleKHR\0" // IR(26874, 22)
  "NonPrivatePointer\0" // IR(26896, 18)
  "NonPrivatePointerKHR\0" // IR(26914, 21)
  "AliasScopeINTELMask\0" // IR(26935, 20)
  "NoAliasINTELMask\0" // IR(26955, 17)
  "Simple\0" // IR(26972, 7)
  "GLSL450\0" // IR(26979, 8)
  "OpenCL\0" // IR(26987, 7)
  "Vulkan\0" // IR(26994, 7)
  "VulkanKHR\0" // IR(27001, 10)
  "Relaxed\0" // IR(27011, 8)
  "Acquire\0" // IR(27019, 8)
  "Release\0" // IR(27027, 8)
  "AcquireRelease\0" // IR(27035, 15)
  "SequentiallyConsistent\0" // IR(27050, 23)
  "UniformMemory\0" // IR(27073, 14)
  "SubgroupMemory\0" // IR(27087, 15)
  "WorkgroupMemory\0" // IR(27102, 16)
  "CrossWorkgroupMemory\0" // IR(27118, 21)
  "AtomicCounterMemory\0" // IR(27139, 20)
  "ImageMemory\0" // IR(27159, 12)
  "OutputMemory\0" // IR(27171, 13)
  "OutputMemoryKHR\0" // IR(27184, 16)
  "MakeAvailable\0" // IR(27200, 14)
  "MakeAvailableKHR\0" // IR(27214, 17)
  "MakeVisible\0" // IR(27231, 12)
  "MakeVisibleKHR\0" // IR(27243, 15)
  "AutoINTEL\0" // IR(27258, 10)
  "WRAP\0" // IR(27268, 5)
  "SAT\0" // IR(27273, 4)
  "SAT_ZERO\0" // IR(27277, 9)
  "SAT_SYM\0" // IR(27286, 8)
  "PackedVectorFormat4x8Bit\0" // IR(27294, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(27319, 28)
  "TRN\0" // IR(27347, 4)
  "TRN_ZERO\0" // IR(27351, 9)
  "RND\0" // IR(27360, 4)
  "RND_ZERO\0" // IR(27364, 9)
  "RND_INF\0" // IR(27373, 8)
  "RND_MIN_INF\0" // IR(27381, 12)
  "RND_CONV\0" // IR(27393, 9)
  "RND_CONV_ODD\0" // IR(27402, 13)
  "RobustnessPerComponentNV\0" // IR(27415, 25)
  "RobustnessPerElementNV\0" // IR(27440, 23)
  "OpaqueKHR\0" // IR(27463, 10)
  "NoOpaqueKHR\0" // IR(27473, 12)
  "TerminateOnFirstHitKHR\0" // IR(27485, 23)
  "SkipClosestHitShaderKHR\0" // IR(27508, 24)
  "CullBackFacingTrianglesKHR\0" // IR(27532, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(27559, 28)
  "CullOpaqueKHR\0" // IR(27587, 14)
  "CullNoOpaqueKHR\0" // IR(27601, 16)
  "SkipTrianglesKHR\0" // IR(27617, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(27634, 24)
  "SkipAABBsKHR\0" // IR(27658, 13)
  "ForceOpacityMicromap2StateKHR\0" // IR(27671, 30)
  "ForceOpacityMicromap2StateEXT\0" // IR(27701, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(27731, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(27772, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(27809, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(27846, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(27887, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(27929, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(27962, 33)
  "ClampToEdge\0" // IR(27995, 12)
  "Clamp\0" // IR(28007, 6)
  "Repeat\0" // IR(28013, 7)
  "RepeatMirrored\0" // IR(28020, 15)
  "Nearest\0" // IR(28035, 8)
  "Linear\0" // IR(28043, 7)
  "Unknown\0" // IR(28050, 8)
  "Rgba32f\0" // IR(28058, 8)
  "Rgba16f\0" // IR(28066, 8)
  "R32f\0" // IR(28074, 5)
  "Rgba8\0" // IR(28079, 6)
  "Rgba8Snorm\0" // IR(28085, 11)
  "Rg32f\0" // IR(28096, 6)
  "Rg16f\0" // IR(28102, 6)
  "R11fG11fB10f\0" // IR(28108, 13)
  "R16f\0" // IR(28121, 5)
  "Rgba16\0" // IR(28126, 7)
  "Rgb10A2\0" // IR(28133, 8)
  "Rg16\0" // IR(28141, 5)
  "Rg8\0" // IR(28146, 4)
  "R16\0" // IR(28150, 4)
  "R8\0" // IR(28154, 3)
  "Rgba16Snorm\0" // IR(28157, 12)
  "Rg16Snorm\0" // IR(28169, 10)
  "Rg8Snorm\0" // IR(28179, 9)
  "R16Snorm\0" // IR(28188, 9)
  "R8Snorm\0" // IR(28197, 8)
  "Rgba32i\0" // IR(28205, 8)
  "Rgba16i\0" // IR(28213, 8)
  "Rgba8i\0" // IR(28221, 7)
  "R32i\0" // IR(28228, 5)
  "Rg32i\0" // IR(28233, 6)
  "Rg16i\0" // IR(28239, 6)
  "Rg8i\0" // IR(28245, 5)
  "R16i\0" // IR(28250, 5)
  "R8i\0" // IR(28255, 4)
  "Rgba32ui\0" // IR(28259, 9)
  "Rgba16ui\0" // IR(28268, 9)
  "Rgba8ui\0" // IR(28277, 8)
  "R32ui\0" // IR(28285, 6)
  "Rgb10a2ui\0" // IR(28291, 10)
  "Rg32ui\0" // IR(28301, 7)
  "Rg16ui\0" // IR(28308, 7)
  "Rg8ui\0" // IR(28315, 6)
  "R16ui\0" // IR(28321, 6)
  "R8ui\0" // IR(28327, 5)
  "R64ui\0" // IR(28332, 6)
  "R64i\0" // IR(28338, 5)
  "CrossDevice\0" // IR(28343, 12)
  "Device\0" // IR(28355, 7)
  "Workgroup\0" // IR(28362, 10)
  "Subgroup\0" // IR(28372, 9)
  "Invocation\0" // IR(28381, 11)
  "QueueFamily\0" // IR(28392, 12)
  "QueueFamilyKHR\0" // IR(28404, 15)
  "ShaderCallKHR\0" // IR(28419, 14)
  "Flatten\0" // IR(28433, 8)
  "DontFlatten\0" // IR(28441, 12)
  "IdentifierPossibleDuplicates\0" // IR(28453, 29)
  "FlagUnknownPhysicalLayout\0" // IR(28482, 26)
  "ESSL\0" // IR(28508, 5)
  "GLSL\0" // IR(28513, 5)
  "OpenCL_C\0" // IR(28518, 9)
  "OpenCL_CPP\0" // IR(28527, 11)
  "HLSL\0" // IR(28538, 5)
  "CPP_for_OpenCL\0" // IR(28543, 15)
  "SYCL\0" // IR(28558, 5)
  "HERO_C\0" // IR(28563, 7)
  "NZSL\0" // IR(28570, 5)
  "WGSL\0" // IR(28575, 5)
  "Slang\0" // IR(28580, 6)
  "Zig\0" // IR(28586, 4)
  "Rust\0" // IR(28590, 5)
  "Pred\0" // IR(28595, 5)
  "ApilaJai\0" // IR(28600, 9)
  "UniformConstant\0" // IR(28609, 16)
  "Input\0" // IR(28625, 6)
  "Output\0" // IR(28631, 7)
  "CrossWorkgroup\0" // IR(28638, 15)
  "Private\0" // IR(28653, 8)
  "Function\0" // IR(28661, 9)
  "Generic\0" // IR(28670, 8)
  "PushConstant\0" // IR(28678, 13)
  "AtomicCounter\0" // IR(28691, 14)
  "Image\0" // IR(28705, 6)
  "StorageBuffer\0" // IR(28711, 14)
  "TileImageEXT\0" // IR(28725, 13)
  "TileAttachmentQCOM\0" // IR(28738, 19)
  "NodePayloadAMDX\0" // IR(28757, 16)
  "CallableDataKHR\0" // IR(28773, 16)
  "CallableDataNV\0" // IR(28789, 15)
  "IncomingCallableDataKHR\0" // IR(28804, 24)
  "IncomingCallableDataNV\0" // IR(28828, 23)
  "RayPayloadKHR\0" // IR(28851, 14)
  "RayPayloadNV\0" // IR(28865, 13)
  "HitAttributeKHR\0" // IR(28878, 16)
  "HitAttributeNV\0" // IR(28894, 15)
  "IncomingRayPayloadKHR\0" // IR(28909, 22)
  "IncomingRayPayloadNV\0" // IR(28931, 21)
  "ShaderRecordBufferKHR\0" // IR(28952, 22)
  "ShaderRecordBufferNV\0" // IR(28974, 21)
  "PhysicalStorageBuffer\0" // IR(28995, 22)
  "PhysicalStorageBufferEXT\0" // IR(29017, 25)
  "HitObjectAttributeNV\0" // IR(29042, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(29063, 24)
  "HitObjectAttributeEXT\0" // IR(29087, 22)
  "CodeSectionINTEL\0" // IR(29109, 17)
  "DeviceOnlyALTERA\0" // IR(29126, 17)
  "DeviceOnlyINTEL\0" // IR(29143, 16)
  "HostOnlyALTERA\0" // IR(29159, 15)
  "HostOnlyINTEL\0" // IR(29174, 14)
  "WriteThroughINTEL\0" // IR(29188, 18)
  "WriteBackINTEL\0" // IR(29206, 15)
  "TensorView\0" // IR(29221, 11)
  "DecodeFunc\0" // IR(29232, 11)
  "DecodeVectorFunc\0" // IR(29243, 17)
  "Undefined\0" // IR(29260, 10)
  "NoneARM\0" // IR(29270, 8)
  "NontemporalARM\0" // IR(29278, 15)
  "OutOfBoundsValueARM\0" // IR(29293, 20)
  "MakeElementAvailableARM\0" // IR(29313, 24)
  "MakeElementVisibleARM\0" // IR(29337, 22)
  "NonPrivateElementARM\0" // IR(29359, 21)
  "Nop\0" // IR(29380, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(29384, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(29409, 27)
  "Undef\0" // IR(29436, 6)
  "SourceContinued\0" // IR(29442, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(29458, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(29491, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(29520, 41)
  "Source\0" // IR(29561, 7)
  "SourceExtension\0" // IR(29568, 16)
  "Name\0" // IR(29584, 5)
  "MemberName\0" // IR(29589, 11)
  "String\0" // IR(29600, 7)
  "Line\0" // IR(29607, 5)
  "Extension\0" // IR(29612, 10)
  "ExtInstImport\0" // IR(29622, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(29636, 46)
  "ExtInst\0" // IR(29682, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(29690, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(29724, 30)
  "MemoryModel\0" // IR(29754, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(29766, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(29799, 29)
  "EntryPoint\0" // IR(29828, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(29839, 32)
  "ExecutionMode\0" // IR(29871, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(29885, 28)
  "Capability\0" // IR(29913, 11)
  "TypeVoid\0" // IR(29924, 9)
  "TypeBool\0" // IR(29933, 9)
  "TypeInt\0" // IR(29942, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(29950, 37)
  "TypeFloat\0" // IR(29987, 10)
  "TypeVector\0" // IR(29997, 11)
  "TypeMatrix\0" // IR(30008, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(30019, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(30051, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(30089, 43)
  "TypeImage\0" // IR(30132, 10)
  "TypeSampler\0" // IR(30142, 12)
  "TypeSampledImage\0" // IR(30154, 17)
  "TypeArray\0" // IR(30171, 10)
  "TypeRuntimeArray\0" // IR(30181, 17)
  "TypeStruct\0" // IR(30198, 11)
  "TypeOpaque\0" // IR(30209, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(30220, 31)
  "TypePointer\0" // IR(30251, 12)
  "TypeFunction\0" // IR(30263, 13)
  "TypeEvent\0" // IR(30276, 10)
  "TypeDeviceEvent\0" // IR(30286, 16)
  "TypeReserveId\0" // IR(30302, 14)
  "TypeQueue\0" // IR(30316, 10)
  "TypePipe\0" // IR(30326, 9)
  "TypeForwardPointer\0" // IR(30335, 19)
  "ConstantTrue\0" // IR(30354, 13)
  "ConstantFalse\0" // IR(30367, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(30381, 38)
  "ConstantComposite\0" // IR(30419, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(30437, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(30478, 37)
  "ConstantSampler\0" // IR(30515, 16)
  "ConstantNull\0" // IR(30531, 13)
  "SpecConstantTrue\0" // IR(30544, 17)
  "SpecConstantFalse\0" // IR(30561, 18)
  "SpecConstant\0" // IR(30579, 13)
  "SpecConstantComposite\0" // IR(30592, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(30614, 41)
  "SpecConstantOp\0" // IR(30655, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(30670, 34)
  "FunctionParameter\0" // IR(30704, 18)
  "FunctionEnd\0" // IR(30722, 12)
  "FunctionCall\0" // IR(30734, 13)
  "Variable\0" // IR(30747, 9)
  "ImageTexelPointer\0" // IR(30756, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(30774, 40)
  "Load\0" // IR(30814, 5)
  "Store\0" // IR(30819, 6)
  "CopyMemory\0" // IR(30825, 11)
  "CopyMemorySized\0" // IR(30836, 16)
  "AccessChain\0" // IR(30852, 12)
  "InBoundsAccessChain\0" // IR(30864, 20)
  "PtrAccessChain\0" // IR(30884, 15)
  "ArrayLength\0" // IR(30899, 12)
  "GenericPtrMemSemantics\0" // IR(30911, 23)
  "InBoundsPtrAccessChain\0" // IR(30934, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(30957, 28)
  "Decorate\0" // IR(30985, 9)
  "MemberDecorate\0" // IR(30994, 15)
  "DecorationGroup\0" // IR(31009, 16)
  "GroupDecorate\0" // IR(31025, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(31039, 45)
  "GroupMemberDecorate\0" // IR(31084, 20)
  "VectorExtractDynamic\0" // IR(31104, 21)
  "VectorInsertDynamic\0" // IR(31125, 20)
  "VectorShuffle\0" // IR(31145, 14)
  "CompositeConstruct\0" // IR(31159, 19)
  "CompositeExtract\0" // IR(31178, 17)
  "CompositeInsert\0" // IR(31195, 16)
  "CopyObject\0" // IR(31211, 11)
  "Transpose\0" // IR(31222, 10)
  "SampledImage\0" // IR(31232, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(31245, 32)
  "ImageSampleImplicitLod\0" // IR(31277, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(31300, 23)
  "ImageSampleExplicitLod\0" // IR(31323, 23)
  "ImageSampleDrefImplicitLod\0" // IR(31346, 27)
  "ImageSampleDrefExplicitLod\0" // IR(31373, 27)
  "ImageSampleProjImplicitLod\0" // IR(31400, 27)
  "ImageSampleProjExplicitLod\0" // IR(31427, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(31454, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(31485, 31)
  "ImageFetch\0" // IR(31516, 11)
  "ImageGather\0" // IR(31527, 12)
  "ImageDrefGather\0" // IR(31539, 16)
  "ImageRead\0" // IR(31555, 10)
  "ImageWrite\0" // IR(31565, 11)
  "ImageQueryFormat\0" // IR(31576, 17)
  "ImageQueryOrder\0" // IR(31593, 16)
  "ImageQuerySizeLod\0" // IR(31609, 18)
  "ImageQuerySize\0" // IR(31627, 15)
  "ImageQueryLod\0" // IR(31642, 14)
  "ImageQueryLevels\0" // IR(31656, 17)
  "ImageQuerySamples\0" // IR(31673, 18)
  "ConvertFToU\0" // IR(31691, 12)
  "ConvertFToS\0" // IR(31703, 12)
  "ConvertSToF\0" // IR(31715, 12)
  "ConvertUToF\0" // IR(31727, 12)
  "UConvert\0" // IR(31739, 9)
  "SConvert\0" // IR(31748, 9)
  "FConvert\0" // IR(31757, 9)
  "QuantizeToF16\0" // IR(31766, 14)
  "ConvertPtrToU\0" // IR(31780, 14)
  "SatConvertSToU\0" // IR(31794, 15)
  "SatConvertUToS\0" // IR(31809, 15)
  "ConvertUToPtr\0" // IR(31824, 14)
  "PtrCastToGeneric\0" // IR(31838, 17)
  "GenericCastToPtr\0" // IR(31855, 17)
  "GenericCastToPtrExplicit\0" // IR(31872, 25)
  "Bitcast\0" // IR(31897, 8)
  "SNegate\0" // IR(31905, 8)
  "FNegate\0" // IR(31913, 8)
  "IAdd\0" // IR(31921, 5)
  "FAdd\0" // IR(31926, 5)
  "ISub\0" // IR(31931, 5)
  "FSub\0" // IR(31936, 5)
  "IMul\0" // IR(31941, 5)
  "FMul\0" // IR(31946, 5)
  "UDiv\0" // IR(31951, 5)
  "SDiv\0" // IR(31956, 5)
  "FDiv\0" // IR(31961, 5)
  "UMod\0" // IR(31966, 5)
  "SRem\0" // IR(31971, 5)
  "SMod\0" // IR(31976, 5)
  "FRem\0" // IR(31981, 5)
  "FMod\0" // IR(31986, 5)
  "VectorTimesScalar\0" // IR(31991, 18)
  "MatrixTimesScalar\0" // IR(32009, 18)
  "VectorTimesMatrix\0" // IR(32027, 18)
  "MatrixTimesVector\0" // IR(32045, 18)
  "MatrixTimesMatrix\0" // IR(32063, 18)
  "OuterProduct\0" // IR(32081, 13)
  "Dot\0" // IR(32094, 4)
  "IAddCarry\0" // IR(32098, 10)
  "ISubBorrow\0" // IR(32108, 11)
  "UMulExtended\0" // IR(32119, 13)
  "SMulExtended\0" // IR(32132, 13)
  "Any\0" // IR(32145, 4)
  "All\0" // IR(32149, 4)
  "IsNan\0" // IR(32153, 6)
  "IsInf\0" // IR(32159, 6)
  "IsFinite\0" // IR(32165, 9)
  "IsNormal\0" // IR(32174, 9)
  "SignBitSet\0" // IR(32183, 11)
  "LessOrGreater\0" // IR(32194, 14)
  "Ordered\0" // IR(32208, 8)
  "Unordered\0" // IR(32216, 10)
  "LogicalEqual\0" // IR(32226, 13)
  "LogicalNotEqual\0" // IR(32239, 16)
  "LogicalOr\0" // IR(32255, 10)
  "LogicalAnd\0" // IR(32265, 11)
  "LogicalNot\0" // IR(32276, 11)
  "Select\0" // IR(32287, 7)
  "IEqual\0" // IR(32294, 7)
  "INotEqual\0" // IR(32301, 10)
  "UGreaterThan\0" // IR(32311, 13)
  "SGreaterThan\0" // IR(32324, 13)
  "UGreaterThanEqual\0" // IR(32337, 18)
  "SGreaterThanEqual\0" // IR(32355, 18)
  "ULessThan\0" // IR(32373, 10)
  "SLessThan\0" // IR(32383, 10)
  "ULessThanEqual\0" // IR(32393, 15)
  "SLessThanEqual\0" // IR(32408, 15)
  "FOrdEqual\0" // IR(32423, 10)
  "FUnordEqual\0" // IR(32433, 12)
  "FOrdNotEqual\0" // IR(32445, 13)
  "FUnordNotEqual\0" // IR(32458, 15)
  "FOrdLessThan\0" // IR(32473, 13)
  "FUnordLessThan\0" // IR(32486, 15)
  "FOrdGreaterThan\0" // IR(32501, 16)
  "FUnordGreaterThan\0" // IR(32517, 18)
  "FOrdLessThanEqual\0" // IR(32535, 18)
  "FUnordLessThanEqual\0" // IR(32553, 20)
  "FOrdGreaterThanEqual\0" // IR(32573, 21)
  "FUnordGreaterThanEqual\0" // IR(32594, 23)
  "ShiftRightLogical\0" // IR(32617, 18)
  "ShiftRightArithmetic\0" // IR(32635, 21)
  "ShiftLeftLogical\0" // IR(32656, 17)
  "BitwiseOr\0" // IR(32673, 10)
  "BitwiseXor\0" // IR(32683, 11)
  "BitwiseAnd\0" // IR(32694, 11)
  "Not\0" // IR(32705, 4)
  "BitFieldInsert\0" // IR(32709, 15)
  "BitFieldSExtract\0" // IR(32724, 17)
  "BitFieldUExtract\0" // IR(32741, 17)
  "BitReverse\0" // IR(32758, 11)
  "BitCount\0" // IR(32769, 9)
  "DPdx\0" // IR(32778, 5)
  "DPdy\0" // IR(32783, 5)
  "Fwidth\0" // IR(32788, 7)
  "DPdxFine\0" // IR(32795, 9)
  "DPdyFine\0" // IR(32804, 9)
  "FwidthFine\0" // IR(32813, 11)
  "DPdxCoarse\0" // IR(32824, 11)
  "DPdyCoarse\0" // IR(32835, 11)
  "FwidthCoarse\0" // IR(32846, 13)
  "EmitVertex\0" // IR(32859, 11)
  "EndPrimitive\0" // IR(32870, 13)
  "EmitStreamVertex\0" // IR(32883, 17)
  "EndStreamPrimitive\0" // IR(32900, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(32919, 37)
  "ControlBarrier\0" // IR(32956, 15)
  "MemoryBarrier\0" // IR(32971, 14)
  "AtomicLoad\0" // IR(32985, 11)
  "AtomicStore\0" // IR(32996, 12)
  "AtomicExchange\0" // IR(33008, 15)
  "AtomicCompareExchange\0" // IR(33023, 22)
  "AtomicCompareExchangeWeak\0" // IR(33045, 26)
  "AtomicIIncrement\0" // IR(33071, 17)
  "AtomicIDecrement\0" // IR(33088, 17)
  "AtomicIAdd\0" // IR(33105, 11)
  "AtomicISub\0" // IR(33116, 11)
  "AtomicSMin\0" // IR(33127, 11)
  "AtomicUMin\0" // IR(33138, 11)
  "AtomicSMax\0" // IR(33149, 11)
  "AtomicUMax\0" // IR(33160, 11)
  "AtomicAnd\0" // IR(33171, 10)
  "AtomicOr\0" // IR(33181, 9)
  "AtomicXor\0" // IR(33190, 10)
  "Phi\0" // IR(33200, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(33204, 30)
  "LoopMerge\0" // IR(33234, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(33244, 35)
  "SelectionMerge\0" // IR(33279, 15)
  "Label\0" // IR(33294, 6)
  "Branch\0" // IR(33300, 7)
  "BranchConditional\0" // IR(33307, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(33325, 45)
  "Switch\0" // IR(33370, 7)
  "Kill\0" // IR(33377, 5)
  "Return\0" // IR(33382, 7)
  "ReturnValue\0" // IR(33389, 12)
  "Unreachable\0" // IR(33401, 12)
  "LifetimeStart\0" // IR(33413, 14)
  "LifetimeStop\0" // IR(33427, 13)
  "GroupAsyncCopy\0" // IR(33440, 15)
  "GroupWaitEvents\0" // IR(33455, 16)
  "GroupAll\0" // IR(33471, 9)
  "GroupAny\0" // IR(33480, 9)
  "GroupBroadcast\0" // IR(33489, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(33504, 33)
  "GroupIAdd\0" // IR(33537, 10)
  "GroupFAdd\0" // IR(33547, 10)
  "GroupFMin\0" // IR(33557, 10)
  "GroupUMin\0" // IR(33567, 10)
  "GroupSMin\0" // IR(33577, 10)
  "GroupFMax\0" // IR(33587, 10)
  "GroupUMax\0" // IR(33597, 10)
  "GroupSMax\0" // IR(33607, 10)
  "ReadPipe\0" // IR(33617, 9)
  "WritePipe\0" // IR(33626, 10)
  "ReservedReadPipe\0" // IR(33636, 17)
  "ReservedWritePipe\0" // IR(33653, 18)
  "ReserveReadPipePackets\0" // IR(33671, 23)
  "ReserveWritePipePackets\0" // IR(33694, 24)
  "CommitReadPipe\0" // IR(33718, 15)
  "CommitWritePipe\0" // IR(33733, 16)
  "IsValidReserveId\0" // IR(33749, 17)
  "GetNumPipePackets\0" // IR(33766, 18)
  "GetMaxPipePackets\0" // IR(33784, 18)
  "GroupReserveReadPipePackets\0" // IR(33802, 28)
  "GroupReserveWritePipePackets\0" // IR(33830, 29)
  "GroupCommitReadPipe\0" // IR(33859, 20)
  "GroupCommitWritePipe\0" // IR(33879, 21)
  "EnqueueMarker\0" // IR(33900, 14)
  "EnqueueKernel\0" // IR(33914, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(33928, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(33958, 32)
  "GetKernelWorkGroupSize\0" // IR(33990, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(34013, 40)
  "RetainEvent\0" // IR(34053, 12)
  "ReleaseEvent\0" // IR(34065, 13)
  "CreateUserEvent\0" // IR(34078, 16)
  "IsValidEvent\0" // IR(34094, 13)
  "SetUserEventStatus\0" // IR(34107, 19)
  "CaptureEventProfilingInfo\0" // IR(34126, 26)
  "GetDefaultQueue\0" // IR(34152, 16)
  "BuildNDRange\0" // IR(34168, 13)
  "ImageSparseSampleImplicitLod\0" // IR(34181, 29)
  "ImageSparseSampleExplicitLod\0" // IR(34210, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(34239, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(34272, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(34305, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(34338, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(34371, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(34408, 37)
  "ImageSparseFetch\0" // IR(34445, 17)
  "ImageSparseGather\0" // IR(34462, 18)
  "ImageSparseDrefGather\0" // IR(34480, 22)
  "ImageSparseTexelsResident\0" // IR(34502, 26)
  "NoLine\0" // IR(34528, 7)
  "AtomicFlagTestAndSet\0" // IR(34535, 21)
  "AtomicFlagClear\0" // IR(34556, 16)
  "ImageSparseRead\0" // IR(34572, 16)
  "SizeOf\0" // IR(34588, 7)
  "TypePipeStorage\0" // IR(34595, 16)
  "ConstantPipeStorage\0" // IR(34611, 20)
  "CreatePipeFromPipeStorage\0" // IR(34631, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(34657, 35)
  "GetKernelMaxNumSubgroups\0" // IR(34692, 25)
  "TypeNamedBarrier\0" // IR(34717, 17)
  "NamedBarrierInitialize\0" // IR(34734, 23)
  "MemoryNamedBarrier\0" // IR(34757, 19)
  "ModuleProcessed\0" // IR(34776, 16)
  "ExecutionModeId\0" // IR(34792, 16)
  "DecorateId\0" // IR(34808, 11)
  "GroupNonUniformElect\0" // IR(34819, 21)
  "GroupNonUniformAll\0" // IR(34840, 19)
  "GroupNonUniformAny\0" // IR(34859, 19)
  "GroupNonUniformAllEqual\0" // IR(34878, 24)
  "GroupNonUniformBroadcast\0" // IR(34902, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(34927, 30)
  "GroupNonUniformInverseBallot\0" // IR(34957, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(34986, 32)
  "GroupNonUniformBallotBitCount\0" // IR(35018, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(35048, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(35077, 29)
  "GroupNonUniformShuffleXor\0" // IR(35106, 26)
  "GroupNonUniformShuffleUp\0" // IR(35132, 25)
  "GroupNonUniformShuffleDown\0" // IR(35157, 27)
  "GroupNonUniformIAdd\0" // IR(35184, 20)
  "GroupNonUniformFAdd\0" // IR(35204, 20)
  "GroupNonUniformIMul\0" // IR(35224, 20)
  "GroupNonUniformFMul\0" // IR(35244, 20)
  "GroupNonUniformSMin\0" // IR(35264, 20)
  "GroupNonUniformUMin\0" // IR(35284, 20)
  "GroupNonUniformFMin\0" // IR(35304, 20)
  "GroupNonUniformSMax\0" // IR(35324, 20)
  "GroupNonUniformUMax\0" // IR(35344, 20)
  "GroupNonUniformFMax\0" // IR(35364, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(35384, 26)
  "GroupNonUniformBitwiseOr\0" // IR(35410, 25)
  "GroupNonUniformBitwiseXor\0" // IR(35435, 26)
  "GroupNonUniformLogicalAnd\0" // IR(35461, 26)
  "GroupNonUniformLogicalOr\0" // IR(35487, 25)
  "GroupNonUniformLogicalXor\0" // IR(35512, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(35538, 29)
  "GroupNonUniformQuadSwap\0" // IR(35567, 24)
  "CopyLogical\0" // IR(35591, 12)
  "PtrEqual\0" // IR(35603, 9)
  "PtrNotEqual\0" // IR(35612, 12)
  "PtrDiff\0" // IR(35624, 8)
  "ColorAttachmentReadEXT\0" // IR(35632, 23)
  "DepthAttachmentReadEXT\0" // IR(35655, 23)
  "StencilAttachmentReadEXT\0" // IR(35678, 25)
  "TypeTensorARM\0" // IR(35703, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(35717, 42)
  "TensorReadARM\0" // IR(35759, 14)
  "TensorWriteARM\0" // IR(35773, 15)
  "TensorQuerySizeARM\0" // IR(35788, 19)
  "GraphConstantARM\0" // IR(35807, 17)
  "GraphEntryPointARM\0" // IR(35824, 19)
  "GraphInputARM\0" // IR(35843, 14)
  "GraphSetOutputARM\0" // IR(35857, 18)
  "GraphEndARM\0" // IR(35875, 12)
  "TypeGraphARM\0" // IR(35887, 13)
  "TerminateInvocation\0" // IR(35900, 20)
  "TypeUntypedPointerKHR\0" // IR(35920, 22)
  "UntypedVariableKHR\0" // IR(35942, 19)
  "UntypedAccessChainKHR\0" // IR(35961, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(35983, 30)
  "SubgroupFirstInvocationKHR\0" // IR(36013, 27)
  "UntypedPtrAccessChainKHR\0" // IR(36040, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(36065, 33)
  "UntypedArrayLengthKHR\0" // IR(36098, 22)
  "UntypedPrefetchKHR\0" // IR(36120, 19)
  "FmaKHR\0" // IR(36139, 7)
  "SubgroupAllKHR\0" // IR(36146, 15)
  "SubgroupAnyKHR\0" // IR(36161, 15)
  "SubgroupAllEqualKHR\0" // IR(36176, 20)
  "SubgroupReadInvocationKHR\0" // IR(36196, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(36222, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(36248, 25)
  "TraceRayKHR\0" // IR(36273, 12)
  "ExecuteCallableKHR\0" // IR(36285, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(36304, 35)
  "IgnoreIntersectionKHR\0" // IR(36339, 22)
  "TerminateRayKHR\0" // IR(36361, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(36377, 47)
  "SDot\0" // IR(36424, 5)
  "SDotKHR\0" // IR(36429, 8)
  "UDot\0" // IR(36437, 5)
  "UDotKHR\0" // IR(36442, 8)
  "SUDot\0" // IR(36450, 6)
  "SUDotKHR\0" // IR(36456, 9)
  "SDotAccSat\0" // IR(36465, 11)
  "SDotAccSatKHR\0" // IR(36476, 14)
  "UDotAccSat\0" // IR(36490, 11)
  "UDotAccSatKHR\0" // IR(36501, 14)
  "SUDotAccSat\0" // IR(36515, 12)
  "SUDotAccSatKHR\0" // IR(36527, 15)
  "TypeCooperativeMatrixKHR\0" // IR(36542, 25)
  "CooperativeMatrixLoadKHR\0" // IR(36567, 25)
  "CooperativeMatrixStoreKHR\0" // IR(36592, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(36618, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(36672, 27)
  "CooperativeMatrixLengthKHR\0" // IR(36699, 27)
  "ConstantCompositeReplicateEXT\0" // IR(36726, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(36756, 34)
  "CompositeConstructReplicateEXT\0" // IR(36790, 31)
  "TypeRayQueryKHR\0" // IR(36821, 16)
  "RayQueryInitializeKHR\0" // IR(36837, 22)
  "RayQueryTerminateKHR\0" // IR(36859, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(36880, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(36912, 31)
  "RayQueryProceedKHR\0" // IR(36943, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(36962, 31)
  "ImageSampleWeightedQCOM\0" // IR(36993, 24)
  "ImageBoxFilterQCOM\0" // IR(37017, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(37036, 23)
  "ImageBlockMatchSADQCOM\0" // IR(37059, 23)
  "BitCastArrayQCOM\0" // IR(37082, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(37099, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(37128, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(37157, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(37186, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(37215, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(37245, 28)
  "ExtractSubArrayQCOM\0" // IR(37273, 20)
  "ImageGatherQCOM\0" // IR(37293, 16)
  "GroupIAddNonUniformAMD\0" // IR(37309, 23)
  "GroupFAddNonUniformAMD\0" // IR(37332, 23)
  "GroupFMinNonUniformAMD\0" // IR(37355, 23)
  "GroupUMinNonUniformAMD\0" // IR(37378, 23)
  "GroupSMinNonUniformAMD\0" // IR(37401, 23)
  "GroupFMaxNonUniformAMD\0" // IR(37424, 23)
  "GroupUMaxNonUniformAMD\0" // IR(37447, 23)
  "GroupSMaxNonUniformAMD\0" // IR(37470, 23)
  "FragmentMaskFetchAMD\0" // IR(37493, 21)
  "FragmentFetchAMD\0" // IR(37514, 17)
  "ReadClockKHR\0" // IR(37531, 13)
  "AllocateNodePayloadsAMDX\0" // IR(37544, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(37569, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(37593, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(37618, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(37647, 27)
  "IsNodePayloadValidAMDX\0" // IR(37674, 23)
  "ConstantStringAMDX\0" // IR(37697, 19)
  "SpecConstantStringAMDX\0" // IR(37716, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(37739, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(37765, 26)
  "TypeBufferEXT\0" // IR(37791, 14)
  "BufferPointerEXT\0" // IR(37805, 17)
  "UntypedImageTexelPointerEXT\0" // IR(37822, 28)
  "MemberDecorateIdEXT\0" // IR(37850, 20)
  "ConstantSizeOfEXT\0" // IR(37870, 18)
  "SpecConstantDataKHR\0" // IR(37888, 20)
  "PoisonKHR\0" // IR(37908, 10)
  "FreezeKHR\0" // IR(37918, 10)
  "HitObjectRecordHitMotionNV\0" // IR(37928, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(37955, 36)
  "HitObjectRecordMissMotionNV\0" // IR(37991, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(38019, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(38047, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(38075, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(38108, 30)
  "HitObjectTraceRayMotionNV\0" // IR(38138, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(38164, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(38203, 44)
  "HitObjectRecordEmptyNV\0" // IR(38247, 23)
  "HitObjectTraceRayNV\0" // IR(38270, 20)
  "HitObjectRecordHitNV\0" // IR(38290, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(38311, 30)
  "HitObjectRecordMissNV\0" // IR(38341, 22)
  "HitObjectExecuteShaderNV\0" // IR(38363, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(38388, 26)
  "HitObjectGetAttributesNV\0" // IR(38414, 25)
  "HitObjectGetHitKindNV\0" // IR(38439, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(38461, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(38490, 28)
  "HitObjectGetInstanceIdNV\0" // IR(38518, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(38543, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(38577, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(38609, 29)
  "HitObjectGetRayTMaxNV\0" // IR(38638, 22)
  "HitObjectGetRayTMinNV\0" // IR(38660, 22)
  "HitObjectIsEmptyNV\0" // IR(38682, 19)
  "HitObjectIsHitNV\0" // IR(38701, 17)
  "HitObjectIsMissNV\0" // IR(38718, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(38736, 29)
  "ReorderThreadWithHintNV\0" // IR(38765, 24)
  "TypeHitObjectNV\0" // IR(38789, 16)
  "ImageSampleFootprintNV\0" // IR(38805, 23)
  "TypeVectorIdEXT\0" // IR(38828, 16)
  "TypeCooperativeVectorNV\0" // IR(38844, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(38868, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(38897, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(38939, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(38978, 32)
  "CooperativeMatrixConvertUseEXT\0" // IR(39010, 31)
  "CooperativeMatrixConvertNV\0" // IR(39041, 27)
  "EmitMeshTasksEXT\0" // IR(39068, 17)
  "SetMeshOutputsEXT\0" // IR(39085, 18)
  "GroupNonUniformPartitionEXT\0" // IR(39103, 28)
  "GroupNonUniformPartitionNV\0" // IR(39131, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(39158, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(39191, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(39226, 38)
  "CooperativeVectorLoadNV\0" // IR(39264, 24)
  "CooperativeVectorStoreNV\0" // IR(39288, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(39313, 28)
  "HitObjectRecordMissEXT\0" // IR(39341, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(39364, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(39393, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(39444, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(39468, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(39513, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(39546, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(39578, 38)
  "TypeHitObjectEXT\0" // IR(39616, 17)
  "ReorderThreadWithHintEXT\0" // IR(39633, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(39658, 30)
  "HitObjectTraceRayEXT\0" // IR(39688, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(39709, 27)
  "HitObjectRecordEmptyEXT\0" // IR(39736, 24)
  "HitObjectExecuteShaderEXT\0" // IR(39760, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(39786, 27)
  "HitObjectGetAttributesEXT\0" // IR(39813, 26)
  "HitObjectGetHitKindEXT\0" // IR(39839, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(39862, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(39892, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(39921, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(39947, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(39982, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(40013, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(40047, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(40080, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(40110, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(40139, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(40168, 23)
  "ReportIntersectionKHR\0" // IR(40191, 22)
  "ReportIntersectionNV\0" // IR(40213, 21)
  "IgnoreIntersectionNV\0" // IR(40234, 21)
  "TerminateRayNV\0" // IR(40255, 15)
  "TraceNV\0" // IR(40270, 8)
  "TraceMotionNV\0" // IR(40278, 14)
  "TraceRayMotionNV\0" // IR(40292, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(40309, 50)
  "TypeAccelerationStructureKHR\0" // IR(40359, 29)
  "TypeAccelerationStructureNV\0" // IR(40388, 28)
  "ExecuteCallableNV\0" // IR(40416, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(40434, 35)
  "RayQueryGetClusterIdNV\0" // IR(40469, 23)
  "HitObjectGetClusterIdNV\0" // IR(40492, 24)
  "HitObjectGetRayTMinEXT\0" // IR(40516, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(40539, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(40584, 40)
  "HitObjectIsEmptyEXT\0" // IR(40624, 20)
  "HitObjectIsHitEXT\0" // IR(40644, 18)
  "HitObjectIsMissEXT\0" // IR(40662, 19)
  "TypeCooperativeMatrixNV\0" // IR(40681, 24)
  "CooperativeMatrixLoadNV\0" // IR(40705, 24)
  "CooperativeMatrixStoreNV\0" // IR(40729, 25)
  "CooperativeMatrixMulAddNV\0" // IR(40754, 26)
  "CooperativeMatrixLengthNV\0" // IR(40780, 26)
  "BeginInvocationInterlockEXT\0" // IR(40806, 28)
  "EndInvocationInterlockEXT\0" // IR(40834, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(40860, 43)
  "CooperativeMatrixReduceEXT\0" // IR(40903, 27)
  "CooperativeMatrixReduceNV\0" // IR(40930, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(40956, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(40987, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(41031, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(41061, 31)
  "CooperativeMatrixPerElementOpEXT\0" // IR(41092, 33)
  "CooperativeMatrixPerElementOpNV\0" // IR(41125, 32)
  "TypeTensorLayoutNV\0" // IR(41157, 19)
  "TypeTensorViewNV\0" // IR(41176, 17)
  "CreateTensorLayoutNV\0" // IR(41193, 21)
  "TensorLayoutSetDimensionNV\0" // IR(41214, 27)
  "TensorLayoutSetStrideNV\0" // IR(41241, 24)
  "TensorLayoutSliceNV\0" // IR(41265, 20)
  "TensorLayoutSetClampValueNV\0" // IR(41285, 28)
  "CreateTensorViewNV\0" // IR(41313, 19)
  "TensorViewSetDimensionNV\0" // IR(41332, 25)
  "TensorViewSetStrideNV\0" // IR(41357, 22)
  "IsHelperInvocationEXT\0" // IR(41379, 22)
  "TensorViewSetClipNV\0" // IR(41401, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(41421, 27)
  "CooperativeMatrixTransposeNV\0" // IR(41448, 29)
  "ConvertUToImageNV\0" // IR(41477, 18)
  "ConvertUToSamplerNV\0" // IR(41495, 20)
  "ConvertImageToUNV\0" // IR(41515, 18)
  "ConvertSamplerToUNV\0" // IR(41533, 20)
  "ConvertUToSampledImageNV\0" // IR(41553, 25)
  "ConvertSampledImageToUNV\0" // IR(41578, 25)
  "SamplerImageAddressingModeNV\0" // IR(41603, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(41632, 52)
  "RawAccessChainNV\0" // IR(41684, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(41701, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(41741, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(41779, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(41817, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(41851, 37)
  "HitObjectGetSpherePositionNV\0" // IR(41888, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(41917, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(41944, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(41971, 23)
  "HitObjectIsSphereHitNV\0" // IR(41994, 23)
  "HitObjectIsLSSHitNV\0" // IR(42017, 20)
  "RayQueryIsSphereHitNV\0" // IR(42037, 22)
  "RayQueryIsLSSHitNV\0" // IR(42059, 19)
  "SubgroupShuffleDownINTEL\0" // IR(42078, 25)
  "SubgroupShuffleUpINTEL\0" // IR(42103, 23)
  "SubgroupShuffleXorINTEL\0" // IR(42126, 24)
  "SubgroupBlockReadINTEL\0" // IR(42150, 23)
  "SubgroupBlockWriteINTEL\0" // IR(42173, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(42197, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(42225, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(42254, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(42287, 34)
  "UCountLeadingZerosINTEL\0" // IR(42321, 24)
  "UCountTrailingZerosINTEL\0" // IR(42345, 25)
  "AbsISubINTEL\0" // IR(42370, 13)
  "AbsUSubINTEL\0" // IR(42383, 13)
  "IAddSatINTEL\0" // IR(42396, 13)
  "UAddSatINTEL\0" // IR(42409, 13)
  "IAverageINTEL\0" // IR(42422, 14)
  "UAverageINTEL\0" // IR(42436, 14)
  "IAverageRoundedINTEL\0" // IR(42450, 21)
  "UAverageRoundedINTEL\0" // IR(42471, 21)
  "ISubSatINTEL\0" // IR(42492, 13)
  "USubSatINTEL\0" // IR(42505, 13)
  "IMul32x16INTEL\0" // IR(42518, 15)
  "UMul32x16INTEL\0" // IR(42533, 15)
  "ConstantFunctionPointerINTEL\0" // IR(42548, 29)
  "FunctionPointerCallINTEL\0" // IR(42577, 25)
  "AsmTargetINTEL\0" // IR(42602, 15)
  "AsmCallINTEL\0" // IR(42617, 13)
  "AtomicFMinEXT\0" // IR(42630, 14)
  "AtomicFMaxEXT\0" // IR(42644, 14)
  "AssumeTrueKHR\0" // IR(42658, 14)
  "ExpectKHR\0" // IR(42672, 10)
  "DecorateString\0" // IR(42682, 15)
  "DecorateStringGOOGLE\0" // IR(42697, 21)
  "MemberDecorateString\0" // IR(42718, 21)
  "MemberDecorateStringGOOGLE\0" // IR(42739, 27)
  "VmeImageINTEL\0" // IR(42766, 14)
  "TypeVmeImageINTEL\0" // IR(42780, 18)
  "TypeAvcImePayloadINTEL\0" // IR(42798, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(42821, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(42844, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(42867, 23)
  "TypeAvcMceResultINTEL\0" // IR(42890, 22)
  "TypeAvcImeResultINTEL\0" // IR(42912, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(42934, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(42980, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(43024, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(43063, 37)
  "TypeAvcRefResultINTEL\0" // IR(43100, 22)
  "TypeAvcSicResultINTEL\0" // IR(43122, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(43144, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(43204, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(43257, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(43304, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(43344, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(43395, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(43439, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(43490, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(43546, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(43596, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(43648, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(43697, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(43744, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(43794, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(43845, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(43901, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(43934, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(43986, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44047, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(44108, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(44147, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(44185, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(44224, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(44262, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(44301, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(44339, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(44375, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(44414, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(44457, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(44495, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(44533, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(44571, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(44616, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44656, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(44718, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(44748, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(44786, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(44822, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(44855, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(44890, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(44929, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(44972, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(45019, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(45073, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(45107, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(45154, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(45199, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(45254, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(45307, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(45363, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(45417, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(45475, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(45531, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(45569, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(45615, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(45659, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(45708, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(45755, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(45825, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(45893, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(45962, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(46030, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(46096, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(46163, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(46199, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(46247, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(46306, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(46364, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(46420, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(46450, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(46480, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(46519, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(46565, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(46608, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(46655, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(46700, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(46746, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(46802, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(46840, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(46870, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(46902, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(46938, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(46980, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(47019, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(47058, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(47102, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(47150, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(47200, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(47243, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(47291, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(47334, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(47365, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(47412, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(47457, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(47503, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(47559, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(47597, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(47632, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(47676, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(47722, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(47763, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(47799, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(47849, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(47897, 35)
  "SaveMemoryINTEL\0" // IR(47932, 16)
  "RestoreMemoryINTEL\0" // IR(47948, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(47967, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(47996, 28)
  "ArbitraryFloatCastALTERA\0" // IR(48024, 25)
  "ArbitraryFloatCastINTEL\0" // IR(48049, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(48073, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(48105, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(48136, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(48166, 29)
  "ArbitraryFloatAddALTERA\0" // IR(48195, 24)
  "ArbitraryFloatAddINTEL\0" // IR(48219, 23)
  "ArbitraryFloatSubALTERA\0" // IR(48242, 24)
  "ArbitraryFloatSubINTEL\0" // IR(48266, 23)
  "ArbitraryFloatMulALTERA\0" // IR(48289, 24)
  "ArbitraryFloatMulINTEL\0" // IR(48313, 23)
  "ArbitraryFloatDivALTERA\0" // IR(48336, 24)
  "ArbitraryFloatDivINTEL\0" // IR(48360, 23)
  "ArbitraryFloatGTALTERA\0" // IR(48383, 23)
  "ArbitraryFloatGTINTEL\0" // IR(48406, 22)
  "ArbitraryFloatGEALTERA\0" // IR(48428, 23)
  "ArbitraryFloatGEINTEL\0" // IR(48451, 22)
  "ArbitraryFloatLTALTERA\0" // IR(48473, 23)
  "ArbitraryFloatLTINTEL\0" // IR(48496, 22)
  "ArbitraryFloatLEALTERA\0" // IR(48518, 23)
  "ArbitraryFloatLEINTEL\0" // IR(48541, 22)
  "ArbitraryFloatEQALTERA\0" // IR(48563, 23)
  "ArbitraryFloatEQINTEL\0" // IR(48586, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(48608, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(48634, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(48659, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(48685, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(48710, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(48735, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(48759, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(48785, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(48810, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(48835, 24)
  "ArbitraryFloatLogINTEL\0" // IR(48859, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(48882, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(48906, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(48931, 25)
  "ArbitraryFloatExpINTEL\0" // IR(48956, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(48979, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(49003, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(49028, 25)
  "ArbitraryFloatSinINTEL\0" // IR(49053, 23)
  "ArbitraryFloatCosINTEL\0" // IR(49076, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(49099, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(49125, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(49150, 25)
  "ArbitraryFloatASinINTEL\0" // IR(49175, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(49199, 26)
  "ArbitraryFloatACosINTEL\0" // IR(49225, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(49249, 26)
  "ArbitraryFloatATanINTEL\0" // IR(49275, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(49299, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(49325, 25)
  "ArbitraryFloatPowINTEL\0" // IR(49350, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(49373, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(49397, 24)
  "LoopControlINTEL\0" // IR(49421, 17)
  "AliasDomainDeclINTEL\0" // IR(49438, 21)
  "AliasScopeDeclINTEL\0" // IR(49459, 20)
  "AliasScopeListDeclINTEL\0" // IR(49479, 24)
  "FixedSqrtALTERA\0" // IR(49503, 16)
  "FixedSqrtINTEL\0" // IR(49519, 15)
  "FixedRecipALTERA\0" // IR(49534, 17)
  "FixedRecipINTEL\0" // IR(49551, 16)
  "FixedRsqrtALTERA\0" // IR(49567, 17)
  "FixedRsqrtINTEL\0" // IR(49584, 16)
  "FixedSinALTERA\0" // IR(49600, 15)
  "FixedSinINTEL\0" // IR(49615, 14)
  "FixedCosALTERA\0" // IR(49629, 15)
  "FixedCosINTEL\0" // IR(49644, 14)
  "FixedSinCosALTERA\0" // IR(49658, 18)
  "FixedSinCosINTEL\0" // IR(49676, 17)
  "FixedSinPiALTERA\0" // IR(49693, 17)
  "FixedSinPiINTEL\0" // IR(49710, 16)
  "FixedCosPiALTERA\0" // IR(49726, 17)
  "FixedCosPiINTEL\0" // IR(49743, 16)
  "FixedSinCosPiALTERA\0" // IR(49759, 20)
  "FixedSinCosPiINTEL\0" // IR(49779, 19)
  "FixedLogALTERA\0" // IR(49798, 15)
  "FixedLogINTEL\0" // IR(49813, 14)
  "FixedExpALTERA\0" // IR(49827, 15)
  "FixedExpINTEL\0" // IR(49842, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(49856, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(49886, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(49915, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(49945, 29)
  "ReadPipeBlockingALTERA\0" // IR(49974, 23)
  "ReadPipeBlockingINTEL\0" // IR(49997, 22)
  "WritePipeBlockingALTERA\0" // IR(50019, 24)
  "WritePipeBlockingINTEL\0" // IR(50043, 23)
  "RayQueryGetRayTMinKHR\0" // IR(50066, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(50088, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(50111, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(50139, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(50185, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(50222, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(50287, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(50327, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(50368, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(50407, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(50443, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(50489, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(50534, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(50576, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(50608, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(50637, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(50677, 40)
  "AtomicFAddEXT\0" // IR(50717, 14)
  "TypeBufferSurfaceINTEL\0" // IR(50731, 23)
  "TypeStructContinuedINTEL\0" // IR(50754, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(50779, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(50811, 36)
  "CompositeConstructContinuedINTEL\0" // IR(50847, 33)
  "ConvertFToBF16INTEL\0" // IR(50880, 20)
  "ConvertBF16ToFINTEL\0" // IR(50900, 20)
  "ControlBarrierArriveEXT\0" // IR(50920, 24)
  "ControlBarrierArriveINTEL\0" // IR(50944, 26)
  "ControlBarrierWaitEXT\0" // IR(50970, 22)
  "ControlBarrierWaitINTEL\0" // IR(50992, 24)
  "TaskSequenceCreateALTERA\0" // IR(51016, 25)
  "TaskSequenceCreateINTEL\0" // IR(51041, 24)
  "TaskSequenceAsyncALTERA\0" // IR(51065, 24)
  "TaskSequenceAsyncINTEL\0" // IR(51089, 23)
  "TaskSequenceGetALTERA\0" // IR(51112, 22)
  "TaskSequenceGetINTEL\0" // IR(51134, 21)
  "TaskSequenceReleaseALTERA\0" // IR(51155, 26)
  "TaskSequenceReleaseINTEL\0" // IR(51181, 25)
  "TypeTaskSequenceALTERA\0" // IR(51206, 23)
  "TypeTaskSequenceINTEL\0" // IR(51229, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(51251, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(51278, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(51303, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(51337, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(51371, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(51400, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(51426, 62)
  "BitwiseFunctionINTEL\0" // IR(51488, 21)
  "ConditionalExtensionINTEL\0" // IR(51509, 26)
  "ConditionalEntryPointINTEL\0" // IR(51535, 27)
  "ConditionalCapabilityINTEL\0" // IR(51562, 27)
  "SpecConstantTargetINTEL\0" // IR(51589, 24)
  "SpecConstantArchitectureINTEL\0" // IR(51613, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(51643, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(51680, 30)
  "ConditionalCopyObjectINTEL\0" // IR(51710, 27)
  "PredicatedLoadINTEL\0" // IR(51737, 20)
  "PredicatedStoreINTEL\0" // IR(51757, 21)
  "GroupIMulKHR\0" // IR(51778, 13)
  "GroupFMulKHR\0" // IR(51791, 13)
  "GroupBitwiseAndKHR\0" // IR(51804, 19)
  "GroupBitwiseOrKHR\0" // IR(51823, 18)
  "GroupBitwiseXorKHR\0" // IR(51841, 19)
  "GroupLogicalAndKHR\0" // IR(51860, 19)
  "GroupLogicalOrKHR\0" // IR(51879, 18)
  "GroupLogicalXorKHR\0" // IR(51897, 19)
  "RoundFToTF32INTEL\0" // IR(51916, 18)
  "MaskedGatherINTEL\0" // IR(51934, 18)
  "MaskedScatterINTEL\0" // IR(51952, 19)
  "ConvertHandleToImageINTEL\0" // IR(51971, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(51997, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(52025, 33)
  "FDot2MixAcc32VALVE\0" // IR(52058, 19)
  "FDot2MixAcc16VALVE\0" // IR(52077, 19)
  "FDot4MixAcc32VALVE\0" // IR(52096, 19)
  "CALL\0" // IR(52115, 5)
  "MIN_SAD\0" // IR(52120, 8)
  "MIN_SAD_COST\0" // IR(52128, 13)
  "RAW_SAD\0" // IR(52141, 8)
  "DebugInfoNone\0" // IR(52149, 14)
  "DebugCompilationUnit\0" // IR(52163, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(52184, 52)
  "DebugTypeBasic\0" // IR(52236, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(52251, 34)
  "DebugTypePointer\0" // IR(52285, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(52302, 38)
  "DebugTypeQualifier\0" // IR(52340, 19)
  "DebugTypeArray\0" // IR(52359, 15)
  "DebugTypeVector\0" // IR(52374, 16)
  "DebugTypedef\0" // IR(52390, 13)
  "DebugTypeFunction\0" // IR(52403, 18)
  "DebugTypeEnum\0" // IR(52421, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(52435, 38)
  "DebugTypeComposite\0" // IR(52473, 19)
  "DebugTypeMember\0" // IR(52492, 16)
  "DebugTypeInheritance\0" // IR(52508, 21)
  "DebugTypePtrToMember\0" // IR(52529, 21)
  "DebugTypeTemplate\0" // IR(52550, 18)
  "DebugTypeTemplateParameter\0" // IR(52568, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(52595, 35)
  "DebugTypeTemplateParameterPack\0" // IR(52630, 31)
  "DebugGlobalVariable\0" // IR(52661, 20)
  "DebugFunctionDeclaration\0" // IR(52681, 25)
  "DebugFunction\0" // IR(52706, 14)
  "DebugLexicalBlock\0" // IR(52720, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(52738, 31)
  "DebugScope\0" // IR(52769, 11)
  "DebugNoScope\0" // IR(52780, 13)
  "DebugInlinedAt\0" // IR(52793, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(52808, 42)
  "DebugLocalVariable\0" // IR(52850, 19)
  "DebugInlinedVariable\0" // IR(52869, 21)
  "DebugDeclare\0" // IR(52890, 13)
  "DebugValue\0" // IR(52903, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(52914, 33)
  "DebugOperation\0" // IR(52947, 15)
  "DebugExpression\0" // IR(52962, 16)
  "DebugMacroDef\0" // IR(52978, 14)
  "DebugMacroUndef\0" // IR(52992, 16)
  "Round\0" // IR(53008, 6)
  "RoundEven\0" // IR(53014, 10)
  "Trunc\0" // IR(53024, 6)
  "FAbs\0" // IR(53030, 5)
  "SAbs\0" // IR(53035, 5)
  "FSign\0" // IR(53040, 6)
  "SSign\0" // IR(53046, 6)
  "Floor\0" // IR(53052, 6)
  "Ceil\0" // IR(53058, 5)
  "Fract\0" // IR(53063, 6)
  "Radians\0" // IR(53069, 8)
  "Degrees\0" // IR(53077, 8)
  "Sin\0" // IR(53085, 4)
  "Cos\0" // IR(53089, 4)
  "Tan\0" // IR(53093, 4)
  "Asin\0" // IR(53097, 5)
  "Acos\0" // IR(53102, 5)
  "Atan\0" // IR(53107, 5)
  "Sinh\0" // IR(53112, 5)
  "Cosh\0" // IR(53117, 5)
  "Tanh\0" // IR(53122, 5)
  "Asinh\0" // IR(53127, 6)
  "Acosh\0" // IR(53133, 6)
  "Atanh\0" // IR(53139, 6)
  "Atan2\0" // IR(53145, 6)
  "Pow\0" // IR(53151, 4)
  "Exp\0" // IR(53155, 4)
  "Log\0" // IR(53159, 4)
  "Exp2\0" // IR(53163, 5)
  "Log2\0" // IR(53168, 5)
  "Sqrt\0" // IR(53173, 5)
  "InverseSqrt\0" // IR(53178, 12)
  "Determinant\0" // IR(53190, 12)
  "MatrixInverse\0" // IR(53202, 14)
  "Modf\0" // IR(53216, 5)
  "ModfStruct\0" // IR(53221, 11)
  "FMin\0" // IR(53232, 5)
  "UMin\0" // IR(53237, 5)
  "SMin\0" // IR(53242, 5)
  "FMax\0" // IR(53247, 5)
  "UMax\0" // IR(53252, 5)
  "SMax\0" // IR(53257, 5)
  "FClamp\0" // IR(53262, 7)
  "UClamp\0" // IR(53269, 7)
  "SClamp\0" // IR(53276, 7)
  "FMix\0" // IR(53283, 5)
  "IMix\0" // IR(53288, 5)
  "Step\0" // IR(53293, 5)
  "SmoothStep\0" // IR(53298, 11)
  "Fma\0" // IR(53309, 4)
  "Frexp\0" // IR(53313, 6)
  "FrexpStruct\0" // IR(53319, 12)
  "Ldexp\0" // IR(53331, 6)
  "PackSnorm4x8\0" // IR(53337, 13)
  "PackUnorm4x8\0" // IR(53350, 13)
  "PackSnorm2x16\0" // IR(53363, 14)
  "PackUnorm2x16\0" // IR(53377, 14)
  "PackHalf2x16\0" // IR(53391, 13)
  "PackDouble2x32\0" // IR(53404, 15)
  "UnpackSnorm2x16\0" // IR(53419, 16)
  "UnpackUnorm2x16\0" // IR(53435, 16)
  "UnpackHalf2x16\0" // IR(53451, 15)
  "UnpackSnorm4x8\0" // IR(53466, 15)
  "UnpackUnorm4x8\0" // IR(53481, 15)
  "UnpackDouble2x32\0" // IR(53496, 17)
  "Length\0" // IR(53513, 7)
  "Distance\0" // IR(53520, 9)
  "Cross\0" // IR(53529, 6)
  "Normalize\0" // IR(53535, 10)
  "FaceForward\0" // IR(53545, 12)
  "Reflect\0" // IR(53557, 8)
  "Refract\0" // IR(53565, 8)
  "FindILsb\0" // IR(53573, 9)
  "FindSMsb\0" // IR(53582, 9)
  "FindUMsb\0" // IR(53591, 9)
  "InterpolateAtCentroid\0" // IR(53600, 22)
  "InterpolateAtSample\0" // IR(53622, 20)
  "InterpolateAtOffset\0" // IR(53642, 20)
  "NMin\0" // IR(53662, 5)
  "NMax\0" // IR(53667, 5)
  "NClamp\0" // IR(53672, 7)
  "ArgumentInfo\0" // IR(53679, 13)
  "ArgumentStorageBuffer\0" // IR(53692, 22)
  "ArgumentUniform\0" // IR(53714, 16)
  "ArgumentPodStorageBuffer\0" // IR(53730, 25)
  "ArgumentPodUniform\0" // IR(53755, 19)
  "ArgumentPodPushConstant\0" // IR(53774, 24)
  "ArgumentSampledImage\0" // IR(53798, 21)
  "ArgumentStorageImage\0" // IR(53819, 21)
  "ArgumentSampler\0" // IR(53840, 16)
  "ArgumentWorkgroup\0" // IR(53856, 18)
  "SpecConstantWorkgroupSize\0" // IR(53874, 26)
  "SpecConstantGlobalOffset\0" // IR(53900, 25)
  "SpecConstantWorkDim\0" // IR(53925, 20)
  "PushConstantGlobalOffset\0" // IR(53945, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(53970, 30)
  "PushConstantGlobalSize\0" // IR(54000, 23)
  "PushConstantRegionOffset\0" // IR(54023, 25)
  "PushConstantNumWorkgroups\0" // IR(54048, 26)
  "PushConstantRegionGroupOffset\0" // IR(54074, 30)
  "ConstantDataStorageBuffer\0" // IR(54104, 26)
  "ConstantDataUniform\0" // IR(54130, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(54150, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(54180, 28)
  "ArgumentPointerPushConstant\0" // IR(54208, 28)
  "ArgumentPointerUniform\0" // IR(54236, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(54259, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(54294, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(54332, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(54374, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(54419, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(54456, 40)
  "ArgumentStorageTexelBuffer\0" // IR(54496, 27)
  "ArgumentUniformTexelBuffer\0" // IR(54523, 27)
  "ConstantDataPointerPushConstant\0" // IR(54550, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(54582, 40)
  "PrintfInfo\0" // IR(54622, 11)
  "PrintfBufferStorageBuffer\0" // IR(54633, 26)
  "PrintfBufferPointerPushConstant\0" // IR(54659, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(54691, 34)
  "WorkgroupVariableSize\0" // IR(54725, 22)
  "DebugGraph\0" // IR(54747, 11)
  "DebugTensor\0" // IR(54758, 12)
  "DebugImportedEntity\0" // IR(54770, 20)
  "DebugSource\0" // IR(54790, 12)
  "DebugFunctionDefinition\0" // IR(54802, 24)
  "DebugSourceContinued\0" // IR(54826, 21)
  "DebugLine\0" // IR(54847, 10)
  "DebugNoLine\0" // IR(54857, 12)
  "DebugBuildIdentifier\0" // IR(54869, 21)
  "DebugStoragePath\0" // IR(54890, 17)
  "DebugEntryPoint\0" // IR(54907, 16)
  "DebugTypeMatrix\0" // IR(54923, 16)
  "DebugTypeVectorIdEXT\0" // IR(54939, 21)
  "DebugTypeCooperativeMatrixKHR\0" // IR(54960, 30)
  "Configuration\0" // IR(54990, 14)
  "StartCounter\0" // IR(55004, 13)
  "StopCounter\0" // IR(55017, 12)
  "PushConstants\0" // IR(55029, 14)
  "SpecializationMapEntry\0" // IR(55043, 23)
  "DescriptorSetBuffer\0" // IR(55066, 20)
  "DescriptorSetImage\0" // IR(55086, 19)
  "DescriptorSetSampler\0" // IR(55105, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(55126, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(55189, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(55234, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(55283, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(55332, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(55376, 50)
  "DebugModuleINTEL\0" // IR(55426, 17)
  "acos\0" // IR(55443, 5)
  "acosh\0" // IR(55448, 6)
  "acospi\0" // IR(55454, 7)
  "asin\0" // IR(55461, 5)
  "asinh\0" // IR(55466, 6)
  "asinpi\0" // IR(55472, 7)
  "atan\0" // IR(55479, 5)
  "atan2\0" // IR(55484, 6)
  "atanh\0" // IR(55490, 6)
  "atanpi\0" // IR(55496, 7)
  "atan2pi\0" // IR(55503, 8)
  "cbrt\0" // IR(55511, 5)
  "ceil\0" // IR(55516, 5)
  "copysign\0" // IR(55521, 9)
  "cos\0" // IR(55530, 4)
  "cosh\0" // IR(55534, 5)
  "cospi\0" // IR(55539, 6)
  "erfc\0" // IR(55545, 5)
  "erf\0" // IR(55550, 4)
  "exp\0" // IR(55554, 4)
  "exp2\0" // IR(55558, 5)
  "exp10\0" // IR(55563, 6)
  "expm1\0" // IR(55569, 6)
  "fabs\0" // IR(55575, 5)
  "fdim\0" // IR(55580, 5)
  "floor\0" // IR(55585, 6)
  "fma\0" // IR(55591, 4)
  "fmax\0" // IR(55595, 5)
  "fmin\0" // IR(55600, 5)
  "fmod\0" // IR(55605, 5)
  "fract\0" // IR(55610, 6)
  "frexp\0" // IR(55616, 6)
  "hypot\0" // IR(55622, 6)
  "ilogb\0" // IR(55628, 6)
  "ldexp\0" // IR(55634, 6)
  "lgamma\0" // IR(55640, 7)
  "lgamma_r\0" // IR(55647, 9)
  "log\0" // IR(55656, 4)
  "log2\0" // IR(55660, 5)
  "log10\0" // IR(55665, 6)
  "log1p\0" // IR(55671, 6)
  "logb\0" // IR(55677, 5)
  "mad\0" // IR(55682, 4)
  "maxmag\0" // IR(55686, 7)
  "minmag\0" // IR(55693, 7)
  "modf\0" // IR(55700, 5)
  "nan\0" // IR(55705, 4)
  "nextafter\0" // IR(55709, 10)
  "pow\0" // IR(55719, 4)
  "pown\0" // IR(55723, 5)
  "powr\0" // IR(55728, 5)
  "remainder\0" // IR(55733, 10)
  "remquo\0" // IR(55743, 7)
  "rint\0" // IR(55750, 5)
  "rootn\0" // IR(55755, 6)
  "round\0" // IR(55761, 6)
  "rsqrt\0" // IR(55767, 6)
  "sin\0" // IR(55773, 4)
  "sincos\0" // IR(55777, 7)
  "sinh\0" // IR(55784, 5)
  "sinpi\0" // IR(55789, 6)
  "sqrt\0" // IR(55795, 5)
  "tan\0" // IR(55800, 4)
  "tanh\0" // IR(55804, 5)
  "tanpi\0" // IR(55809, 6)
  "tgamma\0" // IR(55815, 7)
  "trunc\0" // IR(55822, 6)
  "half_cos\0" // IR(55828, 9)
  "half_divide\0" // IR(55837, 12)
  "half_exp\0" // IR(55849, 9)
  "half_exp2\0" // IR(55858, 10)
  "half_exp10\0" // IR(55868, 11)
  "half_log\0" // IR(55879, 9)
  "half_log2\0" // IR(55888, 10)
  "half_log10\0" // IR(55898, 11)
  "half_powr\0" // IR(55909, 10)
  "half_recip\0" // IR(55919, 11)
  "half_rsqrt\0" // IR(55930, 11)
  "half_sin\0" // IR(55941, 9)
  "half_sqrt\0" // IR(55950, 10)
  "half_tan\0" // IR(55960, 9)
  "native_cos\0" // IR(55969, 11)
  "native_divide\0" // IR(55980, 14)
  "native_exp\0" // IR(55994, 11)
  "native_exp2\0" // IR(56005, 12)
  "native_exp10\0" // IR(56017, 13)
  "native_log\0" // IR(56030, 11)
  "native_log2\0" // IR(56041, 12)
  "native_log10\0" // IR(56053, 13)
  "native_powr\0" // IR(56066, 12)
  "native_recip\0" // IR(56078, 13)
  "native_rsqrt\0" // IR(56091, 13)
  "native_sin\0" // IR(56104, 11)
  "native_sqrt\0" // IR(56115, 12)
  "native_tan\0" // IR(56127, 11)
  "fclamp\0" // IR(56138, 7)
  "degrees\0" // IR(56145, 8)
  "fmax_common\0" // IR(56153, 12)
  "fmin_common\0" // IR(56165, 12)
  "mix\0" // IR(56177, 4)
  "radians\0" // IR(56181, 8)
  "step\0" // IR(56189, 5)
  "smoothstep\0" // IR(56194, 11)
  "sign\0" // IR(56205, 5)
  "cross\0" // IR(56210, 6)
  "distance\0" // IR(56216, 9)
  "length\0" // IR(56225, 7)
  "normalize\0" // IR(56232, 10)
  "fast_distance\0" // IR(56242, 14)
  "fast_length\0" // IR(56256, 12)
  "fast_normalize\0" // IR(56268, 15)
  "s_abs\0" // IR(56283, 6)
  "s_abs_diff\0" // IR(56289, 11)
  "s_add_sat\0" // IR(56300, 10)
  "u_add_sat\0" // IR(56310, 10)
  "s_hadd\0" // IR(56320, 7)
  "u_hadd\0" // IR(56327, 7)
  "s_rhadd\0" // IR(56334, 8)
  "u_rhadd\0" // IR(56342, 8)
  "s_clamp\0" // IR(56350, 8)
  "u_clamp\0" // IR(56358, 8)
  "clz\0" // IR(56366, 4)
  "ctz\0" // IR(56370, 4)
  "s_mad_hi\0" // IR(56374, 9)
  "u_mad_sat\0" // IR(56383, 10)
  "s_mad_sat\0" // IR(56393, 10)
  "s_max\0" // IR(56403, 6)
  "u_max\0" // IR(56409, 6)
  "s_min\0" // IR(56415, 6)
  "u_min\0" // IR(56421, 6)
  "s_mul_hi\0" // IR(56427, 9)
  "rotate\0" // IR(56436, 7)
  "s_sub_sat\0" // IR(56443, 10)
  "u_sub_sat\0" // IR(56453, 10)
  "u_upsample\0" // IR(56463, 11)
  "s_upsample\0" // IR(56474, 11)
  "popcount\0" // IR(56485, 9)
  "s_mad24\0" // IR(56494, 8)
  "u_mad24\0" // IR(56502, 8)
  "s_mul24\0" // IR(56510, 8)
  "u_mul24\0" // IR(56518, 8)
  "vloadn\0" // IR(56526, 7)
  "vstoren\0" // IR(56533, 8)
  "vload_half\0" // IR(56541, 11)
  "vload_halfn\0" // IR(56552, 12)
  "vstore_half\0" // IR(56564, 12)
  "vstore_half_r\0" // IR(56576, 14)
  "vstore_halfn\0" // IR(56590, 13)
  "vstore_halfn_r\0" // IR(56603, 15)
  "vloada_halfn\0" // IR(56618, 13)
  "vstorea_halfn\0" // IR(56631, 14)
  "vstorea_halfn_r\0" // IR(56645, 16)
  "shuffle\0" // IR(56661, 8)
  "shuffle2\0" // IR(56669, 9)
  "printf\0" // IR(56678, 7)
  "prefetch\0" // IR(56685, 9)
  "bitselect\0" // IR(56694, 10)
  "select\0" // IR(56704, 7)
  "u_abs\0" // IR(56711, 6)
  "u_abs_diff\0" // IR(56717, 11)
  "u_mul_hi\0" // IR(56728, 9)
  "u_mad_hi\0" // IR(56737, 9)
  "CubeFaceIndexAMD\0" // IR(56746, 17)
  "CubeFaceCoordAMD\0" // IR(56763, 17)
  "TimeAMD\0" // IR(56780, 8)
  "SwizzleInvocationsAMD\0" // IR(56788, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(56810, 28)
  "WriteInvocationAMD\0" // IR(56838, 19)
  "MbcntAMD\0" // IR(56857, 9)
  "InterpolateAtVertexAMD\0" // IR(56866, 23)
  "FMin3AMD\0" // IR(56889, 9)
  "UMin3AMD\0" // IR(56898, 9)
  "SMin3AMD\0" // IR(56907, 9)
  "FMax3AMD\0" // IR(56916, 9)
  "UMax3AMD\0" // IR(56925, 9)
  "SMax3AMD\0" // IR(56934, 9)
  "FMid3AMD\0" // IR(56943, 9)
  "UMid3AMD\0" // IR(56952, 9)
  "SMid3AMD\0" // IR(56961, 9)
  "ARGMAX\0" // IR(56970, 7)
  "AVG_POOL2D\0" // IR(56977, 11)
  "CONV2D\0" // IR(56988, 7)
  "CONV3D\0" // IR(56995, 7)
  "DEPTHWISE_CONV2D\0" // IR(57002, 17)
  "FFT2D\0" // IR(57019, 6)
  "MATMUL\0" // IR(57025, 7)
  "MAX_POOL2D\0" // IR(57032, 11)
  "RFFT2D\0" // IR(57043, 7)
  "TRANSPOSE_CONV2D\0" // IR(57050, 17)
  "CLAMP\0" // IR(57067, 6)
  "ERF\0" // IR(57073, 4)
  "SIGMOID\0" // IR(57077, 8)
  "TANH\0" // IR(57085, 5)
  "ADD\0" // IR(57090, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(57094, 23)
  "BITWISE_AND\0" // IR(57117, 12)
  "BITWISE_OR\0" // IR(57129, 11)
  "BITWISE_XOR\0" // IR(57140, 12)
  "INTDIV\0" // IR(57152, 7)
  "LOGICAL_AND\0" // IR(57159, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(57171, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(57190, 20)
  "LOGICAL_OR\0" // IR(57210, 11)
  "LOGICAL_XOR\0" // IR(57221, 12)
  "MAXIMUM\0" // IR(57233, 8)
  "MINIMUM\0" // IR(57241, 8)
  "MUL\0" // IR(57249, 4)
  "POW\0" // IR(57253, 4)
  "SUB\0" // IR(57257, 4)
  "TABLE\0" // IR(57261, 6)
  "ABS\0" // IR(57267, 4)
  "BITWISE_NOT\0" // IR(57271, 12)
  "CEIL\0" // IR(57283, 5)
  "CLZ\0" // IR(57288, 4)
  "COS\0" // IR(57292, 4)
  "EXP\0" // IR(57296, 4)
  "FLOOR\0" // IR(57300, 6)
  "LOG\0" // IR(57306, 4)
  "LOGICAL_NOT\0" // IR(57310, 12)
  "NEGATE\0" // IR(57322, 7)
  "RECIPROCAL\0" // IR(57329, 11)
  "RSQRT\0" // IR(57340, 6)
  "SIN\0" // IR(57346, 4)
  "SELECT\0" // IR(57350, 7)
  "EQUAL\0" // IR(57357, 6)
  "GREATER\0" // IR(57363, 8)
  "GREATER_EQUAL\0" // IR(57371, 14)
  "REDUCE_ALL\0" // IR(57385, 11)
  "REDUCE_ANY\0" // IR(57396, 11)
  "REDUCE_MAX\0" // IR(57407, 11)
  "REDUCE_MIN\0" // IR(57418, 11)
  "REDUCE_PRODUCT\0" // IR(57429, 15)
  "REDUCE_SUM\0" // IR(57444, 11)
  "CONCAT\0" // IR(57455, 7)
  "PAD\0" // IR(57462, 4)
  "RESHAPE\0" // IR(57466, 8)
  "REVERSE\0" // IR(57474, 8)
  "SLICE\0" // IR(57482, 6)
  "TILE\0" // IR(57488, 5)
  "TRANSPOSE\0" // IR(57493, 10)
  "GATHER\0" // IR(57503, 7)
  "SCATTER\0" // IR(57510, 8)
  "RESIZE\0" // IR(57518, 7)
  "CAST\0" // IR(57525, 5)
  "RESCALE\0" // IR(57530, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(5958, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6869, 18), // 1 SubgroupEqMaskKHR
  IR(6924, 18), // 2 SubgroupGeMaskKHR
  IR(6957, 18), // 3 SubgroupGtMaskKHR
  IR(6990, 18), // 4 SubgroupLeMaskKHR
  IR(7023, 18), // 5 SubgroupLtMaskKHR
  IR(7930, 12), // 6 BaryCoordNV
  IR(7985, 19), // 7 BaryCoordNoPerspNV
  IR(8016, 15), // 8 FragmentSizeNV
  IR(8073, 22), // 9 InvocationsPerPixelNV
  IR(8201, 11), // 10 LaunchIdNV
  IR(8226, 13), // 11 LaunchSizeNV
  IR(8257, 17), // 12 WorldRayOriginNV
  IR(8295, 20), // 13 WorldRayDirectionNV
  IR(8334, 18), // 14 ObjectRayOriginNV
  IR(8374, 21), // 15 ObjectRayDirectionNV
  IR(8406, 10), // 16 RayTminNV
  IR(8427, 10), // 17 RayTmaxNV
  IR(8460, 22), // 18 InstanceCustomIndexNV
  IR(8499, 16), // 19 ObjectToWorldNV
  IR(8532, 16), // 20 WorldToObjectNV
  IR(8566, 10), // 21 HitKindNV
  IR(8797, 19), // 22 IncomingRayFlagsNV
  IR(10643, 28), // 23 StorageUniformBufferBlock16
  IR(10706, 17), // 24 StorageUniform16
  IR(11651, 27), // 25 ShaderViewportIndexLayerNV
  IR(11695, 22), // 26 FragmentBarycentricNV
  IR(11748, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11778, 14), // 28 ShadingRateNV
  IR(11822, 29), // 29 GroupNonUniformPartitionedNV
  IR(11868, 20), // 30 ShaderNonUniformEXT
  IR(11911, 26), // 31 RuntimeDescriptorArrayEXT
  IR(11973, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(12051, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(12132, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(12211, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(12287, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(12363, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(12439, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(12517, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12601, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12688, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12751, 21), // 42 VulkanMemoryModelKHR
  IR(12801, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(12833, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(12899, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(13103, 28), // 46 DemoteToHelperInvocationEXT
  IR(13183, 29), // 47 RayTracingOpacityMicromapEXT
  IR(13488, 30), // 48 CooperativeMatrixReductionsNV
  IR(13590, 40), // 49 CooperativeMatrixPerElementOperationsNV
  IR(14343, 26), // 50 FPGAMemoryAttributesINTEL
  IR(14422, 32), // 51 ArbitraryPrecisionIntegersINTEL
  IR(14492, 37), // 52 ArbitraryPrecisionFloatingPointINTEL
  IR(14582, 22), // 53 FPGALoopControlsINTEL
  IR(14677, 24), // 54 FPGAMemoryAccessesINTEL
  IR(14729, 27), // 55 FPGAClusterAttributesINTEL
  IR(14771, 14), // 56 LoopFuseINTEL
  IR(14806, 20), // 57 FPGADSPControlINTEL
  IR(14893, 40), // 58 FPGAInvocationPipeliningAttributesINTEL
  IR(14958, 24), // 59 FPGABufferLocationINTEL
  IR(15017, 34), // 60 ArbitraryPrecisionFixedPointINTEL
  IR(15075, 23), // 61 USMStorageClassesINTEL
  IR(15128, 29), // 62 RuntimeAlignedAttributeINTEL
  IR(15171, 13), // 63 IOPipesINTEL
  IR(15204, 19), // 64 BlockingPipesINTEL
  IR(15237, 13), // 65 FPGARegINTEL
  IR(15269, 22), // 66 DotProductInputAllKHR
  IR(15313, 25), // 67 DotProductInput4x8BitKHR
  IR(15366, 31), // 68 DotProductInput4x8BitPackedKHR
  IR(15408, 14), // 69 DotProductKHR
  IR(15622, 13), // 70 OptNoneINTEL
  IR(15716, 18), // 71 SplitBarrierINTEL
  IR(15783, 29), // 72 FPGAClusterAttributesV2INTEL
  IR(15859, 18), // 73 TaskSequenceINTEL
  IR(15918, 24), // 74 FPGALatencyControlINTEL
  IR(15971, 28), // 75 FPGAArgumentInterfacesINTEL
  IR(16065, 35), // 76 GlobalVariableFPGADecorationsINTEL
  IR(19074, 15), // 77 PerPrimitiveNV
  IR(19122, 12), // 78 PerVertexNV
  IR(19145, 14), // 79 NonUniformEXT
  IR(19175, 19), // 80 RestrictPointerEXT
  IR(19209, 18), // 81 AliasedPointerEXT
  IR(19602, 24), // 82 HlslCounterBufferGOOGLE
  IR(19639, 19), // 83 HlslSemanticGOOGLE
  IR(19769, 14), // 84 RegisterINTEL
  IR(19796, 12), // 85 MemoryINTEL
  IR(19823, 14), // 86 NumbanksINTEL
  IR(19853, 15), // 87 BankwidthINTEL
  IR(19891, 22), // 88 MaxPrivateCopiesINTEL
  IR(19930, 16), // 89 SinglepumpINTEL
  IR(19963, 16), // 90 DoublepumpINTEL
  IR(19999, 19), // 91 MaxReplicatesINTEL
  IR(20039, 20), // 92 SimpleDualPortINTEL
  IR(20071, 11), // 93 MergeINTEL
  IR(20139, 14), // 94 BankBitsINTEL
  IR(20174, 20), // 95 ForcePow2DepthINTEL
  IR(20211, 16), // 96 StridesizeINTEL
  IR(20242, 14), // 97 WordsizeINTEL
  IR(20275, 18), // 98 TrueDualPortINTEL
  IR(20313, 19), // 99 BurstCoalesceINTEL
  IR(20348, 15), // 100 CacheSizeINTEL
  IR(20392, 28), // 101 DontStaticallyCoalesceINTEL
  IR(20435, 14), // 102 PrefetchINTEL
  IR(20467, 17), // 103 StallEnableINTEL
  IR(20510, 25), // 104 FuseLoopsInFunctionINTEL
  IR(20555, 19), // 105 MathOpDSPModeINTEL
  IR(20628, 24), // 106 InitiationIntervalINTEL
  IR(20673, 20), // 107 MaxConcurrencyINTEL
  IR(20714, 20), // 108 PipelineEnableINTEL
  IR(20755, 20), // 109 BufferLocationINTEL
  IR(20795, 19), // 110 IOPipeStorageINTEL
  IR(20973, 15), // 111 StallFreeINTEL
  IR(21071, 25), // 112 LatencyControlLabelINTEL
  IR(21127, 30), // 113 LatencyControlConstraintINTEL
  IR(21185, 27), // 114 ConduitKernelArgumentINTEL
  IR(21244, 31), // 115 RegisterMapKernelArgumentINTEL
  IR(21309, 33), // 116 MMHostInterfaceAddressWidthINTEL
  IR(21373, 30), // 117 MMHostInterfaceDataWidthINTEL
  IR(21432, 28), // 118 MMHostInterfaceLatencyINTEL
  IR(21529, 34), // 119 MMHostInterfaceReadWriteModeINTEL
  IR(21593, 29), // 120 MMHostInterfaceMaxBurstINTEL
  IR(21655, 32), // 121 MMHostInterfaceWaitRequestINTEL
  IR(21714, 26), // 122 StableKernelArgumentINTEL
  IR(21857, 14), // 123 InitModeINTEL
  IR(21900, 28), // 124 ImplementInRegisterMapINTEL
  IR(23254, 14), // 125 OutputLinesNV
  IR(23288, 19), // 126 OutputPrimitivesNV
  IR(23331, 23), // 127 DerivativeGroupQuadsNV
  IR(23379, 24), // 128 DerivativeGroupLinearNV
  IR(23422, 18), // 129 OutputTrianglesNV
  IR(24191, 16), // 130 RayGenerationNV
  IR(24223, 15), // 131 IntersectionNV
  IR(24248, 9), // 132 AnyHitNV
  IR(24271, 13), // 133 ClosestHitNV
  IR(24292, 7), // 134 MissNV
  IR(24311, 11), // 135 CallableNV
  IR(24498, 23), // 136 AllowContractFastINTEL
  IR(24534, 18), // 137 AllowReassocINTEL
  IR(24760, 20), // 138 RuntimeAlignedINTEL
  IR(24904, 20), // 139 PartitionedReduceNV
  IR(24952, 27), // 140 PartitionedInclusiveScanNV
  IR(25007, 27), // 141 PartitionedExclusiveScanNV
  IR(25138, 22), // 142 MakeTexelAvailableKHR
  IR(25177, 20), // 143 MakeTexelVisibleKHR
  IR(25213, 19), // 144 NonPrivateTexelKHR
  IR(25246, 17), // 145 VolatileTexelKHR
  IR(25805, 27), // 146 InitOnDeviceReprogramINTEL
  IR(25856, 23), // 147 InitOnDeviceResetINTEL
  IR(26198, 21), // 148 DependencyArrayINTEL
  IR(26238, 18), // 149 LoopCoalesceINTEL
  IR(26278, 21), // 150 MaxInterleavingINTEL
  IR(26326, 26), // 151 SpeculatedIterationsINTEL
  IR(26367, 14), // 152 NoFusionINTEL
  IR(26397, 15), // 153 LoopCountINTEL
  IR(26439, 26), // 154 MaxReinvocationDelayINTEL
  IR(26831, 24), // 155 MakePointerAvailableKHR
  IR(26874, 22), // 156 MakePointerVisibleKHR
  IR(26914, 21), // 157 NonPrivatePointerKHR
  IR(27001, 10), // 158 VulkanKHR
  IR(16803, 5), // 159 None
  IR(27184, 16), // 160 OutputMemoryKHR
  IR(27214, 17), // 161 MakeAvailableKHR
  IR(27243, 15), // 162 MakeVisibleKHR
  IR(27319, 28), // 163 PackedVectorFormat4x8BitKHR
  IR(27634, 24), // 164 SkipBuiltinPrimitivesNV
  IR(27701, 30), // 165 ForceOpacityMicromap2StateEXT
  IR(28404, 15), // 166 QueueFamilyKHR
  IR(28789, 15), // 167 CallableDataNV
  IR(28828, 23), // 168 IncomingCallableDataNV
  IR(28865, 13), // 169 RayPayloadNV
  IR(28894, 15), // 170 HitAttributeNV
  IR(28931, 21), // 171 IncomingRayPayloadNV
  IR(28974, 21), // 172 ShaderRecordBufferNV
  IR(29017, 25), // 173 PhysicalStorageBufferEXT
  IR(29143, 16), // 174 DeviceOnlyINTEL
  IR(29174, 14), // 175 HostOnlyINTEL
  IR(36429, 8), // 176 SDotKHR
  IR(36442, 8), // 177 UDotKHR
  IR(36456, 9), // 178 SUDotKHR
  IR(36476, 14), // 179 SDotAccSatKHR
  IR(36501, 14), // 180 UDotAccSatKHR
  IR(36527, 15), // 181 SUDotAccSatKHR
  IR(38844, 24), // 182 TypeCooperativeVectorNV
  IR(39041, 27), // 183 CooperativeMatrixConvertNV
  IR(39131, 27), // 184 GroupNonUniformPartitionNV
  IR(40213, 21), // 185 ReportIntersectionNV
  IR(40388, 28), // 186 TypeAccelerationStructureNV
  IR(40469, 23), // 187 RayQueryGetClusterIdNV
  IR(40930, 26), // 188 CooperativeMatrixReduceNV
  IR(41125, 32), // 189 CooperativeMatrixPerElementOpNV
  IR(42697, 21), // 190 DecorateStringGOOGLE
  IR(42739, 27), // 191 MemberDecorateStringGOOGLE
  IR(47996, 28), // 192 ArbitraryFloatSinCosPiINTEL
  IR(48049, 24), // 193 ArbitraryFloatCastINTEL
  IR(48105, 31), // 194 ArbitraryFloatCastFromIntINTEL
  IR(48166, 29), // 195 ArbitraryFloatCastToIntINTEL
  IR(48219, 23), // 196 ArbitraryFloatAddINTEL
  IR(48266, 23), // 197 ArbitraryFloatSubINTEL
  IR(48313, 23), // 198 ArbitraryFloatMulINTEL
  IR(48360, 23), // 199 ArbitraryFloatDivINTEL
  IR(48406, 22), // 200 ArbitraryFloatGTINTEL
  IR(48451, 22), // 201 ArbitraryFloatGEINTEL
  IR(48496, 22), // 202 ArbitraryFloatLTINTEL
  IR(48541, 22), // 203 ArbitraryFloatLEINTEL
  IR(48586, 22), // 204 ArbitraryFloatEQINTEL
  IR(48634, 25), // 205 ArbitraryFloatRecipINTEL
  IR(48685, 25), // 206 ArbitraryFloatRSqrtINTEL
  IR(48735, 24), // 207 ArbitraryFloatCbrtINTEL
  IR(48785, 25), // 208 ArbitraryFloatHypotINTEL
  IR(48835, 24), // 209 ArbitraryFloatSqrtINTEL
  IR(49519, 15), // 210 FixedSqrtINTEL
  IR(49551, 16), // 211 FixedRecipINTEL
  IR(49584, 16), // 212 FixedRsqrtINTEL
  IR(49615, 14), // 213 FixedSinINTEL
  IR(49644, 14), // 214 FixedCosINTEL
  IR(49676, 17), // 215 FixedSinCosINTEL
  IR(49710, 16), // 216 FixedSinPiINTEL
  IR(49743, 16), // 217 FixedCosPiINTEL
  IR(49779, 19), // 218 FixedSinCosPiINTEL
  IR(49813, 14), // 219 FixedLogINTEL
  IR(49842, 14), // 220 FixedExpINTEL
  IR(49886, 29), // 221 PtrCastToCrossWorkgroupINTEL
  IR(49945, 29), // 222 CrossWorkgroupCastToPtrINTEL
  IR(49997, 22), // 223 ReadPipeBlockingINTEL
  IR(50043, 23), // 224 WritePipeBlockingINTEL
  IR(50944, 26), // 225 ControlBarrierArriveINTEL
  IR(50992, 24), // 226 ControlBarrierWaitINTEL
  IR(51041, 24), // 227 TaskSequenceCreateINTEL
  IR(51089, 23), // 228 TaskSequenceAsyncINTEL
  IR(51134, 21), // 229 TaskSequenceGetINTEL
  IR(51181, 25), // 230 TaskSequenceReleaseINTEL
  IR(51229, 22), // 231 TypeTaskSequenceINTEL
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
  spv::Capability::Linkage, // 94
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
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 107
  spv::Capability::VectorAnyINTEL, // 108
  spv::Capability::Int8, // 109
  spv::Capability::FPGAClusterAttributesALTERA, // 110
  spv::Capability::FPGAKernelAttributesINTEL, // 111
  spv::Capability::Subgroup2DBlockIOINTEL, // 112
  spv::Capability::VariableLengthArrayINTEL, // 113
  spv::Capability::UntypedPointersKHR, // 114
  spv::Capability::SpecConditionalINTEL, // 115
  spv::Capability::Float16, // 116
  spv::Capability::Float8EXT, // 117
  spv::Capability::CooperativeMatrixLayoutsARM, // 118
  spv::Capability::Shader, // 119
  spv::Capability::Kernel, // 120
  spv::Capability::Shader, // 121
  spv::Capability::UniformDecoration, // 122
  spv::Capability::GeometryStreams, // 123
  spv::Capability::TransformFeedback, // 124
  spv::Capability::Kernel, // 125
  spv::Capability::FloatControls2, // 126
  spv::Capability::ConstantDataKHR, // 127
  spv::Capability::SampleMaskOverrideCoverageNV, // 128
  spv::Capability::GeometryShaderPassthroughNV, // 129
  spv::Capability::MeshShadingNV, // 130
  spv::Capability::MeshShadingEXT, // 131
  spv::Capability::PushConstantBanksNV, // 132
  spv::Capability::ShaderInvocationReorderNV, // 133
  spv::Capability::ShaderInvocationReorderEXT, // 134
  spv::Capability::BindlessTextureNV, // 135
  spv::Capability::CooperativeMatrixConversionsEXT, // 136
  spv::Capability::VectorComputeINTEL, // 137
  spv::Capability::IndirectReferencesINTEL, // 138
  spv::Capability::AsmINTEL, // 139
  spv::Capability::FunctionFloatControlINTEL, // 140
  spv::Capability::FPGAMemoryAttributesALTERA, // 141
  spv::Capability::FPGAMemoryAccessesALTERA, // 142
  spv::Capability::LoopFuseALTERA, // 143
  spv::Capability::FPGADSPControlALTERA, // 144
  spv::Capability::MemoryAccessAliasingINTEL, // 145
  spv::Capability::FPGAInvocationPipeliningAttributesALTERA, // 146
  spv::Capability::FPGABufferLocationALTERA, // 147
  spv::Capability::IOPipesALTERA, // 148
  spv::Capability::FPGAClusterAttributesV2ALTERA, // 149
  spv::Capability::FPMaxErrorINTEL, // 150
  spv::Capability::FPGALatencyControlALTERA, // 151
  spv::Capability::FPGAArgumentInterfacesALTERA, // 152
  spv::Capability::GlobalVariableHostAccessINTEL, // 153
  spv::Capability::GlobalVariableFPGADecorationsALTERA, // 154
  spv::Capability::CacheControlsINTEL, // 155
  spv::Capability::IntrinsicSAMSUNG, // 156
  spv::Capability::TileImageColorReadAccessEXT, // 157
  spv::Capability::Geometry, // 158
  spv::Capability::Tessellation, // 159
  spv::Capability::MeshShadingNV, // 160
  spv::Capability::MeshShadingEXT, // 161
  spv::Capability::Geometry, // 162
  spv::Capability::MeshShadingNV, // 163
  spv::Capability::MeshShadingEXT, // 164
  spv::Capability::SubgroupDispatch, // 165
  spv::Capability::TileImageDepthReadAccessEXT, // 166
  spv::Capability::TileImageStencilReadAccessEXT, // 167
  spv::Capability::SampleMaskPostDepthCoverage, // 168
  spv::Capability::DenormPreserve, // 169
  spv::Capability::DenormFlushToZero, // 170
  spv::Capability::SignedZeroInfNanPreserve, // 171
  spv::Capability::RoundingModeRTE, // 172
  spv::Capability::RoundingModeRTZ, // 173
  spv::Capability::QuadControlKHR, // 174
  spv::Capability::PoisonFreezeKHR, // 175
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 176
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 177
  spv::Capability::FragmentShaderPixelInterlockEXT, // 178
  spv::Capability::FragmentShaderSampleInterlockEXT, // 179
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 180
  spv::Capability::Shader64BitIndexingEXT, // 181
  spv::Capability::RoundToInfinityINTEL, // 182
  spv::Capability::KernelAttributesINTEL, // 183
  spv::Capability::FloatControls2, // 184
  spv::Capability::RayTracingOpacityMicromapExecutionModeKHR, // 185
  spv::Capability::FPGAKernelAttributesv2INTEL, // 186
  spv::Capability::RegisterLimitsINTEL, // 187
  spv::Capability::Float6EXT, // 188
  spv::Capability::Float4EXT, // 189
  spv::Capability::Float8UnsignedE8M0EXT, // 190
  spv::Capability::MXInt8EXT, // 191
  spv::Capability::FloatControls2, // 192
  spv::Capability::FPFastMathModeINTEL, // 193
  spv::Capability::OptNoneEXT, // 194
  spv::Capability::RuntimeAlignedAttributeALTERA, // 195
  spv::Capability::Kernel, // 196
  spv::Capability::GroupNonUniformArithmetic, // 197
  spv::Capability::GroupNonUniformBallot, // 198
  spv::Capability::GroupNonUniformClustered, // 199
  spv::Capability::GroupNonUniformPartitionedEXT, // 200
  spv::Capability::ImageGatherExtended, // 201
  spv::Capability::MinLod, // 202
  spv::Capability::VulkanMemoryModel, // 203
  spv::Capability::WeakLinkageAMD, // 204
  spv::Capability::FPGALoopControlsALTERA, // 205
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 206
  spv::Capability::RawAccessChainsNV, // 207
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 208
  spv::Capability::RayTracingOpacityMicromapKHR, // 209
  spv::Capability::RayQueryKHR, // 210
  spv::Capability::StorageImageExtendedFormats, // 211
  spv::Capability::Int64ImageEXT, // 212
  spv::Capability::Shader, // 213
  spv::Capability::VectorComputeINTEL, // 214
  spv::Capability::GenericPointer, // 215
  spv::Capability::FunctionPointersINTEL, // 216
  spv::Capability::USMStorageClassesALTERA, // 217
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 218
  spv::Capability::CooperativeMatrixDecodeVectorNV, // 219
  spv::Capability::TensorsARM, // 220
  spv::Capability::Addresses, // 221
  spv::Capability::PhysicalStorageBufferAddresses, // 222
  spv::Capability::LiteralSampler, // 223
  spv::Capability::Addresses, // 224
  spv::Capability::UntypedPointersKHR, // 225
  spv::Capability::Addresses, // 226
  spv::Capability::VariablePointers, // 227
  spv::Capability::VariablePointersStorageBuffer, // 228
  spv::Capability::PhysicalStorageBufferAddresses, // 229
  spv::Capability::Kernel, // 230
  spv::Capability::ImageQuery, // 231
  spv::Capability::ImageQuery, // 232
  spv::Capability::Shader, // 233
  spv::Capability::BitInstructions, // 234
  spv::Capability::DerivativeControl, // 235
  spv::Capability::Groups, // 236
  spv::Capability::SparseResidency, // 237
  spv::Capability::PipeStorage, // 238
  spv::Capability::NamedBarrier, // 239
  spv::Capability::GroupNonUniformVote, // 240
  spv::Capability::GroupNonUniformBallot, // 241
  spv::Capability::GroupNonUniformShuffle, // 242
  spv::Capability::GroupNonUniformShuffleRelative, // 243
  spv::Capability::GroupNonUniformArithmetic, // 244
  spv::Capability::GroupNonUniformClustered, // 245
  spv::Capability::GroupNonUniformPartitionedEXT, // 246
  spv::Capability::GroupNonUniformQuad, // 247
  spv::Capability::Addresses, // 248
  spv::Capability::VariablePointers, // 249
  spv::Capability::VariablePointersStorageBuffer, // 250
  spv::Capability::GraphARM, // 251
  spv::Capability::BitcastExtractEXT, // 252
  spv::Capability::SubgroupBallotKHR, // 253
  spv::Capability::FMAKHR, // 254
  spv::Capability::SubgroupVoteKHR, // 255
  spv::Capability::GroupNonUniformRotateKHR, // 256
  spv::Capability::RayTracingKHR, // 257
  spv::Capability::RayQueryKHR, // 258
  spv::Capability::DotProduct, // 259
  spv::Capability::ReplicatedCompositesEXT, // 260
  spv::Capability::TextureSampleWeightedQCOM, // 261
  spv::Capability::TextureBoxFilterQCOM, // 262
  spv::Capability::TextureBlockMatchQCOM, // 263
  spv::Capability::CooperativeMatrixConversionQCOM, // 264
  spv::Capability::TextureBlockMatch2QCOM, // 265
  spv::Capability::ImageGatherLinearQCOM, // 266
  spv::Capability::ImageGatherExtendedModesQCOM, // 267
  spv::Capability::FragmentMaskAMD, // 268
  spv::Capability::ShaderClockKHR, // 269
  spv::Capability::AbortKHR, // 270
  spv::Capability::ShaderInvocationReorderNV, // 271
  spv::Capability::RayTracingMotionBlurNV, // 272
  spv::Capability::ImageFootprintNV, // 273
  spv::Capability::CooperativeVectorNV, // 274
  spv::Capability::LongVectorEXT, // 275
  spv::Capability::CooperativeVectorNV, // 276
  spv::Capability::CooperativeVectorTrainingNV, // 277
  spv::Capability::CooperativeMatrixConversionsEXT, // 278
  spv::Capability::CooperativeMatrixConversionsNV, // 279
  spv::Capability::DisplacementMicromapNV, // 280
  spv::Capability::ShaderInvocationReorderEXT, // 281
  spv::Capability::RayTracingMotionBlurNV, // 282
  spv::Capability::RayQueryPositionFetchKHR, // 283
  spv::Capability::RayTracingNV, // 284
  spv::Capability::RayTracingKHR, // 285
  spv::Capability::RayQueryKHR, // 286
  spv::Capability::DisplacementMicromapNV, // 287
  spv::Capability::CooperativeMatrixNV, // 288
  spv::Capability::CooperativeMatrixGetCoordinateEXT, // 289
  spv::Capability::FragmentShaderSampleInterlockEXT, // 290
  spv::Capability::FragmentShaderPixelInterlockEXT, // 291
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 292
  spv::Capability::CooperativeMatrixReductionsEXT, // 293
  spv::Capability::CooperativeMatrixPerElementOperationsEXT, // 294
  spv::Capability::TensorAddressingNV, // 295
  spv::Capability::DemoteToHelperInvocation, // 296
  spv::Capability::CooperativeMatrixConversionsNV, // 297
  spv::Capability::SubgroupShuffleINTEL, // 298
  spv::Capability::SubgroupBufferBlockIOINTEL, // 299
  spv::Capability::SubgroupImageBlockIOINTEL, // 300
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 301
  spv::Capability::IntegerFunctions2INTEL, // 302
  spv::Capability::AtomicFloat16MinMaxEXT, // 303
  spv::Capability::AtomicFloat32MinMaxEXT, // 304
  spv::Capability::AtomicFloat64MinMaxEXT, // 305
  spv::Capability::AtomicFloat16VectorNV, // 306
  spv::Capability::ExpectAssumeKHR, // 307
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 308
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 309
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 310
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 311
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 312
  spv::Capability::VariableLengthArrayINTEL, // 313
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 314
  spv::Capability::UnstructuredLoopControlsINTEL, // 315
  spv::Capability::BlockingPipesALTERA, // 316
  spv::Capability::FPGARegALTERA, // 317
  spv::Capability::AtomicFloat16AddEXT, // 318
  spv::Capability::AtomicFloat32AddEXT, // 319
  spv::Capability::AtomicFloat64AddEXT, // 320
  spv::Capability::AtomicFloat16VectorNV, // 321
  spv::Capability::LongCompositesINTEL, // 322
  spv::Capability::BFloat16ConversionINTEL, // 323
  spv::Capability::SplitBarrierEXT, // 324
  spv::Capability::ArithmeticFenceEXT, // 325
  spv::Capability::TaskSequenceALTERA, // 326
  spv::Capability::SubgroupBufferPrefetchINTEL, // 327
  spv::Capability::Subgroup2DBlockTransformINTEL, // 328
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 329
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 330
  spv::Capability::TernaryBitwiseFunctionINTEL, // 331
  spv::Capability::UntypedVariableLengthArrayINTEL, // 332
  spv::Capability::FunctionVariantsINTEL, // 333
  spv::Capability::PredicatedIOINTEL, // 334
  spv::Capability::GroupUniformArithmeticKHR, // 335
  spv::Capability::TensorFloat32RoundingINTEL, // 336
  spv::Capability::MaskedGatherScatterINTEL, // 337
  spv::Capability::BindlessImagesINTEL, // 338
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 339
  spv::Capability::DotProductBFloat16AccVALVE, // 340
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 341
  spv::Capability::DotProductBFloat16AccVALVE, // 342
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 343
  spv::Capability::Float64, // 344
  spv::Capability::InterpolationFunction, // 345
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
  spvtools::Extension::kSPV_EXT_ocp_microscaling_types, // 40
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
  spvtools::Extension::kSPV_QCOM_multiple_wait_queues, // 57
  spvtools::Extension::kSPV_QCOM_image_processing3, // 58
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 59
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 60
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 61
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 62
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 63
  spvtools::Extension::kSPV_KHR_shader_clock, // 64
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 65
  spvtools::Extension::kSPV_KHR_quad_control, // 66
  spvtools::Extension::kSPV_INTEL_int4, // 67
  spvtools::Extension::kSPV_KHR_bfloat16, // 68
  spvtools::Extension::kSPV_KHR_abort, // 69
  spvtools::Extension::kSPV_EXT_descriptor_heap, // 70
  spvtools::Extension::kSPV_KHR_constant_data, // 71
  spvtools::Extension::kSPV_KHR_poison_freeze, // 72
  spvtools::Extension::kSPV_AMD_weak_linkage, // 73
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 74
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 75
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 76
  spvtools::Extension::kSPV_NV_viewport_array2, // 77
  spvtools::Extension::kSPV_NV_viewport_array2, // 78
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 79
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 80
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 81
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 82
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 83
  spvtools::Extension::kSPV_EXT_shader_subgroup_partitioned, // 84
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 85
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 86
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 87
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 88
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 89
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 90
  spvtools::Extension::kSPV_NV_displacement_micromap, // 91
  spvtools::Extension::kSPV_KHR_opacity_micromap, // 92
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 93
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 94
  spvtools::Extension::kSPV_EXT_shader_invocation_reorder, // 95
  spvtools::Extension::kSPV_NV_bindless_texture, // 96
  spvtools::Extension::kSPV_NV_cooperative_vector, // 97
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 98
  spvtools::Extension::kSPV_NV_raw_access_chains, // 99
  spvtools::Extension::kSPV_NV_push_constant_bank, // 100
  spvtools::Extension::kSPV_EXT_long_vector, // 101
  spvtools::Extension::kSPV_EXT_shader_64bit_indexing, // 102
  spvtools::Extension::kSPV_EXT_cooperative_matrix_maintenance1, // 103
  spvtools::Extension::kSPV_EXT_cooperative_matrix_maintenance1, // 104
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 105
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 106
  spvtools::Extension::kSPV_NV_tensor_addressing, // 107
  spvtools::Extension::kSPV_NV_cooperative_matrix_decode_vector, // 108
  spvtools::Extension::kSPV_INTEL_subgroups, // 109
  spvtools::Extension::kSPV_INTEL_media_block_io, // 110
  spvtools::Extension::kSPV_INTEL_float_controls2, // 111
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 112
  spvtools::Extension::kSPV_INTEL_function_pointers, // 113
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 114
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 115
  spvtools::Extension::kSPV_INTEL_vector_compute, // 116
  spvtools::Extension::kSPV_KHR_expect_assume, // 117
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 118
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 119
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 120
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 121
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 122
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 123
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 124
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 125
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 126
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 127
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 128
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 129
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 130
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 131
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 132
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 133
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 134
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 135
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 136
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 137
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 138
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 139
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 140
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 141
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 142
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 143
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 144
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 145
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 146
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 147
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 148
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 149
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 150
  spvtools::Extension::kSPV_INTEL_io_pipes, // 151
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 152
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 153
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 154
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 155
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 156
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 157
  spvtools::Extension::kSPV_EXT_replicated_composites, // 158
  spvtools::Extension::kSPV_KHR_bit_instructions, // 159
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 160
  spvtools::Extension::kSPV_KHR_float_controls2, // 161
  spvtools::Extension::kSPV_KHR_fma, // 162
  spvtools::Extension::kSPV_KHR_opacity_micromap, // 163
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 164
  spvtools::Extension::kSPV_INTEL_long_composites, // 165
  spvtools::Extension::kSPV_EXT_optnone, // 166
  spvtools::Extension::kSPV_INTEL_optnone, // 167
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 168
  spvtools::Extension::kSPV_INTEL_debug_module, // 169
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 170
  spvtools::Extension::kSPV_EXT_split_barrier, // 171
  spvtools::Extension::kSPV_INTEL_split_barrier, // 172
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 173
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 174
  spvtools::Extension::kSPV_INTEL_task_sequence, // 175
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 176
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 177
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 178
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 179
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 180
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 181
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 182
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 183
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 184
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 185
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 186
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 187
  spvtools::Extension::kSPV_INTEL_function_variants, // 188
  spvtools::Extension::kSPV_INTEL_predicated_io, // 189
  spvtools::Extension::kSPV_INTEL_rounded_divide_sqrt, // 190
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 191
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 192
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 193
  spvtools::Extension::kSPV_INTEL_cache_controls, // 194
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 195
  spvtools::Extension::kSPV_INTEL_bindless_images, // 196
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 197
  spvtools::Extension::kSPV_SAMSUNG_intrinsic, // 198
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 199
  spvtools::Extension::kSPV_NV_mesh_shader, // 200
  spvtools::Extension::kSPV_EXT_mesh_shader, // 201
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 202
  spvtools::Extension::kSPV_GOOGLE_user_type, // 203
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 204
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 205
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 206
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 207
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 208
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 209
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 210
  spvtools::Extension::kSPV_KHR_variable_pointers, // 211
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 212
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 213
  spvtools::Extension::kSPV_KHR_ray_tracing, // 214
  spvtools::Extension::kSPV_KHR_ray_query, // 215
  spvtools::Extension::kSPV_NV_ray_tracing, // 216
  spvtools::Extension::kSPV_KHR_ray_tracing, // 217
  spvtools::Extension::kSPV_KHR_ray_query, // 218
  spvtools::Extension::kSPV_NV_displacement_micromap, // 219
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 220
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 221
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
  SPV_OPERAND_TYPE_ID, // 481
  SPV_OPERAND_TYPE_ID, // 482
  SPV_OPERAND_TYPE_ID, // 483
  SPV_OPERAND_TYPE_ID, // 484
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 485
  SPV_OPERAND_TYPE_TYPE_ID, // 486
  SPV_OPERAND_TYPE_RESULT_ID, // 487
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 488
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
  SPV_OPERAND_TYPE_ID, // 535
  SPV_OPERAND_TYPE_ID, // 536
  SPV_OPERAND_TYPE_ID, // 537
  SPV_OPERAND_TYPE_ID, // 538
  SPV_OPERAND_TYPE_ID, // 539
  SPV_OPERAND_TYPE_ID, // 540
  SPV_OPERAND_TYPE_ID, // 541
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 542
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 543
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
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 811
  SPV_OPERAND_TYPE_VARIABLE_ID, // 812
  SPV_OPERAND_TYPE_ID, // 813
  SPV_OPERAND_TYPE_ID, // 814
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 815
  SPV_OPERAND_TYPE_ID, // 816
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 817
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 818
  SPV_OPERAND_TYPE_ID, // 819
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 820
  SPV_OPERAND_TYPE_ID, // 821
  SPV_OPERAND_TYPE_ID, // 822
  SPV_OPERAND_TYPE_ID, // 823
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 824
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 825
  SPV_OPERAND_TYPE_ID, // 826
  SPV_OPERAND_TYPE_ID, // 827
  SPV_OPERAND_TYPE_ID, // 828
  SPV_OPERAND_TYPE_ID, // 829
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 830
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 831
  SPV_OPERAND_TYPE_ID, // 832
  SPV_OPERAND_TYPE_ID, // 833
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 834
  SPV_OPERAND_TYPE_VARIABLE_ID, // 835
  SPV_OPERAND_TYPE_ID, // 836
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 837
  SPV_OPERAND_TYPE_ID, // 838
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 839
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 840
  SPV_OPERAND_TYPE_ID, // 841
  SPV_OPERAND_TYPE_ID, // 842
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 843
  SPV_OPERAND_TYPE_VARIABLE_ID, // 844
  SPV_OPERAND_TYPE_ID, // 845
  SPV_OPERAND_TYPE_ID, // 846
  SPV_OPERAND_TYPE_ID, // 847
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 848
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 849
  SPV_OPERAND_TYPE_ID, // 850
  SPV_OPERAND_TYPE_ID, // 851
  SPV_OPERAND_TYPE_ID, // 852
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 853
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 854
  SPV_OPERAND_TYPE_ID, // 855
  SPV_OPERAND_TYPE_ID, // 856
  SPV_OPERAND_TYPE_ID, // 857
  SPV_OPERAND_TYPE_ID, // 858
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 859
  SPV_OPERAND_TYPE_ID, // 860
  SPV_OPERAND_TYPE_ID, // 861
  SPV_OPERAND_TYPE_ID, // 862
  SPV_OPERAND_TYPE_ID, // 863
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 864
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 865
  SPV_OPERAND_TYPE_ID, // 866
  SPV_OPERAND_TYPE_ID, // 867
  SPV_OPERAND_TYPE_ID, // 868
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 869
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 870
  SPV_OPERAND_TYPE_ID, // 871
  SPV_OPERAND_TYPE_ID, // 872
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 873
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 874
  SPV_OPERAND_TYPE_VARIABLE_ID, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_ID, // 877
  SPV_OPERAND_TYPE_ID, // 878
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 879
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 880
  SPV_OPERAND_TYPE_ID, // 881
  SPV_OPERAND_TYPE_ID, // 882
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 883
  SPV_OPERAND_TYPE_ID, // 884
  SPV_OPERAND_TYPE_ID, // 885
  SPV_OPERAND_TYPE_ID, // 886
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 887
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 888
  SPV_OPERAND_TYPE_ID, // 889
  SPV_OPERAND_TYPE_ID, // 890
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 891
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 892
  SPV_OPERAND_TYPE_ID, // 893
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 894
  SPV_OPERAND_TYPE_ID, // 895
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 896
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 901
  SPV_OPERAND_TYPE_ID, // 902
  SPV_OPERAND_TYPE_ID, // 903
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 904
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 905
  SPV_OPERAND_TYPE_ID, // 906
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 907
  SPV_OPERAND_TYPE_ID, // 908
  SPV_OPERAND_TYPE_ID, // 909
  SPV_OPERAND_TYPE_ID, // 910
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 911
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 912
  SPV_OPERAND_TYPE_ID, // 913
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 914
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 915
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 916
  SPV_OPERAND_TYPE_ID, // 917
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 918
  SPV_OPERAND_TYPE_ID, // 919
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 920
  SPV_OPERAND_TYPE_ID, // 921
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 922
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 923
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 924
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 925
  SPV_OPERAND_TYPE_ID, // 926
  SPV_OPERAND_TYPE_ID, // 927
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 928
  SPV_OPERAND_TYPE_ID, // 929
  SPV_OPERAND_TYPE_ID, // 930
  SPV_OPERAND_TYPE_ID, // 931
  SPV_OPERAND_TYPE_ID, // 932
  SPV_OPERAND_TYPE_ID, // 933
  SPV_OPERAND_TYPE_ID, // 934
  SPV_OPERAND_TYPE_ID, // 935
  SPV_OPERAND_TYPE_ID, // 936
  SPV_OPERAND_TYPE_VARIABLE_ID, // 937
  SPV_OPERAND_TYPE_ID, // 938
  SPV_OPERAND_TYPE_ID, // 939
  SPV_OPERAND_TYPE_ID, // 940
  SPV_OPERAND_TYPE_ID, // 941
  SPV_OPERAND_TYPE_ID, // 942
  SPV_OPERAND_TYPE_ID, // 943
  SPV_OPERAND_TYPE_ID, // 944
  SPV_OPERAND_TYPE_ID, // 945
  SPV_OPERAND_TYPE_ID, // 946
  SPV_OPERAND_TYPE_VARIABLE_ID, // 947
  SPV_OPERAND_TYPE_ID, // 948
  SPV_OPERAND_TYPE_ID, // 949
  SPV_OPERAND_TYPE_ID, // 950
  SPV_OPERAND_TYPE_ID, // 951
  SPV_OPERAND_TYPE_ID, // 952
  SPV_OPERAND_TYPE_ID, // 953
  SPV_OPERAND_TYPE_ID, // 954
  SPV_OPERAND_TYPE_ID, // 955
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 956
  SPV_OPERAND_TYPE_ID, // 957
  SPV_OPERAND_TYPE_ID, // 958
  SPV_OPERAND_TYPE_ID, // 959
  SPV_OPERAND_TYPE_ID, // 960
  SPV_OPERAND_TYPE_VARIABLE_ID, // 961
  SPV_OPERAND_TYPE_ID, // 962
  SPV_OPERAND_TYPE_ID, // 963
  SPV_OPERAND_TYPE_ID, // 964
  SPV_OPERAND_TYPE_ID, // 965
  SPV_OPERAND_TYPE_ID, // 966
  SPV_OPERAND_TYPE_ID, // 967
  SPV_OPERAND_TYPE_ID, // 968
  SPV_OPERAND_TYPE_ID, // 969
  SPV_OPERAND_TYPE_ID, // 970
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 971
  SPV_OPERAND_TYPE_ID, // 972
  SPV_OPERAND_TYPE_ID, // 973
  SPV_OPERAND_TYPE_ID, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_ID, // 976
  SPV_OPERAND_TYPE_ID, // 977
  SPV_OPERAND_TYPE_ID, // 978
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 979
  SPV_OPERAND_TYPE_ID, // 980
  SPV_OPERAND_TYPE_ID, // 981
  SPV_OPERAND_TYPE_ID, // 982
  SPV_OPERAND_TYPE_VARIABLE_ID, // 983
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
  SPV_OPERAND_TYPE_ID, // 1049
  SPV_OPERAND_TYPE_ID, // 1050
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1051
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1052
  SPV_OPERAND_TYPE_ID, // 1053
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 1054
  SPV_OPERAND_TYPE_ID, // 1055
  SPV_OPERAND_TYPE_ID, // 1056
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 1057
  SPV_OPERAND_TYPE_ID, // 1058
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 1059
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1060
  SPV_OPERAND_TYPE_ID, // 1061
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 1062
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1063
  SPV_OPERAND_TYPE_ID, // 1064
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1065
  SPV_OPERAND_TYPE_ID, // 1066
  SPV_OPERAND_TYPE_ID, // 1067
  SPV_OPERAND_TYPE_ID, // 1068
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1069
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1070
  SPV_OPERAND_TYPE_ID, // 1071
  SPV_OPERAND_TYPE_ID, // 1072
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1073
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1074
  SPV_OPERAND_TYPE_ID, // 1075
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 1076
  SPV_OPERAND_TYPE_ID, // 1077
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1078
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1079
  SPV_OPERAND_TYPE_ID, // 1080
  SPV_OPERAND_TYPE_ID, // 1081
  SPV_OPERAND_TYPE_ID, // 1082
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1083
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1084
  SPV_OPERAND_TYPE_ID, // 1085
  SPV_OPERAND_TYPE_ID, // 1086
  SPV_OPERAND_TYPE_ID, // 1087
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1088
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1089
  SPV_OPERAND_TYPE_ID, // 1090
  SPV_OPERAND_TYPE_ID, // 1091
  SPV_OPERAND_TYPE_ID, // 1092
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1093
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1094
  SPV_OPERAND_TYPE_ID, // 1095
  SPV_OPERAND_TYPE_ID, // 1096
  SPV_OPERAND_TYPE_ID, // 1097
  SPV_OPERAND_TYPE_ID, // 1098
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1099
  SPV_OPERAND_TYPE_ID, // 1100
  SPV_OPERAND_TYPE_ID, // 1101
  SPV_OPERAND_TYPE_ID, // 1102
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1103
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1104
  SPV_OPERAND_TYPE_ID, // 1105
  SPV_OPERAND_TYPE_ID, // 1106
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1107
  SPV_OPERAND_TYPE_ID, // 1108
  SPV_OPERAND_TYPE_ID, // 1109
  SPV_OPERAND_TYPE_ID, // 1110
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1111
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1112
  SPV_OPERAND_TYPE_ID, // 1113
  SPV_OPERAND_TYPE_ID, // 1114
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1115
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1116
  SPV_OPERAND_TYPE_ID, // 1117
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1118
  SPV_OPERAND_TYPE_ID, // 1119
  SPV_OPERAND_TYPE_ID, // 1120
  SPV_OPERAND_TYPE_ID, // 1121
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1122
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1123
  SPV_OPERAND_TYPE_ID, // 1124
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1125
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1126
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1127
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1128
  SPV_OPERAND_TYPE_ID, // 1129
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1130
  SPV_OPERAND_TYPE_ID, // 1131
  SPV_OPERAND_TYPE_ID, // 1132
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1133
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1134
  SPV_OPERAND_TYPE_ID, // 1135
  SPV_OPERAND_TYPE_ID, // 1136
  SPV_OPERAND_TYPE_ID, // 1137
  SPV_OPERAND_TYPE_ID, // 1138
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1139
  SPV_OPERAND_TYPE_ID, // 1140
  SPV_OPERAND_TYPE_ID, // 1141
  SPV_OPERAND_TYPE_ID, // 1142
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1143
  SPV_OPERAND_TYPE_ID, // 1144
  SPV_OPERAND_TYPE_ID, // 1145
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1146
  SPV_OPERAND_TYPE_ID, // 1147
  SPV_OPERAND_TYPE_ID, // 1148
  SPV_OPERAND_TYPE_ID, // 1149
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1150
};
