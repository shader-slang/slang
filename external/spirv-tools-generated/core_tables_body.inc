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
    case Extension::kSPV_INTEL_device_barrier: return IR(2314, 25);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(2339, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(2383, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(2409, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(2437, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(2460, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(2495, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(2526, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(2560, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(2587, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(2635, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(2666, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(2695, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(2726, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(2759, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(2778, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(2806, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(2834, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(2877, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2915, 26);
    case Extension::kSPV_INTEL_int4: return IR(2941, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2956, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2975, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(3003, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(3029, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(3049, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(3081, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(3109, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(3134, 33);
    case Extension::kSPV_INTEL_optnone: return IR(3167, 18);
    case Extension::kSPV_INTEL_predicated_io: return IR(3185, 24);
    case Extension::kSPV_INTEL_rounded_divide_sqrt: return IR(3209, 30);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(3239, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(3265, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(3301, 24);
    case Extension::kSPV_INTEL_subgroup_bitcast_shuffle: return IR(3325, 35);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3360, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3395, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3441, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3461, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3485, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3521, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3556, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3593, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3623, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3655, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3680, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3702, 21);
    case Extension::kSPV_KHR_abort: return IR(3723, 14);
    case Extension::kSPV_KHR_bfloat16: return IR(3737, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3754, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3779, 35);
    case Extension::kSPV_KHR_constant_data: return IR(3814, 22);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3836, 27);
    case Extension::kSPV_KHR_device_group: return IR(3863, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3884, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3906, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3929, 24);
    case Extension::kSPV_KHR_fma: return IR(3953, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3965, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(4001, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(4031, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(4059, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(4080, 30);
    case Extension::kSPV_KHR_multiview: return IR(4110, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(4128, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(4163, 26);
    case Extension::kSPV_KHR_opacity_micromap: return IR(4189, 25);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(4214, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(4246, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(4268, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4296, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4317, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4339, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4357, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4377, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4412, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4449, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4483, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4505, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4526, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4557, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4594, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4618, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4656, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4678, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4707, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4742, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4767, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4793, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4821, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4862, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4900, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4924, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4962, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4996, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(5022, 27);
    case Extension::kSPV_NV_cooperative_matrix_decode_vector: return IR(5049, 40);
    case Extension::kSPV_NV_cooperative_vector: return IR(5089, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(5115, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(5144, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(5179, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(5214, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(5242, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(5261, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(5287, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(5312, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5331, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5362, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5399, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5432, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5462, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5495, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5521, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5556, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5576, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5605, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5630, 23);
    case Extension::kSPV_QCOM_bfloat16_muladd: return IR(5653, 25);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5678, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5717, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5743, 27);
    case Extension::kSPV_QCOM_image_processing3: return IR(5770, 27);
    case Extension::kSPV_QCOM_multiple_wait_queues: return IR(5797, 30);
    case Extension::kSPV_QCOM_subgroup_size: return IR(5827, 23);
    case Extension::kSPV_QCOM_tile_shading: return IR(5850, 22);
    case Extension::kSPV_SAMSUNG_intrinsic: return IR(5872, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5894, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5932, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,206> kExtensionNames{{
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
    {IR(2314, 25), static_cast<uint32_t>(kSPV_INTEL_device_barrier)},
    {IR(2339, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(2383, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(2409, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(2437, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(2460, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(2495, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(2526, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(2560, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(2587, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(2635, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(2666, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(2695, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(2726, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(2759, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(2778, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(2806, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(2834, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(2877, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2915, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2941, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2956, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2975, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(3003, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(3029, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(3049, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(3081, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(3109, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(3134, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(3167, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(3185, 24), static_cast<uint32_t>(kSPV_INTEL_predicated_io)},
    {IR(3209, 30), static_cast<uint32_t>(kSPV_INTEL_rounded_divide_sqrt)},
    {IR(3239, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(3265, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(3301, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3325, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_bitcast_shuffle)},
    {IR(3360, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3395, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3441, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3461, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3485, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3521, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3556, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3593, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3623, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3655, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3680, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3702, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3723, 14), static_cast<uint32_t>(kSPV_KHR_abort)},
    {IR(3737, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3754, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3779, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3814, 22), static_cast<uint32_t>(kSPV_KHR_constant_data)},
    {IR(3836, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3863, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3884, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3906, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3929, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3953, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3965, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(4001, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(4031, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(4059, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(4080, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(4110, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(4128, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(4163, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(4189, 25), static_cast<uint32_t>(kSPV_KHR_opacity_micromap)},
    {IR(4214, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(4246, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(4268, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4296, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4317, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4339, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4357, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4377, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4412, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4449, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4483, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4505, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4526, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4557, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4594, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4618, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4656, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4678, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4707, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4742, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4767, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4793, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4821, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4862, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4900, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4924, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4962, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4996, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(5022, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(5049, 40), static_cast<uint32_t>(kSPV_NV_cooperative_matrix_decode_vector)},
    {IR(5089, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(5115, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(5144, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(5179, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(5214, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(5242, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(5261, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(5287, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(5312, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5331, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5362, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5399, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5432, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5462, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5495, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5521, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5556, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5576, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5605, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5630, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5653, 25), static_cast<uint32_t>(kSPV_QCOM_bfloat16_muladd)},
    {IR(5678, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5717, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5743, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5770, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing3)},
    {IR(5797, 30), static_cast<uint32_t>(kSPV_QCOM_multiple_wait_queues)},
    {IR(5827, 23), static_cast<uint32_t>(kSPV_QCOM_subgroup_size)},
    {IR(5850, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5872, 22), static_cast<uint32_t>(kSPV_SAMSUNG_intrinsic)},
    {IR(5894, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5932, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1252> kOperandsByValue{{
  {0,IR(0, 0),IR(5966, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5982, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5992, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(6002, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(6010, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(6031, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(6042, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(6124, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(6140, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(6150, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(6163, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(6176, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(6185, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(6196, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(6286, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(6299, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(6345, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(6393, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(6408, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(6423, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(6433, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(6447, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6457, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6468, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6480, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6507, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6522, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6533, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6543, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6560, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6574, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6588, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6600, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6618, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6637, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6658, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6666, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6677, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6699, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6712, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6727, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6774, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6790, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6803, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6824, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6835, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6861, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6873, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6887, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6913, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6926, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6939, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6949, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6962, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(7017, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(7050, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(7083, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(7116, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(7149, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(7175, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(7188, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(7198, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(7245, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(7269, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(7289, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(7304, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(7335, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(7353, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(7371, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(7391, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(7419, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(7445, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7464, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7491, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7516, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7538, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7573, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7620, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7636, 15)/* SamplerHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7669, 16)/* ResourceHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7685, 15)/* ViewportMaskNV */,IR(0, 0),IR(45, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7721, 20)/* SecondaryPositionNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7760, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7784, 18)/* PositionPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7822, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7844, 16)/* FullyCoveredEXT */,IR(0, 0),IR(50, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7884, 12)/* TaskCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7896, 17)/* PrimitiveCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7913, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7932, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7954, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7976, 15)/* LayerPerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7991, 16)/* MeshViewCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(8007, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(8025, 13)/* BaryCoordKHR */,IR(6, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(8073, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(8112, 12)/* FragSizeEXT */,IR(8, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(8158, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(8203, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(8228, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(8252, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(8280, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(8297, 12)/* LaunchIdKHR */,IR(10, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(8320, 14)/* LaunchSizeKHR */,IR(11, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(8347, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(8382, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(8423, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8460, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8503, 11)/* RayTminKHR */,IR(16, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8524, 11)/* RayTmaxKHR */,IR(17, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8545, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8590, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8623, 17)/* WorldToObjectKHR */,IR(20, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8656, 7)/* HitTNV */,IR(0, 0),IR(57, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8663, 11)/* HitKindKHR */,IR(21, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8684, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(58, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8724, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8781, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8848, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8885, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8924, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(61, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8944, 14)/* HitIsSphereNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8986, 11)/* HitIsLSSNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(9036, 20)/* HitSpherePositionNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(9056, 13)/* WarpsPerSMNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(9088, 10)/* SMCountNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(9098, 9)/* WarpIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(9107, 7)/* SMIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(9114, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(9132, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(9166, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(9199, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(9217, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(9231, 12)/* ClusterIDNV */,IR(0, 0),IR(65, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(9284, 12)/* CullMaskKHR */,IR(0, 0),IR(66, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6186,IR(0, 0),IR(9311, 24)/* DeviceBarrierValidINTEL */,IR(0, 0),IR(67, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(9354, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(6133, 7)/* Shader */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(6208, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(6217, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(6021, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(9361, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5975, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(9369, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(9378, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(9392, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(9400, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(9408, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(9414, 13)/* Int64Atomics */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(9427, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(9438, 15)/* ImageReadWrite */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(9453, 12)/* ImageMipmap */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(9465, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(9471, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(9478, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(9492, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9507, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9521, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9527, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9549, 18)/* GeometryPointSize */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9567, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9587, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9611, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9645, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9678, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9712, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(6150, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(6163, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9745, 15)/* ImageCubeArray */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6489, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9777, 10)/* ImageRect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9787, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9799, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9814, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9819, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9835, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9851, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9858, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9868, 8)/* Image1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9760, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9876, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9890, 12)/* ImageBuffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9902, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9915, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9943, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9954, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9972, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9994, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(10012, 16)/* GeometryStreams */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(10028, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(10058, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(6359, 14)/* MultiViewport */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(10089, 17)/* SubgroupDispatch */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(10106, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(10119, 12)/* PipeStorage */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6740, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(10131, 20)/* GroupNonUniformVote */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(10151, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6995, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(10177, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(10200, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(10231, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(10256, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(6305, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(6373, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(10276, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6897, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(10294, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(10322, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(10350, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(10380, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(10391, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(10428, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(10468, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(10477, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10505, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10515, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(79, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4228,IR(0, 0),IR(10563, 10)/* Float6EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4229,IR(0, 0),IR(10573, 10)/* Float4EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4230,IR(0, 0),IR(10583, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4231,IR(0, 0),IR(10605, 10)/* MXInt8EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4232,IR(0, 0),IR(10615, 18)/* BitcastExtractEXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(7222, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6756, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(7160, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10633, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10666, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10709, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10753, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10769, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10822, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(82, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10874, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10896, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(7257, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(7279, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10917, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10947, 17)/* VariablePointers */,IR(0, 0),IR(83, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10964, 17)/* AtomicStorageOps */,IR(0, 0),IR(84, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10981, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(11009, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(11033, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(85, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(11067, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(11088, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(11103, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(11121, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(11146, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(11162, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(11178, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(11201, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(11213, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(11232, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(86, 2),IR(51, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(6243, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(11264, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(11290, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(11311, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(7319, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(11333, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(88, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(11365, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4504,IR(0, 0),IR(11388, 19)/* BFloat16MulAddQCOM */,IR(0, 0),IR(1, 0),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4506,IR(0, 0),IR(11407, 17)/* SubgroupSizeQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4539,IR(0, 0),IR(11424, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4543,IR(0, 0),IR(11447, 22)/* ImageGatherLinearQCOM */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4544,IR(0, 0),IR(11469, 29)/* ImageGatherExtendedModesQCOM */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(11498, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(11514, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(11536, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7556, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(11552, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(11573, 14)/* Int64ImageEXT */,IR(0, 0),IR(69, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(11587, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7602, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(11602, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(11617, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(11631, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(89, 2),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(11658, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(11674, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(91, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11696, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(92, 2),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5120,IR(0, 0),IR(11725, 9)/* AbortKHR */,IR(0, 0),IR(1, 0),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7651, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(94, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5146,IR(0, 0),IR(11734, 16)/* ConstantDataKHR */,IR(0, 0),IR(1, 0),IR(73, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11750, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5181,IR(0, 0),IR(11766, 15)/* WeakLinkageAMD */,IR(0, 0),IR(95, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11781, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11810, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(71, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(6317, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(96, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7700, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(97, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7741, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(98, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7802, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(81, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7860, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(6257, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11865, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(6271, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(8050, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11904, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(8139, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11979, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(85, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(12038, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(12075, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(12124, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(99, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(12199, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(75, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(12280, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(100, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(12361, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(12438, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(12513, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(12590, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(12665, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(102, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12746, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(104, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12833, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(106, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8754, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(6230, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8701, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12920, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12959, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(6093, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(13054, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(13117, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(13142, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(13162, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(13195, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(9069, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(13233, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(13265, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(92, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(13318, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(13341, 29)/* RayTracingOpacityMicromapKHR */,IR(47, 1),IR(3, 1),IR(94, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(13399, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(61, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(13425, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(61, 1),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(13452, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(13470, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(13495, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(13515, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8815, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(61, 1),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(13537, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8958, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8997, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(13555, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(13575, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(13589, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5429,IR(0, 0),IR(13612, 32)/* CooperativeMatrixConversionsEXT */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(13644, 31)/* CooperativeMatrixReductionsEXT */,IR(48, 1),IR(1, 0),IR(106, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(13705, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(13736, 41)/* CooperativeMatrixPerElementOperationsEXT */,IR(49, 1),IR(1, 0),IR(106, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(13817, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13853, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13883, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(9243, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(61, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5438,IR(0, 0),IR(13911, 34)/* CooperativeMatrixGetCoordinateEXT */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13945, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5447,IR(0, 0),IR(13964, 32)/* CooperativeMatrixDecodeVectorNV */,IR(0, 0),IR(108, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13996, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(14017, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(14044, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(14070, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(14101, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(14122, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(14145, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(14168, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(14190, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(14214, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(14223, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(14246, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(14269, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(14292, 19)/* VectorComputeINTEL */,IR(0, 0),IR(109, 1),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(14311, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(14326, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(14342, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(14375, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(14413, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(14452, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(14477, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(14503, 27)/* FPGAMemoryAttributesALTERA */,IR(50, 1),IR(1, 0),IR(122, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(14556, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(14576, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(51, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(14641, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(52, 1),IR(1, 0),IR(127, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(14716, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(129, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(14746, 23)/* FPGALoopControlsALTERA */,IR(53, 1),IR(1, 0),IR(130, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(14791, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(14813, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(14839, 25)/* FPGAMemoryAccessesALTERA */,IR(54, 1),IR(1, 0),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(14888, 28)/* FPGAClusterAttributesALTERA */,IR(55, 1),IR(1, 0),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14943, 15)/* LoopFuseALTERA */,IR(56, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14972, 21)/* FPGADSPControlALTERA */,IR(57, 1),IR(1, 0),IR(139, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(15013, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(15039, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(58, 1),IR(1, 0),IR(142, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(15120, 25)/* FPGABufferLocationALTERA */,IR(59, 1),IR(1, 0),IR(144, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(15169, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(60, 1),IR(1, 0),IR(146, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(15238, 24)/* USMStorageClassesALTERA */,IR(61, 1),IR(1, 0),IR(148, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(15285, 30)/* RuntimeAlignedAttributeALTERA */,IR(62, 1),IR(1, 0),IR(150, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(15344, 14)/* IOPipesALTERA */,IR(63, 1),IR(1, 0),IR(152, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(15371, 20)/* BlockingPipesALTERA */,IR(64, 1),IR(1, 0),IR(154, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(15410, 14)/* FPGARegALTERA */,IR(65, 1),IR(1, 0),IR(156, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(15437, 19)/* DotProductInputAll */,IR(66, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(15478, 22)/* DotProductInput4x8Bit */,IR(67, 1),IR(110, 1),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(15525, 28)/* DotProductInput4x8BitPacked */,IR(68, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(15584, 11)/* DotProduct */,IR(69, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(9296, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10542, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(15609, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(15633, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(15649, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(78, 1),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(15674, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(15689, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(164, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6032,IR(0, 0),IR(15696, 42)/* RayTracingOpacityMicromapExecutionModeKHR */,IR(0, 0),IR(3, 1),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(15738, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(15758, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(15778, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(167, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(15798, 11)/* OptNoneEXT */,IR(70, 1),IR(1, 0),IR(168, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(15822, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(170, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(15842, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(171, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(15863, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(15887, 16)/* SplitBarrierEXT */,IR(71, 1),IR(1, 0),IR(173, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(15921, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(15940, 30)/* FPGAClusterAttributesV2ALTERA */,IR(72, 1),IR(111, 1),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15999, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(112, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(16027, 19)/* TaskSequenceALTERA */,IR(73, 1),IR(1, 0),IR(176, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(16064, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(16080, 25)/* FPGALatencyControlALTERA */,IR(74, 1),IR(1, 0),IR(179, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(16129, 29)/* FPGAArgumentInterfacesALTERA */,IR(75, 1),IR(1, 0),IR(181, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6185,IR(0, 0),IR(9335, 19)/* DeviceBarrierINTEL */,IR(0, 0),IR(1, 0),IR(183, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(16186, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(184, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(16216, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(76, 1),IR(1, 0),IR(185, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6207,IR(0, 0),IR(16287, 28)/* SubgroupBitcastShuffleINTEL */,IR(0, 0),IR(1, 0),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(16315, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(16343, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(16366, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(113, 1),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(16396, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(113, 1),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(16426, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(190, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(16464, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(191, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(16492, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(114, 2),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(16524, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(16545, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(116, 1),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6257,IR(0, 0),IR(16567, 18)/* PredicatedIOINTEL */,IR(0, 0),IR(1, 0),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6265,IR(0, 0),IR(16585, 23)/* RoundedDivideSqrtINTEL */,IR(0, 0),IR(1, 0),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(16608, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(195, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(16634, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(196, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(16661, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(16686, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(198, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(16705, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(199, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(16725, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(16745, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(117, 1),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(16778, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(117, 1),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(16811, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(91, 1),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(16838, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(118, 1),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7041,IR(0, 0),IR(16870, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(1, 0),IR(202, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(16887, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16899, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16907, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16915, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16921, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16928, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16939, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16948, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16961, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16967, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16977, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16983, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16998, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(17023, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(17039, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(17053, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17066, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17078, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17095, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17107, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17122, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17135, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17150, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17168, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17185, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17206, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17226, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17246, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17262, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17278, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17298, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17322, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17328, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17333, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(17372, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(17383, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17392, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17397, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17404, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(17415, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(17422, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17431, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17441, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17454, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17467, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17478, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(17488, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(17498, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(17508, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(17521, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(17535, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(17549, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(17563, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(17578, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(17594, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(17610, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(17626, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(17645, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(17666, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(17678, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(17690, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17702, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(17717, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(17742, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(17770, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17778, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(17805, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(17832, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(17859, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(17891, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17917, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(17921, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(17928, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(17932, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(17943, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(17954, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(17975, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17986, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(18000, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(18021, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16887, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16899, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16907, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16915, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16921, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16928, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16939, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(16948, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16961, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16967, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16977, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(17023, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(17039, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(17053, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17066, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17078, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17095, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17107, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17122, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17135, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17150, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17168, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17185, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17206, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17226, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17246, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17322, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(17328, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(17333, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(17372, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(17383, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(17392, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(17397, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(17404, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(17415, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(17431, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17441, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17454, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(18039, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(18056, 7)/* SpecId */,IR(0, 0),IR(120, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(18063, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(18069, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(18081, 9)/* RowMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(18090, 9)/* ColMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(18099, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(18111, 13)/* MatrixStride */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(18124, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(18135, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(18146, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(18180, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(18188, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(18202, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(18207, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(18213, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(18222, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(18229, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(18239, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(18248, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(18256, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(18265, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(18274, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(18283, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(18295, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(18307, 8)/* Uniform */,IR(0, 0),IR(122, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(18341, 10)/* UniformId */,IR(0, 0),IR(122, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(18351, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(18371, 7)/* Stream */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(18378, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(18387, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(18397, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(18403, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(18411, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(18425, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(18432, 10)/* XfbBuffer */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(18442, 10)/* XfbStride */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(18498, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(18546, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(18596, 15)/* FPFastMathMode */,IR(0, 0),IR(126, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(18673, 18)/* LinkageAttributes */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(18691, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(18705, 21)/* InputAttachmentIndex */,IR(0, 0),IR(99, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(18726, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(18736, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(18770, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(18782, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(18798, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(18842, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(203, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(18855, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(203, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(18870, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(18888, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(18910, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(18932, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(18950, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(18982, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(19002, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(19025, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(19045, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(19070, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(19097, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(19122, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(19150, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(19167, 12)/* OffsetIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5145,IR(0, 0),IR(19179, 14)/* UTFEncodedKHR */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(19193, 19)/* OverrideCoverageNV */,IR(0, 0),IR(129, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(19212, 14)/* PassthroughNV */,IR(0, 0),IR(130, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(19226, 19)/* ViewportRelativeNV */,IR(0, 0),IR(98, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(19245, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(19273, 16)/* PerPrimitiveEXT */,IR(77, 1),IR(131, 2),IR(204, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(19304, 10)/* PerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(19314, 10)/* PerTaskNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(19324, 13)/* PerVertexKHR */,IR(78, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(19349, 11)/* NonUniform */,IR(79, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(19374, 16)/* RestrictPointer */,IR(80, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(19409, 15)/* AliasedPointer */,IR(81, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(19442, 15)/* MemberOffsetNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(19457, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(19487, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(19518, 7)/* BankNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(19525, 18)/* BindlessSamplerNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(19543, 16)/* BindlessImageNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(19559, 15)/* BoundSamplerNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(19574, 13)/* BoundImageNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5440,IR(0, 0),IR(19587, 30)/* CooperativeMatrixTransposeEXT */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(19617, 14)/* SIMTCallINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(19631, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(139, 1),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(19657, 13)/* ClobberINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(19670, 17)/* SideEffectsINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(19687, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(19714, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(19735, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(19762, 15)/* StackCallINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(19777, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(19803, 14)/* CounterBuffer */,IR(82, 1),IR(1, 0),IR(206, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(19841, 13)/* UserSemantic */,IR(83, 1),IR(1, 0),IR(206, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(19873, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(207, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(19888, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(19945, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(19969, 15)/* RegisterALTERA */,IR(84, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(19998, 13)/* MemoryALTERA */,IR(85, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(20023, 15)/* NumbanksALTERA */,IR(86, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(20052, 16)/* BankwidthALTERA */,IR(87, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(20083, 23)/* MaxPrivateCopiesALTERA */,IR(88, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(20128, 17)/* SinglepumpALTERA */,IR(89, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(20161, 17)/* DoublepumpALTERA */,IR(90, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(20194, 20)/* MaxReplicatesALTERA */,IR(91, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(20233, 21)/* SimpleDualPortALTERA */,IR(92, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(20274, 12)/* MergeALTERA */,IR(93, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(20339, 15)/* BankBitsALTERA */,IR(94, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(20368, 21)/* ForcePow2DepthALTERA */,IR(95, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(20409, 17)/* StridesizeALTERA */,IR(96, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(20442, 15)/* WordsizeALTERA */,IR(97, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(20471, 19)/* TrueDualPortALTERA */,IR(98, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(20508, 20)/* BurstCoalesceALTERA */,IR(99, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(20547, 16)/* CacheSizeALTERA */,IR(100, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(20578, 29)/* DontStaticallyCoalesceALTERA */,IR(101, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(20635, 15)/* PrefetchALTERA */,IR(102, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(20664, 18)/* StallEnableALTERA */,IR(103, 1),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(20699, 26)/* FuseLoopsInFunctionALTERA */,IR(104, 1),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(20750, 20)/* MathOpDSPModeALTERA */,IR(105, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(20789, 16)/* AliasScopeINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(20805, 13)/* NoAliasINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(20818, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(20867, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(20908, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(20949, 21)/* BufferLocationALTERA */,IR(109, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(20990, 20)/* IOPipeStorageALTERA */,IR(110, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(21063, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(21094, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(21119, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(21154, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(21172, 16)/* StallFreeALTERA */,IR(111, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(21234, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(21260, 26)/* LatencyControlLabelALTERA */,IR(112, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(21311, 31)/* LatencyControlConstraintALTERA */,IR(113, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(21372, 28)/* ConduitKernelArgumentALTERA */,IR(114, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(21427, 32)/* RegisterMapKernelArgumentALTERA */,IR(115, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(21490, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(116, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(21557, 31)/* MMHostInterfaceDataWidthALTERA */,IR(117, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(21618, 29)/* MMHostInterfaceLatencyALTERA */,IR(118, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(21709, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(119, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(21778, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(120, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(21837, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(121, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(21902, 27)/* StableKernelArgumentALTERA */,IR(122, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(21994, 16)/* HostAccessINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(22057, 15)/* InitModeALTERA */,IR(123, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(22086, 29)/* ImplementInRegisterMapALTERA */,IR(124, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(22143, 17)/* ConditionalINTEL */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(22196, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(22255, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7040,IR(0, 1),IR(16870, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(22278, 3)/* 1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(22281, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(22284, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(22287, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(22292, 5)/* Rect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(22297, 7)/* Buffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(22304, 12)/* SubpassData */,IR(0, 0),IR(99, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(22316, 17)/* TileImageDataEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(22333, 12)/* Invocations */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(22345, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(22358, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(22380, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(22401, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(22415, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(22430, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(22449, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(22465, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(22481, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(22500, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(22510, 4)/* Xfb */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(22514, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(22529, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(22542, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(22552, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(22567, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(22577, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(22591, 12)/* InputPoints */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(22603, 11)/* InputLines */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(22614, 20)/* InputLinesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(22634, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(22644, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(22668, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(22674, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(22683, 15)/* OutputVertices */,IR(0, 0),IR(159, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(22698, 13)/* OutputPoints */,IR(0, 0),IR(163, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(22711, 16)/* OutputLineStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(22727, 20)/* OutputTriangleStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(22747, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(22759, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(22774, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(22786, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6727, 13)/* SubgroupSize */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(22796, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(22818, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(22842, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(22854, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(22870, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(22904, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(167, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(22938, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(22974, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(208, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(23004, 18)/* PostDepthCoverage */,IR(0, 0),IR(169, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(11088, 15)/* DenormPreserve */,IR(0, 0),IR(170, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(11103, 18)/* DenormFlushToZero */,IR(0, 0),IR(171, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(11121, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(172, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(11146, 16)/* RoundingModeRTE */,IR(0, 0),IR(173, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(11162, 16)/* RoundingModeRTZ */,IR(0, 0),IR(174, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(23022, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(23056, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4507,IR(0, 0),IR(23076, 21)/* SubgroupSizeHalfQCOM */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4508,IR(0, 0),IR(23097, 21)/* SubgroupSizeFullQCOM */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(23118, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(209, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(23147, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(23170, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(23185, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(23200, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(23221, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7620, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(23245, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(23266, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(23294, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(23320, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(23343, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(23370, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(23395, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(210, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(23417, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(176, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(23436, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(176, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(23456, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(23476, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(177, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(23496, 15)/* OutputLinesEXT */,IR(125, 1),IR(131, 2),IR(204, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(23525, 20)/* OutputPrimitivesEXT */,IR(126, 1),IR(131, 2),IR(204, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(23564, 24)/* DerivativeGroupQuadsKHR */,IR(127, 1),IR(178, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(23611, 25)/* DerivativeGroupLinearKHR */,IR(128, 1),IR(179, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(23660, 19)/* OutputTrianglesEXT */,IR(129, 1),IR(131, 2),IR(204, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(23697, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(180, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(23722, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(180, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(23749, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(181, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(23775, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(181, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(23803, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(182, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(23834, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(182, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(13589, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(23867, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(23894, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(23915, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(23936, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(23962, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(23989, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(24011, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(24027, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(24047, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(112, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(24069, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(112, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(24097, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(212, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(24121, 18)/* FPFastMathDefault */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6031,IR(10, 1),IR(24139, 21)/* OpacityMicromapIdKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(24160, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(112, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(24184, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(24210, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(24233, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(24255, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(24330, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(24357, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(24364, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(24384, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(6208, 9)/* Geometry */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(17422, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(24407, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5975, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(24417, 7)/* TaskNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(24424, 7)/* MeshNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(24431, 17)/* RayGenerationKHR */,IR(130, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(24464, 16)/* IntersectionKHR */,IR(131, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(24495, 10)/* AnyHitKHR */,IR(132, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(24514, 14)/* ClosestHitKHR */,IR(133, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(24541, 8)/* MissKHR */,IR(134, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(24556, 12)/* CallableKHR */,IR(135, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(24579, 8)/* TaskEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(24587, 8)/* MeshEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(24595, 9)/* Preserve */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(24604, 12)/* FlushToZero */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(24616, 12)/* BFloat16KHR */,IR(0, 0),IR(91, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(24628, 14)/* Float8E4M3EXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(24642, 14)/* Float8E5M2EXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4223,IR(0, 0),IR(24656, 14)/* Float6E2M3EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4224,IR(0, 0),IR(24670, 14)/* Float6E3M2EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4225,IR(0, 0),IR(24684, 14)/* Float4E2M1EXT */,IR(0, 0),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4226,IR(0, 0),IR(10583, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(192, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4227,IR(0, 0),IR(10605, 10)/* MXInt8EXT */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(24698, 5)/* IEEE */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(24703, 4)/* ALT */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(24707, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(24714, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(24721, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(24725, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(24736, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(24741, 14)/* AllowContract */,IR(136, 1),IR(194, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(24778, 13)/* AllowReassoc */,IR(137, 1),IR(194, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(24809, 15)/* AllowTransform */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(24824, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24828, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(24832, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(24836, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24840, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(24856, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(24872, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(24890, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(24908, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(24915, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(24926, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(24931, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(15798, 11)/* OptNoneEXT */,IR(70, 1),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(24937, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(24942, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(24947, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(24953, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(24958, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(24966, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(24976, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(24984, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(24996, 21)/* RuntimeAlignedALTERA */,IR(138, 1),IR(197, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(25037, 14)/* Gather4x1QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {1,IR(0, 0),IR(25051, 12)/* GatherDQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {2,IR(0, 0),IR(25063, 13)/* GatherH2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {3,IR(0, 0),IR(25076, 13)/* GatherV2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {0,IR(0, 0),IR(25089, 7)/* Reduce */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(25096, 14)/* InclusiveScan */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(25110, 14)/* ExclusiveScan */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(25124, 16)/* ClusteredReduce */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(25140, 21)/* PartitionedReduceEXT */,IR(139, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(25181, 28)/* PartitionedInclusiveScanEXT */,IR(140, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(25236, 28)/* PartitionedExclusiveScanEXT */,IR(141, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(25291, 10)/* NoneINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(25301, 10)/* ReadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(25311, 11)/* WriteINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(25322, 15)/* ReadWriteINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(25337, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(25342, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(25346, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(25351, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(18425, 7)/* Offset */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(25363, 13)/* ConstOffsets */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(18222, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9851, 7)/* MinLod */,IR(0, 0),IR(204, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(25376, 19)/* MakeTexelAvailable */,IR(142, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(25417, 17)/* MakeTexelVisible */,IR(143, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(25454, 16)/* NonPrivateTexel */,IR(144, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(25489, 14)/* VolatileTexel */,IR(145, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(25520, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(25531, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(25542, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(25554, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(25562, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(25572, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(25583, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(25593, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(25604, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(25618, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(25632, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(25647, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(25658, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(25670, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(25682, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(25695, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(25709, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(25723, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(16915, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(25733, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(25744, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(25761, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(25777, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(25797, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(25817, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(25837, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(25856, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(25875, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(25894, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(25910, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(25926, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(25928, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(25930, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(25933, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(25936, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(25940, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(25945, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(25950, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(25955, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(25965, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(25975, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(25978, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(25982, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(25987, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(25993, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(26006, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(26011, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(26017, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(26023, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(26029, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(26034, 28)/* InitOnDeviceReprogramALTERA */,IR(146, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(26089, 24)/* InitOnDeviceResetALTERA */,IR(147, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(26136, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(26143, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(26154, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(26168, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(26180, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(26193, 7)/* Export */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(26200, 7)/* Import */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(26207, 12)/* LinkOnceODR */,IR(0, 0),IR(95, 1),IR(213, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {3,IR(0, 0),IR(26219, 8)/* WeakAMD */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(26227, 14)/* UncachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(26241, 12)/* CachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(26253, 15)/* StreamingINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(26268, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(26293, 17)/* ConstCachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(26310, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(26317, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(26328, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(26347, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(26364, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(26378, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(26392, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(26410, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(26420, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(20818, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(20867, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(26433, 22)/* DependencyArrayALTERA */,IR(148, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(20908, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(26476, 19)/* LoopCoalesceALTERA */,IR(149, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(26513, 22)/* MaxInterleavingALTERA */,IR(150, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(26556, 27)/* SpeculatedIterationsALTERA */,IR(151, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(26609, 15)/* NoFusionALTERA */,IR(152, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(26638, 16)/* LoopCountALTERA */,IR(153, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(26669, 27)/* MaxReinvocationDelayALTERA */,IR(154, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {268435456,IR(0, 1),IR(11424, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(26722, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(26751, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(26780, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(26801, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(26827, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(26850, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(26873, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(26896, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(26919, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(26936, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(26953, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(26979, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(27005, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(27032, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(18256, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(27059, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(25542, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(27067, 21)/* MakePointerAvailable */,IR(155, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(27112, 19)/* MakePointerVisible */,IR(156, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(27153, 18)/* NonPrivatePointer */,IR(157, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(27192, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(146, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(27212, 17)/* NoAliasINTELMask */,IR(0, 0),IR(146, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(27229, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(27236, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(27244, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(27251, 7)/* Vulkan */,IR(158, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(27268, 8)/* Relaxed */,IR(159, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(27276, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(27284, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(27292, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(27307, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(27330, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(27344, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(27359, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(27375, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(27396, 20)/* AtomicCounterMemory */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(27416, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(27428, 13)/* OutputMemory */,IR(160, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(27457, 14)/* MakeAvailable */,IR(161, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(27488, 12)/* MakeVisible */,IR(162, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(18256, 9)/* Volatile */,IR(0, 0),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(27515, 10)/* AutoINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(27525, 5)/* WRAP */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(27530, 4)/* SAT */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(27534, 9)/* SAT_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(27543, 8)/* SAT_SYM */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(27551, 25)/* PackedVectorFormat4x8Bit */,IR(163, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(27604, 4)/* TRN */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(27608, 9)/* TRN_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(27617, 4)/* RND */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(27621, 9)/* RND_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(27630, 8)/* RND_INF */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(27638, 12)/* RND_MIN_INF */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(27650, 9)/* RND_CONV */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(27659, 13)/* RND_CONV_ODD */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(27672, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(27697, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(17770, 8)/* NoneKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(27720, 10)/* OpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(27730, 12)/* NoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(27742, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(27765, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(27789, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(27816, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(27844, 14)/* CullOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(27858, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(27874, 17)/* SkipTrianglesKHR */,IR(164, 1),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(27915, 13)/* SkipAABBsKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(27928, 30)/* ForceOpacityMicromap2StateKHR */,IR(165, 1),IR(211, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(27988, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(28029, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(28066, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(28103, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(28144, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(28186, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(28219, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(28252, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(28264, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(28270, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(28277, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(28292, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(28300, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(28307, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(28315, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(28323, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(28331, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(28336, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(28342, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(28353, 6)/* Rg32f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(28359, 6)/* Rg16f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(28365, 13)/* R11fG11fB10f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(28378, 5)/* R16f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(28383, 7)/* Rgba16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(28390, 8)/* Rgb10A2 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(28398, 5)/* Rg16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(28403, 4)/* Rg8 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(28407, 4)/* R16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(28411, 3)/* R8 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(28414, 12)/* Rgba16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(28426, 10)/* Rg16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(28436, 9)/* Rg8Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(28445, 9)/* R16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(28454, 8)/* R8Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(28462, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(28470, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(28478, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(28485, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(28490, 6)/* Rg32i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(28496, 6)/* Rg16i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(28502, 5)/* Rg8i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(28507, 5)/* R16i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(28512, 4)/* R8i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(28516, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(28525, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(28534, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(28542, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(28548, 10)/* Rgb10a2ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(28558, 7)/* Rg32ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(28565, 7)/* Rg16ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(28572, 6)/* Rg8ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(28578, 6)/* R16ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(28584, 5)/* R8ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(28589, 6)/* R64ui */,IR(0, 0),IR(214, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(28595, 5)/* R64i */,IR(0, 0),IR(214, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(28600, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(28612, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(28619, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(28629, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(28638, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(28649, 12)/* QueueFamily */,IR(166, 1),IR(205, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(28676, 14)/* ShaderCallKHR */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28690, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(28698, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28710, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(16887, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16899, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16907, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16915, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16921, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16928, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16939, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16948, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16961, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16967, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16977, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16983, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16998, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(17023, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(17039, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(17053, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17066, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17078, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17095, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17107, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17122, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17135, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17150, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17168, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17185, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17206, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17226, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17246, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17262, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17278, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17298, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(28739, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17322, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17328, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17333, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(17372, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(17383, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17392, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17397, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17404, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(17415, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(17422, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17431, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17441, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17454, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17467, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(28307, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(28765, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(28770, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(28775, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(28784, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(28795, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(28800, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(28815, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(28820, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(28827, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(28832, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(28837, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(28843, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(28847, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {14,IR(0, 0),IR(28852, 5)/* Pred */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {15,IR(0, 0),IR(28857, 9)/* ApilaJai */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(28866, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(28882, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(18307, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(28888, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(28619, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(28895, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(28910, 8)/* Private */,IR(0, 0),IR(215, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(28918, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(28927, 8)/* Generic */,IR(0, 0),IR(217, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(28935, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(28948, 14)/* AtomicCounter */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(28962, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(28968, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(214, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(28982, 13)/* TileImageEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(28995, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(29014, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(29030, 16)/* CallableDataKHR */,IR(167, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(29061, 24)/* IncomingCallableDataKHR */,IR(168, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(29108, 14)/* RayPayloadKHR */,IR(169, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(29135, 16)/* HitAttributeKHR */,IR(170, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(29166, 22)/* IncomingRayPayloadKHR */,IR(171, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(29209, 22)/* ShaderRecordBufferKHR */,IR(172, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(29252, 22)/* PhysicalStorageBuffer */,IR(173, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(29299, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(29320, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(29344, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(29366, 17)/* CodeSectionINTEL */,IR(0, 0),IR(218, 1),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(29383, 17)/* DeviceOnlyALTERA */,IR(174, 1),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(29416, 15)/* HostOnlyALTERA */,IR(175, 1),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(26227, 14)/* UncachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(29445, 18)/* WriteThroughINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(29463, 15)/* WriteBackINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(26253, 15)/* StreamingINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(17018, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(29478, 11)/* TensorView */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(29489, 11)/* DecodeFunc */,IR(0, 0),IR(108, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {4,IR(10, 1),IR(29500, 17)/* DecodeVectorFunc */,IR(0, 0),IR(221, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(29517, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(18265, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(28252, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(28270, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(28277, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(29527, 8)/* NoneARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(29535, 15)/* NontemporalARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(29550, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(29570, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(29594, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(29616, 21)/* NonPrivateElementARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(136, 307);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(443, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(451, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(454, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(456, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(475, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(485, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(489, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(504, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(508, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(514, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(517, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(520, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(524, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(532, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(535, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(551, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(560, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(563, 152);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(715, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(723, 101);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(824, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(841, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(843, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(851, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(853, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(862, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(866, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(870, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(876, 9);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(885, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(889, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(896, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(900, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(917, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(943, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(963, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(965, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(968, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(970, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(972, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(976, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(981, 21);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1002, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1017, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1026, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1030, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1045, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1046, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1050, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1051, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1059, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1062, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1074, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1076, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1079, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1081, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1086, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1088, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1130, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1137, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1140, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1141, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1149, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1152, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1154, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1174, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1184, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1188, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1204, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1233, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1237, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1241, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1246, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(900, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1017, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1050, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(508, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1002, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1059, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(843, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1246, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(136, 307);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1432> kOperandNames{{
  {IR(5966, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5992, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5982, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(6002, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6010, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6031, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6042, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6066, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(8025, 13), 94}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8038, 12), 94}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7371, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7391, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8073, 20), 95}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8093, 19), 95}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7419, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7516, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7445, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7464, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7491, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7175, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7149, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6150, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7932, 22), 89}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9231, 12), 133}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6913, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6887, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6926, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6163, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7954, 22), 90}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9284, 12), 134}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8280, 17), 101}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8684, 17), 115}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9311, 24), 135}, // 32 DeviceBarrierValidINTEL in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7245, 12), 62}, // 33 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7188, 10), 60}, // 34 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6677, 22), 37}, // 35 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6447, 10), 21}, // 36 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6533, 10), 27}, // 37 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8158, 23), 97}, // 38 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8112, 12), 96}, // 39 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7538, 18), 75}, // 40 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8124, 15), 96}, // 41 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6468, 12), 23}, // 42 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7844, 16), 85}, // 43 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6618, 19), 33}, // 44 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6712, 15), 39}, // 45 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6699, 13), 38}, // 46 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6666, 11), 36}, // 47 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6543, 17), 28}, // 48 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8986, 11), 122}, // 49 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8944, 14), 121}, // 50 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9166, 33), 130}, // 51 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9132, 34), 129}, // 52 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8663, 11), 114}, // 53 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8674, 10), 114}, // 54 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9114, 18), 128}, // 55 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9217, 14), 132}, // 56 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8848, 37), 118}, // 57 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8781, 34), 117}, // 58 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9036, 20), 123}, // 59 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9199, 18), 131}, // 60 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8656, 7), 113}, // 61 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8724, 30), 116}, // 62 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8885, 20), 119}, // 63 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8905, 19), 119}, // 64 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8545, 23), 110}, // 65 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8568, 22), 110}, // 66 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6185, 11), 12}, // 67 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6873, 14), 47}, // 68 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6286, 13), 14}, // 69 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8181, 22), 97}, // 70 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8297, 12), 102}, // 71 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8309, 11), 102}, // 72 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8320, 14), 103}, // 73 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8334, 13), 103}, // 74 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6299, 6), 15}, // 75 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7976, 15), 91}, // 76 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6600, 18), 32}, // 77 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6637, 21), 34}, // 78 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7991, 16), 92}, // 79 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8007, 18), 93}, // 80 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6803, 21), 43}, // 81 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6790, 13), 42}, // 82 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6560, 14), 29}, // 83 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8460, 22), 107}, // 84 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8482, 21), 107}, // 85 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8423, 19), 106}, // 86 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8442, 18), 106}, // 87 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8590, 17), 111}, // 88 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8607, 16), 111}, // 89 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6433, 14), 20}, // 90 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6457, 11), 22}, // 91 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6140, 10), 8}, // 92 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6124, 9), 7}, // 93 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7784, 18), 83}, // 94 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7896, 17), 87}, // 95 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6196, 12), 13}, // 96 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7913, 19), 88}, // 97 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8228, 24), 99}, // 98 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8203, 25), 98}, // 99 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7198, 24), 61}, // 100 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8252, 28), 100}, // 101 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8924, 20), 120}, // 102 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8524, 11), 109}, // 103 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8535, 10), 109}, // 104 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8503, 11), 108}, // 105 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8514, 10), 108}, // 106 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7573, 29), 76}, // 107 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7669, 16), 79}, // 108 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9088, 10), 125}, // 109 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9107, 7), 127}, // 110 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6480, 9), 24}, // 111 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6522, 11), 26}, // 112 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6507, 15), 25}, // 113 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7636, 15), 78}, // 114 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7721, 20), 81}, // 115 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7760, 24), 82}, // 116 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7620, 16), 77}, // 117 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7289, 15), 64}, // 118 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6962, 15), 53}, // 119 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6977, 18), 53}, // 120 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7017, 15), 54}, // 121 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7032, 18), 54}, // 122 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7050, 15), 55}, // 123 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7065, 18), 55}, // 124 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6824, 11), 44}, // 125 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7083, 15), 56}, // 126 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7098, 18), 56}, // 127 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6835, 26), 45}, // 128 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7116, 15), 57}, // 129 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7131, 18), 57}, // 130 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6774, 16), 41}, // 131 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6727, 13), 40}, // 132 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7884, 12), 86}, // 133 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6423, 10), 19}, // 134 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6408, 15), 18}, // 135 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6393, 15), 17}, // 136 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7353, 18), 67}, // 137 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7335, 18), 66}, // 138 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7304, 15), 65}, // 139 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6176, 9), 11}, // 140 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6861, 12), 46}, // 141 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7269, 10), 63}, // 142 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6345, 14), 16}, // 143 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7685, 15), 80}, // 144 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7822, 22), 84}, // 145 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6939, 10), 51}, // 146 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9098, 9), 126}, // 147 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6949, 13), 52}, // 148 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9056, 13), 124}, // 149 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6658, 8), 35}, // 150 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6588, 12), 31}, // 151 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6574, 14), 30}, // 152 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8382, 21), 105}, // 153 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8403, 20), 105}, // 154 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8347, 18), 104}, // 155 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8365, 17), 104}, // 156 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8623, 17), 112}, // 157 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8640, 16), 112}, // 158 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(11725, 9), 277}, // 159 AbortKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6021, 10), 140}, // 160 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15169, 35), 384}, // 161 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15204, 34), 384}, // 162 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14641, 38), 372}, // 163 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14679, 37), 372}, // 164 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14576, 33), 371}, // 165 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14609, 32), 371}, // 166 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15921, 19), 410}, // 167 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14214, 9), 357}, // 168 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15822, 20), 406}, // 169 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14269, 23), 360}, // 170 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13515, 22), 329}, // 171 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15738, 20), 402}, // 172 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14223, 23), 358}, // 173 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15758, 20), 403}, // 174 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14246, 23), 359}, // 175 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9507, 14), 156}, // 176 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10964, 17), 237}, // 177 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15863, 24), 408}, // 178 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11696, 29), 276}, // 179 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11674, 22), 275}, // 180 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11388, 19), 258}, // 181 BFloat16MulAddQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11658, 16), 274}, // 182 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16725, 20), 437}, // 183 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13452, 18), 326}, // 184 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15633, 16), 397}, // 185 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10615, 18), 221}, // 186 BitcastExtractEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15371, 20), 388}, // 187 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15391, 19), 388}, // 188 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16686, 19), 435}, // 189 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6150, 13), 166}, // 190 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13054, 32), 314}, // 191 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13086, 31), 314}, // 192 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11904, 31), 293}, // 193 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11935, 30), 293}, // 194 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11734, 16), 279}, // 195 ConstantDataKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13853, 30), 342}, // 196 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11333, 32), 256}, // 197 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13612, 32), 337}, // 198 CooperativeMatrixConversionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13705, 31), 339}, // 199 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13964, 32), 347}, // 200 CooperativeMatrixDecodeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13911, 34), 345}, // 201 CooperativeMatrixGetCoordinateEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10542, 21), 395}, // 202 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10477, 28), 214}, // 203 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13142, 20), 316}, // 204 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13736, 41), 340}, // 205 CooperativeMatrixPerElementOperationsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13777, 40), 340}, // 206 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13644, 31), 338}, // 207 CooperativeMatrixReductionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13675, 30), 338}, // 208 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13817, 36), 341}, // 209 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13495, 20), 328}, // 210 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13883, 28), 343}, // 211 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6897, 16), 206}, // 212 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6163, 13), 167}, // 213 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15842, 21), 407}, // 214 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13265, 25), 321}, // 215 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13290, 28), 321}, // 216 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11103, 18), 243}, // 217 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11088, 15), 242}, // 218 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9954, 18), 185}, // 219 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7651, 18), 278}, // 220 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9335, 19), 417}, // 221 DeviceBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9478, 14), 154}, // 222 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7257, 12), 233}, // 223 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13318, 23), 322}, // 224 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15584, 11), 393}, // 225 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16811, 27), 440}, // 226 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16778, 33), 439}, // 227 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16745, 33), 438}, // 228 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16838, 32), 441}, // 229 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15478, 22), 391}, // 230 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15500, 25), 391}, // 231 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15525, 28), 392}, // 232 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15553, 31), 392}, // 233 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15437, 19), 390}, // 234 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15456, 22), 390}, // 235 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15595, 14), 393}, // 236 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7160, 15), 224}, // 237 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14326, 16), 363}, // 238 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15689, 7), 400}, // 239 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14556, 20), 370}, // 240 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16129, 29), 416}, // 241 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16158, 28), 416}, // 242 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15120, 25), 383}, // 243 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15145, 24), 383}, // 244 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14888, 28), 378}, // 245 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14916, 27), 378}, // 246 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15940, 30), 411}, // 247 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15970, 29), 411}, // 248 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14972, 21), 380}, // 249 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14993, 20), 380}, // 250 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15039, 41), 382}, // 251 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15080, 40), 382}, // 252 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14813, 26), 376}, // 253 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15999, 28), 412}, // 254 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16080, 25), 415}, // 255 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16105, 24), 415}, // 256 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14746, 23), 374}, // 257 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14769, 22), 374}, // 258 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14839, 25), 377}, // 259 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14864, 24), 377}, // 260 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14503, 27), 369}, // 261 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14530, 26), 369}, // 262 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15410, 14), 389}, // 263 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15424, 13), 389}, // 264 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16064, 16), 414}, // 265 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9392, 8), 145}, // 266 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9378, 14), 144}, // 267 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11498, 16), 263}, // 268 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10573, 10), 218}, // 269 Float4EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9400, 8), 146}, // 270 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10563, 10), 217}, // 271 Float6EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10515, 27), 216}, // 272 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10505, 10), 215}, // 273 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10583, 22), 219}, // 274 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15674, 15), 399}, // 275 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14122, 23), 353}, // 276 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8050, 23), 292}, // 277 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11882, 22), 292}, // 278 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8139, 19), 294}, // 279 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7860, 24), 288}, // 280 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11536, 16), 265}, // 281 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13233, 32), 320}, // 282 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13162, 33), 317}, // 283 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13195, 38), 318}, // 284 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7222, 23), 222}, // 285 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14477, 26), 368}, // 286 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14168, 22), 355}, // 287 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16545, 22), 429}, // 288 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9799, 15), 172}, // 289 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6208, 9), 138}, // 290 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9549, 18), 159}, // 291 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11810, 28), 283}, // 292 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10012, 16), 188}, // 293 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16216, 36), 419}, // 294 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16252, 35), 419}, // 295 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16186, 30), 418}, // 296 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10468, 9), 213}, // 297 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6740, 16), 195}, // 298 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10151, 26), 197}, // 299 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6995, 22), 198}, // 300 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10231, 25), 201}, // 301 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11979, 30), 295}, // 302 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12009, 29), 295}, // 303 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10256, 20), 202}, // 304 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15649, 25), 398}, // 305 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10177, 23), 199}, // 306 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10200, 31), 200}, // 307 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10131, 20), 196}, // 308 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16608, 26), 432}, // 309 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9471, 7), 153}, // 310 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15344, 14), 387}, // 311 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15358, 13), 387}, // 312 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9868, 8), 178}, // 313 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9427, 11), 149}, // 314 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9890, 12), 181}, // 315 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9745, 15), 168}, // 316 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11865, 17), 290}, // 317 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11514, 22), 264}, // 318 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9567, 20), 160}, // 319 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11469, 29), 262}, // 320 ImageGatherExtendedModesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11447, 22), 261}, // 321 ImageGatherLinearQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9902, 13), 182}, // 322 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9453, 12), 151}, // 323 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9943, 11), 184}, // 324 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9438, 15), 150}, // 325 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11552, 21), 267}, // 326 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9777, 10), 170}, // 327 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14190, 24), 356}, // 328 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9819, 16), 174}, // 329 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12124, 36), 298}, // 330 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12160, 39), 298}, // 331 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12665, 39), 305}, // 332 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12704, 42), 305}, // 333 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9521, 6), 157}, // 334 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11631, 27), 273}, // 335 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11617, 14), 272}, // 336 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9408, 6), 147}, // 337 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9414, 13), 148}, // 338 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11573, 14), 268}, // 339 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9814, 5), 173}, // 340 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14145, 23), 354}, // 341 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9972, 22), 186}, // 342 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16870, 17), 442}, // 343 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5975, 7), 142}, // 344 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14791, 22), 375}, // 345 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9361, 8), 141}, // 346 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9492, 15), 155}, // 347 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15778, 20), 404}, // 348 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13575, 14), 335}, // 349 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14943, 15), 379}, // 350 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14958, 14), 379}, // 351 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10605, 10), 220}, // 352 MXInt8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16661, 25), 434}, // 353 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9354, 7), 136}, // 354 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15013, 26), 381}, // 355 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6271, 15), 291}, // 356 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6257, 14), 289}, // 357 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9851, 7), 176}, // 358 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7279, 10), 234}, // 359 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6359, 14), 191}, // 360 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11424, 23), 260}, // 361 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10106, 13), 193}, // 362 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15798, 11), 405}, // 363 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15809, 13), 405}, // 364 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7802, 20), 287}, // 365 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6093, 31), 313}, // 366 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13020, 34), 313}, // 367 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10119, 12), 194}, // 368 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9465, 6), 152}, // 369 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11750, 16), 280}, // 370 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16567, 18), 430}, // 371 PredicatedIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13555, 20), 334}, // 372 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11602, 15), 271}, // 373 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13537, 18), 331}, // 374 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9296, 15), 394}, // 375 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11201, 12), 248}, // 376 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13470, 25), 327}, // 377 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11178, 23), 247}, // 378 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9243, 41), 344}, // 379 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8815, 33), 330}, // 380 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6243, 14), 251}, // 381 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8997, 39), 333}, // 382 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8701, 23), 310}, // 383 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6230, 13), 309}, // 384 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13370, 29), 323}, // 385 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15696, 42), 401}, // 386 RayTracingOpacityMicromapExecutionModeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13341, 29), 323}, // 387 RayTracingOpacityMicromapKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8754, 27), 308}, // 388 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13117, 25), 315}, // 389 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8958, 28), 332}, // 390 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11232, 32), 250}, // 391 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16705, 20), 436}, // 392 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15609, 24), 396}, // 393 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14101, 21), 352}, // 394 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16585, 23), 431}, // 395 RoundedDivideSqrtINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11146, 16), 245}, // 396 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11162, 16), 246}, // 397 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15285, 30), 386}, // 398 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15315, 29), 386}, // 399 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12075, 23), 297}, // 400 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12098, 26), 297}, // 401 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11781, 29), 282}, // 402 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10981, 28), 238}, // 403 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6489, 18), 169}, // 404 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9858, 10), 177}, // 405 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9876, 14), 180}, // 406 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9760, 17), 179}, // 407 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9645, 33), 163}, // 408 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12438, 36), 302}, // 409 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12474, 39), 302}, // 410 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9787, 12), 171}, // 411 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6133, 7), 137}, // 412 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13589, 23), 336}, // 413 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11587, 15), 269}, // 414 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7602, 18), 270}, // 415 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13425, 27), 325}, // 416 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13399, 26), 324}, // 417 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6305, 12), 203}, // 418 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12038, 17), 296}, // 419 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12055, 20), 296}, // 420 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9069, 19), 319}, // 421 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7741, 19), 286}, // 422 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6373, 20), 204}, // 423 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6317, 28), 284}, // 424 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11838, 27), 284}, // 425 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7700, 21), 285}, // 426 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11965, 14), 294}, // 427 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11121, 25), 244}, // 428 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9835, 16), 175}, // 429 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16524, 21), 428}, // 430 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15887, 16), 409}, // 431 SplitBarrierEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15903, 18), 409}, // 432 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7556, 17), 266}, // 433 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10769, 25), 229}, // 434 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11009, 24), 239}, // 435 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9678, 34), 164}, // 436 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12513, 37), 303}, // 437 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12550, 40), 303}, // 438 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9712, 33), 165}, // 439 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12590, 36), 304}, // 440 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12626, 39), 304}, // 441 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9915, 28), 183}, // 442 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9587, 24), 161}, // 443 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10028, 30), 189}, // 444 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10058, 31), 190}, // 445 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10896, 21), 232}, // 446 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10874, 22), 231}, // 447 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11067, 21), 241}, // 448 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10391, 37), 211}, // 449 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10428, 40), 212}, // 450 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12280, 39), 300}, // 451 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12319, 42), 300}, // 452 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12833, 42), 307}, // 453 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12875, 45), 307}, // 454 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10857, 17), 230}, // 455 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10794, 28), 229}, // 456 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16343, 23), 422}, // 457 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16366, 30), 423}, // 458 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16396, 30), 424}, // 459 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14413, 39), 366}, // 460 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14342, 33), 364}, // 461 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14375, 38), 365}, // 462 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6756, 18), 223}, // 463 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16287, 28), 420}, // 464 SubgroupBitcastShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14017, 27), 349}, // 465 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16315, 28), 421}, // 466 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10089, 17), 192}, // 467 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14044, 26), 350}, // 468 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14070, 31), 351}, // 469 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16426, 38), 425}, // 470 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13996, 21), 348}, // 471 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11407, 17), 259}, // 472 SubgroupSizeQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10753, 16), 228}, // 473 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16027, 19), 413}, // 474 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16046, 18), 413}, // 475 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13945, 19), 346}, // 476 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16634, 27), 433}, // 477 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10380, 11), 210}, // 478 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16464, 28), 426}, // 479 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6217, 13), 139}, // 480 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9527, 22), 158}, // 481 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11365, 23), 257}, // 482 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11311, 22), 254}, // 483 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11290, 21), 253}, // 484 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11264, 26), 252}, // 485 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10294, 28), 207}, // 486 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10322, 28), 208}, // 487 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10350, 30), 209}, // 488 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7319, 16), 255}, // 489 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9994, 18), 187}, // 490 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15238, 24), 385}, // 491 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15262, 23), 385}, // 492 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10822, 35), 230}, // 493 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11033, 34), 240}, // 494 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9611, 34), 162}, // 495 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12361, 37), 301}, // 496 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12398, 40), 301}, // 497 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10276, 18), 205}, // 498 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12199, 39), 299}, // 499 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12238, 42), 299}, // 500 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12746, 42), 306}, // 501 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12788, 45), 306}, // 502 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14716, 30), 373}, // 503 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11213, 19), 249}, // 504 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16492, 32), 427}, // 505 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14452, 25), 367}, // 506 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10947, 17), 236}, // 507 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10917, 30), 235}, // 508 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9369, 9), 143}, // 509 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14311, 15), 362}, // 510 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14292, 19), 361}, // 511 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12920, 18), 311}, // 512 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12959, 29), 312}, // 513 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12988, 32), 312}, // 514 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12938, 21), 311}, // 515 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11766, 15), 281}, // 516 WeakLinkageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10709, 44), 227}, // 517 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10666, 43), 226}, // 518 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10633, 33), 225}, // 519 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16899, 8), 444}, // 520 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16907, 8), 445}, // 521 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16915, 6), 446}, // 522 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16921, 7), 447}, // 523 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16928, 11), 448}, // 524 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16939, 9), 449}, // 525 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16948, 13), 450}, // 526 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16887, 12), 443}, // 527 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16961, 6), 451}, // 528 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16967, 10), 452}, // 529 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16977, 6), 453}, // 530 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16998, 20), 455}, // 531 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16983, 15), 454}, // 532 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(17107, 15), 463}, // 533 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17122, 13), 464}, // 534 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17095, 12), 462}, // 535 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17185, 21), 468}, // 536 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17078, 17), 461}, // 537 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17262, 16), 472}, // 538 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17066, 12), 460}, // 539 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17246, 16), 471}, // 540 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17039, 14), 458}, // 541 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17023, 16), 457}, // 542 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17053, 13), 459}, // 543 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17206, 20), 469}, // 544 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17150, 18), 466}, // 545 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17135, 15), 465}, // 546 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17226, 20), 470}, // 547 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17168, 17), 467}, // 548 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17298, 24), 474}, // 549 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17278, 20), 473}, // 550 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17018, 5), 456}, // 551 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17383, 9), 479}, // 552 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17415, 7), 483}, // 553 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17322, 6), 475}, // 554 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17422, 9), 484}, // 555 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17333, 6), 477}, // 556 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17328, 5), 476}, // 557 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17372, 11), 478}, // 558 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17404, 11), 482}, // 559 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17392, 5), 480}, // 560 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17397, 7), 481}, // 561 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17467, 11), 488}, // 562 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17431, 10), 485}, // 563 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17454, 13), 487}, // 564 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17441, 13), 486}, // 565 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17478, 10), 489}, // 566 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17488, 10), 490}, // 567 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17498, 10), 491}, // 568 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17666, 12), 502}, // 569 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17678, 12), 503}, // 570 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17521, 14), 493}, // 571 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17535, 14), 494}, // 572 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17549, 14), 495}, // 573 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17508, 13), 492}, // 574 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17626, 19), 500}, // 575 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17578, 16), 497}, // 576 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17594, 16), 498}, // 577 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17610, 16), 499}, // 578 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17563, 15), 496}, // 579 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17645, 21), 501}, // 580 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17742, 28), 507}, // 581 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17702, 15), 505}, // 582 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17717, 25), 506}, // 583 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17690, 12), 504}, // 584 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17778, 27), 509}, // 585 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17805, 27), 510}, // 586 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17832, 27), 511}, // 587 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17859, 32), 512}, // 588 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17770, 8), 508}, // 589 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17891, 26), 513}, // 590 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17928, 4), 516}, // 591 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17921, 7), 515}, // 592 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17917, 4), 514}, // 593 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17932, 11), 517}, // 594 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17954, 21), 519}, // 595 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17943, 11), 518}, // 596 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17986, 14), 521}, // 597 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(18000, 21), 522}, // 598 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17975, 11), 520}, // 599 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(18021, 18), 523}, // 600 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16899, 8), 525}, // 601 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16907, 8), 526}, // 602 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16915, 6), 527}, // 603 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16921, 7), 528}, // 604 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16928, 11), 529}, // 605 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16939, 9), 530}, // 606 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16948, 13), 531}, // 607 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16887, 12), 524}, // 608 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16961, 6), 532}, // 609 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16967, 10), 533}, // 610 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16977, 6), 534}, // 611 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(17107, 15), 542}, // 612 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17122, 13), 543}, // 613 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17095, 12), 541}, // 614 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17185, 21), 547}, // 615 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17078, 17), 540}, // 616 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17066, 12), 539}, // 617 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17246, 16), 550}, // 618 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17039, 14), 537}, // 619 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17023, 16), 536}, // 620 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17053, 13), 538}, // 621 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17206, 20), 548}, // 622 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17150, 18), 545}, // 623 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17135, 15), 544}, // 624 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17226, 20), 549}, // 625 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17168, 17), 546}, // 626 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17018, 5), 535}, // 627 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17383, 9), 555}, // 628 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17415, 7), 559}, // 629 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17322, 6), 551}, // 630 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17333, 6), 553}, // 631 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17328, 5), 552}, // 632 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17372, 11), 554}, // 633 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17404, 11), 558}, // 634 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17392, 5), 556}, // 635 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17397, 7), 557}, // 636 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17431, 10), 560}, // 637 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17454, 13), 562}, // 638 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17441, 13), 561}, // 639 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(20789, 16), 684}, // 640 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18248, 8), 582}, // 641 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(19409, 15), 638}, // 642 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19424, 18), 638}, // 643 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18726, 10), 606}, // 644 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(18770, 12), 608}, // 645 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(18099, 12), 569}, // 646 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(19150, 17), 625}, // 647 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(20339, 15), 672}, // 648 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20354, 14), 672}, // 649 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19518, 7), 642}, // 650 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20052, 16), 665}, // 651 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20068, 15), 665}, // 652 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18403, 8), 595}, // 653 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(19543, 16), 644}, // 654 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19525, 18), 643}, // 655 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18063, 6), 565}, // 656 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(18910, 22), 615}, // 657 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18888, 22), 614}, // 658 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19574, 13), 646}, // 659 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19559, 15), 645}, // 660 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18069, 12), 566}, // 661 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(20949, 21), 689}, // 662 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20970, 20), 689}, // 663 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18180, 8), 574}, // 664 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(20508, 20), 677}, // 665 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20528, 19), 677}, // 666 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18146, 8), 573}, // 667 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(22196, 22), 712}, // 668 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(22255, 23), 713}, // 669 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20547, 16), 678}, // 670 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20563, 15), 678}, // 671 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18213, 9), 578}, // 672 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(19657, 13), 650}, // 673 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18274, 9), 585}, // 674 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(18090, 9), 568}, // 675 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(18387, 10), 593}, // 676 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(22143, 17), 711}, // 677 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21372, 28), 699}, // 678 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21400, 27), 699}, // 679 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18265, 9), 584}, // 680 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(19587, 30), 647}, // 681 CooperativeMatrixTransposeEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19803, 14), 657}, // 682 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18411, 14), 596}, // 683 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(20578, 29), 679}, // 684 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20607, 28), 679}, // 685 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20161, 17), 668}, // 686 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20178, 16), 668}, // 687 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18932, 18), 616}, // 688 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(18596, 15), 602}, // 689 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(21234, 26), 696}, // 690 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18546, 15), 601}, // 691 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(18202, 5), 576}, // 692 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(20368, 21), 673}, // 693 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20389, 20), 673}, // 694 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18498, 14), 600}, // 695 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(19714, 21), 653}, // 696 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19945, 24), 661}, // 697 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21063, 31), 691}, // 698 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19888, 26), 660}, // 699 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20699, 26), 682}, // 700 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20725, 25), 682}, // 701 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18135, 11), 572}, // 702 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18124, 11), 571}, // 703 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(19777, 26), 656}, // 704 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19487, 31), 641}, // 705 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19457, 30), 640}, // 706 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19817, 24), 657}, // 707 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(19854, 19), 658}, // 708 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21994, 16), 708}, // 709 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20990, 20), 690}, // 710 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21010, 19), 690}, // 711 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(22086, 29), 710}, // 712 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(22115, 28), 710}, // 713 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18397, 6), 594}, // 714 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(22057, 15), 709}, // 715 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(22072, 14), 709}, // 716 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20818, 25), 686}, // 717 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20843, 24), 686}, // 718 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18705, 21), 605}, // 719 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(16870, 17), 714}, // 720 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_DECORATION
  {IR(18229, 10), 580}, // 721 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(21311, 31), 698}, // 722 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21342, 30), 698}, // 723 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21260, 26), 697}, // 724 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21286, 25), 697}, // 725 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18673, 18), 603}, // 726 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(18378, 9), 592}, // 727 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(21490, 34), 701}, // 728 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21524, 33), 701}, // 729 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21557, 31), 702}, // 730 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21588, 30), 702}, // 731 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21618, 29), 703}, // 732 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21647, 28), 703}, // 733 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21778, 30), 705}, // 734 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21808, 29), 705}, // 735 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21709, 35), 704}, // 736 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21744, 34), 704}, // 737 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21837, 33), 706}, // 738 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21870, 32), 706}, // 739 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20750, 20), 683}, // 740 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20770, 19), 683}, // 741 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18111, 13), 570}, // 742 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18736, 14), 607}, // 743 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(18782, 16), 609}, // 744 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(20867, 21), 687}, // 745 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20888, 20), 687}, // 746 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20083, 23), 666}, // 747 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20106, 22), 666}, // 748 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20194, 20), 669}, // 749 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20214, 19), 669}, // 750 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21154, 18), 694}, // 751 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19442, 15), 639}, // 752 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19998, 13), 663}, // 753 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20011, 12), 663}, // 754 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20274, 12), 671}, // 755 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20286, 11), 671}, // 756 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20805, 13), 685}, // 757 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18691, 14), 604}, // 758 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(18188, 14), 575}, // 759 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(18842, 13), 611}, // 760 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18855, 15), 612}, // 761 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18982, 20), 618}, // 762 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18950, 32), 617}, // 763 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18295, 12), 587}, // 764 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(19349, 11), 636}, // 765 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(19360, 14), 636}, // 766 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18283, 12), 586}, // 767 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(20023, 15), 664}, // 768 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20038, 14), 664}, // 769 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18425, 7), 597}, // 770 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(19167, 12), 626}, // 771 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19193, 19), 628}, // 772 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19212, 14), 629}, // 773 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18207, 6), 577}, // 774 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(19122, 28), 624}, // 775 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19097, 25), 623}, // 776 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19045, 25), 621}, // 777 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19025, 20), 620}, // 778 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19070, 27), 622}, // 779 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19273, 16), 632}, // 780 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19289, 15), 632}, // 781 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19314, 10), 634}, // 782 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19324, 13), 635}, // 783 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(19337, 12), 635}, // 784 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19304, 10), 633}, // 785 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20908, 21), 688}, // 786 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20929, 20), 688}, // 787 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20635, 15), 680}, // 788 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20650, 14), 680}, // 789 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19631, 26), 649}, // 790 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19969, 15), 662}, // 791 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19984, 14), 662}, // 792 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21427, 32), 700}, // 793 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21459, 31), 700}, // 794 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18039, 17), 563}, // 795 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(18239, 9), 581}, // 796 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(19374, 16), 637}, // 797 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19390, 19), 637}, // 798 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18081, 9), 567}, // 799 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(19617, 14), 648}, // 800 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18222, 7), 579}, // 801 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(18351, 20), 590}, // 802 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(18798, 44), 610}, // 803 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19245, 28), 631}, // 804 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19670, 17), 651}, // 805 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20233, 21), 670}, // 806 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20254, 20), 670}, // 807 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21094, 25), 692}, // 808 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20128, 17), 667}, // 809 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20145, 16), 667}, // 810 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18056, 7), 564}, // 811 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(21902, 27), 707}, // 812 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21929, 26), 707}, // 813 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19762, 15), 655}, // 814 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20664, 18), 681}, // 815 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20682, 17), 681}, // 816 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21172, 16), 695}, // 817 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21188, 15), 695}, // 818 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18371, 7), 591}, // 819 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(20409, 17), 674}, // 820 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20426, 16), 674}, // 821 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19002, 23), 619}, // 822 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(20471, 19), 676}, // 823 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20490, 18), 676}, // 824 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19179, 14), 627}, // 825 UTFEncodedKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18307, 8), 588}, // 826 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18341, 10), 589}, // 827 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(19841, 13), 658}, // 828 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(19873, 15), 659}, // 829 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21119, 35), 693}, // 830 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19735, 27), 654}, // 831 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19687, 27), 652}, // 832 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19226, 19), 630}, // 833 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18256, 9), 583}, // 834 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(18870, 18), 613}, // 835 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(20442, 15), 675}, // 836 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20457, 14), 675}, // 837 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18432, 10), 598}, // 838 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18442, 10), 599}, // 839 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(22278, 3), 715}, // 840 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22281, 3), 716}, // 841 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22284, 3), 717}, // 842 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22297, 7), 720}, // 843 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22287, 5), 718}, // 844 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22292, 5), 719}, // 845 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22304, 12), 721}, // 846 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22316, 17), 722}, // 847 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(23476, 20), 792}, // 848 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23170, 15), 777}, // 849 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22759, 15), 753}, // 850 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11103, 18), 767}, // 851 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11088, 15), 766}, // 852 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22529, 13), 736}, // 853 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22542, 10), 737}, // 854 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22514, 15), 735}, // 855 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22552, 15), 738}, // 856 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23611, 25), 796}, // 857 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23636, 24), 796}, // 858 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23564, 24), 795}, // 859 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23588, 23), 795}, // 860 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23118, 29), 775}, // 861 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22481, 19), 732}, // 862 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24121, 18), 816}, // 863 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22786, 10), 755}, // 864 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23936, 26), 808}, // 865 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23962, 27), 809}, // 866 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22774, 12), 754}, // 867 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22603, 11), 742}, // 868 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22614, 20), 743}, // 869 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22591, 12), 741}, // 870 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22644, 24), 745}, // 871 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22333, 12), 723}, // 872 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23185, 15), 778}, // 873 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22674, 9), 747}, // 874 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22567, 10), 739}, // 875 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22577, 14), 740}, // 876 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22854, 16), 760}, // 877 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22842, 12), 759}, // 878 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23200, 21), 779}, // 879 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23245, 21), 782}, // 880 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24011, 16), 811}, // 881 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23989, 22), 810}, // 882 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24097, 24), 815}, // 883 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24233, 22), 821}, // 884 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24255, 24), 822}, // 885 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24210, 23), 820}, // 886 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24330, 27), 823}, // 887 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24027, 20), 812}, // 888 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22870, 34), 761}, // 889 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22904, 34), 762}, // 890 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22938, 36), 763}, // 891 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23022, 34), 771}, // 892 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24047, 22), 813}, // 893 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24139, 21), 817}, // 894 OpacityMicromapIdKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22465, 16), 731}, // 895 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22449, 16), 730}, // 896 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22711, 16), 750}, // 897 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23496, 15), 793}, // 898 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23511, 14), 793}, // 899 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22698, 13), 749}, // 900 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23525, 20), 794}, // 901 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23545, 19), 794}, // 902 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22727, 20), 751}, // 903 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23660, 19), 797}, // 904 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23679, 18), 797}, // 905 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22683, 15), 748}, // 906 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22430, 19), 729}, // 907 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23697, 25), 798}, // 908 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23722, 27), 799}, // 909 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22500, 10), 733}, // 910 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23004, 18), 765}, // 911 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23417, 19), 789}, // 912 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22668, 6), 746}, // 913 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24184, 26), 819}, // 914 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23436, 20), 790}, // 915 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11146, 16), 769}, // 916 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23915, 21), 807}, // 917 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23894, 21), 806}, // 918 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11162, 16), 770}, // 919 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23749, 26), 800}, // 920 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23775, 28), 801}, // 921 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24069, 28), 814}, // 922 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(13589, 23), 804}, // 923 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7620, 16), 781}, // 924 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23803, 31), 802}, // 925 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23834, 33), 803}, // 926 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23867, 27), 805}, // 927 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23456, 20), 791}, // 928 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11121, 25), 768}, // 929 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22345, 13), 724}, // 930 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22358, 22), 725}, // 931 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22380, 21), 726}, // 932 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23221, 24), 780}, // 933 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23370, 25), 787}, // 934 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23294, 26), 784}, // 935 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23395, 22), 788}, // 936 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23320, 23), 785}, // 937 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23147, 23), 776}, // 938 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23343, 27), 786}, // 939 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23266, 28), 783}, // 940 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24160, 24), 818}, // 941 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6727, 13), 756}, // 942 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23097, 21), 774}, // 943 SubgroupSizeFullQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23076, 21), 773}, // 944 SubgroupSizeHalfQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22974, 30), 764}, // 945 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22796, 22), 757}, // 946 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22818, 24), 758}, // 947 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23056, 20), 772}, // 948 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22634, 10), 744}, // 949 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22747, 12), 752}, // 950 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22415, 15), 728}, // 951 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22401, 14), 727}, // 952 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22510, 4), 734}, // 953 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24495, 10), 835}, // 954 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24505, 9), 835}, // 955 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24556, 12), 838}, // 956 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24568, 11), 838}, // 957 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24514, 14), 836}, // 958 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24528, 13), 836}, // 959 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(17422, 9), 828}, // 960 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24407, 10), 829}, // 961 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(6208, 9), 827}, // 962 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24464, 16), 834}, // 963 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24480, 15), 834}, // 964 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5975, 7), 830}, // 965 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24587, 8), 840}, // 966 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24424, 7), 832}, // 967 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24541, 8), 837}, // 968 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24549, 7), 837}, // 969 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24431, 17), 833}, // 970 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24448, 16), 833}, // 971 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24579, 8), 839}, // 972 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24417, 7), 831}, // 973 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24364, 20), 825}, // 974 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24384, 23), 826}, // 975 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24357, 7), 824}, // 976 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24604, 12), 842}, // 977 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24595, 9), 841}, // 978 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24616, 12), 843}, // 979 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(24684, 14), 848}, // 980 Float4E2M1EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24656, 14), 846}, // 981 Float6E2M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24670, 14), 847}, // 982 Float6E3M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24628, 14), 844}, // 983 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24642, 14), 845}, // 984 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10583, 22), 849}, // 985 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10605, 10), 850}, // 986 MXInt8EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24703, 4), 852}, // 987 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24698, 5), 851}, // 988 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24741, 14), 859}, // 989 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24755, 23), 859}, // 990 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24778, 13), 860}, // 991 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24791, 18), 860}, // 992 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24725, 11), 857}, // 993 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24809, 15), 861}, // 994 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24736, 5), 858}, // 995 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24721, 4), 856}, // 996 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(17018, 5), 853}, // 997 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24714, 7), 855}, // 998 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24707, 7), 854}, // 999 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24824, 4), 862}, // 1000 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24836, 4), 865}, // 1001 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24832, 4), 864}, // 1002 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24828, 4), 863}, // 1003 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24872, 18), 868}, // 1004 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24890, 18), 869}, // 1005 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24840, 16), 866}, // 1006 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24856, 16), 867}, // 1007 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24931, 6), 874}, // 1008 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24915, 11), 872}, // 1009 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24908, 7), 871}, // 1010 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(17018, 5), 870}, // 1011 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15798, 11), 875}, // 1012 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15809, 13), 875}, // 1013 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24926, 5), 873}, // 1014 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24947, 6), 878}, // 1015 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24958, 8), 880}, // 1016 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24966, 10), 881}, // 1017 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24984, 12), 883}, // 1018 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24976, 8), 882}, // 1019 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24996, 21), 884}, // 1020 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(25017, 20), 884}, // 1021 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24942, 5), 877}, // 1022 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24953, 5), 879}, // 1023 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24937, 5), 876}, // 1024 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(25037, 14), 885}, // 1025 Gather4x1QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25051, 12), 886}, // 1026 GatherDQCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25063, 13), 887}, // 1027 GatherH2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25076, 13), 888}, // 1028 GatherV2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25124, 16), 892}, // 1029 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25110, 14), 891}, // 1030 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25096, 14), 890}, // 1031 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25236, 28), 895}, // 1032 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25264, 27), 895}, // 1033 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25181, 28), 894}, // 1034 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25209, 27), 894}, // 1035 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25140, 21), 893}, // 1036 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25161, 20), 893}, // 1037 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25089, 7), 889}, // 1038 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25291, 10), 896}, // 1039 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25301, 10), 897}, // 1040 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25322, 15), 899}, // 1041 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25311, 11), 898}, // 1042 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25337, 5), 901}, // 1043 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(25351, 12), 904}, // 1044 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(25363, 13), 906}, // 1045 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(25346, 5), 903}, // 1046 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(25342, 4), 902}, // 1047 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(25376, 19), 909}, // 1048 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(25395, 22), 909}, // 1049 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25417, 17), 910}, // 1050 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(25434, 20), 910}, // 1051 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9851, 7), 908}, // 1052 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(25454, 16), 911}, // 1053 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25470, 19), 911}, // 1054 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(17018, 5), 900}, // 1055 None in SPV_OPERAND_TYPE_IMAGE
  {IR(25542, 12), 915}, // 1056 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(18425, 7), 905}, // 1057 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(25554, 8), 916}, // 1058 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(18222, 7), 907}, // 1059 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(25520, 11), 913}, // 1060 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(25489, 14), 912}, // 1061 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25503, 17), 912}, // 1062 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25531, 11), 914}, // 1063 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(16915, 6), 931}, // 1064 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25723, 10), 930}, // 1065 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25658, 12), 925}, // 1066 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25670, 12), 926}, // 1067 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25647, 11), 924}, // 1068 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25572, 11), 918}, // 1069 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25562, 10), 917}, // 1070 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25632, 15), 923}, // 1071 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25744, 17), 933}, // 1072 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25761, 16), 934}, // 1073 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25894, 16), 941}, // 1074 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25910, 16), 942}, // 1075 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25593, 11), 920}, // 1076 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25733, 11), 932}, // 1077 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25817, 20), 937}, // 1078 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25583, 10), 919}, // 1079 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25618, 14), 922}, // 1080 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25604, 14), 921}, // 1081 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25837, 19), 938}, // 1082 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25856, 19), 939}, // 1083 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25875, 19), 940}, // 1084 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25695, 14), 928}, // 1085 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25709, 14), 929}, // 1086 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25682, 13), 927}, // 1087 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25777, 20), 935}, // 1088 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25797, 20), 936}, // 1089 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25928, 2), 944}, // 1090 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26029, 5), 962}, // 1091 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25950, 5), 950}, // 1092 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25945, 5), 949}, // 1093 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25987, 6), 956}, // 1094 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25993, 13), 957}, // 1095 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25955, 10), 951}, // 1096 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25965, 10), 952}, // 1097 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25926, 2), 943}, // 1098 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25933, 3), 946}, // 1099 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25930, 3), 945}, // 1100 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25936, 4), 947}, // 1101 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25940, 5), 948}, // 1102 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25982, 5), 955}, // 1103 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25978, 4), 954}, // 1104 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25975, 3), 953}, // 1105 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26023, 6), 961}, // 1106 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26006, 5), 958}, // 1107 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26017, 6), 960}, // 1108 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26011, 6), 959}, // 1109 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(26034, 28), 963}, // 1110 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26062, 27), 963}, // 1111 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26089, 24), 964}, // 1112 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26113, 23), 964}, // 1113 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26136, 7), 965}, // 1114 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26143, 11), 966}, // 1115 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26154, 14), 967}, // 1116 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26168, 12), 969}, // 1117 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(17018, 5), 968}, // 1118 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(26180, 13), 971}, // 1119 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(17018, 5), 970}, // 1120 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(26193, 7), 972}, // 1121 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26200, 7), 973}, // 1122 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26207, 12), 974}, // 1123 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26219, 8), 975}, // 1124 WeakAMD in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26241, 12), 977}, // 1125 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26293, 17), 980}, // 1126 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26268, 25), 979}, // 1127 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26253, 15), 978}, // 1128 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26227, 14), 976}, // 1129 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26433, 22), 993}, // 1130 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26455, 21), 993}, // 1131 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26328, 19), 984}, // 1132 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26347, 17), 985}, // 1133 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26317, 11), 983}, // 1134 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20818, 25), 991}, // 1135 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20843, 24), 991}, // 1136 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26392, 18), 988}, // 1137 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26476, 19), 995}, // 1138 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26495, 18), 995}, // 1139 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26638, 16), 999}, // 1140 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26654, 15), 999}, // 1141 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20867, 21), 992}, // 1142 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20888, 20), 992}, // 1143 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26513, 22), 996}, // 1144 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26535, 21), 996}, // 1145 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26378, 14), 987}, // 1146 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26669, 27), 1000}, // 1147 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26696, 26), 1000}, // 1148 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26364, 14), 986}, // 1149 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(11424, 23), 1001}, // 1150 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26609, 15), 998}, // 1151 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26624, 14), 998}, // 1152 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17018, 5), 981}, // 1153 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26420, 13), 990}, // 1154 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26410, 10), 989}, // 1155 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20908, 21), 994}, // 1156 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20929, 20), 994}, // 1157 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26556, 27), 997}, // 1158 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26583, 26), 997}, // 1159 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26310, 7), 982}, // 1160 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(27005, 27), 1015}, // 1161 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26953, 26), 1013}, // 1162 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26873, 23), 1009}, // 1163 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26827, 23), 1007}, // 1164 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26722, 29), 1003}, // 1165 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26919, 17), 1011}, // 1166 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(27032, 27), 1016}, // 1167 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26979, 26), 1014}, // 1168 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26896, 23), 1010}, // 1169 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26850, 23), 1008}, // 1170 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26751, 29), 1004}, // 1171 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26936, 17), 1012}, // 1172 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26780, 21), 1005}, // 1173 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26801, 26), 1006}, // 1174 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(17018, 5), 1002}, // 1175 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(27192, 20), 1024}, // 1176 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27059, 8), 1019}, // 1177 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27067, 21), 1021}, // 1178 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27088, 24), 1021}, // 1179 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27112, 19), 1022}, // 1180 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27131, 22), 1022}, // 1181 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27212, 17), 1025}, // 1182 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27153, 18), 1023}, // 1183 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27171, 21), 1023}, // 1184 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(17018, 5), 1017}, // 1185 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25542, 12), 1020}, // 1186 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(18256, 9), 1018}, // 1187 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27236, 8), 1027}, // 1188 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27244, 7), 1028}, // 1189 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27229, 7), 1026}, // 1190 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27251, 7), 1029}, // 1191 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27258, 10), 1029}, // 1192 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27276, 8), 1031}, // 1193 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27292, 15), 1033}, // 1194 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27396, 20), 1039}, // 1195 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27375, 21), 1038}, // 1196 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27416, 12), 1040}, // 1197 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27457, 14), 1042}, // 1198 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27471, 17), 1042}, // 1199 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27488, 12), 1043}, // 1200 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27500, 15), 1043}, // 1201 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(17018, 5), 1030}, // 1202 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27428, 13), 1041}, // 1203 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27441, 16), 1041}, // 1204 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27268, 8), 1030}, // 1205 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27284, 8), 1032}, // 1206 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27307, 23), 1034}, // 1207 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27344, 15), 1036}, // 1208 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27330, 14), 1035}, // 1209 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(18256, 9), 1044}, // 1210 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27359, 16), 1037}, // 1211 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27515, 10), 1045}, // 1212 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(27530, 4), 1047}, // 1213 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27543, 8), 1049}, // 1214 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27534, 9), 1048}, // 1215 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27525, 5), 1046}, // 1216 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27551, 25), 1050}, // 1217 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27576, 28), 1050}, // 1218 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27617, 4), 1053}, // 1219 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27650, 9), 1057}, // 1220 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27659, 13), 1058}, // 1221 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27630, 8), 1055}, // 1222 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27638, 12), 1056}, // 1223 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27621, 9), 1054}, // 1224 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27604, 4), 1051}, // 1225 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27608, 9), 1052}, // 1226 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(17018, 5), 1059}, // 1227 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27672, 25), 1060}, // 1228 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27697, 23), 1061}, // 1229 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27789, 27), 1067}, // 1230 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27816, 28), 1068}, // 1231 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27858, 16), 1070}, // 1232 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27844, 14), 1069}, // 1233 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27958, 30), 1073}, // 1234 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27928, 30), 1073}, // 1235 ForceOpacityMicromap2StateKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27730, 12), 1064}, // 1236 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(17770, 8), 1062}, // 1237 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27720, 10), 1063}, // 1238 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27915, 13), 1072}, // 1239 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27891, 24), 1071}, // 1240 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27765, 24), 1066}, // 1241 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27874, 17), 1071}, // 1242 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27742, 23), 1065}, // 1243 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(28029, 37), 1075}, // 1244 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27988, 41), 1074}, // 1245 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(28144, 42), 1078}, // 1246 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28066, 37), 1076}, // 1247 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28103, 41), 1077}, // 1248 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28186, 33), 1079}, // 1249 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(28219, 33), 1080}, // 1250 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(28264, 6), 1083}, // 1251 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28252, 12), 1082}, // 1252 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(17018, 5), 1081}, // 1253 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28270, 7), 1084}, // 1254 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28277, 15), 1085}, // 1255 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28300, 7), 1087}, // 1256 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28292, 8), 1086}, // 1257 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28365, 13), 1096}, // 1258 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28407, 4), 1102}, // 1259 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28445, 9), 1107}, // 1260 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28378, 5), 1097}, // 1261 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28507, 5), 1116}, // 1262 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28578, 6), 1126}, // 1263 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28331, 5), 1091}, // 1264 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28485, 5), 1112}, // 1265 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28542, 6), 1121}, // 1266 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28595, 5), 1129}, // 1267 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28589, 6), 1128}, // 1268 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28411, 3), 1103}, // 1269 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28454, 8), 1108}, // 1270 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28512, 4), 1117}, // 1271 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28584, 5), 1127}, // 1272 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28398, 5), 1100}, // 1273 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28426, 10), 1105}, // 1274 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28359, 6), 1095}, // 1275 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28496, 6), 1114}, // 1276 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28565, 7), 1124}, // 1277 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28353, 6), 1094}, // 1278 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28490, 6), 1113}, // 1279 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28558, 7), 1123}, // 1280 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28403, 4), 1101}, // 1281 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28436, 9), 1106}, // 1282 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28502, 5), 1115}, // 1283 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28572, 6), 1125}, // 1284 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28390, 8), 1099}, // 1285 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28548, 10), 1122}, // 1286 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28383, 7), 1098}, // 1287 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28414, 12), 1104}, // 1288 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28323, 8), 1090}, // 1289 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28470, 8), 1110}, // 1290 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28525, 9), 1119}, // 1291 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28315, 8), 1089}, // 1292 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28462, 8), 1109}, // 1293 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28516, 9), 1118}, // 1294 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28336, 6), 1092}, // 1295 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28342, 11), 1093}, // 1296 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28478, 7), 1111}, // 1297 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28534, 8), 1120}, // 1298 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28307, 8), 1088}, // 1299 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28600, 12), 1130}, // 1300 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28612, 7), 1131}, // 1301 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28638, 11), 1134}, // 1302 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28649, 12), 1135}, // 1303 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28661, 15), 1135}, // 1304 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28676, 14), 1136}, // 1305 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28629, 9), 1133}, // 1306 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28619, 10), 1132}, // 1307 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28698, 12), 1139}, // 1308 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28690, 8), 1138}, // 1309 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(17018, 5), 1137}, // 1310 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28710, 29), 1140}, // 1311 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(16899, 8), 1142}, // 1312 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16907, 8), 1143}, // 1313 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16915, 6), 1144}, // 1314 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16921, 7), 1145}, // 1315 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16928, 11), 1146}, // 1316 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16939, 9), 1147}, // 1317 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16948, 13), 1148}, // 1318 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16887, 12), 1141}, // 1319 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16961, 6), 1149}, // 1320 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16967, 10), 1150}, // 1321 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16977, 6), 1151}, // 1322 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16998, 20), 1153}, // 1323 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16983, 15), 1152}, // 1324 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(17107, 15), 1161}, // 1325 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17122, 13), 1162}, // 1326 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17095, 12), 1160}, // 1327 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17185, 21), 1166}, // 1328 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17078, 17), 1159}, // 1329 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17262, 16), 1170}, // 1330 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17066, 12), 1158}, // 1331 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17246, 16), 1169}, // 1332 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17039, 14), 1156}, // 1333 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17023, 16), 1155}, // 1334 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17053, 13), 1157}, // 1335 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17206, 20), 1167}, // 1336 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17150, 18), 1164}, // 1337 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17135, 15), 1163}, // 1338 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17226, 20), 1168}, // 1339 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17168, 17), 1165}, // 1340 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17298, 24), 1172}, // 1341 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17278, 20), 1171}, // 1342 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(28739, 26), 1173}, // 1343 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17018, 5), 1154}, // 1344 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17383, 9), 1178}, // 1345 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17415, 7), 1182}, // 1346 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17322, 6), 1174}, // 1347 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17422, 9), 1183}, // 1348 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17333, 6), 1176}, // 1349 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17328, 5), 1175}, // 1350 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17372, 11), 1177}, // 1351 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17404, 11), 1181}, // 1352 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17392, 5), 1179}, // 1353 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17397, 7), 1180}, // 1354 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17467, 11), 1187}, // 1355 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17431, 10), 1184}, // 1356 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17454, 13), 1186}, // 1357 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17441, 13), 1185}, // 1358 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(28857, 9), 1203}, // 1359 ApilaJai in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28800, 15), 1194}, // 1360 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28765, 5), 1189}, // 1361 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28770, 5), 1190}, // 1362 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28820, 7), 1196}, // 1363 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28795, 5), 1193}, // 1364 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28827, 5), 1197}, // 1365 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28775, 9), 1191}, // 1366 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28784, 11), 1192}, // 1367 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28852, 5), 1202}, // 1368 Pred in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28847, 5), 1201}, // 1369 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28815, 5), 1195}, // 1370 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28837, 6), 1199}, // 1371 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28307, 8), 1188}, // 1372 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28832, 5), 1198}, // 1373 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28843, 4), 1200}, // 1374 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28948, 14), 1214}, // 1375 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29030, 16), 1220}, // 1376 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29046, 15), 1220}, // 1377 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29366, 17), 1230}, // 1378 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28895, 15), 1209}, // 1379 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29383, 17), 1231}, // 1380 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29400, 16), 1231}, // 1381 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28918, 9), 1211}, // 1382 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28927, 8), 1212}, // 1383 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29135, 16), 1223}, // 1384 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29151, 15), 1223}, // 1385 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29344, 22), 1229}, // 1386 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29299, 21), 1227}, // 1387 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29416, 15), 1232}, // 1388 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29431, 14), 1232}, // 1389 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28962, 6), 1215}, // 1390 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29061, 24), 1221}, // 1391 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29085, 23), 1221}, // 1392 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29166, 22), 1224}, // 1393 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29188, 21), 1224}, // 1394 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28882, 6), 1205}, // 1395 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29014, 16), 1219}, // 1396 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28888, 7), 1207}, // 1397 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29252, 22), 1226}, // 1398 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29274, 25), 1226}, // 1399 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28910, 8), 1210}, // 1400 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28935, 13), 1213}, // 1401 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29108, 14), 1222}, // 1402 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29122, 13), 1222}, // 1403 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29209, 22), 1225}, // 1404 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29231, 21), 1225}, // 1405 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28968, 14), 1216}, // 1406 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29320, 24), 1228}, // 1407 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28995, 19), 1218}, // 1408 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28982, 13), 1217}, // 1409 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(18307, 8), 1206}, // 1410 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28866, 16), 1204}, // 1411 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28619, 10), 1208}, // 1412 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26253, 15), 1236}, // 1413 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(26227, 14), 1233}, // 1414 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29463, 15), 1235}, // 1415 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29445, 18), 1234}, // 1416 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29489, 11), 1239}, // 1417 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29500, 17), 1240}, // 1418 DecodeVectorFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(17018, 5), 1237}, // 1419 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29478, 11), 1238}, // 1420 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(28252, 12), 1243}, // 1421 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(18265, 9), 1242}, // 1422 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28270, 7), 1244}, // 1423 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28277, 15), 1245}, // 1424 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29517, 10), 1241}, // 1425 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29570, 24), 1249}, // 1426 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29594, 22), 1250}, // 1427 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29616, 21), 1251}, // 1428 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29527, 8), 1246}, // 1429 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29535, 15), 1247}, // 1430 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29550, 20), 1248}, // 1431 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 151);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(159, 361);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(520, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(528, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(531, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(533, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(552, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(562, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(566, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(581, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(585, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(591, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(594, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(597, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(601, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(609, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(612, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(628, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(637, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(640, 200);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(840, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(848, 106);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(954, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(977, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(979, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(987, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(989, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(1000, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(1004, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(1008, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(1015, 10);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(1025, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(1029, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(1039, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1043, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1064, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1090, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1110, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1114, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1117, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1119, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1121, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1125, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1130, 31);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1161, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1176, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1188, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1193, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1212, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1213, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1217, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1219, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1227, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1230, 14);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1244, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1246, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1249, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1251, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1256, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1258, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1300, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1308, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1311, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1312, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1320, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1323, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1325, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1345, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1355, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1359, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1375, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1413, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1417, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1421, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1426, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1043, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1176, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1217, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(585, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1161, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1227, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(979, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1426, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(159, 361);
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
static const std::array<InstructionDesc, 878> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(29637, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(29693, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(29699, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(29818, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(29825, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(29841, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(29846, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(29857, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(29864, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(29869, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(29879, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(29939, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(30011, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(30085, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(30128, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(30170, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(30181, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(30190, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(30199, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(30244, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(30254, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(30265, 11), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(30389, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(30399, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(30411, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(30428, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(30438, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(30455, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(30466, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(30508, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(30520, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(30533, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(30543, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(30559, 14), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(30573, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(30583, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(30592, 19), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(30611, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(30624, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(18265, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(30676, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(30772, 16), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(30788, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(30801, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(30818, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(30836, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(30849, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(30912, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(28918, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(30961, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(30979, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(30991, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(31004, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(31013, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(31071, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(31076, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(31082, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(31093, 16), IR(0, 0), IR(226, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(31109, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(31121, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(31141, 15), IR(0, 0), IR(228, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(31156, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(31168, 23), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(31191, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(31242, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(31251, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(31266, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(31282, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(31341, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(31361, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(31382, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(31402, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(31416, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(31435, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(31452, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(31468, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(31479, 10), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(31489, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(31534, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(31580, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(31603, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(31630, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(31657, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(31684, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(31711, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(31742, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(31773, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(31784, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(31796, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(31812, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(31822, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(28962, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(31833, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(31850, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(31866, 18), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(31884, 15), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(31899, 14), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(31913, 17), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(31930, 18), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(31948, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(31960, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(31972, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(31984, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(31996, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(32005, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(32014, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(32023, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(32037, 14), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(32051, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(32066, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(32081, 14), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(32095, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(32112, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(32129, 25), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(32154, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(32162, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(32170, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(32178, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(32183, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(32188, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(32193, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(32198, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(32203, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(32208, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(32213, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(32218, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(32223, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(32228, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(32233, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(32238, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(32243, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(32248, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(32266, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(32284, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(32302, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(32320, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(32338, 13), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(32351, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(32355, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(32365, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(32376, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(32389, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(32402, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(32406, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(32410, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(32416, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(32422, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(32431, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(32440, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(32451, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(32465, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(32473, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(32483, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(32496, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(32512, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(32522, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(32533, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(32544, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(32551, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(32558, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(32568, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(32581, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(32594, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(32612, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(32630, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(32640, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(32650, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(32665, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(32680, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(32690, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(32702, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(32715, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(32730, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(32743, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(32758, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(32774, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(32792, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(32810, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(32830, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(32851, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(32874, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(32892, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(32913, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(32930, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(32940, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(32951, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(32962, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(32966, 15), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(32981, 17), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(32998, 17), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(33015, 11), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(33026, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(33035, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(33040, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(33045, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(33052, 9), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(33061, 9), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(33070, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(33081, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(33092, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(33103, 13), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(33116, 11), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(33127, 13), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(33140, 17), IR(0, 0), IR(124, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(33157, 19), IR(0, 0), IR(124, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(33213, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(33228, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(33242, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(33253, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(33265, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(33280, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(33302, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(33328, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(33345, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(33362, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(33373, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(33384, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(33395, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(33406, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(33417, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(33428, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(33438, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(33447, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(33457, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(33491, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(33536, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(33551, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(33557, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(33564, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(33627, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(33634, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(33639, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(33646, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(33658, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(33670, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(33684, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(33697, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(33712, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(33728, 9), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(33737, 9), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(33746, 15), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(33794, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(33804, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(33814, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(33824, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(33834, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(33844, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(33854, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(33864, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(33874, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(33883, 10), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(33893, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(33910, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(33928, 23), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(33951, 24), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(33975, 15), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(33990, 16), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(34006, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(34023, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(34041, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(34059, 28), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(34087, 29), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(34116, 20), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(34136, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(34157, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(34171, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(34185, 30), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(34215, 32), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(34247, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(34270, 40), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(34310, 12), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(34322, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(34335, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(34351, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(34364, 19), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(34383, 26), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(34409, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(34425, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(34438, 29), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(34467, 29), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(34496, 33), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(34529, 33), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(34562, 33), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(34595, 33), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(34628, 37), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(34665, 37), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(34702, 17), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(34719, 18), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(34737, 22), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(34759, 26), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(34785, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(34792, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(34813, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(34829, 16), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(34845, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(34852, 16), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(34868, 20), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(34888, 26), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(34914, 35), IR(0, 0), IR(166, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(34949, 25), IR(0, 0), IR(166, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(34974, 17), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(34991, 23), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(35014, 19), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(35033, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(35049, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(35065, 11), IR(0, 0), IR(1, 0), IR(206, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(35076, 21), IR(0, 0), IR(78, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(35097, 19), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(35116, 19), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(35135, 24), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(35159, 25), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(35184, 30), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6995, 22), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(35214, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(35243, 32), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(35275, 30), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(35305, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(35334, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(10177, 23), IR(0, 0), IR(244, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(35363, 26), IR(0, 0), IR(244, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(35389, 25), IR(0, 0), IR(245, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(35414, 27), IR(0, 0), IR(245, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(35441, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(35461, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(35481, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(35501, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(35521, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(35541, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(35561, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(35581, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(35601, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(35621, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(35641, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(35667, 25), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(35692, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(35718, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(35744, 25), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(35769, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(35795, 29), IR(0, 0), IR(249, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(35824, 24), IR(0, 0), IR(249, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(35848, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(35860, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(35869, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(35881, 8), IR(0, 0), IR(250, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(35889, 23), IR(0, 0), IR(158, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(35912, 23), IR(0, 0), IR(167, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(35935, 25), IR(0, 0), IR(168, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(35960, 14), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(36016, 14), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(36030, 15), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(36045, 19), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(36064, 17), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(36081, 19), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(10468, 9), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(36100, 14), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(36114, 18), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(36132, 12), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(36144, 13), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBitcastExtractEXT, true, true, IR(172, 4), IR(10615, 18), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(36157, 20), IR(0, 0), IR(3, 1), IR(216, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(36177, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(36199, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(36218, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(36240, 30), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6756, 18), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(36270, 27), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(36297, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(36322, 33), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(36355, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(36377, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(36396, 7), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(36403, 15), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(36418, 15), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(36433, 20), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(15649, 25), IR(0, 0), IR(258, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(36453, 26), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(36479, 26), IR(0, 0), IR(1, 0), IR(217, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(36505, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(36530, 12), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(36542, 19), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(36561, 35), IR(0, 0), IR(259, 2), IR(218, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(36596, 22), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(36618, 16), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(36681, 5), IR(176, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(36694, 5), IR(177, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(36707, 6), IR(178, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(36722, 11), IR(179, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(36747, 11), IR(180, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(36772, 12), IR(181, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(36799, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(36824, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(36849, 26), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(36929, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(36956, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36983, 30), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(37013, 34), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(37047, 31), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(37078, 16), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(37094, 22), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(37116, 21), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(37137, 32), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(37169, 31), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(37200, 19), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(37219, 31), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(37250, 24), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(37274, 19), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(37293, 23), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(37316, 23), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(37339, 17), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(37356, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(37385, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(37414, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(37443, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBFloat16MulAddQCOM, true, true, IR(130, 5), IR(11388, 19), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(37472, 30), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(37502, 28), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(37530, 20), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpImageGatherQCOM, true, true, IR(479, 7), IR(37550, 16), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(37566, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(37589, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(37612, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(37635, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(37658, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(37681, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(37704, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(37727, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(37750, 21), IR(0, 0), IR(271, 1), IR(63, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(37771, 17), IR(0, 0), IR(271, 1), IR(63, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(37788, 13), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(37801, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(37826, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(37850, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(37875, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(37904, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(37931, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(37954, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(37973, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(37996, 26), IR(0, 0), IR(176, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(38022, 26), IR(0, 0), IR(176, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(38048, 14), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(38062, 17), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAbortKHR, false, false, IR(36, 2), IR(11725, 9), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(38079, 28), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(38107, 20), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(38127, 18), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantDataKHR, true, true, IR(486, 3), IR(11734, 16), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantDataKHR, true, true, IR(486, 3), IR(38145, 20), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(38165, 10), IR(0, 0), IR(177, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(38175, 10), IR(0, 0), IR(177, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(489, 14), IR(38185, 27), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(503, 13), IR(38212, 36), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(516, 7), IR(38248, 28), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(38276, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(38304, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(38332, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(38365, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(503, 13), IR(38395, 26), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(38421, 39), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(38460, 44), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(38504, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(523, 12), IR(38527, 20), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(503, 13), IR(38547, 21), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(523, 12), IR(38568, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(535, 6), IR(38598, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(38620, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(38645, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(38671, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(38696, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(38718, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(38747, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(38775, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(38800, 34), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(38834, 32), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(38866, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(38895, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(38917, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(38939, 19), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(38958, 17), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(38975, 18), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(541, 3), IR(38993, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(39022, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(39046, 16), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(479, 7), IR(39062, 23), IR(0, 0), IR(276, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(39085, 16), IR(182, 1), IR(277, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(544, 13), IR(39125, 29), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(557, 7), IR(39154, 42), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(39196, 39), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(564, 16), IR(39235, 32), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertUseEXT, true, true, IR(160, 3), IR(39267, 31), IR(183, 1), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(580, 4), IR(39325, 17), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(39342, 18), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(39360, 28), IR(184, 1), IR(202, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(39415, 33), IR(0, 0), IR(51, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(39448, 35), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(39483, 38), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(584, 5), IR(39521, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(589, 4), IR(39545, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(593, 5), IR(39570, 28), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(516, 7), IR(39598, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(39621, 29), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(39650, 51), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(39701, 24), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(39725, 45), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(598, 4), IR(39770, 33), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(602, 14), IR(39803, 32), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(616, 15), IR(39835, 38), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(39873, 17), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(39890, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(541, 3), IR(39915, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(523, 12), IR(39945, 21), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(503, 13), IR(39966, 27), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(39993, 24), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(40017, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(40043, 27), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(40070, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(40096, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(40119, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(40149, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(40178, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(40204, 35), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(40239, 31), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(40270, 34), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(40304, 33), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(40337, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(40367, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(40396, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(40425, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(40448, 22), IR(185, 1), IR(55, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(40491, 21), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(40512, 15), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(40527, 8), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(523, 12), IR(40535, 14), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(523, 12), IR(40549, 17), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(40566, 50), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(40616, 29), IR(186, 1), IR(287, 4), IR(220, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(40673, 18), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(40691, 35), IR(187, 1), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(40749, 24), IR(0, 0), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(40773, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(40796, 45), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(40841, 40), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(40881, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(40901, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(40919, 19), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(631, 5), IR(40938, 24), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(636, 6), IR(40962, 24), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(642, 5), IR(40986, 25), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(41011, 26), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(41037, 26), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixGetCoordinateEXT, true, true, IR(172, 4), IR(13911, 34), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(41063, 28), IR(0, 0), IR(293, 3), IR(91, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(41091, 26), IR(0, 0), IR(293, 3), IR(91, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceEXT, true, true, IR(647, 5), IR(41160, 27), IR(188, 1), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(652, 7), IR(41288, 30), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(659, 5), IR(41318, 31), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpEXT, true, true, IR(151, 5), IR(41349, 33), IR(189, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(41414, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(664, 4), IR(41433, 17), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(41450, 21), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(41471, 27), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(41498, 24), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(41522, 20), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(41542, 28), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(41570, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(41589, 25), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(41614, 22), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(13265, 25), IR(46, 1), IR(299, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(41636, 22), IR(0, 0), IR(299, 1), IR(92, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(41658, 20), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(41678, 27), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(41705, 29), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(41734, 18), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(41752, 20), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(41772, 18), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(41790, 20), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(41810, 25), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(41835, 25), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(41860, 29), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(668, 7), IR(41941, 17), IR(0, 0), IR(209, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(41958, 40), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(41998, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(42036, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(42074, 34), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(42108, 37), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(42145, 29), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(42174, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(42201, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(42228, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(42251, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(42274, 20), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(42294, 22), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(42316, 19), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13996, 21), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(42335, 25), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(42360, 23), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(42383, 24), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(42407, 23), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(42430, 24), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(42454, 28), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(42482, 29), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(42511, 33), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(675, 5), IR(42544, 34), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(42578, 24), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(42602, 25), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(42627, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(42640, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(42653, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(42666, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(42679, 14), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(42693, 14), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(42707, 21), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(42728, 21), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(42749, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(42762, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(42775, 15), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(42790, 15), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(42805, 29), IR(0, 0), IR(218, 1), IR(115, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(42834, 25), IR(0, 0), IR(218, 1), IR(115, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(42859, 15), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(680, 6), IR(14214, 9), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(42874, 13), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(42887, 14), IR(0, 0), IR(306, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(42901, 14), IR(0, 0), IR(306, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(42915, 14), IR(0, 0), IR(310, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(42929, 10), IR(0, 0), IR(310, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(42939, 15), IR(190, 1), IR(1, 0), IR(224, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(42975, 21), IR(191, 1), IR(1, 0), IR(224, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(43023, 14), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(43037, 18), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(43055, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(43078, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(43101, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(43124, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(43147, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(43169, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(43191, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(43237, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(43281, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(43320, 37), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(43357, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(43379, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43401, 60), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43461, 53), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43514, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43561, 40), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43601, 51), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43652, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(43696, 51), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(43747, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43803, 50), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43853, 52), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43905, 49), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(43954, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(44001, 50), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(44051, 51), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(44102, 56), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(44158, 33), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(44191, 52), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(44243, 61), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44304, 61), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(44365, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(44404, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(44442, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(44481, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(44519, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(44558, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(44596, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(44632, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(44671, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(44714, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(44752, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(44790, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(44828, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(44873, 40), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44913, 62), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(44975, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(45005, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(45043, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(45079, 33), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(45112, 35), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(45147, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(45186, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(45229, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(45276, 54), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(45330, 34), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(45364, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(45411, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(45456, 55), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(45511, 53), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(45564, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(45620, 54), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(45674, 58), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(45732, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(45788, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(45826, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(45872, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45916, 49), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45965, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(46012, 70), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(46082, 68), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(46150, 69), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(46219, 68), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(46287, 66), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(46353, 67), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(46420, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(46456, 48), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(46504, 59), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(46563, 58), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(46621, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(686, 9), IR(46677, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(695, 10), IR(46707, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46737, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(46776, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46822, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46865, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46912, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46957, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(47003, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(47059, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(47097, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(47127, 32), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(695, 10), IR(47159, 36), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(705, 13), IR(47195, 42), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(47237, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(47276, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(47315, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(47359, 48), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(47407, 50), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(47457, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(47500, 48), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(47548, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(47591, 31), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(47622, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(47669, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(47714, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(47760, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(47816, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(47854, 35), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(47889, 44), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(47933, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(47979, 41), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(48020, 36), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(48056, 50), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(48106, 48), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(48154, 35), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(14452, 25), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(48189, 16), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(48205, 19), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(718, 8), IR(48224, 29), IR(192, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(718, 8), IR(48281, 25), IR(193, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(718, 8), IR(48330, 32), IR(194, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(718, 8), IR(48393, 30), IR(195, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(726, 10), IR(48452, 24), IR(196, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(726, 10), IR(48499, 24), IR(197, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(726, 10), IR(48546, 24), IR(198, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(726, 10), IR(48593, 24), IR(199, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(736, 6), IR(48640, 23), IR(200, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(736, 6), IR(48685, 23), IR(201, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(736, 6), IR(48730, 23), IR(202, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(736, 6), IR(48775, 23), IR(203, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(736, 6), IR(48820, 23), IR(204, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(718, 8), IR(48865, 26), IR(205, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(718, 8), IR(48916, 26), IR(206, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(718, 8), IR(48967, 25), IR(207, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(726, 10), IR(49016, 26), IR(208, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(718, 8), IR(49067, 25), IR(209, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(718, 8), IR(49116, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(718, 8), IR(49139, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(718, 8), IR(49163, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(718, 8), IR(49188, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(718, 8), IR(49213, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(718, 8), IR(49236, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(718, 8), IR(49260, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(718, 8), IR(49285, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(718, 8), IR(49310, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(718, 8), IR(49333, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(718, 8), IR(49356, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(718, 8), IR(49382, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(718, 8), IR(49407, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(718, 8), IR(49432, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(718, 8), IR(49456, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(718, 8), IR(49482, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(718, 8), IR(49506, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(718, 8), IR(49532, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(718, 8), IR(49556, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(726, 10), IR(49582, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(726, 10), IR(49607, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(726, 10), IR(49630, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(726, 10), IR(49654, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(49678, 17), IR(0, 0), IR(318, 1), IR(129, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(742, 2), IR(49695, 21), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(744, 3), IR(49716, 20), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(49736, 24), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(718, 8), IR(49760, 16), IR(210, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(718, 8), IR(49791, 17), IR(211, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(718, 8), IR(49824, 17), IR(212, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(718, 8), IR(49857, 15), IR(213, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(718, 8), IR(49886, 15), IR(214, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(718, 8), IR(49915, 18), IR(215, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(718, 8), IR(49950, 17), IR(216, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(718, 8), IR(49983, 17), IR(217, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(718, 8), IR(50016, 20), IR(218, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(718, 8), IR(50055, 15), IR(219, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(718, 8), IR(50084, 15), IR(220, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(50113, 30), IR(221, 1), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(50172, 30), IR(222, 1), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, false, false, IR(296, 4), IR(50231, 23), IR(223, 1), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, false, false, IR(296, 4), IR(50276, 24), IR(224, 1), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(15410, 14), IR(65, 1), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(50323, 22), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(50345, 23), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(50368, 28), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(50396, 46), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(50442, 37), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(50479, 65), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(50544, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(50584, 41), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(50625, 39), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(50664, 36), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(50700, 46), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(50746, 45), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(50791, 42), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(50833, 32), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(50865, 29), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(50894, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(50934, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(50974, 14), IR(0, 0), IR(321, 4), IR(166, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(50988, 23), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(747, 1), IR(51011, 25), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(51036, 32), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(51068, 36), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, false, false, IR(747, 1), IR(51104, 33), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(51137, 20), IR(0, 0), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(51157, 20), IR(0, 0), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveEXT, false, false, IR(221, 3), IR(51177, 24), IR(225, 1), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitEXT, false, false, IR(221, 3), IR(51227, 22), IR(226, 1), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(15921, 19), IR(0, 0), IR(328, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(748, 7), IR(51273, 25), IR(227, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(51322, 24), IR(228, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(51369, 22), IR(229, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(51412, 26), IR(230, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(51463, 23), IR(231, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBitcastShuffleINTEL, true, true, IR(160, 3), IR(16287, 28), IR(0, 0), IR(330, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(51508, 27), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(755, 10), IR(51535, 25), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(755, 10), IR(51560, 34), IR(0, 0), IR(332, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(755, 10), IR(51594, 34), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(765, 9), IR(51628, 29), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(755, 10), IR(51657, 26), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(774, 7), IR(16426, 38), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(51745, 21), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(16492, 32), IR(0, 0), IR(336, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(51766, 26), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(781, 5), IR(51792, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(786, 2), IR(51819, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(788, 4), IR(51846, 24), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(792, 6), IR(51870, 30), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(798, 3), IR(51937, 30), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(51967, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPredicatedLoadINTEL, true, true, IR(636, 6), IR(51994, 20), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPredicatedStoreINTEL, false, false, IR(589, 4), IR(52014, 21), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(52035, 13), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(52048, 13), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(52061, 19), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(52080, 18), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(52098, 19), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(52117, 19), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(52136, 18), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(52154, 19), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(52173, 18), IR(0, 0), IR(340, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(801, 6), IR(52191, 18), IR(0, 0), IR(341, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(807, 4), IR(52209, 19), IR(0, 0), IR(341, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(52228, 26), IR(0, 0), IR(342, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(52254, 28), IR(0, 0), IR(342, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(52282, 33), IR(0, 0), IR(342, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(52315, 19), IR(0, 0), IR(343, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(52334, 19), IR(0, 0), IR(345, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(52353, 19), IR(0, 0), IR(347, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 936> kInstructionNames{{
  {IR(11725, 9), 441}, // 0 AbortKHR
  {IR(42627, 13), 604}, // 1 AbsISubINTEL
  {IR(42640, 13), 605}, // 2 AbsUSubINTEL
  {IR(31109, 12), 58}, // 3 AccessChain
  {IR(49695, 21), 790}, // 4 AliasDomainDeclINTEL
  {IR(49716, 20), 791}, // 5 AliasScopeDeclINTEL
  {IR(49736, 24), 792}, // 6 AliasScopeListDeclINTEL
  {IR(32406, 4), 143}, // 7 All
  {IR(37801, 25), 429}, // 8 AllocateNodePayloadsAMDX
  {IR(32402, 4), 142}, // 9 Any
  {IR(49482, 24), 781}, // 10 ArbitraryFloatACosINTEL
  {IR(49506, 26), 782}, // 11 ArbitraryFloatACosPiINTEL
  {IR(49432, 24), 779}, // 12 ArbitraryFloatASinINTEL
  {IR(49456, 26), 780}, // 13 ArbitraryFloatASinPiINTEL
  {IR(49582, 25), 785}, // 14 ArbitraryFloatATan2INTEL
  {IR(49532, 24), 783}, // 15 ArbitraryFloatATanINTEL
  {IR(49556, 26), 784}, // 16 ArbitraryFloatATanPiINTEL
  {IR(48452, 24), 752}, // 17 ArbitraryFloatAddALTERA
  {IR(48476, 23), 752}, // 18 ArbitraryFloatAddINTEL
  {IR(48281, 25), 749}, // 19 ArbitraryFloatCastALTERA
  {IR(48330, 32), 750}, // 20 ArbitraryFloatCastFromIntALTERA
  {IR(48362, 31), 750}, // 21 ArbitraryFloatCastFromIntINTEL
  {IR(48306, 24), 749}, // 22 ArbitraryFloatCastINTEL
  {IR(48393, 30), 751}, // 23 ArbitraryFloatCastToIntALTERA
  {IR(48423, 29), 751}, // 24 ArbitraryFloatCastToIntINTEL
  {IR(48967, 25), 763}, // 25 ArbitraryFloatCbrtALTERA
  {IR(48992, 24), 763}, // 26 ArbitraryFloatCbrtINTEL
  {IR(49333, 23), 775}, // 27 ArbitraryFloatCosINTEL
  {IR(49407, 25), 778}, // 28 ArbitraryFloatCosPiINTEL
  {IR(48593, 24), 755}, // 29 ArbitraryFloatDivALTERA
  {IR(48617, 23), 755}, // 30 ArbitraryFloatDivINTEL
  {IR(48820, 23), 760}, // 31 ArbitraryFloatEQALTERA
  {IR(48843, 22), 760}, // 32 ArbitraryFloatEQINTEL
  {IR(49260, 25), 772}, // 33 ArbitraryFloatExp10INTEL
  {IR(49236, 24), 771}, // 34 ArbitraryFloatExp2INTEL
  {IR(49213, 23), 770}, // 35 ArbitraryFloatExpINTEL
  {IR(49285, 25), 773}, // 36 ArbitraryFloatExpm1INTEL
  {IR(48685, 23), 757}, // 37 ArbitraryFloatGEALTERA
  {IR(48708, 22), 757}, // 38 ArbitraryFloatGEINTEL
  {IR(48640, 23), 756}, // 39 ArbitraryFloatGTALTERA
  {IR(48663, 22), 756}, // 40 ArbitraryFloatGTINTEL
  {IR(49016, 26), 764}, // 41 ArbitraryFloatHypotALTERA
  {IR(49042, 25), 764}, // 42 ArbitraryFloatHypotINTEL
  {IR(48775, 23), 759}, // 43 ArbitraryFloatLEALTERA
  {IR(48798, 22), 759}, // 44 ArbitraryFloatLEINTEL
  {IR(48730, 23), 758}, // 45 ArbitraryFloatLTALTERA
  {IR(48753, 22), 758}, // 46 ArbitraryFloatLTINTEL
  {IR(49163, 25), 768}, // 47 ArbitraryFloatLog10INTEL
  {IR(49188, 25), 769}, // 48 ArbitraryFloatLog1pINTEL
  {IR(49139, 24), 767}, // 49 ArbitraryFloatLog2INTEL
  {IR(49116, 23), 766}, // 50 ArbitraryFloatLogINTEL
  {IR(48546, 24), 754}, // 51 ArbitraryFloatMulALTERA
  {IR(48570, 23), 754}, // 52 ArbitraryFloatMulINTEL
  {IR(49607, 23), 786}, // 53 ArbitraryFloatPowINTEL
  {IR(49654, 24), 788}, // 54 ArbitraryFloatPowNINTEL
  {IR(49630, 24), 787}, // 55 ArbitraryFloatPowRINTEL
  {IR(48916, 26), 762}, // 56 ArbitraryFloatRSqrtALTERA
  {IR(48942, 25), 762}, // 57 ArbitraryFloatRSqrtINTEL
  {IR(48865, 26), 761}, // 58 ArbitraryFloatRecipALTERA
  {IR(48891, 25), 761}, // 59 ArbitraryFloatRecipINTEL
  {IR(49356, 26), 776}, // 60 ArbitraryFloatSinCosINTEL
  {IR(48224, 29), 748}, // 61 ArbitraryFloatSinCosPiALTERA
  {IR(48253, 28), 748}, // 62 ArbitraryFloatSinCosPiINTEL
  {IR(49310, 23), 774}, // 63 ArbitraryFloatSinINTEL
  {IR(49382, 25), 777}, // 64 ArbitraryFloatSinPiINTEL
  {IR(49067, 25), 765}, // 65 ArbitraryFloatSqrtALTERA
  {IR(49092, 24), 765}, // 66 ArbitraryFloatSqrtINTEL
  {IR(48499, 24), 753}, // 67 ArbitraryFloatSubALTERA
  {IR(48523, 23), 753}, // 68 ArbitraryFloatSubINTEL
  {IR(15921, 19), 836}, // 69 ArithmeticFenceEXT
  {IR(31156, 12), 61}, // 70 ArrayLength
  {IR(42874, 13), 620}, // 71 AsmCallINTEL
  {IR(14214, 9), 619}, // 72 AsmINTEL
  {IR(42859, 15), 618}, // 73 AsmTargetINTEL
  {IR(42915, 14), 623}, // 74 AssumeTrueKHR
  {IR(33428, 10), 220}, // 75 AtomicAnd
  {IR(33280, 22), 210}, // 76 AtomicCompareExchange
  {IR(33302, 26), 211}, // 77 AtomicCompareExchangeWeak
  {IR(33265, 15), 209}, // 78 AtomicExchange
  {IR(50974, 14), 826}, // 79 AtomicFAddEXT
  {IR(42901, 14), 622}, // 80 AtomicFMaxEXT
  {IR(42887, 14), 621}, // 81 AtomicFMinEXT
  {IR(34813, 16), 292}, // 82 AtomicFlagClear
  {IR(34792, 21), 291}, // 83 AtomicFlagTestAndSet
  {IR(33362, 11), 214}, // 84 AtomicIAdd
  {IR(33345, 17), 213}, // 85 AtomicIDecrement
  {IR(33328, 17), 212}, // 86 AtomicIIncrement
  {IR(33373, 11), 215}, // 87 AtomicISub
  {IR(33242, 11), 207}, // 88 AtomicLoad
  {IR(33438, 9), 221}, // 89 AtomicOr
  {IR(33406, 11), 218}, // 90 AtomicSMax
  {IR(33384, 11), 216}, // 91 AtomicSMin
  {IR(33253, 12), 208}, // 92 AtomicStore
  {IR(33417, 11), 219}, // 93 AtomicUMax
  {IR(33395, 11), 217}, // 94 AtomicUMin
  {IR(33447, 10), 222}, // 95 AtomicXor
  {IR(11388, 19), 413}, // 96 BFloat16MulAddQCOM
  {IR(41063, 28), 550}, // 97 BeginInvocationInterlockEXT
  {IR(37339, 17), 408}, // 98 BitCastArrayQCOM
  {IR(33026, 9), 191}, // 99 BitCount
  {IR(32966, 15), 187}, // 100 BitFieldInsert
  {IR(32981, 17), 188}, // 101 BitFieldSExtract
  {IR(32998, 17), 189}, // 102 BitFieldUExtract
  {IR(33015, 11), 190}, // 103 BitReverse
  {IR(32154, 8), 114}, // 104 Bitcast
  {IR(10615, 18), 358}, // 105 BitcastExtractEXT
  {IR(32951, 11), 185}, // 106 BitwiseAnd
  {IR(51745, 21), 850}, // 107 BitwiseFunctionINTEL
  {IR(32930, 10), 183}, // 108 BitwiseOr
  {IR(32940, 11), 184}, // 109 BitwiseXor
  {IR(33557, 7), 227}, // 110 Branch
  {IR(33564, 18), 228}, // 111 BranchConditional
  {IR(38062, 17), 440}, // 112 BufferPointerEXT
  {IR(34425, 13), 277}, // 113 BuildNDRange
  {IR(30170, 11), 15}, // 114 Capability
  {IR(34383, 26), 275}, // 115 CaptureEventProfilingInfo
  {IR(35889, 23), 344}, // 116 ColorAttachmentReadEXT
  {IR(33975, 15), 255}, // 117 CommitReadPipe
  {IR(33990, 16), 256}, // 118 CommitWritePipe
  {IR(31416, 19), 72}, // 119 CompositeConstruct
  {IR(51104, 33), 831}, // 120 CompositeConstructContinuedINTEL
  {IR(37472, 30), 414}, // 121 CompositeConstructCoopMatQCOM
  {IR(37047, 31), 396}, // 122 CompositeConstructReplicateEXT
  {IR(31435, 17), 73}, // 123 CompositeExtract
  {IR(37502, 28), 415}, // 124 CompositeExtractCoopMatQCOM
  {IR(31452, 16), 74}, // 125 CompositeInsert
  {IR(51819, 27), 854}, // 126 ConditionalCapabilityINTEL
  {IR(51967, 27), 858}, // 127 ConditionalCopyObjectINTEL
  {IR(51792, 27), 853}, // 128 ConditionalEntryPointINTEL
  {IR(51766, 26), 852}, // 129 ConditionalExtensionINTEL
  {IR(18265, 9), 39}, // 130 Constant
  {IR(30676, 18), 40}, // 131 ConstantComposite
  {IR(51036, 32), 829}, // 132 ConstantCompositeContinuedINTEL
  {IR(36983, 30), 394}, // 133 ConstantCompositeReplicateEXT
  {IR(11734, 16), 445}, // 134 ConstantDataKHR
  {IR(30624, 14), 38}, // 135 ConstantFalse
  {IR(42805, 29), 616}, // 136 ConstantFunctionPointerINTEL
  {IR(30788, 13), 42}, // 137 ConstantNull
  {IR(34868, 20), 296}, // 138 ConstantPipeStorage
  {IR(30772, 16), 41}, // 139 ConstantSampler
  {IR(38127, 18), 444}, // 140 ConstantSizeOfEXT
  {IR(37954, 19), 435}, // 141 ConstantStringAMDX
  {IR(30611, 13), 37}, // 142 ConstantTrue
  {IR(33213, 15), 205}, // 143 ControlBarrier
  {IR(51177, 24), 834}, // 144 ControlBarrierArriveEXT
  {IR(51201, 26), 834}, // 145 ControlBarrierArriveINTEL
  {IR(51227, 22), 835}, // 146 ControlBarrierWaitEXT
  {IR(51249, 24), 835}, // 147 ControlBarrierWaitINTEL
  {IR(51157, 20), 833}, // 148 ConvertBF16ToFINTEL
  {IR(51137, 20), 832}, // 149 ConvertFToBF16INTEL
  {IR(31960, 12), 100}, // 150 ConvertFToS
  {IR(31948, 12), 99}, // 151 ConvertFToU
  {IR(52228, 26), 872}, // 152 ConvertHandleToImageINTEL
  {IR(52282, 33), 874}, // 153 ConvertHandleToSampledImageINTEL
  {IR(52254, 28), 873}, // 154 ConvertHandleToSamplerINTEL
  {IR(41772, 18), 573}, // 155 ConvertImageToUNV
  {IR(32037, 14), 107}, // 156 ConvertPtrToU
  {IR(31972, 12), 101}, // 157 ConvertSToF
  {IR(41835, 25), 576}, // 158 ConvertSampledImageToUNV
  {IR(41790, 20), 574}, // 159 ConvertSamplerToUNV
  {IR(36561, 35), 380}, // 160 ConvertUToAccelerationStructureKHR
  {IR(31984, 12), 102}, // 161 ConvertUToF
  {IR(41734, 18), 571}, // 162 ConvertUToImageNV
  {IR(32081, 14), 110}, // 163 ConvertUToPtr
  {IR(41810, 25), 575}, // 164 ConvertUToSampledImageNV
  {IR(41752, 20), 572}, // 165 ConvertUToSamplerNV
  {IR(39298, 27), 488}, // 166 CooperativeMatrixConvertNV
  {IR(39267, 31), 488}, // 167 CooperativeMatrixConvertUseEXT
  {IR(13911, 34), 549}, // 168 CooperativeMatrixGetCoordinateEXT
  {IR(36956, 27), 393}, // 169 CooperativeMatrixLengthKHR
  {IR(41037, 26), 548}, // 170 CooperativeMatrixLengthNV
  {IR(36824, 25), 390}, // 171 CooperativeMatrixLoadKHR
  {IR(40962, 24), 545}, // 172 CooperativeMatrixLoadNV
  {IR(41288, 30), 553}, // 173 CooperativeMatrixLoadTensorNV
  {IR(36929, 27), 392}, // 174 CooperativeMatrixMulAddKHR
  {IR(41011, 26), 547}, // 175 CooperativeMatrixMulAddNV
  {IR(41349, 33), 555}, // 176 CooperativeMatrixPerElementOpEXT
  {IR(41382, 32), 555}, // 177 CooperativeMatrixPerElementOpNV
  {IR(41160, 27), 552}, // 178 CooperativeMatrixReduceEXT
  {IR(41187, 26), 552}, // 179 CooperativeMatrixReduceNV
  {IR(36849, 26), 391}, // 180 CooperativeMatrixStoreKHR
  {IR(40986, 25), 546}, // 181 CooperativeMatrixStoreNV
  {IR(41318, 31), 554}, // 182 CooperativeMatrixStoreTensorNV
  {IR(41705, 29), 570}, // 183 CooperativeMatrixTransposeNV
  {IR(39521, 24), 495}, // 184 CooperativeVectorLoadNV
  {IR(39235, 32), 487}, // 185 CooperativeVectorMatrixMulAddNV
  {IR(39125, 29), 484}, // 186 CooperativeVectorMatrixMulNV
  {IR(39154, 42), 485}, // 187 CooperativeVectorOuterProductAccumulateNV
  {IR(39196, 39), 486}, // 188 CooperativeVectorReduceSumAccumulateNV
  {IR(39545, 25), 496}, // 189 CooperativeVectorStoreNV
  {IR(35848, 12), 340}, // 190 CopyLogical
  {IR(31082, 11), 56}, // 191 CopyMemory
  {IR(31093, 16), 57}, // 192 CopyMemorySized
  {IR(31468, 11), 75}, // 193 CopyObject
  {IR(34888, 26), 297}, // 194 CreatePipeFromPipeStorage
  {IR(41450, 21), 558}, // 195 CreateTensorLayoutNV
  {IR(41570, 19), 563}, // 196 CreateTensorViewNV
  {IR(34335, 16), 272}, // 197 CreateUserEvent
  {IR(50172, 30), 805}, // 198 CrossWorkgroupCastToPtrALTERA
  {IR(50202, 29), 805}, // 199 CrossWorkgroupCastToPtrINTEL
  {IR(33035, 5), 192}, // 200 DPdx
  {IR(33081, 11), 198}, // 201 DPdxCoarse
  {IR(33052, 9), 195}, // 202 DPdxFine
  {IR(33040, 5), 193}, // 203 DPdy
  {IR(33092, 11), 199}, // 204 DPdyCoarse
  {IR(33061, 9), 196}, // 205 DPdyFine
  {IR(31242, 9), 64}, // 206 Decorate
  {IR(35065, 11), 305}, // 207 DecorateId
  {IR(42939, 15), 625}, // 208 DecorateString
  {IR(42954, 21), 625}, // 209 DecorateStringGOOGLE
  {IR(31266, 16), 66}, // 210 DecorationGroup
  {IR(13265, 25), 566}, // 211 DemoteToHelperInvocation
  {IR(13290, 28), 566}, // 212 DemoteToHelperInvocationEXT
  {IR(35912, 23), 345}, // 213 DepthAttachmentReadEXT
  {IR(32351, 4), 137}, // 214 Dot
  {IR(39325, 17), 489}, // 215 EmitMeshTasksEXT
  {IR(33140, 17), 203}, // 216 EmitStreamVertex
  {IR(33116, 11), 201}, // 217 EmitVertex
  {IR(41091, 26), 551}, // 218 EndInvocationInterlockEXT
  {IR(33127, 13), 202}, // 219 EndPrimitive
  {IR(33157, 19), 204}, // 220 EndStreamPrimitive
  {IR(34171, 14), 265}, // 221 EnqueueKernel
  {IR(34157, 14), 264}, // 222 EnqueueMarker
  {IR(37826, 24), 430}, // 223 EnqueueNodePayloadsAMDX
  {IR(30085, 11), 13}, // 224 EntryPoint
  {IR(36542, 19), 379}, // 225 ExecuteCallableKHR
  {IR(40673, 18), 535}, // 226 ExecuteCallableNV
  {IR(30128, 14), 14}, // 227 ExecutionMode
  {IR(35049, 16), 304}, // 228 ExecutionModeId
  {IR(42929, 10), 624}, // 229 ExpectKHR
  {IR(29939, 8), 11}, // 230 ExtInst
  {IR(29879, 14), 10}, // 231 ExtInstImport
  {IR(36479, 26), 376}, // 232 ExtInstWithForwardRefsKHR
  {IR(29869, 10), 9}, // 233 Extension
  {IR(37530, 20), 416}, // 234 ExtractSubArrayQCOM
  {IR(32183, 5), 118}, // 235 FAdd
  {IR(32014, 9), 105}, // 236 FConvert
  {IR(32218, 5), 125}, // 237 FDiv
  {IR(52334, 19), 876}, // 238 FDot2MixAcc16VALVE
  {IR(52315, 19), 875}, // 239 FDot2MixAcc32VALVE
  {IR(52353, 19), 877}, // 240 FDot4MixAcc32VALVE
  {IR(32243, 5), 130}, // 241 FMod
  {IR(32203, 5), 122}, // 242 FMul
  {IR(32170, 8), 116}, // 243 FNegate
  {IR(32680, 10), 168}, // 244 FOrdEqual
  {IR(32758, 16), 174}, // 245 FOrdGreaterThan
  {IR(32830, 21), 178}, // 246 FOrdGreaterThanEqual
  {IR(32730, 13), 172}, // 247 FOrdLessThan
  {IR(32792, 18), 176}, // 248 FOrdLessThanEqual
  {IR(32702, 13), 170}, // 249 FOrdNotEqual
  {IR(15410, 14), 808}, // 250 FPGARegALTERA
  {IR(15424, 13), 808}, // 251 FPGARegINTEL
  {IR(32238, 5), 129}, // 252 FRem
  {IR(32193, 5), 120}, // 253 FSub
  {IR(32690, 12), 169}, // 254 FUnordEqual
  {IR(32774, 18), 175}, // 255 FUnordGreaterThan
  {IR(32851, 23), 179}, // 256 FUnordGreaterThanEqual
  {IR(32743, 15), 173}, // 257 FUnordLessThan
  {IR(32810, 20), 177}, // 258 FUnordLessThanEqual
  {IR(32715, 15), 171}, // 259 FUnordNotEqual
  {IR(39483, 38), 494}, // 260 FetchMicroTriangleVertexBarycentricNV
  {IR(39448, 35), 493}, // 261 FetchMicroTriangleVertexPositionNV
  {IR(37875, 29), 432}, // 262 FinishWritingNodePayloadAMDX
  {IR(49886, 15), 797}, // 263 FixedCosALTERA
  {IR(49901, 14), 797}, // 264 FixedCosINTEL
  {IR(49983, 17), 800}, // 265 FixedCosPiALTERA
  {IR(50000, 16), 800}, // 266 FixedCosPiINTEL
  {IR(50084, 15), 803}, // 267 FixedExpALTERA
  {IR(50099, 14), 803}, // 268 FixedExpINTEL
  {IR(50055, 15), 802}, // 269 FixedLogALTERA
  {IR(50070, 14), 802}, // 270 FixedLogINTEL
  {IR(49791, 17), 794}, // 271 FixedRecipALTERA
  {IR(49808, 16), 794}, // 272 FixedRecipINTEL
  {IR(49824, 17), 795}, // 273 FixedRsqrtALTERA
  {IR(49841, 16), 795}, // 274 FixedRsqrtINTEL
  {IR(49857, 15), 796}, // 275 FixedSinALTERA
  {IR(49915, 18), 798}, // 276 FixedSinCosALTERA
  {IR(49933, 17), 798}, // 277 FixedSinCosINTEL
  {IR(50016, 20), 801}, // 278 FixedSinCosPiALTERA
  {IR(50036, 19), 801}, // 279 FixedSinCosPiINTEL
  {IR(49872, 14), 796}, // 280 FixedSinINTEL
  {IR(49950, 17), 799}, // 281 FixedSinPiALTERA
  {IR(49967, 16), 799}, // 282 FixedSinPiINTEL
  {IR(49760, 16), 793}, // 283 FixedSqrtALTERA
  {IR(49776, 15), 793}, // 284 FixedSqrtINTEL
  {IR(36396, 7), 370}, // 285 FmaKHR
  {IR(37771, 17), 427}, // 286 FragmentFetchAMD
  {IR(37750, 21), 426}, // 287 FragmentMaskFetchAMD
  {IR(38175, 10), 448}, // 288 FreezeKHR
  {IR(28918, 9), 48}, // 289 Function
  {IR(30991, 13), 51}, // 290 FunctionCall
  {IR(30979, 12), 50}, // 291 FunctionEnd
  {IR(30961, 18), 49}, // 292 FunctionParameter
  {IR(42834, 25), 617}, // 293 FunctionPointerCallINTEL
  {IR(33045, 7), 194}, // 294 Fwidth
  {IR(33103, 13), 200}, // 295 FwidthCoarse
  {IR(33070, 11), 197}, // 296 FwidthFine
  {IR(32112, 17), 112}, // 297 GenericCastToPtr
  {IR(32129, 25), 113}, // 298 GenericCastToPtrExplicit
  {IR(31168, 23), 62}, // 299 GenericPtrMemSemantics
  {IR(34409, 16), 276}, // 300 GetDefaultQueue
  {IR(34914, 35), 298}, // 301 GetKernelLocalSizeForSubgroupCount
  {IR(34949, 25), 299}, // 302 GetKernelMaxNumSubgroups
  {IR(34215, 32), 267}, // 303 GetKernelNDrangeMaxSubGroupSize
  {IR(34185, 30), 266}, // 304 GetKernelNDrangeSubGroupCount
  {IR(34270, 40), 269}, // 305 GetKernelPreferredWorkGroupSizeMultiple
  {IR(34247, 23), 268}, // 306 GetKernelWorkGroupSize
  {IR(34041, 18), 259}, // 307 GetMaxPipePackets
  {IR(34023, 18), 258}, // 308 GetNumPipePackets
  {IR(10468, 9), 353}, // 309 GraphARM
  {IR(36064, 17), 351}, // 310 GraphConstantARM
  {IR(36132, 12), 356}, // 311 GraphEndARM
  {IR(36081, 19), 352}, // 312 GraphEntryPointARM
  {IR(36100, 14), 354}, // 313 GraphInputARM
  {IR(36114, 18), 355}, // 314 GraphSetOutputARM
  {IR(33728, 9), 238}, // 315 GroupAll
  {IR(33737, 9), 239}, // 316 GroupAny
  {IR(33697, 15), 236}, // 317 GroupAsyncCopy
  {IR(52061, 19), 863}, // 318 GroupBitwiseAndKHR
  {IR(52080, 18), 864}, // 319 GroupBitwiseOrKHR
  {IR(52098, 19), 865}, // 320 GroupBitwiseXorKHR
  {IR(33746, 15), 240}, // 321 GroupBroadcast
  {IR(34116, 20), 262}, // 322 GroupCommitReadPipe
  {IR(34136, 21), 263}, // 323 GroupCommitWritePipe
  {IR(31282, 14), 67}, // 324 GroupDecorate
  {IR(33804, 10), 242}, // 325 GroupFAdd
  {IR(37589, 23), 419}, // 326 GroupFAddNonUniformAMD
  {IR(33844, 10), 246}, // 327 GroupFMax
  {IR(37681, 23), 423}, // 328 GroupFMaxNonUniformAMD
  {IR(33814, 10), 243}, // 329 GroupFMin
  {IR(37612, 23), 420}, // 330 GroupFMinNonUniformAMD
  {IR(52048, 13), 862}, // 331 GroupFMulKHR
  {IR(33794, 10), 241}, // 332 GroupIAdd
  {IR(37566, 23), 418}, // 333 GroupIAddNonUniformAMD
  {IR(52035, 13), 861}, // 334 GroupIMulKHR
  {IR(52117, 19), 866}, // 335 GroupLogicalAndKHR
  {IR(52136, 18), 867}, // 336 GroupLogicalOrKHR
  {IR(52154, 19), 868}, // 337 GroupLogicalXorKHR
  {IR(31341, 20), 68}, // 338 GroupMemberDecorate
  {IR(35097, 19), 307}, // 339 GroupNonUniformAll
  {IR(35135, 24), 309}, // 340 GroupNonUniformAllEqual
  {IR(35116, 19), 308}, // 341 GroupNonUniformAny
  {IR(6995, 22), 312}, // 342 GroupNonUniformBallot
  {IR(35275, 30), 315}, // 343 GroupNonUniformBallotBitCount
  {IR(35243, 32), 314}, // 344 GroupNonUniformBallotBitExtract
  {IR(35305, 29), 316}, // 345 GroupNonUniformBallotFindLSB
  {IR(35334, 29), 317}, // 346 GroupNonUniformBallotFindMSB
  {IR(35641, 26), 332}, // 347 GroupNonUniformBitwiseAnd
  {IR(35667, 25), 333}, // 348 GroupNonUniformBitwiseOr
  {IR(35692, 26), 334}, // 349 GroupNonUniformBitwiseXor
  {IR(35159, 25), 310}, // 350 GroupNonUniformBroadcast
  {IR(35184, 30), 311}, // 351 GroupNonUniformBroadcastFirst
  {IR(35076, 21), 306}, // 352 GroupNonUniformElect
  {IR(35461, 20), 323}, // 353 GroupNonUniformFAdd
  {IR(35621, 20), 331}, // 354 GroupNonUniformFMax
  {IR(35561, 20), 328}, // 355 GroupNonUniformFMin
  {IR(35501, 20), 325}, // 356 GroupNonUniformFMul
  {IR(35441, 20), 322}, // 357 GroupNonUniformIAdd
  {IR(35481, 20), 324}, // 358 GroupNonUniformIMul
  {IR(35214, 29), 313}, // 359 GroupNonUniformInverseBallot
  {IR(35718, 26), 335}, // 360 GroupNonUniformLogicalAnd
  {IR(35744, 25), 336}, // 361 GroupNonUniformLogicalOr
  {IR(35769, 26), 337}, // 362 GroupNonUniformLogicalXor
  {IR(39360, 28), 491}, // 363 GroupNonUniformPartitionEXT
  {IR(39388, 27), 491}, // 364 GroupNonUniformPartitionNV
  {IR(37996, 26), 437}, // 365 GroupNonUniformQuadAllKHR
  {IR(38022, 26), 438}, // 366 GroupNonUniformQuadAnyKHR
  {IR(35795, 29), 338}, // 367 GroupNonUniformQuadBroadcast
  {IR(35824, 24), 339}, // 368 GroupNonUniformQuadSwap
  {IR(15649, 25), 374}, // 369 GroupNonUniformRotateKHR
  {IR(35581, 20), 329}, // 370 GroupNonUniformSMax
  {IR(35521, 20), 326}, // 371 GroupNonUniformSMin
  {IR(10177, 23), 318}, // 372 GroupNonUniformShuffle
  {IR(35414, 27), 321}, // 373 GroupNonUniformShuffleDown
  {IR(35389, 25), 320}, // 374 GroupNonUniformShuffleUp
  {IR(35363, 26), 319}, // 375 GroupNonUniformShuffleXor
  {IR(35601, 20), 330}, // 376 GroupNonUniformUMax
  {IR(35541, 20), 327}, // 377 GroupNonUniformUMin
  {IR(34059, 28), 260}, // 378 GroupReserveReadPipePackets
  {IR(34087, 29), 261}, // 379 GroupReserveWritePipePackets
  {IR(33864, 10), 248}, // 380 GroupSMax
  {IR(37727, 23), 425}, // 381 GroupSMaxNonUniformAMD
  {IR(33834, 10), 245}, // 382 GroupSMin
  {IR(37658, 23), 422}, // 383 GroupSMinNonUniformAMD
  {IR(33854, 10), 247}, // 384 GroupUMax
  {IR(37704, 23), 424}, // 385 GroupUMaxNonUniformAMD
  {IR(33824, 10), 244}, // 386 GroupUMin
  {IR(37635, 23), 421}, // 387 GroupUMinNonUniformAMD
  {IR(33712, 16), 237}, // 388 GroupWaitEvents
  {IR(40017, 26), 512}, // 389 HitObjectExecuteShaderEXT
  {IR(38620, 25), 464}, // 390 HitObjectExecuteShaderNV
  {IR(40070, 26), 514}, // 391 HitObjectGetAttributesEXT
  {IR(38671, 25), 466}, // 392 HitObjectGetAttributesNV
  {IR(40749, 24), 537}, // 393 HitObjectGetClusterIdNV
  {IR(40043, 27), 513}, // 394 HitObjectGetCurrentTimeEXT
  {IR(38645, 26), 465}, // 395 HitObjectGetCurrentTimeNV
  {IR(40149, 29), 517}, // 396 HitObjectGetGeometryIndexEXT
  {IR(38747, 28), 469}, // 397 HitObjectGetGeometryIndexNV
  {IR(40096, 23), 515}, // 398 HitObjectGetHitKindEXT
  {IR(38696, 22), 467}, // 399 HitObjectGetHitKindNV
  {IR(40204, 35), 519}, // 400 HitObjectGetInstanceCustomIndexEXT
  {IR(38800, 34), 471}, // 401 HitObjectGetInstanceCustomIndexNV
  {IR(40178, 26), 518}, // 402 HitObjectGetInstanceIdEXT
  {IR(38775, 25), 470}, // 403 HitObjectGetInstanceIdNV
  {IR(39650, 51), 500}, // 404 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(42201, 27), 586}, // 405 HitObjectGetLSSPositionsNV
  {IR(42228, 23), 587}, // 406 HitObjectGetLSSRadiiNV
  {IR(40270, 34), 521}, // 407 HitObjectGetObjectRayDirectionEXT
  {IR(38332, 33), 454}, // 408 HitObjectGetObjectRayDirectionNV
  {IR(40239, 31), 520}, // 409 HitObjectGetObjectRayOriginEXT
  {IR(38365, 30), 455}, // 410 HitObjectGetObjectRayOriginNV
  {IR(40367, 29), 524}, // 411 HitObjectGetObjectToWorldEXT
  {IR(38304, 28), 453}, // 412 HitObjectGetObjectToWorldNV
  {IR(40119, 30), 516}, // 413 HitObjectGetPrimitiveIndexEXT
  {IR(38718, 29), 468}, // 414 HitObjectGetPrimitiveIndexNV
  {IR(39701, 24), 501}, // 415 HitObjectGetRayFlagsEXT
  {IR(40425, 23), 526}, // 416 HitObjectGetRayTMaxEXT
  {IR(38895, 22), 474}, // 417 HitObjectGetRayTMaxNV
  {IR(40773, 23), 538}, // 418 HitObjectGetRayTMinEXT
  {IR(38917, 22), 475}, // 419 HitObjectGetRayTMinNV
  {IR(40796, 45), 539}, // 420 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(38460, 44), 458}, // 421 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(40841, 40), 540}, // 422 HitObjectGetShaderRecordBufferHandleEXT
  {IR(38421, 39), 457}, // 423 HitObjectGetShaderRecordBufferHandleNV
  {IR(42145, 29), 584}, // 424 HitObjectGetSpherePositionNV
  {IR(42174, 27), 585}, // 425 HitObjectGetSphereRadiusNV
  {IR(40304, 33), 522}, // 426 HitObjectGetWorldRayDirectionEXT
  {IR(38834, 32), 472}, // 427 HitObjectGetWorldRayDirectionNV
  {IR(40337, 30), 523}, // 428 HitObjectGetWorldRayOriginEXT
  {IR(38866, 29), 473}, // 429 HitObjectGetWorldRayOriginNV
  {IR(40396, 29), 525}, // 430 HitObjectGetWorldToObjectEXT
  {IR(38276, 28), 452}, // 431 HitObjectGetWorldToObjectNV
  {IR(40881, 20), 541}, // 432 HitObjectIsEmptyEXT
  {IR(38939, 19), 476}, // 433 HitObjectIsEmptyNV
  {IR(40901, 18), 542}, // 434 HitObjectIsHitEXT
  {IR(38958, 17), 477}, // 435 HitObjectIsHitNV
  {IR(42274, 20), 589}, // 436 HitObjectIsLSSHitNV
  {IR(40919, 19), 543}, // 437 HitObjectIsMissEXT
  {IR(38975, 18), 478}, // 438 HitObjectIsMissNV
  {IR(42251, 23), 588}, // 439 HitObjectIsSphereHitNV
  {IR(39993, 24), 511}, // 440 HitObjectRecordEmptyEXT
  {IR(38504, 23), 459}, // 441 HitObjectRecordEmptyNV
  {IR(39570, 28), 497}, // 442 HitObjectRecordFromQueryEXT
  {IR(38185, 27), 449}, // 443 HitObjectRecordHitMotionNV
  {IR(38547, 21), 461}, // 444 HitObjectRecordHitNV
  {IR(38212, 36), 450}, // 445 HitObjectRecordHitWithIndexMotionNV
  {IR(38568, 30), 462}, // 446 HitObjectRecordHitWithIndexNV
  {IR(39598, 23), 498}, // 447 HitObjectRecordMissEXT
  {IR(39621, 29), 499}, // 448 HitObjectRecordMissMotionEXT
  {IR(38248, 28), 451}, // 449 HitObjectRecordMissMotionNV
  {IR(38598, 22), 463}, // 450 HitObjectRecordMissNV
  {IR(39770, 33), 503}, // 451 HitObjectReorderExecuteShaderEXT
  {IR(39725, 45), 502}, // 452 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(39835, 38), 505}, // 453 HitObjectTraceMotionReorderExecuteEXT
  {IR(39945, 21), 509}, // 454 HitObjectTraceRayEXT
  {IR(39966, 27), 510}, // 455 HitObjectTraceRayMotionEXT
  {IR(38395, 26), 456}, // 456 HitObjectTraceRayMotionNV
  {IR(38527, 20), 460}, // 457 HitObjectTraceRayNV
  {IR(39803, 32), 504}, // 458 HitObjectTraceReorderExecuteEXT
  {IR(32178, 5), 117}, // 459 IAdd
  {IR(32355, 10), 138}, // 460 IAddCarry
  {IR(42653, 13), 606}, // 461 IAddSatINTEL
  {IR(42679, 14), 608}, // 462 IAverageINTEL
  {IR(42707, 21), 610}, // 463 IAverageRoundedINTEL
  {IR(32551, 7), 158}, // 464 IEqual
  {IR(32198, 5), 121}, // 465 IMul
  {IR(42775, 15), 614}, // 466 IMul32x16INTEL
  {IR(32558, 10), 159}, // 467 INotEqual
  {IR(32188, 5), 119}, // 468 ISub
  {IR(32365, 11), 139}, // 469 ISubBorrow
  {IR(42749, 13), 612}, // 470 ISubSatINTEL
  {IR(36596, 22), 381}, // 471 IgnoreIntersectionKHR
  {IR(40491, 21), 528}, // 472 IgnoreIntersectionNV
  {IR(28962, 6), 91}, // 473 Image
  {IR(37443, 29), 412}, // 474 ImageBlockMatchGatherSADQCOM
  {IR(37414, 29), 411}, // 475 ImageBlockMatchGatherSSDQCOM
  {IR(37316, 23), 407}, // 476 ImageBlockMatchSADQCOM
  {IR(37293, 23), 406}, // 477 ImageBlockMatchSSDQCOM
  {IR(37385, 29), 410}, // 478 ImageBlockMatchWindowSADQCOM
  {IR(37356, 29), 409}, // 479 ImageBlockMatchWindowSSDQCOM
  {IR(37274, 19), 405}, // 480 ImageBoxFilterQCOM
  {IR(31796, 16), 88}, // 481 ImageDrefGather
  {IR(31773, 11), 86}, // 482 ImageFetch
  {IR(31784, 12), 87}, // 483 ImageGather
  {IR(37550, 16), 417}, // 484 ImageGatherQCOM
  {IR(31833, 17), 92}, // 485 ImageQueryFormat
  {IR(31913, 17), 97}, // 486 ImageQueryLevels
  {IR(31899, 14), 96}, // 487 ImageQueryLod
  {IR(31850, 16), 93}, // 488 ImageQueryOrder
  {IR(31930, 18), 98}, // 489 ImageQuerySamples
  {IR(31884, 15), 95}, // 490 ImageQuerySize
  {IR(31866, 18), 94}, // 491 ImageQuerySizeLod
  {IR(31812, 10), 89}, // 492 ImageRead
  {IR(31630, 27), 81}, // 493 ImageSampleDrefExplicitLod
  {IR(31603, 27), 80}, // 494 ImageSampleDrefImplicitLod
  {IR(31580, 23), 79}, // 495 ImageSampleExplicitLod
  {IR(39062, 23), 482}, // 496 ImageSampleFootprintNV
  {IR(31534, 23), 78}, // 497 ImageSampleImplicitLod
  {IR(31742, 31), 85}, // 498 ImageSampleProjDrefExplicitLod
  {IR(31711, 31), 84}, // 499 ImageSampleProjDrefImplicitLod
  {IR(31684, 27), 83}, // 500 ImageSampleProjExplicitLod
  {IR(31657, 27), 82}, // 501 ImageSampleProjImplicitLod
  {IR(37250, 24), 404}, // 502 ImageSampleWeightedQCOM
  {IR(34737, 22), 288}, // 503 ImageSparseDrefGather
  {IR(34702, 17), 286}, // 504 ImageSparseFetch
  {IR(34719, 18), 287}, // 505 ImageSparseGather
  {IR(34829, 16), 293}, // 506 ImageSparseRead
  {IR(34529, 33), 281}, // 507 ImageSparseSampleDrefExplicitLod
  {IR(34496, 33), 280}, // 508 ImageSparseSampleDrefImplicitLod
  {IR(34467, 29), 279}, // 509 ImageSparseSampleExplicitLod
  {IR(34438, 29), 278}, // 510 ImageSparseSampleImplicitLod
  {IR(34665, 37), 285}, // 511 ImageSparseSampleProjDrefExplicitLod
  {IR(34628, 37), 284}, // 512 ImageSparseSampleProjDrefImplicitLod
  {IR(34595, 33), 283}, // 513 ImageSparseSampleProjExplicitLod
  {IR(34562, 33), 282}, // 514 ImageSparseSampleProjImplicitLod
  {IR(34759, 26), 289}, // 515 ImageSparseTexelsResident
  {IR(31013, 18), 53}, // 516 ImageTexelPointer
  {IR(31822, 11), 90}, // 517 ImageWrite
  {IR(31121, 20), 59}, // 518 InBoundsAccessChain
  {IR(31191, 23), 63}, // 519 InBoundsPtrAccessChain
  {IR(32422, 9), 146}, // 520 IsFinite
  {IR(41636, 22), 567}, // 521 IsHelperInvocationEXT
  {IR(32416, 6), 145}, // 522 IsInf
  {IR(32410, 6), 144}, // 523 IsNan
  {IR(37931, 23), 434}, // 524 IsNodePayloadValidAMDX
  {IR(32431, 9), 147}, // 525 IsNormal
  {IR(34351, 13), 273}, // 526 IsValidEvent
  {IR(34006, 17), 257}, // 527 IsValidReserveId
  {IR(33634, 5), 230}, // 528 Kill
  {IR(33551, 6), 226}, // 529 Label
  {IR(32451, 14), 149}, // 530 LessOrGreater
  {IR(33670, 14), 234}, // 531 LifetimeStart
  {IR(33684, 13), 235}, // 532 LifetimeStop
  {IR(29864, 5), 8}, // 533 Line
  {IR(31071, 5), 54}, // 534 Load
  {IR(32522, 11), 155}, // 535 LogicalAnd
  {IR(32483, 13), 152}, // 536 LogicalEqual
  {IR(32533, 11), 156}, // 537 LogicalNot
  {IR(32496, 16), 153}, // 538 LogicalNotEqual
  {IR(32512, 10), 154}, // 539 LogicalOr
  {IR(49678, 17), 789}, // 540 LoopControlINTEL
  {IR(33491, 10), 224}, // 541 LoopMerge
  {IR(52191, 18), 870}, // 542 MaskedGatherINTEL
  {IR(52209, 19), 871}, // 543 MaskedScatterINTEL
  {IR(32320, 18), 135}, // 544 MatrixTimesMatrix
  {IR(32266, 18), 132}, // 545 MatrixTimesScalar
  {IR(32302, 18), 134}, // 546 MatrixTimesVector
  {IR(31251, 15), 65}, // 547 MemberDecorate
  {IR(38107, 20), 443}, // 548 MemberDecorateIdEXT
  {IR(42975, 21), 626}, // 549 MemberDecorateString
  {IR(42996, 27), 626}, // 550 MemberDecorateStringGOOGLE
  {IR(29846, 11), 6}, // 551 MemberName
  {IR(33228, 14), 206}, // 552 MemoryBarrier
  {IR(30011, 12), 12}, // 553 MemoryModel
  {IR(35014, 19), 302}, // 554 MemoryNamedBarrier
  {IR(35033, 16), 303}, // 555 ModuleProcessed
  {IR(29841, 5), 5}, // 556 Name
  {IR(34991, 23), 301}, // 557 NamedBarrierInitialize
  {IR(34785, 7), 290}, // 558 NoLine
  {IR(37904, 27), 433}, // 559 NodePayloadArrayLengthAMDX
  {IR(29637, 4), 0}, // 560 Nop
  {IR(32962, 4), 186}, // 561 Not
  {IR(32465, 8), 150}, // 562 Ordered
  {IR(32338, 13), 136}, // 563 OuterProduct
  {IR(33457, 4), 223}, // 564 Phi
  {IR(38165, 10), 447}, // 565 PoisonKHR
  {IR(51994, 20), 859}, // 566 PredicatedLoadINTEL
  {IR(52014, 21), 860}, // 567 PredicatedStoreINTEL
  {IR(31141, 15), 60}, // 568 PtrAccessChain
  {IR(50113, 30), 804}, // 569 PtrCastToCrossWorkgroupALTERA
  {IR(50143, 29), 804}, // 570 PtrCastToCrossWorkgroupINTEL
  {IR(32095, 17), 111}, // 571 PtrCastToGeneric
  {IR(35881, 8), 343}, // 572 PtrDiff
  {IR(35860, 9), 341}, // 573 PtrEqual
  {IR(35869, 12), 342}, // 574 PtrNotEqual
  {IR(32023, 14), 106}, // 575 QuantizeToF16
  {IR(41941, 17), 578}, // 576 RawAccessChainNV
  {IR(37169, 31), 401}, // 577 RayQueryConfirmIntersectionKHR
  {IR(37137, 32), 400}, // 578 RayQueryGenerateIntersectionKHR
  {IR(40726, 23), 536}, // 579 RayQueryGetClusterIdNV
  {IR(50625, 39), 817}, // 580 RayQueryGetIntersectionBarycentricsKHR
  {IR(50700, 46), 819}, // 581 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(40691, 35), 536}, // 582 RayQueryGetIntersectionClusterIdNV
  {IR(50664, 36), 818}, // 583 RayQueryGetIntersectionFrontFaceKHR
  {IR(50544, 40), 815}, // 584 RayQueryGetIntersectionGeometryIndexKHR
  {IR(50396, 46), 812}, // 585 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(50442, 37), 813}, // 586 RayQueryGetIntersectionInstanceIdKHR
  {IR(50479, 65), 814}, // 587 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(42108, 37), 583}, // 588 RayQueryGetIntersectionLSSHitValueNV
  {IR(42036, 38), 581}, // 589 RayQueryGetIntersectionLSSPositionsNV
  {IR(42074, 34), 582}, // 590 RayQueryGetIntersectionLSSRadiiNV
  {IR(50746, 45), 820}, // 591 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(50791, 42), 821}, // 592 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(50894, 40), 824}, // 593 RayQueryGetIntersectionObjectToWorldKHR
  {IR(50584, 41), 816}, // 594 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(41958, 40), 579}, // 595 RayQueryGetIntersectionSpherePositionNV
  {IR(41998, 38), 580}, // 596 RayQueryGetIntersectionSphereRadiusNV
  {IR(50368, 28), 811}, // 597 RayQueryGetIntersectionTKHR
  {IR(40566, 50), 533}, // 598 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(37219, 31), 403}, // 599 RayQueryGetIntersectionTypeKHR
  {IR(50934, 40), 825}, // 600 RayQueryGetIntersectionWorldToObjectKHR
  {IR(50345, 23), 810}, // 601 RayQueryGetRayFlagsKHR
  {IR(50323, 22), 809}, // 602 RayQueryGetRayTMinKHR
  {IR(50833, 32), 822}, // 603 RayQueryGetWorldRayDirectionKHR
  {IR(50865, 29), 823}, // 604 RayQueryGetWorldRayOriginKHR
  {IR(37094, 22), 398}, // 605 RayQueryInitializeKHR
  {IR(42316, 19), 591}, // 606 RayQueryIsLSSHitNV
  {IR(42294, 22), 590}, // 607 RayQueryIsSphereHitNV
  {IR(37200, 19), 402}, // 608 RayQueryProceedKHR
  {IR(37116, 21), 399}, // 609 RayQueryTerminateKHR
  {IR(37788, 13), 428}, // 610 ReadClockKHR
  {IR(33874, 9), 249}, // 611 ReadPipe
  {IR(50231, 23), 806}, // 612 ReadPipeBlockingALTERA
  {IR(50254, 22), 806}, // 613 ReadPipeBlockingINTEL
  {IR(34322, 13), 271}, // 614 ReleaseEvent
  {IR(39890, 25), 507}, // 615 ReorderThreadWithHintEXT
  {IR(39022, 24), 480}, // 616 ReorderThreadWithHintNV
  {IR(39915, 30), 508}, // 617 ReorderThreadWithHitObjectEXT
  {IR(38993, 29), 479}, // 618 ReorderThreadWithHitObjectNV
  {IR(40448, 22), 527}, // 619 ReportIntersectionKHR
  {IR(40470, 21), 527}, // 620 ReportIntersectionNV
  {IR(33928, 23), 253}, // 621 ReserveReadPipePackets
  {IR(33951, 24), 254}, // 622 ReserveWritePipePackets
  {IR(33893, 17), 251}, // 623 ReservedReadPipe
  {IR(33910, 18), 252}, // 624 ReservedWritePipe
  {IR(48205, 19), 747}, // 625 RestoreMemoryINTEL
  {IR(34310, 12), 270}, // 626 RetainEvent
  {IR(33639, 7), 231}, // 627 Return
  {IR(33646, 12), 232}, // 628 ReturnValue
  {IR(52173, 18), 869}, // 629 RoundFToTF32INTEL
  {IR(32005, 9), 104}, // 630 SConvert
  {IR(32213, 5), 124}, // 631 SDiv
  {IR(36681, 5), 383}, // 632 SDot
  {IR(36722, 11), 386}, // 633 SDotAccSat
  {IR(36733, 14), 386}, // 634 SDotAccSatKHR
  {IR(36686, 8), 383}, // 635 SDotKHR
  {IR(32581, 13), 161}, // 636 SGreaterThan
  {IR(32612, 18), 163}, // 637 SGreaterThanEqual
  {IR(32640, 10), 165}, // 638 SLessThan
  {IR(32665, 15), 167}, // 639 SLessThanEqual
  {IR(32233, 5), 128}, // 640 SMod
  {IR(32389, 13), 141}, // 641 SMulExtended
  {IR(32162, 8), 115}, // 642 SNegate
  {IR(32228, 5), 127}, // 643 SRem
  {IR(36707, 6), 385}, // 644 SUDot
  {IR(36772, 12), 388}, // 645 SUDotAccSat
  {IR(36784, 15), 388}, // 646 SUDotAccSatKHR
  {IR(36713, 9), 385}, // 647 SUDotKHR
  {IR(31489, 13), 77}, // 648 SampledImage
  {IR(41860, 29), 577}, // 649 SamplerImageAddressingModeNV
  {IR(32051, 15), 108}, // 650 SatConvertSToU
  {IR(32066, 15), 109}, // 651 SatConvertUToS
  {IR(48189, 16), 746}, // 652 SaveMemoryINTEL
  {IR(32544, 7), 157}, // 653 Select
  {IR(33536, 15), 225}, // 654 SelectionMerge
  {IR(39342, 18), 490}, // 655 SetMeshOutputsEXT
  {IR(34364, 19), 274}, // 656 SetUserEventStatus
  {IR(32913, 17), 182}, // 657 ShiftLeftLogical
  {IR(32892, 21), 181}, // 658 ShiftRightArithmetic
  {IR(32874, 18), 180}, // 659 ShiftRightLogical
  {IR(32440, 11), 148}, // 660 SignBitSet
  {IR(34845, 7), 294}, // 661 SizeOf
  {IR(29818, 7), 3}, // 662 Source
  {IR(29699, 16), 2}, // 663 SourceContinued
  {IR(29825, 16), 4}, // 664 SourceExtension
  {IR(30836, 13), 45}, // 665 SpecConstant
  {IR(51870, 30), 856}, // 666 SpecConstantArchitectureINTEL
  {IR(51937, 30), 857}, // 667 SpecConstantCapabilitiesINTEL
  {IR(30849, 22), 46}, // 668 SpecConstantComposite
  {IR(51068, 36), 830}, // 669 SpecConstantCompositeContinuedINTEL
  {IR(37013, 34), 395}, // 670 SpecConstantCompositeReplicateEXT
  {IR(38145, 20), 446}, // 671 SpecConstantDataKHR
  {IR(30818, 18), 44}, // 672 SpecConstantFalse
  {IR(30912, 15), 47}, // 673 SpecConstantOp
  {IR(37973, 23), 436}, // 674 SpecConstantStringAMDX
  {IR(51846, 24), 855}, // 675 SpecConstantTargetINTEL
  {IR(30801, 17), 43}, // 676 SpecConstantTrue
  {IR(35935, 25), 346}, // 677 StencilAttachmentReadEXT
  {IR(31076, 6), 55}, // 678 Store
  {IR(29857, 7), 7}, // 679 String
  {IR(51535, 25), 844}, // 680 Subgroup2DBlockLoadINTEL
  {IR(51560, 34), 845}, // 681 Subgroup2DBlockLoadTransformINTEL
  {IR(51594, 34), 846}, // 682 Subgroup2DBlockLoadTransposeINTEL
  {IR(51628, 29), 847}, // 683 Subgroup2DBlockPrefetchINTEL
  {IR(51657, 26), 848}, // 684 Subgroup2DBlockStoreINTEL
  {IR(36433, 20), 373}, // 685 SubgroupAllEqualKHR
  {IR(36403, 15), 371}, // 686 SubgroupAllKHR
  {IR(36418, 15), 372}, // 687 SubgroupAnyKHR
  {IR(46707, 30), 710}, // 688 SubgroupAvcBmeInitializeINTEL
  {IR(46677, 30), 709}, // 689 SubgroupAvcFmeInitializeINTEL
  {IR(45112, 35), 679}, // 690 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(45147, 39), 680}, // 691 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(45788, 38), 693}, // 692 SubgroupAvcImeConvertToMceResultINTEL
  {IR(45411, 45), 686}, // 693 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(45511, 53), 688}, // 694 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(45732, 56), 692}, // 695 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(45620, 54), 690}, // 696 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(45364, 47), 685}, // 697 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(45456, 55), 687}, // 698 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(45674, 58), 691}, // 699 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(45564, 56), 689}, // 700 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(46420, 36), 704}, // 701 SubgroupAvcImeGetBorderReachedINTEL
  {IR(45872, 44), 695}, // 702 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(45826, 46), 694}, // 703 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(46287, 66), 702}, // 704 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(46219, 68), 701}, // 705 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(46353, 67), 703}, // 706 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(46082, 68), 699}, // 707 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(46012, 70), 698}, // 708 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(46150, 69), 700}, // 709 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(46456, 48), 705}, // 710 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(46504, 59), 706}, // 711 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(46621, 56), 708}, // 712 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(46563, 58), 707}, // 713 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(44975, 30), 675}, // 714 SubgroupAvcImeInitializeINTEL
  {IR(45079, 33), 678}, // 715 SubgroupAvcImeRefWindowSizeINTEL
  {IR(45043, 36), 677}, // 716 SubgroupAvcImeSetDualReferenceINTEL
  {IR(45276, 54), 683}, // 717 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(45186, 43), 681}, // 718 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(45005, 38), 676}, // 719 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(45229, 47), 682}, // 720 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(45330, 34), 684}, // 721 SubgroupAvcImeSetWeightedSadINTEL
  {IR(45965, 47), 697}, // 722 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(45916, 49), 696}, // 723 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(44365, 39), 660}, // 724 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(44404, 38), 661}, // 725 SubgroupAvcMceConvertToImeResultINTEL
  {IR(44442, 39), 662}, // 726 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(44481, 38), 663}, // 727 SubgroupAvcMceConvertToRefResultINTEL
  {IR(44519, 39), 664}, // 728 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(44558, 38), 665}, // 729 SubgroupAvcMceConvertToSicResultINTEL
  {IR(44671, 43), 668}, // 730 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(43803, 50), 649}, // 731 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(43401, 60), 641}, // 732 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(43601, 51), 645}, // 733 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(43747, 56), 648}, // 734 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(43514, 47), 643}, // 735 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(44102, 56), 655}, // 736 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(44001, 50), 653}, // 737 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(43696, 51), 647}, // 738 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(43905, 49), 651}, // 739 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(43853, 52), 650}, // 740 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(44051, 51), 654}, // 741 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(44790, 38), 671}, // 742 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(44632, 39), 667}, // 743 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(44714, 38), 669}, // 744 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(44752, 38), 670}, // 745 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(44828, 45), 672}, // 746 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(44873, 40), 673}, // 747 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(44913, 62), 674}, // 748 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(44596, 36), 666}, // 749 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(44158, 33), 656}, // 750 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(44304, 61), 659}, // 751 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(43461, 53), 642}, // 752 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(43652, 44), 646}, // 753 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(43561, 40), 644}, // 754 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(43954, 47), 652}, // 755 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(44243, 61), 658}, // 756 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(44191, 52), 657}, // 757 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(46737, 39), 711}, // 758 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(47059, 38), 718}, // 759 SubgroupAvcRefConvertToMceResultINTEL
  {IR(46912, 45), 715}, // 760 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(46957, 46), 716}, // 761 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(47003, 56), 717}, // 762 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46865, 47), 714}, // 763 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(46776, 46), 712}, // 764 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(46822, 43), 713}, // 765 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(47195, 42), 722}, // 766 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(47159, 36), 721}, // 767 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(47127, 32), 720}, // 768 SubgroupAvcSicConfigureSkcINTEL
  {IR(47276, 39), 724}, // 769 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(47816, 38), 736}, // 770 SubgroupAvcSicConvertToMceResultINTEL
  {IR(47591, 31), 731}, // 771 SubgroupAvcSicEvaluateIpeINTEL
  {IR(47669, 45), 733}, // 772 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(47714, 46), 734}, // 773 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(47760, 56), 735}, // 774 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(47622, 47), 732}, // 775 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(47933, 46), 739}, // 776 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(47889, 44), 738}, // 777 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(48154, 35), 744}, // 778 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(48020, 36), 741}, // 779 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(47854, 35), 737}, // 780 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(47237, 39), 723}, // 781 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(47979, 41), 740}, // 782 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(48056, 50), 742}, // 783 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(48106, 48), 743}, // 784 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(47097, 30), 719}, // 785 SubgroupAvcSicInitializeINTEL
  {IR(47457, 43), 728}, // 786 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(47548, 43), 730}, // 787 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(47407, 50), 727}, // 788 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(47359, 48), 726}, // 789 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(47315, 44), 725}, // 790 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(47500, 48), 729}, // 791 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6756, 18), 364}, // 792 SubgroupBallotKHR
  {IR(16287, 28), 842}, // 793 SubgroupBitcastShuffleINTEL
  {IR(51508, 27), 843}, // 794 SubgroupBlockPrefetchINTEL
  {IR(42407, 23), 596}, // 795 SubgroupBlockReadINTEL
  {IR(42430, 24), 597}, // 796 SubgroupBlockWriteINTEL
  {IR(36270, 27), 365}, // 797 SubgroupFirstInvocationKHR
  {IR(42454, 28), 598}, // 798 SubgroupImageBlockReadINTEL
  {IR(42482, 29), 599}, // 799 SubgroupImageBlockWriteINTEL
  {IR(42511, 33), 600}, // 800 SubgroupImageMediaBlockReadINTEL
  {IR(42544, 34), 601}, // 801 SubgroupImageMediaBlockWriteINTEL
  {IR(16426, 38), 849}, // 802 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(36453, 26), 375}, // 803 SubgroupReadInvocationKHR
  {IR(42335, 25), 593}, // 804 SubgroupShuffleDownINTEL
  {IR(13996, 21), 592}, // 805 SubgroupShuffleINTEL
  {IR(42360, 23), 594}, // 806 SubgroupShuffleUpINTEL
  {IR(42383, 24), 595}, // 807 SubgroupShuffleXorINTEL
  {IR(33627, 7), 229}, // 808 Switch
  {IR(51322, 24), 838}, // 809 TaskSequenceAsyncALTERA
  {IR(51346, 23), 838}, // 810 TaskSequenceAsyncINTEL
  {IR(51273, 25), 837}, // 811 TaskSequenceCreateALTERA
  {IR(51298, 24), 837}, // 812 TaskSequenceCreateINTEL
  {IR(51369, 22), 839}, // 813 TaskSequenceGetALTERA
  {IR(51391, 21), 839}, // 814 TaskSequenceGetINTEL
  {IR(51412, 26), 840}, // 815 TaskSequenceReleaseALTERA
  {IR(51438, 25), 840}, // 816 TaskSequenceReleaseINTEL
  {IR(41678, 27), 569}, // 817 TensorLayoutSetBlockSizeNV
  {IR(41542, 28), 562}, // 818 TensorLayoutSetClampValueNV
  {IR(41471, 27), 559}, // 819 TensorLayoutSetDimensionNV
  {IR(41498, 24), 560}, // 820 TensorLayoutSetStrideNV
  {IR(41522, 20), 561}, // 821 TensorLayoutSliceNV
  {IR(36045, 19), 350}, // 822 TensorQuerySizeARM
  {IR(36016, 14), 348}, // 823 TensorReadARM
  {IR(41658, 20), 568}, // 824 TensorViewSetClipNV
  {IR(41589, 25), 564}, // 825 TensorViewSetDimensionNV
  {IR(41614, 22), 565}, // 826 TensorViewSetStrideNV
  {IR(36030, 15), 349}, // 827 TensorWriteARM
  {IR(36157, 20), 359}, // 828 TerminateInvocation
  {IR(36618, 16), 382}, // 829 TerminateRayKHR
  {IR(40512, 15), 529}, // 830 TerminateRayNV
  {IR(40535, 14), 531}, // 831 TraceMotionNV
  {IR(40527, 8), 530}, // 832 TraceNV
  {IR(36530, 12), 378}, // 833 TraceRayKHR
  {IR(40549, 17), 532}, // 834 TraceRayMotionNV
  {IR(31479, 10), 76}, // 835 Transpose
  {IR(40616, 29), 534}, // 836 TypeAccelerationStructureKHR
  {IR(40645, 28), 534}, // 837 TypeAccelerationStructureNV
  {IR(30428, 10), 25}, // 838 TypeArray
  {IR(43320, 37), 638}, // 839 TypeAvcImeDualReferenceStreaminINTEL
  {IR(43055, 23), 629}, // 840 TypeAvcImePayloadINTEL
  {IR(43237, 44), 636}, // 841 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(43169, 22), 634}, // 842 TypeAvcImeResultINTEL
  {IR(43191, 46), 635}, // 843 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(43281, 39), 637}, // 844 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(43124, 23), 632}, // 845 TypeAvcMcePayloadINTEL
  {IR(43147, 22), 633}, // 846 TypeAvcMceResultINTEL
  {IR(43078, 23), 630}, // 847 TypeAvcRefPayloadINTEL
  {IR(43357, 22), 639}, // 848 TypeAvcRefResultINTEL
  {IR(43101, 23), 631}, // 849 TypeAvcSicPayloadINTEL
  {IR(43379, 22), 640}, // 850 TypeAvcSicResultINTEL
  {IR(30190, 9), 17}, // 851 TypeBool
  {IR(38048, 14), 439}, // 852 TypeBufferEXT
  {IR(50988, 23), 827}, // 853 TypeBufferSurfaceINTEL
  {IR(36799, 25), 389}, // 854 TypeCooperativeMatrixKHR
  {IR(40938, 24), 544}, // 855 TypeCooperativeMatrixNV
  {IR(39101, 24), 483}, // 856 TypeCooperativeVectorNV
  {IR(30543, 16), 32}, // 857 TypeDeviceEvent
  {IR(30533, 10), 31}, // 858 TypeEvent
  {IR(30244, 10), 19}, // 859 TypeFloat
  {IR(30592, 19), 36}, // 860 TypeForwardPointer
  {IR(30520, 13), 30}, // 861 TypeFunction
  {IR(36144, 13), 357}, // 862 TypeGraphARM
  {IR(39873, 17), 506}, // 863 TypeHitObjectEXT
  {IR(39046, 16), 481}, // 864 TypeHitObjectNV
  {IR(30389, 10), 22}, // 865 TypeImage
  {IR(30199, 8), 18}, // 866 TypeInt
  {IR(30265, 11), 21}, // 867 TypeMatrix
  {IR(34974, 17), 300}, // 868 TypeNamedBarrier
  {IR(37850, 25), 431}, // 869 TypeNodePayloadArrayAMDX
  {IR(30466, 11), 28}, // 870 TypeOpaque
  {IR(30583, 9), 35}, // 871 TypePipe
  {IR(34852, 16), 295}, // 872 TypePipeStorage
  {IR(30508, 12), 29}, // 873 TypePointer
  {IR(30573, 10), 34}, // 874 TypeQueue
  {IR(37078, 16), 397}, // 875 TypeRayQueryKHR
  {IR(30559, 14), 33}, // 876 TypeReserveId
  {IR(30438, 17), 26}, // 877 TypeRuntimeArray
  {IR(30411, 17), 24}, // 878 TypeSampledImage
  {IR(30399, 12), 23}, // 879 TypeSampler
  {IR(30455, 11), 27}, // 880 TypeStruct
  {IR(51011, 25), 828}, // 881 TypeStructContinuedINTEL
  {IR(51463, 23), 841}, // 882 TypeTaskSequenceALTERA
  {IR(51486, 22), 841}, // 883 TypeTaskSequenceINTEL
  {IR(35960, 14), 347}, // 884 TypeTensorARM
  {IR(41414, 19), 556}, // 885 TypeTensorLayoutNV
  {IR(41433, 17), 557}, // 886 TypeTensorViewNV
  {IR(36177, 22), 360}, // 887 TypeUntypedPointerKHR
  {IR(30254, 11), 20}, // 888 TypeVector
  {IR(39085, 16), 483}, // 889 TypeVectorIdEXT
  {IR(43037, 18), 628}, // 890 TypeVmeImageINTEL
  {IR(30181, 9), 16}, // 891 TypeVoid
  {IR(42666, 13), 607}, // 892 UAddSatINTEL
  {IR(42693, 14), 609}, // 893 UAverageINTEL
  {IR(42728, 21), 611}, // 894 UAverageRoundedINTEL
  {IR(31996, 9), 103}, // 895 UConvert
  {IR(42578, 24), 602}, // 896 UCountLeadingZerosINTEL
  {IR(42602, 25), 603}, // 897 UCountTrailingZerosINTEL
  {IR(32208, 5), 123}, // 898 UDiv
  {IR(36694, 5), 384}, // 899 UDot
  {IR(36747, 11), 387}, // 900 UDotAccSat
  {IR(36758, 14), 387}, // 901 UDotAccSatKHR
  {IR(36699, 8), 384}, // 902 UDotKHR
  {IR(32568, 13), 160}, // 903 UGreaterThan
  {IR(32594, 18), 162}, // 904 UGreaterThanEqual
  {IR(32630, 10), 164}, // 905 ULessThan
  {IR(32650, 15), 166}, // 906 ULessThanEqual
  {IR(32223, 5), 126}, // 907 UMod
  {IR(42790, 15), 615}, // 908 UMul32x16INTEL
  {IR(32376, 13), 140}, // 909 UMulExtended
  {IR(42762, 13), 613}, // 910 USubSatINTEL
  {IR(29693, 6), 1}, // 911 Undef
  {IR(32473, 10), 151}, // 912 Unordered
  {IR(33658, 12), 233}, // 913 Unreachable
  {IR(36218, 22), 362}, // 914 UntypedAccessChainKHR
  {IR(36355, 22), 368}, // 915 UntypedArrayLengthKHR
  {IR(36505, 25), 377}, // 916 UntypedGroupAsyncCopyKHR
  {IR(38079, 28), 442}, // 917 UntypedImageTexelPointerEXT
  {IR(36240, 30), 363}, // 918 UntypedInBoundsAccessChainKHR
  {IR(36322, 33), 367}, // 919 UntypedInBoundsPtrAccessChainKHR
  {IR(36377, 19), 369}, // 920 UntypedPrefetchKHR
  {IR(36297, 25), 366}, // 921 UntypedPtrAccessChainKHR
  {IR(36199, 19), 361}, // 922 UntypedVariableKHR
  {IR(16492, 32), 851}, // 923 UntypedVariableLengthArrayINTEL
  {IR(31004, 9), 52}, // 924 Variable
  {IR(14452, 25), 745}, // 925 VariableLengthArrayINTEL
  {IR(31361, 21), 69}, // 926 VectorExtractDynamic
  {IR(31382, 20), 70}, // 927 VectorInsertDynamic
  {IR(31402, 14), 71}, // 928 VectorShuffle
  {IR(32284, 18), 133}, // 929 VectorTimesMatrix
  {IR(32248, 18), 131}, // 930 VectorTimesScalar
  {IR(43023, 14), 627}, // 931 VmeImageINTEL
  {IR(39415, 33), 492}, // 932 WritePackedPrimitiveIndices4x8NV
  {IR(33883, 10), 250}, // 933 WritePipe
  {IR(50276, 24), 807}, // 934 WritePipeBlockingALTERA
  {IR(50300, 23), 807}, // 935 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 501> kExtInstByValue{{
    {0,IR(811, 2),IR(52372, 5),IR(1, 0)}, // CALL in arm_experimental_ml_operations
    {0,IR(765, 9),IR(52377, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(765, 9),IR(52385, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(52398, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(52406, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(52420, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(813, 3),IR(52493, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(816, 3),IR(52542, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(819, 2),IR(52597, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(52616, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(52631, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(821, 6),IR(52647, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(52660, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(827, 9),IR(52678, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(836, 9),IR(52730, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(845, 10),IR(52749, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(855, 5),IR(52765, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(52786, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(52807, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(860, 6),IR(52825, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(866, 5),IR(52852, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(871, 5),IR(52887, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(845, 10),IR(52918, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(876, 8),IR(52938, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(884, 11),IR(52963, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(895, 5),IR(52977, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(900, 3),IR(52995, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(903, 2),IR(53026, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(53037, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(905, 3),IR(53050, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(908, 7),IR(53107, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(53126, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(53147, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(53160, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(915, 2),IR(53204, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(747, 1),IR(53219, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(917, 4),IR(53235, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(900, 3),IR(53249, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(53265, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(53271, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(53281, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(53287, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(53292, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(53297, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(53303, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(53309, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(53315, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(53320, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(53326, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(53334, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(53342, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(53346, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(53350, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(53354, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(53359, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(53364, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(53369, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(53374, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(53379, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(53384, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(53390, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(53396, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(53402, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(53408, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(53412, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(53416, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(53420, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(53425, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(53430, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(53435, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(53447, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(53459, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(53473, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(53478, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(53489, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(53494, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(53499, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(53504, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(53509, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(53514, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(53519, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(53526, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(53533, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(53540, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(53545, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(53550, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(53555, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(53566, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(53570, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(53576, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(53588, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(53594, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(53607, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(53620, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(53634, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(53648, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(53661, 15),IR(348, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(53676, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(53692, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(53708, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(53723, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(53738, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(53753, 17),IR(348, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(53770, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(53777, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(53786, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(53792, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(53802, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(53814, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(53822, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(53830, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(53839, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(53848, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(53857, 22),IR(349, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(53879, 20),IR(349, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(53899, 20),IR(349, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(53919, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(53924, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(53929, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5975, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(921, 5),IR(53936, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(593, 5),IR(53949, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(593, 5),IR(53971, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(557, 7),IR(53987, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(557, 7),IR(54012, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(593, 5),IR(54031, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(593, 5),IR(54055, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(593, 5),IR(54076, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(593, 5),IR(54097, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(593, 5),IR(54113, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(54131, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(54157, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(54182, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(54202, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(54227, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(54257, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(54280, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(54305, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(54331, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(54361, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(54387, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(9492, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(54407, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(54437, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(593, 5),IR(54465, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(557, 7),IR(54493, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(54516, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(54551, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(54589, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(54631, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(535, 6),IR(54676, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(535, 6),IR(54713, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(593, 5),IR(54753, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(593, 5),IR(54780, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(54807, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(54839, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(54879, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(54890, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(54916, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(54948, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(54982, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {1,IR(168, 2),IR(55004, 12),IR(1, 0)}, // DebugPrintf in nonsemantic_debugprintf
    {1,IR(36, 2),IR(55016, 11),IR(1, 0)}, // DebugGraph in nonsemantic_graph_debuginfo
    {2,IR(375, 3),IR(53204, 15),IR(1, 0)}, // DebugOperation in nonsemantic_graph_debuginfo
    {3,IR(926, 3),IR(55027, 12),IR(1, 0)}, // DebugTensor in nonsemantic_graph_debuginfo
    {0,IR(0, 0),IR(52406, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {1,IR(296, 4),IR(52420, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {2,IR(593, 5),IR(52493, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {3,IR(33, 3),IR(52542, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {4,IR(36, 2),IR(52597, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {5,IR(168, 2),IR(52616, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {6,IR(36, 2),IR(52631, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {7,IR(535, 6),IR(52647, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo
    {8,IR(375, 3),IR(52660, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {9,IR(929, 9),IR(52678, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {10,IR(938, 10),IR(52730, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {11,IR(948, 9),IR(52749, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {12,IR(296, 4),IR(52765, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {13,IR(36, 2),IR(52786, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {14,IR(168, 2),IR(52807, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {15,IR(535, 6),IR(52825, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {16,IR(675, 5),IR(52852, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {17,IR(957, 5),IR(52887, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {18,IR(962, 10),IR(52918, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {19,IR(471, 8),IR(52938, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {20,IR(962, 10),IR(52963, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo
    {21,IR(593, 5),IR(52977, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {22,IR(33, 3),IR(52995, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {23,IR(903, 2),IR(53026, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo
    {24,IR(0, 0),IR(53037, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo
    {25,IR(926, 3),IR(53050, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {26,IR(972, 8),IR(53107, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {27,IR(36, 2),IR(53126, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {28,IR(980, 4),IR(53147, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo
    {29,IR(980, 4),IR(53160, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo
    {30,IR(168, 2),IR(53204, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo
    {31,IR(747, 1),IR(53219, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo
    {32,IR(580, 4),IR(53235, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {33,IR(33, 3),IR(53249, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {34,IR(516, 7),IR(55039, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {35,IR(903, 2),IR(55059, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo
    {101,IR(36, 2),IR(55071, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {102,IR(10, 1),IR(55095, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {103,IR(675, 5),IR(55116, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo
    {104,IR(0, 0),IR(55126, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo
    {105,IR(36, 2),IR(55138, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {106,IR(10, 1),IR(55159, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {107,IR(296, 4),IR(55176, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {108,IR(33, 3),IR(55192, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {109,IR(36, 2),IR(55208, 21),IR(1, 0)}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {110,IR(675, 5),IR(55229, 30),IR(1, 0)}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {1,IR(765, 9),IR(55259, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(55273, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(55286, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(55298, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(55312, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(984, 15),IR(55335, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(999, 33),IR(55355, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1032, 19),IR(55374, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(52406, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1051, 4),IR(52420, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1055, 3),IR(52493, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1058, 3),IR(52542, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1061, 2),IR(52597, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(52616, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(52631, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(821, 6),IR(52647, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1063, 3),IR(52660, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1066, 9),IR(52678, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1075, 10),IR(52730, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1085, 10),IR(52749, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1095, 5),IR(52765, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(52786, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(52807, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(860, 6),IR(52825, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(866, 5),IR(52852, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(871, 5),IR(52887, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1085, 10),IR(52918, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1100, 8),IR(52938, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1108, 11),IR(52963, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(895, 5),IR(52977, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(900, 3),IR(52995, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(903, 2),IR(53026, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(53037, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(905, 3),IR(53050, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1119, 8),IR(53107, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(53126, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(53147, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(980, 4),IR(53160, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1127, 2),IR(53204, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(747, 1),IR(53219, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(917, 4),IR(53235, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(900, 3),IR(53249, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1129, 7),IR(55039, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(903, 2),IR(55059, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1136, 8),IR(55695, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(55712, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(55717, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(55723, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(55730, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(55735, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(55741, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(55748, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(55753, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(55759, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(55765, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(55772, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(55780, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(55785, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(55790, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(55799, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(55803, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(55808, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(55814, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(55819, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(55823, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(55827, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(55832, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(55838, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(55844, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(55849, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(55854, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(55860, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(55864, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(55869, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(55874, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(55879, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(55885, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(55891, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(55897, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(55903, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(55909, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(55916, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(55925, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(55929, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(55934, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(55940, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(55946, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(55951, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(55955, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(55962, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(55969, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(55974, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(55978, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(55988, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(55992, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(55997, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(56002, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(56012, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(56019, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(56024, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(56030, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(56036, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(56042, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(56046, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(56053, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(56058, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(56064, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(56069, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(56073, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(56078, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(56084, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(56091, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(56097, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(56106, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(56118, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(56127, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(56137, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(56148, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(56157, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(56167, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(56178, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(56188, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(56199, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(56210, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(56219, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(56229, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(56238, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(56249, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(56263, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(56274, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(56286, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(56299, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(56310, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(56322, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(56335, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(56347, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(56360, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(56373, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(56384, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(56396, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(56407, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(56414, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(56422, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(56434, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(56446, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(56450, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(56458, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(56463, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(56474, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(56479, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(56485, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(56494, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(56501, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(56511, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(56525, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(56537, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(56552, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(56558, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(56569, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(56579, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(56589, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(56596, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(56603, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(56611, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(56619, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(56627, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(56635, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(56639, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(56643, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(56652, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(56662, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(56672, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(56678, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(56684, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(56690, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(56696, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(56705, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(56712, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(56722, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(56732, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(56743, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(56754, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(56763, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(56771, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(56779, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(56787, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1144, 3),IR(56795, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(56802, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(56810, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1144, 3),IR(56821, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(56833, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1147, 4),IR(56845, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(56859, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1147, 4),IR(56872, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1144, 3),IR(56887, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(56900, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1147, 4),IR(56914, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(56930, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(56938, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(56947, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(56954, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(56963, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(56973, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(56980, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(56986, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(56997, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(57006, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(57015, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(57032, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(57049, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(57057, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(57079, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(57107, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(57126, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(57135, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(57158, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(57167, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(57176, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(57185, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(57194, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(57203, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(57212, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(57221, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(57230, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(57239, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(516, 7),IR(57246, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(755, 10),IR(57257, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(755, 10),IR(57264, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(755, 10),IR(57271, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(57288, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(57294, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(675, 5),IR(57301, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(57312, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(765, 9),IR(57319, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(57336, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(57342, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(57346, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(57354, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(57359, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(57363, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(57386, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(57398, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(57409, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(57421, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(57428, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(57440, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(57459, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(57479, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(57490, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(57502, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(57510, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(57518, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(57522, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(57526, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(57530, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(57536, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(57540, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(57552, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(57557, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(57561, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(57565, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(57569, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(57575, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(57579, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(57591, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(57598, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(57609, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(57615, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(57619, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(57626, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(57632, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(57640, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(57654, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(57665, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(57676, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(57687, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(57698, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(57713, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(57724, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(57731, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(57735, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(57743, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(57751, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(57757, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(57762, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(57772, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(57779, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(675, 5),IR(57787, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(57794, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(755, 10),IR(57799, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
}};

IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_EXPERIMENTAL_ML_OPERATIONS: return IR(0, 1);
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(1, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(4, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(38, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(119, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_DEBUGPRINTF: return IR(161, 1);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_GRAPH_DEBUGINFO: return IR(162, 3);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(165, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(211, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(219, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(256, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(418, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(421, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(425, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(426, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(435, 66);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 501> kExtInstNames{{
    {IR(52372, 5), 0}, // CALL in arm_experimental_ml_operations
    {IR(52377, 8), 1}, // MIN_SAD in arm_motion_engine_100
    {IR(52385, 13), 2}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(52398, 8), 3}, // RAW_SAD in arm_motion_engine_100
    {IR(52420, 21), 5}, // DebugCompilationUnit in debuginfo
    {IR(53147, 13), 32}, // DebugDeclare in debuginfo
    {IR(53219, 16), 35}, // DebugExpression in debuginfo
    {IR(52963, 14), 24}, // DebugFunction in debuginfo
    {IR(52938, 25), 23}, // DebugFunctionDeclaration in debuginfo
    {IR(52918, 20), 22}, // DebugGlobalVariable in debuginfo
    {IR(52406, 14), 4}, // DebugInfoNone in debuginfo
    {IR(53050, 15), 29}, // DebugInlinedAt in debuginfo
    {IR(53126, 21), 31}, // DebugInlinedVariable in debuginfo
    {IR(52977, 18), 25}, // DebugLexicalBlock in debuginfo
    {IR(52995, 31), 26}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(53107, 19), 30}, // DebugLocalVariable in debuginfo
    {IR(53235, 14), 36}, // DebugMacroDef in debuginfo
    {IR(53249, 16), 37}, // DebugMacroUndef in debuginfo
    {IR(53037, 13), 28}, // DebugNoScope in debuginfo
    {IR(53204, 15), 34}, // DebugOperation in debuginfo
    {IR(53026, 11), 27}, // DebugScope in debuginfo
    {IR(52616, 15), 9}, // DebugTypeArray in debuginfo
    {IR(52493, 15), 6}, // DebugTypeBasic in debuginfo
    {IR(52730, 19), 14}, // DebugTypeComposite in debuginfo
    {IR(52678, 14), 13}, // DebugTypeEnum in debuginfo
    {IR(52660, 18), 12}, // DebugTypeFunction in debuginfo
    {IR(52765, 21), 16}, // DebugTypeInheritance in debuginfo
    {IR(52749, 16), 15}, // DebugTypeMember in debuginfo
    {IR(52542, 17), 7}, // DebugTypePointer in debuginfo
    {IR(52786, 21), 17}, // DebugTypePtrToMember in debuginfo
    {IR(52597, 19), 8}, // DebugTypeQualifier in debuginfo
    {IR(52807, 18), 18}, // DebugTypeTemplate in debuginfo
    {IR(52825, 27), 19}, // DebugTypeTemplateParameter in debuginfo
    {IR(52887, 31), 21}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(52852, 35), 20}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(52631, 16), 10}, // DebugTypeVector in debuginfo
    {IR(52647, 13), 11}, // DebugTypedef in debuginfo
    {IR(53160, 11), 33}, // DebugValue in debuginfo
    {IR(53359, 5), 54}, // Acos in glsl_std_450
    {IR(53390, 6), 60}, // Acosh in glsl_std_450
    {IR(53354, 5), 53}, // Asin in glsl_std_450
    {IR(53384, 6), 59}, // Asinh in glsl_std_450
    {IR(53364, 5), 55}, // Atan in glsl_std_450
    {IR(53402, 6), 62}, // Atan2 in glsl_std_450
    {IR(53396, 6), 61}, // Atanh in glsl_std_450
    {IR(53315, 5), 46}, // Ceil in glsl_std_450
    {IR(53346, 4), 51}, // Cos in glsl_std_450
    {IR(53374, 5), 57}, // Cosh in glsl_std_450
    {IR(53786, 6), 105}, // Cross in glsl_std_450
    {IR(53334, 8), 49}, // Degrees in glsl_std_450
    {IR(53447, 12), 70}, // Determinant in glsl_std_450
    {IR(53777, 9), 104}, // Distance in glsl_std_450
    {IR(53412, 4), 64}, // Exp in glsl_std_450
    {IR(53420, 5), 66}, // Exp2 in glsl_std_450
    {IR(53287, 5), 41}, // FAbs in glsl_std_450
    {IR(53519, 7), 80}, // FClamp in glsl_std_450
    {IR(53504, 5), 77}, // FMax in glsl_std_450
    {IR(53489, 5), 74}, // FMin in glsl_std_450
    {IR(53540, 5), 83}, // FMix in glsl_std_450
    {IR(53297, 6), 43}, // FSign in glsl_std_450
    {IR(53802, 12), 107}, // FaceForward in glsl_std_450
    {IR(53830, 9), 110}, // FindILsb in glsl_std_450
    {IR(53839, 9), 111}, // FindSMsb in glsl_std_450
    {IR(53848, 9), 112}, // FindUMsb in glsl_std_450
    {IR(53309, 6), 45}, // Floor in glsl_std_450
    {IR(53566, 4), 87}, // Fma in glsl_std_450
    {IR(53320, 6), 47}, // Fract in glsl_std_450
    {IR(53570, 6), 88}, // Frexp in glsl_std_450
    {IR(53576, 12), 89}, // FrexpStruct in glsl_std_450
    {IR(53545, 5), 84}, // IMix in glsl_std_450
    {IR(53857, 22), 113}, // InterpolateAtCentroid in glsl_std_450
    {IR(53899, 20), 115}, // InterpolateAtOffset in glsl_std_450
    {IR(53879, 20), 114}, // InterpolateAtSample in glsl_std_450
    {IR(53435, 12), 69}, // InverseSqrt in glsl_std_450
    {IR(53588, 6), 90}, // Ldexp in glsl_std_450
    {IR(53770, 7), 103}, // Length in glsl_std_450
    {IR(53416, 4), 65}, // Log in glsl_std_450
    {IR(53425, 5), 67}, // Log2 in glsl_std_450
    {IR(53459, 14), 71}, // MatrixInverse in glsl_std_450
    {IR(53473, 5), 72}, // Modf in glsl_std_450
    {IR(53478, 11), 73}, // ModfStruct in glsl_std_450
    {IR(53929, 7), 118}, // NClamp in glsl_std_450
    {IR(53924, 5), 117}, // NMax in glsl_std_450
    {IR(53919, 5), 116}, // NMin in glsl_std_450
    {IR(53792, 10), 106}, // Normalize in glsl_std_450
    {IR(53661, 15), 96}, // PackDouble2x32 in glsl_std_450
    {IR(53648, 13), 95}, // PackHalf2x16 in glsl_std_450
    {IR(53620, 14), 93}, // PackSnorm2x16 in glsl_std_450
    {IR(53594, 13), 91}, // PackSnorm4x8 in glsl_std_450
    {IR(53634, 14), 94}, // PackUnorm2x16 in glsl_std_450
    {IR(53607, 13), 92}, // PackUnorm4x8 in glsl_std_450
    {IR(53408, 4), 63}, // Pow in glsl_std_450
    {IR(53326, 8), 48}, // Radians in glsl_std_450
    {IR(53814, 8), 108}, // Reflect in glsl_std_450
    {IR(53822, 8), 109}, // Refract in glsl_std_450
    {IR(53265, 6), 38}, // Round in glsl_std_450
    {IR(53271, 10), 39}, // RoundEven in glsl_std_450
    {IR(53292, 5), 42}, // SAbs in glsl_std_450
    {IR(53533, 7), 82}, // SClamp in glsl_std_450
    {IR(53514, 5), 79}, // SMax in glsl_std_450
    {IR(53499, 5), 76}, // SMin in glsl_std_450
    {IR(53303, 6), 44}, // SSign in glsl_std_450
    {IR(53342, 4), 50}, // Sin in glsl_std_450
    {IR(53369, 5), 56}, // Sinh in glsl_std_450
    {IR(53555, 11), 86}, // SmoothStep in glsl_std_450
    {IR(53430, 5), 68}, // Sqrt in glsl_std_450
    {IR(53550, 5), 85}, // Step in glsl_std_450
    {IR(53350, 4), 52}, // Tan in glsl_std_450
    {IR(53379, 5), 58}, // Tanh in glsl_std_450
    {IR(53281, 6), 40}, // Trunc in glsl_std_450
    {IR(53526, 7), 81}, // UClamp in glsl_std_450
    {IR(53509, 5), 78}, // UMax in glsl_std_450
    {IR(53494, 5), 75}, // UMin in glsl_std_450
    {IR(53753, 17), 102}, // UnpackDouble2x32 in glsl_std_450
    {IR(53708, 15), 99}, // UnpackHalf2x16 in glsl_std_450
    {IR(53676, 16), 97}, // UnpackSnorm2x16 in glsl_std_450
    {IR(53723, 15), 100}, // UnpackSnorm4x8 in glsl_std_450
    {IR(53692, 16), 98}, // UnpackUnorm2x16 in glsl_std_450
    {IR(53738, 15), 101}, // UnpackUnorm4x8 in glsl_std_450
    {IR(53936, 13), 120}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(54031, 24), 125}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(53987, 25), 123}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(54012, 19), 124}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(54465, 28), 144}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(54493, 23), 145}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(54055, 21), 126}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(54097, 16), 128}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(53949, 22), 121}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(54076, 21), 127}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(54753, 27), 152}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(53971, 16), 122}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(54780, 27), 153}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(54113, 18), 129}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(54807, 32), 154}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(54361, 26), 139}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(54387, 20), 140}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(54631, 45), 149}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(54713, 40), 151}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(54589, 42), 148}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(54676, 37), 150}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5975, 7), 119}, // Kernel in nonsemantic_clspvreflection
    {IR(9492, 15), 141}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(54948, 34), 159}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(54916, 32), 158}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(54890, 26), 157}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(54879, 11), 156}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(54839, 40), 155}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(54551, 38), 147}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(54516, 35), 146}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(54407, 30), 142}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(54227, 30), 134}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(54202, 25), 133}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54257, 23), 135}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(54305, 26), 137}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(54331, 30), 138}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(54280, 25), 136}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(54157, 25), 131}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54437, 28), 143}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(54182, 20), 132}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(54131, 26), 130}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(54982, 22), 160}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(55004, 12), 161}, // DebugPrintf in nonsemantic_debugprintf
    {IR(55016, 11), 162}, // DebugGraph in nonsemantic_graph_debuginfo
    {IR(53204, 15), 163}, // DebugOperation in nonsemantic_graph_debuginfo
    {IR(55027, 12), 164}, // DebugTensor in nonsemantic_graph_debuginfo
    {IR(55138, 21), 205}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {IR(52420, 21), 166}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {IR(53147, 13), 193}, // DebugDeclare in nonsemantic_shader_debuginfo
    {IR(55176, 16), 207}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {IR(53219, 16), 196}, // DebugExpression in nonsemantic_shader_debuginfo
    {IR(52963, 14), 185}, // DebugFunction in nonsemantic_shader_debuginfo
    {IR(52938, 25), 184}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {IR(55071, 24), 201}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {IR(52918, 20), 183}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {IR(55039, 20), 199}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {IR(52406, 14), 165}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {IR(53050, 15), 190}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {IR(53126, 21), 192}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {IR(52977, 18), 186}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {IR(52995, 31), 187}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {IR(55116, 10), 203}, // DebugLine in nonsemantic_shader_debuginfo
    {IR(53107, 19), 191}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {IR(53235, 14), 197}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {IR(53249, 16), 198}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {IR(55126, 12), 204}, // DebugNoLine in nonsemantic_shader_debuginfo
    {IR(53037, 13), 189}, // DebugNoScope in nonsemantic_shader_debuginfo
    {IR(53204, 15), 195}, // DebugOperation in nonsemantic_shader_debuginfo
    {IR(53026, 11), 188}, // DebugScope in nonsemantic_shader_debuginfo
    {IR(55059, 12), 200}, // DebugSource in nonsemantic_shader_debuginfo
    {IR(55095, 21), 202}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {IR(55159, 17), 206}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {IR(52616, 15), 170}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {IR(52493, 15), 167}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {IR(52730, 19), 175}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {IR(55229, 30), 210}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {IR(52678, 14), 174}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {IR(52660, 18), 173}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {IR(52765, 21), 177}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {IR(55192, 16), 208}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {IR(52749, 16), 176}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {IR(52542, 17), 168}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {IR(52786, 21), 178}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {IR(52597, 19), 169}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {IR(52807, 18), 179}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {IR(52825, 27), 180}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52887, 31), 182}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {IR(52852, 35), 181}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52631, 16), 171}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {IR(55208, 21), 209}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {IR(52647, 13), 172}, // DebugTypedef in nonsemantic_shader_debuginfo
    {IR(53160, 11), 194}, // DebugValue in nonsemantic_shader_debuginfo
    {IR(55259, 14), 211}, // Configuration in nonsemantic_vkspreflection
    {IR(55335, 20), 216}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(55355, 19), 217}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(55374, 21), 218}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(55298, 14), 214}, // PushConstants in nonsemantic_vkspreflection
    {IR(55312, 23), 215}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(55273, 13), 212}, // StartCounter in nonsemantic_vkspreflection
    {IR(55286, 12), 213}, // StopCounter in nonsemantic_vkspreflection
    {IR(52420, 21), 220}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(53147, 13), 247}, // DebugDeclare in opencl_debuginfo_100
    {IR(53219, 16), 250}, // DebugExpression in opencl_debuginfo_100
    {IR(52963, 14), 239}, // DebugFunction in opencl_debuginfo_100
    {IR(52938, 25), 238}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(52918, 20), 237}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(55039, 20), 253}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(52406, 14), 219}, // DebugInfoNone in opencl_debuginfo_100
    {IR(53050, 15), 244}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(53126, 21), 246}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(52977, 18), 240}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(52995, 31), 241}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(53107, 19), 245}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(53235, 14), 251}, // DebugMacroDef in opencl_debuginfo_100
    {IR(53249, 16), 252}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(55695, 17), 255}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(53037, 13), 243}, // DebugNoScope in opencl_debuginfo_100
    {IR(53204, 15), 249}, // DebugOperation in opencl_debuginfo_100
    {IR(53026, 11), 242}, // DebugScope in opencl_debuginfo_100
    {IR(55059, 12), 254}, // DebugSource in opencl_debuginfo_100
    {IR(52616, 15), 224}, // DebugTypeArray in opencl_debuginfo_100
    {IR(52493, 15), 221}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(52730, 19), 229}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(52678, 14), 228}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(52660, 18), 227}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(52765, 21), 231}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(52749, 16), 230}, // DebugTypeMember in opencl_debuginfo_100
    {IR(52542, 17), 222}, // DebugTypePointer in opencl_debuginfo_100
    {IR(52786, 21), 232}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(52597, 19), 223}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(52807, 18), 233}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(52825, 27), 234}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(52887, 31), 236}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(52852, 35), 235}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(52631, 16), 225}, // DebugTypeVector in opencl_debuginfo_100
    {IR(52647, 13), 226}, // DebugTypedef in opencl_debuginfo_100
    {IR(53160, 11), 248}, // DebugValue in opencl_debuginfo_100
    {IR(55712, 5), 256}, // acos in opencl_std_100
    {IR(55717, 6), 257}, // acosh in opencl_std_100
    {IR(55723, 7), 258}, // acospi in opencl_std_100
    {IR(55730, 5), 259}, // asin in opencl_std_100
    {IR(55735, 6), 260}, // asinh in opencl_std_100
    {IR(55741, 7), 261}, // asinpi in opencl_std_100
    {IR(55748, 5), 262}, // atan in opencl_std_100
    {IR(55753, 6), 263}, // atan2 in opencl_std_100
    {IR(55772, 8), 266}, // atan2pi in opencl_std_100
    {IR(55759, 6), 264}, // atanh in opencl_std_100
    {IR(55765, 7), 265}, // atanpi in opencl_std_100
    {IR(56963, 10), 412}, // bitselect in opencl_std_100
    {IR(55780, 5), 267}, // cbrt in opencl_std_100
    {IR(55785, 5), 268}, // ceil in opencl_std_100
    {IR(56635, 4), 377}, // clz in opencl_std_100
    {IR(55790, 9), 269}, // copysign in opencl_std_100
    {IR(55799, 4), 270}, // cos in opencl_std_100
    {IR(55803, 5), 271}, // cosh in opencl_std_100
    {IR(55808, 6), 272}, // cospi in opencl_std_100
    {IR(56479, 6), 360}, // cross in opencl_std_100
    {IR(56639, 4), 378}, // ctz in opencl_std_100
    {IR(56414, 8), 352}, // degrees in opencl_std_100
    {IR(56485, 9), 361}, // distance in opencl_std_100
    {IR(55819, 4), 274}, // erf in opencl_std_100
    {IR(55814, 5), 273}, // erfc in opencl_std_100
    {IR(55823, 4), 275}, // exp in opencl_std_100
    {IR(55832, 6), 277}, // exp10 in opencl_std_100
    {IR(55827, 5), 276}, // exp2 in opencl_std_100
    {IR(55838, 6), 278}, // expm1 in opencl_std_100
    {IR(55844, 5), 279}, // fabs in opencl_std_100
    {IR(56511, 14), 364}, // fast_distance in opencl_std_100
    {IR(56525, 12), 365}, // fast_length in opencl_std_100
    {IR(56537, 15), 366}, // fast_normalize in opencl_std_100
    {IR(56407, 7), 351}, // fclamp in opencl_std_100
    {IR(55849, 5), 280}, // fdim in opencl_std_100
    {IR(55854, 6), 281}, // floor in opencl_std_100
    {IR(55860, 4), 282}, // fma in opencl_std_100
    {IR(55864, 5), 283}, // fmax in opencl_std_100
    {IR(56422, 12), 353}, // fmax_common in opencl_std_100
    {IR(55869, 5), 284}, // fmin in opencl_std_100
    {IR(56434, 12), 354}, // fmin_common in opencl_std_100
    {IR(55874, 5), 285}, // fmod in opencl_std_100
    {IR(55879, 6), 286}, // fract in opencl_std_100
    {IR(55885, 6), 287}, // frexp in opencl_std_100
    {IR(56097, 9), 323}, // half_cos in opencl_std_100
    {IR(56106, 12), 324}, // half_divide in opencl_std_100
    {IR(56118, 9), 325}, // half_exp in opencl_std_100
    {IR(56137, 11), 327}, // half_exp10 in opencl_std_100
    {IR(56127, 10), 326}, // half_exp2 in opencl_std_100
    {IR(56148, 9), 328}, // half_log in opencl_std_100
    {IR(56167, 11), 330}, // half_log10 in opencl_std_100
    {IR(56157, 10), 329}, // half_log2 in opencl_std_100
    {IR(56178, 10), 331}, // half_powr in opencl_std_100
    {IR(56188, 11), 332}, // half_recip in opencl_std_100
    {IR(56199, 11), 333}, // half_rsqrt in opencl_std_100
    {IR(56210, 9), 334}, // half_sin in opencl_std_100
    {IR(56219, 10), 335}, // half_sqrt in opencl_std_100
    {IR(56229, 9), 336}, // half_tan in opencl_std_100
    {IR(55891, 6), 288}, // hypot in opencl_std_100
    {IR(55897, 6), 289}, // ilogb in opencl_std_100
    {IR(55903, 6), 290}, // ldexp in opencl_std_100
    {IR(56494, 7), 362}, // length in opencl_std_100
    {IR(55909, 7), 291}, // lgamma in opencl_std_100
    {IR(55916, 9), 292}, // lgamma_r in opencl_std_100
    {IR(55925, 4), 293}, // log in opencl_std_100
    {IR(55934, 6), 295}, // log10 in opencl_std_100
    {IR(55940, 6), 296}, // log1p in opencl_std_100
    {IR(55929, 5), 294}, // log2 in opencl_std_100
    {IR(55946, 5), 297}, // logb in opencl_std_100
    {IR(55951, 4), 298}, // mad in opencl_std_100
    {IR(55955, 7), 299}, // maxmag in opencl_std_100
    {IR(55962, 7), 300}, // minmag in opencl_std_100
    {IR(56446, 4), 355}, // mix in opencl_std_100
    {IR(55969, 5), 301}, // modf in opencl_std_100
    {IR(55974, 4), 302}, // nan in opencl_std_100
    {IR(56238, 11), 337}, // native_cos in opencl_std_100
    {IR(56249, 14), 338}, // native_divide in opencl_std_100
    {IR(56263, 11), 339}, // native_exp in opencl_std_100
    {IR(56286, 13), 341}, // native_exp10 in opencl_std_100
    {IR(56274, 12), 340}, // native_exp2 in opencl_std_100
    {IR(56299, 11), 342}, // native_log in opencl_std_100
    {IR(56322, 13), 344}, // native_log10 in opencl_std_100
    {IR(56310, 12), 343}, // native_log2 in opencl_std_100
    {IR(56335, 12), 345}, // native_powr in opencl_std_100
    {IR(56347, 13), 346}, // native_recip in opencl_std_100
    {IR(56360, 13), 347}, // native_rsqrt in opencl_std_100
    {IR(56373, 11), 348}, // native_sin in opencl_std_100
    {IR(56384, 12), 349}, // native_sqrt in opencl_std_100
    {IR(56396, 11), 350}, // native_tan in opencl_std_100
    {IR(55978, 10), 303}, // nextafter in opencl_std_100
    {IR(56501, 10), 363}, // normalize in opencl_std_100
    {IR(56754, 9), 392}, // popcount in opencl_std_100
    {IR(55988, 4), 304}, // pow in opencl_std_100
    {IR(55992, 5), 305}, // pown in opencl_std_100
    {IR(55997, 5), 306}, // powr in opencl_std_100
    {IR(56954, 9), 411}, // prefetch in opencl_std_100
    {IR(56947, 7), 410}, // printf in opencl_std_100
    {IR(56450, 8), 356}, // radians in opencl_std_100
    {IR(56002, 10), 307}, // remainder in opencl_std_100
    {IR(56012, 7), 308}, // remquo in opencl_std_100
    {IR(56019, 5), 309}, // rint in opencl_std_100
    {IR(56024, 6), 310}, // rootn in opencl_std_100
    {IR(56705, 7), 387}, // rotate in opencl_std_100
    {IR(56030, 6), 311}, // round in opencl_std_100
    {IR(56036, 6), 312}, // rsqrt in opencl_std_100
    {IR(56552, 6), 367}, // s_abs in opencl_std_100
    {IR(56558, 11), 368}, // s_abs_diff in opencl_std_100
    {IR(56569, 10), 369}, // s_add_sat in opencl_std_100
    {IR(56619, 8), 375}, // s_clamp in opencl_std_100
    {IR(56589, 7), 371}, // s_hadd in opencl_std_100
    {IR(56763, 8), 393}, // s_mad24 in opencl_std_100
    {IR(56643, 9), 379}, // s_mad_hi in opencl_std_100
    {IR(56662, 10), 381}, // s_mad_sat in opencl_std_100
    {IR(56672, 6), 382}, // s_max in opencl_std_100
    {IR(56684, 6), 384}, // s_min in opencl_std_100
    {IR(56779, 8), 395}, // s_mul24 in opencl_std_100
    {IR(56696, 9), 386}, // s_mul_hi in opencl_std_100
    {IR(56603, 8), 373}, // s_rhadd in opencl_std_100
    {IR(56712, 10), 388}, // s_sub_sat in opencl_std_100
    {IR(56743, 11), 391}, // s_upsample in opencl_std_100
    {IR(56973, 7), 413}, // select in opencl_std_100
    {IR(56930, 8), 408}, // shuffle in opencl_std_100
    {IR(56938, 9), 409}, // shuffle2 in opencl_std_100
    {IR(56474, 5), 359}, // sign in opencl_std_100
    {IR(56042, 4), 313}, // sin in opencl_std_100
    {IR(56046, 7), 314}, // sincos in opencl_std_100
    {IR(56053, 5), 315}, // sinh in opencl_std_100
    {IR(56058, 6), 316}, // sinpi in opencl_std_100
    {IR(56463, 11), 358}, // smoothstep in opencl_std_100
    {IR(56064, 5), 317}, // sqrt in opencl_std_100
    {IR(56458, 5), 357}, // step in opencl_std_100
    {IR(56069, 4), 318}, // tan in opencl_std_100
    {IR(56073, 5), 319}, // tanh in opencl_std_100
    {IR(56078, 6), 320}, // tanpi in opencl_std_100
    {IR(56084, 7), 321}, // tgamma in opencl_std_100
    {IR(56091, 6), 322}, // trunc in opencl_std_100
    {IR(56980, 6), 414}, // u_abs in opencl_std_100
    {IR(56986, 11), 415}, // u_abs_diff in opencl_std_100
    {IR(56579, 10), 370}, // u_add_sat in opencl_std_100
    {IR(56627, 8), 376}, // u_clamp in opencl_std_100
    {IR(56596, 7), 372}, // u_hadd in opencl_std_100
    {IR(56771, 8), 394}, // u_mad24 in opencl_std_100
    {IR(57006, 9), 417}, // u_mad_hi in opencl_std_100
    {IR(56652, 10), 380}, // u_mad_sat in opencl_std_100
    {IR(56678, 6), 383}, // u_max in opencl_std_100
    {IR(56690, 6), 385}, // u_min in opencl_std_100
    {IR(56787, 8), 396}, // u_mul24 in opencl_std_100
    {IR(56997, 9), 416}, // u_mul_hi in opencl_std_100
    {IR(56611, 8), 374}, // u_rhadd in opencl_std_100
    {IR(56722, 10), 389}, // u_sub_sat in opencl_std_100
    {IR(56732, 11), 390}, // u_upsample in opencl_std_100
    {IR(56810, 11), 399}, // vload_half in opencl_std_100
    {IR(56821, 12), 400}, // vload_halfn in opencl_std_100
    {IR(56887, 13), 405}, // vloada_halfn in opencl_std_100
    {IR(56795, 7), 397}, // vloadn in opencl_std_100
    {IR(56833, 12), 401}, // vstore_half in opencl_std_100
    {IR(56845, 14), 402}, // vstore_half_r in opencl_std_100
    {IR(56859, 13), 403}, // vstore_halfn in opencl_std_100
    {IR(56872, 15), 404}, // vstore_halfn_r in opencl_std_100
    {IR(56900, 14), 406}, // vstorea_halfn in opencl_std_100
    {IR(56914, 16), 407}, // vstorea_halfn_r in opencl_std_100
    {IR(56802, 8), 398}, // vstoren in opencl_std_100
    {IR(57032, 17), 419}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(57015, 17), 418}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(57049, 8), 420}, // TimeAMD in spv_amd_gcn_shader
    {IR(57126, 9), 424}, // MbcntAMD in spv_amd_shader_ballot
    {IR(57057, 22), 421}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(57079, 28), 422}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(57107, 19), 423}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(57135, 23), 425}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(57185, 9), 429}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57212, 9), 432}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57158, 9), 426}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57203, 9), 431}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57230, 9), 434}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57176, 9), 428}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57194, 9), 430}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57221, 9), 433}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57167, 9), 427}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57536, 4), 466}, // ABS in tosa_001000_1
    {IR(57359, 4), 449}, // ADD in tosa_001000_1
    {IR(57239, 7), 435}, // ARGMAX in tosa_001000_1
    {IR(57363, 23), 450}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(57246, 11), 436}, // AVG_POOL2D in tosa_001000_1
    {IR(57386, 12), 451}, // BITWISE_AND in tosa_001000_1
    {IR(57540, 12), 467}, // BITWISE_NOT in tosa_001000_1
    {IR(57398, 11), 452}, // BITWISE_OR in tosa_001000_1
    {IR(57409, 12), 453}, // BITWISE_XOR in tosa_001000_1
    {IR(57794, 5), 499}, // CAST in tosa_001000_1
    {IR(57552, 5), 468}, // CEIL in tosa_001000_1
    {IR(57336, 6), 445}, // CLAMP in tosa_001000_1
    {IR(57557, 4), 469}, // CLZ in tosa_001000_1
    {IR(57724, 7), 489}, // CONCAT in tosa_001000_1
    {IR(57257, 7), 437}, // CONV2D in tosa_001000_1
    {IR(57264, 7), 438}, // CONV3D in tosa_001000_1
    {IR(57561, 4), 470}, // COS in tosa_001000_1
    {IR(57271, 17), 439}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(57626, 6), 480}, // EQUAL in tosa_001000_1
    {IR(57342, 4), 446}, // ERF in tosa_001000_1
    {IR(57565, 4), 471}, // EXP in tosa_001000_1
    {IR(57288, 6), 440}, // FFT2D in tosa_001000_1
    {IR(57569, 6), 472}, // FLOOR in tosa_001000_1
    {IR(57772, 7), 496}, // GATHER in tosa_001000_1
    {IR(57632, 8), 481}, // GREATER in tosa_001000_1
    {IR(57640, 14), 482}, // GREATER_EQUAL in tosa_001000_1
    {IR(57421, 7), 454}, // INTDIV in tosa_001000_1
    {IR(57575, 4), 473}, // LOG in tosa_001000_1
    {IR(57428, 12), 455}, // LOGICAL_AND in tosa_001000_1
    {IR(57440, 19), 456}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(57579, 12), 474}, // LOGICAL_NOT in tosa_001000_1
    {IR(57479, 11), 458}, // LOGICAL_OR in tosa_001000_1
    {IR(57459, 20), 457}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(57490, 12), 459}, // LOGICAL_XOR in tosa_001000_1
    {IR(57294, 7), 441}, // MATMUL in tosa_001000_1
    {IR(57502, 8), 460}, // MAXIMUM in tosa_001000_1
    {IR(57301, 11), 442}, // MAX_POOL2D in tosa_001000_1
    {IR(57510, 8), 461}, // MINIMUM in tosa_001000_1
    {IR(57518, 4), 462}, // MUL in tosa_001000_1
    {IR(57591, 7), 475}, // NEGATE in tosa_001000_1
    {IR(57731, 4), 490}, // PAD in tosa_001000_1
    {IR(57522, 4), 463}, // POW in tosa_001000_1
    {IR(57598, 11), 476}, // RECIPROCAL in tosa_001000_1
    {IR(57654, 11), 483}, // REDUCE_ALL in tosa_001000_1
    {IR(57665, 11), 484}, // REDUCE_ANY in tosa_001000_1
    {IR(57676, 11), 485}, // REDUCE_MAX in tosa_001000_1
    {IR(57687, 11), 486}, // REDUCE_MIN in tosa_001000_1
    {IR(57698, 15), 487}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(57713, 11), 488}, // REDUCE_SUM in tosa_001000_1
    {IR(57799, 8), 500}, // RESCALE in tosa_001000_1
    {IR(57735, 8), 491}, // RESHAPE in tosa_001000_1
    {IR(57787, 7), 498}, // RESIZE in tosa_001000_1
    {IR(57743, 8), 492}, // REVERSE in tosa_001000_1
    {IR(57312, 7), 443}, // RFFT2D in tosa_001000_1
    {IR(57609, 6), 477}, // RSQRT in tosa_001000_1
    {IR(57779, 8), 497}, // SCATTER in tosa_001000_1
    {IR(57619, 7), 479}, // SELECT in tosa_001000_1
    {IR(57346, 8), 447}, // SIGMOID in tosa_001000_1
    {IR(57615, 4), 478}, // SIN in tosa_001000_1
    {IR(57751, 6), 493}, // SLICE in tosa_001000_1
    {IR(57526, 4), 464}, // SUB in tosa_001000_1
    {IR(57530, 6), 465}, // TABLE in tosa_001000_1
    {IR(57354, 5), 448}, // TANH in tosa_001000_1
    {IR(57757, 5), 494}, // TILE in tosa_001000_1
    {IR(57762, 10), 495}, // TRANSPOSE in tosa_001000_1
    {IR(57319, 17), 444}, // TRANSPOSE_CONV2D in tosa_001000_1
}};

IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type) {
  switch(type) {
    case SPV_EXT_INST_TYPE_ARM_EXPERIMENTAL_ML_OPERATIONS: return IR(0, 1);
    case SPV_EXT_INST_TYPE_ARM_MOTION_ENGINE_100: return IR(1, 3);
    case SPV_EXT_INST_TYPE_DEBUGINFO: return IR(4, 34);
    case SPV_EXT_INST_TYPE_GLSL_STD_450: return IR(38, 81);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION: return IR(119, 42);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_DEBUGPRINTF: return IR(161, 1);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_GRAPH_DEBUGINFO: return IR(162, 3);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100: return IR(165, 46);
    case SPV_EXT_INST_TYPE_NONSEMANTIC_VKSPREFLECTION: return IR(211, 8);
    case SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100: return IR(219, 37);
    case SPV_EXT_INST_TYPE_OPENCL_STD: return IR(256, 162);
    case SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER: return IR(418, 3);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT: return IR(421, 4);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER: return IR(425, 1);
    case SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX: return IR(426, 9);
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(435, 66);
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
  "SPV_INTEL_device_barrier\0" // IR(2314, 25)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(2339, 44)
  "SPV_INTEL_float_controls2\0" // IR(2383, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(2409, 28)
  "SPV_INTEL_fp_max_error\0" // IR(2437, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(2460, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(2495, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(2526, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(2560, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(2587, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(2635, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(2666, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(2695, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(2726, 33)
  "SPV_INTEL_fpga_reg\0" // IR(2759, 19)
  "SPV_INTEL_function_pointers\0" // IR(2778, 28)
  "SPV_INTEL_function_variants\0" // IR(2806, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(2834, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(2877, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2915, 26)
  "SPV_INTEL_int4\0" // IR(2941, 15)
  "SPV_INTEL_io_pipes\0" // IR(2956, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2975, 28)
  "SPV_INTEL_long_composites\0" // IR(3003, 26)
  "SPV_INTEL_loop_fuse\0" // IR(3029, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(3049, 32)
  "SPV_INTEL_maximum_registers\0" // IR(3081, 28)
  "SPV_INTEL_media_block_io\0" // IR(3109, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(3134, 33)
  "SPV_INTEL_optnone\0" // IR(3167, 18)
  "SPV_INTEL_predicated_io\0" // IR(3185, 24)
  "SPV_INTEL_rounded_divide_sqrt\0" // IR(3209, 30)
  "SPV_INTEL_runtime_aligned\0" // IR(3239, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(3265, 36)
  "SPV_INTEL_split_barrier\0" // IR(3301, 24)
  "SPV_INTEL_subgroup_bitcast_shuffle\0" // IR(3325, 35)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3360, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3395, 46)
  "SPV_INTEL_subgroups\0" // IR(3441, 20)
  "SPV_INTEL_task_sequence\0" // IR(3461, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3485, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3521, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3556, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3593, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3623, 32)
  "SPV_INTEL_vector_compute\0" // IR(3655, 25)
  "SPV_KHR_16bit_storage\0" // IR(3680, 22)
  "SPV_KHR_8bit_storage\0" // IR(3702, 21)
  "SPV_KHR_abort\0" // IR(3723, 14)
  "SPV_KHR_bfloat16\0" // IR(3737, 17)
  "SPV_KHR_bit_instructions\0" // IR(3754, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3779, 35)
  "SPV_KHR_constant_data\0" // IR(3814, 22)
  "SPV_KHR_cooperative_matrix\0" // IR(3836, 27)
  "SPV_KHR_device_group\0" // IR(3863, 21)
  "SPV_KHR_expect_assume\0" // IR(3884, 22)
  "SPV_KHR_float_controls\0" // IR(3906, 23)
  "SPV_KHR_float_controls2\0" // IR(3929, 24)
  "SPV_KHR_fma\0" // IR(3953, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3965, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(4001, 30)
  "SPV_KHR_integer_dot_product\0" // IR(4031, 28)
  "SPV_KHR_linkonce_odr\0" // IR(4059, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(4080, 30)
  "SPV_KHR_multiview\0" // IR(4110, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(4128, 35)
  "SPV_KHR_non_semantic_info\0" // IR(4163, 26)
  "SPV_KHR_opacity_micromap\0" // IR(4189, 25)
  "SPV_KHR_physical_storage_buffer\0" // IR(4214, 32)
  "SPV_KHR_poison_freeze\0" // IR(4246, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(4268, 28)
  "SPV_KHR_quad_control\0" // IR(4296, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4317, 22)
  "SPV_KHR_ray_query\0" // IR(4339, 18)
  "SPV_KHR_ray_tracing\0" // IR(4357, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4377, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4412, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4449, 34)
  "SPV_KHR_shader_ballot\0" // IR(4483, 22)
  "SPV_KHR_shader_clock\0" // IR(4505, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4526, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4557, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4594, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4618, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4656, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4678, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4707, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4742, 25)
  "SPV_KHR_variable_pointers\0" // IR(4767, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4793, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4821, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4862, 38)
  "SPV_NV_bindless_texture\0" // IR(4900, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4924, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4962, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4996, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(5022, 27)
  "SPV_NV_cooperative_matrix_decode_vector\0" // IR(5049, 40)
  "SPV_NV_cooperative_vector\0" // IR(5089, 26)
  "SPV_NV_displacement_micromap\0" // IR(5115, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(5144, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(5179, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(5214, 28)
  "SPV_NV_mesh_shader\0" // IR(5242, 19)
  "SPV_NV_push_constant_bank\0" // IR(5261, 26)
  "SPV_NV_raw_access_chains\0" // IR(5287, 25)
  "SPV_NV_ray_tracing\0" // IR(5312, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5331, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5362, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5399, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5432, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5462, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5495, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5521, 35)
  "SPV_NV_shading_rate\0" // IR(5556, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5576, 29)
  "SPV_NV_tensor_addressing\0" // IR(5605, 25)
  "SPV_NV_viewport_array2\0" // IR(5630, 23)
  "SPV_QCOM_bfloat16_muladd\0" // IR(5653, 25)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5678, 39)
  "SPV_QCOM_image_processing\0" // IR(5717, 26)
  "SPV_QCOM_image_processing2\0" // IR(5743, 27)
  "SPV_QCOM_image_processing3\0" // IR(5770, 27)
  "SPV_QCOM_multiple_wait_queues\0" // IR(5797, 30)
  "SPV_QCOM_subgroup_size\0" // IR(5827, 23)
  "SPV_QCOM_tile_shading\0" // IR(5850, 22)
  "SPV_SAMSUNG_intrinsic\0" // IR(5872, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5894, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5932, 34)
  "ReadOnly\0" // IR(5966, 9)
  "Kernel\0" // IR(5975, 7)
  "WriteOnly\0" // IR(5982, 10)
  "ReadWrite\0" // IR(5992, 10)
  "Logical\0" // IR(6002, 8)
  "Physical32\0" // IR(6010, 11)
  "Addresses\0" // IR(6021, 10)
  "Physical64\0" // IR(6031, 11)
  "PhysicalStorageBuffer64\0" // IR(6042, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(6066, 27)
  "PhysicalStorageBufferAddresses\0" // IR(6093, 31)
  "Position\0" // IR(6124, 9)
  "Shader\0" // IR(6133, 7)
  "PointSize\0" // IR(6140, 10)
  "ClipDistance\0" // IR(6150, 13)
  "CullDistance\0" // IR(6163, 13)
  "VertexId\0" // IR(6176, 9)
  "InstanceId\0" // IR(6185, 11)
  "PrimitiveId\0" // IR(6196, 12)
  "Geometry\0" // IR(6208, 9)
  "Tessellation\0" // IR(6217, 13)
  "RayTracingNV\0" // IR(6230, 13)
  "RayTracingKHR\0" // IR(6243, 14)
  "MeshShadingNV\0" // IR(6257, 14)
  "MeshShadingEXT\0" // IR(6271, 15)
  "InvocationId\0" // IR(6286, 13)
  "Layer\0" // IR(6299, 6)
  "ShaderLayer\0" // IR(6305, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(6317, 28)
  "ViewportIndex\0" // IR(6345, 14)
  "MultiViewport\0" // IR(6359, 14)
  "ShaderViewportIndex\0" // IR(6373, 20)
  "TessLevelOuter\0" // IR(6393, 15)
  "TessLevelInner\0" // IR(6408, 15)
  "TessCoord\0" // IR(6423, 10)
  "PatchVertices\0" // IR(6433, 14)
  "FragCoord\0" // IR(6447, 10)
  "PointCoord\0" // IR(6457, 11)
  "FrontFacing\0" // IR(6468, 12)
  "SampleId\0" // IR(6480, 9)
  "SampleRateShading\0" // IR(6489, 18)
  "SamplePosition\0" // IR(6507, 15)
  "SampleMask\0" // IR(6522, 11)
  "FragDepth\0" // IR(6533, 10)
  "HelperInvocation\0" // IR(6543, 17)
  "NumWorkgroups\0" // IR(6560, 14)
  "WorkgroupSize\0" // IR(6574, 14)
  "WorkgroupId\0" // IR(6588, 12)
  "LocalInvocationId\0" // IR(6600, 18)
  "GlobalInvocationId\0" // IR(6618, 19)
  "LocalInvocationIndex\0" // IR(6637, 21)
  "WorkDim\0" // IR(6658, 8)
  "GlobalSize\0" // IR(6666, 11)
  "EnqueuedWorkgroupSize\0" // IR(6677, 22)
  "GlobalOffset\0" // IR(6699, 13)
  "GlobalLinearId\0" // IR(6712, 15)
  "SubgroupSize\0" // IR(6727, 13)
  "GroupNonUniform\0" // IR(6740, 16)
  "SubgroupBallotKHR\0" // IR(6756, 18)
  "SubgroupMaxSize\0" // IR(6774, 16)
  "NumSubgroups\0" // IR(6790, 13)
  "NumEnqueuedSubgroups\0" // IR(6803, 21)
  "SubgroupId\0" // IR(6824, 11)
  "SubgroupLocalInvocationId\0" // IR(6835, 26)
  "VertexIndex\0" // IR(6861, 12)
  "InstanceIndex\0" // IR(6873, 14)
  "CoreIDARM\0" // IR(6887, 10)
  "CoreBuiltinsARM\0" // IR(6897, 16)
  "CoreCountARM\0" // IR(6913, 13)
  "CoreMaxIDARM\0" // IR(6926, 13)
  "WarpIDARM\0" // IR(6939, 10)
  "WarpMaxIDARM\0" // IR(6949, 13)
  "SubgroupEqMask\0" // IR(6962, 15)
  "SubgroupEqMaskKHR\0" // IR(6977, 18)
  "GroupNonUniformBallot\0" // IR(6995, 22)
  "SubgroupGeMask\0" // IR(7017, 15)
  "SubgroupGeMaskKHR\0" // IR(7032, 18)
  "SubgroupGtMask\0" // IR(7050, 15)
  "SubgroupGtMaskKHR\0" // IR(7065, 18)
  "SubgroupLeMask\0" // IR(7083, 15)
  "SubgroupLeMaskKHR\0" // IR(7098, 18)
  "SubgroupLtMask\0" // IR(7116, 15)
  "SubgroupLtMaskKHR\0" // IR(7131, 18)
  "BaseVertex\0" // IR(7149, 11)
  "DrawParameters\0" // IR(7160, 15)
  "BaseInstance\0" // IR(7175, 13)
  "DrawIndex\0" // IR(7188, 10)
  "PrimitiveShadingRateKHR\0" // IR(7198, 24)
  "FragmentShadingRateKHR\0" // IR(7222, 23)
  "DeviceIndex\0" // IR(7245, 12)
  "DeviceGroup\0" // IR(7257, 12)
  "ViewIndex\0" // IR(7269, 10)
  "MultiView\0" // IR(7279, 10)
  "ShadingRateKHR\0" // IR(7289, 15)
  "TileOffsetQCOM\0" // IR(7304, 15)
  "TileShadingQCOM\0" // IR(7319, 16)
  "TileDimensionQCOM\0" // IR(7335, 18)
  "TileApronSizeQCOM\0" // IR(7353, 18)
  "BaryCoordNoPerspAMD\0" // IR(7371, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(7391, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(7419, 26)
  "BaryCoordSmoothAMD\0" // IR(7445, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7464, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7491, 25)
  "BaryCoordPullModelAMD\0" // IR(7516, 22)
  "FragStencilRefEXT\0" // IR(7538, 18)
  "StencilExportEXT\0" // IR(7556, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7573, 29)
  "ShaderEnqueueAMDX\0" // IR(7602, 18)
  "ShaderIndexAMDX\0" // IR(7620, 16)
  "SamplerHeapEXT\0" // IR(7636, 15)
  "DescriptorHeapEXT\0" // IR(7651, 18)
  "ResourceHeapEXT\0" // IR(7669, 16)
  "ViewportMaskNV\0" // IR(7685, 15)
  "ShaderViewportMaskNV\0" // IR(7700, 21)
  "SecondaryPositionNV\0" // IR(7721, 20)
  "ShaderStereoViewNV\0" // IR(7741, 19)
  "SecondaryViewportMaskNV\0" // IR(7760, 24)
  "PositionPerViewNV\0" // IR(7784, 18)
  "PerViewAttributesNV\0" // IR(7802, 20)
  "ViewportMaskPerViewNV\0" // IR(7822, 22)
  "FullyCoveredEXT\0" // IR(7844, 16)
  "FragmentFullyCoveredEXT\0" // IR(7860, 24)
  "TaskCountNV\0" // IR(7884, 12)
  "PrimitiveCountNV\0" // IR(7896, 17)
  "PrimitiveIndicesNV\0" // IR(7913, 19)
  "ClipDistancePerViewNV\0" // IR(7932, 22)
  "CullDistancePerViewNV\0" // IR(7954, 22)
  "LayerPerViewNV\0" // IR(7976, 15)
  "MeshViewCountNV\0" // IR(7991, 16)
  "MeshViewIndicesNV\0" // IR(8007, 18)
  "BaryCoordKHR\0" // IR(8025, 13)
  "BaryCoordNV\0" // IR(8038, 12)
  "FragmentBarycentricKHR\0" // IR(8050, 23)
  "BaryCoordNoPerspKHR\0" // IR(8073, 20)
  "BaryCoordNoPerspNV\0" // IR(8093, 19)
  "FragSizeEXT\0" // IR(8112, 12)
  "FragmentSizeNV\0" // IR(8124, 15)
  "FragmentDensityEXT\0" // IR(8139, 19)
  "FragInvocationCountEXT\0" // IR(8158, 23)
  "InvocationsPerPixelNV\0" // IR(8181, 22)
  "PrimitivePointIndicesEXT\0" // IR(8203, 25)
  "PrimitiveLineIndicesEXT\0" // IR(8228, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(8252, 28)
  "CullPrimitiveEXT\0" // IR(8280, 17)
  "LaunchIdKHR\0" // IR(8297, 12)
  "LaunchIdNV\0" // IR(8309, 11)
  "LaunchSizeKHR\0" // IR(8320, 14)
  "LaunchSizeNV\0" // IR(8334, 13)
  "WorldRayOriginKHR\0" // IR(8347, 18)
  "WorldRayOriginNV\0" // IR(8365, 17)
  "WorldRayDirectionKHR\0" // IR(8382, 21)
  "WorldRayDirectionNV\0" // IR(8403, 20)
  "ObjectRayOriginKHR\0" // IR(8423, 19)
  "ObjectRayOriginNV\0" // IR(8442, 18)
  "ObjectRayDirectionKHR\0" // IR(8460, 22)
  "ObjectRayDirectionNV\0" // IR(8482, 21)
  "RayTminKHR\0" // IR(8503, 11)
  "RayTminNV\0" // IR(8514, 10)
  "RayTmaxKHR\0" // IR(8524, 11)
  "RayTmaxNV\0" // IR(8535, 10)
  "InstanceCustomIndexKHR\0" // IR(8545, 23)
  "InstanceCustomIndexNV\0" // IR(8568, 22)
  "ObjectToWorldKHR\0" // IR(8590, 17)
  "ObjectToWorldNV\0" // IR(8607, 16)
  "WorldToObjectKHR\0" // IR(8623, 17)
  "WorldToObjectNV\0" // IR(8640, 16)
  "HitTNV\0" // IR(8656, 7)
  "HitKindKHR\0" // IR(8663, 11)
  "HitKindNV\0" // IR(8674, 10)
  "CurrentRayTimeNV\0" // IR(8684, 17)
  "RayTracingMotionBlurNV\0" // IR(8701, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8724, 30)
  "RayTracingPositionFetchKHR\0" // IR(8754, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8781, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8815, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8848, 37)
  "IncomingRayFlagsKHR\0" // IR(8885, 20)
  "IncomingRayFlagsNV\0" // IR(8905, 19)
  "RayGeometryIndexKHR\0" // IR(8924, 20)
  "HitIsSphereNV\0" // IR(8944, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8958, 28)
  "HitIsLSSNV\0" // IR(8986, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8997, 39)
  "HitSpherePositionNV\0" // IR(9036, 20)
  "WarpsPerSMNV\0" // IR(9056, 13)
  "ShaderSMBuiltinsNV\0" // IR(9069, 19)
  "SMCountNV\0" // IR(9088, 10)
  "WarpIDNV\0" // IR(9098, 9)
  "SMIDNV\0" // IR(9107, 7)
  "HitLSSPositionsNV\0" // IR(9114, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(9132, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(9166, 33)
  "HitSphereRadiusNV\0" // IR(9199, 18)
  "HitLSSRadiiNV\0" // IR(9217, 14)
  "ClusterIDNV\0" // IR(9231, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(9243, 41)
  "CullMaskKHR\0" // IR(9284, 12)
  "RayCullMaskKHR\0" // IR(9296, 15)
  "DeviceBarrierValidINTEL\0" // IR(9311, 24)
  "DeviceBarrierINTEL\0" // IR(9335, 19)
  "Matrix\0" // IR(9354, 7)
  "Linkage\0" // IR(9361, 8)
  "Vector16\0" // IR(9369, 9)
  "Float16Buffer\0" // IR(9378, 14)
  "Float16\0" // IR(9392, 8)
  "Float64\0" // IR(9400, 8)
  "Int64\0" // IR(9408, 6)
  "Int64Atomics\0" // IR(9414, 13)
  "ImageBasic\0" // IR(9427, 11)
  "ImageReadWrite\0" // IR(9438, 15)
  "ImageMipmap\0" // IR(9453, 12)
  "Pipes\0" // IR(9465, 6)
  "Groups\0" // IR(9471, 7)
  "DeviceEnqueue\0" // IR(9478, 14)
  "LiteralSampler\0" // IR(9492, 15)
  "AtomicStorage\0" // IR(9507, 14)
  "Int16\0" // IR(9521, 6)
  "TessellationPointSize\0" // IR(9527, 22)
  "GeometryPointSize\0" // IR(9549, 18)
  "ImageGatherExtended\0" // IR(9567, 20)
  "StorageImageMultisample\0" // IR(9587, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9611, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9645, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9678, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9712, 33)
  "ImageCubeArray\0" // IR(9745, 15)
  "SampledCubeArray\0" // IR(9760, 17)
  "ImageRect\0" // IR(9777, 10)
  "SampledRect\0" // IR(9787, 12)
  "GenericPointer\0" // IR(9799, 15)
  "Int8\0" // IR(9814, 5)
  "InputAttachment\0" // IR(9819, 16)
  "SparseResidency\0" // IR(9835, 16)
  "MinLod\0" // IR(9851, 7)
  "Sampled1D\0" // IR(9858, 10)
  "Image1D\0" // IR(9868, 8)
  "SampledBuffer\0" // IR(9876, 14)
  "ImageBuffer\0" // IR(9890, 12)
  "ImageMSArray\0" // IR(9902, 13)
  "StorageImageExtendedFormats\0" // IR(9915, 28)
  "ImageQuery\0" // IR(9943, 11)
  "DerivativeControl\0" // IR(9954, 18)
  "InterpolationFunction\0" // IR(9972, 22)
  "TransformFeedback\0" // IR(9994, 18)
  "GeometryStreams\0" // IR(10012, 16)
  "StorageImageReadWithoutFormat\0" // IR(10028, 30)
  "StorageImageWriteWithoutFormat\0" // IR(10058, 31)
  "SubgroupDispatch\0" // IR(10089, 17)
  "NamedBarrier\0" // IR(10106, 13)
  "PipeStorage\0" // IR(10119, 12)
  "GroupNonUniformVote\0" // IR(10131, 20)
  "GroupNonUniformArithmetic\0" // IR(10151, 26)
  "GroupNonUniformShuffle\0" // IR(10177, 23)
  "GroupNonUniformShuffleRelative\0" // IR(10200, 31)
  "GroupNonUniformClustered\0" // IR(10231, 25)
  "GroupNonUniformQuad\0" // IR(10256, 20)
  "UniformDecoration\0" // IR(10276, 18)
  "TileImageColorReadAccessEXT\0" // IR(10294, 28)
  "TileImageDepthReadAccessEXT\0" // IR(10322, 28)
  "TileImageStencilReadAccessEXT\0" // IR(10350, 30)
  "TensorsARM\0" // IR(10380, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(10391, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(10428, 40)
  "GraphARM\0" // IR(10468, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(10477, 28)
  "Float8EXT\0" // IR(10505, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10515, 27)
  "CooperativeMatrixKHR\0" // IR(10542, 21)
  "Float6EXT\0" // IR(10563, 10)
  "Float4EXT\0" // IR(10573, 10)
  "Float8UnsignedE8M0EXT\0" // IR(10583, 22)
  "MXInt8EXT\0" // IR(10605, 10)
  "BitcastExtractEXT\0" // IR(10615, 18)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10633, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10666, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10709, 44)
  "SubgroupVoteKHR\0" // IR(10753, 16)
  "StorageBuffer16BitAccess\0" // IR(10769, 25)
  "StorageUniformBufferBlock16\0" // IR(10794, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10822, 35)
  "StorageUniform16\0" // IR(10857, 17)
  "StoragePushConstant16\0" // IR(10874, 22)
  "StorageInputOutput16\0" // IR(10896, 21)
  "VariablePointersStorageBuffer\0" // IR(10917, 30)
  "VariablePointers\0" // IR(10947, 17)
  "AtomicStorageOps\0" // IR(10964, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10981, 28)
  "StorageBuffer8BitAccess\0" // IR(11009, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(11033, 34)
  "StoragePushConstant8\0" // IR(11067, 21)
  "DenormPreserve\0" // IR(11088, 15)
  "DenormFlushToZero\0" // IR(11103, 18)
  "SignedZeroInfNanPreserve\0" // IR(11121, 25)
  "RoundingModeRTE\0" // IR(11146, 16)
  "RoundingModeRTZ\0" // IR(11162, 16)
  "RayQueryProvisionalKHR\0" // IR(11178, 23)
  "RayQueryKHR\0" // IR(11201, 12)
  "UntypedPointersKHR\0" // IR(11213, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(11232, 32)
  "TextureSampleWeightedQCOM\0" // IR(11264, 26)
  "TextureBoxFilterQCOM\0" // IR(11290, 21)
  "TextureBlockMatchQCOM\0" // IR(11311, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(11333, 32)
  "TextureBlockMatch2QCOM\0" // IR(11365, 23)
  "BFloat16MulAddQCOM\0" // IR(11388, 19)
  "SubgroupSizeQCOM\0" // IR(11407, 17)
  "MultipleWaitQueuesQCOM\0" // IR(11424, 23)
  "ImageGatherLinearQCOM\0" // IR(11447, 22)
  "ImageGatherExtendedModesQCOM\0" // IR(11469, 29)
  "Float16ImageAMD\0" // IR(11498, 16)
  "ImageGatherBiasLodAMD\0" // IR(11514, 22)
  "FragmentMaskAMD\0" // IR(11536, 16)
  "ImageReadWriteLodAMD\0" // IR(11552, 21)
  "Int64ImageEXT\0" // IR(11573, 14)
  "ShaderClockKHR\0" // IR(11587, 15)
  "QuadControlKHR\0" // IR(11602, 15)
  "Int4TypeINTEL\0" // IR(11617, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(11631, 27)
  "BFloat16TypeKHR\0" // IR(11658, 16)
  "BFloat16DotProductKHR\0" // IR(11674, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11696, 29)
  "AbortKHR\0" // IR(11725, 9)
  "ConstantDataKHR\0" // IR(11734, 16)
  "PoisonFreezeKHR\0" // IR(11750, 16)
  "WeakLinkageAMD\0" // IR(11766, 15)
  "SampleMaskOverrideCoverageNV\0" // IR(11781, 29)
  "GeometryShaderPassthroughNV\0" // IR(11810, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11838, 27)
  "ImageFootprintNV\0" // IR(11865, 17)
  "FragmentBarycentricNV\0" // IR(11882, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11904, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11935, 30)
  "ShadingRateNV\0" // IR(11965, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11979, 30)
  "GroupNonUniformPartitionedNV\0" // IR(12009, 29)
  "ShaderNonUniform\0" // IR(12038, 17)
  "ShaderNonUniformEXT\0" // IR(12055, 20)
  "RuntimeDescriptorArray\0" // IR(12075, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(12098, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(12124, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(12160, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(12199, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(12238, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(12280, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(12319, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(12361, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(12398, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(12438, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(12474, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(12513, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(12550, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(12590, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(12626, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(12665, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(12704, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12746, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12788, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12833, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12875, 45)
  "VulkanMemoryModel\0" // IR(12920, 18)
  "VulkanMemoryModelKHR\0" // IR(12938, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12959, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12988, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(13020, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(13054, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(13086, 31)
  "RayTracingProvisionalKHR\0" // IR(13117, 25)
  "CooperativeMatrixNV\0" // IR(13142, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(13162, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(13195, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(13233, 32)
  "DemoteToHelperInvocation\0" // IR(13265, 25)
  "DemoteToHelperInvocationEXT\0" // IR(13290, 28)
  "DisplacementMicromapNV\0" // IR(13318, 23)
  "RayTracingOpacityMicromapKHR\0" // IR(13341, 29)
  "RayTracingOpacityMicromapEXT\0" // IR(13370, 29)
  "ShaderInvocationReorderNV\0" // IR(13399, 26)
  "ShaderInvocationReorderEXT\0" // IR(13425, 27)
  "BindlessTextureNV\0" // IR(13452, 18)
  "RayQueryPositionFetchKHR\0" // IR(13470, 25)
  "CooperativeVectorNV\0" // IR(13495, 20)
  "AtomicFloat16VectorNV\0" // IR(13515, 22)
  "RawAccessChainsNV\0" // IR(13537, 18)
  "PushConstantBanksNV\0" // IR(13555, 20)
  "LongVectorEXT\0" // IR(13575, 14)
  "Shader64BitIndexingEXT\0" // IR(13589, 23)
  "CooperativeMatrixConversionsEXT\0" // IR(13612, 32)
  "CooperativeMatrixReductionsEXT\0" // IR(13644, 31)
  "CooperativeMatrixReductionsNV\0" // IR(13675, 30)
  "CooperativeMatrixConversionsNV\0" // IR(13705, 31)
  "CooperativeMatrixPerElementOperationsEXT\0" // IR(13736, 41)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(13777, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(13817, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13853, 30)
  "CooperativeVectorTrainingNV\0" // IR(13883, 28)
  "CooperativeMatrixGetCoordinateEXT\0" // IR(13911, 34)
  "TensorAddressingNV\0" // IR(13945, 19)
  "CooperativeMatrixDecodeVectorNV\0" // IR(13964, 32)
  "SubgroupShuffleINTEL\0" // IR(13996, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(14017, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(14044, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(14070, 31)
  "RoundToInfinityINTEL\0" // IR(14101, 21)
  "FloatingPointModeINTEL\0" // IR(14122, 23)
  "IntegerFunctions2INTEL\0" // IR(14145, 23)
  "FunctionPointersINTEL\0" // IR(14168, 22)
  "IndirectReferencesINTEL\0" // IR(14190, 24)
  "AsmINTEL\0" // IR(14214, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(14223, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(14246, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(14269, 23)
  "VectorComputeINTEL\0" // IR(14292, 19)
  "VectorAnyINTEL\0" // IR(14311, 15)
  "ExpectAssumeKHR\0" // IR(14326, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(14342, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(14375, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(14413, 39)
  "VariableLengthArrayINTEL\0" // IR(14452, 25)
  "FunctionFloatControlINTEL\0" // IR(14477, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(14503, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(14530, 26)
  "FPFastMathModeINTEL\0" // IR(14556, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(14576, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(14609, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(14641, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(14679, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(14716, 30)
  "FPGALoopControlsALTERA\0" // IR(14746, 23)
  "FPGALoopControlsINTEL\0" // IR(14769, 22)
  "KernelAttributesINTEL\0" // IR(14791, 22)
  "FPGAKernelAttributesINTEL\0" // IR(14813, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(14839, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(14864, 24)
  "FPGAClusterAttributesALTERA\0" // IR(14888, 28)
  "FPGAClusterAttributesINTEL\0" // IR(14916, 27)
  "LoopFuseALTERA\0" // IR(14943, 15)
  "LoopFuseINTEL\0" // IR(14958, 14)
  "FPGADSPControlALTERA\0" // IR(14972, 21)
  "FPGADSPControlINTEL\0" // IR(14993, 20)
  "MemoryAccessAliasingINTEL\0" // IR(15013, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(15039, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(15080, 40)
  "FPGABufferLocationALTERA\0" // IR(15120, 25)
  "FPGABufferLocationINTEL\0" // IR(15145, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(15169, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(15204, 34)
  "USMStorageClassesALTERA\0" // IR(15238, 24)
  "USMStorageClassesINTEL\0" // IR(15262, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(15285, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(15315, 29)
  "IOPipesALTERA\0" // IR(15344, 14)
  "IOPipesINTEL\0" // IR(15358, 13)
  "BlockingPipesALTERA\0" // IR(15371, 20)
  "BlockingPipesINTEL\0" // IR(15391, 19)
  "FPGARegALTERA\0" // IR(15410, 14)
  "FPGARegINTEL\0" // IR(15424, 13)
  "DotProductInputAll\0" // IR(15437, 19)
  "DotProductInputAllKHR\0" // IR(15456, 22)
  "DotProductInput4x8Bit\0" // IR(15478, 22)
  "DotProductInput4x8BitKHR\0" // IR(15500, 25)
  "DotProductInput4x8BitPacked\0" // IR(15525, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(15553, 31)
  "DotProduct\0" // IR(15584, 11)
  "DotProductKHR\0" // IR(15595, 14)
  "ReplicatedCompositesEXT\0" // IR(15609, 24)
  "BitInstructions\0" // IR(15633, 16)
  "GroupNonUniformRotateKHR\0" // IR(15649, 25)
  "FloatControls2\0" // IR(15674, 15)
  "FMAKHR\0" // IR(15689, 7)
  "RayTracingOpacityMicromapExecutionModeKHR\0" // IR(15696, 42)
  "AtomicFloat32AddEXT\0" // IR(15738, 20)
  "AtomicFloat64AddEXT\0" // IR(15758, 20)
  "LongCompositesINTEL\0" // IR(15778, 20)
  "OptNoneEXT\0" // IR(15798, 11)
  "OptNoneINTEL\0" // IR(15809, 13)
  "AtomicFloat16AddEXT\0" // IR(15822, 20)
  "DebugInfoModuleINTEL\0" // IR(15842, 21)
  "BFloat16ConversionINTEL\0" // IR(15863, 24)
  "SplitBarrierEXT\0" // IR(15887, 16)
  "SplitBarrierINTEL\0" // IR(15903, 18)
  "ArithmeticFenceEXT\0" // IR(15921, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(15940, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(15970, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15999, 28)
  "TaskSequenceALTERA\0" // IR(16027, 19)
  "TaskSequenceINTEL\0" // IR(16046, 18)
  "FPMaxErrorINTEL\0" // IR(16064, 16)
  "FPGALatencyControlALTERA\0" // IR(16080, 25)
  "FPGALatencyControlINTEL\0" // IR(16105, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(16129, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(16158, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(16186, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(16216, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(16252, 35)
  "SubgroupBitcastShuffleINTEL\0" // IR(16287, 28)
  "SubgroupBufferPrefetchINTEL\0" // IR(16315, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(16343, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(16366, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(16396, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(16426, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(16464, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(16492, 32)
  "SpecConditionalINTEL\0" // IR(16524, 21)
  "FunctionVariantsINTEL\0" // IR(16545, 22)
  "PredicatedIOINTEL\0" // IR(16567, 18)
  "RoundedDivideSqrtINTEL\0" // IR(16585, 23)
  "GroupUniformArithmeticKHR\0" // IR(16608, 26)
  "TensorFloat32RoundingINTEL\0" // IR(16634, 27)
  "MaskedGatherScatterINTEL\0" // IR(16661, 25)
  "CacheControlsINTEL\0" // IR(16686, 19)
  "RegisterLimitsINTEL\0" // IR(16705, 20)
  "BindlessImagesINTEL\0" // IR(16725, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(16745, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(16778, 33)
  "DotProductBFloat16AccVALVE\0" // IR(16811, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(16838, 32)
  "IntrinsicSAMSUNG\0" // IR(16870, 17)
  "Unspecified\0" // IR(16887, 12)
  "Address\0" // IR(16899, 8)
  "Boolean\0" // IR(16907, 8)
  "Float\0" // IR(16915, 6)
  "Signed\0" // IR(16921, 7)
  "SignedChar\0" // IR(16928, 11)
  "Unsigned\0" // IR(16939, 9)
  "UnsignedChar\0" // IR(16948, 13)
  "Class\0" // IR(16961, 6)
  "Structure\0" // IR(16967, 10)
  "Union\0" // IR(16977, 6)
  "ImportedModule\0" // IR(16983, 15)
  "ImportedDeclaration\0" // IR(16998, 20)
  "None\0" // IR(17018, 5)
  "FlagIsProtected\0" // IR(17023, 16)
  "FlagIsPrivate\0" // IR(17039, 14)
  "FlagIsPublic\0" // IR(17053, 13)
  "FlagIsLocal\0" // IR(17066, 12)
  "FlagIsDefinition\0" // IR(17078, 17)
  "FlagFwdDecl\0" // IR(17095, 12)
  "FlagArtificial\0" // IR(17107, 15)
  "FlagExplicit\0" // IR(17122, 13)
  "FlagPrototyped\0" // IR(17135, 15)
  "FlagObjectPointer\0" // IR(17150, 18)
  "FlagStaticMember\0" // IR(17168, 17)
  "FlagIndirectVariable\0" // IR(17185, 21)
  "FlagLValueReference\0" // IR(17206, 20)
  "FlagRValueReference\0" // IR(17226, 20)
  "FlagIsOptimized\0" // IR(17246, 16)
  "FlagIsEnumClass\0" // IR(17262, 16)
  "FlagTypePassByValue\0" // IR(17278, 20)
  "FlagTypePassByReference\0" // IR(17298, 24)
  "Deref\0" // IR(17322, 6)
  "Plus\0" // IR(17328, 5)
  "Minus\0" // IR(17333, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(17339, 33)
  "PlusUconst\0" // IR(17372, 11)
  "BitPiece\0" // IR(17383, 9)
  "Swap\0" // IR(17392, 5)
  "Xderef\0" // IR(17397, 7)
  "StackValue\0" // IR(17404, 11)
  "Constu\0" // IR(17415, 7)
  "Fragment\0" // IR(17422, 9)
  "ConstType\0" // IR(17431, 10)
  "VolatileType\0" // IR(17441, 13)
  "RestrictType\0" // IR(17454, 13)
  "AtomicType\0" // IR(17467, 11)
  "Float16NV\0" // IR(17478, 10)
  "Float32NV\0" // IR(17488, 10)
  "Float64NV\0" // IR(17498, 10)
  "SignedInt8NV\0" // IR(17508, 13)
  "SignedInt16NV\0" // IR(17521, 14)
  "SignedInt32NV\0" // IR(17535, 14)
  "SignedInt64NV\0" // IR(17549, 14)
  "UnsignedInt8NV\0" // IR(17563, 15)
  "UnsignedInt16NV\0" // IR(17578, 16)
  "UnsignedInt32NV\0" // IR(17594, 16)
  "UnsignedInt64NV\0" // IR(17610, 16)
  "SignedInt8PackedNV\0" // IR(17626, 19)
  "UnsignedInt8PackedNV\0" // IR(17645, 21)
  "FloatE4M3NV\0" // IR(17666, 12)
  "FloatE5M2NV\0" // IR(17678, 12)
  "RowMajorKHR\0" // IR(17690, 12)
  "ColumnMajorKHR\0" // IR(17702, 15)
  "RowBlockedInterleavedARM\0" // IR(17717, 25)
  "ColumnBlockedInterleavedARM\0" // IR(17742, 28)
  "NoneKHR\0" // IR(17770, 8)
  "MatrixASignedComponentsKHR\0" // IR(17778, 27)
  "MatrixBSignedComponentsKHR\0" // IR(17805, 27)
  "MatrixCSignedComponentsKHR\0" // IR(17832, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(17859, 32)
  "SaturatingAccumulationKHR\0" // IR(17891, 26)
  "Row\0" // IR(17917, 4)
  "Column\0" // IR(17921, 7)
  "2x2\0" // IR(17928, 4)
  "MatrixAKHR\0" // IR(17932, 11)
  "MatrixBKHR\0" // IR(17943, 11)
  "MatrixAccumulatorKHR\0" // IR(17954, 21)
  "RowMajorNV\0" // IR(17975, 11)
  "ColumnMajorNV\0" // IR(17986, 14)
  "InferencingOptimalNV\0" // IR(18000, 21)
  "TrainingOptimalNV\0" // IR(18021, 18)
  "RelaxedPrecision\0" // IR(18039, 17)
  "SpecId\0" // IR(18056, 7)
  "Block\0" // IR(18063, 6)
  "BufferBlock\0" // IR(18069, 12)
  "RowMajor\0" // IR(18081, 9)
  "ColMajor\0" // IR(18090, 9)
  "ArrayStride\0" // IR(18099, 12)
  "MatrixStride\0" // IR(18111, 13)
  "GLSLShared\0" // IR(18124, 11)
  "GLSLPacked\0" // IR(18135, 11)
  "CPacked\0" // IR(18146, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(18154, 26)
  "BuiltIn\0" // IR(18180, 8)
  "NoPerspective\0" // IR(18188, 14)
  "Flat\0" // IR(18202, 5)
  "Patch\0" // IR(18207, 6)
  "Centroid\0" // IR(18213, 9)
  "Sample\0" // IR(18222, 7)
  "Invariant\0" // IR(18229, 10)
  "Restrict\0" // IR(18239, 9)
  "Aliased\0" // IR(18248, 8)
  "Volatile\0" // IR(18256, 9)
  "Constant\0" // IR(18265, 9)
  "Coherent\0" // IR(18274, 9)
  "NonWritable\0" // IR(18283, 12)
  "NonReadable\0" // IR(18295, 12)
  "Uniform\0" // IR(18307, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(18315, 26)
  "UniformId\0" // IR(18341, 10)
  "SaturatedConversion\0" // IR(18351, 20)
  "Stream\0" // IR(18371, 7)
  "Location\0" // IR(18378, 9)
  "Component\0" // IR(18387, 10)
  "Index\0" // IR(18397, 6)
  "Binding\0" // IR(18403, 8)
  "DescriptorSet\0" // IR(18411, 14)
  "Offset\0" // IR(18425, 7)
  "XfbBuffer\0" // IR(18432, 10)
  "XfbStride\0" // IR(18442, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(18452, 46)
  "FuncParamAttr\0" // IR(18498, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(18512, 34)
  "FPRoundingMode\0" // IR(18546, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(18561, 35)
  "FPFastMathMode\0" // IR(18596, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(18611, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(18643, 30)
  "LinkageAttributes\0" // IR(18673, 18)
  "NoContraction\0" // IR(18691, 14)
  "InputAttachmentIndex\0" // IR(18705, 21)
  "Alignment\0" // IR(18726, 10)
  "MaxByteOffset\0" // IR(18736, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(18750, 20)
  "AlignmentId\0" // IR(18770, 12)
  "MaxByteOffsetId\0" // IR(18782, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(18798, 44)
  "NoSignedWrap\0" // IR(18842, 13)
  "NoUnsignedWrap\0" // IR(18855, 15)
  "WeightTextureQCOM\0" // IR(18870, 18)
  "BlockMatchTextureQCOM\0" // IR(18888, 22)
  "BlockMatchSamplerQCOM\0" // IR(18910, 22)
  "ExplicitInterpAMD\0" // IR(18932, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(18950, 32)
  "NodeMaxPayloadsAMDX\0" // IR(18982, 20)
  "TrackFinishWritingAMDX\0" // IR(19002, 23)
  "PayloadNodeNameAMDX\0" // IR(19025, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(19045, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(19070, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(19097, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(19122, 28)
  "ArrayStrideIdEXT\0" // IR(19150, 17)
  "OffsetIdEXT\0" // IR(19167, 12)
  "UTFEncodedKHR\0" // IR(19179, 14)
  "OverrideCoverageNV\0" // IR(19193, 19)
  "PassthroughNV\0" // IR(19212, 14)
  "ViewportRelativeNV\0" // IR(19226, 19)
  "SecondaryViewportRelativeNV\0" // IR(19245, 28)
  "PerPrimitiveEXT\0" // IR(19273, 16)
  "PerPrimitiveNV\0" // IR(19289, 15)
  "PerViewNV\0" // IR(19304, 10)
  "PerTaskNV\0" // IR(19314, 10)
  "PerVertexKHR\0" // IR(19324, 13)
  "PerVertexNV\0" // IR(19337, 12)
  "NonUniform\0" // IR(19349, 11)
  "NonUniformEXT\0" // IR(19360, 14)
  "RestrictPointer\0" // IR(19374, 16)
  "RestrictPointerEXT\0" // IR(19390, 19)
  "AliasedPointer\0" // IR(19409, 15)
  "AliasedPointerEXT\0" // IR(19424, 18)
  "MemberOffsetNV\0" // IR(19442, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(19457, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(19487, 31)
  "BankNV\0" // IR(19518, 7)
  "BindlessSamplerNV\0" // IR(19525, 18)
  "BindlessImageNV\0" // IR(19543, 16)
  "BoundSamplerNV\0" // IR(19559, 15)
  "BoundImageNV\0" // IR(19574, 13)
  "CooperativeMatrixTransposeEXT\0" // IR(19587, 30)
  "SIMTCallINTEL\0" // IR(19617, 14)
  "ReferencedIndirectlyINTEL\0" // IR(19631, 26)
  "ClobberINTEL\0" // IR(19657, 13)
  "SideEffectsINTEL\0" // IR(19670, 17)
  "VectorComputeVariableINTEL\0" // IR(19687, 27)
  "FuncParamIOKindINTEL\0" // IR(19714, 21)
  "VectorComputeFunctionINTEL\0" // IR(19735, 27)
  "StackCallINTEL\0" // IR(19762, 15)
  "GlobalVariableOffsetINTEL\0" // IR(19777, 26)
  "CounterBuffer\0" // IR(19803, 14)
  "HlslCounterBufferGOOGLE\0" // IR(19817, 24)
  "UserSemantic\0" // IR(19841, 13)
  "HlslSemanticGOOGLE\0" // IR(19854, 19)
  "UserTypeGOOGLE\0" // IR(19873, 15)
  "FunctionRoundingModeINTEL\0" // IR(19888, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(19914, 31)
  "FunctionDenormModeINTEL\0" // IR(19945, 24)
  "RegisterALTERA\0" // IR(19969, 15)
  "RegisterINTEL\0" // IR(19984, 14)
  "MemoryALTERA\0" // IR(19998, 13)
  "MemoryINTEL\0" // IR(20011, 12)
  "NumbanksALTERA\0" // IR(20023, 15)
  "NumbanksINTEL\0" // IR(20038, 14)
  "BankwidthALTERA\0" // IR(20052, 16)
  "BankwidthINTEL\0" // IR(20068, 15)
  "MaxPrivateCopiesALTERA\0" // IR(20083, 23)
  "MaxPrivateCopiesINTEL\0" // IR(20106, 22)
  "SinglepumpALTERA\0" // IR(20128, 17)
  "SinglepumpINTEL\0" // IR(20145, 16)
  "DoublepumpALTERA\0" // IR(20161, 17)
  "DoublepumpINTEL\0" // IR(20178, 16)
  "MaxReplicatesALTERA\0" // IR(20194, 20)
  "MaxReplicatesINTEL\0" // IR(20214, 19)
  "SimpleDualPortALTERA\0" // IR(20233, 21)
  "SimpleDualPortINTEL\0" // IR(20254, 20)
  "MergeALTERA\0" // IR(20274, 12)
  "MergeINTEL\0" // IR(20286, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(20297, 42)
  "BankBitsALTERA\0" // IR(20339, 15)
  "BankBitsINTEL\0" // IR(20354, 14)
  "ForcePow2DepthALTERA\0" // IR(20368, 21)
  "ForcePow2DepthINTEL\0" // IR(20389, 20)
  "StridesizeALTERA\0" // IR(20409, 17)
  "StridesizeINTEL\0" // IR(20426, 16)
  "WordsizeALTERA\0" // IR(20442, 15)
  "WordsizeINTEL\0" // IR(20457, 14)
  "TrueDualPortALTERA\0" // IR(20471, 19)
  "TrueDualPortINTEL\0" // IR(20490, 18)
  "BurstCoalesceALTERA\0" // IR(20508, 20)
  "BurstCoalesceINTEL\0" // IR(20528, 19)
  "CacheSizeALTERA\0" // IR(20547, 16)
  "CacheSizeINTEL\0" // IR(20563, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(20578, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(20607, 28)
  "PrefetchALTERA\0" // IR(20635, 15)
  "PrefetchINTEL\0" // IR(20650, 14)
  "StallEnableALTERA\0" // IR(20664, 18)
  "StallEnableINTEL\0" // IR(20682, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(20699, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(20725, 25)
  "MathOpDSPModeALTERA\0" // IR(20750, 20)
  "MathOpDSPModeINTEL\0" // IR(20770, 19)
  "AliasScopeINTEL\0" // IR(20789, 16)
  "NoAliasINTEL\0" // IR(20805, 13)
  "InitiationIntervalALTERA\0" // IR(20818, 25)
  "InitiationIntervalINTEL\0" // IR(20843, 24)
  "MaxConcurrencyALTERA\0" // IR(20867, 21)
  "MaxConcurrencyINTEL\0" // IR(20888, 20)
  "PipelineEnableALTERA\0" // IR(20908, 21)
  "PipelineEnableINTEL\0" // IR(20929, 20)
  "BufferLocationALTERA\0" // IR(20949, 21)
  "BufferLocationINTEL\0" // IR(20970, 20)
  "IOPipeStorageALTERA\0" // IR(20990, 20)
  "IOPipeStorageINTEL\0" // IR(21010, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(21029, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(21063, 31)
  "SingleElementVectorINTEL\0" // IR(21094, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(21119, 35)
  "MediaBlockIOINTEL\0" // IR(21154, 18)
  "StallFreeALTERA\0" // IR(21172, 16)
  "StallFreeINTEL\0" // IR(21188, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(21203, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(21234, 26)
  "LatencyControlLabelALTERA\0" // IR(21260, 26)
  "LatencyControlLabelINTEL\0" // IR(21286, 25)
  "LatencyControlConstraintALTERA\0" // IR(21311, 31)
  "LatencyControlConstraintINTEL\0" // IR(21342, 30)
  "ConduitKernelArgumentALTERA\0" // IR(21372, 28)
  "ConduitKernelArgumentINTEL\0" // IR(21400, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(21427, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(21459, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(21490, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(21524, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(21557, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(21588, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(21618, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(21647, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(21675, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(21709, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(21744, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(21778, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(21808, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(21837, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(21870, 32)
  "StableKernelArgumentALTERA\0" // IR(21902, 27)
  "StableKernelArgumentINTEL\0" // IR(21929, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(21955, 39)
  "HostAccessINTEL\0" // IR(21994, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(22010, 47)
  "InitModeALTERA\0" // IR(22057, 15)
  "InitModeINTEL\0" // IR(22072, 14)
  "ImplementInRegisterMapALTERA\0" // IR(22086, 29)
  "ImplementInRegisterMapINTEL\0" // IR(22115, 28)
  "ConditionalINTEL\0" // IR(22143, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(22160, 36)
  "CacheControlLoadINTEL\0" // IR(22196, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(22218, 37)
  "CacheControlStoreINTEL\0" // IR(22255, 23)
  "1D\0" // IR(22278, 3)
  "2D\0" // IR(22281, 3)
  "3D\0" // IR(22284, 3)
  "Cube\0" // IR(22287, 5)
  "Rect\0" // IR(22292, 5)
  "Buffer\0" // IR(22297, 7)
  "SubpassData\0" // IR(22304, 12)
  "TileImageDataEXT\0" // IR(22316, 17)
  "Invocations\0" // IR(22333, 12)
  "SpacingEqual\0" // IR(22345, 13)
  "SpacingFractionalEven\0" // IR(22358, 22)
  "SpacingFractionalOdd\0" // IR(22380, 21)
  "VertexOrderCw\0" // IR(22401, 14)
  "VertexOrderCcw\0" // IR(22415, 15)
  "PixelCenterInteger\0" // IR(22430, 19)
  "OriginUpperLeft\0" // IR(22449, 16)
  "OriginLowerLeft\0" // IR(22465, 16)
  "EarlyFragmentTests\0" // IR(22481, 19)
  "PointMode\0" // IR(22500, 10)
  "Xfb\0" // IR(22510, 4)
  "DepthReplacing\0" // IR(22514, 15)
  "DepthGreater\0" // IR(22529, 13)
  "DepthLess\0" // IR(22542, 10)
  "DepthUnchanged\0" // IR(22552, 15)
  "LocalSize\0" // IR(22567, 10)
  "LocalSizeHint\0" // IR(22577, 14)
  "InputPoints\0" // IR(22591, 12)
  "InputLines\0" // IR(22603, 11)
  "InputLinesAdjacency\0" // IR(22614, 20)
  "Triangles\0" // IR(22634, 10)
  "InputTrianglesAdjacency\0" // IR(22644, 24)
  "Quads\0" // IR(22668, 6)
  "Isolines\0" // IR(22674, 9)
  "OutputVertices\0" // IR(22683, 15)
  "OutputPoints\0" // IR(22698, 13)
  "OutputLineStrip\0" // IR(22711, 16)
  "OutputTriangleStrip\0" // IR(22727, 20)
  "VecTypeHint\0" // IR(22747, 12)
  "ContractionOff\0" // IR(22759, 15)
  "Initializer\0" // IR(22774, 12)
  "Finalizer\0" // IR(22786, 10)
  "SubgroupsPerWorkgroup\0" // IR(22796, 22)
  "SubgroupsPerWorkgroupId\0" // IR(22818, 24)
  "LocalSizeId\0" // IR(22842, 12)
  "LocalSizeHintId\0" // IR(22854, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(22870, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(22904, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(22938, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(22974, 30)
  "PostDepthCoverage\0" // IR(23004, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(23022, 34)
  "TileShadingRateQCOM\0" // IR(23056, 20)
  "SubgroupSizeHalfQCOM\0" // IR(23076, 21)
  "SubgroupSizeFullQCOM\0" // IR(23097, 21)
  "EarlyAndLateFragmentTestsAMD\0" // IR(23118, 29)
  "StencilRefReplacingEXT\0" // IR(23147, 23)
  "CoalescingAMDX\0" // IR(23170, 15)
  "IsApiEntryAMDX\0" // IR(23185, 15)
  "MaxNodeRecursionAMDX\0" // IR(23200, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(23221, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(23245, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(23266, 28)
  "StencilRefGreaterFrontAMD\0" // IR(23294, 26)
  "StencilRefLessFrontAMD\0" // IR(23320, 23)
  "StencilRefUnchangedBackAMD\0" // IR(23343, 27)
  "StencilRefGreaterBackAMD\0" // IR(23370, 25)
  "StencilRefLessBackAMD\0" // IR(23395, 22)
  "QuadDerivativesKHR\0" // IR(23417, 19)
  "RequireFullQuadsKHR\0" // IR(23436, 20)
  "SharesInputWithAMDX\0" // IR(23456, 20)
  "ArithmeticPoisonKHR\0" // IR(23476, 20)
  "OutputLinesEXT\0" // IR(23496, 15)
  "OutputLinesNV\0" // IR(23511, 14)
  "OutputPrimitivesEXT\0" // IR(23525, 20)
  "OutputPrimitivesNV\0" // IR(23545, 19)
  "DerivativeGroupQuadsKHR\0" // IR(23564, 24)
  "DerivativeGroupQuadsNV\0" // IR(23588, 23)
  "DerivativeGroupLinearKHR\0" // IR(23611, 25)
  "DerivativeGroupLinearNV\0" // IR(23636, 24)
  "OutputTrianglesEXT\0" // IR(23660, 19)
  "OutputTrianglesNV\0" // IR(23679, 18)
  "PixelInterlockOrderedEXT\0" // IR(23697, 25)
  "PixelInterlockUnorderedEXT\0" // IR(23722, 27)
  "SampleInterlockOrderedEXT\0" // IR(23749, 26)
  "SampleInterlockUnorderedEXT\0" // IR(23775, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(23803, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(23834, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(23867, 27)
  "RoundingModeRTPINTEL\0" // IR(23894, 21)
  "RoundingModeRTNINTEL\0" // IR(23915, 21)
  "FloatingPointModeALTINTEL\0" // IR(23936, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(23962, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(23989, 22)
  "MaxWorkDimINTEL\0" // IR(24011, 16)
  "NoGlobalOffsetINTEL\0" // IR(24027, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(24047, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(24069, 28)
  "MaximallyReconvergesKHR\0" // IR(24097, 24)
  "FPFastMathDefault\0" // IR(24121, 18)
  "OpacityMicromapIdKHR\0" // IR(24139, 21)
  "StreamingInterfaceINTEL\0" // IR(24160, 24)
  "RegisterMapInterfaceINTEL\0" // IR(24184, 26)
  "NamedBarrierCountINTEL\0" // IR(24210, 23)
  "MaximumRegistersINTEL\0" // IR(24233, 22)
  "MaximumRegistersIdINTEL\0" // IR(24255, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(24279, 51)
  "NamedMaximumRegistersINTEL\0" // IR(24330, 27)
  "Vertex\0" // IR(24357, 7)
  "TessellationControl\0" // IR(24364, 20)
  "TessellationEvaluation\0" // IR(24384, 23)
  "GLCompute\0" // IR(24407, 10)
  "TaskNV\0" // IR(24417, 7)
  "MeshNV\0" // IR(24424, 7)
  "RayGenerationKHR\0" // IR(24431, 17)
  "RayGenerationNV\0" // IR(24448, 16)
  "IntersectionKHR\0" // IR(24464, 16)
  "IntersectionNV\0" // IR(24480, 15)
  "AnyHitKHR\0" // IR(24495, 10)
  "AnyHitNV\0" // IR(24505, 9)
  "ClosestHitKHR\0" // IR(24514, 14)
  "ClosestHitNV\0" // IR(24528, 13)
  "MissKHR\0" // IR(24541, 8)
  "MissNV\0" // IR(24549, 7)
  "CallableKHR\0" // IR(24556, 12)
  "CallableNV\0" // IR(24568, 11)
  "TaskEXT\0" // IR(24579, 8)
  "MeshEXT\0" // IR(24587, 8)
  "Preserve\0" // IR(24595, 9)
  "FlushToZero\0" // IR(24604, 12)
  "BFloat16KHR\0" // IR(24616, 12)
  "Float8E4M3EXT\0" // IR(24628, 14)
  "Float8E5M2EXT\0" // IR(24642, 14)
  "Float6E2M3EXT\0" // IR(24656, 14)
  "Float6E3M2EXT\0" // IR(24670, 14)
  "Float4E2M1EXT\0" // IR(24684, 14)
  "IEEE\0" // IR(24698, 5)
  "ALT\0" // IR(24703, 4)
  "NotNaN\0" // IR(24707, 7)
  "NotInf\0" // IR(24714, 7)
  "NSZ\0" // IR(24721, 4)
  "AllowRecip\0" // IR(24725, 11)
  "Fast\0" // IR(24736, 5)
  "AllowContract\0" // IR(24741, 14)
  "AllowContractFastINTEL\0" // IR(24755, 23)
  "AllowReassoc\0" // IR(24778, 13)
  "AllowReassocINTEL\0" // IR(24791, 18)
  "AllowTransform\0" // IR(24809, 15)
  "RTE\0" // IR(24824, 4)
  "RTZ\0" // IR(24828, 4)
  "RTP\0" // IR(24832, 4)
  "RTN\0" // IR(24836, 4)
  "Vertical2Pixels\0" // IR(24840, 16)
  "Vertical4Pixels\0" // IR(24856, 16)
  "Horizontal2Pixels\0" // IR(24872, 18)
  "Horizontal4Pixels\0" // IR(24890, 18)
  "Inline\0" // IR(24908, 7)
  "DontInline\0" // IR(24915, 11)
  "Pure\0" // IR(24926, 5)
  "Const\0" // IR(24931, 6)
  "Zext\0" // IR(24937, 5)
  "Sext\0" // IR(24942, 5)
  "ByVal\0" // IR(24947, 6)
  "Sret\0" // IR(24953, 5)
  "NoAlias\0" // IR(24958, 8)
  "NoCapture\0" // IR(24966, 10)
  "NoWrite\0" // IR(24976, 8)
  "NoReadWrite\0" // IR(24984, 12)
  "RuntimeAlignedALTERA\0" // IR(24996, 21)
  "RuntimeAlignedINTEL\0" // IR(25017, 20)
  "Gather4x1QCOM\0" // IR(25037, 14)
  "GatherDQCOM\0" // IR(25051, 12)
  "GatherH2QCOM\0" // IR(25063, 13)
  "GatherV2QCOM\0" // IR(25076, 13)
  "Reduce\0" // IR(25089, 7)
  "InclusiveScan\0" // IR(25096, 14)
  "ExclusiveScan\0" // IR(25110, 14)
  "ClusteredReduce\0" // IR(25124, 16)
  "PartitionedReduceEXT\0" // IR(25140, 21)
  "PartitionedReduceNV\0" // IR(25161, 20)
  "PartitionedInclusiveScanEXT\0" // IR(25181, 28)
  "PartitionedInclusiveScanNV\0" // IR(25209, 27)
  "PartitionedExclusiveScanEXT\0" // IR(25236, 28)
  "PartitionedExclusiveScanNV\0" // IR(25264, 27)
  "NoneINTEL\0" // IR(25291, 10)
  "ReadINTEL\0" // IR(25301, 10)
  "WriteINTEL\0" // IR(25311, 11)
  "ReadWriteINTEL\0" // IR(25322, 15)
  "Bias\0" // IR(25337, 5)
  "Lod\0" // IR(25342, 4)
  "Grad\0" // IR(25346, 5)
  "ConstOffset\0" // IR(25351, 12)
  "ConstOffsets\0" // IR(25363, 13)
  "MakeTexelAvailable\0" // IR(25376, 19)
  "MakeTexelAvailableKHR\0" // IR(25395, 22)
  "MakeTexelVisible\0" // IR(25417, 17)
  "MakeTexelVisibleKHR\0" // IR(25434, 20)
  "NonPrivateTexel\0" // IR(25454, 16)
  "NonPrivateTexelKHR\0" // IR(25470, 19)
  "VolatileTexel\0" // IR(25489, 14)
  "VolatileTexelKHR\0" // IR(25503, 17)
  "SignExtend\0" // IR(25520, 11)
  "ZeroExtend\0" // IR(25531, 11)
  "Nontemporal\0" // IR(25542, 12)
  "Offsets\0" // IR(25554, 8)
  "SnormInt8\0" // IR(25562, 10)
  "SnormInt16\0" // IR(25572, 11)
  "UnormInt8\0" // IR(25583, 10)
  "UnormInt16\0" // IR(25593, 11)
  "UnormShort565\0" // IR(25604, 14)
  "UnormShort555\0" // IR(25618, 14)
  "UnormInt101010\0" // IR(25632, 15)
  "SignedInt8\0" // IR(25647, 11)
  "SignedInt16\0" // IR(25658, 12)
  "SignedInt32\0" // IR(25670, 12)
  "UnsignedInt8\0" // IR(25682, 13)
  "UnsignedInt16\0" // IR(25695, 14)
  "UnsignedInt32\0" // IR(25709, 14)
  "HalfFloat\0" // IR(25723, 10)
  "UnormInt24\0" // IR(25733, 11)
  "UnormInt101010_2\0" // IR(25744, 17)
  "UnormInt10X6EXT\0" // IR(25761, 16)
  "UnsignedIntRaw10EXT\0" // IR(25777, 20)
  "UnsignedIntRaw12EXT\0" // IR(25797, 20)
  "UnormInt2_101010EXT\0" // IR(25817, 20)
  "UnsignedInt10X6EXT\0" // IR(25837, 19)
  "UnsignedInt12X4EXT\0" // IR(25856, 19)
  "UnsignedInt14X2EXT\0" // IR(25875, 19)
  "UnormInt12X4EXT\0" // IR(25894, 16)
  "UnormInt14X2EXT\0" // IR(25910, 16)
  "R\0" // IR(25926, 2)
  "A\0" // IR(25928, 2)
  "RG\0" // IR(25930, 3)
  "RA\0" // IR(25933, 3)
  "RGB\0" // IR(25936, 4)
  "RGBA\0" // IR(25940, 5)
  "BGRA\0" // IR(25945, 5)
  "ARGB\0" // IR(25950, 5)
  "Intensity\0" // IR(25955, 10)
  "Luminance\0" // IR(25965, 10)
  "Rx\0" // IR(25975, 3)
  "RGx\0" // IR(25978, 4)
  "RGBx\0" // IR(25982, 5)
  "Depth\0" // IR(25987, 6)
  "DepthStencil\0" // IR(25993, 13)
  "sRGB\0" // IR(26006, 5)
  "sRGBx\0" // IR(26011, 6)
  "sRGBA\0" // IR(26017, 6)
  "sBGRA\0" // IR(26023, 6)
  "ABGR\0" // IR(26029, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(26034, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(26062, 27)
  "InitOnDeviceResetALTERA\0" // IR(26089, 24)
  "InitOnDeviceResetINTEL\0" // IR(26113, 23)
  "NoWait\0" // IR(26136, 7)
  "WaitKernel\0" // IR(26143, 11)
  "WaitWorkGroup\0" // IR(26154, 14)
  "CmdExecTime\0" // IR(26168, 12)
  "MayUsePrintf\0" // IR(26180, 13)
  "Export\0" // IR(26193, 7)
  "Import\0" // IR(26200, 7)
  "LinkOnceODR\0" // IR(26207, 12)
  "WeakAMD\0" // IR(26219, 8)
  "UncachedINTEL\0" // IR(26227, 14)
  "CachedINTEL\0" // IR(26241, 12)
  "StreamingINTEL\0" // IR(26253, 15)
  "InvalidateAfterReadINTEL\0" // IR(26268, 25)
  "ConstCachedINTEL\0" // IR(26293, 17)
  "Unroll\0" // IR(26310, 7)
  "DontUnroll\0" // IR(26317, 11)
  "DependencyInfinite\0" // IR(26328, 19)
  "DependencyLength\0" // IR(26347, 17)
  "MinIterations\0" // IR(26364, 14)
  "MaxIterations\0" // IR(26378, 14)
  "IterationMultiple\0" // IR(26392, 18)
  "PeelCount\0" // IR(26410, 10)
  "PartialCount\0" // IR(26420, 13)
  "DependencyArrayALTERA\0" // IR(26433, 22)
  "DependencyArrayINTEL\0" // IR(26455, 21)
  "LoopCoalesceALTERA\0" // IR(26476, 19)
  "LoopCoalesceINTEL\0" // IR(26495, 18)
  "MaxInterleavingALTERA\0" // IR(26513, 22)
  "MaxInterleavingINTEL\0" // IR(26535, 21)
  "SpeculatedIterationsALTERA\0" // IR(26556, 27)
  "SpeculatedIterationsINTEL\0" // IR(26583, 26)
  "NoFusionALTERA\0" // IR(26609, 15)
  "NoFusionINTEL\0" // IR(26624, 14)
  "LoopCountALTERA\0" // IR(26638, 16)
  "LoopCountINTEL\0" // IR(26654, 15)
  "MaxReinvocationDelayALTERA\0" // IR(26669, 27)
  "MaxReinvocationDelayINTEL\0" // IR(26696, 26)
  "MatrixASignedComponentsINTEL\0" // IR(26722, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(26751, 29)
  "MatrixCBFloat16INTEL\0" // IR(26780, 21)
  "MatrixResultBFloat16INTEL\0" // IR(26801, 26)
  "MatrixAPackedInt8INTEL\0" // IR(26827, 23)
  "MatrixBPackedInt8INTEL\0" // IR(26850, 23)
  "MatrixAPackedInt4INTEL\0" // IR(26873, 23)
  "MatrixBPackedInt4INTEL\0" // IR(26896, 23)
  "MatrixATF32INTEL\0" // IR(26919, 17)
  "MatrixBTF32INTEL\0" // IR(26936, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(26953, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(26979, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(27005, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(27032, 27)
  "Aligned\0" // IR(27059, 8)
  "MakePointerAvailable\0" // IR(27067, 21)
  "MakePointerAvailableKHR\0" // IR(27088, 24)
  "MakePointerVisible\0" // IR(27112, 19)
  "MakePointerVisibleKHR\0" // IR(27131, 22)
  "NonPrivatePointer\0" // IR(27153, 18)
  "NonPrivatePointerKHR\0" // IR(27171, 21)
  "AliasScopeINTELMask\0" // IR(27192, 20)
  "NoAliasINTELMask\0" // IR(27212, 17)
  "Simple\0" // IR(27229, 7)
  "GLSL450\0" // IR(27236, 8)
  "OpenCL\0" // IR(27244, 7)
  "Vulkan\0" // IR(27251, 7)
  "VulkanKHR\0" // IR(27258, 10)
  "Relaxed\0" // IR(27268, 8)
  "Acquire\0" // IR(27276, 8)
  "Release\0" // IR(27284, 8)
  "AcquireRelease\0" // IR(27292, 15)
  "SequentiallyConsistent\0" // IR(27307, 23)
  "UniformMemory\0" // IR(27330, 14)
  "SubgroupMemory\0" // IR(27344, 15)
  "WorkgroupMemory\0" // IR(27359, 16)
  "CrossWorkgroupMemory\0" // IR(27375, 21)
  "AtomicCounterMemory\0" // IR(27396, 20)
  "ImageMemory\0" // IR(27416, 12)
  "OutputMemory\0" // IR(27428, 13)
  "OutputMemoryKHR\0" // IR(27441, 16)
  "MakeAvailable\0" // IR(27457, 14)
  "MakeAvailableKHR\0" // IR(27471, 17)
  "MakeVisible\0" // IR(27488, 12)
  "MakeVisibleKHR\0" // IR(27500, 15)
  "AutoINTEL\0" // IR(27515, 10)
  "WRAP\0" // IR(27525, 5)
  "SAT\0" // IR(27530, 4)
  "SAT_ZERO\0" // IR(27534, 9)
  "SAT_SYM\0" // IR(27543, 8)
  "PackedVectorFormat4x8Bit\0" // IR(27551, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(27576, 28)
  "TRN\0" // IR(27604, 4)
  "TRN_ZERO\0" // IR(27608, 9)
  "RND\0" // IR(27617, 4)
  "RND_ZERO\0" // IR(27621, 9)
  "RND_INF\0" // IR(27630, 8)
  "RND_MIN_INF\0" // IR(27638, 12)
  "RND_CONV\0" // IR(27650, 9)
  "RND_CONV_ODD\0" // IR(27659, 13)
  "RobustnessPerComponentNV\0" // IR(27672, 25)
  "RobustnessPerElementNV\0" // IR(27697, 23)
  "OpaqueKHR\0" // IR(27720, 10)
  "NoOpaqueKHR\0" // IR(27730, 12)
  "TerminateOnFirstHitKHR\0" // IR(27742, 23)
  "SkipClosestHitShaderKHR\0" // IR(27765, 24)
  "CullBackFacingTrianglesKHR\0" // IR(27789, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(27816, 28)
  "CullOpaqueKHR\0" // IR(27844, 14)
  "CullNoOpaqueKHR\0" // IR(27858, 16)
  "SkipTrianglesKHR\0" // IR(27874, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(27891, 24)
  "SkipAABBsKHR\0" // IR(27915, 13)
  "ForceOpacityMicromap2StateKHR\0" // IR(27928, 30)
  "ForceOpacityMicromap2StateEXT\0" // IR(27958, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(27988, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(28029, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(28066, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(28103, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(28144, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(28186, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(28219, 33)
  "ClampToEdge\0" // IR(28252, 12)
  "Clamp\0" // IR(28264, 6)
  "Repeat\0" // IR(28270, 7)
  "RepeatMirrored\0" // IR(28277, 15)
  "Nearest\0" // IR(28292, 8)
  "Linear\0" // IR(28300, 7)
  "Unknown\0" // IR(28307, 8)
  "Rgba32f\0" // IR(28315, 8)
  "Rgba16f\0" // IR(28323, 8)
  "R32f\0" // IR(28331, 5)
  "Rgba8\0" // IR(28336, 6)
  "Rgba8Snorm\0" // IR(28342, 11)
  "Rg32f\0" // IR(28353, 6)
  "Rg16f\0" // IR(28359, 6)
  "R11fG11fB10f\0" // IR(28365, 13)
  "R16f\0" // IR(28378, 5)
  "Rgba16\0" // IR(28383, 7)
  "Rgb10A2\0" // IR(28390, 8)
  "Rg16\0" // IR(28398, 5)
  "Rg8\0" // IR(28403, 4)
  "R16\0" // IR(28407, 4)
  "R8\0" // IR(28411, 3)
  "Rgba16Snorm\0" // IR(28414, 12)
  "Rg16Snorm\0" // IR(28426, 10)
  "Rg8Snorm\0" // IR(28436, 9)
  "R16Snorm\0" // IR(28445, 9)
  "R8Snorm\0" // IR(28454, 8)
  "Rgba32i\0" // IR(28462, 8)
  "Rgba16i\0" // IR(28470, 8)
  "Rgba8i\0" // IR(28478, 7)
  "R32i\0" // IR(28485, 5)
  "Rg32i\0" // IR(28490, 6)
  "Rg16i\0" // IR(28496, 6)
  "Rg8i\0" // IR(28502, 5)
  "R16i\0" // IR(28507, 5)
  "R8i\0" // IR(28512, 4)
  "Rgba32ui\0" // IR(28516, 9)
  "Rgba16ui\0" // IR(28525, 9)
  "Rgba8ui\0" // IR(28534, 8)
  "R32ui\0" // IR(28542, 6)
  "Rgb10a2ui\0" // IR(28548, 10)
  "Rg32ui\0" // IR(28558, 7)
  "Rg16ui\0" // IR(28565, 7)
  "Rg8ui\0" // IR(28572, 6)
  "R16ui\0" // IR(28578, 6)
  "R8ui\0" // IR(28584, 5)
  "R64ui\0" // IR(28589, 6)
  "R64i\0" // IR(28595, 5)
  "CrossDevice\0" // IR(28600, 12)
  "Device\0" // IR(28612, 7)
  "Workgroup\0" // IR(28619, 10)
  "Subgroup\0" // IR(28629, 9)
  "Invocation\0" // IR(28638, 11)
  "QueueFamily\0" // IR(28649, 12)
  "QueueFamilyKHR\0" // IR(28661, 15)
  "ShaderCallKHR\0" // IR(28676, 14)
  "Flatten\0" // IR(28690, 8)
  "DontFlatten\0" // IR(28698, 12)
  "IdentifierPossibleDuplicates\0" // IR(28710, 29)
  "FlagUnknownPhysicalLayout\0" // IR(28739, 26)
  "ESSL\0" // IR(28765, 5)
  "GLSL\0" // IR(28770, 5)
  "OpenCL_C\0" // IR(28775, 9)
  "OpenCL_CPP\0" // IR(28784, 11)
  "HLSL\0" // IR(28795, 5)
  "CPP_for_OpenCL\0" // IR(28800, 15)
  "SYCL\0" // IR(28815, 5)
  "HERO_C\0" // IR(28820, 7)
  "NZSL\0" // IR(28827, 5)
  "WGSL\0" // IR(28832, 5)
  "Slang\0" // IR(28837, 6)
  "Zig\0" // IR(28843, 4)
  "Rust\0" // IR(28847, 5)
  "Pred\0" // IR(28852, 5)
  "ApilaJai\0" // IR(28857, 9)
  "UniformConstant\0" // IR(28866, 16)
  "Input\0" // IR(28882, 6)
  "Output\0" // IR(28888, 7)
  "CrossWorkgroup\0" // IR(28895, 15)
  "Private\0" // IR(28910, 8)
  "Function\0" // IR(28918, 9)
  "Generic\0" // IR(28927, 8)
  "PushConstant\0" // IR(28935, 13)
  "AtomicCounter\0" // IR(28948, 14)
  "Image\0" // IR(28962, 6)
  "StorageBuffer\0" // IR(28968, 14)
  "TileImageEXT\0" // IR(28982, 13)
  "TileAttachmentQCOM\0" // IR(28995, 19)
  "NodePayloadAMDX\0" // IR(29014, 16)
  "CallableDataKHR\0" // IR(29030, 16)
  "CallableDataNV\0" // IR(29046, 15)
  "IncomingCallableDataKHR\0" // IR(29061, 24)
  "IncomingCallableDataNV\0" // IR(29085, 23)
  "RayPayloadKHR\0" // IR(29108, 14)
  "RayPayloadNV\0" // IR(29122, 13)
  "HitAttributeKHR\0" // IR(29135, 16)
  "HitAttributeNV\0" // IR(29151, 15)
  "IncomingRayPayloadKHR\0" // IR(29166, 22)
  "IncomingRayPayloadNV\0" // IR(29188, 21)
  "ShaderRecordBufferKHR\0" // IR(29209, 22)
  "ShaderRecordBufferNV\0" // IR(29231, 21)
  "PhysicalStorageBuffer\0" // IR(29252, 22)
  "PhysicalStorageBufferEXT\0" // IR(29274, 25)
  "HitObjectAttributeNV\0" // IR(29299, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(29320, 24)
  "HitObjectAttributeEXT\0" // IR(29344, 22)
  "CodeSectionINTEL\0" // IR(29366, 17)
  "DeviceOnlyALTERA\0" // IR(29383, 17)
  "DeviceOnlyINTEL\0" // IR(29400, 16)
  "HostOnlyALTERA\0" // IR(29416, 15)
  "HostOnlyINTEL\0" // IR(29431, 14)
  "WriteThroughINTEL\0" // IR(29445, 18)
  "WriteBackINTEL\0" // IR(29463, 15)
  "TensorView\0" // IR(29478, 11)
  "DecodeFunc\0" // IR(29489, 11)
  "DecodeVectorFunc\0" // IR(29500, 17)
  "Undefined\0" // IR(29517, 10)
  "NoneARM\0" // IR(29527, 8)
  "NontemporalARM\0" // IR(29535, 15)
  "OutOfBoundsValueARM\0" // IR(29550, 20)
  "MakeElementAvailableARM\0" // IR(29570, 24)
  "MakeElementVisibleARM\0" // IR(29594, 22)
  "NonPrivateElementARM\0" // IR(29616, 21)
  "Nop\0" // IR(29637, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(29641, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(29666, 27)
  "Undef\0" // IR(29693, 6)
  "SourceContinued\0" // IR(29699, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(29715, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(29748, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(29777, 41)
  "Source\0" // IR(29818, 7)
  "SourceExtension\0" // IR(29825, 16)
  "Name\0" // IR(29841, 5)
  "MemberName\0" // IR(29846, 11)
  "String\0" // IR(29857, 7)
  "Line\0" // IR(29864, 5)
  "Extension\0" // IR(29869, 10)
  "ExtInstImport\0" // IR(29879, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(29893, 46)
  "ExtInst\0" // IR(29939, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(29947, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(29981, 30)
  "MemoryModel\0" // IR(30011, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(30023, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(30056, 29)
  "EntryPoint\0" // IR(30085, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(30096, 32)
  "ExecutionMode\0" // IR(30128, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(30142, 28)
  "Capability\0" // IR(30170, 11)
  "TypeVoid\0" // IR(30181, 9)
  "TypeBool\0" // IR(30190, 9)
  "TypeInt\0" // IR(30199, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(30207, 37)
  "TypeFloat\0" // IR(30244, 10)
  "TypeVector\0" // IR(30254, 11)
  "TypeMatrix\0" // IR(30265, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(30276, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(30308, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(30346, 43)
  "TypeImage\0" // IR(30389, 10)
  "TypeSampler\0" // IR(30399, 12)
  "TypeSampledImage\0" // IR(30411, 17)
  "TypeArray\0" // IR(30428, 10)
  "TypeRuntimeArray\0" // IR(30438, 17)
  "TypeStruct\0" // IR(30455, 11)
  "TypeOpaque\0" // IR(30466, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(30477, 31)
  "TypePointer\0" // IR(30508, 12)
  "TypeFunction\0" // IR(30520, 13)
  "TypeEvent\0" // IR(30533, 10)
  "TypeDeviceEvent\0" // IR(30543, 16)
  "TypeReserveId\0" // IR(30559, 14)
  "TypeQueue\0" // IR(30573, 10)
  "TypePipe\0" // IR(30583, 9)
  "TypeForwardPointer\0" // IR(30592, 19)
  "ConstantTrue\0" // IR(30611, 13)
  "ConstantFalse\0" // IR(30624, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(30638, 38)
  "ConstantComposite\0" // IR(30676, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(30694, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(30735, 37)
  "ConstantSampler\0" // IR(30772, 16)
  "ConstantNull\0" // IR(30788, 13)
  "SpecConstantTrue\0" // IR(30801, 17)
  "SpecConstantFalse\0" // IR(30818, 18)
  "SpecConstant\0" // IR(30836, 13)
  "SpecConstantComposite\0" // IR(30849, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(30871, 41)
  "SpecConstantOp\0" // IR(30912, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(30927, 34)
  "FunctionParameter\0" // IR(30961, 18)
  "FunctionEnd\0" // IR(30979, 12)
  "FunctionCall\0" // IR(30991, 13)
  "Variable\0" // IR(31004, 9)
  "ImageTexelPointer\0" // IR(31013, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(31031, 40)
  "Load\0" // IR(31071, 5)
  "Store\0" // IR(31076, 6)
  "CopyMemory\0" // IR(31082, 11)
  "CopyMemorySized\0" // IR(31093, 16)
  "AccessChain\0" // IR(31109, 12)
  "InBoundsAccessChain\0" // IR(31121, 20)
  "PtrAccessChain\0" // IR(31141, 15)
  "ArrayLength\0" // IR(31156, 12)
  "GenericPtrMemSemantics\0" // IR(31168, 23)
  "InBoundsPtrAccessChain\0" // IR(31191, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(31214, 28)
  "Decorate\0" // IR(31242, 9)
  "MemberDecorate\0" // IR(31251, 15)
  "DecorationGroup\0" // IR(31266, 16)
  "GroupDecorate\0" // IR(31282, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(31296, 45)
  "GroupMemberDecorate\0" // IR(31341, 20)
  "VectorExtractDynamic\0" // IR(31361, 21)
  "VectorInsertDynamic\0" // IR(31382, 20)
  "VectorShuffle\0" // IR(31402, 14)
  "CompositeConstruct\0" // IR(31416, 19)
  "CompositeExtract\0" // IR(31435, 17)
  "CompositeInsert\0" // IR(31452, 16)
  "CopyObject\0" // IR(31468, 11)
  "Transpose\0" // IR(31479, 10)
  "SampledImage\0" // IR(31489, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(31502, 32)
  "ImageSampleImplicitLod\0" // IR(31534, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(31557, 23)
  "ImageSampleExplicitLod\0" // IR(31580, 23)
  "ImageSampleDrefImplicitLod\0" // IR(31603, 27)
  "ImageSampleDrefExplicitLod\0" // IR(31630, 27)
  "ImageSampleProjImplicitLod\0" // IR(31657, 27)
  "ImageSampleProjExplicitLod\0" // IR(31684, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(31711, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(31742, 31)
  "ImageFetch\0" // IR(31773, 11)
  "ImageGather\0" // IR(31784, 12)
  "ImageDrefGather\0" // IR(31796, 16)
  "ImageRead\0" // IR(31812, 10)
  "ImageWrite\0" // IR(31822, 11)
  "ImageQueryFormat\0" // IR(31833, 17)
  "ImageQueryOrder\0" // IR(31850, 16)
  "ImageQuerySizeLod\0" // IR(31866, 18)
  "ImageQuerySize\0" // IR(31884, 15)
  "ImageQueryLod\0" // IR(31899, 14)
  "ImageQueryLevels\0" // IR(31913, 17)
  "ImageQuerySamples\0" // IR(31930, 18)
  "ConvertFToU\0" // IR(31948, 12)
  "ConvertFToS\0" // IR(31960, 12)
  "ConvertSToF\0" // IR(31972, 12)
  "ConvertUToF\0" // IR(31984, 12)
  "UConvert\0" // IR(31996, 9)
  "SConvert\0" // IR(32005, 9)
  "FConvert\0" // IR(32014, 9)
  "QuantizeToF16\0" // IR(32023, 14)
  "ConvertPtrToU\0" // IR(32037, 14)
  "SatConvertSToU\0" // IR(32051, 15)
  "SatConvertUToS\0" // IR(32066, 15)
  "ConvertUToPtr\0" // IR(32081, 14)
  "PtrCastToGeneric\0" // IR(32095, 17)
  "GenericCastToPtr\0" // IR(32112, 17)
  "GenericCastToPtrExplicit\0" // IR(32129, 25)
  "Bitcast\0" // IR(32154, 8)
  "SNegate\0" // IR(32162, 8)
  "FNegate\0" // IR(32170, 8)
  "IAdd\0" // IR(32178, 5)
  "FAdd\0" // IR(32183, 5)
  "ISub\0" // IR(32188, 5)
  "FSub\0" // IR(32193, 5)
  "IMul\0" // IR(32198, 5)
  "FMul\0" // IR(32203, 5)
  "UDiv\0" // IR(32208, 5)
  "SDiv\0" // IR(32213, 5)
  "FDiv\0" // IR(32218, 5)
  "UMod\0" // IR(32223, 5)
  "SRem\0" // IR(32228, 5)
  "SMod\0" // IR(32233, 5)
  "FRem\0" // IR(32238, 5)
  "FMod\0" // IR(32243, 5)
  "VectorTimesScalar\0" // IR(32248, 18)
  "MatrixTimesScalar\0" // IR(32266, 18)
  "VectorTimesMatrix\0" // IR(32284, 18)
  "MatrixTimesVector\0" // IR(32302, 18)
  "MatrixTimesMatrix\0" // IR(32320, 18)
  "OuterProduct\0" // IR(32338, 13)
  "Dot\0" // IR(32351, 4)
  "IAddCarry\0" // IR(32355, 10)
  "ISubBorrow\0" // IR(32365, 11)
  "UMulExtended\0" // IR(32376, 13)
  "SMulExtended\0" // IR(32389, 13)
  "Any\0" // IR(32402, 4)
  "All\0" // IR(32406, 4)
  "IsNan\0" // IR(32410, 6)
  "IsInf\0" // IR(32416, 6)
  "IsFinite\0" // IR(32422, 9)
  "IsNormal\0" // IR(32431, 9)
  "SignBitSet\0" // IR(32440, 11)
  "LessOrGreater\0" // IR(32451, 14)
  "Ordered\0" // IR(32465, 8)
  "Unordered\0" // IR(32473, 10)
  "LogicalEqual\0" // IR(32483, 13)
  "LogicalNotEqual\0" // IR(32496, 16)
  "LogicalOr\0" // IR(32512, 10)
  "LogicalAnd\0" // IR(32522, 11)
  "LogicalNot\0" // IR(32533, 11)
  "Select\0" // IR(32544, 7)
  "IEqual\0" // IR(32551, 7)
  "INotEqual\0" // IR(32558, 10)
  "UGreaterThan\0" // IR(32568, 13)
  "SGreaterThan\0" // IR(32581, 13)
  "UGreaterThanEqual\0" // IR(32594, 18)
  "SGreaterThanEqual\0" // IR(32612, 18)
  "ULessThan\0" // IR(32630, 10)
  "SLessThan\0" // IR(32640, 10)
  "ULessThanEqual\0" // IR(32650, 15)
  "SLessThanEqual\0" // IR(32665, 15)
  "FOrdEqual\0" // IR(32680, 10)
  "FUnordEqual\0" // IR(32690, 12)
  "FOrdNotEqual\0" // IR(32702, 13)
  "FUnordNotEqual\0" // IR(32715, 15)
  "FOrdLessThan\0" // IR(32730, 13)
  "FUnordLessThan\0" // IR(32743, 15)
  "FOrdGreaterThan\0" // IR(32758, 16)
  "FUnordGreaterThan\0" // IR(32774, 18)
  "FOrdLessThanEqual\0" // IR(32792, 18)
  "FUnordLessThanEqual\0" // IR(32810, 20)
  "FOrdGreaterThanEqual\0" // IR(32830, 21)
  "FUnordGreaterThanEqual\0" // IR(32851, 23)
  "ShiftRightLogical\0" // IR(32874, 18)
  "ShiftRightArithmetic\0" // IR(32892, 21)
  "ShiftLeftLogical\0" // IR(32913, 17)
  "BitwiseOr\0" // IR(32930, 10)
  "BitwiseXor\0" // IR(32940, 11)
  "BitwiseAnd\0" // IR(32951, 11)
  "Not\0" // IR(32962, 4)
  "BitFieldInsert\0" // IR(32966, 15)
  "BitFieldSExtract\0" // IR(32981, 17)
  "BitFieldUExtract\0" // IR(32998, 17)
  "BitReverse\0" // IR(33015, 11)
  "BitCount\0" // IR(33026, 9)
  "DPdx\0" // IR(33035, 5)
  "DPdy\0" // IR(33040, 5)
  "Fwidth\0" // IR(33045, 7)
  "DPdxFine\0" // IR(33052, 9)
  "DPdyFine\0" // IR(33061, 9)
  "FwidthFine\0" // IR(33070, 11)
  "DPdxCoarse\0" // IR(33081, 11)
  "DPdyCoarse\0" // IR(33092, 11)
  "FwidthCoarse\0" // IR(33103, 13)
  "EmitVertex\0" // IR(33116, 11)
  "EndPrimitive\0" // IR(33127, 13)
  "EmitStreamVertex\0" // IR(33140, 17)
  "EndStreamPrimitive\0" // IR(33157, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(33176, 37)
  "ControlBarrier\0" // IR(33213, 15)
  "MemoryBarrier\0" // IR(33228, 14)
  "AtomicLoad\0" // IR(33242, 11)
  "AtomicStore\0" // IR(33253, 12)
  "AtomicExchange\0" // IR(33265, 15)
  "AtomicCompareExchange\0" // IR(33280, 22)
  "AtomicCompareExchangeWeak\0" // IR(33302, 26)
  "AtomicIIncrement\0" // IR(33328, 17)
  "AtomicIDecrement\0" // IR(33345, 17)
  "AtomicIAdd\0" // IR(33362, 11)
  "AtomicISub\0" // IR(33373, 11)
  "AtomicSMin\0" // IR(33384, 11)
  "AtomicUMin\0" // IR(33395, 11)
  "AtomicSMax\0" // IR(33406, 11)
  "AtomicUMax\0" // IR(33417, 11)
  "AtomicAnd\0" // IR(33428, 10)
  "AtomicOr\0" // IR(33438, 9)
  "AtomicXor\0" // IR(33447, 10)
  "Phi\0" // IR(33457, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(33461, 30)
  "LoopMerge\0" // IR(33491, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(33501, 35)
  "SelectionMerge\0" // IR(33536, 15)
  "Label\0" // IR(33551, 6)
  "Branch\0" // IR(33557, 7)
  "BranchConditional\0" // IR(33564, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(33582, 45)
  "Switch\0" // IR(33627, 7)
  "Kill\0" // IR(33634, 5)
  "Return\0" // IR(33639, 7)
  "ReturnValue\0" // IR(33646, 12)
  "Unreachable\0" // IR(33658, 12)
  "LifetimeStart\0" // IR(33670, 14)
  "LifetimeStop\0" // IR(33684, 13)
  "GroupAsyncCopy\0" // IR(33697, 15)
  "GroupWaitEvents\0" // IR(33712, 16)
  "GroupAll\0" // IR(33728, 9)
  "GroupAny\0" // IR(33737, 9)
  "GroupBroadcast\0" // IR(33746, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(33761, 33)
  "GroupIAdd\0" // IR(33794, 10)
  "GroupFAdd\0" // IR(33804, 10)
  "GroupFMin\0" // IR(33814, 10)
  "GroupUMin\0" // IR(33824, 10)
  "GroupSMin\0" // IR(33834, 10)
  "GroupFMax\0" // IR(33844, 10)
  "GroupUMax\0" // IR(33854, 10)
  "GroupSMax\0" // IR(33864, 10)
  "ReadPipe\0" // IR(33874, 9)
  "WritePipe\0" // IR(33883, 10)
  "ReservedReadPipe\0" // IR(33893, 17)
  "ReservedWritePipe\0" // IR(33910, 18)
  "ReserveReadPipePackets\0" // IR(33928, 23)
  "ReserveWritePipePackets\0" // IR(33951, 24)
  "CommitReadPipe\0" // IR(33975, 15)
  "CommitWritePipe\0" // IR(33990, 16)
  "IsValidReserveId\0" // IR(34006, 17)
  "GetNumPipePackets\0" // IR(34023, 18)
  "GetMaxPipePackets\0" // IR(34041, 18)
  "GroupReserveReadPipePackets\0" // IR(34059, 28)
  "GroupReserveWritePipePackets\0" // IR(34087, 29)
  "GroupCommitReadPipe\0" // IR(34116, 20)
  "GroupCommitWritePipe\0" // IR(34136, 21)
  "EnqueueMarker\0" // IR(34157, 14)
  "EnqueueKernel\0" // IR(34171, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(34185, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(34215, 32)
  "GetKernelWorkGroupSize\0" // IR(34247, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(34270, 40)
  "RetainEvent\0" // IR(34310, 12)
  "ReleaseEvent\0" // IR(34322, 13)
  "CreateUserEvent\0" // IR(34335, 16)
  "IsValidEvent\0" // IR(34351, 13)
  "SetUserEventStatus\0" // IR(34364, 19)
  "CaptureEventProfilingInfo\0" // IR(34383, 26)
  "GetDefaultQueue\0" // IR(34409, 16)
  "BuildNDRange\0" // IR(34425, 13)
  "ImageSparseSampleImplicitLod\0" // IR(34438, 29)
  "ImageSparseSampleExplicitLod\0" // IR(34467, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(34496, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(34529, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(34562, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(34595, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(34628, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(34665, 37)
  "ImageSparseFetch\0" // IR(34702, 17)
  "ImageSparseGather\0" // IR(34719, 18)
  "ImageSparseDrefGather\0" // IR(34737, 22)
  "ImageSparseTexelsResident\0" // IR(34759, 26)
  "NoLine\0" // IR(34785, 7)
  "AtomicFlagTestAndSet\0" // IR(34792, 21)
  "AtomicFlagClear\0" // IR(34813, 16)
  "ImageSparseRead\0" // IR(34829, 16)
  "SizeOf\0" // IR(34845, 7)
  "TypePipeStorage\0" // IR(34852, 16)
  "ConstantPipeStorage\0" // IR(34868, 20)
  "CreatePipeFromPipeStorage\0" // IR(34888, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(34914, 35)
  "GetKernelMaxNumSubgroups\0" // IR(34949, 25)
  "TypeNamedBarrier\0" // IR(34974, 17)
  "NamedBarrierInitialize\0" // IR(34991, 23)
  "MemoryNamedBarrier\0" // IR(35014, 19)
  "ModuleProcessed\0" // IR(35033, 16)
  "ExecutionModeId\0" // IR(35049, 16)
  "DecorateId\0" // IR(35065, 11)
  "GroupNonUniformElect\0" // IR(35076, 21)
  "GroupNonUniformAll\0" // IR(35097, 19)
  "GroupNonUniformAny\0" // IR(35116, 19)
  "GroupNonUniformAllEqual\0" // IR(35135, 24)
  "GroupNonUniformBroadcast\0" // IR(35159, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(35184, 30)
  "GroupNonUniformInverseBallot\0" // IR(35214, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(35243, 32)
  "GroupNonUniformBallotBitCount\0" // IR(35275, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(35305, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(35334, 29)
  "GroupNonUniformShuffleXor\0" // IR(35363, 26)
  "GroupNonUniformShuffleUp\0" // IR(35389, 25)
  "GroupNonUniformShuffleDown\0" // IR(35414, 27)
  "GroupNonUniformIAdd\0" // IR(35441, 20)
  "GroupNonUniformFAdd\0" // IR(35461, 20)
  "GroupNonUniformIMul\0" // IR(35481, 20)
  "GroupNonUniformFMul\0" // IR(35501, 20)
  "GroupNonUniformSMin\0" // IR(35521, 20)
  "GroupNonUniformUMin\0" // IR(35541, 20)
  "GroupNonUniformFMin\0" // IR(35561, 20)
  "GroupNonUniformSMax\0" // IR(35581, 20)
  "GroupNonUniformUMax\0" // IR(35601, 20)
  "GroupNonUniformFMax\0" // IR(35621, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(35641, 26)
  "GroupNonUniformBitwiseOr\0" // IR(35667, 25)
  "GroupNonUniformBitwiseXor\0" // IR(35692, 26)
  "GroupNonUniformLogicalAnd\0" // IR(35718, 26)
  "GroupNonUniformLogicalOr\0" // IR(35744, 25)
  "GroupNonUniformLogicalXor\0" // IR(35769, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(35795, 29)
  "GroupNonUniformQuadSwap\0" // IR(35824, 24)
  "CopyLogical\0" // IR(35848, 12)
  "PtrEqual\0" // IR(35860, 9)
  "PtrNotEqual\0" // IR(35869, 12)
  "PtrDiff\0" // IR(35881, 8)
  "ColorAttachmentReadEXT\0" // IR(35889, 23)
  "DepthAttachmentReadEXT\0" // IR(35912, 23)
  "StencilAttachmentReadEXT\0" // IR(35935, 25)
  "TypeTensorARM\0" // IR(35960, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(35974, 42)
  "TensorReadARM\0" // IR(36016, 14)
  "TensorWriteARM\0" // IR(36030, 15)
  "TensorQuerySizeARM\0" // IR(36045, 19)
  "GraphConstantARM\0" // IR(36064, 17)
  "GraphEntryPointARM\0" // IR(36081, 19)
  "GraphInputARM\0" // IR(36100, 14)
  "GraphSetOutputARM\0" // IR(36114, 18)
  "GraphEndARM\0" // IR(36132, 12)
  "TypeGraphARM\0" // IR(36144, 13)
  "TerminateInvocation\0" // IR(36157, 20)
  "TypeUntypedPointerKHR\0" // IR(36177, 22)
  "UntypedVariableKHR\0" // IR(36199, 19)
  "UntypedAccessChainKHR\0" // IR(36218, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(36240, 30)
  "SubgroupFirstInvocationKHR\0" // IR(36270, 27)
  "UntypedPtrAccessChainKHR\0" // IR(36297, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(36322, 33)
  "UntypedArrayLengthKHR\0" // IR(36355, 22)
  "UntypedPrefetchKHR\0" // IR(36377, 19)
  "FmaKHR\0" // IR(36396, 7)
  "SubgroupAllKHR\0" // IR(36403, 15)
  "SubgroupAnyKHR\0" // IR(36418, 15)
  "SubgroupAllEqualKHR\0" // IR(36433, 20)
  "SubgroupReadInvocationKHR\0" // IR(36453, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(36479, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(36505, 25)
  "TraceRayKHR\0" // IR(36530, 12)
  "ExecuteCallableKHR\0" // IR(36542, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(36561, 35)
  "IgnoreIntersectionKHR\0" // IR(36596, 22)
  "TerminateRayKHR\0" // IR(36618, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(36634, 47)
  "SDot\0" // IR(36681, 5)
  "SDotKHR\0" // IR(36686, 8)
  "UDot\0" // IR(36694, 5)
  "UDotKHR\0" // IR(36699, 8)
  "SUDot\0" // IR(36707, 6)
  "SUDotKHR\0" // IR(36713, 9)
  "SDotAccSat\0" // IR(36722, 11)
  "SDotAccSatKHR\0" // IR(36733, 14)
  "UDotAccSat\0" // IR(36747, 11)
  "UDotAccSatKHR\0" // IR(36758, 14)
  "SUDotAccSat\0" // IR(36772, 12)
  "SUDotAccSatKHR\0" // IR(36784, 15)
  "TypeCooperativeMatrixKHR\0" // IR(36799, 25)
  "CooperativeMatrixLoadKHR\0" // IR(36824, 25)
  "CooperativeMatrixStoreKHR\0" // IR(36849, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(36875, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(36929, 27)
  "CooperativeMatrixLengthKHR\0" // IR(36956, 27)
  "ConstantCompositeReplicateEXT\0" // IR(36983, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(37013, 34)
  "CompositeConstructReplicateEXT\0" // IR(37047, 31)
  "TypeRayQueryKHR\0" // IR(37078, 16)
  "RayQueryInitializeKHR\0" // IR(37094, 22)
  "RayQueryTerminateKHR\0" // IR(37116, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(37137, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(37169, 31)
  "RayQueryProceedKHR\0" // IR(37200, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(37219, 31)
  "ImageSampleWeightedQCOM\0" // IR(37250, 24)
  "ImageBoxFilterQCOM\0" // IR(37274, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(37293, 23)
  "ImageBlockMatchSADQCOM\0" // IR(37316, 23)
  "BitCastArrayQCOM\0" // IR(37339, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(37356, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(37385, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(37414, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(37443, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(37472, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(37502, 28)
  "ExtractSubArrayQCOM\0" // IR(37530, 20)
  "ImageGatherQCOM\0" // IR(37550, 16)
  "GroupIAddNonUniformAMD\0" // IR(37566, 23)
  "GroupFAddNonUniformAMD\0" // IR(37589, 23)
  "GroupFMinNonUniformAMD\0" // IR(37612, 23)
  "GroupUMinNonUniformAMD\0" // IR(37635, 23)
  "GroupSMinNonUniformAMD\0" // IR(37658, 23)
  "GroupFMaxNonUniformAMD\0" // IR(37681, 23)
  "GroupUMaxNonUniformAMD\0" // IR(37704, 23)
  "GroupSMaxNonUniformAMD\0" // IR(37727, 23)
  "FragmentMaskFetchAMD\0" // IR(37750, 21)
  "FragmentFetchAMD\0" // IR(37771, 17)
  "ReadClockKHR\0" // IR(37788, 13)
  "AllocateNodePayloadsAMDX\0" // IR(37801, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(37826, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(37850, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(37875, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(37904, 27)
  "IsNodePayloadValidAMDX\0" // IR(37931, 23)
  "ConstantStringAMDX\0" // IR(37954, 19)
  "SpecConstantStringAMDX\0" // IR(37973, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(37996, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(38022, 26)
  "TypeBufferEXT\0" // IR(38048, 14)
  "BufferPointerEXT\0" // IR(38062, 17)
  "UntypedImageTexelPointerEXT\0" // IR(38079, 28)
  "MemberDecorateIdEXT\0" // IR(38107, 20)
  "ConstantSizeOfEXT\0" // IR(38127, 18)
  "SpecConstantDataKHR\0" // IR(38145, 20)
  "PoisonKHR\0" // IR(38165, 10)
  "FreezeKHR\0" // IR(38175, 10)
  "HitObjectRecordHitMotionNV\0" // IR(38185, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(38212, 36)
  "HitObjectRecordMissMotionNV\0" // IR(38248, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(38276, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(38304, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(38332, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(38365, 30)
  "HitObjectTraceRayMotionNV\0" // IR(38395, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(38421, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(38460, 44)
  "HitObjectRecordEmptyNV\0" // IR(38504, 23)
  "HitObjectTraceRayNV\0" // IR(38527, 20)
  "HitObjectRecordHitNV\0" // IR(38547, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(38568, 30)
  "HitObjectRecordMissNV\0" // IR(38598, 22)
  "HitObjectExecuteShaderNV\0" // IR(38620, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(38645, 26)
  "HitObjectGetAttributesNV\0" // IR(38671, 25)
  "HitObjectGetHitKindNV\0" // IR(38696, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(38718, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(38747, 28)
  "HitObjectGetInstanceIdNV\0" // IR(38775, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(38800, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(38834, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(38866, 29)
  "HitObjectGetRayTMaxNV\0" // IR(38895, 22)
  "HitObjectGetRayTMinNV\0" // IR(38917, 22)
  "HitObjectIsEmptyNV\0" // IR(38939, 19)
  "HitObjectIsHitNV\0" // IR(38958, 17)
  "HitObjectIsMissNV\0" // IR(38975, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(38993, 29)
  "ReorderThreadWithHintNV\0" // IR(39022, 24)
  "TypeHitObjectNV\0" // IR(39046, 16)
  "ImageSampleFootprintNV\0" // IR(39062, 23)
  "TypeVectorIdEXT\0" // IR(39085, 16)
  "TypeCooperativeVectorNV\0" // IR(39101, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(39125, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(39154, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(39196, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(39235, 32)
  "CooperativeMatrixConvertUseEXT\0" // IR(39267, 31)
  "CooperativeMatrixConvertNV\0" // IR(39298, 27)
  "EmitMeshTasksEXT\0" // IR(39325, 17)
  "SetMeshOutputsEXT\0" // IR(39342, 18)
  "GroupNonUniformPartitionEXT\0" // IR(39360, 28)
  "GroupNonUniformPartitionNV\0" // IR(39388, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(39415, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(39448, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(39483, 38)
  "CooperativeVectorLoadNV\0" // IR(39521, 24)
  "CooperativeVectorStoreNV\0" // IR(39545, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(39570, 28)
  "HitObjectRecordMissEXT\0" // IR(39598, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(39621, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(39650, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(39701, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(39725, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(39770, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(39803, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(39835, 38)
  "TypeHitObjectEXT\0" // IR(39873, 17)
  "ReorderThreadWithHintEXT\0" // IR(39890, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(39915, 30)
  "HitObjectTraceRayEXT\0" // IR(39945, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(39966, 27)
  "HitObjectRecordEmptyEXT\0" // IR(39993, 24)
  "HitObjectExecuteShaderEXT\0" // IR(40017, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(40043, 27)
  "HitObjectGetAttributesEXT\0" // IR(40070, 26)
  "HitObjectGetHitKindEXT\0" // IR(40096, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(40119, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(40149, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(40178, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(40204, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(40239, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(40270, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(40304, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(40337, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(40367, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(40396, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(40425, 23)
  "ReportIntersectionKHR\0" // IR(40448, 22)
  "ReportIntersectionNV\0" // IR(40470, 21)
  "IgnoreIntersectionNV\0" // IR(40491, 21)
  "TerminateRayNV\0" // IR(40512, 15)
  "TraceNV\0" // IR(40527, 8)
  "TraceMotionNV\0" // IR(40535, 14)
  "TraceRayMotionNV\0" // IR(40549, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(40566, 50)
  "TypeAccelerationStructureKHR\0" // IR(40616, 29)
  "TypeAccelerationStructureNV\0" // IR(40645, 28)
  "ExecuteCallableNV\0" // IR(40673, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(40691, 35)
  "RayQueryGetClusterIdNV\0" // IR(40726, 23)
  "HitObjectGetClusterIdNV\0" // IR(40749, 24)
  "HitObjectGetRayTMinEXT\0" // IR(40773, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(40796, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(40841, 40)
  "HitObjectIsEmptyEXT\0" // IR(40881, 20)
  "HitObjectIsHitEXT\0" // IR(40901, 18)
  "HitObjectIsMissEXT\0" // IR(40919, 19)
  "TypeCooperativeMatrixNV\0" // IR(40938, 24)
  "CooperativeMatrixLoadNV\0" // IR(40962, 24)
  "CooperativeMatrixStoreNV\0" // IR(40986, 25)
  "CooperativeMatrixMulAddNV\0" // IR(41011, 26)
  "CooperativeMatrixLengthNV\0" // IR(41037, 26)
  "BeginInvocationInterlockEXT\0" // IR(41063, 28)
  "EndInvocationInterlockEXT\0" // IR(41091, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(41117, 43)
  "CooperativeMatrixReduceEXT\0" // IR(41160, 27)
  "CooperativeMatrixReduceNV\0" // IR(41187, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(41213, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(41244, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(41288, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(41318, 31)
  "CooperativeMatrixPerElementOpEXT\0" // IR(41349, 33)
  "CooperativeMatrixPerElementOpNV\0" // IR(41382, 32)
  "TypeTensorLayoutNV\0" // IR(41414, 19)
  "TypeTensorViewNV\0" // IR(41433, 17)
  "CreateTensorLayoutNV\0" // IR(41450, 21)
  "TensorLayoutSetDimensionNV\0" // IR(41471, 27)
  "TensorLayoutSetStrideNV\0" // IR(41498, 24)
  "TensorLayoutSliceNV\0" // IR(41522, 20)
  "TensorLayoutSetClampValueNV\0" // IR(41542, 28)
  "CreateTensorViewNV\0" // IR(41570, 19)
  "TensorViewSetDimensionNV\0" // IR(41589, 25)
  "TensorViewSetStrideNV\0" // IR(41614, 22)
  "IsHelperInvocationEXT\0" // IR(41636, 22)
  "TensorViewSetClipNV\0" // IR(41658, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(41678, 27)
  "CooperativeMatrixTransposeNV\0" // IR(41705, 29)
  "ConvertUToImageNV\0" // IR(41734, 18)
  "ConvertUToSamplerNV\0" // IR(41752, 20)
  "ConvertImageToUNV\0" // IR(41772, 18)
  "ConvertSamplerToUNV\0" // IR(41790, 20)
  "ConvertUToSampledImageNV\0" // IR(41810, 25)
  "ConvertSampledImageToUNV\0" // IR(41835, 25)
  "SamplerImageAddressingModeNV\0" // IR(41860, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(41889, 52)
  "RawAccessChainNV\0" // IR(41941, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(41958, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(41998, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(42036, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(42074, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(42108, 37)
  "HitObjectGetSpherePositionNV\0" // IR(42145, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(42174, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(42201, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(42228, 23)
  "HitObjectIsSphereHitNV\0" // IR(42251, 23)
  "HitObjectIsLSSHitNV\0" // IR(42274, 20)
  "RayQueryIsSphereHitNV\0" // IR(42294, 22)
  "RayQueryIsLSSHitNV\0" // IR(42316, 19)
  "SubgroupShuffleDownINTEL\0" // IR(42335, 25)
  "SubgroupShuffleUpINTEL\0" // IR(42360, 23)
  "SubgroupShuffleXorINTEL\0" // IR(42383, 24)
  "SubgroupBlockReadINTEL\0" // IR(42407, 23)
  "SubgroupBlockWriteINTEL\0" // IR(42430, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(42454, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(42482, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(42511, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(42544, 34)
  "UCountLeadingZerosINTEL\0" // IR(42578, 24)
  "UCountTrailingZerosINTEL\0" // IR(42602, 25)
  "AbsISubINTEL\0" // IR(42627, 13)
  "AbsUSubINTEL\0" // IR(42640, 13)
  "IAddSatINTEL\0" // IR(42653, 13)
  "UAddSatINTEL\0" // IR(42666, 13)
  "IAverageINTEL\0" // IR(42679, 14)
  "UAverageINTEL\0" // IR(42693, 14)
  "IAverageRoundedINTEL\0" // IR(42707, 21)
  "UAverageRoundedINTEL\0" // IR(42728, 21)
  "ISubSatINTEL\0" // IR(42749, 13)
  "USubSatINTEL\0" // IR(42762, 13)
  "IMul32x16INTEL\0" // IR(42775, 15)
  "UMul32x16INTEL\0" // IR(42790, 15)
  "ConstantFunctionPointerINTEL\0" // IR(42805, 29)
  "FunctionPointerCallINTEL\0" // IR(42834, 25)
  "AsmTargetINTEL\0" // IR(42859, 15)
  "AsmCallINTEL\0" // IR(42874, 13)
  "AtomicFMinEXT\0" // IR(42887, 14)
  "AtomicFMaxEXT\0" // IR(42901, 14)
  "AssumeTrueKHR\0" // IR(42915, 14)
  "ExpectKHR\0" // IR(42929, 10)
  "DecorateString\0" // IR(42939, 15)
  "DecorateStringGOOGLE\0" // IR(42954, 21)
  "MemberDecorateString\0" // IR(42975, 21)
  "MemberDecorateStringGOOGLE\0" // IR(42996, 27)
  "VmeImageINTEL\0" // IR(43023, 14)
  "TypeVmeImageINTEL\0" // IR(43037, 18)
  "TypeAvcImePayloadINTEL\0" // IR(43055, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(43078, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(43101, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(43124, 23)
  "TypeAvcMceResultINTEL\0" // IR(43147, 22)
  "TypeAvcImeResultINTEL\0" // IR(43169, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(43191, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(43237, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(43281, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(43320, 37)
  "TypeAvcRefResultINTEL\0" // IR(43357, 22)
  "TypeAvcSicResultINTEL\0" // IR(43379, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(43401, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(43461, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(43514, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(43561, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(43601, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(43652, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(43696, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(43747, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(43803, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(43853, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(43905, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(43954, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(44001, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(44051, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(44102, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(44158, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(44191, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(44243, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44304, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(44365, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(44404, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(44442, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(44481, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(44519, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(44558, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(44596, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(44632, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(44671, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(44714, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(44752, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(44790, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(44828, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(44873, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44913, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(44975, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(45005, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(45043, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(45079, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(45112, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(45147, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(45186, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(45229, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(45276, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(45330, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(45364, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(45411, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(45456, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(45511, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(45564, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(45620, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(45674, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(45732, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(45788, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(45826, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(45872, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(45916, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(45965, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(46012, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(46082, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(46150, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(46219, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(46287, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(46353, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(46420, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(46456, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(46504, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(46563, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(46621, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(46677, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(46707, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(46737, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(46776, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(46822, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(46865, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(46912, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(46957, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(47003, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(47059, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(47097, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(47127, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(47159, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(47195, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(47237, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(47276, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(47315, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(47359, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(47407, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(47457, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(47500, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(47548, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(47591, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(47622, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(47669, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(47714, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(47760, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(47816, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(47854, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(47889, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(47933, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(47979, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(48020, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(48056, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(48106, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(48154, 35)
  "SaveMemoryINTEL\0" // IR(48189, 16)
  "RestoreMemoryINTEL\0" // IR(48205, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(48224, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(48253, 28)
  "ArbitraryFloatCastALTERA\0" // IR(48281, 25)
  "ArbitraryFloatCastINTEL\0" // IR(48306, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(48330, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(48362, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(48393, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(48423, 29)
  "ArbitraryFloatAddALTERA\0" // IR(48452, 24)
  "ArbitraryFloatAddINTEL\0" // IR(48476, 23)
  "ArbitraryFloatSubALTERA\0" // IR(48499, 24)
  "ArbitraryFloatSubINTEL\0" // IR(48523, 23)
  "ArbitraryFloatMulALTERA\0" // IR(48546, 24)
  "ArbitraryFloatMulINTEL\0" // IR(48570, 23)
  "ArbitraryFloatDivALTERA\0" // IR(48593, 24)
  "ArbitraryFloatDivINTEL\0" // IR(48617, 23)
  "ArbitraryFloatGTALTERA\0" // IR(48640, 23)
  "ArbitraryFloatGTINTEL\0" // IR(48663, 22)
  "ArbitraryFloatGEALTERA\0" // IR(48685, 23)
  "ArbitraryFloatGEINTEL\0" // IR(48708, 22)
  "ArbitraryFloatLTALTERA\0" // IR(48730, 23)
  "ArbitraryFloatLTINTEL\0" // IR(48753, 22)
  "ArbitraryFloatLEALTERA\0" // IR(48775, 23)
  "ArbitraryFloatLEINTEL\0" // IR(48798, 22)
  "ArbitraryFloatEQALTERA\0" // IR(48820, 23)
  "ArbitraryFloatEQINTEL\0" // IR(48843, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(48865, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(48891, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(48916, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(48942, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(48967, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(48992, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(49016, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(49042, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(49067, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(49092, 24)
  "ArbitraryFloatLogINTEL\0" // IR(49116, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(49139, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(49163, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(49188, 25)
  "ArbitraryFloatExpINTEL\0" // IR(49213, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(49236, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(49260, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(49285, 25)
  "ArbitraryFloatSinINTEL\0" // IR(49310, 23)
  "ArbitraryFloatCosINTEL\0" // IR(49333, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(49356, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(49382, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(49407, 25)
  "ArbitraryFloatASinINTEL\0" // IR(49432, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(49456, 26)
  "ArbitraryFloatACosINTEL\0" // IR(49482, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(49506, 26)
  "ArbitraryFloatATanINTEL\0" // IR(49532, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(49556, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(49582, 25)
  "ArbitraryFloatPowINTEL\0" // IR(49607, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(49630, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(49654, 24)
  "LoopControlINTEL\0" // IR(49678, 17)
  "AliasDomainDeclINTEL\0" // IR(49695, 21)
  "AliasScopeDeclINTEL\0" // IR(49716, 20)
  "AliasScopeListDeclINTEL\0" // IR(49736, 24)
  "FixedSqrtALTERA\0" // IR(49760, 16)
  "FixedSqrtINTEL\0" // IR(49776, 15)
  "FixedRecipALTERA\0" // IR(49791, 17)
  "FixedRecipINTEL\0" // IR(49808, 16)
  "FixedRsqrtALTERA\0" // IR(49824, 17)
  "FixedRsqrtINTEL\0" // IR(49841, 16)
  "FixedSinALTERA\0" // IR(49857, 15)
  "FixedSinINTEL\0" // IR(49872, 14)
  "FixedCosALTERA\0" // IR(49886, 15)
  "FixedCosINTEL\0" // IR(49901, 14)
  "FixedSinCosALTERA\0" // IR(49915, 18)
  "FixedSinCosINTEL\0" // IR(49933, 17)
  "FixedSinPiALTERA\0" // IR(49950, 17)
  "FixedSinPiINTEL\0" // IR(49967, 16)
  "FixedCosPiALTERA\0" // IR(49983, 17)
  "FixedCosPiINTEL\0" // IR(50000, 16)
  "FixedSinCosPiALTERA\0" // IR(50016, 20)
  "FixedSinCosPiINTEL\0" // IR(50036, 19)
  "FixedLogALTERA\0" // IR(50055, 15)
  "FixedLogINTEL\0" // IR(50070, 14)
  "FixedExpALTERA\0" // IR(50084, 15)
  "FixedExpINTEL\0" // IR(50099, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(50113, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(50143, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(50172, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(50202, 29)
  "ReadPipeBlockingALTERA\0" // IR(50231, 23)
  "ReadPipeBlockingINTEL\0" // IR(50254, 22)
  "WritePipeBlockingALTERA\0" // IR(50276, 24)
  "WritePipeBlockingINTEL\0" // IR(50300, 23)
  "RayQueryGetRayTMinKHR\0" // IR(50323, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(50345, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(50368, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(50396, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(50442, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(50479, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(50544, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(50584, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(50625, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(50664, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(50700, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(50746, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(50791, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(50833, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(50865, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(50894, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(50934, 40)
  "AtomicFAddEXT\0" // IR(50974, 14)
  "TypeBufferSurfaceINTEL\0" // IR(50988, 23)
  "TypeStructContinuedINTEL\0" // IR(51011, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(51036, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(51068, 36)
  "CompositeConstructContinuedINTEL\0" // IR(51104, 33)
  "ConvertFToBF16INTEL\0" // IR(51137, 20)
  "ConvertBF16ToFINTEL\0" // IR(51157, 20)
  "ControlBarrierArriveEXT\0" // IR(51177, 24)
  "ControlBarrierArriveINTEL\0" // IR(51201, 26)
  "ControlBarrierWaitEXT\0" // IR(51227, 22)
  "ControlBarrierWaitINTEL\0" // IR(51249, 24)
  "TaskSequenceCreateALTERA\0" // IR(51273, 25)
  "TaskSequenceCreateINTEL\0" // IR(51298, 24)
  "TaskSequenceAsyncALTERA\0" // IR(51322, 24)
  "TaskSequenceAsyncINTEL\0" // IR(51346, 23)
  "TaskSequenceGetALTERA\0" // IR(51369, 22)
  "TaskSequenceGetINTEL\0" // IR(51391, 21)
  "TaskSequenceReleaseALTERA\0" // IR(51412, 26)
  "TaskSequenceReleaseINTEL\0" // IR(51438, 25)
  "TypeTaskSequenceALTERA\0" // IR(51463, 23)
  "TypeTaskSequenceINTEL\0" // IR(51486, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(51508, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(51535, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(51560, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(51594, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(51628, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(51657, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(51683, 62)
  "BitwiseFunctionINTEL\0" // IR(51745, 21)
  "ConditionalExtensionINTEL\0" // IR(51766, 26)
  "ConditionalEntryPointINTEL\0" // IR(51792, 27)
  "ConditionalCapabilityINTEL\0" // IR(51819, 27)
  "SpecConstantTargetINTEL\0" // IR(51846, 24)
  "SpecConstantArchitectureINTEL\0" // IR(51870, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(51900, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(51937, 30)
  "ConditionalCopyObjectINTEL\0" // IR(51967, 27)
  "PredicatedLoadINTEL\0" // IR(51994, 20)
  "PredicatedStoreINTEL\0" // IR(52014, 21)
  "GroupIMulKHR\0" // IR(52035, 13)
  "GroupFMulKHR\0" // IR(52048, 13)
  "GroupBitwiseAndKHR\0" // IR(52061, 19)
  "GroupBitwiseOrKHR\0" // IR(52080, 18)
  "GroupBitwiseXorKHR\0" // IR(52098, 19)
  "GroupLogicalAndKHR\0" // IR(52117, 19)
  "GroupLogicalOrKHR\0" // IR(52136, 18)
  "GroupLogicalXorKHR\0" // IR(52154, 19)
  "RoundFToTF32INTEL\0" // IR(52173, 18)
  "MaskedGatherINTEL\0" // IR(52191, 18)
  "MaskedScatterINTEL\0" // IR(52209, 19)
  "ConvertHandleToImageINTEL\0" // IR(52228, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(52254, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(52282, 33)
  "FDot2MixAcc32VALVE\0" // IR(52315, 19)
  "FDot2MixAcc16VALVE\0" // IR(52334, 19)
  "FDot4MixAcc32VALVE\0" // IR(52353, 19)
  "CALL\0" // IR(52372, 5)
  "MIN_SAD\0" // IR(52377, 8)
  "MIN_SAD_COST\0" // IR(52385, 13)
  "RAW_SAD\0" // IR(52398, 8)
  "DebugInfoNone\0" // IR(52406, 14)
  "DebugCompilationUnit\0" // IR(52420, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(52441, 52)
  "DebugTypeBasic\0" // IR(52493, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(52508, 34)
  "DebugTypePointer\0" // IR(52542, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(52559, 38)
  "DebugTypeQualifier\0" // IR(52597, 19)
  "DebugTypeArray\0" // IR(52616, 15)
  "DebugTypeVector\0" // IR(52631, 16)
  "DebugTypedef\0" // IR(52647, 13)
  "DebugTypeFunction\0" // IR(52660, 18)
  "DebugTypeEnum\0" // IR(52678, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(52692, 38)
  "DebugTypeComposite\0" // IR(52730, 19)
  "DebugTypeMember\0" // IR(52749, 16)
  "DebugTypeInheritance\0" // IR(52765, 21)
  "DebugTypePtrToMember\0" // IR(52786, 21)
  "DebugTypeTemplate\0" // IR(52807, 18)
  "DebugTypeTemplateParameter\0" // IR(52825, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(52852, 35)
  "DebugTypeTemplateParameterPack\0" // IR(52887, 31)
  "DebugGlobalVariable\0" // IR(52918, 20)
  "DebugFunctionDeclaration\0" // IR(52938, 25)
  "DebugFunction\0" // IR(52963, 14)
  "DebugLexicalBlock\0" // IR(52977, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(52995, 31)
  "DebugScope\0" // IR(53026, 11)
  "DebugNoScope\0" // IR(53037, 13)
  "DebugInlinedAt\0" // IR(53050, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(53065, 42)
  "DebugLocalVariable\0" // IR(53107, 19)
  "DebugInlinedVariable\0" // IR(53126, 21)
  "DebugDeclare\0" // IR(53147, 13)
  "DebugValue\0" // IR(53160, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(53171, 33)
  "DebugOperation\0" // IR(53204, 15)
  "DebugExpression\0" // IR(53219, 16)
  "DebugMacroDef\0" // IR(53235, 14)
  "DebugMacroUndef\0" // IR(53249, 16)
  "Round\0" // IR(53265, 6)
  "RoundEven\0" // IR(53271, 10)
  "Trunc\0" // IR(53281, 6)
  "FAbs\0" // IR(53287, 5)
  "SAbs\0" // IR(53292, 5)
  "FSign\0" // IR(53297, 6)
  "SSign\0" // IR(53303, 6)
  "Floor\0" // IR(53309, 6)
  "Ceil\0" // IR(53315, 5)
  "Fract\0" // IR(53320, 6)
  "Radians\0" // IR(53326, 8)
  "Degrees\0" // IR(53334, 8)
  "Sin\0" // IR(53342, 4)
  "Cos\0" // IR(53346, 4)
  "Tan\0" // IR(53350, 4)
  "Asin\0" // IR(53354, 5)
  "Acos\0" // IR(53359, 5)
  "Atan\0" // IR(53364, 5)
  "Sinh\0" // IR(53369, 5)
  "Cosh\0" // IR(53374, 5)
  "Tanh\0" // IR(53379, 5)
  "Asinh\0" // IR(53384, 6)
  "Acosh\0" // IR(53390, 6)
  "Atanh\0" // IR(53396, 6)
  "Atan2\0" // IR(53402, 6)
  "Pow\0" // IR(53408, 4)
  "Exp\0" // IR(53412, 4)
  "Log\0" // IR(53416, 4)
  "Exp2\0" // IR(53420, 5)
  "Log2\0" // IR(53425, 5)
  "Sqrt\0" // IR(53430, 5)
  "InverseSqrt\0" // IR(53435, 12)
  "Determinant\0" // IR(53447, 12)
  "MatrixInverse\0" // IR(53459, 14)
  "Modf\0" // IR(53473, 5)
  "ModfStruct\0" // IR(53478, 11)
  "FMin\0" // IR(53489, 5)
  "UMin\0" // IR(53494, 5)
  "SMin\0" // IR(53499, 5)
  "FMax\0" // IR(53504, 5)
  "UMax\0" // IR(53509, 5)
  "SMax\0" // IR(53514, 5)
  "FClamp\0" // IR(53519, 7)
  "UClamp\0" // IR(53526, 7)
  "SClamp\0" // IR(53533, 7)
  "FMix\0" // IR(53540, 5)
  "IMix\0" // IR(53545, 5)
  "Step\0" // IR(53550, 5)
  "SmoothStep\0" // IR(53555, 11)
  "Fma\0" // IR(53566, 4)
  "Frexp\0" // IR(53570, 6)
  "FrexpStruct\0" // IR(53576, 12)
  "Ldexp\0" // IR(53588, 6)
  "PackSnorm4x8\0" // IR(53594, 13)
  "PackUnorm4x8\0" // IR(53607, 13)
  "PackSnorm2x16\0" // IR(53620, 14)
  "PackUnorm2x16\0" // IR(53634, 14)
  "PackHalf2x16\0" // IR(53648, 13)
  "PackDouble2x32\0" // IR(53661, 15)
  "UnpackSnorm2x16\0" // IR(53676, 16)
  "UnpackUnorm2x16\0" // IR(53692, 16)
  "UnpackHalf2x16\0" // IR(53708, 15)
  "UnpackSnorm4x8\0" // IR(53723, 15)
  "UnpackUnorm4x8\0" // IR(53738, 15)
  "UnpackDouble2x32\0" // IR(53753, 17)
  "Length\0" // IR(53770, 7)
  "Distance\0" // IR(53777, 9)
  "Cross\0" // IR(53786, 6)
  "Normalize\0" // IR(53792, 10)
  "FaceForward\0" // IR(53802, 12)
  "Reflect\0" // IR(53814, 8)
  "Refract\0" // IR(53822, 8)
  "FindILsb\0" // IR(53830, 9)
  "FindSMsb\0" // IR(53839, 9)
  "FindUMsb\0" // IR(53848, 9)
  "InterpolateAtCentroid\0" // IR(53857, 22)
  "InterpolateAtSample\0" // IR(53879, 20)
  "InterpolateAtOffset\0" // IR(53899, 20)
  "NMin\0" // IR(53919, 5)
  "NMax\0" // IR(53924, 5)
  "NClamp\0" // IR(53929, 7)
  "ArgumentInfo\0" // IR(53936, 13)
  "ArgumentStorageBuffer\0" // IR(53949, 22)
  "ArgumentUniform\0" // IR(53971, 16)
  "ArgumentPodStorageBuffer\0" // IR(53987, 25)
  "ArgumentPodUniform\0" // IR(54012, 19)
  "ArgumentPodPushConstant\0" // IR(54031, 24)
  "ArgumentSampledImage\0" // IR(54055, 21)
  "ArgumentStorageImage\0" // IR(54076, 21)
  "ArgumentSampler\0" // IR(54097, 16)
  "ArgumentWorkgroup\0" // IR(54113, 18)
  "SpecConstantWorkgroupSize\0" // IR(54131, 26)
  "SpecConstantGlobalOffset\0" // IR(54157, 25)
  "SpecConstantWorkDim\0" // IR(54182, 20)
  "PushConstantGlobalOffset\0" // IR(54202, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(54227, 30)
  "PushConstantGlobalSize\0" // IR(54257, 23)
  "PushConstantRegionOffset\0" // IR(54280, 25)
  "PushConstantNumWorkgroups\0" // IR(54305, 26)
  "PushConstantRegionGroupOffset\0" // IR(54331, 30)
  "ConstantDataStorageBuffer\0" // IR(54361, 26)
  "ConstantDataUniform\0" // IR(54387, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(54407, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(54437, 28)
  "ArgumentPointerPushConstant\0" // IR(54465, 28)
  "ArgumentPointerUniform\0" // IR(54493, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(54516, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(54551, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(54589, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(54631, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(54676, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(54713, 40)
  "ArgumentStorageTexelBuffer\0" // IR(54753, 27)
  "ArgumentUniformTexelBuffer\0" // IR(54780, 27)
  "ConstantDataPointerPushConstant\0" // IR(54807, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(54839, 40)
  "PrintfInfo\0" // IR(54879, 11)
  "PrintfBufferStorageBuffer\0" // IR(54890, 26)
  "PrintfBufferPointerPushConstant\0" // IR(54916, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(54948, 34)
  "WorkgroupVariableSize\0" // IR(54982, 22)
  "DebugPrintf\0" // IR(55004, 12)
  "DebugGraph\0" // IR(55016, 11)
  "DebugTensor\0" // IR(55027, 12)
  "DebugImportedEntity\0" // IR(55039, 20)
  "DebugSource\0" // IR(55059, 12)
  "DebugFunctionDefinition\0" // IR(55071, 24)
  "DebugSourceContinued\0" // IR(55095, 21)
  "DebugLine\0" // IR(55116, 10)
  "DebugNoLine\0" // IR(55126, 12)
  "DebugBuildIdentifier\0" // IR(55138, 21)
  "DebugStoragePath\0" // IR(55159, 17)
  "DebugEntryPoint\0" // IR(55176, 16)
  "DebugTypeMatrix\0" // IR(55192, 16)
  "DebugTypeVectorIdEXT\0" // IR(55208, 21)
  "DebugTypeCooperativeMatrixKHR\0" // IR(55229, 30)
  "Configuration\0" // IR(55259, 14)
  "StartCounter\0" // IR(55273, 13)
  "StopCounter\0" // IR(55286, 12)
  "PushConstants\0" // IR(55298, 14)
  "SpecializationMapEntry\0" // IR(55312, 23)
  "DescriptorSetBuffer\0" // IR(55335, 20)
  "DescriptorSetImage\0" // IR(55355, 19)
  "DescriptorSetSampler\0" // IR(55374, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(55395, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(55458, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(55503, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(55552, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(55601, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(55645, 50)
  "DebugModuleINTEL\0" // IR(55695, 17)
  "acos\0" // IR(55712, 5)
  "acosh\0" // IR(55717, 6)
  "acospi\0" // IR(55723, 7)
  "asin\0" // IR(55730, 5)
  "asinh\0" // IR(55735, 6)
  "asinpi\0" // IR(55741, 7)
  "atan\0" // IR(55748, 5)
  "atan2\0" // IR(55753, 6)
  "atanh\0" // IR(55759, 6)
  "atanpi\0" // IR(55765, 7)
  "atan2pi\0" // IR(55772, 8)
  "cbrt\0" // IR(55780, 5)
  "ceil\0" // IR(55785, 5)
  "copysign\0" // IR(55790, 9)
  "cos\0" // IR(55799, 4)
  "cosh\0" // IR(55803, 5)
  "cospi\0" // IR(55808, 6)
  "erfc\0" // IR(55814, 5)
  "erf\0" // IR(55819, 4)
  "exp\0" // IR(55823, 4)
  "exp2\0" // IR(55827, 5)
  "exp10\0" // IR(55832, 6)
  "expm1\0" // IR(55838, 6)
  "fabs\0" // IR(55844, 5)
  "fdim\0" // IR(55849, 5)
  "floor\0" // IR(55854, 6)
  "fma\0" // IR(55860, 4)
  "fmax\0" // IR(55864, 5)
  "fmin\0" // IR(55869, 5)
  "fmod\0" // IR(55874, 5)
  "fract\0" // IR(55879, 6)
  "frexp\0" // IR(55885, 6)
  "hypot\0" // IR(55891, 6)
  "ilogb\0" // IR(55897, 6)
  "ldexp\0" // IR(55903, 6)
  "lgamma\0" // IR(55909, 7)
  "lgamma_r\0" // IR(55916, 9)
  "log\0" // IR(55925, 4)
  "log2\0" // IR(55929, 5)
  "log10\0" // IR(55934, 6)
  "log1p\0" // IR(55940, 6)
  "logb\0" // IR(55946, 5)
  "mad\0" // IR(55951, 4)
  "maxmag\0" // IR(55955, 7)
  "minmag\0" // IR(55962, 7)
  "modf\0" // IR(55969, 5)
  "nan\0" // IR(55974, 4)
  "nextafter\0" // IR(55978, 10)
  "pow\0" // IR(55988, 4)
  "pown\0" // IR(55992, 5)
  "powr\0" // IR(55997, 5)
  "remainder\0" // IR(56002, 10)
  "remquo\0" // IR(56012, 7)
  "rint\0" // IR(56019, 5)
  "rootn\0" // IR(56024, 6)
  "round\0" // IR(56030, 6)
  "rsqrt\0" // IR(56036, 6)
  "sin\0" // IR(56042, 4)
  "sincos\0" // IR(56046, 7)
  "sinh\0" // IR(56053, 5)
  "sinpi\0" // IR(56058, 6)
  "sqrt\0" // IR(56064, 5)
  "tan\0" // IR(56069, 4)
  "tanh\0" // IR(56073, 5)
  "tanpi\0" // IR(56078, 6)
  "tgamma\0" // IR(56084, 7)
  "trunc\0" // IR(56091, 6)
  "half_cos\0" // IR(56097, 9)
  "half_divide\0" // IR(56106, 12)
  "half_exp\0" // IR(56118, 9)
  "half_exp2\0" // IR(56127, 10)
  "half_exp10\0" // IR(56137, 11)
  "half_log\0" // IR(56148, 9)
  "half_log2\0" // IR(56157, 10)
  "half_log10\0" // IR(56167, 11)
  "half_powr\0" // IR(56178, 10)
  "half_recip\0" // IR(56188, 11)
  "half_rsqrt\0" // IR(56199, 11)
  "half_sin\0" // IR(56210, 9)
  "half_sqrt\0" // IR(56219, 10)
  "half_tan\0" // IR(56229, 9)
  "native_cos\0" // IR(56238, 11)
  "native_divide\0" // IR(56249, 14)
  "native_exp\0" // IR(56263, 11)
  "native_exp2\0" // IR(56274, 12)
  "native_exp10\0" // IR(56286, 13)
  "native_log\0" // IR(56299, 11)
  "native_log2\0" // IR(56310, 12)
  "native_log10\0" // IR(56322, 13)
  "native_powr\0" // IR(56335, 12)
  "native_recip\0" // IR(56347, 13)
  "native_rsqrt\0" // IR(56360, 13)
  "native_sin\0" // IR(56373, 11)
  "native_sqrt\0" // IR(56384, 12)
  "native_tan\0" // IR(56396, 11)
  "fclamp\0" // IR(56407, 7)
  "degrees\0" // IR(56414, 8)
  "fmax_common\0" // IR(56422, 12)
  "fmin_common\0" // IR(56434, 12)
  "mix\0" // IR(56446, 4)
  "radians\0" // IR(56450, 8)
  "step\0" // IR(56458, 5)
  "smoothstep\0" // IR(56463, 11)
  "sign\0" // IR(56474, 5)
  "cross\0" // IR(56479, 6)
  "distance\0" // IR(56485, 9)
  "length\0" // IR(56494, 7)
  "normalize\0" // IR(56501, 10)
  "fast_distance\0" // IR(56511, 14)
  "fast_length\0" // IR(56525, 12)
  "fast_normalize\0" // IR(56537, 15)
  "s_abs\0" // IR(56552, 6)
  "s_abs_diff\0" // IR(56558, 11)
  "s_add_sat\0" // IR(56569, 10)
  "u_add_sat\0" // IR(56579, 10)
  "s_hadd\0" // IR(56589, 7)
  "u_hadd\0" // IR(56596, 7)
  "s_rhadd\0" // IR(56603, 8)
  "u_rhadd\0" // IR(56611, 8)
  "s_clamp\0" // IR(56619, 8)
  "u_clamp\0" // IR(56627, 8)
  "clz\0" // IR(56635, 4)
  "ctz\0" // IR(56639, 4)
  "s_mad_hi\0" // IR(56643, 9)
  "u_mad_sat\0" // IR(56652, 10)
  "s_mad_sat\0" // IR(56662, 10)
  "s_max\0" // IR(56672, 6)
  "u_max\0" // IR(56678, 6)
  "s_min\0" // IR(56684, 6)
  "u_min\0" // IR(56690, 6)
  "s_mul_hi\0" // IR(56696, 9)
  "rotate\0" // IR(56705, 7)
  "s_sub_sat\0" // IR(56712, 10)
  "u_sub_sat\0" // IR(56722, 10)
  "u_upsample\0" // IR(56732, 11)
  "s_upsample\0" // IR(56743, 11)
  "popcount\0" // IR(56754, 9)
  "s_mad24\0" // IR(56763, 8)
  "u_mad24\0" // IR(56771, 8)
  "s_mul24\0" // IR(56779, 8)
  "u_mul24\0" // IR(56787, 8)
  "vloadn\0" // IR(56795, 7)
  "vstoren\0" // IR(56802, 8)
  "vload_half\0" // IR(56810, 11)
  "vload_halfn\0" // IR(56821, 12)
  "vstore_half\0" // IR(56833, 12)
  "vstore_half_r\0" // IR(56845, 14)
  "vstore_halfn\0" // IR(56859, 13)
  "vstore_halfn_r\0" // IR(56872, 15)
  "vloada_halfn\0" // IR(56887, 13)
  "vstorea_halfn\0" // IR(56900, 14)
  "vstorea_halfn_r\0" // IR(56914, 16)
  "shuffle\0" // IR(56930, 8)
  "shuffle2\0" // IR(56938, 9)
  "printf\0" // IR(56947, 7)
  "prefetch\0" // IR(56954, 9)
  "bitselect\0" // IR(56963, 10)
  "select\0" // IR(56973, 7)
  "u_abs\0" // IR(56980, 6)
  "u_abs_diff\0" // IR(56986, 11)
  "u_mul_hi\0" // IR(56997, 9)
  "u_mad_hi\0" // IR(57006, 9)
  "CubeFaceIndexAMD\0" // IR(57015, 17)
  "CubeFaceCoordAMD\0" // IR(57032, 17)
  "TimeAMD\0" // IR(57049, 8)
  "SwizzleInvocationsAMD\0" // IR(57057, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(57079, 28)
  "WriteInvocationAMD\0" // IR(57107, 19)
  "MbcntAMD\0" // IR(57126, 9)
  "InterpolateAtVertexAMD\0" // IR(57135, 23)
  "FMin3AMD\0" // IR(57158, 9)
  "UMin3AMD\0" // IR(57167, 9)
  "SMin3AMD\0" // IR(57176, 9)
  "FMax3AMD\0" // IR(57185, 9)
  "UMax3AMD\0" // IR(57194, 9)
  "SMax3AMD\0" // IR(57203, 9)
  "FMid3AMD\0" // IR(57212, 9)
  "UMid3AMD\0" // IR(57221, 9)
  "SMid3AMD\0" // IR(57230, 9)
  "ARGMAX\0" // IR(57239, 7)
  "AVG_POOL2D\0" // IR(57246, 11)
  "CONV2D\0" // IR(57257, 7)
  "CONV3D\0" // IR(57264, 7)
  "DEPTHWISE_CONV2D\0" // IR(57271, 17)
  "FFT2D\0" // IR(57288, 6)
  "MATMUL\0" // IR(57294, 7)
  "MAX_POOL2D\0" // IR(57301, 11)
  "RFFT2D\0" // IR(57312, 7)
  "TRANSPOSE_CONV2D\0" // IR(57319, 17)
  "CLAMP\0" // IR(57336, 6)
  "ERF\0" // IR(57342, 4)
  "SIGMOID\0" // IR(57346, 8)
  "TANH\0" // IR(57354, 5)
  "ADD\0" // IR(57359, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(57363, 23)
  "BITWISE_AND\0" // IR(57386, 12)
  "BITWISE_OR\0" // IR(57398, 11)
  "BITWISE_XOR\0" // IR(57409, 12)
  "INTDIV\0" // IR(57421, 7)
  "LOGICAL_AND\0" // IR(57428, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(57440, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(57459, 20)
  "LOGICAL_OR\0" // IR(57479, 11)
  "LOGICAL_XOR\0" // IR(57490, 12)
  "MAXIMUM\0" // IR(57502, 8)
  "MINIMUM\0" // IR(57510, 8)
  "MUL\0" // IR(57518, 4)
  "POW\0" // IR(57522, 4)
  "SUB\0" // IR(57526, 4)
  "TABLE\0" // IR(57530, 6)
  "ABS\0" // IR(57536, 4)
  "BITWISE_NOT\0" // IR(57540, 12)
  "CEIL\0" // IR(57552, 5)
  "CLZ\0" // IR(57557, 4)
  "COS\0" // IR(57561, 4)
  "EXP\0" // IR(57565, 4)
  "FLOOR\0" // IR(57569, 6)
  "LOG\0" // IR(57575, 4)
  "LOGICAL_NOT\0" // IR(57579, 12)
  "NEGATE\0" // IR(57591, 7)
  "RECIPROCAL\0" // IR(57598, 11)
  "RSQRT\0" // IR(57609, 6)
  "SIN\0" // IR(57615, 4)
  "SELECT\0" // IR(57619, 7)
  "EQUAL\0" // IR(57626, 6)
  "GREATER\0" // IR(57632, 8)
  "GREATER_EQUAL\0" // IR(57640, 14)
  "REDUCE_ALL\0" // IR(57654, 11)
  "REDUCE_ANY\0" // IR(57665, 11)
  "REDUCE_MAX\0" // IR(57676, 11)
  "REDUCE_MIN\0" // IR(57687, 11)
  "REDUCE_PRODUCT\0" // IR(57698, 15)
  "REDUCE_SUM\0" // IR(57713, 11)
  "CONCAT\0" // IR(57724, 7)
  "PAD\0" // IR(57731, 4)
  "RESHAPE\0" // IR(57735, 8)
  "REVERSE\0" // IR(57743, 8)
  "SLICE\0" // IR(57751, 6)
  "TILE\0" // IR(57757, 5)
  "TRANSPOSE\0" // IR(57762, 10)
  "GATHER\0" // IR(57772, 7)
  "SCATTER\0" // IR(57779, 8)
  "RESIZE\0" // IR(57787, 7)
  "CAST\0" // IR(57794, 5)
  "RESCALE\0" // IR(57799, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(6066, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6977, 18), // 1 SubgroupEqMaskKHR
  IR(7032, 18), // 2 SubgroupGeMaskKHR
  IR(7065, 18), // 3 SubgroupGtMaskKHR
  IR(7098, 18), // 4 SubgroupLeMaskKHR
  IR(7131, 18), // 5 SubgroupLtMaskKHR
  IR(8038, 12), // 6 BaryCoordNV
  IR(8093, 19), // 7 BaryCoordNoPerspNV
  IR(8124, 15), // 8 FragmentSizeNV
  IR(8181, 22), // 9 InvocationsPerPixelNV
  IR(8309, 11), // 10 LaunchIdNV
  IR(8334, 13), // 11 LaunchSizeNV
  IR(8365, 17), // 12 WorldRayOriginNV
  IR(8403, 20), // 13 WorldRayDirectionNV
  IR(8442, 18), // 14 ObjectRayOriginNV
  IR(8482, 21), // 15 ObjectRayDirectionNV
  IR(8514, 10), // 16 RayTminNV
  IR(8535, 10), // 17 RayTmaxNV
  IR(8568, 22), // 18 InstanceCustomIndexNV
  IR(8607, 16), // 19 ObjectToWorldNV
  IR(8640, 16), // 20 WorldToObjectNV
  IR(8674, 10), // 21 HitKindNV
  IR(8905, 19), // 22 IncomingRayFlagsNV
  IR(10794, 28), // 23 StorageUniformBufferBlock16
  IR(10857, 17), // 24 StorageUniform16
  IR(11838, 27), // 25 ShaderViewportIndexLayerNV
  IR(11882, 22), // 26 FragmentBarycentricNV
  IR(11935, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11965, 14), // 28 ShadingRateNV
  IR(12009, 29), // 29 GroupNonUniformPartitionedNV
  IR(12055, 20), // 30 ShaderNonUniformEXT
  IR(12098, 26), // 31 RuntimeDescriptorArrayEXT
  IR(12160, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(12238, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(12319, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(12398, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(12474, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(12550, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(12626, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(12704, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12788, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12875, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12938, 21), // 42 VulkanMemoryModelKHR
  IR(12988, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(13020, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(13086, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(13290, 28), // 46 DemoteToHelperInvocationEXT
  IR(13370, 29), // 47 RayTracingOpacityMicromapEXT
  IR(13675, 30), // 48 CooperativeMatrixReductionsNV
  IR(13777, 40), // 49 CooperativeMatrixPerElementOperationsNV
  IR(14530, 26), // 50 FPGAMemoryAttributesINTEL
  IR(14609, 32), // 51 ArbitraryPrecisionIntegersINTEL
  IR(14679, 37), // 52 ArbitraryPrecisionFloatingPointINTEL
  IR(14769, 22), // 53 FPGALoopControlsINTEL
  IR(14864, 24), // 54 FPGAMemoryAccessesINTEL
  IR(14916, 27), // 55 FPGAClusterAttributesINTEL
  IR(14958, 14), // 56 LoopFuseINTEL
  IR(14993, 20), // 57 FPGADSPControlINTEL
  IR(15080, 40), // 58 FPGAInvocationPipeliningAttributesINTEL
  IR(15145, 24), // 59 FPGABufferLocationINTEL
  IR(15204, 34), // 60 ArbitraryPrecisionFixedPointINTEL
  IR(15262, 23), // 61 USMStorageClassesINTEL
  IR(15315, 29), // 62 RuntimeAlignedAttributeINTEL
  IR(15358, 13), // 63 IOPipesINTEL
  IR(15391, 19), // 64 BlockingPipesINTEL
  IR(15424, 13), // 65 FPGARegINTEL
  IR(15456, 22), // 66 DotProductInputAllKHR
  IR(15500, 25), // 67 DotProductInput4x8BitKHR
  IR(15553, 31), // 68 DotProductInput4x8BitPackedKHR
  IR(15595, 14), // 69 DotProductKHR
  IR(15809, 13), // 70 OptNoneINTEL
  IR(15903, 18), // 71 SplitBarrierINTEL
  IR(15970, 29), // 72 FPGAClusterAttributesV2INTEL
  IR(16046, 18), // 73 TaskSequenceINTEL
  IR(16105, 24), // 74 FPGALatencyControlINTEL
  IR(16158, 28), // 75 FPGAArgumentInterfacesINTEL
  IR(16252, 35), // 76 GlobalVariableFPGADecorationsINTEL
  IR(19289, 15), // 77 PerPrimitiveNV
  IR(19337, 12), // 78 PerVertexNV
  IR(19360, 14), // 79 NonUniformEXT
  IR(19390, 19), // 80 RestrictPointerEXT
  IR(19424, 18), // 81 AliasedPointerEXT
  IR(19817, 24), // 82 HlslCounterBufferGOOGLE
  IR(19854, 19), // 83 HlslSemanticGOOGLE
  IR(19984, 14), // 84 RegisterINTEL
  IR(20011, 12), // 85 MemoryINTEL
  IR(20038, 14), // 86 NumbanksINTEL
  IR(20068, 15), // 87 BankwidthINTEL
  IR(20106, 22), // 88 MaxPrivateCopiesINTEL
  IR(20145, 16), // 89 SinglepumpINTEL
  IR(20178, 16), // 90 DoublepumpINTEL
  IR(20214, 19), // 91 MaxReplicatesINTEL
  IR(20254, 20), // 92 SimpleDualPortINTEL
  IR(20286, 11), // 93 MergeINTEL
  IR(20354, 14), // 94 BankBitsINTEL
  IR(20389, 20), // 95 ForcePow2DepthINTEL
  IR(20426, 16), // 96 StridesizeINTEL
  IR(20457, 14), // 97 WordsizeINTEL
  IR(20490, 18), // 98 TrueDualPortINTEL
  IR(20528, 19), // 99 BurstCoalesceINTEL
  IR(20563, 15), // 100 CacheSizeINTEL
  IR(20607, 28), // 101 DontStaticallyCoalesceINTEL
  IR(20650, 14), // 102 PrefetchINTEL
  IR(20682, 17), // 103 StallEnableINTEL
  IR(20725, 25), // 104 FuseLoopsInFunctionINTEL
  IR(20770, 19), // 105 MathOpDSPModeINTEL
  IR(20843, 24), // 106 InitiationIntervalINTEL
  IR(20888, 20), // 107 MaxConcurrencyINTEL
  IR(20929, 20), // 108 PipelineEnableINTEL
  IR(20970, 20), // 109 BufferLocationINTEL
  IR(21010, 19), // 110 IOPipeStorageINTEL
  IR(21188, 15), // 111 StallFreeINTEL
  IR(21286, 25), // 112 LatencyControlLabelINTEL
  IR(21342, 30), // 113 LatencyControlConstraintINTEL
  IR(21400, 27), // 114 ConduitKernelArgumentINTEL
  IR(21459, 31), // 115 RegisterMapKernelArgumentINTEL
  IR(21524, 33), // 116 MMHostInterfaceAddressWidthINTEL
  IR(21588, 30), // 117 MMHostInterfaceDataWidthINTEL
  IR(21647, 28), // 118 MMHostInterfaceLatencyINTEL
  IR(21744, 34), // 119 MMHostInterfaceReadWriteModeINTEL
  IR(21808, 29), // 120 MMHostInterfaceMaxBurstINTEL
  IR(21870, 32), // 121 MMHostInterfaceWaitRequestINTEL
  IR(21929, 26), // 122 StableKernelArgumentINTEL
  IR(22072, 14), // 123 InitModeINTEL
  IR(22115, 28), // 124 ImplementInRegisterMapINTEL
  IR(23511, 14), // 125 OutputLinesNV
  IR(23545, 19), // 126 OutputPrimitivesNV
  IR(23588, 23), // 127 DerivativeGroupQuadsNV
  IR(23636, 24), // 128 DerivativeGroupLinearNV
  IR(23679, 18), // 129 OutputTrianglesNV
  IR(24448, 16), // 130 RayGenerationNV
  IR(24480, 15), // 131 IntersectionNV
  IR(24505, 9), // 132 AnyHitNV
  IR(24528, 13), // 133 ClosestHitNV
  IR(24549, 7), // 134 MissNV
  IR(24568, 11), // 135 CallableNV
  IR(24755, 23), // 136 AllowContractFastINTEL
  IR(24791, 18), // 137 AllowReassocINTEL
  IR(25017, 20), // 138 RuntimeAlignedINTEL
  IR(25161, 20), // 139 PartitionedReduceNV
  IR(25209, 27), // 140 PartitionedInclusiveScanNV
  IR(25264, 27), // 141 PartitionedExclusiveScanNV
  IR(25395, 22), // 142 MakeTexelAvailableKHR
  IR(25434, 20), // 143 MakeTexelVisibleKHR
  IR(25470, 19), // 144 NonPrivateTexelKHR
  IR(25503, 17), // 145 VolatileTexelKHR
  IR(26062, 27), // 146 InitOnDeviceReprogramINTEL
  IR(26113, 23), // 147 InitOnDeviceResetINTEL
  IR(26455, 21), // 148 DependencyArrayINTEL
  IR(26495, 18), // 149 LoopCoalesceINTEL
  IR(26535, 21), // 150 MaxInterleavingINTEL
  IR(26583, 26), // 151 SpeculatedIterationsINTEL
  IR(26624, 14), // 152 NoFusionINTEL
  IR(26654, 15), // 153 LoopCountINTEL
  IR(26696, 26), // 154 MaxReinvocationDelayINTEL
  IR(27088, 24), // 155 MakePointerAvailableKHR
  IR(27131, 22), // 156 MakePointerVisibleKHR
  IR(27171, 21), // 157 NonPrivatePointerKHR
  IR(27258, 10), // 158 VulkanKHR
  IR(17018, 5), // 159 None
  IR(27441, 16), // 160 OutputMemoryKHR
  IR(27471, 17), // 161 MakeAvailableKHR
  IR(27500, 15), // 162 MakeVisibleKHR
  IR(27576, 28), // 163 PackedVectorFormat4x8BitKHR
  IR(27891, 24), // 164 SkipBuiltinPrimitivesNV
  IR(27958, 30), // 165 ForceOpacityMicromap2StateEXT
  IR(28661, 15), // 166 QueueFamilyKHR
  IR(29046, 15), // 167 CallableDataNV
  IR(29085, 23), // 168 IncomingCallableDataNV
  IR(29122, 13), // 169 RayPayloadNV
  IR(29151, 15), // 170 HitAttributeNV
  IR(29188, 21), // 171 IncomingRayPayloadNV
  IR(29231, 21), // 172 ShaderRecordBufferNV
  IR(29274, 25), // 173 PhysicalStorageBufferEXT
  IR(29400, 16), // 174 DeviceOnlyINTEL
  IR(29431, 14), // 175 HostOnlyINTEL
  IR(36686, 8), // 176 SDotKHR
  IR(36699, 8), // 177 UDotKHR
  IR(36713, 9), // 178 SUDotKHR
  IR(36733, 14), // 179 SDotAccSatKHR
  IR(36758, 14), // 180 UDotAccSatKHR
  IR(36784, 15), // 181 SUDotAccSatKHR
  IR(39101, 24), // 182 TypeCooperativeVectorNV
  IR(39298, 27), // 183 CooperativeMatrixConvertNV
  IR(39388, 27), // 184 GroupNonUniformPartitionNV
  IR(40470, 21), // 185 ReportIntersectionNV
  IR(40645, 28), // 186 TypeAccelerationStructureNV
  IR(40726, 23), // 187 RayQueryGetClusterIdNV
  IR(41187, 26), // 188 CooperativeMatrixReduceNV
  IR(41382, 32), // 189 CooperativeMatrixPerElementOpNV
  IR(42954, 21), // 190 DecorateStringGOOGLE
  IR(42996, 27), // 191 MemberDecorateStringGOOGLE
  IR(48253, 28), // 192 ArbitraryFloatSinCosPiINTEL
  IR(48306, 24), // 193 ArbitraryFloatCastINTEL
  IR(48362, 31), // 194 ArbitraryFloatCastFromIntINTEL
  IR(48423, 29), // 195 ArbitraryFloatCastToIntINTEL
  IR(48476, 23), // 196 ArbitraryFloatAddINTEL
  IR(48523, 23), // 197 ArbitraryFloatSubINTEL
  IR(48570, 23), // 198 ArbitraryFloatMulINTEL
  IR(48617, 23), // 199 ArbitraryFloatDivINTEL
  IR(48663, 22), // 200 ArbitraryFloatGTINTEL
  IR(48708, 22), // 201 ArbitraryFloatGEINTEL
  IR(48753, 22), // 202 ArbitraryFloatLTINTEL
  IR(48798, 22), // 203 ArbitraryFloatLEINTEL
  IR(48843, 22), // 204 ArbitraryFloatEQINTEL
  IR(48891, 25), // 205 ArbitraryFloatRecipINTEL
  IR(48942, 25), // 206 ArbitraryFloatRSqrtINTEL
  IR(48992, 24), // 207 ArbitraryFloatCbrtINTEL
  IR(49042, 25), // 208 ArbitraryFloatHypotINTEL
  IR(49092, 24), // 209 ArbitraryFloatSqrtINTEL
  IR(49776, 15), // 210 FixedSqrtINTEL
  IR(49808, 16), // 211 FixedRecipINTEL
  IR(49841, 16), // 212 FixedRsqrtINTEL
  IR(49872, 14), // 213 FixedSinINTEL
  IR(49901, 14), // 214 FixedCosINTEL
  IR(49933, 17), // 215 FixedSinCosINTEL
  IR(49967, 16), // 216 FixedSinPiINTEL
  IR(50000, 16), // 217 FixedCosPiINTEL
  IR(50036, 19), // 218 FixedSinCosPiINTEL
  IR(50070, 14), // 219 FixedLogINTEL
  IR(50099, 14), // 220 FixedExpINTEL
  IR(50143, 29), // 221 PtrCastToCrossWorkgroupINTEL
  IR(50202, 29), // 222 CrossWorkgroupCastToPtrINTEL
  IR(50254, 22), // 223 ReadPipeBlockingINTEL
  IR(50300, 23), // 224 WritePipeBlockingINTEL
  IR(51201, 26), // 225 ControlBarrierArriveINTEL
  IR(51249, 24), // 226 ControlBarrierWaitINTEL
  IR(51298, 24), // 227 TaskSequenceCreateINTEL
  IR(51346, 23), // 228 TaskSequenceAsyncINTEL
  IR(51391, 21), // 229 TaskSequenceGetINTEL
  IR(51438, 25), // 230 TaskSequenceReleaseINTEL
  IR(51486, 22), // 231 TypeTaskSequenceINTEL
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
  spv::Capability::DeviceBarrierINTEL, // 67
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
  spv::Capability::Linkage, // 95
  spv::Capability::MultiViewport, // 96
  spv::Capability::ShaderViewportIndexLayerEXT, // 97
  spv::Capability::ShaderViewportMaskNV, // 98
  spv::Capability::InputAttachment, // 99
  spv::Capability::ImageBuffer, // 100
  spv::Capability::ShaderNonUniform, // 101
  spv::Capability::InputAttachment, // 102
  spv::Capability::ShaderNonUniform, // 103
  spv::Capability::SampledBuffer, // 104
  spv::Capability::ShaderNonUniform, // 105
  spv::Capability::ImageBuffer, // 106
  spv::Capability::ShaderNonUniform, // 107
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 108
  spv::Capability::VectorAnyINTEL, // 109
  spv::Capability::Int8, // 110
  spv::Capability::FPGAClusterAttributesALTERA, // 111
  spv::Capability::FPGAKernelAttributesINTEL, // 112
  spv::Capability::Subgroup2DBlockIOINTEL, // 113
  spv::Capability::VariableLengthArrayINTEL, // 114
  spv::Capability::UntypedPointersKHR, // 115
  spv::Capability::SpecConditionalINTEL, // 116
  spv::Capability::Float16, // 117
  spv::Capability::Float8EXT, // 118
  spv::Capability::CooperativeMatrixLayoutsARM, // 119
  spv::Capability::Shader, // 120
  spv::Capability::Kernel, // 121
  spv::Capability::Shader, // 122
  spv::Capability::UniformDecoration, // 123
  spv::Capability::GeometryStreams, // 124
  spv::Capability::TransformFeedback, // 125
  spv::Capability::Kernel, // 126
  spv::Capability::FloatControls2, // 127
  spv::Capability::ConstantDataKHR, // 128
  spv::Capability::SampleMaskOverrideCoverageNV, // 129
  spv::Capability::GeometryShaderPassthroughNV, // 130
  spv::Capability::MeshShadingNV, // 131
  spv::Capability::MeshShadingEXT, // 132
  spv::Capability::PushConstantBanksNV, // 133
  spv::Capability::ShaderInvocationReorderNV, // 134
  spv::Capability::ShaderInvocationReorderEXT, // 135
  spv::Capability::BindlessTextureNV, // 136
  spv::Capability::CooperativeMatrixConversionsEXT, // 137
  spv::Capability::VectorComputeINTEL, // 138
  spv::Capability::IndirectReferencesINTEL, // 139
  spv::Capability::AsmINTEL, // 140
  spv::Capability::FunctionFloatControlINTEL, // 141
  spv::Capability::FPGAMemoryAttributesALTERA, // 142
  spv::Capability::FPGAMemoryAccessesALTERA, // 143
  spv::Capability::LoopFuseALTERA, // 144
  spv::Capability::FPGADSPControlALTERA, // 145
  spv::Capability::MemoryAccessAliasingINTEL, // 146
  spv::Capability::FPGAInvocationPipeliningAttributesALTERA, // 147
  spv::Capability::FPGABufferLocationALTERA, // 148
  spv::Capability::IOPipesALTERA, // 149
  spv::Capability::FPGAClusterAttributesV2ALTERA, // 150
  spv::Capability::FPMaxErrorINTEL, // 151
  spv::Capability::FPGALatencyControlALTERA, // 152
  spv::Capability::FPGAArgumentInterfacesALTERA, // 153
  spv::Capability::GlobalVariableHostAccessINTEL, // 154
  spv::Capability::GlobalVariableFPGADecorationsALTERA, // 155
  spv::Capability::CacheControlsINTEL, // 156
  spv::Capability::IntrinsicSAMSUNG, // 157
  spv::Capability::TileImageColorReadAccessEXT, // 158
  spv::Capability::Geometry, // 159
  spv::Capability::Tessellation, // 160
  spv::Capability::MeshShadingNV, // 161
  spv::Capability::MeshShadingEXT, // 162
  spv::Capability::Geometry, // 163
  spv::Capability::MeshShadingNV, // 164
  spv::Capability::MeshShadingEXT, // 165
  spv::Capability::SubgroupDispatch, // 166
  spv::Capability::TileImageDepthReadAccessEXT, // 167
  spv::Capability::TileImageStencilReadAccessEXT, // 168
  spv::Capability::SampleMaskPostDepthCoverage, // 169
  spv::Capability::DenormPreserve, // 170
  spv::Capability::DenormFlushToZero, // 171
  spv::Capability::SignedZeroInfNanPreserve, // 172
  spv::Capability::RoundingModeRTE, // 173
  spv::Capability::RoundingModeRTZ, // 174
  spv::Capability::SubgroupSizeQCOM, // 175
  spv::Capability::QuadControlKHR, // 176
  spv::Capability::PoisonFreezeKHR, // 177
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 178
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 179
  spv::Capability::FragmentShaderPixelInterlockEXT, // 180
  spv::Capability::FragmentShaderSampleInterlockEXT, // 181
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 182
  spv::Capability::Shader64BitIndexingEXT, // 183
  spv::Capability::RoundToInfinityINTEL, // 184
  spv::Capability::KernelAttributesINTEL, // 185
  spv::Capability::FloatControls2, // 186
  spv::Capability::RayTracingOpacityMicromapExecutionModeKHR, // 187
  spv::Capability::FPGAKernelAttributesv2INTEL, // 188
  spv::Capability::RegisterLimitsINTEL, // 189
  spv::Capability::Float6EXT, // 190
  spv::Capability::Float4EXT, // 191
  spv::Capability::Float8UnsignedE8M0EXT, // 192
  spv::Capability::MXInt8EXT, // 193
  spv::Capability::FloatControls2, // 194
  spv::Capability::FPFastMathModeINTEL, // 195
  spv::Capability::OptNoneEXT, // 196
  spv::Capability::RuntimeAlignedAttributeALTERA, // 197
  spv::Capability::Kernel, // 198
  spv::Capability::GroupNonUniformArithmetic, // 199
  spv::Capability::GroupNonUniformBallot, // 200
  spv::Capability::GroupNonUniformClustered, // 201
  spv::Capability::GroupNonUniformPartitionedEXT, // 202
  spv::Capability::ImageGatherExtended, // 203
  spv::Capability::MinLod, // 204
  spv::Capability::VulkanMemoryModel, // 205
  spv::Capability::WeakLinkageAMD, // 206
  spv::Capability::FPGALoopControlsALTERA, // 207
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 208
  spv::Capability::RawAccessChainsNV, // 209
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 210
  spv::Capability::RayTracingOpacityMicromapKHR, // 211
  spv::Capability::RayQueryKHR, // 212
  spv::Capability::StorageImageExtendedFormats, // 213
  spv::Capability::Int64ImageEXT, // 214
  spv::Capability::Shader, // 215
  spv::Capability::VectorComputeINTEL, // 216
  spv::Capability::GenericPointer, // 217
  spv::Capability::FunctionPointersINTEL, // 218
  spv::Capability::USMStorageClassesALTERA, // 219
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 220
  spv::Capability::CooperativeMatrixDecodeVectorNV, // 221
  spv::Capability::TensorsARM, // 222
  spv::Capability::Addresses, // 223
  spv::Capability::PhysicalStorageBufferAddresses, // 224
  spv::Capability::LiteralSampler, // 225
  spv::Capability::Addresses, // 226
  spv::Capability::UntypedPointersKHR, // 227
  spv::Capability::Addresses, // 228
  spv::Capability::VariablePointers, // 229
  spv::Capability::VariablePointersStorageBuffer, // 230
  spv::Capability::PhysicalStorageBufferAddresses, // 231
  spv::Capability::Kernel, // 232
  spv::Capability::ImageQuery, // 233
  spv::Capability::ImageQuery, // 234
  spv::Capability::Shader, // 235
  spv::Capability::BitInstructions, // 236
  spv::Capability::DerivativeControl, // 237
  spv::Capability::Groups, // 238
  spv::Capability::SparseResidency, // 239
  spv::Capability::PipeStorage, // 240
  spv::Capability::NamedBarrier, // 241
  spv::Capability::GroupNonUniformVote, // 242
  spv::Capability::GroupNonUniformBallot, // 243
  spv::Capability::GroupNonUniformShuffle, // 244
  spv::Capability::GroupNonUniformShuffleRelative, // 245
  spv::Capability::GroupNonUniformArithmetic, // 246
  spv::Capability::GroupNonUniformClustered, // 247
  spv::Capability::GroupNonUniformPartitionedEXT, // 248
  spv::Capability::GroupNonUniformQuad, // 249
  spv::Capability::Addresses, // 250
  spv::Capability::VariablePointers, // 251
  spv::Capability::VariablePointersStorageBuffer, // 252
  spv::Capability::GraphARM, // 253
  spv::Capability::BitcastExtractEXT, // 254
  spv::Capability::SubgroupBallotKHR, // 255
  spv::Capability::FMAKHR, // 256
  spv::Capability::SubgroupVoteKHR, // 257
  spv::Capability::GroupNonUniformRotateKHR, // 258
  spv::Capability::RayTracingKHR, // 259
  spv::Capability::RayQueryKHR, // 260
  spv::Capability::DotProduct, // 261
  spv::Capability::ReplicatedCompositesEXT, // 262
  spv::Capability::TextureSampleWeightedQCOM, // 263
  spv::Capability::TextureBoxFilterQCOM, // 264
  spv::Capability::TextureBlockMatchQCOM, // 265
  spv::Capability::CooperativeMatrixConversionQCOM, // 266
  spv::Capability::TextureBlockMatch2QCOM, // 267
  spv::Capability::BFloat16MulAddQCOM, // 268
  spv::Capability::ImageGatherLinearQCOM, // 269
  spv::Capability::ImageGatherExtendedModesQCOM, // 270
  spv::Capability::FragmentMaskAMD, // 271
  spv::Capability::ShaderClockKHR, // 272
  spv::Capability::AbortKHR, // 273
  spv::Capability::ShaderInvocationReorderNV, // 274
  spv::Capability::RayTracingMotionBlurNV, // 275
  spv::Capability::ImageFootprintNV, // 276
  spv::Capability::CooperativeVectorNV, // 277
  spv::Capability::LongVectorEXT, // 278
  spv::Capability::CooperativeVectorNV, // 279
  spv::Capability::CooperativeVectorTrainingNV, // 280
  spv::Capability::CooperativeMatrixConversionsEXT, // 281
  spv::Capability::CooperativeMatrixConversionsNV, // 282
  spv::Capability::DisplacementMicromapNV, // 283
  spv::Capability::ShaderInvocationReorderEXT, // 284
  spv::Capability::RayTracingMotionBlurNV, // 285
  spv::Capability::RayQueryPositionFetchKHR, // 286
  spv::Capability::RayTracingNV, // 287
  spv::Capability::RayTracingKHR, // 288
  spv::Capability::RayQueryKHR, // 289
  spv::Capability::DisplacementMicromapNV, // 290
  spv::Capability::CooperativeMatrixNV, // 291
  spv::Capability::CooperativeMatrixGetCoordinateEXT, // 292
  spv::Capability::FragmentShaderSampleInterlockEXT, // 293
  spv::Capability::FragmentShaderPixelInterlockEXT, // 294
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 295
  spv::Capability::CooperativeMatrixReductionsEXT, // 296
  spv::Capability::CooperativeMatrixPerElementOperationsEXT, // 297
  spv::Capability::TensorAddressingNV, // 298
  spv::Capability::DemoteToHelperInvocation, // 299
  spv::Capability::CooperativeMatrixConversionsNV, // 300
  spv::Capability::SubgroupShuffleINTEL, // 301
  spv::Capability::SubgroupBufferBlockIOINTEL, // 302
  spv::Capability::SubgroupImageBlockIOINTEL, // 303
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 304
  spv::Capability::IntegerFunctions2INTEL, // 305
  spv::Capability::AtomicFloat16MinMaxEXT, // 306
  spv::Capability::AtomicFloat32MinMaxEXT, // 307
  spv::Capability::AtomicFloat64MinMaxEXT, // 308
  spv::Capability::AtomicFloat16VectorNV, // 309
  spv::Capability::ExpectAssumeKHR, // 310
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 311
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 312
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 313
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 314
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 315
  spv::Capability::VariableLengthArrayINTEL, // 316
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 317
  spv::Capability::UnstructuredLoopControlsINTEL, // 318
  spv::Capability::BlockingPipesALTERA, // 319
  spv::Capability::FPGARegALTERA, // 320
  spv::Capability::AtomicFloat16AddEXT, // 321
  spv::Capability::AtomicFloat32AddEXT, // 322
  spv::Capability::AtomicFloat64AddEXT, // 323
  spv::Capability::AtomicFloat16VectorNV, // 324
  spv::Capability::LongCompositesINTEL, // 325
  spv::Capability::BFloat16ConversionINTEL, // 326
  spv::Capability::SplitBarrierEXT, // 327
  spv::Capability::ArithmeticFenceEXT, // 328
  spv::Capability::TaskSequenceALTERA, // 329
  spv::Capability::SubgroupBitcastShuffleINTEL, // 330
  spv::Capability::SubgroupBufferPrefetchINTEL, // 331
  spv::Capability::Subgroup2DBlockTransformINTEL, // 332
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 333
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 334
  spv::Capability::TernaryBitwiseFunctionINTEL, // 335
  spv::Capability::UntypedVariableLengthArrayINTEL, // 336
  spv::Capability::FunctionVariantsINTEL, // 337
  spv::Capability::PredicatedIOINTEL, // 338
  spv::Capability::GroupUniformArithmeticKHR, // 339
  spv::Capability::TensorFloat32RoundingINTEL, // 340
  spv::Capability::MaskedGatherScatterINTEL, // 341
  spv::Capability::BindlessImagesINTEL, // 342
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 343
  spv::Capability::DotProductBFloat16AccVALVE, // 344
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 345
  spv::Capability::DotProductBFloat16AccVALVE, // 346
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 347
  spv::Capability::Float64, // 348
  spv::Capability::InterpolationFunction, // 349
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
  spvtools::Extension::kSPV_QCOM_bfloat16_muladd, // 57
  spvtools::Extension::kSPV_QCOM_subgroup_size, // 58
  spvtools::Extension::kSPV_QCOM_multiple_wait_queues, // 59
  spvtools::Extension::kSPV_QCOM_image_processing3, // 60
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 61
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 62
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 63
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 64
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 65
  spvtools::Extension::kSPV_KHR_shader_clock, // 66
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 67
  spvtools::Extension::kSPV_KHR_quad_control, // 68
  spvtools::Extension::kSPV_INTEL_int4, // 69
  spvtools::Extension::kSPV_KHR_bfloat16, // 70
  spvtools::Extension::kSPV_KHR_abort, // 71
  spvtools::Extension::kSPV_EXT_descriptor_heap, // 72
  spvtools::Extension::kSPV_KHR_constant_data, // 73
  spvtools::Extension::kSPV_KHR_poison_freeze, // 74
  spvtools::Extension::kSPV_AMD_weak_linkage, // 75
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 76
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 77
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 78
  spvtools::Extension::kSPV_NV_viewport_array2, // 79
  spvtools::Extension::kSPV_NV_viewport_array2, // 80
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 81
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 82
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 83
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 84
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 85
  spvtools::Extension::kSPV_EXT_shader_subgroup_partitioned, // 86
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 87
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 88
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 89
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 90
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 91
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 92
  spvtools::Extension::kSPV_NV_displacement_micromap, // 93
  spvtools::Extension::kSPV_KHR_opacity_micromap, // 94
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 95
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 96
  spvtools::Extension::kSPV_EXT_shader_invocation_reorder, // 97
  spvtools::Extension::kSPV_NV_bindless_texture, // 98
  spvtools::Extension::kSPV_NV_cooperative_vector, // 99
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 100
  spvtools::Extension::kSPV_NV_raw_access_chains, // 101
  spvtools::Extension::kSPV_NV_push_constant_bank, // 102
  spvtools::Extension::kSPV_EXT_long_vector, // 103
  spvtools::Extension::kSPV_EXT_shader_64bit_indexing, // 104
  spvtools::Extension::kSPV_EXT_cooperative_matrix_maintenance1, // 105
  spvtools::Extension::kSPV_EXT_cooperative_matrix_maintenance1, // 106
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 107
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 108
  spvtools::Extension::kSPV_NV_tensor_addressing, // 109
  spvtools::Extension::kSPV_NV_cooperative_matrix_decode_vector, // 110
  spvtools::Extension::kSPV_INTEL_subgroups, // 111
  spvtools::Extension::kSPV_INTEL_media_block_io, // 112
  spvtools::Extension::kSPV_INTEL_float_controls2, // 113
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 114
  spvtools::Extension::kSPV_INTEL_function_pointers, // 115
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 116
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 117
  spvtools::Extension::kSPV_INTEL_vector_compute, // 118
  spvtools::Extension::kSPV_KHR_expect_assume, // 119
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 120
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 121
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 122
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 123
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 124
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 125
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 126
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 127
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 128
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 129
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 130
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 131
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 132
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 133
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 134
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 135
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 136
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 137
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 138
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 139
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 140
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 141
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 142
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 143
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 144
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 145
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 146
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 147
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 148
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 149
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 150
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 151
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 152
  spvtools::Extension::kSPV_INTEL_io_pipes, // 153
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 154
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 155
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 156
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 157
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 158
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 159
  spvtools::Extension::kSPV_EXT_replicated_composites, // 160
  spvtools::Extension::kSPV_KHR_bit_instructions, // 161
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 162
  spvtools::Extension::kSPV_KHR_float_controls2, // 163
  spvtools::Extension::kSPV_KHR_fma, // 164
  spvtools::Extension::kSPV_KHR_opacity_micromap, // 165
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 166
  spvtools::Extension::kSPV_INTEL_long_composites, // 167
  spvtools::Extension::kSPV_EXT_optnone, // 168
  spvtools::Extension::kSPV_INTEL_optnone, // 169
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 170
  spvtools::Extension::kSPV_INTEL_debug_module, // 171
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 172
  spvtools::Extension::kSPV_EXT_split_barrier, // 173
  spvtools::Extension::kSPV_INTEL_split_barrier, // 174
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 175
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 176
  spvtools::Extension::kSPV_INTEL_task_sequence, // 177
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 178
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 179
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 180
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 181
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 182
  spvtools::Extension::kSPV_INTEL_device_barrier, // 183
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 184
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 185
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 186
  spvtools::Extension::kSPV_INTEL_subgroup_bitcast_shuffle, // 187
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 188
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 189
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 190
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 191
  spvtools::Extension::kSPV_INTEL_function_variants, // 192
  spvtools::Extension::kSPV_INTEL_predicated_io, // 193
  spvtools::Extension::kSPV_INTEL_rounded_divide_sqrt, // 194
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 195
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 196
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 197
  spvtools::Extension::kSPV_INTEL_cache_controls, // 198
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 199
  spvtools::Extension::kSPV_INTEL_bindless_images, // 200
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 201
  spvtools::Extension::kSPV_SAMSUNG_intrinsic, // 202
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 203
  spvtools::Extension::kSPV_NV_mesh_shader, // 204
  spvtools::Extension::kSPV_EXT_mesh_shader, // 205
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 206
  spvtools::Extension::kSPV_GOOGLE_user_type, // 207
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 208
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 209
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 210
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 211
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 212
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 213
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 214
  spvtools::Extension::kSPV_KHR_variable_pointers, // 215
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 216
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 217
  spvtools::Extension::kSPV_KHR_ray_tracing, // 218
  spvtools::Extension::kSPV_KHR_ray_query, // 219
  spvtools::Extension::kSPV_NV_ray_tracing, // 220
  spvtools::Extension::kSPV_KHR_ray_tracing, // 221
  spvtools::Extension::kSPV_KHR_ray_query, // 222
  spvtools::Extension::kSPV_NV_displacement_micromap, // 223
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 224
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 225
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
