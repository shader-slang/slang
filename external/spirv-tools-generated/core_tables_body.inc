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
    case Extension::kSPV_EXT_demote_to_helper_invocation: return IR(1153, 36);
    case Extension::kSPV_EXT_descriptor_heap: return IR(1189, 24);
    case Extension::kSPV_EXT_descriptor_indexing: return IR(1213, 28);
    case Extension::kSPV_EXT_float8: return IR(1241, 15);
    case Extension::kSPV_EXT_fragment_fully_covered: return IR(1256, 31);
    case Extension::kSPV_EXT_fragment_invocation_density: return IR(1287, 36);
    case Extension::kSPV_EXT_fragment_shader_interlock: return IR(1323, 34);
    case Extension::kSPV_EXT_long_vector: return IR(1357, 20);
    case Extension::kSPV_EXT_mesh_shader: return IR(1377, 20);
    case Extension::kSPV_EXT_ocp_microscaling_types: return IR(1397, 31);
    case Extension::kSPV_EXT_opacity_micromap: return IR(1428, 25);
    case Extension::kSPV_EXT_optnone: return IR(1453, 16);
    case Extension::kSPV_EXT_physical_storage_buffer: return IR(1469, 32);
    case Extension::kSPV_EXT_relaxed_printf_string_address_space: return IR(1501, 44);
    case Extension::kSPV_EXT_replicated_composites: return IR(1545, 30);
    case Extension::kSPV_EXT_shader_64bit_indexing: return IR(1575, 30);
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(1605, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(1639, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(1671, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(1707, 27);
    case Extension::kSPV_EXT_shader_invocation_reorder: return IR(1734, 34);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(1768, 30);
    case Extension::kSPV_EXT_shader_subgroup_partitioned: return IR(1798, 36);
    case Extension::kSPV_EXT_shader_tile_image: return IR(1834, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1860, 36);
    case Extension::kSPV_EXT_split_barrier: return IR(1896, 22);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1918, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1945, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1976, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1997, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(2019, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(2061, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(2106, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(2145, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(2175, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(2201, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(2226, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(2251, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(2274, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(2318, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(2344, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(2372, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(2395, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(2430, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(2461, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(2495, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(2522, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(2570, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(2601, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(2630, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(2661, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(2694, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(2713, 28);
    case Extension::kSPV_INTEL_function_variants: return IR(2741, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(2769, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(2812, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(2850, 26);
    case Extension::kSPV_INTEL_int4: return IR(2876, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2891, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2910, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2938, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2964, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2984, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(3016, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(3044, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(3069, 33);
    case Extension::kSPV_INTEL_optnone: return IR(3102, 18);
    case Extension::kSPV_INTEL_predicated_io: return IR(3120, 24);
    case Extension::kSPV_INTEL_rounded_divide_sqrt: return IR(3144, 30);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(3174, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(3200, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(3236, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3260, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3295, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3341, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3361, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3385, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3421, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3456, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3493, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3523, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3555, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3580, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3602, 21);
    case Extension::kSPV_KHR_abort: return IR(3623, 14);
    case Extension::kSPV_KHR_bfloat16: return IR(3637, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3654, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3679, 35);
    case Extension::kSPV_KHR_constant_data: return IR(3714, 22);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3736, 27);
    case Extension::kSPV_KHR_device_group: return IR(3763, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3784, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3806, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3829, 24);
    case Extension::kSPV_KHR_fma: return IR(3853, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3865, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3901, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3931, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(3959, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3980, 30);
    case Extension::kSPV_KHR_multiview: return IR(4010, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(4028, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(4063, 26);
    case Extension::kSPV_KHR_opacity_micromap: return IR(4089, 25);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(4114, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(4146, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(4168, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4196, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4217, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4239, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4257, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4277, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4312, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4349, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4383, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4405, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4426, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4457, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4494, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4518, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4556, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4578, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4607, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4642, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4667, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4693, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4721, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4762, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4800, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4824, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4862, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4896, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4922, 27);
    case Extension::kSPV_NV_cooperative_matrix_decode_vector: return IR(4949, 40);
    case Extension::kSPV_NV_cooperative_vector: return IR(4989, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(5015, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(5044, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(5079, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(5114, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(5142, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(5161, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(5187, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(5212, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5231, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5262, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5299, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5332, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5362, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5395, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5421, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5456, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5476, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5505, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5530, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5553, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5592, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5618, 27);
    case Extension::kSPV_QCOM_image_processing3: return IR(5645, 27);
    case Extension::kSPV_QCOM_multiple_wait_queues: return IR(5672, 30);
    case Extension::kSPV_QCOM_tile_shading: return IR(5702, 22);
    case Extension::kSPV_SAMSUNG_intrinsic: return IR(5724, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5746, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5784, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,201> kExtensionNames{{
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
    {IR(1153, 36), static_cast<uint32_t>(kSPV_EXT_demote_to_helper_invocation)},
    {IR(1189, 24), static_cast<uint32_t>(kSPV_EXT_descriptor_heap)},
    {IR(1213, 28), static_cast<uint32_t>(kSPV_EXT_descriptor_indexing)},
    {IR(1241, 15), static_cast<uint32_t>(kSPV_EXT_float8)},
    {IR(1256, 31), static_cast<uint32_t>(kSPV_EXT_fragment_fully_covered)},
    {IR(1287, 36), static_cast<uint32_t>(kSPV_EXT_fragment_invocation_density)},
    {IR(1323, 34), static_cast<uint32_t>(kSPV_EXT_fragment_shader_interlock)},
    {IR(1357, 20), static_cast<uint32_t>(kSPV_EXT_long_vector)},
    {IR(1377, 20), static_cast<uint32_t>(kSPV_EXT_mesh_shader)},
    {IR(1397, 31), static_cast<uint32_t>(kSPV_EXT_ocp_microscaling_types)},
    {IR(1428, 25), static_cast<uint32_t>(kSPV_EXT_opacity_micromap)},
    {IR(1453, 16), static_cast<uint32_t>(kSPV_EXT_optnone)},
    {IR(1469, 32), static_cast<uint32_t>(kSPV_EXT_physical_storage_buffer)},
    {IR(1501, 44), static_cast<uint32_t>(kSPV_EXT_relaxed_printf_string_address_space)},
    {IR(1545, 30), static_cast<uint32_t>(kSPV_EXT_replicated_composites)},
    {IR(1575, 30), static_cast<uint32_t>(kSPV_EXT_shader_64bit_indexing)},
    {IR(1605, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(1639, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(1671, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(1707, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(1734, 34), static_cast<uint32_t>(kSPV_EXT_shader_invocation_reorder)},
    {IR(1768, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(1798, 36), static_cast<uint32_t>(kSPV_EXT_shader_subgroup_partitioned)},
    {IR(1834, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1860, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1896, 22), static_cast<uint32_t>(kSPV_EXT_split_barrier)},
    {IR(1918, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1945, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1976, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1997, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(2019, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(2061, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(2106, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(2145, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(2175, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(2201, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(2226, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(2251, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(2274, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(2318, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(2344, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(2372, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(2395, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(2430, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(2461, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(2495, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(2522, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(2570, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(2601, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(2630, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(2661, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(2694, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(2713, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(2741, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(2769, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(2812, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(2850, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2876, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2891, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2910, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2938, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2964, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2984, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(3016, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(3044, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(3069, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(3102, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(3120, 24), static_cast<uint32_t>(kSPV_INTEL_predicated_io)},
    {IR(3144, 30), static_cast<uint32_t>(kSPV_INTEL_rounded_divide_sqrt)},
    {IR(3174, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(3200, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(3236, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(3260, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3295, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3341, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3361, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3385, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3421, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3456, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3493, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3523, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3555, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3580, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3602, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3623, 14), static_cast<uint32_t>(kSPV_KHR_abort)},
    {IR(3637, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3654, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3679, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3714, 22), static_cast<uint32_t>(kSPV_KHR_constant_data)},
    {IR(3736, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3763, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3784, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3806, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3829, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3853, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3865, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3901, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3931, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(3959, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3980, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(4010, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(4028, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(4063, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(4089, 25), static_cast<uint32_t>(kSPV_KHR_opacity_micromap)},
    {IR(4114, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(4146, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(4168, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4196, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4217, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4239, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4257, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4277, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4312, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4349, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4383, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4405, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4426, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4457, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4494, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4518, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4556, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4578, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4607, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4642, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4667, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4693, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4721, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4762, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4800, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4824, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4862, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4896, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4922, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(4949, 40), static_cast<uint32_t>(kSPV_NV_cooperative_matrix_decode_vector)},
    {IR(4989, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(5015, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(5044, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(5079, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(5114, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(5142, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(5161, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(5187, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(5212, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5231, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5262, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5299, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5332, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5362, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5395, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5421, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5456, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5476, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5505, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5530, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5553, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5592, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5618, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5645, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing3)},
    {IR(5672, 30), static_cast<uint32_t>(kSPV_QCOM_multiple_wait_queues)},
    {IR(5702, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5724, 22), static_cast<uint32_t>(kSPV_SAMSUNG_intrinsic)},
    {IR(5746, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5784, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1242> kOperandsByValue{{
  {0,IR(0, 0),IR(5818, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5834, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5844, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5854, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5862, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5883, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(5894, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(5976, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(5992, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(6002, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(6015, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(6028, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(6037, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(6048, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(6138, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(6151, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(6197, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(6245, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(6260, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(6275, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(6285, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(6299, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6309, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6320, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6332, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6359, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6374, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6385, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6395, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6412, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6426, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6440, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6452, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6470, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6489, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6510, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6518, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6529, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6551, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6564, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6579, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6626, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6642, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6655, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6676, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6687, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6713, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6725, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6739, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6765, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6778, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6791, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6801, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6814, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6869, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(6902, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(6935, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(6968, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(7001, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(7027, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(7040, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(7050, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(7097, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(7121, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(7141, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(7156, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(7187, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(7205, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(7223, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(7243, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(7271, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(7297, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7316, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7343, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7368, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7390, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7425, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7472, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7488, 15)/* SamplerHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7521, 16)/* ResourceHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7537, 15)/* ViewportMaskNV */,IR(0, 0),IR(45, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7573, 20)/* SecondaryPositionNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7612, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7636, 18)/* PositionPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7674, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7696, 16)/* FullyCoveredEXT */,IR(0, 0),IR(50, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7736, 12)/* TaskCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7748, 17)/* PrimitiveCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7765, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7784, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7806, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7828, 15)/* LayerPerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7843, 16)/* MeshViewCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7859, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7877, 13)/* BaryCoordKHR */,IR(6, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(7925, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(7964, 12)/* FragSizeEXT */,IR(8, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(8010, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(8055, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(8080, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(8104, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(8132, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(8149, 12)/* LaunchIdKHR */,IR(10, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(8172, 14)/* LaunchSizeKHR */,IR(11, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(8199, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(8234, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(8275, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8312, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8355, 11)/* RayTminKHR */,IR(16, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8376, 11)/* RayTmaxKHR */,IR(17, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8397, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8442, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8475, 17)/* WorldToObjectKHR */,IR(20, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8508, 7)/* HitTNV */,IR(0, 0),IR(57, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8515, 11)/* HitKindKHR */,IR(21, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8536, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(58, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8576, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8633, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8700, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8737, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8776, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(61, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8796, 14)/* HitIsSphereNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8838, 11)/* HitIsLSSNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(8888, 20)/* HitSpherePositionNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(8908, 13)/* WarpsPerSMNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(8940, 10)/* SMCountNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(8950, 9)/* WarpIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(8959, 7)/* SMIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(8966, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(8984, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(9018, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(9051, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(9069, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(9083, 12)/* ClusterIDNV */,IR(0, 0),IR(65, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(9136, 12)/* CullMaskKHR */,IR(0, 0),IR(66, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(9163, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(5985, 7)/* Shader */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(6060, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(6069, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5873, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(9170, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5827, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(9178, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(9187, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(9201, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(9209, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(9217, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(9223, 13)/* Int64Atomics */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(9236, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(9247, 15)/* ImageReadWrite */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(9262, 12)/* ImageMipmap */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(9274, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(9280, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(9287, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(9301, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9316, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9330, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9336, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9358, 18)/* GeometryPointSize */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9376, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9396, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9420, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9454, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9487, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9521, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(6002, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(6015, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9554, 15)/* ImageCubeArray */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6341, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9586, 10)/* ImageRect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9596, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9608, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9623, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9628, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9644, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9660, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9667, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9677, 8)/* Image1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9569, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9685, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9699, 12)/* ImageBuffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9711, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9724, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9752, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9763, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9781, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9803, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9821, 16)/* GeometryStreams */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9837, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(9867, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(6211, 14)/* MultiViewport */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(9898, 17)/* SubgroupDispatch */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(9915, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(9928, 12)/* PipeStorage */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6592, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(9940, 20)/* GroupNonUniformVote */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(9960, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6847, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(9986, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(10009, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(10040, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(10065, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(6157, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(6225, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(10085, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6749, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(10103, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(10131, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(10159, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(10189, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(10200, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(10237, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(10277, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(10286, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10314, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10324, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(78, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4228,IR(0, 0),IR(10372, 10)/* Float6EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4229,IR(0, 0),IR(10382, 10)/* Float4EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4230,IR(0, 0),IR(10392, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4231,IR(0, 0),IR(10414, 10)/* MXInt8EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4232,IR(0, 0),IR(10424, 18)/* BitcastExtractEXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(7074, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6608, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(7012, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10442, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10475, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(80, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10518, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(80, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10562, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10578, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10631, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10683, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10705, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(7109, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(7131, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10726, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10756, 17)/* VariablePointers */,IR(0, 0),IR(82, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10773, 17)/* AtomicStorageOps */,IR(0, 0),IR(83, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10790, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10818, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10842, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(84, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(10876, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(10897, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(10912, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(10930, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(10955, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(10971, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(10987, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(11010, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(11022, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(11041, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(85, 2),IR(51, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(6095, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(11073, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(11099, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(11120, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(7171, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(11142, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(87, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(11174, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4539,IR(0, 0),IR(11197, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4543,IR(0, 0),IR(11220, 22)/* ImageGatherLinearQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4544,IR(0, 0),IR(11242, 29)/* ImageGatherExtendedModesQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(11271, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(11287, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(11309, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7408, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(11325, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(11346, 14)/* Int64ImageEXT */,IR(0, 0),IR(68, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(11360, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7454, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(11375, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(11390, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(11404, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(88, 2),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(11431, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(11447, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(90, 1),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11469, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(91, 2),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5120,IR(0, 0),IR(11498, 9)/* AbortKHR */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7503, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(93, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5146,IR(0, 0),IR(11507, 16)/* ConstantDataKHR */,IR(0, 0),IR(1, 0),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11523, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5181,IR(0, 0),IR(11539, 15)/* WeakLinkageAMD */,IR(0, 0),IR(94, 1),IR(73, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11554, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11583, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(70, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(6169, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(95, 1),IR(76, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7552, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(96, 1),IR(78, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7593, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(97, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7654, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7712, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(6109, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11638, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(6123, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(7902, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11677, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(7991, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11752, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(11811, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(11848, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(11897, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(98, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(11972, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(74, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(12053, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(99, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(12134, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(12211, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(12286, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(12363, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(12438, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(101, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12519, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(103, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12606, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(105, 2),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8606, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(6082, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8553, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12693, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12732, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(5945, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(12827, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(12890, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(12915, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(12935, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(12968, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(8921, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(13006, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(13038, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(90, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(13091, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(13114, 29)/* RayTracingOpacityMicromapKHR */,IR(47, 1),IR(3, 1),IR(92, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(13172, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(61, 1),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(13198, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(61, 1),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(13225, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(13243, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(13268, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(13288, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8667, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(61, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(13310, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8810, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8849, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(13328, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(13348, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(13362, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(13385, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(13415, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(13446, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(13486, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13522, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13552, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(9095, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(61, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13580, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5447,IR(0, 0),IR(13599, 32)/* CooperativeMatrixDecodeVectorNV */,IR(0, 0),IR(107, 1),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13631, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(13652, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(13679, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(13705, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(13736, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(13757, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(13780, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(13803, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(13825, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(13849, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(13858, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(13881, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(13904, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(13927, 19)/* VectorComputeINTEL */,IR(0, 0),IR(108, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(13946, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(13961, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(13977, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(14010, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(14048, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(14087, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(14112, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(14138, 27)/* FPGAMemoryAttributesALTERA */,IR(48, 1),IR(1, 0),IR(117, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(14191, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(14211, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(49, 1),IR(1, 0),IR(120, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(14276, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(50, 1),IR(1, 0),IR(122, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(14351, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(14381, 23)/* FPGALoopControlsALTERA */,IR(51, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(14426, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(14448, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(14474, 25)/* FPGAMemoryAccessesALTERA */,IR(52, 1),IR(1, 0),IR(128, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(14523, 28)/* FPGAClusterAttributesALTERA */,IR(53, 1),IR(1, 0),IR(130, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14578, 15)/* LoopFuseALTERA */,IR(54, 1),IR(1, 0),IR(132, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14607, 21)/* FPGADSPControlALTERA */,IR(55, 1),IR(1, 0),IR(134, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(14648, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(14674, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(56, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(14755, 25)/* FPGABufferLocationALTERA */,IR(57, 1),IR(1, 0),IR(139, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(14804, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(58, 1),IR(1, 0),IR(141, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(14873, 24)/* USMStorageClassesALTERA */,IR(59, 1),IR(1, 0),IR(143, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(14920, 30)/* RuntimeAlignedAttributeALTERA */,IR(60, 1),IR(1, 0),IR(145, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(14979, 14)/* IOPipesALTERA */,IR(61, 1),IR(1, 0),IR(147, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(15006, 20)/* BlockingPipesALTERA */,IR(62, 1),IR(1, 0),IR(149, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(15045, 14)/* FPGARegALTERA */,IR(63, 1),IR(1, 0),IR(151, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(15072, 19)/* DotProductInputAll */,IR(64, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(15113, 22)/* DotProductInput4x8Bit */,IR(65, 1),IR(109, 1),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(15160, 28)/* DotProductInput4x8BitPacked */,IR(66, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(15219, 11)/* DotProduct */,IR(67, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(9148, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10351, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(154, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(15244, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(155, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(15268, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(156, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(15284, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(77, 1),IR(157, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(15309, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(158, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(15324, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6032,IR(0, 0),IR(15331, 42)/* RayTracingOpacityMicromapExecutionModeKHR */,IR(0, 0),IR(3, 1),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(15373, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(15393, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(15413, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(15433, 11)/* OptNoneEXT */,IR(68, 1),IR(1, 0),IR(163, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(15457, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(15477, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(15498, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(167, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(15522, 16)/* SplitBarrierEXT */,IR(69, 1),IR(1, 0),IR(168, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(15556, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(170, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(15575, 30)/* FPGAClusterAttributesV2ALTERA */,IR(70, 1),IR(110, 1),IR(130, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15634, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(111, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(15662, 19)/* TaskSequenceALTERA */,IR(71, 1),IR(1, 0),IR(171, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(15699, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(173, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(15715, 25)/* FPGALatencyControlALTERA */,IR(72, 1),IR(1, 0),IR(174, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(15764, 29)/* FPGAArgumentInterfacesALTERA */,IR(73, 1),IR(1, 0),IR(176, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(15821, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(15851, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(74, 1),IR(1, 0),IR(179, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(15922, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(181, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(15950, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(15973, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(112, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(16003, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(112, 1),IR(182, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(16033, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(183, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(16071, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(184, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(16099, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(113, 2),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(16131, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(16152, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(115, 1),IR(185, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6257,IR(0, 0),IR(16174, 18)/* PredicatedIOINTEL */,IR(0, 0),IR(1, 0),IR(186, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6265,IR(0, 0),IR(16192, 23)/* RoundedDivideSqrtINTEL */,IR(0, 0),IR(1, 0),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(16215, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(16241, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(16268, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(190, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(16293, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(191, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(16312, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(16332, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(16352, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(116, 1),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(16385, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(116, 1),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(16418, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(90, 1),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(16445, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(117, 1),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7041,IR(0, 0),IR(16477, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(1, 0),IR(195, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(16494, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16506, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16514, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16522, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16528, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16535, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16546, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16555, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16568, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16574, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16584, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16590, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16605, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16630, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16646, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16660, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16673, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16685, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16702, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16714, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16729, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16742, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16757, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16775, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16792, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16813, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16833, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16853, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16869, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16885, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16905, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16929, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16935, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16940, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(16979, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(16990, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16999, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17004, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17011, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(17022, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(17029, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17038, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17048, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17061, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17074, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17085, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(17095, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(17105, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(17115, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(17128, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(17142, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(17156, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(17170, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(17185, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(17201, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(17217, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(17233, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(17252, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(17273, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(17285, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(17297, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17309, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(17324, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(17349, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(17377, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17385, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(17412, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(17439, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(17466, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(17498, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17524, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(17528, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(17535, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(17539, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(17550, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(17561, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(17582, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17593, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(17607, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(17628, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16494, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16506, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16514, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16522, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16528, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16535, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16546, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(16555, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16568, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16574, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16584, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16630, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16646, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16660, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16673, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16685, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16702, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16714, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16729, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16742, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16757, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16775, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16792, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16813, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16833, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16853, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16929, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(16935, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(16940, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(16979, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(16990, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(16999, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(17004, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(17011, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(17022, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(17038, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17048, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17061, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17646, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(17663, 7)/* SpecId */,IR(0, 0),IR(119, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(17670, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(17676, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(17688, 9)/* RowMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(17697, 9)/* ColMajor */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(17706, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(17718, 13)/* MatrixStride */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(17731, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(17742, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(17753, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(17787, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(17795, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(17809, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(17814, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(17820, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(17829, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(17836, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(17846, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(17855, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(17863, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(17872, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(17881, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(17890, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(17902, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(17914, 8)/* Uniform */,IR(0, 0),IR(121, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(17948, 10)/* UniformId */,IR(0, 0),IR(121, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(17958, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(17978, 7)/* Stream */,IR(0, 0),IR(123, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(17985, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(17994, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(18004, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(18010, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(18018, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(18032, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(18039, 10)/* XfbBuffer */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(18049, 10)/* XfbStride */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(18105, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(18153, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(18203, 15)/* FPFastMathMode */,IR(0, 0),IR(125, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(18280, 18)/* LinkageAttributes */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(18298, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(18312, 21)/* InputAttachmentIndex */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(18333, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(18343, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(18377, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(18389, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(18405, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(18449, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(196, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(18462, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(196, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(18477, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(18495, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(18517, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(18539, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(18557, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(18589, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(18609, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(18632, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(18652, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(18677, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(18704, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(18729, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(18757, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(18774, 12)/* OffsetIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5145,IR(0, 0),IR(18786, 14)/* UTFEncodedKHR */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(18800, 19)/* OverrideCoverageNV */,IR(0, 0),IR(128, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(18819, 14)/* PassthroughNV */,IR(0, 0),IR(129, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(18833, 19)/* ViewportRelativeNV */,IR(0, 0),IR(97, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(18852, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(18880, 16)/* PerPrimitiveEXT */,IR(75, 1),IR(130, 2),IR(197, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(18911, 10)/* PerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(18921, 10)/* PerTaskNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(18931, 13)/* PerVertexKHR */,IR(76, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(18956, 11)/* NonUniform */,IR(77, 1),IR(100, 1),IR(85, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(18981, 16)/* RestrictPointer */,IR(78, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(19016, 15)/* AliasedPointer */,IR(79, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(19049, 15)/* MemberOffsetNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(19064, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(19094, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(19125, 7)/* BankNV */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(19132, 18)/* BindlessSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(19150, 16)/* BindlessImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(19166, 15)/* BoundSamplerNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(19181, 13)/* BoundImageNV */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(19194, 14)/* SIMTCallINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(19208, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(137, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(19234, 13)/* ClobberINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(19247, 17)/* SideEffectsINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(19264, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(19291, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(19312, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(19339, 15)/* StackCallINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(19354, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(19380, 14)/* CounterBuffer */,IR(80, 1),IR(1, 0),IR(199, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(19418, 13)/* UserSemantic */,IR(81, 1),IR(1, 0),IR(199, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(19450, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(19465, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(19522, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(19546, 15)/* RegisterALTERA */,IR(82, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(19575, 13)/* MemoryALTERA */,IR(83, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(19600, 15)/* NumbanksALTERA */,IR(84, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(19629, 16)/* BankwidthALTERA */,IR(85, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(19660, 23)/* MaxPrivateCopiesALTERA */,IR(86, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(19705, 17)/* SinglepumpALTERA */,IR(87, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(19738, 17)/* DoublepumpALTERA */,IR(88, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(19771, 20)/* MaxReplicatesALTERA */,IR(89, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(19810, 21)/* SimpleDualPortALTERA */,IR(90, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(19851, 12)/* MergeALTERA */,IR(91, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(19916, 15)/* BankBitsALTERA */,IR(92, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(19945, 21)/* ForcePow2DepthALTERA */,IR(93, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(19986, 17)/* StridesizeALTERA */,IR(94, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(20019, 15)/* WordsizeALTERA */,IR(95, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(20048, 19)/* TrueDualPortALTERA */,IR(96, 1),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(20085, 20)/* BurstCoalesceALTERA */,IR(97, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(20124, 16)/* CacheSizeALTERA */,IR(98, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(20155, 29)/* DontStaticallyCoalesceALTERA */,IR(99, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(20212, 15)/* PrefetchALTERA */,IR(100, 1),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(20241, 18)/* StallEnableALTERA */,IR(101, 1),IR(110, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(20276, 26)/* FuseLoopsInFunctionALTERA */,IR(102, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(20327, 20)/* MathOpDSPModeALTERA */,IR(103, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(20366, 16)/* AliasScopeINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(20382, 13)/* NoAliasINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(20395, 25)/* InitiationIntervalALTERA */,IR(104, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(20444, 21)/* MaxConcurrencyALTERA */,IR(105, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(20485, 21)/* PipelineEnableALTERA */,IR(106, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(20526, 21)/* BufferLocationALTERA */,IR(107, 1),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(20567, 20)/* IOPipeStorageALTERA */,IR(108, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(20640, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(20671, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(20696, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(20731, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(20749, 16)/* StallFreeALTERA */,IR(109, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(20811, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(20837, 26)/* LatencyControlLabelALTERA */,IR(110, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(20888, 31)/* LatencyControlConstraintALTERA */,IR(111, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(20949, 28)/* ConduitKernelArgumentALTERA */,IR(112, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(21004, 32)/* RegisterMapKernelArgumentALTERA */,IR(113, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(21067, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(114, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(21134, 31)/* MMHostInterfaceDataWidthALTERA */,IR(115, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(21195, 29)/* MMHostInterfaceLatencyALTERA */,IR(116, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(21286, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(117, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(21355, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(118, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(21414, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(119, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(21479, 27)/* StableKernelArgumentALTERA */,IR(120, 1),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(21571, 16)/* HostAccessINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(21634, 15)/* InitModeALTERA */,IR(121, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(21663, 29)/* ImplementInRegisterMapALTERA */,IR(122, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(21720, 17)/* ConditionalINTEL */,IR(0, 0),IR(115, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(21773, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(21832, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7040,IR(0, 1),IR(16477, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(21855, 3)/* 1D */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(21858, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(21861, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(21864, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(21869, 5)/* Rect */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(21874, 7)/* Buffer */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(21881, 12)/* SubpassData */,IR(0, 0),IR(98, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(21893, 17)/* TileImageDataEXT */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(21910, 12)/* Invocations */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(21922, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(21935, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(21957, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(21978, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(21992, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(22007, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(22026, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(22042, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(22058, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(22077, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(22087, 4)/* Xfb */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(22091, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(22106, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(22119, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(22129, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(22144, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(22154, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(22168, 12)/* InputPoints */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(22180, 11)/* InputLines */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(22191, 20)/* InputLinesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(22211, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(22221, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(22245, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(22251, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(22260, 15)/* OutputVertices */,IR(0, 0),IR(157, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(22275, 13)/* OutputPoints */,IR(0, 0),IR(161, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(22288, 16)/* OutputLineStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(22304, 20)/* OutputTriangleStrip */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(22324, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(22336, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(22351, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(22363, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6579, 13)/* SubgroupSize */,IR(0, 0),IR(164, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(22373, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(164, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(22395, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(164, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(22419, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(22431, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(22447, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(22481, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(165, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(22515, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(166, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(22551, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(22581, 18)/* PostDepthCoverage */,IR(0, 0),IR(167, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(10897, 15)/* DenormPreserve */,IR(0, 0),IR(168, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(10912, 18)/* DenormFlushToZero */,IR(0, 0),IR(169, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(10930, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(170, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(10955, 16)/* RoundingModeRTE */,IR(0, 0),IR(171, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(10971, 16)/* RoundingModeRTZ */,IR(0, 0),IR(172, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(22599, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(22633, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(22653, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(202, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(22682, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(22705, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(22720, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(22735, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(22756, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7472, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(22780, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(22801, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(22829, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(22855, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(22878, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(22905, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(22930, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(22952, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(22971, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(173, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(22991, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(23011, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(174, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(23031, 15)/* OutputLinesEXT */,IR(123, 1),IR(130, 2),IR(197, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(23060, 20)/* OutputPrimitivesEXT */,IR(124, 1),IR(130, 2),IR(197, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(23099, 24)/* DerivativeGroupQuadsKHR */,IR(125, 1),IR(175, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(23146, 25)/* DerivativeGroupLinearKHR */,IR(126, 1),IR(176, 1),IR(81, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(23195, 19)/* OutputTrianglesEXT */,IR(127, 1),IR(130, 2),IR(197, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(23232, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(177, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(23257, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(177, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(23284, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(178, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(23310, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(178, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(23338, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(179, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(23369, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(179, 1),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(13362, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(180, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(23402, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(23429, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(23450, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(23471, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(23497, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(181, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(23524, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(182, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(23546, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(182, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(23562, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(182, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(23582, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(111, 1),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(23604, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(23632, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(205, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(23656, 18)/* FPFastMathDefault */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6031,IR(10, 1),IR(23674, 21)/* OpacityMicromapIdKHR */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(23695, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(23719, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(23745, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(23768, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(23790, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(23865, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(23892, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(23899, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(23919, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(6060, 9)/* Geometry */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(17029, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(23942, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5827, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(23952, 7)/* TaskNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(23959, 7)/* MeshNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(23966, 17)/* RayGenerationKHR */,IR(128, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(23999, 16)/* IntersectionKHR */,IR(129, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(24030, 10)/* AnyHitKHR */,IR(130, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(24049, 14)/* ClosestHitKHR */,IR(131, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(24076, 8)/* MissKHR */,IR(132, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(24091, 12)/* CallableKHR */,IR(133, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(24114, 8)/* TaskEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(24122, 8)/* MeshEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(24130, 9)/* Preserve */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(24139, 12)/* FlushToZero */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(24151, 12)/* BFloat16KHR */,IR(0, 0),IR(90, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(24163, 14)/* Float8E4M3EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(24177, 14)/* Float8E5M2EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4223,IR(0, 0),IR(24191, 14)/* Float6E2M3EXT */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4224,IR(0, 0),IR(24205, 14)/* Float6E3M2EXT */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4225,IR(0, 0),IR(24219, 14)/* Float4E2M1EXT */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4226,IR(0, 0),IR(10392, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4227,IR(0, 0),IR(10414, 10)/* MXInt8EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(24233, 5)/* IEEE */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(24238, 4)/* ALT */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(24242, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(24249, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(24256, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(24260, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(24271, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(24276, 14)/* AllowContract */,IR(134, 1),IR(191, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(24313, 13)/* AllowReassoc */,IR(135, 1),IR(191, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(24344, 15)/* AllowTransform */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(24359, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24363, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(24367, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(24371, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24375, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(24391, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(24407, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(24425, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(24443, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(24450, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(24461, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(24466, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(15433, 11)/* OptNoneEXT */,IR(68, 1),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(24472, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(24477, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(24482, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(24488, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(24493, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(24501, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(24511, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(24519, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(24531, 21)/* RuntimeAlignedALTERA */,IR(136, 1),IR(194, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(24572, 14)/* Gather4x1QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {1,IR(0, 0),IR(24586, 12)/* GatherDQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {2,IR(0, 0),IR(24598, 13)/* GatherH2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {3,IR(0, 0),IR(24611, 13)/* GatherV2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {0,IR(0, 0),IR(24624, 7)/* Reduce */,IR(0, 0),IR(195, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(24631, 14)/* InclusiveScan */,IR(0, 0),IR(195, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(24645, 14)/* ExclusiveScan */,IR(0, 0),IR(195, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(24659, 16)/* ClusteredReduce */,IR(0, 0),IR(198, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(24675, 21)/* PartitionedReduceEXT */,IR(137, 1),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(24716, 28)/* PartitionedInclusiveScanEXT */,IR(138, 1),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(24771, 28)/* PartitionedExclusiveScanEXT */,IR(139, 1),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(24826, 10)/* NoneINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(24836, 10)/* ReadINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(24846, 11)/* WriteINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(24857, 15)/* ReadWriteINTEL */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(24872, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(24877, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(24881, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(24886, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(18032, 7)/* Offset */,IR(0, 0),IR(200, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(24898, 13)/* ConstOffsets */,IR(0, 0),IR(200, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(17829, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9660, 7)/* MinLod */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(24911, 19)/* MakeTexelAvailable */,IR(140, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(24952, 17)/* MakeTexelVisible */,IR(141, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(24989, 16)/* NonPrivateTexel */,IR(142, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(25024, 14)/* VolatileTexel */,IR(143, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(25055, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(25066, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(25077, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(25089, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(25097, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(25107, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(25118, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(25128, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(25139, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(25153, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(25167, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(25182, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(25193, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(25205, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(25217, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(25230, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(25244, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(25258, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(16522, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(25268, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(25279, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(25296, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(25312, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(25332, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(25352, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(25372, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(25391, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(25410, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(25429, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(25445, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(25461, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(25463, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(25465, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(25468, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(25471, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(25475, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(25480, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(25485, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(25490, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(25500, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(25510, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(25513, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(25517, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(25522, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(25528, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(25541, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(25546, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(25552, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(25558, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(25564, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(25569, 28)/* InitOnDeviceReprogramALTERA */,IR(144, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(25624, 24)/* InitOnDeviceResetALTERA */,IR(145, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(25671, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(25678, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(25689, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(25703, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(25715, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(25728, 7)/* Export */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(25735, 7)/* Import */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(25742, 12)/* LinkOnceODR */,IR(0, 0),IR(94, 1),IR(206, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {3,IR(0, 0),IR(25754, 8)/* WeakAMD */,IR(0, 0),IR(203, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(25762, 14)/* UncachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(25776, 12)/* CachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(25788, 15)/* StreamingINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(25803, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(25828, 17)/* ConstCachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(25845, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(25852, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(25863, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(25882, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(25899, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(25913, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(25927, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(25945, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(25955, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(20395, 25)/* InitiationIntervalALTERA */,IR(104, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(20444, 21)/* MaxConcurrencyALTERA */,IR(105, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(25968, 22)/* DependencyArrayALTERA */,IR(146, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(20485, 21)/* PipelineEnableALTERA */,IR(106, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(26011, 19)/* LoopCoalesceALTERA */,IR(147, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(26048, 22)/* MaxInterleavingALTERA */,IR(148, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(26091, 27)/* SpeculatedIterationsALTERA */,IR(149, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(26144, 15)/* NoFusionALTERA */,IR(150, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(26173, 16)/* LoopCountALTERA */,IR(151, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(26204, 27)/* MaxReinvocationDelayALTERA */,IR(152, 1),IR(204, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {268435456,IR(0, 1),IR(11197, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(26257, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(26286, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(26315, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(26336, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(26362, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(26385, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(26408, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(26431, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(26454, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(26471, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(26488, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(26514, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(26540, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(26567, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(17863, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(26594, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(25077, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(26602, 21)/* MakePointerAvailable */,IR(153, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(26647, 19)/* MakePointerVisible */,IR(154, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(26688, 18)/* NonPrivatePointer */,IR(155, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(26727, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(144, 1),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(26747, 17)/* NoAliasINTELMask */,IR(0, 0),IR(144, 1),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(26764, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(26771, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(26779, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(26786, 7)/* Vulkan */,IR(156, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(26803, 8)/* Relaxed */,IR(157, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(26811, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(26819, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(26827, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(26842, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(26865, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(26879, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(26894, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(26910, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(26931, 20)/* AtomicCounterMemory */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(26951, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(26963, 13)/* OutputMemory */,IR(158, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(26992, 14)/* MakeAvailable */,IR(159, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(27023, 12)/* MakeVisible */,IR(160, 1),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(17863, 9)/* Volatile */,IR(0, 0),IR(202, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(27050, 10)/* AutoINTEL */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(27060, 5)/* WRAP */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(27065, 4)/* SAT */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(27069, 9)/* SAT_ZERO */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(27078, 8)/* SAT_SYM */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(27086, 25)/* PackedVectorFormat4x8Bit */,IR(161, 1),IR(1, 0),IR(153, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(27139, 4)/* TRN */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(27143, 9)/* TRN_ZERO */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(27152, 4)/* RND */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(27156, 9)/* RND_ZERO */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(27165, 8)/* RND_INF */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(27173, 12)/* RND_MIN_INF */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(27185, 9)/* RND_CONV */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(27194, 13)/* RND_CONV_ODD */,IR(0, 0),IR(205, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(27207, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(27232, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(17377, 8)/* NoneKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(27255, 10)/* OpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(27265, 12)/* NoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(27277, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(27300, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(27324, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(27351, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(27379, 14)/* CullOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(27393, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(85, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(27409, 17)/* SkipTrianglesKHR */,IR(162, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(27450, 13)/* SkipAABBsKHR */,IR(0, 0),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(27463, 30)/* ForceOpacityMicromap2StateKHR */,IR(163, 1),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(27523, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27564, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27601, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27638, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(27679, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(27721, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(27754, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(27787, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(27799, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(27805, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(27812, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(27827, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(27835, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(27842, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(27850, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(27858, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(27866, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(27871, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(27877, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(27888, 6)/* Rg32f */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(27894, 6)/* Rg16f */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(27900, 13)/* R11fG11fB10f */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(27913, 5)/* R16f */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(27918, 7)/* Rgba16 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(27925, 8)/* Rgb10A2 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(27933, 5)/* Rg16 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(27938, 4)/* Rg8 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(27942, 4)/* R16 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(27946, 3)/* R8 */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(27949, 12)/* Rgba16Snorm */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(27961, 10)/* Rg16Snorm */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(27971, 9)/* Rg8Snorm */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(27980, 9)/* R16Snorm */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(27989, 8)/* R8Snorm */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(27997, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(28005, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(28013, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(28020, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(28025, 6)/* Rg32i */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(28031, 6)/* Rg16i */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(28037, 5)/* Rg8i */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(28042, 5)/* R16i */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(28047, 4)/* R8i */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(28051, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(28060, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(28069, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(28077, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(28083, 10)/* Rgb10a2ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(28093, 7)/* Rg32ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(28100, 7)/* Rg16ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(28107, 6)/* Rg8ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(28113, 6)/* R16ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(28119, 5)/* R8ui */,IR(0, 0),IR(210, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(28124, 6)/* R64ui */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(28130, 5)/* R64i */,IR(0, 0),IR(211, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(28135, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(28147, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(28154, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(28164, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(28173, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(28184, 12)/* QueueFamily */,IR(164, 1),IR(202, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(28211, 14)/* ShaderCallKHR */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28225, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(28233, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28245, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(16494, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16506, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16514, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16522, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16528, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16535, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16546, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16555, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16568, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16574, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16584, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16590, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16605, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16630, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16646, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16660, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(16673, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(16685, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(16702, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(16714, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(16729, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(16742, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(16757, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(16775, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(16792, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(16813, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(16833, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(16853, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(16869, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(16885, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(16905, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(28274, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(16929, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(16935, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(16940, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(16979, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(16990, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(16999, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17004, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17011, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(17022, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(17029, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17038, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17048, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17061, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17074, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(27842, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(28300, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(28305, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(28310, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(28319, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(28330, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(28335, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(28350, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(28355, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(28362, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(28367, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(28372, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(28378, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(28382, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {14,IR(0, 0),IR(28387, 5)/* Pred */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {15,IR(0, 0),IR(28392, 9)/* ApilaJai */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(28401, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(28417, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(17914, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(28423, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(28154, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(28430, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(28445, 8)/* Private */,IR(0, 0),IR(212, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(28453, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(28462, 8)/* Generic */,IR(0, 0),IR(214, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(28470, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(28483, 14)/* AtomicCounter */,IR(0, 0),IR(83, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(28497, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(28503, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(207, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(28517, 13)/* TileImageEXT */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(28530, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(28549, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(28565, 16)/* CallableDataKHR */,IR(165, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(28596, 24)/* IncomingCallableDataKHR */,IR(166, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(28643, 14)/* RayPayloadKHR */,IR(167, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(28670, 16)/* HitAttributeKHR */,IR(168, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(28701, 22)/* IncomingRayPayloadKHR */,IR(169, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(28744, 22)/* ShaderRecordBufferKHR */,IR(170, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(28787, 22)/* PhysicalStorageBuffer */,IR(171, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(28834, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(28855, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(54, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(28879, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(28901, 17)/* CodeSectionINTEL */,IR(0, 0),IR(215, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(28918, 17)/* DeviceOnlyALTERA */,IR(172, 1),IR(216, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(28951, 15)/* HostOnlyALTERA */,IR(173, 1),IR(216, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(25762, 14)/* UncachedINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(28980, 18)/* WriteThroughINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(28998, 15)/* WriteBackINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(25788, 15)/* StreamingINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(16625, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(29013, 11)/* TensorView */,IR(0, 0),IR(217, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(29024, 11)/* DecodeFunc */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {4,IR(10, 1),IR(29035, 17)/* DecodeVectorFunc */,IR(0, 0),IR(218, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(29052, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(17872, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(27787, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(27805, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(27812, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(29062, 8)/* NoneARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(29070, 15)/* NontemporalARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(29085, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(29105, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(29129, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(29151, 21)/* NonPrivateElementARM */,IR(0, 0),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(135, 301);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(436, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(444, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(447, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(449, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(468, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(478, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(482, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(497, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(501, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(507, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(510, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(513, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(517, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(525, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(528, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(544, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(553, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(556, 151);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(707, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(715, 99);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(814, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(831, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(833, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(841, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(843, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(852, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(856, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(860, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(866, 9);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(875, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(879, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(886, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(890, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(907, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(933, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(953, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(955, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(958, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(960, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(962, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(966, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(971, 21);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(992, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1007, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1016, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1020, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1035, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1036, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1040, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1041, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1049, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1052, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1064, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1066, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1069, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1071, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1076, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1078, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1120, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1127, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1130, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1131, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1139, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1142, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1144, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1164, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1174, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1178, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1194, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1223, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1227, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1231, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1236, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(890, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1007, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1040, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(501, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(992, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1049, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(833, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1236, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(135, 301);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1420> kOperandNames{{
  {IR(5818, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5844, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5834, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5854, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5862, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5883, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5894, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5918, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7877, 13), 94}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7890, 12), 94}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7223, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7243, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7925, 20), 95}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7945, 19), 95}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7271, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7368, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7297, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7316, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7343, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7027, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7001, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6002, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7784, 22), 89}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9083, 12), 133}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6765, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6739, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6778, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6015, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7806, 22), 90}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9136, 12), 134}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8132, 17), 101}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8536, 17), 115}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7097, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7040, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6529, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6299, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6385, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8010, 23), 97}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7964, 12), 96}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7390, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7976, 15), 96}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6320, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7696, 16), 85}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6470, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6564, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6551, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6518, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6395, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8838, 11), 122}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8796, 14), 121}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9018, 33), 130}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8984, 34), 129}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8515, 11), 114}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8526, 10), 114}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8966, 18), 128}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9069, 14), 132}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8700, 37), 118}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8633, 34), 117}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8888, 20), 123}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9051, 18), 131}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8508, 7), 113}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8576, 30), 116}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8737, 20), 119}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8757, 19), 119}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8397, 23), 110}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8420, 22), 110}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6037, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6725, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6138, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8033, 22), 97}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8149, 12), 102}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8161, 11), 102}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8172, 14), 103}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8186, 13), 103}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6151, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7828, 15), 91}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6452, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6489, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7843, 16), 92}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7859, 18), 93}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6655, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6642, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6412, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8312, 22), 107}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8334, 21), 107}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8275, 19), 106}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8294, 18), 106}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8442, 17), 111}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8459, 16), 111}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6285, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6309, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5992, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5976, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7636, 18), 83}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7748, 17), 87}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6048, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7765, 19), 88}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8080, 24), 99}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8055, 25), 98}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7050, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8104, 28), 100}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8776, 20), 120}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8376, 11), 109}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8387, 10), 109}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8355, 11), 108}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8366, 10), 108}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7425, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7521, 16), 79}, // 107 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8940, 10), 125}, // 108 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8959, 7), 127}, // 109 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6332, 9), 24}, // 110 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6374, 11), 26}, // 111 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6359, 15), 25}, // 112 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7488, 15), 78}, // 113 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7573, 20), 81}, // 114 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7612, 24), 82}, // 115 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7472, 16), 77}, // 116 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7141, 15), 64}, // 117 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6814, 15), 53}, // 118 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6829, 18), 53}, // 119 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6869, 15), 54}, // 120 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6884, 18), 54}, // 121 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6902, 15), 55}, // 122 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6917, 18), 55}, // 123 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6676, 11), 44}, // 124 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6935, 15), 56}, // 125 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6950, 18), 56}, // 126 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6687, 26), 45}, // 127 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6968, 15), 57}, // 128 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6983, 18), 57}, // 129 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6626, 16), 41}, // 130 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6579, 13), 40}, // 131 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7736, 12), 86}, // 132 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6275, 10), 19}, // 133 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6260, 15), 18}, // 134 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6245, 15), 17}, // 135 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7205, 18), 67}, // 136 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7187, 18), 66}, // 137 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7156, 15), 65}, // 138 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6028, 9), 11}, // 139 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6713, 12), 46}, // 140 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7121, 10), 63}, // 141 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6197, 14), 16}, // 142 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7537, 15), 80}, // 143 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7674, 22), 84}, // 144 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6791, 10), 51}, // 145 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8950, 9), 126}, // 146 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6801, 13), 52}, // 147 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8908, 13), 124}, // 148 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6510, 8), 35}, // 149 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6440, 12), 31}, // 150 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6426, 14), 30}, // 151 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8234, 21), 105}, // 152 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8255, 20), 105}, // 153 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8199, 18), 104}, // 154 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8217, 17), 104}, // 155 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8475, 17), 112}, // 156 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8492, 16), 112}, // 157 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(11498, 9), 274}, // 158 AbortKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5873, 10), 139}, // 159 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14804, 35), 379}, // 160 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14839, 34), 379}, // 161 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14276, 38), 367}, // 162 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14314, 37), 367}, // 163 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14211, 33), 366}, // 164 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14244, 32), 366}, // 165 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15556, 19), 405}, // 166 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13849, 9), 352}, // 167 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15457, 20), 401}, // 168 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13904, 23), 355}, // 169 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13288, 22), 326}, // 170 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15373, 20), 397}, // 171 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13858, 23), 353}, // 172 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15393, 20), 398}, // 173 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13881, 23), 354}, // 174 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9316, 14), 155}, // 175 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10773, 17), 236}, // 176 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15498, 24), 403}, // 177 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11469, 29), 273}, // 178 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11447, 22), 272}, // 179 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11431, 16), 271}, // 180 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16332, 20), 430}, // 181 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13225, 18), 323}, // 182 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15268, 16), 392}, // 183 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10424, 18), 220}, // 184 BitcastExtractEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15006, 20), 383}, // 185 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15026, 19), 383}, // 186 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16293, 19), 428}, // 187 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6002, 13), 165}, // 188 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12827, 32), 311}, // 189 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12859, 31), 311}, // 190 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11677, 31), 290}, // 191 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11708, 30), 290}, // 192 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11507, 16), 276}, // 193 ConstantDataKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13522, 30), 338}, // 194 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11142, 32), 255}, // 195 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13415, 31), 335}, // 196 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13599, 32), 342}, // 197 CooperativeMatrixDecodeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10351, 21), 390}, // 198 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10286, 28), 213}, // 199 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12915, 20), 313}, // 200 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13446, 40), 336}, // 201 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13385, 30), 334}, // 202 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13486, 36), 337}, // 203 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13268, 20), 325}, // 204 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13552, 28), 339}, // 205 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6749, 16), 205}, // 206 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6015, 13), 166}, // 207 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15477, 21), 402}, // 208 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13038, 25), 318}, // 209 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13063, 28), 318}, // 210 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10912, 18), 242}, // 211 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10897, 15), 241}, // 212 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9763, 18), 184}, // 213 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7503, 18), 275}, // 214 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9287, 14), 153}, // 215 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7109, 12), 232}, // 216 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13091, 23), 319}, // 217 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15219, 11), 388}, // 218 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16418, 27), 433}, // 219 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16385, 33), 432}, // 220 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16352, 33), 431}, // 221 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16445, 32), 434}, // 222 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15113, 22), 386}, // 223 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15135, 25), 386}, // 224 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15160, 28), 387}, // 225 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15188, 31), 387}, // 226 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15072, 19), 385}, // 227 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15091, 22), 385}, // 228 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15230, 14), 388}, // 229 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7012, 15), 223}, // 230 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13961, 16), 358}, // 231 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15324, 7), 395}, // 232 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14191, 20), 365}, // 233 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15764, 29), 411}, // 234 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15793, 28), 411}, // 235 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14755, 25), 378}, // 236 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14780, 24), 378}, // 237 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14523, 28), 373}, // 238 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14551, 27), 373}, // 239 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15575, 30), 406}, // 240 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15605, 29), 406}, // 241 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14607, 21), 375}, // 242 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14628, 20), 375}, // 243 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14674, 41), 377}, // 244 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14715, 40), 377}, // 245 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14448, 26), 371}, // 246 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15634, 28), 407}, // 247 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15715, 25), 410}, // 248 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15740, 24), 410}, // 249 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14381, 23), 369}, // 250 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14404, 22), 369}, // 251 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14474, 25), 372}, // 252 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14499, 24), 372}, // 253 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14138, 27), 364}, // 254 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14165, 26), 364}, // 255 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15045, 14), 384}, // 256 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15059, 13), 384}, // 257 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15699, 16), 409}, // 258 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9201, 8), 144}, // 259 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9187, 14), 143}, // 260 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11271, 16), 260}, // 261 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10382, 10), 217}, // 262 Float4EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9209, 8), 145}, // 263 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10372, 10), 216}, // 264 Float6EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10324, 27), 215}, // 265 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10314, 10), 214}, // 266 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10392, 22), 218}, // 267 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15309, 15), 394}, // 268 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13757, 23), 348}, // 269 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7902, 23), 289}, // 270 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11655, 22), 289}, // 271 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7991, 19), 291}, // 272 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7712, 24), 285}, // 273 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11309, 16), 262}, // 274 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13006, 32), 317}, // 275 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12935, 33), 314}, // 276 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12968, 38), 315}, // 277 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7074, 23), 221}, // 278 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14112, 26), 363}, // 279 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13803, 22), 350}, // 280 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16152, 22), 422}, // 281 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9608, 15), 171}, // 282 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6060, 9), 137}, // 283 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9358, 18), 158}, // 284 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11583, 28), 280}, // 285 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9821, 16), 187}, // 286 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15851, 36), 413}, // 287 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15887, 35), 413}, // 288 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15821, 30), 412}, // 289 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10277, 9), 212}, // 290 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6592, 16), 194}, // 291 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9960, 26), 196}, // 292 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6847, 22), 197}, // 293 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10040, 25), 200}, // 294 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11752, 30), 292}, // 295 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11782, 29), 292}, // 296 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10065, 20), 201}, // 297 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15284, 25), 393}, // 298 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9986, 23), 198}, // 299 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10009, 31), 199}, // 300 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9940, 20), 195}, // 301 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16215, 26), 425}, // 302 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9280, 7), 152}, // 303 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14979, 14), 382}, // 304 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14993, 13), 382}, // 305 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9677, 8), 177}, // 306 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9236, 11), 148}, // 307 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9699, 12), 180}, // 308 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9554, 15), 167}, // 309 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11638, 17), 287}, // 310 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11287, 22), 261}, // 311 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9376, 20), 159}, // 312 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11242, 29), 259}, // 313 ImageGatherExtendedModesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11220, 22), 258}, // 314 ImageGatherLinearQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9711, 13), 181}, // 315 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9262, 12), 150}, // 316 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9752, 11), 183}, // 317 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9247, 15), 149}, // 318 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11325, 21), 264}, // 319 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9586, 10), 169}, // 320 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13825, 24), 351}, // 321 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9628, 16), 173}, // 322 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11897, 36), 295}, // 323 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11933, 39), 295}, // 324 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12438, 39), 302}, // 325 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12477, 42), 302}, // 326 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9330, 6), 156}, // 327 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11404, 27), 270}, // 328 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11390, 14), 269}, // 329 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9217, 6), 146}, // 330 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9223, 13), 147}, // 331 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11346, 14), 265}, // 332 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9623, 5), 172}, // 333 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13780, 23), 349}, // 334 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9781, 22), 185}, // 335 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16477, 17), 435}, // 336 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5827, 7), 141}, // 337 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14426, 22), 370}, // 338 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9170, 8), 140}, // 339 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9301, 15), 154}, // 340 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15413, 20), 399}, // 341 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13348, 14), 332}, // 342 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14578, 15), 374}, // 343 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14593, 14), 374}, // 344 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10414, 10), 219}, // 345 MXInt8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16268, 25), 427}, // 346 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9163, 7), 135}, // 347 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14648, 26), 376}, // 348 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6123, 15), 288}, // 349 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6109, 14), 286}, // 350 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9660, 7), 175}, // 351 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7131, 10), 233}, // 352 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6211, 14), 190}, // 353 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11197, 23), 257}, // 354 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9915, 13), 192}, // 355 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15433, 11), 400}, // 356 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15444, 13), 400}, // 357 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7654, 20), 284}, // 358 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5945, 31), 310}, // 359 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12793, 34), 310}, // 360 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9928, 12), 193}, // 361 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9274, 6), 151}, // 362 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11523, 16), 277}, // 363 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16174, 18), 423}, // 364 PredicatedIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13328, 20), 331}, // 365 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11375, 15), 268}, // 366 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13310, 18), 328}, // 367 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9148, 15), 389}, // 368 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11010, 12), 247}, // 369 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13243, 25), 324}, // 370 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10987, 23), 246}, // 371 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9095, 41), 340}, // 372 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8667, 33), 327}, // 373 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6095, 14), 250}, // 374 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8849, 39), 330}, // 375 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8553, 23), 307}, // 376 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6082, 13), 306}, // 377 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13143, 29), 320}, // 378 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15331, 42), 396}, // 379 RayTracingOpacityMicromapExecutionModeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13114, 29), 320}, // 380 RayTracingOpacityMicromapKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8606, 27), 305}, // 381 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12890, 25), 312}, // 382 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8810, 28), 329}, // 383 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11041, 32), 249}, // 384 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16312, 20), 429}, // 385 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15244, 24), 391}, // 386 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13736, 21), 347}, // 387 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16192, 23), 424}, // 388 RoundedDivideSqrtINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10955, 16), 244}, // 389 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10971, 16), 245}, // 390 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14920, 30), 381}, // 391 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14950, 29), 381}, // 392 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11848, 23), 294}, // 393 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11871, 26), 294}, // 394 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11554, 29), 279}, // 395 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10790, 28), 237}, // 396 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6341, 18), 168}, // 397 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9667, 10), 176}, // 398 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9685, 14), 179}, // 399 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9569, 17), 178}, // 400 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9454, 33), 162}, // 401 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12211, 36), 299}, // 402 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12247, 39), 299}, // 403 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9596, 12), 170}, // 404 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5985, 7), 136}, // 405 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13362, 23), 333}, // 406 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11360, 15), 266}, // 407 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7454, 18), 267}, // 408 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13198, 27), 322}, // 409 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13172, 26), 321}, // 410 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6157, 12), 202}, // 411 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11811, 17), 293}, // 412 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11828, 20), 293}, // 413 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8921, 19), 316}, // 414 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7593, 19), 283}, // 415 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6225, 20), 203}, // 416 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6169, 28), 281}, // 417 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11611, 27), 281}, // 418 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7552, 21), 282}, // 419 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11738, 14), 291}, // 420 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10930, 25), 243}, // 421 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9644, 16), 174}, // 422 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16131, 21), 421}, // 423 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15522, 16), 404}, // 424 SplitBarrierEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15538, 18), 404}, // 425 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7408, 17), 263}, // 426 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10578, 25), 228}, // 427 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10818, 24), 238}, // 428 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9487, 34), 163}, // 429 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12286, 37), 300}, // 430 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12323, 40), 300}, // 431 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9521, 33), 164}, // 432 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12363, 36), 301}, // 433 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12399, 39), 301}, // 434 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9724, 28), 182}, // 435 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9396, 24), 160}, // 436 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9837, 30), 188}, // 437 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9867, 31), 189}, // 438 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10705, 21), 231}, // 439 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10683, 22), 230}, // 440 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10876, 21), 240}, // 441 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10200, 37), 210}, // 442 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10237, 40), 211}, // 443 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12053, 39), 297}, // 444 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12092, 42), 297}, // 445 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12606, 42), 304}, // 446 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12648, 45), 304}, // 447 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10666, 17), 229}, // 448 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10603, 28), 228}, // 449 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15950, 23), 415}, // 450 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15973, 30), 416}, // 451 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16003, 30), 417}, // 452 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14048, 39), 361}, // 453 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13977, 33), 359}, // 454 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14010, 38), 360}, // 455 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6608, 18), 222}, // 456 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13652, 27), 344}, // 457 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15922, 28), 414}, // 458 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9898, 17), 191}, // 459 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13679, 26), 345}, // 460 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13705, 31), 346}, // 461 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16033, 38), 418}, // 462 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13631, 21), 343}, // 463 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10562, 16), 227}, // 464 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15662, 19), 408}, // 465 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15681, 18), 408}, // 466 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13580, 19), 341}, // 467 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16241, 27), 426}, // 468 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10189, 11), 209}, // 469 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16071, 28), 419}, // 470 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6069, 13), 138}, // 471 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9336, 22), 157}, // 472 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11174, 23), 256}, // 473 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11120, 22), 253}, // 474 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11099, 21), 252}, // 475 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11073, 26), 251}, // 476 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10103, 28), 206}, // 477 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10131, 28), 207}, // 478 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10159, 30), 208}, // 479 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7171, 16), 254}, // 480 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9803, 18), 186}, // 481 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14873, 24), 380}, // 482 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14897, 23), 380}, // 483 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10631, 35), 229}, // 484 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10842, 34), 239}, // 485 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9420, 34), 161}, // 486 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12134, 37), 298}, // 487 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12171, 40), 298}, // 488 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10085, 18), 204}, // 489 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11972, 39), 296}, // 490 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12011, 42), 296}, // 491 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12519, 42), 303}, // 492 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12561, 45), 303}, // 493 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14351, 30), 368}, // 494 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11022, 19), 248}, // 495 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16099, 32), 420}, // 496 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14087, 25), 362}, // 497 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10756, 17), 235}, // 498 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10726, 30), 234}, // 499 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9178, 9), 142}, // 500 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13946, 15), 357}, // 501 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13927, 19), 356}, // 502 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12693, 18), 308}, // 503 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12732, 29), 309}, // 504 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12761, 32), 309}, // 505 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12711, 21), 308}, // 506 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11539, 15), 278}, // 507 WeakLinkageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10518, 44), 226}, // 508 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10475, 43), 225}, // 509 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10442, 33), 224}, // 510 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16506, 8), 437}, // 511 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16514, 8), 438}, // 512 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16522, 6), 439}, // 513 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16528, 7), 440}, // 514 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16535, 11), 441}, // 515 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16546, 9), 442}, // 516 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16555, 13), 443}, // 517 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16494, 12), 436}, // 518 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16568, 6), 444}, // 519 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16574, 10), 445}, // 520 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16584, 6), 446}, // 521 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16605, 20), 448}, // 522 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16590, 15), 447}, // 523 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16714, 15), 456}, // 524 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16729, 13), 457}, // 525 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16702, 12), 455}, // 526 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16792, 21), 461}, // 527 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16685, 17), 454}, // 528 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16869, 16), 465}, // 529 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16673, 12), 453}, // 530 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16853, 16), 464}, // 531 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16646, 14), 451}, // 532 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16630, 16), 450}, // 533 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16660, 13), 452}, // 534 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16813, 20), 462}, // 535 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16757, 18), 459}, // 536 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16742, 15), 458}, // 537 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16833, 20), 463}, // 538 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16775, 17), 460}, // 539 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16905, 24), 467}, // 540 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16885, 20), 466}, // 541 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16625, 5), 449}, // 542 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16990, 9), 472}, // 543 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17022, 7), 476}, // 544 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16929, 6), 468}, // 545 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17029, 9), 477}, // 546 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16940, 6), 470}, // 547 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16935, 5), 469}, // 548 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16979, 11), 471}, // 549 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17011, 11), 475}, // 550 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(16999, 5), 473}, // 551 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17004, 7), 474}, // 552 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17074, 11), 481}, // 553 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17038, 10), 478}, // 554 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17061, 13), 480}, // 555 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17048, 13), 479}, // 556 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17085, 10), 482}, // 557 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17095, 10), 483}, // 558 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17105, 10), 484}, // 559 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17273, 12), 495}, // 560 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17285, 12), 496}, // 561 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17128, 14), 486}, // 562 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17142, 14), 487}, // 563 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17156, 14), 488}, // 564 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17115, 13), 485}, // 565 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17233, 19), 493}, // 566 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17185, 16), 490}, // 567 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17201, 16), 491}, // 568 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17217, 16), 492}, // 569 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17170, 15), 489}, // 570 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17252, 21), 494}, // 571 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17349, 28), 500}, // 572 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17309, 15), 498}, // 573 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17324, 25), 499}, // 574 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17297, 12), 497}, // 575 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17385, 27), 502}, // 576 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17412, 27), 503}, // 577 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17439, 27), 504}, // 578 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17466, 32), 505}, // 579 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17377, 8), 501}, // 580 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17498, 26), 506}, // 581 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17535, 4), 509}, // 582 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17528, 7), 508}, // 583 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17524, 4), 507}, // 584 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17539, 11), 510}, // 585 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17561, 21), 512}, // 586 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17550, 11), 511}, // 587 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17593, 14), 514}, // 588 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17607, 21), 515}, // 589 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17582, 11), 513}, // 590 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17628, 18), 516}, // 591 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16506, 8), 518}, // 592 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16514, 8), 519}, // 593 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16522, 6), 520}, // 594 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16528, 7), 521}, // 595 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16535, 11), 522}, // 596 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16546, 9), 523}, // 597 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16555, 13), 524}, // 598 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16494, 12), 517}, // 599 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16568, 6), 525}, // 600 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16574, 10), 526}, // 601 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16584, 6), 527}, // 602 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16714, 15), 535}, // 603 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16729, 13), 536}, // 604 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16702, 12), 534}, // 605 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16792, 21), 540}, // 606 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16685, 17), 533}, // 607 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16673, 12), 532}, // 608 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16853, 16), 543}, // 609 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16646, 14), 530}, // 610 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16630, 16), 529}, // 611 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16660, 13), 531}, // 612 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16813, 20), 541}, // 613 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16757, 18), 538}, // 614 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16742, 15), 537}, // 615 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16833, 20), 542}, // 616 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16775, 17), 539}, // 617 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16625, 5), 528}, // 618 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16990, 9), 548}, // 619 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17022, 7), 552}, // 620 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16929, 6), 544}, // 621 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16940, 6), 546}, // 622 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16935, 5), 545}, // 623 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16979, 11), 547}, // 624 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17011, 11), 551}, // 625 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(16999, 5), 549}, // 626 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17004, 7), 550}, // 627 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17038, 10), 553}, // 628 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17061, 13), 555}, // 629 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17048, 13), 554}, // 630 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(20366, 16), 676}, // 631 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17855, 8), 575}, // 632 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(19016, 15), 631}, // 633 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19031, 18), 631}, // 634 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18333, 10), 599}, // 635 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(18377, 12), 601}, // 636 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(17706, 12), 562}, // 637 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18757, 17), 618}, // 638 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19916, 15), 664}, // 639 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19931, 14), 664}, // 640 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19125, 7), 635}, // 641 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19629, 16), 657}, // 642 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19645, 15), 657}, // 643 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18010, 8), 588}, // 644 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(19150, 16), 637}, // 645 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19132, 18), 636}, // 646 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17670, 6), 558}, // 647 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(18517, 22), 608}, // 648 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18495, 22), 607}, // 649 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19181, 13), 639}, // 650 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19166, 15), 638}, // 651 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17676, 12), 559}, // 652 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(20526, 21), 681}, // 653 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20547, 20), 681}, // 654 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17787, 8), 567}, // 655 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(20085, 20), 669}, // 656 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20105, 19), 669}, // 657 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17753, 8), 566}, // 658 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(21773, 22), 704}, // 659 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21832, 23), 705}, // 660 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20124, 16), 670}, // 661 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20140, 15), 670}, // 662 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17820, 9), 571}, // 663 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(19234, 13), 642}, // 664 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17881, 9), 578}, // 665 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(17697, 9), 561}, // 666 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(17994, 10), 586}, // 667 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(21720, 17), 703}, // 668 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20949, 28), 691}, // 669 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20977, 27), 691}, // 670 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17872, 9), 577}, // 671 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(19380, 14), 649}, // 672 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18018, 14), 589}, // 673 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(20155, 29), 671}, // 674 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20184, 28), 671}, // 675 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19738, 17), 660}, // 676 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19755, 16), 660}, // 677 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18539, 18), 609}, // 678 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(18203, 15), 595}, // 679 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(20811, 26), 688}, // 680 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18153, 15), 594}, // 681 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17809, 5), 569}, // 682 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(19945, 21), 665}, // 683 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19966, 20), 665}, // 684 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18105, 14), 593}, // 685 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(19291, 21), 645}, // 686 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19522, 24), 653}, // 687 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20640, 31), 683}, // 688 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19465, 26), 652}, // 689 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20276, 26), 674}, // 690 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20302, 25), 674}, // 691 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17742, 11), 565}, // 692 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(17731, 11), 564}, // 693 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(19354, 26), 648}, // 694 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19094, 31), 634}, // 695 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19064, 30), 633}, // 696 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19394, 24), 649}, // 697 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(19431, 19), 650}, // 698 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21571, 16), 700}, // 699 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20567, 20), 682}, // 700 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20587, 19), 682}, // 701 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21663, 29), 702}, // 702 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21692, 28), 702}, // 703 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18004, 6), 587}, // 704 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(21634, 15), 701}, // 705 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21649, 14), 701}, // 706 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20395, 25), 678}, // 707 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20420, 24), 678}, // 708 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18312, 21), 598}, // 709 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(16477, 17), 706}, // 710 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_DECORATION
  {IR(17836, 10), 573}, // 711 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(20888, 31), 690}, // 712 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20919, 30), 690}, // 713 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20837, 26), 689}, // 714 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20863, 25), 689}, // 715 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18280, 18), 596}, // 716 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(17985, 9), 585}, // 717 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(21067, 34), 693}, // 718 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21101, 33), 693}, // 719 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21134, 31), 694}, // 720 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21165, 30), 694}, // 721 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21195, 29), 695}, // 722 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21224, 28), 695}, // 723 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21355, 30), 697}, // 724 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21385, 29), 697}, // 725 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21286, 35), 696}, // 726 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21321, 34), 696}, // 727 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21414, 33), 698}, // 728 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21447, 32), 698}, // 729 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20327, 20), 675}, // 730 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20347, 19), 675}, // 731 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17718, 13), 563}, // 732 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18343, 14), 600}, // 733 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(18389, 16), 602}, // 734 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(20444, 21), 679}, // 735 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20465, 20), 679}, // 736 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19660, 23), 658}, // 737 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19683, 22), 658}, // 738 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19771, 20), 661}, // 739 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19791, 19), 661}, // 740 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20731, 18), 686}, // 741 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19049, 15), 632}, // 742 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19575, 13), 655}, // 743 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19588, 12), 655}, // 744 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19851, 12), 663}, // 745 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19863, 11), 663}, // 746 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20382, 13), 677}, // 747 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18298, 14), 597}, // 748 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(17795, 14), 568}, // 749 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(18449, 13), 604}, // 750 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18462, 15), 605}, // 751 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18589, 20), 611}, // 752 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18557, 32), 610}, // 753 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17902, 12), 580}, // 754 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(18956, 11), 629}, // 755 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18967, 14), 629}, // 756 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17890, 12), 579}, // 757 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(19600, 15), 656}, // 758 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19615, 14), 656}, // 759 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18032, 7), 590}, // 760 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(18774, 12), 619}, // 761 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18800, 19), 621}, // 762 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18819, 14), 622}, // 763 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17814, 6), 570}, // 764 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(18729, 28), 617}, // 765 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18704, 25), 616}, // 766 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18652, 25), 614}, // 767 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18632, 20), 613}, // 768 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18677, 27), 615}, // 769 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18880, 16), 625}, // 770 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18896, 15), 625}, // 771 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18921, 10), 627}, // 772 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18931, 13), 628}, // 773 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18944, 12), 628}, // 774 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18911, 10), 626}, // 775 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20485, 21), 680}, // 776 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20506, 20), 680}, // 777 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20212, 15), 672}, // 778 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20227, 14), 672}, // 779 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19208, 26), 641}, // 780 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19546, 15), 654}, // 781 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19561, 14), 654}, // 782 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21004, 32), 692}, // 783 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21036, 31), 692}, // 784 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17646, 17), 556}, // 785 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(17846, 9), 574}, // 786 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(18981, 16), 630}, // 787 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(18997, 19), 630}, // 788 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(17688, 9), 560}, // 789 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(19194, 14), 640}, // 790 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17829, 7), 572}, // 791 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(17958, 20), 583}, // 792 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(18405, 44), 603}, // 793 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18852, 28), 624}, // 794 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19247, 17), 643}, // 795 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19810, 21), 662}, // 796 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19831, 20), 662}, // 797 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20671, 25), 684}, // 798 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19705, 17), 659}, // 799 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19722, 16), 659}, // 800 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17663, 7), 557}, // 801 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(21479, 27), 699}, // 802 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21506, 26), 699}, // 803 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19339, 15), 647}, // 804 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20241, 18), 673}, // 805 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20259, 17), 673}, // 806 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20749, 16), 687}, // 807 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20765, 15), 687}, // 808 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17978, 7), 584}, // 809 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(19986, 17), 666}, // 810 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20003, 16), 666}, // 811 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18609, 23), 612}, // 812 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(20048, 19), 668}, // 813 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20067, 18), 668}, // 814 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18786, 14), 620}, // 815 UTFEncodedKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(17914, 8), 581}, // 816 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(17948, 10), 582}, // 817 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(19418, 13), 650}, // 818 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(19450, 15), 651}, // 819 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(20696, 35), 685}, // 820 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19312, 27), 646}, // 821 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19264, 27), 644}, // 822 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18833, 19), 623}, // 823 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17863, 9), 576}, // 824 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(18477, 18), 606}, // 825 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(20019, 15), 667}, // 826 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20034, 14), 667}, // 827 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18039, 10), 591}, // 828 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18049, 10), 592}, // 829 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(21855, 3), 707}, // 830 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21858, 3), 708}, // 831 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21861, 3), 709}, // 832 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21874, 7), 712}, // 833 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21864, 5), 710}, // 834 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21869, 5), 711}, // 835 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21881, 12), 713}, // 836 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(21893, 17), 714}, // 837 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(23011, 20), 782}, // 838 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22705, 15), 767}, // 839 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22336, 15), 745}, // 840 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10912, 18), 759}, // 841 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10897, 15), 758}, // 842 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22106, 13), 728}, // 843 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22119, 10), 729}, // 844 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22091, 15), 727}, // 845 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22129, 15), 730}, // 846 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23146, 25), 786}, // 847 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23171, 24), 786}, // 848 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23099, 24), 785}, // 849 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23123, 23), 785}, // 850 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22653, 29), 765}, // 851 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22058, 19), 724}, // 852 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23656, 18), 806}, // 853 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22363, 10), 747}, // 854 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23471, 26), 798}, // 855 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23497, 27), 799}, // 856 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22351, 12), 746}, // 857 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22180, 11), 734}, // 858 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22191, 20), 735}, // 859 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22168, 12), 733}, // 860 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22221, 24), 737}, // 861 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21910, 12), 715}, // 862 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22720, 15), 768}, // 863 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22251, 9), 739}, // 864 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22144, 10), 731}, // 865 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22154, 14), 732}, // 866 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22431, 16), 752}, // 867 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22419, 12), 751}, // 868 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22735, 21), 769}, // 869 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22780, 21), 772}, // 870 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23546, 16), 801}, // 871 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23524, 22), 800}, // 872 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23632, 24), 805}, // 873 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23768, 22), 811}, // 874 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23790, 24), 812}, // 875 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23745, 23), 810}, // 876 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23865, 27), 813}, // 877 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23562, 20), 802}, // 878 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22447, 34), 753}, // 879 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22481, 34), 754}, // 880 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22515, 36), 755}, // 881 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22599, 34), 763}, // 882 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23582, 22), 803}, // 883 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23674, 21), 807}, // 884 OpacityMicromapIdKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22042, 16), 723}, // 885 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22026, 16), 722}, // 886 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22288, 16), 742}, // 887 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23031, 15), 783}, // 888 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23046, 14), 783}, // 889 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22275, 13), 741}, // 890 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23060, 20), 784}, // 891 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23080, 19), 784}, // 892 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22304, 20), 743}, // 893 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23195, 19), 787}, // 894 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23214, 18), 787}, // 895 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22260, 15), 740}, // 896 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22007, 19), 721}, // 897 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23232, 25), 788}, // 898 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23257, 27), 789}, // 899 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22077, 10), 725}, // 900 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22581, 18), 757}, // 901 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22952, 19), 779}, // 902 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22245, 6), 738}, // 903 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23719, 26), 809}, // 904 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22971, 20), 780}, // 905 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10955, 16), 761}, // 906 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23450, 21), 797}, // 907 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23429, 21), 796}, // 908 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10971, 16), 762}, // 909 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23284, 26), 790}, // 910 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23310, 28), 791}, // 911 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23604, 28), 804}, // 912 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(13362, 23), 794}, // 913 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7472, 16), 771}, // 914 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23338, 31), 792}, // 915 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23369, 33), 793}, // 916 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23402, 27), 795}, // 917 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22991, 20), 781}, // 918 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(10930, 25), 760}, // 919 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21922, 13), 716}, // 920 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21935, 22), 717}, // 921 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21957, 21), 718}, // 922 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22756, 24), 770}, // 923 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22905, 25), 777}, // 924 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22829, 26), 774}, // 925 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22930, 22), 778}, // 926 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22855, 23), 775}, // 927 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22682, 23), 766}, // 928 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22878, 27), 776}, // 929 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22801, 28), 773}, // 930 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23695, 24), 808}, // 931 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6579, 13), 748}, // 932 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22551, 30), 756}, // 933 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22373, 22), 749}, // 934 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22395, 24), 750}, // 935 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22633, 20), 764}, // 936 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22211, 10), 736}, // 937 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22324, 12), 744}, // 938 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21992, 15), 720}, // 939 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(21978, 14), 719}, // 940 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22087, 4), 726}, // 941 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24030, 10), 825}, // 942 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24040, 9), 825}, // 943 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24091, 12), 828}, // 944 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24103, 11), 828}, // 945 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24049, 14), 826}, // 946 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24063, 13), 826}, // 947 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(17029, 9), 818}, // 948 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23942, 10), 819}, // 949 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(6060, 9), 817}, // 950 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23999, 16), 824}, // 951 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24015, 15), 824}, // 952 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5827, 7), 820}, // 953 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24122, 8), 830}, // 954 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23959, 7), 822}, // 955 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24076, 8), 827}, // 956 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24084, 7), 827}, // 957 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23966, 17), 823}, // 958 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23983, 16), 823}, // 959 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24114, 8), 829}, // 960 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23952, 7), 821}, // 961 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23899, 20), 815}, // 962 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23919, 23), 816}, // 963 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(23892, 7), 814}, // 964 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24139, 12), 832}, // 965 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24130, 9), 831}, // 966 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24151, 12), 833}, // 967 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(24219, 14), 838}, // 968 Float4E2M1EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24191, 14), 836}, // 969 Float6E2M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24205, 14), 837}, // 970 Float6E3M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24163, 14), 834}, // 971 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24177, 14), 835}, // 972 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10392, 22), 839}, // 973 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10414, 10), 840}, // 974 MXInt8EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24238, 4), 842}, // 975 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24233, 5), 841}, // 976 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24276, 14), 849}, // 977 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24290, 23), 849}, // 978 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24313, 13), 850}, // 979 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24326, 18), 850}, // 980 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24260, 11), 847}, // 981 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24344, 15), 851}, // 982 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24271, 5), 848}, // 983 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24256, 4), 846}, // 984 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(16625, 5), 843}, // 985 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24249, 7), 845}, // 986 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24242, 7), 844}, // 987 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24359, 4), 852}, // 988 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24371, 4), 855}, // 989 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24367, 4), 854}, // 990 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24363, 4), 853}, // 991 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24407, 18), 858}, // 992 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24425, 18), 859}, // 993 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24375, 16), 856}, // 994 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24391, 16), 857}, // 995 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24466, 6), 864}, // 996 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24450, 11), 862}, // 997 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24443, 7), 861}, // 998 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(16625, 5), 860}, // 999 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15433, 11), 865}, // 1000 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15444, 13), 865}, // 1001 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24461, 5), 863}, // 1002 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24482, 6), 868}, // 1003 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24493, 8), 870}, // 1004 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24501, 10), 871}, // 1005 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24519, 12), 873}, // 1006 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24511, 8), 872}, // 1007 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24531, 21), 874}, // 1008 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24552, 20), 874}, // 1009 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24477, 5), 867}, // 1010 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24488, 5), 869}, // 1011 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24472, 5), 866}, // 1012 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24572, 14), 875}, // 1013 Gather4x1QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24586, 12), 876}, // 1014 GatherDQCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24598, 13), 877}, // 1015 GatherH2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24611, 13), 878}, // 1016 GatherV2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24659, 16), 882}, // 1017 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24645, 14), 881}, // 1018 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24631, 14), 880}, // 1019 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24771, 28), 885}, // 1020 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24799, 27), 885}, // 1021 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24716, 28), 884}, // 1022 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24744, 27), 884}, // 1023 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24675, 21), 883}, // 1024 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24696, 20), 883}, // 1025 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24624, 7), 879}, // 1026 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(24826, 10), 886}, // 1027 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24836, 10), 887}, // 1028 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24857, 15), 889}, // 1029 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24846, 11), 888}, // 1030 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(24872, 5), 891}, // 1031 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(24886, 12), 894}, // 1032 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(24898, 13), 896}, // 1033 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(24881, 5), 893}, // 1034 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(24877, 4), 892}, // 1035 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(24911, 19), 899}, // 1036 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(24930, 22), 899}, // 1037 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(24952, 17), 900}, // 1038 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(24969, 20), 900}, // 1039 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9660, 7), 898}, // 1040 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(24989, 16), 901}, // 1041 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25005, 19), 901}, // 1042 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(16625, 5), 890}, // 1043 None in SPV_OPERAND_TYPE_IMAGE
  {IR(25077, 12), 905}, // 1044 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(18032, 7), 895}, // 1045 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(25089, 8), 906}, // 1046 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(17829, 7), 897}, // 1047 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(25055, 11), 903}, // 1048 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(25024, 14), 902}, // 1049 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25038, 17), 902}, // 1050 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25066, 11), 904}, // 1051 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(16522, 6), 921}, // 1052 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25258, 10), 920}, // 1053 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25193, 12), 915}, // 1054 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25205, 12), 916}, // 1055 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25182, 11), 914}, // 1056 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25107, 11), 908}, // 1057 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25097, 10), 907}, // 1058 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25167, 15), 913}, // 1059 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25279, 17), 923}, // 1060 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25296, 16), 924}, // 1061 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25429, 16), 931}, // 1062 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25445, 16), 932}, // 1063 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25128, 11), 910}, // 1064 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25268, 11), 922}, // 1065 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25352, 20), 927}, // 1066 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25118, 10), 909}, // 1067 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25153, 14), 912}, // 1068 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25139, 14), 911}, // 1069 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25372, 19), 928}, // 1070 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25391, 19), 929}, // 1071 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25410, 19), 930}, // 1072 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25230, 14), 918}, // 1073 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25244, 14), 919}, // 1074 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25217, 13), 917}, // 1075 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25312, 20), 925}, // 1076 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25332, 20), 926}, // 1077 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25463, 2), 934}, // 1078 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25564, 5), 952}, // 1079 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25485, 5), 940}, // 1080 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25480, 5), 939}, // 1081 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25522, 6), 946}, // 1082 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25528, 13), 947}, // 1083 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25490, 10), 941}, // 1084 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25500, 10), 942}, // 1085 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25461, 2), 933}, // 1086 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25468, 3), 936}, // 1087 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25465, 3), 935}, // 1088 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25471, 4), 937}, // 1089 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25475, 5), 938}, // 1090 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25517, 5), 945}, // 1091 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25513, 4), 944}, // 1092 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25510, 3), 943}, // 1093 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25558, 6), 951}, // 1094 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25541, 5), 948}, // 1095 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25552, 6), 950}, // 1096 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25546, 6), 949}, // 1097 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25569, 28), 953}, // 1098 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25597, 27), 953}, // 1099 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25624, 24), 954}, // 1100 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25648, 23), 954}, // 1101 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25671, 7), 955}, // 1102 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25678, 11), 956}, // 1103 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25689, 14), 957}, // 1104 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(25703, 12), 959}, // 1105 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(16625, 5), 958}, // 1106 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(25715, 13), 961}, // 1107 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(16625, 5), 960}, // 1108 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(25728, 7), 962}, // 1109 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25735, 7), 963}, // 1110 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25742, 12), 964}, // 1111 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25754, 8), 965}, // 1112 WeakAMD in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(25776, 12), 967}, // 1113 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25828, 17), 970}, // 1114 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25803, 25), 969}, // 1115 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25788, 15), 968}, // 1116 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25762, 14), 966}, // 1117 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(25968, 22), 983}, // 1118 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25990, 21), 983}, // 1119 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25863, 19), 974}, // 1120 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25882, 17), 975}, // 1121 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25852, 11), 973}, // 1122 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20395, 25), 981}, // 1123 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20420, 24), 981}, // 1124 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25927, 18), 978}, // 1125 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26011, 19), 985}, // 1126 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26030, 18), 985}, // 1127 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26173, 16), 989}, // 1128 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26189, 15), 989}, // 1129 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20444, 21), 982}, // 1130 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20465, 20), 982}, // 1131 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26048, 22), 986}, // 1132 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26070, 21), 986}, // 1133 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25913, 14), 977}, // 1134 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26204, 27), 990}, // 1135 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26231, 26), 990}, // 1136 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25899, 14), 976}, // 1137 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(11197, 23), 991}, // 1138 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26144, 15), 988}, // 1139 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26159, 14), 988}, // 1140 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(16625, 5), 971}, // 1141 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25955, 13), 980}, // 1142 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25945, 10), 979}, // 1143 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20485, 21), 984}, // 1144 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20506, 20), 984}, // 1145 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26091, 27), 987}, // 1146 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26118, 26), 987}, // 1147 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(25845, 7), 972}, // 1148 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26540, 27), 1005}, // 1149 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26488, 26), 1003}, // 1150 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26408, 23), 999}, // 1151 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26362, 23), 997}, // 1152 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26257, 29), 993}, // 1153 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26454, 17), 1001}, // 1154 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26567, 27), 1006}, // 1155 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26514, 26), 1004}, // 1156 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26431, 23), 1000}, // 1157 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26385, 23), 998}, // 1158 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26286, 29), 994}, // 1159 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26471, 17), 1002}, // 1160 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26315, 21), 995}, // 1161 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26336, 26), 996}, // 1162 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(16625, 5), 992}, // 1163 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26727, 20), 1014}, // 1164 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26594, 8), 1009}, // 1165 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26602, 21), 1011}, // 1166 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26623, 24), 1011}, // 1167 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26647, 19), 1012}, // 1168 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26666, 22), 1012}, // 1169 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26747, 17), 1015}, // 1170 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26688, 18), 1013}, // 1171 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26706, 21), 1013}, // 1172 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(16625, 5), 1007}, // 1173 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25077, 12), 1010}, // 1174 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(17863, 9), 1008}, // 1175 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26771, 8), 1017}, // 1176 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26779, 7), 1018}, // 1177 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26764, 7), 1016}, // 1178 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26786, 7), 1019}, // 1179 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26793, 10), 1019}, // 1180 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(26811, 8), 1021}, // 1181 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26827, 15), 1023}, // 1182 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26931, 20), 1029}, // 1183 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26910, 21), 1028}, // 1184 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26951, 12), 1030}, // 1185 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26992, 14), 1032}, // 1186 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27006, 17), 1032}, // 1187 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27023, 12), 1033}, // 1188 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27035, 15), 1033}, // 1189 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(16625, 5), 1020}, // 1190 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26963, 13), 1031}, // 1191 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26976, 16), 1031}, // 1192 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26803, 8), 1020}, // 1193 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26819, 8), 1022}, // 1194 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26842, 23), 1024}, // 1195 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26879, 15), 1026}, // 1196 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26865, 14), 1025}, // 1197 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(17863, 9), 1034}, // 1198 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(26894, 16), 1027}, // 1199 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27050, 10), 1035}, // 1200 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(27065, 4), 1037}, // 1201 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27078, 8), 1039}, // 1202 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27069, 9), 1038}, // 1203 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27060, 5), 1036}, // 1204 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27086, 25), 1040}, // 1205 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27111, 28), 1040}, // 1206 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27152, 4), 1043}, // 1207 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27185, 9), 1047}, // 1208 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27194, 13), 1048}, // 1209 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27165, 8), 1045}, // 1210 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27173, 12), 1046}, // 1211 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27156, 9), 1044}, // 1212 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27139, 4), 1041}, // 1213 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27143, 9), 1042}, // 1214 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(16625, 5), 1049}, // 1215 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27207, 25), 1050}, // 1216 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27232, 23), 1051}, // 1217 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27324, 27), 1057}, // 1218 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27351, 28), 1058}, // 1219 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27393, 16), 1060}, // 1220 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27379, 14), 1059}, // 1221 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27493, 30), 1063}, // 1222 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27463, 30), 1063}, // 1223 ForceOpacityMicromap2StateKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27265, 12), 1054}, // 1224 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(17377, 8), 1052}, // 1225 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27255, 10), 1053}, // 1226 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27450, 13), 1062}, // 1227 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27426, 24), 1061}, // 1228 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27300, 24), 1056}, // 1229 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27409, 17), 1061}, // 1230 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27277, 23), 1055}, // 1231 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27564, 37), 1065}, // 1232 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27523, 41), 1064}, // 1233 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27679, 42), 1068}, // 1234 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27601, 37), 1066}, // 1235 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27638, 41), 1067}, // 1236 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(27721, 33), 1069}, // 1237 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(27754, 33), 1070}, // 1238 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(27799, 6), 1073}, // 1239 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27787, 12), 1072}, // 1240 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(16625, 5), 1071}, // 1241 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27805, 7), 1074}, // 1242 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27812, 15), 1075}, // 1243 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(27835, 7), 1077}, // 1244 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(27827, 8), 1076}, // 1245 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(27900, 13), 1086}, // 1246 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27942, 4), 1092}, // 1247 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27980, 9), 1097}, // 1248 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27913, 5), 1087}, // 1249 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28042, 5), 1106}, // 1250 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28113, 6), 1116}, // 1251 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27866, 5), 1081}, // 1252 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28020, 5), 1102}, // 1253 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28077, 6), 1111}, // 1254 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28130, 5), 1119}, // 1255 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28124, 6), 1118}, // 1256 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27946, 3), 1093}, // 1257 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27989, 8), 1098}, // 1258 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28047, 4), 1107}, // 1259 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28119, 5), 1117}, // 1260 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27933, 5), 1090}, // 1261 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27961, 10), 1095}, // 1262 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27894, 6), 1085}, // 1263 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28031, 6), 1104}, // 1264 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28100, 7), 1114}, // 1265 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27888, 6), 1084}, // 1266 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28025, 6), 1103}, // 1267 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28093, 7), 1113}, // 1268 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27938, 4), 1091}, // 1269 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27971, 9), 1096}, // 1270 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28037, 5), 1105}, // 1271 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28107, 6), 1115}, // 1272 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27925, 8), 1089}, // 1273 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28083, 10), 1112}, // 1274 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27918, 7), 1088}, // 1275 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27949, 12), 1094}, // 1276 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27858, 8), 1080}, // 1277 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28005, 8), 1100}, // 1278 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28060, 9), 1109}, // 1279 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27850, 8), 1079}, // 1280 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27997, 8), 1099}, // 1281 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28051, 9), 1108}, // 1282 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27871, 6), 1082}, // 1283 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27877, 11), 1083}, // 1284 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28013, 7), 1101}, // 1285 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28069, 8), 1110}, // 1286 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(27842, 8), 1078}, // 1287 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28135, 12), 1120}, // 1288 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28147, 7), 1121}, // 1289 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28173, 11), 1124}, // 1290 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28184, 12), 1125}, // 1291 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28196, 15), 1125}, // 1292 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28211, 14), 1126}, // 1293 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28164, 9), 1123}, // 1294 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28154, 10), 1122}, // 1295 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28233, 12), 1129}, // 1296 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28225, 8), 1128}, // 1297 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(16625, 5), 1127}, // 1298 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28245, 29), 1130}, // 1299 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(16506, 8), 1132}, // 1300 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16514, 8), 1133}, // 1301 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16522, 6), 1134}, // 1302 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16528, 7), 1135}, // 1303 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16535, 11), 1136}, // 1304 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16546, 9), 1137}, // 1305 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16555, 13), 1138}, // 1306 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16494, 12), 1131}, // 1307 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16568, 6), 1139}, // 1308 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16574, 10), 1140}, // 1309 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16584, 6), 1141}, // 1310 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16605, 20), 1143}, // 1311 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16590, 15), 1142}, // 1312 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16714, 15), 1151}, // 1313 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16729, 13), 1152}, // 1314 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16702, 12), 1150}, // 1315 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16792, 21), 1156}, // 1316 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16685, 17), 1149}, // 1317 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16869, 16), 1160}, // 1318 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16673, 12), 1148}, // 1319 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16853, 16), 1159}, // 1320 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16646, 14), 1146}, // 1321 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16630, 16), 1145}, // 1322 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16660, 13), 1147}, // 1323 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16813, 20), 1157}, // 1324 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16757, 18), 1154}, // 1325 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16742, 15), 1153}, // 1326 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16833, 20), 1158}, // 1327 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16775, 17), 1155}, // 1328 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16905, 24), 1162}, // 1329 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16885, 20), 1161}, // 1330 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(28274, 26), 1163}, // 1331 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16625, 5), 1144}, // 1332 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16990, 9), 1168}, // 1333 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17022, 7), 1172}, // 1334 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16929, 6), 1164}, // 1335 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17029, 9), 1173}, // 1336 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16940, 6), 1166}, // 1337 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16935, 5), 1165}, // 1338 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16979, 11), 1167}, // 1339 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17011, 11), 1171}, // 1340 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(16999, 5), 1169}, // 1341 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17004, 7), 1170}, // 1342 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17074, 11), 1177}, // 1343 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17038, 10), 1174}, // 1344 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17061, 13), 1176}, // 1345 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17048, 13), 1175}, // 1346 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(28392, 9), 1193}, // 1347 ApilaJai in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28335, 15), 1184}, // 1348 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28300, 5), 1179}, // 1349 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28305, 5), 1180}, // 1350 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28355, 7), 1186}, // 1351 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28330, 5), 1183}, // 1352 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28362, 5), 1187}, // 1353 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28310, 9), 1181}, // 1354 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28319, 11), 1182}, // 1355 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28387, 5), 1192}, // 1356 Pred in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28382, 5), 1191}, // 1357 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28350, 5), 1185}, // 1358 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28372, 6), 1189}, // 1359 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(27842, 8), 1178}, // 1360 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28367, 5), 1188}, // 1361 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28378, 4), 1190}, // 1362 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28483, 14), 1204}, // 1363 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28565, 16), 1210}, // 1364 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28581, 15), 1210}, // 1365 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28901, 17), 1220}, // 1366 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28430, 15), 1199}, // 1367 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28918, 17), 1221}, // 1368 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28935, 16), 1221}, // 1369 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28453, 9), 1201}, // 1370 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28462, 8), 1202}, // 1371 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28670, 16), 1213}, // 1372 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28686, 15), 1213}, // 1373 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28879, 22), 1219}, // 1374 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28834, 21), 1217}, // 1375 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28951, 15), 1222}, // 1376 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28966, 14), 1222}, // 1377 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28497, 6), 1205}, // 1378 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28596, 24), 1211}, // 1379 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28620, 23), 1211}, // 1380 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28701, 22), 1214}, // 1381 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28723, 21), 1214}, // 1382 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28417, 6), 1195}, // 1383 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28549, 16), 1209}, // 1384 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28423, 7), 1197}, // 1385 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28787, 22), 1216}, // 1386 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28809, 25), 1216}, // 1387 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28445, 8), 1200}, // 1388 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28470, 13), 1203}, // 1389 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28643, 14), 1212}, // 1390 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28657, 13), 1212}, // 1391 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28744, 22), 1215}, // 1392 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28766, 21), 1215}, // 1393 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28503, 14), 1206}, // 1394 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28855, 24), 1218}, // 1395 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28530, 19), 1208}, // 1396 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28517, 13), 1207}, // 1397 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(17914, 8), 1196}, // 1398 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28401, 16), 1194}, // 1399 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28154, 10), 1198}, // 1400 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(25788, 15), 1226}, // 1401 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25762, 14), 1223}, // 1402 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28998, 15), 1225}, // 1403 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(28980, 18), 1224}, // 1404 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29024, 11), 1229}, // 1405 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29035, 17), 1230}, // 1406 DecodeVectorFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(16625, 5), 1227}, // 1407 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29013, 11), 1228}, // 1408 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(27787, 12), 1233}, // 1409 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(17872, 9), 1232}, // 1410 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27805, 7), 1234}, // 1411 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(27812, 15), 1235}, // 1412 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29052, 10), 1231}, // 1413 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29105, 24), 1239}, // 1414 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29129, 22), 1240}, // 1415 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29151, 21), 1241}, // 1416 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29062, 8), 1236}, // 1417 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29070, 15), 1237}, // 1418 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29085, 20), 1238}, // 1419 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 150);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(158, 353);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(511, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(519, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(522, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(524, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(543, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(553, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(557, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(572, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(576, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(582, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(585, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(588, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(592, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(600, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(603, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(619, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(628, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(631, 199);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(830, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(838, 104);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(942, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(965, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(967, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(975, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(977, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(988, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(992, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(996, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(1003, 10);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(1013, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(1017, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(1027, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1031, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1052, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1078, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1098, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1102, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1105, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1107, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1109, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1113, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1118, 31);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1149, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1164, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1176, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1181, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1200, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1201, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1205, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1207, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1215, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1218, 14);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1232, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1234, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1237, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1239, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1244, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1246, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1288, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1296, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1299, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1300, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1308, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1311, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1313, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1333, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1343, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1347, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1363, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1401, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1405, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1409, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1414, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1031, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1164, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1205, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(576, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1149, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1215, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(967, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1414, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(158, 353);
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
static const std::array<InstructionDesc, 875> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(29172, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(29228, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(29234, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(29353, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(29360, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(29376, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(29381, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(29392, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(29399, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(29404, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(29414, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(29474, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(29546, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(29620, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(29663, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(29705, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(29716, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(29725, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(29734, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(29779, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(29789, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(29800, 11), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(29924, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(29934, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(29946, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(29963, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(29973, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(29990, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(30001, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(30043, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(30055, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(30068, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(30078, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(30094, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(30108, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(30118, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(30127, 19), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(30146, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(30159, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(17872, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(30211, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(30307, 16), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(30323, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(30336, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(30353, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(30371, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(30384, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(30447, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(28453, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(30496, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(30514, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(30526, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(30539, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(30548, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(30606, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(30611, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(30617, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(30628, 16), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(30644, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(30656, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(30676, 15), IR(0, 0), IR(225, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(30691, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(30703, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(30726, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(30777, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(30786, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(30801, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(30817, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(30876, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(30896, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(30917, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(30937, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(30951, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(30970, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(30987, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(31003, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(31014, 10), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(31024, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(31069, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(31115, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(31138, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(31165, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(31192, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(31219, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(31246, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(31277, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(31308, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(31319, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(31331, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(31347, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(31357, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(28497, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(31368, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(31385, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(31401, 18), IR(0, 0), IR(229, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(31419, 15), IR(0, 0), IR(229, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(31434, 14), IR(0, 0), IR(231, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(31448, 17), IR(0, 0), IR(229, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(31465, 18), IR(0, 0), IR(229, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(31483, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(31495, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(31507, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(31519, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(31531, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(31540, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(31549, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(31558, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(31572, 14), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(31586, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(31601, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(31616, 14), IR(0, 0), IR(220, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(31630, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(31647, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(31664, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(31689, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(31697, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(31705, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(31713, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(31718, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(31723, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(31728, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(31733, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(31738, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(31743, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(31748, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(31753, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(31758, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(31763, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(31768, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(31773, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(31778, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(31783, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(31801, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(31819, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(31837, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(31855, 18), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(31873, 13), IR(0, 0), IR(67, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(31886, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(31890, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(31900, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(31911, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(31924, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(31937, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(31941, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(31945, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(31951, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(31957, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(31966, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(31975, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(31986, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(32000, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(32008, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(32018, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(32031, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(32047, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(32057, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(32068, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(32079, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(32086, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(32093, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(32103, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(32116, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(32129, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(32147, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(32165, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(32175, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(32185, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(32200, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(32215, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(32225, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(32237, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(32250, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(32265, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(32278, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(32293, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(32309, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(32327, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(32345, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(32365, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(32386, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(32409, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(32427, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(32448, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(32465, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(32475, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(32486, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(32497, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(32501, 15), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(32516, 17), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(32533, 17), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(32550, 11), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(32561, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(32570, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(32575, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(32580, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(32587, 9), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(32596, 9), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(32605, 11), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(32616, 11), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(32627, 11), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(32638, 13), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(32651, 11), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(32662, 13), IR(0, 0), IR(70, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(32675, 17), IR(0, 0), IR(123, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(32692, 19), IR(0, 0), IR(123, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(32748, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(32763, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(32777, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(32788, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(32800, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(32815, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(32837, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(32863, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(32880, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(32897, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(32908, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(32919, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(32930, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(32941, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(32952, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(32963, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(32973, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(32982, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(32992, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(33026, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(33071, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(33086, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(33092, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(33099, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(33162, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(33169, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(33174, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(33181, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(33193, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(33205, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(33219, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(33232, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(33247, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(33263, 9), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(33272, 9), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(33281, 15), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(33329, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(33339, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(33349, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(33359, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(33369, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(33379, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(33389, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(33399, 10), IR(0, 0), IR(235, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(33409, 9), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(33418, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(33428, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(33445, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(33463, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(33486, 24), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(33510, 15), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(33525, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(33541, 17), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(33558, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(33576, 18), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(33594, 28), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(33622, 29), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(33651, 20), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(33671, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(33692, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(33706, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(33720, 30), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(33750, 32), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(33782, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(33805, 40), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(33845, 12), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(33857, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(33870, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(33886, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(33899, 19), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(33918, 26), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(33944, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(33960, 13), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(33973, 29), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(34002, 29), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(34031, 33), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(34064, 33), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(34097, 33), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(34130, 33), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(34163, 37), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(34200, 37), IR(0, 0), IR(236, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(34237, 17), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(34254, 18), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(34272, 22), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(34294, 26), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(34320, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(34327, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(34348, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(34364, 16), IR(0, 0), IR(236, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(34380, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(34387, 16), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(34403, 20), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(34423, 26), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(34449, 35), IR(0, 0), IR(164, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(34484, 25), IR(0, 0), IR(164, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(34509, 17), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(34526, 23), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(34549, 19), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(34568, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(34584, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(34600, 11), IR(0, 0), IR(1, 0), IR(199, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(34611, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(34632, 19), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(34651, 19), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(34670, 24), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(34694, 25), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(34719, 30), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6847, 22), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(34749, 29), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(34778, 32), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(34810, 30), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(34840, 29), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(34869, 29), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(9986, 23), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(34898, 26), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(34924, 25), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(34949, 27), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(34976, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(34996, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(35016, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(35036, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(35056, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(35076, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(35096, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(35116, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(35136, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(35156, 20), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(35176, 26), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(35202, 25), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(35227, 26), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(35253, 26), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(35279, 25), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(35304, 26), IR(0, 0), IR(243, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(35330, 29), IR(0, 0), IR(246, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(35359, 24), IR(0, 0), IR(246, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(35383, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(35395, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(35404, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(35416, 8), IR(0, 0), IR(247, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(35424, 23), IR(0, 0), IR(156, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(35447, 23), IR(0, 0), IR(165, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(35470, 25), IR(0, 0), IR(166, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(35495, 14), IR(0, 0), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(35551, 14), IR(0, 0), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(35565, 15), IR(0, 0), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(35580, 19), IR(0, 0), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(35599, 17), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(35616, 19), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(10277, 9), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(35635, 14), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(35649, 18), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(35667, 12), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(35679, 13), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBitcastExtractEXT, true, true, IR(172, 4), IR(10424, 18), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(35692, 20), IR(0, 0), IR(3, 1), IR(209, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(35712, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(35734, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(35753, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(35775, 30), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6608, 18), IR(0, 0), IR(252, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(35805, 27), IR(0, 0), IR(252, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(35832, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(35857, 33), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(35890, 22), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(35912, 19), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(35931, 7), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(35938, 15), IR(0, 0), IR(254, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(35953, 15), IR(0, 0), IR(254, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(35968, 20), IR(0, 0), IR(254, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(15284, 25), IR(0, 0), IR(255, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(35988, 26), IR(0, 0), IR(252, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(36014, 26), IR(0, 0), IR(1, 0), IR(210, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(36040, 25), IR(0, 0), IR(93, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(36065, 12), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(36077, 19), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(36096, 35), IR(0, 0), IR(256, 2), IR(211, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(36131, 22), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(36153, 16), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(36216, 5), IR(174, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(36229, 5), IR(175, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(36242, 6), IR(176, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(36257, 11), IR(177, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(36282, 11), IR(178, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(36307, 12), IR(179, 1), IR(258, 1), IR(153, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(36334, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(36359, 25), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(36384, 26), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(36464, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(36491, 27), IR(0, 0), IR(87, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36518, 30), IR(0, 0), IR(259, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36548, 34), IR(0, 0), IR(259, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(36582, 31), IR(0, 0), IR(259, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(36613, 16), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(36629, 22), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(36651, 21), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(36672, 32), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(36704, 31), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(36735, 19), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(36754, 31), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(36785, 24), IR(0, 0), IR(260, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(36809, 19), IR(0, 0), IR(261, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(36828, 23), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(36851, 23), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(36874, 17), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(36891, 29), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(36920, 29), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(36949, 29), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(36978, 29), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(37007, 30), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(37037, 28), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(37065, 20), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpImageGatherQCOM, true, true, IR(479, 7), IR(37085, 16), IR(0, 0), IR(265, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(37101, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(37124, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(37147, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(37170, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(37193, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(37216, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(37239, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(37262, 23), IR(0, 0), IR(235, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(37285, 21), IR(0, 0), IR(267, 1), IR(61, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(37306, 17), IR(0, 0), IR(267, 1), IR(61, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(37323, 13), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(37336, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(37361, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(37385, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(37410, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(37439, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(37466, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(37489, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(37508, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(37531, 26), IR(0, 0), IR(173, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(37557, 26), IR(0, 0), IR(173, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(37583, 14), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(37597, 17), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAbortKHR, false, false, IR(36, 2), IR(11498, 9), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(37614, 28), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(37642, 20), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(37662, 18), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantDataKHR, true, true, IR(486, 3), IR(11507, 16), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantDataKHR, true, true, IR(486, 3), IR(37680, 20), IR(0, 0), IR(127, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(37700, 10), IR(0, 0), IR(174, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(37710, 10), IR(0, 0), IR(174, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(489, 14), IR(37720, 27), IR(0, 0), IR(270, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(503, 13), IR(37747, 36), IR(0, 0), IR(270, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(516, 7), IR(37783, 28), IR(0, 0), IR(270, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(37811, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(37839, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(37867, 33), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(37900, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(503, 13), IR(37930, 26), IR(0, 0), IR(270, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(37956, 39), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(37995, 44), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(38039, 23), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(523, 12), IR(38062, 20), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(503, 13), IR(38082, 21), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(523, 12), IR(38103, 30), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(535, 6), IR(38133, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(38155, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(38180, 26), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(38206, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(38231, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(38253, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(38282, 28), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(38310, 25), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(38335, 34), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(38369, 32), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(38401, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(38430, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(38452, 22), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(38474, 19), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(38493, 17), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(38510, 18), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(541, 3), IR(38528, 29), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(38557, 24), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(38581, 16), IR(0, 0), IR(133, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(479, 7), IR(38597, 23), IR(0, 0), IR(272, 1), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(38620, 16), IR(180, 1), IR(273, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(544, 13), IR(38660, 29), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(557, 7), IR(38689, 42), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(38731, 39), IR(0, 0), IR(276, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(564, 16), IR(38770, 32), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(38802, 27), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(580, 4), IR(38829, 17), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(38846, 18), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(38864, 28), IR(181, 1), IR(199, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(38919, 33), IR(0, 0), IR(51, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(38952, 35), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(38987, 38), IR(0, 0), IR(278, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(584, 5), IR(39025, 24), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(589, 4), IR(39049, 25), IR(0, 0), IR(275, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(593, 5), IR(39074, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(516, 7), IR(39102, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(39125, 29), IR(0, 0), IR(279, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(39154, 51), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(39205, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(39229, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(598, 4), IR(39274, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(602, 14), IR(39307, 32), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(616, 15), IR(39339, 38), IR(0, 0), IR(279, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(39377, 17), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(39394, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(541, 3), IR(39419, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(523, 12), IR(39449, 21), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(503, 13), IR(39470, 27), IR(0, 0), IR(279, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(39497, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(39521, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(39547, 27), IR(0, 0), IR(279, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(39574, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(39600, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(39623, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(39653, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(39682, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(39708, 35), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(39743, 31), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(39774, 34), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(39808, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(39841, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(39871, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(39900, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(39929, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(39952, 22), IR(182, 1), IR(55, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(39995, 21), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(40016, 15), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(40031, 8), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(523, 12), IR(40039, 14), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(523, 12), IR(40053, 17), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(40070, 50), IR(0, 0), IR(281, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(40120, 29), IR(183, 1), IR(282, 4), IR(213, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(40177, 18), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(40195, 35), IR(184, 1), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(40253, 24), IR(0, 0), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(40277, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(40300, 45), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(40345, 40), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(40385, 20), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(40405, 18), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(40423, 19), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(631, 5), IR(40442, 24), IR(0, 0), IR(286, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(636, 6), IR(40466, 24), IR(0, 0), IR(286, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(642, 5), IR(40490, 25), IR(0, 0), IR(286, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(40515, 26), IR(0, 0), IR(286, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(40541, 26), IR(0, 0), IR(286, 1), IR(88, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(40567, 28), IR(0, 0), IR(287, 3), IR(89, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(40595, 26), IR(0, 0), IR(287, 3), IR(89, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(647, 5), IR(40664, 26), IR(0, 0), IR(290, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(652, 7), IR(40765, 30), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(659, 5), IR(40795, 31), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(40826, 32), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(40858, 19), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(664, 4), IR(40877, 17), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(40894, 21), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(40915, 27), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(40942, 24), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(40966, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(40986, 28), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(41014, 19), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(41033, 25), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(41058, 22), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(13038, 25), IR(46, 1), IR(293, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(41080, 22), IR(0, 0), IR(293, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(41102, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(41122, 27), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(41149, 29), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(41178, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(41196, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(41216, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(41234, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(41254, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(41279, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(41304, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(668, 7), IR(41385, 17), IR(0, 0), IR(206, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(41402, 40), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(41442, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(41480, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(41518, 34), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(41552, 37), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(41589, 29), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(41618, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(41645, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(41672, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(41695, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(41718, 20), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(41738, 22), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(41760, 19), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13631, 21), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(41779, 25), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(41804, 23), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(41827, 24), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(41851, 23), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(41874, 24), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(41898, 28), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(41926, 29), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(41955, 33), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(675, 5), IR(41988, 34), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(42022, 24), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(42046, 25), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(42071, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(42084, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(42097, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(42110, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(42123, 14), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(42137, 14), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(42151, 21), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(42172, 21), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(42193, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(42206, 13), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(42219, 15), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(42234, 15), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(42249, 29), IR(0, 0), IR(215, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(42278, 25), IR(0, 0), IR(215, 1), IR(110, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(42303, 15), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(680, 6), IR(13849, 9), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(42318, 13), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(42331, 14), IR(0, 0), IR(299, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(42345, 14), IR(0, 0), IR(299, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(42359, 14), IR(0, 0), IR(303, 1), IR(114, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(42373, 10), IR(0, 0), IR(303, 1), IR(114, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(42383, 15), IR(185, 1), IR(1, 0), IR(217, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(42419, 21), IR(186, 1), IR(1, 0), IR(217, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(42467, 14), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(42481, 18), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(42499, 23), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(42522, 23), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(42545, 23), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(42568, 23), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(42591, 22), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(42613, 22), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42635, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(42681, 44), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(42725, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(42764, 37), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(42801, 22), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(42823, 22), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(42845, 60), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(42905, 53), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(42958, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43005, 40), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43045, 51), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43096, 44), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(43140, 51), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(43191, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43247, 50), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43297, 52), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43349, 49), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(43398, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(43445, 50), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(43495, 51), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(43546, 56), IR(0, 0), IR(307, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(43602, 33), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43635, 52), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(43687, 61), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(43748, 61), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(43809, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(43848, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(43886, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(43925, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(43963, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(44002, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(44040, 36), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(44076, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(44115, 43), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(44158, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(44196, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(44234, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(44272, 45), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(44317, 40), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44357, 62), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(44419, 30), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(44449, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(44487, 36), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(44523, 33), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(44556, 35), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(44591, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(44630, 43), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(44673, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(44720, 54), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(44774, 34), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(44808, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(44855, 45), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(44900, 55), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(44955, 53), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(45008, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(45064, 54), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(45118, 58), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(45176, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(45232, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(45270, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(45316, 44), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45360, 49), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45409, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(45456, 70), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(45526, 68), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(45594, 69), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(45663, 68), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(45731, 66), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(45797, 67), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(45864, 36), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(45900, 48), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(45948, 59), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(46007, 58), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(46065, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(686, 9), IR(46121, 30), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(695, 10), IR(46151, 30), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46181, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(46220, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46266, 43), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46309, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46356, 45), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46401, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(46447, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(46503, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(46541, 30), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(46571, 32), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(695, 10), IR(46603, 36), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(705, 13), IR(46639, 42), IR(0, 0), IR(307, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(46681, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46720, 39), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(46759, 44), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(46803, 48), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(46851, 50), IR(0, 0), IR(307, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46901, 43), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(46944, 48), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(46992, 43), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(47035, 31), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(47066, 47), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(47113, 45), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(47158, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(47204, 56), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(47260, 38), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(47298, 35), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(47333, 44), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(47377, 46), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(47423, 41), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(47464, 36), IR(0, 0), IR(307, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(47500, 50), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(47550, 48), IR(0, 0), IR(305, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(47598, 35), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(14087, 25), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(47633, 16), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(47649, 19), IR(0, 0), IR(309, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(718, 8), IR(47668, 29), IR(187, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(718, 8), IR(47725, 25), IR(188, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(718, 8), IR(47774, 32), IR(189, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(718, 8), IR(47837, 30), IR(190, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(726, 10), IR(47896, 24), IR(191, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(726, 10), IR(47943, 24), IR(192, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(726, 10), IR(47990, 24), IR(193, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(726, 10), IR(48037, 24), IR(194, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(736, 6), IR(48084, 23), IR(195, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(736, 6), IR(48129, 23), IR(196, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(736, 6), IR(48174, 23), IR(197, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(736, 6), IR(48219, 23), IR(198, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(736, 6), IR(48264, 23), IR(199, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(718, 8), IR(48309, 26), IR(200, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(718, 8), IR(48360, 26), IR(201, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(718, 8), IR(48411, 25), IR(202, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(726, 10), IR(48460, 26), IR(203, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(718, 8), IR(48511, 25), IR(204, 1), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(718, 8), IR(48560, 23), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(718, 8), IR(48583, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(718, 8), IR(48607, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(718, 8), IR(48632, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(718, 8), IR(48657, 23), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(718, 8), IR(48680, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(718, 8), IR(48704, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(718, 8), IR(48729, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(718, 8), IR(48754, 23), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(718, 8), IR(48777, 23), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(718, 8), IR(48800, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(718, 8), IR(48826, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(718, 8), IR(48851, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(718, 8), IR(48876, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(718, 8), IR(48900, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(718, 8), IR(48926, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(718, 8), IR(48950, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(718, 8), IR(48976, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(718, 8), IR(49000, 26), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(726, 10), IR(49026, 25), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(726, 10), IR(49051, 23), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(726, 10), IR(49074, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(726, 10), IR(49098, 24), IR(0, 0), IR(310, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(49122, 17), IR(0, 0), IR(311, 1), IR(124, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(742, 2), IR(49139, 21), IR(0, 0), IR(144, 1), IR(136, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(744, 3), IR(49160, 20), IR(0, 0), IR(144, 1), IR(136, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(49180, 24), IR(0, 0), IR(144, 1), IR(136, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(718, 8), IR(49204, 16), IR(205, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(718, 8), IR(49235, 17), IR(206, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(718, 8), IR(49268, 17), IR(207, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(718, 8), IR(49301, 15), IR(208, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(718, 8), IR(49330, 15), IR(209, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(718, 8), IR(49359, 18), IR(210, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(718, 8), IR(49394, 17), IR(211, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(718, 8), IR(49427, 17), IR(212, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(718, 8), IR(49460, 20), IR(213, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(718, 8), IR(49499, 15), IR(214, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(718, 8), IR(49528, 15), IR(215, 1), IR(205, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(49557, 30), IR(216, 1), IR(216, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(49616, 30), IR(217, 1), IR(216, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, false, false, IR(296, 4), IR(49675, 23), IR(218, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, false, false, IR(296, 4), IR(49720, 24), IR(219, 1), IR(312, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(15045, 14), IR(63, 1), IR(313, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(49767, 22), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(49789, 23), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(49812, 28), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(49840, 46), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(49886, 37), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(49923, 65), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(49988, 40), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(50028, 41), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(50069, 39), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(50108, 36), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(50144, 46), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(50190, 45), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(50235, 42), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(50277, 32), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(50309, 29), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(50338, 40), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(50378, 40), IR(0, 0), IR(209, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(50418, 14), IR(0, 0), IR(314, 4), IR(161, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(50432, 23), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(747, 1), IR(50455, 25), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(50480, 32), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(50512, 36), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, false, false, IR(747, 1), IR(50548, 33), IR(0, 0), IR(318, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(50581, 20), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(50601, 20), IR(0, 0), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveEXT, false, false, IR(221, 3), IR(50621, 24), IR(220, 1), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitEXT, false, false, IR(221, 3), IR(50671, 22), IR(221, 1), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(15556, 19), IR(0, 0), IR(321, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(748, 7), IR(50717, 25), IR(222, 1), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(50766, 24), IR(223, 1), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(50813, 22), IR(224, 1), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(50856, 26), IR(225, 1), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(50907, 23), IR(226, 1), IR(322, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(50952, 27), IR(0, 0), IR(323, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(755, 10), IR(50979, 25), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(755, 10), IR(51004, 34), IR(0, 0), IR(324, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(755, 10), IR(51038, 34), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(765, 9), IR(51072, 29), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(755, 10), IR(51101, 26), IR(0, 0), IR(112, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(774, 7), IR(16033, 38), IR(0, 0), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(51189, 21), IR(0, 0), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(16099, 32), IR(0, 0), IR(328, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(51210, 26), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(781, 5), IR(51236, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(786, 2), IR(51263, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(788, 4), IR(51290, 24), IR(0, 0), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(792, 6), IR(51314, 30), IR(0, 0), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(798, 3), IR(51381, 30), IR(0, 0), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(51411, 27), IR(0, 0), IR(115, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPredicatedLoadINTEL, true, true, IR(636, 6), IR(51438, 20), IR(0, 0), IR(330, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPredicatedStoreINTEL, false, false, IR(589, 4), IR(51458, 21), IR(0, 0), IR(330, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(51479, 13), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(51492, 13), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(51505, 19), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(51524, 18), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(51542, 19), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(51561, 19), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(51580, 18), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(51598, 19), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(51617, 18), IR(0, 0), IR(332, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(801, 6), IR(51635, 18), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(807, 4), IR(51653, 19), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(51672, 26), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(51698, 28), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(51726, 33), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(51759, 19), IR(0, 0), IR(335, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(51778, 19), IR(0, 0), IR(337, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(51797, 19), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 930> kInstructionNames{{
  {IR(11498, 9), 440}, // 0 AbortKHR
  {IR(42071, 13), 602}, // 1 AbsISubINTEL
  {IR(42084, 13), 603}, // 2 AbsUSubINTEL
  {IR(30644, 12), 58}, // 3 AccessChain
  {IR(49139, 21), 788}, // 4 AliasDomainDeclINTEL
  {IR(49160, 20), 789}, // 5 AliasScopeDeclINTEL
  {IR(49180, 24), 790}, // 6 AliasScopeListDeclINTEL
  {IR(31941, 4), 143}, // 7 All
  {IR(37336, 25), 428}, // 8 AllocateNodePayloadsAMDX
  {IR(31937, 4), 142}, // 9 Any
  {IR(48926, 24), 779}, // 10 ArbitraryFloatACosINTEL
  {IR(48950, 26), 780}, // 11 ArbitraryFloatACosPiINTEL
  {IR(48876, 24), 777}, // 12 ArbitraryFloatASinINTEL
  {IR(48900, 26), 778}, // 13 ArbitraryFloatASinPiINTEL
  {IR(49026, 25), 783}, // 14 ArbitraryFloatATan2INTEL
  {IR(48976, 24), 781}, // 15 ArbitraryFloatATanINTEL
  {IR(49000, 26), 782}, // 16 ArbitraryFloatATanPiINTEL
  {IR(47896, 24), 750}, // 17 ArbitraryFloatAddALTERA
  {IR(47920, 23), 750}, // 18 ArbitraryFloatAddINTEL
  {IR(47725, 25), 747}, // 19 ArbitraryFloatCastALTERA
  {IR(47774, 32), 748}, // 20 ArbitraryFloatCastFromIntALTERA
  {IR(47806, 31), 748}, // 21 ArbitraryFloatCastFromIntINTEL
  {IR(47750, 24), 747}, // 22 ArbitraryFloatCastINTEL
  {IR(47837, 30), 749}, // 23 ArbitraryFloatCastToIntALTERA
  {IR(47867, 29), 749}, // 24 ArbitraryFloatCastToIntINTEL
  {IR(48411, 25), 761}, // 25 ArbitraryFloatCbrtALTERA
  {IR(48436, 24), 761}, // 26 ArbitraryFloatCbrtINTEL
  {IR(48777, 23), 773}, // 27 ArbitraryFloatCosINTEL
  {IR(48851, 25), 776}, // 28 ArbitraryFloatCosPiINTEL
  {IR(48037, 24), 753}, // 29 ArbitraryFloatDivALTERA
  {IR(48061, 23), 753}, // 30 ArbitraryFloatDivINTEL
  {IR(48264, 23), 758}, // 31 ArbitraryFloatEQALTERA
  {IR(48287, 22), 758}, // 32 ArbitraryFloatEQINTEL
  {IR(48704, 25), 770}, // 33 ArbitraryFloatExp10INTEL
  {IR(48680, 24), 769}, // 34 ArbitraryFloatExp2INTEL
  {IR(48657, 23), 768}, // 35 ArbitraryFloatExpINTEL
  {IR(48729, 25), 771}, // 36 ArbitraryFloatExpm1INTEL
  {IR(48129, 23), 755}, // 37 ArbitraryFloatGEALTERA
  {IR(48152, 22), 755}, // 38 ArbitraryFloatGEINTEL
  {IR(48084, 23), 754}, // 39 ArbitraryFloatGTALTERA
  {IR(48107, 22), 754}, // 40 ArbitraryFloatGTINTEL
  {IR(48460, 26), 762}, // 41 ArbitraryFloatHypotALTERA
  {IR(48486, 25), 762}, // 42 ArbitraryFloatHypotINTEL
  {IR(48219, 23), 757}, // 43 ArbitraryFloatLEALTERA
  {IR(48242, 22), 757}, // 44 ArbitraryFloatLEINTEL
  {IR(48174, 23), 756}, // 45 ArbitraryFloatLTALTERA
  {IR(48197, 22), 756}, // 46 ArbitraryFloatLTINTEL
  {IR(48607, 25), 766}, // 47 ArbitraryFloatLog10INTEL
  {IR(48632, 25), 767}, // 48 ArbitraryFloatLog1pINTEL
  {IR(48583, 24), 765}, // 49 ArbitraryFloatLog2INTEL
  {IR(48560, 23), 764}, // 50 ArbitraryFloatLogINTEL
  {IR(47990, 24), 752}, // 51 ArbitraryFloatMulALTERA
  {IR(48014, 23), 752}, // 52 ArbitraryFloatMulINTEL
  {IR(49051, 23), 784}, // 53 ArbitraryFloatPowINTEL
  {IR(49098, 24), 786}, // 54 ArbitraryFloatPowNINTEL
  {IR(49074, 24), 785}, // 55 ArbitraryFloatPowRINTEL
  {IR(48360, 26), 760}, // 56 ArbitraryFloatRSqrtALTERA
  {IR(48386, 25), 760}, // 57 ArbitraryFloatRSqrtINTEL
  {IR(48309, 26), 759}, // 58 ArbitraryFloatRecipALTERA
  {IR(48335, 25), 759}, // 59 ArbitraryFloatRecipINTEL
  {IR(48800, 26), 774}, // 60 ArbitraryFloatSinCosINTEL
  {IR(47668, 29), 746}, // 61 ArbitraryFloatSinCosPiALTERA
  {IR(47697, 28), 746}, // 62 ArbitraryFloatSinCosPiINTEL
  {IR(48754, 23), 772}, // 63 ArbitraryFloatSinINTEL
  {IR(48826, 25), 775}, // 64 ArbitraryFloatSinPiINTEL
  {IR(48511, 25), 763}, // 65 ArbitraryFloatSqrtALTERA
  {IR(48536, 24), 763}, // 66 ArbitraryFloatSqrtINTEL
  {IR(47943, 24), 751}, // 67 ArbitraryFloatSubALTERA
  {IR(47967, 23), 751}, // 68 ArbitraryFloatSubINTEL
  {IR(15556, 19), 834}, // 69 ArithmeticFenceEXT
  {IR(30691, 12), 61}, // 70 ArrayLength
  {IR(42318, 13), 618}, // 71 AsmCallINTEL
  {IR(13849, 9), 617}, // 72 AsmINTEL
  {IR(42303, 15), 616}, // 73 AsmTargetINTEL
  {IR(42359, 14), 621}, // 74 AssumeTrueKHR
  {IR(32963, 10), 220}, // 75 AtomicAnd
  {IR(32815, 22), 210}, // 76 AtomicCompareExchange
  {IR(32837, 26), 211}, // 77 AtomicCompareExchangeWeak
  {IR(32800, 15), 209}, // 78 AtomicExchange
  {IR(50418, 14), 824}, // 79 AtomicFAddEXT
  {IR(42345, 14), 620}, // 80 AtomicFMaxEXT
  {IR(42331, 14), 619}, // 81 AtomicFMinEXT
  {IR(34348, 16), 292}, // 82 AtomicFlagClear
  {IR(34327, 21), 291}, // 83 AtomicFlagTestAndSet
  {IR(32897, 11), 214}, // 84 AtomicIAdd
  {IR(32880, 17), 213}, // 85 AtomicIDecrement
  {IR(32863, 17), 212}, // 86 AtomicIIncrement
  {IR(32908, 11), 215}, // 87 AtomicISub
  {IR(32777, 11), 207}, // 88 AtomicLoad
  {IR(32973, 9), 221}, // 89 AtomicOr
  {IR(32941, 11), 218}, // 90 AtomicSMax
  {IR(32919, 11), 216}, // 91 AtomicSMin
  {IR(32788, 12), 208}, // 92 AtomicStore
  {IR(32952, 11), 219}, // 93 AtomicUMax
  {IR(32930, 11), 217}, // 94 AtomicUMin
  {IR(32982, 10), 222}, // 95 AtomicXor
  {IR(40567, 28), 548}, // 96 BeginInvocationInterlockEXT
  {IR(36874, 17), 408}, // 97 BitCastArrayQCOM
  {IR(32561, 9), 191}, // 98 BitCount
  {IR(32501, 15), 187}, // 99 BitFieldInsert
  {IR(32516, 17), 188}, // 100 BitFieldSExtract
  {IR(32533, 17), 189}, // 101 BitFieldUExtract
  {IR(32550, 11), 190}, // 102 BitReverse
  {IR(31689, 8), 114}, // 103 Bitcast
  {IR(10424, 18), 358}, // 104 BitcastExtractEXT
  {IR(32486, 11), 185}, // 105 BitwiseAnd
  {IR(51189, 21), 847}, // 106 BitwiseFunctionINTEL
  {IR(32465, 10), 183}, // 107 BitwiseOr
  {IR(32475, 11), 184}, // 108 BitwiseXor
  {IR(33092, 7), 227}, // 109 Branch
  {IR(33099, 18), 228}, // 110 BranchConditional
  {IR(37597, 17), 439}, // 111 BufferPointerEXT
  {IR(33960, 13), 277}, // 112 BuildNDRange
  {IR(29705, 11), 15}, // 113 Capability
  {IR(33918, 26), 275}, // 114 CaptureEventProfilingInfo
  {IR(35424, 23), 344}, // 115 ColorAttachmentReadEXT
  {IR(33510, 15), 255}, // 116 CommitReadPipe
  {IR(33525, 16), 256}, // 117 CommitWritePipe
  {IR(30951, 19), 72}, // 118 CompositeConstruct
  {IR(50548, 33), 829}, // 119 CompositeConstructContinuedINTEL
  {IR(37007, 30), 413}, // 120 CompositeConstructCoopMatQCOM
  {IR(36582, 31), 396}, // 121 CompositeConstructReplicateEXT
  {IR(30970, 17), 73}, // 122 CompositeExtract
  {IR(37037, 28), 414}, // 123 CompositeExtractCoopMatQCOM
  {IR(30987, 16), 74}, // 124 CompositeInsert
  {IR(51263, 27), 851}, // 125 ConditionalCapabilityINTEL
  {IR(51411, 27), 855}, // 126 ConditionalCopyObjectINTEL
  {IR(51236, 27), 850}, // 127 ConditionalEntryPointINTEL
  {IR(51210, 26), 849}, // 128 ConditionalExtensionINTEL
  {IR(17872, 9), 39}, // 129 Constant
  {IR(30211, 18), 40}, // 130 ConstantComposite
  {IR(50480, 32), 827}, // 131 ConstantCompositeContinuedINTEL
  {IR(36518, 30), 394}, // 132 ConstantCompositeReplicateEXT
  {IR(11507, 16), 444}, // 133 ConstantDataKHR
  {IR(30159, 14), 38}, // 134 ConstantFalse
  {IR(42249, 29), 614}, // 135 ConstantFunctionPointerINTEL
  {IR(30323, 13), 42}, // 136 ConstantNull
  {IR(34403, 20), 296}, // 137 ConstantPipeStorage
  {IR(30307, 16), 41}, // 138 ConstantSampler
  {IR(37662, 18), 443}, // 139 ConstantSizeOfEXT
  {IR(37489, 19), 434}, // 140 ConstantStringAMDX
  {IR(30146, 13), 37}, // 141 ConstantTrue
  {IR(32748, 15), 205}, // 142 ControlBarrier
  {IR(50621, 24), 832}, // 143 ControlBarrierArriveEXT
  {IR(50645, 26), 832}, // 144 ControlBarrierArriveINTEL
  {IR(50671, 22), 833}, // 145 ControlBarrierWaitEXT
  {IR(50693, 24), 833}, // 146 ControlBarrierWaitINTEL
  {IR(50601, 20), 831}, // 147 ConvertBF16ToFINTEL
  {IR(50581, 20), 830}, // 148 ConvertFToBF16INTEL
  {IR(31495, 12), 100}, // 149 ConvertFToS
  {IR(31483, 12), 99}, // 150 ConvertFToU
  {IR(51672, 26), 869}, // 151 ConvertHandleToImageINTEL
  {IR(51726, 33), 871}, // 152 ConvertHandleToSampledImageINTEL
  {IR(51698, 28), 870}, // 153 ConvertHandleToSamplerINTEL
  {IR(41216, 18), 571}, // 154 ConvertImageToUNV
  {IR(31572, 14), 107}, // 155 ConvertPtrToU
  {IR(31507, 12), 101}, // 156 ConvertSToF
  {IR(41279, 25), 574}, // 157 ConvertSampledImageToUNV
  {IR(41234, 20), 572}, // 158 ConvertSamplerToUNV
  {IR(36096, 35), 380}, // 159 ConvertUToAccelerationStructureKHR
  {IR(31519, 12), 102}, // 160 ConvertUToF
  {IR(41178, 18), 569}, // 161 ConvertUToImageNV
  {IR(31616, 14), 110}, // 162 ConvertUToPtr
  {IR(41254, 25), 573}, // 163 ConvertUToSampledImageNV
  {IR(41196, 20), 570}, // 164 ConvertUToSamplerNV
  {IR(38802, 27), 487}, // 165 CooperativeMatrixConvertNV
  {IR(36491, 27), 393}, // 166 CooperativeMatrixLengthKHR
  {IR(40541, 26), 547}, // 167 CooperativeMatrixLengthNV
  {IR(36359, 25), 390}, // 168 CooperativeMatrixLoadKHR
  {IR(40466, 24), 544}, // 169 CooperativeMatrixLoadNV
  {IR(40765, 30), 551}, // 170 CooperativeMatrixLoadTensorNV
  {IR(36464, 27), 392}, // 171 CooperativeMatrixMulAddKHR
  {IR(40515, 26), 546}, // 172 CooperativeMatrixMulAddNV
  {IR(40826, 32), 553}, // 173 CooperativeMatrixPerElementOpNV
  {IR(40664, 26), 550}, // 174 CooperativeMatrixReduceNV
  {IR(36384, 26), 391}, // 175 CooperativeMatrixStoreKHR
  {IR(40490, 25), 545}, // 176 CooperativeMatrixStoreNV
  {IR(40795, 31), 552}, // 177 CooperativeMatrixStoreTensorNV
  {IR(41149, 29), 568}, // 178 CooperativeMatrixTransposeNV
  {IR(39025, 24), 494}, // 179 CooperativeVectorLoadNV
  {IR(38770, 32), 486}, // 180 CooperativeVectorMatrixMulAddNV
  {IR(38660, 29), 483}, // 181 CooperativeVectorMatrixMulNV
  {IR(38689, 42), 484}, // 182 CooperativeVectorOuterProductAccumulateNV
  {IR(38731, 39), 485}, // 183 CooperativeVectorReduceSumAccumulateNV
  {IR(39049, 25), 495}, // 184 CooperativeVectorStoreNV
  {IR(35383, 12), 340}, // 185 CopyLogical
  {IR(30617, 11), 56}, // 186 CopyMemory
  {IR(30628, 16), 57}, // 187 CopyMemorySized
  {IR(31003, 11), 75}, // 188 CopyObject
  {IR(34423, 26), 297}, // 189 CreatePipeFromPipeStorage
  {IR(40894, 21), 556}, // 190 CreateTensorLayoutNV
  {IR(41014, 19), 561}, // 191 CreateTensorViewNV
  {IR(33870, 16), 272}, // 192 CreateUserEvent
  {IR(49616, 30), 803}, // 193 CrossWorkgroupCastToPtrALTERA
  {IR(49646, 29), 803}, // 194 CrossWorkgroupCastToPtrINTEL
  {IR(32570, 5), 192}, // 195 DPdx
  {IR(32616, 11), 198}, // 196 DPdxCoarse
  {IR(32587, 9), 195}, // 197 DPdxFine
  {IR(32575, 5), 193}, // 198 DPdy
  {IR(32627, 11), 199}, // 199 DPdyCoarse
  {IR(32596, 9), 196}, // 200 DPdyFine
  {IR(30777, 9), 64}, // 201 Decorate
  {IR(34600, 11), 305}, // 202 DecorateId
  {IR(42383, 15), 623}, // 203 DecorateString
  {IR(42398, 21), 623}, // 204 DecorateStringGOOGLE
  {IR(30801, 16), 66}, // 205 DecorationGroup
  {IR(13038, 25), 564}, // 206 DemoteToHelperInvocation
  {IR(13063, 28), 564}, // 207 DemoteToHelperInvocationEXT
  {IR(35447, 23), 345}, // 208 DepthAttachmentReadEXT
  {IR(31886, 4), 137}, // 209 Dot
  {IR(38829, 17), 488}, // 210 EmitMeshTasksEXT
  {IR(32675, 17), 203}, // 211 EmitStreamVertex
  {IR(32651, 11), 201}, // 212 EmitVertex
  {IR(40595, 26), 549}, // 213 EndInvocationInterlockEXT
  {IR(32662, 13), 202}, // 214 EndPrimitive
  {IR(32692, 19), 204}, // 215 EndStreamPrimitive
  {IR(33706, 14), 265}, // 216 EnqueueKernel
  {IR(33692, 14), 264}, // 217 EnqueueMarker
  {IR(37361, 24), 429}, // 218 EnqueueNodePayloadsAMDX
  {IR(29620, 11), 13}, // 219 EntryPoint
  {IR(36077, 19), 379}, // 220 ExecuteCallableKHR
  {IR(40177, 18), 534}, // 221 ExecuteCallableNV
  {IR(29663, 14), 14}, // 222 ExecutionMode
  {IR(34584, 16), 304}, // 223 ExecutionModeId
  {IR(42373, 10), 622}, // 224 ExpectKHR
  {IR(29474, 8), 11}, // 225 ExtInst
  {IR(29414, 14), 10}, // 226 ExtInstImport
  {IR(36014, 26), 376}, // 227 ExtInstWithForwardRefsKHR
  {IR(29404, 10), 9}, // 228 Extension
  {IR(37065, 20), 415}, // 229 ExtractSubArrayQCOM
  {IR(31718, 5), 118}, // 230 FAdd
  {IR(31549, 9), 105}, // 231 FConvert
  {IR(31753, 5), 125}, // 232 FDiv
  {IR(51778, 19), 873}, // 233 FDot2MixAcc16VALVE
  {IR(51759, 19), 872}, // 234 FDot2MixAcc32VALVE
  {IR(51797, 19), 874}, // 235 FDot4MixAcc32VALVE
  {IR(31778, 5), 130}, // 236 FMod
  {IR(31738, 5), 122}, // 237 FMul
  {IR(31705, 8), 116}, // 238 FNegate
  {IR(32215, 10), 168}, // 239 FOrdEqual
  {IR(32293, 16), 174}, // 240 FOrdGreaterThan
  {IR(32365, 21), 178}, // 241 FOrdGreaterThanEqual
  {IR(32265, 13), 172}, // 242 FOrdLessThan
  {IR(32327, 18), 176}, // 243 FOrdLessThanEqual
  {IR(32237, 13), 170}, // 244 FOrdNotEqual
  {IR(15045, 14), 806}, // 245 FPGARegALTERA
  {IR(15059, 13), 806}, // 246 FPGARegINTEL
  {IR(31773, 5), 129}, // 247 FRem
  {IR(31728, 5), 120}, // 248 FSub
  {IR(32225, 12), 169}, // 249 FUnordEqual
  {IR(32309, 18), 175}, // 250 FUnordGreaterThan
  {IR(32386, 23), 179}, // 251 FUnordGreaterThanEqual
  {IR(32278, 15), 173}, // 252 FUnordLessThan
  {IR(32345, 20), 177}, // 253 FUnordLessThanEqual
  {IR(32250, 15), 171}, // 254 FUnordNotEqual
  {IR(38987, 38), 493}, // 255 FetchMicroTriangleVertexBarycentricNV
  {IR(38952, 35), 492}, // 256 FetchMicroTriangleVertexPositionNV
  {IR(37410, 29), 431}, // 257 FinishWritingNodePayloadAMDX
  {IR(49330, 15), 795}, // 258 FixedCosALTERA
  {IR(49345, 14), 795}, // 259 FixedCosINTEL
  {IR(49427, 17), 798}, // 260 FixedCosPiALTERA
  {IR(49444, 16), 798}, // 261 FixedCosPiINTEL
  {IR(49528, 15), 801}, // 262 FixedExpALTERA
  {IR(49543, 14), 801}, // 263 FixedExpINTEL
  {IR(49499, 15), 800}, // 264 FixedLogALTERA
  {IR(49514, 14), 800}, // 265 FixedLogINTEL
  {IR(49235, 17), 792}, // 266 FixedRecipALTERA
  {IR(49252, 16), 792}, // 267 FixedRecipINTEL
  {IR(49268, 17), 793}, // 268 FixedRsqrtALTERA
  {IR(49285, 16), 793}, // 269 FixedRsqrtINTEL
  {IR(49301, 15), 794}, // 270 FixedSinALTERA
  {IR(49359, 18), 796}, // 271 FixedSinCosALTERA
  {IR(49377, 17), 796}, // 272 FixedSinCosINTEL
  {IR(49460, 20), 799}, // 273 FixedSinCosPiALTERA
  {IR(49480, 19), 799}, // 274 FixedSinCosPiINTEL
  {IR(49316, 14), 794}, // 275 FixedSinINTEL
  {IR(49394, 17), 797}, // 276 FixedSinPiALTERA
  {IR(49411, 16), 797}, // 277 FixedSinPiINTEL
  {IR(49204, 16), 791}, // 278 FixedSqrtALTERA
  {IR(49220, 15), 791}, // 279 FixedSqrtINTEL
  {IR(35931, 7), 370}, // 280 FmaKHR
  {IR(37306, 17), 426}, // 281 FragmentFetchAMD
  {IR(37285, 21), 425}, // 282 FragmentMaskFetchAMD
  {IR(37710, 10), 447}, // 283 FreezeKHR
  {IR(28453, 9), 48}, // 284 Function
  {IR(30526, 13), 51}, // 285 FunctionCall
  {IR(30514, 12), 50}, // 286 FunctionEnd
  {IR(30496, 18), 49}, // 287 FunctionParameter
  {IR(42278, 25), 615}, // 288 FunctionPointerCallINTEL
  {IR(32580, 7), 194}, // 289 Fwidth
  {IR(32638, 13), 200}, // 290 FwidthCoarse
  {IR(32605, 11), 197}, // 291 FwidthFine
  {IR(31647, 17), 112}, // 292 GenericCastToPtr
  {IR(31664, 25), 113}, // 293 GenericCastToPtrExplicit
  {IR(30703, 23), 62}, // 294 GenericPtrMemSemantics
  {IR(33944, 16), 276}, // 295 GetDefaultQueue
  {IR(34449, 35), 298}, // 296 GetKernelLocalSizeForSubgroupCount
  {IR(34484, 25), 299}, // 297 GetKernelMaxNumSubgroups
  {IR(33750, 32), 267}, // 298 GetKernelNDrangeMaxSubGroupSize
  {IR(33720, 30), 266}, // 299 GetKernelNDrangeSubGroupCount
  {IR(33805, 40), 269}, // 300 GetKernelPreferredWorkGroupSizeMultiple
  {IR(33782, 23), 268}, // 301 GetKernelWorkGroupSize
  {IR(33576, 18), 259}, // 302 GetMaxPipePackets
  {IR(33558, 18), 258}, // 303 GetNumPipePackets
  {IR(10277, 9), 353}, // 304 GraphARM
  {IR(35599, 17), 351}, // 305 GraphConstantARM
  {IR(35667, 12), 356}, // 306 GraphEndARM
  {IR(35616, 19), 352}, // 307 GraphEntryPointARM
  {IR(35635, 14), 354}, // 308 GraphInputARM
  {IR(35649, 18), 355}, // 309 GraphSetOutputARM
  {IR(33263, 9), 238}, // 310 GroupAll
  {IR(33272, 9), 239}, // 311 GroupAny
  {IR(33232, 15), 236}, // 312 GroupAsyncCopy
  {IR(51505, 19), 860}, // 313 GroupBitwiseAndKHR
  {IR(51524, 18), 861}, // 314 GroupBitwiseOrKHR
  {IR(51542, 19), 862}, // 315 GroupBitwiseXorKHR
  {IR(33281, 15), 240}, // 316 GroupBroadcast
  {IR(33651, 20), 262}, // 317 GroupCommitReadPipe
  {IR(33671, 21), 263}, // 318 GroupCommitWritePipe
  {IR(30817, 14), 67}, // 319 GroupDecorate
  {IR(33339, 10), 242}, // 320 GroupFAdd
  {IR(37124, 23), 418}, // 321 GroupFAddNonUniformAMD
  {IR(33379, 10), 246}, // 322 GroupFMax
  {IR(37216, 23), 422}, // 323 GroupFMaxNonUniformAMD
  {IR(33349, 10), 243}, // 324 GroupFMin
  {IR(37147, 23), 419}, // 325 GroupFMinNonUniformAMD
  {IR(51492, 13), 859}, // 326 GroupFMulKHR
  {IR(33329, 10), 241}, // 327 GroupIAdd
  {IR(37101, 23), 417}, // 328 GroupIAddNonUniformAMD
  {IR(51479, 13), 858}, // 329 GroupIMulKHR
  {IR(51561, 19), 863}, // 330 GroupLogicalAndKHR
  {IR(51580, 18), 864}, // 331 GroupLogicalOrKHR
  {IR(51598, 19), 865}, // 332 GroupLogicalXorKHR
  {IR(30876, 20), 68}, // 333 GroupMemberDecorate
  {IR(34632, 19), 307}, // 334 GroupNonUniformAll
  {IR(34670, 24), 309}, // 335 GroupNonUniformAllEqual
  {IR(34651, 19), 308}, // 336 GroupNonUniformAny
  {IR(6847, 22), 312}, // 337 GroupNonUniformBallot
  {IR(34810, 30), 315}, // 338 GroupNonUniformBallotBitCount
  {IR(34778, 32), 314}, // 339 GroupNonUniformBallotBitExtract
  {IR(34840, 29), 316}, // 340 GroupNonUniformBallotFindLSB
  {IR(34869, 29), 317}, // 341 GroupNonUniformBallotFindMSB
  {IR(35176, 26), 332}, // 342 GroupNonUniformBitwiseAnd
  {IR(35202, 25), 333}, // 343 GroupNonUniformBitwiseOr
  {IR(35227, 26), 334}, // 344 GroupNonUniformBitwiseXor
  {IR(34694, 25), 310}, // 345 GroupNonUniformBroadcast
  {IR(34719, 30), 311}, // 346 GroupNonUniformBroadcastFirst
  {IR(34611, 21), 306}, // 347 GroupNonUniformElect
  {IR(34996, 20), 323}, // 348 GroupNonUniformFAdd
  {IR(35156, 20), 331}, // 349 GroupNonUniformFMax
  {IR(35096, 20), 328}, // 350 GroupNonUniformFMin
  {IR(35036, 20), 325}, // 351 GroupNonUniformFMul
  {IR(34976, 20), 322}, // 352 GroupNonUniformIAdd
  {IR(35016, 20), 324}, // 353 GroupNonUniformIMul
  {IR(34749, 29), 313}, // 354 GroupNonUniformInverseBallot
  {IR(35253, 26), 335}, // 355 GroupNonUniformLogicalAnd
  {IR(35279, 25), 336}, // 356 GroupNonUniformLogicalOr
  {IR(35304, 26), 337}, // 357 GroupNonUniformLogicalXor
  {IR(38864, 28), 490}, // 358 GroupNonUniformPartitionEXT
  {IR(38892, 27), 490}, // 359 GroupNonUniformPartitionNV
  {IR(37531, 26), 436}, // 360 GroupNonUniformQuadAllKHR
  {IR(37557, 26), 437}, // 361 GroupNonUniformQuadAnyKHR
  {IR(35330, 29), 338}, // 362 GroupNonUniformQuadBroadcast
  {IR(35359, 24), 339}, // 363 GroupNonUniformQuadSwap
  {IR(15284, 25), 374}, // 364 GroupNonUniformRotateKHR
  {IR(35116, 20), 329}, // 365 GroupNonUniformSMax
  {IR(35056, 20), 326}, // 366 GroupNonUniformSMin
  {IR(9986, 23), 318}, // 367 GroupNonUniformShuffle
  {IR(34949, 27), 321}, // 368 GroupNonUniformShuffleDown
  {IR(34924, 25), 320}, // 369 GroupNonUniformShuffleUp
  {IR(34898, 26), 319}, // 370 GroupNonUniformShuffleXor
  {IR(35136, 20), 330}, // 371 GroupNonUniformUMax
  {IR(35076, 20), 327}, // 372 GroupNonUniformUMin
  {IR(33594, 28), 260}, // 373 GroupReserveReadPipePackets
  {IR(33622, 29), 261}, // 374 GroupReserveWritePipePackets
  {IR(33399, 10), 248}, // 375 GroupSMax
  {IR(37262, 23), 424}, // 376 GroupSMaxNonUniformAMD
  {IR(33369, 10), 245}, // 377 GroupSMin
  {IR(37193, 23), 421}, // 378 GroupSMinNonUniformAMD
  {IR(33389, 10), 247}, // 379 GroupUMax
  {IR(37239, 23), 423}, // 380 GroupUMaxNonUniformAMD
  {IR(33359, 10), 244}, // 381 GroupUMin
  {IR(37170, 23), 420}, // 382 GroupUMinNonUniformAMD
  {IR(33247, 16), 237}, // 383 GroupWaitEvents
  {IR(39521, 26), 511}, // 384 HitObjectExecuteShaderEXT
  {IR(38155, 25), 463}, // 385 HitObjectExecuteShaderNV
  {IR(39574, 26), 513}, // 386 HitObjectGetAttributesEXT
  {IR(38206, 25), 465}, // 387 HitObjectGetAttributesNV
  {IR(40253, 24), 536}, // 388 HitObjectGetClusterIdNV
  {IR(39547, 27), 512}, // 389 HitObjectGetCurrentTimeEXT
  {IR(38180, 26), 464}, // 390 HitObjectGetCurrentTimeNV
  {IR(39653, 29), 516}, // 391 HitObjectGetGeometryIndexEXT
  {IR(38282, 28), 468}, // 392 HitObjectGetGeometryIndexNV
  {IR(39600, 23), 514}, // 393 HitObjectGetHitKindEXT
  {IR(38231, 22), 466}, // 394 HitObjectGetHitKindNV
  {IR(39708, 35), 518}, // 395 HitObjectGetInstanceCustomIndexEXT
  {IR(38335, 34), 470}, // 396 HitObjectGetInstanceCustomIndexNV
  {IR(39682, 26), 517}, // 397 HitObjectGetInstanceIdEXT
  {IR(38310, 25), 469}, // 398 HitObjectGetInstanceIdNV
  {IR(39154, 51), 499}, // 399 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(41645, 27), 584}, // 400 HitObjectGetLSSPositionsNV
  {IR(41672, 23), 585}, // 401 HitObjectGetLSSRadiiNV
  {IR(39774, 34), 520}, // 402 HitObjectGetObjectRayDirectionEXT
  {IR(37867, 33), 453}, // 403 HitObjectGetObjectRayDirectionNV
  {IR(39743, 31), 519}, // 404 HitObjectGetObjectRayOriginEXT
  {IR(37900, 30), 454}, // 405 HitObjectGetObjectRayOriginNV
  {IR(39871, 29), 523}, // 406 HitObjectGetObjectToWorldEXT
  {IR(37839, 28), 452}, // 407 HitObjectGetObjectToWorldNV
  {IR(39623, 30), 515}, // 408 HitObjectGetPrimitiveIndexEXT
  {IR(38253, 29), 467}, // 409 HitObjectGetPrimitiveIndexNV
  {IR(39205, 24), 500}, // 410 HitObjectGetRayFlagsEXT
  {IR(39929, 23), 525}, // 411 HitObjectGetRayTMaxEXT
  {IR(38430, 22), 473}, // 412 HitObjectGetRayTMaxNV
  {IR(40277, 23), 537}, // 413 HitObjectGetRayTMinEXT
  {IR(38452, 22), 474}, // 414 HitObjectGetRayTMinNV
  {IR(40300, 45), 538}, // 415 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(37995, 44), 457}, // 416 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(40345, 40), 539}, // 417 HitObjectGetShaderRecordBufferHandleEXT
  {IR(37956, 39), 456}, // 418 HitObjectGetShaderRecordBufferHandleNV
  {IR(41589, 29), 582}, // 419 HitObjectGetSpherePositionNV
  {IR(41618, 27), 583}, // 420 HitObjectGetSphereRadiusNV
  {IR(39808, 33), 521}, // 421 HitObjectGetWorldRayDirectionEXT
  {IR(38369, 32), 471}, // 422 HitObjectGetWorldRayDirectionNV
  {IR(39841, 30), 522}, // 423 HitObjectGetWorldRayOriginEXT
  {IR(38401, 29), 472}, // 424 HitObjectGetWorldRayOriginNV
  {IR(39900, 29), 524}, // 425 HitObjectGetWorldToObjectEXT
  {IR(37811, 28), 451}, // 426 HitObjectGetWorldToObjectNV
  {IR(40385, 20), 540}, // 427 HitObjectIsEmptyEXT
  {IR(38474, 19), 475}, // 428 HitObjectIsEmptyNV
  {IR(40405, 18), 541}, // 429 HitObjectIsHitEXT
  {IR(38493, 17), 476}, // 430 HitObjectIsHitNV
  {IR(41718, 20), 587}, // 431 HitObjectIsLSSHitNV
  {IR(40423, 19), 542}, // 432 HitObjectIsMissEXT
  {IR(38510, 18), 477}, // 433 HitObjectIsMissNV
  {IR(41695, 23), 586}, // 434 HitObjectIsSphereHitNV
  {IR(39497, 24), 510}, // 435 HitObjectRecordEmptyEXT
  {IR(38039, 23), 458}, // 436 HitObjectRecordEmptyNV
  {IR(39074, 28), 496}, // 437 HitObjectRecordFromQueryEXT
  {IR(37720, 27), 448}, // 438 HitObjectRecordHitMotionNV
  {IR(38082, 21), 460}, // 439 HitObjectRecordHitNV
  {IR(37747, 36), 449}, // 440 HitObjectRecordHitWithIndexMotionNV
  {IR(38103, 30), 461}, // 441 HitObjectRecordHitWithIndexNV
  {IR(39102, 23), 497}, // 442 HitObjectRecordMissEXT
  {IR(39125, 29), 498}, // 443 HitObjectRecordMissMotionEXT
  {IR(37783, 28), 450}, // 444 HitObjectRecordMissMotionNV
  {IR(38133, 22), 462}, // 445 HitObjectRecordMissNV
  {IR(39274, 33), 502}, // 446 HitObjectReorderExecuteShaderEXT
  {IR(39229, 45), 501}, // 447 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(39339, 38), 504}, // 448 HitObjectTraceMotionReorderExecuteEXT
  {IR(39449, 21), 508}, // 449 HitObjectTraceRayEXT
  {IR(39470, 27), 509}, // 450 HitObjectTraceRayMotionEXT
  {IR(37930, 26), 455}, // 451 HitObjectTraceRayMotionNV
  {IR(38062, 20), 459}, // 452 HitObjectTraceRayNV
  {IR(39307, 32), 503}, // 453 HitObjectTraceReorderExecuteEXT
  {IR(31713, 5), 117}, // 454 IAdd
  {IR(31890, 10), 138}, // 455 IAddCarry
  {IR(42097, 13), 604}, // 456 IAddSatINTEL
  {IR(42123, 14), 606}, // 457 IAverageINTEL
  {IR(42151, 21), 608}, // 458 IAverageRoundedINTEL
  {IR(32086, 7), 158}, // 459 IEqual
  {IR(31733, 5), 121}, // 460 IMul
  {IR(42219, 15), 612}, // 461 IMul32x16INTEL
  {IR(32093, 10), 159}, // 462 INotEqual
  {IR(31723, 5), 119}, // 463 ISub
  {IR(31900, 11), 139}, // 464 ISubBorrow
  {IR(42193, 13), 610}, // 465 ISubSatINTEL
  {IR(36131, 22), 381}, // 466 IgnoreIntersectionKHR
  {IR(39995, 21), 527}, // 467 IgnoreIntersectionNV
  {IR(28497, 6), 91}, // 468 Image
  {IR(36978, 29), 412}, // 469 ImageBlockMatchGatherSADQCOM
  {IR(36949, 29), 411}, // 470 ImageBlockMatchGatherSSDQCOM
  {IR(36851, 23), 407}, // 471 ImageBlockMatchSADQCOM
  {IR(36828, 23), 406}, // 472 ImageBlockMatchSSDQCOM
  {IR(36920, 29), 410}, // 473 ImageBlockMatchWindowSADQCOM
  {IR(36891, 29), 409}, // 474 ImageBlockMatchWindowSSDQCOM
  {IR(36809, 19), 405}, // 475 ImageBoxFilterQCOM
  {IR(31331, 16), 88}, // 476 ImageDrefGather
  {IR(31308, 11), 86}, // 477 ImageFetch
  {IR(31319, 12), 87}, // 478 ImageGather
  {IR(37085, 16), 416}, // 479 ImageGatherQCOM
  {IR(31368, 17), 92}, // 480 ImageQueryFormat
  {IR(31448, 17), 97}, // 481 ImageQueryLevels
  {IR(31434, 14), 96}, // 482 ImageQueryLod
  {IR(31385, 16), 93}, // 483 ImageQueryOrder
  {IR(31465, 18), 98}, // 484 ImageQuerySamples
  {IR(31419, 15), 95}, // 485 ImageQuerySize
  {IR(31401, 18), 94}, // 486 ImageQuerySizeLod
  {IR(31347, 10), 89}, // 487 ImageRead
  {IR(31165, 27), 81}, // 488 ImageSampleDrefExplicitLod
  {IR(31138, 27), 80}, // 489 ImageSampleDrefImplicitLod
  {IR(31115, 23), 79}, // 490 ImageSampleExplicitLod
  {IR(38597, 23), 481}, // 491 ImageSampleFootprintNV
  {IR(31069, 23), 78}, // 492 ImageSampleImplicitLod
  {IR(31277, 31), 85}, // 493 ImageSampleProjDrefExplicitLod
  {IR(31246, 31), 84}, // 494 ImageSampleProjDrefImplicitLod
  {IR(31219, 27), 83}, // 495 ImageSampleProjExplicitLod
  {IR(31192, 27), 82}, // 496 ImageSampleProjImplicitLod
  {IR(36785, 24), 404}, // 497 ImageSampleWeightedQCOM
  {IR(34272, 22), 288}, // 498 ImageSparseDrefGather
  {IR(34237, 17), 286}, // 499 ImageSparseFetch
  {IR(34254, 18), 287}, // 500 ImageSparseGather
  {IR(34364, 16), 293}, // 501 ImageSparseRead
  {IR(34064, 33), 281}, // 502 ImageSparseSampleDrefExplicitLod
  {IR(34031, 33), 280}, // 503 ImageSparseSampleDrefImplicitLod
  {IR(34002, 29), 279}, // 504 ImageSparseSampleExplicitLod
  {IR(33973, 29), 278}, // 505 ImageSparseSampleImplicitLod
  {IR(34200, 37), 285}, // 506 ImageSparseSampleProjDrefExplicitLod
  {IR(34163, 37), 284}, // 507 ImageSparseSampleProjDrefImplicitLod
  {IR(34130, 33), 283}, // 508 ImageSparseSampleProjExplicitLod
  {IR(34097, 33), 282}, // 509 ImageSparseSampleProjImplicitLod
  {IR(34294, 26), 289}, // 510 ImageSparseTexelsResident
  {IR(30548, 18), 53}, // 511 ImageTexelPointer
  {IR(31357, 11), 90}, // 512 ImageWrite
  {IR(30656, 20), 59}, // 513 InBoundsAccessChain
  {IR(30726, 23), 63}, // 514 InBoundsPtrAccessChain
  {IR(31957, 9), 146}, // 515 IsFinite
  {IR(41080, 22), 565}, // 516 IsHelperInvocationEXT
  {IR(31951, 6), 145}, // 517 IsInf
  {IR(31945, 6), 144}, // 518 IsNan
  {IR(37466, 23), 433}, // 519 IsNodePayloadValidAMDX
  {IR(31966, 9), 147}, // 520 IsNormal
  {IR(33886, 13), 273}, // 521 IsValidEvent
  {IR(33541, 17), 257}, // 522 IsValidReserveId
  {IR(33169, 5), 230}, // 523 Kill
  {IR(33086, 6), 226}, // 524 Label
  {IR(31986, 14), 149}, // 525 LessOrGreater
  {IR(33205, 14), 234}, // 526 LifetimeStart
  {IR(33219, 13), 235}, // 527 LifetimeStop
  {IR(29399, 5), 8}, // 528 Line
  {IR(30606, 5), 54}, // 529 Load
  {IR(32057, 11), 155}, // 530 LogicalAnd
  {IR(32018, 13), 152}, // 531 LogicalEqual
  {IR(32068, 11), 156}, // 532 LogicalNot
  {IR(32031, 16), 153}, // 533 LogicalNotEqual
  {IR(32047, 10), 154}, // 534 LogicalOr
  {IR(49122, 17), 787}, // 535 LoopControlINTEL
  {IR(33026, 10), 224}, // 536 LoopMerge
  {IR(51635, 18), 867}, // 537 MaskedGatherINTEL
  {IR(51653, 19), 868}, // 538 MaskedScatterINTEL
  {IR(31855, 18), 135}, // 539 MatrixTimesMatrix
  {IR(31801, 18), 132}, // 540 MatrixTimesScalar
  {IR(31837, 18), 134}, // 541 MatrixTimesVector
  {IR(30786, 15), 65}, // 542 MemberDecorate
  {IR(37642, 20), 442}, // 543 MemberDecorateIdEXT
  {IR(42419, 21), 624}, // 544 MemberDecorateString
  {IR(42440, 27), 624}, // 545 MemberDecorateStringGOOGLE
  {IR(29381, 11), 6}, // 546 MemberName
  {IR(32763, 14), 206}, // 547 MemoryBarrier
  {IR(29546, 12), 12}, // 548 MemoryModel
  {IR(34549, 19), 302}, // 549 MemoryNamedBarrier
  {IR(34568, 16), 303}, // 550 ModuleProcessed
  {IR(29376, 5), 5}, // 551 Name
  {IR(34526, 23), 301}, // 552 NamedBarrierInitialize
  {IR(34320, 7), 290}, // 553 NoLine
  {IR(37439, 27), 432}, // 554 NodePayloadArrayLengthAMDX
  {IR(29172, 4), 0}, // 555 Nop
  {IR(32497, 4), 186}, // 556 Not
  {IR(32000, 8), 150}, // 557 Ordered
  {IR(31873, 13), 136}, // 558 OuterProduct
  {IR(32992, 4), 223}, // 559 Phi
  {IR(37700, 10), 446}, // 560 PoisonKHR
  {IR(51438, 20), 856}, // 561 PredicatedLoadINTEL
  {IR(51458, 21), 857}, // 562 PredicatedStoreINTEL
  {IR(30676, 15), 60}, // 563 PtrAccessChain
  {IR(49557, 30), 802}, // 564 PtrCastToCrossWorkgroupALTERA
  {IR(49587, 29), 802}, // 565 PtrCastToCrossWorkgroupINTEL
  {IR(31630, 17), 111}, // 566 PtrCastToGeneric
  {IR(35416, 8), 343}, // 567 PtrDiff
  {IR(35395, 9), 341}, // 568 PtrEqual
  {IR(35404, 12), 342}, // 569 PtrNotEqual
  {IR(31558, 14), 106}, // 570 QuantizeToF16
  {IR(41385, 17), 576}, // 571 RawAccessChainNV
  {IR(36704, 31), 401}, // 572 RayQueryConfirmIntersectionKHR
  {IR(36672, 32), 400}, // 573 RayQueryGenerateIntersectionKHR
  {IR(40230, 23), 535}, // 574 RayQueryGetClusterIdNV
  {IR(50069, 39), 815}, // 575 RayQueryGetIntersectionBarycentricsKHR
  {IR(50144, 46), 817}, // 576 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(40195, 35), 535}, // 577 RayQueryGetIntersectionClusterIdNV
  {IR(50108, 36), 816}, // 578 RayQueryGetIntersectionFrontFaceKHR
  {IR(49988, 40), 813}, // 579 RayQueryGetIntersectionGeometryIndexKHR
  {IR(49840, 46), 810}, // 580 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(49886, 37), 811}, // 581 RayQueryGetIntersectionInstanceIdKHR
  {IR(49923, 65), 812}, // 582 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(41552, 37), 581}, // 583 RayQueryGetIntersectionLSSHitValueNV
  {IR(41480, 38), 579}, // 584 RayQueryGetIntersectionLSSPositionsNV
  {IR(41518, 34), 580}, // 585 RayQueryGetIntersectionLSSRadiiNV
  {IR(50190, 45), 818}, // 586 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(50235, 42), 819}, // 587 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(50338, 40), 822}, // 588 RayQueryGetIntersectionObjectToWorldKHR
  {IR(50028, 41), 814}, // 589 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(41402, 40), 577}, // 590 RayQueryGetIntersectionSpherePositionNV
  {IR(41442, 38), 578}, // 591 RayQueryGetIntersectionSphereRadiusNV
  {IR(49812, 28), 809}, // 592 RayQueryGetIntersectionTKHR
  {IR(40070, 50), 532}, // 593 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(36754, 31), 403}, // 594 RayQueryGetIntersectionTypeKHR
  {IR(50378, 40), 823}, // 595 RayQueryGetIntersectionWorldToObjectKHR
  {IR(49789, 23), 808}, // 596 RayQueryGetRayFlagsKHR
  {IR(49767, 22), 807}, // 597 RayQueryGetRayTMinKHR
  {IR(50277, 32), 820}, // 598 RayQueryGetWorldRayDirectionKHR
  {IR(50309, 29), 821}, // 599 RayQueryGetWorldRayOriginKHR
  {IR(36629, 22), 398}, // 600 RayQueryInitializeKHR
  {IR(41760, 19), 589}, // 601 RayQueryIsLSSHitNV
  {IR(41738, 22), 588}, // 602 RayQueryIsSphereHitNV
  {IR(36735, 19), 402}, // 603 RayQueryProceedKHR
  {IR(36651, 21), 399}, // 604 RayQueryTerminateKHR
  {IR(37323, 13), 427}, // 605 ReadClockKHR
  {IR(33409, 9), 249}, // 606 ReadPipe
  {IR(49675, 23), 804}, // 607 ReadPipeBlockingALTERA
  {IR(49698, 22), 804}, // 608 ReadPipeBlockingINTEL
  {IR(33857, 13), 271}, // 609 ReleaseEvent
  {IR(39394, 25), 506}, // 610 ReorderThreadWithHintEXT
  {IR(38557, 24), 479}, // 611 ReorderThreadWithHintNV
  {IR(39419, 30), 507}, // 612 ReorderThreadWithHitObjectEXT
  {IR(38528, 29), 478}, // 613 ReorderThreadWithHitObjectNV
  {IR(39952, 22), 526}, // 614 ReportIntersectionKHR
  {IR(39974, 21), 526}, // 615 ReportIntersectionNV
  {IR(33463, 23), 253}, // 616 ReserveReadPipePackets
  {IR(33486, 24), 254}, // 617 ReserveWritePipePackets
  {IR(33428, 17), 251}, // 618 ReservedReadPipe
  {IR(33445, 18), 252}, // 619 ReservedWritePipe
  {IR(47649, 19), 745}, // 620 RestoreMemoryINTEL
  {IR(33845, 12), 270}, // 621 RetainEvent
  {IR(33174, 7), 231}, // 622 Return
  {IR(33181, 12), 232}, // 623 ReturnValue
  {IR(51617, 18), 866}, // 624 RoundFToTF32INTEL
  {IR(31540, 9), 104}, // 625 SConvert
  {IR(31748, 5), 124}, // 626 SDiv
  {IR(36216, 5), 383}, // 627 SDot
  {IR(36257, 11), 386}, // 628 SDotAccSat
  {IR(36268, 14), 386}, // 629 SDotAccSatKHR
  {IR(36221, 8), 383}, // 630 SDotKHR
  {IR(32116, 13), 161}, // 631 SGreaterThan
  {IR(32147, 18), 163}, // 632 SGreaterThanEqual
  {IR(32175, 10), 165}, // 633 SLessThan
  {IR(32200, 15), 167}, // 634 SLessThanEqual
  {IR(31768, 5), 128}, // 635 SMod
  {IR(31924, 13), 141}, // 636 SMulExtended
  {IR(31697, 8), 115}, // 637 SNegate
  {IR(31763, 5), 127}, // 638 SRem
  {IR(36242, 6), 385}, // 639 SUDot
  {IR(36307, 12), 388}, // 640 SUDotAccSat
  {IR(36319, 15), 388}, // 641 SUDotAccSatKHR
  {IR(36248, 9), 385}, // 642 SUDotKHR
  {IR(31024, 13), 77}, // 643 SampledImage
  {IR(41304, 29), 575}, // 644 SamplerImageAddressingModeNV
  {IR(31586, 15), 108}, // 645 SatConvertSToU
  {IR(31601, 15), 109}, // 646 SatConvertUToS
  {IR(47633, 16), 744}, // 647 SaveMemoryINTEL
  {IR(32079, 7), 157}, // 648 Select
  {IR(33071, 15), 225}, // 649 SelectionMerge
  {IR(38846, 18), 489}, // 650 SetMeshOutputsEXT
  {IR(33899, 19), 274}, // 651 SetUserEventStatus
  {IR(32448, 17), 182}, // 652 ShiftLeftLogical
  {IR(32427, 21), 181}, // 653 ShiftRightArithmetic
  {IR(32409, 18), 180}, // 654 ShiftRightLogical
  {IR(31975, 11), 148}, // 655 SignBitSet
  {IR(34380, 7), 294}, // 656 SizeOf
  {IR(29353, 7), 3}, // 657 Source
  {IR(29234, 16), 2}, // 658 SourceContinued
  {IR(29360, 16), 4}, // 659 SourceExtension
  {IR(30371, 13), 45}, // 660 SpecConstant
  {IR(51314, 30), 853}, // 661 SpecConstantArchitectureINTEL
  {IR(51381, 30), 854}, // 662 SpecConstantCapabilitiesINTEL
  {IR(30384, 22), 46}, // 663 SpecConstantComposite
  {IR(50512, 36), 828}, // 664 SpecConstantCompositeContinuedINTEL
  {IR(36548, 34), 395}, // 665 SpecConstantCompositeReplicateEXT
  {IR(37680, 20), 445}, // 666 SpecConstantDataKHR
  {IR(30353, 18), 44}, // 667 SpecConstantFalse
  {IR(30447, 15), 47}, // 668 SpecConstantOp
  {IR(37508, 23), 435}, // 669 SpecConstantStringAMDX
  {IR(51290, 24), 852}, // 670 SpecConstantTargetINTEL
  {IR(30336, 17), 43}, // 671 SpecConstantTrue
  {IR(35470, 25), 346}, // 672 StencilAttachmentReadEXT
  {IR(30611, 6), 55}, // 673 Store
  {IR(29392, 7), 7}, // 674 String
  {IR(50979, 25), 841}, // 675 Subgroup2DBlockLoadINTEL
  {IR(51004, 34), 842}, // 676 Subgroup2DBlockLoadTransformINTEL
  {IR(51038, 34), 843}, // 677 Subgroup2DBlockLoadTransposeINTEL
  {IR(51072, 29), 844}, // 678 Subgroup2DBlockPrefetchINTEL
  {IR(51101, 26), 845}, // 679 Subgroup2DBlockStoreINTEL
  {IR(35968, 20), 373}, // 680 SubgroupAllEqualKHR
  {IR(35938, 15), 371}, // 681 SubgroupAllKHR
  {IR(35953, 15), 372}, // 682 SubgroupAnyKHR
  {IR(46151, 30), 708}, // 683 SubgroupAvcBmeInitializeINTEL
  {IR(46121, 30), 707}, // 684 SubgroupAvcFmeInitializeINTEL
  {IR(44556, 35), 677}, // 685 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(44591, 39), 678}, // 686 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(45232, 38), 691}, // 687 SubgroupAvcImeConvertToMceResultINTEL
  {IR(44855, 45), 684}, // 688 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(44955, 53), 686}, // 689 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(45176, 56), 690}, // 690 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(45064, 54), 688}, // 691 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(44808, 47), 683}, // 692 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(44900, 55), 685}, // 693 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(45118, 58), 689}, // 694 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(45008, 56), 687}, // 695 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(45864, 36), 702}, // 696 SubgroupAvcImeGetBorderReachedINTEL
  {IR(45316, 44), 693}, // 697 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(45270, 46), 692}, // 698 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(45731, 66), 700}, // 699 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(45663, 68), 699}, // 700 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(45797, 67), 701}, // 701 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(45526, 68), 697}, // 702 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(45456, 70), 696}, // 703 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(45594, 69), 698}, // 704 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(45900, 48), 703}, // 705 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(45948, 59), 704}, // 706 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(46065, 56), 706}, // 707 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(46007, 58), 705}, // 708 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(44419, 30), 673}, // 709 SubgroupAvcImeInitializeINTEL
  {IR(44523, 33), 676}, // 710 SubgroupAvcImeRefWindowSizeINTEL
  {IR(44487, 36), 675}, // 711 SubgroupAvcImeSetDualReferenceINTEL
  {IR(44720, 54), 681}, // 712 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(44630, 43), 679}, // 713 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(44449, 38), 674}, // 714 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(44673, 47), 680}, // 715 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(44774, 34), 682}, // 716 SubgroupAvcImeSetWeightedSadINTEL
  {IR(45409, 47), 695}, // 717 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(45360, 49), 694}, // 718 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(43809, 39), 658}, // 719 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(43848, 38), 659}, // 720 SubgroupAvcMceConvertToImeResultINTEL
  {IR(43886, 39), 660}, // 721 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(43925, 38), 661}, // 722 SubgroupAvcMceConvertToRefResultINTEL
  {IR(43963, 39), 662}, // 723 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(44002, 38), 663}, // 724 SubgroupAvcMceConvertToSicResultINTEL
  {IR(44115, 43), 666}, // 725 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(43247, 50), 647}, // 726 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(42845, 60), 639}, // 727 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(43045, 51), 643}, // 728 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(43191, 56), 646}, // 729 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(42958, 47), 641}, // 730 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(43546, 56), 653}, // 731 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(43445, 50), 651}, // 732 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(43140, 51), 645}, // 733 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(43349, 49), 649}, // 734 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(43297, 52), 648}, // 735 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(43495, 51), 652}, // 736 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(44234, 38), 669}, // 737 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(44076, 39), 665}, // 738 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(44158, 38), 667}, // 739 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(44196, 38), 668}, // 740 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(44272, 45), 670}, // 741 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(44317, 40), 671}, // 742 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(44357, 62), 672}, // 743 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(44040, 36), 664}, // 744 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(43602, 33), 654}, // 745 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(43748, 61), 657}, // 746 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(42905, 53), 640}, // 747 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(43096, 44), 644}, // 748 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(43005, 40), 642}, // 749 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(43398, 47), 650}, // 750 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(43687, 61), 656}, // 751 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(43635, 52), 655}, // 752 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(46181, 39), 709}, // 753 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(46503, 38), 716}, // 754 SubgroupAvcRefConvertToMceResultINTEL
  {IR(46356, 45), 713}, // 755 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(46401, 46), 714}, // 756 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(46447, 56), 715}, // 757 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46309, 47), 712}, // 758 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(46220, 46), 710}, // 759 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(46266, 43), 711}, // 760 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(46639, 42), 720}, // 761 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(46603, 36), 719}, // 762 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(46571, 32), 718}, // 763 SubgroupAvcSicConfigureSkcINTEL
  {IR(46720, 39), 722}, // 764 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(47260, 38), 734}, // 765 SubgroupAvcSicConvertToMceResultINTEL
  {IR(47035, 31), 729}, // 766 SubgroupAvcSicEvaluateIpeINTEL
  {IR(47113, 45), 731}, // 767 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(47158, 46), 732}, // 768 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(47204, 56), 733}, // 769 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(47066, 47), 730}, // 770 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(47377, 46), 737}, // 771 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(47333, 44), 736}, // 772 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(47598, 35), 742}, // 773 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(47464, 36), 739}, // 774 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(47298, 35), 735}, // 775 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(46681, 39), 721}, // 776 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(47423, 41), 738}, // 777 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(47500, 50), 740}, // 778 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(47550, 48), 741}, // 779 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(46541, 30), 717}, // 780 SubgroupAvcSicInitializeINTEL
  {IR(46901, 43), 726}, // 781 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(46992, 43), 728}, // 782 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(46851, 50), 725}, // 783 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(46803, 48), 724}, // 784 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(46759, 44), 723}, // 785 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(46944, 48), 727}, // 786 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6608, 18), 364}, // 787 SubgroupBallotKHR
  {IR(50952, 27), 840}, // 788 SubgroupBlockPrefetchINTEL
  {IR(41851, 23), 594}, // 789 SubgroupBlockReadINTEL
  {IR(41874, 24), 595}, // 790 SubgroupBlockWriteINTEL
  {IR(35805, 27), 365}, // 791 SubgroupFirstInvocationKHR
  {IR(41898, 28), 596}, // 792 SubgroupImageBlockReadINTEL
  {IR(41926, 29), 597}, // 793 SubgroupImageBlockWriteINTEL
  {IR(41955, 33), 598}, // 794 SubgroupImageMediaBlockReadINTEL
  {IR(41988, 34), 599}, // 795 SubgroupImageMediaBlockWriteINTEL
  {IR(16033, 38), 846}, // 796 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(35988, 26), 375}, // 797 SubgroupReadInvocationKHR
  {IR(41779, 25), 591}, // 798 SubgroupShuffleDownINTEL
  {IR(13631, 21), 590}, // 799 SubgroupShuffleINTEL
  {IR(41804, 23), 592}, // 800 SubgroupShuffleUpINTEL
  {IR(41827, 24), 593}, // 801 SubgroupShuffleXorINTEL
  {IR(33162, 7), 229}, // 802 Switch
  {IR(50766, 24), 836}, // 803 TaskSequenceAsyncALTERA
  {IR(50790, 23), 836}, // 804 TaskSequenceAsyncINTEL
  {IR(50717, 25), 835}, // 805 TaskSequenceCreateALTERA
  {IR(50742, 24), 835}, // 806 TaskSequenceCreateINTEL
  {IR(50813, 22), 837}, // 807 TaskSequenceGetALTERA
  {IR(50835, 21), 837}, // 808 TaskSequenceGetINTEL
  {IR(50856, 26), 838}, // 809 TaskSequenceReleaseALTERA
  {IR(50882, 25), 838}, // 810 TaskSequenceReleaseINTEL
  {IR(41122, 27), 567}, // 811 TensorLayoutSetBlockSizeNV
  {IR(40986, 28), 560}, // 812 TensorLayoutSetClampValueNV
  {IR(40915, 27), 557}, // 813 TensorLayoutSetDimensionNV
  {IR(40942, 24), 558}, // 814 TensorLayoutSetStrideNV
  {IR(40966, 20), 559}, // 815 TensorLayoutSliceNV
  {IR(35580, 19), 350}, // 816 TensorQuerySizeARM
  {IR(35551, 14), 348}, // 817 TensorReadARM
  {IR(41102, 20), 566}, // 818 TensorViewSetClipNV
  {IR(41033, 25), 562}, // 819 TensorViewSetDimensionNV
  {IR(41058, 22), 563}, // 820 TensorViewSetStrideNV
  {IR(35565, 15), 349}, // 821 TensorWriteARM
  {IR(35692, 20), 359}, // 822 TerminateInvocation
  {IR(36153, 16), 382}, // 823 TerminateRayKHR
  {IR(40016, 15), 528}, // 824 TerminateRayNV
  {IR(40039, 14), 530}, // 825 TraceMotionNV
  {IR(40031, 8), 529}, // 826 TraceNV
  {IR(36065, 12), 378}, // 827 TraceRayKHR
  {IR(40053, 17), 531}, // 828 TraceRayMotionNV
  {IR(31014, 10), 76}, // 829 Transpose
  {IR(40120, 29), 533}, // 830 TypeAccelerationStructureKHR
  {IR(40149, 28), 533}, // 831 TypeAccelerationStructureNV
  {IR(29963, 10), 25}, // 832 TypeArray
  {IR(42764, 37), 636}, // 833 TypeAvcImeDualReferenceStreaminINTEL
  {IR(42499, 23), 627}, // 834 TypeAvcImePayloadINTEL
  {IR(42681, 44), 634}, // 835 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(42613, 22), 632}, // 836 TypeAvcImeResultINTEL
  {IR(42635, 46), 633}, // 837 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(42725, 39), 635}, // 838 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(42568, 23), 630}, // 839 TypeAvcMcePayloadINTEL
  {IR(42591, 22), 631}, // 840 TypeAvcMceResultINTEL
  {IR(42522, 23), 628}, // 841 TypeAvcRefPayloadINTEL
  {IR(42801, 22), 637}, // 842 TypeAvcRefResultINTEL
  {IR(42545, 23), 629}, // 843 TypeAvcSicPayloadINTEL
  {IR(42823, 22), 638}, // 844 TypeAvcSicResultINTEL
  {IR(29725, 9), 17}, // 845 TypeBool
  {IR(37583, 14), 438}, // 846 TypeBufferEXT
  {IR(50432, 23), 825}, // 847 TypeBufferSurfaceINTEL
  {IR(36334, 25), 389}, // 848 TypeCooperativeMatrixKHR
  {IR(40442, 24), 543}, // 849 TypeCooperativeMatrixNV
  {IR(38636, 24), 482}, // 850 TypeCooperativeVectorNV
  {IR(30078, 16), 32}, // 851 TypeDeviceEvent
  {IR(30068, 10), 31}, // 852 TypeEvent
  {IR(29779, 10), 19}, // 853 TypeFloat
  {IR(30127, 19), 36}, // 854 TypeForwardPointer
  {IR(30055, 13), 30}, // 855 TypeFunction
  {IR(35679, 13), 357}, // 856 TypeGraphARM
  {IR(39377, 17), 505}, // 857 TypeHitObjectEXT
  {IR(38581, 16), 480}, // 858 TypeHitObjectNV
  {IR(29924, 10), 22}, // 859 TypeImage
  {IR(29734, 8), 18}, // 860 TypeInt
  {IR(29800, 11), 21}, // 861 TypeMatrix
  {IR(34509, 17), 300}, // 862 TypeNamedBarrier
  {IR(37385, 25), 430}, // 863 TypeNodePayloadArrayAMDX
  {IR(30001, 11), 28}, // 864 TypeOpaque
  {IR(30118, 9), 35}, // 865 TypePipe
  {IR(34387, 16), 295}, // 866 TypePipeStorage
  {IR(30043, 12), 29}, // 867 TypePointer
  {IR(30108, 10), 34}, // 868 TypeQueue
  {IR(36613, 16), 397}, // 869 TypeRayQueryKHR
  {IR(30094, 14), 33}, // 870 TypeReserveId
  {IR(29973, 17), 26}, // 871 TypeRuntimeArray
  {IR(29946, 17), 24}, // 872 TypeSampledImage
  {IR(29934, 12), 23}, // 873 TypeSampler
  {IR(29990, 11), 27}, // 874 TypeStruct
  {IR(50455, 25), 826}, // 875 TypeStructContinuedINTEL
  {IR(50907, 23), 839}, // 876 TypeTaskSequenceALTERA
  {IR(50930, 22), 839}, // 877 TypeTaskSequenceINTEL
  {IR(35495, 14), 347}, // 878 TypeTensorARM
  {IR(40858, 19), 554}, // 879 TypeTensorLayoutNV
  {IR(40877, 17), 555}, // 880 TypeTensorViewNV
  {IR(35712, 22), 360}, // 881 TypeUntypedPointerKHR
  {IR(29789, 11), 20}, // 882 TypeVector
  {IR(38620, 16), 482}, // 883 TypeVectorIdEXT
  {IR(42481, 18), 626}, // 884 TypeVmeImageINTEL
  {IR(29716, 9), 16}, // 885 TypeVoid
  {IR(42110, 13), 605}, // 886 UAddSatINTEL
  {IR(42137, 14), 607}, // 887 UAverageINTEL
  {IR(42172, 21), 609}, // 888 UAverageRoundedINTEL
  {IR(31531, 9), 103}, // 889 UConvert
  {IR(42022, 24), 600}, // 890 UCountLeadingZerosINTEL
  {IR(42046, 25), 601}, // 891 UCountTrailingZerosINTEL
  {IR(31743, 5), 123}, // 892 UDiv
  {IR(36229, 5), 384}, // 893 UDot
  {IR(36282, 11), 387}, // 894 UDotAccSat
  {IR(36293, 14), 387}, // 895 UDotAccSatKHR
  {IR(36234, 8), 384}, // 896 UDotKHR
  {IR(32103, 13), 160}, // 897 UGreaterThan
  {IR(32129, 18), 162}, // 898 UGreaterThanEqual
  {IR(32165, 10), 164}, // 899 ULessThan
  {IR(32185, 15), 166}, // 900 ULessThanEqual
  {IR(31758, 5), 126}, // 901 UMod
  {IR(42234, 15), 613}, // 902 UMul32x16INTEL
  {IR(31911, 13), 140}, // 903 UMulExtended
  {IR(42206, 13), 611}, // 904 USubSatINTEL
  {IR(29228, 6), 1}, // 905 Undef
  {IR(32008, 10), 151}, // 906 Unordered
  {IR(33193, 12), 233}, // 907 Unreachable
  {IR(35753, 22), 362}, // 908 UntypedAccessChainKHR
  {IR(35890, 22), 368}, // 909 UntypedArrayLengthKHR
  {IR(36040, 25), 377}, // 910 UntypedGroupAsyncCopyKHR
  {IR(37614, 28), 441}, // 911 UntypedImageTexelPointerEXT
  {IR(35775, 30), 363}, // 912 UntypedInBoundsAccessChainKHR
  {IR(35857, 33), 367}, // 913 UntypedInBoundsPtrAccessChainKHR
  {IR(35912, 19), 369}, // 914 UntypedPrefetchKHR
  {IR(35832, 25), 366}, // 915 UntypedPtrAccessChainKHR
  {IR(35734, 19), 361}, // 916 UntypedVariableKHR
  {IR(16099, 32), 848}, // 917 UntypedVariableLengthArrayINTEL
  {IR(30539, 9), 52}, // 918 Variable
  {IR(14087, 25), 743}, // 919 VariableLengthArrayINTEL
  {IR(30896, 21), 69}, // 920 VectorExtractDynamic
  {IR(30917, 20), 70}, // 921 VectorInsertDynamic
  {IR(30937, 14), 71}, // 922 VectorShuffle
  {IR(31819, 18), 133}, // 923 VectorTimesMatrix
  {IR(31783, 18), 131}, // 924 VectorTimesScalar
  {IR(42467, 14), 625}, // 925 VmeImageINTEL
  {IR(38919, 33), 491}, // 926 WritePackedPrimitiveIndices4x8NV
  {IR(33418, 10), 250}, // 927 WritePipe
  {IR(49720, 24), 805}, // 928 WritePipeBlockingALTERA
  {IR(49744, 23), 805}, // 929 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 500> kExtInstByValue{{
    {0,IR(811, 2),IR(51816, 5),IR(1, 0)}, // CALL in arm_experimental_ml_operations
    {0,IR(765, 9),IR(51821, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(765, 9),IR(51829, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(51842, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(51850, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(51864, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(813, 3),IR(51937, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(816, 3),IR(51986, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(819, 2),IR(52041, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(52060, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(52075, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(821, 6),IR(52091, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(52104, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(827, 9),IR(52122, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(836, 9),IR(52174, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(845, 10),IR(52193, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(855, 5),IR(52209, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(52230, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(52251, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(860, 6),IR(52269, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(866, 5),IR(52296, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(871, 5),IR(52331, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(845, 10),IR(52362, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(876, 8),IR(52382, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(884, 11),IR(52407, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(895, 5),IR(52421, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(900, 3),IR(52439, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(903, 2),IR(52470, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(52481, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(905, 3),IR(52494, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(908, 7),IR(52551, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(52570, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(52591, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(52604, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(915, 2),IR(52648, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(747, 1),IR(52663, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(917, 4),IR(52679, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(900, 3),IR(52693, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(52709, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(52715, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(52725, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(52731, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(52736, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(52741, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(52747, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(52753, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(52759, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(52764, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(52770, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(52778, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(52786, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(52790, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(52794, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(52798, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(52803, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(52808, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(52813, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(52818, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(52823, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(52828, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(52834, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(52840, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(52846, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(52852, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(52856, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(52860, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(52864, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(52869, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(52874, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(52879, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(52891, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(52903, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(52917, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(52922, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(52933, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(52938, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(52943, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(52948, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(52953, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(52958, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(52963, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(52970, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(52977, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(52984, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(52989, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(52994, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(52999, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(53010, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(53014, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(53020, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(53032, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(53038, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(53051, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(53064, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(53078, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(53092, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(53105, 15),IR(340, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(53120, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(53136, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(53152, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(53167, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(53182, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(53197, 17),IR(340, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(53214, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(53221, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(53230, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(53236, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(53246, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(53258, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(53266, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(53274, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(53283, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(53292, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(53301, 22),IR(341, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(53323, 20),IR(341, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(53343, 20),IR(341, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(53363, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(53368, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(53373, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5827, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(921, 5),IR(53380, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(593, 5),IR(53393, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(593, 5),IR(53415, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(557, 7),IR(53431, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(557, 7),IR(53456, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(593, 5),IR(53475, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(593, 5),IR(53499, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(593, 5),IR(53520, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(593, 5),IR(53541, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(593, 5),IR(53557, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(53575, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(53601, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(53626, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(53646, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(53671, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(53701, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(53724, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(53749, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(53775, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(53805, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(53831, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(9301, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(53851, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(53881, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(593, 5),IR(53909, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(557, 7),IR(53937, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(53960, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(53995, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(54033, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(54075, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(535, 6),IR(54120, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(535, 6),IR(54157, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(593, 5),IR(54197, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(593, 5),IR(54224, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(54251, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(54283, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(54323, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(54334, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(54360, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(54392, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(54426, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {1,IR(36, 2),IR(54448, 11),IR(1, 0)}, // DebugGraph in nonsemantic_graph_debuginfo
    {2,IR(375, 3),IR(52648, 15),IR(1, 0)}, // DebugOperation in nonsemantic_graph_debuginfo
    {3,IR(926, 3),IR(54459, 12),IR(1, 0)}, // DebugTensor in nonsemantic_graph_debuginfo
    {0,IR(0, 0),IR(51850, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {1,IR(296, 4),IR(51864, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {2,IR(593, 5),IR(51937, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {3,IR(33, 3),IR(51986, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {4,IR(36, 2),IR(52041, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {5,IR(168, 2),IR(52060, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {6,IR(36, 2),IR(52075, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {7,IR(535, 6),IR(52091, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo
    {8,IR(375, 3),IR(52104, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {9,IR(929, 9),IR(52122, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {10,IR(938, 10),IR(52174, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {11,IR(948, 9),IR(52193, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {12,IR(296, 4),IR(52209, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {13,IR(36, 2),IR(52230, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {14,IR(168, 2),IR(52251, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {15,IR(535, 6),IR(52269, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {16,IR(675, 5),IR(52296, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {17,IR(957, 5),IR(52331, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {18,IR(962, 10),IR(52362, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {19,IR(471, 8),IR(52382, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {20,IR(962, 10),IR(52407, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo
    {21,IR(593, 5),IR(52421, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {22,IR(33, 3),IR(52439, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {23,IR(903, 2),IR(52470, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo
    {24,IR(0, 0),IR(52481, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo
    {25,IR(926, 3),IR(52494, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {26,IR(972, 8),IR(52551, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {27,IR(36, 2),IR(52570, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {28,IR(980, 4),IR(52591, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo
    {29,IR(980, 4),IR(52604, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo
    {30,IR(168, 2),IR(52648, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo
    {31,IR(747, 1),IR(52663, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo
    {32,IR(580, 4),IR(52679, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {33,IR(33, 3),IR(52693, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {34,IR(516, 7),IR(54471, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {35,IR(903, 2),IR(54491, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo
    {101,IR(36, 2),IR(54503, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {102,IR(10, 1),IR(54527, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {103,IR(675, 5),IR(54548, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo
    {104,IR(0, 0),IR(54558, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo
    {105,IR(36, 2),IR(54570, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {106,IR(10, 1),IR(54591, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {107,IR(296, 4),IR(54608, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {108,IR(33, 3),IR(54624, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {109,IR(36, 2),IR(54640, 21),IR(1, 0)}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {110,IR(675, 5),IR(54661, 30),IR(1, 0)}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {1,IR(765, 9),IR(54691, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(54705, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(54718, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(54730, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(54744, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(984, 15),IR(54767, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(999, 33),IR(54787, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1032, 19),IR(54806, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(51850, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1051, 4),IR(51864, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1055, 3),IR(51937, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1058, 3),IR(51986, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1061, 2),IR(52041, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(52060, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(52075, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(821, 6),IR(52091, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1063, 3),IR(52104, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1066, 9),IR(52122, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1075, 10),IR(52174, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1085, 10),IR(52193, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1095, 5),IR(52209, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(52230, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(52251, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(860, 6),IR(52269, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(866, 5),IR(52296, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(871, 5),IR(52331, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1085, 10),IR(52362, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1100, 8),IR(52382, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1108, 11),IR(52407, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(895, 5),IR(52421, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(900, 3),IR(52439, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(903, 2),IR(52470, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(52481, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(905, 3),IR(52494, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1119, 8),IR(52551, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(52570, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(52591, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(980, 4),IR(52604, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1127, 2),IR(52648, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(747, 1),IR(52663, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(917, 4),IR(52679, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(900, 3),IR(52693, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1129, 7),IR(54471, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(903, 2),IR(54491, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1136, 8),IR(55127, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(55144, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(55149, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(55155, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(55162, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(55167, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(55173, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(55180, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(55185, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(55191, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(55197, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(55204, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(55212, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(55217, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(55222, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(55231, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(55235, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(55240, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(55246, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(55251, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(55255, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(55259, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(55264, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(55270, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(55276, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(55281, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(55286, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(55292, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(55296, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(55301, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(55306, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(55311, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(55317, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(55323, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(55329, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(55335, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(55341, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(55348, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(55357, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(55361, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(55366, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(55372, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(55378, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(55383, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(55387, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(55394, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(55401, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(55406, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(55410, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(55420, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(55424, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(55429, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(55434, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(55444, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(55451, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(55456, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(55462, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(55468, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(55474, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(55478, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(55485, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(55490, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(55496, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(55501, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(55505, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(55510, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(55516, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(55523, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(55529, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(55538, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(55550, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(55559, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(55569, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(55580, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(55589, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(55599, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(55610, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(55620, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(55631, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(55642, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(55651, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(55661, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(55670, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(55681, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(55695, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(55706, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(55718, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(55731, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(55742, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(55754, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(55767, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(55779, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(55792, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(55805, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(55816, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(55828, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(55839, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(55846, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(55854, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(55866, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(55878, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(55882, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(55890, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(55895, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(55906, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(55911, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(55917, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(55926, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(55933, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(55943, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(55957, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(55969, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(55984, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(55990, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(56001, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(56011, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(56021, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(56028, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(56035, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(56043, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(56051, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(56059, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(56067, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(56071, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(56075, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(56084, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(56094, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(56104, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(56110, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(56116, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(56122, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(56128, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(56137, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(56144, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(56154, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(56164, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(56175, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(56186, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(56195, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(56203, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(56211, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(56219, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1144, 3),IR(56227, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(56234, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(56242, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1144, 3),IR(56253, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(56265, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1147, 4),IR(56277, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(56291, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1147, 4),IR(56304, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1144, 3),IR(56319, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(56332, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1147, 4),IR(56346, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(56362, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(56370, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(56379, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(56386, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(56395, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(56405, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(56412, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(56418, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(56429, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(56438, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(56447, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(56464, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(56481, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(56489, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(56511, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(56539, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(56558, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(56567, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(56590, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(56599, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(56608, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(56617, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(56626, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(56635, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(56644, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(56653, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(56662, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(56671, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(516, 7),IR(56678, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(755, 10),IR(56689, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(755, 10),IR(56696, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(755, 10),IR(56703, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(56720, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(56726, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(675, 5),IR(56733, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(56744, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(765, 9),IR(56751, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(56768, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(56774, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(56778, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(56786, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(56791, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(56795, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(56818, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(56830, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(56841, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(56853, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(56860, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(56872, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(56891, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(56911, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(56922, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(56934, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(56942, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(56950, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(56954, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(56958, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(56962, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(56968, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(56972, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(56984, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(56989, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(56993, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(56997, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(57001, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(57007, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(57011, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(57023, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(57030, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(57041, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(57047, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(57051, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(57058, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(57064, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(57072, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(57086, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(57097, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(57108, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(57119, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(57130, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(57145, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(57156, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(57163, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(57167, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(57175, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(57183, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(57189, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(57194, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(57204, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(57211, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(675, 5),IR(57219, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(57226, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(755, 10),IR(57231, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    {IR(51816, 5), 0}, // CALL in arm_experimental_ml_operations
    {IR(51821, 8), 1}, // MIN_SAD in arm_motion_engine_100
    {IR(51829, 13), 2}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(51842, 8), 3}, // RAW_SAD in arm_motion_engine_100
    {IR(51864, 21), 5}, // DebugCompilationUnit in debuginfo
    {IR(52591, 13), 32}, // DebugDeclare in debuginfo
    {IR(52663, 16), 35}, // DebugExpression in debuginfo
    {IR(52407, 14), 24}, // DebugFunction in debuginfo
    {IR(52382, 25), 23}, // DebugFunctionDeclaration in debuginfo
    {IR(52362, 20), 22}, // DebugGlobalVariable in debuginfo
    {IR(51850, 14), 4}, // DebugInfoNone in debuginfo
    {IR(52494, 15), 29}, // DebugInlinedAt in debuginfo
    {IR(52570, 21), 31}, // DebugInlinedVariable in debuginfo
    {IR(52421, 18), 25}, // DebugLexicalBlock in debuginfo
    {IR(52439, 31), 26}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(52551, 19), 30}, // DebugLocalVariable in debuginfo
    {IR(52679, 14), 36}, // DebugMacroDef in debuginfo
    {IR(52693, 16), 37}, // DebugMacroUndef in debuginfo
    {IR(52481, 13), 28}, // DebugNoScope in debuginfo
    {IR(52648, 15), 34}, // DebugOperation in debuginfo
    {IR(52470, 11), 27}, // DebugScope in debuginfo
    {IR(52060, 15), 9}, // DebugTypeArray in debuginfo
    {IR(51937, 15), 6}, // DebugTypeBasic in debuginfo
    {IR(52174, 19), 14}, // DebugTypeComposite in debuginfo
    {IR(52122, 14), 13}, // DebugTypeEnum in debuginfo
    {IR(52104, 18), 12}, // DebugTypeFunction in debuginfo
    {IR(52209, 21), 16}, // DebugTypeInheritance in debuginfo
    {IR(52193, 16), 15}, // DebugTypeMember in debuginfo
    {IR(51986, 17), 7}, // DebugTypePointer in debuginfo
    {IR(52230, 21), 17}, // DebugTypePtrToMember in debuginfo
    {IR(52041, 19), 8}, // DebugTypeQualifier in debuginfo
    {IR(52251, 18), 18}, // DebugTypeTemplate in debuginfo
    {IR(52269, 27), 19}, // DebugTypeTemplateParameter in debuginfo
    {IR(52331, 31), 21}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(52296, 35), 20}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(52075, 16), 10}, // DebugTypeVector in debuginfo
    {IR(52091, 13), 11}, // DebugTypedef in debuginfo
    {IR(52604, 11), 33}, // DebugValue in debuginfo
    {IR(52803, 5), 54}, // Acos in glsl_std_450
    {IR(52834, 6), 60}, // Acosh in glsl_std_450
    {IR(52798, 5), 53}, // Asin in glsl_std_450
    {IR(52828, 6), 59}, // Asinh in glsl_std_450
    {IR(52808, 5), 55}, // Atan in glsl_std_450
    {IR(52846, 6), 62}, // Atan2 in glsl_std_450
    {IR(52840, 6), 61}, // Atanh in glsl_std_450
    {IR(52759, 5), 46}, // Ceil in glsl_std_450
    {IR(52790, 4), 51}, // Cos in glsl_std_450
    {IR(52818, 5), 57}, // Cosh in glsl_std_450
    {IR(53230, 6), 105}, // Cross in glsl_std_450
    {IR(52778, 8), 49}, // Degrees in glsl_std_450
    {IR(52891, 12), 70}, // Determinant in glsl_std_450
    {IR(53221, 9), 104}, // Distance in glsl_std_450
    {IR(52856, 4), 64}, // Exp in glsl_std_450
    {IR(52864, 5), 66}, // Exp2 in glsl_std_450
    {IR(52731, 5), 41}, // FAbs in glsl_std_450
    {IR(52963, 7), 80}, // FClamp in glsl_std_450
    {IR(52948, 5), 77}, // FMax in glsl_std_450
    {IR(52933, 5), 74}, // FMin in glsl_std_450
    {IR(52984, 5), 83}, // FMix in glsl_std_450
    {IR(52741, 6), 43}, // FSign in glsl_std_450
    {IR(53246, 12), 107}, // FaceForward in glsl_std_450
    {IR(53274, 9), 110}, // FindILsb in glsl_std_450
    {IR(53283, 9), 111}, // FindSMsb in glsl_std_450
    {IR(53292, 9), 112}, // FindUMsb in glsl_std_450
    {IR(52753, 6), 45}, // Floor in glsl_std_450
    {IR(53010, 4), 87}, // Fma in glsl_std_450
    {IR(52764, 6), 47}, // Fract in glsl_std_450
    {IR(53014, 6), 88}, // Frexp in glsl_std_450
    {IR(53020, 12), 89}, // FrexpStruct in glsl_std_450
    {IR(52989, 5), 84}, // IMix in glsl_std_450
    {IR(53301, 22), 113}, // InterpolateAtCentroid in glsl_std_450
    {IR(53343, 20), 115}, // InterpolateAtOffset in glsl_std_450
    {IR(53323, 20), 114}, // InterpolateAtSample in glsl_std_450
    {IR(52879, 12), 69}, // InverseSqrt in glsl_std_450
    {IR(53032, 6), 90}, // Ldexp in glsl_std_450
    {IR(53214, 7), 103}, // Length in glsl_std_450
    {IR(52860, 4), 65}, // Log in glsl_std_450
    {IR(52869, 5), 67}, // Log2 in glsl_std_450
    {IR(52903, 14), 71}, // MatrixInverse in glsl_std_450
    {IR(52917, 5), 72}, // Modf in glsl_std_450
    {IR(52922, 11), 73}, // ModfStruct in glsl_std_450
    {IR(53373, 7), 118}, // NClamp in glsl_std_450
    {IR(53368, 5), 117}, // NMax in glsl_std_450
    {IR(53363, 5), 116}, // NMin in glsl_std_450
    {IR(53236, 10), 106}, // Normalize in glsl_std_450
    {IR(53105, 15), 96}, // PackDouble2x32 in glsl_std_450
    {IR(53092, 13), 95}, // PackHalf2x16 in glsl_std_450
    {IR(53064, 14), 93}, // PackSnorm2x16 in glsl_std_450
    {IR(53038, 13), 91}, // PackSnorm4x8 in glsl_std_450
    {IR(53078, 14), 94}, // PackUnorm2x16 in glsl_std_450
    {IR(53051, 13), 92}, // PackUnorm4x8 in glsl_std_450
    {IR(52852, 4), 63}, // Pow in glsl_std_450
    {IR(52770, 8), 48}, // Radians in glsl_std_450
    {IR(53258, 8), 108}, // Reflect in glsl_std_450
    {IR(53266, 8), 109}, // Refract in glsl_std_450
    {IR(52709, 6), 38}, // Round in glsl_std_450
    {IR(52715, 10), 39}, // RoundEven in glsl_std_450
    {IR(52736, 5), 42}, // SAbs in glsl_std_450
    {IR(52977, 7), 82}, // SClamp in glsl_std_450
    {IR(52958, 5), 79}, // SMax in glsl_std_450
    {IR(52943, 5), 76}, // SMin in glsl_std_450
    {IR(52747, 6), 44}, // SSign in glsl_std_450
    {IR(52786, 4), 50}, // Sin in glsl_std_450
    {IR(52813, 5), 56}, // Sinh in glsl_std_450
    {IR(52999, 11), 86}, // SmoothStep in glsl_std_450
    {IR(52874, 5), 68}, // Sqrt in glsl_std_450
    {IR(52994, 5), 85}, // Step in glsl_std_450
    {IR(52794, 4), 52}, // Tan in glsl_std_450
    {IR(52823, 5), 58}, // Tanh in glsl_std_450
    {IR(52725, 6), 40}, // Trunc in glsl_std_450
    {IR(52970, 7), 81}, // UClamp in glsl_std_450
    {IR(52953, 5), 78}, // UMax in glsl_std_450
    {IR(52938, 5), 75}, // UMin in glsl_std_450
    {IR(53197, 17), 102}, // UnpackDouble2x32 in glsl_std_450
    {IR(53152, 15), 99}, // UnpackHalf2x16 in glsl_std_450
    {IR(53120, 16), 97}, // UnpackSnorm2x16 in glsl_std_450
    {IR(53167, 15), 100}, // UnpackSnorm4x8 in glsl_std_450
    {IR(53136, 16), 98}, // UnpackUnorm2x16 in glsl_std_450
    {IR(53182, 15), 101}, // UnpackUnorm4x8 in glsl_std_450
    {IR(53380, 13), 120}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(53475, 24), 125}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(53431, 25), 123}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(53456, 19), 124}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(53909, 28), 144}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(53937, 23), 145}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(53499, 21), 126}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(53541, 16), 128}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(53393, 22), 121}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(53520, 21), 127}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(54197, 27), 152}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(53415, 16), 122}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(54224, 27), 153}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(53557, 18), 129}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(54251, 32), 154}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(53805, 26), 139}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(53831, 20), 140}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(54075, 45), 149}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(54157, 40), 151}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(54033, 42), 148}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(54120, 37), 150}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5827, 7), 119}, // Kernel in nonsemantic_clspvreflection
    {IR(9301, 15), 141}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(54392, 34), 159}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(54360, 32), 158}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(54334, 26), 157}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(54323, 11), 156}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(54283, 40), 155}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(53995, 38), 147}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(53960, 35), 146}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(53851, 30), 142}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(53671, 30), 134}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(53646, 25), 133}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(53701, 23), 135}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(53749, 26), 137}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(53775, 30), 138}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(53724, 25), 136}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(53601, 25), 131}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(53881, 28), 143}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(53626, 20), 132}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(53575, 26), 130}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(54426, 22), 160}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(54448, 11), 161}, // DebugGraph in nonsemantic_graph_debuginfo
    {IR(52648, 15), 162}, // DebugOperation in nonsemantic_graph_debuginfo
    {IR(54459, 12), 163}, // DebugTensor in nonsemantic_graph_debuginfo
    {IR(54570, 21), 204}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {IR(51864, 21), 165}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {IR(52591, 13), 192}, // DebugDeclare in nonsemantic_shader_debuginfo
    {IR(54608, 16), 206}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {IR(52663, 16), 195}, // DebugExpression in nonsemantic_shader_debuginfo
    {IR(52407, 14), 184}, // DebugFunction in nonsemantic_shader_debuginfo
    {IR(52382, 25), 183}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {IR(54503, 24), 200}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {IR(52362, 20), 182}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {IR(54471, 20), 198}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {IR(51850, 14), 164}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {IR(52494, 15), 189}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {IR(52570, 21), 191}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {IR(52421, 18), 185}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {IR(52439, 31), 186}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {IR(54548, 10), 202}, // DebugLine in nonsemantic_shader_debuginfo
    {IR(52551, 19), 190}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {IR(52679, 14), 196}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {IR(52693, 16), 197}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {IR(54558, 12), 203}, // DebugNoLine in nonsemantic_shader_debuginfo
    {IR(52481, 13), 188}, // DebugNoScope in nonsemantic_shader_debuginfo
    {IR(52648, 15), 194}, // DebugOperation in nonsemantic_shader_debuginfo
    {IR(52470, 11), 187}, // DebugScope in nonsemantic_shader_debuginfo
    {IR(54491, 12), 199}, // DebugSource in nonsemantic_shader_debuginfo
    {IR(54527, 21), 201}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {IR(54591, 17), 205}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {IR(52060, 15), 169}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {IR(51937, 15), 166}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {IR(52174, 19), 174}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {IR(54661, 30), 209}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {IR(52122, 14), 173}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {IR(52104, 18), 172}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {IR(52209, 21), 176}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {IR(54624, 16), 207}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {IR(52193, 16), 175}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {IR(51986, 17), 167}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {IR(52230, 21), 177}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {IR(52041, 19), 168}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {IR(52251, 18), 178}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {IR(52269, 27), 179}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52331, 31), 181}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {IR(52296, 35), 180}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52075, 16), 170}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {IR(54640, 21), 208}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {IR(52091, 13), 171}, // DebugTypedef in nonsemantic_shader_debuginfo
    {IR(52604, 11), 193}, // DebugValue in nonsemantic_shader_debuginfo
    {IR(54691, 14), 210}, // Configuration in nonsemantic_vkspreflection
    {IR(54767, 20), 215}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(54787, 19), 216}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(54806, 21), 217}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(54730, 14), 213}, // PushConstants in nonsemantic_vkspreflection
    {IR(54744, 23), 214}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(54705, 13), 211}, // StartCounter in nonsemantic_vkspreflection
    {IR(54718, 12), 212}, // StopCounter in nonsemantic_vkspreflection
    {IR(51864, 21), 219}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(52591, 13), 246}, // DebugDeclare in opencl_debuginfo_100
    {IR(52663, 16), 249}, // DebugExpression in opencl_debuginfo_100
    {IR(52407, 14), 238}, // DebugFunction in opencl_debuginfo_100
    {IR(52382, 25), 237}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(52362, 20), 236}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(54471, 20), 252}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(51850, 14), 218}, // DebugInfoNone in opencl_debuginfo_100
    {IR(52494, 15), 243}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(52570, 21), 245}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(52421, 18), 239}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(52439, 31), 240}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(52551, 19), 244}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(52679, 14), 250}, // DebugMacroDef in opencl_debuginfo_100
    {IR(52693, 16), 251}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(55127, 17), 254}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(52481, 13), 242}, // DebugNoScope in opencl_debuginfo_100
    {IR(52648, 15), 248}, // DebugOperation in opencl_debuginfo_100
    {IR(52470, 11), 241}, // DebugScope in opencl_debuginfo_100
    {IR(54491, 12), 253}, // DebugSource in opencl_debuginfo_100
    {IR(52060, 15), 223}, // DebugTypeArray in opencl_debuginfo_100
    {IR(51937, 15), 220}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(52174, 19), 228}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(52122, 14), 227}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(52104, 18), 226}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(52209, 21), 230}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(52193, 16), 229}, // DebugTypeMember in opencl_debuginfo_100
    {IR(51986, 17), 221}, // DebugTypePointer in opencl_debuginfo_100
    {IR(52230, 21), 231}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(52041, 19), 222}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(52251, 18), 232}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(52269, 27), 233}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(52331, 31), 235}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(52296, 35), 234}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(52075, 16), 224}, // DebugTypeVector in opencl_debuginfo_100
    {IR(52091, 13), 225}, // DebugTypedef in opencl_debuginfo_100
    {IR(52604, 11), 247}, // DebugValue in opencl_debuginfo_100
    {IR(55144, 5), 255}, // acos in opencl_std_100
    {IR(55149, 6), 256}, // acosh in opencl_std_100
    {IR(55155, 7), 257}, // acospi in opencl_std_100
    {IR(55162, 5), 258}, // asin in opencl_std_100
    {IR(55167, 6), 259}, // asinh in opencl_std_100
    {IR(55173, 7), 260}, // asinpi in opencl_std_100
    {IR(55180, 5), 261}, // atan in opencl_std_100
    {IR(55185, 6), 262}, // atan2 in opencl_std_100
    {IR(55204, 8), 265}, // atan2pi in opencl_std_100
    {IR(55191, 6), 263}, // atanh in opencl_std_100
    {IR(55197, 7), 264}, // atanpi in opencl_std_100
    {IR(56395, 10), 411}, // bitselect in opencl_std_100
    {IR(55212, 5), 266}, // cbrt in opencl_std_100
    {IR(55217, 5), 267}, // ceil in opencl_std_100
    {IR(56067, 4), 376}, // clz in opencl_std_100
    {IR(55222, 9), 268}, // copysign in opencl_std_100
    {IR(55231, 4), 269}, // cos in opencl_std_100
    {IR(55235, 5), 270}, // cosh in opencl_std_100
    {IR(55240, 6), 271}, // cospi in opencl_std_100
    {IR(55911, 6), 359}, // cross in opencl_std_100
    {IR(56071, 4), 377}, // ctz in opencl_std_100
    {IR(55846, 8), 351}, // degrees in opencl_std_100
    {IR(55917, 9), 360}, // distance in opencl_std_100
    {IR(55251, 4), 273}, // erf in opencl_std_100
    {IR(55246, 5), 272}, // erfc in opencl_std_100
    {IR(55255, 4), 274}, // exp in opencl_std_100
    {IR(55264, 6), 276}, // exp10 in opencl_std_100
    {IR(55259, 5), 275}, // exp2 in opencl_std_100
    {IR(55270, 6), 277}, // expm1 in opencl_std_100
    {IR(55276, 5), 278}, // fabs in opencl_std_100
    {IR(55943, 14), 363}, // fast_distance in opencl_std_100
    {IR(55957, 12), 364}, // fast_length in opencl_std_100
    {IR(55969, 15), 365}, // fast_normalize in opencl_std_100
    {IR(55839, 7), 350}, // fclamp in opencl_std_100
    {IR(55281, 5), 279}, // fdim in opencl_std_100
    {IR(55286, 6), 280}, // floor in opencl_std_100
    {IR(55292, 4), 281}, // fma in opencl_std_100
    {IR(55296, 5), 282}, // fmax in opencl_std_100
    {IR(55854, 12), 352}, // fmax_common in opencl_std_100
    {IR(55301, 5), 283}, // fmin in opencl_std_100
    {IR(55866, 12), 353}, // fmin_common in opencl_std_100
    {IR(55306, 5), 284}, // fmod in opencl_std_100
    {IR(55311, 6), 285}, // fract in opencl_std_100
    {IR(55317, 6), 286}, // frexp in opencl_std_100
    {IR(55529, 9), 322}, // half_cos in opencl_std_100
    {IR(55538, 12), 323}, // half_divide in opencl_std_100
    {IR(55550, 9), 324}, // half_exp in opencl_std_100
    {IR(55569, 11), 326}, // half_exp10 in opencl_std_100
    {IR(55559, 10), 325}, // half_exp2 in opencl_std_100
    {IR(55580, 9), 327}, // half_log in opencl_std_100
    {IR(55599, 11), 329}, // half_log10 in opencl_std_100
    {IR(55589, 10), 328}, // half_log2 in opencl_std_100
    {IR(55610, 10), 330}, // half_powr in opencl_std_100
    {IR(55620, 11), 331}, // half_recip in opencl_std_100
    {IR(55631, 11), 332}, // half_rsqrt in opencl_std_100
    {IR(55642, 9), 333}, // half_sin in opencl_std_100
    {IR(55651, 10), 334}, // half_sqrt in opencl_std_100
    {IR(55661, 9), 335}, // half_tan in opencl_std_100
    {IR(55323, 6), 287}, // hypot in opencl_std_100
    {IR(55329, 6), 288}, // ilogb in opencl_std_100
    {IR(55335, 6), 289}, // ldexp in opencl_std_100
    {IR(55926, 7), 361}, // length in opencl_std_100
    {IR(55341, 7), 290}, // lgamma in opencl_std_100
    {IR(55348, 9), 291}, // lgamma_r in opencl_std_100
    {IR(55357, 4), 292}, // log in opencl_std_100
    {IR(55366, 6), 294}, // log10 in opencl_std_100
    {IR(55372, 6), 295}, // log1p in opencl_std_100
    {IR(55361, 5), 293}, // log2 in opencl_std_100
    {IR(55378, 5), 296}, // logb in opencl_std_100
    {IR(55383, 4), 297}, // mad in opencl_std_100
    {IR(55387, 7), 298}, // maxmag in opencl_std_100
    {IR(55394, 7), 299}, // minmag in opencl_std_100
    {IR(55878, 4), 354}, // mix in opencl_std_100
    {IR(55401, 5), 300}, // modf in opencl_std_100
    {IR(55406, 4), 301}, // nan in opencl_std_100
    {IR(55670, 11), 336}, // native_cos in opencl_std_100
    {IR(55681, 14), 337}, // native_divide in opencl_std_100
    {IR(55695, 11), 338}, // native_exp in opencl_std_100
    {IR(55718, 13), 340}, // native_exp10 in opencl_std_100
    {IR(55706, 12), 339}, // native_exp2 in opencl_std_100
    {IR(55731, 11), 341}, // native_log in opencl_std_100
    {IR(55754, 13), 343}, // native_log10 in opencl_std_100
    {IR(55742, 12), 342}, // native_log2 in opencl_std_100
    {IR(55767, 12), 344}, // native_powr in opencl_std_100
    {IR(55779, 13), 345}, // native_recip in opencl_std_100
    {IR(55792, 13), 346}, // native_rsqrt in opencl_std_100
    {IR(55805, 11), 347}, // native_sin in opencl_std_100
    {IR(55816, 12), 348}, // native_sqrt in opencl_std_100
    {IR(55828, 11), 349}, // native_tan in opencl_std_100
    {IR(55410, 10), 302}, // nextafter in opencl_std_100
    {IR(55933, 10), 362}, // normalize in opencl_std_100
    {IR(56186, 9), 391}, // popcount in opencl_std_100
    {IR(55420, 4), 303}, // pow in opencl_std_100
    {IR(55424, 5), 304}, // pown in opencl_std_100
    {IR(55429, 5), 305}, // powr in opencl_std_100
    {IR(56386, 9), 410}, // prefetch in opencl_std_100
    {IR(56379, 7), 409}, // printf in opencl_std_100
    {IR(55882, 8), 355}, // radians in opencl_std_100
    {IR(55434, 10), 306}, // remainder in opencl_std_100
    {IR(55444, 7), 307}, // remquo in opencl_std_100
    {IR(55451, 5), 308}, // rint in opencl_std_100
    {IR(55456, 6), 309}, // rootn in opencl_std_100
    {IR(56137, 7), 386}, // rotate in opencl_std_100
    {IR(55462, 6), 310}, // round in opencl_std_100
    {IR(55468, 6), 311}, // rsqrt in opencl_std_100
    {IR(55984, 6), 366}, // s_abs in opencl_std_100
    {IR(55990, 11), 367}, // s_abs_diff in opencl_std_100
    {IR(56001, 10), 368}, // s_add_sat in opencl_std_100
    {IR(56051, 8), 374}, // s_clamp in opencl_std_100
    {IR(56021, 7), 370}, // s_hadd in opencl_std_100
    {IR(56195, 8), 392}, // s_mad24 in opencl_std_100
    {IR(56075, 9), 378}, // s_mad_hi in opencl_std_100
    {IR(56094, 10), 380}, // s_mad_sat in opencl_std_100
    {IR(56104, 6), 381}, // s_max in opencl_std_100
    {IR(56116, 6), 383}, // s_min in opencl_std_100
    {IR(56211, 8), 394}, // s_mul24 in opencl_std_100
    {IR(56128, 9), 385}, // s_mul_hi in opencl_std_100
    {IR(56035, 8), 372}, // s_rhadd in opencl_std_100
    {IR(56144, 10), 387}, // s_sub_sat in opencl_std_100
    {IR(56175, 11), 390}, // s_upsample in opencl_std_100
    {IR(56405, 7), 412}, // select in opencl_std_100
    {IR(56362, 8), 407}, // shuffle in opencl_std_100
    {IR(56370, 9), 408}, // shuffle2 in opencl_std_100
    {IR(55906, 5), 358}, // sign in opencl_std_100
    {IR(55474, 4), 312}, // sin in opencl_std_100
    {IR(55478, 7), 313}, // sincos in opencl_std_100
    {IR(55485, 5), 314}, // sinh in opencl_std_100
    {IR(55490, 6), 315}, // sinpi in opencl_std_100
    {IR(55895, 11), 357}, // smoothstep in opencl_std_100
    {IR(55496, 5), 316}, // sqrt in opencl_std_100
    {IR(55890, 5), 356}, // step in opencl_std_100
    {IR(55501, 4), 317}, // tan in opencl_std_100
    {IR(55505, 5), 318}, // tanh in opencl_std_100
    {IR(55510, 6), 319}, // tanpi in opencl_std_100
    {IR(55516, 7), 320}, // tgamma in opencl_std_100
    {IR(55523, 6), 321}, // trunc in opencl_std_100
    {IR(56412, 6), 413}, // u_abs in opencl_std_100
    {IR(56418, 11), 414}, // u_abs_diff in opencl_std_100
    {IR(56011, 10), 369}, // u_add_sat in opencl_std_100
    {IR(56059, 8), 375}, // u_clamp in opencl_std_100
    {IR(56028, 7), 371}, // u_hadd in opencl_std_100
    {IR(56203, 8), 393}, // u_mad24 in opencl_std_100
    {IR(56438, 9), 416}, // u_mad_hi in opencl_std_100
    {IR(56084, 10), 379}, // u_mad_sat in opencl_std_100
    {IR(56110, 6), 382}, // u_max in opencl_std_100
    {IR(56122, 6), 384}, // u_min in opencl_std_100
    {IR(56219, 8), 395}, // u_mul24 in opencl_std_100
    {IR(56429, 9), 415}, // u_mul_hi in opencl_std_100
    {IR(56043, 8), 373}, // u_rhadd in opencl_std_100
    {IR(56154, 10), 388}, // u_sub_sat in opencl_std_100
    {IR(56164, 11), 389}, // u_upsample in opencl_std_100
    {IR(56242, 11), 398}, // vload_half in opencl_std_100
    {IR(56253, 12), 399}, // vload_halfn in opencl_std_100
    {IR(56319, 13), 404}, // vloada_halfn in opencl_std_100
    {IR(56227, 7), 396}, // vloadn in opencl_std_100
    {IR(56265, 12), 400}, // vstore_half in opencl_std_100
    {IR(56277, 14), 401}, // vstore_half_r in opencl_std_100
    {IR(56291, 13), 402}, // vstore_halfn in opencl_std_100
    {IR(56304, 15), 403}, // vstore_halfn_r in opencl_std_100
    {IR(56332, 14), 405}, // vstorea_halfn in opencl_std_100
    {IR(56346, 16), 406}, // vstorea_halfn_r in opencl_std_100
    {IR(56234, 8), 397}, // vstoren in opencl_std_100
    {IR(56464, 17), 418}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(56447, 17), 417}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(56481, 8), 419}, // TimeAMD in spv_amd_gcn_shader
    {IR(56558, 9), 423}, // MbcntAMD in spv_amd_shader_ballot
    {IR(56489, 22), 420}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(56511, 28), 421}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(56539, 19), 422}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(56567, 23), 424}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(56617, 9), 428}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56644, 9), 431}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56590, 9), 425}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56635, 9), 430}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56662, 9), 433}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56608, 9), 427}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56626, 9), 429}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(56653, 9), 432}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(56599, 9), 426}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(56968, 4), 465}, // ABS in tosa_001000_1
    {IR(56791, 4), 448}, // ADD in tosa_001000_1
    {IR(56671, 7), 434}, // ARGMAX in tosa_001000_1
    {IR(56795, 23), 449}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(56678, 11), 435}, // AVG_POOL2D in tosa_001000_1
    {IR(56818, 12), 450}, // BITWISE_AND in tosa_001000_1
    {IR(56972, 12), 466}, // BITWISE_NOT in tosa_001000_1
    {IR(56830, 11), 451}, // BITWISE_OR in tosa_001000_1
    {IR(56841, 12), 452}, // BITWISE_XOR in tosa_001000_1
    {IR(57226, 5), 498}, // CAST in tosa_001000_1
    {IR(56984, 5), 467}, // CEIL in tosa_001000_1
    {IR(56768, 6), 444}, // CLAMP in tosa_001000_1
    {IR(56989, 4), 468}, // CLZ in tosa_001000_1
    {IR(57156, 7), 488}, // CONCAT in tosa_001000_1
    {IR(56689, 7), 436}, // CONV2D in tosa_001000_1
    {IR(56696, 7), 437}, // CONV3D in tosa_001000_1
    {IR(56993, 4), 469}, // COS in tosa_001000_1
    {IR(56703, 17), 438}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(57058, 6), 479}, // EQUAL in tosa_001000_1
    {IR(56774, 4), 445}, // ERF in tosa_001000_1
    {IR(56997, 4), 470}, // EXP in tosa_001000_1
    {IR(56720, 6), 439}, // FFT2D in tosa_001000_1
    {IR(57001, 6), 471}, // FLOOR in tosa_001000_1
    {IR(57204, 7), 495}, // GATHER in tosa_001000_1
    {IR(57064, 8), 480}, // GREATER in tosa_001000_1
    {IR(57072, 14), 481}, // GREATER_EQUAL in tosa_001000_1
    {IR(56853, 7), 453}, // INTDIV in tosa_001000_1
    {IR(57007, 4), 472}, // LOG in tosa_001000_1
    {IR(56860, 12), 454}, // LOGICAL_AND in tosa_001000_1
    {IR(56872, 19), 455}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(57011, 12), 473}, // LOGICAL_NOT in tosa_001000_1
    {IR(56911, 11), 457}, // LOGICAL_OR in tosa_001000_1
    {IR(56891, 20), 456}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(56922, 12), 458}, // LOGICAL_XOR in tosa_001000_1
    {IR(56726, 7), 440}, // MATMUL in tosa_001000_1
    {IR(56934, 8), 459}, // MAXIMUM in tosa_001000_1
    {IR(56733, 11), 441}, // MAX_POOL2D in tosa_001000_1
    {IR(56942, 8), 460}, // MINIMUM in tosa_001000_1
    {IR(56950, 4), 461}, // MUL in tosa_001000_1
    {IR(57023, 7), 474}, // NEGATE in tosa_001000_1
    {IR(57163, 4), 489}, // PAD in tosa_001000_1
    {IR(56954, 4), 462}, // POW in tosa_001000_1
    {IR(57030, 11), 475}, // RECIPROCAL in tosa_001000_1
    {IR(57086, 11), 482}, // REDUCE_ALL in tosa_001000_1
    {IR(57097, 11), 483}, // REDUCE_ANY in tosa_001000_1
    {IR(57108, 11), 484}, // REDUCE_MAX in tosa_001000_1
    {IR(57119, 11), 485}, // REDUCE_MIN in tosa_001000_1
    {IR(57130, 15), 486}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(57145, 11), 487}, // REDUCE_SUM in tosa_001000_1
    {IR(57231, 8), 499}, // RESCALE in tosa_001000_1
    {IR(57167, 8), 490}, // RESHAPE in tosa_001000_1
    {IR(57219, 7), 497}, // RESIZE in tosa_001000_1
    {IR(57175, 8), 491}, // REVERSE in tosa_001000_1
    {IR(56744, 7), 442}, // RFFT2D in tosa_001000_1
    {IR(57041, 6), 476}, // RSQRT in tosa_001000_1
    {IR(57211, 8), 496}, // SCATTER in tosa_001000_1
    {IR(57051, 7), 478}, // SELECT in tosa_001000_1
    {IR(56778, 8), 446}, // SIGMOID in tosa_001000_1
    {IR(57047, 4), 477}, // SIN in tosa_001000_1
    {IR(57183, 6), 492}, // SLICE in tosa_001000_1
    {IR(56958, 4), 463}, // SUB in tosa_001000_1
    {IR(56962, 6), 464}, // TABLE in tosa_001000_1
    {IR(56786, 5), 447}, // TANH in tosa_001000_1
    {IR(57189, 5), 493}, // TILE in tosa_001000_1
    {IR(57194, 10), 494}, // TRANSPOSE in tosa_001000_1
    {IR(56751, 17), 443}, // TRANSPOSE_CONV2D in tosa_001000_1
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
  "SPV_EXT_demote_to_helper_invocation\0" // IR(1153, 36)
  "SPV_EXT_descriptor_heap\0" // IR(1189, 24)
  "SPV_EXT_descriptor_indexing\0" // IR(1213, 28)
  "SPV_EXT_float8\0" // IR(1241, 15)
  "SPV_EXT_fragment_fully_covered\0" // IR(1256, 31)
  "SPV_EXT_fragment_invocation_density\0" // IR(1287, 36)
  "SPV_EXT_fragment_shader_interlock\0" // IR(1323, 34)
  "SPV_EXT_long_vector\0" // IR(1357, 20)
  "SPV_EXT_mesh_shader\0" // IR(1377, 20)
  "SPV_EXT_ocp_microscaling_types\0" // IR(1397, 31)
  "SPV_EXT_opacity_micromap\0" // IR(1428, 25)
  "SPV_EXT_optnone\0" // IR(1453, 16)
  "SPV_EXT_physical_storage_buffer\0" // IR(1469, 32)
  "SPV_EXT_relaxed_printf_string_address_space\0" // IR(1501, 44)
  "SPV_EXT_replicated_composites\0" // IR(1545, 30)
  "SPV_EXT_shader_64bit_indexing\0" // IR(1575, 30)
  "SPV_EXT_shader_atomic_float16_add\0" // IR(1605, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(1639, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(1671, 36)
  "SPV_EXT_shader_image_int64\0" // IR(1707, 27)
  "SPV_EXT_shader_invocation_reorder\0" // IR(1734, 34)
  "SPV_EXT_shader_stencil_export\0" // IR(1768, 30)
  "SPV_EXT_shader_subgroup_partitioned\0" // IR(1798, 36)
  "SPV_EXT_shader_tile_image\0" // IR(1834, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1860, 36)
  "SPV_EXT_split_barrier\0" // IR(1896, 22)
  "SPV_GOOGLE_decorate_string\0" // IR(1918, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1945, 31)
  "SPV_GOOGLE_user_type\0" // IR(1976, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1997, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(2019, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(2061, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(2106, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(2145, 30)
  "SPV_INTEL_bindless_images\0" // IR(2175, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(2201, 25)
  "SPV_INTEL_cache_controls\0" // IR(2226, 25)
  "SPV_INTEL_debug_module\0" // IR(2251, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(2274, 44)
  "SPV_INTEL_float_controls2\0" // IR(2318, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(2344, 28)
  "SPV_INTEL_fp_max_error\0" // IR(2372, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(2395, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(2430, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(2461, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(2495, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(2522, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(2570, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(2601, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(2630, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(2661, 33)
  "SPV_INTEL_fpga_reg\0" // IR(2694, 19)
  "SPV_INTEL_function_pointers\0" // IR(2713, 28)
  "SPV_INTEL_function_variants\0" // IR(2741, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(2769, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(2812, 38)
  "SPV_INTEL_inline_assembly\0" // IR(2850, 26)
  "SPV_INTEL_int4\0" // IR(2876, 15)
  "SPV_INTEL_io_pipes\0" // IR(2891, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2910, 28)
  "SPV_INTEL_long_composites\0" // IR(2938, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2964, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2984, 32)
  "SPV_INTEL_maximum_registers\0" // IR(3016, 28)
  "SPV_INTEL_media_block_io\0" // IR(3044, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(3069, 33)
  "SPV_INTEL_optnone\0" // IR(3102, 18)
  "SPV_INTEL_predicated_io\0" // IR(3120, 24)
  "SPV_INTEL_rounded_divide_sqrt\0" // IR(3144, 30)
  "SPV_INTEL_runtime_aligned\0" // IR(3174, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(3200, 36)
  "SPV_INTEL_split_barrier\0" // IR(3236, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3260, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3295, 46)
  "SPV_INTEL_subgroups\0" // IR(3341, 20)
  "SPV_INTEL_task_sequence\0" // IR(3361, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3385, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3421, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3456, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3493, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3523, 32)
  "SPV_INTEL_vector_compute\0" // IR(3555, 25)
  "SPV_KHR_16bit_storage\0" // IR(3580, 22)
  "SPV_KHR_8bit_storage\0" // IR(3602, 21)
  "SPV_KHR_abort\0" // IR(3623, 14)
  "SPV_KHR_bfloat16\0" // IR(3637, 17)
  "SPV_KHR_bit_instructions\0" // IR(3654, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3679, 35)
  "SPV_KHR_constant_data\0" // IR(3714, 22)
  "SPV_KHR_cooperative_matrix\0" // IR(3736, 27)
  "SPV_KHR_device_group\0" // IR(3763, 21)
  "SPV_KHR_expect_assume\0" // IR(3784, 22)
  "SPV_KHR_float_controls\0" // IR(3806, 23)
  "SPV_KHR_float_controls2\0" // IR(3829, 24)
  "SPV_KHR_fma\0" // IR(3853, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3865, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3901, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3931, 28)
  "SPV_KHR_linkonce_odr\0" // IR(3959, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3980, 30)
  "SPV_KHR_multiview\0" // IR(4010, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(4028, 35)
  "SPV_KHR_non_semantic_info\0" // IR(4063, 26)
  "SPV_KHR_opacity_micromap\0" // IR(4089, 25)
  "SPV_KHR_physical_storage_buffer\0" // IR(4114, 32)
  "SPV_KHR_poison_freeze\0" // IR(4146, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(4168, 28)
  "SPV_KHR_quad_control\0" // IR(4196, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4217, 22)
  "SPV_KHR_ray_query\0" // IR(4239, 18)
  "SPV_KHR_ray_tracing\0" // IR(4257, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4277, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4312, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4349, 34)
  "SPV_KHR_shader_ballot\0" // IR(4383, 22)
  "SPV_KHR_shader_clock\0" // IR(4405, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4426, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4457, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4494, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4518, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4556, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4578, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4607, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4642, 25)
  "SPV_KHR_variable_pointers\0" // IR(4667, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4693, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4721, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4762, 38)
  "SPV_NV_bindless_texture\0" // IR(4800, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4824, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4862, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4896, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4922, 27)
  "SPV_NV_cooperative_matrix_decode_vector\0" // IR(4949, 40)
  "SPV_NV_cooperative_vector\0" // IR(4989, 26)
  "SPV_NV_displacement_micromap\0" // IR(5015, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(5044, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(5079, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(5114, 28)
  "SPV_NV_mesh_shader\0" // IR(5142, 19)
  "SPV_NV_push_constant_bank\0" // IR(5161, 26)
  "SPV_NV_raw_access_chains\0" // IR(5187, 25)
  "SPV_NV_ray_tracing\0" // IR(5212, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5231, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5262, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5299, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5332, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5362, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5395, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5421, 35)
  "SPV_NV_shading_rate\0" // IR(5456, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5476, 29)
  "SPV_NV_tensor_addressing\0" // IR(5505, 25)
  "SPV_NV_viewport_array2\0" // IR(5530, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5553, 39)
  "SPV_QCOM_image_processing\0" // IR(5592, 26)
  "SPV_QCOM_image_processing2\0" // IR(5618, 27)
  "SPV_QCOM_image_processing3\0" // IR(5645, 27)
  "SPV_QCOM_multiple_wait_queues\0" // IR(5672, 30)
  "SPV_QCOM_tile_shading\0" // IR(5702, 22)
  "SPV_SAMSUNG_intrinsic\0" // IR(5724, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5746, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5784, 34)
  "ReadOnly\0" // IR(5818, 9)
  "Kernel\0" // IR(5827, 7)
  "WriteOnly\0" // IR(5834, 10)
  "ReadWrite\0" // IR(5844, 10)
  "Logical\0" // IR(5854, 8)
  "Physical32\0" // IR(5862, 11)
  "Addresses\0" // IR(5873, 10)
  "Physical64\0" // IR(5883, 11)
  "PhysicalStorageBuffer64\0" // IR(5894, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(5918, 27)
  "PhysicalStorageBufferAddresses\0" // IR(5945, 31)
  "Position\0" // IR(5976, 9)
  "Shader\0" // IR(5985, 7)
  "PointSize\0" // IR(5992, 10)
  "ClipDistance\0" // IR(6002, 13)
  "CullDistance\0" // IR(6015, 13)
  "VertexId\0" // IR(6028, 9)
  "InstanceId\0" // IR(6037, 11)
  "PrimitiveId\0" // IR(6048, 12)
  "Geometry\0" // IR(6060, 9)
  "Tessellation\0" // IR(6069, 13)
  "RayTracingNV\0" // IR(6082, 13)
  "RayTracingKHR\0" // IR(6095, 14)
  "MeshShadingNV\0" // IR(6109, 14)
  "MeshShadingEXT\0" // IR(6123, 15)
  "InvocationId\0" // IR(6138, 13)
  "Layer\0" // IR(6151, 6)
  "ShaderLayer\0" // IR(6157, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(6169, 28)
  "ViewportIndex\0" // IR(6197, 14)
  "MultiViewport\0" // IR(6211, 14)
  "ShaderViewportIndex\0" // IR(6225, 20)
  "TessLevelOuter\0" // IR(6245, 15)
  "TessLevelInner\0" // IR(6260, 15)
  "TessCoord\0" // IR(6275, 10)
  "PatchVertices\0" // IR(6285, 14)
  "FragCoord\0" // IR(6299, 10)
  "PointCoord\0" // IR(6309, 11)
  "FrontFacing\0" // IR(6320, 12)
  "SampleId\0" // IR(6332, 9)
  "SampleRateShading\0" // IR(6341, 18)
  "SamplePosition\0" // IR(6359, 15)
  "SampleMask\0" // IR(6374, 11)
  "FragDepth\0" // IR(6385, 10)
  "HelperInvocation\0" // IR(6395, 17)
  "NumWorkgroups\0" // IR(6412, 14)
  "WorkgroupSize\0" // IR(6426, 14)
  "WorkgroupId\0" // IR(6440, 12)
  "LocalInvocationId\0" // IR(6452, 18)
  "GlobalInvocationId\0" // IR(6470, 19)
  "LocalInvocationIndex\0" // IR(6489, 21)
  "WorkDim\0" // IR(6510, 8)
  "GlobalSize\0" // IR(6518, 11)
  "EnqueuedWorkgroupSize\0" // IR(6529, 22)
  "GlobalOffset\0" // IR(6551, 13)
  "GlobalLinearId\0" // IR(6564, 15)
  "SubgroupSize\0" // IR(6579, 13)
  "GroupNonUniform\0" // IR(6592, 16)
  "SubgroupBallotKHR\0" // IR(6608, 18)
  "SubgroupMaxSize\0" // IR(6626, 16)
  "NumSubgroups\0" // IR(6642, 13)
  "NumEnqueuedSubgroups\0" // IR(6655, 21)
  "SubgroupId\0" // IR(6676, 11)
  "SubgroupLocalInvocationId\0" // IR(6687, 26)
  "VertexIndex\0" // IR(6713, 12)
  "InstanceIndex\0" // IR(6725, 14)
  "CoreIDARM\0" // IR(6739, 10)
  "CoreBuiltinsARM\0" // IR(6749, 16)
  "CoreCountARM\0" // IR(6765, 13)
  "CoreMaxIDARM\0" // IR(6778, 13)
  "WarpIDARM\0" // IR(6791, 10)
  "WarpMaxIDARM\0" // IR(6801, 13)
  "SubgroupEqMask\0" // IR(6814, 15)
  "SubgroupEqMaskKHR\0" // IR(6829, 18)
  "GroupNonUniformBallot\0" // IR(6847, 22)
  "SubgroupGeMask\0" // IR(6869, 15)
  "SubgroupGeMaskKHR\0" // IR(6884, 18)
  "SubgroupGtMask\0" // IR(6902, 15)
  "SubgroupGtMaskKHR\0" // IR(6917, 18)
  "SubgroupLeMask\0" // IR(6935, 15)
  "SubgroupLeMaskKHR\0" // IR(6950, 18)
  "SubgroupLtMask\0" // IR(6968, 15)
  "SubgroupLtMaskKHR\0" // IR(6983, 18)
  "BaseVertex\0" // IR(7001, 11)
  "DrawParameters\0" // IR(7012, 15)
  "BaseInstance\0" // IR(7027, 13)
  "DrawIndex\0" // IR(7040, 10)
  "PrimitiveShadingRateKHR\0" // IR(7050, 24)
  "FragmentShadingRateKHR\0" // IR(7074, 23)
  "DeviceIndex\0" // IR(7097, 12)
  "DeviceGroup\0" // IR(7109, 12)
  "ViewIndex\0" // IR(7121, 10)
  "MultiView\0" // IR(7131, 10)
  "ShadingRateKHR\0" // IR(7141, 15)
  "TileOffsetQCOM\0" // IR(7156, 15)
  "TileShadingQCOM\0" // IR(7171, 16)
  "TileDimensionQCOM\0" // IR(7187, 18)
  "TileApronSizeQCOM\0" // IR(7205, 18)
  "BaryCoordNoPerspAMD\0" // IR(7223, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(7243, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(7271, 26)
  "BaryCoordSmoothAMD\0" // IR(7297, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7316, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7343, 25)
  "BaryCoordPullModelAMD\0" // IR(7368, 22)
  "FragStencilRefEXT\0" // IR(7390, 18)
  "StencilExportEXT\0" // IR(7408, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7425, 29)
  "ShaderEnqueueAMDX\0" // IR(7454, 18)
  "ShaderIndexAMDX\0" // IR(7472, 16)
  "SamplerHeapEXT\0" // IR(7488, 15)
  "DescriptorHeapEXT\0" // IR(7503, 18)
  "ResourceHeapEXT\0" // IR(7521, 16)
  "ViewportMaskNV\0" // IR(7537, 15)
  "ShaderViewportMaskNV\0" // IR(7552, 21)
  "SecondaryPositionNV\0" // IR(7573, 20)
  "ShaderStereoViewNV\0" // IR(7593, 19)
  "SecondaryViewportMaskNV\0" // IR(7612, 24)
  "PositionPerViewNV\0" // IR(7636, 18)
  "PerViewAttributesNV\0" // IR(7654, 20)
  "ViewportMaskPerViewNV\0" // IR(7674, 22)
  "FullyCoveredEXT\0" // IR(7696, 16)
  "FragmentFullyCoveredEXT\0" // IR(7712, 24)
  "TaskCountNV\0" // IR(7736, 12)
  "PrimitiveCountNV\0" // IR(7748, 17)
  "PrimitiveIndicesNV\0" // IR(7765, 19)
  "ClipDistancePerViewNV\0" // IR(7784, 22)
  "CullDistancePerViewNV\0" // IR(7806, 22)
  "LayerPerViewNV\0" // IR(7828, 15)
  "MeshViewCountNV\0" // IR(7843, 16)
  "MeshViewIndicesNV\0" // IR(7859, 18)
  "BaryCoordKHR\0" // IR(7877, 13)
  "BaryCoordNV\0" // IR(7890, 12)
  "FragmentBarycentricKHR\0" // IR(7902, 23)
  "BaryCoordNoPerspKHR\0" // IR(7925, 20)
  "BaryCoordNoPerspNV\0" // IR(7945, 19)
  "FragSizeEXT\0" // IR(7964, 12)
  "FragmentSizeNV\0" // IR(7976, 15)
  "FragmentDensityEXT\0" // IR(7991, 19)
  "FragInvocationCountEXT\0" // IR(8010, 23)
  "InvocationsPerPixelNV\0" // IR(8033, 22)
  "PrimitivePointIndicesEXT\0" // IR(8055, 25)
  "PrimitiveLineIndicesEXT\0" // IR(8080, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(8104, 28)
  "CullPrimitiveEXT\0" // IR(8132, 17)
  "LaunchIdKHR\0" // IR(8149, 12)
  "LaunchIdNV\0" // IR(8161, 11)
  "LaunchSizeKHR\0" // IR(8172, 14)
  "LaunchSizeNV\0" // IR(8186, 13)
  "WorldRayOriginKHR\0" // IR(8199, 18)
  "WorldRayOriginNV\0" // IR(8217, 17)
  "WorldRayDirectionKHR\0" // IR(8234, 21)
  "WorldRayDirectionNV\0" // IR(8255, 20)
  "ObjectRayOriginKHR\0" // IR(8275, 19)
  "ObjectRayOriginNV\0" // IR(8294, 18)
  "ObjectRayDirectionKHR\0" // IR(8312, 22)
  "ObjectRayDirectionNV\0" // IR(8334, 21)
  "RayTminKHR\0" // IR(8355, 11)
  "RayTminNV\0" // IR(8366, 10)
  "RayTmaxKHR\0" // IR(8376, 11)
  "RayTmaxNV\0" // IR(8387, 10)
  "InstanceCustomIndexKHR\0" // IR(8397, 23)
  "InstanceCustomIndexNV\0" // IR(8420, 22)
  "ObjectToWorldKHR\0" // IR(8442, 17)
  "ObjectToWorldNV\0" // IR(8459, 16)
  "WorldToObjectKHR\0" // IR(8475, 17)
  "WorldToObjectNV\0" // IR(8492, 16)
  "HitTNV\0" // IR(8508, 7)
  "HitKindKHR\0" // IR(8515, 11)
  "HitKindNV\0" // IR(8526, 10)
  "CurrentRayTimeNV\0" // IR(8536, 17)
  "RayTracingMotionBlurNV\0" // IR(8553, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8576, 30)
  "RayTracingPositionFetchKHR\0" // IR(8606, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8633, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8667, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8700, 37)
  "IncomingRayFlagsKHR\0" // IR(8737, 20)
  "IncomingRayFlagsNV\0" // IR(8757, 19)
  "RayGeometryIndexKHR\0" // IR(8776, 20)
  "HitIsSphereNV\0" // IR(8796, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8810, 28)
  "HitIsLSSNV\0" // IR(8838, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8849, 39)
  "HitSpherePositionNV\0" // IR(8888, 20)
  "WarpsPerSMNV\0" // IR(8908, 13)
  "ShaderSMBuiltinsNV\0" // IR(8921, 19)
  "SMCountNV\0" // IR(8940, 10)
  "WarpIDNV\0" // IR(8950, 9)
  "SMIDNV\0" // IR(8959, 7)
  "HitLSSPositionsNV\0" // IR(8966, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(8984, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(9018, 33)
  "HitSphereRadiusNV\0" // IR(9051, 18)
  "HitLSSRadiiNV\0" // IR(9069, 14)
  "ClusterIDNV\0" // IR(9083, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(9095, 41)
  "CullMaskKHR\0" // IR(9136, 12)
  "RayCullMaskKHR\0" // IR(9148, 15)
  "Matrix\0" // IR(9163, 7)
  "Linkage\0" // IR(9170, 8)
  "Vector16\0" // IR(9178, 9)
  "Float16Buffer\0" // IR(9187, 14)
  "Float16\0" // IR(9201, 8)
  "Float64\0" // IR(9209, 8)
  "Int64\0" // IR(9217, 6)
  "Int64Atomics\0" // IR(9223, 13)
  "ImageBasic\0" // IR(9236, 11)
  "ImageReadWrite\0" // IR(9247, 15)
  "ImageMipmap\0" // IR(9262, 12)
  "Pipes\0" // IR(9274, 6)
  "Groups\0" // IR(9280, 7)
  "DeviceEnqueue\0" // IR(9287, 14)
  "LiteralSampler\0" // IR(9301, 15)
  "AtomicStorage\0" // IR(9316, 14)
  "Int16\0" // IR(9330, 6)
  "TessellationPointSize\0" // IR(9336, 22)
  "GeometryPointSize\0" // IR(9358, 18)
  "ImageGatherExtended\0" // IR(9376, 20)
  "StorageImageMultisample\0" // IR(9396, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9420, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9454, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9487, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9521, 33)
  "ImageCubeArray\0" // IR(9554, 15)
  "SampledCubeArray\0" // IR(9569, 17)
  "ImageRect\0" // IR(9586, 10)
  "SampledRect\0" // IR(9596, 12)
  "GenericPointer\0" // IR(9608, 15)
  "Int8\0" // IR(9623, 5)
  "InputAttachment\0" // IR(9628, 16)
  "SparseResidency\0" // IR(9644, 16)
  "MinLod\0" // IR(9660, 7)
  "Sampled1D\0" // IR(9667, 10)
  "Image1D\0" // IR(9677, 8)
  "SampledBuffer\0" // IR(9685, 14)
  "ImageBuffer\0" // IR(9699, 12)
  "ImageMSArray\0" // IR(9711, 13)
  "StorageImageExtendedFormats\0" // IR(9724, 28)
  "ImageQuery\0" // IR(9752, 11)
  "DerivativeControl\0" // IR(9763, 18)
  "InterpolationFunction\0" // IR(9781, 22)
  "TransformFeedback\0" // IR(9803, 18)
  "GeometryStreams\0" // IR(9821, 16)
  "StorageImageReadWithoutFormat\0" // IR(9837, 30)
  "StorageImageWriteWithoutFormat\0" // IR(9867, 31)
  "SubgroupDispatch\0" // IR(9898, 17)
  "NamedBarrier\0" // IR(9915, 13)
  "PipeStorage\0" // IR(9928, 12)
  "GroupNonUniformVote\0" // IR(9940, 20)
  "GroupNonUniformArithmetic\0" // IR(9960, 26)
  "GroupNonUniformShuffle\0" // IR(9986, 23)
  "GroupNonUniformShuffleRelative\0" // IR(10009, 31)
  "GroupNonUniformClustered\0" // IR(10040, 25)
  "GroupNonUniformQuad\0" // IR(10065, 20)
  "UniformDecoration\0" // IR(10085, 18)
  "TileImageColorReadAccessEXT\0" // IR(10103, 28)
  "TileImageDepthReadAccessEXT\0" // IR(10131, 28)
  "TileImageStencilReadAccessEXT\0" // IR(10159, 30)
  "TensorsARM\0" // IR(10189, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(10200, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(10237, 40)
  "GraphARM\0" // IR(10277, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(10286, 28)
  "Float8EXT\0" // IR(10314, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10324, 27)
  "CooperativeMatrixKHR\0" // IR(10351, 21)
  "Float6EXT\0" // IR(10372, 10)
  "Float4EXT\0" // IR(10382, 10)
  "Float8UnsignedE8M0EXT\0" // IR(10392, 22)
  "MXInt8EXT\0" // IR(10414, 10)
  "BitcastExtractEXT\0" // IR(10424, 18)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10442, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10475, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10518, 44)
  "SubgroupVoteKHR\0" // IR(10562, 16)
  "StorageBuffer16BitAccess\0" // IR(10578, 25)
  "StorageUniformBufferBlock16\0" // IR(10603, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10631, 35)
  "StorageUniform16\0" // IR(10666, 17)
  "StoragePushConstant16\0" // IR(10683, 22)
  "StorageInputOutput16\0" // IR(10705, 21)
  "VariablePointersStorageBuffer\0" // IR(10726, 30)
  "VariablePointers\0" // IR(10756, 17)
  "AtomicStorageOps\0" // IR(10773, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10790, 28)
  "StorageBuffer8BitAccess\0" // IR(10818, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10842, 34)
  "StoragePushConstant8\0" // IR(10876, 21)
  "DenormPreserve\0" // IR(10897, 15)
  "DenormFlushToZero\0" // IR(10912, 18)
  "SignedZeroInfNanPreserve\0" // IR(10930, 25)
  "RoundingModeRTE\0" // IR(10955, 16)
  "RoundingModeRTZ\0" // IR(10971, 16)
  "RayQueryProvisionalKHR\0" // IR(10987, 23)
  "RayQueryKHR\0" // IR(11010, 12)
  "UntypedPointersKHR\0" // IR(11022, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(11041, 32)
  "TextureSampleWeightedQCOM\0" // IR(11073, 26)
  "TextureBoxFilterQCOM\0" // IR(11099, 21)
  "TextureBlockMatchQCOM\0" // IR(11120, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(11142, 32)
  "TextureBlockMatch2QCOM\0" // IR(11174, 23)
  "MultipleWaitQueuesQCOM\0" // IR(11197, 23)
  "ImageGatherLinearQCOM\0" // IR(11220, 22)
  "ImageGatherExtendedModesQCOM\0" // IR(11242, 29)
  "Float16ImageAMD\0" // IR(11271, 16)
  "ImageGatherBiasLodAMD\0" // IR(11287, 22)
  "FragmentMaskAMD\0" // IR(11309, 16)
  "ImageReadWriteLodAMD\0" // IR(11325, 21)
  "Int64ImageEXT\0" // IR(11346, 14)
  "ShaderClockKHR\0" // IR(11360, 15)
  "QuadControlKHR\0" // IR(11375, 15)
  "Int4TypeINTEL\0" // IR(11390, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(11404, 27)
  "BFloat16TypeKHR\0" // IR(11431, 16)
  "BFloat16DotProductKHR\0" // IR(11447, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11469, 29)
  "AbortKHR\0" // IR(11498, 9)
  "ConstantDataKHR\0" // IR(11507, 16)
  "PoisonFreezeKHR\0" // IR(11523, 16)
  "WeakLinkageAMD\0" // IR(11539, 15)
  "SampleMaskOverrideCoverageNV\0" // IR(11554, 29)
  "GeometryShaderPassthroughNV\0" // IR(11583, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11611, 27)
  "ImageFootprintNV\0" // IR(11638, 17)
  "FragmentBarycentricNV\0" // IR(11655, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11677, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11708, 30)
  "ShadingRateNV\0" // IR(11738, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11752, 30)
  "GroupNonUniformPartitionedNV\0" // IR(11782, 29)
  "ShaderNonUniform\0" // IR(11811, 17)
  "ShaderNonUniformEXT\0" // IR(11828, 20)
  "RuntimeDescriptorArray\0" // IR(11848, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(11871, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(11897, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(11933, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(11972, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(12011, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(12053, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(12092, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(12134, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(12171, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(12211, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(12247, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(12286, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(12323, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(12363, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(12399, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(12438, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(12477, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12519, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12561, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12606, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12648, 45)
  "VulkanMemoryModel\0" // IR(12693, 18)
  "VulkanMemoryModelKHR\0" // IR(12711, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12732, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12761, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12793, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(12827, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(12859, 31)
  "RayTracingProvisionalKHR\0" // IR(12890, 25)
  "CooperativeMatrixNV\0" // IR(12915, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(12935, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(12968, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(13006, 32)
  "DemoteToHelperInvocation\0" // IR(13038, 25)
  "DemoteToHelperInvocationEXT\0" // IR(13063, 28)
  "DisplacementMicromapNV\0" // IR(13091, 23)
  "RayTracingOpacityMicromapKHR\0" // IR(13114, 29)
  "RayTracingOpacityMicromapEXT\0" // IR(13143, 29)
  "ShaderInvocationReorderNV\0" // IR(13172, 26)
  "ShaderInvocationReorderEXT\0" // IR(13198, 27)
  "BindlessTextureNV\0" // IR(13225, 18)
  "RayQueryPositionFetchKHR\0" // IR(13243, 25)
  "CooperativeVectorNV\0" // IR(13268, 20)
  "AtomicFloat16VectorNV\0" // IR(13288, 22)
  "RawAccessChainsNV\0" // IR(13310, 18)
  "PushConstantBanksNV\0" // IR(13328, 20)
  "LongVectorEXT\0" // IR(13348, 14)
  "Shader64BitIndexingEXT\0" // IR(13362, 23)
  "CooperativeMatrixReductionsNV\0" // IR(13385, 30)
  "CooperativeMatrixConversionsNV\0" // IR(13415, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(13446, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(13486, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13522, 30)
  "CooperativeVectorTrainingNV\0" // IR(13552, 28)
  "TensorAddressingNV\0" // IR(13580, 19)
  "CooperativeMatrixDecodeVectorNV\0" // IR(13599, 32)
  "SubgroupShuffleINTEL\0" // IR(13631, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(13652, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(13679, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(13705, 31)
  "RoundToInfinityINTEL\0" // IR(13736, 21)
  "FloatingPointModeINTEL\0" // IR(13757, 23)
  "IntegerFunctions2INTEL\0" // IR(13780, 23)
  "FunctionPointersINTEL\0" // IR(13803, 22)
  "IndirectReferencesINTEL\0" // IR(13825, 24)
  "AsmINTEL\0" // IR(13849, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(13858, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(13881, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(13904, 23)
  "VectorComputeINTEL\0" // IR(13927, 19)
  "VectorAnyINTEL\0" // IR(13946, 15)
  "ExpectAssumeKHR\0" // IR(13961, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(13977, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(14010, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(14048, 39)
  "VariableLengthArrayINTEL\0" // IR(14087, 25)
  "FunctionFloatControlINTEL\0" // IR(14112, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(14138, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(14165, 26)
  "FPFastMathModeINTEL\0" // IR(14191, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(14211, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(14244, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(14276, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(14314, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(14351, 30)
  "FPGALoopControlsALTERA\0" // IR(14381, 23)
  "FPGALoopControlsINTEL\0" // IR(14404, 22)
  "KernelAttributesINTEL\0" // IR(14426, 22)
  "FPGAKernelAttributesINTEL\0" // IR(14448, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(14474, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(14499, 24)
  "FPGAClusterAttributesALTERA\0" // IR(14523, 28)
  "FPGAClusterAttributesINTEL\0" // IR(14551, 27)
  "LoopFuseALTERA\0" // IR(14578, 15)
  "LoopFuseINTEL\0" // IR(14593, 14)
  "FPGADSPControlALTERA\0" // IR(14607, 21)
  "FPGADSPControlINTEL\0" // IR(14628, 20)
  "MemoryAccessAliasingINTEL\0" // IR(14648, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(14674, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(14715, 40)
  "FPGABufferLocationALTERA\0" // IR(14755, 25)
  "FPGABufferLocationINTEL\0" // IR(14780, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(14804, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(14839, 34)
  "USMStorageClassesALTERA\0" // IR(14873, 24)
  "USMStorageClassesINTEL\0" // IR(14897, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(14920, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(14950, 29)
  "IOPipesALTERA\0" // IR(14979, 14)
  "IOPipesINTEL\0" // IR(14993, 13)
  "BlockingPipesALTERA\0" // IR(15006, 20)
  "BlockingPipesINTEL\0" // IR(15026, 19)
  "FPGARegALTERA\0" // IR(15045, 14)
  "FPGARegINTEL\0" // IR(15059, 13)
  "DotProductInputAll\0" // IR(15072, 19)
  "DotProductInputAllKHR\0" // IR(15091, 22)
  "DotProductInput4x8Bit\0" // IR(15113, 22)
  "DotProductInput4x8BitKHR\0" // IR(15135, 25)
  "DotProductInput4x8BitPacked\0" // IR(15160, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(15188, 31)
  "DotProduct\0" // IR(15219, 11)
  "DotProductKHR\0" // IR(15230, 14)
  "ReplicatedCompositesEXT\0" // IR(15244, 24)
  "BitInstructions\0" // IR(15268, 16)
  "GroupNonUniformRotateKHR\0" // IR(15284, 25)
  "FloatControls2\0" // IR(15309, 15)
  "FMAKHR\0" // IR(15324, 7)
  "RayTracingOpacityMicromapExecutionModeKHR\0" // IR(15331, 42)
  "AtomicFloat32AddEXT\0" // IR(15373, 20)
  "AtomicFloat64AddEXT\0" // IR(15393, 20)
  "LongCompositesINTEL\0" // IR(15413, 20)
  "OptNoneEXT\0" // IR(15433, 11)
  "OptNoneINTEL\0" // IR(15444, 13)
  "AtomicFloat16AddEXT\0" // IR(15457, 20)
  "DebugInfoModuleINTEL\0" // IR(15477, 21)
  "BFloat16ConversionINTEL\0" // IR(15498, 24)
  "SplitBarrierEXT\0" // IR(15522, 16)
  "SplitBarrierINTEL\0" // IR(15538, 18)
  "ArithmeticFenceEXT\0" // IR(15556, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(15575, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(15605, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15634, 28)
  "TaskSequenceALTERA\0" // IR(15662, 19)
  "TaskSequenceINTEL\0" // IR(15681, 18)
  "FPMaxErrorINTEL\0" // IR(15699, 16)
  "FPGALatencyControlALTERA\0" // IR(15715, 25)
  "FPGALatencyControlINTEL\0" // IR(15740, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(15764, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(15793, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(15821, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(15851, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(15887, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(15922, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(15950, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(15973, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(16003, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(16033, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(16071, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(16099, 32)
  "SpecConditionalINTEL\0" // IR(16131, 21)
  "FunctionVariantsINTEL\0" // IR(16152, 22)
  "PredicatedIOINTEL\0" // IR(16174, 18)
  "RoundedDivideSqrtINTEL\0" // IR(16192, 23)
  "GroupUniformArithmeticKHR\0" // IR(16215, 26)
  "TensorFloat32RoundingINTEL\0" // IR(16241, 27)
  "MaskedGatherScatterINTEL\0" // IR(16268, 25)
  "CacheControlsINTEL\0" // IR(16293, 19)
  "RegisterLimitsINTEL\0" // IR(16312, 20)
  "BindlessImagesINTEL\0" // IR(16332, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(16352, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(16385, 33)
  "DotProductBFloat16AccVALVE\0" // IR(16418, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(16445, 32)
  "IntrinsicSAMSUNG\0" // IR(16477, 17)
  "Unspecified\0" // IR(16494, 12)
  "Address\0" // IR(16506, 8)
  "Boolean\0" // IR(16514, 8)
  "Float\0" // IR(16522, 6)
  "Signed\0" // IR(16528, 7)
  "SignedChar\0" // IR(16535, 11)
  "Unsigned\0" // IR(16546, 9)
  "UnsignedChar\0" // IR(16555, 13)
  "Class\0" // IR(16568, 6)
  "Structure\0" // IR(16574, 10)
  "Union\0" // IR(16584, 6)
  "ImportedModule\0" // IR(16590, 15)
  "ImportedDeclaration\0" // IR(16605, 20)
  "None\0" // IR(16625, 5)
  "FlagIsProtected\0" // IR(16630, 16)
  "FlagIsPrivate\0" // IR(16646, 14)
  "FlagIsPublic\0" // IR(16660, 13)
  "FlagIsLocal\0" // IR(16673, 12)
  "FlagIsDefinition\0" // IR(16685, 17)
  "FlagFwdDecl\0" // IR(16702, 12)
  "FlagArtificial\0" // IR(16714, 15)
  "FlagExplicit\0" // IR(16729, 13)
  "FlagPrototyped\0" // IR(16742, 15)
  "FlagObjectPointer\0" // IR(16757, 18)
  "FlagStaticMember\0" // IR(16775, 17)
  "FlagIndirectVariable\0" // IR(16792, 21)
  "FlagLValueReference\0" // IR(16813, 20)
  "FlagRValueReference\0" // IR(16833, 20)
  "FlagIsOptimized\0" // IR(16853, 16)
  "FlagIsEnumClass\0" // IR(16869, 16)
  "FlagTypePassByValue\0" // IR(16885, 20)
  "FlagTypePassByReference\0" // IR(16905, 24)
  "Deref\0" // IR(16929, 6)
  "Plus\0" // IR(16935, 5)
  "Minus\0" // IR(16940, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(16946, 33)
  "PlusUconst\0" // IR(16979, 11)
  "BitPiece\0" // IR(16990, 9)
  "Swap\0" // IR(16999, 5)
  "Xderef\0" // IR(17004, 7)
  "StackValue\0" // IR(17011, 11)
  "Constu\0" // IR(17022, 7)
  "Fragment\0" // IR(17029, 9)
  "ConstType\0" // IR(17038, 10)
  "VolatileType\0" // IR(17048, 13)
  "RestrictType\0" // IR(17061, 13)
  "AtomicType\0" // IR(17074, 11)
  "Float16NV\0" // IR(17085, 10)
  "Float32NV\0" // IR(17095, 10)
  "Float64NV\0" // IR(17105, 10)
  "SignedInt8NV\0" // IR(17115, 13)
  "SignedInt16NV\0" // IR(17128, 14)
  "SignedInt32NV\0" // IR(17142, 14)
  "SignedInt64NV\0" // IR(17156, 14)
  "UnsignedInt8NV\0" // IR(17170, 15)
  "UnsignedInt16NV\0" // IR(17185, 16)
  "UnsignedInt32NV\0" // IR(17201, 16)
  "UnsignedInt64NV\0" // IR(17217, 16)
  "SignedInt8PackedNV\0" // IR(17233, 19)
  "UnsignedInt8PackedNV\0" // IR(17252, 21)
  "FloatE4M3NV\0" // IR(17273, 12)
  "FloatE5M2NV\0" // IR(17285, 12)
  "RowMajorKHR\0" // IR(17297, 12)
  "ColumnMajorKHR\0" // IR(17309, 15)
  "RowBlockedInterleavedARM\0" // IR(17324, 25)
  "ColumnBlockedInterleavedARM\0" // IR(17349, 28)
  "NoneKHR\0" // IR(17377, 8)
  "MatrixASignedComponentsKHR\0" // IR(17385, 27)
  "MatrixBSignedComponentsKHR\0" // IR(17412, 27)
  "MatrixCSignedComponentsKHR\0" // IR(17439, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(17466, 32)
  "SaturatingAccumulationKHR\0" // IR(17498, 26)
  "Row\0" // IR(17524, 4)
  "Column\0" // IR(17528, 7)
  "2x2\0" // IR(17535, 4)
  "MatrixAKHR\0" // IR(17539, 11)
  "MatrixBKHR\0" // IR(17550, 11)
  "MatrixAccumulatorKHR\0" // IR(17561, 21)
  "RowMajorNV\0" // IR(17582, 11)
  "ColumnMajorNV\0" // IR(17593, 14)
  "InferencingOptimalNV\0" // IR(17607, 21)
  "TrainingOptimalNV\0" // IR(17628, 18)
  "RelaxedPrecision\0" // IR(17646, 17)
  "SpecId\0" // IR(17663, 7)
  "Block\0" // IR(17670, 6)
  "BufferBlock\0" // IR(17676, 12)
  "RowMajor\0" // IR(17688, 9)
  "ColMajor\0" // IR(17697, 9)
  "ArrayStride\0" // IR(17706, 12)
  "MatrixStride\0" // IR(17718, 13)
  "GLSLShared\0" // IR(17731, 11)
  "GLSLPacked\0" // IR(17742, 11)
  "CPacked\0" // IR(17753, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(17761, 26)
  "BuiltIn\0" // IR(17787, 8)
  "NoPerspective\0" // IR(17795, 14)
  "Flat\0" // IR(17809, 5)
  "Patch\0" // IR(17814, 6)
  "Centroid\0" // IR(17820, 9)
  "Sample\0" // IR(17829, 7)
  "Invariant\0" // IR(17836, 10)
  "Restrict\0" // IR(17846, 9)
  "Aliased\0" // IR(17855, 8)
  "Volatile\0" // IR(17863, 9)
  "Constant\0" // IR(17872, 9)
  "Coherent\0" // IR(17881, 9)
  "NonWritable\0" // IR(17890, 12)
  "NonReadable\0" // IR(17902, 12)
  "Uniform\0" // IR(17914, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(17922, 26)
  "UniformId\0" // IR(17948, 10)
  "SaturatedConversion\0" // IR(17958, 20)
  "Stream\0" // IR(17978, 7)
  "Location\0" // IR(17985, 9)
  "Component\0" // IR(17994, 10)
  "Index\0" // IR(18004, 6)
  "Binding\0" // IR(18010, 8)
  "DescriptorSet\0" // IR(18018, 14)
  "Offset\0" // IR(18032, 7)
  "XfbBuffer\0" // IR(18039, 10)
  "XfbStride\0" // IR(18049, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(18059, 46)
  "FuncParamAttr\0" // IR(18105, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(18119, 34)
  "FPRoundingMode\0" // IR(18153, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(18168, 35)
  "FPFastMathMode\0" // IR(18203, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(18218, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(18250, 30)
  "LinkageAttributes\0" // IR(18280, 18)
  "NoContraction\0" // IR(18298, 14)
  "InputAttachmentIndex\0" // IR(18312, 21)
  "Alignment\0" // IR(18333, 10)
  "MaxByteOffset\0" // IR(18343, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(18357, 20)
  "AlignmentId\0" // IR(18377, 12)
  "MaxByteOffsetId\0" // IR(18389, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(18405, 44)
  "NoSignedWrap\0" // IR(18449, 13)
  "NoUnsignedWrap\0" // IR(18462, 15)
  "WeightTextureQCOM\0" // IR(18477, 18)
  "BlockMatchTextureQCOM\0" // IR(18495, 22)
  "BlockMatchSamplerQCOM\0" // IR(18517, 22)
  "ExplicitInterpAMD\0" // IR(18539, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(18557, 32)
  "NodeMaxPayloadsAMDX\0" // IR(18589, 20)
  "TrackFinishWritingAMDX\0" // IR(18609, 23)
  "PayloadNodeNameAMDX\0" // IR(18632, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(18652, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(18677, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(18704, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(18729, 28)
  "ArrayStrideIdEXT\0" // IR(18757, 17)
  "OffsetIdEXT\0" // IR(18774, 12)
  "UTFEncodedKHR\0" // IR(18786, 14)
  "OverrideCoverageNV\0" // IR(18800, 19)
  "PassthroughNV\0" // IR(18819, 14)
  "ViewportRelativeNV\0" // IR(18833, 19)
  "SecondaryViewportRelativeNV\0" // IR(18852, 28)
  "PerPrimitiveEXT\0" // IR(18880, 16)
  "PerPrimitiveNV\0" // IR(18896, 15)
  "PerViewNV\0" // IR(18911, 10)
  "PerTaskNV\0" // IR(18921, 10)
  "PerVertexKHR\0" // IR(18931, 13)
  "PerVertexNV\0" // IR(18944, 12)
  "NonUniform\0" // IR(18956, 11)
  "NonUniformEXT\0" // IR(18967, 14)
  "RestrictPointer\0" // IR(18981, 16)
  "RestrictPointerEXT\0" // IR(18997, 19)
  "AliasedPointer\0" // IR(19016, 15)
  "AliasedPointerEXT\0" // IR(19031, 18)
  "MemberOffsetNV\0" // IR(19049, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(19064, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(19094, 31)
  "BankNV\0" // IR(19125, 7)
  "BindlessSamplerNV\0" // IR(19132, 18)
  "BindlessImageNV\0" // IR(19150, 16)
  "BoundSamplerNV\0" // IR(19166, 15)
  "BoundImageNV\0" // IR(19181, 13)
  "SIMTCallINTEL\0" // IR(19194, 14)
  "ReferencedIndirectlyINTEL\0" // IR(19208, 26)
  "ClobberINTEL\0" // IR(19234, 13)
  "SideEffectsINTEL\0" // IR(19247, 17)
  "VectorComputeVariableINTEL\0" // IR(19264, 27)
  "FuncParamIOKindINTEL\0" // IR(19291, 21)
  "VectorComputeFunctionINTEL\0" // IR(19312, 27)
  "StackCallINTEL\0" // IR(19339, 15)
  "GlobalVariableOffsetINTEL\0" // IR(19354, 26)
  "CounterBuffer\0" // IR(19380, 14)
  "HlslCounterBufferGOOGLE\0" // IR(19394, 24)
  "UserSemantic\0" // IR(19418, 13)
  "HlslSemanticGOOGLE\0" // IR(19431, 19)
  "UserTypeGOOGLE\0" // IR(19450, 15)
  "FunctionRoundingModeINTEL\0" // IR(19465, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(19491, 31)
  "FunctionDenormModeINTEL\0" // IR(19522, 24)
  "RegisterALTERA\0" // IR(19546, 15)
  "RegisterINTEL\0" // IR(19561, 14)
  "MemoryALTERA\0" // IR(19575, 13)
  "MemoryINTEL\0" // IR(19588, 12)
  "NumbanksALTERA\0" // IR(19600, 15)
  "NumbanksINTEL\0" // IR(19615, 14)
  "BankwidthALTERA\0" // IR(19629, 16)
  "BankwidthINTEL\0" // IR(19645, 15)
  "MaxPrivateCopiesALTERA\0" // IR(19660, 23)
  "MaxPrivateCopiesINTEL\0" // IR(19683, 22)
  "SinglepumpALTERA\0" // IR(19705, 17)
  "SinglepumpINTEL\0" // IR(19722, 16)
  "DoublepumpALTERA\0" // IR(19738, 17)
  "DoublepumpINTEL\0" // IR(19755, 16)
  "MaxReplicatesALTERA\0" // IR(19771, 20)
  "MaxReplicatesINTEL\0" // IR(19791, 19)
  "SimpleDualPortALTERA\0" // IR(19810, 21)
  "SimpleDualPortINTEL\0" // IR(19831, 20)
  "MergeALTERA\0" // IR(19851, 12)
  "MergeINTEL\0" // IR(19863, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(19874, 42)
  "BankBitsALTERA\0" // IR(19916, 15)
  "BankBitsINTEL\0" // IR(19931, 14)
  "ForcePow2DepthALTERA\0" // IR(19945, 21)
  "ForcePow2DepthINTEL\0" // IR(19966, 20)
  "StridesizeALTERA\0" // IR(19986, 17)
  "StridesizeINTEL\0" // IR(20003, 16)
  "WordsizeALTERA\0" // IR(20019, 15)
  "WordsizeINTEL\0" // IR(20034, 14)
  "TrueDualPortALTERA\0" // IR(20048, 19)
  "TrueDualPortINTEL\0" // IR(20067, 18)
  "BurstCoalesceALTERA\0" // IR(20085, 20)
  "BurstCoalesceINTEL\0" // IR(20105, 19)
  "CacheSizeALTERA\0" // IR(20124, 16)
  "CacheSizeINTEL\0" // IR(20140, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(20155, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(20184, 28)
  "PrefetchALTERA\0" // IR(20212, 15)
  "PrefetchINTEL\0" // IR(20227, 14)
  "StallEnableALTERA\0" // IR(20241, 18)
  "StallEnableINTEL\0" // IR(20259, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(20276, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(20302, 25)
  "MathOpDSPModeALTERA\0" // IR(20327, 20)
  "MathOpDSPModeINTEL\0" // IR(20347, 19)
  "AliasScopeINTEL\0" // IR(20366, 16)
  "NoAliasINTEL\0" // IR(20382, 13)
  "InitiationIntervalALTERA\0" // IR(20395, 25)
  "InitiationIntervalINTEL\0" // IR(20420, 24)
  "MaxConcurrencyALTERA\0" // IR(20444, 21)
  "MaxConcurrencyINTEL\0" // IR(20465, 20)
  "PipelineEnableALTERA\0" // IR(20485, 21)
  "PipelineEnableINTEL\0" // IR(20506, 20)
  "BufferLocationALTERA\0" // IR(20526, 21)
  "BufferLocationINTEL\0" // IR(20547, 20)
  "IOPipeStorageALTERA\0" // IR(20567, 20)
  "IOPipeStorageINTEL\0" // IR(20587, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(20606, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(20640, 31)
  "SingleElementVectorINTEL\0" // IR(20671, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(20696, 35)
  "MediaBlockIOINTEL\0" // IR(20731, 18)
  "StallFreeALTERA\0" // IR(20749, 16)
  "StallFreeINTEL\0" // IR(20765, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(20780, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(20811, 26)
  "LatencyControlLabelALTERA\0" // IR(20837, 26)
  "LatencyControlLabelINTEL\0" // IR(20863, 25)
  "LatencyControlConstraintALTERA\0" // IR(20888, 31)
  "LatencyControlConstraintINTEL\0" // IR(20919, 30)
  "ConduitKernelArgumentALTERA\0" // IR(20949, 28)
  "ConduitKernelArgumentINTEL\0" // IR(20977, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(21004, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(21036, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(21067, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(21101, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(21134, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(21165, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(21195, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(21224, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(21252, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(21286, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(21321, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(21355, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(21385, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(21414, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(21447, 32)
  "StableKernelArgumentALTERA\0" // IR(21479, 27)
  "StableKernelArgumentINTEL\0" // IR(21506, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(21532, 39)
  "HostAccessINTEL\0" // IR(21571, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(21587, 47)
  "InitModeALTERA\0" // IR(21634, 15)
  "InitModeINTEL\0" // IR(21649, 14)
  "ImplementInRegisterMapALTERA\0" // IR(21663, 29)
  "ImplementInRegisterMapINTEL\0" // IR(21692, 28)
  "ConditionalINTEL\0" // IR(21720, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(21737, 36)
  "CacheControlLoadINTEL\0" // IR(21773, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(21795, 37)
  "CacheControlStoreINTEL\0" // IR(21832, 23)
  "1D\0" // IR(21855, 3)
  "2D\0" // IR(21858, 3)
  "3D\0" // IR(21861, 3)
  "Cube\0" // IR(21864, 5)
  "Rect\0" // IR(21869, 5)
  "Buffer\0" // IR(21874, 7)
  "SubpassData\0" // IR(21881, 12)
  "TileImageDataEXT\0" // IR(21893, 17)
  "Invocations\0" // IR(21910, 12)
  "SpacingEqual\0" // IR(21922, 13)
  "SpacingFractionalEven\0" // IR(21935, 22)
  "SpacingFractionalOdd\0" // IR(21957, 21)
  "VertexOrderCw\0" // IR(21978, 14)
  "VertexOrderCcw\0" // IR(21992, 15)
  "PixelCenterInteger\0" // IR(22007, 19)
  "OriginUpperLeft\0" // IR(22026, 16)
  "OriginLowerLeft\0" // IR(22042, 16)
  "EarlyFragmentTests\0" // IR(22058, 19)
  "PointMode\0" // IR(22077, 10)
  "Xfb\0" // IR(22087, 4)
  "DepthReplacing\0" // IR(22091, 15)
  "DepthGreater\0" // IR(22106, 13)
  "DepthLess\0" // IR(22119, 10)
  "DepthUnchanged\0" // IR(22129, 15)
  "LocalSize\0" // IR(22144, 10)
  "LocalSizeHint\0" // IR(22154, 14)
  "InputPoints\0" // IR(22168, 12)
  "InputLines\0" // IR(22180, 11)
  "InputLinesAdjacency\0" // IR(22191, 20)
  "Triangles\0" // IR(22211, 10)
  "InputTrianglesAdjacency\0" // IR(22221, 24)
  "Quads\0" // IR(22245, 6)
  "Isolines\0" // IR(22251, 9)
  "OutputVertices\0" // IR(22260, 15)
  "OutputPoints\0" // IR(22275, 13)
  "OutputLineStrip\0" // IR(22288, 16)
  "OutputTriangleStrip\0" // IR(22304, 20)
  "VecTypeHint\0" // IR(22324, 12)
  "ContractionOff\0" // IR(22336, 15)
  "Initializer\0" // IR(22351, 12)
  "Finalizer\0" // IR(22363, 10)
  "SubgroupsPerWorkgroup\0" // IR(22373, 22)
  "SubgroupsPerWorkgroupId\0" // IR(22395, 24)
  "LocalSizeId\0" // IR(22419, 12)
  "LocalSizeHintId\0" // IR(22431, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(22447, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(22481, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(22515, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(22551, 30)
  "PostDepthCoverage\0" // IR(22581, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(22599, 34)
  "TileShadingRateQCOM\0" // IR(22633, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(22653, 29)
  "StencilRefReplacingEXT\0" // IR(22682, 23)
  "CoalescingAMDX\0" // IR(22705, 15)
  "IsApiEntryAMDX\0" // IR(22720, 15)
  "MaxNodeRecursionAMDX\0" // IR(22735, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(22756, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(22780, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(22801, 28)
  "StencilRefGreaterFrontAMD\0" // IR(22829, 26)
  "StencilRefLessFrontAMD\0" // IR(22855, 23)
  "StencilRefUnchangedBackAMD\0" // IR(22878, 27)
  "StencilRefGreaterBackAMD\0" // IR(22905, 25)
  "StencilRefLessBackAMD\0" // IR(22930, 22)
  "QuadDerivativesKHR\0" // IR(22952, 19)
  "RequireFullQuadsKHR\0" // IR(22971, 20)
  "SharesInputWithAMDX\0" // IR(22991, 20)
  "ArithmeticPoisonKHR\0" // IR(23011, 20)
  "OutputLinesEXT\0" // IR(23031, 15)
  "OutputLinesNV\0" // IR(23046, 14)
  "OutputPrimitivesEXT\0" // IR(23060, 20)
  "OutputPrimitivesNV\0" // IR(23080, 19)
  "DerivativeGroupQuadsKHR\0" // IR(23099, 24)
  "DerivativeGroupQuadsNV\0" // IR(23123, 23)
  "DerivativeGroupLinearKHR\0" // IR(23146, 25)
  "DerivativeGroupLinearNV\0" // IR(23171, 24)
  "OutputTrianglesEXT\0" // IR(23195, 19)
  "OutputTrianglesNV\0" // IR(23214, 18)
  "PixelInterlockOrderedEXT\0" // IR(23232, 25)
  "PixelInterlockUnorderedEXT\0" // IR(23257, 27)
  "SampleInterlockOrderedEXT\0" // IR(23284, 26)
  "SampleInterlockUnorderedEXT\0" // IR(23310, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(23338, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(23369, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(23402, 27)
  "RoundingModeRTPINTEL\0" // IR(23429, 21)
  "RoundingModeRTNINTEL\0" // IR(23450, 21)
  "FloatingPointModeALTINTEL\0" // IR(23471, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(23497, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(23524, 22)
  "MaxWorkDimINTEL\0" // IR(23546, 16)
  "NoGlobalOffsetINTEL\0" // IR(23562, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(23582, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(23604, 28)
  "MaximallyReconvergesKHR\0" // IR(23632, 24)
  "FPFastMathDefault\0" // IR(23656, 18)
  "OpacityMicromapIdKHR\0" // IR(23674, 21)
  "StreamingInterfaceINTEL\0" // IR(23695, 24)
  "RegisterMapInterfaceINTEL\0" // IR(23719, 26)
  "NamedBarrierCountINTEL\0" // IR(23745, 23)
  "MaximumRegistersINTEL\0" // IR(23768, 22)
  "MaximumRegistersIdINTEL\0" // IR(23790, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(23814, 51)
  "NamedMaximumRegistersINTEL\0" // IR(23865, 27)
  "Vertex\0" // IR(23892, 7)
  "TessellationControl\0" // IR(23899, 20)
  "TessellationEvaluation\0" // IR(23919, 23)
  "GLCompute\0" // IR(23942, 10)
  "TaskNV\0" // IR(23952, 7)
  "MeshNV\0" // IR(23959, 7)
  "RayGenerationKHR\0" // IR(23966, 17)
  "RayGenerationNV\0" // IR(23983, 16)
  "IntersectionKHR\0" // IR(23999, 16)
  "IntersectionNV\0" // IR(24015, 15)
  "AnyHitKHR\0" // IR(24030, 10)
  "AnyHitNV\0" // IR(24040, 9)
  "ClosestHitKHR\0" // IR(24049, 14)
  "ClosestHitNV\0" // IR(24063, 13)
  "MissKHR\0" // IR(24076, 8)
  "MissNV\0" // IR(24084, 7)
  "CallableKHR\0" // IR(24091, 12)
  "CallableNV\0" // IR(24103, 11)
  "TaskEXT\0" // IR(24114, 8)
  "MeshEXT\0" // IR(24122, 8)
  "Preserve\0" // IR(24130, 9)
  "FlushToZero\0" // IR(24139, 12)
  "BFloat16KHR\0" // IR(24151, 12)
  "Float8E4M3EXT\0" // IR(24163, 14)
  "Float8E5M2EXT\0" // IR(24177, 14)
  "Float6E2M3EXT\0" // IR(24191, 14)
  "Float6E3M2EXT\0" // IR(24205, 14)
  "Float4E2M1EXT\0" // IR(24219, 14)
  "IEEE\0" // IR(24233, 5)
  "ALT\0" // IR(24238, 4)
  "NotNaN\0" // IR(24242, 7)
  "NotInf\0" // IR(24249, 7)
  "NSZ\0" // IR(24256, 4)
  "AllowRecip\0" // IR(24260, 11)
  "Fast\0" // IR(24271, 5)
  "AllowContract\0" // IR(24276, 14)
  "AllowContractFastINTEL\0" // IR(24290, 23)
  "AllowReassoc\0" // IR(24313, 13)
  "AllowReassocINTEL\0" // IR(24326, 18)
  "AllowTransform\0" // IR(24344, 15)
  "RTE\0" // IR(24359, 4)
  "RTZ\0" // IR(24363, 4)
  "RTP\0" // IR(24367, 4)
  "RTN\0" // IR(24371, 4)
  "Vertical2Pixels\0" // IR(24375, 16)
  "Vertical4Pixels\0" // IR(24391, 16)
  "Horizontal2Pixels\0" // IR(24407, 18)
  "Horizontal4Pixels\0" // IR(24425, 18)
  "Inline\0" // IR(24443, 7)
  "DontInline\0" // IR(24450, 11)
  "Pure\0" // IR(24461, 5)
  "Const\0" // IR(24466, 6)
  "Zext\0" // IR(24472, 5)
  "Sext\0" // IR(24477, 5)
  "ByVal\0" // IR(24482, 6)
  "Sret\0" // IR(24488, 5)
  "NoAlias\0" // IR(24493, 8)
  "NoCapture\0" // IR(24501, 10)
  "NoWrite\0" // IR(24511, 8)
  "NoReadWrite\0" // IR(24519, 12)
  "RuntimeAlignedALTERA\0" // IR(24531, 21)
  "RuntimeAlignedINTEL\0" // IR(24552, 20)
  "Gather4x1QCOM\0" // IR(24572, 14)
  "GatherDQCOM\0" // IR(24586, 12)
  "GatherH2QCOM\0" // IR(24598, 13)
  "GatherV2QCOM\0" // IR(24611, 13)
  "Reduce\0" // IR(24624, 7)
  "InclusiveScan\0" // IR(24631, 14)
  "ExclusiveScan\0" // IR(24645, 14)
  "ClusteredReduce\0" // IR(24659, 16)
  "PartitionedReduceEXT\0" // IR(24675, 21)
  "PartitionedReduceNV\0" // IR(24696, 20)
  "PartitionedInclusiveScanEXT\0" // IR(24716, 28)
  "PartitionedInclusiveScanNV\0" // IR(24744, 27)
  "PartitionedExclusiveScanEXT\0" // IR(24771, 28)
  "PartitionedExclusiveScanNV\0" // IR(24799, 27)
  "NoneINTEL\0" // IR(24826, 10)
  "ReadINTEL\0" // IR(24836, 10)
  "WriteINTEL\0" // IR(24846, 11)
  "ReadWriteINTEL\0" // IR(24857, 15)
  "Bias\0" // IR(24872, 5)
  "Lod\0" // IR(24877, 4)
  "Grad\0" // IR(24881, 5)
  "ConstOffset\0" // IR(24886, 12)
  "ConstOffsets\0" // IR(24898, 13)
  "MakeTexelAvailable\0" // IR(24911, 19)
  "MakeTexelAvailableKHR\0" // IR(24930, 22)
  "MakeTexelVisible\0" // IR(24952, 17)
  "MakeTexelVisibleKHR\0" // IR(24969, 20)
  "NonPrivateTexel\0" // IR(24989, 16)
  "NonPrivateTexelKHR\0" // IR(25005, 19)
  "VolatileTexel\0" // IR(25024, 14)
  "VolatileTexelKHR\0" // IR(25038, 17)
  "SignExtend\0" // IR(25055, 11)
  "ZeroExtend\0" // IR(25066, 11)
  "Nontemporal\0" // IR(25077, 12)
  "Offsets\0" // IR(25089, 8)
  "SnormInt8\0" // IR(25097, 10)
  "SnormInt16\0" // IR(25107, 11)
  "UnormInt8\0" // IR(25118, 10)
  "UnormInt16\0" // IR(25128, 11)
  "UnormShort565\0" // IR(25139, 14)
  "UnormShort555\0" // IR(25153, 14)
  "UnormInt101010\0" // IR(25167, 15)
  "SignedInt8\0" // IR(25182, 11)
  "SignedInt16\0" // IR(25193, 12)
  "SignedInt32\0" // IR(25205, 12)
  "UnsignedInt8\0" // IR(25217, 13)
  "UnsignedInt16\0" // IR(25230, 14)
  "UnsignedInt32\0" // IR(25244, 14)
  "HalfFloat\0" // IR(25258, 10)
  "UnormInt24\0" // IR(25268, 11)
  "UnormInt101010_2\0" // IR(25279, 17)
  "UnormInt10X6EXT\0" // IR(25296, 16)
  "UnsignedIntRaw10EXT\0" // IR(25312, 20)
  "UnsignedIntRaw12EXT\0" // IR(25332, 20)
  "UnormInt2_101010EXT\0" // IR(25352, 20)
  "UnsignedInt10X6EXT\0" // IR(25372, 19)
  "UnsignedInt12X4EXT\0" // IR(25391, 19)
  "UnsignedInt14X2EXT\0" // IR(25410, 19)
  "UnormInt12X4EXT\0" // IR(25429, 16)
  "UnormInt14X2EXT\0" // IR(25445, 16)
  "R\0" // IR(25461, 2)
  "A\0" // IR(25463, 2)
  "RG\0" // IR(25465, 3)
  "RA\0" // IR(25468, 3)
  "RGB\0" // IR(25471, 4)
  "RGBA\0" // IR(25475, 5)
  "BGRA\0" // IR(25480, 5)
  "ARGB\0" // IR(25485, 5)
  "Intensity\0" // IR(25490, 10)
  "Luminance\0" // IR(25500, 10)
  "Rx\0" // IR(25510, 3)
  "RGx\0" // IR(25513, 4)
  "RGBx\0" // IR(25517, 5)
  "Depth\0" // IR(25522, 6)
  "DepthStencil\0" // IR(25528, 13)
  "sRGB\0" // IR(25541, 5)
  "sRGBx\0" // IR(25546, 6)
  "sRGBA\0" // IR(25552, 6)
  "sBGRA\0" // IR(25558, 6)
  "ABGR\0" // IR(25564, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(25569, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(25597, 27)
  "InitOnDeviceResetALTERA\0" // IR(25624, 24)
  "InitOnDeviceResetINTEL\0" // IR(25648, 23)
  "NoWait\0" // IR(25671, 7)
  "WaitKernel\0" // IR(25678, 11)
  "WaitWorkGroup\0" // IR(25689, 14)
  "CmdExecTime\0" // IR(25703, 12)
  "MayUsePrintf\0" // IR(25715, 13)
  "Export\0" // IR(25728, 7)
  "Import\0" // IR(25735, 7)
  "LinkOnceODR\0" // IR(25742, 12)
  "WeakAMD\0" // IR(25754, 8)
  "UncachedINTEL\0" // IR(25762, 14)
  "CachedINTEL\0" // IR(25776, 12)
  "StreamingINTEL\0" // IR(25788, 15)
  "InvalidateAfterReadINTEL\0" // IR(25803, 25)
  "ConstCachedINTEL\0" // IR(25828, 17)
  "Unroll\0" // IR(25845, 7)
  "DontUnroll\0" // IR(25852, 11)
  "DependencyInfinite\0" // IR(25863, 19)
  "DependencyLength\0" // IR(25882, 17)
  "MinIterations\0" // IR(25899, 14)
  "MaxIterations\0" // IR(25913, 14)
  "IterationMultiple\0" // IR(25927, 18)
  "PeelCount\0" // IR(25945, 10)
  "PartialCount\0" // IR(25955, 13)
  "DependencyArrayALTERA\0" // IR(25968, 22)
  "DependencyArrayINTEL\0" // IR(25990, 21)
  "LoopCoalesceALTERA\0" // IR(26011, 19)
  "LoopCoalesceINTEL\0" // IR(26030, 18)
  "MaxInterleavingALTERA\0" // IR(26048, 22)
  "MaxInterleavingINTEL\0" // IR(26070, 21)
  "SpeculatedIterationsALTERA\0" // IR(26091, 27)
  "SpeculatedIterationsINTEL\0" // IR(26118, 26)
  "NoFusionALTERA\0" // IR(26144, 15)
  "NoFusionINTEL\0" // IR(26159, 14)
  "LoopCountALTERA\0" // IR(26173, 16)
  "LoopCountINTEL\0" // IR(26189, 15)
  "MaxReinvocationDelayALTERA\0" // IR(26204, 27)
  "MaxReinvocationDelayINTEL\0" // IR(26231, 26)
  "MatrixASignedComponentsINTEL\0" // IR(26257, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(26286, 29)
  "MatrixCBFloat16INTEL\0" // IR(26315, 21)
  "MatrixResultBFloat16INTEL\0" // IR(26336, 26)
  "MatrixAPackedInt8INTEL\0" // IR(26362, 23)
  "MatrixBPackedInt8INTEL\0" // IR(26385, 23)
  "MatrixAPackedInt4INTEL\0" // IR(26408, 23)
  "MatrixBPackedInt4INTEL\0" // IR(26431, 23)
  "MatrixATF32INTEL\0" // IR(26454, 17)
  "MatrixBTF32INTEL\0" // IR(26471, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(26488, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(26514, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(26540, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(26567, 27)
  "Aligned\0" // IR(26594, 8)
  "MakePointerAvailable\0" // IR(26602, 21)
  "MakePointerAvailableKHR\0" // IR(26623, 24)
  "MakePointerVisible\0" // IR(26647, 19)
  "MakePointerVisibleKHR\0" // IR(26666, 22)
  "NonPrivatePointer\0" // IR(26688, 18)
  "NonPrivatePointerKHR\0" // IR(26706, 21)
  "AliasScopeINTELMask\0" // IR(26727, 20)
  "NoAliasINTELMask\0" // IR(26747, 17)
  "Simple\0" // IR(26764, 7)
  "GLSL450\0" // IR(26771, 8)
  "OpenCL\0" // IR(26779, 7)
  "Vulkan\0" // IR(26786, 7)
  "VulkanKHR\0" // IR(26793, 10)
  "Relaxed\0" // IR(26803, 8)
  "Acquire\0" // IR(26811, 8)
  "Release\0" // IR(26819, 8)
  "AcquireRelease\0" // IR(26827, 15)
  "SequentiallyConsistent\0" // IR(26842, 23)
  "UniformMemory\0" // IR(26865, 14)
  "SubgroupMemory\0" // IR(26879, 15)
  "WorkgroupMemory\0" // IR(26894, 16)
  "CrossWorkgroupMemory\0" // IR(26910, 21)
  "AtomicCounterMemory\0" // IR(26931, 20)
  "ImageMemory\0" // IR(26951, 12)
  "OutputMemory\0" // IR(26963, 13)
  "OutputMemoryKHR\0" // IR(26976, 16)
  "MakeAvailable\0" // IR(26992, 14)
  "MakeAvailableKHR\0" // IR(27006, 17)
  "MakeVisible\0" // IR(27023, 12)
  "MakeVisibleKHR\0" // IR(27035, 15)
  "AutoINTEL\0" // IR(27050, 10)
  "WRAP\0" // IR(27060, 5)
  "SAT\0" // IR(27065, 4)
  "SAT_ZERO\0" // IR(27069, 9)
  "SAT_SYM\0" // IR(27078, 8)
  "PackedVectorFormat4x8Bit\0" // IR(27086, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(27111, 28)
  "TRN\0" // IR(27139, 4)
  "TRN_ZERO\0" // IR(27143, 9)
  "RND\0" // IR(27152, 4)
  "RND_ZERO\0" // IR(27156, 9)
  "RND_INF\0" // IR(27165, 8)
  "RND_MIN_INF\0" // IR(27173, 12)
  "RND_CONV\0" // IR(27185, 9)
  "RND_CONV_ODD\0" // IR(27194, 13)
  "RobustnessPerComponentNV\0" // IR(27207, 25)
  "RobustnessPerElementNV\0" // IR(27232, 23)
  "OpaqueKHR\0" // IR(27255, 10)
  "NoOpaqueKHR\0" // IR(27265, 12)
  "TerminateOnFirstHitKHR\0" // IR(27277, 23)
  "SkipClosestHitShaderKHR\0" // IR(27300, 24)
  "CullBackFacingTrianglesKHR\0" // IR(27324, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(27351, 28)
  "CullOpaqueKHR\0" // IR(27379, 14)
  "CullNoOpaqueKHR\0" // IR(27393, 16)
  "SkipTrianglesKHR\0" // IR(27409, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(27426, 24)
  "SkipAABBsKHR\0" // IR(27450, 13)
  "ForceOpacityMicromap2StateKHR\0" // IR(27463, 30)
  "ForceOpacityMicromap2StateEXT\0" // IR(27493, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(27523, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(27564, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(27601, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(27638, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(27679, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(27721, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(27754, 33)
  "ClampToEdge\0" // IR(27787, 12)
  "Clamp\0" // IR(27799, 6)
  "Repeat\0" // IR(27805, 7)
  "RepeatMirrored\0" // IR(27812, 15)
  "Nearest\0" // IR(27827, 8)
  "Linear\0" // IR(27835, 7)
  "Unknown\0" // IR(27842, 8)
  "Rgba32f\0" // IR(27850, 8)
  "Rgba16f\0" // IR(27858, 8)
  "R32f\0" // IR(27866, 5)
  "Rgba8\0" // IR(27871, 6)
  "Rgba8Snorm\0" // IR(27877, 11)
  "Rg32f\0" // IR(27888, 6)
  "Rg16f\0" // IR(27894, 6)
  "R11fG11fB10f\0" // IR(27900, 13)
  "R16f\0" // IR(27913, 5)
  "Rgba16\0" // IR(27918, 7)
  "Rgb10A2\0" // IR(27925, 8)
  "Rg16\0" // IR(27933, 5)
  "Rg8\0" // IR(27938, 4)
  "R16\0" // IR(27942, 4)
  "R8\0" // IR(27946, 3)
  "Rgba16Snorm\0" // IR(27949, 12)
  "Rg16Snorm\0" // IR(27961, 10)
  "Rg8Snorm\0" // IR(27971, 9)
  "R16Snorm\0" // IR(27980, 9)
  "R8Snorm\0" // IR(27989, 8)
  "Rgba32i\0" // IR(27997, 8)
  "Rgba16i\0" // IR(28005, 8)
  "Rgba8i\0" // IR(28013, 7)
  "R32i\0" // IR(28020, 5)
  "Rg32i\0" // IR(28025, 6)
  "Rg16i\0" // IR(28031, 6)
  "Rg8i\0" // IR(28037, 5)
  "R16i\0" // IR(28042, 5)
  "R8i\0" // IR(28047, 4)
  "Rgba32ui\0" // IR(28051, 9)
  "Rgba16ui\0" // IR(28060, 9)
  "Rgba8ui\0" // IR(28069, 8)
  "R32ui\0" // IR(28077, 6)
  "Rgb10a2ui\0" // IR(28083, 10)
  "Rg32ui\0" // IR(28093, 7)
  "Rg16ui\0" // IR(28100, 7)
  "Rg8ui\0" // IR(28107, 6)
  "R16ui\0" // IR(28113, 6)
  "R8ui\0" // IR(28119, 5)
  "R64ui\0" // IR(28124, 6)
  "R64i\0" // IR(28130, 5)
  "CrossDevice\0" // IR(28135, 12)
  "Device\0" // IR(28147, 7)
  "Workgroup\0" // IR(28154, 10)
  "Subgroup\0" // IR(28164, 9)
  "Invocation\0" // IR(28173, 11)
  "QueueFamily\0" // IR(28184, 12)
  "QueueFamilyKHR\0" // IR(28196, 15)
  "ShaderCallKHR\0" // IR(28211, 14)
  "Flatten\0" // IR(28225, 8)
  "DontFlatten\0" // IR(28233, 12)
  "IdentifierPossibleDuplicates\0" // IR(28245, 29)
  "FlagUnknownPhysicalLayout\0" // IR(28274, 26)
  "ESSL\0" // IR(28300, 5)
  "GLSL\0" // IR(28305, 5)
  "OpenCL_C\0" // IR(28310, 9)
  "OpenCL_CPP\0" // IR(28319, 11)
  "HLSL\0" // IR(28330, 5)
  "CPP_for_OpenCL\0" // IR(28335, 15)
  "SYCL\0" // IR(28350, 5)
  "HERO_C\0" // IR(28355, 7)
  "NZSL\0" // IR(28362, 5)
  "WGSL\0" // IR(28367, 5)
  "Slang\0" // IR(28372, 6)
  "Zig\0" // IR(28378, 4)
  "Rust\0" // IR(28382, 5)
  "Pred\0" // IR(28387, 5)
  "ApilaJai\0" // IR(28392, 9)
  "UniformConstant\0" // IR(28401, 16)
  "Input\0" // IR(28417, 6)
  "Output\0" // IR(28423, 7)
  "CrossWorkgroup\0" // IR(28430, 15)
  "Private\0" // IR(28445, 8)
  "Function\0" // IR(28453, 9)
  "Generic\0" // IR(28462, 8)
  "PushConstant\0" // IR(28470, 13)
  "AtomicCounter\0" // IR(28483, 14)
  "Image\0" // IR(28497, 6)
  "StorageBuffer\0" // IR(28503, 14)
  "TileImageEXT\0" // IR(28517, 13)
  "TileAttachmentQCOM\0" // IR(28530, 19)
  "NodePayloadAMDX\0" // IR(28549, 16)
  "CallableDataKHR\0" // IR(28565, 16)
  "CallableDataNV\0" // IR(28581, 15)
  "IncomingCallableDataKHR\0" // IR(28596, 24)
  "IncomingCallableDataNV\0" // IR(28620, 23)
  "RayPayloadKHR\0" // IR(28643, 14)
  "RayPayloadNV\0" // IR(28657, 13)
  "HitAttributeKHR\0" // IR(28670, 16)
  "HitAttributeNV\0" // IR(28686, 15)
  "IncomingRayPayloadKHR\0" // IR(28701, 22)
  "IncomingRayPayloadNV\0" // IR(28723, 21)
  "ShaderRecordBufferKHR\0" // IR(28744, 22)
  "ShaderRecordBufferNV\0" // IR(28766, 21)
  "PhysicalStorageBuffer\0" // IR(28787, 22)
  "PhysicalStorageBufferEXT\0" // IR(28809, 25)
  "HitObjectAttributeNV\0" // IR(28834, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(28855, 24)
  "HitObjectAttributeEXT\0" // IR(28879, 22)
  "CodeSectionINTEL\0" // IR(28901, 17)
  "DeviceOnlyALTERA\0" // IR(28918, 17)
  "DeviceOnlyINTEL\0" // IR(28935, 16)
  "HostOnlyALTERA\0" // IR(28951, 15)
  "HostOnlyINTEL\0" // IR(28966, 14)
  "WriteThroughINTEL\0" // IR(28980, 18)
  "WriteBackINTEL\0" // IR(28998, 15)
  "TensorView\0" // IR(29013, 11)
  "DecodeFunc\0" // IR(29024, 11)
  "DecodeVectorFunc\0" // IR(29035, 17)
  "Undefined\0" // IR(29052, 10)
  "NoneARM\0" // IR(29062, 8)
  "NontemporalARM\0" // IR(29070, 15)
  "OutOfBoundsValueARM\0" // IR(29085, 20)
  "MakeElementAvailableARM\0" // IR(29105, 24)
  "MakeElementVisibleARM\0" // IR(29129, 22)
  "NonPrivateElementARM\0" // IR(29151, 21)
  "Nop\0" // IR(29172, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(29176, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(29201, 27)
  "Undef\0" // IR(29228, 6)
  "SourceContinued\0" // IR(29234, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(29250, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(29283, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(29312, 41)
  "Source\0" // IR(29353, 7)
  "SourceExtension\0" // IR(29360, 16)
  "Name\0" // IR(29376, 5)
  "MemberName\0" // IR(29381, 11)
  "String\0" // IR(29392, 7)
  "Line\0" // IR(29399, 5)
  "Extension\0" // IR(29404, 10)
  "ExtInstImport\0" // IR(29414, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(29428, 46)
  "ExtInst\0" // IR(29474, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(29482, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(29516, 30)
  "MemoryModel\0" // IR(29546, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(29558, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(29591, 29)
  "EntryPoint\0" // IR(29620, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(29631, 32)
  "ExecutionMode\0" // IR(29663, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(29677, 28)
  "Capability\0" // IR(29705, 11)
  "TypeVoid\0" // IR(29716, 9)
  "TypeBool\0" // IR(29725, 9)
  "TypeInt\0" // IR(29734, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(29742, 37)
  "TypeFloat\0" // IR(29779, 10)
  "TypeVector\0" // IR(29789, 11)
  "TypeMatrix\0" // IR(29800, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(29811, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(29843, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(29881, 43)
  "TypeImage\0" // IR(29924, 10)
  "TypeSampler\0" // IR(29934, 12)
  "TypeSampledImage\0" // IR(29946, 17)
  "TypeArray\0" // IR(29963, 10)
  "TypeRuntimeArray\0" // IR(29973, 17)
  "TypeStruct\0" // IR(29990, 11)
  "TypeOpaque\0" // IR(30001, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(30012, 31)
  "TypePointer\0" // IR(30043, 12)
  "TypeFunction\0" // IR(30055, 13)
  "TypeEvent\0" // IR(30068, 10)
  "TypeDeviceEvent\0" // IR(30078, 16)
  "TypeReserveId\0" // IR(30094, 14)
  "TypeQueue\0" // IR(30108, 10)
  "TypePipe\0" // IR(30118, 9)
  "TypeForwardPointer\0" // IR(30127, 19)
  "ConstantTrue\0" // IR(30146, 13)
  "ConstantFalse\0" // IR(30159, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(30173, 38)
  "ConstantComposite\0" // IR(30211, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(30229, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(30270, 37)
  "ConstantSampler\0" // IR(30307, 16)
  "ConstantNull\0" // IR(30323, 13)
  "SpecConstantTrue\0" // IR(30336, 17)
  "SpecConstantFalse\0" // IR(30353, 18)
  "SpecConstant\0" // IR(30371, 13)
  "SpecConstantComposite\0" // IR(30384, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(30406, 41)
  "SpecConstantOp\0" // IR(30447, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(30462, 34)
  "FunctionParameter\0" // IR(30496, 18)
  "FunctionEnd\0" // IR(30514, 12)
  "FunctionCall\0" // IR(30526, 13)
  "Variable\0" // IR(30539, 9)
  "ImageTexelPointer\0" // IR(30548, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(30566, 40)
  "Load\0" // IR(30606, 5)
  "Store\0" // IR(30611, 6)
  "CopyMemory\0" // IR(30617, 11)
  "CopyMemorySized\0" // IR(30628, 16)
  "AccessChain\0" // IR(30644, 12)
  "InBoundsAccessChain\0" // IR(30656, 20)
  "PtrAccessChain\0" // IR(30676, 15)
  "ArrayLength\0" // IR(30691, 12)
  "GenericPtrMemSemantics\0" // IR(30703, 23)
  "InBoundsPtrAccessChain\0" // IR(30726, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(30749, 28)
  "Decorate\0" // IR(30777, 9)
  "MemberDecorate\0" // IR(30786, 15)
  "DecorationGroup\0" // IR(30801, 16)
  "GroupDecorate\0" // IR(30817, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(30831, 45)
  "GroupMemberDecorate\0" // IR(30876, 20)
  "VectorExtractDynamic\0" // IR(30896, 21)
  "VectorInsertDynamic\0" // IR(30917, 20)
  "VectorShuffle\0" // IR(30937, 14)
  "CompositeConstruct\0" // IR(30951, 19)
  "CompositeExtract\0" // IR(30970, 17)
  "CompositeInsert\0" // IR(30987, 16)
  "CopyObject\0" // IR(31003, 11)
  "Transpose\0" // IR(31014, 10)
  "SampledImage\0" // IR(31024, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(31037, 32)
  "ImageSampleImplicitLod\0" // IR(31069, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(31092, 23)
  "ImageSampleExplicitLod\0" // IR(31115, 23)
  "ImageSampleDrefImplicitLod\0" // IR(31138, 27)
  "ImageSampleDrefExplicitLod\0" // IR(31165, 27)
  "ImageSampleProjImplicitLod\0" // IR(31192, 27)
  "ImageSampleProjExplicitLod\0" // IR(31219, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(31246, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(31277, 31)
  "ImageFetch\0" // IR(31308, 11)
  "ImageGather\0" // IR(31319, 12)
  "ImageDrefGather\0" // IR(31331, 16)
  "ImageRead\0" // IR(31347, 10)
  "ImageWrite\0" // IR(31357, 11)
  "ImageQueryFormat\0" // IR(31368, 17)
  "ImageQueryOrder\0" // IR(31385, 16)
  "ImageQuerySizeLod\0" // IR(31401, 18)
  "ImageQuerySize\0" // IR(31419, 15)
  "ImageQueryLod\0" // IR(31434, 14)
  "ImageQueryLevels\0" // IR(31448, 17)
  "ImageQuerySamples\0" // IR(31465, 18)
  "ConvertFToU\0" // IR(31483, 12)
  "ConvertFToS\0" // IR(31495, 12)
  "ConvertSToF\0" // IR(31507, 12)
  "ConvertUToF\0" // IR(31519, 12)
  "UConvert\0" // IR(31531, 9)
  "SConvert\0" // IR(31540, 9)
  "FConvert\0" // IR(31549, 9)
  "QuantizeToF16\0" // IR(31558, 14)
  "ConvertPtrToU\0" // IR(31572, 14)
  "SatConvertSToU\0" // IR(31586, 15)
  "SatConvertUToS\0" // IR(31601, 15)
  "ConvertUToPtr\0" // IR(31616, 14)
  "PtrCastToGeneric\0" // IR(31630, 17)
  "GenericCastToPtr\0" // IR(31647, 17)
  "GenericCastToPtrExplicit\0" // IR(31664, 25)
  "Bitcast\0" // IR(31689, 8)
  "SNegate\0" // IR(31697, 8)
  "FNegate\0" // IR(31705, 8)
  "IAdd\0" // IR(31713, 5)
  "FAdd\0" // IR(31718, 5)
  "ISub\0" // IR(31723, 5)
  "FSub\0" // IR(31728, 5)
  "IMul\0" // IR(31733, 5)
  "FMul\0" // IR(31738, 5)
  "UDiv\0" // IR(31743, 5)
  "SDiv\0" // IR(31748, 5)
  "FDiv\0" // IR(31753, 5)
  "UMod\0" // IR(31758, 5)
  "SRem\0" // IR(31763, 5)
  "SMod\0" // IR(31768, 5)
  "FRem\0" // IR(31773, 5)
  "FMod\0" // IR(31778, 5)
  "VectorTimesScalar\0" // IR(31783, 18)
  "MatrixTimesScalar\0" // IR(31801, 18)
  "VectorTimesMatrix\0" // IR(31819, 18)
  "MatrixTimesVector\0" // IR(31837, 18)
  "MatrixTimesMatrix\0" // IR(31855, 18)
  "OuterProduct\0" // IR(31873, 13)
  "Dot\0" // IR(31886, 4)
  "IAddCarry\0" // IR(31890, 10)
  "ISubBorrow\0" // IR(31900, 11)
  "UMulExtended\0" // IR(31911, 13)
  "SMulExtended\0" // IR(31924, 13)
  "Any\0" // IR(31937, 4)
  "All\0" // IR(31941, 4)
  "IsNan\0" // IR(31945, 6)
  "IsInf\0" // IR(31951, 6)
  "IsFinite\0" // IR(31957, 9)
  "IsNormal\0" // IR(31966, 9)
  "SignBitSet\0" // IR(31975, 11)
  "LessOrGreater\0" // IR(31986, 14)
  "Ordered\0" // IR(32000, 8)
  "Unordered\0" // IR(32008, 10)
  "LogicalEqual\0" // IR(32018, 13)
  "LogicalNotEqual\0" // IR(32031, 16)
  "LogicalOr\0" // IR(32047, 10)
  "LogicalAnd\0" // IR(32057, 11)
  "LogicalNot\0" // IR(32068, 11)
  "Select\0" // IR(32079, 7)
  "IEqual\0" // IR(32086, 7)
  "INotEqual\0" // IR(32093, 10)
  "UGreaterThan\0" // IR(32103, 13)
  "SGreaterThan\0" // IR(32116, 13)
  "UGreaterThanEqual\0" // IR(32129, 18)
  "SGreaterThanEqual\0" // IR(32147, 18)
  "ULessThan\0" // IR(32165, 10)
  "SLessThan\0" // IR(32175, 10)
  "ULessThanEqual\0" // IR(32185, 15)
  "SLessThanEqual\0" // IR(32200, 15)
  "FOrdEqual\0" // IR(32215, 10)
  "FUnordEqual\0" // IR(32225, 12)
  "FOrdNotEqual\0" // IR(32237, 13)
  "FUnordNotEqual\0" // IR(32250, 15)
  "FOrdLessThan\0" // IR(32265, 13)
  "FUnordLessThan\0" // IR(32278, 15)
  "FOrdGreaterThan\0" // IR(32293, 16)
  "FUnordGreaterThan\0" // IR(32309, 18)
  "FOrdLessThanEqual\0" // IR(32327, 18)
  "FUnordLessThanEqual\0" // IR(32345, 20)
  "FOrdGreaterThanEqual\0" // IR(32365, 21)
  "FUnordGreaterThanEqual\0" // IR(32386, 23)
  "ShiftRightLogical\0" // IR(32409, 18)
  "ShiftRightArithmetic\0" // IR(32427, 21)
  "ShiftLeftLogical\0" // IR(32448, 17)
  "BitwiseOr\0" // IR(32465, 10)
  "BitwiseXor\0" // IR(32475, 11)
  "BitwiseAnd\0" // IR(32486, 11)
  "Not\0" // IR(32497, 4)
  "BitFieldInsert\0" // IR(32501, 15)
  "BitFieldSExtract\0" // IR(32516, 17)
  "BitFieldUExtract\0" // IR(32533, 17)
  "BitReverse\0" // IR(32550, 11)
  "BitCount\0" // IR(32561, 9)
  "DPdx\0" // IR(32570, 5)
  "DPdy\0" // IR(32575, 5)
  "Fwidth\0" // IR(32580, 7)
  "DPdxFine\0" // IR(32587, 9)
  "DPdyFine\0" // IR(32596, 9)
  "FwidthFine\0" // IR(32605, 11)
  "DPdxCoarse\0" // IR(32616, 11)
  "DPdyCoarse\0" // IR(32627, 11)
  "FwidthCoarse\0" // IR(32638, 13)
  "EmitVertex\0" // IR(32651, 11)
  "EndPrimitive\0" // IR(32662, 13)
  "EmitStreamVertex\0" // IR(32675, 17)
  "EndStreamPrimitive\0" // IR(32692, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(32711, 37)
  "ControlBarrier\0" // IR(32748, 15)
  "MemoryBarrier\0" // IR(32763, 14)
  "AtomicLoad\0" // IR(32777, 11)
  "AtomicStore\0" // IR(32788, 12)
  "AtomicExchange\0" // IR(32800, 15)
  "AtomicCompareExchange\0" // IR(32815, 22)
  "AtomicCompareExchangeWeak\0" // IR(32837, 26)
  "AtomicIIncrement\0" // IR(32863, 17)
  "AtomicIDecrement\0" // IR(32880, 17)
  "AtomicIAdd\0" // IR(32897, 11)
  "AtomicISub\0" // IR(32908, 11)
  "AtomicSMin\0" // IR(32919, 11)
  "AtomicUMin\0" // IR(32930, 11)
  "AtomicSMax\0" // IR(32941, 11)
  "AtomicUMax\0" // IR(32952, 11)
  "AtomicAnd\0" // IR(32963, 10)
  "AtomicOr\0" // IR(32973, 9)
  "AtomicXor\0" // IR(32982, 10)
  "Phi\0" // IR(32992, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(32996, 30)
  "LoopMerge\0" // IR(33026, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(33036, 35)
  "SelectionMerge\0" // IR(33071, 15)
  "Label\0" // IR(33086, 6)
  "Branch\0" // IR(33092, 7)
  "BranchConditional\0" // IR(33099, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(33117, 45)
  "Switch\0" // IR(33162, 7)
  "Kill\0" // IR(33169, 5)
  "Return\0" // IR(33174, 7)
  "ReturnValue\0" // IR(33181, 12)
  "Unreachable\0" // IR(33193, 12)
  "LifetimeStart\0" // IR(33205, 14)
  "LifetimeStop\0" // IR(33219, 13)
  "GroupAsyncCopy\0" // IR(33232, 15)
  "GroupWaitEvents\0" // IR(33247, 16)
  "GroupAll\0" // IR(33263, 9)
  "GroupAny\0" // IR(33272, 9)
  "GroupBroadcast\0" // IR(33281, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(33296, 33)
  "GroupIAdd\0" // IR(33329, 10)
  "GroupFAdd\0" // IR(33339, 10)
  "GroupFMin\0" // IR(33349, 10)
  "GroupUMin\0" // IR(33359, 10)
  "GroupSMin\0" // IR(33369, 10)
  "GroupFMax\0" // IR(33379, 10)
  "GroupUMax\0" // IR(33389, 10)
  "GroupSMax\0" // IR(33399, 10)
  "ReadPipe\0" // IR(33409, 9)
  "WritePipe\0" // IR(33418, 10)
  "ReservedReadPipe\0" // IR(33428, 17)
  "ReservedWritePipe\0" // IR(33445, 18)
  "ReserveReadPipePackets\0" // IR(33463, 23)
  "ReserveWritePipePackets\0" // IR(33486, 24)
  "CommitReadPipe\0" // IR(33510, 15)
  "CommitWritePipe\0" // IR(33525, 16)
  "IsValidReserveId\0" // IR(33541, 17)
  "GetNumPipePackets\0" // IR(33558, 18)
  "GetMaxPipePackets\0" // IR(33576, 18)
  "GroupReserveReadPipePackets\0" // IR(33594, 28)
  "GroupReserveWritePipePackets\0" // IR(33622, 29)
  "GroupCommitReadPipe\0" // IR(33651, 20)
  "GroupCommitWritePipe\0" // IR(33671, 21)
  "EnqueueMarker\0" // IR(33692, 14)
  "EnqueueKernel\0" // IR(33706, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(33720, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(33750, 32)
  "GetKernelWorkGroupSize\0" // IR(33782, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(33805, 40)
  "RetainEvent\0" // IR(33845, 12)
  "ReleaseEvent\0" // IR(33857, 13)
  "CreateUserEvent\0" // IR(33870, 16)
  "IsValidEvent\0" // IR(33886, 13)
  "SetUserEventStatus\0" // IR(33899, 19)
  "CaptureEventProfilingInfo\0" // IR(33918, 26)
  "GetDefaultQueue\0" // IR(33944, 16)
  "BuildNDRange\0" // IR(33960, 13)
  "ImageSparseSampleImplicitLod\0" // IR(33973, 29)
  "ImageSparseSampleExplicitLod\0" // IR(34002, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(34031, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(34064, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(34097, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(34130, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(34163, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(34200, 37)
  "ImageSparseFetch\0" // IR(34237, 17)
  "ImageSparseGather\0" // IR(34254, 18)
  "ImageSparseDrefGather\0" // IR(34272, 22)
  "ImageSparseTexelsResident\0" // IR(34294, 26)
  "NoLine\0" // IR(34320, 7)
  "AtomicFlagTestAndSet\0" // IR(34327, 21)
  "AtomicFlagClear\0" // IR(34348, 16)
  "ImageSparseRead\0" // IR(34364, 16)
  "SizeOf\0" // IR(34380, 7)
  "TypePipeStorage\0" // IR(34387, 16)
  "ConstantPipeStorage\0" // IR(34403, 20)
  "CreatePipeFromPipeStorage\0" // IR(34423, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(34449, 35)
  "GetKernelMaxNumSubgroups\0" // IR(34484, 25)
  "TypeNamedBarrier\0" // IR(34509, 17)
  "NamedBarrierInitialize\0" // IR(34526, 23)
  "MemoryNamedBarrier\0" // IR(34549, 19)
  "ModuleProcessed\0" // IR(34568, 16)
  "ExecutionModeId\0" // IR(34584, 16)
  "DecorateId\0" // IR(34600, 11)
  "GroupNonUniformElect\0" // IR(34611, 21)
  "GroupNonUniformAll\0" // IR(34632, 19)
  "GroupNonUniformAny\0" // IR(34651, 19)
  "GroupNonUniformAllEqual\0" // IR(34670, 24)
  "GroupNonUniformBroadcast\0" // IR(34694, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(34719, 30)
  "GroupNonUniformInverseBallot\0" // IR(34749, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(34778, 32)
  "GroupNonUniformBallotBitCount\0" // IR(34810, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(34840, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(34869, 29)
  "GroupNonUniformShuffleXor\0" // IR(34898, 26)
  "GroupNonUniformShuffleUp\0" // IR(34924, 25)
  "GroupNonUniformShuffleDown\0" // IR(34949, 27)
  "GroupNonUniformIAdd\0" // IR(34976, 20)
  "GroupNonUniformFAdd\0" // IR(34996, 20)
  "GroupNonUniformIMul\0" // IR(35016, 20)
  "GroupNonUniformFMul\0" // IR(35036, 20)
  "GroupNonUniformSMin\0" // IR(35056, 20)
  "GroupNonUniformUMin\0" // IR(35076, 20)
  "GroupNonUniformFMin\0" // IR(35096, 20)
  "GroupNonUniformSMax\0" // IR(35116, 20)
  "GroupNonUniformUMax\0" // IR(35136, 20)
  "GroupNonUniformFMax\0" // IR(35156, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(35176, 26)
  "GroupNonUniformBitwiseOr\0" // IR(35202, 25)
  "GroupNonUniformBitwiseXor\0" // IR(35227, 26)
  "GroupNonUniformLogicalAnd\0" // IR(35253, 26)
  "GroupNonUniformLogicalOr\0" // IR(35279, 25)
  "GroupNonUniformLogicalXor\0" // IR(35304, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(35330, 29)
  "GroupNonUniformQuadSwap\0" // IR(35359, 24)
  "CopyLogical\0" // IR(35383, 12)
  "PtrEqual\0" // IR(35395, 9)
  "PtrNotEqual\0" // IR(35404, 12)
  "PtrDiff\0" // IR(35416, 8)
  "ColorAttachmentReadEXT\0" // IR(35424, 23)
  "DepthAttachmentReadEXT\0" // IR(35447, 23)
  "StencilAttachmentReadEXT\0" // IR(35470, 25)
  "TypeTensorARM\0" // IR(35495, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(35509, 42)
  "TensorReadARM\0" // IR(35551, 14)
  "TensorWriteARM\0" // IR(35565, 15)
  "TensorQuerySizeARM\0" // IR(35580, 19)
  "GraphConstantARM\0" // IR(35599, 17)
  "GraphEntryPointARM\0" // IR(35616, 19)
  "GraphInputARM\0" // IR(35635, 14)
  "GraphSetOutputARM\0" // IR(35649, 18)
  "GraphEndARM\0" // IR(35667, 12)
  "TypeGraphARM\0" // IR(35679, 13)
  "TerminateInvocation\0" // IR(35692, 20)
  "TypeUntypedPointerKHR\0" // IR(35712, 22)
  "UntypedVariableKHR\0" // IR(35734, 19)
  "UntypedAccessChainKHR\0" // IR(35753, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(35775, 30)
  "SubgroupFirstInvocationKHR\0" // IR(35805, 27)
  "UntypedPtrAccessChainKHR\0" // IR(35832, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(35857, 33)
  "UntypedArrayLengthKHR\0" // IR(35890, 22)
  "UntypedPrefetchKHR\0" // IR(35912, 19)
  "FmaKHR\0" // IR(35931, 7)
  "SubgroupAllKHR\0" // IR(35938, 15)
  "SubgroupAnyKHR\0" // IR(35953, 15)
  "SubgroupAllEqualKHR\0" // IR(35968, 20)
  "SubgroupReadInvocationKHR\0" // IR(35988, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(36014, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(36040, 25)
  "TraceRayKHR\0" // IR(36065, 12)
  "ExecuteCallableKHR\0" // IR(36077, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(36096, 35)
  "IgnoreIntersectionKHR\0" // IR(36131, 22)
  "TerminateRayKHR\0" // IR(36153, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(36169, 47)
  "SDot\0" // IR(36216, 5)
  "SDotKHR\0" // IR(36221, 8)
  "UDot\0" // IR(36229, 5)
  "UDotKHR\0" // IR(36234, 8)
  "SUDot\0" // IR(36242, 6)
  "SUDotKHR\0" // IR(36248, 9)
  "SDotAccSat\0" // IR(36257, 11)
  "SDotAccSatKHR\0" // IR(36268, 14)
  "UDotAccSat\0" // IR(36282, 11)
  "UDotAccSatKHR\0" // IR(36293, 14)
  "SUDotAccSat\0" // IR(36307, 12)
  "SUDotAccSatKHR\0" // IR(36319, 15)
  "TypeCooperativeMatrixKHR\0" // IR(36334, 25)
  "CooperativeMatrixLoadKHR\0" // IR(36359, 25)
  "CooperativeMatrixStoreKHR\0" // IR(36384, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(36410, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(36464, 27)
  "CooperativeMatrixLengthKHR\0" // IR(36491, 27)
  "ConstantCompositeReplicateEXT\0" // IR(36518, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(36548, 34)
  "CompositeConstructReplicateEXT\0" // IR(36582, 31)
  "TypeRayQueryKHR\0" // IR(36613, 16)
  "RayQueryInitializeKHR\0" // IR(36629, 22)
  "RayQueryTerminateKHR\0" // IR(36651, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(36672, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(36704, 31)
  "RayQueryProceedKHR\0" // IR(36735, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(36754, 31)
  "ImageSampleWeightedQCOM\0" // IR(36785, 24)
  "ImageBoxFilterQCOM\0" // IR(36809, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(36828, 23)
  "ImageBlockMatchSADQCOM\0" // IR(36851, 23)
  "BitCastArrayQCOM\0" // IR(36874, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(36891, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(36920, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(36949, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(36978, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(37007, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(37037, 28)
  "ExtractSubArrayQCOM\0" // IR(37065, 20)
  "ImageGatherQCOM\0" // IR(37085, 16)
  "GroupIAddNonUniformAMD\0" // IR(37101, 23)
  "GroupFAddNonUniformAMD\0" // IR(37124, 23)
  "GroupFMinNonUniformAMD\0" // IR(37147, 23)
  "GroupUMinNonUniformAMD\0" // IR(37170, 23)
  "GroupSMinNonUniformAMD\0" // IR(37193, 23)
  "GroupFMaxNonUniformAMD\0" // IR(37216, 23)
  "GroupUMaxNonUniformAMD\0" // IR(37239, 23)
  "GroupSMaxNonUniformAMD\0" // IR(37262, 23)
  "FragmentMaskFetchAMD\0" // IR(37285, 21)
  "FragmentFetchAMD\0" // IR(37306, 17)
  "ReadClockKHR\0" // IR(37323, 13)
  "AllocateNodePayloadsAMDX\0" // IR(37336, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(37361, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(37385, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(37410, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(37439, 27)
  "IsNodePayloadValidAMDX\0" // IR(37466, 23)
  "ConstantStringAMDX\0" // IR(37489, 19)
  "SpecConstantStringAMDX\0" // IR(37508, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(37531, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(37557, 26)
  "TypeBufferEXT\0" // IR(37583, 14)
  "BufferPointerEXT\0" // IR(37597, 17)
  "UntypedImageTexelPointerEXT\0" // IR(37614, 28)
  "MemberDecorateIdEXT\0" // IR(37642, 20)
  "ConstantSizeOfEXT\0" // IR(37662, 18)
  "SpecConstantDataKHR\0" // IR(37680, 20)
  "PoisonKHR\0" // IR(37700, 10)
  "FreezeKHR\0" // IR(37710, 10)
  "HitObjectRecordHitMotionNV\0" // IR(37720, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(37747, 36)
  "HitObjectRecordMissMotionNV\0" // IR(37783, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(37811, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(37839, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(37867, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(37900, 30)
  "HitObjectTraceRayMotionNV\0" // IR(37930, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(37956, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(37995, 44)
  "HitObjectRecordEmptyNV\0" // IR(38039, 23)
  "HitObjectTraceRayNV\0" // IR(38062, 20)
  "HitObjectRecordHitNV\0" // IR(38082, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(38103, 30)
  "HitObjectRecordMissNV\0" // IR(38133, 22)
  "HitObjectExecuteShaderNV\0" // IR(38155, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(38180, 26)
  "HitObjectGetAttributesNV\0" // IR(38206, 25)
  "HitObjectGetHitKindNV\0" // IR(38231, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(38253, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(38282, 28)
  "HitObjectGetInstanceIdNV\0" // IR(38310, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(38335, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(38369, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(38401, 29)
  "HitObjectGetRayTMaxNV\0" // IR(38430, 22)
  "HitObjectGetRayTMinNV\0" // IR(38452, 22)
  "HitObjectIsEmptyNV\0" // IR(38474, 19)
  "HitObjectIsHitNV\0" // IR(38493, 17)
  "HitObjectIsMissNV\0" // IR(38510, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(38528, 29)
  "ReorderThreadWithHintNV\0" // IR(38557, 24)
  "TypeHitObjectNV\0" // IR(38581, 16)
  "ImageSampleFootprintNV\0" // IR(38597, 23)
  "TypeVectorIdEXT\0" // IR(38620, 16)
  "TypeCooperativeVectorNV\0" // IR(38636, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(38660, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(38689, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(38731, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(38770, 32)
  "CooperativeMatrixConvertNV\0" // IR(38802, 27)
  "EmitMeshTasksEXT\0" // IR(38829, 17)
  "SetMeshOutputsEXT\0" // IR(38846, 18)
  "GroupNonUniformPartitionEXT\0" // IR(38864, 28)
  "GroupNonUniformPartitionNV\0" // IR(38892, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(38919, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(38952, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(38987, 38)
  "CooperativeVectorLoadNV\0" // IR(39025, 24)
  "CooperativeVectorStoreNV\0" // IR(39049, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(39074, 28)
  "HitObjectRecordMissEXT\0" // IR(39102, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(39125, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(39154, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(39205, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(39229, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(39274, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(39307, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(39339, 38)
  "TypeHitObjectEXT\0" // IR(39377, 17)
  "ReorderThreadWithHintEXT\0" // IR(39394, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(39419, 30)
  "HitObjectTraceRayEXT\0" // IR(39449, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(39470, 27)
  "HitObjectRecordEmptyEXT\0" // IR(39497, 24)
  "HitObjectExecuteShaderEXT\0" // IR(39521, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(39547, 27)
  "HitObjectGetAttributesEXT\0" // IR(39574, 26)
  "HitObjectGetHitKindEXT\0" // IR(39600, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(39623, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(39653, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(39682, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(39708, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(39743, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(39774, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(39808, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(39841, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(39871, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(39900, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(39929, 23)
  "ReportIntersectionKHR\0" // IR(39952, 22)
  "ReportIntersectionNV\0" // IR(39974, 21)
  "IgnoreIntersectionNV\0" // IR(39995, 21)
  "TerminateRayNV\0" // IR(40016, 15)
  "TraceNV\0" // IR(40031, 8)
  "TraceMotionNV\0" // IR(40039, 14)
  "TraceRayMotionNV\0" // IR(40053, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(40070, 50)
  "TypeAccelerationStructureKHR\0" // IR(40120, 29)
  "TypeAccelerationStructureNV\0" // IR(40149, 28)
  "ExecuteCallableNV\0" // IR(40177, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(40195, 35)
  "RayQueryGetClusterIdNV\0" // IR(40230, 23)
  "HitObjectGetClusterIdNV\0" // IR(40253, 24)
  "HitObjectGetRayTMinEXT\0" // IR(40277, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(40300, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(40345, 40)
  "HitObjectIsEmptyEXT\0" // IR(40385, 20)
  "HitObjectIsHitEXT\0" // IR(40405, 18)
  "HitObjectIsMissEXT\0" // IR(40423, 19)
  "TypeCooperativeMatrixNV\0" // IR(40442, 24)
  "CooperativeMatrixLoadNV\0" // IR(40466, 24)
  "CooperativeMatrixStoreNV\0" // IR(40490, 25)
  "CooperativeMatrixMulAddNV\0" // IR(40515, 26)
  "CooperativeMatrixLengthNV\0" // IR(40541, 26)
  "BeginInvocationInterlockEXT\0" // IR(40567, 28)
  "EndInvocationInterlockEXT\0" // IR(40595, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(40621, 43)
  "CooperativeMatrixReduceNV\0" // IR(40664, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(40690, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(40721, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(40765, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(40795, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(40826, 32)
  "TypeTensorLayoutNV\0" // IR(40858, 19)
  "TypeTensorViewNV\0" // IR(40877, 17)
  "CreateTensorLayoutNV\0" // IR(40894, 21)
  "TensorLayoutSetDimensionNV\0" // IR(40915, 27)
  "TensorLayoutSetStrideNV\0" // IR(40942, 24)
  "TensorLayoutSliceNV\0" // IR(40966, 20)
  "TensorLayoutSetClampValueNV\0" // IR(40986, 28)
  "CreateTensorViewNV\0" // IR(41014, 19)
  "TensorViewSetDimensionNV\0" // IR(41033, 25)
  "TensorViewSetStrideNV\0" // IR(41058, 22)
  "IsHelperInvocationEXT\0" // IR(41080, 22)
  "TensorViewSetClipNV\0" // IR(41102, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(41122, 27)
  "CooperativeMatrixTransposeNV\0" // IR(41149, 29)
  "ConvertUToImageNV\0" // IR(41178, 18)
  "ConvertUToSamplerNV\0" // IR(41196, 20)
  "ConvertImageToUNV\0" // IR(41216, 18)
  "ConvertSamplerToUNV\0" // IR(41234, 20)
  "ConvertUToSampledImageNV\0" // IR(41254, 25)
  "ConvertSampledImageToUNV\0" // IR(41279, 25)
  "SamplerImageAddressingModeNV\0" // IR(41304, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(41333, 52)
  "RawAccessChainNV\0" // IR(41385, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(41402, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(41442, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(41480, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(41518, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(41552, 37)
  "HitObjectGetSpherePositionNV\0" // IR(41589, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(41618, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(41645, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(41672, 23)
  "HitObjectIsSphereHitNV\0" // IR(41695, 23)
  "HitObjectIsLSSHitNV\0" // IR(41718, 20)
  "RayQueryIsSphereHitNV\0" // IR(41738, 22)
  "RayQueryIsLSSHitNV\0" // IR(41760, 19)
  "SubgroupShuffleDownINTEL\0" // IR(41779, 25)
  "SubgroupShuffleUpINTEL\0" // IR(41804, 23)
  "SubgroupShuffleXorINTEL\0" // IR(41827, 24)
  "SubgroupBlockReadINTEL\0" // IR(41851, 23)
  "SubgroupBlockWriteINTEL\0" // IR(41874, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(41898, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(41926, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(41955, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(41988, 34)
  "UCountLeadingZerosINTEL\0" // IR(42022, 24)
  "UCountTrailingZerosINTEL\0" // IR(42046, 25)
  "AbsISubINTEL\0" // IR(42071, 13)
  "AbsUSubINTEL\0" // IR(42084, 13)
  "IAddSatINTEL\0" // IR(42097, 13)
  "UAddSatINTEL\0" // IR(42110, 13)
  "IAverageINTEL\0" // IR(42123, 14)
  "UAverageINTEL\0" // IR(42137, 14)
  "IAverageRoundedINTEL\0" // IR(42151, 21)
  "UAverageRoundedINTEL\0" // IR(42172, 21)
  "ISubSatINTEL\0" // IR(42193, 13)
  "USubSatINTEL\0" // IR(42206, 13)
  "IMul32x16INTEL\0" // IR(42219, 15)
  "UMul32x16INTEL\0" // IR(42234, 15)
  "ConstantFunctionPointerINTEL\0" // IR(42249, 29)
  "FunctionPointerCallINTEL\0" // IR(42278, 25)
  "AsmTargetINTEL\0" // IR(42303, 15)
  "AsmCallINTEL\0" // IR(42318, 13)
  "AtomicFMinEXT\0" // IR(42331, 14)
  "AtomicFMaxEXT\0" // IR(42345, 14)
  "AssumeTrueKHR\0" // IR(42359, 14)
  "ExpectKHR\0" // IR(42373, 10)
  "DecorateString\0" // IR(42383, 15)
  "DecorateStringGOOGLE\0" // IR(42398, 21)
  "MemberDecorateString\0" // IR(42419, 21)
  "MemberDecorateStringGOOGLE\0" // IR(42440, 27)
  "VmeImageINTEL\0" // IR(42467, 14)
  "TypeVmeImageINTEL\0" // IR(42481, 18)
  "TypeAvcImePayloadINTEL\0" // IR(42499, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(42522, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(42545, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(42568, 23)
  "TypeAvcMceResultINTEL\0" // IR(42591, 22)
  "TypeAvcImeResultINTEL\0" // IR(42613, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(42635, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(42681, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(42725, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(42764, 37)
  "TypeAvcRefResultINTEL\0" // IR(42801, 22)
  "TypeAvcSicResultINTEL\0" // IR(42823, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(42845, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(42905, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(42958, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(43005, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(43045, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(43096, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(43140, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(43191, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(43247, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(43297, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(43349, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(43398, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(43445, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(43495, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(43546, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(43602, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(43635, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(43687, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(43748, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(43809, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(43848, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(43886, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(43925, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(43963, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(44002, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(44040, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(44076, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(44115, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(44158, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(44196, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(44234, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(44272, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(44317, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44357, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(44419, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(44449, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(44487, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(44523, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(44556, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(44591, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(44630, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(44673, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(44720, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(44774, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(44808, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(44855, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(44900, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(44955, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(45008, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(45064, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(45118, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(45176, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(45232, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(45270, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(45316, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(45360, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(45409, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(45456, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(45526, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(45594, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(45663, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(45731, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(45797, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(45864, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(45900, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(45948, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(46007, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(46065, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(46121, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(46151, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(46181, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(46220, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(46266, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(46309, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(46356, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(46401, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(46447, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(46503, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(46541, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(46571, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(46603, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(46639, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(46681, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(46720, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(46759, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(46803, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(46851, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(46901, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(46944, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(46992, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(47035, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(47066, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(47113, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(47158, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(47204, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(47260, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(47298, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(47333, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(47377, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(47423, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(47464, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(47500, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(47550, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(47598, 35)
  "SaveMemoryINTEL\0" // IR(47633, 16)
  "RestoreMemoryINTEL\0" // IR(47649, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(47668, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(47697, 28)
  "ArbitraryFloatCastALTERA\0" // IR(47725, 25)
  "ArbitraryFloatCastINTEL\0" // IR(47750, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(47774, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(47806, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(47837, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(47867, 29)
  "ArbitraryFloatAddALTERA\0" // IR(47896, 24)
  "ArbitraryFloatAddINTEL\0" // IR(47920, 23)
  "ArbitraryFloatSubALTERA\0" // IR(47943, 24)
  "ArbitraryFloatSubINTEL\0" // IR(47967, 23)
  "ArbitraryFloatMulALTERA\0" // IR(47990, 24)
  "ArbitraryFloatMulINTEL\0" // IR(48014, 23)
  "ArbitraryFloatDivALTERA\0" // IR(48037, 24)
  "ArbitraryFloatDivINTEL\0" // IR(48061, 23)
  "ArbitraryFloatGTALTERA\0" // IR(48084, 23)
  "ArbitraryFloatGTINTEL\0" // IR(48107, 22)
  "ArbitraryFloatGEALTERA\0" // IR(48129, 23)
  "ArbitraryFloatGEINTEL\0" // IR(48152, 22)
  "ArbitraryFloatLTALTERA\0" // IR(48174, 23)
  "ArbitraryFloatLTINTEL\0" // IR(48197, 22)
  "ArbitraryFloatLEALTERA\0" // IR(48219, 23)
  "ArbitraryFloatLEINTEL\0" // IR(48242, 22)
  "ArbitraryFloatEQALTERA\0" // IR(48264, 23)
  "ArbitraryFloatEQINTEL\0" // IR(48287, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(48309, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(48335, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(48360, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(48386, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(48411, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(48436, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(48460, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(48486, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(48511, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(48536, 24)
  "ArbitraryFloatLogINTEL\0" // IR(48560, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(48583, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(48607, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(48632, 25)
  "ArbitraryFloatExpINTEL\0" // IR(48657, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(48680, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(48704, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(48729, 25)
  "ArbitraryFloatSinINTEL\0" // IR(48754, 23)
  "ArbitraryFloatCosINTEL\0" // IR(48777, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(48800, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(48826, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(48851, 25)
  "ArbitraryFloatASinINTEL\0" // IR(48876, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(48900, 26)
  "ArbitraryFloatACosINTEL\0" // IR(48926, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(48950, 26)
  "ArbitraryFloatATanINTEL\0" // IR(48976, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(49000, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(49026, 25)
  "ArbitraryFloatPowINTEL\0" // IR(49051, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(49074, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(49098, 24)
  "LoopControlINTEL\0" // IR(49122, 17)
  "AliasDomainDeclINTEL\0" // IR(49139, 21)
  "AliasScopeDeclINTEL\0" // IR(49160, 20)
  "AliasScopeListDeclINTEL\0" // IR(49180, 24)
  "FixedSqrtALTERA\0" // IR(49204, 16)
  "FixedSqrtINTEL\0" // IR(49220, 15)
  "FixedRecipALTERA\0" // IR(49235, 17)
  "FixedRecipINTEL\0" // IR(49252, 16)
  "FixedRsqrtALTERA\0" // IR(49268, 17)
  "FixedRsqrtINTEL\0" // IR(49285, 16)
  "FixedSinALTERA\0" // IR(49301, 15)
  "FixedSinINTEL\0" // IR(49316, 14)
  "FixedCosALTERA\0" // IR(49330, 15)
  "FixedCosINTEL\0" // IR(49345, 14)
  "FixedSinCosALTERA\0" // IR(49359, 18)
  "FixedSinCosINTEL\0" // IR(49377, 17)
  "FixedSinPiALTERA\0" // IR(49394, 17)
  "FixedSinPiINTEL\0" // IR(49411, 16)
  "FixedCosPiALTERA\0" // IR(49427, 17)
  "FixedCosPiINTEL\0" // IR(49444, 16)
  "FixedSinCosPiALTERA\0" // IR(49460, 20)
  "FixedSinCosPiINTEL\0" // IR(49480, 19)
  "FixedLogALTERA\0" // IR(49499, 15)
  "FixedLogINTEL\0" // IR(49514, 14)
  "FixedExpALTERA\0" // IR(49528, 15)
  "FixedExpINTEL\0" // IR(49543, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(49557, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(49587, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(49616, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(49646, 29)
  "ReadPipeBlockingALTERA\0" // IR(49675, 23)
  "ReadPipeBlockingINTEL\0" // IR(49698, 22)
  "WritePipeBlockingALTERA\0" // IR(49720, 24)
  "WritePipeBlockingINTEL\0" // IR(49744, 23)
  "RayQueryGetRayTMinKHR\0" // IR(49767, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(49789, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(49812, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(49840, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(49886, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(49923, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(49988, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(50028, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(50069, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(50108, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(50144, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(50190, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(50235, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(50277, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(50309, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(50338, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(50378, 40)
  "AtomicFAddEXT\0" // IR(50418, 14)
  "TypeBufferSurfaceINTEL\0" // IR(50432, 23)
  "TypeStructContinuedINTEL\0" // IR(50455, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(50480, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(50512, 36)
  "CompositeConstructContinuedINTEL\0" // IR(50548, 33)
  "ConvertFToBF16INTEL\0" // IR(50581, 20)
  "ConvertBF16ToFINTEL\0" // IR(50601, 20)
  "ControlBarrierArriveEXT\0" // IR(50621, 24)
  "ControlBarrierArriveINTEL\0" // IR(50645, 26)
  "ControlBarrierWaitEXT\0" // IR(50671, 22)
  "ControlBarrierWaitINTEL\0" // IR(50693, 24)
  "TaskSequenceCreateALTERA\0" // IR(50717, 25)
  "TaskSequenceCreateINTEL\0" // IR(50742, 24)
  "TaskSequenceAsyncALTERA\0" // IR(50766, 24)
  "TaskSequenceAsyncINTEL\0" // IR(50790, 23)
  "TaskSequenceGetALTERA\0" // IR(50813, 22)
  "TaskSequenceGetINTEL\0" // IR(50835, 21)
  "TaskSequenceReleaseALTERA\0" // IR(50856, 26)
  "TaskSequenceReleaseINTEL\0" // IR(50882, 25)
  "TypeTaskSequenceALTERA\0" // IR(50907, 23)
  "TypeTaskSequenceINTEL\0" // IR(50930, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(50952, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(50979, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(51004, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(51038, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(51072, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(51101, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(51127, 62)
  "BitwiseFunctionINTEL\0" // IR(51189, 21)
  "ConditionalExtensionINTEL\0" // IR(51210, 26)
  "ConditionalEntryPointINTEL\0" // IR(51236, 27)
  "ConditionalCapabilityINTEL\0" // IR(51263, 27)
  "SpecConstantTargetINTEL\0" // IR(51290, 24)
  "SpecConstantArchitectureINTEL\0" // IR(51314, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(51344, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(51381, 30)
  "ConditionalCopyObjectINTEL\0" // IR(51411, 27)
  "PredicatedLoadINTEL\0" // IR(51438, 20)
  "PredicatedStoreINTEL\0" // IR(51458, 21)
  "GroupIMulKHR\0" // IR(51479, 13)
  "GroupFMulKHR\0" // IR(51492, 13)
  "GroupBitwiseAndKHR\0" // IR(51505, 19)
  "GroupBitwiseOrKHR\0" // IR(51524, 18)
  "GroupBitwiseXorKHR\0" // IR(51542, 19)
  "GroupLogicalAndKHR\0" // IR(51561, 19)
  "GroupLogicalOrKHR\0" // IR(51580, 18)
  "GroupLogicalXorKHR\0" // IR(51598, 19)
  "RoundFToTF32INTEL\0" // IR(51617, 18)
  "MaskedGatherINTEL\0" // IR(51635, 18)
  "MaskedScatterINTEL\0" // IR(51653, 19)
  "ConvertHandleToImageINTEL\0" // IR(51672, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(51698, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(51726, 33)
  "FDot2MixAcc32VALVE\0" // IR(51759, 19)
  "FDot2MixAcc16VALVE\0" // IR(51778, 19)
  "FDot4MixAcc32VALVE\0" // IR(51797, 19)
  "CALL\0" // IR(51816, 5)
  "MIN_SAD\0" // IR(51821, 8)
  "MIN_SAD_COST\0" // IR(51829, 13)
  "RAW_SAD\0" // IR(51842, 8)
  "DebugInfoNone\0" // IR(51850, 14)
  "DebugCompilationUnit\0" // IR(51864, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(51885, 52)
  "DebugTypeBasic\0" // IR(51937, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(51952, 34)
  "DebugTypePointer\0" // IR(51986, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(52003, 38)
  "DebugTypeQualifier\0" // IR(52041, 19)
  "DebugTypeArray\0" // IR(52060, 15)
  "DebugTypeVector\0" // IR(52075, 16)
  "DebugTypedef\0" // IR(52091, 13)
  "DebugTypeFunction\0" // IR(52104, 18)
  "DebugTypeEnum\0" // IR(52122, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(52136, 38)
  "DebugTypeComposite\0" // IR(52174, 19)
  "DebugTypeMember\0" // IR(52193, 16)
  "DebugTypeInheritance\0" // IR(52209, 21)
  "DebugTypePtrToMember\0" // IR(52230, 21)
  "DebugTypeTemplate\0" // IR(52251, 18)
  "DebugTypeTemplateParameter\0" // IR(52269, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(52296, 35)
  "DebugTypeTemplateParameterPack\0" // IR(52331, 31)
  "DebugGlobalVariable\0" // IR(52362, 20)
  "DebugFunctionDeclaration\0" // IR(52382, 25)
  "DebugFunction\0" // IR(52407, 14)
  "DebugLexicalBlock\0" // IR(52421, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(52439, 31)
  "DebugScope\0" // IR(52470, 11)
  "DebugNoScope\0" // IR(52481, 13)
  "DebugInlinedAt\0" // IR(52494, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(52509, 42)
  "DebugLocalVariable\0" // IR(52551, 19)
  "DebugInlinedVariable\0" // IR(52570, 21)
  "DebugDeclare\0" // IR(52591, 13)
  "DebugValue\0" // IR(52604, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(52615, 33)
  "DebugOperation\0" // IR(52648, 15)
  "DebugExpression\0" // IR(52663, 16)
  "DebugMacroDef\0" // IR(52679, 14)
  "DebugMacroUndef\0" // IR(52693, 16)
  "Round\0" // IR(52709, 6)
  "RoundEven\0" // IR(52715, 10)
  "Trunc\0" // IR(52725, 6)
  "FAbs\0" // IR(52731, 5)
  "SAbs\0" // IR(52736, 5)
  "FSign\0" // IR(52741, 6)
  "SSign\0" // IR(52747, 6)
  "Floor\0" // IR(52753, 6)
  "Ceil\0" // IR(52759, 5)
  "Fract\0" // IR(52764, 6)
  "Radians\0" // IR(52770, 8)
  "Degrees\0" // IR(52778, 8)
  "Sin\0" // IR(52786, 4)
  "Cos\0" // IR(52790, 4)
  "Tan\0" // IR(52794, 4)
  "Asin\0" // IR(52798, 5)
  "Acos\0" // IR(52803, 5)
  "Atan\0" // IR(52808, 5)
  "Sinh\0" // IR(52813, 5)
  "Cosh\0" // IR(52818, 5)
  "Tanh\0" // IR(52823, 5)
  "Asinh\0" // IR(52828, 6)
  "Acosh\0" // IR(52834, 6)
  "Atanh\0" // IR(52840, 6)
  "Atan2\0" // IR(52846, 6)
  "Pow\0" // IR(52852, 4)
  "Exp\0" // IR(52856, 4)
  "Log\0" // IR(52860, 4)
  "Exp2\0" // IR(52864, 5)
  "Log2\0" // IR(52869, 5)
  "Sqrt\0" // IR(52874, 5)
  "InverseSqrt\0" // IR(52879, 12)
  "Determinant\0" // IR(52891, 12)
  "MatrixInverse\0" // IR(52903, 14)
  "Modf\0" // IR(52917, 5)
  "ModfStruct\0" // IR(52922, 11)
  "FMin\0" // IR(52933, 5)
  "UMin\0" // IR(52938, 5)
  "SMin\0" // IR(52943, 5)
  "FMax\0" // IR(52948, 5)
  "UMax\0" // IR(52953, 5)
  "SMax\0" // IR(52958, 5)
  "FClamp\0" // IR(52963, 7)
  "UClamp\0" // IR(52970, 7)
  "SClamp\0" // IR(52977, 7)
  "FMix\0" // IR(52984, 5)
  "IMix\0" // IR(52989, 5)
  "Step\0" // IR(52994, 5)
  "SmoothStep\0" // IR(52999, 11)
  "Fma\0" // IR(53010, 4)
  "Frexp\0" // IR(53014, 6)
  "FrexpStruct\0" // IR(53020, 12)
  "Ldexp\0" // IR(53032, 6)
  "PackSnorm4x8\0" // IR(53038, 13)
  "PackUnorm4x8\0" // IR(53051, 13)
  "PackSnorm2x16\0" // IR(53064, 14)
  "PackUnorm2x16\0" // IR(53078, 14)
  "PackHalf2x16\0" // IR(53092, 13)
  "PackDouble2x32\0" // IR(53105, 15)
  "UnpackSnorm2x16\0" // IR(53120, 16)
  "UnpackUnorm2x16\0" // IR(53136, 16)
  "UnpackHalf2x16\0" // IR(53152, 15)
  "UnpackSnorm4x8\0" // IR(53167, 15)
  "UnpackUnorm4x8\0" // IR(53182, 15)
  "UnpackDouble2x32\0" // IR(53197, 17)
  "Length\0" // IR(53214, 7)
  "Distance\0" // IR(53221, 9)
  "Cross\0" // IR(53230, 6)
  "Normalize\0" // IR(53236, 10)
  "FaceForward\0" // IR(53246, 12)
  "Reflect\0" // IR(53258, 8)
  "Refract\0" // IR(53266, 8)
  "FindILsb\0" // IR(53274, 9)
  "FindSMsb\0" // IR(53283, 9)
  "FindUMsb\0" // IR(53292, 9)
  "InterpolateAtCentroid\0" // IR(53301, 22)
  "InterpolateAtSample\0" // IR(53323, 20)
  "InterpolateAtOffset\0" // IR(53343, 20)
  "NMin\0" // IR(53363, 5)
  "NMax\0" // IR(53368, 5)
  "NClamp\0" // IR(53373, 7)
  "ArgumentInfo\0" // IR(53380, 13)
  "ArgumentStorageBuffer\0" // IR(53393, 22)
  "ArgumentUniform\0" // IR(53415, 16)
  "ArgumentPodStorageBuffer\0" // IR(53431, 25)
  "ArgumentPodUniform\0" // IR(53456, 19)
  "ArgumentPodPushConstant\0" // IR(53475, 24)
  "ArgumentSampledImage\0" // IR(53499, 21)
  "ArgumentStorageImage\0" // IR(53520, 21)
  "ArgumentSampler\0" // IR(53541, 16)
  "ArgumentWorkgroup\0" // IR(53557, 18)
  "SpecConstantWorkgroupSize\0" // IR(53575, 26)
  "SpecConstantGlobalOffset\0" // IR(53601, 25)
  "SpecConstantWorkDim\0" // IR(53626, 20)
  "PushConstantGlobalOffset\0" // IR(53646, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(53671, 30)
  "PushConstantGlobalSize\0" // IR(53701, 23)
  "PushConstantRegionOffset\0" // IR(53724, 25)
  "PushConstantNumWorkgroups\0" // IR(53749, 26)
  "PushConstantRegionGroupOffset\0" // IR(53775, 30)
  "ConstantDataStorageBuffer\0" // IR(53805, 26)
  "ConstantDataUniform\0" // IR(53831, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(53851, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(53881, 28)
  "ArgumentPointerPushConstant\0" // IR(53909, 28)
  "ArgumentPointerUniform\0" // IR(53937, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(53960, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(53995, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(54033, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(54075, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(54120, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(54157, 40)
  "ArgumentStorageTexelBuffer\0" // IR(54197, 27)
  "ArgumentUniformTexelBuffer\0" // IR(54224, 27)
  "ConstantDataPointerPushConstant\0" // IR(54251, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(54283, 40)
  "PrintfInfo\0" // IR(54323, 11)
  "PrintfBufferStorageBuffer\0" // IR(54334, 26)
  "PrintfBufferPointerPushConstant\0" // IR(54360, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(54392, 34)
  "WorkgroupVariableSize\0" // IR(54426, 22)
  "DebugGraph\0" // IR(54448, 11)
  "DebugTensor\0" // IR(54459, 12)
  "DebugImportedEntity\0" // IR(54471, 20)
  "DebugSource\0" // IR(54491, 12)
  "DebugFunctionDefinition\0" // IR(54503, 24)
  "DebugSourceContinued\0" // IR(54527, 21)
  "DebugLine\0" // IR(54548, 10)
  "DebugNoLine\0" // IR(54558, 12)
  "DebugBuildIdentifier\0" // IR(54570, 21)
  "DebugStoragePath\0" // IR(54591, 17)
  "DebugEntryPoint\0" // IR(54608, 16)
  "DebugTypeMatrix\0" // IR(54624, 16)
  "DebugTypeVectorIdEXT\0" // IR(54640, 21)
  "DebugTypeCooperativeMatrixKHR\0" // IR(54661, 30)
  "Configuration\0" // IR(54691, 14)
  "StartCounter\0" // IR(54705, 13)
  "StopCounter\0" // IR(54718, 12)
  "PushConstants\0" // IR(54730, 14)
  "SpecializationMapEntry\0" // IR(54744, 23)
  "DescriptorSetBuffer\0" // IR(54767, 20)
  "DescriptorSetImage\0" // IR(54787, 19)
  "DescriptorSetSampler\0" // IR(54806, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(54827, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(54890, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(54935, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(54984, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(55033, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(55077, 50)
  "DebugModuleINTEL\0" // IR(55127, 17)
  "acos\0" // IR(55144, 5)
  "acosh\0" // IR(55149, 6)
  "acospi\0" // IR(55155, 7)
  "asin\0" // IR(55162, 5)
  "asinh\0" // IR(55167, 6)
  "asinpi\0" // IR(55173, 7)
  "atan\0" // IR(55180, 5)
  "atan2\0" // IR(55185, 6)
  "atanh\0" // IR(55191, 6)
  "atanpi\0" // IR(55197, 7)
  "atan2pi\0" // IR(55204, 8)
  "cbrt\0" // IR(55212, 5)
  "ceil\0" // IR(55217, 5)
  "copysign\0" // IR(55222, 9)
  "cos\0" // IR(55231, 4)
  "cosh\0" // IR(55235, 5)
  "cospi\0" // IR(55240, 6)
  "erfc\0" // IR(55246, 5)
  "erf\0" // IR(55251, 4)
  "exp\0" // IR(55255, 4)
  "exp2\0" // IR(55259, 5)
  "exp10\0" // IR(55264, 6)
  "expm1\0" // IR(55270, 6)
  "fabs\0" // IR(55276, 5)
  "fdim\0" // IR(55281, 5)
  "floor\0" // IR(55286, 6)
  "fma\0" // IR(55292, 4)
  "fmax\0" // IR(55296, 5)
  "fmin\0" // IR(55301, 5)
  "fmod\0" // IR(55306, 5)
  "fract\0" // IR(55311, 6)
  "frexp\0" // IR(55317, 6)
  "hypot\0" // IR(55323, 6)
  "ilogb\0" // IR(55329, 6)
  "ldexp\0" // IR(55335, 6)
  "lgamma\0" // IR(55341, 7)
  "lgamma_r\0" // IR(55348, 9)
  "log\0" // IR(55357, 4)
  "log2\0" // IR(55361, 5)
  "log10\0" // IR(55366, 6)
  "log1p\0" // IR(55372, 6)
  "logb\0" // IR(55378, 5)
  "mad\0" // IR(55383, 4)
  "maxmag\0" // IR(55387, 7)
  "minmag\0" // IR(55394, 7)
  "modf\0" // IR(55401, 5)
  "nan\0" // IR(55406, 4)
  "nextafter\0" // IR(55410, 10)
  "pow\0" // IR(55420, 4)
  "pown\0" // IR(55424, 5)
  "powr\0" // IR(55429, 5)
  "remainder\0" // IR(55434, 10)
  "remquo\0" // IR(55444, 7)
  "rint\0" // IR(55451, 5)
  "rootn\0" // IR(55456, 6)
  "round\0" // IR(55462, 6)
  "rsqrt\0" // IR(55468, 6)
  "sin\0" // IR(55474, 4)
  "sincos\0" // IR(55478, 7)
  "sinh\0" // IR(55485, 5)
  "sinpi\0" // IR(55490, 6)
  "sqrt\0" // IR(55496, 5)
  "tan\0" // IR(55501, 4)
  "tanh\0" // IR(55505, 5)
  "tanpi\0" // IR(55510, 6)
  "tgamma\0" // IR(55516, 7)
  "trunc\0" // IR(55523, 6)
  "half_cos\0" // IR(55529, 9)
  "half_divide\0" // IR(55538, 12)
  "half_exp\0" // IR(55550, 9)
  "half_exp2\0" // IR(55559, 10)
  "half_exp10\0" // IR(55569, 11)
  "half_log\0" // IR(55580, 9)
  "half_log2\0" // IR(55589, 10)
  "half_log10\0" // IR(55599, 11)
  "half_powr\0" // IR(55610, 10)
  "half_recip\0" // IR(55620, 11)
  "half_rsqrt\0" // IR(55631, 11)
  "half_sin\0" // IR(55642, 9)
  "half_sqrt\0" // IR(55651, 10)
  "half_tan\0" // IR(55661, 9)
  "native_cos\0" // IR(55670, 11)
  "native_divide\0" // IR(55681, 14)
  "native_exp\0" // IR(55695, 11)
  "native_exp2\0" // IR(55706, 12)
  "native_exp10\0" // IR(55718, 13)
  "native_log\0" // IR(55731, 11)
  "native_log2\0" // IR(55742, 12)
  "native_log10\0" // IR(55754, 13)
  "native_powr\0" // IR(55767, 12)
  "native_recip\0" // IR(55779, 13)
  "native_rsqrt\0" // IR(55792, 13)
  "native_sin\0" // IR(55805, 11)
  "native_sqrt\0" // IR(55816, 12)
  "native_tan\0" // IR(55828, 11)
  "fclamp\0" // IR(55839, 7)
  "degrees\0" // IR(55846, 8)
  "fmax_common\0" // IR(55854, 12)
  "fmin_common\0" // IR(55866, 12)
  "mix\0" // IR(55878, 4)
  "radians\0" // IR(55882, 8)
  "step\0" // IR(55890, 5)
  "smoothstep\0" // IR(55895, 11)
  "sign\0" // IR(55906, 5)
  "cross\0" // IR(55911, 6)
  "distance\0" // IR(55917, 9)
  "length\0" // IR(55926, 7)
  "normalize\0" // IR(55933, 10)
  "fast_distance\0" // IR(55943, 14)
  "fast_length\0" // IR(55957, 12)
  "fast_normalize\0" // IR(55969, 15)
  "s_abs\0" // IR(55984, 6)
  "s_abs_diff\0" // IR(55990, 11)
  "s_add_sat\0" // IR(56001, 10)
  "u_add_sat\0" // IR(56011, 10)
  "s_hadd\0" // IR(56021, 7)
  "u_hadd\0" // IR(56028, 7)
  "s_rhadd\0" // IR(56035, 8)
  "u_rhadd\0" // IR(56043, 8)
  "s_clamp\0" // IR(56051, 8)
  "u_clamp\0" // IR(56059, 8)
  "clz\0" // IR(56067, 4)
  "ctz\0" // IR(56071, 4)
  "s_mad_hi\0" // IR(56075, 9)
  "u_mad_sat\0" // IR(56084, 10)
  "s_mad_sat\0" // IR(56094, 10)
  "s_max\0" // IR(56104, 6)
  "u_max\0" // IR(56110, 6)
  "s_min\0" // IR(56116, 6)
  "u_min\0" // IR(56122, 6)
  "s_mul_hi\0" // IR(56128, 9)
  "rotate\0" // IR(56137, 7)
  "s_sub_sat\0" // IR(56144, 10)
  "u_sub_sat\0" // IR(56154, 10)
  "u_upsample\0" // IR(56164, 11)
  "s_upsample\0" // IR(56175, 11)
  "popcount\0" // IR(56186, 9)
  "s_mad24\0" // IR(56195, 8)
  "u_mad24\0" // IR(56203, 8)
  "s_mul24\0" // IR(56211, 8)
  "u_mul24\0" // IR(56219, 8)
  "vloadn\0" // IR(56227, 7)
  "vstoren\0" // IR(56234, 8)
  "vload_half\0" // IR(56242, 11)
  "vload_halfn\0" // IR(56253, 12)
  "vstore_half\0" // IR(56265, 12)
  "vstore_half_r\0" // IR(56277, 14)
  "vstore_halfn\0" // IR(56291, 13)
  "vstore_halfn_r\0" // IR(56304, 15)
  "vloada_halfn\0" // IR(56319, 13)
  "vstorea_halfn\0" // IR(56332, 14)
  "vstorea_halfn_r\0" // IR(56346, 16)
  "shuffle\0" // IR(56362, 8)
  "shuffle2\0" // IR(56370, 9)
  "printf\0" // IR(56379, 7)
  "prefetch\0" // IR(56386, 9)
  "bitselect\0" // IR(56395, 10)
  "select\0" // IR(56405, 7)
  "u_abs\0" // IR(56412, 6)
  "u_abs_diff\0" // IR(56418, 11)
  "u_mul_hi\0" // IR(56429, 9)
  "u_mad_hi\0" // IR(56438, 9)
  "CubeFaceIndexAMD\0" // IR(56447, 17)
  "CubeFaceCoordAMD\0" // IR(56464, 17)
  "TimeAMD\0" // IR(56481, 8)
  "SwizzleInvocationsAMD\0" // IR(56489, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(56511, 28)
  "WriteInvocationAMD\0" // IR(56539, 19)
  "MbcntAMD\0" // IR(56558, 9)
  "InterpolateAtVertexAMD\0" // IR(56567, 23)
  "FMin3AMD\0" // IR(56590, 9)
  "UMin3AMD\0" // IR(56599, 9)
  "SMin3AMD\0" // IR(56608, 9)
  "FMax3AMD\0" // IR(56617, 9)
  "UMax3AMD\0" // IR(56626, 9)
  "SMax3AMD\0" // IR(56635, 9)
  "FMid3AMD\0" // IR(56644, 9)
  "UMid3AMD\0" // IR(56653, 9)
  "SMid3AMD\0" // IR(56662, 9)
  "ARGMAX\0" // IR(56671, 7)
  "AVG_POOL2D\0" // IR(56678, 11)
  "CONV2D\0" // IR(56689, 7)
  "CONV3D\0" // IR(56696, 7)
  "DEPTHWISE_CONV2D\0" // IR(56703, 17)
  "FFT2D\0" // IR(56720, 6)
  "MATMUL\0" // IR(56726, 7)
  "MAX_POOL2D\0" // IR(56733, 11)
  "RFFT2D\0" // IR(56744, 7)
  "TRANSPOSE_CONV2D\0" // IR(56751, 17)
  "CLAMP\0" // IR(56768, 6)
  "ERF\0" // IR(56774, 4)
  "SIGMOID\0" // IR(56778, 8)
  "TANH\0" // IR(56786, 5)
  "ADD\0" // IR(56791, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(56795, 23)
  "BITWISE_AND\0" // IR(56818, 12)
  "BITWISE_OR\0" // IR(56830, 11)
  "BITWISE_XOR\0" // IR(56841, 12)
  "INTDIV\0" // IR(56853, 7)
  "LOGICAL_AND\0" // IR(56860, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(56872, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(56891, 20)
  "LOGICAL_OR\0" // IR(56911, 11)
  "LOGICAL_XOR\0" // IR(56922, 12)
  "MAXIMUM\0" // IR(56934, 8)
  "MINIMUM\0" // IR(56942, 8)
  "MUL\0" // IR(56950, 4)
  "POW\0" // IR(56954, 4)
  "SUB\0" // IR(56958, 4)
  "TABLE\0" // IR(56962, 6)
  "ABS\0" // IR(56968, 4)
  "BITWISE_NOT\0" // IR(56972, 12)
  "CEIL\0" // IR(56984, 5)
  "CLZ\0" // IR(56989, 4)
  "COS\0" // IR(56993, 4)
  "EXP\0" // IR(56997, 4)
  "FLOOR\0" // IR(57001, 6)
  "LOG\0" // IR(57007, 4)
  "LOGICAL_NOT\0" // IR(57011, 12)
  "NEGATE\0" // IR(57023, 7)
  "RECIPROCAL\0" // IR(57030, 11)
  "RSQRT\0" // IR(57041, 6)
  "SIN\0" // IR(57047, 4)
  "SELECT\0" // IR(57051, 7)
  "EQUAL\0" // IR(57058, 6)
  "GREATER\0" // IR(57064, 8)
  "GREATER_EQUAL\0" // IR(57072, 14)
  "REDUCE_ALL\0" // IR(57086, 11)
  "REDUCE_ANY\0" // IR(57097, 11)
  "REDUCE_MAX\0" // IR(57108, 11)
  "REDUCE_MIN\0" // IR(57119, 11)
  "REDUCE_PRODUCT\0" // IR(57130, 15)
  "REDUCE_SUM\0" // IR(57145, 11)
  "CONCAT\0" // IR(57156, 7)
  "PAD\0" // IR(57163, 4)
  "RESHAPE\0" // IR(57167, 8)
  "REVERSE\0" // IR(57175, 8)
  "SLICE\0" // IR(57183, 6)
  "TILE\0" // IR(57189, 5)
  "TRANSPOSE\0" // IR(57194, 10)
  "GATHER\0" // IR(57204, 7)
  "SCATTER\0" // IR(57211, 8)
  "RESIZE\0" // IR(57219, 7)
  "CAST\0" // IR(57226, 5)
  "RESCALE\0" // IR(57231, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(5918, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6829, 18), // 1 SubgroupEqMaskKHR
  IR(6884, 18), // 2 SubgroupGeMaskKHR
  IR(6917, 18), // 3 SubgroupGtMaskKHR
  IR(6950, 18), // 4 SubgroupLeMaskKHR
  IR(6983, 18), // 5 SubgroupLtMaskKHR
  IR(7890, 12), // 6 BaryCoordNV
  IR(7945, 19), // 7 BaryCoordNoPerspNV
  IR(7976, 15), // 8 FragmentSizeNV
  IR(8033, 22), // 9 InvocationsPerPixelNV
  IR(8161, 11), // 10 LaunchIdNV
  IR(8186, 13), // 11 LaunchSizeNV
  IR(8217, 17), // 12 WorldRayOriginNV
  IR(8255, 20), // 13 WorldRayDirectionNV
  IR(8294, 18), // 14 ObjectRayOriginNV
  IR(8334, 21), // 15 ObjectRayDirectionNV
  IR(8366, 10), // 16 RayTminNV
  IR(8387, 10), // 17 RayTmaxNV
  IR(8420, 22), // 18 InstanceCustomIndexNV
  IR(8459, 16), // 19 ObjectToWorldNV
  IR(8492, 16), // 20 WorldToObjectNV
  IR(8526, 10), // 21 HitKindNV
  IR(8757, 19), // 22 IncomingRayFlagsNV
  IR(10603, 28), // 23 StorageUniformBufferBlock16
  IR(10666, 17), // 24 StorageUniform16
  IR(11611, 27), // 25 ShaderViewportIndexLayerNV
  IR(11655, 22), // 26 FragmentBarycentricNV
  IR(11708, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11738, 14), // 28 ShadingRateNV
  IR(11782, 29), // 29 GroupNonUniformPartitionedNV
  IR(11828, 20), // 30 ShaderNonUniformEXT
  IR(11871, 26), // 31 RuntimeDescriptorArrayEXT
  IR(11933, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(12011, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(12092, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(12171, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(12247, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(12323, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(12399, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(12477, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12561, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12648, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12711, 21), // 42 VulkanMemoryModelKHR
  IR(12761, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(12793, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(12859, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(13063, 28), // 46 DemoteToHelperInvocationEXT
  IR(13143, 29), // 47 RayTracingOpacityMicromapEXT
  IR(14165, 26), // 48 FPGAMemoryAttributesINTEL
  IR(14244, 32), // 49 ArbitraryPrecisionIntegersINTEL
  IR(14314, 37), // 50 ArbitraryPrecisionFloatingPointINTEL
  IR(14404, 22), // 51 FPGALoopControlsINTEL
  IR(14499, 24), // 52 FPGAMemoryAccessesINTEL
  IR(14551, 27), // 53 FPGAClusterAttributesINTEL
  IR(14593, 14), // 54 LoopFuseINTEL
  IR(14628, 20), // 55 FPGADSPControlINTEL
  IR(14715, 40), // 56 FPGAInvocationPipeliningAttributesINTEL
  IR(14780, 24), // 57 FPGABufferLocationINTEL
  IR(14839, 34), // 58 ArbitraryPrecisionFixedPointINTEL
  IR(14897, 23), // 59 USMStorageClassesINTEL
  IR(14950, 29), // 60 RuntimeAlignedAttributeINTEL
  IR(14993, 13), // 61 IOPipesINTEL
  IR(15026, 19), // 62 BlockingPipesINTEL
  IR(15059, 13), // 63 FPGARegINTEL
  IR(15091, 22), // 64 DotProductInputAllKHR
  IR(15135, 25), // 65 DotProductInput4x8BitKHR
  IR(15188, 31), // 66 DotProductInput4x8BitPackedKHR
  IR(15230, 14), // 67 DotProductKHR
  IR(15444, 13), // 68 OptNoneINTEL
  IR(15538, 18), // 69 SplitBarrierINTEL
  IR(15605, 29), // 70 FPGAClusterAttributesV2INTEL
  IR(15681, 18), // 71 TaskSequenceINTEL
  IR(15740, 24), // 72 FPGALatencyControlINTEL
  IR(15793, 28), // 73 FPGAArgumentInterfacesINTEL
  IR(15887, 35), // 74 GlobalVariableFPGADecorationsINTEL
  IR(18896, 15), // 75 PerPrimitiveNV
  IR(18944, 12), // 76 PerVertexNV
  IR(18967, 14), // 77 NonUniformEXT
  IR(18997, 19), // 78 RestrictPointerEXT
  IR(19031, 18), // 79 AliasedPointerEXT
  IR(19394, 24), // 80 HlslCounterBufferGOOGLE
  IR(19431, 19), // 81 HlslSemanticGOOGLE
  IR(19561, 14), // 82 RegisterINTEL
  IR(19588, 12), // 83 MemoryINTEL
  IR(19615, 14), // 84 NumbanksINTEL
  IR(19645, 15), // 85 BankwidthINTEL
  IR(19683, 22), // 86 MaxPrivateCopiesINTEL
  IR(19722, 16), // 87 SinglepumpINTEL
  IR(19755, 16), // 88 DoublepumpINTEL
  IR(19791, 19), // 89 MaxReplicatesINTEL
  IR(19831, 20), // 90 SimpleDualPortINTEL
  IR(19863, 11), // 91 MergeINTEL
  IR(19931, 14), // 92 BankBitsINTEL
  IR(19966, 20), // 93 ForcePow2DepthINTEL
  IR(20003, 16), // 94 StridesizeINTEL
  IR(20034, 14), // 95 WordsizeINTEL
  IR(20067, 18), // 96 TrueDualPortINTEL
  IR(20105, 19), // 97 BurstCoalesceINTEL
  IR(20140, 15), // 98 CacheSizeINTEL
  IR(20184, 28), // 99 DontStaticallyCoalesceINTEL
  IR(20227, 14), // 100 PrefetchINTEL
  IR(20259, 17), // 101 StallEnableINTEL
  IR(20302, 25), // 102 FuseLoopsInFunctionINTEL
  IR(20347, 19), // 103 MathOpDSPModeINTEL
  IR(20420, 24), // 104 InitiationIntervalINTEL
  IR(20465, 20), // 105 MaxConcurrencyINTEL
  IR(20506, 20), // 106 PipelineEnableINTEL
  IR(20547, 20), // 107 BufferLocationINTEL
  IR(20587, 19), // 108 IOPipeStorageINTEL
  IR(20765, 15), // 109 StallFreeINTEL
  IR(20863, 25), // 110 LatencyControlLabelINTEL
  IR(20919, 30), // 111 LatencyControlConstraintINTEL
  IR(20977, 27), // 112 ConduitKernelArgumentINTEL
  IR(21036, 31), // 113 RegisterMapKernelArgumentINTEL
  IR(21101, 33), // 114 MMHostInterfaceAddressWidthINTEL
  IR(21165, 30), // 115 MMHostInterfaceDataWidthINTEL
  IR(21224, 28), // 116 MMHostInterfaceLatencyINTEL
  IR(21321, 34), // 117 MMHostInterfaceReadWriteModeINTEL
  IR(21385, 29), // 118 MMHostInterfaceMaxBurstINTEL
  IR(21447, 32), // 119 MMHostInterfaceWaitRequestINTEL
  IR(21506, 26), // 120 StableKernelArgumentINTEL
  IR(21649, 14), // 121 InitModeINTEL
  IR(21692, 28), // 122 ImplementInRegisterMapINTEL
  IR(23046, 14), // 123 OutputLinesNV
  IR(23080, 19), // 124 OutputPrimitivesNV
  IR(23123, 23), // 125 DerivativeGroupQuadsNV
  IR(23171, 24), // 126 DerivativeGroupLinearNV
  IR(23214, 18), // 127 OutputTrianglesNV
  IR(23983, 16), // 128 RayGenerationNV
  IR(24015, 15), // 129 IntersectionNV
  IR(24040, 9), // 130 AnyHitNV
  IR(24063, 13), // 131 ClosestHitNV
  IR(24084, 7), // 132 MissNV
  IR(24103, 11), // 133 CallableNV
  IR(24290, 23), // 134 AllowContractFastINTEL
  IR(24326, 18), // 135 AllowReassocINTEL
  IR(24552, 20), // 136 RuntimeAlignedINTEL
  IR(24696, 20), // 137 PartitionedReduceNV
  IR(24744, 27), // 138 PartitionedInclusiveScanNV
  IR(24799, 27), // 139 PartitionedExclusiveScanNV
  IR(24930, 22), // 140 MakeTexelAvailableKHR
  IR(24969, 20), // 141 MakeTexelVisibleKHR
  IR(25005, 19), // 142 NonPrivateTexelKHR
  IR(25038, 17), // 143 VolatileTexelKHR
  IR(25597, 27), // 144 InitOnDeviceReprogramINTEL
  IR(25648, 23), // 145 InitOnDeviceResetINTEL
  IR(25990, 21), // 146 DependencyArrayINTEL
  IR(26030, 18), // 147 LoopCoalesceINTEL
  IR(26070, 21), // 148 MaxInterleavingINTEL
  IR(26118, 26), // 149 SpeculatedIterationsINTEL
  IR(26159, 14), // 150 NoFusionINTEL
  IR(26189, 15), // 151 LoopCountINTEL
  IR(26231, 26), // 152 MaxReinvocationDelayINTEL
  IR(26623, 24), // 153 MakePointerAvailableKHR
  IR(26666, 22), // 154 MakePointerVisibleKHR
  IR(26706, 21), // 155 NonPrivatePointerKHR
  IR(26793, 10), // 156 VulkanKHR
  IR(16625, 5), // 157 None
  IR(26976, 16), // 158 OutputMemoryKHR
  IR(27006, 17), // 159 MakeAvailableKHR
  IR(27035, 15), // 160 MakeVisibleKHR
  IR(27111, 28), // 161 PackedVectorFormat4x8BitKHR
  IR(27426, 24), // 162 SkipBuiltinPrimitivesNV
  IR(27493, 30), // 163 ForceOpacityMicromap2StateEXT
  IR(28196, 15), // 164 QueueFamilyKHR
  IR(28581, 15), // 165 CallableDataNV
  IR(28620, 23), // 166 IncomingCallableDataNV
  IR(28657, 13), // 167 RayPayloadNV
  IR(28686, 15), // 168 HitAttributeNV
  IR(28723, 21), // 169 IncomingRayPayloadNV
  IR(28766, 21), // 170 ShaderRecordBufferNV
  IR(28809, 25), // 171 PhysicalStorageBufferEXT
  IR(28935, 16), // 172 DeviceOnlyINTEL
  IR(28966, 14), // 173 HostOnlyINTEL
  IR(36221, 8), // 174 SDotKHR
  IR(36234, 8), // 175 UDotKHR
  IR(36248, 9), // 176 SUDotKHR
  IR(36268, 14), // 177 SDotAccSatKHR
  IR(36293, 14), // 178 UDotAccSatKHR
  IR(36319, 15), // 179 SUDotAccSatKHR
  IR(38636, 24), // 180 TypeCooperativeVectorNV
  IR(38892, 27), // 181 GroupNonUniformPartitionNV
  IR(39974, 21), // 182 ReportIntersectionNV
  IR(40149, 28), // 183 TypeAccelerationStructureNV
  IR(40230, 23), // 184 RayQueryGetClusterIdNV
  IR(42398, 21), // 185 DecorateStringGOOGLE
  IR(42440, 27), // 186 MemberDecorateStringGOOGLE
  IR(47697, 28), // 187 ArbitraryFloatSinCosPiINTEL
  IR(47750, 24), // 188 ArbitraryFloatCastINTEL
  IR(47806, 31), // 189 ArbitraryFloatCastFromIntINTEL
  IR(47867, 29), // 190 ArbitraryFloatCastToIntINTEL
  IR(47920, 23), // 191 ArbitraryFloatAddINTEL
  IR(47967, 23), // 192 ArbitraryFloatSubINTEL
  IR(48014, 23), // 193 ArbitraryFloatMulINTEL
  IR(48061, 23), // 194 ArbitraryFloatDivINTEL
  IR(48107, 22), // 195 ArbitraryFloatGTINTEL
  IR(48152, 22), // 196 ArbitraryFloatGEINTEL
  IR(48197, 22), // 197 ArbitraryFloatLTINTEL
  IR(48242, 22), // 198 ArbitraryFloatLEINTEL
  IR(48287, 22), // 199 ArbitraryFloatEQINTEL
  IR(48335, 25), // 200 ArbitraryFloatRecipINTEL
  IR(48386, 25), // 201 ArbitraryFloatRSqrtINTEL
  IR(48436, 24), // 202 ArbitraryFloatCbrtINTEL
  IR(48486, 25), // 203 ArbitraryFloatHypotINTEL
  IR(48536, 24), // 204 ArbitraryFloatSqrtINTEL
  IR(49220, 15), // 205 FixedSqrtINTEL
  IR(49252, 16), // 206 FixedRecipINTEL
  IR(49285, 16), // 207 FixedRsqrtINTEL
  IR(49316, 14), // 208 FixedSinINTEL
  IR(49345, 14), // 209 FixedCosINTEL
  IR(49377, 17), // 210 FixedSinCosINTEL
  IR(49411, 16), // 211 FixedSinPiINTEL
  IR(49444, 16), // 212 FixedCosPiINTEL
  IR(49480, 19), // 213 FixedSinCosPiINTEL
  IR(49514, 14), // 214 FixedLogINTEL
  IR(49543, 14), // 215 FixedExpINTEL
  IR(49587, 29), // 216 PtrCastToCrossWorkgroupINTEL
  IR(49646, 29), // 217 CrossWorkgroupCastToPtrINTEL
  IR(49698, 22), // 218 ReadPipeBlockingINTEL
  IR(49744, 23), // 219 WritePipeBlockingINTEL
  IR(50645, 26), // 220 ControlBarrierArriveINTEL
  IR(50693, 24), // 221 ControlBarrierWaitINTEL
  IR(50742, 24), // 222 TaskSequenceCreateINTEL
  IR(50790, 23), // 223 TaskSequenceAsyncINTEL
  IR(50835, 21), // 224 TaskSequenceGetINTEL
  IR(50882, 25), // 225 TaskSequenceReleaseINTEL
  IR(50930, 22), // 226 TypeTaskSequenceINTEL
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
  spv::Capability::IntrinsicSAMSUNG, // 155
  spv::Capability::TileImageColorReadAccessEXT, // 156
  spv::Capability::Geometry, // 157
  spv::Capability::Tessellation, // 158
  spv::Capability::MeshShadingNV, // 159
  spv::Capability::MeshShadingEXT, // 160
  spv::Capability::Geometry, // 161
  spv::Capability::MeshShadingNV, // 162
  spv::Capability::MeshShadingEXT, // 163
  spv::Capability::SubgroupDispatch, // 164
  spv::Capability::TileImageDepthReadAccessEXT, // 165
  spv::Capability::TileImageStencilReadAccessEXT, // 166
  spv::Capability::SampleMaskPostDepthCoverage, // 167
  spv::Capability::DenormPreserve, // 168
  spv::Capability::DenormFlushToZero, // 169
  spv::Capability::SignedZeroInfNanPreserve, // 170
  spv::Capability::RoundingModeRTE, // 171
  spv::Capability::RoundingModeRTZ, // 172
  spv::Capability::QuadControlKHR, // 173
  spv::Capability::PoisonFreezeKHR, // 174
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 175
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 176
  spv::Capability::FragmentShaderPixelInterlockEXT, // 177
  spv::Capability::FragmentShaderSampleInterlockEXT, // 178
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 179
  spv::Capability::Shader64BitIndexingEXT, // 180
  spv::Capability::RoundToInfinityINTEL, // 181
  spv::Capability::KernelAttributesINTEL, // 182
  spv::Capability::FloatControls2, // 183
  spv::Capability::RayTracingOpacityMicromapExecutionModeKHR, // 184
  spv::Capability::FPGAKernelAttributesv2INTEL, // 185
  spv::Capability::RegisterLimitsINTEL, // 186
  spv::Capability::Float6EXT, // 187
  spv::Capability::Float4EXT, // 188
  spv::Capability::Float8UnsignedE8M0EXT, // 189
  spv::Capability::MXInt8EXT, // 190
  spv::Capability::FloatControls2, // 191
  spv::Capability::FPFastMathModeINTEL, // 192
  spv::Capability::OptNoneEXT, // 193
  spv::Capability::RuntimeAlignedAttributeALTERA, // 194
  spv::Capability::Kernel, // 195
  spv::Capability::GroupNonUniformArithmetic, // 196
  spv::Capability::GroupNonUniformBallot, // 197
  spv::Capability::GroupNonUniformClustered, // 198
  spv::Capability::GroupNonUniformPartitionedEXT, // 199
  spv::Capability::ImageGatherExtended, // 200
  spv::Capability::MinLod, // 201
  spv::Capability::VulkanMemoryModel, // 202
  spv::Capability::WeakLinkageAMD, // 203
  spv::Capability::FPGALoopControlsALTERA, // 204
  spv::Capability::ArbitraryPrecisionFixedPointALTERA, // 205
  spv::Capability::RawAccessChainsNV, // 206
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 207
  spv::Capability::RayTracingOpacityMicromapKHR, // 208
  spv::Capability::RayQueryKHR, // 209
  spv::Capability::StorageImageExtendedFormats, // 210
  spv::Capability::Int64ImageEXT, // 211
  spv::Capability::Shader, // 212
  spv::Capability::VectorComputeINTEL, // 213
  spv::Capability::GenericPointer, // 214
  spv::Capability::FunctionPointersINTEL, // 215
  spv::Capability::USMStorageClassesALTERA, // 216
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 217
  spv::Capability::CooperativeMatrixDecodeVectorNV, // 218
  spv::Capability::TensorsARM, // 219
  spv::Capability::Addresses, // 220
  spv::Capability::PhysicalStorageBufferAddresses, // 221
  spv::Capability::LiteralSampler, // 222
  spv::Capability::Addresses, // 223
  spv::Capability::UntypedPointersKHR, // 224
  spv::Capability::Addresses, // 225
  spv::Capability::VariablePointers, // 226
  spv::Capability::VariablePointersStorageBuffer, // 227
  spv::Capability::PhysicalStorageBufferAddresses, // 228
  spv::Capability::Kernel, // 229
  spv::Capability::ImageQuery, // 230
  spv::Capability::ImageQuery, // 231
  spv::Capability::Shader, // 232
  spv::Capability::BitInstructions, // 233
  spv::Capability::DerivativeControl, // 234
  spv::Capability::Groups, // 235
  spv::Capability::SparseResidency, // 236
  spv::Capability::PipeStorage, // 237
  spv::Capability::NamedBarrier, // 238
  spv::Capability::GroupNonUniformVote, // 239
  spv::Capability::GroupNonUniformBallot, // 240
  spv::Capability::GroupNonUniformShuffle, // 241
  spv::Capability::GroupNonUniformShuffleRelative, // 242
  spv::Capability::GroupNonUniformArithmetic, // 243
  spv::Capability::GroupNonUniformClustered, // 244
  spv::Capability::GroupNonUniformPartitionedEXT, // 245
  spv::Capability::GroupNonUniformQuad, // 246
  spv::Capability::Addresses, // 247
  spv::Capability::VariablePointers, // 248
  spv::Capability::VariablePointersStorageBuffer, // 249
  spv::Capability::GraphARM, // 250
  spv::Capability::BitcastExtractEXT, // 251
  spv::Capability::SubgroupBallotKHR, // 252
  spv::Capability::FMAKHR, // 253
  spv::Capability::SubgroupVoteKHR, // 254
  spv::Capability::GroupNonUniformRotateKHR, // 255
  spv::Capability::RayTracingKHR, // 256
  spv::Capability::RayQueryKHR, // 257
  spv::Capability::DotProduct, // 258
  spv::Capability::ReplicatedCompositesEXT, // 259
  spv::Capability::TextureSampleWeightedQCOM, // 260
  spv::Capability::TextureBoxFilterQCOM, // 261
  spv::Capability::TextureBlockMatchQCOM, // 262
  spv::Capability::CooperativeMatrixConversionQCOM, // 263
  spv::Capability::TextureBlockMatch2QCOM, // 264
  spv::Capability::ImageGatherLinearQCOM, // 265
  spv::Capability::ImageGatherExtendedModesQCOM, // 266
  spv::Capability::FragmentMaskAMD, // 267
  spv::Capability::ShaderClockKHR, // 268
  spv::Capability::AbortKHR, // 269
  spv::Capability::ShaderInvocationReorderNV, // 270
  spv::Capability::RayTracingMotionBlurNV, // 271
  spv::Capability::ImageFootprintNV, // 272
  spv::Capability::CooperativeVectorNV, // 273
  spv::Capability::LongVectorEXT, // 274
  spv::Capability::CooperativeVectorNV, // 275
  spv::Capability::CooperativeVectorTrainingNV, // 276
  spv::Capability::CooperativeMatrixConversionsNV, // 277
  spv::Capability::DisplacementMicromapNV, // 278
  spv::Capability::ShaderInvocationReorderEXT, // 279
  spv::Capability::RayTracingMotionBlurNV, // 280
  spv::Capability::RayQueryPositionFetchKHR, // 281
  spv::Capability::RayTracingNV, // 282
  spv::Capability::RayTracingKHR, // 283
  spv::Capability::RayQueryKHR, // 284
  spv::Capability::DisplacementMicromapNV, // 285
  spv::Capability::CooperativeMatrixNV, // 286
  spv::Capability::FragmentShaderSampleInterlockEXT, // 287
  spv::Capability::FragmentShaderPixelInterlockEXT, // 288
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 289
  spv::Capability::CooperativeMatrixReductionsNV, // 290
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 291
  spv::Capability::TensorAddressingNV, // 292
  spv::Capability::DemoteToHelperInvocation, // 293
  spv::Capability::SubgroupShuffleINTEL, // 294
  spv::Capability::SubgroupBufferBlockIOINTEL, // 295
  spv::Capability::SubgroupImageBlockIOINTEL, // 296
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 297
  spv::Capability::IntegerFunctions2INTEL, // 298
  spv::Capability::AtomicFloat16MinMaxEXT, // 299
  spv::Capability::AtomicFloat32MinMaxEXT, // 300
  spv::Capability::AtomicFloat64MinMaxEXT, // 301
  spv::Capability::AtomicFloat16VectorNV, // 302
  spv::Capability::ExpectAssumeKHR, // 303
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 304
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 305
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 306
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 307
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 308
  spv::Capability::VariableLengthArrayINTEL, // 309
  spv::Capability::ArbitraryPrecisionFloatingPointALTERA, // 310
  spv::Capability::UnstructuredLoopControlsINTEL, // 311
  spv::Capability::BlockingPipesALTERA, // 312
  spv::Capability::FPGARegALTERA, // 313
  spv::Capability::AtomicFloat16AddEXT, // 314
  spv::Capability::AtomicFloat32AddEXT, // 315
  spv::Capability::AtomicFloat64AddEXT, // 316
  spv::Capability::AtomicFloat16VectorNV, // 317
  spv::Capability::LongCompositesINTEL, // 318
  spv::Capability::BFloat16ConversionINTEL, // 319
  spv::Capability::SplitBarrierEXT, // 320
  spv::Capability::ArithmeticFenceEXT, // 321
  spv::Capability::TaskSequenceALTERA, // 322
  spv::Capability::SubgroupBufferPrefetchINTEL, // 323
  spv::Capability::Subgroup2DBlockTransformINTEL, // 324
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 325
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 326
  spv::Capability::TernaryBitwiseFunctionINTEL, // 327
  spv::Capability::UntypedVariableLengthArrayINTEL, // 328
  spv::Capability::FunctionVariantsINTEL, // 329
  spv::Capability::PredicatedIOINTEL, // 330
  spv::Capability::GroupUniformArithmeticKHR, // 331
  spv::Capability::TensorFloat32RoundingINTEL, // 332
  spv::Capability::MaskedGatherScatterINTEL, // 333
  spv::Capability::BindlessImagesINTEL, // 334
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 335
  spv::Capability::DotProductBFloat16AccVALVE, // 336
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 337
  spv::Capability::DotProductBFloat16AccVALVE, // 338
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 339
  spv::Capability::Float64, // 340
  spv::Capability::InterpolationFunction, // 341
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
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 103
  spvtools::Extension::kSPV_NV_tensor_addressing, // 104
  spvtools::Extension::kSPV_NV_cooperative_matrix_decode_vector, // 105
  spvtools::Extension::kSPV_INTEL_subgroups, // 106
  spvtools::Extension::kSPV_INTEL_media_block_io, // 107
  spvtools::Extension::kSPV_INTEL_float_controls2, // 108
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 109
  spvtools::Extension::kSPV_INTEL_function_pointers, // 110
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 111
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 112
  spvtools::Extension::kSPV_INTEL_vector_compute, // 113
  spvtools::Extension::kSPV_KHR_expect_assume, // 114
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 115
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 116
  spvtools::Extension::kSPV_ALTERA_fpga_memory_attributes, // 117
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 118
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 119
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_integers, // 120
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 121
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_floating_point, // 122
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 123
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 124
  spvtools::Extension::kSPV_ALTERA_fpga_loop_controls, // 125
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 126
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 127
  spvtools::Extension::kSPV_ALTERA_fpga_memory_accesses, // 128
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 129
  spvtools::Extension::kSPV_ALTERA_fpga_cluster_attributes, // 130
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 131
  spvtools::Extension::kSPV_ALTERA_loop_fuse, // 132
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 133
  spvtools::Extension::kSPV_ALTERA_fpga_dsp_control, // 134
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 135
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 136
  spvtools::Extension::kSPV_ALTERA_fpga_invocation_pipelining_attributes, // 137
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 138
  spvtools::Extension::kSPV_ALTERA_fpga_buffer_location, // 139
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 140
  spvtools::Extension::kSPV_ALTERA_arbitrary_precision_fixed_point, // 141
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 142
  spvtools::Extension::kSPV_ALTERA_usm_storage_classes, // 143
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 144
  spvtools::Extension::kSPV_ALTERA_runtime_aligned, // 145
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 146
  spvtools::Extension::kSPV_ALTERA_io_pipes, // 147
  spvtools::Extension::kSPV_INTEL_io_pipes, // 148
  spvtools::Extension::kSPV_ALTERA_blocking_pipes, // 149
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 150
  spvtools::Extension::kSPV_ALTERA_fpga_reg, // 151
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 152
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 153
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 154
  spvtools::Extension::kSPV_EXT_replicated_composites, // 155
  spvtools::Extension::kSPV_KHR_bit_instructions, // 156
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 157
  spvtools::Extension::kSPV_KHR_float_controls2, // 158
  spvtools::Extension::kSPV_KHR_fma, // 159
  spvtools::Extension::kSPV_KHR_opacity_micromap, // 160
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 161
  spvtools::Extension::kSPV_INTEL_long_composites, // 162
  spvtools::Extension::kSPV_EXT_optnone, // 163
  spvtools::Extension::kSPV_INTEL_optnone, // 164
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 165
  spvtools::Extension::kSPV_INTEL_debug_module, // 166
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 167
  spvtools::Extension::kSPV_EXT_split_barrier, // 168
  spvtools::Extension::kSPV_INTEL_split_barrier, // 169
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 170
  spvtools::Extension::kSPV_ALTERA_task_sequence, // 171
  spvtools::Extension::kSPV_INTEL_task_sequence, // 172
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 173
  spvtools::Extension::kSPV_ALTERA_fpga_latency_control, // 174
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 175
  spvtools::Extension::kSPV_ALTERA_fpga_argument_interfaces, // 176
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 177
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 178
  spvtools::Extension::kSPV_ALTERA_global_variable_fpga_decorations, // 179
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 180
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 181
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 182
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 183
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 184
  spvtools::Extension::kSPV_INTEL_function_variants, // 185
  spvtools::Extension::kSPV_INTEL_predicated_io, // 186
  spvtools::Extension::kSPV_INTEL_rounded_divide_sqrt, // 187
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 188
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 189
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 190
  spvtools::Extension::kSPV_INTEL_cache_controls, // 191
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 192
  spvtools::Extension::kSPV_INTEL_bindless_images, // 193
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 194
  spvtools::Extension::kSPV_SAMSUNG_intrinsic, // 195
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 196
  spvtools::Extension::kSPV_NV_mesh_shader, // 197
  spvtools::Extension::kSPV_EXT_mesh_shader, // 198
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 199
  spvtools::Extension::kSPV_GOOGLE_user_type, // 200
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 201
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 202
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 203
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 204
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 205
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 206
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 207
  spvtools::Extension::kSPV_KHR_variable_pointers, // 208
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 209
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 210
  spvtools::Extension::kSPV_KHR_ray_tracing, // 211
  spvtools::Extension::kSPV_KHR_ray_query, // 212
  spvtools::Extension::kSPV_NV_ray_tracing, // 213
  spvtools::Extension::kSPV_KHR_ray_tracing, // 214
  spvtools::Extension::kSPV_KHR_ray_query, // 215
  spvtools::Extension::kSPV_NV_displacement_micromap, // 216
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 217
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 218
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
