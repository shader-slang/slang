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
    case Extension::kSPV_EXT_descriptor_indexing: return IR(1168, 28);
    case Extension::kSPV_EXT_float8: return IR(1196, 15);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(1211, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(1242, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(1278, 34);
    case Extension::kSPV_EXT_mesh_shader: return IR(1312, 20);
    case Extension::kSPV_EXT_opacity_micromap: return IR(1332, 25);
    case Extension::kSPV_EXT_optnone: return IR(1357, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(1373, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(1405, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(1449, 30);
    case Extension::kSPV_EXT_shader_64bit_indexing: return IR(1479, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(1509, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(1543, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(1575, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(1611, 27);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(1638, 30);
    case Extension::kSPV_EXT_shader_tile_image: return IR(1668, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1694, 36);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1730, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1757, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1788, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1809, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(1831, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(1873, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(1918, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(1957, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(1987, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(2013, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(2038, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(2063, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(2086, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(2130, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(2156, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(2184, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(2207, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(2242, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(2273, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(2307, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(2334, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(2382, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(2413, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(2442, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(2473, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(2506, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(2525, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(2553, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(2581, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(2624, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2662, 26);
    case Extension::kSPV_INTEL_int4: return IR(2688, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2703, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2722, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2750, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2776, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2796, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2828, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2856, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2881, 33);
    case Extension::kSPV_INTEL_optnone: return IR(2914, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(2932, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(2958, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(2994, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3018, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3053, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3099, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3119, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3143, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3179, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3214, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3251, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3281, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3313, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3338, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3360, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(3381, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3398, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3423, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3458, 27);
    case Extension::kSPV_KHR_device_group: return IR(3485, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3506, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3528, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3551, 24);
    case Extension::kSPV_KHR_fma: return IR(3575, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3587, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3623, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3653, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3681, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3702, 30);
    case Extension::kSPV_KHR_multiview: return IR(3732, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3750, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3785, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3811, 32);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3843, 28);
    case Extension::kSPV_KHR_quad_control: return IR(3871, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(3892, 22);
    case Extension::kSPV_KHR_ray_query: return IR(3914, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(3932, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(3952, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(3987, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4024, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4058, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4080, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4101, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4132, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4169, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4193, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4231, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4253, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4282, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4317, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4342, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4368, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4396, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4437, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4475, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4499, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4537, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4571, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4597, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(4624, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(4650, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(4679, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(4714, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(4749, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4777, 19);
    case Extension::kSPV_NV_raw_access_chains: return IR(4796, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4821, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(4840, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(4871, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(4908, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(4941, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(4971, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5004, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5030, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5065, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5085, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5114, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5139, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5162, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5201, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5227, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(5254, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5276, 38);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,182> kExtensionNames{{
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
    {IR(1168, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(1196, 15), static_cast<uint32_t>(kSPV_EXT_float8)},
    {IR(1211, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(1242, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(1278, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(1312, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(1332, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(1357, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(1373, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(1405, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(1449, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(1479, 30), static_cast<uint32_t>(kSPV_EXT_shader_64bit_indexing)},
    {IR(1509, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(1543, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(1575, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(1611, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(1638, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(1668, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1694, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1730, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1757, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1788, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1809, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(1831, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(1873, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(1918, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(1957, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(1987, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(2013, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(2038, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(2063, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(2086, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(2130, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(2156, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(2184, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(2207, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(2242, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(2273, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(2307, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(2334, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(2382, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(2413, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(2442, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(2473, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(2506, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(2525, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(2553, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(2581, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(2624, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2662, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2688, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2703, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2722, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2750, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2776, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2796, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2828, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2856, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2881, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(2914, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(2932, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(2958, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(2994, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3018, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3053, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3099, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3119, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3143, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3179, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3214, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3251, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3281, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3313, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3338, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3360, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3381, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3398, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3423, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3458, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3485, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3506, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3528, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3551, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3575, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3587, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3623, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3653, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3681, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3702, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3732, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3750, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3785, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3811, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3843, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(3871, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(3892, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(3914, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(3932, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(3952, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(3987, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4024, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4058, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4080, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4101, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4132, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4169, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4193, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4231, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4253, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4282, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4317, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4342, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4368, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4396, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4437, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4475, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4499, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4537, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4571, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4597, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(4624, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(4650, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(4679, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(4714, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(4749, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4777, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4796, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4821, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(4840, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(4871, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(4908, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(4941, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(4971, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5004, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5030, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5065, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5085, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5114, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5139, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5162, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5201, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5227, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5254, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5276, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
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
  {0,IR(0, 0),IR(5314, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5330, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5340, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5350, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5358, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5379, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(5390, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(5472, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(5488, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(5498, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(5511, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(5524, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(5533, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(5544, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(5634, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(5647, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(5693, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(5741, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(5756, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(5771, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(5781, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5795, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(5805, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(5816, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(5828, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(5855, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(5870, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(5881, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(5891, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(5908, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(5922, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(5936, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(5948, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(5966, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(5985, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6006, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6014, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6025, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6047, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6060, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6075, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6122, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6138, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6151, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6172, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6183, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6209, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6221, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6235, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6261, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6274, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6287, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6297, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6310, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6365, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(6398, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(6431, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(6464, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(6497, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(6523, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(6536, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(6546, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(6593, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(6617, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(6637, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(6652, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(6683, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(6701, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(6719, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(6739, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(6767, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6793, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(6812, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(6839, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(6864, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(6886, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(6921, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(6968, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(6984, 15)/* ViewportMaskNV */,IR(0, 0),IR(44, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7020, 20)/* SecondaryPositionNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7059, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7083, 18)/* PositionPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7121, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7143, 16)/* FullyCoveredEXT */,IR(0, 0),IR(49, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7183, 12)/* TaskCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7195, 17)/* PrimitiveCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7212, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7231, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7253, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7275, 15)/* LayerPerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7290, 16)/* MeshViewCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7306, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7324, 13)/* BaryCoordKHR */,IR(6, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(7372, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(7411, 12)/* FragSizeEXT */,IR(8, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(7457, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(7502, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(7527, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(7551, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(7579, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(7596, 12)/* LaunchIdKHR */,IR(10, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(7619, 14)/* LaunchSizeKHR */,IR(11, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(7646, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(7681, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(7722, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(7759, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(7802, 11)/* RayTminKHR */,IR(16, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(7823, 11)/* RayTmaxKHR */,IR(17, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(7844, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(7889, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(7922, 17)/* WorldToObjectKHR */,IR(20, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(7955, 7)/* HitTNV */,IR(0, 0),IR(56, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(7962, 11)/* HitKindKHR */,IR(21, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(7983, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(57, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8023, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(58, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8080, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8147, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8184, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8223, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(60, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8243, 14)/* HitIsSphereNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8285, 11)/* HitIsLSSNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(8335, 20)/* HitSpherePositionNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(8355, 13)/* WarpsPerSMNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(8387, 10)/* SMCountNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(8397, 9)/* WarpIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(8406, 7)/* SMIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(8413, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(8431, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(8465, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(8498, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(8516, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(8530, 12)/* ClusterIDNV */,IR(0, 0),IR(64, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(8583, 12)/* CullMaskKHR */,IR(0, 0),IR(65, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(8610, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(5481, 7)/* Shader */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(5556, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(5565, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5369, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(8617, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5323, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(8625, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(8634, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(8648, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(8656, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(8664, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(8670, 13)/* Int64Atomics */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(8683, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(8694, 15)/* ImageReadWrite */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(8709, 12)/* ImageMipmap */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(8721, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(8727, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(8734, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(8748, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(8763, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(8777, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(8783, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(8805, 18)/* GeometryPointSize */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(8823, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(8843, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(8867, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(8901, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(8934, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(8968, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(5498, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(5511, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9001, 15)/* ImageCubeArray */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(5837, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9033, 10)/* ImageRect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9043, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9055, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9070, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9075, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9091, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9107, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9114, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9124, 8)/* Image1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9016, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9132, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9146, 12)/* ImageBuffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9158, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9171, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9199, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9210, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9228, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9250, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9268, 16)/* GeometryStreams */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9284, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(9314, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(5707, 14)/* MultiViewport */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(9345, 17)/* SubgroupDispatch */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(9362, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(9375, 12)/* PipeStorage */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6088, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(9387, 20)/* GroupNonUniformVote */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(9407, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6343, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(9433, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(9456, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(9487, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(9512, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(5653, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(5721, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(9532, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6245, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(9550, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(9578, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(9606, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(9636, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(9647, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(9684, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(9724, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(9733, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(9761, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(9771, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(77, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(6570, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6104, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(6508, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(9819, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(9852, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(9895, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(9939, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(9955, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10008, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(80, 1),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10060, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10082, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(6605, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(6627, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10103, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10133, 17)/* VariablePointers */,IR(0, 0),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10150, 17)/* AtomicStorageOps */,IR(0, 0),IR(82, 1),IR(44, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10167, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10195, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10219, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(83, 1),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(10253, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(10274, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(10289, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(10307, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(10332, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(10348, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(10364, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(10387, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(10399, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(10418, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(84, 2),IR(50, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(5591, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(10450, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(10476, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(10497, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(6667, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(10519, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(86, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(10551, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(10574, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(10590, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(10612, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(6904, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(10628, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(10649, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(10663, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(6950, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(10678, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(10693, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(10707, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(87, 2),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(10734, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(10750, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(89, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(10772, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(90, 2),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(10801, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(10830, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(69, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(5665, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(92, 1),IR(68, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(6999, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(93, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7040, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(94, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7101, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7159, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(5605, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(10885, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(5619, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(7349, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(10924, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(7438, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(10999, 29)/* GroupNonUniformPartitionedNV */,IR(0, 0),IR(1, 0),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(11028, 17)/* ShaderNonUniform */,IR(29, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(11065, 23)/* RuntimeDescriptorArray */,IR(30, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(11114, 36)/* InputAttachmentArrayDynamicIndexing */,IR(31, 1),IR(95, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(11189, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(32, 1),IR(73, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(11270, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(96, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(11351, 37)/* UniformBufferArrayNonUniformIndexing */,IR(34, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(11428, 36)/* SampledImageArrayNonUniformIndexing */,IR(35, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(11503, 37)/* StorageBufferArrayNonUniformIndexing */,IR(36, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(11580, 36)/* StorageImageArrayNonUniformIndexing */,IR(37, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(11655, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(38, 1),IR(98, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(11736, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(39, 1),IR(100, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(11823, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(102, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8053, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(5578, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8000, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(11910, 18)/* VulkanMemoryModel */,IR(41, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(11949, 29)/* VulkanMemoryModelDeviceScope */,IR(42, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(5441, 31)/* PhysicalStorageBufferAddresses */,IR(43, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(12044, 32)/* ComputeDerivativeGroupLinearKHR */,IR(44, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(12107, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(12132, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(12152, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(12185, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(8368, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(12223, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(12255, 25)/* DemoteToHelperInvocation */,IR(45, 1),IR(3, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(12308, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(12331, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(12360, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(60, 1),IR(84, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(12386, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(12404, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(12429, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(12449, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8114, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(60, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(12471, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8257, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8296, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(12489, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(12512, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(12542, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(12573, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(12613, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(12649, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(12679, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(8542, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(60, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(12707, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(12726, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(12747, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(12774, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(12800, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(12831, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(12852, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(12875, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(12898, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(12920, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(12944, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(12953, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(12976, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(12999, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(13022, 19)/* VectorComputeINTEL */,IR(0, 0),IR(104, 1),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(13041, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(13056, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(13072, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(13105, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(13143, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(13182, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(13207, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(13233, 27)/* FPGAMemoryAttributesALTERA */,IR(46, 1),IR(1, 0),IR(103, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(13286, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(13306, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(47, 1),IR(1, 0),IR(106, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(13371, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(48, 1),IR(1, 0),IR(108, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(13446, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(13476, 23)/* FPGALoopControlsALTERA */,IR(49, 1),IR(1, 0),IR(111, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(13521, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(13543, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(13569, 25)/* FPGAMemoryAccessesALTERA */,IR(50, 1),IR(1, 0),IR(114, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(13618, 28)/* FPGAClusterAttributesALTERA */,IR(51, 1),IR(1, 0),IR(116, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(13673, 15)/* LoopFuseALTERA */,IR(52, 1),IR(1, 0),IR(118, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(13702, 21)/* FPGADSPControlALTERA */,IR(53, 1),IR(1, 0),IR(120, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(13743, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(13769, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(54, 1),IR(1, 0),IR(123, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(13850, 25)/* FPGABufferLocationALTERA */,IR(55, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(13899, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(56, 1),IR(1, 0),IR(127, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(13968, 24)/* USMStorageClassesALTERA */,IR(57, 1),IR(1, 0),IR(129, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(14015, 30)/* RuntimeAlignedAttributeALTERA */,IR(58, 1),IR(1, 0),IR(131, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(14074, 14)/* IOPipesALTERA */,IR(59, 1),IR(1, 0),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(14101, 20)/* BlockingPipesALTERA */,IR(60, 1),IR(1, 0),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(14140, 14)/* FPGARegALTERA */,IR(61, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(14167, 19)/* DotProductInputAll */,IR(62, 1),IR(1, 0),IR(139, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(14208, 22)/* DotProductInput4x8Bit */,IR(63, 1),IR(105, 1),IR(139, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(14255, 28)/* DotProductInput4x8BitPacked */,IR(64, 1),IR(1, 0),IR(139, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(14314, 11)/* DotProduct */,IR(65, 1),IR(1, 0),IR(139, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(8595, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(9798, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(140, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(14339, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(14363, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(142, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(14379, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(76, 1),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(14404, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(14419, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(145, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(14426, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(14446, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(14466, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(14486, 11)/* OptNoneEXT */,IR(66, 1),IR(1, 0),IR(148, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(14510, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(14530, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(14551, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(14575, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(153, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(14593, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(154, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(14612, 30)/* FPGAClusterAttributesV2ALTERA */,IR(67, 1),IR(106, 1),IR(116, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(14671, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(107, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(14699, 19)/* TaskSequenceALTERA */,IR(68, 1),IR(1, 0),IR(155, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(14736, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(157, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(14752, 25)/* FPGALatencyControlALTERA */,IR(69, 1),IR(1, 0),IR(158, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(14801, 29)/* FPGAArgumentInterfacesALTERA */,IR(70, 1),IR(1, 0),IR(160, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(14858, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(14888, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(71, 1),IR(1, 0),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(14959, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(14987, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(15010, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(108, 1),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(15040, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(108, 1),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(15070, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(167, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(15108, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(168, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(15136, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(109, 2),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(15168, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(169, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(15189, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(111, 1),IR(169, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(15211, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(170, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(15237, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(171, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(15264, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(15289, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(173, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(15308, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(174, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(15328, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(15348, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15360, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15368, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(15376, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15382, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15389, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15400, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15409, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15422, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15428, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15438, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15444, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(15459, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15484, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15500, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15514, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(15527, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(15539, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(15556, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(15568, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(15583, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(15596, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(15611, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(15629, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(15646, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(15667, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(15687, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(15707, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(15723, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(15739, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(15759, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(15783, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(15789, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(15794, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(15833, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(15844, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(15853, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(15858, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(15865, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(15876, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(15883, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(15892, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(15902, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(15915, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(15928, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(15939, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(15949, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(15959, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(15969, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(15982, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(15996, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(16010, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(16024, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(16039, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(16055, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(16071, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(16087, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(16106, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(16127, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(16139, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(16151, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(16163, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(16178, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(16203, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16231, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(16239, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(16266, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(16293, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(16320, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(16352, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(16378, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(16382, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(16389, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(16393, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(16404, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(16415, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(16436, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(16447, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(16461, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(16482, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(15348, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15360, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15368, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15376, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15382, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15389, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15400, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(15409, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15422, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15428, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15438, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15484, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15500, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15514, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(15527, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(15539, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(15556, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(15568, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(15583, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(15596, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(15611, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(15629, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(15646, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(15667, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(15687, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(15707, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(15783, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(15789, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(15794, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(15833, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(15844, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(15853, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(15858, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(15865, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(15876, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(15892, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(15902, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(15915, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(16500, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(16517, 7)/* SpecId */,IR(0, 0),IR(112, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(16524, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(16530, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(16542, 9)/* RowMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(16551, 9)/* ColMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(16560, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(16572, 13)/* MatrixStride */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(16585, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(16596, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(16607, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(16641, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(16649, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(16663, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(16668, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(16674, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(16683, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(16690, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(16700, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(16709, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(16717, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(16726, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(16735, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(16744, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(16756, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(16768, 8)/* Uniform */,IR(0, 0),IR(114, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(16802, 10)/* UniformId */,IR(0, 0),IR(114, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(16812, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(16832, 7)/* Stream */,IR(0, 0),IR(116, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(16839, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(16848, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(16858, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(16864, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(16872, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(16886, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(16893, 10)/* XfbBuffer */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(16903, 10)/* XfbStride */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(16959, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(17007, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(17057, 15)/* FPFastMathMode */,IR(0, 0),IR(118, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(17134, 18)/* LinkageAttributes */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(17152, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(17166, 21)/* InputAttachmentIndex */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(17187, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(17197, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(17231, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(17243, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(17259, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(17303, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(176, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(17316, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(176, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(17331, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(17349, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(17371, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(17393, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(17411, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(17443, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(17463, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(17486, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(17506, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(17531, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(17558, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(17583, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(17611, 19)/* OverrideCoverageNV */,IR(0, 0),IR(122, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(17630, 14)/* PassthroughNV */,IR(0, 0),IR(123, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(17644, 19)/* ViewportRelativeNV */,IR(0, 0),IR(94, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(17663, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(17691, 16)/* PerPrimitiveEXT */,IR(72, 1),IR(124, 2),IR(177, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(17722, 10)/* PerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(17732, 10)/* PerTaskNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(17742, 13)/* PerVertexKHR */,IR(73, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(17767, 11)/* NonUniform */,IR(74, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(17792, 16)/* RestrictPointer */,IR(75, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(17827, 15)/* AliasedPointer */,IR(76, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(17860, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(17890, 18)/* BindlessSamplerNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(17908, 16)/* BindlessImageNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(17924, 15)/* BoundSamplerNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(17939, 13)/* BoundImageNV */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(17952, 14)/* SIMTCallINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(17966, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(129, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(17992, 13)/* ClobberINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(18005, 17)/* SideEffectsINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(18022, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(18049, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(18070, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(18097, 15)/* StackCallINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(18112, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(18138, 14)/* CounterBuffer */,IR(77, 1),IR(1, 0),IR(179, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(18176, 13)/* UserSemantic */,IR(78, 1),IR(1, 0),IR(179, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(18208, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(180, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(18223, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(18280, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(18304, 15)/* RegisterALTERA */,IR(79, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(18333, 13)/* MemoryALTERA */,IR(80, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(18358, 15)/* NumbanksALTERA */,IR(81, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(18387, 16)/* BankwidthALTERA */,IR(82, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(18418, 23)/* MaxPrivateCopiesALTERA */,IR(83, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(18463, 17)/* SinglepumpALTERA */,IR(84, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(18496, 17)/* DoublepumpALTERA */,IR(85, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(18529, 20)/* MaxReplicatesALTERA */,IR(86, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(18568, 21)/* SimpleDualPortALTERA */,IR(87, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(18609, 12)/* MergeALTERA */,IR(88, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(18674, 15)/* BankBitsALTERA */,IR(89, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(18703, 21)/* ForcePow2DepthALTERA */,IR(90, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(18744, 17)/* StridesizeALTERA */,IR(91, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(18777, 15)/* WordsizeALTERA */,IR(92, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(18806, 19)/* TrueDualPortALTERA */,IR(93, 1),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(18843, 20)/* BurstCoalesceALTERA */,IR(94, 1),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(18882, 16)/* CacheSizeALTERA */,IR(95, 1),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(18913, 29)/* DontStaticallyCoalesceALTERA */,IR(96, 1),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(18970, 15)/* PrefetchALTERA */,IR(97, 1),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(18999, 18)/* StallEnableALTERA */,IR(98, 1),IR(106, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(19034, 26)/* FuseLoopsInFunctionALTERA */,IR(99, 1),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(19085, 20)/* MathOpDSPModeALTERA */,IR(100, 1),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(19124, 16)/* AliasScopeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(19140, 13)/* NoAliasINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(19153, 25)/* InitiationIntervalALTERA */,IR(101, 1),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(19202, 21)/* MaxConcurrencyALTERA */,IR(102, 1),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(19243, 21)/* PipelineEnableALTERA */,IR(103, 1),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(19284, 21)/* BufferLocationALTERA */,IR(104, 1),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(19325, 20)/* IOPipeStorageALTERA */,IR(105, 1),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(19398, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(19429, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(19454, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(19489, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(19507, 16)/* StallFreeALTERA */,IR(106, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(19569, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(19595, 26)/* LatencyControlLabelALTERA */,IR(107, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(19646, 31)/* LatencyControlConstraintALTERA */,IR(108, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(19707, 28)/* ConduitKernelArgumentALTERA */,IR(109, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(19762, 32)/* RegisterMapKernelArgumentALTERA */,IR(110, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(19825, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(111, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(19892, 31)/* MMHostInterfaceDataWidthALTERA */,IR(112, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(19953, 29)/* MMHostInterfaceLatencyALTERA */,IR(113, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(20044, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(114, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(20113, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(115, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(20172, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(116, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(20237, 27)/* StableKernelArgumentALTERA */,IR(117, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(20329, 16)/* HostAccessINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(20392, 15)/* InitModeALTERA */,IR(118, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(20421, 29)/* ImplementInRegisterMapALTERA */,IR(119, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(20478, 17)/* ConditionalINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(20531, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(20590, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(20613, 3)/* 1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(20616, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(20619, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(20622, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(20627, 5)/* Rect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(20632, 7)/* Buffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(20639, 12)/* SubpassData */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(20651, 17)/* TileImageDataEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(20668, 12)/* Invocations */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(20680, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(20693, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(20715, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(20736, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(20750, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(20765, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(20784, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(20800, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(20816, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(20835, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(20845, 4)/* Xfb */,IR(0, 0),IR(117, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(20849, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(20864, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(20877, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(20887, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(20902, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(20912, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(20926, 12)/* InputPoints */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(20938, 11)/* InputLines */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(20949, 20)/* InputLinesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(20969, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(20979, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(21003, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(21009, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(21018, 15)/* OutputVertices */,IR(0, 0),IR(148, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(21033, 13)/* OutputPoints */,IR(0, 0),IR(152, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(21046, 16)/* OutputLineStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(21062, 20)/* OutputTriangleStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(21082, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(21094, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(21109, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(21121, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6075, 13)/* SubgroupSize */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(21131, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(21153, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(155, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(21177, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(21189, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(21205, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(21239, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(21273, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(21309, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(181, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(21339, 18)/* PostDepthCoverage */,IR(0, 0),IR(158, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(10274, 15)/* DenormPreserve */,IR(0, 0),IR(159, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(10289, 18)/* DenormFlushToZero */,IR(0, 0),IR(160, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(10307, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(161, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(10332, 16)/* RoundingModeRTE */,IR(0, 0),IR(162, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(10348, 16)/* RoundingModeRTZ */,IR(0, 0),IR(163, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(21357, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(21391, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(21411, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(21440, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(21463, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(21478, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(21493, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(21514, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(6968, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(21538, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(21559, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(21587, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(21613, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(21636, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(21663, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(21688, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(183, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(21710, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(164, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(21729, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(164, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(21749, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(21769, 15)/* OutputLinesEXT */,IR(120, 1),IR(124, 2),IR(177, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(21798, 20)/* OutputPrimitivesEXT */,IR(121, 1),IR(124, 2),IR(177, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(21837, 24)/* DerivativeGroupQuadsKHR */,IR(122, 1),IR(165, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(21884, 25)/* DerivativeGroupLinearKHR */,IR(123, 1),IR(166, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(21933, 19)/* OutputTrianglesEXT */,IR(124, 1),IR(124, 2),IR(177, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(21970, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(21995, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(22022, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(168, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(22048, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(168, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(22076, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(169, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(22107, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(169, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(12489, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(22140, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(22167, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(22188, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(22209, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(22235, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(22262, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(172, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(22284, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(172, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(22300, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(172, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(22320, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(107, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(22342, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(22370, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(22394, 18)/* FPFastMathDefault */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(22412, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(22436, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(174, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(22462, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(22485, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(22507, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(22582, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(22609, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(22616, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(22636, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(5556, 9)/* Geometry */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(15883, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(22659, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5323, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(22669, 7)/* TaskNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(22676, 7)/* MeshNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(22683, 17)/* RayGenerationKHR */,IR(125, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(22716, 16)/* IntersectionKHR */,IR(126, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(22747, 10)/* AnyHitKHR */,IR(127, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(22766, 14)/* ClosestHitKHR */,IR(128, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(22793, 8)/* MissKHR */,IR(129, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(22808, 12)/* CallableKHR */,IR(130, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(22831, 8)/* TaskEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(22839, 8)/* MeshEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(22847, 9)/* Preserve */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(22856, 12)/* FlushToZero */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(22868, 12)/* BFloat16KHR */,IR(0, 0),IR(89, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(22880, 14)/* Float8E4M3EXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(22894, 14)/* Float8E5M2EXT */,IR(0, 0),IR(121, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(22908, 5)/* IEEE */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(22913, 4)/* ALT */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(22917, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(22924, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(22931, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(22935, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(22946, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(22951, 14)/* AllowContract */,IR(131, 1),IR(176, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(22988, 13)/* AllowReassoc */,IR(132, 1),IR(176, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(23019, 15)/* AllowTransform */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(23034, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23038, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(23042, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(23046, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(23050, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(23066, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(23082, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(23100, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(23118, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(23125, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(23136, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(23141, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(14486, 11)/* OptNoneEXT */,IR(66, 1),IR(178, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(23147, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(23152, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(23157, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(23163, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(23168, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(23176, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(23186, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(23194, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(23206, 21)/* RuntimeAlignedALTERA */,IR(133, 1),IR(179, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(23247, 7)/* Reduce */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(23254, 14)/* InclusiveScan */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(23268, 14)/* ExclusiveScan */,IR(0, 0),IR(180, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(23282, 16)/* ClusteredReduce */,IR(0, 0),IR(183, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(23298, 20)/* PartitionedReduceNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(23318, 27)/* PartitionedInclusiveScanNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(23345, 27)/* PartitionedExclusiveScanNV */,IR(0, 0),IR(184, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(23372, 10)/* NoneINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(23382, 10)/* ReadINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(23392, 11)/* WriteINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(23403, 15)/* ReadWriteINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(23418, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(23423, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(23427, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(23432, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(16886, 7)/* Offset */,IR(0, 0),IR(185, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(23444, 13)/* ConstOffsets */,IR(0, 0),IR(185, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(16683, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9107, 7)/* MinLod */,IR(0, 0),IR(186, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(23457, 19)/* MakeTexelAvailable */,IR(134, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(23498, 17)/* MakeTexelVisible */,IR(135, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(23535, 16)/* NonPrivateTexel */,IR(136, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(23570, 14)/* VolatileTexel */,IR(137, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(23601, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(23612, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(23623, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(23635, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(23643, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(23653, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(23664, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(23674, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(23685, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(23699, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(23713, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(23728, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(23739, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(23751, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(23763, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(23776, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(23790, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(23804, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(15376, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(23814, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(23825, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(23842, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(23858, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(23878, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(23898, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(23918, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(23937, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(23956, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(23975, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(23991, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(24007, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(24009, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(24011, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(24014, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(24017, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(24021, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(24026, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(24031, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(24036, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(24046, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(24056, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(24059, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(24063, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(24068, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(24074, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(24087, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(24092, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(24098, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(24104, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(24110, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(24115, 28)/* InitOnDeviceReprogramALTERA */,IR(138, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(24170, 24)/* InitOnDeviceResetALTERA */,IR(139, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(24217, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(24224, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(24235, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(24249, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(24261, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(24274, 7)/* Export */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(24281, 7)/* Import */,IR(0, 0),IR(120, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(24288, 12)/* LinkOnceODR */,IR(0, 0),IR(120, 1),IR(186, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(24300, 14)/* UncachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(24314, 12)/* CachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(24326, 15)/* StreamingINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(24341, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(24366, 17)/* ConstCachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(24383, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(24390, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(24401, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(24420, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(24437, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(24451, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(24465, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(24483, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(24493, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(19153, 25)/* InitiationIntervalALTERA */,IR(101, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(19202, 21)/* MaxConcurrencyALTERA */,IR(102, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(24506, 22)/* DependencyArrayALTERA */,IR(140, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(19243, 21)/* PipelineEnableALTERA */,IR(103, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(24549, 19)/* LoopCoalesceALTERA */,IR(141, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(24586, 22)/* MaxInterleavingALTERA */,IR(142, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(24629, 27)/* SpeculatedIterationsALTERA */,IR(143, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(24682, 15)/* NoFusionALTERA */,IR(144, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(24711, 16)/* LoopCountALTERA */,IR(145, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(24742, 27)/* MaxReinvocationDelayALTERA */,IR(146, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(24795, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(24824, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(24853, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(24874, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(24900, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(24923, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(24946, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(24969, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(24992, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(25009, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(25026, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(25052, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(25078, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(25105, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(16717, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(25132, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(23623, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(25140, 21)/* MakePointerAvailable */,IR(147, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(25185, 19)/* MakePointerVisible */,IR(148, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(25226, 18)/* NonPrivatePointer */,IR(149, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(25265, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(136, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(25285, 17)/* NoAliasINTELMask */,IR(0, 0),IR(136, 1),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(25302, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(25309, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(25317, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(25324, 7)/* Vulkan */,IR(150, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(25341, 8)/* Relaxed */,IR(151, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(25349, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(25357, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(25365, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(25380, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(25403, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(25417, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(25432, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(25448, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(25469, 20)/* AtomicCounterMemory */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(25489, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(25501, 13)/* OutputMemory */,IR(152, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(25530, 14)/* MakeAvailable */,IR(153, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(25561, 12)/* MakeVisible */,IR(154, 1),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(16717, 9)/* Volatile */,IR(0, 0),IR(187, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(25588, 10)/* AutoINTEL */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(25598, 5)/* WRAP */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(25603, 4)/* SAT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(25607, 9)/* SAT_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(25616, 8)/* SAT_SYM */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(25624, 25)/* PackedVectorFormat4x8Bit */,IR(155, 1),IR(1, 0),IR(139, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(25677, 4)/* TRN */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(25681, 9)/* TRN_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(25690, 4)/* RND */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(25694, 9)/* RND_ZERO */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(25703, 8)/* RND_INF */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(25711, 12)/* RND_MIN_INF */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(25723, 9)/* RND_CONV */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(25732, 13)/* RND_CONV_ODD */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(25745, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(25770, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(16231, 8)/* NoneKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(25793, 10)/* OpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(25803, 12)/* NoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(25815, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(25838, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(25862, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(25889, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(25917, 14)/* CullOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(25931, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(25947, 17)/* SkipTrianglesKHR */,IR(156, 1),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(25988, 13)/* SkipAABBsKHR */,IR(0, 0),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(26001, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(192, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(26031, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(26072, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(26109, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(26146, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(26187, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(26229, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(26262, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(26295, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(26307, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(26313, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(26320, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(26335, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(26343, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(26350, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(26358, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(26366, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(26374, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(26379, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(26385, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(26396, 6)/* Rg32f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(26402, 6)/* Rg16f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(26408, 13)/* R11fG11fB10f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(26421, 5)/* R16f */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(26426, 7)/* Rgba16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(26433, 8)/* Rgb10A2 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(26441, 5)/* Rg16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(26446, 4)/* Rg8 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(26450, 4)/* R16 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(26454, 3)/* R8 */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(26457, 12)/* Rgba16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(26469, 10)/* Rg16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(26479, 9)/* Rg8Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(26488, 9)/* R16Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(26497, 8)/* R8Snorm */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(26505, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(26513, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(26521, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(26528, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(26533, 6)/* Rg32i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(26539, 6)/* Rg16i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(26545, 5)/* Rg8i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(26550, 5)/* R16i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(26555, 4)/* R8i */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(26559, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(26568, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(26577, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(26585, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(26591, 10)/* Rgb10a2ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(26601, 7)/* Rg32ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(26608, 7)/* Rg16ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(26615, 6)/* Rg8ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(26621, 6)/* R16ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(26627, 5)/* R8ui */,IR(0, 0),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(26632, 6)/* R64ui */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(26638, 5)/* R64i */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(26643, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(26655, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(26662, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(26672, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(26681, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(26692, 12)/* QueueFamily */,IR(157, 1),IR(187, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(26719, 14)/* ShaderCallKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(26733, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(26741, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(26753, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(15348, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(15360, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(15368, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(15376, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(15382, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(15389, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(15400, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(15409, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(15422, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(15428, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(15438, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(15444, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(15459, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(15484, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(15500, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(15514, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(15527, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(15539, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(15556, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(15568, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(15583, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(15596, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(15611, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(15629, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(15646, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(15667, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(15687, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(15707, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(15723, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(15739, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(15759, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(26782, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(15783, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(15789, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(15794, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(15833, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(15844, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(15853, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(15858, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(15865, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(15876, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(15883, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(15892, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(15902, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(15915, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(15928, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(26350, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(26808, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(26813, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(26818, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(26827, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(26838, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(26843, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(26858, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(26863, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(26870, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(26875, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(26880, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(26886, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(26890, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(26895, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(26911, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(16768, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(26917, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(26662, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(26924, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(26939, 8)/* Private */,IR(0, 0),IR(196, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(26947, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(26956, 8)/* Generic */,IR(0, 0),IR(198, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(26964, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(26977, 14)/* AtomicCounter */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(26991, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(26997, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(187, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(27011, 13)/* TileImageEXT */,IR(0, 0),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(27024, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(27043, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(27059, 16)/* CallableDataKHR */,IR(158, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(27090, 24)/* IncomingCallableDataKHR */,IR(159, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(27137, 14)/* RayPayloadKHR */,IR(160, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(27164, 16)/* HitAttributeKHR */,IR(161, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(27195, 22)/* IncomingRayPayloadKHR */,IR(162, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(27238, 22)/* ShaderRecordBufferKHR */,IR(163, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(27281, 22)/* PhysicalStorageBuffer */,IR(164, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(27328, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(27349, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(53, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(27373, 17)/* CodeSectionINTEL */,IR(0, 0),IR(199, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(27390, 17)/* DeviceOnlyALTERA */,IR(165, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(27423, 15)/* HostOnlyALTERA */,IR(166, 1),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(24300, 14)/* UncachedINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(27452, 18)/* WriteThroughINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(27470, 15)/* WriteBackINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(24326, 15)/* StreamingINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(15479, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(27485, 11)/* TensorView */,IR(0, 0),IR(201, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(27496, 11)/* DecodeFunc */,IR(0, 0),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(27507, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(16726, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(26295, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(26313, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(26320, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(27517, 8)/* NoneARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(27525, 15)/* NontemporalARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(27540, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(27560, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(27584, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(27606, 21)/* NonPrivateElementARM */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
static const std::array<NameIndex, 1362> kOperandNames{{
  {IR(5314, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5340, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5330, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5350, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5358, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5379, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5390, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5414, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7324, 13), 92}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7337, 12), 92}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6719, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6739, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7372, 20), 93}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7392, 19), 93}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6767, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6864, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6793, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6812, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6839, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6523, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6497, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5498, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7231, 22), 87}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8530, 12), 131}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6261, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6235, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6274, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5511, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7253, 22), 88}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8583, 12), 132}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7579, 17), 99}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7983, 17), 113}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6593, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6536, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6025, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5795, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5881, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7457, 23), 95}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7411, 12), 94}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6886, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7423, 15), 94}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5816, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7143, 16), 83}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5966, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6060, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6047, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6014, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5891, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8285, 11), 120}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8243, 14), 119}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8465, 33), 128}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8431, 34), 127}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7962, 11), 112}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7973, 10), 112}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8413, 18), 126}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8516, 14), 130}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8147, 37), 116}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8080, 34), 115}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8335, 20), 121}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8498, 18), 129}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7955, 7), 111}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8023, 30), 114}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8184, 20), 117}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8204, 19), 117}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7844, 23), 108}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7867, 22), 108}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5533, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6221, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5634, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7480, 22), 95}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7596, 12), 100}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7608, 11), 100}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7619, 14), 101}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7633, 13), 101}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5647, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7275, 15), 89}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5948, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5985, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7290, 16), 90}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7306, 18), 91}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6151, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6138, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5908, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7759, 22), 105}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7781, 21), 105}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7722, 19), 104}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7741, 18), 104}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7889, 17), 109}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7906, 16), 109}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5781, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5805, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5488, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5472, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7083, 18), 81}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7195, 17), 85}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5544, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7212, 19), 86}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7527, 24), 97}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7502, 25), 96}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6546, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7551, 28), 98}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8223, 20), 118}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7823, 11), 107}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7834, 10), 107}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7802, 11), 106}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7813, 10), 106}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6921, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8387, 10), 123}, // 107 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8406, 7), 125}, // 108 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5828, 9), 24}, // 109 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5870, 11), 26}, // 110 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5855, 15), 25}, // 111 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7020, 20), 79}, // 112 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7059, 24), 80}, // 113 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6968, 16), 77}, // 114 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6637, 15), 64}, // 115 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6310, 15), 53}, // 116 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6325, 18), 53}, // 117 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6365, 15), 54}, // 118 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6380, 18), 54}, // 119 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6398, 15), 55}, // 120 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6413, 18), 55}, // 121 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6172, 11), 44}, // 122 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6431, 15), 56}, // 123 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6446, 18), 56}, // 124 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6183, 26), 45}, // 125 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6464, 15), 57}, // 126 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6479, 18), 57}, // 127 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6122, 16), 41}, // 128 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6075, 13), 40}, // 129 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7183, 12), 84}, // 130 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5771, 10), 19}, // 131 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5756, 15), 18}, // 132 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5741, 15), 17}, // 133 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6701, 18), 67}, // 134 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6683, 18), 66}, // 135 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6652, 15), 65}, // 136 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5524, 9), 11}, // 137 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6209, 12), 46}, // 138 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6617, 10), 63}, // 139 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5693, 14), 16}, // 140 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6984, 15), 78}, // 141 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7121, 22), 82}, // 142 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6287, 10), 51}, // 143 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8397, 9), 124}, // 144 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6297, 13), 52}, // 145 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8355, 13), 122}, // 146 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6006, 8), 35}, // 147 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5936, 12), 31}, // 148 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5922, 14), 30}, // 149 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7681, 21), 103}, // 150 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7702, 20), 103}, // 151 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7646, 18), 102}, // 152 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7664, 17), 102}, // 153 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7922, 17), 110}, // 154 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7939, 16), 110}, // 155 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5369, 10), 137}, // 156 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13899, 35), 360}, // 157 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13934, 34), 360}, // 158 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13371, 38), 348}, // 159 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13409, 37), 348}, // 160 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13306, 33), 347}, // 161 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13339, 32), 347}, // 162 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14593, 19), 385}, // 163 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12944, 9), 333}, // 164 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14510, 20), 381}, // 165 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12999, 23), 336}, // 166 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12449, 22), 310}, // 167 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14426, 20), 377}, // 168 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12953, 23), 334}, // 169 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14446, 20), 378}, // 170 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12976, 23), 335}, // 171 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8763, 14), 153}, // 172 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10150, 17), 229}, // 173 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14551, 24), 383}, // 174 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10772, 29), 263}, // 175 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10750, 22), 262}, // 176 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10734, 16), 261}, // 177 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15328, 20), 408}, // 178 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12386, 18), 307}, // 179 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14363, 16), 373}, // 180 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14101, 20), 364}, // 181 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14121, 19), 364}, // 182 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15289, 19), 406}, // 183 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5498, 13), 163}, // 184 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12044, 32), 296}, // 185 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12076, 31), 296}, // 186 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10924, 31), 275}, // 187 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10955, 30), 275}, // 188 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12649, 30), 320}, // 189 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10519, 32), 248}, // 190 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12542, 31), 317}, // 191 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9798, 21), 371}, // 192 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9733, 28), 211}, // 193 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12132, 20), 298}, // 194 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12573, 40), 318}, // 195 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12512, 30), 316}, // 196 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12613, 36), 319}, // 197 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12429, 20), 309}, // 198 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12679, 28), 321}, // 199 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6245, 16), 203}, // 200 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5511, 13), 164}, // 201 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14530, 21), 382}, // 202 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12255, 25), 303}, // 203 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12280, 28), 303}, // 204 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10289, 18), 235}, // 205 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10274, 15), 234}, // 206 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9210, 18), 182}, // 207 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8734, 14), 151}, // 208 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6605, 12), 225}, // 209 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12308, 23), 304}, // 210 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14314, 11), 369}, // 211 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14208, 22), 367}, // 212 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14230, 25), 367}, // 213 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14255, 28), 368}, // 214 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14283, 31), 368}, // 215 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14167, 19), 366}, // 216 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14186, 22), 366}, // 217 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14325, 14), 369}, // 218 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6508, 15), 216}, // 219 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13056, 16), 339}, // 220 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14419, 7), 376}, // 221 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13286, 20), 346}, // 222 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14801, 29), 391}, // 223 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14830, 28), 391}, // 224 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13850, 25), 359}, // 225 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13875, 24), 359}, // 226 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13618, 28), 354}, // 227 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13646, 27), 354}, // 228 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14612, 30), 386}, // 229 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14642, 29), 386}, // 230 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13702, 21), 356}, // 231 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13723, 20), 356}, // 232 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13769, 41), 358}, // 233 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13810, 40), 358}, // 234 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13543, 26), 352}, // 235 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14671, 28), 387}, // 236 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14752, 25), 390}, // 237 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14777, 24), 390}, // 238 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13476, 23), 350}, // 239 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13499, 22), 350}, // 240 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13569, 25), 353}, // 241 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13594, 24), 353}, // 242 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13233, 27), 345}, // 243 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13260, 26), 345}, // 244 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14140, 14), 365}, // 245 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14154, 13), 365}, // 246 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14736, 16), 389}, // 247 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8648, 8), 142}, // 248 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8634, 14), 141}, // 249 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10574, 16), 250}, // 250 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8656, 8), 143}, // 251 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9771, 27), 213}, // 252 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9761, 10), 212}, // 253 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14404, 15), 375}, // 254 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12852, 23), 329}, // 255 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7349, 23), 274}, // 256 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10902, 22), 274}, // 257 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7438, 19), 276}, // 258 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7159, 24), 270}, // 259 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10612, 16), 252}, // 260 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12223, 32), 302}, // 261 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12152, 33), 299}, // 262 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12185, 38), 300}, // 263 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6570, 23), 214}, // 264 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13207, 26), 344}, // 265 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12898, 22), 331}, // 266 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15189, 22), 402}, // 267 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9055, 15), 169}, // 268 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5556, 9), 135}, // 269 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8805, 18), 156}, // 270 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10830, 28), 265}, // 271 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9268, 16), 185}, // 272 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14888, 36), 393}, // 273 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14924, 35), 393}, // 274 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14858, 30), 392}, // 275 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9724, 9), 210}, // 276 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6088, 16), 192}, // 277 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9407, 26), 194}, // 278 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6343, 22), 195}, // 279 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9487, 25), 198}, // 280 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10999, 29), 277}, // 281 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9512, 20), 199}, // 282 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14379, 25), 374}, // 283 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9433, 23), 196}, // 284 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9456, 31), 197}, // 285 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9387, 20), 193}, // 286 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15211, 26), 403}, // 287 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8727, 7), 150}, // 288 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14074, 14), 363}, // 289 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14088, 13), 363}, // 290 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9124, 8), 175}, // 291 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8683, 11), 146}, // 292 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9146, 12), 178}, // 293 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9001, 15), 165}, // 294 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10885, 17), 272}, // 295 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10590, 22), 251}, // 296 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8823, 20), 157}, // 297 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9158, 13), 179}, // 298 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8709, 12), 148}, // 299 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9199, 11), 181}, // 300 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8694, 15), 147}, // 301 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10628, 21), 254}, // 302 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9033, 10), 167}, // 303 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12920, 24), 332}, // 304 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9075, 16), 171}, // 305 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11114, 36), 280}, // 306 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11150, 39), 280}, // 307 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11655, 39), 287}, // 308 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11694, 42), 287}, // 309 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8777, 6), 154}, // 310 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10707, 27), 260}, // 311 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10693, 14), 259}, // 312 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8664, 6), 144}, // 313 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8670, 13), 145}, // 314 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10649, 14), 255}, // 315 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9070, 5), 170}, // 316 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12875, 23), 330}, // 317 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9228, 22), 183}, // 318 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5323, 7), 139}, // 319 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13521, 22), 351}, // 320 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8617, 8), 138}, // 321 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8748, 15), 152}, // 322 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14466, 20), 379}, // 323 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13673, 15), 355}, // 324 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13688, 14), 355}, // 325 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15264, 25), 405}, // 326 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8610, 7), 133}, // 327 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13743, 26), 357}, // 328 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5619, 15), 273}, // 329 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5605, 14), 271}, // 330 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9107, 7), 173}, // 331 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6627, 10), 226}, // 332 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5707, 14), 188}, // 333 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9362, 13), 190}, // 334 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14486, 11), 380}, // 335 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14497, 13), 380}, // 336 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7101, 20), 269}, // 337 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5441, 31), 295}, // 338 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12010, 34), 295}, // 339 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9375, 12), 191}, // 340 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8721, 6), 149}, // 341 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10678, 15), 258}, // 342 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12471, 18), 312}, // 343 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8595, 15), 370}, // 344 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10387, 12), 240}, // 345 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12404, 25), 308}, // 346 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10364, 23), 239}, // 347 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8542, 41), 322}, // 348 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8114, 33), 311}, // 349 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5591, 14), 243}, // 350 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8296, 39), 314}, // 351 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8000, 23), 292}, // 352 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5578, 13), 291}, // 353 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12331, 29), 305}, // 354 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8053, 27), 290}, // 355 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12107, 25), 297}, // 356 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8257, 28), 313}, // 357 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10418, 32), 242}, // 358 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15308, 20), 407}, // 359 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14339, 24), 372}, // 360 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12831, 21), 328}, // 361 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10332, 16), 237}, // 362 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10348, 16), 238}, // 363 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14015, 30), 362}, // 364 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14045, 29), 362}, // 365 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11065, 23), 279}, // 366 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11088, 26), 279}, // 367 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10801, 29), 264}, // 368 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10167, 28), 230}, // 369 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5837, 18), 166}, // 370 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9114, 10), 174}, // 371 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9132, 14), 177}, // 372 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9016, 17), 176}, // 373 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8901, 33), 160}, // 374 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11428, 36), 284}, // 375 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11464, 39), 284}, // 376 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9043, 12), 168}, // 377 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5481, 7), 134}, // 378 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12489, 23), 315}, // 379 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10663, 15), 256}, // 380 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6950, 18), 257}, // 381 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12360, 26), 306}, // 382 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5653, 12), 200}, // 383 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11028, 17), 278}, // 384 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11045, 20), 278}, // 385 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8368, 19), 301}, // 386 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7040, 19), 268}, // 387 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5721, 20), 201}, // 388 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5665, 28), 266}, // 389 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10858, 27), 266}, // 390 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6999, 21), 267}, // 391 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10985, 14), 276}, // 392 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10307, 25), 236}, // 393 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9091, 16), 172}, // 394 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15168, 21), 401}, // 395 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14575, 18), 384}, // 396 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6904, 17), 253}, // 397 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9955, 25), 221}, // 398 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10195, 24), 231}, // 399 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8934, 34), 161}, // 400 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11503, 37), 285}, // 401 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11540, 40), 285}, // 402 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8968, 33), 162}, // 403 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11580, 36), 286}, // 404 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11616, 39), 286}, // 405 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9171, 28), 180}, // 406 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8843, 24), 158}, // 407 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9284, 30), 186}, // 408 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9314, 31), 187}, // 409 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10082, 21), 224}, // 410 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10060, 22), 223}, // 411 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10253, 21), 233}, // 412 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9647, 37), 208}, // 413 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9684, 40), 209}, // 414 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11270, 39), 282}, // 415 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11309, 42), 282}, // 416 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11823, 42), 289}, // 417 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11865, 45), 289}, // 418 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10043, 17), 222}, // 419 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9980, 28), 221}, // 420 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14987, 23), 395}, // 421 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15010, 30), 396}, // 422 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15040, 30), 397}, // 423 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13143, 39), 342}, // 424 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13072, 33), 340}, // 425 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13105, 38), 341}, // 426 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6104, 18), 215}, // 427 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12747, 27), 325}, // 428 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14959, 28), 394}, // 429 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9345, 17), 189}, // 430 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12774, 26), 326}, // 431 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12800, 31), 327}, // 432 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15070, 38), 398}, // 433 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12726, 21), 324}, // 434 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9939, 16), 220}, // 435 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14699, 19), 388}, // 436 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14718, 18), 388}, // 437 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12707, 19), 323}, // 438 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15237, 27), 404}, // 439 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9636, 11), 207}, // 440 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15108, 28), 399}, // 441 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5565, 13), 136}, // 442 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8783, 22), 155}, // 443 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10551, 23), 249}, // 444 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10497, 22), 246}, // 445 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10476, 21), 245}, // 446 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10450, 26), 244}, // 447 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9550, 28), 204}, // 448 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9578, 28), 205}, // 449 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9606, 30), 206}, // 450 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6667, 16), 247}, // 451 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9250, 18), 184}, // 452 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13968, 24), 361}, // 453 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13992, 23), 361}, // 454 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10008, 35), 222}, // 455 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10219, 34), 232}, // 456 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8867, 34), 159}, // 457 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11351, 37), 283}, // 458 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11388, 40), 283}, // 459 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9532, 18), 202}, // 460 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11189, 39), 281}, // 461 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11228, 42), 281}, // 462 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11736, 42), 288}, // 463 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11778, 45), 288}, // 464 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13446, 30), 349}, // 465 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10399, 19), 241}, // 466 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15136, 32), 400}, // 467 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13182, 25), 343}, // 468 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10133, 17), 228}, // 469 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10103, 30), 227}, // 470 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8625, 9), 140}, // 471 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13041, 15), 338}, // 472 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13022, 19), 337}, // 473 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11910, 18), 293}, // 474 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11949, 29), 294}, // 475 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11978, 32), 294}, // 476 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11928, 21), 293}, // 477 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9895, 44), 219}, // 478 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9852, 43), 218}, // 479 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9819, 33), 217}, // 480 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15360, 8), 410}, // 481 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15368, 8), 411}, // 482 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15376, 6), 412}, // 483 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15382, 7), 413}, // 484 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15389, 11), 414}, // 485 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15400, 9), 415}, // 486 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15409, 13), 416}, // 487 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15348, 12), 409}, // 488 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15422, 6), 417}, // 489 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15428, 10), 418}, // 490 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15438, 6), 419}, // 491 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15459, 20), 421}, // 492 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15444, 15), 420}, // 493 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15568, 15), 429}, // 494 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15583, 13), 430}, // 495 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15556, 12), 428}, // 496 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15646, 21), 434}, // 497 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15539, 17), 427}, // 498 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15723, 16), 438}, // 499 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15527, 12), 426}, // 500 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15707, 16), 437}, // 501 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15500, 14), 424}, // 502 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15484, 16), 423}, // 503 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15514, 13), 425}, // 504 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15667, 20), 435}, // 505 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15611, 18), 432}, // 506 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15596, 15), 431}, // 507 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15687, 20), 436}, // 508 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15629, 17), 433}, // 509 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15759, 24), 440}, // 510 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15739, 20), 439}, // 511 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15479, 5), 422}, // 512 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(15844, 9), 445}, // 513 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15876, 7), 449}, // 514 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15783, 6), 441}, // 515 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15883, 9), 450}, // 516 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15794, 6), 443}, // 517 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15789, 5), 442}, // 518 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15833, 11), 444}, // 519 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15865, 11), 448}, // 520 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15853, 5), 446}, // 521 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15858, 7), 447}, // 522 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(15928, 11), 454}, // 523 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15892, 10), 451}, // 524 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15915, 13), 453}, // 525 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15902, 13), 452}, // 526 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15939, 10), 455}, // 527 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15949, 10), 456}, // 528 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15959, 10), 457}, // 529 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16127, 12), 468}, // 530 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16139, 12), 469}, // 531 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15982, 14), 459}, // 532 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15996, 14), 460}, // 533 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16010, 14), 461}, // 534 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(15969, 13), 458}, // 535 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16087, 19), 466}, // 536 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16039, 16), 463}, // 537 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16055, 16), 464}, // 538 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16071, 16), 465}, // 539 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16024, 15), 462}, // 540 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16106, 21), 467}, // 541 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(16203, 28), 473}, // 542 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16163, 15), 471}, // 543 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16178, 25), 472}, // 544 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16151, 12), 470}, // 545 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(16239, 27), 475}, // 546 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16266, 27), 476}, // 547 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16293, 27), 477}, // 548 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16320, 32), 478}, // 549 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16231, 8), 474}, // 550 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16352, 26), 479}, // 551 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(16389, 4), 482}, // 552 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16382, 7), 481}, // 553 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16378, 4), 480}, // 554 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(16393, 11), 483}, // 555 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16415, 21), 485}, // 556 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16404, 11), 484}, // 557 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(16447, 14), 487}, // 558 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16461, 21), 488}, // 559 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16436, 11), 486}, // 560 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16482, 18), 489}, // 561 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15360, 8), 491}, // 562 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15368, 8), 492}, // 563 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15376, 6), 493}, // 564 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15382, 7), 494}, // 565 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15389, 11), 495}, // 566 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15400, 9), 496}, // 567 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15409, 13), 497}, // 568 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15348, 12), 490}, // 569 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15422, 6), 498}, // 570 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(15428, 10), 499}, // 571 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(15438, 6), 500}, // 572 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(15568, 15), 508}, // 573 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15583, 13), 509}, // 574 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15556, 12), 507}, // 575 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15646, 21), 513}, // 576 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15539, 17), 506}, // 577 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15527, 12), 505}, // 578 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15707, 16), 516}, // 579 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15500, 14), 503}, // 580 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15484, 16), 502}, // 581 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15514, 13), 504}, // 582 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15667, 20), 514}, // 583 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15611, 18), 511}, // 584 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15596, 15), 510}, // 585 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15687, 20), 515}, // 586 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15629, 17), 512}, // 587 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15479, 5), 501}, // 588 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(15844, 9), 521}, // 589 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15876, 7), 525}, // 590 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15783, 6), 517}, // 591 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15794, 6), 519}, // 592 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15789, 5), 518}, // 593 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15833, 11), 520}, // 594 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15865, 11), 524}, // 595 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15853, 5), 522}, // 596 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15858, 7), 523}, // 597 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(15892, 10), 526}, // 598 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(15915, 13), 528}, // 599 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(15902, 13), 527}, // 600 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(19124, 16), 643}, // 601 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16709, 8), 548}, // 602 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(17827, 15), 601}, // 603 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(17842, 18), 601}, // 604 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17187, 10), 572}, // 605 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(17231, 12), 574}, // 606 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(16560, 12), 535}, // 607 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18674, 15), 631}, // 608 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18689, 14), 631}, // 609 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18387, 16), 624}, // 610 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18403, 15), 624}, // 611 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16864, 8), 561}, // 612 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(17908, 16), 604}, // 613 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17890, 18), 603}, // 614 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16524, 6), 531}, // 615 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(17371, 22), 581}, // 616 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17349, 22), 580}, // 617 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17939, 13), 606}, // 618 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17924, 15), 605}, // 619 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16530, 12), 532}, // 620 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(19284, 21), 648}, // 621 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19305, 20), 648}, // 622 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16641, 8), 540}, // 623 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(18843, 20), 636}, // 624 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18863, 19), 636}, // 625 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16607, 8), 539}, // 626 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(20531, 22), 671}, // 627 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20590, 23), 672}, // 628 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18882, 16), 637}, // 629 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18898, 15), 637}, // 630 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16674, 9), 544}, // 631 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(17992, 13), 609}, // 632 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16735, 9), 551}, // 633 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(16551, 9), 534}, // 634 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(16848, 10), 559}, // 635 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(20478, 17), 670}, // 636 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19707, 28), 658}, // 637 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19735, 27), 658}, // 638 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16726, 9), 550}, // 639 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(18138, 14), 616}, // 640 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(16872, 14), 562}, // 641 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(18913, 29), 638}, // 642 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18942, 28), 638}, // 643 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18496, 17), 627}, // 644 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18513, 16), 627}, // 645 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17393, 18), 582}, // 646 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(17057, 15), 568}, // 647 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(19569, 26), 655}, // 648 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17007, 15), 567}, // 649 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(16663, 5), 542}, // 650 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(18703, 21), 632}, // 651 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18724, 20), 632}, // 652 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16959, 14), 566}, // 653 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(18049, 21), 612}, // 654 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18280, 24), 620}, // 655 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19398, 31), 650}, // 656 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18223, 26), 619}, // 657 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19034, 26), 641}, // 658 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19060, 25), 641}, // 659 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16596, 11), 538}, // 660 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(16585, 11), 537}, // 661 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(18112, 26), 615}, // 662 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17860, 30), 602}, // 663 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18152, 24), 616}, // 664 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18189, 19), 617}, // 665 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20329, 16), 667}, // 666 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19325, 20), 649}, // 667 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19345, 19), 649}, // 668 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20421, 29), 669}, // 669 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20450, 28), 669}, // 670 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16858, 6), 560}, // 671 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(20392, 15), 668}, // 672 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20407, 14), 668}, // 673 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19153, 25), 645}, // 674 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19178, 24), 645}, // 675 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17166, 21), 571}, // 676 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(16690, 10), 546}, // 677 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(19646, 31), 657}, // 678 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19677, 30), 657}, // 679 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19595, 26), 656}, // 680 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19621, 25), 656}, // 681 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17134, 18), 569}, // 682 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(16839, 9), 558}, // 683 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(19825, 34), 660}, // 684 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19859, 33), 660}, // 685 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19892, 31), 661}, // 686 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19923, 30), 661}, // 687 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19953, 29), 662}, // 688 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19982, 28), 662}, // 689 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20113, 30), 664}, // 690 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20143, 29), 664}, // 691 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20044, 35), 663}, // 692 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20079, 34), 663}, // 693 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20172, 33), 665}, // 694 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20205, 32), 665}, // 695 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19085, 20), 642}, // 696 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19105, 19), 642}, // 697 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16572, 13), 536}, // 698 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17197, 14), 573}, // 699 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(17243, 16), 575}, // 700 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(19202, 21), 646}, // 701 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19223, 20), 646}, // 702 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18418, 23), 625}, // 703 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18441, 22), 625}, // 704 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18529, 20), 628}, // 705 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18549, 19), 628}, // 706 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19489, 18), 653}, // 707 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18333, 13), 622}, // 708 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18346, 12), 622}, // 709 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18609, 12), 630}, // 710 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18621, 11), 630}, // 711 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19140, 13), 644}, // 712 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17152, 14), 570}, // 713 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(16649, 14), 541}, // 714 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(17303, 13), 577}, // 715 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(17316, 15), 578}, // 716 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(17443, 20), 584}, // 717 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17411, 32), 583}, // 718 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16756, 12), 553}, // 719 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(17767, 11), 599}, // 720 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(17778, 14), 599}, // 721 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16744, 12), 552}, // 722 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(18358, 15), 623}, // 723 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18373, 14), 623}, // 724 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16886, 7), 563}, // 725 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(17611, 19), 591}, // 726 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17630, 14), 592}, // 727 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16668, 6), 543}, // 728 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(17583, 28), 590}, // 729 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17558, 25), 589}, // 730 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17506, 25), 587}, // 731 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17486, 20), 586}, // 732 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17531, 27), 588}, // 733 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17691, 16), 595}, // 734 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17707, 15), 595}, // 735 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17732, 10), 597}, // 736 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17742, 13), 598}, // 737 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(17755, 12), 598}, // 738 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17722, 10), 596}, // 739 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19243, 21), 647}, // 740 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19264, 20), 647}, // 741 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18970, 15), 639}, // 742 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18985, 14), 639}, // 743 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17966, 26), 608}, // 744 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18304, 15), 621}, // 745 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18319, 14), 621}, // 746 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19762, 32), 659}, // 747 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19794, 31), 659}, // 748 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16500, 17), 529}, // 749 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(16700, 9), 547}, // 750 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(17792, 16), 600}, // 751 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(17808, 19), 600}, // 752 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16542, 9), 533}, // 753 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(17952, 14), 607}, // 754 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16683, 7), 545}, // 755 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(16812, 20), 556}, // 756 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(17259, 44), 576}, // 757 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17663, 28), 594}, // 758 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18005, 17), 610}, // 759 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18568, 21), 629}, // 760 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18589, 20), 629}, // 761 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19429, 25), 651}, // 762 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18463, 17), 626}, // 763 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18480, 16), 626}, // 764 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16517, 7), 530}, // 765 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(20237, 27), 666}, // 766 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20264, 26), 666}, // 767 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18097, 15), 614}, // 768 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18999, 18), 640}, // 769 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19017, 17), 640}, // 770 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19507, 16), 654}, // 771 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19523, 15), 654}, // 772 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16832, 7), 557}, // 773 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(18744, 17), 633}, // 774 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18761, 16), 633}, // 775 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17463, 23), 585}, // 776 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18806, 19), 635}, // 777 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18825, 18), 635}, // 778 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16768, 8), 554}, // 779 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(16802, 10), 555}, // 780 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(18176, 13), 617}, // 781 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(18208, 15), 618}, // 782 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(19454, 35), 652}, // 783 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18070, 27), 613}, // 784 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18022, 27), 611}, // 785 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17644, 19), 593}, // 786 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16717, 9), 549}, // 787 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(17331, 18), 579}, // 788 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18777, 15), 634}, // 789 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(18792, 14), 634}, // 790 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16893, 10), 564}, // 791 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(16903, 10), 565}, // 792 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(20613, 3), 673}, // 793 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20616, 3), 674}, // 794 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20619, 3), 675}, // 795 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20632, 7), 678}, // 796 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20622, 5), 676}, // 797 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20627, 5), 677}, // 798 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20639, 12), 679}, // 799 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(20651, 17), 680}, // 800 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21463, 15), 733}, // 801 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21094, 15), 711}, // 802 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10289, 18), 725}, // 803 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10274, 15), 724}, // 804 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20864, 13), 694}, // 805 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20877, 10), 695}, // 806 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20849, 15), 693}, // 807 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20887, 15), 696}, // 808 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21884, 25), 751}, // 809 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21909, 24), 751}, // 810 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21837, 24), 750}, // 811 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21861, 23), 750}, // 812 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21411, 29), 731}, // 813 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20816, 19), 690}, // 814 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22394, 18), 771}, // 815 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21121, 10), 713}, // 816 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22209, 26), 763}, // 817 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22235, 27), 764}, // 818 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21109, 12), 712}, // 819 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20938, 11), 700}, // 820 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20949, 20), 701}, // 821 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20926, 12), 699}, // 822 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20979, 24), 703}, // 823 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20668, 12), 681}, // 824 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21478, 15), 734}, // 825 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21009, 9), 705}, // 826 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20902, 10), 697}, // 827 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20912, 14), 698}, // 828 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21189, 16), 718}, // 829 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21177, 12), 717}, // 830 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21493, 21), 735}, // 831 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21538, 21), 738}, // 832 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22284, 16), 766}, // 833 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22262, 22), 765}, // 834 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22370, 24), 770}, // 835 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22485, 22), 775}, // 836 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22507, 24), 776}, // 837 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22462, 23), 774}, // 838 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22582, 27), 777}, // 839 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22300, 20), 767}, // 840 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21205, 34), 719}, // 841 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21239, 34), 720}, // 842 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21273, 36), 721}, // 843 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21357, 34), 729}, // 844 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22320, 22), 768}, // 845 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20800, 16), 689}, // 846 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20784, 16), 688}, // 847 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21046, 16), 708}, // 848 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21769, 15), 748}, // 849 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21784, 14), 748}, // 850 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21033, 13), 707}, // 851 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21798, 20), 749}, // 852 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21818, 19), 749}, // 853 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21062, 20), 709}, // 854 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21933, 19), 752}, // 855 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21952, 18), 752}, // 856 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21018, 15), 706}, // 857 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20765, 19), 687}, // 858 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21970, 25), 753}, // 859 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21995, 27), 754}, // 860 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20835, 10), 691}, // 861 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21339, 18), 723}, // 862 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21710, 19), 745}, // 863 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21003, 6), 704}, // 864 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22436, 26), 773}, // 865 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21729, 20), 746}, // 866 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10332, 16), 727}, // 867 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22188, 21), 762}, // 868 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22167, 21), 761}, // 869 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10348, 16), 728}, // 870 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22022, 26), 755}, // 871 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22048, 28), 756}, // 872 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22342, 28), 769}, // 873 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(12489, 23), 759}, // 874 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6968, 16), 737}, // 875 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22076, 31), 757}, // 876 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22107, 33), 758}, // 877 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22140, 27), 760}, // 878 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21749, 20), 747}, // 879 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10307, 25), 726}, // 880 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20680, 13), 682}, // 881 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20693, 22), 683}, // 882 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20715, 21), 684}, // 883 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21514, 24), 736}, // 884 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21663, 25), 743}, // 885 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21587, 26), 740}, // 886 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21688, 22), 744}, // 887 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21613, 23), 741}, // 888 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21440, 23), 732}, // 889 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21636, 27), 742}, // 890 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21559, 28), 739}, // 891 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22412, 24), 772}, // 892 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6075, 13), 714}, // 893 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21309, 30), 722}, // 894 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21131, 22), 715}, // 895 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21153, 24), 716}, // 896 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21391, 20), 730}, // 897 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20969, 10), 702}, // 898 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21082, 12), 710}, // 899 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20750, 15), 686}, // 900 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20736, 14), 685}, // 901 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20845, 4), 692}, // 902 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22747, 10), 789}, // 903 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22757, 9), 789}, // 904 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22808, 12), 792}, // 905 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22820, 11), 792}, // 906 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22766, 14), 790}, // 907 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22780, 13), 790}, // 908 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(15883, 9), 782}, // 909 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22659, 10), 783}, // 910 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5556, 9), 781}, // 911 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22716, 16), 788}, // 912 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22732, 15), 788}, // 913 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5323, 7), 784}, // 914 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22839, 8), 794}, // 915 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22676, 7), 786}, // 916 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22793, 8), 791}, // 917 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22801, 7), 791}, // 918 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22683, 17), 787}, // 919 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22700, 16), 787}, // 920 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22831, 8), 793}, // 921 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22669, 7), 785}, // 922 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22616, 20), 779}, // 923 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22636, 23), 780}, // 924 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22609, 7), 778}, // 925 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(22856, 12), 796}, // 926 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(22847, 9), 795}, // 927 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(22868, 12), 797}, // 928 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(22880, 14), 798}, // 929 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(22894, 14), 799}, // 930 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(22913, 4), 801}, // 931 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(22908, 5), 800}, // 932 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(22951, 14), 808}, // 933 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22965, 23), 808}, // 934 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22988, 13), 809}, // 935 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23001, 18), 809}, // 936 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22935, 11), 806}, // 937 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23019, 15), 810}, // 938 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22946, 5), 807}, // 939 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22931, 4), 805}, // 940 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(15479, 5), 802}, // 941 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22924, 7), 804}, // 942 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(22917, 7), 803}, // 943 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(23034, 4), 811}, // 944 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23046, 4), 814}, // 945 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23042, 4), 813}, // 946 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23038, 4), 812}, // 947 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(23082, 18), 817}, // 948 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23100, 18), 818}, // 949 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23050, 16), 815}, // 950 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23066, 16), 816}, // 951 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(23141, 6), 823}, // 952 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23125, 11), 821}, // 953 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23118, 7), 820}, // 954 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15479, 5), 819}, // 955 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14486, 11), 824}, // 956 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14497, 13), 824}, // 957 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23136, 5), 822}, // 958 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(23157, 6), 827}, // 959 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23168, 8), 829}, // 960 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23176, 10), 830}, // 961 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23194, 12), 832}, // 962 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23186, 8), 831}, // 963 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23206, 21), 833}, // 964 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23227, 20), 833}, // 965 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23152, 5), 826}, // 966 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23163, 5), 828}, // 967 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23147, 5), 825}, // 968 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(23282, 16), 837}, // 969 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23268, 14), 836}, // 970 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23254, 14), 835}, // 971 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23345, 27), 840}, // 972 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23318, 27), 839}, // 973 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23298, 20), 838}, // 974 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23247, 7), 834}, // 975 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(23372, 10), 841}, // 976 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(23382, 10), 842}, // 977 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(23403, 15), 844}, // 978 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(23392, 11), 843}, // 979 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(23418, 5), 846}, // 980 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(23432, 12), 849}, // 981 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(23444, 13), 851}, // 982 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(23427, 5), 848}, // 983 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(23423, 4), 847}, // 984 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(23457, 19), 854}, // 985 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(23476, 22), 854}, // 986 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(23498, 17), 855}, // 987 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(23515, 20), 855}, // 988 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9107, 7), 853}, // 989 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(23535, 16), 856}, // 990 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(23551, 19), 856}, // 991 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(15479, 5), 845}, // 992 None in SPV_OPERAND_TYPE_IMAGE
  {IR(23623, 12), 860}, // 993 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(16886, 7), 850}, // 994 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(23635, 8), 861}, // 995 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(16683, 7), 852}, // 996 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(23601, 11), 858}, // 997 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(23570, 14), 857}, // 998 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(23584, 17), 857}, // 999 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(23612, 11), 859}, // 1000 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(15376, 6), 876}, // 1001 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23804, 10), 875}, // 1002 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23739, 12), 870}, // 1003 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23751, 12), 871}, // 1004 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23728, 11), 869}, // 1005 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23653, 11), 863}, // 1006 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23643, 10), 862}, // 1007 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23713, 15), 868}, // 1008 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23825, 17), 878}, // 1009 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23842, 16), 879}, // 1010 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23975, 16), 886}, // 1011 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23991, 16), 887}, // 1012 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23674, 11), 865}, // 1013 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23814, 11), 877}, // 1014 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23898, 20), 882}, // 1015 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23664, 10), 864}, // 1016 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23699, 14), 867}, // 1017 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23685, 14), 866}, // 1018 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23918, 19), 883}, // 1019 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23937, 19), 884}, // 1020 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23956, 19), 885}, // 1021 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23776, 14), 873}, // 1022 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23790, 14), 874}, // 1023 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23763, 13), 872}, // 1024 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23858, 20), 880}, // 1025 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(23878, 20), 881}, // 1026 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(24009, 2), 889}, // 1027 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24110, 5), 907}, // 1028 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24031, 5), 895}, // 1029 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24026, 5), 894}, // 1030 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24068, 6), 901}, // 1031 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24074, 13), 902}, // 1032 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24036, 10), 896}, // 1033 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24046, 10), 897}, // 1034 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24007, 2), 888}, // 1035 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24014, 3), 891}, // 1036 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24011, 3), 890}, // 1037 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24017, 4), 892}, // 1038 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24021, 5), 893}, // 1039 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24063, 5), 900}, // 1040 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24059, 4), 899}, // 1041 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24056, 3), 898}, // 1042 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24104, 6), 906}, // 1043 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24087, 5), 903}, // 1044 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24098, 6), 905}, // 1045 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24092, 6), 904}, // 1046 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(24115, 28), 908}, // 1047 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24143, 27), 908}, // 1048 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24170, 24), 909}, // 1049 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24194, 23), 909}, // 1050 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(24217, 7), 910}, // 1051 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24224, 11), 911}, // 1052 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24235, 14), 912}, // 1053 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(24249, 12), 914}, // 1054 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(15479, 5), 913}, // 1055 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(24261, 13), 916}, // 1056 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(15479, 5), 915}, // 1057 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(24274, 7), 917}, // 1058 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24281, 7), 918}, // 1059 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24288, 12), 919}, // 1060 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(24314, 12), 921}, // 1061 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24366, 17), 924}, // 1062 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24341, 25), 923}, // 1063 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24326, 15), 922}, // 1064 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24300, 14), 920}, // 1065 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(24506, 22), 937}, // 1066 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24528, 21), 937}, // 1067 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24401, 19), 928}, // 1068 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24420, 17), 929}, // 1069 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24390, 11), 927}, // 1070 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19153, 25), 935}, // 1071 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19178, 24), 935}, // 1072 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24465, 18), 932}, // 1073 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24549, 19), 939}, // 1074 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24568, 18), 939}, // 1075 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24711, 16), 943}, // 1076 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24727, 15), 943}, // 1077 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19202, 21), 936}, // 1078 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19223, 20), 936}, // 1079 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24586, 22), 940}, // 1080 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24608, 21), 940}, // 1081 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24451, 14), 931}, // 1082 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24742, 27), 944}, // 1083 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24769, 26), 944}, // 1084 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24437, 14), 930}, // 1085 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24682, 15), 942}, // 1086 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24697, 14), 942}, // 1087 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(15479, 5), 925}, // 1088 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24493, 13), 934}, // 1089 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24483, 10), 933}, // 1090 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19243, 21), 938}, // 1091 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(19264, 20), 938}, // 1092 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24629, 27), 941}, // 1093 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24656, 26), 941}, // 1094 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(24383, 7), 926}, // 1095 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25078, 27), 958}, // 1096 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25026, 26), 956}, // 1097 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24946, 23), 952}, // 1098 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24900, 23), 950}, // 1099 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24795, 29), 946}, // 1100 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24992, 17), 954}, // 1101 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25105, 27), 959}, // 1102 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25052, 26), 957}, // 1103 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24969, 23), 953}, // 1104 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24923, 23), 951}, // 1105 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24824, 29), 947}, // 1106 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25009, 17), 955}, // 1107 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24853, 21), 948}, // 1108 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(24874, 26), 949}, // 1109 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(15479, 5), 945}, // 1110 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(25265, 20), 967}, // 1111 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25132, 8), 962}, // 1112 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25140, 21), 964}, // 1113 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25161, 24), 964}, // 1114 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25185, 19), 965}, // 1115 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25204, 22), 965}, // 1116 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25285, 17), 968}, // 1117 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25226, 18), 966}, // 1118 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25244, 21), 966}, // 1119 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15479, 5), 960}, // 1120 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(23623, 12), 963}, // 1121 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(16717, 9), 961}, // 1122 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25309, 8), 970}, // 1123 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25317, 7), 971}, // 1124 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25302, 7), 969}, // 1125 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25324, 7), 972}, // 1126 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25331, 10), 972}, // 1127 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(25349, 8), 974}, // 1128 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25365, 15), 976}, // 1129 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25469, 20), 982}, // 1130 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25448, 21), 981}, // 1131 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25489, 12), 983}, // 1132 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25530, 14), 985}, // 1133 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25544, 17), 985}, // 1134 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25561, 12), 986}, // 1135 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25573, 15), 986}, // 1136 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15479, 5), 973}, // 1137 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25501, 13), 984}, // 1138 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25514, 16), 984}, // 1139 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25341, 8), 973}, // 1140 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25357, 8), 975}, // 1141 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25380, 23), 977}, // 1142 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25417, 15), 979}, // 1143 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25403, 14), 978}, // 1144 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(16717, 9), 987}, // 1145 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25432, 16), 980}, // 1146 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(25588, 10), 988}, // 1147 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(25603, 4), 990}, // 1148 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(25616, 8), 992}, // 1149 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(25607, 9), 991}, // 1150 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(25598, 5), 989}, // 1151 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(25624, 25), 993}, // 1152 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(25649, 28), 993}, // 1153 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(25690, 4), 996}, // 1154 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25723, 9), 1000}, // 1155 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25732, 13), 1001}, // 1156 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25703, 8), 998}, // 1157 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25711, 12), 999}, // 1158 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25694, 9), 997}, // 1159 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25677, 4), 994}, // 1160 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(25681, 9), 995}, // 1161 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(15479, 5), 1002}, // 1162 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(25745, 25), 1003}, // 1163 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(25770, 23), 1004}, // 1164 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(25862, 27), 1010}, // 1165 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25889, 28), 1011}, // 1166 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25931, 16), 1013}, // 1167 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25917, 14), 1012}, // 1168 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26001, 30), 1016}, // 1169 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25803, 12), 1007}, // 1170 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(16231, 8), 1005}, // 1171 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25793, 10), 1006}, // 1172 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25988, 13), 1015}, // 1173 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25964, 24), 1014}, // 1174 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25838, 24), 1009}, // 1175 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25947, 17), 1014}, // 1176 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(25815, 23), 1008}, // 1177 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(26072, 37), 1018}, // 1178 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(26031, 41), 1017}, // 1179 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(26187, 42), 1021}, // 1180 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26109, 37), 1019}, // 1181 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26146, 41), 1020}, // 1182 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(26229, 33), 1022}, // 1183 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(26262, 33), 1023}, // 1184 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(26307, 6), 1026}, // 1185 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26295, 12), 1025}, // 1186 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(15479, 5), 1024}, // 1187 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26313, 7), 1027}, // 1188 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26320, 15), 1028}, // 1189 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(26343, 7), 1030}, // 1190 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(26335, 8), 1029}, // 1191 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(26408, 13), 1039}, // 1192 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26450, 4), 1045}, // 1193 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26488, 9), 1050}, // 1194 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26421, 5), 1040}, // 1195 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26550, 5), 1059}, // 1196 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26621, 6), 1069}, // 1197 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26374, 5), 1034}, // 1198 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26528, 5), 1055}, // 1199 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26585, 6), 1064}, // 1200 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26638, 5), 1072}, // 1201 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26632, 6), 1071}, // 1202 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26454, 3), 1046}, // 1203 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26497, 8), 1051}, // 1204 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26555, 4), 1060}, // 1205 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26627, 5), 1070}, // 1206 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26441, 5), 1043}, // 1207 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26469, 10), 1048}, // 1208 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26402, 6), 1038}, // 1209 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26539, 6), 1057}, // 1210 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26608, 7), 1067}, // 1211 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26396, 6), 1037}, // 1212 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26533, 6), 1056}, // 1213 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26601, 7), 1066}, // 1214 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26446, 4), 1044}, // 1215 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26479, 9), 1049}, // 1216 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26545, 5), 1058}, // 1217 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26615, 6), 1068}, // 1218 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26433, 8), 1042}, // 1219 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26591, 10), 1065}, // 1220 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26426, 7), 1041}, // 1221 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26457, 12), 1047}, // 1222 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26366, 8), 1033}, // 1223 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26513, 8), 1053}, // 1224 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26568, 9), 1062}, // 1225 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26358, 8), 1032}, // 1226 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26505, 8), 1052}, // 1227 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26559, 9), 1061}, // 1228 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26379, 6), 1035}, // 1229 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26385, 11), 1036}, // 1230 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26521, 7), 1054}, // 1231 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26577, 8), 1063}, // 1232 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26350, 8), 1031}, // 1233 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(26643, 12), 1073}, // 1234 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26655, 7), 1074}, // 1235 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26681, 11), 1077}, // 1236 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26692, 12), 1078}, // 1237 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26704, 15), 1078}, // 1238 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26719, 14), 1079}, // 1239 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26672, 9), 1076}, // 1240 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26662, 10), 1075}, // 1241 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(26741, 12), 1082}, // 1242 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(26733, 8), 1081}, // 1243 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(15479, 5), 1080}, // 1244 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(26753, 29), 1083}, // 1245 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(15360, 8), 1085}, // 1246 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15368, 8), 1086}, // 1247 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15376, 6), 1087}, // 1248 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15382, 7), 1088}, // 1249 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15389, 11), 1089}, // 1250 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15400, 9), 1090}, // 1251 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15409, 13), 1091}, // 1252 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15348, 12), 1084}, // 1253 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(15422, 6), 1092}, // 1254 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15428, 10), 1093}, // 1255 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15438, 6), 1094}, // 1256 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(15459, 20), 1096}, // 1257 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15444, 15), 1095}, // 1258 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(15568, 15), 1104}, // 1259 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15583, 13), 1105}, // 1260 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15556, 12), 1103}, // 1261 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15646, 21), 1109}, // 1262 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15539, 17), 1102}, // 1263 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15723, 16), 1113}, // 1264 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15527, 12), 1101}, // 1265 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15707, 16), 1112}, // 1266 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15500, 14), 1099}, // 1267 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15484, 16), 1098}, // 1268 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15514, 13), 1100}, // 1269 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15667, 20), 1110}, // 1270 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15611, 18), 1107}, // 1271 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15596, 15), 1106}, // 1272 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15687, 20), 1111}, // 1273 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15629, 17), 1108}, // 1274 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15759, 24), 1115}, // 1275 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15739, 20), 1114}, // 1276 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(26782, 26), 1116}, // 1277 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15479, 5), 1097}, // 1278 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(15844, 9), 1121}, // 1279 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15876, 7), 1125}, // 1280 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15783, 6), 1117}, // 1281 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15883, 9), 1126}, // 1282 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15794, 6), 1119}, // 1283 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15789, 5), 1118}, // 1284 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15833, 11), 1120}, // 1285 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15865, 11), 1124}, // 1286 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15853, 5), 1122}, // 1287 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15858, 7), 1123}, // 1288 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(15928, 11), 1130}, // 1289 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15892, 10), 1127}, // 1290 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15915, 13), 1129}, // 1291 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(15902, 13), 1128}, // 1292 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(26843, 15), 1137}, // 1293 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26808, 5), 1132}, // 1294 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26813, 5), 1133}, // 1295 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26863, 7), 1139}, // 1296 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26838, 5), 1136}, // 1297 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26870, 5), 1140}, // 1298 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26818, 9), 1134}, // 1299 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26827, 11), 1135}, // 1300 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26890, 5), 1144}, // 1301 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26858, 5), 1138}, // 1302 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26880, 6), 1142}, // 1303 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26350, 8), 1131}, // 1304 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26875, 5), 1141}, // 1305 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26886, 4), 1143}, // 1306 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(26977, 14), 1155}, // 1307 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27059, 16), 1161}, // 1308 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27075, 15), 1161}, // 1309 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27373, 17), 1170}, // 1310 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26924, 15), 1150}, // 1311 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27390, 17), 1171}, // 1312 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27407, 16), 1171}, // 1313 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26947, 9), 1152}, // 1314 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26956, 8), 1153}, // 1315 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27164, 16), 1164}, // 1316 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27180, 15), 1164}, // 1317 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27328, 21), 1168}, // 1318 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27423, 15), 1172}, // 1319 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27438, 14), 1172}, // 1320 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26991, 6), 1156}, // 1321 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27090, 24), 1162}, // 1322 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27114, 23), 1162}, // 1323 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27195, 22), 1165}, // 1324 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27217, 21), 1165}, // 1325 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26911, 6), 1146}, // 1326 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27043, 16), 1160}, // 1327 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26917, 7), 1148}, // 1328 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27281, 22), 1167}, // 1329 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27303, 25), 1167}, // 1330 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26939, 8), 1151}, // 1331 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26964, 13), 1154}, // 1332 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27137, 14), 1163}, // 1333 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27151, 13), 1163}, // 1334 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27238, 22), 1166}, // 1335 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27260, 21), 1166}, // 1336 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26997, 14), 1157}, // 1337 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27349, 24), 1169}, // 1338 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27024, 19), 1159}, // 1339 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(27011, 13), 1158}, // 1340 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(16768, 8), 1147}, // 1341 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26895, 16), 1145}, // 1342 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26662, 10), 1149}, // 1343 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24326, 15), 1176}, // 1344 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24300, 14), 1173}, // 1345 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(27470, 15), 1175}, // 1346 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(27452, 18), 1174}, // 1347 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(27496, 11), 1179}, // 1348 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(15479, 5), 1177}, // 1349 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(27485, 11), 1178}, // 1350 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(26295, 12), 1182}, // 1351 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(16726, 9), 1181}, // 1352 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(26313, 7), 1183}, // 1353 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(26320, 15), 1184}, // 1354 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27507, 10), 1180}, // 1355 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27560, 24), 1188}, // 1356 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(27584, 22), 1189}, // 1357 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(27606, 21), 1190}, // 1358 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(27517, 8), 1185}, // 1359 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(27525, 15), 1186}, // 1360 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(27540, 20), 1187}, // 1361 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 148);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(156, 325);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(481, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(489, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(492, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(494, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(513, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(523, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(527, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(542, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(546, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(552, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(555, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(558, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(562, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(570, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(573, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(589, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(598, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(601, 192);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(793, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(801, 102);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(903, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(926, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(928, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(931, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(933, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(944, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(948, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(952, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(959, 10);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(969, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(976, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(980, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1001, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1027, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1047, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1051, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1054, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1056, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1058, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1061, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1066, 30);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1096, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1111, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1123, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1128, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1147, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1148, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1152, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1154, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1162, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1165, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1178, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1180, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1183, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1185, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1190, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1192, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1234, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1242, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1245, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1246, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1254, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1257, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1259, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1279, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1289, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1293, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1307, 37);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1344, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1348, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1351, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1356, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(980, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1111, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1152, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(546, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1096, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1162, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(928, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1356, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(156, 325);
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
  {spv::Op::OpNop, false, false, IR(0, 0), IR(27627, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(27683, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(27689, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(27808, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(27815, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(27831, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(27836, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(27847, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(27854, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(27859, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(27869, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(27929, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(28001, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(28075, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(28118, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(28160, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(28171, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(28180, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(28189, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(28234, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(28244, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(28255, 11), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(28379, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(28389, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(28401, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(28418, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(28428, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(28445, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(28456, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(28498, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(28510, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(28523, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(28533, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(28549, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(28563, 10), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(28573, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(28582, 19), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(28601, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(28614, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(16726, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(28666, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(28762, 16), IR(0, 0), IR(206, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(28778, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(28791, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(28808, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(28826, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(28839, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(28902, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(26947, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(28951, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(28969, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(28981, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(28994, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(29003, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(29061, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(29066, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(29072, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(29083, 16), IR(0, 0), IR(207, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(29099, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(29111, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(29131, 15), IR(0, 0), IR(209, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(29146, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(29158, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(29181, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(29232, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(29241, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(29256, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(29272, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(29331, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(29351, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(29372, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(29392, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(29406, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(29425, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(29442, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(29458, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(29469, 10), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(29479, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(29524, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(29570, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(29593, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(29620, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(29647, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(29674, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(29701, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(29732, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(29763, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(29774, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(29786, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(29802, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(29812, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(26991, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(29823, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(29840, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(29856, 18), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(29874, 15), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(29889, 14), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(29903, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(29920, 18), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(29938, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(29950, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(29962, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(29974, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(29986, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(29995, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(30004, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(30013, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(30027, 14), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(30041, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(30056, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(30071, 14), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(30085, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(30102, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(30119, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(30144, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(30152, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(30160, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(30168, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(30173, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(30178, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(30183, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(30188, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(30193, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(30198, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(30203, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(30208, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(30213, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(30218, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(30223, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(30228, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(30233, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(30238, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(30256, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(30274, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(30292, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(30310, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(30328, 13), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(30341, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(30345, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(30355, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(30366, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(30379, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(30392, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(30396, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(30400, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(30406, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(30412, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(30421, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(30430, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(30441, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(30455, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(30463, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(30473, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(30486, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(30502, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(30512, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(30523, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(30534, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(30541, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(30548, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(30558, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(30571, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(30584, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(30602, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(30620, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(30630, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(30640, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(30655, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(30670, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(30680, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(30692, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(30705, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(30720, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(30733, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(30748, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(30764, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(30782, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(30800, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(30820, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(30841, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(30864, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(30882, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(30903, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(30920, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(30930, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(30941, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(30952, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(30956, 15), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(30971, 17), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(30988, 17), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(31005, 11), IR(0, 0), IR(216, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(31016, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(31025, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(31030, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(31035, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(31042, 9), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(31051, 9), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(31060, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(31071, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(31082, 11), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(31093, 13), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(31106, 11), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(31117, 13), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(31130, 17), IR(0, 0), IR(116, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(31147, 19), IR(0, 0), IR(116, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(31203, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(31218, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(31232, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(31243, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(31255, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(31270, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(31292, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(31318, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(31335, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(31352, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(31363, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(31374, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(31385, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(31396, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(31407, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(31418, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(31428, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(31437, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(31447, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(31481, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(31526, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(31541, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(31547, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(31554, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(31617, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(31624, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(31629, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(31636, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(31648, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(31660, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(31674, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(31687, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(31702, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(31718, 9), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(31727, 9), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(31736, 15), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(31784, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(31794, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(31804, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(31814, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(31824, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(31834, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(31844, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(31854, 10), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(31864, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(31873, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(31883, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(31900, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(31918, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(31941, 24), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(31965, 15), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(31980, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(31996, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(32013, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(32031, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(32049, 28), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(32077, 29), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(32106, 20), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(32126, 21), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(32147, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(32161, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(32175, 30), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(32205, 32), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(32237, 23), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(32260, 40), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(32300, 12), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(32312, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(32325, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(32341, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(32354, 19), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(32373, 26), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(32399, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(32415, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(32428, 29), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(32457, 29), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(32486, 33), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(32519, 33), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(32552, 33), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(32585, 33), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(32618, 37), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(32655, 37), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(32692, 17), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(32709, 18), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(32727, 22), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(32749, 26), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(32775, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(32782, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(32803, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(32819, 16), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(32835, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(32842, 16), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(32858, 20), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(32878, 26), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(32904, 35), IR(0, 0), IR(155, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(32939, 25), IR(0, 0), IR(155, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(32964, 17), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(32981, 23), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(33004, 19), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(33023, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(33039, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(33055, 11), IR(0, 0), IR(1, 0), IR(179, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(33066, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(33087, 19), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(33106, 19), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(33125, 24), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(33149, 25), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(33174, 30), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6343, 22), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(33204, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(33233, 32), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(33265, 30), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(33295, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(33324, 29), IR(0, 0), IR(224, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(9433, 23), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(33353, 26), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(33379, 25), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(33404, 27), IR(0, 0), IR(226, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(33431, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(33451, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(33471, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(33491, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(33511, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(33531, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(33551, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(33571, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(33591, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(33611, 20), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(33631, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(33657, 25), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(33682, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(33708, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(33734, 25), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(33759, 26), IR(0, 0), IR(227, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(33785, 29), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(33814, 24), IR(0, 0), IR(230, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(33838, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(33850, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(33859, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(33871, 8), IR(0, 0), IR(231, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(33879, 23), IR(0, 0), IR(147, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(33902, 23), IR(0, 0), IR(156, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(33925, 25), IR(0, 0), IR(157, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(33950, 14), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(34006, 14), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(34020, 15), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(34035, 19), IR(0, 0), IR(203, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(34054, 17), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(34071, 19), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(9724, 9), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(34090, 14), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(34104, 18), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(34122, 12), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(34134, 13), IR(0, 0), IR(234, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(34147, 20), IR(0, 0), IR(3, 1), IR(189, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(34167, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(34189, 19), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(34208, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(34230, 30), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6104, 18), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(34260, 27), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(34287, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(34312, 33), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(34345, 22), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(34367, 19), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(34386, 7), IR(0, 0), IR(237, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(34393, 15), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(34408, 15), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(34423, 20), IR(0, 0), IR(238, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(14379, 25), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(34443, 26), IR(0, 0), IR(236, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(34469, 26), IR(0, 0), IR(1, 0), IR(190, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(34495, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(34520, 12), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(34532, 19), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(34551, 35), IR(0, 0), IR(240, 2), IR(191, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(34586, 22), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(34608, 16), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(34671, 5), IR(167, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(34684, 5), IR(168, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(34697, 6), IR(169, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(34712, 11), IR(170, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(34737, 11), IR(171, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(34762, 12), IR(172, 1), IR(242, 1), IR(139, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(34789, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(34814, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(34839, 26), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(34919, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(34946, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(34973, 30), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(35003, 34), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(35037, 31), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(35068, 16), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(35084, 22), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(35106, 21), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(35127, 32), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(35159, 31), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(35190, 19), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(35209, 31), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(35240, 24), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(35264, 19), IR(0, 0), IR(245, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(35283, 23), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(35306, 23), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(35329, 17), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(35346, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(35375, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(35404, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(35433, 29), IR(0, 0), IR(248, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(35462, 30), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(35492, 28), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(35520, 20), IR(0, 0), IR(247, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(35540, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(35563, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(35586, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(35609, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(35632, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(35655, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(35678, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(35701, 23), IR(0, 0), IR(219, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(35724, 21), IR(0, 0), IR(249, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(35745, 17), IR(0, 0), IR(249, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(35762, 13), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(35775, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(35800, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(35824, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(35849, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(35878, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(35905, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(35928, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(35947, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(35970, 26), IR(0, 0), IR(164, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(35996, 26), IR(0, 0), IR(164, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(479, 14), IR(36022, 27), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(493, 13), IR(36049, 36), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(506, 7), IR(36085, 28), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(36113, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(36141, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(36169, 33), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(36202, 30), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(493, 13), IR(36232, 26), IR(0, 0), IR(251, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(36258, 39), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(36297, 44), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(36341, 23), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(513, 12), IR(36364, 20), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(493, 13), IR(36384, 21), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(513, 12), IR(36405, 30), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(525, 6), IR(36435, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(36457, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(36482, 26), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(36508, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(36533, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(36555, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(36584, 28), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(36612, 25), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(36637, 34), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(36671, 32), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(36703, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(36732, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(36754, 22), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(36776, 19), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(36795, 17), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(36812, 18), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(531, 3), IR(36830, 29), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(36859, 24), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(36883, 16), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(534, 7), IR(36899, 23), IR(0, 0), IR(253, 1), IR(72, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeCooperativeVectorNV, true, false, IR(89, 3), IR(36922, 24), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(541, 13), IR(36946, 29), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(554, 7), IR(36975, 42), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(37017, 39), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(561, 16), IR(37056, 32), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(37088, 27), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(577, 4), IR(37115, 17), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(37132, 18), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionNV, true, true, IR(160, 3), IR(37150, 27), IR(0, 0), IR(184, 1), IR(75, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(37177, 33), IR(0, 0), IR(50, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(37210, 35), IR(0, 0), IR(257, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(37245, 38), IR(0, 0), IR(257, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(581, 5), IR(37283, 24), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(586, 4), IR(37307, 25), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(37332, 22), IR(173, 1), IR(54, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(37375, 21), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(37396, 15), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(37411, 8), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(513, 12), IR(37419, 14), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(513, 12), IR(37433, 17), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(37450, 50), IR(0, 0), IR(258, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(37500, 29), IR(174, 1), IR(259, 4), IR(193, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(37557, 18), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(37575, 35), IR(175, 1), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(37633, 24), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(590, 5), IR(37657, 24), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(595, 6), IR(37681, 24), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(601, 5), IR(37705, 25), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(37730, 26), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(37756, 26), IR(0, 0), IR(263, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(37782, 28), IR(0, 0), IR(264, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(37810, 26), IR(0, 0), IR(264, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(606, 5), IR(37879, 26), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(611, 7), IR(37980, 30), IR(0, 0), IR(201, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(618, 5), IR(38010, 31), IR(0, 0), IR(201, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(38041, 32), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(38073, 19), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(623, 4), IR(38092, 17), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(38109, 21), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(38130, 27), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(38157, 24), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(38181, 20), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(38201, 28), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(38229, 19), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(38248, 25), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(38273, 22), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(12255, 25), IR(45, 1), IR(270, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(38295, 22), IR(0, 0), IR(270, 1), IR(81, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(38317, 20), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(38337, 27), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(38364, 29), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(38393, 18), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(38411, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(38431, 18), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(38449, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(38469, 25), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(38494, 25), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(38519, 29), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(627, 7), IR(38600, 17), IR(0, 0), IR(190, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(38617, 40), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(38657, 38), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(38695, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(38733, 34), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(38767, 37), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(38804, 29), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(38833, 27), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(38860, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(38887, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(38910, 23), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(38933, 20), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(38953, 22), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(38975, 19), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(12726, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(38994, 25), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(39019, 23), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(39042, 24), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(39066, 23), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(39089, 24), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(39113, 28), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(39141, 29), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(39170, 33), IR(0, 0), IR(274, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(634, 5), IR(39203, 34), IR(0, 0), IR(274, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(39237, 24), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(39261, 25), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(39286, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(39299, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(39312, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(39325, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(39338, 14), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(39352, 14), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(39366, 21), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(39387, 21), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(39408, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(39421, 13), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(39434, 15), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(39449, 15), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(39464, 29), IR(0, 0), IR(199, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(39493, 25), IR(0, 0), IR(199, 1), IR(96, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(39518, 15), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(639, 6), IR(12944, 9), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(39533, 13), IR(0, 0), IR(130, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(39546, 14), IR(0, 0), IR(276, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(39560, 14), IR(0, 0), IR(276, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(39574, 14), IR(0, 0), IR(280, 1), IR(100, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(39588, 10), IR(0, 0), IR(280, 1), IR(100, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(39598, 15), IR(176, 1), IR(1, 0), IR(197, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(39634, 21), IR(177, 1), IR(1, 0), IR(197, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(39682, 14), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(39696, 18), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(39714, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(39737, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(39760, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(39783, 23), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(39806, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(39828, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(39850, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(39896, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(39940, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(39979, 37), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(40016, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(40038, 22), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(40060, 60), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(40120, 53), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(40173, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(40220, 40), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(40260, 51), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(40311, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(40355, 51), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(40406, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(40462, 50), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(40512, 52), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(40564, 49), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(40613, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(40660, 50), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(40710, 51), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(40761, 56), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(40817, 33), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(40850, 52), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(40902, 61), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(40963, 61), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(41024, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(41063, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(41101, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(41140, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(41178, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(41217, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(41255, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(41291, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(41330, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(41373, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(41411, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(41449, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(41487, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(41532, 40), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(41572, 62), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(41634, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(41664, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(41702, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(41738, 33), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(41771, 35), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(41806, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(41845, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(41888, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(41935, 54), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(41989, 34), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(42023, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(42070, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(42115, 55), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(42170, 53), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(42223, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(42279, 54), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(42333, 58), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(42391, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(42447, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(42485, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(42531, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(42575, 49), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(42624, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(42671, 70), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(42741, 68), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(42809, 69), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(42878, 68), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(42946, 66), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(43012, 67), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(43079, 36), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(43115, 48), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(43163, 59), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(43222, 58), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(43280, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(645, 9), IR(43336, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(654, 10), IR(43366, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(43396, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(43435, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(43481, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(43524, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(43571, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(43616, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(43662, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(43718, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(43756, 30), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(43786, 32), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(654, 10), IR(43818, 36), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(664, 13), IR(43854, 42), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(43896, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(43935, 39), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(43974, 44), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(44018, 48), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(44066, 50), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(44116, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(44159, 48), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(44207, 43), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(44250, 31), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(44281, 47), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(44328, 45), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(44373, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(44419, 56), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(44475, 38), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(44513, 35), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(44548, 44), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(44592, 46), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(44638, 41), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(44679, 36), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(44715, 50), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(44765, 48), IR(0, 0), IR(282, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(44813, 35), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(13182, 25), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(44848, 16), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(44864, 19), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(677, 8), IR(44883, 29), IR(178, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(677, 8), IR(44940, 25), IR(179, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(677, 8), IR(44989, 32), IR(180, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(677, 8), IR(45052, 30), IR(181, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(685, 10), IR(45111, 24), IR(182, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(685, 10), IR(45158, 24), IR(183, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(685, 10), IR(45205, 24), IR(184, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(685, 10), IR(45252, 24), IR(185, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(695, 6), IR(45299, 23), IR(186, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(695, 6), IR(45344, 23), IR(187, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(695, 6), IR(45389, 23), IR(188, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(695, 6), IR(45434, 23), IR(189, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(695, 6), IR(45479, 23), IR(190, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(677, 8), IR(45524, 26), IR(191, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(677, 8), IR(45575, 26), IR(192, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(677, 8), IR(45626, 25), IR(193, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(685, 10), IR(45675, 26), IR(194, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(677, 8), IR(45726, 25), IR(195, 1), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(677, 8), IR(45775, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(677, 8), IR(45798, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(677, 8), IR(45822, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(677, 8), IR(45847, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(677, 8), IR(45872, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(677, 8), IR(45895, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(677, 8), IR(45919, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(677, 8), IR(45944, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(677, 8), IR(45969, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(677, 8), IR(45992, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(677, 8), IR(46015, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(677, 8), IR(46041, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(677, 8), IR(46066, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(677, 8), IR(46091, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(677, 8), IR(46115, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(677, 8), IR(46141, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(677, 8), IR(46165, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(677, 8), IR(46191, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(677, 8), IR(46215, 26), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(685, 10), IR(46241, 25), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(685, 10), IR(46266, 23), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(685, 10), IR(46289, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(685, 10), IR(46313, 24), IR(0, 0), IR(287, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(46337, 17), IR(0, 0), IR(288, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(701, 2), IR(46354, 21), IR(0, 0), IR(136, 1), IR(122, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(703, 3), IR(46375, 20), IR(0, 0), IR(136, 1), IR(122, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(46395, 24), IR(0, 0), IR(136, 1), IR(122, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(677, 8), IR(46419, 16), IR(196, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(677, 8), IR(46450, 17), IR(197, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(677, 8), IR(46483, 17), IR(198, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(677, 8), IR(46516, 15), IR(199, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(677, 8), IR(46545, 15), IR(200, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(677, 8), IR(46574, 18), IR(201, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(677, 8), IR(46609, 17), IR(202, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(677, 8), IR(46642, 17), IR(203, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(677, 8), IR(46675, 20), IR(204, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(677, 8), IR(46714, 15), IR(205, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(677, 8), IR(46743, 15), IR(206, 1), IR(189, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(46772, 30), IR(207, 1), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(46831, 30), IR(208, 1), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, true, true, IR(172, 4), IR(46890, 23), IR(209, 1), IR(289, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, true, true, IR(172, 4), IR(46935, 24), IR(210, 1), IR(289, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(14140, 14), IR(61, 1), IR(290, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(46982, 22), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(47004, 23), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(47027, 28), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(47055, 46), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(47101, 37), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(47138, 65), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(47203, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(47243, 41), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(47284, 39), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(47323, 36), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(47359, 46), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(47405, 45), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(47450, 42), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(47492, 32), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(47524, 29), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(47553, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(47593, 40), IR(0, 0), IR(193, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(47633, 14), IR(0, 0), IR(291, 4), IR(146, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(47647, 23), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(706, 1), IR(47670, 25), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(47695, 32), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(47727, 36), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(47763, 33), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(47796, 20), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(47816, 20), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(47836, 26), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(47862, 24), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(14593, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(707, 7), IR(47886, 25), IR(211, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(47935, 24), IR(212, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(47982, 22), IR(213, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(48025, 26), IR(214, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(48076, 23), IR(215, 1), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(48121, 27), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(714, 10), IR(48148, 25), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(714, 10), IR(48173, 34), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(714, 10), IR(48207, 34), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(724, 9), IR(48241, 29), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(714, 10), IR(48270, 26), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(733, 7), IR(15070, 38), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(48358, 21), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(15136, 32), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(48379, 26), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(740, 5), IR(48405, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(745, 2), IR(48432, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(747, 4), IR(48459, 24), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(751, 6), IR(48483, 30), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(757, 3), IR(48550, 30), IR(0, 0), IR(306, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(48580, 27), IR(0, 0), IR(111, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(48607, 13), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(48620, 13), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(48633, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(48652, 18), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(48670, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(48689, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(48708, 18), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(48726, 19), IR(0, 0), IR(307, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(48745, 18), IR(0, 0), IR(308, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(760, 6), IR(48763, 18), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(766, 4), IR(48781, 19), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(48800, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(48826, 28), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(48854, 33), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 873> kInstructionNames{{
  {IR(39286, 13), 554}, // 0 AbsISubINTEL
  {IR(39299, 13), 555}, // 1 AbsUSubINTEL
  {IR(29099, 12), 58}, // 2 AccessChain
  {IR(46354, 21), 740}, // 3 AliasDomainDeclINTEL
  {IR(46375, 20), 741}, // 4 AliasScopeDeclINTEL
  {IR(46395, 24), 742}, // 5 AliasScopeListDeclINTEL
  {IR(30396, 4), 143}, // 6 All
  {IR(35775, 25), 426}, // 7 AllocateNodePayloadsAMDX
  {IR(30392, 4), 142}, // 8 Any
  {IR(46141, 24), 731}, // 9 ArbitraryFloatACosINTEL
  {IR(46165, 26), 732}, // 10 ArbitraryFloatACosPiINTEL
  {IR(46091, 24), 729}, // 11 ArbitraryFloatASinINTEL
  {IR(46115, 26), 730}, // 12 ArbitraryFloatASinPiINTEL
  {IR(46241, 25), 735}, // 13 ArbitraryFloatATan2INTEL
  {IR(46191, 24), 733}, // 14 ArbitraryFloatATanINTEL
  {IR(46215, 26), 734}, // 15 ArbitraryFloatATanPiINTEL
  {IR(45111, 24), 702}, // 16 ArbitraryFloatAddALTERA
  {IR(45135, 23), 702}, // 17 ArbitraryFloatAddINTEL
  {IR(44940, 25), 699}, // 18 ArbitraryFloatCastALTERA
  {IR(44989, 32), 700}, // 19 ArbitraryFloatCastFromIntALTERA
  {IR(45021, 31), 700}, // 20 ArbitraryFloatCastFromIntINTEL
  {IR(44965, 24), 699}, // 21 ArbitraryFloatCastINTEL
  {IR(45052, 30), 701}, // 22 ArbitraryFloatCastToIntALTERA
  {IR(45082, 29), 701}, // 23 ArbitraryFloatCastToIntINTEL
  {IR(45626, 25), 713}, // 24 ArbitraryFloatCbrtALTERA
  {IR(45651, 24), 713}, // 25 ArbitraryFloatCbrtINTEL
  {IR(45992, 23), 725}, // 26 ArbitraryFloatCosINTEL
  {IR(46066, 25), 728}, // 27 ArbitraryFloatCosPiINTEL
  {IR(45252, 24), 705}, // 28 ArbitraryFloatDivALTERA
  {IR(45276, 23), 705}, // 29 ArbitraryFloatDivINTEL
  {IR(45479, 23), 710}, // 30 ArbitraryFloatEQALTERA
  {IR(45502, 22), 710}, // 31 ArbitraryFloatEQINTEL
  {IR(45919, 25), 722}, // 32 ArbitraryFloatExp10INTEL
  {IR(45895, 24), 721}, // 33 ArbitraryFloatExp2INTEL
  {IR(45872, 23), 720}, // 34 ArbitraryFloatExpINTEL
  {IR(45944, 25), 723}, // 35 ArbitraryFloatExpm1INTEL
  {IR(45344, 23), 707}, // 36 ArbitraryFloatGEALTERA
  {IR(45367, 22), 707}, // 37 ArbitraryFloatGEINTEL
  {IR(45299, 23), 706}, // 38 ArbitraryFloatGTALTERA
  {IR(45322, 22), 706}, // 39 ArbitraryFloatGTINTEL
  {IR(45675, 26), 714}, // 40 ArbitraryFloatHypotALTERA
  {IR(45701, 25), 714}, // 41 ArbitraryFloatHypotINTEL
  {IR(45434, 23), 709}, // 42 ArbitraryFloatLEALTERA
  {IR(45457, 22), 709}, // 43 ArbitraryFloatLEINTEL
  {IR(45389, 23), 708}, // 44 ArbitraryFloatLTALTERA
  {IR(45412, 22), 708}, // 45 ArbitraryFloatLTINTEL
  {IR(45822, 25), 718}, // 46 ArbitraryFloatLog10INTEL
  {IR(45847, 25), 719}, // 47 ArbitraryFloatLog1pINTEL
  {IR(45798, 24), 717}, // 48 ArbitraryFloatLog2INTEL
  {IR(45775, 23), 716}, // 49 ArbitraryFloatLogINTEL
  {IR(45205, 24), 704}, // 50 ArbitraryFloatMulALTERA
  {IR(45229, 23), 704}, // 51 ArbitraryFloatMulINTEL
  {IR(46266, 23), 736}, // 52 ArbitraryFloatPowINTEL
  {IR(46313, 24), 738}, // 53 ArbitraryFloatPowNINTEL
  {IR(46289, 24), 737}, // 54 ArbitraryFloatPowRINTEL
  {IR(45575, 26), 712}, // 55 ArbitraryFloatRSqrtALTERA
  {IR(45601, 25), 712}, // 56 ArbitraryFloatRSqrtINTEL
  {IR(45524, 26), 711}, // 57 ArbitraryFloatRecipALTERA
  {IR(45550, 25), 711}, // 58 ArbitraryFloatRecipINTEL
  {IR(46015, 26), 726}, // 59 ArbitraryFloatSinCosINTEL
  {IR(44883, 29), 698}, // 60 ArbitraryFloatSinCosPiALTERA
  {IR(44912, 28), 698}, // 61 ArbitraryFloatSinCosPiINTEL
  {IR(45969, 23), 724}, // 62 ArbitraryFloatSinINTEL
  {IR(46041, 25), 727}, // 63 ArbitraryFloatSinPiINTEL
  {IR(45726, 25), 715}, // 64 ArbitraryFloatSqrtALTERA
  {IR(45751, 24), 715}, // 65 ArbitraryFloatSqrtINTEL
  {IR(45158, 24), 703}, // 66 ArbitraryFloatSubALTERA
  {IR(45182, 23), 703}, // 67 ArbitraryFloatSubINTEL
  {IR(14593, 19), 786}, // 68 ArithmeticFenceEXT
  {IR(29146, 12), 61}, // 69 ArrayLength
  {IR(39533, 13), 570}, // 70 AsmCallINTEL
  {IR(12944, 9), 569}, // 71 AsmINTEL
  {IR(39518, 15), 568}, // 72 AsmTargetINTEL
  {IR(39574, 14), 573}, // 73 AssumeTrueKHR
  {IR(31418, 10), 220}, // 74 AtomicAnd
  {IR(31270, 22), 210}, // 75 AtomicCompareExchange
  {IR(31292, 26), 211}, // 76 AtomicCompareExchangeWeak
  {IR(31255, 15), 209}, // 77 AtomicExchange
  {IR(47633, 14), 776}, // 78 AtomicFAddEXT
  {IR(39560, 14), 572}, // 79 AtomicFMaxEXT
  {IR(39546, 14), 571}, // 80 AtomicFMinEXT
  {IR(32803, 16), 292}, // 81 AtomicFlagClear
  {IR(32782, 21), 291}, // 82 AtomicFlagTestAndSet
  {IR(31352, 11), 214}, // 83 AtomicIAdd
  {IR(31335, 17), 213}, // 84 AtomicIDecrement
  {IR(31318, 17), 212}, // 85 AtomicIIncrement
  {IR(31363, 11), 215}, // 86 AtomicISub
  {IR(31232, 11), 207}, // 87 AtomicLoad
  {IR(31428, 9), 221}, // 88 AtomicOr
  {IR(31396, 11), 218}, // 89 AtomicSMax
  {IR(31374, 11), 216}, // 90 AtomicSMin
  {IR(31243, 12), 208}, // 91 AtomicStore
  {IR(31407, 11), 219}, // 92 AtomicUMax
  {IR(31385, 11), 217}, // 93 AtomicUMin
  {IR(31437, 10), 222}, // 94 AtomicXor
  {IR(37782, 28), 500}, // 95 BeginInvocationInterlockEXT
  {IR(35329, 17), 407}, // 96 BitCastArrayQCOM
  {IR(31016, 9), 191}, // 97 BitCount
  {IR(30956, 15), 187}, // 98 BitFieldInsert
  {IR(30971, 17), 188}, // 99 BitFieldSExtract
  {IR(30988, 17), 189}, // 100 BitFieldUExtract
  {IR(31005, 11), 190}, // 101 BitReverse
  {IR(30144, 8), 114}, // 102 Bitcast
  {IR(30941, 11), 185}, // 103 BitwiseAnd
  {IR(48358, 21), 799}, // 104 BitwiseFunctionINTEL
  {IR(30920, 10), 183}, // 105 BitwiseOr
  {IR(30930, 11), 184}, // 106 BitwiseXor
  {IR(31547, 7), 227}, // 107 Branch
  {IR(31554, 18), 228}, // 108 BranchConditional
  {IR(32415, 13), 277}, // 109 BuildNDRange
  {IR(28160, 11), 15}, // 110 Capability
  {IR(32373, 26), 275}, // 111 CaptureEventProfilingInfo
  {IR(33879, 23), 344}, // 112 ColorAttachmentReadEXT
  {IR(31965, 15), 255}, // 113 CommitReadPipe
  {IR(31980, 16), 256}, // 114 CommitWritePipe
  {IR(29406, 19), 72}, // 115 CompositeConstruct
  {IR(47763, 33), 781}, // 116 CompositeConstructContinuedINTEL
  {IR(35462, 30), 412}, // 117 CompositeConstructCoopMatQCOM
  {IR(35037, 31), 395}, // 118 CompositeConstructReplicateEXT
  {IR(29425, 17), 73}, // 119 CompositeExtract
  {IR(35492, 28), 413}, // 120 CompositeExtractCoopMatQCOM
  {IR(29442, 16), 74}, // 121 CompositeInsert
  {IR(48432, 27), 803}, // 122 ConditionalCapabilityINTEL
  {IR(48580, 27), 807}, // 123 ConditionalCopyObjectINTEL
  {IR(48405, 27), 802}, // 124 ConditionalEntryPointINTEL
  {IR(48379, 26), 801}, // 125 ConditionalExtensionINTEL
  {IR(16726, 9), 39}, // 126 Constant
  {IR(28666, 18), 40}, // 127 ConstantComposite
  {IR(47695, 32), 779}, // 128 ConstantCompositeContinuedINTEL
  {IR(34973, 30), 393}, // 129 ConstantCompositeReplicateEXT
  {IR(28614, 14), 38}, // 130 ConstantFalse
  {IR(39464, 29), 566}, // 131 ConstantFunctionPointerINTEL
  {IR(28778, 13), 42}, // 132 ConstantNull
  {IR(32858, 20), 296}, // 133 ConstantPipeStorage
  {IR(28762, 16), 41}, // 134 ConstantSampler
  {IR(35928, 19), 432}, // 135 ConstantStringAMDX
  {IR(28601, 13), 37}, // 136 ConstantTrue
  {IR(31203, 15), 205}, // 137 ControlBarrier
  {IR(47836, 26), 784}, // 138 ControlBarrierArriveINTEL
  {IR(47862, 24), 785}, // 139 ControlBarrierWaitINTEL
  {IR(47816, 20), 783}, // 140 ConvertBF16ToFINTEL
  {IR(47796, 20), 782}, // 141 ConvertFToBF16INTEL
  {IR(29950, 12), 100}, // 142 ConvertFToS
  {IR(29938, 12), 99}, // 143 ConvertFToU
  {IR(48800, 26), 819}, // 144 ConvertHandleToImageINTEL
  {IR(48854, 33), 821}, // 145 ConvertHandleToSampledImageINTEL
  {IR(48826, 28), 820}, // 146 ConvertHandleToSamplerINTEL
  {IR(38431, 18), 523}, // 147 ConvertImageToUNV
  {IR(30027, 14), 107}, // 148 ConvertPtrToU
  {IR(29962, 12), 101}, // 149 ConvertSToF
  {IR(38494, 25), 526}, // 150 ConvertSampledImageToUNV
  {IR(38449, 20), 524}, // 151 ConvertSamplerToUNV
  {IR(34551, 35), 379}, // 152 ConvertUToAccelerationStructureKHR
  {IR(29974, 12), 102}, // 153 ConvertUToF
  {IR(38393, 18), 521}, // 154 ConvertUToImageNV
  {IR(30071, 14), 110}, // 155 ConvertUToPtr
  {IR(38469, 25), 525}, // 156 ConvertUToSampledImageNV
  {IR(38411, 20), 522}, // 157 ConvertUToSamplerNV
  {IR(37088, 27), 475}, // 158 CooperativeMatrixConvertNV
  {IR(34946, 27), 392}, // 159 CooperativeMatrixLengthKHR
  {IR(37756, 26), 499}, // 160 CooperativeMatrixLengthNV
  {IR(34814, 25), 389}, // 161 CooperativeMatrixLoadKHR
  {IR(37681, 24), 496}, // 162 CooperativeMatrixLoadNV
  {IR(37980, 30), 503}, // 163 CooperativeMatrixLoadTensorNV
  {IR(34919, 27), 391}, // 164 CooperativeMatrixMulAddKHR
  {IR(37730, 26), 498}, // 165 CooperativeMatrixMulAddNV
  {IR(38041, 32), 505}, // 166 CooperativeMatrixPerElementOpNV
  {IR(37879, 26), 502}, // 167 CooperativeMatrixReduceNV
  {IR(34839, 26), 390}, // 168 CooperativeMatrixStoreKHR
  {IR(37705, 25), 497}, // 169 CooperativeMatrixStoreNV
  {IR(38010, 31), 504}, // 170 CooperativeMatrixStoreTensorNV
  {IR(38364, 29), 520}, // 171 CooperativeMatrixTransposeNV
  {IR(37283, 24), 482}, // 172 CooperativeVectorLoadNV
  {IR(37056, 32), 474}, // 173 CooperativeVectorMatrixMulAddNV
  {IR(36946, 29), 471}, // 174 CooperativeVectorMatrixMulNV
  {IR(36975, 42), 472}, // 175 CooperativeVectorOuterProductAccumulateNV
  {IR(37017, 39), 473}, // 176 CooperativeVectorReduceSumAccumulateNV
  {IR(37307, 25), 483}, // 177 CooperativeVectorStoreNV
  {IR(33838, 12), 340}, // 178 CopyLogical
  {IR(29072, 11), 56}, // 179 CopyMemory
  {IR(29083, 16), 57}, // 180 CopyMemorySized
  {IR(29458, 11), 75}, // 181 CopyObject
  {IR(32878, 26), 297}, // 182 CreatePipeFromPipeStorage
  {IR(38109, 21), 508}, // 183 CreateTensorLayoutNV
  {IR(38229, 19), 513}, // 184 CreateTensorViewNV
  {IR(32325, 16), 272}, // 185 CreateUserEvent
  {IR(46831, 30), 755}, // 186 CrossWorkgroupCastToPtrALTERA
  {IR(46861, 29), 755}, // 187 CrossWorkgroupCastToPtrINTEL
  {IR(31025, 5), 192}, // 188 DPdx
  {IR(31071, 11), 198}, // 189 DPdxCoarse
  {IR(31042, 9), 195}, // 190 DPdxFine
  {IR(31030, 5), 193}, // 191 DPdy
  {IR(31082, 11), 199}, // 192 DPdyCoarse
  {IR(31051, 9), 196}, // 193 DPdyFine
  {IR(29232, 9), 64}, // 194 Decorate
  {IR(33055, 11), 305}, // 195 DecorateId
  {IR(39598, 15), 575}, // 196 DecorateString
  {IR(39613, 21), 575}, // 197 DecorateStringGOOGLE
  {IR(29256, 16), 66}, // 198 DecorationGroup
  {IR(12255, 25), 516}, // 199 DemoteToHelperInvocation
  {IR(12280, 28), 516}, // 200 DemoteToHelperInvocationEXT
  {IR(33902, 23), 345}, // 201 DepthAttachmentReadEXT
  {IR(30341, 4), 137}, // 202 Dot
  {IR(37115, 17), 476}, // 203 EmitMeshTasksEXT
  {IR(31130, 17), 203}, // 204 EmitStreamVertex
  {IR(31106, 11), 201}, // 205 EmitVertex
  {IR(37810, 26), 501}, // 206 EndInvocationInterlockEXT
  {IR(31117, 13), 202}, // 207 EndPrimitive
  {IR(31147, 19), 204}, // 208 EndStreamPrimitive
  {IR(32161, 14), 265}, // 209 EnqueueKernel
  {IR(32147, 14), 264}, // 210 EnqueueMarker
  {IR(35800, 24), 427}, // 211 EnqueueNodePayloadsAMDX
  {IR(28075, 11), 13}, // 212 EntryPoint
  {IR(34532, 19), 378}, // 213 ExecuteCallableKHR
  {IR(37557, 18), 492}, // 214 ExecuteCallableNV
  {IR(28118, 14), 14}, // 215 ExecutionMode
  {IR(33039, 16), 304}, // 216 ExecutionModeId
  {IR(39588, 10), 574}, // 217 ExpectKHR
  {IR(27929, 8), 11}, // 218 ExtInst
  {IR(27869, 14), 10}, // 219 ExtInstImport
  {IR(34469, 26), 375}, // 220 ExtInstWithForwardRefsKHR
  {IR(27859, 10), 9}, // 221 Extension
  {IR(35520, 20), 414}, // 222 ExtractSubArrayQCOM
  {IR(30173, 5), 118}, // 223 FAdd
  {IR(30004, 9), 105}, // 224 FConvert
  {IR(30208, 5), 125}, // 225 FDiv
  {IR(30233, 5), 130}, // 226 FMod
  {IR(30193, 5), 122}, // 227 FMul
  {IR(30160, 8), 116}, // 228 FNegate
  {IR(30670, 10), 168}, // 229 FOrdEqual
  {IR(30748, 16), 174}, // 230 FOrdGreaterThan
  {IR(30820, 21), 178}, // 231 FOrdGreaterThanEqual
  {IR(30720, 13), 172}, // 232 FOrdLessThan
  {IR(30782, 18), 176}, // 233 FOrdLessThanEqual
  {IR(30692, 13), 170}, // 234 FOrdNotEqual
  {IR(14140, 14), 758}, // 235 FPGARegALTERA
  {IR(14154, 13), 758}, // 236 FPGARegINTEL
  {IR(30228, 5), 129}, // 237 FRem
  {IR(30183, 5), 120}, // 238 FSub
  {IR(30680, 12), 169}, // 239 FUnordEqual
  {IR(30764, 18), 175}, // 240 FUnordGreaterThan
  {IR(30841, 23), 179}, // 241 FUnordGreaterThanEqual
  {IR(30733, 15), 173}, // 242 FUnordLessThan
  {IR(30800, 20), 177}, // 243 FUnordLessThanEqual
  {IR(30705, 15), 171}, // 244 FUnordNotEqual
  {IR(37245, 38), 481}, // 245 FetchMicroTriangleVertexBarycentricNV
  {IR(37210, 35), 480}, // 246 FetchMicroTriangleVertexPositionNV
  {IR(35849, 29), 429}, // 247 FinishWritingNodePayloadAMDX
  {IR(46545, 15), 747}, // 248 FixedCosALTERA
  {IR(46560, 14), 747}, // 249 FixedCosINTEL
  {IR(46642, 17), 750}, // 250 FixedCosPiALTERA
  {IR(46659, 16), 750}, // 251 FixedCosPiINTEL
  {IR(46743, 15), 753}, // 252 FixedExpALTERA
  {IR(46758, 14), 753}, // 253 FixedExpINTEL
  {IR(46714, 15), 752}, // 254 FixedLogALTERA
  {IR(46729, 14), 752}, // 255 FixedLogINTEL
  {IR(46450, 17), 744}, // 256 FixedRecipALTERA
  {IR(46467, 16), 744}, // 257 FixedRecipINTEL
  {IR(46483, 17), 745}, // 258 FixedRsqrtALTERA
  {IR(46500, 16), 745}, // 259 FixedRsqrtINTEL
  {IR(46516, 15), 746}, // 260 FixedSinALTERA
  {IR(46574, 18), 748}, // 261 FixedSinCosALTERA
  {IR(46592, 17), 748}, // 262 FixedSinCosINTEL
  {IR(46675, 20), 751}, // 263 FixedSinCosPiALTERA
  {IR(46695, 19), 751}, // 264 FixedSinCosPiINTEL
  {IR(46531, 14), 746}, // 265 FixedSinINTEL
  {IR(46609, 17), 749}, // 266 FixedSinPiALTERA
  {IR(46626, 16), 749}, // 267 FixedSinPiINTEL
  {IR(46419, 16), 743}, // 268 FixedSqrtALTERA
  {IR(46435, 15), 743}, // 269 FixedSqrtINTEL
  {IR(34386, 7), 369}, // 270 FmaKHR
  {IR(35745, 17), 424}, // 271 FragmentFetchAMD
  {IR(35724, 21), 423}, // 272 FragmentMaskFetchAMD
  {IR(26947, 9), 48}, // 273 Function
  {IR(28981, 13), 51}, // 274 FunctionCall
  {IR(28969, 12), 50}, // 275 FunctionEnd
  {IR(28951, 18), 49}, // 276 FunctionParameter
  {IR(39493, 25), 567}, // 277 FunctionPointerCallINTEL
  {IR(31035, 7), 194}, // 278 Fwidth
  {IR(31093, 13), 200}, // 279 FwidthCoarse
  {IR(31060, 11), 197}, // 280 FwidthFine
  {IR(30102, 17), 112}, // 281 GenericCastToPtr
  {IR(30119, 25), 113}, // 282 GenericCastToPtrExplicit
  {IR(29158, 23), 62}, // 283 GenericPtrMemSemantics
  {IR(32399, 16), 276}, // 284 GetDefaultQueue
  {IR(32904, 35), 298}, // 285 GetKernelLocalSizeForSubgroupCount
  {IR(32939, 25), 299}, // 286 GetKernelMaxNumSubgroups
  {IR(32205, 32), 267}, // 287 GetKernelNDrangeMaxSubGroupSize
  {IR(32175, 30), 266}, // 288 GetKernelNDrangeSubGroupCount
  {IR(32260, 40), 269}, // 289 GetKernelPreferredWorkGroupSizeMultiple
  {IR(32237, 23), 268}, // 290 GetKernelWorkGroupSize
  {IR(32031, 18), 259}, // 291 GetMaxPipePackets
  {IR(32013, 18), 258}, // 292 GetNumPipePackets
  {IR(9724, 9), 353}, // 293 GraphARM
  {IR(34054, 17), 351}, // 294 GraphConstantARM
  {IR(34122, 12), 356}, // 295 GraphEndARM
  {IR(34071, 19), 352}, // 296 GraphEntryPointARM
  {IR(34090, 14), 354}, // 297 GraphInputARM
  {IR(34104, 18), 355}, // 298 GraphSetOutputARM
  {IR(31718, 9), 238}, // 299 GroupAll
  {IR(31727, 9), 239}, // 300 GroupAny
  {IR(31687, 15), 236}, // 301 GroupAsyncCopy
  {IR(48633, 19), 810}, // 302 GroupBitwiseAndKHR
  {IR(48652, 18), 811}, // 303 GroupBitwiseOrKHR
  {IR(48670, 19), 812}, // 304 GroupBitwiseXorKHR
  {IR(31736, 15), 240}, // 305 GroupBroadcast
  {IR(32106, 20), 262}, // 306 GroupCommitReadPipe
  {IR(32126, 21), 263}, // 307 GroupCommitWritePipe
  {IR(29272, 14), 67}, // 308 GroupDecorate
  {IR(31794, 10), 242}, // 309 GroupFAdd
  {IR(35563, 23), 416}, // 310 GroupFAddNonUniformAMD
  {IR(31834, 10), 246}, // 311 GroupFMax
  {IR(35655, 23), 420}, // 312 GroupFMaxNonUniformAMD
  {IR(31804, 10), 243}, // 313 GroupFMin
  {IR(35586, 23), 417}, // 314 GroupFMinNonUniformAMD
  {IR(48620, 13), 809}, // 315 GroupFMulKHR
  {IR(31784, 10), 241}, // 316 GroupIAdd
  {IR(35540, 23), 415}, // 317 GroupIAddNonUniformAMD
  {IR(48607, 13), 808}, // 318 GroupIMulKHR
  {IR(48689, 19), 813}, // 319 GroupLogicalAndKHR
  {IR(48708, 18), 814}, // 320 GroupLogicalOrKHR
  {IR(48726, 19), 815}, // 321 GroupLogicalXorKHR
  {IR(29331, 20), 68}, // 322 GroupMemberDecorate
  {IR(33087, 19), 307}, // 323 GroupNonUniformAll
  {IR(33125, 24), 309}, // 324 GroupNonUniformAllEqual
  {IR(33106, 19), 308}, // 325 GroupNonUniformAny
  {IR(6343, 22), 312}, // 326 GroupNonUniformBallot
  {IR(33265, 30), 315}, // 327 GroupNonUniformBallotBitCount
  {IR(33233, 32), 314}, // 328 GroupNonUniformBallotBitExtract
  {IR(33295, 29), 316}, // 329 GroupNonUniformBallotFindLSB
  {IR(33324, 29), 317}, // 330 GroupNonUniformBallotFindMSB
  {IR(33631, 26), 332}, // 331 GroupNonUniformBitwiseAnd
  {IR(33657, 25), 333}, // 332 GroupNonUniformBitwiseOr
  {IR(33682, 26), 334}, // 333 GroupNonUniformBitwiseXor
  {IR(33149, 25), 310}, // 334 GroupNonUniformBroadcast
  {IR(33174, 30), 311}, // 335 GroupNonUniformBroadcastFirst
  {IR(33066, 21), 306}, // 336 GroupNonUniformElect
  {IR(33451, 20), 323}, // 337 GroupNonUniformFAdd
  {IR(33611, 20), 331}, // 338 GroupNonUniformFMax
  {IR(33551, 20), 328}, // 339 GroupNonUniformFMin
  {IR(33491, 20), 325}, // 340 GroupNonUniformFMul
  {IR(33431, 20), 322}, // 341 GroupNonUniformIAdd
  {IR(33471, 20), 324}, // 342 GroupNonUniformIMul
  {IR(33204, 29), 313}, // 343 GroupNonUniformInverseBallot
  {IR(33708, 26), 335}, // 344 GroupNonUniformLogicalAnd
  {IR(33734, 25), 336}, // 345 GroupNonUniformLogicalOr
  {IR(33759, 26), 337}, // 346 GroupNonUniformLogicalXor
  {IR(37150, 27), 478}, // 347 GroupNonUniformPartitionNV
  {IR(35970, 26), 434}, // 348 GroupNonUniformQuadAllKHR
  {IR(35996, 26), 435}, // 349 GroupNonUniformQuadAnyKHR
  {IR(33785, 29), 338}, // 350 GroupNonUniformQuadBroadcast
  {IR(33814, 24), 339}, // 351 GroupNonUniformQuadSwap
  {IR(14379, 25), 373}, // 352 GroupNonUniformRotateKHR
  {IR(33571, 20), 329}, // 353 GroupNonUniformSMax
  {IR(33511, 20), 326}, // 354 GroupNonUniformSMin
  {IR(9433, 23), 318}, // 355 GroupNonUniformShuffle
  {IR(33404, 27), 321}, // 356 GroupNonUniformShuffleDown
  {IR(33379, 25), 320}, // 357 GroupNonUniformShuffleUp
  {IR(33353, 26), 319}, // 358 GroupNonUniformShuffleXor
  {IR(33591, 20), 330}, // 359 GroupNonUniformUMax
  {IR(33531, 20), 327}, // 360 GroupNonUniformUMin
  {IR(32049, 28), 260}, // 361 GroupReserveReadPipePackets
  {IR(32077, 29), 261}, // 362 GroupReserveWritePipePackets
  {IR(31854, 10), 248}, // 363 GroupSMax
  {IR(35701, 23), 422}, // 364 GroupSMaxNonUniformAMD
  {IR(31824, 10), 245}, // 365 GroupSMin
  {IR(35632, 23), 419}, // 366 GroupSMinNonUniformAMD
  {IR(31844, 10), 247}, // 367 GroupUMax
  {IR(35678, 23), 421}, // 368 GroupUMaxNonUniformAMD
  {IR(31814, 10), 244}, // 369 GroupUMin
  {IR(35609, 23), 418}, // 370 GroupUMinNonUniformAMD
  {IR(31702, 16), 237}, // 371 GroupWaitEvents
  {IR(36457, 25), 451}, // 372 HitObjectExecuteShaderNV
  {IR(36508, 25), 453}, // 373 HitObjectGetAttributesNV
  {IR(37633, 24), 494}, // 374 HitObjectGetClusterIdNV
  {IR(36482, 26), 452}, // 375 HitObjectGetCurrentTimeNV
  {IR(36584, 28), 456}, // 376 HitObjectGetGeometryIndexNV
  {IR(36533, 22), 454}, // 377 HitObjectGetHitKindNV
  {IR(36637, 34), 458}, // 378 HitObjectGetInstanceCustomIndexNV
  {IR(36612, 25), 457}, // 379 HitObjectGetInstanceIdNV
  {IR(38860, 27), 536}, // 380 HitObjectGetLSSPositionsNV
  {IR(38887, 23), 537}, // 381 HitObjectGetLSSRadiiNV
  {IR(36169, 33), 441}, // 382 HitObjectGetObjectRayDirectionNV
  {IR(36202, 30), 442}, // 383 HitObjectGetObjectRayOriginNV
  {IR(36141, 28), 440}, // 384 HitObjectGetObjectToWorldNV
  {IR(36555, 29), 455}, // 385 HitObjectGetPrimitiveIndexNV
  {IR(36732, 22), 461}, // 386 HitObjectGetRayTMaxNV
  {IR(36754, 22), 462}, // 387 HitObjectGetRayTMinNV
  {IR(36297, 44), 445}, // 388 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(36258, 39), 444}, // 389 HitObjectGetShaderRecordBufferHandleNV
  {IR(38804, 29), 534}, // 390 HitObjectGetSpherePositionNV
  {IR(38833, 27), 535}, // 391 HitObjectGetSphereRadiusNV
  {IR(36671, 32), 459}, // 392 HitObjectGetWorldRayDirectionNV
  {IR(36703, 29), 460}, // 393 HitObjectGetWorldRayOriginNV
  {IR(36113, 28), 439}, // 394 HitObjectGetWorldToObjectNV
  {IR(36776, 19), 463}, // 395 HitObjectIsEmptyNV
  {IR(36795, 17), 464}, // 396 HitObjectIsHitNV
  {IR(38933, 20), 539}, // 397 HitObjectIsLSSHitNV
  {IR(36812, 18), 465}, // 398 HitObjectIsMissNV
  {IR(38910, 23), 538}, // 399 HitObjectIsSphereHitNV
  {IR(36341, 23), 446}, // 400 HitObjectRecordEmptyNV
  {IR(36022, 27), 436}, // 401 HitObjectRecordHitMotionNV
  {IR(36384, 21), 448}, // 402 HitObjectRecordHitNV
  {IR(36049, 36), 437}, // 403 HitObjectRecordHitWithIndexMotionNV
  {IR(36405, 30), 449}, // 404 HitObjectRecordHitWithIndexNV
  {IR(36085, 28), 438}, // 405 HitObjectRecordMissMotionNV
  {IR(36435, 22), 450}, // 406 HitObjectRecordMissNV
  {IR(36232, 26), 443}, // 407 HitObjectTraceRayMotionNV
  {IR(36364, 20), 447}, // 408 HitObjectTraceRayNV
  {IR(30168, 5), 117}, // 409 IAdd
  {IR(30345, 10), 138}, // 410 IAddCarry
  {IR(39312, 13), 556}, // 411 IAddSatINTEL
  {IR(39338, 14), 558}, // 412 IAverageINTEL
  {IR(39366, 21), 560}, // 413 IAverageRoundedINTEL
  {IR(30541, 7), 158}, // 414 IEqual
  {IR(30188, 5), 121}, // 415 IMul
  {IR(39434, 15), 564}, // 416 IMul32x16INTEL
  {IR(30548, 10), 159}, // 417 INotEqual
  {IR(30178, 5), 119}, // 418 ISub
  {IR(30355, 11), 139}, // 419 ISubBorrow
  {IR(39408, 13), 562}, // 420 ISubSatINTEL
  {IR(34586, 22), 380}, // 421 IgnoreIntersectionKHR
  {IR(37375, 21), 485}, // 422 IgnoreIntersectionNV
  {IR(26991, 6), 91}, // 423 Image
  {IR(35433, 29), 411}, // 424 ImageBlockMatchGatherSADQCOM
  {IR(35404, 29), 410}, // 425 ImageBlockMatchGatherSSDQCOM
  {IR(35306, 23), 406}, // 426 ImageBlockMatchSADQCOM
  {IR(35283, 23), 405}, // 427 ImageBlockMatchSSDQCOM
  {IR(35375, 29), 409}, // 428 ImageBlockMatchWindowSADQCOM
  {IR(35346, 29), 408}, // 429 ImageBlockMatchWindowSSDQCOM
  {IR(35264, 19), 404}, // 430 ImageBoxFilterQCOM
  {IR(29786, 16), 88}, // 431 ImageDrefGather
  {IR(29763, 11), 86}, // 432 ImageFetch
  {IR(29774, 12), 87}, // 433 ImageGather
  {IR(29823, 17), 92}, // 434 ImageQueryFormat
  {IR(29903, 17), 97}, // 435 ImageQueryLevels
  {IR(29889, 14), 96}, // 436 ImageQueryLod
  {IR(29840, 16), 93}, // 437 ImageQueryOrder
  {IR(29920, 18), 98}, // 438 ImageQuerySamples
  {IR(29874, 15), 95}, // 439 ImageQuerySize
  {IR(29856, 18), 94}, // 440 ImageQuerySizeLod
  {IR(29802, 10), 89}, // 441 ImageRead
  {IR(29620, 27), 81}, // 442 ImageSampleDrefExplicitLod
  {IR(29593, 27), 80}, // 443 ImageSampleDrefImplicitLod
  {IR(29570, 23), 79}, // 444 ImageSampleExplicitLod
  {IR(36899, 23), 469}, // 445 ImageSampleFootprintNV
  {IR(29524, 23), 78}, // 446 ImageSampleImplicitLod
  {IR(29732, 31), 85}, // 447 ImageSampleProjDrefExplicitLod
  {IR(29701, 31), 84}, // 448 ImageSampleProjDrefImplicitLod
  {IR(29674, 27), 83}, // 449 ImageSampleProjExplicitLod
  {IR(29647, 27), 82}, // 450 ImageSampleProjImplicitLod
  {IR(35240, 24), 403}, // 451 ImageSampleWeightedQCOM
  {IR(32727, 22), 288}, // 452 ImageSparseDrefGather
  {IR(32692, 17), 286}, // 453 ImageSparseFetch
  {IR(32709, 18), 287}, // 454 ImageSparseGather
  {IR(32819, 16), 293}, // 455 ImageSparseRead
  {IR(32519, 33), 281}, // 456 ImageSparseSampleDrefExplicitLod
  {IR(32486, 33), 280}, // 457 ImageSparseSampleDrefImplicitLod
  {IR(32457, 29), 279}, // 458 ImageSparseSampleExplicitLod
  {IR(32428, 29), 278}, // 459 ImageSparseSampleImplicitLod
  {IR(32655, 37), 285}, // 460 ImageSparseSampleProjDrefExplicitLod
  {IR(32618, 37), 284}, // 461 ImageSparseSampleProjDrefImplicitLod
  {IR(32585, 33), 283}, // 462 ImageSparseSampleProjExplicitLod
  {IR(32552, 33), 282}, // 463 ImageSparseSampleProjImplicitLod
  {IR(32749, 26), 289}, // 464 ImageSparseTexelsResident
  {IR(29003, 18), 53}, // 465 ImageTexelPointer
  {IR(29812, 11), 90}, // 466 ImageWrite
  {IR(29111, 20), 59}, // 467 InBoundsAccessChain
  {IR(29181, 23), 63}, // 468 InBoundsPtrAccessChain
  {IR(30412, 9), 146}, // 469 IsFinite
  {IR(38295, 22), 517}, // 470 IsHelperInvocationEXT
  {IR(30406, 6), 145}, // 471 IsInf
  {IR(30400, 6), 144}, // 472 IsNan
  {IR(35905, 23), 431}, // 473 IsNodePayloadValidAMDX
  {IR(30421, 9), 147}, // 474 IsNormal
  {IR(32341, 13), 273}, // 475 IsValidEvent
  {IR(31996, 17), 257}, // 476 IsValidReserveId
  {IR(31624, 5), 230}, // 477 Kill
  {IR(31541, 6), 226}, // 478 Label
  {IR(30441, 14), 149}, // 479 LessOrGreater
  {IR(31660, 14), 234}, // 480 LifetimeStart
  {IR(31674, 13), 235}, // 481 LifetimeStop
  {IR(27854, 5), 8}, // 482 Line
  {IR(29061, 5), 54}, // 483 Load
  {IR(30512, 11), 155}, // 484 LogicalAnd
  {IR(30473, 13), 152}, // 485 LogicalEqual
  {IR(30523, 11), 156}, // 486 LogicalNot
  {IR(30486, 16), 153}, // 487 LogicalNotEqual
  {IR(30502, 10), 154}, // 488 LogicalOr
  {IR(46337, 17), 739}, // 489 LoopControlINTEL
  {IR(31481, 10), 224}, // 490 LoopMerge
  {IR(48763, 18), 817}, // 491 MaskedGatherINTEL
  {IR(48781, 19), 818}, // 492 MaskedScatterINTEL
  {IR(30310, 18), 135}, // 493 MatrixTimesMatrix
  {IR(30256, 18), 132}, // 494 MatrixTimesScalar
  {IR(30292, 18), 134}, // 495 MatrixTimesVector
  {IR(29241, 15), 65}, // 496 MemberDecorate
  {IR(39634, 21), 576}, // 497 MemberDecorateString
  {IR(39655, 27), 576}, // 498 MemberDecorateStringGOOGLE
  {IR(27836, 11), 6}, // 499 MemberName
  {IR(31218, 14), 206}, // 500 MemoryBarrier
  {IR(28001, 12), 12}, // 501 MemoryModel
  {IR(33004, 19), 302}, // 502 MemoryNamedBarrier
  {IR(33023, 16), 303}, // 503 ModuleProcessed
  {IR(27831, 5), 5}, // 504 Name
  {IR(32981, 23), 301}, // 505 NamedBarrierInitialize
  {IR(32775, 7), 290}, // 506 NoLine
  {IR(35878, 27), 430}, // 507 NodePayloadArrayLengthAMDX
  {IR(27627, 4), 0}, // 508 Nop
  {IR(30952, 4), 186}, // 509 Not
  {IR(30455, 8), 150}, // 510 Ordered
  {IR(30328, 13), 136}, // 511 OuterProduct
  {IR(31447, 4), 223}, // 512 Phi
  {IR(29131, 15), 60}, // 513 PtrAccessChain
  {IR(46772, 30), 754}, // 514 PtrCastToCrossWorkgroupALTERA
  {IR(46802, 29), 754}, // 515 PtrCastToCrossWorkgroupINTEL
  {IR(30085, 17), 111}, // 516 PtrCastToGeneric
  {IR(33871, 8), 343}, // 517 PtrDiff
  {IR(33850, 9), 341}, // 518 PtrEqual
  {IR(33859, 12), 342}, // 519 PtrNotEqual
  {IR(30013, 14), 106}, // 520 QuantizeToF16
  {IR(38600, 17), 528}, // 521 RawAccessChainNV
  {IR(35159, 31), 400}, // 522 RayQueryConfirmIntersectionKHR
  {IR(35127, 32), 399}, // 523 RayQueryGenerateIntersectionKHR
  {IR(37610, 23), 493}, // 524 RayQueryGetClusterIdNV
  {IR(47284, 39), 767}, // 525 RayQueryGetIntersectionBarycentricsKHR
  {IR(47359, 46), 769}, // 526 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(37575, 35), 493}, // 527 RayQueryGetIntersectionClusterIdNV
  {IR(47323, 36), 768}, // 528 RayQueryGetIntersectionFrontFaceKHR
  {IR(47203, 40), 765}, // 529 RayQueryGetIntersectionGeometryIndexKHR
  {IR(47055, 46), 762}, // 530 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(47101, 37), 763}, // 531 RayQueryGetIntersectionInstanceIdKHR
  {IR(47138, 65), 764}, // 532 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(38767, 37), 533}, // 533 RayQueryGetIntersectionLSSHitValueNV
  {IR(38695, 38), 531}, // 534 RayQueryGetIntersectionLSSPositionsNV
  {IR(38733, 34), 532}, // 535 RayQueryGetIntersectionLSSRadiiNV
  {IR(47405, 45), 770}, // 536 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(47450, 42), 771}, // 537 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(47553, 40), 774}, // 538 RayQueryGetIntersectionObjectToWorldKHR
  {IR(47243, 41), 766}, // 539 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(38617, 40), 529}, // 540 RayQueryGetIntersectionSpherePositionNV
  {IR(38657, 38), 530}, // 541 RayQueryGetIntersectionSphereRadiusNV
  {IR(47027, 28), 761}, // 542 RayQueryGetIntersectionTKHR
  {IR(37450, 50), 490}, // 543 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(35209, 31), 402}, // 544 RayQueryGetIntersectionTypeKHR
  {IR(47593, 40), 775}, // 545 RayQueryGetIntersectionWorldToObjectKHR
  {IR(47004, 23), 760}, // 546 RayQueryGetRayFlagsKHR
  {IR(46982, 22), 759}, // 547 RayQueryGetRayTMinKHR
  {IR(47492, 32), 772}, // 548 RayQueryGetWorldRayDirectionKHR
  {IR(47524, 29), 773}, // 549 RayQueryGetWorldRayOriginKHR
  {IR(35084, 22), 397}, // 550 RayQueryInitializeKHR
  {IR(38975, 19), 541}, // 551 RayQueryIsLSSHitNV
  {IR(38953, 22), 540}, // 552 RayQueryIsSphereHitNV
  {IR(35190, 19), 401}, // 553 RayQueryProceedKHR
  {IR(35106, 21), 398}, // 554 RayQueryTerminateKHR
  {IR(35762, 13), 425}, // 555 ReadClockKHR
  {IR(31864, 9), 249}, // 556 ReadPipe
  {IR(46890, 23), 756}, // 557 ReadPipeBlockingALTERA
  {IR(46913, 22), 756}, // 558 ReadPipeBlockingINTEL
  {IR(32312, 13), 271}, // 559 ReleaseEvent
  {IR(36859, 24), 467}, // 560 ReorderThreadWithHintNV
  {IR(36830, 29), 466}, // 561 ReorderThreadWithHitObjectNV
  {IR(37332, 22), 484}, // 562 ReportIntersectionKHR
  {IR(37354, 21), 484}, // 563 ReportIntersectionNV
  {IR(31918, 23), 253}, // 564 ReserveReadPipePackets
  {IR(31941, 24), 254}, // 565 ReserveWritePipePackets
  {IR(31883, 17), 251}, // 566 ReservedReadPipe
  {IR(31900, 18), 252}, // 567 ReservedWritePipe
  {IR(44864, 19), 697}, // 568 RestoreMemoryINTEL
  {IR(32300, 12), 270}, // 569 RetainEvent
  {IR(31629, 7), 231}, // 570 Return
  {IR(31636, 12), 232}, // 571 ReturnValue
  {IR(48745, 18), 816}, // 572 RoundFToTF32INTEL
  {IR(29995, 9), 104}, // 573 SConvert
  {IR(30203, 5), 124}, // 574 SDiv
  {IR(34671, 5), 382}, // 575 SDot
  {IR(34712, 11), 385}, // 576 SDotAccSat
  {IR(34723, 14), 385}, // 577 SDotAccSatKHR
  {IR(34676, 8), 382}, // 578 SDotKHR
  {IR(30571, 13), 161}, // 579 SGreaterThan
  {IR(30602, 18), 163}, // 580 SGreaterThanEqual
  {IR(30630, 10), 165}, // 581 SLessThan
  {IR(30655, 15), 167}, // 582 SLessThanEqual
  {IR(30223, 5), 128}, // 583 SMod
  {IR(30379, 13), 141}, // 584 SMulExtended
  {IR(30152, 8), 115}, // 585 SNegate
  {IR(30218, 5), 127}, // 586 SRem
  {IR(34697, 6), 384}, // 587 SUDot
  {IR(34762, 12), 387}, // 588 SUDotAccSat
  {IR(34774, 15), 387}, // 589 SUDotAccSatKHR
  {IR(34703, 9), 384}, // 590 SUDotKHR
  {IR(29479, 13), 77}, // 591 SampledImage
  {IR(38519, 29), 527}, // 592 SamplerImageAddressingModeNV
  {IR(30041, 15), 108}, // 593 SatConvertSToU
  {IR(30056, 15), 109}, // 594 SatConvertUToS
  {IR(44848, 16), 696}, // 595 SaveMemoryINTEL
  {IR(30534, 7), 157}, // 596 Select
  {IR(31526, 15), 225}, // 597 SelectionMerge
  {IR(37132, 18), 477}, // 598 SetMeshOutputsEXT
  {IR(32354, 19), 274}, // 599 SetUserEventStatus
  {IR(30903, 17), 182}, // 600 ShiftLeftLogical
  {IR(30882, 21), 181}, // 601 ShiftRightArithmetic
  {IR(30864, 18), 180}, // 602 ShiftRightLogical
  {IR(30430, 11), 148}, // 603 SignBitSet
  {IR(32835, 7), 294}, // 604 SizeOf
  {IR(27808, 7), 3}, // 605 Source
  {IR(27689, 16), 2}, // 606 SourceContinued
  {IR(27815, 16), 4}, // 607 SourceExtension
  {IR(28826, 13), 45}, // 608 SpecConstant
  {IR(48483, 30), 805}, // 609 SpecConstantArchitectureINTEL
  {IR(48550, 30), 806}, // 610 SpecConstantCapabilitiesINTEL
  {IR(28839, 22), 46}, // 611 SpecConstantComposite
  {IR(47727, 36), 780}, // 612 SpecConstantCompositeContinuedINTEL
  {IR(35003, 34), 394}, // 613 SpecConstantCompositeReplicateEXT
  {IR(28808, 18), 44}, // 614 SpecConstantFalse
  {IR(28902, 15), 47}, // 615 SpecConstantOp
  {IR(35947, 23), 433}, // 616 SpecConstantStringAMDX
  {IR(48459, 24), 804}, // 617 SpecConstantTargetINTEL
  {IR(28791, 17), 43}, // 618 SpecConstantTrue
  {IR(33925, 25), 346}, // 619 StencilAttachmentReadEXT
  {IR(29066, 6), 55}, // 620 Store
  {IR(27847, 7), 7}, // 621 String
  {IR(48148, 25), 793}, // 622 Subgroup2DBlockLoadINTEL
  {IR(48173, 34), 794}, // 623 Subgroup2DBlockLoadTransformINTEL
  {IR(48207, 34), 795}, // 624 Subgroup2DBlockLoadTransposeINTEL
  {IR(48241, 29), 796}, // 625 Subgroup2DBlockPrefetchINTEL
  {IR(48270, 26), 797}, // 626 Subgroup2DBlockStoreINTEL
  {IR(34423, 20), 372}, // 627 SubgroupAllEqualKHR
  {IR(34393, 15), 370}, // 628 SubgroupAllKHR
  {IR(34408, 15), 371}, // 629 SubgroupAnyKHR
  {IR(43366, 30), 660}, // 630 SubgroupAvcBmeInitializeINTEL
  {IR(43336, 30), 659}, // 631 SubgroupAvcFmeInitializeINTEL
  {IR(41771, 35), 629}, // 632 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(41806, 39), 630}, // 633 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(42447, 38), 643}, // 634 SubgroupAvcImeConvertToMceResultINTEL
  {IR(42070, 45), 636}, // 635 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(42170, 53), 638}, // 636 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(42391, 56), 642}, // 637 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(42279, 54), 640}, // 638 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(42023, 47), 635}, // 639 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(42115, 55), 637}, // 640 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(42333, 58), 641}, // 641 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(42223, 56), 639}, // 642 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(43079, 36), 654}, // 643 SubgroupAvcImeGetBorderReachedINTEL
  {IR(42531, 44), 645}, // 644 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(42485, 46), 644}, // 645 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(42946, 66), 652}, // 646 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(42878, 68), 651}, // 647 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(43012, 67), 653}, // 648 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(42741, 68), 649}, // 649 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(42671, 70), 648}, // 650 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(42809, 69), 650}, // 651 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(43115, 48), 655}, // 652 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(43163, 59), 656}, // 653 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(43280, 56), 658}, // 654 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(43222, 58), 657}, // 655 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(41634, 30), 625}, // 656 SubgroupAvcImeInitializeINTEL
  {IR(41738, 33), 628}, // 657 SubgroupAvcImeRefWindowSizeINTEL
  {IR(41702, 36), 627}, // 658 SubgroupAvcImeSetDualReferenceINTEL
  {IR(41935, 54), 633}, // 659 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(41845, 43), 631}, // 660 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(41664, 38), 626}, // 661 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(41888, 47), 632}, // 662 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(41989, 34), 634}, // 663 SubgroupAvcImeSetWeightedSadINTEL
  {IR(42624, 47), 647}, // 664 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(42575, 49), 646}, // 665 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(41024, 39), 610}, // 666 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(41063, 38), 611}, // 667 SubgroupAvcMceConvertToImeResultINTEL
  {IR(41101, 39), 612}, // 668 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(41140, 38), 613}, // 669 SubgroupAvcMceConvertToRefResultINTEL
  {IR(41178, 39), 614}, // 670 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(41217, 38), 615}, // 671 SubgroupAvcMceConvertToSicResultINTEL
  {IR(41330, 43), 618}, // 672 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(40462, 50), 599}, // 673 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(40060, 60), 591}, // 674 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(40260, 51), 595}, // 675 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(40406, 56), 598}, // 676 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(40173, 47), 593}, // 677 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(40761, 56), 605}, // 678 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(40660, 50), 603}, // 679 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(40355, 51), 597}, // 680 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(40564, 49), 601}, // 681 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(40512, 52), 600}, // 682 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(40710, 51), 604}, // 683 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(41449, 38), 621}, // 684 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(41291, 39), 617}, // 685 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(41373, 38), 619}, // 686 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(41411, 38), 620}, // 687 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(41487, 45), 622}, // 688 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(41532, 40), 623}, // 689 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(41572, 62), 624}, // 690 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(41255, 36), 616}, // 691 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(40817, 33), 606}, // 692 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(40963, 61), 609}, // 693 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(40120, 53), 592}, // 694 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(40311, 44), 596}, // 695 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(40220, 40), 594}, // 696 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(40613, 47), 602}, // 697 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(40902, 61), 608}, // 698 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(40850, 52), 607}, // 699 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(43396, 39), 661}, // 700 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(43718, 38), 668}, // 701 SubgroupAvcRefConvertToMceResultINTEL
  {IR(43571, 45), 665}, // 702 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(43616, 46), 666}, // 703 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(43662, 56), 667}, // 704 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(43524, 47), 664}, // 705 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(43435, 46), 662}, // 706 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(43481, 43), 663}, // 707 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(43854, 42), 672}, // 708 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(43818, 36), 671}, // 709 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(43786, 32), 670}, // 710 SubgroupAvcSicConfigureSkcINTEL
  {IR(43935, 39), 674}, // 711 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(44475, 38), 686}, // 712 SubgroupAvcSicConvertToMceResultINTEL
  {IR(44250, 31), 681}, // 713 SubgroupAvcSicEvaluateIpeINTEL
  {IR(44328, 45), 683}, // 714 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(44373, 46), 684}, // 715 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(44419, 56), 685}, // 716 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(44281, 47), 682}, // 717 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(44592, 46), 689}, // 718 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(44548, 44), 688}, // 719 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(44813, 35), 694}, // 720 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(44679, 36), 691}, // 721 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(44513, 35), 687}, // 722 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(43896, 39), 673}, // 723 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(44638, 41), 690}, // 724 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(44715, 50), 692}, // 725 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(44765, 48), 693}, // 726 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(43756, 30), 669}, // 727 SubgroupAvcSicInitializeINTEL
  {IR(44116, 43), 678}, // 728 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(44207, 43), 680}, // 729 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(44066, 50), 677}, // 730 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(44018, 48), 676}, // 731 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(43974, 44), 675}, // 732 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(44159, 48), 679}, // 733 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6104, 18), 363}, // 734 SubgroupBallotKHR
  {IR(48121, 27), 792}, // 735 SubgroupBlockPrefetchINTEL
  {IR(39066, 23), 546}, // 736 SubgroupBlockReadINTEL
  {IR(39089, 24), 547}, // 737 SubgroupBlockWriteINTEL
  {IR(34260, 27), 364}, // 738 SubgroupFirstInvocationKHR
  {IR(39113, 28), 548}, // 739 SubgroupImageBlockReadINTEL
  {IR(39141, 29), 549}, // 740 SubgroupImageBlockWriteINTEL
  {IR(39170, 33), 550}, // 741 SubgroupImageMediaBlockReadINTEL
  {IR(39203, 34), 551}, // 742 SubgroupImageMediaBlockWriteINTEL
  {IR(15070, 38), 798}, // 743 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(34443, 26), 374}, // 744 SubgroupReadInvocationKHR
  {IR(38994, 25), 543}, // 745 SubgroupShuffleDownINTEL
  {IR(12726, 21), 542}, // 746 SubgroupShuffleINTEL
  {IR(39019, 23), 544}, // 747 SubgroupShuffleUpINTEL
  {IR(39042, 24), 545}, // 748 SubgroupShuffleXorINTEL
  {IR(31617, 7), 229}, // 749 Switch
  {IR(47935, 24), 788}, // 750 TaskSequenceAsyncALTERA
  {IR(47959, 23), 788}, // 751 TaskSequenceAsyncINTEL
  {IR(47886, 25), 787}, // 752 TaskSequenceCreateALTERA
  {IR(47911, 24), 787}, // 753 TaskSequenceCreateINTEL
  {IR(47982, 22), 789}, // 754 TaskSequenceGetALTERA
  {IR(48004, 21), 789}, // 755 TaskSequenceGetINTEL
  {IR(48025, 26), 790}, // 756 TaskSequenceReleaseALTERA
  {IR(48051, 25), 790}, // 757 TaskSequenceReleaseINTEL
  {IR(38337, 27), 519}, // 758 TensorLayoutSetBlockSizeNV
  {IR(38201, 28), 512}, // 759 TensorLayoutSetClampValueNV
  {IR(38130, 27), 509}, // 760 TensorLayoutSetDimensionNV
  {IR(38157, 24), 510}, // 761 TensorLayoutSetStrideNV
  {IR(38181, 20), 511}, // 762 TensorLayoutSliceNV
  {IR(34035, 19), 350}, // 763 TensorQuerySizeARM
  {IR(34006, 14), 348}, // 764 TensorReadARM
  {IR(38317, 20), 518}, // 765 TensorViewSetClipNV
  {IR(38248, 25), 514}, // 766 TensorViewSetDimensionNV
  {IR(38273, 22), 515}, // 767 TensorViewSetStrideNV
  {IR(34020, 15), 349}, // 768 TensorWriteARM
  {IR(34147, 20), 358}, // 769 TerminateInvocation
  {IR(34608, 16), 381}, // 770 TerminateRayKHR
  {IR(37396, 15), 486}, // 771 TerminateRayNV
  {IR(37419, 14), 488}, // 772 TraceMotionNV
  {IR(37411, 8), 487}, // 773 TraceNV
  {IR(34520, 12), 377}, // 774 TraceRayKHR
  {IR(37433, 17), 489}, // 775 TraceRayMotionNV
  {IR(29469, 10), 76}, // 776 Transpose
  {IR(37500, 29), 491}, // 777 TypeAccelerationStructureKHR
  {IR(37529, 28), 491}, // 778 TypeAccelerationStructureNV
  {IR(28418, 10), 25}, // 779 TypeArray
  {IR(39979, 37), 588}, // 780 TypeAvcImeDualReferenceStreaminINTEL
  {IR(39714, 23), 579}, // 781 TypeAvcImePayloadINTEL
  {IR(39896, 44), 586}, // 782 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(39828, 22), 584}, // 783 TypeAvcImeResultINTEL
  {IR(39850, 46), 585}, // 784 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(39940, 39), 587}, // 785 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(39783, 23), 582}, // 786 TypeAvcMcePayloadINTEL
  {IR(39806, 22), 583}, // 787 TypeAvcMceResultINTEL
  {IR(39737, 23), 580}, // 788 TypeAvcRefPayloadINTEL
  {IR(40016, 22), 589}, // 789 TypeAvcRefResultINTEL
  {IR(39760, 23), 581}, // 790 TypeAvcSicPayloadINTEL
  {IR(40038, 22), 590}, // 791 TypeAvcSicResultINTEL
  {IR(28180, 9), 17}, // 792 TypeBool
  {IR(47647, 23), 777}, // 793 TypeBufferSurfaceINTEL
  {IR(34789, 25), 388}, // 794 TypeCooperativeMatrixKHR
  {IR(37657, 24), 495}, // 795 TypeCooperativeMatrixNV
  {IR(36922, 24), 470}, // 796 TypeCooperativeVectorNV
  {IR(28533, 16), 32}, // 797 TypeDeviceEvent
  {IR(28523, 10), 31}, // 798 TypeEvent
  {IR(28234, 10), 19}, // 799 TypeFloat
  {IR(28582, 19), 36}, // 800 TypeForwardPointer
  {IR(28510, 13), 30}, // 801 TypeFunction
  {IR(34134, 13), 357}, // 802 TypeGraphARM
  {IR(36883, 16), 468}, // 803 TypeHitObjectNV
  {IR(28379, 10), 22}, // 804 TypeImage
  {IR(28189, 8), 18}, // 805 TypeInt
  {IR(28255, 11), 21}, // 806 TypeMatrix
  {IR(32964, 17), 300}, // 807 TypeNamedBarrier
  {IR(35824, 25), 428}, // 808 TypeNodePayloadArrayAMDX
  {IR(28456, 11), 28}, // 809 TypeOpaque
  {IR(28573, 9), 35}, // 810 TypePipe
  {IR(32842, 16), 295}, // 811 TypePipeStorage
  {IR(28498, 12), 29}, // 812 TypePointer
  {IR(28563, 10), 34}, // 813 TypeQueue
  {IR(35068, 16), 396}, // 814 TypeRayQueryKHR
  {IR(28549, 14), 33}, // 815 TypeReserveId
  {IR(28428, 17), 26}, // 816 TypeRuntimeArray
  {IR(28401, 17), 24}, // 817 TypeSampledImage
  {IR(28389, 12), 23}, // 818 TypeSampler
  {IR(28445, 11), 27}, // 819 TypeStruct
  {IR(47670, 25), 778}, // 820 TypeStructContinuedINTEL
  {IR(48076, 23), 791}, // 821 TypeTaskSequenceALTERA
  {IR(48099, 22), 791}, // 822 TypeTaskSequenceINTEL
  {IR(33950, 14), 347}, // 823 TypeTensorARM
  {IR(38073, 19), 506}, // 824 TypeTensorLayoutNV
  {IR(38092, 17), 507}, // 825 TypeTensorViewNV
  {IR(34167, 22), 359}, // 826 TypeUntypedPointerKHR
  {IR(28244, 11), 20}, // 827 TypeVector
  {IR(39696, 18), 578}, // 828 TypeVmeImageINTEL
  {IR(28171, 9), 16}, // 829 TypeVoid
  {IR(39325, 13), 557}, // 830 UAddSatINTEL
  {IR(39352, 14), 559}, // 831 UAverageINTEL
  {IR(39387, 21), 561}, // 832 UAverageRoundedINTEL
  {IR(29986, 9), 103}, // 833 UConvert
  {IR(39237, 24), 552}, // 834 UCountLeadingZerosINTEL
  {IR(39261, 25), 553}, // 835 UCountTrailingZerosINTEL
  {IR(30198, 5), 123}, // 836 UDiv
  {IR(34684, 5), 383}, // 837 UDot
  {IR(34737, 11), 386}, // 838 UDotAccSat
  {IR(34748, 14), 386}, // 839 UDotAccSatKHR
  {IR(34689, 8), 383}, // 840 UDotKHR
  {IR(30558, 13), 160}, // 841 UGreaterThan
  {IR(30584, 18), 162}, // 842 UGreaterThanEqual
  {IR(30620, 10), 164}, // 843 ULessThan
  {IR(30640, 15), 166}, // 844 ULessThanEqual
  {IR(30213, 5), 126}, // 845 UMod
  {IR(39449, 15), 565}, // 846 UMul32x16INTEL
  {IR(30366, 13), 140}, // 847 UMulExtended
  {IR(39421, 13), 563}, // 848 USubSatINTEL
  {IR(27683, 6), 1}, // 849 Undef
  {IR(30463, 10), 151}, // 850 Unordered
  {IR(31648, 12), 233}, // 851 Unreachable
  {IR(34208, 22), 361}, // 852 UntypedAccessChainKHR
  {IR(34345, 22), 367}, // 853 UntypedArrayLengthKHR
  {IR(34495, 25), 376}, // 854 UntypedGroupAsyncCopyKHR
  {IR(34230, 30), 362}, // 855 UntypedInBoundsAccessChainKHR
  {IR(34312, 33), 366}, // 856 UntypedInBoundsPtrAccessChainKHR
  {IR(34367, 19), 368}, // 857 UntypedPrefetchKHR
  {IR(34287, 25), 365}, // 858 UntypedPtrAccessChainKHR
  {IR(34189, 19), 360}, // 859 UntypedVariableKHR
  {IR(15136, 32), 800}, // 860 UntypedVariableLengthArrayINTEL
  {IR(28994, 9), 52}, // 861 Variable
  {IR(13182, 25), 695}, // 862 VariableLengthArrayINTEL
  {IR(29351, 21), 69}, // 863 VectorExtractDynamic
  {IR(29372, 20), 70}, // 864 VectorInsertDynamic
  {IR(29392, 14), 71}, // 865 VectorShuffle
  {IR(30274, 18), 133}, // 866 VectorTimesMatrix
  {IR(30238, 18), 131}, // 867 VectorTimesScalar
  {IR(39682, 14), 577}, // 868 VmeImageINTEL
  {IR(37177, 33), 479}, // 869 WritePackedPrimitiveIndices4x8NV
  {IR(31873, 10), 250}, // 870 WritePipe
  {IR(46935, 24), 757}, // 871 WritePipeBlockingALTERA
  {IR(46959, 23), 757}, // 872 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 494> kExtInstByValue{{
    {0,IR(724, 9),IR(48887, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(724, 9),IR(48895, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(48908, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(48916, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(48930, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(770, 3),IR(49003, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(773, 3),IR(49052, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(776, 2),IR(49107, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(49126, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(49141, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(778, 6),IR(49157, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(49170, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(784, 9),IR(49188, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(793, 9),IR(49240, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(802, 10),IR(49259, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(812, 5),IR(49275, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(49296, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(49317, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(817, 6),IR(49335, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(823, 5),IR(49362, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(828, 5),IR(49397, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(802, 10),IR(49428, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(833, 8),IR(49448, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(841, 11),IR(49473, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(852, 5),IR(49487, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(857, 3),IR(49505, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(860, 2),IR(49536, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(49547, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(862, 3),IR(49560, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(865, 7),IR(49617, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(49636, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(49657, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(49670, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(872, 2),IR(49714, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(706, 1),IR(49729, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(874, 4),IR(49745, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(857, 3),IR(49759, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(49775, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(49781, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(49791, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(49797, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(49802, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(49807, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(49813, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(49819, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(49825, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(49830, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(49836, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(49844, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(49852, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(49856, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(49860, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(49864, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(49869, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(49874, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(49879, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(49884, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(49889, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(49894, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(49900, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(49906, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(49912, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(49918, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(49922, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(49926, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(49930, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(49935, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(49940, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(49945, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(49957, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(49969, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(49983, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(49988, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(49999, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(50004, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(50009, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(50014, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(50019, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(50024, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(50029, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(50036, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(50043, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(50050, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(50055, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(50060, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(50065, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(50076, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(50080, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(50086, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(50098, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(50104, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(50117, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(50130, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(50144, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(50158, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(50171, 15),IR(311, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(50186, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(50202, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(50218, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(50233, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(50248, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(50263, 17),IR(311, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(50280, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(50287, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(50296, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(50302, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(50312, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(50324, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(50332, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(50340, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(50349, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(50358, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(50367, 22),IR(312, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(50389, 20),IR(312, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(50409, 20),IR(312, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(50429, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(50434, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(50439, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5323, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(878, 5),IR(50446, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(883, 5),IR(50459, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(883, 5),IR(50481, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(554, 7),IR(50497, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(554, 7),IR(50522, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(883, 5),IR(50541, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(883, 5),IR(50565, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(883, 5),IR(50586, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(883, 5),IR(50607, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(883, 5),IR(50623, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(50641, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(50667, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(50692, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(50712, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(50737, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(50767, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(50790, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(50815, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(50841, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(50871, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(50897, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(8748, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(50917, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(50947, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(883, 5),IR(50975, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(554, 7),IR(51003, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(51026, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(51061, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(51099, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(51141, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(525, 6),IR(51186, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(525, 6),IR(51223, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(883, 5),IR(51263, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(883, 5),IR(51290, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(51317, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(51349, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(51389, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(51400, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(51426, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(51458, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(51492, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(48916, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(48930, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(49003, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(49052, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(49107, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(49126, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(49141, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(525, 6),IR(49157, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(375, 3),IR(49170, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(888, 9),IR(49188, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(897, 10),IR(49240, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(907, 9),IR(49259, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(49275, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(49296, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(49317, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(525, 6),IR(49335, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(634, 5),IR(49362, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(916, 5),IR(49397, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(921, 10),IR(49428, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(471, 8),IR(49448, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(921, 10),IR(49473, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(883, 5),IR(49487, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(49505, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(860, 2),IR(49536, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(49547, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(931, 3),IR(49560, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(934, 8),IR(49617, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(49636, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(942, 4),IR(49657, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(942, 4),IR(49670, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(49714, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(706, 1),IR(49729, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(577, 4),IR(49745, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(49759, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(506, 7),IR(51514, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(860, 2),IR(51534, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(51546, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(51570, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(634, 5),IR(51591, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(51601, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(51613, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(51634, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(51651, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(51667, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(724, 9),IR(51683, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(51697, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(51710, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(51722, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(51736, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(946, 15),IR(51759, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(961, 33),IR(51779, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(994, 19),IR(51798, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(48916, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1013, 4),IR(48930, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1017, 3),IR(49003, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1020, 3),IR(49052, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1023, 2),IR(49107, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(49126, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(49141, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(778, 6),IR(49157, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1025, 3),IR(49170, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1028, 9),IR(49188, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1037, 10),IR(49240, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1047, 10),IR(49259, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1057, 5),IR(49275, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(49296, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(49317, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(817, 6),IR(49335, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(823, 5),IR(49362, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(828, 5),IR(49397, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1047, 10),IR(49428, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1062, 8),IR(49448, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1070, 11),IR(49473, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(852, 5),IR(49487, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(857, 3),IR(49505, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(860, 2),IR(49536, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(49547, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(862, 3),IR(49560, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1081, 8),IR(49617, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(49636, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(49657, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(942, 4),IR(49670, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1089, 2),IR(49714, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(706, 1),IR(49729, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(874, 4),IR(49745, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(857, 3),IR(49759, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1091, 7),IR(51514, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(860, 2),IR(51534, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1098, 8),IR(52119, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(52136, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(52141, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(52147, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(52154, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(52159, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(52165, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(52172, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(52177, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(52183, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(52189, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(52196, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(52204, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(52209, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(52214, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(52223, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(52227, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(52232, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(52238, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(52243, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(52247, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(52251, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(52256, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(52262, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(52268, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(52273, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(52278, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(52284, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(52288, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(52293, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(52298, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(52303, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(52309, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(52315, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(52321, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(52327, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(52333, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(52340, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(52349, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(52353, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(52358, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(52364, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(52370, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(52375, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(52379, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(52386, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(52393, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(52398, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(52402, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(52412, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(52416, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(52421, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(52426, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(52436, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(52443, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(52448, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(52454, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(52460, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(52466, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(52470, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(52477, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(52482, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(52488, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(52493, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(52497, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(52502, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(52508, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(52515, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(52521, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(52530, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(52542, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(52551, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(52561, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(52572, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(52581, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(52591, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(52602, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(52612, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(52623, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(52634, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(52643, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(52653, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(52662, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(52673, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(52687, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(52698, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(52710, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(52723, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(52734, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(52746, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(52759, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(52771, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(52784, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(52797, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(52808, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(52820, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(52831, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(52838, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(52846, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(52858, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(52870, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(52874, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(52882, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(52887, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(52898, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(52903, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(52909, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(52918, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(52925, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(52935, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(52949, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(52961, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(52976, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(52982, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(52993, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(53003, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(53013, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(53020, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(53027, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(53035, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(53043, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(53051, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(53059, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(53063, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(53067, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(53076, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(53086, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(53096, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(53102, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(53108, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(53114, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(53120, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(53129, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(53136, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(53146, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(53156, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(53167, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(53178, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(53187, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(53195, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(53203, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(53211, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1106, 3),IR(53219, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(53226, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(53234, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1106, 3),IR(53245, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(53257, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1109, 4),IR(53269, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(53283, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1109, 4),IR(53296, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1106, 3),IR(53311, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(53324, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1109, 4),IR(53338, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(53354, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(53362, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(53371, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(53378, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(53387, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(53397, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(53404, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(53410, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(53421, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(53430, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(53439, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(53456, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(53473, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(53481, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(53503, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(53531, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(53550, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(53559, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(53582, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(53591, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(53600, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(53609, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(53618, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(53627, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(53636, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(53645, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(53654, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(53663, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(506, 7),IR(53670, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(714, 10),IR(53681, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(714, 10),IR(53688, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(714, 10),IR(53695, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(53712, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(53718, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(634, 5),IR(53725, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(53736, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(724, 9),IR(53743, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(53760, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(53766, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(53770, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(53778, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(53783, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(53787, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(53810, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(53822, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(53833, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(53845, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(53852, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(53864, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(53883, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(53903, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(53914, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(53926, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(53934, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(53942, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(53946, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(53950, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(53954, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(53960, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(53964, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(53976, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(53981, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(53985, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(53989, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(53993, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(53999, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(54003, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(54015, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(54022, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(54033, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(54039, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(54043, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(54050, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(54056, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(54064, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(54078, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(54089, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(54100, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(54111, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(54122, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(54137, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(54148, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(54155, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(54159, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(54167, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(54175, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(54181, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(54186, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(54196, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(54203, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(634, 5),IR(54211, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(54218, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(714, 10),IR(54223, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    {IR(48887, 8), 0}, // MIN_SAD in arm_motion_engine_100
    {IR(48895, 13), 1}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(48908, 8), 2}, // RAW_SAD in arm_motion_engine_100
    {IR(48930, 21), 4}, // DebugCompilationUnit in debuginfo
    {IR(49657, 13), 31}, // DebugDeclare in debuginfo
    {IR(49729, 16), 34}, // DebugExpression in debuginfo
    {IR(49473, 14), 23}, // DebugFunction in debuginfo
    {IR(49448, 25), 22}, // DebugFunctionDeclaration in debuginfo
    {IR(49428, 20), 21}, // DebugGlobalVariable in debuginfo
    {IR(48916, 14), 3}, // DebugInfoNone in debuginfo
    {IR(49560, 15), 28}, // DebugInlinedAt in debuginfo
    {IR(49636, 21), 30}, // DebugInlinedVariable in debuginfo
    {IR(49487, 18), 24}, // DebugLexicalBlock in debuginfo
    {IR(49505, 31), 25}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(49617, 19), 29}, // DebugLocalVariable in debuginfo
    {IR(49745, 14), 35}, // DebugMacroDef in debuginfo
    {IR(49759, 16), 36}, // DebugMacroUndef in debuginfo
    {IR(49547, 13), 27}, // DebugNoScope in debuginfo
    {IR(49714, 15), 33}, // DebugOperation in debuginfo
    {IR(49536, 11), 26}, // DebugScope in debuginfo
    {IR(49126, 15), 8}, // DebugTypeArray in debuginfo
    {IR(49003, 15), 5}, // DebugTypeBasic in debuginfo
    {IR(49240, 19), 13}, // DebugTypeComposite in debuginfo
    {IR(49188, 14), 12}, // DebugTypeEnum in debuginfo
    {IR(49170, 18), 11}, // DebugTypeFunction in debuginfo
    {IR(49275, 21), 15}, // DebugTypeInheritance in debuginfo
    {IR(49259, 16), 14}, // DebugTypeMember in debuginfo
    {IR(49052, 17), 6}, // DebugTypePointer in debuginfo
    {IR(49296, 21), 16}, // DebugTypePtrToMember in debuginfo
    {IR(49107, 19), 7}, // DebugTypeQualifier in debuginfo
    {IR(49317, 18), 17}, // DebugTypeTemplate in debuginfo
    {IR(49335, 27), 18}, // DebugTypeTemplateParameter in debuginfo
    {IR(49397, 31), 20}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(49362, 35), 19}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(49141, 16), 9}, // DebugTypeVector in debuginfo
    {IR(49157, 13), 10}, // DebugTypedef in debuginfo
    {IR(49670, 11), 32}, // DebugValue in debuginfo
    {IR(49869, 5), 53}, // Acos in glsl_std_450
    {IR(49900, 6), 59}, // Acosh in glsl_std_450
    {IR(49864, 5), 52}, // Asin in glsl_std_450
    {IR(49894, 6), 58}, // Asinh in glsl_std_450
    {IR(49874, 5), 54}, // Atan in glsl_std_450
    {IR(49912, 6), 61}, // Atan2 in glsl_std_450
    {IR(49906, 6), 60}, // Atanh in glsl_std_450
    {IR(49825, 5), 45}, // Ceil in glsl_std_450
    {IR(49856, 4), 50}, // Cos in glsl_std_450
    {IR(49884, 5), 56}, // Cosh in glsl_std_450
    {IR(50296, 6), 104}, // Cross in glsl_std_450
    {IR(49844, 8), 48}, // Degrees in glsl_std_450
    {IR(49957, 12), 69}, // Determinant in glsl_std_450
    {IR(50287, 9), 103}, // Distance in glsl_std_450
    {IR(49922, 4), 63}, // Exp in glsl_std_450
    {IR(49930, 5), 65}, // Exp2 in glsl_std_450
    {IR(49797, 5), 40}, // FAbs in glsl_std_450
    {IR(50029, 7), 79}, // FClamp in glsl_std_450
    {IR(50014, 5), 76}, // FMax in glsl_std_450
    {IR(49999, 5), 73}, // FMin in glsl_std_450
    {IR(50050, 5), 82}, // FMix in glsl_std_450
    {IR(49807, 6), 42}, // FSign in glsl_std_450
    {IR(50312, 12), 106}, // FaceForward in glsl_std_450
    {IR(50340, 9), 109}, // FindILsb in glsl_std_450
    {IR(50349, 9), 110}, // FindSMsb in glsl_std_450
    {IR(50358, 9), 111}, // FindUMsb in glsl_std_450
    {IR(49819, 6), 44}, // Floor in glsl_std_450
    {IR(50076, 4), 86}, // Fma in glsl_std_450
    {IR(49830, 6), 46}, // Fract in glsl_std_450
    {IR(50080, 6), 87}, // Frexp in glsl_std_450
    {IR(50086, 12), 88}, // FrexpStruct in glsl_std_450
    {IR(50055, 5), 83}, // IMix in glsl_std_450
    {IR(50367, 22), 112}, // InterpolateAtCentroid in glsl_std_450
    {IR(50409, 20), 114}, // InterpolateAtOffset in glsl_std_450
    {IR(50389, 20), 113}, // InterpolateAtSample in glsl_std_450
    {IR(49945, 12), 68}, // InverseSqrt in glsl_std_450
    {IR(50098, 6), 89}, // Ldexp in glsl_std_450
    {IR(50280, 7), 102}, // Length in glsl_std_450
    {IR(49926, 4), 64}, // Log in glsl_std_450
    {IR(49935, 5), 66}, // Log2 in glsl_std_450
    {IR(49969, 14), 70}, // MatrixInverse in glsl_std_450
    {IR(49983, 5), 71}, // Modf in glsl_std_450
    {IR(49988, 11), 72}, // ModfStruct in glsl_std_450
    {IR(50439, 7), 117}, // NClamp in glsl_std_450
    {IR(50434, 5), 116}, // NMax in glsl_std_450
    {IR(50429, 5), 115}, // NMin in glsl_std_450
    {IR(50302, 10), 105}, // Normalize in glsl_std_450
    {IR(50171, 15), 95}, // PackDouble2x32 in glsl_std_450
    {IR(50158, 13), 94}, // PackHalf2x16 in glsl_std_450
    {IR(50130, 14), 92}, // PackSnorm2x16 in glsl_std_450
    {IR(50104, 13), 90}, // PackSnorm4x8 in glsl_std_450
    {IR(50144, 14), 93}, // PackUnorm2x16 in glsl_std_450
    {IR(50117, 13), 91}, // PackUnorm4x8 in glsl_std_450
    {IR(49918, 4), 62}, // Pow in glsl_std_450
    {IR(49836, 8), 47}, // Radians in glsl_std_450
    {IR(50324, 8), 107}, // Reflect in glsl_std_450
    {IR(50332, 8), 108}, // Refract in glsl_std_450
    {IR(49775, 6), 37}, // Round in glsl_std_450
    {IR(49781, 10), 38}, // RoundEven in glsl_std_450
    {IR(49802, 5), 41}, // SAbs in glsl_std_450
    {IR(50043, 7), 81}, // SClamp in glsl_std_450
    {IR(50024, 5), 78}, // SMax in glsl_std_450
    {IR(50009, 5), 75}, // SMin in glsl_std_450
    {IR(49813, 6), 43}, // SSign in glsl_std_450
    {IR(49852, 4), 49}, // Sin in glsl_std_450
    {IR(49879, 5), 55}, // Sinh in glsl_std_450
    {IR(50065, 11), 85}, // SmoothStep in glsl_std_450
    {IR(49940, 5), 67}, // Sqrt in glsl_std_450
    {IR(50060, 5), 84}, // Step in glsl_std_450
    {IR(49860, 4), 51}, // Tan in glsl_std_450
    {IR(49889, 5), 57}, // Tanh in glsl_std_450
    {IR(49791, 6), 39}, // Trunc in glsl_std_450
    {IR(50036, 7), 80}, // UClamp in glsl_std_450
    {IR(50019, 5), 77}, // UMax in glsl_std_450
    {IR(50004, 5), 74}, // UMin in glsl_std_450
    {IR(50263, 17), 101}, // UnpackDouble2x32 in glsl_std_450
    {IR(50218, 15), 98}, // UnpackHalf2x16 in glsl_std_450
    {IR(50186, 16), 96}, // UnpackSnorm2x16 in glsl_std_450
    {IR(50233, 15), 99}, // UnpackSnorm4x8 in glsl_std_450
    {IR(50202, 16), 97}, // UnpackUnorm2x16 in glsl_std_450
    {IR(50248, 15), 100}, // UnpackUnorm4x8 in glsl_std_450
    {IR(50446, 13), 119}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(50541, 24), 124}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(50497, 25), 122}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(50522, 19), 123}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(50975, 28), 143}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(51003, 23), 144}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(50565, 21), 125}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(50607, 16), 127}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(50459, 22), 120}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(50586, 21), 126}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(51263, 27), 151}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(50481, 16), 121}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(51290, 27), 152}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(50623, 18), 128}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(51317, 32), 153}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(50871, 26), 138}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(50897, 20), 139}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(51141, 45), 148}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(51223, 40), 150}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(51099, 42), 147}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(51186, 37), 149}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5323, 7), 118}, // Kernel in nonsemantic_clspvreflection
    {IR(8748, 15), 140}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(51458, 34), 158}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(51426, 32), 157}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(51400, 26), 156}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(51389, 11), 155}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(51349, 40), 154}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(51061, 38), 146}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(51026, 35), 145}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(50917, 30), 141}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(50737, 30), 133}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(50712, 25), 132}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(50767, 23), 134}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(50815, 26), 136}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(50841, 30), 137}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(50790, 25), 135}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(50667, 25), 130}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(50947, 28), 142}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(50692, 20), 131}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(50641, 26), 129}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(51492, 22), 159}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(51613, 21), 200}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(48930, 21), 161}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(49657, 13), 188}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(51651, 16), 202}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(49729, 16), 191}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(49473, 14), 180}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(49448, 25), 179}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(51546, 24), 196}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(49428, 20), 178}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(51514, 20), 194}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(48916, 14), 160}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(49560, 15), 185}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(49636, 21), 187}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(49487, 18), 181}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(49505, 31), 182}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(51591, 10), 198}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(49617, 19), 186}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(49745, 14), 192}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(49759, 16), 193}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(51601, 12), 199}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(49547, 13), 184}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(49714, 15), 190}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(49536, 11), 183}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(51534, 12), 195}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(51570, 21), 197}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(51634, 17), 201}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(49126, 15), 165}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(49003, 15), 162}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(49240, 19), 170}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(49188, 14), 169}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(49170, 18), 168}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(49275, 21), 172}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(51667, 16), 203}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(49259, 16), 171}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(49052, 17), 163}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(49296, 21), 173}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(49107, 19), 164}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(49317, 18), 174}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(49335, 27), 175}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(49397, 31), 177}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(49362, 35), 176}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(49141, 16), 166}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(49157, 13), 167}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(49670, 11), 189}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(51683, 14), 204}, // Configuration in nonsemantic_vkspreflection
    {IR(51759, 20), 209}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(51779, 19), 210}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(51798, 21), 211}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(51722, 14), 207}, // PushConstants in nonsemantic_vkspreflection
    {IR(51736, 23), 208}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(51697, 13), 205}, // StartCounter in nonsemantic_vkspreflection
    {IR(51710, 12), 206}, // StopCounter in nonsemantic_vkspreflection
    {IR(48930, 21), 213}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(49657, 13), 240}, // DebugDeclare in opencl_debuginfo_100
    {IR(49729, 16), 243}, // DebugExpression in opencl_debuginfo_100
    {IR(49473, 14), 232}, // DebugFunction in opencl_debuginfo_100
    {IR(49448, 25), 231}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(49428, 20), 230}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(51514, 20), 246}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(48916, 14), 212}, // DebugInfoNone in opencl_debuginfo_100
    {IR(49560, 15), 237}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(49636, 21), 239}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(49487, 18), 233}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(49505, 31), 234}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(49617, 19), 238}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(49745, 14), 244}, // DebugMacroDef in opencl_debuginfo_100
    {IR(49759, 16), 245}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(52119, 17), 248}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(49547, 13), 236}, // DebugNoScope in opencl_debuginfo_100
    {IR(49714, 15), 242}, // DebugOperation in opencl_debuginfo_100
    {IR(49536, 11), 235}, // DebugScope in opencl_debuginfo_100
    {IR(51534, 12), 247}, // DebugSource in opencl_debuginfo_100
    {IR(49126, 15), 217}, // DebugTypeArray in opencl_debuginfo_100
    {IR(49003, 15), 214}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(49240, 19), 222}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(49188, 14), 221}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(49170, 18), 220}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(49275, 21), 224}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(49259, 16), 223}, // DebugTypeMember in opencl_debuginfo_100
    {IR(49052, 17), 215}, // DebugTypePointer in opencl_debuginfo_100
    {IR(49296, 21), 225}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(49107, 19), 216}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(49317, 18), 226}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(49335, 27), 227}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(49397, 31), 229}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(49362, 35), 228}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(49141, 16), 218}, // DebugTypeVector in opencl_debuginfo_100
    {IR(49157, 13), 219}, // DebugTypedef in opencl_debuginfo_100
    {IR(49670, 11), 241}, // DebugValue in opencl_debuginfo_100
    {IR(52136, 5), 249}, // acos in opencl_std_100
    {IR(52141, 6), 250}, // acosh in opencl_std_100
    {IR(52147, 7), 251}, // acospi in opencl_std_100
    {IR(52154, 5), 252}, // asin in opencl_std_100
    {IR(52159, 6), 253}, // asinh in opencl_std_100
    {IR(52165, 7), 254}, // asinpi in opencl_std_100
    {IR(52172, 5), 255}, // atan in opencl_std_100
    {IR(52177, 6), 256}, // atan2 in opencl_std_100
    {IR(52196, 8), 259}, // atan2pi in opencl_std_100
    {IR(52183, 6), 257}, // atanh in opencl_std_100
    {IR(52189, 7), 258}, // atanpi in opencl_std_100
    {IR(53387, 10), 405}, // bitselect in opencl_std_100
    {IR(52204, 5), 260}, // cbrt in opencl_std_100
    {IR(52209, 5), 261}, // ceil in opencl_std_100
    {IR(53059, 4), 370}, // clz in opencl_std_100
    {IR(52214, 9), 262}, // copysign in opencl_std_100
    {IR(52223, 4), 263}, // cos in opencl_std_100
    {IR(52227, 5), 264}, // cosh in opencl_std_100
    {IR(52232, 6), 265}, // cospi in opencl_std_100
    {IR(52903, 6), 353}, // cross in opencl_std_100
    {IR(53063, 4), 371}, // ctz in opencl_std_100
    {IR(52838, 8), 345}, // degrees in opencl_std_100
    {IR(52909, 9), 354}, // distance in opencl_std_100
    {IR(52243, 4), 267}, // erf in opencl_std_100
    {IR(52238, 5), 266}, // erfc in opencl_std_100
    {IR(52247, 4), 268}, // exp in opencl_std_100
    {IR(52256, 6), 270}, // exp10 in opencl_std_100
    {IR(52251, 5), 269}, // exp2 in opencl_std_100
    {IR(52262, 6), 271}, // expm1 in opencl_std_100
    {IR(52268, 5), 272}, // fabs in opencl_std_100
    {IR(52935, 14), 357}, // fast_distance in opencl_std_100
    {IR(52949, 12), 358}, // fast_length in opencl_std_100
    {IR(52961, 15), 359}, // fast_normalize in opencl_std_100
    {IR(52831, 7), 344}, // fclamp in opencl_std_100
    {IR(52273, 5), 273}, // fdim in opencl_std_100
    {IR(52278, 6), 274}, // floor in opencl_std_100
    {IR(52284, 4), 275}, // fma in opencl_std_100
    {IR(52288, 5), 276}, // fmax in opencl_std_100
    {IR(52846, 12), 346}, // fmax_common in opencl_std_100
    {IR(52293, 5), 277}, // fmin in opencl_std_100
    {IR(52858, 12), 347}, // fmin_common in opencl_std_100
    {IR(52298, 5), 278}, // fmod in opencl_std_100
    {IR(52303, 6), 279}, // fract in opencl_std_100
    {IR(52309, 6), 280}, // frexp in opencl_std_100
    {IR(52521, 9), 316}, // half_cos in opencl_std_100
    {IR(52530, 12), 317}, // half_divide in opencl_std_100
    {IR(52542, 9), 318}, // half_exp in opencl_std_100
    {IR(52561, 11), 320}, // half_exp10 in opencl_std_100
    {IR(52551, 10), 319}, // half_exp2 in opencl_std_100
    {IR(52572, 9), 321}, // half_log in opencl_std_100
    {IR(52591, 11), 323}, // half_log10 in opencl_std_100
    {IR(52581, 10), 322}, // half_log2 in opencl_std_100
    {IR(52602, 10), 324}, // half_powr in opencl_std_100
    {IR(52612, 11), 325}, // half_recip in opencl_std_100
    {IR(52623, 11), 326}, // half_rsqrt in opencl_std_100
    {IR(52634, 9), 327}, // half_sin in opencl_std_100
    {IR(52643, 10), 328}, // half_sqrt in opencl_std_100
    {IR(52653, 9), 329}, // half_tan in opencl_std_100
    {IR(52315, 6), 281}, // hypot in opencl_std_100
    {IR(52321, 6), 282}, // ilogb in opencl_std_100
    {IR(52327, 6), 283}, // ldexp in opencl_std_100
    {IR(52918, 7), 355}, // length in opencl_std_100
    {IR(52333, 7), 284}, // lgamma in opencl_std_100
    {IR(52340, 9), 285}, // lgamma_r in opencl_std_100
    {IR(52349, 4), 286}, // log in opencl_std_100
    {IR(52358, 6), 288}, // log10 in opencl_std_100
    {IR(52364, 6), 289}, // log1p in opencl_std_100
    {IR(52353, 5), 287}, // log2 in opencl_std_100
    {IR(52370, 5), 290}, // logb in opencl_std_100
    {IR(52375, 4), 291}, // mad in opencl_std_100
    {IR(52379, 7), 292}, // maxmag in opencl_std_100
    {IR(52386, 7), 293}, // minmag in opencl_std_100
    {IR(52870, 4), 348}, // mix in opencl_std_100
    {IR(52393, 5), 294}, // modf in opencl_std_100
    {IR(52398, 4), 295}, // nan in opencl_std_100
    {IR(52662, 11), 330}, // native_cos in opencl_std_100
    {IR(52673, 14), 331}, // native_divide in opencl_std_100
    {IR(52687, 11), 332}, // native_exp in opencl_std_100
    {IR(52710, 13), 334}, // native_exp10 in opencl_std_100
    {IR(52698, 12), 333}, // native_exp2 in opencl_std_100
    {IR(52723, 11), 335}, // native_log in opencl_std_100
    {IR(52746, 13), 337}, // native_log10 in opencl_std_100
    {IR(52734, 12), 336}, // native_log2 in opencl_std_100
    {IR(52759, 12), 338}, // native_powr in opencl_std_100
    {IR(52771, 13), 339}, // native_recip in opencl_std_100
    {IR(52784, 13), 340}, // native_rsqrt in opencl_std_100
    {IR(52797, 11), 341}, // native_sin in opencl_std_100
    {IR(52808, 12), 342}, // native_sqrt in opencl_std_100
    {IR(52820, 11), 343}, // native_tan in opencl_std_100
    {IR(52402, 10), 296}, // nextafter in opencl_std_100
    {IR(52925, 10), 356}, // normalize in opencl_std_100
    {IR(53178, 9), 385}, // popcount in opencl_std_100
    {IR(52412, 4), 297}, // pow in opencl_std_100
    {IR(52416, 5), 298}, // pown in opencl_std_100
    {IR(52421, 5), 299}, // powr in opencl_std_100
    {IR(53378, 9), 404}, // prefetch in opencl_std_100
    {IR(53371, 7), 403}, // printf in opencl_std_100
    {IR(52874, 8), 349}, // radians in opencl_std_100
    {IR(52426, 10), 300}, // remainder in opencl_std_100
    {IR(52436, 7), 301}, // remquo in opencl_std_100
    {IR(52443, 5), 302}, // rint in opencl_std_100
    {IR(52448, 6), 303}, // rootn in opencl_std_100
    {IR(53129, 7), 380}, // rotate in opencl_std_100
    {IR(52454, 6), 304}, // round in opencl_std_100
    {IR(52460, 6), 305}, // rsqrt in opencl_std_100
    {IR(52976, 6), 360}, // s_abs in opencl_std_100
    {IR(52982, 11), 361}, // s_abs_diff in opencl_std_100
    {IR(52993, 10), 362}, // s_add_sat in opencl_std_100
    {IR(53043, 8), 368}, // s_clamp in opencl_std_100
    {IR(53013, 7), 364}, // s_hadd in opencl_std_100
    {IR(53187, 8), 386}, // s_mad24 in opencl_std_100
    {IR(53067, 9), 372}, // s_mad_hi in opencl_std_100
    {IR(53086, 10), 374}, // s_mad_sat in opencl_std_100
    {IR(53096, 6), 375}, // s_max in opencl_std_100
    {IR(53108, 6), 377}, // s_min in opencl_std_100
    {IR(53203, 8), 388}, // s_mul24 in opencl_std_100
    {IR(53120, 9), 379}, // s_mul_hi in opencl_std_100
    {IR(53027, 8), 366}, // s_rhadd in opencl_std_100
    {IR(53136, 10), 381}, // s_sub_sat in opencl_std_100
    {IR(53167, 11), 384}, // s_upsample in opencl_std_100
    {IR(53397, 7), 406}, // select in opencl_std_100
    {IR(53354, 8), 401}, // shuffle in opencl_std_100
    {IR(53362, 9), 402}, // shuffle2 in opencl_std_100
    {IR(52898, 5), 352}, // sign in opencl_std_100
    {IR(52466, 4), 306}, // sin in opencl_std_100
    {IR(52470, 7), 307}, // sincos in opencl_std_100
    {IR(52477, 5), 308}, // sinh in opencl_std_100
    {IR(52482, 6), 309}, // sinpi in opencl_std_100
    {IR(52887, 11), 351}, // smoothstep in opencl_std_100
    {IR(52488, 5), 310}, // sqrt in opencl_std_100
    {IR(52882, 5), 350}, // step in opencl_std_100
    {IR(52493, 4), 311}, // tan in opencl_std_100
    {IR(52497, 5), 312}, // tanh in opencl_std_100
    {IR(52502, 6), 313}, // tanpi in opencl_std_100
    {IR(52508, 7), 314}, // tgamma in opencl_std_100
    {IR(52515, 6), 315}, // trunc in opencl_std_100
    {IR(53404, 6), 407}, // u_abs in opencl_std_100
    {IR(53410, 11), 408}, // u_abs_diff in opencl_std_100
    {IR(53003, 10), 363}, // u_add_sat in opencl_std_100
    {IR(53051, 8), 369}, // u_clamp in opencl_std_100
    {IR(53020, 7), 365}, // u_hadd in opencl_std_100
    {IR(53195, 8), 387}, // u_mad24 in opencl_std_100
    {IR(53430, 9), 410}, // u_mad_hi in opencl_std_100
    {IR(53076, 10), 373}, // u_mad_sat in opencl_std_100
    {IR(53102, 6), 376}, // u_max in opencl_std_100
    {IR(53114, 6), 378}, // u_min in opencl_std_100
    {IR(53211, 8), 389}, // u_mul24 in opencl_std_100
    {IR(53421, 9), 409}, // u_mul_hi in opencl_std_100
    {IR(53035, 8), 367}, // u_rhadd in opencl_std_100
    {IR(53146, 10), 382}, // u_sub_sat in opencl_std_100
    {IR(53156, 11), 383}, // u_upsample in opencl_std_100
    {IR(53234, 11), 392}, // vload_half in opencl_std_100
    {IR(53245, 12), 393}, // vload_halfn in opencl_std_100
    {IR(53311, 13), 398}, // vloada_halfn in opencl_std_100
    {IR(53219, 7), 390}, // vloadn in opencl_std_100
    {IR(53257, 12), 394}, // vstore_half in opencl_std_100
    {IR(53269, 14), 395}, // vstore_half_r in opencl_std_100
    {IR(53283, 13), 396}, // vstore_halfn in opencl_std_100
    {IR(53296, 15), 397}, // vstore_halfn_r in opencl_std_100
    {IR(53324, 14), 399}, // vstorea_halfn in opencl_std_100
    {IR(53338, 16), 400}, // vstorea_halfn_r in opencl_std_100
    {IR(53226, 8), 391}, // vstoren in opencl_std_100
    {IR(53456, 17), 412}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(53439, 17), 411}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(53473, 8), 413}, // TimeAMD in spv_amd_gcn_shader
    {IR(53550, 9), 417}, // MbcntAMD in spv_amd_shader_ballot
    {IR(53481, 22), 414}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(53503, 28), 415}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(53531, 19), 416}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(53559, 23), 418}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(53609, 9), 422}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(53636, 9), 425}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(53582, 9), 419}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(53627, 9), 424}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(53654, 9), 427}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(53600, 9), 421}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(53618, 9), 423}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(53645, 9), 426}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(53591, 9), 420}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(53960, 4), 459}, // ABS in tosa_001000_1
    {IR(53783, 4), 442}, // ADD in tosa_001000_1
    {IR(53663, 7), 428}, // ARGMAX in tosa_001000_1
    {IR(53787, 23), 443}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(53670, 11), 429}, // AVG_POOL2D in tosa_001000_1
    {IR(53810, 12), 444}, // BITWISE_AND in tosa_001000_1
    {IR(53964, 12), 460}, // BITWISE_NOT in tosa_001000_1
    {IR(53822, 11), 445}, // BITWISE_OR in tosa_001000_1
    {IR(53833, 12), 446}, // BITWISE_XOR in tosa_001000_1
    {IR(54218, 5), 492}, // CAST in tosa_001000_1
    {IR(53976, 5), 461}, // CEIL in tosa_001000_1
    {IR(53760, 6), 438}, // CLAMP in tosa_001000_1
    {IR(53981, 4), 462}, // CLZ in tosa_001000_1
    {IR(54148, 7), 482}, // CONCAT in tosa_001000_1
    {IR(53681, 7), 430}, // CONV2D in tosa_001000_1
    {IR(53688, 7), 431}, // CONV3D in tosa_001000_1
    {IR(53985, 4), 463}, // COS in tosa_001000_1
    {IR(53695, 17), 432}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(54050, 6), 473}, // EQUAL in tosa_001000_1
    {IR(53766, 4), 439}, // ERF in tosa_001000_1
    {IR(53989, 4), 464}, // EXP in tosa_001000_1
    {IR(53712, 6), 433}, // FFT2D in tosa_001000_1
    {IR(53993, 6), 465}, // FLOOR in tosa_001000_1
    {IR(54196, 7), 489}, // GATHER in tosa_001000_1
    {IR(54056, 8), 474}, // GREATER in tosa_001000_1
    {IR(54064, 14), 475}, // GREATER_EQUAL in tosa_001000_1
    {IR(53845, 7), 447}, // INTDIV in tosa_001000_1
    {IR(53999, 4), 466}, // LOG in tosa_001000_1
    {IR(53852, 12), 448}, // LOGICAL_AND in tosa_001000_1
    {IR(53864, 19), 449}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(54003, 12), 467}, // LOGICAL_NOT in tosa_001000_1
    {IR(53903, 11), 451}, // LOGICAL_OR in tosa_001000_1
    {IR(53883, 20), 450}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(53914, 12), 452}, // LOGICAL_XOR in tosa_001000_1
    {IR(53718, 7), 434}, // MATMUL in tosa_001000_1
    {IR(53926, 8), 453}, // MAXIMUM in tosa_001000_1
    {IR(53725, 11), 435}, // MAX_POOL2D in tosa_001000_1
    {IR(53934, 8), 454}, // MINIMUM in tosa_001000_1
    {IR(53942, 4), 455}, // MUL in tosa_001000_1
    {IR(54015, 7), 468}, // NEGATE in tosa_001000_1
    {IR(54155, 4), 483}, // PAD in tosa_001000_1
    {IR(53946, 4), 456}, // POW in tosa_001000_1
    {IR(54022, 11), 469}, // RECIPROCAL in tosa_001000_1
    {IR(54078, 11), 476}, // REDUCE_ALL in tosa_001000_1
    {IR(54089, 11), 477}, // REDUCE_ANY in tosa_001000_1
    {IR(54100, 11), 478}, // REDUCE_MAX in tosa_001000_1
    {IR(54111, 11), 479}, // REDUCE_MIN in tosa_001000_1
    {IR(54122, 15), 480}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(54137, 11), 481}, // REDUCE_SUM in tosa_001000_1
    {IR(54223, 8), 493}, // RESCALE in tosa_001000_1
    {IR(54159, 8), 484}, // RESHAPE in tosa_001000_1
    {IR(54211, 7), 491}, // RESIZE in tosa_001000_1
    {IR(54167, 8), 485}, // REVERSE in tosa_001000_1
    {IR(53736, 7), 436}, // RFFT2D in tosa_001000_1
    {IR(54033, 6), 470}, // RSQRT in tosa_001000_1
    {IR(54203, 8), 490}, // SCATTER in tosa_001000_1
    {IR(54043, 7), 472}, // SELECT in tosa_001000_1
    {IR(53770, 8), 440}, // SIGMOID in tosa_001000_1
    {IR(54039, 4), 471}, // SIN in tosa_001000_1
    {IR(54175, 6), 486}, // SLICE in tosa_001000_1
    {IR(53950, 4), 457}, // SUB in tosa_001000_1
    {IR(53954, 6), 458}, // TABLE in tosa_001000_1
    {IR(53778, 5), 441}, // TANH in tosa_001000_1
    {IR(54181, 5), 487}, // TILE in tosa_001000_1
    {IR(54186, 10), 488}, // TRANSPOSE in tosa_001000_1
    {IR(53743, 17), 437}, // TRANSPOSE_CONV2D in tosa_001000_1
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
  "SPV_EXT_descriptor_indexing\0" // IR(1168, 28)
  "SPV_EXT_float8\0" // IR(1196, 15)
  "SPV_EXT_fragment_fully_covered\0" // IR(1211, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(1242, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(1278, 34)
  "SPV_EXT_mesh_shader\0" // IR(1312, 20)
  "SPV_EXT_opacity_micromap\0" // IR(1332, 25)
  "SPV_EXT_optnone\0" // IR(1357, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(1373, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(1405, 44)
  "SPV_EXT_replicated_composites\0" // IR(1449, 30)
  "SPV_EXT_shader_64bit_indexing\0" // IR(1479, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(1509, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(1543, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(1575, 36)
  "SPV_EXT_shader_image_int64\0" // IR(1611, 27)
  "SPV_EXT_shader_stencil_export\0" // IR(1638, 30)
  "SPV_EXT_shader_tile_image\0" // IR(1668, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1694, 36)
  "SPV_GOOGLE_decorate_string\0" // IR(1730, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1757, 31)
  "SPV_GOOGLE_user_type\0" // IR(1788, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1809, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(1831, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(1873, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(1918, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(1957, 30)
  "SPV_INTEL_bindless_images\0" // IR(1987, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(2013, 25)
  "SPV_INTEL_cache_controls\0" // IR(2038, 25)
  "SPV_INTEL_debug_module\0" // IR(2063, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(2086, 44)
  "SPV_INTEL_float_controls2\0" // IR(2130, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(2156, 28)
  "SPV_INTEL_fp_max_error\0" // IR(2184, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(2207, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(2242, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(2273, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(2307, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(2334, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(2382, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(2413, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(2442, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(2473, 33)
  "SPV_INTEL_fpga_reg\0" // IR(2506, 19)
  "SPV_INTEL_function_pointers\0" // IR(2525, 28)
  "SPV_INTEL_function_variants\0" // IR(2553, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(2581, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(2624, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2662, 26)
  "SPV_INTEL_int4\0" // IR(2688, 15)
  "SPV_INTEL_io_pipes\0" // IR(2703, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2722, 28)
  "SPV_INTEL_long_composites\0" // IR(2750, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2776, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2796, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2828, 28)
  "SPV_INTEL_media_block_io\0" // IR(2856, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2881, 33)
  "SPV_INTEL_optnone\0" // IR(2914, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(2932, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(2958, 36)
  "SPV_INTEL_split_barrier\0" // IR(2994, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3018, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3053, 46)
  "SPV_INTEL_subgroups\0" // IR(3099, 20)
  "SPV_INTEL_task_sequence\0" // IR(3119, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3143, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3179, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3214, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3251, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3281, 32)
  "SPV_INTEL_vector_compute\0" // IR(3313, 25)
  "SPV_KHR_16bit_storage\0" // IR(3338, 22)
  "SPV_KHR_8bit_storage\0" // IR(3360, 21)
  "SPV_KHR_bfloat16\0" // IR(3381, 17)
  "SPV_KHR_bit_instructions\0" // IR(3398, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3423, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(3458, 27)
  "SPV_KHR_device_group\0" // IR(3485, 21)
  "SPV_KHR_expect_assume\0" // IR(3506, 22)
  "SPV_KHR_float_controls\0" // IR(3528, 23)
  "SPV_KHR_float_controls2\0" // IR(3551, 24)
  "SPV_KHR_fma\0" // IR(3575, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3587, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3623, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3653, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3681, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3702, 30)
  "SPV_KHR_multiview\0" // IR(3732, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3750, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3785, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3811, 32)
  "SPV_KHR_post_depth_coverage\0" // IR(3843, 28)
  "SPV_KHR_quad_control\0" // IR(3871, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(3892, 22)
  "SPV_KHR_ray_query\0" // IR(3914, 18)
  "SPV_KHR_ray_tracing\0" // IR(3932, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(3952, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(3987, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4024, 34)
  "SPV_KHR_shader_ballot\0" // IR(4058, 22)
  "SPV_KHR_shader_clock\0" // IR(4080, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4101, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4132, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4169, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4193, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4231, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4253, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4282, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4317, 25)
  "SPV_KHR_variable_pointers\0" // IR(4342, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4368, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4396, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4437, 38)
  "SPV_NV_bindless_texture\0" // IR(4475, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4499, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4537, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4571, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4597, 27)
  "SPV_NV_cooperative_vector\0" // IR(4624, 26)
  "SPV_NV_displacement_micromap\0" // IR(4650, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(4679, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(4714, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(4749, 28)
  "SPV_NV_mesh_shader\0" // IR(4777, 19)
  "SPV_NV_raw_access_chains\0" // IR(4796, 25)
  "SPV_NV_ray_tracing\0" // IR(4821, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(4840, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(4871, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(4908, 33)
  "SPV_NV_shader_image_footprint\0" // IR(4941, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(4971, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5004, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5030, 35)
  "SPV_NV_shading_rate\0" // IR(5065, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5085, 29)
  "SPV_NV_tensor_addressing\0" // IR(5114, 25)
  "SPV_NV_viewport_array2\0" // IR(5139, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5162, 39)
  "SPV_QCOM_image_processing\0" // IR(5201, 26)
  "SPV_QCOM_image_processing2\0" // IR(5227, 27)
  "SPV_QCOM_tile_shading\0" // IR(5254, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5276, 38)
  "ReadOnly\0" // IR(5314, 9)
  "Kernel\0" // IR(5323, 7)
  "WriteOnly\0" // IR(5330, 10)
  "ReadWrite\0" // IR(5340, 10)
  "Logical\0" // IR(5350, 8)
  "Physical32\0" // IR(5358, 11)
  "Addresses\0" // IR(5369, 10)
  "Physical64\0" // IR(5379, 11)
  "PhysicalStorageBuffer64\0" // IR(5390, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(5414, 27)
  "PhysicalStorageBufferAddresses\0" // IR(5441, 31)
  "Position\0" // IR(5472, 9)
  "Shader\0" // IR(5481, 7)
  "PointSize\0" // IR(5488, 10)
  "ClipDistance\0" // IR(5498, 13)
  "CullDistance\0" // IR(5511, 13)
  "VertexId\0" // IR(5524, 9)
  "InstanceId\0" // IR(5533, 11)
  "PrimitiveId\0" // IR(5544, 12)
  "Geometry\0" // IR(5556, 9)
  "Tessellation\0" // IR(5565, 13)
  "RayTracingNV\0" // IR(5578, 13)
  "RayTracingKHR\0" // IR(5591, 14)
  "MeshShadingNV\0" // IR(5605, 14)
  "MeshShadingEXT\0" // IR(5619, 15)
  "InvocationId\0" // IR(5634, 13)
  "Layer\0" // IR(5647, 6)
  "ShaderLayer\0" // IR(5653, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(5665, 28)
  "ViewportIndex\0" // IR(5693, 14)
  "MultiViewport\0" // IR(5707, 14)
  "ShaderViewportIndex\0" // IR(5721, 20)
  "TessLevelOuter\0" // IR(5741, 15)
  "TessLevelInner\0" // IR(5756, 15)
  "TessCoord\0" // IR(5771, 10)
  "PatchVertices\0" // IR(5781, 14)
  "FragCoord\0" // IR(5795, 10)
  "PointCoord\0" // IR(5805, 11)
  "FrontFacing\0" // IR(5816, 12)
  "SampleId\0" // IR(5828, 9)
  "SampleRateShading\0" // IR(5837, 18)
  "SamplePosition\0" // IR(5855, 15)
  "SampleMask\0" // IR(5870, 11)
  "FragDepth\0" // IR(5881, 10)
  "HelperInvocation\0" // IR(5891, 17)
  "NumWorkgroups\0" // IR(5908, 14)
  "WorkgroupSize\0" // IR(5922, 14)
  "WorkgroupId\0" // IR(5936, 12)
  "LocalInvocationId\0" // IR(5948, 18)
  "GlobalInvocationId\0" // IR(5966, 19)
  "LocalInvocationIndex\0" // IR(5985, 21)
  "WorkDim\0" // IR(6006, 8)
  "GlobalSize\0" // IR(6014, 11)
  "EnqueuedWorkgroupSize\0" // IR(6025, 22)
  "GlobalOffset\0" // IR(6047, 13)
  "GlobalLinearId\0" // IR(6060, 15)
  "SubgroupSize\0" // IR(6075, 13)
  "GroupNonUniform\0" // IR(6088, 16)
  "SubgroupBallotKHR\0" // IR(6104, 18)
  "SubgroupMaxSize\0" // IR(6122, 16)
  "NumSubgroups\0" // IR(6138, 13)
  "NumEnqueuedSubgroups\0" // IR(6151, 21)
  "SubgroupId\0" // IR(6172, 11)
  "SubgroupLocalInvocationId\0" // IR(6183, 26)
  "VertexIndex\0" // IR(6209, 12)
  "InstanceIndex\0" // IR(6221, 14)
  "CoreIDARM\0" // IR(6235, 10)
  "CoreBuiltinsARM\0" // IR(6245, 16)
  "CoreCountARM\0" // IR(6261, 13)
  "CoreMaxIDARM\0" // IR(6274, 13)
  "WarpIDARM\0" // IR(6287, 10)
  "WarpMaxIDARM\0" // IR(6297, 13)
  "SubgroupEqMask\0" // IR(6310, 15)
  "SubgroupEqMaskKHR\0" // IR(6325, 18)
  "GroupNonUniformBallot\0" // IR(6343, 22)
  "SubgroupGeMask\0" // IR(6365, 15)
  "SubgroupGeMaskKHR\0" // IR(6380, 18)
  "SubgroupGtMask\0" // IR(6398, 15)
  "SubgroupGtMaskKHR\0" // IR(6413, 18)
  "SubgroupLeMask\0" // IR(6431, 15)
  "SubgroupLeMaskKHR\0" // IR(6446, 18)
  "SubgroupLtMask\0" // IR(6464, 15)
  "SubgroupLtMaskKHR\0" // IR(6479, 18)
  "BaseVertex\0" // IR(6497, 11)
  "DrawParameters\0" // IR(6508, 15)
  "BaseInstance\0" // IR(6523, 13)
  "DrawIndex\0" // IR(6536, 10)
  "PrimitiveShadingRateKHR\0" // IR(6546, 24)
  "FragmentShadingRateKHR\0" // IR(6570, 23)
  "DeviceIndex\0" // IR(6593, 12)
  "DeviceGroup\0" // IR(6605, 12)
  "ViewIndex\0" // IR(6617, 10)
  "MultiView\0" // IR(6627, 10)
  "ShadingRateKHR\0" // IR(6637, 15)
  "TileOffsetQCOM\0" // IR(6652, 15)
  "TileShadingQCOM\0" // IR(6667, 16)
  "TileDimensionQCOM\0" // IR(6683, 18)
  "TileApronSizeQCOM\0" // IR(6701, 18)
  "BaryCoordNoPerspAMD\0" // IR(6719, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(6739, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(6767, 26)
  "BaryCoordSmoothAMD\0" // IR(6793, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(6812, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(6839, 25)
  "BaryCoordPullModelAMD\0" // IR(6864, 22)
  "FragStencilRefEXT\0" // IR(6886, 18)
  "StencilExportEXT\0" // IR(6904, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(6921, 29)
  "ShaderEnqueueAMDX\0" // IR(6950, 18)
  "ShaderIndexAMDX\0" // IR(6968, 16)
  "ViewportMaskNV\0" // IR(6984, 15)
  "ShaderViewportMaskNV\0" // IR(6999, 21)
  "SecondaryPositionNV\0" // IR(7020, 20)
  "ShaderStereoViewNV\0" // IR(7040, 19)
  "SecondaryViewportMaskNV\0" // IR(7059, 24)
  "PositionPerViewNV\0" // IR(7083, 18)
  "PerViewAttributesNV\0" // IR(7101, 20)
  "ViewportMaskPerViewNV\0" // IR(7121, 22)
  "FullyCoveredEXT\0" // IR(7143, 16)
  "FragmentFullyCoveredEXT\0" // IR(7159, 24)
  "TaskCountNV\0" // IR(7183, 12)
  "PrimitiveCountNV\0" // IR(7195, 17)
  "PrimitiveIndicesNV\0" // IR(7212, 19)
  "ClipDistancePerViewNV\0" // IR(7231, 22)
  "CullDistancePerViewNV\0" // IR(7253, 22)
  "LayerPerViewNV\0" // IR(7275, 15)
  "MeshViewCountNV\0" // IR(7290, 16)
  "MeshViewIndicesNV\0" // IR(7306, 18)
  "BaryCoordKHR\0" // IR(7324, 13)
  "BaryCoordNV\0" // IR(7337, 12)
  "FragmentBarycentricKHR\0" // IR(7349, 23)
  "BaryCoordNoPerspKHR\0" // IR(7372, 20)
  "BaryCoordNoPerspNV\0" // IR(7392, 19)
  "FragSizeEXT\0" // IR(7411, 12)
  "FragmentSizeNV\0" // IR(7423, 15)
  "FragmentDensityEXT\0" // IR(7438, 19)
  "FragInvocationCountEXT\0" // IR(7457, 23)
  "InvocationsPerPixelNV\0" // IR(7480, 22)
  "PrimitivePointIndicesEXT\0" // IR(7502, 25)
  "PrimitiveLineIndicesEXT\0" // IR(7527, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(7551, 28)
  "CullPrimitiveEXT\0" // IR(7579, 17)
  "LaunchIdKHR\0" // IR(7596, 12)
  "LaunchIdNV\0" // IR(7608, 11)
  "LaunchSizeKHR\0" // IR(7619, 14)
  "LaunchSizeNV\0" // IR(7633, 13)
  "WorldRayOriginKHR\0" // IR(7646, 18)
  "WorldRayOriginNV\0" // IR(7664, 17)
  "WorldRayDirectionKHR\0" // IR(7681, 21)
  "WorldRayDirectionNV\0" // IR(7702, 20)
  "ObjectRayOriginKHR\0" // IR(7722, 19)
  "ObjectRayOriginNV\0" // IR(7741, 18)
  "ObjectRayDirectionKHR\0" // IR(7759, 22)
  "ObjectRayDirectionNV\0" // IR(7781, 21)
  "RayTminKHR\0" // IR(7802, 11)
  "RayTminNV\0" // IR(7813, 10)
  "RayTmaxKHR\0" // IR(7823, 11)
  "RayTmaxNV\0" // IR(7834, 10)
  "InstanceCustomIndexKHR\0" // IR(7844, 23)
  "InstanceCustomIndexNV\0" // IR(7867, 22)
  "ObjectToWorldKHR\0" // IR(7889, 17)
  "ObjectToWorldNV\0" // IR(7906, 16)
  "WorldToObjectKHR\0" // IR(7922, 17)
  "WorldToObjectNV\0" // IR(7939, 16)
  "HitTNV\0" // IR(7955, 7)
  "HitKindKHR\0" // IR(7962, 11)
  "HitKindNV\0" // IR(7973, 10)
  "CurrentRayTimeNV\0" // IR(7983, 17)
  "RayTracingMotionBlurNV\0" // IR(8000, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8023, 30)
  "RayTracingPositionFetchKHR\0" // IR(8053, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8080, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8114, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8147, 37)
  "IncomingRayFlagsKHR\0" // IR(8184, 20)
  "IncomingRayFlagsNV\0" // IR(8204, 19)
  "RayGeometryIndexKHR\0" // IR(8223, 20)
  "HitIsSphereNV\0" // IR(8243, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8257, 28)
  "HitIsLSSNV\0" // IR(8285, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8296, 39)
  "HitSpherePositionNV\0" // IR(8335, 20)
  "WarpsPerSMNV\0" // IR(8355, 13)
  "ShaderSMBuiltinsNV\0" // IR(8368, 19)
  "SMCountNV\0" // IR(8387, 10)
  "WarpIDNV\0" // IR(8397, 9)
  "SMIDNV\0" // IR(8406, 7)
  "HitLSSPositionsNV\0" // IR(8413, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(8431, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(8465, 33)
  "HitSphereRadiusNV\0" // IR(8498, 18)
  "HitLSSRadiiNV\0" // IR(8516, 14)
  "ClusterIDNV\0" // IR(8530, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(8542, 41)
  "CullMaskKHR\0" // IR(8583, 12)
  "RayCullMaskKHR\0" // IR(8595, 15)
  "Matrix\0" // IR(8610, 7)
  "Linkage\0" // IR(8617, 8)
  "Vector16\0" // IR(8625, 9)
  "Float16Buffer\0" // IR(8634, 14)
  "Float16\0" // IR(8648, 8)
  "Float64\0" // IR(8656, 8)
  "Int64\0" // IR(8664, 6)
  "Int64Atomics\0" // IR(8670, 13)
  "ImageBasic\0" // IR(8683, 11)
  "ImageReadWrite\0" // IR(8694, 15)
  "ImageMipmap\0" // IR(8709, 12)
  "Pipes\0" // IR(8721, 6)
  "Groups\0" // IR(8727, 7)
  "DeviceEnqueue\0" // IR(8734, 14)
  "LiteralSampler\0" // IR(8748, 15)
  "AtomicStorage\0" // IR(8763, 14)
  "Int16\0" // IR(8777, 6)
  "TessellationPointSize\0" // IR(8783, 22)
  "GeometryPointSize\0" // IR(8805, 18)
  "ImageGatherExtended\0" // IR(8823, 20)
  "StorageImageMultisample\0" // IR(8843, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(8867, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(8901, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(8934, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(8968, 33)
  "ImageCubeArray\0" // IR(9001, 15)
  "SampledCubeArray\0" // IR(9016, 17)
  "ImageRect\0" // IR(9033, 10)
  "SampledRect\0" // IR(9043, 12)
  "GenericPointer\0" // IR(9055, 15)
  "Int8\0" // IR(9070, 5)
  "InputAttachment\0" // IR(9075, 16)
  "SparseResidency\0" // IR(9091, 16)
  "MinLod\0" // IR(9107, 7)
  "Sampled1D\0" // IR(9114, 10)
  "Image1D\0" // IR(9124, 8)
  "SampledBuffer\0" // IR(9132, 14)
  "ImageBuffer\0" // IR(9146, 12)
  "ImageMSArray\0" // IR(9158, 13)
  "StorageImageExtendedFormats\0" // IR(9171, 28)
  "ImageQuery\0" // IR(9199, 11)
  "DerivativeControl\0" // IR(9210, 18)
  "InterpolationFunction\0" // IR(9228, 22)
  "TransformFeedback\0" // IR(9250, 18)
  "GeometryStreams\0" // IR(9268, 16)
  "StorageImageReadWithoutFormat\0" // IR(9284, 30)
  "StorageImageWriteWithoutFormat\0" // IR(9314, 31)
  "SubgroupDispatch\0" // IR(9345, 17)
  "NamedBarrier\0" // IR(9362, 13)
  "PipeStorage\0" // IR(9375, 12)
  "GroupNonUniformVote\0" // IR(9387, 20)
  "GroupNonUniformArithmetic\0" // IR(9407, 26)
  "GroupNonUniformShuffle\0" // IR(9433, 23)
  "GroupNonUniformShuffleRelative\0" // IR(9456, 31)
  "GroupNonUniformClustered\0" // IR(9487, 25)
  "GroupNonUniformQuad\0" // IR(9512, 20)
  "UniformDecoration\0" // IR(9532, 18)
  "TileImageColorReadAccessEXT\0" // IR(9550, 28)
  "TileImageDepthReadAccessEXT\0" // IR(9578, 28)
  "TileImageStencilReadAccessEXT\0" // IR(9606, 30)
  "TensorsARM\0" // IR(9636, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(9647, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(9684, 40)
  "GraphARM\0" // IR(9724, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(9733, 28)
  "Float8EXT\0" // IR(9761, 10)
  "Float8CooperativeMatrixEXT\0" // IR(9771, 27)
  "CooperativeMatrixKHR\0" // IR(9798, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(9819, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(9852, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(9895, 44)
  "SubgroupVoteKHR\0" // IR(9939, 16)
  "StorageBuffer16BitAccess\0" // IR(9955, 25)
  "StorageUniformBufferBlock16\0" // IR(9980, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10008, 35)
  "StorageUniform16\0" // IR(10043, 17)
  "StoragePushConstant16\0" // IR(10060, 22)
  "StorageInputOutput16\0" // IR(10082, 21)
  "VariablePointersStorageBuffer\0" // IR(10103, 30)
  "VariablePointers\0" // IR(10133, 17)
  "AtomicStorageOps\0" // IR(10150, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10167, 28)
  "StorageBuffer8BitAccess\0" // IR(10195, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10219, 34)
  "StoragePushConstant8\0" // IR(10253, 21)
  "DenormPreserve\0" // IR(10274, 15)
  "DenormFlushToZero\0" // IR(10289, 18)
  "SignedZeroInfNanPreserve\0" // IR(10307, 25)
  "RoundingModeRTE\0" // IR(10332, 16)
  "RoundingModeRTZ\0" // IR(10348, 16)
  "RayQueryProvisionalKHR\0" // IR(10364, 23)
  "RayQueryKHR\0" // IR(10387, 12)
  "UntypedPointersKHR\0" // IR(10399, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(10418, 32)
  "TextureSampleWeightedQCOM\0" // IR(10450, 26)
  "TextureBoxFilterQCOM\0" // IR(10476, 21)
  "TextureBlockMatchQCOM\0" // IR(10497, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(10519, 32)
  "TextureBlockMatch2QCOM\0" // IR(10551, 23)
  "Float16ImageAMD\0" // IR(10574, 16)
  "ImageGatherBiasLodAMD\0" // IR(10590, 22)
  "FragmentMaskAMD\0" // IR(10612, 16)
  "ImageReadWriteLodAMD\0" // IR(10628, 21)
  "Int64ImageEXT\0" // IR(10649, 14)
  "ShaderClockKHR\0" // IR(10663, 15)
  "QuadControlKHR\0" // IR(10678, 15)
  "Int4TypeINTEL\0" // IR(10693, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(10707, 27)
  "BFloat16TypeKHR\0" // IR(10734, 16)
  "BFloat16DotProductKHR\0" // IR(10750, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(10772, 29)
  "SampleMaskOverrideCoverageNV\0" // IR(10801, 29)
  "GeometryShaderPassthroughNV\0" // IR(10830, 28)
  "ShaderViewportIndexLayerNV\0" // IR(10858, 27)
  "ImageFootprintNV\0" // IR(10885, 17)
  "FragmentBarycentricNV\0" // IR(10902, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(10924, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(10955, 30)
  "ShadingRateNV\0" // IR(10985, 14)
  "GroupNonUniformPartitionedNV\0" // IR(10999, 29)
  "ShaderNonUniform\0" // IR(11028, 17)
  "ShaderNonUniformEXT\0" // IR(11045, 20)
  "RuntimeDescriptorArray\0" // IR(11065, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(11088, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(11114, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(11150, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(11189, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(11228, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(11270, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(11309, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(11351, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(11388, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(11428, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(11464, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(11503, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(11540, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(11580, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(11616, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(11655, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(11694, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(11736, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(11778, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(11823, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(11865, 45)
  "VulkanMemoryModel\0" // IR(11910, 18)
  "VulkanMemoryModelKHR\0" // IR(11928, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(11949, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(11978, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12010, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(12044, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(12076, 31)
  "RayTracingProvisionalKHR\0" // IR(12107, 25)
  "CooperativeMatrixNV\0" // IR(12132, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(12152, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(12185, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(12223, 32)
  "DemoteToHelperInvocation\0" // IR(12255, 25)
  "DemoteToHelperInvocationEXT\0" // IR(12280, 28)
  "DisplacementMicromapNV\0" // IR(12308, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(12331, 29)
  "ShaderInvocationReorderNV\0" // IR(12360, 26)
  "BindlessTextureNV\0" // IR(12386, 18)
  "RayQueryPositionFetchKHR\0" // IR(12404, 25)
  "CooperativeVectorNV\0" // IR(12429, 20)
  "AtomicFloat16VectorNV\0" // IR(12449, 22)
  "RawAccessChainsNV\0" // IR(12471, 18)
  "Shader64BitIndexingEXT\0" // IR(12489, 23)
  "CooperativeMatrixReductionsNV\0" // IR(12512, 30)
  "CooperativeMatrixConversionsNV\0" // IR(12542, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(12573, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(12613, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(12649, 30)
  "CooperativeVectorTrainingNV\0" // IR(12679, 28)
  "TensorAddressingNV\0" // IR(12707, 19)
  "SubgroupShuffleINTEL\0" // IR(12726, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(12747, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(12774, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(12800, 31)
  "RoundToInfinityINTEL\0" // IR(12831, 21)
  "FloatingPointModeINTEL\0" // IR(12852, 23)
  "IntegerFunctions2INTEL\0" // IR(12875, 23)
  "FunctionPointersINTEL\0" // IR(12898, 22)
  "IndirectReferencesINTEL\0" // IR(12920, 24)
  "AsmINTEL\0" // IR(12944, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(12953, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(12976, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(12999, 23)
  "VectorComputeINTEL\0" // IR(13022, 19)
  "VectorAnyINTEL\0" // IR(13041, 15)
  "ExpectAssumeKHR\0" // IR(13056, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(13072, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(13105, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(13143, 39)
  "VariableLengthArrayINTEL\0" // IR(13182, 25)
  "FunctionFloatControlINTEL\0" // IR(13207, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(13233, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(13260, 26)
  "FPFastMathModeINTEL\0" // IR(13286, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(13306, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(13339, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(13371, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(13409, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(13446, 30)
  "FPGALoopControlsALTERA\0" // IR(13476, 23)
  "FPGALoopControlsINTEL\0" // IR(13499, 22)
  "KernelAttributesINTEL\0" // IR(13521, 22)
  "FPGAKernelAttributesINTEL\0" // IR(13543, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(13569, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(13594, 24)
  "FPGAClusterAttributesALTERA\0" // IR(13618, 28)
  "FPGAClusterAttributesINTEL\0" // IR(13646, 27)
  "LoopFuseALTERA\0" // IR(13673, 15)
  "LoopFuseINTEL\0" // IR(13688, 14)
  "FPGADSPControlALTERA\0" // IR(13702, 21)
  "FPGADSPControlINTEL\0" // IR(13723, 20)
  "MemoryAccessAliasingINTEL\0" // IR(13743, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(13769, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(13810, 40)
  "FPGABufferLocationALTERA\0" // IR(13850, 25)
  "FPGABufferLocationINTEL\0" // IR(13875, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(13899, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(13934, 34)
  "USMStorageClassesALTERA\0" // IR(13968, 24)
  "USMStorageClassesINTEL\0" // IR(13992, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(14015, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(14045, 29)
  "IOPipesALTERA\0" // IR(14074, 14)
  "IOPipesINTEL\0" // IR(14088, 13)
  "BlockingPipesALTERA\0" // IR(14101, 20)
  "BlockingPipesINTEL\0" // IR(14121, 19)
  "FPGARegALTERA\0" // IR(14140, 14)
  "FPGARegINTEL\0" // IR(14154, 13)
  "DotProductInputAll\0" // IR(14167, 19)
  "DotProductInputAllKHR\0" // IR(14186, 22)
  "DotProductInput4x8Bit\0" // IR(14208, 22)
  "DotProductInput4x8BitKHR\0" // IR(14230, 25)
  "DotProductInput4x8BitPacked\0" // IR(14255, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(14283, 31)
  "DotProduct\0" // IR(14314, 11)
  "DotProductKHR\0" // IR(14325, 14)
  "ReplicatedCompositesEXT\0" // IR(14339, 24)
  "BitInstructions\0" // IR(14363, 16)
  "GroupNonUniformRotateKHR\0" // IR(14379, 25)
  "FloatControls2\0" // IR(14404, 15)
  "FMAKHR\0" // IR(14419, 7)
  "AtomicFloat32AddEXT\0" // IR(14426, 20)
  "AtomicFloat64AddEXT\0" // IR(14446, 20)
  "LongCompositesINTEL\0" // IR(14466, 20)
  "OptNoneEXT\0" // IR(14486, 11)
  "OptNoneINTEL\0" // IR(14497, 13)
  "AtomicFloat16AddEXT\0" // IR(14510, 20)
  "DebugInfoModuleINTEL\0" // IR(14530, 21)
  "BFloat16ConversionINTEL\0" // IR(14551, 24)
  "SplitBarrierINTEL\0" // IR(14575, 18)
  "ArithmeticFenceEXT\0" // IR(14593, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(14612, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(14642, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(14671, 28)
  "TaskSequenceALTERA\0" // IR(14699, 19)
  "TaskSequenceINTEL\0" // IR(14718, 18)
  "FPMaxErrorINTEL\0" // IR(14736, 16)
  "FPGALatencyControlALTERA\0" // IR(14752, 25)
  "FPGALatencyControlINTEL\0" // IR(14777, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(14801, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(14830, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(14858, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(14888, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(14924, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(14959, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(14987, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(15010, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(15040, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(15070, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(15108, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(15136, 32)
  "SpecConditionalINTEL\0" // IR(15168, 21)
  "FunctionVariantsINTEL\0" // IR(15189, 22)
  "GroupUniformArithmeticKHR\0" // IR(15211, 26)
  "TensorFloat32RoundingINTEL\0" // IR(15237, 27)
  "MaskedGatherScatterINTEL\0" // IR(15264, 25)
  "CacheControlsINTEL\0" // IR(15289, 19)
  "RegisterLimitsINTEL\0" // IR(15308, 20)
  "BindlessImagesINTEL\0" // IR(15328, 20)
  "Unspecified\0" // IR(15348, 12)
  "Address\0" // IR(15360, 8)
  "Boolean\0" // IR(15368, 8)
  "Float\0" // IR(15376, 6)
  "Signed\0" // IR(15382, 7)
  "SignedChar\0" // IR(15389, 11)
  "Unsigned\0" // IR(15400, 9)
  "UnsignedChar\0" // IR(15409, 13)
  "Class\0" // IR(15422, 6)
  "Structure\0" // IR(15428, 10)
  "Union\0" // IR(15438, 6)
  "ImportedModule\0" // IR(15444, 15)
  "ImportedDeclaration\0" // IR(15459, 20)
  "None\0" // IR(15479, 5)
  "FlagIsProtected\0" // IR(15484, 16)
  "FlagIsPrivate\0" // IR(15500, 14)
  "FlagIsPublic\0" // IR(15514, 13)
  "FlagIsLocal\0" // IR(15527, 12)
  "FlagIsDefinition\0" // IR(15539, 17)
  "FlagFwdDecl\0" // IR(15556, 12)
  "FlagArtificial\0" // IR(15568, 15)
  "FlagExplicit\0" // IR(15583, 13)
  "FlagPrototyped\0" // IR(15596, 15)
  "FlagObjectPointer\0" // IR(15611, 18)
  "FlagStaticMember\0" // IR(15629, 17)
  "FlagIndirectVariable\0" // IR(15646, 21)
  "FlagLValueReference\0" // IR(15667, 20)
  "FlagRValueReference\0" // IR(15687, 20)
  "FlagIsOptimized\0" // IR(15707, 16)
  "FlagIsEnumClass\0" // IR(15723, 16)
  "FlagTypePassByValue\0" // IR(15739, 20)
  "FlagTypePassByReference\0" // IR(15759, 24)
  "Deref\0" // IR(15783, 6)
  "Plus\0" // IR(15789, 5)
  "Minus\0" // IR(15794, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(15800, 33)
  "PlusUconst\0" // IR(15833, 11)
  "BitPiece\0" // IR(15844, 9)
  "Swap\0" // IR(15853, 5)
  "Xderef\0" // IR(15858, 7)
  "StackValue\0" // IR(15865, 11)
  "Constu\0" // IR(15876, 7)
  "Fragment\0" // IR(15883, 9)
  "ConstType\0" // IR(15892, 10)
  "VolatileType\0" // IR(15902, 13)
  "RestrictType\0" // IR(15915, 13)
  "AtomicType\0" // IR(15928, 11)
  "Float16NV\0" // IR(15939, 10)
  "Float32NV\0" // IR(15949, 10)
  "Float64NV\0" // IR(15959, 10)
  "SignedInt8NV\0" // IR(15969, 13)
  "SignedInt16NV\0" // IR(15982, 14)
  "SignedInt32NV\0" // IR(15996, 14)
  "SignedInt64NV\0" // IR(16010, 14)
  "UnsignedInt8NV\0" // IR(16024, 15)
  "UnsignedInt16NV\0" // IR(16039, 16)
  "UnsignedInt32NV\0" // IR(16055, 16)
  "UnsignedInt64NV\0" // IR(16071, 16)
  "SignedInt8PackedNV\0" // IR(16087, 19)
  "UnsignedInt8PackedNV\0" // IR(16106, 21)
  "FloatE4M3NV\0" // IR(16127, 12)
  "FloatE5M2NV\0" // IR(16139, 12)
  "RowMajorKHR\0" // IR(16151, 12)
  "ColumnMajorKHR\0" // IR(16163, 15)
  "RowBlockedInterleavedARM\0" // IR(16178, 25)
  "ColumnBlockedInterleavedARM\0" // IR(16203, 28)
  "NoneKHR\0" // IR(16231, 8)
  "MatrixASignedComponentsKHR\0" // IR(16239, 27)
  "MatrixBSignedComponentsKHR\0" // IR(16266, 27)
  "MatrixCSignedComponentsKHR\0" // IR(16293, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(16320, 32)
  "SaturatingAccumulationKHR\0" // IR(16352, 26)
  "Row\0" // IR(16378, 4)
  "Column\0" // IR(16382, 7)
  "2x2\0" // IR(16389, 4)
  "MatrixAKHR\0" // IR(16393, 11)
  "MatrixBKHR\0" // IR(16404, 11)
  "MatrixAccumulatorKHR\0" // IR(16415, 21)
  "RowMajorNV\0" // IR(16436, 11)
  "ColumnMajorNV\0" // IR(16447, 14)
  "InferencingOptimalNV\0" // IR(16461, 21)
  "TrainingOptimalNV\0" // IR(16482, 18)
  "RelaxedPrecision\0" // IR(16500, 17)
  "SpecId\0" // IR(16517, 7)
  "Block\0" // IR(16524, 6)
  "BufferBlock\0" // IR(16530, 12)
  "RowMajor\0" // IR(16542, 9)
  "ColMajor\0" // IR(16551, 9)
  "ArrayStride\0" // IR(16560, 12)
  "MatrixStride\0" // IR(16572, 13)
  "GLSLShared\0" // IR(16585, 11)
  "GLSLPacked\0" // IR(16596, 11)
  "CPacked\0" // IR(16607, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(16615, 26)
  "BuiltIn\0" // IR(16641, 8)
  "NoPerspective\0" // IR(16649, 14)
  "Flat\0" // IR(16663, 5)
  "Patch\0" // IR(16668, 6)
  "Centroid\0" // IR(16674, 9)
  "Sample\0" // IR(16683, 7)
  "Invariant\0" // IR(16690, 10)
  "Restrict\0" // IR(16700, 9)
  "Aliased\0" // IR(16709, 8)
  "Volatile\0" // IR(16717, 9)
  "Constant\0" // IR(16726, 9)
  "Coherent\0" // IR(16735, 9)
  "NonWritable\0" // IR(16744, 12)
  "NonReadable\0" // IR(16756, 12)
  "Uniform\0" // IR(16768, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(16776, 26)
  "UniformId\0" // IR(16802, 10)
  "SaturatedConversion\0" // IR(16812, 20)
  "Stream\0" // IR(16832, 7)
  "Location\0" // IR(16839, 9)
  "Component\0" // IR(16848, 10)
  "Index\0" // IR(16858, 6)
  "Binding\0" // IR(16864, 8)
  "DescriptorSet\0" // IR(16872, 14)
  "Offset\0" // IR(16886, 7)
  "XfbBuffer\0" // IR(16893, 10)
  "XfbStride\0" // IR(16903, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(16913, 46)
  "FuncParamAttr\0" // IR(16959, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(16973, 34)
  "FPRoundingMode\0" // IR(17007, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(17022, 35)
  "FPFastMathMode\0" // IR(17057, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(17072, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(17104, 30)
  "LinkageAttributes\0" // IR(17134, 18)
  "NoContraction\0" // IR(17152, 14)
  "InputAttachmentIndex\0" // IR(17166, 21)
  "Alignment\0" // IR(17187, 10)
  "MaxByteOffset\0" // IR(17197, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(17211, 20)
  "AlignmentId\0" // IR(17231, 12)
  "MaxByteOffsetId\0" // IR(17243, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(17259, 44)
  "NoSignedWrap\0" // IR(17303, 13)
  "NoUnsignedWrap\0" // IR(17316, 15)
  "WeightTextureQCOM\0" // IR(17331, 18)
  "BlockMatchTextureQCOM\0" // IR(17349, 22)
  "BlockMatchSamplerQCOM\0" // IR(17371, 22)
  "ExplicitInterpAMD\0" // IR(17393, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(17411, 32)
  "NodeMaxPayloadsAMDX\0" // IR(17443, 20)
  "TrackFinishWritingAMDX\0" // IR(17463, 23)
  "PayloadNodeNameAMDX\0" // IR(17486, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(17506, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(17531, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(17558, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(17583, 28)
  "OverrideCoverageNV\0" // IR(17611, 19)
  "PassthroughNV\0" // IR(17630, 14)
  "ViewportRelativeNV\0" // IR(17644, 19)
  "SecondaryViewportRelativeNV\0" // IR(17663, 28)
  "PerPrimitiveEXT\0" // IR(17691, 16)
  "PerPrimitiveNV\0" // IR(17707, 15)
  "PerViewNV\0" // IR(17722, 10)
  "PerTaskNV\0" // IR(17732, 10)
  "PerVertexKHR\0" // IR(17742, 13)
  "PerVertexNV\0" // IR(17755, 12)
  "NonUniform\0" // IR(17767, 11)
  "NonUniformEXT\0" // IR(17778, 14)
  "RestrictPointer\0" // IR(17792, 16)
  "RestrictPointerEXT\0" // IR(17808, 19)
  "AliasedPointer\0" // IR(17827, 15)
  "AliasedPointerEXT\0" // IR(17842, 18)
  "HitObjectShaderRecordBufferNV\0" // IR(17860, 30)
  "BindlessSamplerNV\0" // IR(17890, 18)
  "BindlessImageNV\0" // IR(17908, 16)
  "BoundSamplerNV\0" // IR(17924, 15)
  "BoundImageNV\0" // IR(17939, 13)
  "SIMTCallINTEL\0" // IR(17952, 14)
  "ReferencedIndirectlyINTEL\0" // IR(17966, 26)
  "ClobberINTEL\0" // IR(17992, 13)
  "SideEffectsINTEL\0" // IR(18005, 17)
  "VectorComputeVariableINTEL\0" // IR(18022, 27)
  "FuncParamIOKindINTEL\0" // IR(18049, 21)
  "VectorComputeFunctionINTEL\0" // IR(18070, 27)
  "StackCallINTEL\0" // IR(18097, 15)
  "GlobalVariableOffsetINTEL\0" // IR(18112, 26)
  "CounterBuffer\0" // IR(18138, 14)
  "HlslCounterBufferGOOGLE\0" // IR(18152, 24)
  "UserSemantic\0" // IR(18176, 13)
  "HlslSemanticGOOGLE\0" // IR(18189, 19)
  "UserTypeGOOGLE\0" // IR(18208, 15)
  "FunctionRoundingModeINTEL\0" // IR(18223, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(18249, 31)
  "FunctionDenormModeINTEL\0" // IR(18280, 24)
  "RegisterALTERA\0" // IR(18304, 15)
  "RegisterINTEL\0" // IR(18319, 14)
  "MemoryALTERA\0" // IR(18333, 13)
  "MemoryINTEL\0" // IR(18346, 12)
  "NumbanksALTERA\0" // IR(18358, 15)
  "NumbanksINTEL\0" // IR(18373, 14)
  "BankwidthALTERA\0" // IR(18387, 16)
  "BankwidthINTEL\0" // IR(18403, 15)
  "MaxPrivateCopiesALTERA\0" // IR(18418, 23)
  "MaxPrivateCopiesINTEL\0" // IR(18441, 22)
  "SinglepumpALTERA\0" // IR(18463, 17)
  "SinglepumpINTEL\0" // IR(18480, 16)
  "DoublepumpALTERA\0" // IR(18496, 17)
  "DoublepumpINTEL\0" // IR(18513, 16)
  "MaxReplicatesALTERA\0" // IR(18529, 20)
  "MaxReplicatesINTEL\0" // IR(18549, 19)
  "SimpleDualPortALTERA\0" // IR(18568, 21)
  "SimpleDualPortINTEL\0" // IR(18589, 20)
  "MergeALTERA\0" // IR(18609, 12)
  "MergeINTEL\0" // IR(18621, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(18632, 42)
  "BankBitsALTERA\0" // IR(18674, 15)
  "BankBitsINTEL\0" // IR(18689, 14)
  "ForcePow2DepthALTERA\0" // IR(18703, 21)
  "ForcePow2DepthINTEL\0" // IR(18724, 20)
  "StridesizeALTERA\0" // IR(18744, 17)
  "StridesizeINTEL\0" // IR(18761, 16)
  "WordsizeALTERA\0" // IR(18777, 15)
  "WordsizeINTEL\0" // IR(18792, 14)
  "TrueDualPortALTERA\0" // IR(18806, 19)
  "TrueDualPortINTEL\0" // IR(18825, 18)
  "BurstCoalesceALTERA\0" // IR(18843, 20)
  "BurstCoalesceINTEL\0" // IR(18863, 19)
  "CacheSizeALTERA\0" // IR(18882, 16)
  "CacheSizeINTEL\0" // IR(18898, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(18913, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(18942, 28)
  "PrefetchALTERA\0" // IR(18970, 15)
  "PrefetchINTEL\0" // IR(18985, 14)
  "StallEnableALTERA\0" // IR(18999, 18)
  "StallEnableINTEL\0" // IR(19017, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(19034, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(19060, 25)
  "MathOpDSPModeALTERA\0" // IR(19085, 20)
  "MathOpDSPModeINTEL\0" // IR(19105, 19)
  "AliasScopeINTEL\0" // IR(19124, 16)
  "NoAliasINTEL\0" // IR(19140, 13)
  "InitiationIntervalALTERA\0" // IR(19153, 25)
  "InitiationIntervalINTEL\0" // IR(19178, 24)
  "MaxConcurrencyALTERA\0" // IR(19202, 21)
  "MaxConcurrencyINTEL\0" // IR(19223, 20)
  "PipelineEnableALTERA\0" // IR(19243, 21)
  "PipelineEnableINTEL\0" // IR(19264, 20)
  "BufferLocationALTERA\0" // IR(19284, 21)
  "BufferLocationINTEL\0" // IR(19305, 20)
  "IOPipeStorageALTERA\0" // IR(19325, 20)
  "IOPipeStorageINTEL\0" // IR(19345, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(19364, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(19398, 31)
  "SingleElementVectorINTEL\0" // IR(19429, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(19454, 35)
  "MediaBlockIOINTEL\0" // IR(19489, 18)
  "StallFreeALTERA\0" // IR(19507, 16)
  "StallFreeINTEL\0" // IR(19523, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(19538, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(19569, 26)
  "LatencyControlLabelALTERA\0" // IR(19595, 26)
  "LatencyControlLabelINTEL\0" // IR(19621, 25)
  "LatencyControlConstraintALTERA\0" // IR(19646, 31)
  "LatencyControlConstraintINTEL\0" // IR(19677, 30)
  "ConduitKernelArgumentALTERA\0" // IR(19707, 28)
  "ConduitKernelArgumentINTEL\0" // IR(19735, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(19762, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(19794, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(19825, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(19859, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(19892, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(19923, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(19953, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(19982, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(20010, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(20044, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(20079, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(20113, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(20143, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(20172, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(20205, 32)
  "StableKernelArgumentALTERA\0" // IR(20237, 27)
  "StableKernelArgumentINTEL\0" // IR(20264, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(20290, 39)
  "HostAccessINTEL\0" // IR(20329, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(20345, 47)
  "InitModeALTERA\0" // IR(20392, 15)
  "InitModeINTEL\0" // IR(20407, 14)
  "ImplementInRegisterMapALTERA\0" // IR(20421, 29)
  "ImplementInRegisterMapINTEL\0" // IR(20450, 28)
  "ConditionalINTEL\0" // IR(20478, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(20495, 36)
  "CacheControlLoadINTEL\0" // IR(20531, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(20553, 37)
  "CacheControlStoreINTEL\0" // IR(20590, 23)
  "1D\0" // IR(20613, 3)
  "2D\0" // IR(20616, 3)
  "3D\0" // IR(20619, 3)
  "Cube\0" // IR(20622, 5)
  "Rect\0" // IR(20627, 5)
  "Buffer\0" // IR(20632, 7)
  "SubpassData\0" // IR(20639, 12)
  "TileImageDataEXT\0" // IR(20651, 17)
  "Invocations\0" // IR(20668, 12)
  "SpacingEqual\0" // IR(20680, 13)
  "SpacingFractionalEven\0" // IR(20693, 22)
  "SpacingFractionalOdd\0" // IR(20715, 21)
  "VertexOrderCw\0" // IR(20736, 14)
  "VertexOrderCcw\0" // IR(20750, 15)
  "PixelCenterInteger\0" // IR(20765, 19)
  "OriginUpperLeft\0" // IR(20784, 16)
  "OriginLowerLeft\0" // IR(20800, 16)
  "EarlyFragmentTests\0" // IR(20816, 19)
  "PointMode\0" // IR(20835, 10)
  "Xfb\0" // IR(20845, 4)
  "DepthReplacing\0" // IR(20849, 15)
  "DepthGreater\0" // IR(20864, 13)
  "DepthLess\0" // IR(20877, 10)
  "DepthUnchanged\0" // IR(20887, 15)
  "LocalSize\0" // IR(20902, 10)
  "LocalSizeHint\0" // IR(20912, 14)
  "InputPoints\0" // IR(20926, 12)
  "InputLines\0" // IR(20938, 11)
  "InputLinesAdjacency\0" // IR(20949, 20)
  "Triangles\0" // IR(20969, 10)
  "InputTrianglesAdjacency\0" // IR(20979, 24)
  "Quads\0" // IR(21003, 6)
  "Isolines\0" // IR(21009, 9)
  "OutputVertices\0" // IR(21018, 15)
  "OutputPoints\0" // IR(21033, 13)
  "OutputLineStrip\0" // IR(21046, 16)
  "OutputTriangleStrip\0" // IR(21062, 20)
  "VecTypeHint\0" // IR(21082, 12)
  "ContractionOff\0" // IR(21094, 15)
  "Initializer\0" // IR(21109, 12)
  "Finalizer\0" // IR(21121, 10)
  "SubgroupsPerWorkgroup\0" // IR(21131, 22)
  "SubgroupsPerWorkgroupId\0" // IR(21153, 24)
  "LocalSizeId\0" // IR(21177, 12)
  "LocalSizeHintId\0" // IR(21189, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(21205, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(21239, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(21273, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(21309, 30)
  "PostDepthCoverage\0" // IR(21339, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(21357, 34)
  "TileShadingRateQCOM\0" // IR(21391, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(21411, 29)
  "StencilRefReplacingEXT\0" // IR(21440, 23)
  "CoalescingAMDX\0" // IR(21463, 15)
  "IsApiEntryAMDX\0" // IR(21478, 15)
  "MaxNodeRecursionAMDX\0" // IR(21493, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(21514, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(21538, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(21559, 28)
  "StencilRefGreaterFrontAMD\0" // IR(21587, 26)
  "StencilRefLessFrontAMD\0" // IR(21613, 23)
  "StencilRefUnchangedBackAMD\0" // IR(21636, 27)
  "StencilRefGreaterBackAMD\0" // IR(21663, 25)
  "StencilRefLessBackAMD\0" // IR(21688, 22)
  "QuadDerivativesKHR\0" // IR(21710, 19)
  "RequireFullQuadsKHR\0" // IR(21729, 20)
  "SharesInputWithAMDX\0" // IR(21749, 20)
  "OutputLinesEXT\0" // IR(21769, 15)
  "OutputLinesNV\0" // IR(21784, 14)
  "OutputPrimitivesEXT\0" // IR(21798, 20)
  "OutputPrimitivesNV\0" // IR(21818, 19)
  "DerivativeGroupQuadsKHR\0" // IR(21837, 24)
  "DerivativeGroupQuadsNV\0" // IR(21861, 23)
  "DerivativeGroupLinearKHR\0" // IR(21884, 25)
  "DerivativeGroupLinearNV\0" // IR(21909, 24)
  "OutputTrianglesEXT\0" // IR(21933, 19)
  "OutputTrianglesNV\0" // IR(21952, 18)
  "PixelInterlockOrderedEXT\0" // IR(21970, 25)
  "PixelInterlockUnorderedEXT\0" // IR(21995, 27)
  "SampleInterlockOrderedEXT\0" // IR(22022, 26)
  "SampleInterlockUnorderedEXT\0" // IR(22048, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(22076, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(22107, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(22140, 27)
  "RoundingModeRTPINTEL\0" // IR(22167, 21)
  "RoundingModeRTNINTEL\0" // IR(22188, 21)
  "FloatingPointModeALTINTEL\0" // IR(22209, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(22235, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(22262, 22)
  "MaxWorkDimINTEL\0" // IR(22284, 16)
  "NoGlobalOffsetINTEL\0" // IR(22300, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(22320, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(22342, 28)
  "MaximallyReconvergesKHR\0" // IR(22370, 24)
  "FPFastMathDefault\0" // IR(22394, 18)
  "StreamingInterfaceINTEL\0" // IR(22412, 24)
  "RegisterMapInterfaceINTEL\0" // IR(22436, 26)
  "NamedBarrierCountINTEL\0" // IR(22462, 23)
  "MaximumRegistersINTEL\0" // IR(22485, 22)
  "MaximumRegistersIdINTEL\0" // IR(22507, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(22531, 51)
  "NamedMaximumRegistersINTEL\0" // IR(22582, 27)
  "Vertex\0" // IR(22609, 7)
  "TessellationControl\0" // IR(22616, 20)
  "TessellationEvaluation\0" // IR(22636, 23)
  "GLCompute\0" // IR(22659, 10)
  "TaskNV\0" // IR(22669, 7)
  "MeshNV\0" // IR(22676, 7)
  "RayGenerationKHR\0" // IR(22683, 17)
  "RayGenerationNV\0" // IR(22700, 16)
  "IntersectionKHR\0" // IR(22716, 16)
  "IntersectionNV\0" // IR(22732, 15)
  "AnyHitKHR\0" // IR(22747, 10)
  "AnyHitNV\0" // IR(22757, 9)
  "ClosestHitKHR\0" // IR(22766, 14)
  "ClosestHitNV\0" // IR(22780, 13)
  "MissKHR\0" // IR(22793, 8)
  "MissNV\0" // IR(22801, 7)
  "CallableKHR\0" // IR(22808, 12)
  "CallableNV\0" // IR(22820, 11)
  "TaskEXT\0" // IR(22831, 8)
  "MeshEXT\0" // IR(22839, 8)
  "Preserve\0" // IR(22847, 9)
  "FlushToZero\0" // IR(22856, 12)
  "BFloat16KHR\0" // IR(22868, 12)
  "Float8E4M3EXT\0" // IR(22880, 14)
  "Float8E5M2EXT\0" // IR(22894, 14)
  "IEEE\0" // IR(22908, 5)
  "ALT\0" // IR(22913, 4)
  "NotNaN\0" // IR(22917, 7)
  "NotInf\0" // IR(22924, 7)
  "NSZ\0" // IR(22931, 4)
  "AllowRecip\0" // IR(22935, 11)
  "Fast\0" // IR(22946, 5)
  "AllowContract\0" // IR(22951, 14)
  "AllowContractFastINTEL\0" // IR(22965, 23)
  "AllowReassoc\0" // IR(22988, 13)
  "AllowReassocINTEL\0" // IR(23001, 18)
  "AllowTransform\0" // IR(23019, 15)
  "RTE\0" // IR(23034, 4)
  "RTZ\0" // IR(23038, 4)
  "RTP\0" // IR(23042, 4)
  "RTN\0" // IR(23046, 4)
  "Vertical2Pixels\0" // IR(23050, 16)
  "Vertical4Pixels\0" // IR(23066, 16)
  "Horizontal2Pixels\0" // IR(23082, 18)
  "Horizontal4Pixels\0" // IR(23100, 18)
  "Inline\0" // IR(23118, 7)
  "DontInline\0" // IR(23125, 11)
  "Pure\0" // IR(23136, 5)
  "Const\0" // IR(23141, 6)
  "Zext\0" // IR(23147, 5)
  "Sext\0" // IR(23152, 5)
  "ByVal\0" // IR(23157, 6)
  "Sret\0" // IR(23163, 5)
  "NoAlias\0" // IR(23168, 8)
  "NoCapture\0" // IR(23176, 10)
  "NoWrite\0" // IR(23186, 8)
  "NoReadWrite\0" // IR(23194, 12)
  "RuntimeAlignedALTERA\0" // IR(23206, 21)
  "RuntimeAlignedINTEL\0" // IR(23227, 20)
  "Reduce\0" // IR(23247, 7)
  "InclusiveScan\0" // IR(23254, 14)
  "ExclusiveScan\0" // IR(23268, 14)
  "ClusteredReduce\0" // IR(23282, 16)
  "PartitionedReduceNV\0" // IR(23298, 20)
  "PartitionedInclusiveScanNV\0" // IR(23318, 27)
  "PartitionedExclusiveScanNV\0" // IR(23345, 27)
  "NoneINTEL\0" // IR(23372, 10)
  "ReadINTEL\0" // IR(23382, 10)
  "WriteINTEL\0" // IR(23392, 11)
  "ReadWriteINTEL\0" // IR(23403, 15)
  "Bias\0" // IR(23418, 5)
  "Lod\0" // IR(23423, 4)
  "Grad\0" // IR(23427, 5)
  "ConstOffset\0" // IR(23432, 12)
  "ConstOffsets\0" // IR(23444, 13)
  "MakeTexelAvailable\0" // IR(23457, 19)
  "MakeTexelAvailableKHR\0" // IR(23476, 22)
  "MakeTexelVisible\0" // IR(23498, 17)
  "MakeTexelVisibleKHR\0" // IR(23515, 20)
  "NonPrivateTexel\0" // IR(23535, 16)
  "NonPrivateTexelKHR\0" // IR(23551, 19)
  "VolatileTexel\0" // IR(23570, 14)
  "VolatileTexelKHR\0" // IR(23584, 17)
  "SignExtend\0" // IR(23601, 11)
  "ZeroExtend\0" // IR(23612, 11)
  "Nontemporal\0" // IR(23623, 12)
  "Offsets\0" // IR(23635, 8)
  "SnormInt8\0" // IR(23643, 10)
  "SnormInt16\0" // IR(23653, 11)
  "UnormInt8\0" // IR(23664, 10)
  "UnormInt16\0" // IR(23674, 11)
  "UnormShort565\0" // IR(23685, 14)
  "UnormShort555\0" // IR(23699, 14)
  "UnormInt101010\0" // IR(23713, 15)
  "SignedInt8\0" // IR(23728, 11)
  "SignedInt16\0" // IR(23739, 12)
  "SignedInt32\0" // IR(23751, 12)
  "UnsignedInt8\0" // IR(23763, 13)
  "UnsignedInt16\0" // IR(23776, 14)
  "UnsignedInt32\0" // IR(23790, 14)
  "HalfFloat\0" // IR(23804, 10)
  "UnormInt24\0" // IR(23814, 11)
  "UnormInt101010_2\0" // IR(23825, 17)
  "UnormInt10X6EXT\0" // IR(23842, 16)
  "UnsignedIntRaw10EXT\0" // IR(23858, 20)
  "UnsignedIntRaw12EXT\0" // IR(23878, 20)
  "UnormInt2_101010EXT\0" // IR(23898, 20)
  "UnsignedInt10X6EXT\0" // IR(23918, 19)
  "UnsignedInt12X4EXT\0" // IR(23937, 19)
  "UnsignedInt14X2EXT\0" // IR(23956, 19)
  "UnormInt12X4EXT\0" // IR(23975, 16)
  "UnormInt14X2EXT\0" // IR(23991, 16)
  "R\0" // IR(24007, 2)
  "A\0" // IR(24009, 2)
  "RG\0" // IR(24011, 3)
  "RA\0" // IR(24014, 3)
  "RGB\0" // IR(24017, 4)
  "RGBA\0" // IR(24021, 5)
  "BGRA\0" // IR(24026, 5)
  "ARGB\0" // IR(24031, 5)
  "Intensity\0" // IR(24036, 10)
  "Luminance\0" // IR(24046, 10)
  "Rx\0" // IR(24056, 3)
  "RGx\0" // IR(24059, 4)
  "RGBx\0" // IR(24063, 5)
  "Depth\0" // IR(24068, 6)
  "DepthStencil\0" // IR(24074, 13)
  "sRGB\0" // IR(24087, 5)
  "sRGBx\0" // IR(24092, 6)
  "sRGBA\0" // IR(24098, 6)
  "sBGRA\0" // IR(24104, 6)
  "ABGR\0" // IR(24110, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(24115, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(24143, 27)
  "InitOnDeviceResetALTERA\0" // IR(24170, 24)
  "InitOnDeviceResetINTEL\0" // IR(24194, 23)
  "NoWait\0" // IR(24217, 7)
  "WaitKernel\0" // IR(24224, 11)
  "WaitWorkGroup\0" // IR(24235, 14)
  "CmdExecTime\0" // IR(24249, 12)
  "MayUsePrintf\0" // IR(24261, 13)
  "Export\0" // IR(24274, 7)
  "Import\0" // IR(24281, 7)
  "LinkOnceODR\0" // IR(24288, 12)
  "UncachedINTEL\0" // IR(24300, 14)
  "CachedINTEL\0" // IR(24314, 12)
  "StreamingINTEL\0" // IR(24326, 15)
  "InvalidateAfterReadINTEL\0" // IR(24341, 25)
  "ConstCachedINTEL\0" // IR(24366, 17)
  "Unroll\0" // IR(24383, 7)
  "DontUnroll\0" // IR(24390, 11)
  "DependencyInfinite\0" // IR(24401, 19)
  "DependencyLength\0" // IR(24420, 17)
  "MinIterations\0" // IR(24437, 14)
  "MaxIterations\0" // IR(24451, 14)
  "IterationMultiple\0" // IR(24465, 18)
  "PeelCount\0" // IR(24483, 10)
  "PartialCount\0" // IR(24493, 13)
  "DependencyArrayALTERA\0" // IR(24506, 22)
  "DependencyArrayINTEL\0" // IR(24528, 21)
  "LoopCoalesceALTERA\0" // IR(24549, 19)
  "LoopCoalesceINTEL\0" // IR(24568, 18)
  "MaxInterleavingALTERA\0" // IR(24586, 22)
  "MaxInterleavingINTEL\0" // IR(24608, 21)
  "SpeculatedIterationsALTERA\0" // IR(24629, 27)
  "SpeculatedIterationsINTEL\0" // IR(24656, 26)
  "NoFusionALTERA\0" // IR(24682, 15)
  "NoFusionINTEL\0" // IR(24697, 14)
  "LoopCountALTERA\0" // IR(24711, 16)
  "LoopCountINTEL\0" // IR(24727, 15)
  "MaxReinvocationDelayALTERA\0" // IR(24742, 27)
  "MaxReinvocationDelayINTEL\0" // IR(24769, 26)
  "MatrixASignedComponentsINTEL\0" // IR(24795, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(24824, 29)
  "MatrixCBFloat16INTEL\0" // IR(24853, 21)
  "MatrixResultBFloat16INTEL\0" // IR(24874, 26)
  "MatrixAPackedInt8INTEL\0" // IR(24900, 23)
  "MatrixBPackedInt8INTEL\0" // IR(24923, 23)
  "MatrixAPackedInt4INTEL\0" // IR(24946, 23)
  "MatrixBPackedInt4INTEL\0" // IR(24969, 23)
  "MatrixATF32INTEL\0" // IR(24992, 17)
  "MatrixBTF32INTEL\0" // IR(25009, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(25026, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(25052, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(25078, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(25105, 27)
  "Aligned\0" // IR(25132, 8)
  "MakePointerAvailable\0" // IR(25140, 21)
  "MakePointerAvailableKHR\0" // IR(25161, 24)
  "MakePointerVisible\0" // IR(25185, 19)
  "MakePointerVisibleKHR\0" // IR(25204, 22)
  "NonPrivatePointer\0" // IR(25226, 18)
  "NonPrivatePointerKHR\0" // IR(25244, 21)
  "AliasScopeINTELMask\0" // IR(25265, 20)
  "NoAliasINTELMask\0" // IR(25285, 17)
  "Simple\0" // IR(25302, 7)
  "GLSL450\0" // IR(25309, 8)
  "OpenCL\0" // IR(25317, 7)
  "Vulkan\0" // IR(25324, 7)
  "VulkanKHR\0" // IR(25331, 10)
  "Relaxed\0" // IR(25341, 8)
  "Acquire\0" // IR(25349, 8)
  "Release\0" // IR(25357, 8)
  "AcquireRelease\0" // IR(25365, 15)
  "SequentiallyConsistent\0" // IR(25380, 23)
  "UniformMemory\0" // IR(25403, 14)
  "SubgroupMemory\0" // IR(25417, 15)
  "WorkgroupMemory\0" // IR(25432, 16)
  "CrossWorkgroupMemory\0" // IR(25448, 21)
  "AtomicCounterMemory\0" // IR(25469, 20)
  "ImageMemory\0" // IR(25489, 12)
  "OutputMemory\0" // IR(25501, 13)
  "OutputMemoryKHR\0" // IR(25514, 16)
  "MakeAvailable\0" // IR(25530, 14)
  "MakeAvailableKHR\0" // IR(25544, 17)
  "MakeVisible\0" // IR(25561, 12)
  "MakeVisibleKHR\0" // IR(25573, 15)
  "AutoINTEL\0" // IR(25588, 10)
  "WRAP\0" // IR(25598, 5)
  "SAT\0" // IR(25603, 4)
  "SAT_ZERO\0" // IR(25607, 9)
  "SAT_SYM\0" // IR(25616, 8)
  "PackedVectorFormat4x8Bit\0" // IR(25624, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(25649, 28)
  "TRN\0" // IR(25677, 4)
  "TRN_ZERO\0" // IR(25681, 9)
  "RND\0" // IR(25690, 4)
  "RND_ZERO\0" // IR(25694, 9)
  "RND_INF\0" // IR(25703, 8)
  "RND_MIN_INF\0" // IR(25711, 12)
  "RND_CONV\0" // IR(25723, 9)
  "RND_CONV_ODD\0" // IR(25732, 13)
  "RobustnessPerComponentNV\0" // IR(25745, 25)
  "RobustnessPerElementNV\0" // IR(25770, 23)
  "OpaqueKHR\0" // IR(25793, 10)
  "NoOpaqueKHR\0" // IR(25803, 12)
  "TerminateOnFirstHitKHR\0" // IR(25815, 23)
  "SkipClosestHitShaderKHR\0" // IR(25838, 24)
  "CullBackFacingTrianglesKHR\0" // IR(25862, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(25889, 28)
  "CullOpaqueKHR\0" // IR(25917, 14)
  "CullNoOpaqueKHR\0" // IR(25931, 16)
  "SkipTrianglesKHR\0" // IR(25947, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(25964, 24)
  "SkipAABBsKHR\0" // IR(25988, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(26001, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(26031, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(26072, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(26109, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(26146, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(26187, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(26229, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(26262, 33)
  "ClampToEdge\0" // IR(26295, 12)
  "Clamp\0" // IR(26307, 6)
  "Repeat\0" // IR(26313, 7)
  "RepeatMirrored\0" // IR(26320, 15)
  "Nearest\0" // IR(26335, 8)
  "Linear\0" // IR(26343, 7)
  "Unknown\0" // IR(26350, 8)
  "Rgba32f\0" // IR(26358, 8)
  "Rgba16f\0" // IR(26366, 8)
  "R32f\0" // IR(26374, 5)
  "Rgba8\0" // IR(26379, 6)
  "Rgba8Snorm\0" // IR(26385, 11)
  "Rg32f\0" // IR(26396, 6)
  "Rg16f\0" // IR(26402, 6)
  "R11fG11fB10f\0" // IR(26408, 13)
  "R16f\0" // IR(26421, 5)
  "Rgba16\0" // IR(26426, 7)
  "Rgb10A2\0" // IR(26433, 8)
  "Rg16\0" // IR(26441, 5)
  "Rg8\0" // IR(26446, 4)
  "R16\0" // IR(26450, 4)
  "R8\0" // IR(26454, 3)
  "Rgba16Snorm\0" // IR(26457, 12)
  "Rg16Snorm\0" // IR(26469, 10)
  "Rg8Snorm\0" // IR(26479, 9)
  "R16Snorm\0" // IR(26488, 9)
  "R8Snorm\0" // IR(26497, 8)
  "Rgba32i\0" // IR(26505, 8)
  "Rgba16i\0" // IR(26513, 8)
  "Rgba8i\0" // IR(26521, 7)
  "R32i\0" // IR(26528, 5)
  "Rg32i\0" // IR(26533, 6)
  "Rg16i\0" // IR(26539, 6)
  "Rg8i\0" // IR(26545, 5)
  "R16i\0" // IR(26550, 5)
  "R8i\0" // IR(26555, 4)
  "Rgba32ui\0" // IR(26559, 9)
  "Rgba16ui\0" // IR(26568, 9)
  "Rgba8ui\0" // IR(26577, 8)
  "R32ui\0" // IR(26585, 6)
  "Rgb10a2ui\0" // IR(26591, 10)
  "Rg32ui\0" // IR(26601, 7)
  "Rg16ui\0" // IR(26608, 7)
  "Rg8ui\0" // IR(26615, 6)
  "R16ui\0" // IR(26621, 6)
  "R8ui\0" // IR(26627, 5)
  "R64ui\0" // IR(26632, 6)
  "R64i\0" // IR(26638, 5)
  "CrossDevice\0" // IR(26643, 12)
  "Device\0" // IR(26655, 7)
  "Workgroup\0" // IR(26662, 10)
  "Subgroup\0" // IR(26672, 9)
  "Invocation\0" // IR(26681, 11)
  "QueueFamily\0" // IR(26692, 12)
  "QueueFamilyKHR\0" // IR(26704, 15)
  "ShaderCallKHR\0" // IR(26719, 14)
  "Flatten\0" // IR(26733, 8)
  "DontFlatten\0" // IR(26741, 12)
  "IdentifierPossibleDuplicates\0" // IR(26753, 29)
  "FlagUnknownPhysicalLayout\0" // IR(26782, 26)
  "ESSL\0" // IR(26808, 5)
  "GLSL\0" // IR(26813, 5)
  "OpenCL_C\0" // IR(26818, 9)
  "OpenCL_CPP\0" // IR(26827, 11)
  "HLSL\0" // IR(26838, 5)
  "CPP_for_OpenCL\0" // IR(26843, 15)
  "SYCL\0" // IR(26858, 5)
  "HERO_C\0" // IR(26863, 7)
  "NZSL\0" // IR(26870, 5)
  "WGSL\0" // IR(26875, 5)
  "Slang\0" // IR(26880, 6)
  "Zig\0" // IR(26886, 4)
  "Rust\0" // IR(26890, 5)
  "UniformConstant\0" // IR(26895, 16)
  "Input\0" // IR(26911, 6)
  "Output\0" // IR(26917, 7)
  "CrossWorkgroup\0" // IR(26924, 15)
  "Private\0" // IR(26939, 8)
  "Function\0" // IR(26947, 9)
  "Generic\0" // IR(26956, 8)
  "PushConstant\0" // IR(26964, 13)
  "AtomicCounter\0" // IR(26977, 14)
  "Image\0" // IR(26991, 6)
  "StorageBuffer\0" // IR(26997, 14)
  "TileImageEXT\0" // IR(27011, 13)
  "TileAttachmentQCOM\0" // IR(27024, 19)
  "NodePayloadAMDX\0" // IR(27043, 16)
  "CallableDataKHR\0" // IR(27059, 16)
  "CallableDataNV\0" // IR(27075, 15)
  "IncomingCallableDataKHR\0" // IR(27090, 24)
  "IncomingCallableDataNV\0" // IR(27114, 23)
  "RayPayloadKHR\0" // IR(27137, 14)
  "RayPayloadNV\0" // IR(27151, 13)
  "HitAttributeKHR\0" // IR(27164, 16)
  "HitAttributeNV\0" // IR(27180, 15)
  "IncomingRayPayloadKHR\0" // IR(27195, 22)
  "IncomingRayPayloadNV\0" // IR(27217, 21)
  "ShaderRecordBufferKHR\0" // IR(27238, 22)
  "ShaderRecordBufferNV\0" // IR(27260, 21)
  "PhysicalStorageBuffer\0" // IR(27281, 22)
  "PhysicalStorageBufferEXT\0" // IR(27303, 25)
  "HitObjectAttributeNV\0" // IR(27328, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(27349, 24)
  "CodeSectionINTEL\0" // IR(27373, 17)
  "DeviceOnlyALTERA\0" // IR(27390, 17)
  "DeviceOnlyINTEL\0" // IR(27407, 16)
  "HostOnlyALTERA\0" // IR(27423, 15)
  "HostOnlyINTEL\0" // IR(27438, 14)
  "WriteThroughINTEL\0" // IR(27452, 18)
  "WriteBackINTEL\0" // IR(27470, 15)
  "TensorView\0" // IR(27485, 11)
  "DecodeFunc\0" // IR(27496, 11)
  "Undefined\0" // IR(27507, 10)
  "NoneARM\0" // IR(27517, 8)
  "NontemporalARM\0" // IR(27525, 15)
  "OutOfBoundsValueARM\0" // IR(27540, 20)
  "MakeElementAvailableARM\0" // IR(27560, 24)
  "MakeElementVisibleARM\0" // IR(27584, 22)
  "NonPrivateElementARM\0" // IR(27606, 21)
  "Nop\0" // IR(27627, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(27631, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(27656, 27)
  "Undef\0" // IR(27683, 6)
  "SourceContinued\0" // IR(27689, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(27705, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(27738, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(27767, 41)
  "Source\0" // IR(27808, 7)
  "SourceExtension\0" // IR(27815, 16)
  "Name\0" // IR(27831, 5)
  "MemberName\0" // IR(27836, 11)
  "String\0" // IR(27847, 7)
  "Line\0" // IR(27854, 5)
  "Extension\0" // IR(27859, 10)
  "ExtInstImport\0" // IR(27869, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(27883, 46)
  "ExtInst\0" // IR(27929, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(27937, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(27971, 30)
  "MemoryModel\0" // IR(28001, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(28013, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(28046, 29)
  "EntryPoint\0" // IR(28075, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(28086, 32)
  "ExecutionMode\0" // IR(28118, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(28132, 28)
  "Capability\0" // IR(28160, 11)
  "TypeVoid\0" // IR(28171, 9)
  "TypeBool\0" // IR(28180, 9)
  "TypeInt\0" // IR(28189, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(28197, 37)
  "TypeFloat\0" // IR(28234, 10)
  "TypeVector\0" // IR(28244, 11)
  "TypeMatrix\0" // IR(28255, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(28266, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(28298, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(28336, 43)
  "TypeImage\0" // IR(28379, 10)
  "TypeSampler\0" // IR(28389, 12)
  "TypeSampledImage\0" // IR(28401, 17)
  "TypeArray\0" // IR(28418, 10)
  "TypeRuntimeArray\0" // IR(28428, 17)
  "TypeStruct\0" // IR(28445, 11)
  "TypeOpaque\0" // IR(28456, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(28467, 31)
  "TypePointer\0" // IR(28498, 12)
  "TypeFunction\0" // IR(28510, 13)
  "TypeEvent\0" // IR(28523, 10)
  "TypeDeviceEvent\0" // IR(28533, 16)
  "TypeReserveId\0" // IR(28549, 14)
  "TypeQueue\0" // IR(28563, 10)
  "TypePipe\0" // IR(28573, 9)
  "TypeForwardPointer\0" // IR(28582, 19)
  "ConstantTrue\0" // IR(28601, 13)
  "ConstantFalse\0" // IR(28614, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(28628, 38)
  "ConstantComposite\0" // IR(28666, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(28684, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(28725, 37)
  "ConstantSampler\0" // IR(28762, 16)
  "ConstantNull\0" // IR(28778, 13)
  "SpecConstantTrue\0" // IR(28791, 17)
  "SpecConstantFalse\0" // IR(28808, 18)
  "SpecConstant\0" // IR(28826, 13)
  "SpecConstantComposite\0" // IR(28839, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(28861, 41)
  "SpecConstantOp\0" // IR(28902, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(28917, 34)
  "FunctionParameter\0" // IR(28951, 18)
  "FunctionEnd\0" // IR(28969, 12)
  "FunctionCall\0" // IR(28981, 13)
  "Variable\0" // IR(28994, 9)
  "ImageTexelPointer\0" // IR(29003, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(29021, 40)
  "Load\0" // IR(29061, 5)
  "Store\0" // IR(29066, 6)
  "CopyMemory\0" // IR(29072, 11)
  "CopyMemorySized\0" // IR(29083, 16)
  "AccessChain\0" // IR(29099, 12)
  "InBoundsAccessChain\0" // IR(29111, 20)
  "PtrAccessChain\0" // IR(29131, 15)
  "ArrayLength\0" // IR(29146, 12)
  "GenericPtrMemSemantics\0" // IR(29158, 23)
  "InBoundsPtrAccessChain\0" // IR(29181, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(29204, 28)
  "Decorate\0" // IR(29232, 9)
  "MemberDecorate\0" // IR(29241, 15)
  "DecorationGroup\0" // IR(29256, 16)
  "GroupDecorate\0" // IR(29272, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(29286, 45)
  "GroupMemberDecorate\0" // IR(29331, 20)
  "VectorExtractDynamic\0" // IR(29351, 21)
  "VectorInsertDynamic\0" // IR(29372, 20)
  "VectorShuffle\0" // IR(29392, 14)
  "CompositeConstruct\0" // IR(29406, 19)
  "CompositeExtract\0" // IR(29425, 17)
  "CompositeInsert\0" // IR(29442, 16)
  "CopyObject\0" // IR(29458, 11)
  "Transpose\0" // IR(29469, 10)
  "SampledImage\0" // IR(29479, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(29492, 32)
  "ImageSampleImplicitLod\0" // IR(29524, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(29547, 23)
  "ImageSampleExplicitLod\0" // IR(29570, 23)
  "ImageSampleDrefImplicitLod\0" // IR(29593, 27)
  "ImageSampleDrefExplicitLod\0" // IR(29620, 27)
  "ImageSampleProjImplicitLod\0" // IR(29647, 27)
  "ImageSampleProjExplicitLod\0" // IR(29674, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(29701, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(29732, 31)
  "ImageFetch\0" // IR(29763, 11)
  "ImageGather\0" // IR(29774, 12)
  "ImageDrefGather\0" // IR(29786, 16)
  "ImageRead\0" // IR(29802, 10)
  "ImageWrite\0" // IR(29812, 11)
  "ImageQueryFormat\0" // IR(29823, 17)
  "ImageQueryOrder\0" // IR(29840, 16)
  "ImageQuerySizeLod\0" // IR(29856, 18)
  "ImageQuerySize\0" // IR(29874, 15)
  "ImageQueryLod\0" // IR(29889, 14)
  "ImageQueryLevels\0" // IR(29903, 17)
  "ImageQuerySamples\0" // IR(29920, 18)
  "ConvertFToU\0" // IR(29938, 12)
  "ConvertFToS\0" // IR(29950, 12)
  "ConvertSToF\0" // IR(29962, 12)
  "ConvertUToF\0" // IR(29974, 12)
  "UConvert\0" // IR(29986, 9)
  "SConvert\0" // IR(29995, 9)
  "FConvert\0" // IR(30004, 9)
  "QuantizeToF16\0" // IR(30013, 14)
  "ConvertPtrToU\0" // IR(30027, 14)
  "SatConvertSToU\0" // IR(30041, 15)
  "SatConvertUToS\0" // IR(30056, 15)
  "ConvertUToPtr\0" // IR(30071, 14)
  "PtrCastToGeneric\0" // IR(30085, 17)
  "GenericCastToPtr\0" // IR(30102, 17)
  "GenericCastToPtrExplicit\0" // IR(30119, 25)
  "Bitcast\0" // IR(30144, 8)
  "SNegate\0" // IR(30152, 8)
  "FNegate\0" // IR(30160, 8)
  "IAdd\0" // IR(30168, 5)
  "FAdd\0" // IR(30173, 5)
  "ISub\0" // IR(30178, 5)
  "FSub\0" // IR(30183, 5)
  "IMul\0" // IR(30188, 5)
  "FMul\0" // IR(30193, 5)
  "UDiv\0" // IR(30198, 5)
  "SDiv\0" // IR(30203, 5)
  "FDiv\0" // IR(30208, 5)
  "UMod\0" // IR(30213, 5)
  "SRem\0" // IR(30218, 5)
  "SMod\0" // IR(30223, 5)
  "FRem\0" // IR(30228, 5)
  "FMod\0" // IR(30233, 5)
  "VectorTimesScalar\0" // IR(30238, 18)
  "MatrixTimesScalar\0" // IR(30256, 18)
  "VectorTimesMatrix\0" // IR(30274, 18)
  "MatrixTimesVector\0" // IR(30292, 18)
  "MatrixTimesMatrix\0" // IR(30310, 18)
  "OuterProduct\0" // IR(30328, 13)
  "Dot\0" // IR(30341, 4)
  "IAddCarry\0" // IR(30345, 10)
  "ISubBorrow\0" // IR(30355, 11)
  "UMulExtended\0" // IR(30366, 13)
  "SMulExtended\0" // IR(30379, 13)
  "Any\0" // IR(30392, 4)
  "All\0" // IR(30396, 4)
  "IsNan\0" // IR(30400, 6)
  "IsInf\0" // IR(30406, 6)
  "IsFinite\0" // IR(30412, 9)
  "IsNormal\0" // IR(30421, 9)
  "SignBitSet\0" // IR(30430, 11)
  "LessOrGreater\0" // IR(30441, 14)
  "Ordered\0" // IR(30455, 8)
  "Unordered\0" // IR(30463, 10)
  "LogicalEqual\0" // IR(30473, 13)
  "LogicalNotEqual\0" // IR(30486, 16)
  "LogicalOr\0" // IR(30502, 10)
  "LogicalAnd\0" // IR(30512, 11)
  "LogicalNot\0" // IR(30523, 11)
  "Select\0" // IR(30534, 7)
  "IEqual\0" // IR(30541, 7)
  "INotEqual\0" // IR(30548, 10)
  "UGreaterThan\0" // IR(30558, 13)
  "SGreaterThan\0" // IR(30571, 13)
  "UGreaterThanEqual\0" // IR(30584, 18)
  "SGreaterThanEqual\0" // IR(30602, 18)
  "ULessThan\0" // IR(30620, 10)
  "SLessThan\0" // IR(30630, 10)
  "ULessThanEqual\0" // IR(30640, 15)
  "SLessThanEqual\0" // IR(30655, 15)
  "FOrdEqual\0" // IR(30670, 10)
  "FUnordEqual\0" // IR(30680, 12)
  "FOrdNotEqual\0" // IR(30692, 13)
  "FUnordNotEqual\0" // IR(30705, 15)
  "FOrdLessThan\0" // IR(30720, 13)
  "FUnordLessThan\0" // IR(30733, 15)
  "FOrdGreaterThan\0" // IR(30748, 16)
  "FUnordGreaterThan\0" // IR(30764, 18)
  "FOrdLessThanEqual\0" // IR(30782, 18)
  "FUnordLessThanEqual\0" // IR(30800, 20)
  "FOrdGreaterThanEqual\0" // IR(30820, 21)
  "FUnordGreaterThanEqual\0" // IR(30841, 23)
  "ShiftRightLogical\0" // IR(30864, 18)
  "ShiftRightArithmetic\0" // IR(30882, 21)
  "ShiftLeftLogical\0" // IR(30903, 17)
  "BitwiseOr\0" // IR(30920, 10)
  "BitwiseXor\0" // IR(30930, 11)
  "BitwiseAnd\0" // IR(30941, 11)
  "Not\0" // IR(30952, 4)
  "BitFieldInsert\0" // IR(30956, 15)
  "BitFieldSExtract\0" // IR(30971, 17)
  "BitFieldUExtract\0" // IR(30988, 17)
  "BitReverse\0" // IR(31005, 11)
  "BitCount\0" // IR(31016, 9)
  "DPdx\0" // IR(31025, 5)
  "DPdy\0" // IR(31030, 5)
  "Fwidth\0" // IR(31035, 7)
  "DPdxFine\0" // IR(31042, 9)
  "DPdyFine\0" // IR(31051, 9)
  "FwidthFine\0" // IR(31060, 11)
  "DPdxCoarse\0" // IR(31071, 11)
  "DPdyCoarse\0" // IR(31082, 11)
  "FwidthCoarse\0" // IR(31093, 13)
  "EmitVertex\0" // IR(31106, 11)
  "EndPrimitive\0" // IR(31117, 13)
  "EmitStreamVertex\0" // IR(31130, 17)
  "EndStreamPrimitive\0" // IR(31147, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(31166, 37)
  "ControlBarrier\0" // IR(31203, 15)
  "MemoryBarrier\0" // IR(31218, 14)
  "AtomicLoad\0" // IR(31232, 11)
  "AtomicStore\0" // IR(31243, 12)
  "AtomicExchange\0" // IR(31255, 15)
  "AtomicCompareExchange\0" // IR(31270, 22)
  "AtomicCompareExchangeWeak\0" // IR(31292, 26)
  "AtomicIIncrement\0" // IR(31318, 17)
  "AtomicIDecrement\0" // IR(31335, 17)
  "AtomicIAdd\0" // IR(31352, 11)
  "AtomicISub\0" // IR(31363, 11)
  "AtomicSMin\0" // IR(31374, 11)
  "AtomicUMin\0" // IR(31385, 11)
  "AtomicSMax\0" // IR(31396, 11)
  "AtomicUMax\0" // IR(31407, 11)
  "AtomicAnd\0" // IR(31418, 10)
  "AtomicOr\0" // IR(31428, 9)
  "AtomicXor\0" // IR(31437, 10)
  "Phi\0" // IR(31447, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(31451, 30)
  "LoopMerge\0" // IR(31481, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(31491, 35)
  "SelectionMerge\0" // IR(31526, 15)
  "Label\0" // IR(31541, 6)
  "Branch\0" // IR(31547, 7)
  "BranchConditional\0" // IR(31554, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(31572, 45)
  "Switch\0" // IR(31617, 7)
  "Kill\0" // IR(31624, 5)
  "Return\0" // IR(31629, 7)
  "ReturnValue\0" // IR(31636, 12)
  "Unreachable\0" // IR(31648, 12)
  "LifetimeStart\0" // IR(31660, 14)
  "LifetimeStop\0" // IR(31674, 13)
  "GroupAsyncCopy\0" // IR(31687, 15)
  "GroupWaitEvents\0" // IR(31702, 16)
  "GroupAll\0" // IR(31718, 9)
  "GroupAny\0" // IR(31727, 9)
  "GroupBroadcast\0" // IR(31736, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(31751, 33)
  "GroupIAdd\0" // IR(31784, 10)
  "GroupFAdd\0" // IR(31794, 10)
  "GroupFMin\0" // IR(31804, 10)
  "GroupUMin\0" // IR(31814, 10)
  "GroupSMin\0" // IR(31824, 10)
  "GroupFMax\0" // IR(31834, 10)
  "GroupUMax\0" // IR(31844, 10)
  "GroupSMax\0" // IR(31854, 10)
  "ReadPipe\0" // IR(31864, 9)
  "WritePipe\0" // IR(31873, 10)
  "ReservedReadPipe\0" // IR(31883, 17)
  "ReservedWritePipe\0" // IR(31900, 18)
  "ReserveReadPipePackets\0" // IR(31918, 23)
  "ReserveWritePipePackets\0" // IR(31941, 24)
  "CommitReadPipe\0" // IR(31965, 15)
  "CommitWritePipe\0" // IR(31980, 16)
  "IsValidReserveId\0" // IR(31996, 17)
  "GetNumPipePackets\0" // IR(32013, 18)
  "GetMaxPipePackets\0" // IR(32031, 18)
  "GroupReserveReadPipePackets\0" // IR(32049, 28)
  "GroupReserveWritePipePackets\0" // IR(32077, 29)
  "GroupCommitReadPipe\0" // IR(32106, 20)
  "GroupCommitWritePipe\0" // IR(32126, 21)
  "EnqueueMarker\0" // IR(32147, 14)
  "EnqueueKernel\0" // IR(32161, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(32175, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(32205, 32)
  "GetKernelWorkGroupSize\0" // IR(32237, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(32260, 40)
  "RetainEvent\0" // IR(32300, 12)
  "ReleaseEvent\0" // IR(32312, 13)
  "CreateUserEvent\0" // IR(32325, 16)
  "IsValidEvent\0" // IR(32341, 13)
  "SetUserEventStatus\0" // IR(32354, 19)
  "CaptureEventProfilingInfo\0" // IR(32373, 26)
  "GetDefaultQueue\0" // IR(32399, 16)
  "BuildNDRange\0" // IR(32415, 13)
  "ImageSparseSampleImplicitLod\0" // IR(32428, 29)
  "ImageSparseSampleExplicitLod\0" // IR(32457, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(32486, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(32519, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(32552, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(32585, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(32618, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(32655, 37)
  "ImageSparseFetch\0" // IR(32692, 17)
  "ImageSparseGather\0" // IR(32709, 18)
  "ImageSparseDrefGather\0" // IR(32727, 22)
  "ImageSparseTexelsResident\0" // IR(32749, 26)
  "NoLine\0" // IR(32775, 7)
  "AtomicFlagTestAndSet\0" // IR(32782, 21)
  "AtomicFlagClear\0" // IR(32803, 16)
  "ImageSparseRead\0" // IR(32819, 16)
  "SizeOf\0" // IR(32835, 7)
  "TypePipeStorage\0" // IR(32842, 16)
  "ConstantPipeStorage\0" // IR(32858, 20)
  "CreatePipeFromPipeStorage\0" // IR(32878, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(32904, 35)
  "GetKernelMaxNumSubgroups\0" // IR(32939, 25)
  "TypeNamedBarrier\0" // IR(32964, 17)
  "NamedBarrierInitialize\0" // IR(32981, 23)
  "MemoryNamedBarrier\0" // IR(33004, 19)
  "ModuleProcessed\0" // IR(33023, 16)
  "ExecutionModeId\0" // IR(33039, 16)
  "DecorateId\0" // IR(33055, 11)
  "GroupNonUniformElect\0" // IR(33066, 21)
  "GroupNonUniformAll\0" // IR(33087, 19)
  "GroupNonUniformAny\0" // IR(33106, 19)
  "GroupNonUniformAllEqual\0" // IR(33125, 24)
  "GroupNonUniformBroadcast\0" // IR(33149, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(33174, 30)
  "GroupNonUniformInverseBallot\0" // IR(33204, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(33233, 32)
  "GroupNonUniformBallotBitCount\0" // IR(33265, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(33295, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(33324, 29)
  "GroupNonUniformShuffleXor\0" // IR(33353, 26)
  "GroupNonUniformShuffleUp\0" // IR(33379, 25)
  "GroupNonUniformShuffleDown\0" // IR(33404, 27)
  "GroupNonUniformIAdd\0" // IR(33431, 20)
  "GroupNonUniformFAdd\0" // IR(33451, 20)
  "GroupNonUniformIMul\0" // IR(33471, 20)
  "GroupNonUniformFMul\0" // IR(33491, 20)
  "GroupNonUniformSMin\0" // IR(33511, 20)
  "GroupNonUniformUMin\0" // IR(33531, 20)
  "GroupNonUniformFMin\0" // IR(33551, 20)
  "GroupNonUniformSMax\0" // IR(33571, 20)
  "GroupNonUniformUMax\0" // IR(33591, 20)
  "GroupNonUniformFMax\0" // IR(33611, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(33631, 26)
  "GroupNonUniformBitwiseOr\0" // IR(33657, 25)
  "GroupNonUniformBitwiseXor\0" // IR(33682, 26)
  "GroupNonUniformLogicalAnd\0" // IR(33708, 26)
  "GroupNonUniformLogicalOr\0" // IR(33734, 25)
  "GroupNonUniformLogicalXor\0" // IR(33759, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(33785, 29)
  "GroupNonUniformQuadSwap\0" // IR(33814, 24)
  "CopyLogical\0" // IR(33838, 12)
  "PtrEqual\0" // IR(33850, 9)
  "PtrNotEqual\0" // IR(33859, 12)
  "PtrDiff\0" // IR(33871, 8)
  "ColorAttachmentReadEXT\0" // IR(33879, 23)
  "DepthAttachmentReadEXT\0" // IR(33902, 23)
  "StencilAttachmentReadEXT\0" // IR(33925, 25)
  "TypeTensorARM\0" // IR(33950, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(33964, 42)
  "TensorReadARM\0" // IR(34006, 14)
  "TensorWriteARM\0" // IR(34020, 15)
  "TensorQuerySizeARM\0" // IR(34035, 19)
  "GraphConstantARM\0" // IR(34054, 17)
  "GraphEntryPointARM\0" // IR(34071, 19)
  "GraphInputARM\0" // IR(34090, 14)
  "GraphSetOutputARM\0" // IR(34104, 18)
  "GraphEndARM\0" // IR(34122, 12)
  "TypeGraphARM\0" // IR(34134, 13)
  "TerminateInvocation\0" // IR(34147, 20)
  "TypeUntypedPointerKHR\0" // IR(34167, 22)
  "UntypedVariableKHR\0" // IR(34189, 19)
  "UntypedAccessChainKHR\0" // IR(34208, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(34230, 30)
  "SubgroupFirstInvocationKHR\0" // IR(34260, 27)
  "UntypedPtrAccessChainKHR\0" // IR(34287, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(34312, 33)
  "UntypedArrayLengthKHR\0" // IR(34345, 22)
  "UntypedPrefetchKHR\0" // IR(34367, 19)
  "FmaKHR\0" // IR(34386, 7)
  "SubgroupAllKHR\0" // IR(34393, 15)
  "SubgroupAnyKHR\0" // IR(34408, 15)
  "SubgroupAllEqualKHR\0" // IR(34423, 20)
  "SubgroupReadInvocationKHR\0" // IR(34443, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(34469, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(34495, 25)
  "TraceRayKHR\0" // IR(34520, 12)
  "ExecuteCallableKHR\0" // IR(34532, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(34551, 35)
  "IgnoreIntersectionKHR\0" // IR(34586, 22)
  "TerminateRayKHR\0" // IR(34608, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(34624, 47)
  "SDot\0" // IR(34671, 5)
  "SDotKHR\0" // IR(34676, 8)
  "UDot\0" // IR(34684, 5)
  "UDotKHR\0" // IR(34689, 8)
  "SUDot\0" // IR(34697, 6)
  "SUDotKHR\0" // IR(34703, 9)
  "SDotAccSat\0" // IR(34712, 11)
  "SDotAccSatKHR\0" // IR(34723, 14)
  "UDotAccSat\0" // IR(34737, 11)
  "UDotAccSatKHR\0" // IR(34748, 14)
  "SUDotAccSat\0" // IR(34762, 12)
  "SUDotAccSatKHR\0" // IR(34774, 15)
  "TypeCooperativeMatrixKHR\0" // IR(34789, 25)
  "CooperativeMatrixLoadKHR\0" // IR(34814, 25)
  "CooperativeMatrixStoreKHR\0" // IR(34839, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(34865, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(34919, 27)
  "CooperativeMatrixLengthKHR\0" // IR(34946, 27)
  "ConstantCompositeReplicateEXT\0" // IR(34973, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(35003, 34)
  "CompositeConstructReplicateEXT\0" // IR(35037, 31)
  "TypeRayQueryKHR\0" // IR(35068, 16)
  "RayQueryInitializeKHR\0" // IR(35084, 22)
  "RayQueryTerminateKHR\0" // IR(35106, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(35127, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(35159, 31)
  "RayQueryProceedKHR\0" // IR(35190, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(35209, 31)
  "ImageSampleWeightedQCOM\0" // IR(35240, 24)
  "ImageBoxFilterQCOM\0" // IR(35264, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(35283, 23)
  "ImageBlockMatchSADQCOM\0" // IR(35306, 23)
  "BitCastArrayQCOM\0" // IR(35329, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(35346, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(35375, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(35404, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(35433, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(35462, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(35492, 28)
  "ExtractSubArrayQCOM\0" // IR(35520, 20)
  "GroupIAddNonUniformAMD\0" // IR(35540, 23)
  "GroupFAddNonUniformAMD\0" // IR(35563, 23)
  "GroupFMinNonUniformAMD\0" // IR(35586, 23)
  "GroupUMinNonUniformAMD\0" // IR(35609, 23)
  "GroupSMinNonUniformAMD\0" // IR(35632, 23)
  "GroupFMaxNonUniformAMD\0" // IR(35655, 23)
  "GroupUMaxNonUniformAMD\0" // IR(35678, 23)
  "GroupSMaxNonUniformAMD\0" // IR(35701, 23)
  "FragmentMaskFetchAMD\0" // IR(35724, 21)
  "FragmentFetchAMD\0" // IR(35745, 17)
  "ReadClockKHR\0" // IR(35762, 13)
  "AllocateNodePayloadsAMDX\0" // IR(35775, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(35800, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(35824, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(35849, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(35878, 27)
  "IsNodePayloadValidAMDX\0" // IR(35905, 23)
  "ConstantStringAMDX\0" // IR(35928, 19)
  "SpecConstantStringAMDX\0" // IR(35947, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(35970, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(35996, 26)
  "HitObjectRecordHitMotionNV\0" // IR(36022, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(36049, 36)
  "HitObjectRecordMissMotionNV\0" // IR(36085, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(36113, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(36141, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(36169, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(36202, 30)
  "HitObjectTraceRayMotionNV\0" // IR(36232, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(36258, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(36297, 44)
  "HitObjectRecordEmptyNV\0" // IR(36341, 23)
  "HitObjectTraceRayNV\0" // IR(36364, 20)
  "HitObjectRecordHitNV\0" // IR(36384, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(36405, 30)
  "HitObjectRecordMissNV\0" // IR(36435, 22)
  "HitObjectExecuteShaderNV\0" // IR(36457, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(36482, 26)
  "HitObjectGetAttributesNV\0" // IR(36508, 25)
  "HitObjectGetHitKindNV\0" // IR(36533, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(36555, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(36584, 28)
  "HitObjectGetInstanceIdNV\0" // IR(36612, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(36637, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(36671, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(36703, 29)
  "HitObjectGetRayTMaxNV\0" // IR(36732, 22)
  "HitObjectGetRayTMinNV\0" // IR(36754, 22)
  "HitObjectIsEmptyNV\0" // IR(36776, 19)
  "HitObjectIsHitNV\0" // IR(36795, 17)
  "HitObjectIsMissNV\0" // IR(36812, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(36830, 29)
  "ReorderThreadWithHintNV\0" // IR(36859, 24)
  "TypeHitObjectNV\0" // IR(36883, 16)
  "ImageSampleFootprintNV\0" // IR(36899, 23)
  "TypeCooperativeVectorNV\0" // IR(36922, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(36946, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(36975, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(37017, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(37056, 32)
  "CooperativeMatrixConvertNV\0" // IR(37088, 27)
  "EmitMeshTasksEXT\0" // IR(37115, 17)
  "SetMeshOutputsEXT\0" // IR(37132, 18)
  "GroupNonUniformPartitionNV\0" // IR(37150, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(37177, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(37210, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(37245, 38)
  "CooperativeVectorLoadNV\0" // IR(37283, 24)
  "CooperativeVectorStoreNV\0" // IR(37307, 25)
  "ReportIntersectionKHR\0" // IR(37332, 22)
  "ReportIntersectionNV\0" // IR(37354, 21)
  "IgnoreIntersectionNV\0" // IR(37375, 21)
  "TerminateRayNV\0" // IR(37396, 15)
  "TraceNV\0" // IR(37411, 8)
  "TraceMotionNV\0" // IR(37419, 14)
  "TraceRayMotionNV\0" // IR(37433, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(37450, 50)
  "TypeAccelerationStructureKHR\0" // IR(37500, 29)
  "TypeAccelerationStructureNV\0" // IR(37529, 28)
  "ExecuteCallableNV\0" // IR(37557, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(37575, 35)
  "RayQueryGetClusterIdNV\0" // IR(37610, 23)
  "HitObjectGetClusterIdNV\0" // IR(37633, 24)
  "TypeCooperativeMatrixNV\0" // IR(37657, 24)
  "CooperativeMatrixLoadNV\0" // IR(37681, 24)
  "CooperativeMatrixStoreNV\0" // IR(37705, 25)
  "CooperativeMatrixMulAddNV\0" // IR(37730, 26)
  "CooperativeMatrixLengthNV\0" // IR(37756, 26)
  "BeginInvocationInterlockEXT\0" // IR(37782, 28)
  "EndInvocationInterlockEXT\0" // IR(37810, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(37836, 43)
  "CooperativeMatrixReduceNV\0" // IR(37879, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(37905, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(37936, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(37980, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(38010, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(38041, 32)
  "TypeTensorLayoutNV\0" // IR(38073, 19)
  "TypeTensorViewNV\0" // IR(38092, 17)
  "CreateTensorLayoutNV\0" // IR(38109, 21)
  "TensorLayoutSetDimensionNV\0" // IR(38130, 27)
  "TensorLayoutSetStrideNV\0" // IR(38157, 24)
  "TensorLayoutSliceNV\0" // IR(38181, 20)
  "TensorLayoutSetClampValueNV\0" // IR(38201, 28)
  "CreateTensorViewNV\0" // IR(38229, 19)
  "TensorViewSetDimensionNV\0" // IR(38248, 25)
  "TensorViewSetStrideNV\0" // IR(38273, 22)
  "IsHelperInvocationEXT\0" // IR(38295, 22)
  "TensorViewSetClipNV\0" // IR(38317, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(38337, 27)
  "CooperativeMatrixTransposeNV\0" // IR(38364, 29)
  "ConvertUToImageNV\0" // IR(38393, 18)
  "ConvertUToSamplerNV\0" // IR(38411, 20)
  "ConvertImageToUNV\0" // IR(38431, 18)
  "ConvertSamplerToUNV\0" // IR(38449, 20)
  "ConvertUToSampledImageNV\0" // IR(38469, 25)
  "ConvertSampledImageToUNV\0" // IR(38494, 25)
  "SamplerImageAddressingModeNV\0" // IR(38519, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(38548, 52)
  "RawAccessChainNV\0" // IR(38600, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(38617, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(38657, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(38695, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(38733, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(38767, 37)
  "HitObjectGetSpherePositionNV\0" // IR(38804, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(38833, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(38860, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(38887, 23)
  "HitObjectIsSphereHitNV\0" // IR(38910, 23)
  "HitObjectIsLSSHitNV\0" // IR(38933, 20)
  "RayQueryIsSphereHitNV\0" // IR(38953, 22)
  "RayQueryIsLSSHitNV\0" // IR(38975, 19)
  "SubgroupShuffleDownINTEL\0" // IR(38994, 25)
  "SubgroupShuffleUpINTEL\0" // IR(39019, 23)
  "SubgroupShuffleXorINTEL\0" // IR(39042, 24)
  "SubgroupBlockReadINTEL\0" // IR(39066, 23)
  "SubgroupBlockWriteINTEL\0" // IR(39089, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(39113, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(39141, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(39170, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(39203, 34)
  "UCountLeadingZerosINTEL\0" // IR(39237, 24)
  "UCountTrailingZerosINTEL\0" // IR(39261, 25)
  "AbsISubINTEL\0" // IR(39286, 13)
  "AbsUSubINTEL\0" // IR(39299, 13)
  "IAddSatINTEL\0" // IR(39312, 13)
  "UAddSatINTEL\0" // IR(39325, 13)
  "IAverageINTEL\0" // IR(39338, 14)
  "UAverageINTEL\0" // IR(39352, 14)
  "IAverageRoundedINTEL\0" // IR(39366, 21)
  "UAverageRoundedINTEL\0" // IR(39387, 21)
  "ISubSatINTEL\0" // IR(39408, 13)
  "USubSatINTEL\0" // IR(39421, 13)
  "IMul32x16INTEL\0" // IR(39434, 15)
  "UMul32x16INTEL\0" // IR(39449, 15)
  "ConstantFunctionPointerINTEL\0" // IR(39464, 29)
  "FunctionPointerCallINTEL\0" // IR(39493, 25)
  "AsmTargetINTEL\0" // IR(39518, 15)
  "AsmCallINTEL\0" // IR(39533, 13)
  "AtomicFMinEXT\0" // IR(39546, 14)
  "AtomicFMaxEXT\0" // IR(39560, 14)
  "AssumeTrueKHR\0" // IR(39574, 14)
  "ExpectKHR\0" // IR(39588, 10)
  "DecorateString\0" // IR(39598, 15)
  "DecorateStringGOOGLE\0" // IR(39613, 21)
  "MemberDecorateString\0" // IR(39634, 21)
  "MemberDecorateStringGOOGLE\0" // IR(39655, 27)
  "VmeImageINTEL\0" // IR(39682, 14)
  "TypeVmeImageINTEL\0" // IR(39696, 18)
  "TypeAvcImePayloadINTEL\0" // IR(39714, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(39737, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(39760, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(39783, 23)
  "TypeAvcMceResultINTEL\0" // IR(39806, 22)
  "TypeAvcImeResultINTEL\0" // IR(39828, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(39850, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(39896, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(39940, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(39979, 37)
  "TypeAvcRefResultINTEL\0" // IR(40016, 22)
  "TypeAvcSicResultINTEL\0" // IR(40038, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(40060, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(40120, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(40173, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(40220, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(40260, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(40311, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(40355, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(40406, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(40462, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(40512, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(40564, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(40613, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(40660, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(40710, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(40761, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(40817, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(40850, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(40902, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(40963, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(41024, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(41063, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(41101, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(41140, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(41178, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(41217, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(41255, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(41291, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(41330, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(41373, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(41411, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(41449, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(41487, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(41532, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(41572, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(41634, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(41664, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(41702, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(41738, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(41771, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(41806, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(41845, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(41888, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(41935, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(41989, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(42023, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(42070, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(42115, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(42170, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(42223, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(42279, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(42333, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(42391, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(42447, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(42485, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(42531, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(42575, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(42624, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(42671, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(42741, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(42809, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(42878, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(42946, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(43012, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(43079, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(43115, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(43163, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(43222, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(43280, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(43336, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(43366, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(43396, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(43435, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(43481, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(43524, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(43571, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(43616, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(43662, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(43718, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(43756, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(43786, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(43818, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(43854, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(43896, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(43935, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(43974, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(44018, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(44066, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(44116, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(44159, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(44207, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(44250, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(44281, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(44328, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(44373, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(44419, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(44475, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(44513, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(44548, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(44592, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(44638, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(44679, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(44715, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(44765, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(44813, 35)
  "SaveMemoryINTEL\0" // IR(44848, 16)
  "RestoreMemoryINTEL\0" // IR(44864, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(44883, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(44912, 28)
  "ArbitraryFloatCastALTERA\0" // IR(44940, 25)
  "ArbitraryFloatCastINTEL\0" // IR(44965, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(44989, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(45021, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(45052, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(45082, 29)
  "ArbitraryFloatAddALTERA\0" // IR(45111, 24)
  "ArbitraryFloatAddINTEL\0" // IR(45135, 23)
  "ArbitraryFloatSubALTERA\0" // IR(45158, 24)
  "ArbitraryFloatSubINTEL\0" // IR(45182, 23)
  "ArbitraryFloatMulALTERA\0" // IR(45205, 24)
  "ArbitraryFloatMulINTEL\0" // IR(45229, 23)
  "ArbitraryFloatDivALTERA\0" // IR(45252, 24)
  "ArbitraryFloatDivINTEL\0" // IR(45276, 23)
  "ArbitraryFloatGTALTERA\0" // IR(45299, 23)
  "ArbitraryFloatGTINTEL\0" // IR(45322, 22)
  "ArbitraryFloatGEALTERA\0" // IR(45344, 23)
  "ArbitraryFloatGEINTEL\0" // IR(45367, 22)
  "ArbitraryFloatLTALTERA\0" // IR(45389, 23)
  "ArbitraryFloatLTINTEL\0" // IR(45412, 22)
  "ArbitraryFloatLEALTERA\0" // IR(45434, 23)
  "ArbitraryFloatLEINTEL\0" // IR(45457, 22)
  "ArbitraryFloatEQALTERA\0" // IR(45479, 23)
  "ArbitraryFloatEQINTEL\0" // IR(45502, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(45524, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(45550, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(45575, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(45601, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(45626, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(45651, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(45675, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(45701, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(45726, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(45751, 24)
  "ArbitraryFloatLogINTEL\0" // IR(45775, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(45798, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(45822, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(45847, 25)
  "ArbitraryFloatExpINTEL\0" // IR(45872, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(45895, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(45919, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(45944, 25)
  "ArbitraryFloatSinINTEL\0" // IR(45969, 23)
  "ArbitraryFloatCosINTEL\0" // IR(45992, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(46015, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(46041, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(46066, 25)
  "ArbitraryFloatASinINTEL\0" // IR(46091, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(46115, 26)
  "ArbitraryFloatACosINTEL\0" // IR(46141, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(46165, 26)
  "ArbitraryFloatATanINTEL\0" // IR(46191, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(46215, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(46241, 25)
  "ArbitraryFloatPowINTEL\0" // IR(46266, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(46289, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(46313, 24)
  "LoopControlINTEL\0" // IR(46337, 17)
  "AliasDomainDeclINTEL\0" // IR(46354, 21)
  "AliasScopeDeclINTEL\0" // IR(46375, 20)
  "AliasScopeListDeclINTEL\0" // IR(46395, 24)
  "FixedSqrtALTERA\0" // IR(46419, 16)
  "FixedSqrtINTEL\0" // IR(46435, 15)
  "FixedRecipALTERA\0" // IR(46450, 17)
  "FixedRecipINTEL\0" // IR(46467, 16)
  "FixedRsqrtALTERA\0" // IR(46483, 17)
  "FixedRsqrtINTEL\0" // IR(46500, 16)
  "FixedSinALTERA\0" // IR(46516, 15)
  "FixedSinINTEL\0" // IR(46531, 14)
  "FixedCosALTERA\0" // IR(46545, 15)
  "FixedCosINTEL\0" // IR(46560, 14)
  "FixedSinCosALTERA\0" // IR(46574, 18)
  "FixedSinCosINTEL\0" // IR(46592, 17)
  "FixedSinPiALTERA\0" // IR(46609, 17)
  "FixedSinPiINTEL\0" // IR(46626, 16)
  "FixedCosPiALTERA\0" // IR(46642, 17)
  "FixedCosPiINTEL\0" // IR(46659, 16)
  "FixedSinCosPiALTERA\0" // IR(46675, 20)
  "FixedSinCosPiINTEL\0" // IR(46695, 19)
  "FixedLogALTERA\0" // IR(46714, 15)
  "FixedLogINTEL\0" // IR(46729, 14)
  "FixedExpALTERA\0" // IR(46743, 15)
  "FixedExpINTEL\0" // IR(46758, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(46772, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(46802, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(46831, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(46861, 29)
  "ReadPipeBlockingALTERA\0" // IR(46890, 23)
  "ReadPipeBlockingINTEL\0" // IR(46913, 22)
  "WritePipeBlockingALTERA\0" // IR(46935, 24)
  "WritePipeBlockingINTEL\0" // IR(46959, 23)
  "RayQueryGetRayTMinKHR\0" // IR(46982, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(47004, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(47027, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(47055, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(47101, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(47138, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(47203, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(47243, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(47284, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(47323, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(47359, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(47405, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(47450, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(47492, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(47524, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(47553, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(47593, 40)
  "AtomicFAddEXT\0" // IR(47633, 14)
  "TypeBufferSurfaceINTEL\0" // IR(47647, 23)
  "TypeStructContinuedINTEL\0" // IR(47670, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(47695, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(47727, 36)
  "CompositeConstructContinuedINTEL\0" // IR(47763, 33)
  "ConvertFToBF16INTEL\0" // IR(47796, 20)
  "ConvertBF16ToFINTEL\0" // IR(47816, 20)
  "ControlBarrierArriveINTEL\0" // IR(47836, 26)
  "ControlBarrierWaitINTEL\0" // IR(47862, 24)
  "TaskSequenceCreateALTERA\0" // IR(47886, 25)
  "TaskSequenceCreateINTEL\0" // IR(47911, 24)
  "TaskSequenceAsyncALTERA\0" // IR(47935, 24)
  "TaskSequenceAsyncINTEL\0" // IR(47959, 23)
  "TaskSequenceGetALTERA\0" // IR(47982, 22)
  "TaskSequenceGetINTEL\0" // IR(48004, 21)
  "TaskSequenceReleaseALTERA\0" // IR(48025, 26)
  "TaskSequenceReleaseINTEL\0" // IR(48051, 25)
  "TypeTaskSequenceALTERA\0" // IR(48076, 23)
  "TypeTaskSequenceINTEL\0" // IR(48099, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(48121, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(48148, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(48173, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(48207, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(48241, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(48270, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(48296, 62)
  "BitwiseFunctionINTEL\0" // IR(48358, 21)
  "ConditionalExtensionINTEL\0" // IR(48379, 26)
  "ConditionalEntryPointINTEL\0" // IR(48405, 27)
  "ConditionalCapabilityINTEL\0" // IR(48432, 27)
  "SpecConstantTargetINTEL\0" // IR(48459, 24)
  "SpecConstantArchitectureINTEL\0" // IR(48483, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(48513, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(48550, 30)
  "ConditionalCopyObjectINTEL\0" // IR(48580, 27)
  "GroupIMulKHR\0" // IR(48607, 13)
  "GroupFMulKHR\0" // IR(48620, 13)
  "GroupBitwiseAndKHR\0" // IR(48633, 19)
  "GroupBitwiseOrKHR\0" // IR(48652, 18)
  "GroupBitwiseXorKHR\0" // IR(48670, 19)
  "GroupLogicalAndKHR\0" // IR(48689, 19)
  "GroupLogicalOrKHR\0" // IR(48708, 18)
  "GroupLogicalXorKHR\0" // IR(48726, 19)
  "RoundFToTF32INTEL\0" // IR(48745, 18)
  "MaskedGatherINTEL\0" // IR(48763, 18)
  "MaskedScatterINTEL\0" // IR(48781, 19)
  "ConvertHandleToImageINTEL\0" // IR(48800, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(48826, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(48854, 33)
  "MIN_SAD\0" // IR(48887, 8)
  "MIN_SAD_COST\0" // IR(48895, 13)
  "RAW_SAD\0" // IR(48908, 8)
  "DebugInfoNone\0" // IR(48916, 14)
  "DebugCompilationUnit\0" // IR(48930, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(48951, 52)
  "DebugTypeBasic\0" // IR(49003, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(49018, 34)
  "DebugTypePointer\0" // IR(49052, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(49069, 38)
  "DebugTypeQualifier\0" // IR(49107, 19)
  "DebugTypeArray\0" // IR(49126, 15)
  "DebugTypeVector\0" // IR(49141, 16)
  "DebugTypedef\0" // IR(49157, 13)
  "DebugTypeFunction\0" // IR(49170, 18)
  "DebugTypeEnum\0" // IR(49188, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(49202, 38)
  "DebugTypeComposite\0" // IR(49240, 19)
  "DebugTypeMember\0" // IR(49259, 16)
  "DebugTypeInheritance\0" // IR(49275, 21)
  "DebugTypePtrToMember\0" // IR(49296, 21)
  "DebugTypeTemplate\0" // IR(49317, 18)
  "DebugTypeTemplateParameter\0" // IR(49335, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(49362, 35)
  "DebugTypeTemplateParameterPack\0" // IR(49397, 31)
  "DebugGlobalVariable\0" // IR(49428, 20)
  "DebugFunctionDeclaration\0" // IR(49448, 25)
  "DebugFunction\0" // IR(49473, 14)
  "DebugLexicalBlock\0" // IR(49487, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(49505, 31)
  "DebugScope\0" // IR(49536, 11)
  "DebugNoScope\0" // IR(49547, 13)
  "DebugInlinedAt\0" // IR(49560, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(49575, 42)
  "DebugLocalVariable\0" // IR(49617, 19)
  "DebugInlinedVariable\0" // IR(49636, 21)
  "DebugDeclare\0" // IR(49657, 13)
  "DebugValue\0" // IR(49670, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(49681, 33)
  "DebugOperation\0" // IR(49714, 15)
  "DebugExpression\0" // IR(49729, 16)
  "DebugMacroDef\0" // IR(49745, 14)
  "DebugMacroUndef\0" // IR(49759, 16)
  "Round\0" // IR(49775, 6)
  "RoundEven\0" // IR(49781, 10)
  "Trunc\0" // IR(49791, 6)
  "FAbs\0" // IR(49797, 5)
  "SAbs\0" // IR(49802, 5)
  "FSign\0" // IR(49807, 6)
  "SSign\0" // IR(49813, 6)
  "Floor\0" // IR(49819, 6)
  "Ceil\0" // IR(49825, 5)
  "Fract\0" // IR(49830, 6)
  "Radians\0" // IR(49836, 8)
  "Degrees\0" // IR(49844, 8)
  "Sin\0" // IR(49852, 4)
  "Cos\0" // IR(49856, 4)
  "Tan\0" // IR(49860, 4)
  "Asin\0" // IR(49864, 5)
  "Acos\0" // IR(49869, 5)
  "Atan\0" // IR(49874, 5)
  "Sinh\0" // IR(49879, 5)
  "Cosh\0" // IR(49884, 5)
  "Tanh\0" // IR(49889, 5)
  "Asinh\0" // IR(49894, 6)
  "Acosh\0" // IR(49900, 6)
  "Atanh\0" // IR(49906, 6)
  "Atan2\0" // IR(49912, 6)
  "Pow\0" // IR(49918, 4)
  "Exp\0" // IR(49922, 4)
  "Log\0" // IR(49926, 4)
  "Exp2\0" // IR(49930, 5)
  "Log2\0" // IR(49935, 5)
  "Sqrt\0" // IR(49940, 5)
  "InverseSqrt\0" // IR(49945, 12)
  "Determinant\0" // IR(49957, 12)
  "MatrixInverse\0" // IR(49969, 14)
  "Modf\0" // IR(49983, 5)
  "ModfStruct\0" // IR(49988, 11)
  "FMin\0" // IR(49999, 5)
  "UMin\0" // IR(50004, 5)
  "SMin\0" // IR(50009, 5)
  "FMax\0" // IR(50014, 5)
  "UMax\0" // IR(50019, 5)
  "SMax\0" // IR(50024, 5)
  "FClamp\0" // IR(50029, 7)
  "UClamp\0" // IR(50036, 7)
  "SClamp\0" // IR(50043, 7)
  "FMix\0" // IR(50050, 5)
  "IMix\0" // IR(50055, 5)
  "Step\0" // IR(50060, 5)
  "SmoothStep\0" // IR(50065, 11)
  "Fma\0" // IR(50076, 4)
  "Frexp\0" // IR(50080, 6)
  "FrexpStruct\0" // IR(50086, 12)
  "Ldexp\0" // IR(50098, 6)
  "PackSnorm4x8\0" // IR(50104, 13)
  "PackUnorm4x8\0" // IR(50117, 13)
  "PackSnorm2x16\0" // IR(50130, 14)
  "PackUnorm2x16\0" // IR(50144, 14)
  "PackHalf2x16\0" // IR(50158, 13)
  "PackDouble2x32\0" // IR(50171, 15)
  "UnpackSnorm2x16\0" // IR(50186, 16)
  "UnpackUnorm2x16\0" // IR(50202, 16)
  "UnpackHalf2x16\0" // IR(50218, 15)
  "UnpackSnorm4x8\0" // IR(50233, 15)
  "UnpackUnorm4x8\0" // IR(50248, 15)
  "UnpackDouble2x32\0" // IR(50263, 17)
  "Length\0" // IR(50280, 7)
  "Distance\0" // IR(50287, 9)
  "Cross\0" // IR(50296, 6)
  "Normalize\0" // IR(50302, 10)
  "FaceForward\0" // IR(50312, 12)
  "Reflect\0" // IR(50324, 8)
  "Refract\0" // IR(50332, 8)
  "FindILsb\0" // IR(50340, 9)
  "FindSMsb\0" // IR(50349, 9)
  "FindUMsb\0" // IR(50358, 9)
  "InterpolateAtCentroid\0" // IR(50367, 22)
  "InterpolateAtSample\0" // IR(50389, 20)
  "InterpolateAtOffset\0" // IR(50409, 20)
  "NMin\0" // IR(50429, 5)
  "NMax\0" // IR(50434, 5)
  "NClamp\0" // IR(50439, 7)
  "ArgumentInfo\0" // IR(50446, 13)
  "ArgumentStorageBuffer\0" // IR(50459, 22)
  "ArgumentUniform\0" // IR(50481, 16)
  "ArgumentPodStorageBuffer\0" // IR(50497, 25)
  "ArgumentPodUniform\0" // IR(50522, 19)
  "ArgumentPodPushConstant\0" // IR(50541, 24)
  "ArgumentSampledImage\0" // IR(50565, 21)
  "ArgumentStorageImage\0" // IR(50586, 21)
  "ArgumentSampler\0" // IR(50607, 16)
  "ArgumentWorkgroup\0" // IR(50623, 18)
  "SpecConstantWorkgroupSize\0" // IR(50641, 26)
  "SpecConstantGlobalOffset\0" // IR(50667, 25)
  "SpecConstantWorkDim\0" // IR(50692, 20)
  "PushConstantGlobalOffset\0" // IR(50712, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(50737, 30)
  "PushConstantGlobalSize\0" // IR(50767, 23)
  "PushConstantRegionOffset\0" // IR(50790, 25)
  "PushConstantNumWorkgroups\0" // IR(50815, 26)
  "PushConstantRegionGroupOffset\0" // IR(50841, 30)
  "ConstantDataStorageBuffer\0" // IR(50871, 26)
  "ConstantDataUniform\0" // IR(50897, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(50917, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(50947, 28)
  "ArgumentPointerPushConstant\0" // IR(50975, 28)
  "ArgumentPointerUniform\0" // IR(51003, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(51026, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(51061, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(51099, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(51141, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(51186, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(51223, 40)
  "ArgumentStorageTexelBuffer\0" // IR(51263, 27)
  "ArgumentUniformTexelBuffer\0" // IR(51290, 27)
  "ConstantDataPointerPushConstant\0" // IR(51317, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(51349, 40)
  "PrintfInfo\0" // IR(51389, 11)
  "PrintfBufferStorageBuffer\0" // IR(51400, 26)
  "PrintfBufferPointerPushConstant\0" // IR(51426, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(51458, 34)
  "WorkgroupVariableSize\0" // IR(51492, 22)
  "DebugImportedEntity\0" // IR(51514, 20)
  "DebugSource\0" // IR(51534, 12)
  "DebugFunctionDefinition\0" // IR(51546, 24)
  "DebugSourceContinued\0" // IR(51570, 21)
  "DebugLine\0" // IR(51591, 10)
  "DebugNoLine\0" // IR(51601, 12)
  "DebugBuildIdentifier\0" // IR(51613, 21)
  "DebugStoragePath\0" // IR(51634, 17)
  "DebugEntryPoint\0" // IR(51651, 16)
  "DebugTypeMatrix\0" // IR(51667, 16)
  "Configuration\0" // IR(51683, 14)
  "StartCounter\0" // IR(51697, 13)
  "StopCounter\0" // IR(51710, 12)
  "PushConstants\0" // IR(51722, 14)
  "SpecializationMapEntry\0" // IR(51736, 23)
  "DescriptorSetBuffer\0" // IR(51759, 20)
  "DescriptorSetImage\0" // IR(51779, 19)
  "DescriptorSetSampler\0" // IR(51798, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(51819, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(51882, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(51927, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(51976, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(52025, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(52069, 50)
  "DebugModuleINTEL\0" // IR(52119, 17)
  "acos\0" // IR(52136, 5)
  "acosh\0" // IR(52141, 6)
  "acospi\0" // IR(52147, 7)
  "asin\0" // IR(52154, 5)
  "asinh\0" // IR(52159, 6)
  "asinpi\0" // IR(52165, 7)
  "atan\0" // IR(52172, 5)
  "atan2\0" // IR(52177, 6)
  "atanh\0" // IR(52183, 6)
  "atanpi\0" // IR(52189, 7)
  "atan2pi\0" // IR(52196, 8)
  "cbrt\0" // IR(52204, 5)
  "ceil\0" // IR(52209, 5)
  "copysign\0" // IR(52214, 9)
  "cos\0" // IR(52223, 4)
  "cosh\0" // IR(52227, 5)
  "cospi\0" // IR(52232, 6)
  "erfc\0" // IR(52238, 5)
  "erf\0" // IR(52243, 4)
  "exp\0" // IR(52247, 4)
  "exp2\0" // IR(52251, 5)
  "exp10\0" // IR(52256, 6)
  "expm1\0" // IR(52262, 6)
  "fabs\0" // IR(52268, 5)
  "fdim\0" // IR(52273, 5)
  "floor\0" // IR(52278, 6)
  "fma\0" // IR(52284, 4)
  "fmax\0" // IR(52288, 5)
  "fmin\0" // IR(52293, 5)
  "fmod\0" // IR(52298, 5)
  "fract\0" // IR(52303, 6)
  "frexp\0" // IR(52309, 6)
  "hypot\0" // IR(52315, 6)
  "ilogb\0" // IR(52321, 6)
  "ldexp\0" // IR(52327, 6)
  "lgamma\0" // IR(52333, 7)
  "lgamma_r\0" // IR(52340, 9)
  "log\0" // IR(52349, 4)
  "log2\0" // IR(52353, 5)
  "log10\0" // IR(52358, 6)
  "log1p\0" // IR(52364, 6)
  "logb\0" // IR(52370, 5)
  "mad\0" // IR(52375, 4)
  "maxmag\0" // IR(52379, 7)
  "minmag\0" // IR(52386, 7)
  "modf\0" // IR(52393, 5)
  "nan\0" // IR(52398, 4)
  "nextafter\0" // IR(52402, 10)
  "pow\0" // IR(52412, 4)
  "pown\0" // IR(52416, 5)
  "powr\0" // IR(52421, 5)
  "remainder\0" // IR(52426, 10)
  "remquo\0" // IR(52436, 7)
  "rint\0" // IR(52443, 5)
  "rootn\0" // IR(52448, 6)
  "round\0" // IR(52454, 6)
  "rsqrt\0" // IR(52460, 6)
  "sin\0" // IR(52466, 4)
  "sincos\0" // IR(52470, 7)
  "sinh\0" // IR(52477, 5)
  "sinpi\0" // IR(52482, 6)
  "sqrt\0" // IR(52488, 5)
  "tan\0" // IR(52493, 4)
  "tanh\0" // IR(52497, 5)
  "tanpi\0" // IR(52502, 6)
  "tgamma\0" // IR(52508, 7)
  "trunc\0" // IR(52515, 6)
  "half_cos\0" // IR(52521, 9)
  "half_divide\0" // IR(52530, 12)
  "half_exp\0" // IR(52542, 9)
  "half_exp2\0" // IR(52551, 10)
  "half_exp10\0" // IR(52561, 11)
  "half_log\0" // IR(52572, 9)
  "half_log2\0" // IR(52581, 10)
  "half_log10\0" // IR(52591, 11)
  "half_powr\0" // IR(52602, 10)
  "half_recip\0" // IR(52612, 11)
  "half_rsqrt\0" // IR(52623, 11)
  "half_sin\0" // IR(52634, 9)
  "half_sqrt\0" // IR(52643, 10)
  "half_tan\0" // IR(52653, 9)
  "native_cos\0" // IR(52662, 11)
  "native_divide\0" // IR(52673, 14)
  "native_exp\0" // IR(52687, 11)
  "native_exp2\0" // IR(52698, 12)
  "native_exp10\0" // IR(52710, 13)
  "native_log\0" // IR(52723, 11)
  "native_log2\0" // IR(52734, 12)
  "native_log10\0" // IR(52746, 13)
  "native_powr\0" // IR(52759, 12)
  "native_recip\0" // IR(52771, 13)
  "native_rsqrt\0" // IR(52784, 13)
  "native_sin\0" // IR(52797, 11)
  "native_sqrt\0" // IR(52808, 12)
  "native_tan\0" // IR(52820, 11)
  "fclamp\0" // IR(52831, 7)
  "degrees\0" // IR(52838, 8)
  "fmax_common\0" // IR(52846, 12)
  "fmin_common\0" // IR(52858, 12)
  "mix\0" // IR(52870, 4)
  "radians\0" // IR(52874, 8)
  "step\0" // IR(52882, 5)
  "smoothstep\0" // IR(52887, 11)
  "sign\0" // IR(52898, 5)
  "cross\0" // IR(52903, 6)
  "distance\0" // IR(52909, 9)
  "length\0" // IR(52918, 7)
  "normalize\0" // IR(52925, 10)
  "fast_distance\0" // IR(52935, 14)
  "fast_length\0" // IR(52949, 12)
  "fast_normalize\0" // IR(52961, 15)
  "s_abs\0" // IR(52976, 6)
  "s_abs_diff\0" // IR(52982, 11)
  "s_add_sat\0" // IR(52993, 10)
  "u_add_sat\0" // IR(53003, 10)
  "s_hadd\0" // IR(53013, 7)
  "u_hadd\0" // IR(53020, 7)
  "s_rhadd\0" // IR(53027, 8)
  "u_rhadd\0" // IR(53035, 8)
  "s_clamp\0" // IR(53043, 8)
  "u_clamp\0" // IR(53051, 8)
  "clz\0" // IR(53059, 4)
  "ctz\0" // IR(53063, 4)
  "s_mad_hi\0" // IR(53067, 9)
  "u_mad_sat\0" // IR(53076, 10)
  "s_mad_sat\0" // IR(53086, 10)
  "s_max\0" // IR(53096, 6)
  "u_max\0" // IR(53102, 6)
  "s_min\0" // IR(53108, 6)
  "u_min\0" // IR(53114, 6)
  "s_mul_hi\0" // IR(53120, 9)
  "rotate\0" // IR(53129, 7)
  "s_sub_sat\0" // IR(53136, 10)
  "u_sub_sat\0" // IR(53146, 10)
  "u_upsample\0" // IR(53156, 11)
  "s_upsample\0" // IR(53167, 11)
  "popcount\0" // IR(53178, 9)
  "s_mad24\0" // IR(53187, 8)
  "u_mad24\0" // IR(53195, 8)
  "s_mul24\0" // IR(53203, 8)
  "u_mul24\0" // IR(53211, 8)
  "vloadn\0" // IR(53219, 7)
  "vstoren\0" // IR(53226, 8)
  "vload_half\0" // IR(53234, 11)
  "vload_halfn\0" // IR(53245, 12)
  "vstore_half\0" // IR(53257, 12)
  "vstore_half_r\0" // IR(53269, 14)
  "vstore_halfn\0" // IR(53283, 13)
  "vstore_halfn_r\0" // IR(53296, 15)
  "vloada_halfn\0" // IR(53311, 13)
  "vstorea_halfn\0" // IR(53324, 14)
  "vstorea_halfn_r\0" // IR(53338, 16)
  "shuffle\0" // IR(53354, 8)
  "shuffle2\0" // IR(53362, 9)
  "printf\0" // IR(53371, 7)
  "prefetch\0" // IR(53378, 9)
  "bitselect\0" // IR(53387, 10)
  "select\0" // IR(53397, 7)
  "u_abs\0" // IR(53404, 6)
  "u_abs_diff\0" // IR(53410, 11)
  "u_mul_hi\0" // IR(53421, 9)
  "u_mad_hi\0" // IR(53430, 9)
  "CubeFaceIndexAMD\0" // IR(53439, 17)
  "CubeFaceCoordAMD\0" // IR(53456, 17)
  "TimeAMD\0" // IR(53473, 8)
  "SwizzleInvocationsAMD\0" // IR(53481, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(53503, 28)
  "WriteInvocationAMD\0" // IR(53531, 19)
  "MbcntAMD\0" // IR(53550, 9)
  "InterpolateAtVertexAMD\0" // IR(53559, 23)
  "FMin3AMD\0" // IR(53582, 9)
  "UMin3AMD\0" // IR(53591, 9)
  "SMin3AMD\0" // IR(53600, 9)
  "FMax3AMD\0" // IR(53609, 9)
  "UMax3AMD\0" // IR(53618, 9)
  "SMax3AMD\0" // IR(53627, 9)
  "FMid3AMD\0" // IR(53636, 9)
  "UMid3AMD\0" // IR(53645, 9)
  "SMid3AMD\0" // IR(53654, 9)
  "ARGMAX\0" // IR(53663, 7)
  "AVG_POOL2D\0" // IR(53670, 11)
  "CONV2D\0" // IR(53681, 7)
  "CONV3D\0" // IR(53688, 7)
  "DEPTHWISE_CONV2D\0" // IR(53695, 17)
  "FFT2D\0" // IR(53712, 6)
  "MATMUL\0" // IR(53718, 7)
  "MAX_POOL2D\0" // IR(53725, 11)
  "RFFT2D\0" // IR(53736, 7)
  "TRANSPOSE_CONV2D\0" // IR(53743, 17)
  "CLAMP\0" // IR(53760, 6)
  "ERF\0" // IR(53766, 4)
  "SIGMOID\0" // IR(53770, 8)
  "TANH\0" // IR(53778, 5)
  "ADD\0" // IR(53783, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(53787, 23)
  "BITWISE_AND\0" // IR(53810, 12)
  "BITWISE_OR\0" // IR(53822, 11)
  "BITWISE_XOR\0" // IR(53833, 12)
  "INTDIV\0" // IR(53845, 7)
  "LOGICAL_AND\0" // IR(53852, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(53864, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(53883, 20)
  "LOGICAL_OR\0" // IR(53903, 11)
  "LOGICAL_XOR\0" // IR(53914, 12)
  "MAXIMUM\0" // IR(53926, 8)
  "MINIMUM\0" // IR(53934, 8)
  "MUL\0" // IR(53942, 4)
  "POW\0" // IR(53946, 4)
  "SUB\0" // IR(53950, 4)
  "TABLE\0" // IR(53954, 6)
  "ABS\0" // IR(53960, 4)
  "BITWISE_NOT\0" // IR(53964, 12)
  "CEIL\0" // IR(53976, 5)
  "CLZ\0" // IR(53981, 4)
  "COS\0" // IR(53985, 4)
  "EXP\0" // IR(53989, 4)
  "FLOOR\0" // IR(53993, 6)
  "LOG\0" // IR(53999, 4)
  "LOGICAL_NOT\0" // IR(54003, 12)
  "NEGATE\0" // IR(54015, 7)
  "RECIPROCAL\0" // IR(54022, 11)
  "RSQRT\0" // IR(54033, 6)
  "SIN\0" // IR(54039, 4)
  "SELECT\0" // IR(54043, 7)
  "EQUAL\0" // IR(54050, 6)
  "GREATER\0" // IR(54056, 8)
  "GREATER_EQUAL\0" // IR(54064, 14)
  "REDUCE_ALL\0" // IR(54078, 11)
  "REDUCE_ANY\0" // IR(54089, 11)
  "REDUCE_MAX\0" // IR(54100, 11)
  "REDUCE_MIN\0" // IR(54111, 11)
  "REDUCE_PRODUCT\0" // IR(54122, 15)
  "REDUCE_SUM\0" // IR(54137, 11)
  "CONCAT\0" // IR(54148, 7)
  "PAD\0" // IR(54155, 4)
  "RESHAPE\0" // IR(54159, 8)
  "REVERSE\0" // IR(54167, 8)
  "SLICE\0" // IR(54175, 6)
  "TILE\0" // IR(54181, 5)
  "TRANSPOSE\0" // IR(54186, 10)
  "GATHER\0" // IR(54196, 7)
  "SCATTER\0" // IR(54203, 8)
  "RESIZE\0" // IR(54211, 7)
  "CAST\0" // IR(54218, 5)
  "RESCALE\0" // IR(54223, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(5414, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6325, 18), // 1 SubgroupEqMaskKHR
  IR(6380, 18), // 2 SubgroupGeMaskKHR
  IR(6413, 18), // 3 SubgroupGtMaskKHR
  IR(6446, 18), // 4 SubgroupLeMaskKHR
  IR(6479, 18), // 5 SubgroupLtMaskKHR
  IR(7337, 12), // 6 BaryCoordNV
  IR(7392, 19), // 7 BaryCoordNoPerspNV
  IR(7423, 15), // 8 FragmentSizeNV
  IR(7480, 22), // 9 InvocationsPerPixelNV
  IR(7608, 11), // 10 LaunchIdNV
  IR(7633, 13), // 11 LaunchSizeNV
  IR(7664, 17), // 12 WorldRayOriginNV
  IR(7702, 20), // 13 WorldRayDirectionNV
  IR(7741, 18), // 14 ObjectRayOriginNV
  IR(7781, 21), // 15 ObjectRayDirectionNV
  IR(7813, 10), // 16 RayTminNV
  IR(7834, 10), // 17 RayTmaxNV
  IR(7867, 22), // 18 InstanceCustomIndexNV
  IR(7906, 16), // 19 ObjectToWorldNV
  IR(7939, 16), // 20 WorldToObjectNV
  IR(7973, 10), // 21 HitKindNV
  IR(8204, 19), // 22 IncomingRayFlagsNV
  IR(9980, 28), // 23 StorageUniformBufferBlock16
  IR(10043, 17), // 24 StorageUniform16
  IR(10858, 27), // 25 ShaderViewportIndexLayerNV
  IR(10902, 22), // 26 FragmentBarycentricNV
  IR(10955, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(10985, 14), // 28 ShadingRateNV
  IR(11045, 20), // 29 ShaderNonUniformEXT
  IR(11088, 26), // 30 RuntimeDescriptorArrayEXT
  IR(11150, 39), // 31 InputAttachmentArrayDynamicIndexingEXT
  IR(11228, 42), // 32 UniformTexelBufferArrayDynamicIndexingEXT
  IR(11309, 42), // 33 StorageTexelBufferArrayDynamicIndexingEXT
  IR(11388, 40), // 34 UniformBufferArrayNonUniformIndexingEXT
  IR(11464, 39), // 35 SampledImageArrayNonUniformIndexingEXT
  IR(11540, 40), // 36 StorageBufferArrayNonUniformIndexingEXT
  IR(11616, 39), // 37 StorageImageArrayNonUniformIndexingEXT
  IR(11694, 42), // 38 InputAttachmentArrayNonUniformIndexingEXT
  IR(11778, 45), // 39 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(11865, 45), // 40 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(11928, 21), // 41 VulkanMemoryModelKHR
  IR(11978, 32), // 42 VulkanMemoryModelDeviceScopeKHR
  IR(12010, 34), // 43 PhysicalStorageBufferAddressesEXT
  IR(12076, 31), // 44 ComputeDerivativeGroupLinearNV
  IR(12280, 28), // 45 DemoteToHelperInvocationEXT
  IR(13260, 26), // 46 FPGAMemoryAttributesINTEL
  IR(13339, 32), // 47 ArbitraryPrecisionIntegersINTEL
  IR(13409, 37), // 48 ArbitraryPrecisionFloatingPointINTEL
  IR(13499, 22), // 49 FPGALoopControlsINTEL
  IR(13594, 24), // 50 FPGAMemoryAccessesINTEL
  IR(13646, 27), // 51 FPGAClusterAttributesINTEL
  IR(13688, 14), // 52 LoopFuseINTEL
  IR(13723, 20), // 53 FPGADSPControlINTEL
  IR(13810, 40), // 54 FPGAInvocationPipeliningAttributesINTEL
  IR(13875, 24), // 55 FPGABufferLocationINTEL
  IR(13934, 34), // 56 ArbitraryPrecisionFixedPointINTEL
  IR(13992, 23), // 57 USMStorageClassesINTEL
  IR(14045, 29), // 58 RuntimeAlignedAttributeINTEL
  IR(14088, 13), // 59 IOPipesINTEL
  IR(14121, 19), // 60 BlockingPipesINTEL
  IR(14154, 13), // 61 FPGARegINTEL
  IR(14186, 22), // 62 DotProductInputAllKHR
  IR(14230, 25), // 63 DotProductInput4x8BitKHR
  IR(14283, 31), // 64 DotProductInput4x8BitPackedKHR
  IR(14325, 14), // 65 DotProductKHR
  IR(14497, 13), // 66 OptNoneINTEL
  IR(14642, 29), // 67 FPGAClusterAttributesV2INTEL
  IR(14718, 18), // 68 TaskSequenceINTEL
  IR(14777, 24), // 69 FPGALatencyControlINTEL
  IR(14830, 28), // 70 FPGAArgumentInterfacesINTEL
  IR(14924, 35), // 71 GlobalVariableFPGADecorationsINTEL
  IR(17707, 15), // 72 PerPrimitiveNV
  IR(17755, 12), // 73 PerVertexNV
  IR(17778, 14), // 74 NonUniformEXT
  IR(17808, 19), // 75 RestrictPointerEXT
  IR(17842, 18), // 76 AliasedPointerEXT
  IR(18152, 24), // 77 HlslCounterBufferGOOGLE
  IR(18189, 19), // 78 HlslSemanticGOOGLE
  IR(18319, 14), // 79 RegisterINTEL
  IR(18346, 12), // 80 MemoryINTEL
  IR(18373, 14), // 81 NumbanksINTEL
  IR(18403, 15), // 82 BankwidthINTEL
  IR(18441, 22), // 83 MaxPrivateCopiesINTEL
  IR(18480, 16), // 84 SinglepumpINTEL
  IR(18513, 16), // 85 DoublepumpINTEL
  IR(18549, 19), // 86 MaxReplicatesINTEL
  IR(18589, 20), // 87 SimpleDualPortINTEL
  IR(18621, 11), // 88 MergeINTEL
  IR(18689, 14), // 89 BankBitsINTEL
  IR(18724, 20), // 90 ForcePow2DepthINTEL
  IR(18761, 16), // 91 StridesizeINTEL
  IR(18792, 14), // 92 WordsizeINTEL
  IR(18825, 18), // 93 TrueDualPortINTEL
  IR(18863, 19), // 94 BurstCoalesceINTEL
  IR(18898, 15), // 95 CacheSizeINTEL
  IR(18942, 28), // 96 DontStaticallyCoalesceINTEL
  IR(18985, 14), // 97 PrefetchINTEL
  IR(19017, 17), // 98 StallEnableINTEL
  IR(19060, 25), // 99 FuseLoopsInFunctionINTEL
  IR(19105, 19), // 100 MathOpDSPModeINTEL
  IR(19178, 24), // 101 InitiationIntervalINTEL
  IR(19223, 20), // 102 MaxConcurrencyINTEL
  IR(19264, 20), // 103 PipelineEnableINTEL
  IR(19305, 20), // 104 BufferLocationINTEL
  IR(19345, 19), // 105 IOPipeStorageINTEL
  IR(19523, 15), // 106 StallFreeINTEL
  IR(19621, 25), // 107 LatencyControlLabelINTEL
  IR(19677, 30), // 108 LatencyControlConstraintINTEL
  IR(19735, 27), // 109 ConduitKernelArgumentINTEL
  IR(19794, 31), // 110 RegisterMapKernelArgumentINTEL
  IR(19859, 33), // 111 MMHostInterfaceAddressWidthINTEL
  IR(19923, 30), // 112 MMHostInterfaceDataWidthINTEL
  IR(19982, 28), // 113 MMHostInterfaceLatencyINTEL
  IR(20079, 34), // 114 MMHostInterfaceReadWriteModeINTEL
  IR(20143, 29), // 115 MMHostInterfaceMaxBurstINTEL
  IR(20205, 32), // 116 MMHostInterfaceWaitRequestINTEL
  IR(20264, 26), // 117 StableKernelArgumentINTEL
  IR(20407, 14), // 118 InitModeINTEL
  IR(20450, 28), // 119 ImplementInRegisterMapINTEL
  IR(21784, 14), // 120 OutputLinesNV
  IR(21818, 19), // 121 OutputPrimitivesNV
  IR(21861, 23), // 122 DerivativeGroupQuadsNV
  IR(21909, 24), // 123 DerivativeGroupLinearNV
  IR(21952, 18), // 124 OutputTrianglesNV
  IR(22700, 16), // 125 RayGenerationNV
  IR(22732, 15), // 126 IntersectionNV
  IR(22757, 9), // 127 AnyHitNV
  IR(22780, 13), // 128 ClosestHitNV
  IR(22801, 7), // 129 MissNV
  IR(22820, 11), // 130 CallableNV
  IR(22965, 23), // 131 AllowContractFastINTEL
  IR(23001, 18), // 132 AllowReassocINTEL
  IR(23227, 20), // 133 RuntimeAlignedINTEL
  IR(23476, 22), // 134 MakeTexelAvailableKHR
  IR(23515, 20), // 135 MakeTexelVisibleKHR
  IR(23551, 19), // 136 NonPrivateTexelKHR
  IR(23584, 17), // 137 VolatileTexelKHR
  IR(24143, 27), // 138 InitOnDeviceReprogramINTEL
  IR(24194, 23), // 139 InitOnDeviceResetINTEL
  IR(24528, 21), // 140 DependencyArrayINTEL
  IR(24568, 18), // 141 LoopCoalesceINTEL
  IR(24608, 21), // 142 MaxInterleavingINTEL
  IR(24656, 26), // 143 SpeculatedIterationsINTEL
  IR(24697, 14), // 144 NoFusionINTEL
  IR(24727, 15), // 145 LoopCountINTEL
  IR(24769, 26), // 146 MaxReinvocationDelayINTEL
  IR(25161, 24), // 147 MakePointerAvailableKHR
  IR(25204, 22), // 148 MakePointerVisibleKHR
  IR(25244, 21), // 149 NonPrivatePointerKHR
  IR(25331, 10), // 150 VulkanKHR
  IR(15479, 5), // 151 None
  IR(25514, 16), // 152 OutputMemoryKHR
  IR(25544, 17), // 153 MakeAvailableKHR
  IR(25573, 15), // 154 MakeVisibleKHR
  IR(25649, 28), // 155 PackedVectorFormat4x8BitKHR
  IR(25964, 24), // 156 SkipBuiltinPrimitivesNV
  IR(26704, 15), // 157 QueueFamilyKHR
  IR(27075, 15), // 158 CallableDataNV
  IR(27114, 23), // 159 IncomingCallableDataNV
  IR(27151, 13), // 160 RayPayloadNV
  IR(27180, 15), // 161 HitAttributeNV
  IR(27217, 21), // 162 IncomingRayPayloadNV
  IR(27260, 21), // 163 ShaderRecordBufferNV
  IR(27303, 25), // 164 PhysicalStorageBufferEXT
  IR(27407, 16), // 165 DeviceOnlyINTEL
  IR(27438, 14), // 166 HostOnlyINTEL
  IR(34676, 8), // 167 SDotKHR
  IR(34689, 8), // 168 UDotKHR
  IR(34703, 9), // 169 SUDotKHR
  IR(34723, 14), // 170 SDotAccSatKHR
  IR(34748, 14), // 171 UDotAccSatKHR
  IR(34774, 15), // 172 SUDotAccSatKHR
  IR(37354, 21), // 173 ReportIntersectionNV
  IR(37529, 28), // 174 TypeAccelerationStructureNV
  IR(37610, 23), // 175 RayQueryGetClusterIdNV
  IR(39613, 21), // 176 DecorateStringGOOGLE
  IR(39655, 27), // 177 MemberDecorateStringGOOGLE
  IR(44912, 28), // 178 ArbitraryFloatSinCosPiINTEL
  IR(44965, 24), // 179 ArbitraryFloatCastINTEL
  IR(45021, 31), // 180 ArbitraryFloatCastFromIntINTEL
  IR(45082, 29), // 181 ArbitraryFloatCastToIntINTEL
  IR(45135, 23), // 182 ArbitraryFloatAddINTEL
  IR(45182, 23), // 183 ArbitraryFloatSubINTEL
  IR(45229, 23), // 184 ArbitraryFloatMulINTEL
  IR(45276, 23), // 185 ArbitraryFloatDivINTEL
  IR(45322, 22), // 186 ArbitraryFloatGTINTEL
  IR(45367, 22), // 187 ArbitraryFloatGEINTEL
  IR(45412, 22), // 188 ArbitraryFloatLTINTEL
  IR(45457, 22), // 189 ArbitraryFloatLEINTEL
  IR(45502, 22), // 190 ArbitraryFloatEQINTEL
  IR(45550, 25), // 191 ArbitraryFloatRecipINTEL
  IR(45601, 25), // 192 ArbitraryFloatRSqrtINTEL
  IR(45651, 24), // 193 ArbitraryFloatCbrtINTEL
  IR(45701, 25), // 194 ArbitraryFloatHypotINTEL
  IR(45751, 24), // 195 ArbitraryFloatSqrtINTEL
  IR(46435, 15), // 196 FixedSqrtINTEL
  IR(46467, 16), // 197 FixedRecipINTEL
  IR(46500, 16), // 198 FixedRsqrtINTEL
  IR(46531, 14), // 199 FixedSinINTEL
  IR(46560, 14), // 200 FixedCosINTEL
  IR(46592, 17), // 201 FixedSinCosINTEL
  IR(46626, 16), // 202 FixedSinPiINTEL
  IR(46659, 16), // 203 FixedCosPiINTEL
  IR(46695, 19), // 204 FixedSinCosPiINTEL
  IR(46729, 14), // 205 FixedLogINTEL
  IR(46758, 14), // 206 FixedExpINTEL
  IR(46802, 29), // 207 PtrCastToCrossWorkgroupINTEL
  IR(46861, 29), // 208 CrossWorkgroupCastToPtrINTEL
  IR(46913, 22), // 209 ReadPipeBlockingINTEL
  IR(46959, 23), // 210 WritePipeBlockingINTEL
  IR(47911, 24), // 211 TaskSequenceCreateINTEL
  IR(47959, 23), // 212 TaskSequenceAsyncINTEL
  IR(48004, 21), // 213 TaskSequenceGetINTEL
  IR(48051, 25), // 214 TaskSequenceReleaseINTEL
  IR(48099, 22), // 215 TypeTaskSequenceINTEL
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
  spv::Capability::FPGAClusterAttributesALTERA, // 106
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
  spv::Capability::FPGAMemoryAttributesALTERA, // 132
  spv::Capability::FPGAMemoryAccessesALTERA, // 133
  spv::Capability::LoopFuseALTERA, // 134
  spv::Capability::FPGADSPControlALTERA, // 135
  spv::Capability::MemoryAccessAliasingINTEL, // 136
  spv::Capability::FPGAInvocationPipeliningAttributesALTERA, // 137
  spv::Capability::FPGABufferLocationALTERA, // 138
  spv::Capability::IOPipesALTERA, // 139
  spv::Capability::FPGAClusterAttributesV2ALTERA, // 140
  spv::Capability::FPMaxErrorINTEL, // 141
  spv::Capability::FPGALatencyControlALTERA, // 142
  spv::Capability::FPGAArgumentInterfacesALTERA, // 143
  spv::Capability::GlobalVariableHostAccessINTEL, // 144
  spv::Capability::GlobalVariableFPGADecorationsALTERA, // 145
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
  spv::Capability::RuntimeAlignedAttributeALTERA, // 179
  spv::Capability::Kernel, // 180
  spv::Capability::GroupNonUniformArithmetic, // 181
  spv::Capability::GroupNonUniformBallot, // 182
  spv::Capability::GroupNonUniformClustered, // 183
  spv::Capability::GroupNonUniformPartitionedNV, // 184
  spv::Capability::ImageGatherExtended, // 185
  spv::Capability::MinLod, // 186
  spv::Capability::VulkanMemoryModel, // 187
  spv::Capability::FPGALoopControlsALTERA, // 188
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 189
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
  spv::Capability::USMStorageClassesALTERA, // 200
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
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 287
  spv::Capability::UnstructuredLoopControlsINTEL, // 288
  spv::Capability::BlockingPipesALTERA, // 289
  spv::Capability::FPGARegALTERA, // 290
  spv::Capability::AtomicFloat16AddEXT, // 291
  spv::Capability::AtomicFloat32AddEXT, // 292
  spv::Capability::AtomicFloat64AddEXT, // 293
  spv::Capability::AtomicFloat16VectorNV, // 294
  spv::Capability::LongCompositesINTEL, // 295
  spv::Capability::BFloat16ConversionINTEL, // 296
  spv::Capability::SplitBarrierINTEL, // 297
  spv::Capability::ArithmeticFenceEXT, // 298
  spv::Capability::TaskSequenceALTERA, // 299
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
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 103
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 104
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 105
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 106
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 107
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 108
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 109
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 110
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 111
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 112
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 113
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 114
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 115
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 116
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 117
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 118
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 119
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 120
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 121
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 122
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 123
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 124
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 125
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 126
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 127
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 128
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 129
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 130
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 131
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 132
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 133
  spvtools::Extension::kSPV_INTEL_io_pipes, // 134
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 135
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 136
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 137
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 138
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 139
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 140
  spvtools::Extension::kSPV_EXT_replicated_composites, // 141
  spvtools::Extension::kSPV_KHR_bit_instructions, // 142
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 143
  spvtools::Extension::kSPV_KHR_float_controls2, // 144
  spvtools::Extension::kSPV_KHR_fma, // 145
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 146
  spvtools::Extension::kSPV_INTEL_long_composites, // 147
  spvtools::Extension::kSPV_EXT_optnone, // 148
  spvtools::Extension::kSPV_INTEL_optnone, // 149
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 150
  spvtools::Extension::kSPV_INTEL_debug_module, // 151
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 152
  spvtools::Extension::kSPV_INTEL_split_barrier, // 153
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 154
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 155
  spvtools::Extension::kSPV_INTEL_task_sequence, // 156
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 157
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 158
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 159
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 160
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 161
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 162
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 163
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 164
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 165
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 166
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 167
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 168
  spvtools::Extension::kSPV_INTEL_function_variants, // 169
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 170
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 171
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 172
  spvtools::Extension::kSPV_INTEL_cache_controls, // 173
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 174
  spvtools::Extension::kSPV_INTEL_bindless_images, // 175
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 176
  spvtools::Extension::kSPV_NV_mesh_shader, // 177
  spvtools::Extension::kSPV_EXT_mesh_shader, // 178
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 179
  spvtools::Extension::kSPV_GOOGLE_user_type, // 180
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 181
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 182
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 183
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 184
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 185
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 186
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 187
  spvtools::Extension::kSPV_KHR_variable_pointers, // 188
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 189
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 190
  spvtools::Extension::kSPV_KHR_ray_tracing, // 191
  spvtools::Extension::kSPV_KHR_ray_query, // 192
  spvtools::Extension::kSPV_NV_ray_tracing, // 193
  spvtools::Extension::kSPV_KHR_ray_tracing, // 194
  spvtools::Extension::kSPV_KHR_ray_query, // 195
  spvtools::Extension::kSPV_NV_displacement_micromap, // 196
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 197
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 198
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
