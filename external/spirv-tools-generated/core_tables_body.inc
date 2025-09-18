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
    case Extension::kSPV_EXT_shader_atomic_float16_add: return IR(828, 34);
    case Extension::kSPV_EXT_shader_atomic_float_add: return IR(862, 32);
    case Extension::kSPV_EXT_shader_atomic_float_min_max: return IR(894, 36);
    case Extension::kSPV_EXT_shader_image_int64: return IR(930, 27);
    case Extension::kSPV_EXT_shader_stencil_export: return IR(957, 30);
    case Extension::kSPV_EXT_shader_tile_image: return IR(987, 26);
    case Extension::kSPV_EXT_shader_viewport_index_layer: return IR(1013, 36);
    case Extension::kSPV_GOOGLE_decorate_string: return IR(1049, 27);
    case Extension::kSPV_GOOGLE_hlsl_functionality1: return IR(1076, 31);
    case Extension::kSPV_GOOGLE_user_type: return IR(1107, 21);
    case Extension::kSPV_INTEL_2d_block_io: return IR(1128, 22);
    case Extension::kSPV_INTEL_arbitrary_precision_fixed_point: return IR(1150, 42);
    case Extension::kSPV_INTEL_arbitrary_precision_floating_point: return IR(1192, 45);
    case Extension::kSPV_INTEL_arbitrary_precision_integers: return IR(1237, 39);
    case Extension::kSPV_INTEL_bfloat16_conversion: return IR(1276, 30);
    case Extension::kSPV_INTEL_bindless_images: return IR(1306, 26);
    case Extension::kSPV_INTEL_blocking_pipes: return IR(1332, 25);
    case Extension::kSPV_INTEL_cache_controls: return IR(1357, 25);
    case Extension::kSPV_INTEL_debug_module: return IR(1382, 23);
    case Extension::kSPV_INTEL_device_side_avc_motion_estimation: return IR(1405, 44);
    case Extension::kSPV_INTEL_float_controls2: return IR(1449, 26);
    case Extension::kSPV_INTEL_fp_fast_math_mode: return IR(1475, 28);
    case Extension::kSPV_INTEL_fp_max_error: return IR(1503, 23);
    case Extension::kSPV_INTEL_fpga_argument_interfaces: return IR(1526, 35);
    case Extension::kSPV_INTEL_fpga_buffer_location: return IR(1561, 31);
    case Extension::kSPV_INTEL_fpga_cluster_attributes: return IR(1592, 34);
    case Extension::kSPV_INTEL_fpga_dsp_control: return IR(1626, 27);
    case Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes: return IR(1653, 48);
    case Extension::kSPV_INTEL_fpga_latency_control: return IR(1701, 31);
    case Extension::kSPV_INTEL_fpga_loop_controls: return IR(1732, 29);
    case Extension::kSPV_INTEL_fpga_memory_accesses: return IR(1761, 31);
    case Extension::kSPV_INTEL_fpga_memory_attributes: return IR(1792, 33);
    case Extension::kSPV_INTEL_fpga_reg: return IR(1825, 19);
    case Extension::kSPV_INTEL_function_pointers: return IR(1844, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(1872, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(1915, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(1953, 26);
    case Extension::kSPV_INTEL_int4: return IR(1979, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(1994, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2013, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2041, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2067, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2087, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2119, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2147, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2172, 33);
    case Extension::kSPV_INTEL_optnone: return IR(2205, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(2223, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(2249, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(2285, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(2309, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(2344, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(2390, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(2410, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(2434, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(2470, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(2505, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(2542, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(2572, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(2604, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(2629, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(2651, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(2672, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(2689, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(2714, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(2749, 27);
    case Extension::kSPV_KHR_device_group: return IR(2776, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(2797, 22);
    case Extension::kSPV_KHR_float_controls: return IR(2819, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(2842, 24);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(2866, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(2902, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(2932, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(2960, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(2981, 30);
    case Extension::kSPV_KHR_multiview: return IR(3011, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3029, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3064, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3090, 32);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3122, 28);
    case Extension::kSPV_KHR_quad_control: return IR(3150, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(3171, 22);
    case Extension::kSPV_KHR_ray_query: return IR(3193, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(3211, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(3231, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(3266, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(3303, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(3337, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(3359, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(3380, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(3411, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(3448, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(3472, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(3510, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(3532, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(3561, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(3596, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(3621, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(3647, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(3675, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(3716, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(3754, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(3778, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(3816, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(3850, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(3876, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(3903, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(3929, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(3958, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(3993, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(4028, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4056, 19);
    case Extension::kSPV_NV_raw_access_chains: return IR(4075, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4100, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(4119, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(4150, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(4187, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(4220, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(4250, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(4283, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(4309, 35);
    case Extension::kSPV_NV_shading_rate: return IR(4344, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(4364, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(4393, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(4418, 23);
    case Extension::kSPV_QCOM_image_processing: return IR(4441, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(4467, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(4494, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(4516, 38);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,158> kExtensionNames{{
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
    {IR(828, 34), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float16_add)},
    {IR(862, 32), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_add)},
    {IR(894, 36), static_cast<uint32_t>(kSPV_EXT_shader_atomic_float_min_max)},
    {IR(930, 27), static_cast<uint32_t>(kSPV_EXT_shader_image_int64)},
    {IR(957, 30), static_cast<uint32_t>(kSPV_EXT_shader_stencil_export)},
    {IR(987, 26), static_cast<uint32_t>(kSPV_EXT_shader_tile_image)},
    {IR(1013, 36), static_cast<uint32_t>(kSPV_EXT_shader_viewport_index_layer)},
    {IR(1049, 27), static_cast<uint32_t>(kSPV_GOOGLE_decorate_string)},
    {IR(1076, 31), static_cast<uint32_t>(kSPV_GOOGLE_hlsl_functionality1)},
    {IR(1107, 21), static_cast<uint32_t>(kSPV_GOOGLE_user_type)},
    {IR(1128, 22), static_cast<uint32_t>(kSPV_INTEL_2d_block_io)},
    {IR(1150, 42), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_fixed_point)},
    {IR(1192, 45), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_floating_point)},
    {IR(1237, 39), static_cast<uint32_t>(kSPV_INTEL_arbitrary_precision_integers)},
    {IR(1276, 30), static_cast<uint32_t>(kSPV_INTEL_bfloat16_conversion)},
    {IR(1306, 26), static_cast<uint32_t>(kSPV_INTEL_bindless_images)},
    {IR(1332, 25), static_cast<uint32_t>(kSPV_INTEL_blocking_pipes)},
    {IR(1357, 25), static_cast<uint32_t>(kSPV_INTEL_cache_controls)},
    {IR(1382, 23), static_cast<uint32_t>(kSPV_INTEL_debug_module)},
    {IR(1405, 44), static_cast<uint32_t>(kSPV_INTEL_device_side_avc_motion_estimation)},
    {IR(1449, 26), static_cast<uint32_t>(kSPV_INTEL_float_controls2)},
    {IR(1475, 28), static_cast<uint32_t>(kSPV_INTEL_fp_fast_math_mode)},
    {IR(1503, 23), static_cast<uint32_t>(kSPV_INTEL_fp_max_error)},
    {IR(1526, 35), static_cast<uint32_t>(kSPV_INTEL_fpga_argument_interfaces)},
    {IR(1561, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_buffer_location)},
    {IR(1592, 34), static_cast<uint32_t>(kSPV_INTEL_fpga_cluster_attributes)},
    {IR(1626, 27), static_cast<uint32_t>(kSPV_INTEL_fpga_dsp_control)},
    {IR(1653, 48), static_cast<uint32_t>(kSPV_INTEL_fpga_invocation_pipelining_attributes)},
    {IR(1701, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_latency_control)},
    {IR(1732, 29), static_cast<uint32_t>(kSPV_INTEL_fpga_loop_controls)},
    {IR(1761, 31), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_accesses)},
    {IR(1792, 33), static_cast<uint32_t>(kSPV_INTEL_fpga_memory_attributes)},
    {IR(1825, 19), static_cast<uint32_t>(kSPV_INTEL_fpga_reg)},
    {IR(1844, 28), static_cast<uint32_t>(kSPV_INTEL_function_pointers)},
    {IR(1872, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(1915, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(1953, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(1979, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(1994, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2013, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2041, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2067, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2087, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2119, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2147, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2172, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(2205, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(2223, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(2249, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(2285, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(2309, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(2344, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(2390, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(2410, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(2434, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(2470, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(2505, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(2542, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(2572, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(2604, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(2629, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(2651, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(2672, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(2689, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(2714, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(2749, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(2776, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(2797, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(2819, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(2842, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(2866, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(2902, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(2932, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(2960, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(2981, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3011, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3029, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3064, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3090, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3122, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(3150, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(3171, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(3193, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(3211, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(3231, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(3266, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(3303, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(3337, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(3359, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(3380, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(3411, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(3448, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(3472, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(3510, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(3532, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(3561, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(3596, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(3621, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(3647, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(3675, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(3716, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(3754, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(3778, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(3816, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(3850, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(3876, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(3903, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(3929, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(3958, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(3993, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(4028, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4056, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4075, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4100, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(4119, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(4150, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(4187, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(4220, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(4250, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(4283, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(4309, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(4344, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(4364, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(4393, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(4418, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(4441, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(4467, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(4494, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(4516, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
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
static const std::array<OperandDesc, 1183> kOperandsByValue{{
  {0,IR(0, 0),IR(4554, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(4570, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(4580, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(4590, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(4598, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(4619, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(4630, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(4712, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(4728, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(4738, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(4751, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(4764, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(4773, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(4784, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(4874, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(4887, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(4933, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(4981, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(4996, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(5011, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(5021, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5035, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(5045, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(5056, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(5068, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(5095, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(5110, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(5121, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(5131, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(5148, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(5162, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(5176, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(5188, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(5206, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(5225, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(5246, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(5254, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(5265, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(5287, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(5300, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(5315, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(5362, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(5378, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(5391, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(5412, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(5423, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(5449, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(5461, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(5475, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(5501, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(5514, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(5527, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(5537, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(5550, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(5605, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(5638, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(5671, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(5704, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(5737, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(5763, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(5776, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(5786, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(5833, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(5857, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(5877, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(5892, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(5923, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(5941, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(5959, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(5979, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(6007, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6033, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(6052, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(6079, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(6104, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(6126, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(6161, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(6208, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(6224, 15)/* ViewportMaskNV */,IR(0, 0),IR(44, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(6260, 20)/* SecondaryPositionNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(6299, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(6323, 18)/* PositionPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(6361, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(6383, 16)/* FullyCoveredEXT */,IR(0, 0),IR(49, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(6423, 12)/* TaskCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(6435, 17)/* PrimitiveCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(6452, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(6471, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(6493, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(6515, 15)/* LayerPerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(6530, 16)/* MeshViewCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(6546, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(6564, 13)/* BaryCoordKHR */,IR(6, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(6612, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(6651, 12)/* FragSizeEXT */,IR(8, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(6697, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(6742, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(6767, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(6791, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(6819, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(6836, 12)/* LaunchIdKHR */,IR(10, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(6859, 14)/* LaunchSizeKHR */,IR(11, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(6886, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(6921, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(6962, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(6999, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(7042, 11)/* RayTminKHR */,IR(16, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(7063, 11)/* RayTmaxKHR */,IR(17, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(7084, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(7129, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(7162, 17)/* WorldToObjectKHR */,IR(20, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(7195, 7)/* HitTNV */,IR(0, 0),IR(56, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(7202, 11)/* HitKindKHR */,IR(21, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(7223, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(57, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(7263, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(58, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(7320, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(7387, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(7424, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(7463, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(60, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(7483, 14)/* HitIsSphereNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(7525, 11)/* HitIsLSSNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(7575, 20)/* HitSpherePositionNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(7595, 13)/* WarpsPerSMNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(7627, 10)/* SMCountNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(7637, 9)/* WarpIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(7646, 7)/* SMIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(7653, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(7671, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(7705, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(7738, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(7756, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(7770, 12)/* ClusterIDNV */,IR(0, 0),IR(64, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(7823, 12)/* CullMaskKHR */,IR(0, 0),IR(65, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(7850, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(4721, 7)/* Shader */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(4796, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(4805, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(4609, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(7857, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(4563, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(7865, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(7874, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(7888, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(7896, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(7904, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(7910, 13)/* Int64Atomics */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(7923, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(7934, 15)/* ImageReadWrite */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(7949, 12)/* ImageMipmap */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(7961, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(7967, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(7974, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(7988, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(8003, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(8017, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(8023, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(8045, 18)/* GeometryPointSize */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(8063, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(8083, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(8107, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(8141, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(8174, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(8208, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(4738, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(4751, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(8241, 15)/* ImageCubeArray */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(5077, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(8273, 10)/* ImageRect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(8283, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(8295, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(8310, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(8315, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(8331, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(8347, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(8354, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(8364, 8)/* Image1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(8256, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(8372, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(8386, 12)/* ImageBuffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(8398, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(8411, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(8439, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(8450, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(8468, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(8490, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(8508, 16)/* GeometryStreams */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(8524, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(8554, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(4947, 14)/* MultiViewport */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(8585, 17)/* SubgroupDispatch */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(8602, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(8615, 12)/* PipeStorage */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(5328, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(8627, 20)/* GroupNonUniformVote */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(8647, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(5583, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(8673, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(8696, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(8727, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(8752, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(4893, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(4961, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(8772, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(5485, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(8790, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(8818, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(8846, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(8876, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(8887, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(8924, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(8964, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(8973, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(9001, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(9011, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(77, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(5810, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(5344, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(5748, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(9059, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(9092, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(9135, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(9179, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(9195, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(9248, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(80, 1),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(9300, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(9322, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(5845, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(5867, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(9343, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(9373, 17)/* VariablePointers */,IR(0, 0),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(9390, 17)/* AtomicStorageOps */,IR(0, 0),IR(82, 1),IR(44, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(9407, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(9435, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(9459, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(83, 1),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(9493, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(9514, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(9529, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(9547, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(9572, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(9588, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(9604, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(9627, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(9639, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(9658, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(84, 2),IR(50, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(4831, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(9690, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(9716, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(9737, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(5907, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(9759, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(9782, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(9798, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(9820, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(6144, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(9836, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(9857, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(9871, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(6190, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(9886, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(9901, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(9915, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(86, 2),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(9942, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(9958, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(88, 1),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(9980, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(89, 2),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(10009, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(10038, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(69, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(4905, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(91, 1),IR(67, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(6239, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(92, 1),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(6280, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(93, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(6341, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(6399, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(4845, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(10093, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(4859, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(6589, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(10132, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(72, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(6678, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(10207, 29)/* GroupNonUniformPartitionedNV */,IR(0, 0),IR(1, 0),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(10236, 17)/* ShaderNonUniform */,IR(29, 1),IR(3, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(10273, 23)/* RuntimeDescriptorArray */,IR(30, 1),IR(3, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(10322, 36)/* InputAttachmentArrayDynamicIndexing */,IR(31, 1),IR(94, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(10397, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(32, 1),IR(73, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(10478, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(95, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(10559, 37)/* UniformBufferArrayNonUniformIndexing */,IR(34, 1),IR(96, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(10636, 36)/* SampledImageArrayNonUniformIndexing */,IR(35, 1),IR(96, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(10711, 37)/* StorageBufferArrayNonUniformIndexing */,IR(36, 1),IR(96, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(10788, 36)/* StorageImageArrayNonUniformIndexing */,IR(37, 1),IR(96, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(10863, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(38, 1),IR(97, 2),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(10944, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(39, 1),IR(99, 2),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(11031, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(101, 2),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(7293, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(4818, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(7240, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(11118, 18)/* VulkanMemoryModel */,IR(41, 1),IR(1, 0),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(11157, 29)/* VulkanMemoryModelDeviceScope */,IR(42, 1),IR(1, 0),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(4681, 31)/* PhysicalStorageBufferAddresses */,IR(43, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(11252, 32)/* ComputeDerivativeGroupLinearKHR */,IR(44, 1),IR(3, 1),IR(72, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(11315, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(11340, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(78, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(11360, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(11393, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(7608, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(11431, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(11463, 25)/* DemoteToHelperInvocation */,IR(45, 1),IR(3, 1),IR(80, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(11516, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(81, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(11539, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(11568, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(60, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(11594, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(84, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(11612, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(11637, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(11657, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(7354, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(60, 1),IR(81, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(11679, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(7497, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(7536, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(11697, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(11727, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(11758, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(11798, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(11834, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(11864, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(7782, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(60, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(11892, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(11911, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(11932, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(11959, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(11985, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(12016, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(12037, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(12060, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(12083, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(12105, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(12129, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(12138, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(12161, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(12184, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(12207, 19)/* VectorComputeINTEL */,IR(0, 0),IR(103, 1),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(12226, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(12241, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(12257, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(12290, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(12328, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(12367, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(12392, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(12418, 26)/* FPGAMemoryAttributesINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(12444, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(12464, 32)/* ArbitraryPrecisionIntegersINTEL */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(12496, 37)/* ArbitraryPrecisionFloatingPointINTEL */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(12533, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(12563, 22)/* FPGALoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(12585, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(12607, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(12633, 24)/* FPGAMemoryAccessesINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(12657, 27)/* FPGAClusterAttributesINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(12684, 14)/* LoopFuseINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(12698, 20)/* FPGADSPControlINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(12718, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(12744, 40)/* FPGAInvocationPipeliningAttributesINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(12784, 24)/* FPGABufferLocationINTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(12808, 34)/* ArbitraryPrecisionFixedPointINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(12842, 23)/* USMStorageClassesINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(12865, 29)/* RuntimeAlignedAttributeINTEL */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(12894, 13)/* IOPipesINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(12907, 19)/* BlockingPipesINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(12926, 13)/* FPGARegINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(12939, 19)/* DotProductInputAll */,IR(46, 1),IR(1, 0),IR(121, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(12980, 22)/* DotProductInput4x8Bit */,IR(47, 1),IR(104, 1),IR(121, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(13027, 28)/* DotProductInput4x8BitPacked */,IR(48, 1),IR(1, 0),IR(121, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(13086, 11)/* DotProduct */,IR(49, 1),IR(1, 0),IR(121, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(7835, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(9038, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(122, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(13111, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(123, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(13135, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(13151, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(76, 1),IR(125, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(13176, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(126, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(13191, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(13211, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(13231, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(13251, 11)/* OptNoneEXT */,IR(50, 1),IR(1, 0),IR(129, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(13275, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(131, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(13295, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(13316, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(133, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(13340, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(134, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(13358, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(135, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(13377, 29)/* FPGAClusterAttributesV2INTEL */,IR(0, 0),IR(105, 1),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(13406, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(106, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(13434, 18)/* TaskSequenceINTEL */,IR(0, 0),IR(1, 0),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(13452, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(137, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(13468, 24)/* FPGALatencyControlINTEL */,IR(0, 0),IR(1, 0),IR(138, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(13492, 28)/* FPGAArgumentInterfacesINTEL */,IR(0, 0),IR(1, 0),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(13520, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(140, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(13550, 35)/* GlobalVariableFPGADecorationsINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(13585, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(142, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(13613, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(13636, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(107, 1),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(13666, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(107, 1),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(13696, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(13734, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(145, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(13762, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(13788, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(13815, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(148, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(13840, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(13859, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(13879, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(13899, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13911, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13919, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(13927, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13933, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13940, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13951, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13960, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13973, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13979, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13989, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(13995, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14010, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14035, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14051, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14065, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14078, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14090, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14107, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14119, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14134, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14147, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14162, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14180, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14197, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14218, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14238, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14258, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14274, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14290, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14310, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14334, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14340, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14345, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(14384, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(14395, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14404, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14409, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14416, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(14427, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(14434, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14443, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14453, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14466, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14479, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(14490, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(14500, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(14510, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(14520, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(14533, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(14547, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(14561, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(14575, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(14590, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(14606, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(14622, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(14638, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(14657, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(14678, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(14690, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(14702, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14714, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(14729, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(14754, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(14782, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(14790, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(14817, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(14844, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(14871, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(14903, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(14929, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(14933, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(14940, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(14944, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(14955, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(14966, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(14987, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14998, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(15012, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(15033, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(13899, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13911, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13919, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13927, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13933, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13940, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13951, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(13960, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13973, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13979, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13989, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14035, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14051, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14065, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14078, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14090, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14107, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14119, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14134, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14147, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14162, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14180, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14197, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14218, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14238, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14258, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14334, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(14340, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(14345, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(14384, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(14395, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(14404, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(14409, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(14416, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(14427, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(14443, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14453, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14466, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(15051, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(15068, 7)/* SpecId */,IR(0, 0),IR(108, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(15075, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(15081, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(15093, 9)/* RowMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(15102, 9)/* ColMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(15111, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(15123, 13)/* MatrixStride */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(15136, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(15147, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(15158, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(15192, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(15200, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(15214, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(15219, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(15225, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(15234, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(15241, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(15251, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(15260, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(15268, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(15277, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(15286, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(15295, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(15307, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(15319, 8)/* Uniform */,IR(0, 0),IR(110, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(15353, 10)/* UniformId */,IR(0, 0),IR(110, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(15363, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(15383, 7)/* Stream */,IR(0, 0),IR(112, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(15390, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(15399, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(15409, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(15415, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(15423, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(15437, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(15444, 10)/* XfbBuffer */,IR(0, 0),IR(113, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(15454, 10)/* XfbStride */,IR(0, 0),IR(113, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(15510, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(15558, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(15608, 15)/* FPFastMathMode */,IR(0, 0),IR(114, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(15685, 18)/* LinkageAttributes */,IR(0, 0),IR(116, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(15703, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(15717, 21)/* InputAttachmentIndex */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(15738, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(15748, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(15782, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(15794, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(15810, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(15854, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(152, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(15867, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(152, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(15882, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(15900, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(15922, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(15944, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(15962, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(15994, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(16014, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(16037, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(16057, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(16082, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(16109, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(16134, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(16162, 19)/* OverrideCoverageNV */,IR(0, 0),IR(118, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(16181, 14)/* PassthroughNV */,IR(0, 0),IR(119, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(16195, 19)/* ViewportRelativeNV */,IR(0, 0),IR(93, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(16214, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(16242, 16)/* PerPrimitiveEXT */,IR(51, 1),IR(120, 2),IR(153, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(16273, 10)/* PerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(16283, 10)/* PerTaskNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(16293, 13)/* PerVertexKHR */,IR(52, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(16318, 11)/* NonUniform */,IR(53, 1),IR(96, 1),IR(75, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(16343, 16)/* RestrictPointer */,IR(54, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(16378, 15)/* AliasedPointer */,IR(55, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(16411, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(122, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(16441, 18)/* BindlessSamplerNV */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(16459, 16)/* BindlessImageNV */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(16475, 15)/* BoundSamplerNV */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(16490, 13)/* BoundImageNV */,IR(0, 0),IR(123, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(16503, 14)/* SIMTCallINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(16517, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(125, 1),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(16543, 13)/* ClobberINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(16556, 17)/* SideEffectsINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(16573, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(16600, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(16621, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(16648, 15)/* StackCallINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(16663, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(16689, 14)/* CounterBuffer */,IR(56, 1),IR(1, 0),IR(155, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(16727, 13)/* UserSemantic */,IR(57, 1),IR(1, 0),IR(155, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(16759, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(156, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(16774, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(16831, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(16855, 14)/* RegisterINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(16869, 12)/* MemoryINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(16881, 14)/* NumbanksINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(16895, 15)/* BankwidthINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(16910, 22)/* MaxPrivateCopiesINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(16932, 16)/* SinglepumpINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(16948, 16)/* DoublepumpINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(16964, 19)/* MaxReplicatesINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(16983, 20)/* SimpleDualPortINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(17003, 11)/* MergeINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(17056, 14)/* BankBitsINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(17070, 20)/* ForcePow2DepthINTEL */,IR(0, 0),IR(128, 1),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(17090, 16)/* StridesizeINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(17106, 14)/* WordsizeINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(17120, 18)/* TrueDualPortINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(17138, 19)/* BurstCoalesceINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(17157, 15)/* CacheSizeINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(17172, 28)/* DontStaticallyCoalesceINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(17200, 14)/* PrefetchINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(17214, 17)/* StallEnableINTEL */,IR(0, 0),IR(105, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(17231, 25)/* FuseLoopsInFunctionINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(17256, 19)/* MathOpDSPModeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(17275, 16)/* AliasScopeINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(17291, 13)/* NoAliasINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(17304, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(17328, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(17348, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(17368, 20)/* BufferLocationINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(17388, 19)/* IOPipeStorageINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(17441, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(17472, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(17497, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(17532, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(17550, 15)/* StallFreeINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(17596, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(17622, 25)/* LatencyControlLabelINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(17647, 30)/* LatencyControlConstraintINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(17677, 27)/* ConduitKernelArgumentINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(17704, 31)/* RegisterMapKernelArgumentINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(17735, 33)/* MMHostInterfaceAddressWidthINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(17768, 30)/* MMHostInterfaceDataWidthINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(17798, 28)/* MMHostInterfaceLatencyINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(17860, 34)/* MMHostInterfaceReadWriteModeINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(17894, 29)/* MMHostInterfaceMaxBurstINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(17923, 32)/* MMHostInterfaceWaitRequestINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(17955, 26)/* StableKernelArgumentINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(18020, 16)/* HostAccessINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(18083, 14)/* InitModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(18097, 28)/* ImplementInRegisterMapINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(18161, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(18220, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(18243, 3)/* 1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(18246, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(18249, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(18252, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(18257, 5)/* Rect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(18262, 7)/* Buffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(18269, 12)/* SubpassData */,IR(0, 0),IR(94, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(18281, 17)/* TileImageDataEXT */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(18298, 12)/* Invocations */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(18310, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(18323, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(18345, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(18366, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(18380, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(18395, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(18414, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(18430, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(18446, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(18465, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(18475, 4)/* Xfb */,IR(0, 0),IR(113, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(18479, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(18494, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(18507, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(18517, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(18532, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(18542, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(18556, 12)/* InputPoints */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(18568, 11)/* InputLines */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(18579, 20)/* InputLinesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(18599, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(18609, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(18633, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(18639, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(18648, 15)/* OutputVertices */,IR(0, 0),IR(144, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(18663, 13)/* OutputPoints */,IR(0, 0),IR(148, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(18676, 16)/* OutputLineStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(18692, 20)/* OutputTriangleStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(18712, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(18724, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(18739, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(18751, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(5315, 13)/* SubgroupSize */,IR(0, 0),IR(151, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(18761, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(151, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(18783, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(151, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(18807, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(18819, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(18835, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(18869, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(18903, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(18939, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(157, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(18969, 18)/* PostDepthCoverage */,IR(0, 0),IR(154, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(9514, 15)/* DenormPreserve */,IR(0, 0),IR(155, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(9529, 18)/* DenormFlushToZero */,IR(0, 0),IR(156, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(9547, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(157, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(9572, 16)/* RoundingModeRTE */,IR(0, 0),IR(158, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(9588, 16)/* RoundingModeRTZ */,IR(0, 0),IR(159, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(18987, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(19021, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(19041, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(158, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(19070, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(19093, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(19108, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(19123, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(19144, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(6208, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(19168, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(19189, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(19217, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(19243, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(19266, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(19293, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(19318, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(159, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(19340, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(160, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(19359, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(160, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(19379, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(19399, 15)/* OutputLinesEXT */,IR(58, 1),IR(120, 2),IR(153, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(19428, 20)/* OutputPrimitivesEXT */,IR(59, 1),IR(120, 2),IR(153, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(19467, 24)/* DerivativeGroupQuadsKHR */,IR(60, 1),IR(161, 2),IR(72, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(19514, 25)/* DerivativeGroupLinearKHR */,IR(61, 1),IR(163, 2),IR(72, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(19563, 19)/* OutputTrianglesEXT */,IR(62, 1),IR(120, 2),IR(153, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(19600, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(165, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(19625, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(165, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(19652, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(166, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(19678, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(166, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(19706, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(167, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(19737, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(167, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(19770, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(19797, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(19818, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(19839, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(19865, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(19892, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(169, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(19914, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(169, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(19930, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(169, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(19950, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(106, 1),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(19972, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(106, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(20000, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(20024, 18)/* FPFastMathDefault */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(20042, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(106, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(20066, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(20092, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(20115, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(20137, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(20212, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(20239, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(20246, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(20266, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(4796, 9)/* Geometry */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(14434, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(20289, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(4563, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(20299, 7)/* TaskNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(20306, 7)/* MeshNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(20313, 17)/* RayGenerationKHR */,IR(63, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(20346, 16)/* IntersectionKHR */,IR(64, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(20377, 10)/* AnyHitKHR */,IR(65, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(20396, 14)/* ClosestHitKHR */,IR(66, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(20423, 8)/* MissKHR */,IR(67, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(20438, 12)/* CallableKHR */,IR(68, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(20461, 8)/* TaskEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(20469, 8)/* MeshEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(20477, 9)/* Preserve */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(20486, 12)/* FlushToZero */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(20498, 12)/* BFloat16KHR */,IR(0, 0),IR(88, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(20510, 14)/* Float8E4M3EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(20524, 14)/* Float8E5M2EXT */,IR(0, 0),IR(117, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(20538, 5)/* IEEE */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(20543, 4)/* ALT */,IR(0, 0),IR(127, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(20547, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(20554, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(20561, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(20565, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(20576, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(20581, 14)/* AllowContract */,IR(69, 1),IR(173, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(20618, 13)/* AllowReassoc */,IR(70, 1),IR(173, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(20649, 15)/* AllowTransform */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(20664, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20668, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(20672, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(20676, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20680, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(20696, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(20712, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(20730, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(20748, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(20755, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(20766, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(20771, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(13251, 11)/* OptNoneEXT */,IR(50, 1),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(20777, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(20782, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(20787, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(20793, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(20798, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(20806, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(20816, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(20824, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(20836, 20)/* RuntimeAlignedINTEL */,IR(0, 0),IR(176, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(20856, 7)/* Reduce */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(20863, 14)/* InclusiveScan */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(20877, 14)/* ExclusiveScan */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(20891, 16)/* ClusteredReduce */,IR(0, 0),IR(180, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(20907, 20)/* PartitionedReduceNV */,IR(0, 0),IR(181, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(20927, 27)/* PartitionedInclusiveScanNV */,IR(0, 0),IR(181, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(20954, 27)/* PartitionedExclusiveScanNV */,IR(0, 0),IR(181, 1),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(20981, 10)/* NoneINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(20991, 10)/* ReadINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(21001, 11)/* WriteINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(21012, 15)/* ReadWriteINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(21027, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(21032, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(21036, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(21041, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(15437, 7)/* Offset */,IR(0, 0),IR(182, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(21053, 13)/* ConstOffsets */,IR(0, 0),IR(182, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(15234, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(8347, 7)/* MinLod */,IR(0, 0),IR(183, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(21066, 19)/* MakeTexelAvailable */,IR(71, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(21107, 17)/* MakeTexelVisible */,IR(72, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(21144, 16)/* NonPrivateTexel */,IR(73, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(21179, 14)/* VolatileTexel */,IR(74, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(21210, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(21221, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(21232, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(21244, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(21252, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(21262, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(21273, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(21283, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(21294, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(21308, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(21322, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(21337, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(21348, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(21360, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(21372, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(21385, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(21399, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(21413, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(13927, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(21423, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(21434, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(21451, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(21467, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(21487, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(21507, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(21527, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(21546, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(21565, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(21584, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(21600, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(21616, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(21618, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(21620, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(21623, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(21626, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(21630, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(21635, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(21640, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(21645, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(21655, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(21665, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(21668, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(21672, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(21677, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(21683, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(21696, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(21701, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(21707, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(21713, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(21719, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(21724, 27)/* InitOnDeviceReprogramINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(21751, 23)/* InitOnDeviceResetINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(21774, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(21781, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(21792, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(21806, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(21818, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(21831, 7)/* Export */,IR(0, 0),IR(116, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(21838, 7)/* Import */,IR(0, 0),IR(116, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(21845, 12)/* LinkOnceODR */,IR(0, 0),IR(116, 1),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(21857, 14)/* UncachedINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(21871, 12)/* CachedINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(21883, 15)/* StreamingINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(21898, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(21923, 17)/* ConstCachedINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(21940, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(21947, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(21958, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(21977, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(21994, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(22008, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(22022, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(22040, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(22050, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(17304, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(17328, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(22063, 21)/* DependencyArrayINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(17348, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(22084, 18)/* LoopCoalesceINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(22102, 21)/* MaxInterleavingINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(22123, 26)/* SpeculatedIterationsINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(22149, 14)/* NoFusionINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(22163, 15)/* LoopCountINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(22178, 26)/* MaxReinvocationDelayINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(22204, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(22233, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(22262, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(22283, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(22309, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(22332, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(22355, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(22378, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(22401, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(22418, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(22435, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(22461, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(22487, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(22514, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(15268, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(22541, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(21232, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(22549, 21)/* MakePointerAvailable */,IR(75, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(22594, 19)/* MakePointerVisible */,IR(76, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(22635, 18)/* NonPrivatePointer */,IR(77, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(22674, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(132, 1),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(22694, 17)/* NoAliasINTELMask */,IR(0, 0),IR(132, 1),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(22711, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(22718, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(22726, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(22733, 7)/* Vulkan */,IR(78, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(22750, 8)/* Relaxed */,IR(79, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(22758, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(22766, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(22774, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(22789, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(22812, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(22826, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(22841, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(22857, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(22878, 20)/* AtomicCounterMemory */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(22898, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(22910, 13)/* OutputMemory */,IR(80, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(22939, 14)/* MakeAvailable */,IR(81, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(22970, 12)/* MakeVisible */,IR(82, 1),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(15268, 9)/* Volatile */,IR(0, 0),IR(184, 1),IR(77, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(22997, 10)/* AutoINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(23007, 5)/* WRAP */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(23012, 4)/* SAT */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(23016, 9)/* SAT_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(23025, 8)/* SAT_SYM */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(23033, 25)/* PackedVectorFormat4x8Bit */,IR(83, 1),IR(1, 0),IR(121, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(23086, 4)/* TRN */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(23090, 9)/* TRN_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(23099, 4)/* RND */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(23103, 9)/* RND_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(23112, 8)/* RND_INF */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(23120, 12)/* RND_MIN_INF */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(23132, 9)/* RND_CONV */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(23141, 13)/* RND_CONV_ODD */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(23154, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(23179, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(14782, 8)/* NoneKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(23202, 10)/* OpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(23212, 12)/* NoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(23224, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(23247, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(23271, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(23298, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(23326, 14)/* CullOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(23340, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(23356, 17)/* SkipTrianglesKHR */,IR(84, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(23397, 13)/* SkipAABBsKHR */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(23410, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(23440, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23481, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23518, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23555, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(23596, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23638, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(23671, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(23704, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(23716, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(23722, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(23729, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(23744, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(23752, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(23759, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(23767, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(23775, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(23783, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(23788, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(23794, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(23805, 6)/* Rg32f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(23811, 6)/* Rg16f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(23817, 13)/* R11fG11fB10f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(23830, 5)/* R16f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(23835, 7)/* Rgba16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(23842, 8)/* Rgb10A2 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(23850, 5)/* Rg16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(23855, 4)/* Rg8 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(23859, 4)/* R16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(23863, 3)/* R8 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(23866, 12)/* Rgba16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(23878, 10)/* Rg16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(23888, 9)/* Rg8Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(23897, 9)/* R16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(23906, 8)/* R8Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(23914, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(23922, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(23930, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(23937, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(23942, 6)/* Rg32i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(23948, 6)/* Rg16i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(23954, 5)/* Rg8i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(23959, 5)/* R16i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(23964, 4)/* R8i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(23968, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(23977, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(23986, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(23994, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(24000, 10)/* Rgb10a2ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(24010, 7)/* Rg32ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(24017, 7)/* Rg16ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(24024, 6)/* Rg8ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(24030, 6)/* R16ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(24036, 5)/* R8ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(24041, 6)/* R64ui */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(24047, 5)/* R64i */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(24052, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(24064, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(24071, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(24081, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(24090, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(24101, 12)/* QueueFamily */,IR(85, 1),IR(184, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(24128, 14)/* ShaderCallKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24142, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(24150, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24162, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(13899, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(13911, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(13919, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(13927, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(13933, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(13940, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(13951, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(13960, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(13973, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(13979, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(13989, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(13995, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14010, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14035, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14051, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14065, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14078, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14090, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14107, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14119, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14134, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14147, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14162, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14180, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14197, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14218, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14238, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14258, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14274, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14290, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14310, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(24191, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14334, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14340, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14345, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(14384, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(14395, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14404, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14409, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14416, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(14427, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(14434, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14443, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14453, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14466, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14479, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(23759, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(24217, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(24222, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(24227, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(24236, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(24247, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(24252, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(24267, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(24272, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(24279, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(24284, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(24289, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(24295, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(24299, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(24304, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(24320, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(15319, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(24326, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(24071, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(24333, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(24348, 8)/* Private */,IR(0, 0),IR(193, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(24356, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(24365, 8)/* Generic */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(24373, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(24386, 14)/* AtomicCounter */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(24400, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(24406, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(163, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(24420, 13)/* TileImageEXT */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(24433, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(24452, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(24468, 16)/* CallableDataKHR */,IR(86, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(24499, 24)/* IncomingCallableDataKHR */,IR(87, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(24546, 14)/* RayPayloadKHR */,IR(88, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(24573, 16)/* HitAttributeKHR */,IR(89, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(24604, 22)/* IncomingRayPayloadKHR */,IR(90, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(24647, 22)/* ShaderRecordBufferKHR */,IR(91, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(24690, 22)/* PhysicalStorageBuffer */,IR(92, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(24737, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(122, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(24758, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(53, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(24782, 17)/* CodeSectionINTEL */,IR(0, 0),IR(196, 1),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(24799, 16)/* DeviceOnlyINTEL */,IR(0, 0),IR(197, 1),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(24815, 14)/* HostOnlyINTEL */,IR(0, 0),IR(197, 1),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(21857, 14)/* UncachedINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(24829, 18)/* WriteThroughINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(24847, 15)/* WriteBackINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(21883, 15)/* StreamingINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(14030, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(24862, 11)/* TensorView */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(24873, 11)/* DecodeFunc */,IR(0, 0),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(24884, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(15277, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(23704, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(23722, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(23729, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(24894, 8)/* NoneARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(24902, 15)/* NontemporalARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(24917, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(24937, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(24961, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(24983, 21)/* NonPrivateElementARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(133, 270);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(403, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(411, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(414, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(416, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(435, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(445, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(449, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(464, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(468, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(474, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(477, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(480, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(484, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(492, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(495, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(511, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(520, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(523, 143);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(666, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(674, 96);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(770, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(787, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(789, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(792, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(794, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(803, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(807, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(811, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(817, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(826, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(833, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(837, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(854, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(880, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(900, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(902, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(905, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(907, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(909, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(912, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(917, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(937, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(952, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(961, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(965, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(980, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(981, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(985, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(986, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(994, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(997, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1009, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1011, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1014, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1016, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1021, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1023, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1065, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1072, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1075, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1076, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1084, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1087, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1089, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1109, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1119, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1123, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1137, 28);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1165, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1169, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1172, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1177, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(837, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(952, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(985, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(468, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(937, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(994, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(789, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1177, 6);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1277> kOperandNames{{
  {IR(4554, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4580, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4570, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4590, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4598, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4619, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4630, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4654, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6564, 13), 92}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6577, 12), 92}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5959, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5979, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6612, 20), 93}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6632, 19), 93}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6007, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6104, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6033, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6052, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6079, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5763, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5737, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4738, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6471, 22), 87}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7770, 12), 131}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5501, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5475, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5514, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4751, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6493, 22), 88}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7823, 12), 132}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6819, 17), 99}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7223, 17), 113}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5833, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5776, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5265, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5035, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5121, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6697, 23), 95}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6651, 12), 94}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6126, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6663, 15), 94}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5056, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6383, 16), 83}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5206, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5300, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5287, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5254, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5131, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7525, 11), 120}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7483, 14), 119}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7705, 33), 128}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7671, 34), 127}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7202, 11), 112}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7213, 10), 112}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7653, 18), 126}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7756, 14), 130}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7387, 37), 116}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7320, 34), 115}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7575, 20), 121}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7738, 18), 129}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7195, 7), 111}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7263, 30), 114}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7424, 20), 117}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7444, 19), 117}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7084, 23), 108}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7107, 22), 108}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4773, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5461, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4874, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6720, 22), 95}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6836, 12), 100}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6848, 11), 100}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6859, 14), 101}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6873, 13), 101}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4887, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6515, 15), 89}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5188, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5225, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6530, 16), 90}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6546, 18), 91}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5391, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5378, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5148, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6999, 22), 105}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7021, 21), 105}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6962, 19), 104}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6981, 18), 104}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7129, 17), 109}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7146, 16), 109}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5021, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5045, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4728, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4712, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6323, 18), 81}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6435, 17), 85}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4784, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6452, 19), 86}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6767, 24), 97}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6742, 25), 96}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5786, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6791, 28), 98}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7463, 20), 118}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7063, 11), 107}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7074, 10), 107}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7042, 11), 106}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7053, 10), 106}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6161, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7627, 10), 123}, // 107 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7646, 7), 125}, // 108 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5068, 9), 24}, // 109 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5110, 11), 26}, // 110 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5095, 15), 25}, // 111 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6260, 20), 79}, // 112 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6299, 24), 80}, // 113 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6208, 16), 77}, // 114 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5877, 15), 64}, // 115 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5550, 15), 53}, // 116 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5565, 18), 53}, // 117 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5605, 15), 54}, // 118 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5620, 18), 54}, // 119 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5638, 15), 55}, // 120 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5653, 18), 55}, // 121 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5412, 11), 44}, // 122 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5671, 15), 56}, // 123 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5686, 18), 56}, // 124 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5423, 26), 45}, // 125 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5704, 15), 57}, // 126 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5719, 18), 57}, // 127 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5362, 16), 41}, // 128 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5315, 13), 40}, // 129 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6423, 12), 84}, // 130 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5011, 10), 19}, // 131 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4996, 15), 18}, // 132 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4981, 15), 17}, // 133 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5941, 18), 67}, // 134 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5923, 18), 66}, // 135 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5892, 15), 65}, // 136 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4764, 9), 11}, // 137 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5449, 12), 46}, // 138 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5857, 10), 63}, // 139 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4933, 14), 16}, // 140 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6224, 15), 78}, // 141 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6361, 22), 82}, // 142 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5527, 10), 51}, // 143 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7637, 9), 124}, // 144 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5537, 13), 52}, // 145 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7595, 13), 122}, // 146 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5246, 8), 35}, // 147 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5176, 12), 31}, // 148 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5162, 14), 30}, // 149 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6921, 21), 103}, // 150 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6942, 20), 103}, // 151 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6886, 18), 102}, // 152 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6904, 17), 102}, // 153 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7162, 17), 110}, // 154 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7179, 16), 110}, // 155 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4609, 10), 137}, // 156 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12808, 34), 358}, // 157 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12496, 37), 346}, // 158 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12464, 32), 345}, // 159 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13358, 19), 382}, // 160 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12129, 9), 331}, // 161 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13275, 20), 378}, // 162 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12184, 23), 334}, // 163 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11657, 22), 309}, // 164 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13191, 20), 374}, // 165 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12138, 23), 332}, // 166 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13211, 20), 375}, // 167 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12161, 23), 333}, // 168 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8003, 14), 153}, // 169 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9390, 17), 229}, // 170 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13316, 24), 380}, // 171 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9980, 29), 262}, // 172 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9958, 22), 261}, // 173 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9942, 16), 260}, // 174 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13879, 20), 402}, // 175 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11594, 18), 306}, // 176 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13135, 16), 371}, // 177 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12907, 19), 362}, // 178 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13840, 19), 400}, // 179 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4738, 13), 163}, // 180 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11252, 32), 295}, // 181 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11284, 31), 295}, // 182 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10132, 31), 274}, // 183 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10163, 30), 274}, // 184 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11834, 30), 318}, // 185 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11727, 31), 315}, // 186 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9038, 21), 369}, // 187 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8973, 28), 211}, // 188 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11340, 20), 297}, // 189 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11758, 40), 316}, // 190 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11697, 30), 314}, // 191 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11798, 36), 317}, // 192 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11637, 20), 308}, // 193 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11864, 28), 319}, // 194 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5485, 16), 203}, // 195 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4751, 13), 164}, // 196 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13295, 21), 379}, // 197 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11463, 25), 302}, // 198 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11488, 28), 302}, // 199 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9529, 18), 235}, // 200 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9514, 15), 234}, // 201 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8450, 18), 182}, // 202 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7974, 14), 151}, // 203 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5845, 12), 225}, // 204 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11516, 23), 303}, // 205 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13086, 11), 367}, // 206 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12980, 22), 365}, // 207 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13002, 25), 365}, // 208 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13027, 28), 366}, // 209 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13055, 31), 366}, // 210 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12939, 19), 364}, // 211 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12958, 22), 364}, // 212 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13097, 14), 367}, // 213 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5748, 15), 216}, // 214 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12241, 16), 337}, // 215 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12444, 20), 344}, // 216 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13492, 28), 388}, // 217 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12784, 24), 357}, // 218 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12657, 27), 352}, // 219 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13377, 29), 383}, // 220 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12698, 20), 354}, // 221 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12744, 40), 356}, // 222 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12607, 26), 350}, // 223 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13406, 28), 384}, // 224 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13468, 24), 387}, // 225 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12563, 22), 348}, // 226 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12633, 24), 351}, // 227 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12418, 26), 343}, // 228 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12926, 13), 363}, // 229 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13452, 16), 386}, // 230 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7888, 8), 142}, // 231 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7874, 14), 141}, // 232 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9782, 16), 249}, // 233 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7896, 8), 143}, // 234 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9011, 27), 213}, // 235 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9001, 10), 212}, // 236 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13176, 15), 373}, // 237 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12037, 23), 327}, // 238 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6589, 23), 273}, // 239 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10110, 22), 273}, // 240 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6678, 19), 275}, // 241 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6399, 24), 269}, // 242 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9820, 16), 251}, // 243 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11431, 32), 301}, // 244 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11360, 33), 298}, // 245 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11393, 38), 299}, // 246 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5810, 23), 214}, // 247 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12392, 26), 342}, // 248 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12083, 22), 329}, // 249 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8295, 15), 169}, // 250 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4796, 9), 135}, // 251 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8045, 18), 156}, // 252 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10038, 28), 264}, // 253 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8508, 16), 185}, // 254 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13550, 35), 390}, // 255 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13520, 30), 389}, // 256 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8964, 9), 210}, // 257 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5328, 16), 192}, // 258 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8647, 26), 194}, // 259 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5583, 22), 195}, // 260 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8727, 25), 198}, // 261 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10207, 29), 276}, // 262 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8752, 20), 199}, // 263 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13151, 25), 372}, // 264 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8673, 23), 196}, // 265 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8696, 31), 197}, // 266 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8627, 20), 193}, // 267 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13762, 26), 397}, // 268 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7967, 7), 150}, // 269 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12894, 13), 361}, // 270 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8364, 8), 175}, // 271 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7923, 11), 146}, // 272 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8386, 12), 178}, // 273 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8241, 15), 165}, // 274 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10093, 17), 271}, // 275 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9798, 22), 250}, // 276 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8063, 20), 157}, // 277 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8398, 13), 179}, // 278 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7949, 12), 148}, // 279 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8439, 11), 181}, // 280 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7934, 15), 147}, // 281 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9836, 21), 253}, // 282 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8273, 10), 167}, // 283 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12105, 24), 330}, // 284 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8315, 16), 171}, // 285 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10322, 36), 279}, // 286 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10358, 39), 279}, // 287 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10863, 39), 286}, // 288 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10902, 42), 286}, // 289 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8017, 6), 154}, // 290 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9915, 27), 259}, // 291 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9901, 14), 258}, // 292 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7904, 6), 144}, // 293 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7910, 13), 145}, // 294 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9857, 14), 254}, // 295 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8310, 5), 170}, // 296 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12060, 23), 328}, // 297 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8468, 22), 183}, // 298 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4563, 7), 139}, // 299 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12585, 22), 349}, // 300 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7857, 8), 138}, // 301 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7988, 15), 152}, // 302 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13231, 20), 376}, // 303 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12684, 14), 353}, // 304 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13815, 25), 399}, // 305 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7850, 7), 133}, // 306 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12718, 26), 355}, // 307 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4859, 15), 272}, // 308 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4845, 14), 270}, // 309 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8347, 7), 173}, // 310 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5867, 10), 226}, // 311 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4947, 14), 188}, // 312 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8602, 13), 190}, // 313 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13251, 11), 377}, // 314 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13262, 13), 377}, // 315 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6341, 20), 268}, // 316 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4681, 31), 294}, // 317 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11218, 34), 294}, // 318 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8615, 12), 191}, // 319 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7961, 6), 149}, // 320 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9886, 15), 257}, // 321 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11679, 18), 311}, // 322 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7835, 15), 368}, // 323 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9627, 12), 240}, // 324 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11612, 25), 307}, // 325 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9604, 23), 239}, // 326 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7782, 41), 320}, // 327 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7354, 33), 310}, // 328 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4831, 14), 243}, // 329 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7536, 39), 313}, // 330 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7240, 23), 291}, // 331 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4818, 13), 290}, // 332 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11539, 29), 304}, // 333 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7293, 27), 289}, // 334 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11315, 25), 296}, // 335 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7497, 28), 312}, // 336 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9658, 32), 242}, // 337 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13859, 20), 401}, // 338 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13111, 24), 370}, // 339 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12016, 21), 326}, // 340 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9572, 16), 237}, // 341 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9588, 16), 238}, // 342 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12865, 29), 360}, // 343 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10273, 23), 278}, // 344 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10296, 26), 278}, // 345 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10009, 29), 263}, // 346 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9407, 28), 230}, // 347 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5077, 18), 166}, // 348 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8354, 10), 174}, // 349 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8372, 14), 177}, // 350 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8256, 17), 176}, // 351 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8141, 33), 160}, // 352 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10636, 36), 283}, // 353 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10672, 39), 283}, // 354 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8283, 12), 168}, // 355 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4721, 7), 134}, // 356 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9871, 15), 255}, // 357 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6190, 18), 256}, // 358 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11568, 26), 305}, // 359 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4893, 12), 200}, // 360 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10236, 17), 277}, // 361 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10253, 20), 277}, // 362 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7608, 19), 300}, // 363 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6280, 19), 267}, // 364 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4961, 20), 201}, // 365 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4905, 28), 265}, // 366 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10066, 27), 265}, // 367 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6239, 21), 266}, // 368 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10193, 14), 275}, // 369 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9547, 25), 236}, // 370 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8331, 16), 172}, // 371 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13340, 18), 381}, // 372 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6144, 17), 252}, // 373 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9195, 25), 221}, // 374 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9435, 24), 231}, // 375 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8174, 34), 161}, // 376 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10711, 37), 284}, // 377 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10748, 40), 284}, // 378 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8208, 33), 162}, // 379 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10788, 36), 285}, // 380 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10824, 39), 285}, // 381 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8411, 28), 180}, // 382 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8083, 24), 158}, // 383 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8524, 30), 186}, // 384 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8554, 31), 187}, // 385 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9322, 21), 224}, // 386 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9300, 22), 223}, // 387 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9493, 21), 233}, // 388 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8887, 37), 208}, // 389 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8924, 40), 209}, // 390 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10478, 39), 281}, // 391 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10517, 42), 281}, // 392 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11031, 42), 288}, // 393 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11073, 45), 288}, // 394 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9283, 17), 222}, // 395 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9220, 28), 221}, // 396 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13613, 23), 392}, // 397 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13636, 30), 393}, // 398 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13666, 30), 394}, // 399 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12328, 39), 340}, // 400 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12257, 33), 338}, // 401 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12290, 38), 339}, // 402 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5344, 18), 215}, // 403 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11932, 27), 323}, // 404 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13585, 28), 391}, // 405 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8585, 17), 189}, // 406 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11959, 26), 324}, // 407 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11985, 31), 325}, // 408 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13696, 38), 395}, // 409 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11911, 21), 322}, // 410 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9179, 16), 220}, // 411 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13434, 18), 385}, // 412 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11892, 19), 321}, // 413 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13788, 27), 398}, // 414 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8876, 11), 207}, // 415 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13734, 28), 396}, // 416 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4805, 13), 136}, // 417 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8023, 22), 155}, // 418 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9759, 23), 248}, // 419 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9737, 22), 246}, // 420 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9716, 21), 245}, // 421 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9690, 26), 244}, // 422 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8790, 28), 204}, // 423 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8818, 28), 205}, // 424 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8846, 30), 206}, // 425 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5907, 16), 247}, // 426 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8490, 18), 184}, // 427 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12842, 23), 359}, // 428 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9248, 35), 222}, // 429 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9459, 34), 232}, // 430 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8107, 34), 159}, // 431 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10559, 37), 282}, // 432 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10596, 40), 282}, // 433 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8772, 18), 202}, // 434 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10397, 39), 280}, // 435 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10436, 42), 280}, // 436 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10944, 42), 287}, // 437 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10986, 45), 287}, // 438 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12533, 30), 347}, // 439 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9639, 19), 241}, // 440 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12367, 25), 341}, // 441 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9373, 17), 228}, // 442 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9343, 30), 227}, // 443 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7865, 9), 140}, // 444 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12226, 15), 336}, // 445 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12207, 19), 335}, // 446 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11118, 18), 292}, // 447 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11157, 29), 293}, // 448 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11186, 32), 293}, // 449 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11136, 21), 292}, // 450 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9135, 44), 219}, // 451 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9092, 43), 218}, // 452 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9059, 33), 217}, // 453 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13911, 8), 404}, // 454 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13919, 8), 405}, // 455 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13927, 6), 406}, // 456 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13933, 7), 407}, // 457 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13940, 11), 408}, // 458 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13951, 9), 409}, // 459 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13960, 13), 410}, // 460 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13899, 12), 403}, // 461 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13973, 6), 411}, // 462 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13979, 10), 412}, // 463 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13989, 6), 413}, // 464 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14010, 20), 415}, // 465 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(13995, 15), 414}, // 466 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14119, 15), 423}, // 467 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14134, 13), 424}, // 468 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14107, 12), 422}, // 469 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14197, 21), 428}, // 470 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14090, 17), 421}, // 471 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14274, 16), 432}, // 472 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14078, 12), 420}, // 473 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14258, 16), 431}, // 474 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14051, 14), 418}, // 475 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14035, 16), 417}, // 476 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14065, 13), 419}, // 477 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14218, 20), 429}, // 478 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14162, 18), 426}, // 479 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14147, 15), 425}, // 480 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14238, 20), 430}, // 481 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14180, 17), 427}, // 482 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14310, 24), 434}, // 483 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14290, 20), 433}, // 484 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14030, 5), 416}, // 485 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14395, 9), 439}, // 486 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14427, 7), 443}, // 487 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14334, 6), 435}, // 488 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14434, 9), 444}, // 489 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14345, 6), 437}, // 490 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14340, 5), 436}, // 491 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14384, 11), 438}, // 492 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14416, 11), 442}, // 493 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14404, 5), 440}, // 494 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14409, 7), 441}, // 495 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14479, 11), 448}, // 496 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14443, 10), 445}, // 497 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14466, 13), 447}, // 498 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14453, 13), 446}, // 499 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14490, 10), 449}, // 500 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14500, 10), 450}, // 501 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14510, 10), 451}, // 502 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14678, 12), 462}, // 503 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14690, 12), 463}, // 504 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14533, 14), 453}, // 505 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14547, 14), 454}, // 506 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14561, 14), 455}, // 507 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14520, 13), 452}, // 508 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14638, 19), 460}, // 509 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14590, 16), 457}, // 510 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14606, 16), 458}, // 511 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14622, 16), 459}, // 512 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14575, 15), 456}, // 513 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14657, 21), 461}, // 514 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14754, 28), 467}, // 515 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14714, 15), 465}, // 516 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14729, 25), 466}, // 517 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14702, 12), 464}, // 518 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14790, 27), 469}, // 519 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14817, 27), 470}, // 520 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14844, 27), 471}, // 521 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14871, 32), 472}, // 522 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14782, 8), 468}, // 523 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14903, 26), 473}, // 524 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14940, 4), 476}, // 525 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14933, 7), 475}, // 526 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14929, 4), 474}, // 527 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(14944, 11), 477}, // 528 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14966, 21), 479}, // 529 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14955, 11), 478}, // 530 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(14998, 14), 481}, // 531 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15012, 21), 482}, // 532 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14987, 11), 480}, // 533 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15033, 18), 483}, // 534 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(13911, 8), 485}, // 535 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13919, 8), 486}, // 536 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13927, 6), 487}, // 537 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13933, 7), 488}, // 538 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13940, 11), 489}, // 539 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13951, 9), 490}, // 540 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13960, 13), 491}, // 541 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13899, 12), 484}, // 542 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13973, 6), 492}, // 543 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(13979, 10), 493}, // 544 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(13989, 6), 494}, // 545 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14119, 15), 502}, // 546 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14134, 13), 503}, // 547 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14107, 12), 501}, // 548 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14197, 21), 507}, // 549 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14090, 17), 500}, // 550 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14078, 12), 499}, // 551 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14258, 16), 510}, // 552 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14051, 14), 497}, // 553 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14035, 16), 496}, // 554 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14065, 13), 498}, // 555 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14218, 20), 508}, // 556 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14162, 18), 505}, // 557 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14147, 15), 504}, // 558 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14238, 20), 509}, // 559 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14180, 17), 506}, // 560 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14030, 5), 495}, // 561 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14395, 9), 515}, // 562 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14427, 7), 519}, // 563 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14334, 6), 511}, // 564 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14345, 6), 513}, // 565 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14340, 5), 512}, // 566 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14384, 11), 514}, // 567 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14416, 11), 518}, // 568 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14404, 5), 516}, // 569 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14409, 7), 517}, // 570 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14443, 10), 520}, // 571 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14466, 13), 522}, // 572 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14453, 13), 521}, // 573 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17275, 16), 637}, // 574 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15260, 8), 542}, // 575 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(16378, 15), 595}, // 576 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16393, 18), 595}, // 577 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15738, 10), 566}, // 578 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(15782, 12), 568}, // 579 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(15111, 12), 529}, // 580 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17056, 14), 625}, // 581 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16895, 15), 618}, // 582 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15415, 8), 555}, // 583 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(16459, 16), 598}, // 584 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16441, 18), 597}, // 585 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15075, 6), 525}, // 586 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(15922, 22), 575}, // 587 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(15900, 22), 574}, // 588 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16490, 13), 600}, // 589 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16475, 15), 599}, // 590 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15081, 12), 526}, // 591 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(17368, 20), 642}, // 592 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15192, 8), 534}, // 593 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(17138, 19), 630}, // 594 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15158, 8), 533}, // 595 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18161, 22), 664}, // 596 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18220, 23), 665}, // 597 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17157, 15), 631}, // 598 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15225, 9), 538}, // 599 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(16543, 13), 603}, // 600 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15286, 9), 545}, // 601 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(15102, 9), 528}, // 602 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(15399, 10), 553}, // 603 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(17677, 27), 652}, // 604 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15277, 9), 544}, // 605 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(16689, 14), 610}, // 606 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15423, 14), 556}, // 607 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(17172, 28), 632}, // 608 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16948, 16), 621}, // 609 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15944, 18), 576}, // 610 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(15608, 15), 562}, // 611 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17596, 26), 649}, // 612 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15558, 15), 561}, // 613 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(15214, 5), 536}, // 614 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(17070, 20), 626}, // 615 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15510, 14), 560}, // 616 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(16600, 21), 606}, // 617 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16831, 24), 614}, // 618 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17441, 31), 644}, // 619 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16774, 26), 613}, // 620 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17231, 25), 635}, // 621 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15147, 11), 532}, // 622 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(15136, 11), 531}, // 623 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(16663, 26), 609}, // 624 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16411, 30), 596}, // 625 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16703, 24), 610}, // 626 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(16740, 19), 611}, // 627 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18020, 16), 661}, // 628 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17388, 19), 643}, // 629 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18097, 28), 663}, // 630 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15409, 6), 554}, // 631 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(18083, 14), 662}, // 632 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17304, 24), 639}, // 633 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15717, 21), 565}, // 634 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(15241, 10), 540}, // 635 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(17647, 30), 651}, // 636 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17622, 25), 650}, // 637 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15685, 18), 563}, // 638 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(15390, 9), 552}, // 639 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(17735, 33), 654}, // 640 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17768, 30), 655}, // 641 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17798, 28), 656}, // 642 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17894, 29), 658}, // 643 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17860, 34), 657}, // 644 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17923, 32), 659}, // 645 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17256, 19), 636}, // 646 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15123, 13), 530}, // 647 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(15748, 14), 567}, // 648 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(15794, 16), 569}, // 649 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(17328, 20), 640}, // 650 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16910, 22), 619}, // 651 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16964, 19), 622}, // 652 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17532, 18), 647}, // 653 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16869, 12), 616}, // 654 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17003, 11), 624}, // 655 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17291, 13), 638}, // 656 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15703, 14), 564}, // 657 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(15200, 14), 535}, // 658 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(15854, 13), 571}, // 659 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(15867, 15), 572}, // 660 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(15994, 20), 578}, // 661 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15962, 32), 577}, // 662 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15307, 12), 547}, // 663 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(16318, 11), 593}, // 664 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(16329, 14), 593}, // 665 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15295, 12), 546}, // 666 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(16881, 14), 617}, // 667 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15437, 7), 557}, // 668 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(16162, 19), 585}, // 669 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16181, 14), 586}, // 670 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15219, 6), 537}, // 671 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(16134, 28), 584}, // 672 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16109, 25), 583}, // 673 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16057, 25), 581}, // 674 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16037, 20), 580}, // 675 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16082, 27), 582}, // 676 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16242, 16), 589}, // 677 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16258, 15), 589}, // 678 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16283, 10), 591}, // 679 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16293, 13), 592}, // 680 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(16306, 12), 592}, // 681 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16273, 10), 590}, // 682 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17348, 20), 641}, // 683 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17200, 14), 633}, // 684 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16517, 26), 602}, // 685 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16855, 14), 615}, // 686 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17704, 31), 653}, // 687 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15051, 17), 523}, // 688 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(15251, 9), 541}, // 689 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(16343, 16), 594}, // 690 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16359, 19), 594}, // 691 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15093, 9), 527}, // 692 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(16503, 14), 601}, // 693 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15234, 7), 539}, // 694 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(15363, 20), 550}, // 695 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(15810, 44), 570}, // 696 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16214, 28), 588}, // 697 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16556, 17), 604}, // 698 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16983, 20), 623}, // 699 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17472, 25), 645}, // 700 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16932, 16), 620}, // 701 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15068, 7), 524}, // 702 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(17955, 26), 660}, // 703 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16648, 15), 608}, // 704 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17214, 17), 634}, // 705 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17550, 15), 648}, // 706 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15383, 7), 551}, // 707 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(17090, 16), 627}, // 708 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16014, 23), 579}, // 709 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17120, 18), 629}, // 710 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15319, 8), 548}, // 711 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(15353, 10), 549}, // 712 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(16727, 13), 611}, // 713 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(16759, 15), 612}, // 714 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(17497, 35), 646}, // 715 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16621, 27), 607}, // 716 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16573, 27), 605}, // 717 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16195, 19), 587}, // 718 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15268, 9), 543}, // 719 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(15882, 18), 573}, // 720 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17106, 14), 628}, // 721 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15444, 10), 558}, // 722 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15454, 10), 559}, // 723 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18243, 3), 666}, // 724 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18246, 3), 667}, // 725 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18249, 3), 668}, // 726 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18262, 7), 671}, // 727 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18252, 5), 669}, // 728 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18257, 5), 670}, // 729 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18269, 12), 672}, // 730 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18281, 17), 673}, // 731 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(19093, 15), 726}, // 732 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18724, 15), 704}, // 733 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9529, 18), 718}, // 734 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9514, 15), 717}, // 735 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18494, 13), 687}, // 736 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18507, 10), 688}, // 737 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18479, 15), 686}, // 738 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18517, 15), 689}, // 739 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19514, 25), 744}, // 740 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19539, 24), 744}, // 741 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19467, 24), 743}, // 742 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19491, 23), 743}, // 743 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19041, 29), 724}, // 744 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18446, 19), 683}, // 745 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20024, 18), 763}, // 746 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18751, 10), 706}, // 747 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19839, 26), 755}, // 748 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19865, 27), 756}, // 749 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18739, 12), 705}, // 750 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18568, 11), 693}, // 751 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18579, 20), 694}, // 752 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18556, 12), 692}, // 753 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18609, 24), 696}, // 754 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18298, 12), 674}, // 755 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19108, 15), 727}, // 756 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18639, 9), 698}, // 757 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18532, 10), 690}, // 758 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18542, 14), 691}, // 759 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18819, 16), 711}, // 760 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18807, 12), 710}, // 761 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19123, 21), 728}, // 762 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19168, 21), 731}, // 763 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19914, 16), 758}, // 764 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19892, 22), 757}, // 765 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20000, 24), 762}, // 766 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20115, 22), 767}, // 767 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20137, 24), 768}, // 768 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20092, 23), 766}, // 769 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20212, 27), 769}, // 770 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19930, 20), 759}, // 771 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18835, 34), 712}, // 772 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18869, 34), 713}, // 773 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18903, 36), 714}, // 774 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18987, 34), 722}, // 775 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19950, 22), 760}, // 776 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18430, 16), 682}, // 777 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18414, 16), 681}, // 778 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18676, 16), 701}, // 779 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19399, 15), 741}, // 780 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19414, 14), 741}, // 781 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18663, 13), 700}, // 782 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19428, 20), 742}, // 783 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19448, 19), 742}, // 784 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18692, 20), 702}, // 785 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19563, 19), 745}, // 786 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19582, 18), 745}, // 787 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18648, 15), 699}, // 788 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18395, 19), 680}, // 789 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19600, 25), 746}, // 790 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19625, 27), 747}, // 791 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18465, 10), 684}, // 792 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18969, 18), 716}, // 793 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19340, 19), 738}, // 794 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18633, 6), 697}, // 795 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20066, 26), 765}, // 796 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19359, 20), 739}, // 797 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9572, 16), 720}, // 798 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19818, 21), 754}, // 799 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19797, 21), 753}, // 800 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9588, 16), 721}, // 801 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19652, 26), 748}, // 802 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19678, 28), 749}, // 803 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19972, 28), 761}, // 804 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6208, 16), 730}, // 805 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19706, 31), 750}, // 806 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19737, 33), 751}, // 807 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19770, 27), 752}, // 808 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19379, 20), 740}, // 809 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9547, 25), 719}, // 810 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18310, 13), 675}, // 811 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18323, 22), 676}, // 812 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18345, 21), 677}, // 813 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19144, 24), 729}, // 814 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19293, 25), 736}, // 815 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19217, 26), 733}, // 816 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19318, 22), 737}, // 817 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19243, 23), 734}, // 818 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19070, 23), 725}, // 819 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19266, 27), 735}, // 820 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19189, 28), 732}, // 821 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20042, 24), 764}, // 822 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(5315, 13), 707}, // 823 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18939, 30), 715}, // 824 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18761, 22), 708}, // 825 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18783, 24), 709}, // 826 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19021, 20), 723}, // 827 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18599, 10), 695}, // 828 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18712, 12), 703}, // 829 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18380, 15), 679}, // 830 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18366, 14), 678}, // 831 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18475, 4), 685}, // 832 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20377, 10), 781}, // 833 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20387, 9), 781}, // 834 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20438, 12), 784}, // 835 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20450, 11), 784}, // 836 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20396, 14), 782}, // 837 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20410, 13), 782}, // 838 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(14434, 9), 774}, // 839 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20289, 10), 775}, // 840 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4796, 9), 773}, // 841 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20346, 16), 780}, // 842 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20362, 15), 780}, // 843 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4563, 7), 776}, // 844 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20469, 8), 786}, // 845 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20306, 7), 778}, // 846 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20423, 8), 783}, // 847 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20431, 7), 783}, // 848 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20313, 17), 779}, // 849 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20330, 16), 779}, // 850 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20461, 8), 785}, // 851 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20299, 7), 777}, // 852 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20246, 20), 771}, // 853 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20266, 23), 772}, // 854 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20239, 7), 770}, // 855 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20486, 12), 788}, // 856 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20477, 9), 787}, // 857 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20498, 12), 789}, // 858 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(20510, 14), 790}, // 859 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20524, 14), 791}, // 860 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20543, 4), 793}, // 861 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20538, 5), 792}, // 862 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20581, 14), 800}, // 863 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20595, 23), 800}, // 864 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20618, 13), 801}, // 865 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20631, 18), 801}, // 866 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20565, 11), 798}, // 867 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20649, 15), 802}, // 868 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20576, 5), 799}, // 869 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20561, 4), 797}, // 870 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(14030, 5), 794}, // 871 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20554, 7), 796}, // 872 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20547, 7), 795}, // 873 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20664, 4), 803}, // 874 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20676, 4), 806}, // 875 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20672, 4), 805}, // 876 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20668, 4), 804}, // 877 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20712, 18), 809}, // 878 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20730, 18), 810}, // 879 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20680, 16), 807}, // 880 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20696, 16), 808}, // 881 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20771, 6), 815}, // 882 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20755, 11), 813}, // 883 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20748, 7), 812}, // 884 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14030, 5), 811}, // 885 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13251, 11), 816}, // 886 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13262, 13), 816}, // 887 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20766, 5), 814}, // 888 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20787, 6), 819}, // 889 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20798, 8), 821}, // 890 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20806, 10), 822}, // 891 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20824, 12), 824}, // 892 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20816, 8), 823}, // 893 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20836, 20), 825}, // 894 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20782, 5), 818}, // 895 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20793, 5), 820}, // 896 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20777, 5), 817}, // 897 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20891, 16), 829}, // 898 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20877, 14), 828}, // 899 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20863, 14), 827}, // 900 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20954, 27), 832}, // 901 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20927, 27), 831}, // 902 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20907, 20), 830}, // 903 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20856, 7), 826}, // 904 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(20981, 10), 833}, // 905 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(20991, 10), 834}, // 906 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21012, 15), 836}, // 907 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21001, 11), 835}, // 908 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21027, 5), 838}, // 909 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(21041, 12), 841}, // 910 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(21053, 13), 843}, // 911 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(21036, 5), 840}, // 912 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(21032, 4), 839}, // 913 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(21066, 19), 846}, // 914 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(21085, 22), 846}, // 915 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21107, 17), 847}, // 916 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(21124, 20), 847}, // 917 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(8347, 7), 845}, // 918 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(21144, 16), 848}, // 919 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21160, 19), 848}, // 920 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(14030, 5), 837}, // 921 None in SPV_OPERAND_TYPE_IMAGE
  {IR(21232, 12), 852}, // 922 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(15437, 7), 842}, // 923 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(21244, 8), 853}, // 924 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(15234, 7), 844}, // 925 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(21210, 11), 850}, // 926 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(21179, 14), 849}, // 927 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21193, 17), 849}, // 928 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21221, 11), 851}, // 929 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(13927, 6), 868}, // 930 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21413, 10), 867}, // 931 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21348, 12), 862}, // 932 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21360, 12), 863}, // 933 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21337, 11), 861}, // 934 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21262, 11), 855}, // 935 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21252, 10), 854}, // 936 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21322, 15), 860}, // 937 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21434, 17), 870}, // 938 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21451, 16), 871}, // 939 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21584, 16), 878}, // 940 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21600, 16), 879}, // 941 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21283, 11), 857}, // 942 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21423, 11), 869}, // 943 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21507, 20), 874}, // 944 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21273, 10), 856}, // 945 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21308, 14), 859}, // 946 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21294, 14), 858}, // 947 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21527, 19), 875}, // 948 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21546, 19), 876}, // 949 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21565, 19), 877}, // 950 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21385, 14), 865}, // 951 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21399, 14), 866}, // 952 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21372, 13), 864}, // 953 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21467, 20), 872}, // 954 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21487, 20), 873}, // 955 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21618, 2), 881}, // 956 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21719, 5), 899}, // 957 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21640, 5), 887}, // 958 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21635, 5), 886}, // 959 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21677, 6), 893}, // 960 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21683, 13), 894}, // 961 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21645, 10), 888}, // 962 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21655, 10), 889}, // 963 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21616, 2), 880}, // 964 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21623, 3), 883}, // 965 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21620, 3), 882}, // 966 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21626, 4), 884}, // 967 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21630, 5), 885}, // 968 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21672, 5), 892}, // 969 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21668, 4), 891}, // 970 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21665, 3), 890}, // 971 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21713, 6), 898}, // 972 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21696, 5), 895}, // 973 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21707, 6), 897}, // 974 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21701, 6), 896}, // 975 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21724, 27), 900}, // 976 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21751, 23), 901}, // 977 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21774, 7), 902}, // 978 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21781, 11), 903}, // 979 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21792, 14), 904}, // 980 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21806, 12), 906}, // 981 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(14030, 5), 905}, // 982 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(21818, 13), 908}, // 983 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(14030, 5), 907}, // 984 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(21831, 7), 909}, // 985 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21838, 7), 910}, // 986 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21845, 12), 911}, // 987 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21871, 12), 913}, // 988 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21923, 17), 916}, // 989 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21898, 25), 915}, // 990 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21883, 15), 914}, // 991 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(21857, 14), 912}, // 992 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22063, 21), 929}, // 993 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21958, 19), 920}, // 994 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21977, 17), 921}, // 995 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21947, 11), 919}, // 996 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17304, 24), 927}, // 997 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22022, 18), 924}, // 998 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22084, 18), 931}, // 999 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22163, 15), 935}, // 1000 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17328, 20), 928}, // 1001 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22102, 21), 932}, // 1002 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22008, 14), 923}, // 1003 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22178, 26), 936}, // 1004 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21994, 14), 922}, // 1005 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22149, 14), 934}, // 1006 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(14030, 5), 917}, // 1007 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22050, 13), 926}, // 1008 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22040, 10), 925}, // 1009 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17348, 20), 930}, // 1010 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22123, 26), 933}, // 1011 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(21940, 7), 918}, // 1012 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22487, 27), 950}, // 1013 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22435, 26), 948}, // 1014 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22355, 23), 944}, // 1015 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22309, 23), 942}, // 1016 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22204, 29), 938}, // 1017 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22401, 17), 946}, // 1018 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22514, 27), 951}, // 1019 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22461, 26), 949}, // 1020 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22378, 23), 945}, // 1021 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22332, 23), 943}, // 1022 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22233, 29), 939}, // 1023 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22418, 17), 947}, // 1024 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22262, 21), 940}, // 1025 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22283, 26), 941}, // 1026 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(14030, 5), 937}, // 1027 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22674, 20), 959}, // 1028 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22541, 8), 954}, // 1029 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22549, 21), 956}, // 1030 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22570, 24), 956}, // 1031 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22594, 19), 957}, // 1032 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22613, 22), 957}, // 1033 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22694, 17), 960}, // 1034 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22635, 18), 958}, // 1035 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22653, 21), 958}, // 1036 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(14030, 5), 952}, // 1037 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(21232, 12), 955}, // 1038 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15268, 9), 953}, // 1039 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22718, 8), 962}, // 1040 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22726, 7), 963}, // 1041 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22711, 7), 961}, // 1042 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22733, 7), 964}, // 1043 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22740, 10), 964}, // 1044 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22758, 8), 966}, // 1045 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22774, 15), 968}, // 1046 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22878, 20), 974}, // 1047 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22857, 21), 973}, // 1048 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22898, 12), 975}, // 1049 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22939, 14), 977}, // 1050 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22953, 17), 977}, // 1051 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22970, 12), 978}, // 1052 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22982, 15), 978}, // 1053 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(14030, 5), 965}, // 1054 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22910, 13), 976}, // 1055 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22923, 16), 976}, // 1056 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22750, 8), 965}, // 1057 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22766, 8), 967}, // 1058 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22789, 23), 969}, // 1059 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22826, 15), 971}, // 1060 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22812, 14), 970}, // 1061 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15268, 9), 979}, // 1062 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22841, 16), 972}, // 1063 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22997, 10), 980}, // 1064 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(23012, 4), 982}, // 1065 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23025, 8), 984}, // 1066 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23016, 9), 983}, // 1067 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23007, 5), 981}, // 1068 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23033, 25), 985}, // 1069 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23058, 28), 985}, // 1070 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23099, 4), 988}, // 1071 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23132, 9), 992}, // 1072 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23141, 13), 993}, // 1073 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23112, 8), 990}, // 1074 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23120, 12), 991}, // 1075 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23103, 9), 989}, // 1076 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23086, 4), 986}, // 1077 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23090, 9), 987}, // 1078 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(14030, 5), 994}, // 1079 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23154, 25), 995}, // 1080 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23179, 23), 996}, // 1081 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23271, 27), 1002}, // 1082 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23298, 28), 1003}, // 1083 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23340, 16), 1005}, // 1084 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23326, 14), 1004}, // 1085 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23410, 30), 1008}, // 1086 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23212, 12), 999}, // 1087 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(14782, 8), 997}, // 1088 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23202, 10), 998}, // 1089 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23397, 13), 1007}, // 1090 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23373, 24), 1006}, // 1091 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23247, 24), 1001}, // 1092 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23356, 17), 1006}, // 1093 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23224, 23), 1000}, // 1094 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23481, 37), 1010}, // 1095 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23440, 41), 1009}, // 1096 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23596, 42), 1013}, // 1097 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23518, 37), 1011}, // 1098 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23555, 41), 1012}, // 1099 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23638, 33), 1014}, // 1100 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23671, 33), 1015}, // 1101 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23716, 6), 1018}, // 1102 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23704, 12), 1017}, // 1103 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(14030, 5), 1016}, // 1104 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23722, 7), 1019}, // 1105 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23729, 15), 1020}, // 1106 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23752, 7), 1022}, // 1107 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23744, 8), 1021}, // 1108 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23817, 13), 1031}, // 1109 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23859, 4), 1037}, // 1110 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23897, 9), 1042}, // 1111 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23830, 5), 1032}, // 1112 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23959, 5), 1051}, // 1113 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24030, 6), 1061}, // 1114 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23783, 5), 1026}, // 1115 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23937, 5), 1047}, // 1116 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23994, 6), 1056}, // 1117 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24047, 5), 1064}, // 1118 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24041, 6), 1063}, // 1119 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23863, 3), 1038}, // 1120 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23906, 8), 1043}, // 1121 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23964, 4), 1052}, // 1122 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24036, 5), 1062}, // 1123 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23850, 5), 1035}, // 1124 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23878, 10), 1040}, // 1125 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23811, 6), 1030}, // 1126 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23948, 6), 1049}, // 1127 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24017, 7), 1059}, // 1128 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23805, 6), 1029}, // 1129 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23942, 6), 1048}, // 1130 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24010, 7), 1058}, // 1131 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23855, 4), 1036}, // 1132 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23888, 9), 1041}, // 1133 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23954, 5), 1050}, // 1134 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24024, 6), 1060}, // 1135 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23842, 8), 1034}, // 1136 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24000, 10), 1057}, // 1137 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23835, 7), 1033}, // 1138 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23866, 12), 1039}, // 1139 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23775, 8), 1025}, // 1140 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23922, 8), 1045}, // 1141 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23977, 9), 1054}, // 1142 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23767, 8), 1024}, // 1143 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23914, 8), 1044}, // 1144 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23968, 9), 1053}, // 1145 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23788, 6), 1027}, // 1146 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23794, 11), 1028}, // 1147 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23930, 7), 1046}, // 1148 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23986, 8), 1055}, // 1149 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23759, 8), 1023}, // 1150 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24052, 12), 1065}, // 1151 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24064, 7), 1066}, // 1152 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24090, 11), 1069}, // 1153 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24101, 12), 1070}, // 1154 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24113, 15), 1070}, // 1155 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24128, 14), 1071}, // 1156 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24081, 9), 1068}, // 1157 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24071, 10), 1067}, // 1158 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24150, 12), 1074}, // 1159 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24142, 8), 1073}, // 1160 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(14030, 5), 1072}, // 1161 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24162, 29), 1075}, // 1162 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(13911, 8), 1077}, // 1163 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13919, 8), 1078}, // 1164 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13927, 6), 1079}, // 1165 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13933, 7), 1080}, // 1166 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13940, 11), 1081}, // 1167 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13951, 9), 1082}, // 1168 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13960, 13), 1083}, // 1169 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13899, 12), 1076}, // 1170 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(13973, 6), 1084}, // 1171 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13979, 10), 1085}, // 1172 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(13989, 6), 1086}, // 1173 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14010, 20), 1088}, // 1174 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(13995, 15), 1087}, // 1175 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14119, 15), 1096}, // 1176 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14134, 13), 1097}, // 1177 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14107, 12), 1095}, // 1178 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14197, 21), 1101}, // 1179 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14090, 17), 1094}, // 1180 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14274, 16), 1105}, // 1181 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14078, 12), 1093}, // 1182 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14258, 16), 1104}, // 1183 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14051, 14), 1091}, // 1184 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14035, 16), 1090}, // 1185 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14065, 13), 1092}, // 1186 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14218, 20), 1102}, // 1187 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14162, 18), 1099}, // 1188 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14147, 15), 1098}, // 1189 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14238, 20), 1103}, // 1190 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14180, 17), 1100}, // 1191 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14310, 24), 1107}, // 1192 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14290, 20), 1106}, // 1193 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(24191, 26), 1108}, // 1194 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14030, 5), 1089}, // 1195 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14395, 9), 1113}, // 1196 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14427, 7), 1117}, // 1197 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14334, 6), 1109}, // 1198 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14434, 9), 1118}, // 1199 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14345, 6), 1111}, // 1200 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14340, 5), 1110}, // 1201 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14384, 11), 1112}, // 1202 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14416, 11), 1116}, // 1203 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14404, 5), 1114}, // 1204 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14409, 7), 1115}, // 1205 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14479, 11), 1122}, // 1206 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14443, 10), 1119}, // 1207 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14466, 13), 1121}, // 1208 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14453, 13), 1120}, // 1209 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(24252, 15), 1129}, // 1210 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24217, 5), 1124}, // 1211 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24222, 5), 1125}, // 1212 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24272, 7), 1131}, // 1213 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24247, 5), 1128}, // 1214 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24279, 5), 1132}, // 1215 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24227, 9), 1126}, // 1216 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24236, 11), 1127}, // 1217 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24299, 5), 1136}, // 1218 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24267, 5), 1130}, // 1219 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24289, 6), 1134}, // 1220 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(23759, 8), 1123}, // 1221 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24284, 5), 1133}, // 1222 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24295, 4), 1135}, // 1223 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24386, 14), 1147}, // 1224 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24468, 16), 1153}, // 1225 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24484, 15), 1153}, // 1226 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24782, 17), 1162}, // 1227 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24333, 15), 1142}, // 1228 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24799, 16), 1163}, // 1229 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24356, 9), 1144}, // 1230 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24365, 8), 1145}, // 1231 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24573, 16), 1156}, // 1232 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24589, 15), 1156}, // 1233 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24737, 21), 1160}, // 1234 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24815, 14), 1164}, // 1235 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24400, 6), 1148}, // 1236 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24499, 24), 1154}, // 1237 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24523, 23), 1154}, // 1238 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24604, 22), 1157}, // 1239 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24626, 21), 1157}, // 1240 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24320, 6), 1138}, // 1241 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24452, 16), 1152}, // 1242 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24326, 7), 1140}, // 1243 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24690, 22), 1159}, // 1244 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24712, 25), 1159}, // 1245 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24348, 8), 1143}, // 1246 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24373, 13), 1146}, // 1247 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24546, 14), 1155}, // 1248 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24560, 13), 1155}, // 1249 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24647, 22), 1158}, // 1250 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24669, 21), 1158}, // 1251 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24406, 14), 1149}, // 1252 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24758, 24), 1161}, // 1253 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24433, 19), 1151}, // 1254 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24420, 13), 1150}, // 1255 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(15319, 8), 1139}, // 1256 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24304, 16), 1137}, // 1257 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24071, 10), 1141}, // 1258 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(21883, 15), 1168}, // 1259 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(21857, 14), 1165}, // 1260 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24847, 15), 1167}, // 1261 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24829, 18), 1166}, // 1262 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24873, 11), 1171}, // 1263 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(14030, 5), 1169}, // 1264 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(24862, 11), 1170}, // 1265 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(23704, 12), 1174}, // 1266 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(15277, 9), 1173}, // 1267 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23722, 7), 1175}, // 1268 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23729, 15), 1176}, // 1269 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(24884, 10), 1172}, // 1270 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(24937, 24), 1180}, // 1271 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24961, 22), 1181}, // 1272 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24983, 21), 1182}, // 1273 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24894, 8), 1177}, // 1274 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24902, 15), 1178}, // 1275 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(24917, 20), 1179}, // 1276 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 148);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(156, 298);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(454, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(462, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(465, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(467, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(486, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(496, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(500, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(515, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(519, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(525, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(528, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(531, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(535, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(543, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(546, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(562, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(571, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(574, 150);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(724, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(732, 101);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(833, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(856, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(858, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(861, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(863, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(874, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(878, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(882, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(889, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(898, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(905, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(909, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(930, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(956, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(976, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(978, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(981, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(983, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(985, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(988, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(993, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1013, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1028, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1040, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1045, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1064, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1065, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1069, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1071, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1079, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1082, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1095, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1097, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1100, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1102, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1107, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1109, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1151, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1159, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1162, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1163, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1171, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1174, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1176, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1196, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1206, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1210, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1224, 35);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1259, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1263, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1266, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1271, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(909, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1028, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1069, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(519, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1013, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1079, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(858, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1271, 6);
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
static const std::array<InstructionDesc, 808> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(25004, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(25060, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(25066, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(25185, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(25192, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(25208, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(25213, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(25224, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(25231, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(25236, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(25246, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(25306, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(25378, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(25452, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(25495, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(25537, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(25548, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(25557, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(25566, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(25611, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(25621, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(25632, 11), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(25756, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(25766, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(25778, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(25795, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(25805, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(25822, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(25833, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(25875, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(25887, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(25900, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(25910, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(25926, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(25940, 10), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(25950, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(25959, 19), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(25978, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(25991, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(15277, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(26043, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(26139, 16), IR(0, 0), IR(203, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(26155, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(26168, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(26185, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(26203, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(26216, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(26279, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(24356, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(26328, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(26346, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(26358, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(26371, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(26380, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(26438, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(26443, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(26449, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(26460, 16), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(26476, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(26488, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(26508, 15), IR(0, 0), IR(206, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(26523, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(26535, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(26558, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(26609, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(26618, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(26633, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(26649, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(26708, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(26728, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(26749, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(26769, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(26783, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(26802, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(26819, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(26835, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(26846, 10), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(26856, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(26901, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(26947, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(26970, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(26997, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(27024, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(27051, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(27078, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(27109, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(27140, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(27151, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(27163, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(27179, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(27189, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(24400, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(27200, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(27217, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(27233, 18), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(27251, 15), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(27266, 14), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(27280, 17), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(27297, 18), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(27315, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(27327, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(27339, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(27351, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(27363, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(27372, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(27381, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(27390, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(27404, 14), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(27418, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(27433, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(27448, 14), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(27462, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(27479, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(27496, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(27521, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(27529, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(27537, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(27545, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(27550, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(27555, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(27560, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(27565, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(27570, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(27575, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(27580, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(27585, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(27590, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(27595, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(27600, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(27605, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(27610, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(27615, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(27633, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(27651, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(27669, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(27687, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(27705, 13), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(27718, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(27722, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(27732, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(27743, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(27756, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(27769, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(27773, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(27777, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(27783, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(27789, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(27798, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(27807, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(27818, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(27832, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(27840, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(27850, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(27863, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(27879, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(27889, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(27900, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(27911, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(27918, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(27925, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(27935, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(27948, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(27961, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(27979, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(27997, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(28007, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(28017, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(28032, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(28047, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(28057, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(28069, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(28082, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(28097, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(28110, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(28125, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(28141, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(28159, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(28177, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(28197, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(28218, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(28241, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(28259, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(28280, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(28297, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(28307, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(28318, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(28329, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(28333, 15), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(28348, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(28365, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(28382, 11), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(28393, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(28402, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(28407, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(28412, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(28419, 9), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(28428, 9), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(28437, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(28448, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(28459, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(28470, 13), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(28483, 11), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(28494, 13), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(28507, 17), IR(0, 0), IR(112, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(28524, 19), IR(0, 0), IR(112, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(28580, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(28595, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(28609, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(28620, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(28632, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(28647, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(28669, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(28695, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(28712, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(28729, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(28740, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(28751, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(28762, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(28773, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(28784, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(28795, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(28805, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(28814, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(28824, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(28858, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(28903, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(28918, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(28924, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(28931, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(28994, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(29001, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(29006, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(29013, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(29025, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(29037, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(29051, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(29064, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(29079, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(29095, 9), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(29104, 9), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(29113, 15), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(29161, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(29171, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(29181, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(29191, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(29201, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(29211, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(29221, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(29231, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(29241, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(29250, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(29260, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(29277, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(29295, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(29318, 24), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(29342, 15), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(29357, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(29373, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(29390, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(29408, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(29426, 28), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(29454, 29), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(29483, 20), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(29503, 21), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(29524, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(29538, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(29552, 30), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(29582, 32), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(29614, 23), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(29637, 40), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(29677, 12), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(29689, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(29702, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(29718, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(29731, 19), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(29750, 26), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(29776, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(29792, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(29805, 29), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(29834, 29), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(29863, 33), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(29896, 33), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(29929, 33), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(29962, 33), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(29995, 37), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(30032, 37), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(30069, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(30086, 18), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(30104, 22), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(30126, 26), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(30152, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(30159, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(30180, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(30196, 16), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(30212, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(30219, 16), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(30235, 20), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(30255, 26), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(30281, 35), IR(0, 0), IR(151, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(30316, 25), IR(0, 0), IR(151, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(30341, 17), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(30358, 23), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(30381, 19), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(30400, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(30416, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(30432, 11), IR(0, 0), IR(1, 0), IR(155, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(30443, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(30464, 19), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(30483, 19), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(30502, 24), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(30526, 25), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(30551, 30), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(5583, 22), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(30581, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(30610, 32), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(30642, 30), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(30672, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(30701, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(8673, 23), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(30730, 26), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(30756, 25), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(30781, 27), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(30808, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(30828, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(30848, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(30868, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(30888, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(30908, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(30928, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(30948, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(30968, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(30988, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(31008, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(31034, 25), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(31059, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(31085, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(31111, 25), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(31136, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(31162, 29), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(31191, 24), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(31215, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(31227, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(31236, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(31248, 8), IR(0, 0), IR(228, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(31256, 23), IR(0, 0), IR(143, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(31279, 23), IR(0, 0), IR(152, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(31302, 25), IR(0, 0), IR(153, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(31327, 14), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(31383, 14), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(31397, 15), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(31412, 19), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(31431, 17), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(31448, 19), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(8964, 9), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(31467, 14), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(31481, 18), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(31499, 12), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(31511, 13), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(31524, 20), IR(0, 0), IR(3, 1), IR(165, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(31544, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(31566, 19), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(31585, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(31607, 30), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(5344, 18), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(31637, 27), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(31664, 25), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(31689, 33), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(31722, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(31744, 19), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(31763, 15), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(31778, 15), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(31793, 20), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(13151, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(31813, 26), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(31839, 26), IR(0, 0), IR(1, 0), IR(166, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpTraceRayKHR, false, false, IR(415, 11), IR(31865, 12), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(31877, 19), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(31896, 35), IR(0, 0), IR(236, 2), IR(167, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(31931, 22), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(31953, 16), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(426, 5), IR(32016, 5), IR(93, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(426, 5), IR(32029, 5), IR(94, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(426, 5), IR(32042, 6), IR(95, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(431, 6), IR(32057, 11), IR(96, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(431, 6), IR(32082, 11), IR(97, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(431, 6), IR(32107, 12), IR(98, 1), IR(238, 1), IR(121, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(437, 6), IR(32134, 25), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(443, 6), IR(32159, 25), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(449, 5), IR(32184, 26), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(454, 6), IR(32264, 27), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(32291, 27), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32318, 30), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32348, 34), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(32382, 31), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(32413, 16), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(460, 8), IR(32429, 22), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(32451, 21), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(32472, 32), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(32504, 31), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(32535, 19), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(32554, 31), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(32585, 24), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(32609, 19), IR(0, 0), IR(242, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(32628, 23), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(32651, 23), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(32674, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(32703, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(32732, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(32761, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(32790, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(32813, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(32836, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(32859, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(32882, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(32905, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(32928, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(32951, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(32974, 21), IR(0, 0), IR(245, 1), IR(57, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(32995, 17), IR(0, 0), IR(245, 1), IR(57, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(33012, 13), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(33025, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(33050, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(33074, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(33099, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(33128, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(33155, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(33178, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(33197, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(33220, 26), IR(0, 0), IR(160, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(33246, 26), IR(0, 0), IR(160, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(468, 14), IR(33272, 27), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(482, 13), IR(33299, 36), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(495, 7), IR(33335, 28), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(33363, 28), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(33391, 28), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(33419, 33), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(33452, 30), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(482, 13), IR(33482, 26), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(33508, 39), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(33547, 44), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(33591, 23), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(502, 12), IR(33614, 20), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(482, 13), IR(33634, 21), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(502, 12), IR(33655, 30), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(514, 6), IR(33685, 22), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(33707, 25), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(33732, 26), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(33758, 25), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(33783, 22), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(33805, 29), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(33834, 28), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(33862, 25), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(33887, 34), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(33921, 32), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(33953, 29), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(33982, 22), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(34004, 22), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(34026, 19), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(34045, 17), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(34062, 18), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(520, 3), IR(34080, 29), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(34109, 24), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(34133, 16), IR(0, 0), IR(122, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(523, 7), IR(34149, 23), IR(0, 0), IR(249, 1), IR(71, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeCooperativeVectorNV, true, false, IR(89, 3), IR(34172, 24), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(530, 13), IR(34196, 29), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(543, 7), IR(34225, 42), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(34267, 39), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(550, 16), IR(34306, 32), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(34338, 27), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(566, 4), IR(34365, 17), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(34382, 18), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionNV, true, true, IR(160, 3), IR(34400, 27), IR(0, 0), IR(181, 1), IR(74, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(34427, 33), IR(0, 0), IR(50, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(34460, 35), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(34495, 38), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(570, 5), IR(34533, 24), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(575, 4), IR(34557, 25), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(34582, 22), IR(99, 1), IR(54, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(34625, 21), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(34646, 15), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(415, 11), IR(34661, 8), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(502, 12), IR(34669, 14), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(502, 12), IR(34683, 17), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(34700, 50), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(34750, 29), IR(100, 1), IR(255, 4), IR(169, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(34807, 18), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetClusterIdNV, true, true, IR(172, 4), IR(34825, 23), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(34848, 24), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(579, 5), IR(34872, 24), IR(0, 0), IR(259, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(584, 6), IR(34896, 24), IR(0, 0), IR(259, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(590, 5), IR(34920, 25), IR(0, 0), IR(259, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(34945, 26), IR(0, 0), IR(259, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(34971, 26), IR(0, 0), IR(259, 1), IR(78, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(34997, 28), IR(0, 0), IR(260, 3), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(35025, 26), IR(0, 0), IR(260, 3), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(595, 5), IR(35094, 26), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(600, 7), IR(35195, 30), IR(0, 0), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(607, 5), IR(35225, 31), IR(0, 0), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(35256, 32), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(35288, 19), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(612, 4), IR(35307, 17), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(35324, 21), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(35345, 27), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(35372, 24), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(35396, 20), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(35416, 28), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(35444, 19), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(35463, 25), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(35488, 22), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(11463, 25), IR(45, 1), IR(266, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(35510, 22), IR(0, 0), IR(266, 1), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(35532, 20), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(35552, 27), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(35579, 29), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(35608, 18), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(35626, 20), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(35646, 18), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(35664, 20), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(35684, 25), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(35709, 25), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(35734, 29), IR(0, 0), IR(123, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(616, 7), IR(35815, 17), IR(0, 0), IR(187, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(35832, 40), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(35872, 38), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(35910, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(35948, 34), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(35982, 37), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(36019, 29), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(36048, 27), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(36075, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(36102, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(36125, 23), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(36148, 20), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(36168, 22), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(36190, 19), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(11911, 21), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(36209, 25), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(36234, 23), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(36257, 24), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(36281, 23), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(36304, 24), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(36328, 28), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(36356, 29), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(36385, 33), IR(0, 0), IR(270, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(623, 5), IR(36418, 34), IR(0, 0), IR(270, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(36452, 24), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(36476, 25), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(36501, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(36514, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(36527, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(36540, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(36553, 14), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(36567, 14), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(36581, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(36602, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(36623, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(36636, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(36649, 15), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(36664, 15), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(36679, 29), IR(0, 0), IR(196, 1), IR(94, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(36708, 25), IR(0, 0), IR(196, 1), IR(94, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(36733, 15), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(628, 6), IR(12129, 9), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(36748, 13), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(36761, 14), IR(0, 0), IR(272, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(36775, 14), IR(0, 0), IR(272, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(36789, 14), IR(0, 0), IR(276, 1), IR(98, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(36803, 10), IR(0, 0), IR(276, 1), IR(98, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(36813, 15), IR(101, 1), IR(1, 0), IR(173, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(36849, 21), IR(102, 1), IR(1, 0), IR(173, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(36897, 14), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(36911, 18), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(36929, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(36952, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(36975, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(36998, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(37021, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(37043, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37065, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37111, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(37155, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(37194, 37), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(37231, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(37253, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37275, 60), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37335, 53), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37388, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37435, 40), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37475, 51), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37526, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(37570, 51), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(37621, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37677, 50), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37727, 52), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37779, 49), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(37828, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(37875, 50), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(37925, 51), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(37976, 56), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(38032, 33), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38065, 52), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38117, 61), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(38178, 61), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(38239, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(38278, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(38316, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(38355, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(38393, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(38432, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(38470, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(38506, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(38545, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(38588, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(38626, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(38664, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(38702, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(38747, 40), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(38787, 62), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(38849, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(38879, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(38917, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(38953, 33), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(38986, 35), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(39021, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(39060, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(39103, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(39150, 54), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(39204, 34), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(39238, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(39285, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(39330, 55), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(39385, 53), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(39438, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(39494, 54), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(39548, 58), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(39606, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(39662, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(39700, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(39746, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(39790, 49), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(39839, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(39886, 70), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(39956, 68), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(40024, 69), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(40093, 68), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(40161, 66), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(40227, 67), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(40294, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(40330, 48), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(40378, 59), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(40437, 58), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(40495, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(634, 9), IR(40551, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(643, 10), IR(40581, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(40611, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(40650, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(40696, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(40739, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(40786, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(40831, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(40877, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(40933, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(40971, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(41001, 32), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(643, 10), IR(41033, 36), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(653, 13), IR(41069, 42), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(41111, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(41150, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(41189, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(41233, 48), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(41281, 50), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(41331, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(41374, 48), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(41422, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(41465, 31), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41496, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41543, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(41588, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(41634, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(41690, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(41728, 35), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(41763, 44), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(41807, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(41853, 41), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(41894, 36), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(41930, 50), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(41980, 48), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(42028, 35), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(12367, 25), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(42063, 16), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(42079, 19), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosPiINTEL, true, true, IR(666, 8), IR(42098, 28), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastINTEL, true, true, IR(666, 8), IR(42126, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntINTEL, true, true, IR(666, 8), IR(42150, 31), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntINTEL, true, true, IR(666, 8), IR(42181, 29), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddINTEL, true, true, IR(674, 10), IR(42210, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubINTEL, true, true, IR(674, 10), IR(42233, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulINTEL, true, true, IR(674, 10), IR(42256, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivINTEL, true, true, IR(674, 10), IR(42279, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTINTEL, true, true, IR(684, 6), IR(42302, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEINTEL, true, true, IR(684, 6), IR(42324, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTINTEL, true, true, IR(684, 6), IR(42346, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEINTEL, true, true, IR(684, 6), IR(42368, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQINTEL, true, true, IR(684, 6), IR(42390, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipINTEL, true, true, IR(666, 8), IR(42412, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtINTEL, true, true, IR(666, 8), IR(42437, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtINTEL, true, true, IR(666, 8), IR(42462, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotINTEL, true, true, IR(674, 10), IR(42486, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtINTEL, true, true, IR(666, 8), IR(42511, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(666, 8), IR(42535, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(666, 8), IR(42558, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(666, 8), IR(42582, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(666, 8), IR(42607, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(666, 8), IR(42632, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(666, 8), IR(42655, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(666, 8), IR(42679, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(666, 8), IR(42704, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(666, 8), IR(42729, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(666, 8), IR(42752, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(666, 8), IR(42775, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(666, 8), IR(42801, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(666, 8), IR(42826, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(666, 8), IR(42851, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(666, 8), IR(42875, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(666, 8), IR(42901, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(666, 8), IR(42925, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(666, 8), IR(42951, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(666, 8), IR(42975, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(674, 10), IR(43001, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(674, 10), IR(43026, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(674, 10), IR(43049, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(674, 10), IR(43073, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(43097, 17), IR(0, 0), IR(284, 1), IR(105, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(690, 2), IR(43114, 21), IR(0, 0), IR(132, 1), IR(112, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(692, 3), IR(43135, 20), IR(0, 0), IR(132, 1), IR(112, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(43155, 24), IR(0, 0), IR(132, 1), IR(112, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtINTEL, true, true, IR(666, 8), IR(43179, 15), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipINTEL, true, true, IR(666, 8), IR(43194, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtINTEL, true, true, IR(666, 8), IR(43210, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinINTEL, true, true, IR(666, 8), IR(43226, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosINTEL, true, true, IR(666, 8), IR(43240, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosINTEL, true, true, IR(666, 8), IR(43254, 17), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiINTEL, true, true, IR(666, 8), IR(43271, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiINTEL, true, true, IR(666, 8), IR(43287, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiINTEL, true, true, IR(666, 8), IR(43303, 19), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogINTEL, true, true, IR(666, 8), IR(43322, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpINTEL, true, true, IR(666, 8), IR(43336, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupINTEL, true, true, IR(160, 3), IR(43350, 29), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrINTEL, true, true, IR(160, 3), IR(43379, 29), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingINTEL, true, true, IR(172, 4), IR(43408, 22), IR(0, 0), IR(285, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipeBlockingINTEL, true, true, IR(172, 4), IR(43430, 23), IR(0, 0), IR(285, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpFPGARegINTEL, true, true, IR(160, 3), IR(12926, 13), IR(0, 0), IR(286, 1), IR(120, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(43453, 22), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(43475, 23), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(43498, 28), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(43526, 46), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(43572, 37), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(43609, 65), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(43674, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(43714, 41), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(43755, 39), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(43794, 36), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(43830, 46), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(43876, 45), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(43921, 42), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(43963, 32), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(43995, 29), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(44024, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(44064, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(44104, 14), IR(0, 0), IR(287, 4), IR(127, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(44118, 23), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(695, 1), IR(44141, 25), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(695, 1), IR(44166, 32), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(695, 1), IR(44198, 36), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(44234, 33), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(44267, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(44287, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(44307, 26), IR(0, 0), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(44333, 24), IR(0, 0), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(13358, 19), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateINTEL, true, true, IR(696, 7), IR(44357, 24), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceAsyncINTEL, false, false, IR(168, 2), IR(44381, 23), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceGetINTEL, true, true, IR(160, 3), IR(44404, 21), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceReleaseINTEL, false, false, IR(10, 1), IR(44425, 25), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeTaskSequenceINTEL, true, false, IR(68, 1), IR(44450, 22), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(44472, 27), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(703, 10), IR(44499, 25), IR(0, 0), IR(107, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(703, 10), IR(44524, 34), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(703, 10), IR(44558, 34), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(713, 9), IR(44592, 29), IR(0, 0), IR(107, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(703, 10), IR(44621, 26), IR(0, 0), IR(107, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(722, 7), IR(13696, 38), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(44709, 21), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(44730, 13), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(44743, 13), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(44756, 19), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(44775, 18), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(44793, 19), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(44812, 19), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(44831, 18), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(44849, 19), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(44868, 18), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(729, 6), IR(44886, 18), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(735, 4), IR(44904, 19), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(44923, 26), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(44949, 28), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(44977, 33), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 819> kInstructionNames{{
  {IR(36501, 13), 548}, // 0 AbsISubINTEL
  {IR(36514, 13), 549}, // 1 AbsUSubINTEL
  {IR(26476, 12), 58}, // 2 AccessChain
  {IR(43114, 21), 734}, // 3 AliasDomainDeclINTEL
  {IR(43135, 20), 735}, // 4 AliasScopeDeclINTEL
  {IR(43155, 24), 736}, // 5 AliasScopeListDeclINTEL
  {IR(27773, 4), 143}, // 6 All
  {IR(33025, 25), 420}, // 7 AllocateNodePayloadsAMDX
  {IR(27769, 4), 142}, // 8 Any
  {IR(42901, 24), 725}, // 9 ArbitraryFloatACosINTEL
  {IR(42925, 26), 726}, // 10 ArbitraryFloatACosPiINTEL
  {IR(42851, 24), 723}, // 11 ArbitraryFloatASinINTEL
  {IR(42875, 26), 724}, // 12 ArbitraryFloatASinPiINTEL
  {IR(43001, 25), 729}, // 13 ArbitraryFloatATan2INTEL
  {IR(42951, 24), 727}, // 14 ArbitraryFloatATanINTEL
  {IR(42975, 26), 728}, // 15 ArbitraryFloatATanPiINTEL
  {IR(42210, 23), 696}, // 16 ArbitraryFloatAddINTEL
  {IR(42150, 31), 694}, // 17 ArbitraryFloatCastFromIntINTEL
  {IR(42126, 24), 693}, // 18 ArbitraryFloatCastINTEL
  {IR(42181, 29), 695}, // 19 ArbitraryFloatCastToIntINTEL
  {IR(42462, 24), 707}, // 20 ArbitraryFloatCbrtINTEL
  {IR(42752, 23), 719}, // 21 ArbitraryFloatCosINTEL
  {IR(42826, 25), 722}, // 22 ArbitraryFloatCosPiINTEL
  {IR(42279, 23), 699}, // 23 ArbitraryFloatDivINTEL
  {IR(42390, 22), 704}, // 24 ArbitraryFloatEQINTEL
  {IR(42679, 25), 716}, // 25 ArbitraryFloatExp10INTEL
  {IR(42655, 24), 715}, // 26 ArbitraryFloatExp2INTEL
  {IR(42632, 23), 714}, // 27 ArbitraryFloatExpINTEL
  {IR(42704, 25), 717}, // 28 ArbitraryFloatExpm1INTEL
  {IR(42324, 22), 701}, // 29 ArbitraryFloatGEINTEL
  {IR(42302, 22), 700}, // 30 ArbitraryFloatGTINTEL
  {IR(42486, 25), 708}, // 31 ArbitraryFloatHypotINTEL
  {IR(42368, 22), 703}, // 32 ArbitraryFloatLEINTEL
  {IR(42346, 22), 702}, // 33 ArbitraryFloatLTINTEL
  {IR(42582, 25), 712}, // 34 ArbitraryFloatLog10INTEL
  {IR(42607, 25), 713}, // 35 ArbitraryFloatLog1pINTEL
  {IR(42558, 24), 711}, // 36 ArbitraryFloatLog2INTEL
  {IR(42535, 23), 710}, // 37 ArbitraryFloatLogINTEL
  {IR(42256, 23), 698}, // 38 ArbitraryFloatMulINTEL
  {IR(43026, 23), 730}, // 39 ArbitraryFloatPowINTEL
  {IR(43073, 24), 732}, // 40 ArbitraryFloatPowNINTEL
  {IR(43049, 24), 731}, // 41 ArbitraryFloatPowRINTEL
  {IR(42437, 25), 706}, // 42 ArbitraryFloatRSqrtINTEL
  {IR(42412, 25), 705}, // 43 ArbitraryFloatRecipINTEL
  {IR(42775, 26), 720}, // 44 ArbitraryFloatSinCosINTEL
  {IR(42098, 28), 692}, // 45 ArbitraryFloatSinCosPiINTEL
  {IR(42729, 23), 718}, // 46 ArbitraryFloatSinINTEL
  {IR(42801, 25), 721}, // 47 ArbitraryFloatSinPiINTEL
  {IR(42511, 24), 709}, // 48 ArbitraryFloatSqrtINTEL
  {IR(42233, 23), 697}, // 49 ArbitraryFloatSubINTEL
  {IR(13358, 19), 780}, // 50 ArithmeticFenceEXT
  {IR(26523, 12), 61}, // 51 ArrayLength
  {IR(36748, 13), 564}, // 52 AsmCallINTEL
  {IR(12129, 9), 563}, // 53 AsmINTEL
  {IR(36733, 15), 562}, // 54 AsmTargetINTEL
  {IR(36789, 14), 567}, // 55 AssumeTrueKHR
  {IR(28795, 10), 220}, // 56 AtomicAnd
  {IR(28647, 22), 210}, // 57 AtomicCompareExchange
  {IR(28669, 26), 211}, // 58 AtomicCompareExchangeWeak
  {IR(28632, 15), 209}, // 59 AtomicExchange
  {IR(44104, 14), 770}, // 60 AtomicFAddEXT
  {IR(36775, 14), 566}, // 61 AtomicFMaxEXT
  {IR(36761, 14), 565}, // 62 AtomicFMinEXT
  {IR(30180, 16), 292}, // 63 AtomicFlagClear
  {IR(30159, 21), 291}, // 64 AtomicFlagTestAndSet
  {IR(28729, 11), 214}, // 65 AtomicIAdd
  {IR(28712, 17), 213}, // 66 AtomicIDecrement
  {IR(28695, 17), 212}, // 67 AtomicIIncrement
  {IR(28740, 11), 215}, // 68 AtomicISub
  {IR(28609, 11), 207}, // 69 AtomicLoad
  {IR(28805, 9), 221}, // 70 AtomicOr
  {IR(28773, 11), 218}, // 71 AtomicSMax
  {IR(28751, 11), 216}, // 72 AtomicSMin
  {IR(28620, 12), 208}, // 73 AtomicStore
  {IR(28784, 11), 219}, // 74 AtomicUMax
  {IR(28762, 11), 217}, // 75 AtomicUMin
  {IR(28814, 10), 222}, // 76 AtomicXor
  {IR(34997, 28), 494}, // 77 BeginInvocationInterlockEXT
  {IR(28393, 9), 191}, // 78 BitCount
  {IR(28333, 15), 187}, // 79 BitFieldInsert
  {IR(28348, 17), 188}, // 80 BitFieldSExtract
  {IR(28365, 17), 189}, // 81 BitFieldUExtract
  {IR(28382, 11), 190}, // 82 BitReverse
  {IR(27521, 8), 114}, // 83 Bitcast
  {IR(28318, 11), 185}, // 84 BitwiseAnd
  {IR(44709, 21), 793}, // 85 BitwiseFunctionINTEL
  {IR(28297, 10), 183}, // 86 BitwiseOr
  {IR(28307, 11), 184}, // 87 BitwiseXor
  {IR(28924, 7), 227}, // 88 Branch
  {IR(28931, 18), 228}, // 89 BranchConditional
  {IR(29792, 13), 277}, // 90 BuildNDRange
  {IR(25537, 11), 15}, // 91 Capability
  {IR(29750, 26), 275}, // 92 CaptureEventProfilingInfo
  {IR(31256, 23), 344}, // 93 ColorAttachmentReadEXT
  {IR(29342, 15), 255}, // 94 CommitReadPipe
  {IR(29357, 16), 256}, // 95 CommitWritePipe
  {IR(26783, 19), 72}, // 96 CompositeConstruct
  {IR(44234, 33), 775}, // 97 CompositeConstructContinuedINTEL
  {IR(32382, 31), 393}, // 98 CompositeConstructReplicateEXT
  {IR(26802, 17), 73}, // 99 CompositeExtract
  {IR(26819, 16), 74}, // 100 CompositeInsert
  {IR(15277, 9), 39}, // 101 Constant
  {IR(26043, 18), 40}, // 102 ConstantComposite
  {IR(44166, 32), 773}, // 103 ConstantCompositeContinuedINTEL
  {IR(32318, 30), 391}, // 104 ConstantCompositeReplicateEXT
  {IR(25991, 14), 38}, // 105 ConstantFalse
  {IR(36679, 29), 560}, // 106 ConstantFunctionPointerINTEL
  {IR(26155, 13), 42}, // 107 ConstantNull
  {IR(30235, 20), 296}, // 108 ConstantPipeStorage
  {IR(26139, 16), 41}, // 109 ConstantSampler
  {IR(33178, 19), 426}, // 110 ConstantStringAMDX
  {IR(25978, 13), 37}, // 111 ConstantTrue
  {IR(28580, 15), 205}, // 112 ControlBarrier
  {IR(44307, 26), 778}, // 113 ControlBarrierArriveINTEL
  {IR(44333, 24), 779}, // 114 ControlBarrierWaitINTEL
  {IR(44287, 20), 777}, // 115 ConvertBF16ToFINTEL
  {IR(44267, 20), 776}, // 116 ConvertFToBF16INTEL
  {IR(27327, 12), 100}, // 117 ConvertFToS
  {IR(27315, 12), 99}, // 118 ConvertFToU
  {IR(44923, 26), 805}, // 119 ConvertHandleToImageINTEL
  {IR(44977, 33), 807}, // 120 ConvertHandleToSampledImageINTEL
  {IR(44949, 28), 806}, // 121 ConvertHandleToSamplerINTEL
  {IR(35646, 18), 517}, // 122 ConvertImageToUNV
  {IR(27404, 14), 107}, // 123 ConvertPtrToU
  {IR(27339, 12), 101}, // 124 ConvertSToF
  {IR(35709, 25), 520}, // 125 ConvertSampledImageToUNV
  {IR(35664, 20), 518}, // 126 ConvertSamplerToUNV
  {IR(31896, 35), 377}, // 127 ConvertUToAccelerationStructureKHR
  {IR(27351, 12), 102}, // 128 ConvertUToF
  {IR(35608, 18), 515}, // 129 ConvertUToImageNV
  {IR(27448, 14), 110}, // 130 ConvertUToPtr
  {IR(35684, 25), 519}, // 131 ConvertUToSampledImageNV
  {IR(35626, 20), 516}, // 132 ConvertUToSamplerNV
  {IR(34338, 27), 469}, // 133 CooperativeMatrixConvertNV
  {IR(32291, 27), 390}, // 134 CooperativeMatrixLengthKHR
  {IR(34971, 26), 493}, // 135 CooperativeMatrixLengthNV
  {IR(32159, 25), 387}, // 136 CooperativeMatrixLoadKHR
  {IR(34896, 24), 490}, // 137 CooperativeMatrixLoadNV
  {IR(35195, 30), 497}, // 138 CooperativeMatrixLoadTensorNV
  {IR(32264, 27), 389}, // 139 CooperativeMatrixMulAddKHR
  {IR(34945, 26), 492}, // 140 CooperativeMatrixMulAddNV
  {IR(35256, 32), 499}, // 141 CooperativeMatrixPerElementOpNV
  {IR(35094, 26), 496}, // 142 CooperativeMatrixReduceNV
  {IR(32184, 26), 388}, // 143 CooperativeMatrixStoreKHR
  {IR(34920, 25), 491}, // 144 CooperativeMatrixStoreNV
  {IR(35225, 31), 498}, // 145 CooperativeMatrixStoreTensorNV
  {IR(35579, 29), 514}, // 146 CooperativeMatrixTransposeNV
  {IR(34533, 24), 476}, // 147 CooperativeVectorLoadNV
  {IR(34306, 32), 468}, // 148 CooperativeVectorMatrixMulAddNV
  {IR(34196, 29), 465}, // 149 CooperativeVectorMatrixMulNV
  {IR(34225, 42), 466}, // 150 CooperativeVectorOuterProductAccumulateNV
  {IR(34267, 39), 467}, // 151 CooperativeVectorReduceSumAccumulateNV
  {IR(34557, 25), 477}, // 152 CooperativeVectorStoreNV
  {IR(31215, 12), 340}, // 153 CopyLogical
  {IR(26449, 11), 56}, // 154 CopyMemory
  {IR(26460, 16), 57}, // 155 CopyMemorySized
  {IR(26835, 11), 75}, // 156 CopyObject
  {IR(30255, 26), 297}, // 157 CreatePipeFromPipeStorage
  {IR(35324, 21), 502}, // 158 CreateTensorLayoutNV
  {IR(35444, 19), 507}, // 159 CreateTensorViewNV
  {IR(29702, 16), 272}, // 160 CreateUserEvent
  {IR(43379, 29), 749}, // 161 CrossWorkgroupCastToPtrINTEL
  {IR(28402, 5), 192}, // 162 DPdx
  {IR(28448, 11), 198}, // 163 DPdxCoarse
  {IR(28419, 9), 195}, // 164 DPdxFine
  {IR(28407, 5), 193}, // 165 DPdy
  {IR(28459, 11), 199}, // 166 DPdyCoarse
  {IR(28428, 9), 196}, // 167 DPdyFine
  {IR(26609, 9), 64}, // 168 Decorate
  {IR(30432, 11), 305}, // 169 DecorateId
  {IR(36813, 15), 569}, // 170 DecorateString
  {IR(36828, 21), 569}, // 171 DecorateStringGOOGLE
  {IR(26633, 16), 66}, // 172 DecorationGroup
  {IR(11463, 25), 510}, // 173 DemoteToHelperInvocation
  {IR(11488, 28), 510}, // 174 DemoteToHelperInvocationEXT
  {IR(31279, 23), 345}, // 175 DepthAttachmentReadEXT
  {IR(27718, 4), 137}, // 176 Dot
  {IR(34365, 17), 470}, // 177 EmitMeshTasksEXT
  {IR(28507, 17), 203}, // 178 EmitStreamVertex
  {IR(28483, 11), 201}, // 179 EmitVertex
  {IR(35025, 26), 495}, // 180 EndInvocationInterlockEXT
  {IR(28494, 13), 202}, // 181 EndPrimitive
  {IR(28524, 19), 204}, // 182 EndStreamPrimitive
  {IR(29538, 14), 265}, // 183 EnqueueKernel
  {IR(29524, 14), 264}, // 184 EnqueueMarker
  {IR(33050, 24), 421}, // 185 EnqueueNodePayloadsAMDX
  {IR(25452, 11), 13}, // 186 EntryPoint
  {IR(31877, 19), 376}, // 187 ExecuteCallableKHR
  {IR(34807, 18), 486}, // 188 ExecuteCallableNV
  {IR(25495, 14), 14}, // 189 ExecutionMode
  {IR(30416, 16), 304}, // 190 ExecutionModeId
  {IR(36803, 10), 568}, // 191 ExpectKHR
  {IR(25306, 8), 11}, // 192 ExtInst
  {IR(25246, 14), 10}, // 193 ExtInstImport
  {IR(31839, 26), 374}, // 194 ExtInstWithForwardRefsKHR
  {IR(25236, 10), 9}, // 195 Extension
  {IR(27550, 5), 118}, // 196 FAdd
  {IR(27381, 9), 105}, // 197 FConvert
  {IR(27585, 5), 125}, // 198 FDiv
  {IR(27610, 5), 130}, // 199 FMod
  {IR(27570, 5), 122}, // 200 FMul
  {IR(27537, 8), 116}, // 201 FNegate
  {IR(28047, 10), 168}, // 202 FOrdEqual
  {IR(28125, 16), 174}, // 203 FOrdGreaterThan
  {IR(28197, 21), 178}, // 204 FOrdGreaterThanEqual
  {IR(28097, 13), 172}, // 205 FOrdLessThan
  {IR(28159, 18), 176}, // 206 FOrdLessThanEqual
  {IR(28069, 13), 170}, // 207 FOrdNotEqual
  {IR(12926, 13), 752}, // 208 FPGARegINTEL
  {IR(27605, 5), 129}, // 209 FRem
  {IR(27560, 5), 120}, // 210 FSub
  {IR(28057, 12), 169}, // 211 FUnordEqual
  {IR(28141, 18), 175}, // 212 FUnordGreaterThan
  {IR(28218, 23), 179}, // 213 FUnordGreaterThanEqual
  {IR(28110, 15), 173}, // 214 FUnordLessThan
  {IR(28177, 20), 177}, // 215 FUnordLessThanEqual
  {IR(28082, 15), 171}, // 216 FUnordNotEqual
  {IR(34495, 38), 475}, // 217 FetchMicroTriangleVertexBarycentricNV
  {IR(34460, 35), 474}, // 218 FetchMicroTriangleVertexPositionNV
  {IR(33099, 29), 423}, // 219 FinishWritingNodePayloadAMDX
  {IR(43240, 14), 741}, // 220 FixedCosINTEL
  {IR(43287, 16), 744}, // 221 FixedCosPiINTEL
  {IR(43336, 14), 747}, // 222 FixedExpINTEL
  {IR(43322, 14), 746}, // 223 FixedLogINTEL
  {IR(43194, 16), 738}, // 224 FixedRecipINTEL
  {IR(43210, 16), 739}, // 225 FixedRsqrtINTEL
  {IR(43254, 17), 742}, // 226 FixedSinCosINTEL
  {IR(43303, 19), 745}, // 227 FixedSinCosPiINTEL
  {IR(43226, 14), 740}, // 228 FixedSinINTEL
  {IR(43271, 16), 743}, // 229 FixedSinPiINTEL
  {IR(43179, 15), 737}, // 230 FixedSqrtINTEL
  {IR(32995, 17), 418}, // 231 FragmentFetchAMD
  {IR(32974, 21), 417}, // 232 FragmentMaskFetchAMD
  {IR(24356, 9), 48}, // 233 Function
  {IR(26358, 13), 51}, // 234 FunctionCall
  {IR(26346, 12), 50}, // 235 FunctionEnd
  {IR(26328, 18), 49}, // 236 FunctionParameter
  {IR(36708, 25), 561}, // 237 FunctionPointerCallINTEL
  {IR(28412, 7), 194}, // 238 Fwidth
  {IR(28470, 13), 200}, // 239 FwidthCoarse
  {IR(28437, 11), 197}, // 240 FwidthFine
  {IR(27479, 17), 112}, // 241 GenericCastToPtr
  {IR(27496, 25), 113}, // 242 GenericCastToPtrExplicit
  {IR(26535, 23), 62}, // 243 GenericPtrMemSemantics
  {IR(29776, 16), 276}, // 244 GetDefaultQueue
  {IR(30281, 35), 298}, // 245 GetKernelLocalSizeForSubgroupCount
  {IR(30316, 25), 299}, // 246 GetKernelMaxNumSubgroups
  {IR(29582, 32), 267}, // 247 GetKernelNDrangeMaxSubGroupSize
  {IR(29552, 30), 266}, // 248 GetKernelNDrangeSubGroupCount
  {IR(29637, 40), 269}, // 249 GetKernelPreferredWorkGroupSizeMultiple
  {IR(29614, 23), 268}, // 250 GetKernelWorkGroupSize
  {IR(29408, 18), 259}, // 251 GetMaxPipePackets
  {IR(29390, 18), 258}, // 252 GetNumPipePackets
  {IR(8964, 9), 353}, // 253 GraphARM
  {IR(31431, 17), 351}, // 254 GraphConstantARM
  {IR(31499, 12), 356}, // 255 GraphEndARM
  {IR(31448, 19), 352}, // 256 GraphEntryPointARM
  {IR(31467, 14), 354}, // 257 GraphInputARM
  {IR(31481, 18), 355}, // 258 GraphSetOutputARM
  {IR(29095, 9), 238}, // 259 GroupAll
  {IR(29104, 9), 239}, // 260 GroupAny
  {IR(29064, 15), 236}, // 261 GroupAsyncCopy
  {IR(44756, 19), 796}, // 262 GroupBitwiseAndKHR
  {IR(44775, 18), 797}, // 263 GroupBitwiseOrKHR
  {IR(44793, 19), 798}, // 264 GroupBitwiseXorKHR
  {IR(29113, 15), 240}, // 265 GroupBroadcast
  {IR(29483, 20), 262}, // 266 GroupCommitReadPipe
  {IR(29503, 21), 263}, // 267 GroupCommitWritePipe
  {IR(26649, 14), 67}, // 268 GroupDecorate
  {IR(29171, 10), 242}, // 269 GroupFAdd
  {IR(32813, 23), 410}, // 270 GroupFAddNonUniformAMD
  {IR(29211, 10), 246}, // 271 GroupFMax
  {IR(32905, 23), 414}, // 272 GroupFMaxNonUniformAMD
  {IR(29181, 10), 243}, // 273 GroupFMin
  {IR(32836, 23), 411}, // 274 GroupFMinNonUniformAMD
  {IR(44743, 13), 795}, // 275 GroupFMulKHR
  {IR(29161, 10), 241}, // 276 GroupIAdd
  {IR(32790, 23), 409}, // 277 GroupIAddNonUniformAMD
  {IR(44730, 13), 794}, // 278 GroupIMulKHR
  {IR(44812, 19), 799}, // 279 GroupLogicalAndKHR
  {IR(44831, 18), 800}, // 280 GroupLogicalOrKHR
  {IR(44849, 19), 801}, // 281 GroupLogicalXorKHR
  {IR(26708, 20), 68}, // 282 GroupMemberDecorate
  {IR(30464, 19), 307}, // 283 GroupNonUniformAll
  {IR(30502, 24), 309}, // 284 GroupNonUniformAllEqual
  {IR(30483, 19), 308}, // 285 GroupNonUniformAny
  {IR(5583, 22), 312}, // 286 GroupNonUniformBallot
  {IR(30642, 30), 315}, // 287 GroupNonUniformBallotBitCount
  {IR(30610, 32), 314}, // 288 GroupNonUniformBallotBitExtract
  {IR(30672, 29), 316}, // 289 GroupNonUniformBallotFindLSB
  {IR(30701, 29), 317}, // 290 GroupNonUniformBallotFindMSB
  {IR(31008, 26), 332}, // 291 GroupNonUniformBitwiseAnd
  {IR(31034, 25), 333}, // 292 GroupNonUniformBitwiseOr
  {IR(31059, 26), 334}, // 293 GroupNonUniformBitwiseXor
  {IR(30526, 25), 310}, // 294 GroupNonUniformBroadcast
  {IR(30551, 30), 311}, // 295 GroupNonUniformBroadcastFirst
  {IR(30443, 21), 306}, // 296 GroupNonUniformElect
  {IR(30828, 20), 323}, // 297 GroupNonUniformFAdd
  {IR(30988, 20), 331}, // 298 GroupNonUniformFMax
  {IR(30928, 20), 328}, // 299 GroupNonUniformFMin
  {IR(30868, 20), 325}, // 300 GroupNonUniformFMul
  {IR(30808, 20), 322}, // 301 GroupNonUniformIAdd
  {IR(30848, 20), 324}, // 302 GroupNonUniformIMul
  {IR(30581, 29), 313}, // 303 GroupNonUniformInverseBallot
  {IR(31085, 26), 335}, // 304 GroupNonUniformLogicalAnd
  {IR(31111, 25), 336}, // 305 GroupNonUniformLogicalOr
  {IR(31136, 26), 337}, // 306 GroupNonUniformLogicalXor
  {IR(34400, 27), 472}, // 307 GroupNonUniformPartitionNV
  {IR(33220, 26), 428}, // 308 GroupNonUniformQuadAllKHR
  {IR(33246, 26), 429}, // 309 GroupNonUniformQuadAnyKHR
  {IR(31162, 29), 338}, // 310 GroupNonUniformQuadBroadcast
  {IR(31191, 24), 339}, // 311 GroupNonUniformQuadSwap
  {IR(13151, 25), 372}, // 312 GroupNonUniformRotateKHR
  {IR(30948, 20), 329}, // 313 GroupNonUniformSMax
  {IR(30888, 20), 326}, // 314 GroupNonUniformSMin
  {IR(8673, 23), 318}, // 315 GroupNonUniformShuffle
  {IR(30781, 27), 321}, // 316 GroupNonUniformShuffleDown
  {IR(30756, 25), 320}, // 317 GroupNonUniformShuffleUp
  {IR(30730, 26), 319}, // 318 GroupNonUniformShuffleXor
  {IR(30968, 20), 330}, // 319 GroupNonUniformUMax
  {IR(30908, 20), 327}, // 320 GroupNonUniformUMin
  {IR(29426, 28), 260}, // 321 GroupReserveReadPipePackets
  {IR(29454, 29), 261}, // 322 GroupReserveWritePipePackets
  {IR(29231, 10), 248}, // 323 GroupSMax
  {IR(32951, 23), 416}, // 324 GroupSMaxNonUniformAMD
  {IR(29201, 10), 245}, // 325 GroupSMin
  {IR(32882, 23), 413}, // 326 GroupSMinNonUniformAMD
  {IR(29221, 10), 247}, // 327 GroupUMax
  {IR(32928, 23), 415}, // 328 GroupUMaxNonUniformAMD
  {IR(29191, 10), 244}, // 329 GroupUMin
  {IR(32859, 23), 412}, // 330 GroupUMinNonUniformAMD
  {IR(29079, 16), 237}, // 331 GroupWaitEvents
  {IR(33707, 25), 445}, // 332 HitObjectExecuteShaderNV
  {IR(33758, 25), 447}, // 333 HitObjectGetAttributesNV
  {IR(34848, 24), 488}, // 334 HitObjectGetClusterIdNV
  {IR(33732, 26), 446}, // 335 HitObjectGetCurrentTimeNV
  {IR(33834, 28), 450}, // 336 HitObjectGetGeometryIndexNV
  {IR(33783, 22), 448}, // 337 HitObjectGetHitKindNV
  {IR(33887, 34), 452}, // 338 HitObjectGetInstanceCustomIndexNV
  {IR(33862, 25), 451}, // 339 HitObjectGetInstanceIdNV
  {IR(36075, 27), 530}, // 340 HitObjectGetLSSPositionsNV
  {IR(36102, 23), 531}, // 341 HitObjectGetLSSRadiiNV
  {IR(33419, 33), 435}, // 342 HitObjectGetObjectRayDirectionNV
  {IR(33452, 30), 436}, // 343 HitObjectGetObjectRayOriginNV
  {IR(33391, 28), 434}, // 344 HitObjectGetObjectToWorldNV
  {IR(33805, 29), 449}, // 345 HitObjectGetPrimitiveIndexNV
  {IR(33982, 22), 455}, // 346 HitObjectGetRayTMaxNV
  {IR(34004, 22), 456}, // 347 HitObjectGetRayTMinNV
  {IR(33547, 44), 439}, // 348 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(33508, 39), 438}, // 349 HitObjectGetShaderRecordBufferHandleNV
  {IR(36019, 29), 528}, // 350 HitObjectGetSpherePositionNV
  {IR(36048, 27), 529}, // 351 HitObjectGetSphereRadiusNV
  {IR(33921, 32), 453}, // 352 HitObjectGetWorldRayDirectionNV
  {IR(33953, 29), 454}, // 353 HitObjectGetWorldRayOriginNV
  {IR(33363, 28), 433}, // 354 HitObjectGetWorldToObjectNV
  {IR(34026, 19), 457}, // 355 HitObjectIsEmptyNV
  {IR(34045, 17), 458}, // 356 HitObjectIsHitNV
  {IR(36148, 20), 533}, // 357 HitObjectIsLSSHitNV
  {IR(34062, 18), 459}, // 358 HitObjectIsMissNV
  {IR(36125, 23), 532}, // 359 HitObjectIsSphereHitNV
  {IR(33591, 23), 440}, // 360 HitObjectRecordEmptyNV
  {IR(33272, 27), 430}, // 361 HitObjectRecordHitMotionNV
  {IR(33634, 21), 442}, // 362 HitObjectRecordHitNV
  {IR(33299, 36), 431}, // 363 HitObjectRecordHitWithIndexMotionNV
  {IR(33655, 30), 443}, // 364 HitObjectRecordHitWithIndexNV
  {IR(33335, 28), 432}, // 365 HitObjectRecordMissMotionNV
  {IR(33685, 22), 444}, // 366 HitObjectRecordMissNV
  {IR(33482, 26), 437}, // 367 HitObjectTraceRayMotionNV
  {IR(33614, 20), 441}, // 368 HitObjectTraceRayNV
  {IR(27545, 5), 117}, // 369 IAdd
  {IR(27722, 10), 138}, // 370 IAddCarry
  {IR(36527, 13), 550}, // 371 IAddSatINTEL
  {IR(36553, 14), 552}, // 372 IAverageINTEL
  {IR(36581, 21), 554}, // 373 IAverageRoundedINTEL
  {IR(27918, 7), 158}, // 374 IEqual
  {IR(27565, 5), 121}, // 375 IMul
  {IR(36649, 15), 558}, // 376 IMul32x16INTEL
  {IR(27925, 10), 159}, // 377 INotEqual
  {IR(27555, 5), 119}, // 378 ISub
  {IR(27732, 11), 139}, // 379 ISubBorrow
  {IR(36623, 13), 556}, // 380 ISubSatINTEL
  {IR(31931, 22), 378}, // 381 IgnoreIntersectionKHR
  {IR(34625, 21), 479}, // 382 IgnoreIntersectionNV
  {IR(24400, 6), 91}, // 383 Image
  {IR(32761, 29), 408}, // 384 ImageBlockMatchGatherSADQCOM
  {IR(32732, 29), 407}, // 385 ImageBlockMatchGatherSSDQCOM
  {IR(32651, 23), 404}, // 386 ImageBlockMatchSADQCOM
  {IR(32628, 23), 403}, // 387 ImageBlockMatchSSDQCOM
  {IR(32703, 29), 406}, // 388 ImageBlockMatchWindowSADQCOM
  {IR(32674, 29), 405}, // 389 ImageBlockMatchWindowSSDQCOM
  {IR(32609, 19), 402}, // 390 ImageBoxFilterQCOM
  {IR(27163, 16), 88}, // 391 ImageDrefGather
  {IR(27140, 11), 86}, // 392 ImageFetch
  {IR(27151, 12), 87}, // 393 ImageGather
  {IR(27200, 17), 92}, // 394 ImageQueryFormat
  {IR(27280, 17), 97}, // 395 ImageQueryLevels
  {IR(27266, 14), 96}, // 396 ImageQueryLod
  {IR(27217, 16), 93}, // 397 ImageQueryOrder
  {IR(27297, 18), 98}, // 398 ImageQuerySamples
  {IR(27251, 15), 95}, // 399 ImageQuerySize
  {IR(27233, 18), 94}, // 400 ImageQuerySizeLod
  {IR(27179, 10), 89}, // 401 ImageRead
  {IR(26997, 27), 81}, // 402 ImageSampleDrefExplicitLod
  {IR(26970, 27), 80}, // 403 ImageSampleDrefImplicitLod
  {IR(26947, 23), 79}, // 404 ImageSampleExplicitLod
  {IR(34149, 23), 463}, // 405 ImageSampleFootprintNV
  {IR(26901, 23), 78}, // 406 ImageSampleImplicitLod
  {IR(27109, 31), 85}, // 407 ImageSampleProjDrefExplicitLod
  {IR(27078, 31), 84}, // 408 ImageSampleProjDrefImplicitLod
  {IR(27051, 27), 83}, // 409 ImageSampleProjExplicitLod
  {IR(27024, 27), 82}, // 410 ImageSampleProjImplicitLod
  {IR(32585, 24), 401}, // 411 ImageSampleWeightedQCOM
  {IR(30104, 22), 288}, // 412 ImageSparseDrefGather
  {IR(30069, 17), 286}, // 413 ImageSparseFetch
  {IR(30086, 18), 287}, // 414 ImageSparseGather
  {IR(30196, 16), 293}, // 415 ImageSparseRead
  {IR(29896, 33), 281}, // 416 ImageSparseSampleDrefExplicitLod
  {IR(29863, 33), 280}, // 417 ImageSparseSampleDrefImplicitLod
  {IR(29834, 29), 279}, // 418 ImageSparseSampleExplicitLod
  {IR(29805, 29), 278}, // 419 ImageSparseSampleImplicitLod
  {IR(30032, 37), 285}, // 420 ImageSparseSampleProjDrefExplicitLod
  {IR(29995, 37), 284}, // 421 ImageSparseSampleProjDrefImplicitLod
  {IR(29962, 33), 283}, // 422 ImageSparseSampleProjExplicitLod
  {IR(29929, 33), 282}, // 423 ImageSparseSampleProjImplicitLod
  {IR(30126, 26), 289}, // 424 ImageSparseTexelsResident
  {IR(26380, 18), 53}, // 425 ImageTexelPointer
  {IR(27189, 11), 90}, // 426 ImageWrite
  {IR(26488, 20), 59}, // 427 InBoundsAccessChain
  {IR(26558, 23), 63}, // 428 InBoundsPtrAccessChain
  {IR(27789, 9), 146}, // 429 IsFinite
  {IR(35510, 22), 511}, // 430 IsHelperInvocationEXT
  {IR(27783, 6), 145}, // 431 IsInf
  {IR(27777, 6), 144}, // 432 IsNan
  {IR(33155, 23), 425}, // 433 IsNodePayloadValidAMDX
  {IR(27798, 9), 147}, // 434 IsNormal
  {IR(29718, 13), 273}, // 435 IsValidEvent
  {IR(29373, 17), 257}, // 436 IsValidReserveId
  {IR(29001, 5), 230}, // 437 Kill
  {IR(28918, 6), 226}, // 438 Label
  {IR(27818, 14), 149}, // 439 LessOrGreater
  {IR(29037, 14), 234}, // 440 LifetimeStart
  {IR(29051, 13), 235}, // 441 LifetimeStop
  {IR(25231, 5), 8}, // 442 Line
  {IR(26438, 5), 54}, // 443 Load
  {IR(27889, 11), 155}, // 444 LogicalAnd
  {IR(27850, 13), 152}, // 445 LogicalEqual
  {IR(27900, 11), 156}, // 446 LogicalNot
  {IR(27863, 16), 153}, // 447 LogicalNotEqual
  {IR(27879, 10), 154}, // 448 LogicalOr
  {IR(43097, 17), 733}, // 449 LoopControlINTEL
  {IR(28858, 10), 224}, // 450 LoopMerge
  {IR(44886, 18), 803}, // 451 MaskedGatherINTEL
  {IR(44904, 19), 804}, // 452 MaskedScatterINTEL
  {IR(27687, 18), 135}, // 453 MatrixTimesMatrix
  {IR(27633, 18), 132}, // 454 MatrixTimesScalar
  {IR(27669, 18), 134}, // 455 MatrixTimesVector
  {IR(26618, 15), 65}, // 456 MemberDecorate
  {IR(36849, 21), 570}, // 457 MemberDecorateString
  {IR(36870, 27), 570}, // 458 MemberDecorateStringGOOGLE
  {IR(25213, 11), 6}, // 459 MemberName
  {IR(28595, 14), 206}, // 460 MemoryBarrier
  {IR(25378, 12), 12}, // 461 MemoryModel
  {IR(30381, 19), 302}, // 462 MemoryNamedBarrier
  {IR(30400, 16), 303}, // 463 ModuleProcessed
  {IR(25208, 5), 5}, // 464 Name
  {IR(30358, 23), 301}, // 465 NamedBarrierInitialize
  {IR(30152, 7), 290}, // 466 NoLine
  {IR(33128, 27), 424}, // 467 NodePayloadArrayLengthAMDX
  {IR(25004, 4), 0}, // 468 Nop
  {IR(28329, 4), 186}, // 469 Not
  {IR(27832, 8), 150}, // 470 Ordered
  {IR(27705, 13), 136}, // 471 OuterProduct
  {IR(28824, 4), 223}, // 472 Phi
  {IR(26508, 15), 60}, // 473 PtrAccessChain
  {IR(43350, 29), 748}, // 474 PtrCastToCrossWorkgroupINTEL
  {IR(27462, 17), 111}, // 475 PtrCastToGeneric
  {IR(31248, 8), 343}, // 476 PtrDiff
  {IR(31227, 9), 341}, // 477 PtrEqual
  {IR(31236, 12), 342}, // 478 PtrNotEqual
  {IR(27390, 14), 106}, // 479 QuantizeToF16
  {IR(35815, 17), 522}, // 480 RawAccessChainNV
  {IR(32504, 31), 398}, // 481 RayQueryConfirmIntersectionKHR
  {IR(32472, 32), 397}, // 482 RayQueryGenerateIntersectionKHR
  {IR(34825, 23), 487}, // 483 RayQueryGetClusterIdNV
  {IR(43755, 39), 761}, // 484 RayQueryGetIntersectionBarycentricsKHR
  {IR(43830, 46), 763}, // 485 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(43794, 36), 762}, // 486 RayQueryGetIntersectionFrontFaceKHR
  {IR(43674, 40), 759}, // 487 RayQueryGetIntersectionGeometryIndexKHR
  {IR(43526, 46), 756}, // 488 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(43572, 37), 757}, // 489 RayQueryGetIntersectionInstanceIdKHR
  {IR(43609, 65), 758}, // 490 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(35982, 37), 527}, // 491 RayQueryGetIntersectionLSSHitValueNV
  {IR(35910, 38), 525}, // 492 RayQueryGetIntersectionLSSPositionsNV
  {IR(35948, 34), 526}, // 493 RayQueryGetIntersectionLSSRadiiNV
  {IR(43876, 45), 764}, // 494 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(43921, 42), 765}, // 495 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(44024, 40), 768}, // 496 RayQueryGetIntersectionObjectToWorldKHR
  {IR(43714, 41), 760}, // 497 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(35832, 40), 523}, // 498 RayQueryGetIntersectionSpherePositionNV
  {IR(35872, 38), 524}, // 499 RayQueryGetIntersectionSphereRadiusNV
  {IR(43498, 28), 755}, // 500 RayQueryGetIntersectionTKHR
  {IR(34700, 50), 484}, // 501 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(32554, 31), 400}, // 502 RayQueryGetIntersectionTypeKHR
  {IR(44064, 40), 769}, // 503 RayQueryGetIntersectionWorldToObjectKHR
  {IR(43475, 23), 754}, // 504 RayQueryGetRayFlagsKHR
  {IR(43453, 22), 753}, // 505 RayQueryGetRayTMinKHR
  {IR(43963, 32), 766}, // 506 RayQueryGetWorldRayDirectionKHR
  {IR(43995, 29), 767}, // 507 RayQueryGetWorldRayOriginKHR
  {IR(32429, 22), 395}, // 508 RayQueryInitializeKHR
  {IR(36190, 19), 535}, // 509 RayQueryIsLSSHitNV
  {IR(36168, 22), 534}, // 510 RayQueryIsSphereHitNV
  {IR(32535, 19), 399}, // 511 RayQueryProceedKHR
  {IR(32451, 21), 396}, // 512 RayQueryTerminateKHR
  {IR(33012, 13), 419}, // 513 ReadClockKHR
  {IR(29241, 9), 249}, // 514 ReadPipe
  {IR(43408, 22), 750}, // 515 ReadPipeBlockingINTEL
  {IR(29689, 13), 271}, // 516 ReleaseEvent
  {IR(34109, 24), 461}, // 517 ReorderThreadWithHintNV
  {IR(34080, 29), 460}, // 518 ReorderThreadWithHitObjectNV
  {IR(34582, 22), 478}, // 519 ReportIntersectionKHR
  {IR(34604, 21), 478}, // 520 ReportIntersectionNV
  {IR(29295, 23), 253}, // 521 ReserveReadPipePackets
  {IR(29318, 24), 254}, // 522 ReserveWritePipePackets
  {IR(29260, 17), 251}, // 523 ReservedReadPipe
  {IR(29277, 18), 252}, // 524 ReservedWritePipe
  {IR(42079, 19), 691}, // 525 RestoreMemoryINTEL
  {IR(29677, 12), 270}, // 526 RetainEvent
  {IR(29006, 7), 231}, // 527 Return
  {IR(29013, 12), 232}, // 528 ReturnValue
  {IR(44868, 18), 802}, // 529 RoundFToTF32INTEL
  {IR(27372, 9), 104}, // 530 SConvert
  {IR(27580, 5), 124}, // 531 SDiv
  {IR(32016, 5), 380}, // 532 SDot
  {IR(32057, 11), 383}, // 533 SDotAccSat
  {IR(32068, 14), 383}, // 534 SDotAccSatKHR
  {IR(32021, 8), 380}, // 535 SDotKHR
  {IR(27948, 13), 161}, // 536 SGreaterThan
  {IR(27979, 18), 163}, // 537 SGreaterThanEqual
  {IR(28007, 10), 165}, // 538 SLessThan
  {IR(28032, 15), 167}, // 539 SLessThanEqual
  {IR(27600, 5), 128}, // 540 SMod
  {IR(27756, 13), 141}, // 541 SMulExtended
  {IR(27529, 8), 115}, // 542 SNegate
  {IR(27595, 5), 127}, // 543 SRem
  {IR(32042, 6), 382}, // 544 SUDot
  {IR(32107, 12), 385}, // 545 SUDotAccSat
  {IR(32119, 15), 385}, // 546 SUDotAccSatKHR
  {IR(32048, 9), 382}, // 547 SUDotKHR
  {IR(26856, 13), 77}, // 548 SampledImage
  {IR(35734, 29), 521}, // 549 SamplerImageAddressingModeNV
  {IR(27418, 15), 108}, // 550 SatConvertSToU
  {IR(27433, 15), 109}, // 551 SatConvertUToS
  {IR(42063, 16), 690}, // 552 SaveMemoryINTEL
  {IR(27911, 7), 157}, // 553 Select
  {IR(28903, 15), 225}, // 554 SelectionMerge
  {IR(34382, 18), 471}, // 555 SetMeshOutputsEXT
  {IR(29731, 19), 274}, // 556 SetUserEventStatus
  {IR(28280, 17), 182}, // 557 ShiftLeftLogical
  {IR(28259, 21), 181}, // 558 ShiftRightArithmetic
  {IR(28241, 18), 180}, // 559 ShiftRightLogical
  {IR(27807, 11), 148}, // 560 SignBitSet
  {IR(30212, 7), 294}, // 561 SizeOf
  {IR(25185, 7), 3}, // 562 Source
  {IR(25066, 16), 2}, // 563 SourceContinued
  {IR(25192, 16), 4}, // 564 SourceExtension
  {IR(26203, 13), 45}, // 565 SpecConstant
  {IR(26216, 22), 46}, // 566 SpecConstantComposite
  {IR(44198, 36), 774}, // 567 SpecConstantCompositeContinuedINTEL
  {IR(32348, 34), 392}, // 568 SpecConstantCompositeReplicateEXT
  {IR(26185, 18), 44}, // 569 SpecConstantFalse
  {IR(26279, 15), 47}, // 570 SpecConstantOp
  {IR(33197, 23), 427}, // 571 SpecConstantStringAMDX
  {IR(26168, 17), 43}, // 572 SpecConstantTrue
  {IR(31302, 25), 346}, // 573 StencilAttachmentReadEXT
  {IR(26443, 6), 55}, // 574 Store
  {IR(25224, 7), 7}, // 575 String
  {IR(44499, 25), 787}, // 576 Subgroup2DBlockLoadINTEL
  {IR(44524, 34), 788}, // 577 Subgroup2DBlockLoadTransformINTEL
  {IR(44558, 34), 789}, // 578 Subgroup2DBlockLoadTransposeINTEL
  {IR(44592, 29), 790}, // 579 Subgroup2DBlockPrefetchINTEL
  {IR(44621, 26), 791}, // 580 Subgroup2DBlockStoreINTEL
  {IR(31793, 20), 371}, // 581 SubgroupAllEqualKHR
  {IR(31763, 15), 369}, // 582 SubgroupAllKHR
  {IR(31778, 15), 370}, // 583 SubgroupAnyKHR
  {IR(40581, 30), 654}, // 584 SubgroupAvcBmeInitializeINTEL
  {IR(40551, 30), 653}, // 585 SubgroupAvcFmeInitializeINTEL
  {IR(38986, 35), 623}, // 586 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(39021, 39), 624}, // 587 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(39662, 38), 637}, // 588 SubgroupAvcImeConvertToMceResultINTEL
  {IR(39285, 45), 630}, // 589 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(39385, 53), 632}, // 590 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(39606, 56), 636}, // 591 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(39494, 54), 634}, // 592 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(39238, 47), 629}, // 593 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(39330, 55), 631}, // 594 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(39548, 58), 635}, // 595 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(39438, 56), 633}, // 596 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(40294, 36), 648}, // 597 SubgroupAvcImeGetBorderReachedINTEL
  {IR(39746, 44), 639}, // 598 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(39700, 46), 638}, // 599 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(40161, 66), 646}, // 600 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(40093, 68), 645}, // 601 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(40227, 67), 647}, // 602 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(39956, 68), 643}, // 603 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(39886, 70), 642}, // 604 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(40024, 69), 644}, // 605 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(40330, 48), 649}, // 606 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(40378, 59), 650}, // 607 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(40495, 56), 652}, // 608 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(40437, 58), 651}, // 609 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(38849, 30), 619}, // 610 SubgroupAvcImeInitializeINTEL
  {IR(38953, 33), 622}, // 611 SubgroupAvcImeRefWindowSizeINTEL
  {IR(38917, 36), 621}, // 612 SubgroupAvcImeSetDualReferenceINTEL
  {IR(39150, 54), 627}, // 613 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(39060, 43), 625}, // 614 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(38879, 38), 620}, // 615 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(39103, 47), 626}, // 616 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(39204, 34), 628}, // 617 SubgroupAvcImeSetWeightedSadINTEL
  {IR(39839, 47), 641}, // 618 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(39790, 49), 640}, // 619 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(38239, 39), 604}, // 620 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(38278, 38), 605}, // 621 SubgroupAvcMceConvertToImeResultINTEL
  {IR(38316, 39), 606}, // 622 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(38355, 38), 607}, // 623 SubgroupAvcMceConvertToRefResultINTEL
  {IR(38393, 39), 608}, // 624 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(38432, 38), 609}, // 625 SubgroupAvcMceConvertToSicResultINTEL
  {IR(38545, 43), 612}, // 626 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(37677, 50), 593}, // 627 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(37275, 60), 585}, // 628 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(37475, 51), 589}, // 629 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(37621, 56), 592}, // 630 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(37388, 47), 587}, // 631 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(37976, 56), 599}, // 632 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(37875, 50), 597}, // 633 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(37570, 51), 591}, // 634 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(37779, 49), 595}, // 635 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(37727, 52), 594}, // 636 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(37925, 51), 598}, // 637 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(38664, 38), 615}, // 638 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(38506, 39), 611}, // 639 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(38588, 38), 613}, // 640 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(38626, 38), 614}, // 641 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(38702, 45), 616}, // 642 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(38747, 40), 617}, // 643 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(38787, 62), 618}, // 644 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(38470, 36), 610}, // 645 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(38032, 33), 600}, // 646 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(38178, 61), 603}, // 647 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(37335, 53), 586}, // 648 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(37526, 44), 590}, // 649 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(37435, 40), 588}, // 650 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(37828, 47), 596}, // 651 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(38117, 61), 602}, // 652 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(38065, 52), 601}, // 653 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(40611, 39), 655}, // 654 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(40933, 38), 662}, // 655 SubgroupAvcRefConvertToMceResultINTEL
  {IR(40786, 45), 659}, // 656 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(40831, 46), 660}, // 657 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(40877, 56), 661}, // 658 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(40739, 47), 658}, // 659 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(40650, 46), 656}, // 660 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(40696, 43), 657}, // 661 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(41069, 42), 666}, // 662 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(41033, 36), 665}, // 663 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(41001, 32), 664}, // 664 SubgroupAvcSicConfigureSkcINTEL
  {IR(41150, 39), 668}, // 665 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(41690, 38), 680}, // 666 SubgroupAvcSicConvertToMceResultINTEL
  {IR(41465, 31), 675}, // 667 SubgroupAvcSicEvaluateIpeINTEL
  {IR(41543, 45), 677}, // 668 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(41588, 46), 678}, // 669 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(41634, 56), 679}, // 670 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41496, 47), 676}, // 671 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(41807, 46), 683}, // 672 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(41763, 44), 682}, // 673 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(42028, 35), 688}, // 674 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(41894, 36), 685}, // 675 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(41728, 35), 681}, // 676 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(41111, 39), 667}, // 677 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(41853, 41), 684}, // 678 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(41930, 50), 686}, // 679 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(41980, 48), 687}, // 680 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(40971, 30), 663}, // 681 SubgroupAvcSicInitializeINTEL
  {IR(41331, 43), 672}, // 682 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(41422, 43), 674}, // 683 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(41281, 50), 671}, // 684 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(41233, 48), 670}, // 685 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(41189, 44), 669}, // 686 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(41374, 48), 673}, // 687 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(5344, 18), 363}, // 688 SubgroupBallotKHR
  {IR(44472, 27), 786}, // 689 SubgroupBlockPrefetchINTEL
  {IR(36281, 23), 540}, // 690 SubgroupBlockReadINTEL
  {IR(36304, 24), 541}, // 691 SubgroupBlockWriteINTEL
  {IR(31637, 27), 364}, // 692 SubgroupFirstInvocationKHR
  {IR(36328, 28), 542}, // 693 SubgroupImageBlockReadINTEL
  {IR(36356, 29), 543}, // 694 SubgroupImageBlockWriteINTEL
  {IR(36385, 33), 544}, // 695 SubgroupImageMediaBlockReadINTEL
  {IR(36418, 34), 545}, // 696 SubgroupImageMediaBlockWriteINTEL
  {IR(13696, 38), 792}, // 697 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(31813, 26), 373}, // 698 SubgroupReadInvocationKHR
  {IR(36209, 25), 537}, // 699 SubgroupShuffleDownINTEL
  {IR(11911, 21), 536}, // 700 SubgroupShuffleINTEL
  {IR(36234, 23), 538}, // 701 SubgroupShuffleUpINTEL
  {IR(36257, 24), 539}, // 702 SubgroupShuffleXorINTEL
  {IR(28994, 7), 229}, // 703 Switch
  {IR(44381, 23), 782}, // 704 TaskSequenceAsyncINTEL
  {IR(44357, 24), 781}, // 705 TaskSequenceCreateINTEL
  {IR(44404, 21), 783}, // 706 TaskSequenceGetINTEL
  {IR(44425, 25), 784}, // 707 TaskSequenceReleaseINTEL
  {IR(35552, 27), 513}, // 708 TensorLayoutSetBlockSizeNV
  {IR(35416, 28), 506}, // 709 TensorLayoutSetClampValueNV
  {IR(35345, 27), 503}, // 710 TensorLayoutSetDimensionNV
  {IR(35372, 24), 504}, // 711 TensorLayoutSetStrideNV
  {IR(35396, 20), 505}, // 712 TensorLayoutSliceNV
  {IR(31412, 19), 350}, // 713 TensorQuerySizeARM
  {IR(31383, 14), 348}, // 714 TensorReadARM
  {IR(35532, 20), 512}, // 715 TensorViewSetClipNV
  {IR(35463, 25), 508}, // 716 TensorViewSetDimensionNV
  {IR(35488, 22), 509}, // 717 TensorViewSetStrideNV
  {IR(31397, 15), 349}, // 718 TensorWriteARM
  {IR(31524, 20), 358}, // 719 TerminateInvocation
  {IR(31953, 16), 379}, // 720 TerminateRayKHR
  {IR(34646, 15), 480}, // 721 TerminateRayNV
  {IR(34669, 14), 482}, // 722 TraceMotionNV
  {IR(34661, 8), 481}, // 723 TraceNV
  {IR(31865, 12), 375}, // 724 TraceRayKHR
  {IR(34683, 17), 483}, // 725 TraceRayMotionNV
  {IR(26846, 10), 76}, // 726 Transpose
  {IR(34750, 29), 485}, // 727 TypeAccelerationStructureKHR
  {IR(34779, 28), 485}, // 728 TypeAccelerationStructureNV
  {IR(25795, 10), 25}, // 729 TypeArray
  {IR(37194, 37), 582}, // 730 TypeAvcImeDualReferenceStreaminINTEL
  {IR(36929, 23), 573}, // 731 TypeAvcImePayloadINTEL
  {IR(37111, 44), 580}, // 732 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(37043, 22), 578}, // 733 TypeAvcImeResultINTEL
  {IR(37065, 46), 579}, // 734 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(37155, 39), 581}, // 735 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(36998, 23), 576}, // 736 TypeAvcMcePayloadINTEL
  {IR(37021, 22), 577}, // 737 TypeAvcMceResultINTEL
  {IR(36952, 23), 574}, // 738 TypeAvcRefPayloadINTEL
  {IR(37231, 22), 583}, // 739 TypeAvcRefResultINTEL
  {IR(36975, 23), 575}, // 740 TypeAvcSicPayloadINTEL
  {IR(37253, 22), 584}, // 741 TypeAvcSicResultINTEL
  {IR(25557, 9), 17}, // 742 TypeBool
  {IR(44118, 23), 771}, // 743 TypeBufferSurfaceINTEL
  {IR(32134, 25), 386}, // 744 TypeCooperativeMatrixKHR
  {IR(34872, 24), 489}, // 745 TypeCooperativeMatrixNV
  {IR(34172, 24), 464}, // 746 TypeCooperativeVectorNV
  {IR(25910, 16), 32}, // 747 TypeDeviceEvent
  {IR(25900, 10), 31}, // 748 TypeEvent
  {IR(25611, 10), 19}, // 749 TypeFloat
  {IR(25959, 19), 36}, // 750 TypeForwardPointer
  {IR(25887, 13), 30}, // 751 TypeFunction
  {IR(31511, 13), 357}, // 752 TypeGraphARM
  {IR(34133, 16), 462}, // 753 TypeHitObjectNV
  {IR(25756, 10), 22}, // 754 TypeImage
  {IR(25566, 8), 18}, // 755 TypeInt
  {IR(25632, 11), 21}, // 756 TypeMatrix
  {IR(30341, 17), 300}, // 757 TypeNamedBarrier
  {IR(33074, 25), 422}, // 758 TypeNodePayloadArrayAMDX
  {IR(25833, 11), 28}, // 759 TypeOpaque
  {IR(25950, 9), 35}, // 760 TypePipe
  {IR(30219, 16), 295}, // 761 TypePipeStorage
  {IR(25875, 12), 29}, // 762 TypePointer
  {IR(25940, 10), 34}, // 763 TypeQueue
  {IR(32413, 16), 394}, // 764 TypeRayQueryKHR
  {IR(25926, 14), 33}, // 765 TypeReserveId
  {IR(25805, 17), 26}, // 766 TypeRuntimeArray
  {IR(25778, 17), 24}, // 767 TypeSampledImage
  {IR(25766, 12), 23}, // 768 TypeSampler
  {IR(25822, 11), 27}, // 769 TypeStruct
  {IR(44141, 25), 772}, // 770 TypeStructContinuedINTEL
  {IR(44450, 22), 785}, // 771 TypeTaskSequenceINTEL
  {IR(31327, 14), 347}, // 772 TypeTensorARM
  {IR(35288, 19), 500}, // 773 TypeTensorLayoutNV
  {IR(35307, 17), 501}, // 774 TypeTensorViewNV
  {IR(31544, 22), 359}, // 775 TypeUntypedPointerKHR
  {IR(25621, 11), 20}, // 776 TypeVector
  {IR(36911, 18), 572}, // 777 TypeVmeImageINTEL
  {IR(25548, 9), 16}, // 778 TypeVoid
  {IR(36540, 13), 551}, // 779 UAddSatINTEL
  {IR(36567, 14), 553}, // 780 UAverageINTEL
  {IR(36602, 21), 555}, // 781 UAverageRoundedINTEL
  {IR(27363, 9), 103}, // 782 UConvert
  {IR(36452, 24), 546}, // 783 UCountLeadingZerosINTEL
  {IR(36476, 25), 547}, // 784 UCountTrailingZerosINTEL
  {IR(27575, 5), 123}, // 785 UDiv
  {IR(32029, 5), 381}, // 786 UDot
  {IR(32082, 11), 384}, // 787 UDotAccSat
  {IR(32093, 14), 384}, // 788 UDotAccSatKHR
  {IR(32034, 8), 381}, // 789 UDotKHR
  {IR(27935, 13), 160}, // 790 UGreaterThan
  {IR(27961, 18), 162}, // 791 UGreaterThanEqual
  {IR(27997, 10), 164}, // 792 ULessThan
  {IR(28017, 15), 166}, // 793 ULessThanEqual
  {IR(27590, 5), 126}, // 794 UMod
  {IR(36664, 15), 559}, // 795 UMul32x16INTEL
  {IR(27743, 13), 140}, // 796 UMulExtended
  {IR(36636, 13), 557}, // 797 USubSatINTEL
  {IR(25060, 6), 1}, // 798 Undef
  {IR(27840, 10), 151}, // 799 Unordered
  {IR(29025, 12), 233}, // 800 Unreachable
  {IR(31585, 22), 361}, // 801 UntypedAccessChainKHR
  {IR(31722, 22), 367}, // 802 UntypedArrayLengthKHR
  {IR(31607, 30), 362}, // 803 UntypedInBoundsAccessChainKHR
  {IR(31689, 33), 366}, // 804 UntypedInBoundsPtrAccessChainKHR
  {IR(31744, 19), 368}, // 805 UntypedPrefetchKHR
  {IR(31664, 25), 365}, // 806 UntypedPtrAccessChainKHR
  {IR(31566, 19), 360}, // 807 UntypedVariableKHR
  {IR(26371, 9), 52}, // 808 Variable
  {IR(12367, 25), 689}, // 809 VariableLengthArrayINTEL
  {IR(26728, 21), 69}, // 810 VectorExtractDynamic
  {IR(26749, 20), 70}, // 811 VectorInsertDynamic
  {IR(26769, 14), 71}, // 812 VectorShuffle
  {IR(27651, 18), 133}, // 813 VectorTimesMatrix
  {IR(27615, 18), 131}, // 814 VectorTimesScalar
  {IR(36897, 14), 571}, // 815 VmeImageINTEL
  {IR(34427, 33), 473}, // 816 WritePackedPrimitiveIndices4x8NV
  {IR(29250, 10), 250}, // 817 WritePipe
  {IR(43430, 23), 751}, // 818 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 491> kExtInstByValue{{
    {0,IR(0, 0),IR(45010, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(45024, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(739, 3),IR(45097, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(742, 3),IR(45146, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(745, 2),IR(45201, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(45220, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(45235, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(747, 6),IR(45251, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(45264, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(753, 9),IR(45282, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(762, 9),IR(45334, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(771, 10),IR(45353, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(781, 5),IR(45369, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(45390, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(45411, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(786, 6),IR(45429, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(792, 5),IR(45456, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(797, 5),IR(45491, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(771, 10),IR(45522, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(802, 8),IR(45542, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(810, 11),IR(45567, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(821, 5),IR(45581, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(826, 3),IR(45599, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(829, 2),IR(45630, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(45641, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(831, 3),IR(45654, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(834, 7),IR(45711, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(45730, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(45751, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(45764, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(841, 2),IR(45808, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(695, 1),IR(45823, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(843, 4),IR(45839, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(826, 3),IR(45853, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(45869, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(45875, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(45885, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(45891, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(45896, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(45901, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(45907, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(45913, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(45919, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(45924, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(45930, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(45938, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(45946, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(45950, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(45954, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(45958, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(45963, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(45968, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(45973, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(45978, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(45983, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(45988, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(45994, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(46000, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(46006, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(46012, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(46016, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(46020, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(46024, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(46029, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(46034, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(46039, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(46051, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(46063, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(46077, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(46082, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(46093, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(46098, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(46103, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(46108, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(46113, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(46118, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(46123, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(46130, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(46137, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(46144, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(46149, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(46154, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(46159, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(46170, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(46174, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(46180, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(46192, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(46198, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(46211, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(46224, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(46238, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(46252, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(46265, 15),IR(305, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(46280, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(46296, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(46312, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(46327, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(46342, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(46357, 17),IR(305, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(46374, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(46381, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(46390, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(46396, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(46406, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(46418, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(46426, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(46434, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(46443, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(46452, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(46461, 22),IR(306, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(46483, 20),IR(306, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(46503, 20),IR(306, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(46523, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(46528, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(46533, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(4563, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(847, 5),IR(46540, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(852, 5),IR(46553, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(852, 5),IR(46575, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(543, 7),IR(46591, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(543, 7),IR(46616, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(852, 5),IR(46635, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(852, 5),IR(46659, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(852, 5),IR(46680, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(852, 5),IR(46701, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(852, 5),IR(46717, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(46735, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(46761, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(46786, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(46806, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(46831, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(46861, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(46884, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(46909, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(46935, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(46965, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(46991, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(7988, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(47011, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(47041, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(852, 5),IR(47069, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(543, 7),IR(47097, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(47120, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(47155, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(47193, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(47235, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(514, 6),IR(47280, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(514, 6),IR(47317, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(852, 5),IR(47357, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(852, 5),IR(47384, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(47411, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(47443, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(47483, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(47494, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(47520, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(47552, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(47586, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(45010, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(45024, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(45097, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(45146, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(45201, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(45220, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(45235, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(514, 6),IR(45251, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(375, 3),IR(45264, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(857, 9),IR(45282, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(866, 10),IR(45334, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(876, 9),IR(45353, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(45369, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(45390, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(45411, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(514, 6),IR(45429, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(623, 5),IR(45456, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(885, 5),IR(45491, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(890, 10),IR(45522, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(460, 8),IR(45542, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(890, 10),IR(45567, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(852, 5),IR(45581, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(45599, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(829, 2),IR(45630, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(45641, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(900, 3),IR(45654, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(903, 8),IR(45711, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(45730, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(911, 4),IR(45751, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(911, 4),IR(45764, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(45808, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(695, 1),IR(45823, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(566, 4),IR(45839, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(45853, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(495, 7),IR(47608, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(829, 2),IR(47628, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(47640, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(47664, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(623, 5),IR(47685, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(47695, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(47707, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(47728, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(47745, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(47761, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(713, 9),IR(47777, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(47791, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(47804, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(47816, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(47830, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(915, 15),IR(47853, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(930, 33),IR(47873, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(963, 19),IR(47892, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(45010, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(982, 4),IR(45024, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(986, 3),IR(45097, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(989, 3),IR(45146, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(992, 2),IR(45201, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(45220, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(45235, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(747, 6),IR(45251, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(994, 3),IR(45264, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(997, 9),IR(45282, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1006, 10),IR(45334, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1016, 10),IR(45353, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1026, 5),IR(45369, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(45390, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(45411, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(786, 6),IR(45429, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(792, 5),IR(45456, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(797, 5),IR(45491, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1016, 10),IR(45522, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1031, 8),IR(45542, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1039, 11),IR(45567, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(821, 5),IR(45581, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(826, 3),IR(45599, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(829, 2),IR(45630, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(45641, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(831, 3),IR(45654, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1050, 8),IR(45711, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(45730, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(45751, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(911, 4),IR(45764, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1058, 2),IR(45808, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(695, 1),IR(45823, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(843, 4),IR(45839, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(826, 3),IR(45853, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1060, 7),IR(47608, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(829, 2),IR(47628, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1067, 8),IR(48213, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(48230, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(48235, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(48241, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(48248, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(48253, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(48259, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(48266, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(48271, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(48277, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(48283, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(48290, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(48298, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(48303, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(48308, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(48317, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(48321, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(48326, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(48332, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(48337, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(48341, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(48345, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(48350, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(48356, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(48362, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(48367, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(48372, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(48378, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(48382, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(48387, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(48392, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(48397, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(48403, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(48409, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(48415, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(48421, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(48427, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(48434, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(48443, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(48447, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(48452, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(48458, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(48464, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(48469, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(48473, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(48480, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(48487, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(48492, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(48496, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(48506, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(48510, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(48515, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(48520, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(48530, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(48537, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(48542, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(48548, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(48554, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(48560, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(48564, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(48571, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(48576, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(48582, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(48587, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(48591, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(48596, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(48602, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(48609, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(48615, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(48624, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(48636, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(48645, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(48655, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(48666, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(48675, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(48685, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(48696, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(48706, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(48717, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(48728, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(48737, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(48747, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(48756, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(48767, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(48781, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(48792, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(48804, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(48817, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(48828, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(48840, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(48853, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(48865, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(48878, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(48891, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(48902, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(48914, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(48925, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(48932, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(48940, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(48952, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(48964, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(48968, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(48976, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(48981, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(48992, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(48997, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(49003, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(49012, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(49019, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(49029, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(49043, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(49055, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(49070, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(49076, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(49087, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(49097, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(49107, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(49114, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(49121, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(49129, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(49137, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(49145, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(49153, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(49157, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(49161, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(49170, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(49180, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(49190, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(49196, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(49202, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(49208, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(49214, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(49223, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(49230, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(49240, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(49250, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(49261, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(49272, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(49281, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(49289, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(49297, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(49305, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1075, 3),IR(49313, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(49320, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(49328, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1075, 3),IR(49339, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(49351, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1078, 4),IR(49363, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(49377, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1078, 4),IR(49390, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1075, 3),IR(49405, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(49418, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1078, 4),IR(49432, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(49448, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(49456, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(49465, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(49472, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(49481, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(49491, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(49498, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(49504, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(49515, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(49524, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(49533, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(49550, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(49567, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(49575, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(49597, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(49625, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(49644, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(49653, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(49676, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(49685, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(49694, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(49703, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(49712, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(49721, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(49730, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(49739, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(49748, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(49757, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(495, 7),IR(49764, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(703, 10),IR(49775, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(703, 10),IR(49782, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(703, 10),IR(49789, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(49806, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(49812, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(623, 5),IR(49819, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(49830, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(713, 9),IR(49837, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(49854, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(49860, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(49864, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(49872, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(49877, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(49881, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(49904, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(49916, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(49927, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(49939, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(49946, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(49958, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(49977, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(49997, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(50008, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(50020, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(50028, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(50036, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(50040, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(50044, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(50048, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(50054, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(50058, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(50070, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(50075, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(50079, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(50083, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(50087, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(50093, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(50097, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(50109, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(50116, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(50127, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(50133, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(50137, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(50144, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(50150, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(50158, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(50172, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(50183, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(50194, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(50205, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(50216, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(50231, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(50242, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(50249, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(50253, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(50261, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(50269, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(50275, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(50280, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(50290, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(50297, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(623, 5),IR(50305, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(50312, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(703, 10),IR(50317, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(425, 66);
    default: break;
  }
  return IR(0,0);
}

// Extended instruction opcode names sorted by extended instruction kind, then opcode name.
// The fields in order are:
//   name
//   index into kExtInstByValue
static const std::array<NameIndex, 491> kExtInstNames{{
    {IR(45024, 21), 1}, // DebugCompilationUnit in debuginfo
    {IR(45751, 13), 28}, // DebugDeclare in debuginfo
    {IR(45823, 16), 31}, // DebugExpression in debuginfo
    {IR(45567, 14), 20}, // DebugFunction in debuginfo
    {IR(45542, 25), 19}, // DebugFunctionDeclaration in debuginfo
    {IR(45522, 20), 18}, // DebugGlobalVariable in debuginfo
    {IR(45010, 14), 0}, // DebugInfoNone in debuginfo
    {IR(45654, 15), 25}, // DebugInlinedAt in debuginfo
    {IR(45730, 21), 27}, // DebugInlinedVariable in debuginfo
    {IR(45581, 18), 21}, // DebugLexicalBlock in debuginfo
    {IR(45599, 31), 22}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(45711, 19), 26}, // DebugLocalVariable in debuginfo
    {IR(45839, 14), 32}, // DebugMacroDef in debuginfo
    {IR(45853, 16), 33}, // DebugMacroUndef in debuginfo
    {IR(45641, 13), 24}, // DebugNoScope in debuginfo
    {IR(45808, 15), 30}, // DebugOperation in debuginfo
    {IR(45630, 11), 23}, // DebugScope in debuginfo
    {IR(45220, 15), 5}, // DebugTypeArray in debuginfo
    {IR(45097, 15), 2}, // DebugTypeBasic in debuginfo
    {IR(45334, 19), 10}, // DebugTypeComposite in debuginfo
    {IR(45282, 14), 9}, // DebugTypeEnum in debuginfo
    {IR(45264, 18), 8}, // DebugTypeFunction in debuginfo
    {IR(45369, 21), 12}, // DebugTypeInheritance in debuginfo
    {IR(45353, 16), 11}, // DebugTypeMember in debuginfo
    {IR(45146, 17), 3}, // DebugTypePointer in debuginfo
    {IR(45390, 21), 13}, // DebugTypePtrToMember in debuginfo
    {IR(45201, 19), 4}, // DebugTypeQualifier in debuginfo
    {IR(45411, 18), 14}, // DebugTypeTemplate in debuginfo
    {IR(45429, 27), 15}, // DebugTypeTemplateParameter in debuginfo
    {IR(45491, 31), 17}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(45456, 35), 16}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(45235, 16), 6}, // DebugTypeVector in debuginfo
    {IR(45251, 13), 7}, // DebugTypedef in debuginfo
    {IR(45764, 11), 29}, // DebugValue in debuginfo
    {IR(45963, 5), 50}, // Acos in glsl_std_450
    {IR(45994, 6), 56}, // Acosh in glsl_std_450
    {IR(45958, 5), 49}, // Asin in glsl_std_450
    {IR(45988, 6), 55}, // Asinh in glsl_std_450
    {IR(45968, 5), 51}, // Atan in glsl_std_450
    {IR(46006, 6), 58}, // Atan2 in glsl_std_450
    {IR(46000, 6), 57}, // Atanh in glsl_std_450
    {IR(45919, 5), 42}, // Ceil in glsl_std_450
    {IR(45950, 4), 47}, // Cos in glsl_std_450
    {IR(45978, 5), 53}, // Cosh in glsl_std_450
    {IR(46390, 6), 101}, // Cross in glsl_std_450
    {IR(45938, 8), 45}, // Degrees in glsl_std_450
    {IR(46051, 12), 66}, // Determinant in glsl_std_450
    {IR(46381, 9), 100}, // Distance in glsl_std_450
    {IR(46016, 4), 60}, // Exp in glsl_std_450
    {IR(46024, 5), 62}, // Exp2 in glsl_std_450
    {IR(45891, 5), 37}, // FAbs in glsl_std_450
    {IR(46123, 7), 76}, // FClamp in glsl_std_450
    {IR(46108, 5), 73}, // FMax in glsl_std_450
    {IR(46093, 5), 70}, // FMin in glsl_std_450
    {IR(46144, 5), 79}, // FMix in glsl_std_450
    {IR(45901, 6), 39}, // FSign in glsl_std_450
    {IR(46406, 12), 103}, // FaceForward in glsl_std_450
    {IR(46434, 9), 106}, // FindILsb in glsl_std_450
    {IR(46443, 9), 107}, // FindSMsb in glsl_std_450
    {IR(46452, 9), 108}, // FindUMsb in glsl_std_450
    {IR(45913, 6), 41}, // Floor in glsl_std_450
    {IR(46170, 4), 83}, // Fma in glsl_std_450
    {IR(45924, 6), 43}, // Fract in glsl_std_450
    {IR(46174, 6), 84}, // Frexp in glsl_std_450
    {IR(46180, 12), 85}, // FrexpStruct in glsl_std_450
    {IR(46149, 5), 80}, // IMix in glsl_std_450
    {IR(46461, 22), 109}, // InterpolateAtCentroid in glsl_std_450
    {IR(46503, 20), 111}, // InterpolateAtOffset in glsl_std_450
    {IR(46483, 20), 110}, // InterpolateAtSample in glsl_std_450
    {IR(46039, 12), 65}, // InverseSqrt in glsl_std_450
    {IR(46192, 6), 86}, // Ldexp in glsl_std_450
    {IR(46374, 7), 99}, // Length in glsl_std_450
    {IR(46020, 4), 61}, // Log in glsl_std_450
    {IR(46029, 5), 63}, // Log2 in glsl_std_450
    {IR(46063, 14), 67}, // MatrixInverse in glsl_std_450
    {IR(46077, 5), 68}, // Modf in glsl_std_450
    {IR(46082, 11), 69}, // ModfStruct in glsl_std_450
    {IR(46533, 7), 114}, // NClamp in glsl_std_450
    {IR(46528, 5), 113}, // NMax in glsl_std_450
    {IR(46523, 5), 112}, // NMin in glsl_std_450
    {IR(46396, 10), 102}, // Normalize in glsl_std_450
    {IR(46265, 15), 92}, // PackDouble2x32 in glsl_std_450
    {IR(46252, 13), 91}, // PackHalf2x16 in glsl_std_450
    {IR(46224, 14), 89}, // PackSnorm2x16 in glsl_std_450
    {IR(46198, 13), 87}, // PackSnorm4x8 in glsl_std_450
    {IR(46238, 14), 90}, // PackUnorm2x16 in glsl_std_450
    {IR(46211, 13), 88}, // PackUnorm4x8 in glsl_std_450
    {IR(46012, 4), 59}, // Pow in glsl_std_450
    {IR(45930, 8), 44}, // Radians in glsl_std_450
    {IR(46418, 8), 104}, // Reflect in glsl_std_450
    {IR(46426, 8), 105}, // Refract in glsl_std_450
    {IR(45869, 6), 34}, // Round in glsl_std_450
    {IR(45875, 10), 35}, // RoundEven in glsl_std_450
    {IR(45896, 5), 38}, // SAbs in glsl_std_450
    {IR(46137, 7), 78}, // SClamp in glsl_std_450
    {IR(46118, 5), 75}, // SMax in glsl_std_450
    {IR(46103, 5), 72}, // SMin in glsl_std_450
    {IR(45907, 6), 40}, // SSign in glsl_std_450
    {IR(45946, 4), 46}, // Sin in glsl_std_450
    {IR(45973, 5), 52}, // Sinh in glsl_std_450
    {IR(46159, 11), 82}, // SmoothStep in glsl_std_450
    {IR(46034, 5), 64}, // Sqrt in glsl_std_450
    {IR(46154, 5), 81}, // Step in glsl_std_450
    {IR(45954, 4), 48}, // Tan in glsl_std_450
    {IR(45983, 5), 54}, // Tanh in glsl_std_450
    {IR(45885, 6), 36}, // Trunc in glsl_std_450
    {IR(46130, 7), 77}, // UClamp in glsl_std_450
    {IR(46113, 5), 74}, // UMax in glsl_std_450
    {IR(46098, 5), 71}, // UMin in glsl_std_450
    {IR(46357, 17), 98}, // UnpackDouble2x32 in glsl_std_450
    {IR(46312, 15), 95}, // UnpackHalf2x16 in glsl_std_450
    {IR(46280, 16), 93}, // UnpackSnorm2x16 in glsl_std_450
    {IR(46327, 15), 96}, // UnpackSnorm4x8 in glsl_std_450
    {IR(46296, 16), 94}, // UnpackUnorm2x16 in glsl_std_450
    {IR(46342, 15), 97}, // UnpackUnorm4x8 in glsl_std_450
    {IR(46540, 13), 116}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(46635, 24), 121}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(46591, 25), 119}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(46616, 19), 120}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(47069, 28), 140}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(47097, 23), 141}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(46659, 21), 122}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(46701, 16), 124}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(46553, 22), 117}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(46680, 21), 123}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(47357, 27), 148}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(46575, 16), 118}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(47384, 27), 149}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(46717, 18), 125}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(47411, 32), 150}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(46965, 26), 135}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(46991, 20), 136}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(47235, 45), 145}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(47317, 40), 147}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(47193, 42), 144}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(47280, 37), 146}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(4563, 7), 115}, // Kernel in nonsemantic_clspvreflection
    {IR(7988, 15), 137}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(47552, 34), 155}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(47520, 32), 154}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(47494, 26), 153}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(47483, 11), 152}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(47443, 40), 151}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(47155, 38), 143}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(47120, 35), 142}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(47011, 30), 138}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(46831, 30), 130}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(46806, 25), 129}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(46861, 23), 131}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(46909, 26), 133}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(46935, 30), 134}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(46884, 25), 132}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(46761, 25), 127}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(47041, 28), 139}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(46786, 20), 128}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(46735, 26), 126}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(47586, 22), 156}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(47707, 21), 197}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(45024, 21), 158}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(45751, 13), 185}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(47745, 16), 199}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(45823, 16), 188}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(45567, 14), 177}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(45542, 25), 176}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(47640, 24), 193}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(45522, 20), 175}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(47608, 20), 191}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(45010, 14), 157}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(45654, 15), 182}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(45730, 21), 184}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(45581, 18), 178}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(45599, 31), 179}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(47685, 10), 195}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(45711, 19), 183}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(45839, 14), 189}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(45853, 16), 190}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(47695, 12), 196}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(45641, 13), 181}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(45808, 15), 187}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(45630, 11), 180}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(47628, 12), 192}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(47664, 21), 194}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(47728, 17), 198}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(45220, 15), 162}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(45097, 15), 159}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(45334, 19), 167}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(45282, 14), 166}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(45264, 18), 165}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(45369, 21), 169}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(47761, 16), 200}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(45353, 16), 168}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(45146, 17), 160}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(45390, 21), 170}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(45201, 19), 161}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(45411, 18), 171}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(45429, 27), 172}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45491, 31), 174}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(45456, 35), 173}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45235, 16), 163}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(45251, 13), 164}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(45764, 11), 186}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(47777, 14), 201}, // Configuration in nonsemantic_vkspreflection
    {IR(47853, 20), 206}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(47873, 19), 207}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(47892, 21), 208}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(47816, 14), 204}, // PushConstants in nonsemantic_vkspreflection
    {IR(47830, 23), 205}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(47791, 13), 202}, // StartCounter in nonsemantic_vkspreflection
    {IR(47804, 12), 203}, // StopCounter in nonsemantic_vkspreflection
    {IR(45024, 21), 210}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(45751, 13), 237}, // DebugDeclare in opencl_debuginfo_100
    {IR(45823, 16), 240}, // DebugExpression in opencl_debuginfo_100
    {IR(45567, 14), 229}, // DebugFunction in opencl_debuginfo_100
    {IR(45542, 25), 228}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(45522, 20), 227}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(47608, 20), 243}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(45010, 14), 209}, // DebugInfoNone in opencl_debuginfo_100
    {IR(45654, 15), 234}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(45730, 21), 236}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(45581, 18), 230}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(45599, 31), 231}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(45711, 19), 235}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(45839, 14), 241}, // DebugMacroDef in opencl_debuginfo_100
    {IR(45853, 16), 242}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(48213, 17), 245}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(45641, 13), 233}, // DebugNoScope in opencl_debuginfo_100
    {IR(45808, 15), 239}, // DebugOperation in opencl_debuginfo_100
    {IR(45630, 11), 232}, // DebugScope in opencl_debuginfo_100
    {IR(47628, 12), 244}, // DebugSource in opencl_debuginfo_100
    {IR(45220, 15), 214}, // DebugTypeArray in opencl_debuginfo_100
    {IR(45097, 15), 211}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(45334, 19), 219}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(45282, 14), 218}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(45264, 18), 217}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(45369, 21), 221}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(45353, 16), 220}, // DebugTypeMember in opencl_debuginfo_100
    {IR(45146, 17), 212}, // DebugTypePointer in opencl_debuginfo_100
    {IR(45390, 21), 222}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(45201, 19), 213}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(45411, 18), 223}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(45429, 27), 224}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(45491, 31), 226}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(45456, 35), 225}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(45235, 16), 215}, // DebugTypeVector in opencl_debuginfo_100
    {IR(45251, 13), 216}, // DebugTypedef in opencl_debuginfo_100
    {IR(45764, 11), 238}, // DebugValue in opencl_debuginfo_100
    {IR(48230, 5), 246}, // acos in opencl_std_100
    {IR(48235, 6), 247}, // acosh in opencl_std_100
    {IR(48241, 7), 248}, // acospi in opencl_std_100
    {IR(48248, 5), 249}, // asin in opencl_std_100
    {IR(48253, 6), 250}, // asinh in opencl_std_100
    {IR(48259, 7), 251}, // asinpi in opencl_std_100
    {IR(48266, 5), 252}, // atan in opencl_std_100
    {IR(48271, 6), 253}, // atan2 in opencl_std_100
    {IR(48290, 8), 256}, // atan2pi in opencl_std_100
    {IR(48277, 6), 254}, // atanh in opencl_std_100
    {IR(48283, 7), 255}, // atanpi in opencl_std_100
    {IR(49481, 10), 402}, // bitselect in opencl_std_100
    {IR(48298, 5), 257}, // cbrt in opencl_std_100
    {IR(48303, 5), 258}, // ceil in opencl_std_100
    {IR(49153, 4), 367}, // clz in opencl_std_100
    {IR(48308, 9), 259}, // copysign in opencl_std_100
    {IR(48317, 4), 260}, // cos in opencl_std_100
    {IR(48321, 5), 261}, // cosh in opencl_std_100
    {IR(48326, 6), 262}, // cospi in opencl_std_100
    {IR(48997, 6), 350}, // cross in opencl_std_100
    {IR(49157, 4), 368}, // ctz in opencl_std_100
    {IR(48932, 8), 342}, // degrees in opencl_std_100
    {IR(49003, 9), 351}, // distance in opencl_std_100
    {IR(48337, 4), 264}, // erf in opencl_std_100
    {IR(48332, 5), 263}, // erfc in opencl_std_100
    {IR(48341, 4), 265}, // exp in opencl_std_100
    {IR(48350, 6), 267}, // exp10 in opencl_std_100
    {IR(48345, 5), 266}, // exp2 in opencl_std_100
    {IR(48356, 6), 268}, // expm1 in opencl_std_100
    {IR(48362, 5), 269}, // fabs in opencl_std_100
    {IR(49029, 14), 354}, // fast_distance in opencl_std_100
    {IR(49043, 12), 355}, // fast_length in opencl_std_100
    {IR(49055, 15), 356}, // fast_normalize in opencl_std_100
    {IR(48925, 7), 341}, // fclamp in opencl_std_100
    {IR(48367, 5), 270}, // fdim in opencl_std_100
    {IR(48372, 6), 271}, // floor in opencl_std_100
    {IR(48378, 4), 272}, // fma in opencl_std_100
    {IR(48382, 5), 273}, // fmax in opencl_std_100
    {IR(48940, 12), 343}, // fmax_common in opencl_std_100
    {IR(48387, 5), 274}, // fmin in opencl_std_100
    {IR(48952, 12), 344}, // fmin_common in opencl_std_100
    {IR(48392, 5), 275}, // fmod in opencl_std_100
    {IR(48397, 6), 276}, // fract in opencl_std_100
    {IR(48403, 6), 277}, // frexp in opencl_std_100
    {IR(48615, 9), 313}, // half_cos in opencl_std_100
    {IR(48624, 12), 314}, // half_divide in opencl_std_100
    {IR(48636, 9), 315}, // half_exp in opencl_std_100
    {IR(48655, 11), 317}, // half_exp10 in opencl_std_100
    {IR(48645, 10), 316}, // half_exp2 in opencl_std_100
    {IR(48666, 9), 318}, // half_log in opencl_std_100
    {IR(48685, 11), 320}, // half_log10 in opencl_std_100
    {IR(48675, 10), 319}, // half_log2 in opencl_std_100
    {IR(48696, 10), 321}, // half_powr in opencl_std_100
    {IR(48706, 11), 322}, // half_recip in opencl_std_100
    {IR(48717, 11), 323}, // half_rsqrt in opencl_std_100
    {IR(48728, 9), 324}, // half_sin in opencl_std_100
    {IR(48737, 10), 325}, // half_sqrt in opencl_std_100
    {IR(48747, 9), 326}, // half_tan in opencl_std_100
    {IR(48409, 6), 278}, // hypot in opencl_std_100
    {IR(48415, 6), 279}, // ilogb in opencl_std_100
    {IR(48421, 6), 280}, // ldexp in opencl_std_100
    {IR(49012, 7), 352}, // length in opencl_std_100
    {IR(48427, 7), 281}, // lgamma in opencl_std_100
    {IR(48434, 9), 282}, // lgamma_r in opencl_std_100
    {IR(48443, 4), 283}, // log in opencl_std_100
    {IR(48452, 6), 285}, // log10 in opencl_std_100
    {IR(48458, 6), 286}, // log1p in opencl_std_100
    {IR(48447, 5), 284}, // log2 in opencl_std_100
    {IR(48464, 5), 287}, // logb in opencl_std_100
    {IR(48469, 4), 288}, // mad in opencl_std_100
    {IR(48473, 7), 289}, // maxmag in opencl_std_100
    {IR(48480, 7), 290}, // minmag in opencl_std_100
    {IR(48964, 4), 345}, // mix in opencl_std_100
    {IR(48487, 5), 291}, // modf in opencl_std_100
    {IR(48492, 4), 292}, // nan in opencl_std_100
    {IR(48756, 11), 327}, // native_cos in opencl_std_100
    {IR(48767, 14), 328}, // native_divide in opencl_std_100
    {IR(48781, 11), 329}, // native_exp in opencl_std_100
    {IR(48804, 13), 331}, // native_exp10 in opencl_std_100
    {IR(48792, 12), 330}, // native_exp2 in opencl_std_100
    {IR(48817, 11), 332}, // native_log in opencl_std_100
    {IR(48840, 13), 334}, // native_log10 in opencl_std_100
    {IR(48828, 12), 333}, // native_log2 in opencl_std_100
    {IR(48853, 12), 335}, // native_powr in opencl_std_100
    {IR(48865, 13), 336}, // native_recip in opencl_std_100
    {IR(48878, 13), 337}, // native_rsqrt in opencl_std_100
    {IR(48891, 11), 338}, // native_sin in opencl_std_100
    {IR(48902, 12), 339}, // native_sqrt in opencl_std_100
    {IR(48914, 11), 340}, // native_tan in opencl_std_100
    {IR(48496, 10), 293}, // nextafter in opencl_std_100
    {IR(49019, 10), 353}, // normalize in opencl_std_100
    {IR(49272, 9), 382}, // popcount in opencl_std_100
    {IR(48506, 4), 294}, // pow in opencl_std_100
    {IR(48510, 5), 295}, // pown in opencl_std_100
    {IR(48515, 5), 296}, // powr in opencl_std_100
    {IR(49472, 9), 401}, // prefetch in opencl_std_100
    {IR(49465, 7), 400}, // printf in opencl_std_100
    {IR(48968, 8), 346}, // radians in opencl_std_100
    {IR(48520, 10), 297}, // remainder in opencl_std_100
    {IR(48530, 7), 298}, // remquo in opencl_std_100
    {IR(48537, 5), 299}, // rint in opencl_std_100
    {IR(48542, 6), 300}, // rootn in opencl_std_100
    {IR(49223, 7), 377}, // rotate in opencl_std_100
    {IR(48548, 6), 301}, // round in opencl_std_100
    {IR(48554, 6), 302}, // rsqrt in opencl_std_100
    {IR(49070, 6), 357}, // s_abs in opencl_std_100
    {IR(49076, 11), 358}, // s_abs_diff in opencl_std_100
    {IR(49087, 10), 359}, // s_add_sat in opencl_std_100
    {IR(49137, 8), 365}, // s_clamp in opencl_std_100
    {IR(49107, 7), 361}, // s_hadd in opencl_std_100
    {IR(49281, 8), 383}, // s_mad24 in opencl_std_100
    {IR(49161, 9), 369}, // s_mad_hi in opencl_std_100
    {IR(49180, 10), 371}, // s_mad_sat in opencl_std_100
    {IR(49190, 6), 372}, // s_max in opencl_std_100
    {IR(49202, 6), 374}, // s_min in opencl_std_100
    {IR(49297, 8), 385}, // s_mul24 in opencl_std_100
    {IR(49214, 9), 376}, // s_mul_hi in opencl_std_100
    {IR(49121, 8), 363}, // s_rhadd in opencl_std_100
    {IR(49230, 10), 378}, // s_sub_sat in opencl_std_100
    {IR(49261, 11), 381}, // s_upsample in opencl_std_100
    {IR(49491, 7), 403}, // select in opencl_std_100
    {IR(49448, 8), 398}, // shuffle in opencl_std_100
    {IR(49456, 9), 399}, // shuffle2 in opencl_std_100
    {IR(48992, 5), 349}, // sign in opencl_std_100
    {IR(48560, 4), 303}, // sin in opencl_std_100
    {IR(48564, 7), 304}, // sincos in opencl_std_100
    {IR(48571, 5), 305}, // sinh in opencl_std_100
    {IR(48576, 6), 306}, // sinpi in opencl_std_100
    {IR(48981, 11), 348}, // smoothstep in opencl_std_100
    {IR(48582, 5), 307}, // sqrt in opencl_std_100
    {IR(48976, 5), 347}, // step in opencl_std_100
    {IR(48587, 4), 308}, // tan in opencl_std_100
    {IR(48591, 5), 309}, // tanh in opencl_std_100
    {IR(48596, 6), 310}, // tanpi in opencl_std_100
    {IR(48602, 7), 311}, // tgamma in opencl_std_100
    {IR(48609, 6), 312}, // trunc in opencl_std_100
    {IR(49498, 6), 404}, // u_abs in opencl_std_100
    {IR(49504, 11), 405}, // u_abs_diff in opencl_std_100
    {IR(49097, 10), 360}, // u_add_sat in opencl_std_100
    {IR(49145, 8), 366}, // u_clamp in opencl_std_100
    {IR(49114, 7), 362}, // u_hadd in opencl_std_100
    {IR(49289, 8), 384}, // u_mad24 in opencl_std_100
    {IR(49524, 9), 407}, // u_mad_hi in opencl_std_100
    {IR(49170, 10), 370}, // u_mad_sat in opencl_std_100
    {IR(49196, 6), 373}, // u_max in opencl_std_100
    {IR(49208, 6), 375}, // u_min in opencl_std_100
    {IR(49305, 8), 386}, // u_mul24 in opencl_std_100
    {IR(49515, 9), 406}, // u_mul_hi in opencl_std_100
    {IR(49129, 8), 364}, // u_rhadd in opencl_std_100
    {IR(49240, 10), 379}, // u_sub_sat in opencl_std_100
    {IR(49250, 11), 380}, // u_upsample in opencl_std_100
    {IR(49328, 11), 389}, // vload_half in opencl_std_100
    {IR(49339, 12), 390}, // vload_halfn in opencl_std_100
    {IR(49405, 13), 395}, // vloada_halfn in opencl_std_100
    {IR(49313, 7), 387}, // vloadn in opencl_std_100
    {IR(49351, 12), 391}, // vstore_half in opencl_std_100
    {IR(49363, 14), 392}, // vstore_half_r in opencl_std_100
    {IR(49377, 13), 393}, // vstore_halfn in opencl_std_100
    {IR(49390, 15), 394}, // vstore_halfn_r in opencl_std_100
    {IR(49418, 14), 396}, // vstorea_halfn in opencl_std_100
    {IR(49432, 16), 397}, // vstorea_halfn_r in opencl_std_100
    {IR(49320, 8), 388}, // vstoren in opencl_std_100
    {IR(49550, 17), 409}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(49533, 17), 408}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(49567, 8), 410}, // TimeAMD in spv_amd_gcn_shader
    {IR(49644, 9), 414}, // MbcntAMD in spv_amd_shader_ballot
    {IR(49575, 22), 411}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(49597, 28), 412}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(49625, 19), 413}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(49653, 23), 415}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(49703, 9), 419}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49730, 9), 422}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49676, 9), 416}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(49721, 9), 421}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49748, 9), 424}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49694, 9), 418}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(49712, 9), 420}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(49739, 9), 423}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(49685, 9), 417}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50054, 4), 456}, // ABS in tosa_001000_1
    {IR(49877, 4), 439}, // ADD in tosa_001000_1
    {IR(49757, 7), 425}, // ARGMAX in tosa_001000_1
    {IR(49881, 23), 440}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(49764, 11), 426}, // AVG_POOL2D in tosa_001000_1
    {IR(49904, 12), 441}, // BITWISE_AND in tosa_001000_1
    {IR(50058, 12), 457}, // BITWISE_NOT in tosa_001000_1
    {IR(49916, 11), 442}, // BITWISE_OR in tosa_001000_1
    {IR(49927, 12), 443}, // BITWISE_XOR in tosa_001000_1
    {IR(50312, 5), 489}, // CAST in tosa_001000_1
    {IR(50070, 5), 458}, // CEIL in tosa_001000_1
    {IR(49854, 6), 435}, // CLAMP in tosa_001000_1
    {IR(50075, 4), 459}, // CLZ in tosa_001000_1
    {IR(50242, 7), 479}, // CONCAT in tosa_001000_1
    {IR(49775, 7), 427}, // CONV2D in tosa_001000_1
    {IR(49782, 7), 428}, // CONV3D in tosa_001000_1
    {IR(50079, 4), 460}, // COS in tosa_001000_1
    {IR(49789, 17), 429}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(50144, 6), 470}, // EQUAL in tosa_001000_1
    {IR(49860, 4), 436}, // ERF in tosa_001000_1
    {IR(50083, 4), 461}, // EXP in tosa_001000_1
    {IR(49806, 6), 430}, // FFT2D in tosa_001000_1
    {IR(50087, 6), 462}, // FLOOR in tosa_001000_1
    {IR(50290, 7), 486}, // GATHER in tosa_001000_1
    {IR(50150, 8), 471}, // GREATER in tosa_001000_1
    {IR(50158, 14), 472}, // GREATER_EQUAL in tosa_001000_1
    {IR(49939, 7), 444}, // INTDIV in tosa_001000_1
    {IR(50093, 4), 463}, // LOG in tosa_001000_1
    {IR(49946, 12), 445}, // LOGICAL_AND in tosa_001000_1
    {IR(49958, 19), 446}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(50097, 12), 464}, // LOGICAL_NOT in tosa_001000_1
    {IR(49997, 11), 448}, // LOGICAL_OR in tosa_001000_1
    {IR(49977, 20), 447}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(50008, 12), 449}, // LOGICAL_XOR in tosa_001000_1
    {IR(49812, 7), 431}, // MATMUL in tosa_001000_1
    {IR(50020, 8), 450}, // MAXIMUM in tosa_001000_1
    {IR(49819, 11), 432}, // MAX_POOL2D in tosa_001000_1
    {IR(50028, 8), 451}, // MINIMUM in tosa_001000_1
    {IR(50036, 4), 452}, // MUL in tosa_001000_1
    {IR(50109, 7), 465}, // NEGATE in tosa_001000_1
    {IR(50249, 4), 480}, // PAD in tosa_001000_1
    {IR(50040, 4), 453}, // POW in tosa_001000_1
    {IR(50116, 11), 466}, // RECIPROCAL in tosa_001000_1
    {IR(50172, 11), 473}, // REDUCE_ALL in tosa_001000_1
    {IR(50183, 11), 474}, // REDUCE_ANY in tosa_001000_1
    {IR(50194, 11), 475}, // REDUCE_MAX in tosa_001000_1
    {IR(50205, 11), 476}, // REDUCE_MIN in tosa_001000_1
    {IR(50216, 15), 477}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(50231, 11), 478}, // REDUCE_SUM in tosa_001000_1
    {IR(50317, 8), 490}, // RESCALE in tosa_001000_1
    {IR(50253, 8), 481}, // RESHAPE in tosa_001000_1
    {IR(50305, 7), 488}, // RESIZE in tosa_001000_1
    {IR(50261, 8), 482}, // REVERSE in tosa_001000_1
    {IR(49830, 7), 433}, // RFFT2D in tosa_001000_1
    {IR(50127, 6), 467}, // RSQRT in tosa_001000_1
    {IR(50297, 8), 487}, // SCATTER in tosa_001000_1
    {IR(50137, 7), 469}, // SELECT in tosa_001000_1
    {IR(49864, 8), 437}, // SIGMOID in tosa_001000_1
    {IR(50133, 4), 468}, // SIN in tosa_001000_1
    {IR(50269, 6), 483}, // SLICE in tosa_001000_1
    {IR(50044, 4), 454}, // SUB in tosa_001000_1
    {IR(50048, 6), 455}, // TABLE in tosa_001000_1
    {IR(49872, 5), 438}, // TANH in tosa_001000_1
    {IR(50275, 5), 484}, // TILE in tosa_001000_1
    {IR(50280, 10), 485}, // TRANSPOSE in tosa_001000_1
    {IR(49837, 17), 434}, // TRANSPOSE_CONV2D in tosa_001000_1
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
    case SPV_EXT_INST_TYPE_TOSA_001000_1: return IR(425, 66);
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
  "SPV_EXT_shader_atomic_float16_add\0" // IR(828, 34)
  "SPV_EXT_shader_atomic_float_add\0" // IR(862, 32)
  "SPV_EXT_shader_atomic_float_min_max\0" // IR(894, 36)
  "SPV_EXT_shader_image_int64\0" // IR(930, 27)
  "SPV_EXT_shader_stencil_export\0" // IR(957, 30)
  "SPV_EXT_shader_tile_image\0" // IR(987, 26)
  "SPV_EXT_shader_viewport_index_layer\0" // IR(1013, 36)
  "SPV_GOOGLE_decorate_string\0" // IR(1049, 27)
  "SPV_GOOGLE_hlsl_functionality1\0" // IR(1076, 31)
  "SPV_GOOGLE_user_type\0" // IR(1107, 21)
  "SPV_INTEL_2d_block_io\0" // IR(1128, 22)
  "SPV_INTEL_arbitrary_precision_fixed_point\0" // IR(1150, 42)
  "SPV_INTEL_arbitrary_precision_floating_point\0" // IR(1192, 45)
  "SPV_INTEL_arbitrary_precision_integers\0" // IR(1237, 39)
  "SPV_INTEL_bfloat16_conversion\0" // IR(1276, 30)
  "SPV_INTEL_bindless_images\0" // IR(1306, 26)
  "SPV_INTEL_blocking_pipes\0" // IR(1332, 25)
  "SPV_INTEL_cache_controls\0" // IR(1357, 25)
  "SPV_INTEL_debug_module\0" // IR(1382, 23)
  "SPV_INTEL_device_side_avc_motion_estimation\0" // IR(1405, 44)
  "SPV_INTEL_float_controls2\0" // IR(1449, 26)
  "SPV_INTEL_fp_fast_math_mode\0" // IR(1475, 28)
  "SPV_INTEL_fp_max_error\0" // IR(1503, 23)
  "SPV_INTEL_fpga_argument_interfaces\0" // IR(1526, 35)
  "SPV_INTEL_fpga_buffer_location\0" // IR(1561, 31)
  "SPV_INTEL_fpga_cluster_attributes\0" // IR(1592, 34)
  "SPV_INTEL_fpga_dsp_control\0" // IR(1626, 27)
  "SPV_INTEL_fpga_invocation_pipelining_attributes\0" // IR(1653, 48)
  "SPV_INTEL_fpga_latency_control\0" // IR(1701, 31)
  "SPV_INTEL_fpga_loop_controls\0" // IR(1732, 29)
  "SPV_INTEL_fpga_memory_accesses\0" // IR(1761, 31)
  "SPV_INTEL_fpga_memory_attributes\0" // IR(1792, 33)
  "SPV_INTEL_fpga_reg\0" // IR(1825, 19)
  "SPV_INTEL_function_pointers\0" // IR(1844, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(1872, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(1915, 38)
  "SPV_INTEL_inline_assembly\0" // IR(1953, 26)
  "SPV_INTEL_int4\0" // IR(1979, 15)
  "SPV_INTEL_io_pipes\0" // IR(1994, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2013, 28)
  "SPV_INTEL_long_composites\0" // IR(2041, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2067, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2087, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2119, 28)
  "SPV_INTEL_media_block_io\0" // IR(2147, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2172, 33)
  "SPV_INTEL_optnone\0" // IR(2205, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(2223, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(2249, 36)
  "SPV_INTEL_split_barrier\0" // IR(2285, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(2309, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(2344, 46)
  "SPV_INTEL_subgroups\0" // IR(2390, 20)
  "SPV_INTEL_task_sequence\0" // IR(2410, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(2434, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(2470, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(2505, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(2542, 30)
  "SPV_INTEL_variable_length_array\0" // IR(2572, 32)
  "SPV_INTEL_vector_compute\0" // IR(2604, 25)
  "SPV_KHR_16bit_storage\0" // IR(2629, 22)
  "SPV_KHR_8bit_storage\0" // IR(2651, 21)
  "SPV_KHR_bfloat16\0" // IR(2672, 17)
  "SPV_KHR_bit_instructions\0" // IR(2689, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(2714, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(2749, 27)
  "SPV_KHR_device_group\0" // IR(2776, 21)
  "SPV_KHR_expect_assume\0" // IR(2797, 22)
  "SPV_KHR_float_controls\0" // IR(2819, 23)
  "SPV_KHR_float_controls2\0" // IR(2842, 24)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(2866, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(2902, 30)
  "SPV_KHR_integer_dot_product\0" // IR(2932, 28)
  "SPV_KHR_linkonce_odr\0" // IR(2960, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(2981, 30)
  "SPV_KHR_multiview\0" // IR(3011, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3029, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3064, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3090, 32)
  "SPV_KHR_post_depth_coverage\0" // IR(3122, 28)
  "SPV_KHR_quad_control\0" // IR(3150, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(3171, 22)
  "SPV_KHR_ray_query\0" // IR(3193, 18)
  "SPV_KHR_ray_tracing\0" // IR(3211, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(3231, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(3266, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(3303, 34)
  "SPV_KHR_shader_ballot\0" // IR(3337, 22)
  "SPV_KHR_shader_clock\0" // IR(3359, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(3380, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(3411, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(3448, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(3472, 38)
  "SPV_KHR_subgroup_vote\0" // IR(3510, 22)
  "SPV_KHR_terminate_invocation\0" // IR(3532, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(3561, 35)
  "SPV_KHR_untyped_pointers\0" // IR(3596, 25)
  "SPV_KHR_variable_pointers\0" // IR(3621, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(3647, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(3675, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(3716, 38)
  "SPV_NV_bindless_texture\0" // IR(3754, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(3778, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(3816, 34)
  "SPV_NV_cooperative_matrix\0" // IR(3850, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(3876, 27)
  "SPV_NV_cooperative_vector\0" // IR(3903, 26)
  "SPV_NV_displacement_micromap\0" // IR(3929, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(3958, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(3993, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(4028, 28)
  "SPV_NV_mesh_shader\0" // IR(4056, 19)
  "SPV_NV_raw_access_chains\0" // IR(4075, 25)
  "SPV_NV_ray_tracing\0" // IR(4100, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(4119, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(4150, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(4187, 33)
  "SPV_NV_shader_image_footprint\0" // IR(4220, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(4250, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(4283, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(4309, 35)
  "SPV_NV_shading_rate\0" // IR(4344, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(4364, 29)
  "SPV_NV_tensor_addressing\0" // IR(4393, 25)
  "SPV_NV_viewport_array2\0" // IR(4418, 23)
  "SPV_QCOM_image_processing\0" // IR(4441, 26)
  "SPV_QCOM_image_processing2\0" // IR(4467, 27)
  "SPV_QCOM_tile_shading\0" // IR(4494, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(4516, 38)
  "ReadOnly\0" // IR(4554, 9)
  "Kernel\0" // IR(4563, 7)
  "WriteOnly\0" // IR(4570, 10)
  "ReadWrite\0" // IR(4580, 10)
  "Logical\0" // IR(4590, 8)
  "Physical32\0" // IR(4598, 11)
  "Addresses\0" // IR(4609, 10)
  "Physical64\0" // IR(4619, 11)
  "PhysicalStorageBuffer64\0" // IR(4630, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(4654, 27)
  "PhysicalStorageBufferAddresses\0" // IR(4681, 31)
  "Position\0" // IR(4712, 9)
  "Shader\0" // IR(4721, 7)
  "PointSize\0" // IR(4728, 10)
  "ClipDistance\0" // IR(4738, 13)
  "CullDistance\0" // IR(4751, 13)
  "VertexId\0" // IR(4764, 9)
  "InstanceId\0" // IR(4773, 11)
  "PrimitiveId\0" // IR(4784, 12)
  "Geometry\0" // IR(4796, 9)
  "Tessellation\0" // IR(4805, 13)
  "RayTracingNV\0" // IR(4818, 13)
  "RayTracingKHR\0" // IR(4831, 14)
  "MeshShadingNV\0" // IR(4845, 14)
  "MeshShadingEXT\0" // IR(4859, 15)
  "InvocationId\0" // IR(4874, 13)
  "Layer\0" // IR(4887, 6)
  "ShaderLayer\0" // IR(4893, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(4905, 28)
  "ViewportIndex\0" // IR(4933, 14)
  "MultiViewport\0" // IR(4947, 14)
  "ShaderViewportIndex\0" // IR(4961, 20)
  "TessLevelOuter\0" // IR(4981, 15)
  "TessLevelInner\0" // IR(4996, 15)
  "TessCoord\0" // IR(5011, 10)
  "PatchVertices\0" // IR(5021, 14)
  "FragCoord\0" // IR(5035, 10)
  "PointCoord\0" // IR(5045, 11)
  "FrontFacing\0" // IR(5056, 12)
  "SampleId\0" // IR(5068, 9)
  "SampleRateShading\0" // IR(5077, 18)
  "SamplePosition\0" // IR(5095, 15)
  "SampleMask\0" // IR(5110, 11)
  "FragDepth\0" // IR(5121, 10)
  "HelperInvocation\0" // IR(5131, 17)
  "NumWorkgroups\0" // IR(5148, 14)
  "WorkgroupSize\0" // IR(5162, 14)
  "WorkgroupId\0" // IR(5176, 12)
  "LocalInvocationId\0" // IR(5188, 18)
  "GlobalInvocationId\0" // IR(5206, 19)
  "LocalInvocationIndex\0" // IR(5225, 21)
  "WorkDim\0" // IR(5246, 8)
  "GlobalSize\0" // IR(5254, 11)
  "EnqueuedWorkgroupSize\0" // IR(5265, 22)
  "GlobalOffset\0" // IR(5287, 13)
  "GlobalLinearId\0" // IR(5300, 15)
  "SubgroupSize\0" // IR(5315, 13)
  "GroupNonUniform\0" // IR(5328, 16)
  "SubgroupBallotKHR\0" // IR(5344, 18)
  "SubgroupMaxSize\0" // IR(5362, 16)
  "NumSubgroups\0" // IR(5378, 13)
  "NumEnqueuedSubgroups\0" // IR(5391, 21)
  "SubgroupId\0" // IR(5412, 11)
  "SubgroupLocalInvocationId\0" // IR(5423, 26)
  "VertexIndex\0" // IR(5449, 12)
  "InstanceIndex\0" // IR(5461, 14)
  "CoreIDARM\0" // IR(5475, 10)
  "CoreBuiltinsARM\0" // IR(5485, 16)
  "CoreCountARM\0" // IR(5501, 13)
  "CoreMaxIDARM\0" // IR(5514, 13)
  "WarpIDARM\0" // IR(5527, 10)
  "WarpMaxIDARM\0" // IR(5537, 13)
  "SubgroupEqMask\0" // IR(5550, 15)
  "SubgroupEqMaskKHR\0" // IR(5565, 18)
  "GroupNonUniformBallot\0" // IR(5583, 22)
  "SubgroupGeMask\0" // IR(5605, 15)
  "SubgroupGeMaskKHR\0" // IR(5620, 18)
  "SubgroupGtMask\0" // IR(5638, 15)
  "SubgroupGtMaskKHR\0" // IR(5653, 18)
  "SubgroupLeMask\0" // IR(5671, 15)
  "SubgroupLeMaskKHR\0" // IR(5686, 18)
  "SubgroupLtMask\0" // IR(5704, 15)
  "SubgroupLtMaskKHR\0" // IR(5719, 18)
  "BaseVertex\0" // IR(5737, 11)
  "DrawParameters\0" // IR(5748, 15)
  "BaseInstance\0" // IR(5763, 13)
  "DrawIndex\0" // IR(5776, 10)
  "PrimitiveShadingRateKHR\0" // IR(5786, 24)
  "FragmentShadingRateKHR\0" // IR(5810, 23)
  "DeviceIndex\0" // IR(5833, 12)
  "DeviceGroup\0" // IR(5845, 12)
  "ViewIndex\0" // IR(5857, 10)
  "MultiView\0" // IR(5867, 10)
  "ShadingRateKHR\0" // IR(5877, 15)
  "TileOffsetQCOM\0" // IR(5892, 15)
  "TileShadingQCOM\0" // IR(5907, 16)
  "TileDimensionQCOM\0" // IR(5923, 18)
  "TileApronSizeQCOM\0" // IR(5941, 18)
  "BaryCoordNoPerspAMD\0" // IR(5959, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(5979, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(6007, 26)
  "BaryCoordSmoothAMD\0" // IR(6033, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(6052, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(6079, 25)
  "BaryCoordPullModelAMD\0" // IR(6104, 22)
  "FragStencilRefEXT\0" // IR(6126, 18)
  "StencilExportEXT\0" // IR(6144, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(6161, 29)
  "ShaderEnqueueAMDX\0" // IR(6190, 18)
  "ShaderIndexAMDX\0" // IR(6208, 16)
  "ViewportMaskNV\0" // IR(6224, 15)
  "ShaderViewportMaskNV\0" // IR(6239, 21)
  "SecondaryPositionNV\0" // IR(6260, 20)
  "ShaderStereoViewNV\0" // IR(6280, 19)
  "SecondaryViewportMaskNV\0" // IR(6299, 24)
  "PositionPerViewNV\0" // IR(6323, 18)
  "PerViewAttributesNV\0" // IR(6341, 20)
  "ViewportMaskPerViewNV\0" // IR(6361, 22)
  "FullyCoveredEXT\0" // IR(6383, 16)
  "FragmentFullyCoveredEXT\0" // IR(6399, 24)
  "TaskCountNV\0" // IR(6423, 12)
  "PrimitiveCountNV\0" // IR(6435, 17)
  "PrimitiveIndicesNV\0" // IR(6452, 19)
  "ClipDistancePerViewNV\0" // IR(6471, 22)
  "CullDistancePerViewNV\0" // IR(6493, 22)
  "LayerPerViewNV\0" // IR(6515, 15)
  "MeshViewCountNV\0" // IR(6530, 16)
  "MeshViewIndicesNV\0" // IR(6546, 18)
  "BaryCoordKHR\0" // IR(6564, 13)
  "BaryCoordNV\0" // IR(6577, 12)
  "FragmentBarycentricKHR\0" // IR(6589, 23)
  "BaryCoordNoPerspKHR\0" // IR(6612, 20)
  "BaryCoordNoPerspNV\0" // IR(6632, 19)
  "FragSizeEXT\0" // IR(6651, 12)
  "FragmentSizeNV\0" // IR(6663, 15)
  "FragmentDensityEXT\0" // IR(6678, 19)
  "FragInvocationCountEXT\0" // IR(6697, 23)
  "InvocationsPerPixelNV\0" // IR(6720, 22)
  "PrimitivePointIndicesEXT\0" // IR(6742, 25)
  "PrimitiveLineIndicesEXT\0" // IR(6767, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(6791, 28)
  "CullPrimitiveEXT\0" // IR(6819, 17)
  "LaunchIdKHR\0" // IR(6836, 12)
  "LaunchIdNV\0" // IR(6848, 11)
  "LaunchSizeKHR\0" // IR(6859, 14)
  "LaunchSizeNV\0" // IR(6873, 13)
  "WorldRayOriginKHR\0" // IR(6886, 18)
  "WorldRayOriginNV\0" // IR(6904, 17)
  "WorldRayDirectionKHR\0" // IR(6921, 21)
  "WorldRayDirectionNV\0" // IR(6942, 20)
  "ObjectRayOriginKHR\0" // IR(6962, 19)
  "ObjectRayOriginNV\0" // IR(6981, 18)
  "ObjectRayDirectionKHR\0" // IR(6999, 22)
  "ObjectRayDirectionNV\0" // IR(7021, 21)
  "RayTminKHR\0" // IR(7042, 11)
  "RayTminNV\0" // IR(7053, 10)
  "RayTmaxKHR\0" // IR(7063, 11)
  "RayTmaxNV\0" // IR(7074, 10)
  "InstanceCustomIndexKHR\0" // IR(7084, 23)
  "InstanceCustomIndexNV\0" // IR(7107, 22)
  "ObjectToWorldKHR\0" // IR(7129, 17)
  "ObjectToWorldNV\0" // IR(7146, 16)
  "WorldToObjectKHR\0" // IR(7162, 17)
  "WorldToObjectNV\0" // IR(7179, 16)
  "HitTNV\0" // IR(7195, 7)
  "HitKindKHR\0" // IR(7202, 11)
  "HitKindNV\0" // IR(7213, 10)
  "CurrentRayTimeNV\0" // IR(7223, 17)
  "RayTracingMotionBlurNV\0" // IR(7240, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(7263, 30)
  "RayTracingPositionFetchKHR\0" // IR(7293, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(7320, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(7354, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(7387, 37)
  "IncomingRayFlagsKHR\0" // IR(7424, 20)
  "IncomingRayFlagsNV\0" // IR(7444, 19)
  "RayGeometryIndexKHR\0" // IR(7463, 20)
  "HitIsSphereNV\0" // IR(7483, 14)
  "RayTracingSpheresGeometryNV\0" // IR(7497, 28)
  "HitIsLSSNV\0" // IR(7525, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(7536, 39)
  "HitSpherePositionNV\0" // IR(7575, 20)
  "WarpsPerSMNV\0" // IR(7595, 13)
  "ShaderSMBuiltinsNV\0" // IR(7608, 19)
  "SMCountNV\0" // IR(7627, 10)
  "WarpIDNV\0" // IR(7637, 9)
  "SMIDNV\0" // IR(7646, 7)
  "HitLSSPositionsNV\0" // IR(7653, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(7671, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(7705, 33)
  "HitSphereRadiusNV\0" // IR(7738, 18)
  "HitLSSRadiiNV\0" // IR(7756, 14)
  "ClusterIDNV\0" // IR(7770, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(7782, 41)
  "CullMaskKHR\0" // IR(7823, 12)
  "RayCullMaskKHR\0" // IR(7835, 15)
  "Matrix\0" // IR(7850, 7)
  "Linkage\0" // IR(7857, 8)
  "Vector16\0" // IR(7865, 9)
  "Float16Buffer\0" // IR(7874, 14)
  "Float16\0" // IR(7888, 8)
  "Float64\0" // IR(7896, 8)
  "Int64\0" // IR(7904, 6)
  "Int64Atomics\0" // IR(7910, 13)
  "ImageBasic\0" // IR(7923, 11)
  "ImageReadWrite\0" // IR(7934, 15)
  "ImageMipmap\0" // IR(7949, 12)
  "Pipes\0" // IR(7961, 6)
  "Groups\0" // IR(7967, 7)
  "DeviceEnqueue\0" // IR(7974, 14)
  "LiteralSampler\0" // IR(7988, 15)
  "AtomicStorage\0" // IR(8003, 14)
  "Int16\0" // IR(8017, 6)
  "TessellationPointSize\0" // IR(8023, 22)
  "GeometryPointSize\0" // IR(8045, 18)
  "ImageGatherExtended\0" // IR(8063, 20)
  "StorageImageMultisample\0" // IR(8083, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(8107, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(8141, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(8174, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(8208, 33)
  "ImageCubeArray\0" // IR(8241, 15)
  "SampledCubeArray\0" // IR(8256, 17)
  "ImageRect\0" // IR(8273, 10)
  "SampledRect\0" // IR(8283, 12)
  "GenericPointer\0" // IR(8295, 15)
  "Int8\0" // IR(8310, 5)
  "InputAttachment\0" // IR(8315, 16)
  "SparseResidency\0" // IR(8331, 16)
  "MinLod\0" // IR(8347, 7)
  "Sampled1D\0" // IR(8354, 10)
  "Image1D\0" // IR(8364, 8)
  "SampledBuffer\0" // IR(8372, 14)
  "ImageBuffer\0" // IR(8386, 12)
  "ImageMSArray\0" // IR(8398, 13)
  "StorageImageExtendedFormats\0" // IR(8411, 28)
  "ImageQuery\0" // IR(8439, 11)
  "DerivativeControl\0" // IR(8450, 18)
  "InterpolationFunction\0" // IR(8468, 22)
  "TransformFeedback\0" // IR(8490, 18)
  "GeometryStreams\0" // IR(8508, 16)
  "StorageImageReadWithoutFormat\0" // IR(8524, 30)
  "StorageImageWriteWithoutFormat\0" // IR(8554, 31)
  "SubgroupDispatch\0" // IR(8585, 17)
  "NamedBarrier\0" // IR(8602, 13)
  "PipeStorage\0" // IR(8615, 12)
  "GroupNonUniformVote\0" // IR(8627, 20)
  "GroupNonUniformArithmetic\0" // IR(8647, 26)
  "GroupNonUniformShuffle\0" // IR(8673, 23)
  "GroupNonUniformShuffleRelative\0" // IR(8696, 31)
  "GroupNonUniformClustered\0" // IR(8727, 25)
  "GroupNonUniformQuad\0" // IR(8752, 20)
  "UniformDecoration\0" // IR(8772, 18)
  "TileImageColorReadAccessEXT\0" // IR(8790, 28)
  "TileImageDepthReadAccessEXT\0" // IR(8818, 28)
  "TileImageStencilReadAccessEXT\0" // IR(8846, 30)
  "TensorsARM\0" // IR(8876, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(8887, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(8924, 40)
  "GraphARM\0" // IR(8964, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(8973, 28)
  "Float8EXT\0" // IR(9001, 10)
  "Float8CooperativeMatrixEXT\0" // IR(9011, 27)
  "CooperativeMatrixKHR\0" // IR(9038, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(9059, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(9092, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(9135, 44)
  "SubgroupVoteKHR\0" // IR(9179, 16)
  "StorageBuffer16BitAccess\0" // IR(9195, 25)
  "StorageUniformBufferBlock16\0" // IR(9220, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(9248, 35)
  "StorageUniform16\0" // IR(9283, 17)
  "StoragePushConstant16\0" // IR(9300, 22)
  "StorageInputOutput16\0" // IR(9322, 21)
  "VariablePointersStorageBuffer\0" // IR(9343, 30)
  "VariablePointers\0" // IR(9373, 17)
  "AtomicStorageOps\0" // IR(9390, 17)
  "SampleMaskPostDepthCoverage\0" // IR(9407, 28)
  "StorageBuffer8BitAccess\0" // IR(9435, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(9459, 34)
  "StoragePushConstant8\0" // IR(9493, 21)
  "DenormPreserve\0" // IR(9514, 15)
  "DenormFlushToZero\0" // IR(9529, 18)
  "SignedZeroInfNanPreserve\0" // IR(9547, 25)
  "RoundingModeRTE\0" // IR(9572, 16)
  "RoundingModeRTZ\0" // IR(9588, 16)
  "RayQueryProvisionalKHR\0" // IR(9604, 23)
  "RayQueryKHR\0" // IR(9627, 12)
  "UntypedPointersKHR\0" // IR(9639, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(9658, 32)
  "TextureSampleWeightedQCOM\0" // IR(9690, 26)
  "TextureBoxFilterQCOM\0" // IR(9716, 21)
  "TextureBlockMatchQCOM\0" // IR(9737, 22)
  "TextureBlockMatch2QCOM\0" // IR(9759, 23)
  "Float16ImageAMD\0" // IR(9782, 16)
  "ImageGatherBiasLodAMD\0" // IR(9798, 22)
  "FragmentMaskAMD\0" // IR(9820, 16)
  "ImageReadWriteLodAMD\0" // IR(9836, 21)
  "Int64ImageEXT\0" // IR(9857, 14)
  "ShaderClockKHR\0" // IR(9871, 15)
  "QuadControlKHR\0" // IR(9886, 15)
  "Int4TypeINTEL\0" // IR(9901, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(9915, 27)
  "BFloat16TypeKHR\0" // IR(9942, 16)
  "BFloat16DotProductKHR\0" // IR(9958, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(9980, 29)
  "SampleMaskOverrideCoverageNV\0" // IR(10009, 29)
  "GeometryShaderPassthroughNV\0" // IR(10038, 28)
  "ShaderViewportIndexLayerNV\0" // IR(10066, 27)
  "ImageFootprintNV\0" // IR(10093, 17)
  "FragmentBarycentricNV\0" // IR(10110, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(10132, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(10163, 30)
  "ShadingRateNV\0" // IR(10193, 14)
  "GroupNonUniformPartitionedNV\0" // IR(10207, 29)
  "ShaderNonUniform\0" // IR(10236, 17)
  "ShaderNonUniformEXT\0" // IR(10253, 20)
  "RuntimeDescriptorArray\0" // IR(10273, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(10296, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(10322, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(10358, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(10397, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(10436, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(10478, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(10517, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(10559, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(10596, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(10636, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(10672, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(10711, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(10748, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(10788, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(10824, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(10863, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(10902, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(10944, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(10986, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(11031, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(11073, 45)
  "VulkanMemoryModel\0" // IR(11118, 18)
  "VulkanMemoryModelKHR\0" // IR(11136, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(11157, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(11186, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(11218, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(11252, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(11284, 31)
  "RayTracingProvisionalKHR\0" // IR(11315, 25)
  "CooperativeMatrixNV\0" // IR(11340, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(11360, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(11393, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(11431, 32)
  "DemoteToHelperInvocation\0" // IR(11463, 25)
  "DemoteToHelperInvocationEXT\0" // IR(11488, 28)
  "DisplacementMicromapNV\0" // IR(11516, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(11539, 29)
  "ShaderInvocationReorderNV\0" // IR(11568, 26)
  "BindlessTextureNV\0" // IR(11594, 18)
  "RayQueryPositionFetchKHR\0" // IR(11612, 25)
  "CooperativeVectorNV\0" // IR(11637, 20)
  "AtomicFloat16VectorNV\0" // IR(11657, 22)
  "RawAccessChainsNV\0" // IR(11679, 18)
  "CooperativeMatrixReductionsNV\0" // IR(11697, 30)
  "CooperativeMatrixConversionsNV\0" // IR(11727, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(11758, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(11798, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(11834, 30)
  "CooperativeVectorTrainingNV\0" // IR(11864, 28)
  "TensorAddressingNV\0" // IR(11892, 19)
  "SubgroupShuffleINTEL\0" // IR(11911, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(11932, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(11959, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(11985, 31)
  "RoundToInfinityINTEL\0" // IR(12016, 21)
  "FloatingPointModeINTEL\0" // IR(12037, 23)
  "IntegerFunctions2INTEL\0" // IR(12060, 23)
  "FunctionPointersINTEL\0" // IR(12083, 22)
  "IndirectReferencesINTEL\0" // IR(12105, 24)
  "AsmINTEL\0" // IR(12129, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(12138, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(12161, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(12184, 23)
  "VectorComputeINTEL\0" // IR(12207, 19)
  "VectorAnyINTEL\0" // IR(12226, 15)
  "ExpectAssumeKHR\0" // IR(12241, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(12257, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(12290, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(12328, 39)
  "VariableLengthArrayINTEL\0" // IR(12367, 25)
  "FunctionFloatControlINTEL\0" // IR(12392, 26)
  "FPGAMemoryAttributesINTEL\0" // IR(12418, 26)
  "FPFastMathModeINTEL\0" // IR(12444, 20)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(12464, 32)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(12496, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(12533, 30)
  "FPGALoopControlsINTEL\0" // IR(12563, 22)
  "KernelAttributesINTEL\0" // IR(12585, 22)
  "FPGAKernelAttributesINTEL\0" // IR(12607, 26)
  "FPGAMemoryAccessesINTEL\0" // IR(12633, 24)
  "FPGAClusterAttributesINTEL\0" // IR(12657, 27)
  "LoopFuseINTEL\0" // IR(12684, 14)
  "FPGADSPControlINTEL\0" // IR(12698, 20)
  "MemoryAccessAliasingINTEL\0" // IR(12718, 26)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(12744, 40)
  "FPGABufferLocationINTEL\0" // IR(12784, 24)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(12808, 34)
  "USMStorageClassesINTEL\0" // IR(12842, 23)
  "RuntimeAlignedAttributeINTEL\0" // IR(12865, 29)
  "IOPipesINTEL\0" // IR(12894, 13)
  "BlockingPipesINTEL\0" // IR(12907, 19)
  "FPGARegINTEL\0" // IR(12926, 13)
  "DotProductInputAll\0" // IR(12939, 19)
  "DotProductInputAllKHR\0" // IR(12958, 22)
  "DotProductInput4x8Bit\0" // IR(12980, 22)
  "DotProductInput4x8BitKHR\0" // IR(13002, 25)
  "DotProductInput4x8BitPacked\0" // IR(13027, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(13055, 31)
  "DotProduct\0" // IR(13086, 11)
  "DotProductKHR\0" // IR(13097, 14)
  "ReplicatedCompositesEXT\0" // IR(13111, 24)
  "BitInstructions\0" // IR(13135, 16)
  "GroupNonUniformRotateKHR\0" // IR(13151, 25)
  "FloatControls2\0" // IR(13176, 15)
  "AtomicFloat32AddEXT\0" // IR(13191, 20)
  "AtomicFloat64AddEXT\0" // IR(13211, 20)
  "LongCompositesINTEL\0" // IR(13231, 20)
  "OptNoneEXT\0" // IR(13251, 11)
  "OptNoneINTEL\0" // IR(13262, 13)
  "AtomicFloat16AddEXT\0" // IR(13275, 20)
  "DebugInfoModuleINTEL\0" // IR(13295, 21)
  "BFloat16ConversionINTEL\0" // IR(13316, 24)
  "SplitBarrierINTEL\0" // IR(13340, 18)
  "ArithmeticFenceEXT\0" // IR(13358, 19)
  "FPGAClusterAttributesV2INTEL\0" // IR(13377, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(13406, 28)
  "TaskSequenceINTEL\0" // IR(13434, 18)
  "FPMaxErrorINTEL\0" // IR(13452, 16)
  "FPGALatencyControlINTEL\0" // IR(13468, 24)
  "FPGAArgumentInterfacesINTEL\0" // IR(13492, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(13520, 30)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(13550, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(13585, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(13613, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(13636, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(13666, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(13696, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(13734, 28)
  "GroupUniformArithmeticKHR\0" // IR(13762, 26)
  "TensorFloat32RoundingINTEL\0" // IR(13788, 27)
  "MaskedGatherScatterINTEL\0" // IR(13815, 25)
  "CacheControlsINTEL\0" // IR(13840, 19)
  "RegisterLimitsINTEL\0" // IR(13859, 20)
  "BindlessImagesINTEL\0" // IR(13879, 20)
  "Unspecified\0" // IR(13899, 12)
  "Address\0" // IR(13911, 8)
  "Boolean\0" // IR(13919, 8)
  "Float\0" // IR(13927, 6)
  "Signed\0" // IR(13933, 7)
  "SignedChar\0" // IR(13940, 11)
  "Unsigned\0" // IR(13951, 9)
  "UnsignedChar\0" // IR(13960, 13)
  "Class\0" // IR(13973, 6)
  "Structure\0" // IR(13979, 10)
  "Union\0" // IR(13989, 6)
  "ImportedModule\0" // IR(13995, 15)
  "ImportedDeclaration\0" // IR(14010, 20)
  "None\0" // IR(14030, 5)
  "FlagIsProtected\0" // IR(14035, 16)
  "FlagIsPrivate\0" // IR(14051, 14)
  "FlagIsPublic\0" // IR(14065, 13)
  "FlagIsLocal\0" // IR(14078, 12)
  "FlagIsDefinition\0" // IR(14090, 17)
  "FlagFwdDecl\0" // IR(14107, 12)
  "FlagArtificial\0" // IR(14119, 15)
  "FlagExplicit\0" // IR(14134, 13)
  "FlagPrototyped\0" // IR(14147, 15)
  "FlagObjectPointer\0" // IR(14162, 18)
  "FlagStaticMember\0" // IR(14180, 17)
  "FlagIndirectVariable\0" // IR(14197, 21)
  "FlagLValueReference\0" // IR(14218, 20)
  "FlagRValueReference\0" // IR(14238, 20)
  "FlagIsOptimized\0" // IR(14258, 16)
  "FlagIsEnumClass\0" // IR(14274, 16)
  "FlagTypePassByValue\0" // IR(14290, 20)
  "FlagTypePassByReference\0" // IR(14310, 24)
  "Deref\0" // IR(14334, 6)
  "Plus\0" // IR(14340, 5)
  "Minus\0" // IR(14345, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(14351, 33)
  "PlusUconst\0" // IR(14384, 11)
  "BitPiece\0" // IR(14395, 9)
  "Swap\0" // IR(14404, 5)
  "Xderef\0" // IR(14409, 7)
  "StackValue\0" // IR(14416, 11)
  "Constu\0" // IR(14427, 7)
  "Fragment\0" // IR(14434, 9)
  "ConstType\0" // IR(14443, 10)
  "VolatileType\0" // IR(14453, 13)
  "RestrictType\0" // IR(14466, 13)
  "AtomicType\0" // IR(14479, 11)
  "Float16NV\0" // IR(14490, 10)
  "Float32NV\0" // IR(14500, 10)
  "Float64NV\0" // IR(14510, 10)
  "SignedInt8NV\0" // IR(14520, 13)
  "SignedInt16NV\0" // IR(14533, 14)
  "SignedInt32NV\0" // IR(14547, 14)
  "SignedInt64NV\0" // IR(14561, 14)
  "UnsignedInt8NV\0" // IR(14575, 15)
  "UnsignedInt16NV\0" // IR(14590, 16)
  "UnsignedInt32NV\0" // IR(14606, 16)
  "UnsignedInt64NV\0" // IR(14622, 16)
  "SignedInt8PackedNV\0" // IR(14638, 19)
  "UnsignedInt8PackedNV\0" // IR(14657, 21)
  "FloatE4M3NV\0" // IR(14678, 12)
  "FloatE5M2NV\0" // IR(14690, 12)
  "RowMajorKHR\0" // IR(14702, 12)
  "ColumnMajorKHR\0" // IR(14714, 15)
  "RowBlockedInterleavedARM\0" // IR(14729, 25)
  "ColumnBlockedInterleavedARM\0" // IR(14754, 28)
  "NoneKHR\0" // IR(14782, 8)
  "MatrixASignedComponentsKHR\0" // IR(14790, 27)
  "MatrixBSignedComponentsKHR\0" // IR(14817, 27)
  "MatrixCSignedComponentsKHR\0" // IR(14844, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(14871, 32)
  "SaturatingAccumulationKHR\0" // IR(14903, 26)
  "Row\0" // IR(14929, 4)
  "Column\0" // IR(14933, 7)
  "2x2\0" // IR(14940, 4)
  "MatrixAKHR\0" // IR(14944, 11)
  "MatrixBKHR\0" // IR(14955, 11)
  "MatrixAccumulatorKHR\0" // IR(14966, 21)
  "RowMajorNV\0" // IR(14987, 11)
  "ColumnMajorNV\0" // IR(14998, 14)
  "InferencingOptimalNV\0" // IR(15012, 21)
  "TrainingOptimalNV\0" // IR(15033, 18)
  "RelaxedPrecision\0" // IR(15051, 17)
  "SpecId\0" // IR(15068, 7)
  "Block\0" // IR(15075, 6)
  "BufferBlock\0" // IR(15081, 12)
  "RowMajor\0" // IR(15093, 9)
  "ColMajor\0" // IR(15102, 9)
  "ArrayStride\0" // IR(15111, 12)
  "MatrixStride\0" // IR(15123, 13)
  "GLSLShared\0" // IR(15136, 11)
  "GLSLPacked\0" // IR(15147, 11)
  "CPacked\0" // IR(15158, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(15166, 26)
  "BuiltIn\0" // IR(15192, 8)
  "NoPerspective\0" // IR(15200, 14)
  "Flat\0" // IR(15214, 5)
  "Patch\0" // IR(15219, 6)
  "Centroid\0" // IR(15225, 9)
  "Sample\0" // IR(15234, 7)
  "Invariant\0" // IR(15241, 10)
  "Restrict\0" // IR(15251, 9)
  "Aliased\0" // IR(15260, 8)
  "Volatile\0" // IR(15268, 9)
  "Constant\0" // IR(15277, 9)
  "Coherent\0" // IR(15286, 9)
  "NonWritable\0" // IR(15295, 12)
  "NonReadable\0" // IR(15307, 12)
  "Uniform\0" // IR(15319, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(15327, 26)
  "UniformId\0" // IR(15353, 10)
  "SaturatedConversion\0" // IR(15363, 20)
  "Stream\0" // IR(15383, 7)
  "Location\0" // IR(15390, 9)
  "Component\0" // IR(15399, 10)
  "Index\0" // IR(15409, 6)
  "Binding\0" // IR(15415, 8)
  "DescriptorSet\0" // IR(15423, 14)
  "Offset\0" // IR(15437, 7)
  "XfbBuffer\0" // IR(15444, 10)
  "XfbStride\0" // IR(15454, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(15464, 46)
  "FuncParamAttr\0" // IR(15510, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(15524, 34)
  "FPRoundingMode\0" // IR(15558, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(15573, 35)
  "FPFastMathMode\0" // IR(15608, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(15623, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(15655, 30)
  "LinkageAttributes\0" // IR(15685, 18)
  "NoContraction\0" // IR(15703, 14)
  "InputAttachmentIndex\0" // IR(15717, 21)
  "Alignment\0" // IR(15738, 10)
  "MaxByteOffset\0" // IR(15748, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(15762, 20)
  "AlignmentId\0" // IR(15782, 12)
  "MaxByteOffsetId\0" // IR(15794, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(15810, 44)
  "NoSignedWrap\0" // IR(15854, 13)
  "NoUnsignedWrap\0" // IR(15867, 15)
  "WeightTextureQCOM\0" // IR(15882, 18)
  "BlockMatchTextureQCOM\0" // IR(15900, 22)
  "BlockMatchSamplerQCOM\0" // IR(15922, 22)
  "ExplicitInterpAMD\0" // IR(15944, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(15962, 32)
  "NodeMaxPayloadsAMDX\0" // IR(15994, 20)
  "TrackFinishWritingAMDX\0" // IR(16014, 23)
  "PayloadNodeNameAMDX\0" // IR(16037, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(16057, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(16082, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(16109, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(16134, 28)
  "OverrideCoverageNV\0" // IR(16162, 19)
  "PassthroughNV\0" // IR(16181, 14)
  "ViewportRelativeNV\0" // IR(16195, 19)
  "SecondaryViewportRelativeNV\0" // IR(16214, 28)
  "PerPrimitiveEXT\0" // IR(16242, 16)
  "PerPrimitiveNV\0" // IR(16258, 15)
  "PerViewNV\0" // IR(16273, 10)
  "PerTaskNV\0" // IR(16283, 10)
  "PerVertexKHR\0" // IR(16293, 13)
  "PerVertexNV\0" // IR(16306, 12)
  "NonUniform\0" // IR(16318, 11)
  "NonUniformEXT\0" // IR(16329, 14)
  "RestrictPointer\0" // IR(16343, 16)
  "RestrictPointerEXT\0" // IR(16359, 19)
  "AliasedPointer\0" // IR(16378, 15)
  "AliasedPointerEXT\0" // IR(16393, 18)
  "HitObjectShaderRecordBufferNV\0" // IR(16411, 30)
  "BindlessSamplerNV\0" // IR(16441, 18)
  "BindlessImageNV\0" // IR(16459, 16)
  "BoundSamplerNV\0" // IR(16475, 15)
  "BoundImageNV\0" // IR(16490, 13)
  "SIMTCallINTEL\0" // IR(16503, 14)
  "ReferencedIndirectlyINTEL\0" // IR(16517, 26)
  "ClobberINTEL\0" // IR(16543, 13)
  "SideEffectsINTEL\0" // IR(16556, 17)
  "VectorComputeVariableINTEL\0" // IR(16573, 27)
  "FuncParamIOKindINTEL\0" // IR(16600, 21)
  "VectorComputeFunctionINTEL\0" // IR(16621, 27)
  "StackCallINTEL\0" // IR(16648, 15)
  "GlobalVariableOffsetINTEL\0" // IR(16663, 26)
  "CounterBuffer\0" // IR(16689, 14)
  "HlslCounterBufferGOOGLE\0" // IR(16703, 24)
  "UserSemantic\0" // IR(16727, 13)
  "HlslSemanticGOOGLE\0" // IR(16740, 19)
  "UserTypeGOOGLE\0" // IR(16759, 15)
  "FunctionRoundingModeINTEL\0" // IR(16774, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(16800, 31)
  "FunctionDenormModeINTEL\0" // IR(16831, 24)
  "RegisterINTEL\0" // IR(16855, 14)
  "MemoryINTEL\0" // IR(16869, 12)
  "NumbanksINTEL\0" // IR(16881, 14)
  "BankwidthINTEL\0" // IR(16895, 15)
  "MaxPrivateCopiesINTEL\0" // IR(16910, 22)
  "SinglepumpINTEL\0" // IR(16932, 16)
  "DoublepumpINTEL\0" // IR(16948, 16)
  "MaxReplicatesINTEL\0" // IR(16964, 19)
  "SimpleDualPortINTEL\0" // IR(16983, 20)
  "MergeINTEL\0" // IR(17003, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(17014, 42)
  "BankBitsINTEL\0" // IR(17056, 14)
  "ForcePow2DepthINTEL\0" // IR(17070, 20)
  "StridesizeINTEL\0" // IR(17090, 16)
  "WordsizeINTEL\0" // IR(17106, 14)
  "TrueDualPortINTEL\0" // IR(17120, 18)
  "BurstCoalesceINTEL\0" // IR(17138, 19)
  "CacheSizeINTEL\0" // IR(17157, 15)
  "DontStaticallyCoalesceINTEL\0" // IR(17172, 28)
  "PrefetchINTEL\0" // IR(17200, 14)
  "StallEnableINTEL\0" // IR(17214, 17)
  "FuseLoopsInFunctionINTEL\0" // IR(17231, 25)
  "MathOpDSPModeINTEL\0" // IR(17256, 19)
  "AliasScopeINTEL\0" // IR(17275, 16)
  "NoAliasINTEL\0" // IR(17291, 13)
  "InitiationIntervalINTEL\0" // IR(17304, 24)
  "MaxConcurrencyINTEL\0" // IR(17328, 20)
  "PipelineEnableINTEL\0" // IR(17348, 20)
  "BufferLocationINTEL\0" // IR(17368, 20)
  "IOPipeStorageINTEL\0" // IR(17388, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(17407, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(17441, 31)
  "SingleElementVectorINTEL\0" // IR(17472, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(17497, 35)
  "MediaBlockIOINTEL\0" // IR(17532, 18)
  "StallFreeINTEL\0" // IR(17550, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(17565, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(17596, 26)
  "LatencyControlLabelINTEL\0" // IR(17622, 25)
  "LatencyControlConstraintINTEL\0" // IR(17647, 30)
  "ConduitKernelArgumentINTEL\0" // IR(17677, 27)
  "RegisterMapKernelArgumentINTEL\0" // IR(17704, 31)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(17735, 33)
  "MMHostInterfaceDataWidthINTEL\0" // IR(17768, 30)
  "MMHostInterfaceLatencyINTEL\0" // IR(17798, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(17826, 34)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(17860, 34)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(17894, 29)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(17923, 32)
  "StableKernelArgumentINTEL\0" // IR(17955, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(17981, 39)
  "HostAccessINTEL\0" // IR(18020, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(18036, 47)
  "InitModeINTEL\0" // IR(18083, 14)
  "ImplementInRegisterMapINTEL\0" // IR(18097, 28)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(18125, 36)
  "CacheControlLoadINTEL\0" // IR(18161, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(18183, 37)
  "CacheControlStoreINTEL\0" // IR(18220, 23)
  "1D\0" // IR(18243, 3)
  "2D\0" // IR(18246, 3)
  "3D\0" // IR(18249, 3)
  "Cube\0" // IR(18252, 5)
  "Rect\0" // IR(18257, 5)
  "Buffer\0" // IR(18262, 7)
  "SubpassData\0" // IR(18269, 12)
  "TileImageDataEXT\0" // IR(18281, 17)
  "Invocations\0" // IR(18298, 12)
  "SpacingEqual\0" // IR(18310, 13)
  "SpacingFractionalEven\0" // IR(18323, 22)
  "SpacingFractionalOdd\0" // IR(18345, 21)
  "VertexOrderCw\0" // IR(18366, 14)
  "VertexOrderCcw\0" // IR(18380, 15)
  "PixelCenterInteger\0" // IR(18395, 19)
  "OriginUpperLeft\0" // IR(18414, 16)
  "OriginLowerLeft\0" // IR(18430, 16)
  "EarlyFragmentTests\0" // IR(18446, 19)
  "PointMode\0" // IR(18465, 10)
  "Xfb\0" // IR(18475, 4)
  "DepthReplacing\0" // IR(18479, 15)
  "DepthGreater\0" // IR(18494, 13)
  "DepthLess\0" // IR(18507, 10)
  "DepthUnchanged\0" // IR(18517, 15)
  "LocalSize\0" // IR(18532, 10)
  "LocalSizeHint\0" // IR(18542, 14)
  "InputPoints\0" // IR(18556, 12)
  "InputLines\0" // IR(18568, 11)
  "InputLinesAdjacency\0" // IR(18579, 20)
  "Triangles\0" // IR(18599, 10)
  "InputTrianglesAdjacency\0" // IR(18609, 24)
  "Quads\0" // IR(18633, 6)
  "Isolines\0" // IR(18639, 9)
  "OutputVertices\0" // IR(18648, 15)
  "OutputPoints\0" // IR(18663, 13)
  "OutputLineStrip\0" // IR(18676, 16)
  "OutputTriangleStrip\0" // IR(18692, 20)
  "VecTypeHint\0" // IR(18712, 12)
  "ContractionOff\0" // IR(18724, 15)
  "Initializer\0" // IR(18739, 12)
  "Finalizer\0" // IR(18751, 10)
  "SubgroupsPerWorkgroup\0" // IR(18761, 22)
  "SubgroupsPerWorkgroupId\0" // IR(18783, 24)
  "LocalSizeId\0" // IR(18807, 12)
  "LocalSizeHintId\0" // IR(18819, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(18835, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(18869, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(18903, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(18939, 30)
  "PostDepthCoverage\0" // IR(18969, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(18987, 34)
  "TileShadingRateQCOM\0" // IR(19021, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(19041, 29)
  "StencilRefReplacingEXT\0" // IR(19070, 23)
  "CoalescingAMDX\0" // IR(19093, 15)
  "IsApiEntryAMDX\0" // IR(19108, 15)
  "MaxNodeRecursionAMDX\0" // IR(19123, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(19144, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(19168, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(19189, 28)
  "StencilRefGreaterFrontAMD\0" // IR(19217, 26)
  "StencilRefLessFrontAMD\0" // IR(19243, 23)
  "StencilRefUnchangedBackAMD\0" // IR(19266, 27)
  "StencilRefGreaterBackAMD\0" // IR(19293, 25)
  "StencilRefLessBackAMD\0" // IR(19318, 22)
  "QuadDerivativesKHR\0" // IR(19340, 19)
  "RequireFullQuadsKHR\0" // IR(19359, 20)
  "SharesInputWithAMDX\0" // IR(19379, 20)
  "OutputLinesEXT\0" // IR(19399, 15)
  "OutputLinesNV\0" // IR(19414, 14)
  "OutputPrimitivesEXT\0" // IR(19428, 20)
  "OutputPrimitivesNV\0" // IR(19448, 19)
  "DerivativeGroupQuadsKHR\0" // IR(19467, 24)
  "DerivativeGroupQuadsNV\0" // IR(19491, 23)
  "DerivativeGroupLinearKHR\0" // IR(19514, 25)
  "DerivativeGroupLinearNV\0" // IR(19539, 24)
  "OutputTrianglesEXT\0" // IR(19563, 19)
  "OutputTrianglesNV\0" // IR(19582, 18)
  "PixelInterlockOrderedEXT\0" // IR(19600, 25)
  "PixelInterlockUnorderedEXT\0" // IR(19625, 27)
  "SampleInterlockOrderedEXT\0" // IR(19652, 26)
  "SampleInterlockUnorderedEXT\0" // IR(19678, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(19706, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(19737, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(19770, 27)
  "RoundingModeRTPINTEL\0" // IR(19797, 21)
  "RoundingModeRTNINTEL\0" // IR(19818, 21)
  "FloatingPointModeALTINTEL\0" // IR(19839, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(19865, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(19892, 22)
  "MaxWorkDimINTEL\0" // IR(19914, 16)
  "NoGlobalOffsetINTEL\0" // IR(19930, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(19950, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(19972, 28)
  "MaximallyReconvergesKHR\0" // IR(20000, 24)
  "FPFastMathDefault\0" // IR(20024, 18)
  "StreamingInterfaceINTEL\0" // IR(20042, 24)
  "RegisterMapInterfaceINTEL\0" // IR(20066, 26)
  "NamedBarrierCountINTEL\0" // IR(20092, 23)
  "MaximumRegistersINTEL\0" // IR(20115, 22)
  "MaximumRegistersIdINTEL\0" // IR(20137, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(20161, 51)
  "NamedMaximumRegistersINTEL\0" // IR(20212, 27)
  "Vertex\0" // IR(20239, 7)
  "TessellationControl\0" // IR(20246, 20)
  "TessellationEvaluation\0" // IR(20266, 23)
  "GLCompute\0" // IR(20289, 10)
  "TaskNV\0" // IR(20299, 7)
  "MeshNV\0" // IR(20306, 7)
  "RayGenerationKHR\0" // IR(20313, 17)
  "RayGenerationNV\0" // IR(20330, 16)
  "IntersectionKHR\0" // IR(20346, 16)
  "IntersectionNV\0" // IR(20362, 15)
  "AnyHitKHR\0" // IR(20377, 10)
  "AnyHitNV\0" // IR(20387, 9)
  "ClosestHitKHR\0" // IR(20396, 14)
  "ClosestHitNV\0" // IR(20410, 13)
  "MissKHR\0" // IR(20423, 8)
  "MissNV\0" // IR(20431, 7)
  "CallableKHR\0" // IR(20438, 12)
  "CallableNV\0" // IR(20450, 11)
  "TaskEXT\0" // IR(20461, 8)
  "MeshEXT\0" // IR(20469, 8)
  "Preserve\0" // IR(20477, 9)
  "FlushToZero\0" // IR(20486, 12)
  "BFloat16KHR\0" // IR(20498, 12)
  "Float8E4M3EXT\0" // IR(20510, 14)
  "Float8E5M2EXT\0" // IR(20524, 14)
  "IEEE\0" // IR(20538, 5)
  "ALT\0" // IR(20543, 4)
  "NotNaN\0" // IR(20547, 7)
  "NotInf\0" // IR(20554, 7)
  "NSZ\0" // IR(20561, 4)
  "AllowRecip\0" // IR(20565, 11)
  "Fast\0" // IR(20576, 5)
  "AllowContract\0" // IR(20581, 14)
  "AllowContractFastINTEL\0" // IR(20595, 23)
  "AllowReassoc\0" // IR(20618, 13)
  "AllowReassocINTEL\0" // IR(20631, 18)
  "AllowTransform\0" // IR(20649, 15)
  "RTE\0" // IR(20664, 4)
  "RTZ\0" // IR(20668, 4)
  "RTP\0" // IR(20672, 4)
  "RTN\0" // IR(20676, 4)
  "Vertical2Pixels\0" // IR(20680, 16)
  "Vertical4Pixels\0" // IR(20696, 16)
  "Horizontal2Pixels\0" // IR(20712, 18)
  "Horizontal4Pixels\0" // IR(20730, 18)
  "Inline\0" // IR(20748, 7)
  "DontInline\0" // IR(20755, 11)
  "Pure\0" // IR(20766, 5)
  "Const\0" // IR(20771, 6)
  "Zext\0" // IR(20777, 5)
  "Sext\0" // IR(20782, 5)
  "ByVal\0" // IR(20787, 6)
  "Sret\0" // IR(20793, 5)
  "NoAlias\0" // IR(20798, 8)
  "NoCapture\0" // IR(20806, 10)
  "NoWrite\0" // IR(20816, 8)
  "NoReadWrite\0" // IR(20824, 12)
  "RuntimeAlignedINTEL\0" // IR(20836, 20)
  "Reduce\0" // IR(20856, 7)
  "InclusiveScan\0" // IR(20863, 14)
  "ExclusiveScan\0" // IR(20877, 14)
  "ClusteredReduce\0" // IR(20891, 16)
  "PartitionedReduceNV\0" // IR(20907, 20)
  "PartitionedInclusiveScanNV\0" // IR(20927, 27)
  "PartitionedExclusiveScanNV\0" // IR(20954, 27)
  "NoneINTEL\0" // IR(20981, 10)
  "ReadINTEL\0" // IR(20991, 10)
  "WriteINTEL\0" // IR(21001, 11)
  "ReadWriteINTEL\0" // IR(21012, 15)
  "Bias\0" // IR(21027, 5)
  "Lod\0" // IR(21032, 4)
  "Grad\0" // IR(21036, 5)
  "ConstOffset\0" // IR(21041, 12)
  "ConstOffsets\0" // IR(21053, 13)
  "MakeTexelAvailable\0" // IR(21066, 19)
  "MakeTexelAvailableKHR\0" // IR(21085, 22)
  "MakeTexelVisible\0" // IR(21107, 17)
  "MakeTexelVisibleKHR\0" // IR(21124, 20)
  "NonPrivateTexel\0" // IR(21144, 16)
  "NonPrivateTexelKHR\0" // IR(21160, 19)
  "VolatileTexel\0" // IR(21179, 14)
  "VolatileTexelKHR\0" // IR(21193, 17)
  "SignExtend\0" // IR(21210, 11)
  "ZeroExtend\0" // IR(21221, 11)
  "Nontemporal\0" // IR(21232, 12)
  "Offsets\0" // IR(21244, 8)
  "SnormInt8\0" // IR(21252, 10)
  "SnormInt16\0" // IR(21262, 11)
  "UnormInt8\0" // IR(21273, 10)
  "UnormInt16\0" // IR(21283, 11)
  "UnormShort565\0" // IR(21294, 14)
  "UnormShort555\0" // IR(21308, 14)
  "UnormInt101010\0" // IR(21322, 15)
  "SignedInt8\0" // IR(21337, 11)
  "SignedInt16\0" // IR(21348, 12)
  "SignedInt32\0" // IR(21360, 12)
  "UnsignedInt8\0" // IR(21372, 13)
  "UnsignedInt16\0" // IR(21385, 14)
  "UnsignedInt32\0" // IR(21399, 14)
  "HalfFloat\0" // IR(21413, 10)
  "UnormInt24\0" // IR(21423, 11)
  "UnormInt101010_2\0" // IR(21434, 17)
  "UnormInt10X6EXT\0" // IR(21451, 16)
  "UnsignedIntRaw10EXT\0" // IR(21467, 20)
  "UnsignedIntRaw12EXT\0" // IR(21487, 20)
  "UnormInt2_101010EXT\0" // IR(21507, 20)
  "UnsignedInt10X6EXT\0" // IR(21527, 19)
  "UnsignedInt12X4EXT\0" // IR(21546, 19)
  "UnsignedInt14X2EXT\0" // IR(21565, 19)
  "UnormInt12X4EXT\0" // IR(21584, 16)
  "UnormInt14X2EXT\0" // IR(21600, 16)
  "R\0" // IR(21616, 2)
  "A\0" // IR(21618, 2)
  "RG\0" // IR(21620, 3)
  "RA\0" // IR(21623, 3)
  "RGB\0" // IR(21626, 4)
  "RGBA\0" // IR(21630, 5)
  "BGRA\0" // IR(21635, 5)
  "ARGB\0" // IR(21640, 5)
  "Intensity\0" // IR(21645, 10)
  "Luminance\0" // IR(21655, 10)
  "Rx\0" // IR(21665, 3)
  "RGx\0" // IR(21668, 4)
  "RGBx\0" // IR(21672, 5)
  "Depth\0" // IR(21677, 6)
  "DepthStencil\0" // IR(21683, 13)
  "sRGB\0" // IR(21696, 5)
  "sRGBx\0" // IR(21701, 6)
  "sRGBA\0" // IR(21707, 6)
  "sBGRA\0" // IR(21713, 6)
  "ABGR\0" // IR(21719, 5)
  "InitOnDeviceReprogramINTEL\0" // IR(21724, 27)
  "InitOnDeviceResetINTEL\0" // IR(21751, 23)
  "NoWait\0" // IR(21774, 7)
  "WaitKernel\0" // IR(21781, 11)
  "WaitWorkGroup\0" // IR(21792, 14)
  "CmdExecTime\0" // IR(21806, 12)
  "MayUsePrintf\0" // IR(21818, 13)
  "Export\0" // IR(21831, 7)
  "Import\0" // IR(21838, 7)
  "LinkOnceODR\0" // IR(21845, 12)
  "UncachedINTEL\0" // IR(21857, 14)
  "CachedINTEL\0" // IR(21871, 12)
  "StreamingINTEL\0" // IR(21883, 15)
  "InvalidateAfterReadINTEL\0" // IR(21898, 25)
  "ConstCachedINTEL\0" // IR(21923, 17)
  "Unroll\0" // IR(21940, 7)
  "DontUnroll\0" // IR(21947, 11)
  "DependencyInfinite\0" // IR(21958, 19)
  "DependencyLength\0" // IR(21977, 17)
  "MinIterations\0" // IR(21994, 14)
  "MaxIterations\0" // IR(22008, 14)
  "IterationMultiple\0" // IR(22022, 18)
  "PeelCount\0" // IR(22040, 10)
  "PartialCount\0" // IR(22050, 13)
  "DependencyArrayINTEL\0" // IR(22063, 21)
  "LoopCoalesceINTEL\0" // IR(22084, 18)
  "MaxInterleavingINTEL\0" // IR(22102, 21)
  "SpeculatedIterationsINTEL\0" // IR(22123, 26)
  "NoFusionINTEL\0" // IR(22149, 14)
  "LoopCountINTEL\0" // IR(22163, 15)
  "MaxReinvocationDelayINTEL\0" // IR(22178, 26)
  "MatrixASignedComponentsINTEL\0" // IR(22204, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(22233, 29)
  "MatrixCBFloat16INTEL\0" // IR(22262, 21)
  "MatrixResultBFloat16INTEL\0" // IR(22283, 26)
  "MatrixAPackedInt8INTEL\0" // IR(22309, 23)
  "MatrixBPackedInt8INTEL\0" // IR(22332, 23)
  "MatrixAPackedInt4INTEL\0" // IR(22355, 23)
  "MatrixBPackedInt4INTEL\0" // IR(22378, 23)
  "MatrixATF32INTEL\0" // IR(22401, 17)
  "MatrixBTF32INTEL\0" // IR(22418, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(22435, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(22461, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(22487, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(22514, 27)
  "Aligned\0" // IR(22541, 8)
  "MakePointerAvailable\0" // IR(22549, 21)
  "MakePointerAvailableKHR\0" // IR(22570, 24)
  "MakePointerVisible\0" // IR(22594, 19)
  "MakePointerVisibleKHR\0" // IR(22613, 22)
  "NonPrivatePointer\0" // IR(22635, 18)
  "NonPrivatePointerKHR\0" // IR(22653, 21)
  "AliasScopeINTELMask\0" // IR(22674, 20)
  "NoAliasINTELMask\0" // IR(22694, 17)
  "Simple\0" // IR(22711, 7)
  "GLSL450\0" // IR(22718, 8)
  "OpenCL\0" // IR(22726, 7)
  "Vulkan\0" // IR(22733, 7)
  "VulkanKHR\0" // IR(22740, 10)
  "Relaxed\0" // IR(22750, 8)
  "Acquire\0" // IR(22758, 8)
  "Release\0" // IR(22766, 8)
  "AcquireRelease\0" // IR(22774, 15)
  "SequentiallyConsistent\0" // IR(22789, 23)
  "UniformMemory\0" // IR(22812, 14)
  "SubgroupMemory\0" // IR(22826, 15)
  "WorkgroupMemory\0" // IR(22841, 16)
  "CrossWorkgroupMemory\0" // IR(22857, 21)
  "AtomicCounterMemory\0" // IR(22878, 20)
  "ImageMemory\0" // IR(22898, 12)
  "OutputMemory\0" // IR(22910, 13)
  "OutputMemoryKHR\0" // IR(22923, 16)
  "MakeAvailable\0" // IR(22939, 14)
  "MakeAvailableKHR\0" // IR(22953, 17)
  "MakeVisible\0" // IR(22970, 12)
  "MakeVisibleKHR\0" // IR(22982, 15)
  "AutoINTEL\0" // IR(22997, 10)
  "WRAP\0" // IR(23007, 5)
  "SAT\0" // IR(23012, 4)
  "SAT_ZERO\0" // IR(23016, 9)
  "SAT_SYM\0" // IR(23025, 8)
  "PackedVectorFormat4x8Bit\0" // IR(23033, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(23058, 28)
  "TRN\0" // IR(23086, 4)
  "TRN_ZERO\0" // IR(23090, 9)
  "RND\0" // IR(23099, 4)
  "RND_ZERO\0" // IR(23103, 9)
  "RND_INF\0" // IR(23112, 8)
  "RND_MIN_INF\0" // IR(23120, 12)
  "RND_CONV\0" // IR(23132, 9)
  "RND_CONV_ODD\0" // IR(23141, 13)
  "RobustnessPerComponentNV\0" // IR(23154, 25)
  "RobustnessPerElementNV\0" // IR(23179, 23)
  "OpaqueKHR\0" // IR(23202, 10)
  "NoOpaqueKHR\0" // IR(23212, 12)
  "TerminateOnFirstHitKHR\0" // IR(23224, 23)
  "SkipClosestHitShaderKHR\0" // IR(23247, 24)
  "CullBackFacingTrianglesKHR\0" // IR(23271, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(23298, 28)
  "CullOpaqueKHR\0" // IR(23326, 14)
  "CullNoOpaqueKHR\0" // IR(23340, 16)
  "SkipTrianglesKHR\0" // IR(23356, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(23373, 24)
  "SkipAABBsKHR\0" // IR(23397, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(23410, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(23440, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(23481, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(23518, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(23555, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(23596, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(23638, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(23671, 33)
  "ClampToEdge\0" // IR(23704, 12)
  "Clamp\0" // IR(23716, 6)
  "Repeat\0" // IR(23722, 7)
  "RepeatMirrored\0" // IR(23729, 15)
  "Nearest\0" // IR(23744, 8)
  "Linear\0" // IR(23752, 7)
  "Unknown\0" // IR(23759, 8)
  "Rgba32f\0" // IR(23767, 8)
  "Rgba16f\0" // IR(23775, 8)
  "R32f\0" // IR(23783, 5)
  "Rgba8\0" // IR(23788, 6)
  "Rgba8Snorm\0" // IR(23794, 11)
  "Rg32f\0" // IR(23805, 6)
  "Rg16f\0" // IR(23811, 6)
  "R11fG11fB10f\0" // IR(23817, 13)
  "R16f\0" // IR(23830, 5)
  "Rgba16\0" // IR(23835, 7)
  "Rgb10A2\0" // IR(23842, 8)
  "Rg16\0" // IR(23850, 5)
  "Rg8\0" // IR(23855, 4)
  "R16\0" // IR(23859, 4)
  "R8\0" // IR(23863, 3)
  "Rgba16Snorm\0" // IR(23866, 12)
  "Rg16Snorm\0" // IR(23878, 10)
  "Rg8Snorm\0" // IR(23888, 9)
  "R16Snorm\0" // IR(23897, 9)
  "R8Snorm\0" // IR(23906, 8)
  "Rgba32i\0" // IR(23914, 8)
  "Rgba16i\0" // IR(23922, 8)
  "Rgba8i\0" // IR(23930, 7)
  "R32i\0" // IR(23937, 5)
  "Rg32i\0" // IR(23942, 6)
  "Rg16i\0" // IR(23948, 6)
  "Rg8i\0" // IR(23954, 5)
  "R16i\0" // IR(23959, 5)
  "R8i\0" // IR(23964, 4)
  "Rgba32ui\0" // IR(23968, 9)
  "Rgba16ui\0" // IR(23977, 9)
  "Rgba8ui\0" // IR(23986, 8)
  "R32ui\0" // IR(23994, 6)
  "Rgb10a2ui\0" // IR(24000, 10)
  "Rg32ui\0" // IR(24010, 7)
  "Rg16ui\0" // IR(24017, 7)
  "Rg8ui\0" // IR(24024, 6)
  "R16ui\0" // IR(24030, 6)
  "R8ui\0" // IR(24036, 5)
  "R64ui\0" // IR(24041, 6)
  "R64i\0" // IR(24047, 5)
  "CrossDevice\0" // IR(24052, 12)
  "Device\0" // IR(24064, 7)
  "Workgroup\0" // IR(24071, 10)
  "Subgroup\0" // IR(24081, 9)
  "Invocation\0" // IR(24090, 11)
  "QueueFamily\0" // IR(24101, 12)
  "QueueFamilyKHR\0" // IR(24113, 15)
  "ShaderCallKHR\0" // IR(24128, 14)
  "Flatten\0" // IR(24142, 8)
  "DontFlatten\0" // IR(24150, 12)
  "IdentifierPossibleDuplicates\0" // IR(24162, 29)
  "FlagUnknownPhysicalLayout\0" // IR(24191, 26)
  "ESSL\0" // IR(24217, 5)
  "GLSL\0" // IR(24222, 5)
  "OpenCL_C\0" // IR(24227, 9)
  "OpenCL_CPP\0" // IR(24236, 11)
  "HLSL\0" // IR(24247, 5)
  "CPP_for_OpenCL\0" // IR(24252, 15)
  "SYCL\0" // IR(24267, 5)
  "HERO_C\0" // IR(24272, 7)
  "NZSL\0" // IR(24279, 5)
  "WGSL\0" // IR(24284, 5)
  "Slang\0" // IR(24289, 6)
  "Zig\0" // IR(24295, 4)
  "Rust\0" // IR(24299, 5)
  "UniformConstant\0" // IR(24304, 16)
  "Input\0" // IR(24320, 6)
  "Output\0" // IR(24326, 7)
  "CrossWorkgroup\0" // IR(24333, 15)
  "Private\0" // IR(24348, 8)
  "Function\0" // IR(24356, 9)
  "Generic\0" // IR(24365, 8)
  "PushConstant\0" // IR(24373, 13)
  "AtomicCounter\0" // IR(24386, 14)
  "Image\0" // IR(24400, 6)
  "StorageBuffer\0" // IR(24406, 14)
  "TileImageEXT\0" // IR(24420, 13)
  "TileAttachmentQCOM\0" // IR(24433, 19)
  "NodePayloadAMDX\0" // IR(24452, 16)
  "CallableDataKHR\0" // IR(24468, 16)
  "CallableDataNV\0" // IR(24484, 15)
  "IncomingCallableDataKHR\0" // IR(24499, 24)
  "IncomingCallableDataNV\0" // IR(24523, 23)
  "RayPayloadKHR\0" // IR(24546, 14)
  "RayPayloadNV\0" // IR(24560, 13)
  "HitAttributeKHR\0" // IR(24573, 16)
  "HitAttributeNV\0" // IR(24589, 15)
  "IncomingRayPayloadKHR\0" // IR(24604, 22)
  "IncomingRayPayloadNV\0" // IR(24626, 21)
  "ShaderRecordBufferKHR\0" // IR(24647, 22)
  "ShaderRecordBufferNV\0" // IR(24669, 21)
  "PhysicalStorageBuffer\0" // IR(24690, 22)
  "PhysicalStorageBufferEXT\0" // IR(24712, 25)
  "HitObjectAttributeNV\0" // IR(24737, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(24758, 24)
  "CodeSectionINTEL\0" // IR(24782, 17)
  "DeviceOnlyINTEL\0" // IR(24799, 16)
  "HostOnlyINTEL\0" // IR(24815, 14)
  "WriteThroughINTEL\0" // IR(24829, 18)
  "WriteBackINTEL\0" // IR(24847, 15)
  "TensorView\0" // IR(24862, 11)
  "DecodeFunc\0" // IR(24873, 11)
  "Undefined\0" // IR(24884, 10)
  "NoneARM\0" // IR(24894, 8)
  "NontemporalARM\0" // IR(24902, 15)
  "OutOfBoundsValueARM\0" // IR(24917, 20)
  "MakeElementAvailableARM\0" // IR(24937, 24)
  "MakeElementVisibleARM\0" // IR(24961, 22)
  "NonPrivateElementARM\0" // IR(24983, 21)
  "Nop\0" // IR(25004, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(25008, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(25033, 27)
  "Undef\0" // IR(25060, 6)
  "SourceContinued\0" // IR(25066, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(25082, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(25115, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(25144, 41)
  "Source\0" // IR(25185, 7)
  "SourceExtension\0" // IR(25192, 16)
  "Name\0" // IR(25208, 5)
  "MemberName\0" // IR(25213, 11)
  "String\0" // IR(25224, 7)
  "Line\0" // IR(25231, 5)
  "Extension\0" // IR(25236, 10)
  "ExtInstImport\0" // IR(25246, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(25260, 46)
  "ExtInst\0" // IR(25306, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(25314, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(25348, 30)
  "MemoryModel\0" // IR(25378, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(25390, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(25423, 29)
  "EntryPoint\0" // IR(25452, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(25463, 32)
  "ExecutionMode\0" // IR(25495, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(25509, 28)
  "Capability\0" // IR(25537, 11)
  "TypeVoid\0" // IR(25548, 9)
  "TypeBool\0" // IR(25557, 9)
  "TypeInt\0" // IR(25566, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(25574, 37)
  "TypeFloat\0" // IR(25611, 10)
  "TypeVector\0" // IR(25621, 11)
  "TypeMatrix\0" // IR(25632, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(25643, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(25675, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(25713, 43)
  "TypeImage\0" // IR(25756, 10)
  "TypeSampler\0" // IR(25766, 12)
  "TypeSampledImage\0" // IR(25778, 17)
  "TypeArray\0" // IR(25795, 10)
  "TypeRuntimeArray\0" // IR(25805, 17)
  "TypeStruct\0" // IR(25822, 11)
  "TypeOpaque\0" // IR(25833, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(25844, 31)
  "TypePointer\0" // IR(25875, 12)
  "TypeFunction\0" // IR(25887, 13)
  "TypeEvent\0" // IR(25900, 10)
  "TypeDeviceEvent\0" // IR(25910, 16)
  "TypeReserveId\0" // IR(25926, 14)
  "TypeQueue\0" // IR(25940, 10)
  "TypePipe\0" // IR(25950, 9)
  "TypeForwardPointer\0" // IR(25959, 19)
  "ConstantTrue\0" // IR(25978, 13)
  "ConstantFalse\0" // IR(25991, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(26005, 38)
  "ConstantComposite\0" // IR(26043, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(26061, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(26102, 37)
  "ConstantSampler\0" // IR(26139, 16)
  "ConstantNull\0" // IR(26155, 13)
  "SpecConstantTrue\0" // IR(26168, 17)
  "SpecConstantFalse\0" // IR(26185, 18)
  "SpecConstant\0" // IR(26203, 13)
  "SpecConstantComposite\0" // IR(26216, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(26238, 41)
  "SpecConstantOp\0" // IR(26279, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(26294, 34)
  "FunctionParameter\0" // IR(26328, 18)
  "FunctionEnd\0" // IR(26346, 12)
  "FunctionCall\0" // IR(26358, 13)
  "Variable\0" // IR(26371, 9)
  "ImageTexelPointer\0" // IR(26380, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(26398, 40)
  "Load\0" // IR(26438, 5)
  "Store\0" // IR(26443, 6)
  "CopyMemory\0" // IR(26449, 11)
  "CopyMemorySized\0" // IR(26460, 16)
  "AccessChain\0" // IR(26476, 12)
  "InBoundsAccessChain\0" // IR(26488, 20)
  "PtrAccessChain\0" // IR(26508, 15)
  "ArrayLength\0" // IR(26523, 12)
  "GenericPtrMemSemantics\0" // IR(26535, 23)
  "InBoundsPtrAccessChain\0" // IR(26558, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(26581, 28)
  "Decorate\0" // IR(26609, 9)
  "MemberDecorate\0" // IR(26618, 15)
  "DecorationGroup\0" // IR(26633, 16)
  "GroupDecorate\0" // IR(26649, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(26663, 45)
  "GroupMemberDecorate\0" // IR(26708, 20)
  "VectorExtractDynamic\0" // IR(26728, 21)
  "VectorInsertDynamic\0" // IR(26749, 20)
  "VectorShuffle\0" // IR(26769, 14)
  "CompositeConstruct\0" // IR(26783, 19)
  "CompositeExtract\0" // IR(26802, 17)
  "CompositeInsert\0" // IR(26819, 16)
  "CopyObject\0" // IR(26835, 11)
  "Transpose\0" // IR(26846, 10)
  "SampledImage\0" // IR(26856, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(26869, 32)
  "ImageSampleImplicitLod\0" // IR(26901, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(26924, 23)
  "ImageSampleExplicitLod\0" // IR(26947, 23)
  "ImageSampleDrefImplicitLod\0" // IR(26970, 27)
  "ImageSampleDrefExplicitLod\0" // IR(26997, 27)
  "ImageSampleProjImplicitLod\0" // IR(27024, 27)
  "ImageSampleProjExplicitLod\0" // IR(27051, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(27078, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(27109, 31)
  "ImageFetch\0" // IR(27140, 11)
  "ImageGather\0" // IR(27151, 12)
  "ImageDrefGather\0" // IR(27163, 16)
  "ImageRead\0" // IR(27179, 10)
  "ImageWrite\0" // IR(27189, 11)
  "ImageQueryFormat\0" // IR(27200, 17)
  "ImageQueryOrder\0" // IR(27217, 16)
  "ImageQuerySizeLod\0" // IR(27233, 18)
  "ImageQuerySize\0" // IR(27251, 15)
  "ImageQueryLod\0" // IR(27266, 14)
  "ImageQueryLevels\0" // IR(27280, 17)
  "ImageQuerySamples\0" // IR(27297, 18)
  "ConvertFToU\0" // IR(27315, 12)
  "ConvertFToS\0" // IR(27327, 12)
  "ConvertSToF\0" // IR(27339, 12)
  "ConvertUToF\0" // IR(27351, 12)
  "UConvert\0" // IR(27363, 9)
  "SConvert\0" // IR(27372, 9)
  "FConvert\0" // IR(27381, 9)
  "QuantizeToF16\0" // IR(27390, 14)
  "ConvertPtrToU\0" // IR(27404, 14)
  "SatConvertSToU\0" // IR(27418, 15)
  "SatConvertUToS\0" // IR(27433, 15)
  "ConvertUToPtr\0" // IR(27448, 14)
  "PtrCastToGeneric\0" // IR(27462, 17)
  "GenericCastToPtr\0" // IR(27479, 17)
  "GenericCastToPtrExplicit\0" // IR(27496, 25)
  "Bitcast\0" // IR(27521, 8)
  "SNegate\0" // IR(27529, 8)
  "FNegate\0" // IR(27537, 8)
  "IAdd\0" // IR(27545, 5)
  "FAdd\0" // IR(27550, 5)
  "ISub\0" // IR(27555, 5)
  "FSub\0" // IR(27560, 5)
  "IMul\0" // IR(27565, 5)
  "FMul\0" // IR(27570, 5)
  "UDiv\0" // IR(27575, 5)
  "SDiv\0" // IR(27580, 5)
  "FDiv\0" // IR(27585, 5)
  "UMod\0" // IR(27590, 5)
  "SRem\0" // IR(27595, 5)
  "SMod\0" // IR(27600, 5)
  "FRem\0" // IR(27605, 5)
  "FMod\0" // IR(27610, 5)
  "VectorTimesScalar\0" // IR(27615, 18)
  "MatrixTimesScalar\0" // IR(27633, 18)
  "VectorTimesMatrix\0" // IR(27651, 18)
  "MatrixTimesVector\0" // IR(27669, 18)
  "MatrixTimesMatrix\0" // IR(27687, 18)
  "OuterProduct\0" // IR(27705, 13)
  "Dot\0" // IR(27718, 4)
  "IAddCarry\0" // IR(27722, 10)
  "ISubBorrow\0" // IR(27732, 11)
  "UMulExtended\0" // IR(27743, 13)
  "SMulExtended\0" // IR(27756, 13)
  "Any\0" // IR(27769, 4)
  "All\0" // IR(27773, 4)
  "IsNan\0" // IR(27777, 6)
  "IsInf\0" // IR(27783, 6)
  "IsFinite\0" // IR(27789, 9)
  "IsNormal\0" // IR(27798, 9)
  "SignBitSet\0" // IR(27807, 11)
  "LessOrGreater\0" // IR(27818, 14)
  "Ordered\0" // IR(27832, 8)
  "Unordered\0" // IR(27840, 10)
  "LogicalEqual\0" // IR(27850, 13)
  "LogicalNotEqual\0" // IR(27863, 16)
  "LogicalOr\0" // IR(27879, 10)
  "LogicalAnd\0" // IR(27889, 11)
  "LogicalNot\0" // IR(27900, 11)
  "Select\0" // IR(27911, 7)
  "IEqual\0" // IR(27918, 7)
  "INotEqual\0" // IR(27925, 10)
  "UGreaterThan\0" // IR(27935, 13)
  "SGreaterThan\0" // IR(27948, 13)
  "UGreaterThanEqual\0" // IR(27961, 18)
  "SGreaterThanEqual\0" // IR(27979, 18)
  "ULessThan\0" // IR(27997, 10)
  "SLessThan\0" // IR(28007, 10)
  "ULessThanEqual\0" // IR(28017, 15)
  "SLessThanEqual\0" // IR(28032, 15)
  "FOrdEqual\0" // IR(28047, 10)
  "FUnordEqual\0" // IR(28057, 12)
  "FOrdNotEqual\0" // IR(28069, 13)
  "FUnordNotEqual\0" // IR(28082, 15)
  "FOrdLessThan\0" // IR(28097, 13)
  "FUnordLessThan\0" // IR(28110, 15)
  "FOrdGreaterThan\0" // IR(28125, 16)
  "FUnordGreaterThan\0" // IR(28141, 18)
  "FOrdLessThanEqual\0" // IR(28159, 18)
  "FUnordLessThanEqual\0" // IR(28177, 20)
  "FOrdGreaterThanEqual\0" // IR(28197, 21)
  "FUnordGreaterThanEqual\0" // IR(28218, 23)
  "ShiftRightLogical\0" // IR(28241, 18)
  "ShiftRightArithmetic\0" // IR(28259, 21)
  "ShiftLeftLogical\0" // IR(28280, 17)
  "BitwiseOr\0" // IR(28297, 10)
  "BitwiseXor\0" // IR(28307, 11)
  "BitwiseAnd\0" // IR(28318, 11)
  "Not\0" // IR(28329, 4)
  "BitFieldInsert\0" // IR(28333, 15)
  "BitFieldSExtract\0" // IR(28348, 17)
  "BitFieldUExtract\0" // IR(28365, 17)
  "BitReverse\0" // IR(28382, 11)
  "BitCount\0" // IR(28393, 9)
  "DPdx\0" // IR(28402, 5)
  "DPdy\0" // IR(28407, 5)
  "Fwidth\0" // IR(28412, 7)
  "DPdxFine\0" // IR(28419, 9)
  "DPdyFine\0" // IR(28428, 9)
  "FwidthFine\0" // IR(28437, 11)
  "DPdxCoarse\0" // IR(28448, 11)
  "DPdyCoarse\0" // IR(28459, 11)
  "FwidthCoarse\0" // IR(28470, 13)
  "EmitVertex\0" // IR(28483, 11)
  "EndPrimitive\0" // IR(28494, 13)
  "EmitStreamVertex\0" // IR(28507, 17)
  "EndStreamPrimitive\0" // IR(28524, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(28543, 37)
  "ControlBarrier\0" // IR(28580, 15)
  "MemoryBarrier\0" // IR(28595, 14)
  "AtomicLoad\0" // IR(28609, 11)
  "AtomicStore\0" // IR(28620, 12)
  "AtomicExchange\0" // IR(28632, 15)
  "AtomicCompareExchange\0" // IR(28647, 22)
  "AtomicCompareExchangeWeak\0" // IR(28669, 26)
  "AtomicIIncrement\0" // IR(28695, 17)
  "AtomicIDecrement\0" // IR(28712, 17)
  "AtomicIAdd\0" // IR(28729, 11)
  "AtomicISub\0" // IR(28740, 11)
  "AtomicSMin\0" // IR(28751, 11)
  "AtomicUMin\0" // IR(28762, 11)
  "AtomicSMax\0" // IR(28773, 11)
  "AtomicUMax\0" // IR(28784, 11)
  "AtomicAnd\0" // IR(28795, 10)
  "AtomicOr\0" // IR(28805, 9)
  "AtomicXor\0" // IR(28814, 10)
  "Phi\0" // IR(28824, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(28828, 30)
  "LoopMerge\0" // IR(28858, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(28868, 35)
  "SelectionMerge\0" // IR(28903, 15)
  "Label\0" // IR(28918, 6)
  "Branch\0" // IR(28924, 7)
  "BranchConditional\0" // IR(28931, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(28949, 45)
  "Switch\0" // IR(28994, 7)
  "Kill\0" // IR(29001, 5)
  "Return\0" // IR(29006, 7)
  "ReturnValue\0" // IR(29013, 12)
  "Unreachable\0" // IR(29025, 12)
  "LifetimeStart\0" // IR(29037, 14)
  "LifetimeStop\0" // IR(29051, 13)
  "GroupAsyncCopy\0" // IR(29064, 15)
  "GroupWaitEvents\0" // IR(29079, 16)
  "GroupAll\0" // IR(29095, 9)
  "GroupAny\0" // IR(29104, 9)
  "GroupBroadcast\0" // IR(29113, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(29128, 33)
  "GroupIAdd\0" // IR(29161, 10)
  "GroupFAdd\0" // IR(29171, 10)
  "GroupFMin\0" // IR(29181, 10)
  "GroupUMin\0" // IR(29191, 10)
  "GroupSMin\0" // IR(29201, 10)
  "GroupFMax\0" // IR(29211, 10)
  "GroupUMax\0" // IR(29221, 10)
  "GroupSMax\0" // IR(29231, 10)
  "ReadPipe\0" // IR(29241, 9)
  "WritePipe\0" // IR(29250, 10)
  "ReservedReadPipe\0" // IR(29260, 17)
  "ReservedWritePipe\0" // IR(29277, 18)
  "ReserveReadPipePackets\0" // IR(29295, 23)
  "ReserveWritePipePackets\0" // IR(29318, 24)
  "CommitReadPipe\0" // IR(29342, 15)
  "CommitWritePipe\0" // IR(29357, 16)
  "IsValidReserveId\0" // IR(29373, 17)
  "GetNumPipePackets\0" // IR(29390, 18)
  "GetMaxPipePackets\0" // IR(29408, 18)
  "GroupReserveReadPipePackets\0" // IR(29426, 28)
  "GroupReserveWritePipePackets\0" // IR(29454, 29)
  "GroupCommitReadPipe\0" // IR(29483, 20)
  "GroupCommitWritePipe\0" // IR(29503, 21)
  "EnqueueMarker\0" // IR(29524, 14)
  "EnqueueKernel\0" // IR(29538, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(29552, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(29582, 32)
  "GetKernelWorkGroupSize\0" // IR(29614, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(29637, 40)
  "RetainEvent\0" // IR(29677, 12)
  "ReleaseEvent\0" // IR(29689, 13)
  "CreateUserEvent\0" // IR(29702, 16)
  "IsValidEvent\0" // IR(29718, 13)
  "SetUserEventStatus\0" // IR(29731, 19)
  "CaptureEventProfilingInfo\0" // IR(29750, 26)
  "GetDefaultQueue\0" // IR(29776, 16)
  "BuildNDRange\0" // IR(29792, 13)
  "ImageSparseSampleImplicitLod\0" // IR(29805, 29)
  "ImageSparseSampleExplicitLod\0" // IR(29834, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(29863, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(29896, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(29929, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(29962, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(29995, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(30032, 37)
  "ImageSparseFetch\0" // IR(30069, 17)
  "ImageSparseGather\0" // IR(30086, 18)
  "ImageSparseDrefGather\0" // IR(30104, 22)
  "ImageSparseTexelsResident\0" // IR(30126, 26)
  "NoLine\0" // IR(30152, 7)
  "AtomicFlagTestAndSet\0" // IR(30159, 21)
  "AtomicFlagClear\0" // IR(30180, 16)
  "ImageSparseRead\0" // IR(30196, 16)
  "SizeOf\0" // IR(30212, 7)
  "TypePipeStorage\0" // IR(30219, 16)
  "ConstantPipeStorage\0" // IR(30235, 20)
  "CreatePipeFromPipeStorage\0" // IR(30255, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(30281, 35)
  "GetKernelMaxNumSubgroups\0" // IR(30316, 25)
  "TypeNamedBarrier\0" // IR(30341, 17)
  "NamedBarrierInitialize\0" // IR(30358, 23)
  "MemoryNamedBarrier\0" // IR(30381, 19)
  "ModuleProcessed\0" // IR(30400, 16)
  "ExecutionModeId\0" // IR(30416, 16)
  "DecorateId\0" // IR(30432, 11)
  "GroupNonUniformElect\0" // IR(30443, 21)
  "GroupNonUniformAll\0" // IR(30464, 19)
  "GroupNonUniformAny\0" // IR(30483, 19)
  "GroupNonUniformAllEqual\0" // IR(30502, 24)
  "GroupNonUniformBroadcast\0" // IR(30526, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(30551, 30)
  "GroupNonUniformInverseBallot\0" // IR(30581, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(30610, 32)
  "GroupNonUniformBallotBitCount\0" // IR(30642, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(30672, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(30701, 29)
  "GroupNonUniformShuffleXor\0" // IR(30730, 26)
  "GroupNonUniformShuffleUp\0" // IR(30756, 25)
  "GroupNonUniformShuffleDown\0" // IR(30781, 27)
  "GroupNonUniformIAdd\0" // IR(30808, 20)
  "GroupNonUniformFAdd\0" // IR(30828, 20)
  "GroupNonUniformIMul\0" // IR(30848, 20)
  "GroupNonUniformFMul\0" // IR(30868, 20)
  "GroupNonUniformSMin\0" // IR(30888, 20)
  "GroupNonUniformUMin\0" // IR(30908, 20)
  "GroupNonUniformFMin\0" // IR(30928, 20)
  "GroupNonUniformSMax\0" // IR(30948, 20)
  "GroupNonUniformUMax\0" // IR(30968, 20)
  "GroupNonUniformFMax\0" // IR(30988, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(31008, 26)
  "GroupNonUniformBitwiseOr\0" // IR(31034, 25)
  "GroupNonUniformBitwiseXor\0" // IR(31059, 26)
  "GroupNonUniformLogicalAnd\0" // IR(31085, 26)
  "GroupNonUniformLogicalOr\0" // IR(31111, 25)
  "GroupNonUniformLogicalXor\0" // IR(31136, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(31162, 29)
  "GroupNonUniformQuadSwap\0" // IR(31191, 24)
  "CopyLogical\0" // IR(31215, 12)
  "PtrEqual\0" // IR(31227, 9)
  "PtrNotEqual\0" // IR(31236, 12)
  "PtrDiff\0" // IR(31248, 8)
  "ColorAttachmentReadEXT\0" // IR(31256, 23)
  "DepthAttachmentReadEXT\0" // IR(31279, 23)
  "StencilAttachmentReadEXT\0" // IR(31302, 25)
  "TypeTensorARM\0" // IR(31327, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(31341, 42)
  "TensorReadARM\0" // IR(31383, 14)
  "TensorWriteARM\0" // IR(31397, 15)
  "TensorQuerySizeARM\0" // IR(31412, 19)
  "GraphConstantARM\0" // IR(31431, 17)
  "GraphEntryPointARM\0" // IR(31448, 19)
  "GraphInputARM\0" // IR(31467, 14)
  "GraphSetOutputARM\0" // IR(31481, 18)
  "GraphEndARM\0" // IR(31499, 12)
  "TypeGraphARM\0" // IR(31511, 13)
  "TerminateInvocation\0" // IR(31524, 20)
  "TypeUntypedPointerKHR\0" // IR(31544, 22)
  "UntypedVariableKHR\0" // IR(31566, 19)
  "UntypedAccessChainKHR\0" // IR(31585, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(31607, 30)
  "SubgroupFirstInvocationKHR\0" // IR(31637, 27)
  "UntypedPtrAccessChainKHR\0" // IR(31664, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(31689, 33)
  "UntypedArrayLengthKHR\0" // IR(31722, 22)
  "UntypedPrefetchKHR\0" // IR(31744, 19)
  "SubgroupAllKHR\0" // IR(31763, 15)
  "SubgroupAnyKHR\0" // IR(31778, 15)
  "SubgroupAllEqualKHR\0" // IR(31793, 20)
  "SubgroupReadInvocationKHR\0" // IR(31813, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(31839, 26)
  "TraceRayKHR\0" // IR(31865, 12)
  "ExecuteCallableKHR\0" // IR(31877, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(31896, 35)
  "IgnoreIntersectionKHR\0" // IR(31931, 22)
  "TerminateRayKHR\0" // IR(31953, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(31969, 47)
  "SDot\0" // IR(32016, 5)
  "SDotKHR\0" // IR(32021, 8)
  "UDot\0" // IR(32029, 5)
  "UDotKHR\0" // IR(32034, 8)
  "SUDot\0" // IR(32042, 6)
  "SUDotKHR\0" // IR(32048, 9)
  "SDotAccSat\0" // IR(32057, 11)
  "SDotAccSatKHR\0" // IR(32068, 14)
  "UDotAccSat\0" // IR(32082, 11)
  "UDotAccSatKHR\0" // IR(32093, 14)
  "SUDotAccSat\0" // IR(32107, 12)
  "SUDotAccSatKHR\0" // IR(32119, 15)
  "TypeCooperativeMatrixKHR\0" // IR(32134, 25)
  "CooperativeMatrixLoadKHR\0" // IR(32159, 25)
  "CooperativeMatrixStoreKHR\0" // IR(32184, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(32210, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(32264, 27)
  "CooperativeMatrixLengthKHR\0" // IR(32291, 27)
  "ConstantCompositeReplicateEXT\0" // IR(32318, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(32348, 34)
  "CompositeConstructReplicateEXT\0" // IR(32382, 31)
  "TypeRayQueryKHR\0" // IR(32413, 16)
  "RayQueryInitializeKHR\0" // IR(32429, 22)
  "RayQueryTerminateKHR\0" // IR(32451, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(32472, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(32504, 31)
  "RayQueryProceedKHR\0" // IR(32535, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(32554, 31)
  "ImageSampleWeightedQCOM\0" // IR(32585, 24)
  "ImageBoxFilterQCOM\0" // IR(32609, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(32628, 23)
  "ImageBlockMatchSADQCOM\0" // IR(32651, 23)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(32674, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(32703, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(32732, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(32761, 29)
  "GroupIAddNonUniformAMD\0" // IR(32790, 23)
  "GroupFAddNonUniformAMD\0" // IR(32813, 23)
  "GroupFMinNonUniformAMD\0" // IR(32836, 23)
  "GroupUMinNonUniformAMD\0" // IR(32859, 23)
  "GroupSMinNonUniformAMD\0" // IR(32882, 23)
  "GroupFMaxNonUniformAMD\0" // IR(32905, 23)
  "GroupUMaxNonUniformAMD\0" // IR(32928, 23)
  "GroupSMaxNonUniformAMD\0" // IR(32951, 23)
  "FragmentMaskFetchAMD\0" // IR(32974, 21)
  "FragmentFetchAMD\0" // IR(32995, 17)
  "ReadClockKHR\0" // IR(33012, 13)
  "AllocateNodePayloadsAMDX\0" // IR(33025, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(33050, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(33074, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(33099, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(33128, 27)
  "IsNodePayloadValidAMDX\0" // IR(33155, 23)
  "ConstantStringAMDX\0" // IR(33178, 19)
  "SpecConstantStringAMDX\0" // IR(33197, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(33220, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(33246, 26)
  "HitObjectRecordHitMotionNV\0" // IR(33272, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(33299, 36)
  "HitObjectRecordMissMotionNV\0" // IR(33335, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(33363, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(33391, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(33419, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(33452, 30)
  "HitObjectTraceRayMotionNV\0" // IR(33482, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(33508, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(33547, 44)
  "HitObjectRecordEmptyNV\0" // IR(33591, 23)
  "HitObjectTraceRayNV\0" // IR(33614, 20)
  "HitObjectRecordHitNV\0" // IR(33634, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(33655, 30)
  "HitObjectRecordMissNV\0" // IR(33685, 22)
  "HitObjectExecuteShaderNV\0" // IR(33707, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(33732, 26)
  "HitObjectGetAttributesNV\0" // IR(33758, 25)
  "HitObjectGetHitKindNV\0" // IR(33783, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(33805, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(33834, 28)
  "HitObjectGetInstanceIdNV\0" // IR(33862, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(33887, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(33921, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(33953, 29)
  "HitObjectGetRayTMaxNV\0" // IR(33982, 22)
  "HitObjectGetRayTMinNV\0" // IR(34004, 22)
  "HitObjectIsEmptyNV\0" // IR(34026, 19)
  "HitObjectIsHitNV\0" // IR(34045, 17)
  "HitObjectIsMissNV\0" // IR(34062, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(34080, 29)
  "ReorderThreadWithHintNV\0" // IR(34109, 24)
  "TypeHitObjectNV\0" // IR(34133, 16)
  "ImageSampleFootprintNV\0" // IR(34149, 23)
  "TypeCooperativeVectorNV\0" // IR(34172, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(34196, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(34225, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(34267, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(34306, 32)
  "CooperativeMatrixConvertNV\0" // IR(34338, 27)
  "EmitMeshTasksEXT\0" // IR(34365, 17)
  "SetMeshOutputsEXT\0" // IR(34382, 18)
  "GroupNonUniformPartitionNV\0" // IR(34400, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(34427, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(34460, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(34495, 38)
  "CooperativeVectorLoadNV\0" // IR(34533, 24)
  "CooperativeVectorStoreNV\0" // IR(34557, 25)
  "ReportIntersectionKHR\0" // IR(34582, 22)
  "ReportIntersectionNV\0" // IR(34604, 21)
  "IgnoreIntersectionNV\0" // IR(34625, 21)
  "TerminateRayNV\0" // IR(34646, 15)
  "TraceNV\0" // IR(34661, 8)
  "TraceMotionNV\0" // IR(34669, 14)
  "TraceRayMotionNV\0" // IR(34683, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(34700, 50)
  "TypeAccelerationStructureKHR\0" // IR(34750, 29)
  "TypeAccelerationStructureNV\0" // IR(34779, 28)
  "ExecuteCallableNV\0" // IR(34807, 18)
  "RayQueryGetClusterIdNV\0" // IR(34825, 23)
  "HitObjectGetClusterIdNV\0" // IR(34848, 24)
  "TypeCooperativeMatrixNV\0" // IR(34872, 24)
  "CooperativeMatrixLoadNV\0" // IR(34896, 24)
  "CooperativeMatrixStoreNV\0" // IR(34920, 25)
  "CooperativeMatrixMulAddNV\0" // IR(34945, 26)
  "CooperativeMatrixLengthNV\0" // IR(34971, 26)
  "BeginInvocationInterlockEXT\0" // IR(34997, 28)
  "EndInvocationInterlockEXT\0" // IR(35025, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(35051, 43)
  "CooperativeMatrixReduceNV\0" // IR(35094, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(35120, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(35151, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(35195, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(35225, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(35256, 32)
  "TypeTensorLayoutNV\0" // IR(35288, 19)
  "TypeTensorViewNV\0" // IR(35307, 17)
  "CreateTensorLayoutNV\0" // IR(35324, 21)
  "TensorLayoutSetDimensionNV\0" // IR(35345, 27)
  "TensorLayoutSetStrideNV\0" // IR(35372, 24)
  "TensorLayoutSliceNV\0" // IR(35396, 20)
  "TensorLayoutSetClampValueNV\0" // IR(35416, 28)
  "CreateTensorViewNV\0" // IR(35444, 19)
  "TensorViewSetDimensionNV\0" // IR(35463, 25)
  "TensorViewSetStrideNV\0" // IR(35488, 22)
  "IsHelperInvocationEXT\0" // IR(35510, 22)
  "TensorViewSetClipNV\0" // IR(35532, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(35552, 27)
  "CooperativeMatrixTransposeNV\0" // IR(35579, 29)
  "ConvertUToImageNV\0" // IR(35608, 18)
  "ConvertUToSamplerNV\0" // IR(35626, 20)
  "ConvertImageToUNV\0" // IR(35646, 18)
  "ConvertSamplerToUNV\0" // IR(35664, 20)
  "ConvertUToSampledImageNV\0" // IR(35684, 25)
  "ConvertSampledImageToUNV\0" // IR(35709, 25)
  "SamplerImageAddressingModeNV\0" // IR(35734, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(35763, 52)
  "RawAccessChainNV\0" // IR(35815, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(35832, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(35872, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(35910, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(35948, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(35982, 37)
  "HitObjectGetSpherePositionNV\0" // IR(36019, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(36048, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(36075, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(36102, 23)
  "HitObjectIsSphereHitNV\0" // IR(36125, 23)
  "HitObjectIsLSSHitNV\0" // IR(36148, 20)
  "RayQueryIsSphereHitNV\0" // IR(36168, 22)
  "RayQueryIsLSSHitNV\0" // IR(36190, 19)
  "SubgroupShuffleDownINTEL\0" // IR(36209, 25)
  "SubgroupShuffleUpINTEL\0" // IR(36234, 23)
  "SubgroupShuffleXorINTEL\0" // IR(36257, 24)
  "SubgroupBlockReadINTEL\0" // IR(36281, 23)
  "SubgroupBlockWriteINTEL\0" // IR(36304, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(36328, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(36356, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(36385, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(36418, 34)
  "UCountLeadingZerosINTEL\0" // IR(36452, 24)
  "UCountTrailingZerosINTEL\0" // IR(36476, 25)
  "AbsISubINTEL\0" // IR(36501, 13)
  "AbsUSubINTEL\0" // IR(36514, 13)
  "IAddSatINTEL\0" // IR(36527, 13)
  "UAddSatINTEL\0" // IR(36540, 13)
  "IAverageINTEL\0" // IR(36553, 14)
  "UAverageINTEL\0" // IR(36567, 14)
  "IAverageRoundedINTEL\0" // IR(36581, 21)
  "UAverageRoundedINTEL\0" // IR(36602, 21)
  "ISubSatINTEL\0" // IR(36623, 13)
  "USubSatINTEL\0" // IR(36636, 13)
  "IMul32x16INTEL\0" // IR(36649, 15)
  "UMul32x16INTEL\0" // IR(36664, 15)
  "ConstantFunctionPointerINTEL\0" // IR(36679, 29)
  "FunctionPointerCallINTEL\0" // IR(36708, 25)
  "AsmTargetINTEL\0" // IR(36733, 15)
  "AsmCallINTEL\0" // IR(36748, 13)
  "AtomicFMinEXT\0" // IR(36761, 14)
  "AtomicFMaxEXT\0" // IR(36775, 14)
  "AssumeTrueKHR\0" // IR(36789, 14)
  "ExpectKHR\0" // IR(36803, 10)
  "DecorateString\0" // IR(36813, 15)
  "DecorateStringGOOGLE\0" // IR(36828, 21)
  "MemberDecorateString\0" // IR(36849, 21)
  "MemberDecorateStringGOOGLE\0" // IR(36870, 27)
  "VmeImageINTEL\0" // IR(36897, 14)
  "TypeVmeImageINTEL\0" // IR(36911, 18)
  "TypeAvcImePayloadINTEL\0" // IR(36929, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(36952, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(36975, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(36998, 23)
  "TypeAvcMceResultINTEL\0" // IR(37021, 22)
  "TypeAvcImeResultINTEL\0" // IR(37043, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(37065, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(37111, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(37155, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(37194, 37)
  "TypeAvcRefResultINTEL\0" // IR(37231, 22)
  "TypeAvcSicResultINTEL\0" // IR(37253, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(37275, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(37335, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(37388, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(37435, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(37475, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(37526, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(37570, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(37621, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(37677, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(37727, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(37779, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(37828, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(37875, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(37925, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(37976, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(38032, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(38065, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(38117, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(38178, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(38239, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(38278, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(38316, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(38355, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(38393, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(38432, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(38470, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(38506, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(38545, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(38588, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(38626, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(38664, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(38702, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(38747, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(38787, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(38849, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(38879, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(38917, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(38953, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(38986, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(39021, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(39060, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(39103, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(39150, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(39204, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(39238, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(39285, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(39330, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(39385, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(39438, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(39494, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(39548, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(39606, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(39662, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(39700, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(39746, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(39790, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(39839, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(39886, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(39956, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(40024, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(40093, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(40161, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(40227, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(40294, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(40330, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(40378, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(40437, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(40495, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(40551, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(40581, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(40611, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(40650, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(40696, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(40739, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(40786, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(40831, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(40877, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(40933, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(40971, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(41001, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(41033, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(41069, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(41111, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(41150, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(41189, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(41233, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(41281, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(41331, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(41374, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(41422, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(41465, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(41496, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(41543, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(41588, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(41634, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(41690, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(41728, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(41763, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(41807, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(41853, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(41894, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(41930, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(41980, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(42028, 35)
  "SaveMemoryINTEL\0" // IR(42063, 16)
  "RestoreMemoryINTEL\0" // IR(42079, 19)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(42098, 28)
  "ArbitraryFloatCastINTEL\0" // IR(42126, 24)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(42150, 31)
  "ArbitraryFloatCastToIntINTEL\0" // IR(42181, 29)
  "ArbitraryFloatAddINTEL\0" // IR(42210, 23)
  "ArbitraryFloatSubINTEL\0" // IR(42233, 23)
  "ArbitraryFloatMulINTEL\0" // IR(42256, 23)
  "ArbitraryFloatDivINTEL\0" // IR(42279, 23)
  "ArbitraryFloatGTINTEL\0" // IR(42302, 22)
  "ArbitraryFloatGEINTEL\0" // IR(42324, 22)
  "ArbitraryFloatLTINTEL\0" // IR(42346, 22)
  "ArbitraryFloatLEINTEL\0" // IR(42368, 22)
  "ArbitraryFloatEQINTEL\0" // IR(42390, 22)
  "ArbitraryFloatRecipINTEL\0" // IR(42412, 25)
  "ArbitraryFloatRSqrtINTEL\0" // IR(42437, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(42462, 24)
  "ArbitraryFloatHypotINTEL\0" // IR(42486, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(42511, 24)
  "ArbitraryFloatLogINTEL\0" // IR(42535, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(42558, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(42582, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(42607, 25)
  "ArbitraryFloatExpINTEL\0" // IR(42632, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(42655, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(42679, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(42704, 25)
  "ArbitraryFloatSinINTEL\0" // IR(42729, 23)
  "ArbitraryFloatCosINTEL\0" // IR(42752, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(42775, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(42801, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(42826, 25)
  "ArbitraryFloatASinINTEL\0" // IR(42851, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(42875, 26)
  "ArbitraryFloatACosINTEL\0" // IR(42901, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(42925, 26)
  "ArbitraryFloatATanINTEL\0" // IR(42951, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(42975, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(43001, 25)
  "ArbitraryFloatPowINTEL\0" // IR(43026, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(43049, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(43073, 24)
  "LoopControlINTEL\0" // IR(43097, 17)
  "AliasDomainDeclINTEL\0" // IR(43114, 21)
  "AliasScopeDeclINTEL\0" // IR(43135, 20)
  "AliasScopeListDeclINTEL\0" // IR(43155, 24)
  "FixedSqrtINTEL\0" // IR(43179, 15)
  "FixedRecipINTEL\0" // IR(43194, 16)
  "FixedRsqrtINTEL\0" // IR(43210, 16)
  "FixedSinINTEL\0" // IR(43226, 14)
  "FixedCosINTEL\0" // IR(43240, 14)
  "FixedSinCosINTEL\0" // IR(43254, 17)
  "FixedSinPiINTEL\0" // IR(43271, 16)
  "FixedCosPiINTEL\0" // IR(43287, 16)
  "FixedSinCosPiINTEL\0" // IR(43303, 19)
  "FixedLogINTEL\0" // IR(43322, 14)
  "FixedExpINTEL\0" // IR(43336, 14)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(43350, 29)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(43379, 29)
  "ReadPipeBlockingINTEL\0" // IR(43408, 22)
  "WritePipeBlockingINTEL\0" // IR(43430, 23)
  "RayQueryGetRayTMinKHR\0" // IR(43453, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(43475, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(43498, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(43526, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(43572, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(43609, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(43674, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(43714, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(43755, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(43794, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(43830, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(43876, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(43921, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(43963, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(43995, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(44024, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(44064, 40)
  "AtomicFAddEXT\0" // IR(44104, 14)
  "TypeBufferSurfaceINTEL\0" // IR(44118, 23)
  "TypeStructContinuedINTEL\0" // IR(44141, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(44166, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(44198, 36)
  "CompositeConstructContinuedINTEL\0" // IR(44234, 33)
  "ConvertFToBF16INTEL\0" // IR(44267, 20)
  "ConvertBF16ToFINTEL\0" // IR(44287, 20)
  "ControlBarrierArriveINTEL\0" // IR(44307, 26)
  "ControlBarrierWaitINTEL\0" // IR(44333, 24)
  "TaskSequenceCreateINTEL\0" // IR(44357, 24)
  "TaskSequenceAsyncINTEL\0" // IR(44381, 23)
  "TaskSequenceGetINTEL\0" // IR(44404, 21)
  "TaskSequenceReleaseINTEL\0" // IR(44425, 25)
  "TypeTaskSequenceINTEL\0" // IR(44450, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(44472, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(44499, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(44524, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(44558, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(44592, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(44621, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(44647, 62)
  "BitwiseFunctionINTEL\0" // IR(44709, 21)
  "GroupIMulKHR\0" // IR(44730, 13)
  "GroupFMulKHR\0" // IR(44743, 13)
  "GroupBitwiseAndKHR\0" // IR(44756, 19)
  "GroupBitwiseOrKHR\0" // IR(44775, 18)
  "GroupBitwiseXorKHR\0" // IR(44793, 19)
  "GroupLogicalAndKHR\0" // IR(44812, 19)
  "GroupLogicalOrKHR\0" // IR(44831, 18)
  "GroupLogicalXorKHR\0" // IR(44849, 19)
  "RoundFToTF32INTEL\0" // IR(44868, 18)
  "MaskedGatherINTEL\0" // IR(44886, 18)
  "MaskedScatterINTEL\0" // IR(44904, 19)
  "ConvertHandleToImageINTEL\0" // IR(44923, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(44949, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(44977, 33)
  "DebugInfoNone\0" // IR(45010, 14)
  "DebugCompilationUnit\0" // IR(45024, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(45045, 52)
  "DebugTypeBasic\0" // IR(45097, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(45112, 34)
  "DebugTypePointer\0" // IR(45146, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(45163, 38)
  "DebugTypeQualifier\0" // IR(45201, 19)
  "DebugTypeArray\0" // IR(45220, 15)
  "DebugTypeVector\0" // IR(45235, 16)
  "DebugTypedef\0" // IR(45251, 13)
  "DebugTypeFunction\0" // IR(45264, 18)
  "DebugTypeEnum\0" // IR(45282, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(45296, 38)
  "DebugTypeComposite\0" // IR(45334, 19)
  "DebugTypeMember\0" // IR(45353, 16)
  "DebugTypeInheritance\0" // IR(45369, 21)
  "DebugTypePtrToMember\0" // IR(45390, 21)
  "DebugTypeTemplate\0" // IR(45411, 18)
  "DebugTypeTemplateParameter\0" // IR(45429, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(45456, 35)
  "DebugTypeTemplateParameterPack\0" // IR(45491, 31)
  "DebugGlobalVariable\0" // IR(45522, 20)
  "DebugFunctionDeclaration\0" // IR(45542, 25)
  "DebugFunction\0" // IR(45567, 14)
  "DebugLexicalBlock\0" // IR(45581, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(45599, 31)
  "DebugScope\0" // IR(45630, 11)
  "DebugNoScope\0" // IR(45641, 13)
  "DebugInlinedAt\0" // IR(45654, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(45669, 42)
  "DebugLocalVariable\0" // IR(45711, 19)
  "DebugInlinedVariable\0" // IR(45730, 21)
  "DebugDeclare\0" // IR(45751, 13)
  "DebugValue\0" // IR(45764, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(45775, 33)
  "DebugOperation\0" // IR(45808, 15)
  "DebugExpression\0" // IR(45823, 16)
  "DebugMacroDef\0" // IR(45839, 14)
  "DebugMacroUndef\0" // IR(45853, 16)
  "Round\0" // IR(45869, 6)
  "RoundEven\0" // IR(45875, 10)
  "Trunc\0" // IR(45885, 6)
  "FAbs\0" // IR(45891, 5)
  "SAbs\0" // IR(45896, 5)
  "FSign\0" // IR(45901, 6)
  "SSign\0" // IR(45907, 6)
  "Floor\0" // IR(45913, 6)
  "Ceil\0" // IR(45919, 5)
  "Fract\0" // IR(45924, 6)
  "Radians\0" // IR(45930, 8)
  "Degrees\0" // IR(45938, 8)
  "Sin\0" // IR(45946, 4)
  "Cos\0" // IR(45950, 4)
  "Tan\0" // IR(45954, 4)
  "Asin\0" // IR(45958, 5)
  "Acos\0" // IR(45963, 5)
  "Atan\0" // IR(45968, 5)
  "Sinh\0" // IR(45973, 5)
  "Cosh\0" // IR(45978, 5)
  "Tanh\0" // IR(45983, 5)
  "Asinh\0" // IR(45988, 6)
  "Acosh\0" // IR(45994, 6)
  "Atanh\0" // IR(46000, 6)
  "Atan2\0" // IR(46006, 6)
  "Pow\0" // IR(46012, 4)
  "Exp\0" // IR(46016, 4)
  "Log\0" // IR(46020, 4)
  "Exp2\0" // IR(46024, 5)
  "Log2\0" // IR(46029, 5)
  "Sqrt\0" // IR(46034, 5)
  "InverseSqrt\0" // IR(46039, 12)
  "Determinant\0" // IR(46051, 12)
  "MatrixInverse\0" // IR(46063, 14)
  "Modf\0" // IR(46077, 5)
  "ModfStruct\0" // IR(46082, 11)
  "FMin\0" // IR(46093, 5)
  "UMin\0" // IR(46098, 5)
  "SMin\0" // IR(46103, 5)
  "FMax\0" // IR(46108, 5)
  "UMax\0" // IR(46113, 5)
  "SMax\0" // IR(46118, 5)
  "FClamp\0" // IR(46123, 7)
  "UClamp\0" // IR(46130, 7)
  "SClamp\0" // IR(46137, 7)
  "FMix\0" // IR(46144, 5)
  "IMix\0" // IR(46149, 5)
  "Step\0" // IR(46154, 5)
  "SmoothStep\0" // IR(46159, 11)
  "Fma\0" // IR(46170, 4)
  "Frexp\0" // IR(46174, 6)
  "FrexpStruct\0" // IR(46180, 12)
  "Ldexp\0" // IR(46192, 6)
  "PackSnorm4x8\0" // IR(46198, 13)
  "PackUnorm4x8\0" // IR(46211, 13)
  "PackSnorm2x16\0" // IR(46224, 14)
  "PackUnorm2x16\0" // IR(46238, 14)
  "PackHalf2x16\0" // IR(46252, 13)
  "PackDouble2x32\0" // IR(46265, 15)
  "UnpackSnorm2x16\0" // IR(46280, 16)
  "UnpackUnorm2x16\0" // IR(46296, 16)
  "UnpackHalf2x16\0" // IR(46312, 15)
  "UnpackSnorm4x8\0" // IR(46327, 15)
  "UnpackUnorm4x8\0" // IR(46342, 15)
  "UnpackDouble2x32\0" // IR(46357, 17)
  "Length\0" // IR(46374, 7)
  "Distance\0" // IR(46381, 9)
  "Cross\0" // IR(46390, 6)
  "Normalize\0" // IR(46396, 10)
  "FaceForward\0" // IR(46406, 12)
  "Reflect\0" // IR(46418, 8)
  "Refract\0" // IR(46426, 8)
  "FindILsb\0" // IR(46434, 9)
  "FindSMsb\0" // IR(46443, 9)
  "FindUMsb\0" // IR(46452, 9)
  "InterpolateAtCentroid\0" // IR(46461, 22)
  "InterpolateAtSample\0" // IR(46483, 20)
  "InterpolateAtOffset\0" // IR(46503, 20)
  "NMin\0" // IR(46523, 5)
  "NMax\0" // IR(46528, 5)
  "NClamp\0" // IR(46533, 7)
  "ArgumentInfo\0" // IR(46540, 13)
  "ArgumentStorageBuffer\0" // IR(46553, 22)
  "ArgumentUniform\0" // IR(46575, 16)
  "ArgumentPodStorageBuffer\0" // IR(46591, 25)
  "ArgumentPodUniform\0" // IR(46616, 19)
  "ArgumentPodPushConstant\0" // IR(46635, 24)
  "ArgumentSampledImage\0" // IR(46659, 21)
  "ArgumentStorageImage\0" // IR(46680, 21)
  "ArgumentSampler\0" // IR(46701, 16)
  "ArgumentWorkgroup\0" // IR(46717, 18)
  "SpecConstantWorkgroupSize\0" // IR(46735, 26)
  "SpecConstantGlobalOffset\0" // IR(46761, 25)
  "SpecConstantWorkDim\0" // IR(46786, 20)
  "PushConstantGlobalOffset\0" // IR(46806, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(46831, 30)
  "PushConstantGlobalSize\0" // IR(46861, 23)
  "PushConstantRegionOffset\0" // IR(46884, 25)
  "PushConstantNumWorkgroups\0" // IR(46909, 26)
  "PushConstantRegionGroupOffset\0" // IR(46935, 30)
  "ConstantDataStorageBuffer\0" // IR(46965, 26)
  "ConstantDataUniform\0" // IR(46991, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(47011, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(47041, 28)
  "ArgumentPointerPushConstant\0" // IR(47069, 28)
  "ArgumentPointerUniform\0" // IR(47097, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(47120, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(47155, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(47193, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(47235, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(47280, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(47317, 40)
  "ArgumentStorageTexelBuffer\0" // IR(47357, 27)
  "ArgumentUniformTexelBuffer\0" // IR(47384, 27)
  "ConstantDataPointerPushConstant\0" // IR(47411, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(47443, 40)
  "PrintfInfo\0" // IR(47483, 11)
  "PrintfBufferStorageBuffer\0" // IR(47494, 26)
  "PrintfBufferPointerPushConstant\0" // IR(47520, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(47552, 34)
  "WorkgroupVariableSize\0" // IR(47586, 22)
  "DebugImportedEntity\0" // IR(47608, 20)
  "DebugSource\0" // IR(47628, 12)
  "DebugFunctionDefinition\0" // IR(47640, 24)
  "DebugSourceContinued\0" // IR(47664, 21)
  "DebugLine\0" // IR(47685, 10)
  "DebugNoLine\0" // IR(47695, 12)
  "DebugBuildIdentifier\0" // IR(47707, 21)
  "DebugStoragePath\0" // IR(47728, 17)
  "DebugEntryPoint\0" // IR(47745, 16)
  "DebugTypeMatrix\0" // IR(47761, 16)
  "Configuration\0" // IR(47777, 14)
  "StartCounter\0" // IR(47791, 13)
  "StopCounter\0" // IR(47804, 12)
  "PushConstants\0" // IR(47816, 14)
  "SpecializationMapEntry\0" // IR(47830, 23)
  "DescriptorSetBuffer\0" // IR(47853, 20)
  "DescriptorSetImage\0" // IR(47873, 19)
  "DescriptorSetSampler\0" // IR(47892, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(47913, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(47976, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(48021, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(48070, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(48119, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(48163, 50)
  "DebugModuleINTEL\0" // IR(48213, 17)
  "acos\0" // IR(48230, 5)
  "acosh\0" // IR(48235, 6)
  "acospi\0" // IR(48241, 7)
  "asin\0" // IR(48248, 5)
  "asinh\0" // IR(48253, 6)
  "asinpi\0" // IR(48259, 7)
  "atan\0" // IR(48266, 5)
  "atan2\0" // IR(48271, 6)
  "atanh\0" // IR(48277, 6)
  "atanpi\0" // IR(48283, 7)
  "atan2pi\0" // IR(48290, 8)
  "cbrt\0" // IR(48298, 5)
  "ceil\0" // IR(48303, 5)
  "copysign\0" // IR(48308, 9)
  "cos\0" // IR(48317, 4)
  "cosh\0" // IR(48321, 5)
  "cospi\0" // IR(48326, 6)
  "erfc\0" // IR(48332, 5)
  "erf\0" // IR(48337, 4)
  "exp\0" // IR(48341, 4)
  "exp2\0" // IR(48345, 5)
  "exp10\0" // IR(48350, 6)
  "expm1\0" // IR(48356, 6)
  "fabs\0" // IR(48362, 5)
  "fdim\0" // IR(48367, 5)
  "floor\0" // IR(48372, 6)
  "fma\0" // IR(48378, 4)
  "fmax\0" // IR(48382, 5)
  "fmin\0" // IR(48387, 5)
  "fmod\0" // IR(48392, 5)
  "fract\0" // IR(48397, 6)
  "frexp\0" // IR(48403, 6)
  "hypot\0" // IR(48409, 6)
  "ilogb\0" // IR(48415, 6)
  "ldexp\0" // IR(48421, 6)
  "lgamma\0" // IR(48427, 7)
  "lgamma_r\0" // IR(48434, 9)
  "log\0" // IR(48443, 4)
  "log2\0" // IR(48447, 5)
  "log10\0" // IR(48452, 6)
  "log1p\0" // IR(48458, 6)
  "logb\0" // IR(48464, 5)
  "mad\0" // IR(48469, 4)
  "maxmag\0" // IR(48473, 7)
  "minmag\0" // IR(48480, 7)
  "modf\0" // IR(48487, 5)
  "nan\0" // IR(48492, 4)
  "nextafter\0" // IR(48496, 10)
  "pow\0" // IR(48506, 4)
  "pown\0" // IR(48510, 5)
  "powr\0" // IR(48515, 5)
  "remainder\0" // IR(48520, 10)
  "remquo\0" // IR(48530, 7)
  "rint\0" // IR(48537, 5)
  "rootn\0" // IR(48542, 6)
  "round\0" // IR(48548, 6)
  "rsqrt\0" // IR(48554, 6)
  "sin\0" // IR(48560, 4)
  "sincos\0" // IR(48564, 7)
  "sinh\0" // IR(48571, 5)
  "sinpi\0" // IR(48576, 6)
  "sqrt\0" // IR(48582, 5)
  "tan\0" // IR(48587, 4)
  "tanh\0" // IR(48591, 5)
  "tanpi\0" // IR(48596, 6)
  "tgamma\0" // IR(48602, 7)
  "trunc\0" // IR(48609, 6)
  "half_cos\0" // IR(48615, 9)
  "half_divide\0" // IR(48624, 12)
  "half_exp\0" // IR(48636, 9)
  "half_exp2\0" // IR(48645, 10)
  "half_exp10\0" // IR(48655, 11)
  "half_log\0" // IR(48666, 9)
  "half_log2\0" // IR(48675, 10)
  "half_log10\0" // IR(48685, 11)
  "half_powr\0" // IR(48696, 10)
  "half_recip\0" // IR(48706, 11)
  "half_rsqrt\0" // IR(48717, 11)
  "half_sin\0" // IR(48728, 9)
  "half_sqrt\0" // IR(48737, 10)
  "half_tan\0" // IR(48747, 9)
  "native_cos\0" // IR(48756, 11)
  "native_divide\0" // IR(48767, 14)
  "native_exp\0" // IR(48781, 11)
  "native_exp2\0" // IR(48792, 12)
  "native_exp10\0" // IR(48804, 13)
  "native_log\0" // IR(48817, 11)
  "native_log2\0" // IR(48828, 12)
  "native_log10\0" // IR(48840, 13)
  "native_powr\0" // IR(48853, 12)
  "native_recip\0" // IR(48865, 13)
  "native_rsqrt\0" // IR(48878, 13)
  "native_sin\0" // IR(48891, 11)
  "native_sqrt\0" // IR(48902, 12)
  "native_tan\0" // IR(48914, 11)
  "fclamp\0" // IR(48925, 7)
  "degrees\0" // IR(48932, 8)
  "fmax_common\0" // IR(48940, 12)
  "fmin_common\0" // IR(48952, 12)
  "mix\0" // IR(48964, 4)
  "radians\0" // IR(48968, 8)
  "step\0" // IR(48976, 5)
  "smoothstep\0" // IR(48981, 11)
  "sign\0" // IR(48992, 5)
  "cross\0" // IR(48997, 6)
  "distance\0" // IR(49003, 9)
  "length\0" // IR(49012, 7)
  "normalize\0" // IR(49019, 10)
  "fast_distance\0" // IR(49029, 14)
  "fast_length\0" // IR(49043, 12)
  "fast_normalize\0" // IR(49055, 15)
  "s_abs\0" // IR(49070, 6)
  "s_abs_diff\0" // IR(49076, 11)
  "s_add_sat\0" // IR(49087, 10)
  "u_add_sat\0" // IR(49097, 10)
  "s_hadd\0" // IR(49107, 7)
  "u_hadd\0" // IR(49114, 7)
  "s_rhadd\0" // IR(49121, 8)
  "u_rhadd\0" // IR(49129, 8)
  "s_clamp\0" // IR(49137, 8)
  "u_clamp\0" // IR(49145, 8)
  "clz\0" // IR(49153, 4)
  "ctz\0" // IR(49157, 4)
  "s_mad_hi\0" // IR(49161, 9)
  "u_mad_sat\0" // IR(49170, 10)
  "s_mad_sat\0" // IR(49180, 10)
  "s_max\0" // IR(49190, 6)
  "u_max\0" // IR(49196, 6)
  "s_min\0" // IR(49202, 6)
  "u_min\0" // IR(49208, 6)
  "s_mul_hi\0" // IR(49214, 9)
  "rotate\0" // IR(49223, 7)
  "s_sub_sat\0" // IR(49230, 10)
  "u_sub_sat\0" // IR(49240, 10)
  "u_upsample\0" // IR(49250, 11)
  "s_upsample\0" // IR(49261, 11)
  "popcount\0" // IR(49272, 9)
  "s_mad24\0" // IR(49281, 8)
  "u_mad24\0" // IR(49289, 8)
  "s_mul24\0" // IR(49297, 8)
  "u_mul24\0" // IR(49305, 8)
  "vloadn\0" // IR(49313, 7)
  "vstoren\0" // IR(49320, 8)
  "vload_half\0" // IR(49328, 11)
  "vload_halfn\0" // IR(49339, 12)
  "vstore_half\0" // IR(49351, 12)
  "vstore_half_r\0" // IR(49363, 14)
  "vstore_halfn\0" // IR(49377, 13)
  "vstore_halfn_r\0" // IR(49390, 15)
  "vloada_halfn\0" // IR(49405, 13)
  "vstorea_halfn\0" // IR(49418, 14)
  "vstorea_halfn_r\0" // IR(49432, 16)
  "shuffle\0" // IR(49448, 8)
  "shuffle2\0" // IR(49456, 9)
  "printf\0" // IR(49465, 7)
  "prefetch\0" // IR(49472, 9)
  "bitselect\0" // IR(49481, 10)
  "select\0" // IR(49491, 7)
  "u_abs\0" // IR(49498, 6)
  "u_abs_diff\0" // IR(49504, 11)
  "u_mul_hi\0" // IR(49515, 9)
  "u_mad_hi\0" // IR(49524, 9)
  "CubeFaceIndexAMD\0" // IR(49533, 17)
  "CubeFaceCoordAMD\0" // IR(49550, 17)
  "TimeAMD\0" // IR(49567, 8)
  "SwizzleInvocationsAMD\0" // IR(49575, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(49597, 28)
  "WriteInvocationAMD\0" // IR(49625, 19)
  "MbcntAMD\0" // IR(49644, 9)
  "InterpolateAtVertexAMD\0" // IR(49653, 23)
  "FMin3AMD\0" // IR(49676, 9)
  "UMin3AMD\0" // IR(49685, 9)
  "SMin3AMD\0" // IR(49694, 9)
  "FMax3AMD\0" // IR(49703, 9)
  "UMax3AMD\0" // IR(49712, 9)
  "SMax3AMD\0" // IR(49721, 9)
  "FMid3AMD\0" // IR(49730, 9)
  "UMid3AMD\0" // IR(49739, 9)
  "SMid3AMD\0" // IR(49748, 9)
  "ARGMAX\0" // IR(49757, 7)
  "AVG_POOL2D\0" // IR(49764, 11)
  "CONV2D\0" // IR(49775, 7)
  "CONV3D\0" // IR(49782, 7)
  "DEPTHWISE_CONV2D\0" // IR(49789, 17)
  "FFT2D\0" // IR(49806, 6)
  "MATMUL\0" // IR(49812, 7)
  "MAX_POOL2D\0" // IR(49819, 11)
  "RFFT2D\0" // IR(49830, 7)
  "TRANSPOSE_CONV2D\0" // IR(49837, 17)
  "CLAMP\0" // IR(49854, 6)
  "ERF\0" // IR(49860, 4)
  "SIGMOID\0" // IR(49864, 8)
  "TANH\0" // IR(49872, 5)
  "ADD\0" // IR(49877, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(49881, 23)
  "BITWISE_AND\0" // IR(49904, 12)
  "BITWISE_OR\0" // IR(49916, 11)
  "BITWISE_XOR\0" // IR(49927, 12)
  "INTDIV\0" // IR(49939, 7)
  "LOGICAL_AND\0" // IR(49946, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(49958, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(49977, 20)
  "LOGICAL_OR\0" // IR(49997, 11)
  "LOGICAL_XOR\0" // IR(50008, 12)
  "MAXIMUM\0" // IR(50020, 8)
  "MINIMUM\0" // IR(50028, 8)
  "MUL\0" // IR(50036, 4)
  "POW\0" // IR(50040, 4)
  "SUB\0" // IR(50044, 4)
  "TABLE\0" // IR(50048, 6)
  "ABS\0" // IR(50054, 4)
  "BITWISE_NOT\0" // IR(50058, 12)
  "CEIL\0" // IR(50070, 5)
  "CLZ\0" // IR(50075, 4)
  "COS\0" // IR(50079, 4)
  "EXP\0" // IR(50083, 4)
  "FLOOR\0" // IR(50087, 6)
  "LOG\0" // IR(50093, 4)
  "LOGICAL_NOT\0" // IR(50097, 12)
  "NEGATE\0" // IR(50109, 7)
  "RECIPROCAL\0" // IR(50116, 11)
  "RSQRT\0" // IR(50127, 6)
  "SIN\0" // IR(50133, 4)
  "SELECT\0" // IR(50137, 7)
  "EQUAL\0" // IR(50144, 6)
  "GREATER\0" // IR(50150, 8)
  "GREATER_EQUAL\0" // IR(50158, 14)
  "REDUCE_ALL\0" // IR(50172, 11)
  "REDUCE_ANY\0" // IR(50183, 11)
  "REDUCE_MAX\0" // IR(50194, 11)
  "REDUCE_MIN\0" // IR(50205, 11)
  "REDUCE_PRODUCT\0" // IR(50216, 15)
  "REDUCE_SUM\0" // IR(50231, 11)
  "CONCAT\0" // IR(50242, 7)
  "PAD\0" // IR(50249, 4)
  "RESHAPE\0" // IR(50253, 8)
  "REVERSE\0" // IR(50261, 8)
  "SLICE\0" // IR(50269, 6)
  "TILE\0" // IR(50275, 5)
  "TRANSPOSE\0" // IR(50280, 10)
  "GATHER\0" // IR(50290, 7)
  "SCATTER\0" // IR(50297, 8)
  "RESIZE\0" // IR(50305, 7)
  "CAST\0" // IR(50312, 5)
  "RESCALE\0" // IR(50317, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(4654, 27), // 0 PhysicalStorageBuffer64EXT
  IR(5565, 18), // 1 SubgroupEqMaskKHR
  IR(5620, 18), // 2 SubgroupGeMaskKHR
  IR(5653, 18), // 3 SubgroupGtMaskKHR
  IR(5686, 18), // 4 SubgroupLeMaskKHR
  IR(5719, 18), // 5 SubgroupLtMaskKHR
  IR(6577, 12), // 6 BaryCoordNV
  IR(6632, 19), // 7 BaryCoordNoPerspNV
  IR(6663, 15), // 8 FragmentSizeNV
  IR(6720, 22), // 9 InvocationsPerPixelNV
  IR(6848, 11), // 10 LaunchIdNV
  IR(6873, 13), // 11 LaunchSizeNV
  IR(6904, 17), // 12 WorldRayOriginNV
  IR(6942, 20), // 13 WorldRayDirectionNV
  IR(6981, 18), // 14 ObjectRayOriginNV
  IR(7021, 21), // 15 ObjectRayDirectionNV
  IR(7053, 10), // 16 RayTminNV
  IR(7074, 10), // 17 RayTmaxNV
  IR(7107, 22), // 18 InstanceCustomIndexNV
  IR(7146, 16), // 19 ObjectToWorldNV
  IR(7179, 16), // 20 WorldToObjectNV
  IR(7213, 10), // 21 HitKindNV
  IR(7444, 19), // 22 IncomingRayFlagsNV
  IR(9220, 28), // 23 StorageUniformBufferBlock16
  IR(9283, 17), // 24 StorageUniform16
  IR(10066, 27), // 25 ShaderViewportIndexLayerNV
  IR(10110, 22), // 26 FragmentBarycentricNV
  IR(10163, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(10193, 14), // 28 ShadingRateNV
  IR(10253, 20), // 29 ShaderNonUniformEXT
  IR(10296, 26), // 30 RuntimeDescriptorArrayEXT
  IR(10358, 39), // 31 InputAttachmentArrayDynamicIndexingEXT
  IR(10436, 42), // 32 UniformTexelBufferArrayDynamicIndexingEXT
  IR(10517, 42), // 33 StorageTexelBufferArrayDynamicIndexingEXT
  IR(10596, 40), // 34 UniformBufferArrayNonUniformIndexingEXT
  IR(10672, 39), // 35 SampledImageArrayNonUniformIndexingEXT
  IR(10748, 40), // 36 StorageBufferArrayNonUniformIndexingEXT
  IR(10824, 39), // 37 StorageImageArrayNonUniformIndexingEXT
  IR(10902, 42), // 38 InputAttachmentArrayNonUniformIndexingEXT
  IR(10986, 45), // 39 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(11073, 45), // 40 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(11136, 21), // 41 VulkanMemoryModelKHR
  IR(11186, 32), // 42 VulkanMemoryModelDeviceScopeKHR
  IR(11218, 34), // 43 PhysicalStorageBufferAddressesEXT
  IR(11284, 31), // 44 ComputeDerivativeGroupLinearNV
  IR(11488, 28), // 45 DemoteToHelperInvocationEXT
  IR(12958, 22), // 46 DotProductInputAllKHR
  IR(13002, 25), // 47 DotProductInput4x8BitKHR
  IR(13055, 31), // 48 DotProductInput4x8BitPackedKHR
  IR(13097, 14), // 49 DotProductKHR
  IR(13262, 13), // 50 OptNoneINTEL
  IR(16258, 15), // 51 PerPrimitiveNV
  IR(16306, 12), // 52 PerVertexNV
  IR(16329, 14), // 53 NonUniformEXT
  IR(16359, 19), // 54 RestrictPointerEXT
  IR(16393, 18), // 55 AliasedPointerEXT
  IR(16703, 24), // 56 HlslCounterBufferGOOGLE
  IR(16740, 19), // 57 HlslSemanticGOOGLE
  IR(19414, 14), // 58 OutputLinesNV
  IR(19448, 19), // 59 OutputPrimitivesNV
  IR(19491, 23), // 60 DerivativeGroupQuadsNV
  IR(19539, 24), // 61 DerivativeGroupLinearNV
  IR(19582, 18), // 62 OutputTrianglesNV
  IR(20330, 16), // 63 RayGenerationNV
  IR(20362, 15), // 64 IntersectionNV
  IR(20387, 9), // 65 AnyHitNV
  IR(20410, 13), // 66 ClosestHitNV
  IR(20431, 7), // 67 MissNV
  IR(20450, 11), // 68 CallableNV
  IR(20595, 23), // 69 AllowContractFastINTEL
  IR(20631, 18), // 70 AllowReassocINTEL
  IR(21085, 22), // 71 MakeTexelAvailableKHR
  IR(21124, 20), // 72 MakeTexelVisibleKHR
  IR(21160, 19), // 73 NonPrivateTexelKHR
  IR(21193, 17), // 74 VolatileTexelKHR
  IR(22570, 24), // 75 MakePointerAvailableKHR
  IR(22613, 22), // 76 MakePointerVisibleKHR
  IR(22653, 21), // 77 NonPrivatePointerKHR
  IR(22740, 10), // 78 VulkanKHR
  IR(14030, 5), // 79 None
  IR(22923, 16), // 80 OutputMemoryKHR
  IR(22953, 17), // 81 MakeAvailableKHR
  IR(22982, 15), // 82 MakeVisibleKHR
  IR(23058, 28), // 83 PackedVectorFormat4x8BitKHR
  IR(23373, 24), // 84 SkipBuiltinPrimitivesNV
  IR(24113, 15), // 85 QueueFamilyKHR
  IR(24484, 15), // 86 CallableDataNV
  IR(24523, 23), // 87 IncomingCallableDataNV
  IR(24560, 13), // 88 RayPayloadNV
  IR(24589, 15), // 89 HitAttributeNV
  IR(24626, 21), // 90 IncomingRayPayloadNV
  IR(24669, 21), // 91 ShaderRecordBufferNV
  IR(24712, 25), // 92 PhysicalStorageBufferEXT
  IR(32021, 8), // 93 SDotKHR
  IR(32034, 8), // 94 UDotKHR
  IR(32048, 9), // 95 SUDotKHR
  IR(32068, 14), // 96 SDotAccSatKHR
  IR(32093, 14), // 97 UDotAccSatKHR
  IR(32119, 15), // 98 SUDotAccSatKHR
  IR(34604, 21), // 99 ReportIntersectionNV
  IR(34779, 28), // 100 TypeAccelerationStructureNV
  IR(36828, 21), // 101 DecorateStringGOOGLE
  IR(36870, 27), // 102 MemberDecorateStringGOOGLE
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
  spv::Capability::Int4TypeINTEL, // 86
  spv::Capability::CooperativeMatrixKHR, // 87
  spv::Capability::BFloat16TypeKHR, // 88
  spv::Capability::BFloat16TypeKHR, // 89
  spv::Capability::CooperativeMatrixKHR, // 90
  spv::Capability::MultiViewport, // 91
  spv::Capability::ShaderViewportIndexLayerEXT, // 92
  spv::Capability::ShaderViewportMaskNV, // 93
  spv::Capability::InputAttachment, // 94
  spv::Capability::ImageBuffer, // 95
  spv::Capability::ShaderNonUniform, // 96
  spv::Capability::InputAttachment, // 97
  spv::Capability::ShaderNonUniform, // 98
  spv::Capability::SampledBuffer, // 99
  spv::Capability::ShaderNonUniform, // 100
  spv::Capability::ImageBuffer, // 101
  spv::Capability::ShaderNonUniform, // 102
  spv::Capability::VectorAnyINTEL, // 103
  spv::Capability::Int8, // 104
  spv::Capability::FPGAClusterAttributesINTEL, // 105
  spv::Capability::FPGAKernelAttributesINTEL, // 106
  spv::Capability::Subgroup2DBlockIOINTEL, // 107
  spv::Capability::Shader, // 108
  spv::Capability::Kernel, // 109
  spv::Capability::Shader, // 110
  spv::Capability::UniformDecoration, // 111
  spv::Capability::GeometryStreams, // 112
  spv::Capability::TransformFeedback, // 113
  spv::Capability::Kernel, // 114
  spv::Capability::FloatControls2, // 115
  spv::Capability::Linkage, // 116
  spv::Capability::Float8EXT, // 117
  spv::Capability::SampleMaskOverrideCoverageNV, // 118
  spv::Capability::GeometryShaderPassthroughNV, // 119
  spv::Capability::MeshShadingNV, // 120
  spv::Capability::MeshShadingEXT, // 121
  spv::Capability::ShaderInvocationReorderNV, // 122
  spv::Capability::BindlessTextureNV, // 123
  spv::Capability::VectorComputeINTEL, // 124
  spv::Capability::IndirectReferencesINTEL, // 125
  spv::Capability::AsmINTEL, // 126
  spv::Capability::FunctionFloatControlINTEL, // 127
  spv::Capability::FPGAMemoryAttributesINTEL, // 128
  spv::Capability::FPGAMemoryAccessesINTEL, // 129
  spv::Capability::LoopFuseINTEL, // 130
  spv::Capability::FPGADSPControlINTEL, // 131
  spv::Capability::MemoryAccessAliasingINTEL, // 132
  spv::Capability::FPGAInvocationPipeliningAttributesINTEL, // 133
  spv::Capability::FPGABufferLocationINTEL, // 134
  spv::Capability::IOPipesINTEL, // 135
  spv::Capability::FPGAClusterAttributesV2INTEL, // 136
  spv::Capability::FPMaxErrorINTEL, // 137
  spv::Capability::FPGALatencyControlINTEL, // 138
  spv::Capability::FPGAArgumentInterfacesINTEL, // 139
  spv::Capability::GlobalVariableHostAccessINTEL, // 140
  spv::Capability::GlobalVariableFPGADecorationsINTEL, // 141
  spv::Capability::CacheControlsINTEL, // 142
  spv::Capability::TileImageColorReadAccessEXT, // 143
  spv::Capability::Geometry, // 144
  spv::Capability::Tessellation, // 145
  spv::Capability::MeshShadingNV, // 146
  spv::Capability::MeshShadingEXT, // 147
  spv::Capability::Geometry, // 148
  spv::Capability::MeshShadingNV, // 149
  spv::Capability::MeshShadingEXT, // 150
  spv::Capability::SubgroupDispatch, // 151
  spv::Capability::TileImageDepthReadAccessEXT, // 152
  spv::Capability::TileImageStencilReadAccessEXT, // 153
  spv::Capability::SampleMaskPostDepthCoverage, // 154
  spv::Capability::DenormPreserve, // 155
  spv::Capability::DenormFlushToZero, // 156
  spv::Capability::SignedZeroInfNanPreserve, // 157
  spv::Capability::RoundingModeRTE, // 158
  spv::Capability::RoundingModeRTZ, // 159
  spv::Capability::QuadControlKHR, // 160
  spv::Capability::ComputeDerivativeGroupQuadsNV, // 161
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 162
  spv::Capability::ComputeDerivativeGroupLinearNV, // 163
  spv::Capability::ComputeDerivativeGroupLinearKHR, // 164
  spv::Capability::FragmentShaderPixelInterlockEXT, // 165
  spv::Capability::FragmentShaderSampleInterlockEXT, // 166
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 167
  spv::Capability::RoundToInfinityINTEL, // 168
  spv::Capability::KernelAttributesINTEL, // 169
  spv::Capability::FloatControls2, // 170
  spv::Capability::FPGAKernelAttributesv2INTEL, // 171
  spv::Capability::RegisterLimitsINTEL, // 172
  spv::Capability::FloatControls2, // 173
  spv::Capability::FPFastMathModeINTEL, // 174
  spv::Capability::OptNoneEXT, // 175
  spv::Capability::RuntimeAlignedAttributeINTEL, // 176
  spv::Capability::Kernel, // 177
  spv::Capability::GroupNonUniformArithmetic, // 178
  spv::Capability::GroupNonUniformBallot, // 179
  spv::Capability::GroupNonUniformClustered, // 180
  spv::Capability::GroupNonUniformPartitionedNV, // 181
  spv::Capability::ImageGatherExtended, // 182
  spv::Capability::MinLod, // 183
  spv::Capability::VulkanMemoryModel, // 184
  spv::Capability::FPGALoopControlsINTEL, // 185
  spv::Capability::ArbitraryPrecisionFixedPointINTEL, // 186
  spv::Capability::RawAccessChainsNV, // 187
  spv::Capability::RayTraversalPrimitiveCullingKHR, // 188
  spv::Capability::RayTracingOpacityMicromapEXT, // 189
  spv::Capability::RayQueryKHR, // 190
  spv::Capability::StorageImageExtendedFormats, // 191
  spv::Capability::Int64ImageEXT, // 192
  spv::Capability::Shader, // 193
  spv::Capability::VectorComputeINTEL, // 194
  spv::Capability::GenericPointer, // 195
  spv::Capability::FunctionPointersINTEL, // 196
  spv::Capability::USMStorageClassesINTEL, // 197
  spv::Capability::CooperativeMatrixTensorAddressingNV, // 198
  spv::Capability::CooperativeMatrixBlockLoadsNV, // 199
  spv::Capability::TensorsARM, // 200
  spv::Capability::Addresses, // 201
  spv::Capability::PhysicalStorageBufferAddresses, // 202
  spv::Capability::LiteralSampler, // 203
  spv::Capability::Addresses, // 204
  spv::Capability::UntypedPointersKHR, // 205
  spv::Capability::Addresses, // 206
  spv::Capability::VariablePointers, // 207
  spv::Capability::VariablePointersStorageBuffer, // 208
  spv::Capability::PhysicalStorageBufferAddresses, // 209
  spv::Capability::Kernel, // 210
  spv::Capability::ImageQuery, // 211
  spv::Capability::ImageQuery, // 212
  spv::Capability::Shader, // 213
  spv::Capability::BitInstructions, // 214
  spv::Capability::DerivativeControl, // 215
  spv::Capability::Groups, // 216
  spv::Capability::SparseResidency, // 217
  spv::Capability::PipeStorage, // 218
  spv::Capability::NamedBarrier, // 219
  spv::Capability::GroupNonUniformVote, // 220
  spv::Capability::GroupNonUniformBallot, // 221
  spv::Capability::GroupNonUniformShuffle, // 222
  spv::Capability::GroupNonUniformShuffleRelative, // 223
  spv::Capability::GroupNonUniformArithmetic, // 224
  spv::Capability::GroupNonUniformClustered, // 225
  spv::Capability::GroupNonUniformPartitionedNV, // 226
  spv::Capability::GroupNonUniformQuad, // 227
  spv::Capability::Addresses, // 228
  spv::Capability::VariablePointers, // 229
  spv::Capability::VariablePointersStorageBuffer, // 230
  spv::Capability::GraphARM, // 231
  spv::Capability::UntypedPointersKHR, // 232
  spv::Capability::SubgroupBallotKHR, // 233
  spv::Capability::SubgroupVoteKHR, // 234
  spv::Capability::GroupNonUniformRotateKHR, // 235
  spv::Capability::RayTracingKHR, // 236
  spv::Capability::RayQueryKHR, // 237
  spv::Capability::DotProduct, // 238
  spv::Capability::CooperativeMatrixKHR, // 239
  spv::Capability::ReplicatedCompositesEXT, // 240
  spv::Capability::TextureSampleWeightedQCOM, // 241
  spv::Capability::TextureBoxFilterQCOM, // 242
  spv::Capability::TextureBlockMatchQCOM, // 243
  spv::Capability::TextureBlockMatch2QCOM, // 244
  spv::Capability::FragmentMaskAMD, // 245
  spv::Capability::ShaderClockKHR, // 246
  spv::Capability::ShaderInvocationReorderNV, // 247
  spv::Capability::RayTracingMotionBlurNV, // 248
  spv::Capability::ImageFootprintNV, // 249
  spv::Capability::CooperativeVectorNV, // 250
  spv::Capability::CooperativeVectorTrainingNV, // 251
  spv::Capability::CooperativeMatrixConversionsNV, // 252
  spv::Capability::DisplacementMicromapNV, // 253
  spv::Capability::RayQueryPositionFetchKHR, // 254
  spv::Capability::RayTracingNV, // 255
  spv::Capability::RayTracingKHR, // 256
  spv::Capability::RayQueryKHR, // 257
  spv::Capability::DisplacementMicromapNV, // 258
  spv::Capability::CooperativeMatrixNV, // 259
  spv::Capability::FragmentShaderSampleInterlockEXT, // 260
  spv::Capability::FragmentShaderPixelInterlockEXT, // 261
  spv::Capability::FragmentShaderShadingRateInterlockEXT, // 262
  spv::Capability::CooperativeMatrixReductionsNV, // 263
  spv::Capability::CooperativeMatrixPerElementOperationsNV, // 264
  spv::Capability::TensorAddressingNV, // 265
  spv::Capability::DemoteToHelperInvocation, // 266
  spv::Capability::SubgroupShuffleINTEL, // 267
  spv::Capability::SubgroupBufferBlockIOINTEL, // 268
  spv::Capability::SubgroupImageBlockIOINTEL, // 269
  spv::Capability::SubgroupImageMediaBlockIOINTEL, // 270
  spv::Capability::IntegerFunctions2INTEL, // 271
  spv::Capability::AtomicFloat16MinMaxEXT, // 272
  spv::Capability::AtomicFloat32MinMaxEXT, // 273
  spv::Capability::AtomicFloat64MinMaxEXT, // 274
  spv::Capability::AtomicFloat16VectorNV, // 275
  spv::Capability::ExpectAssumeKHR, // 276
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 277
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 278
  spv::Capability::SubgroupAvcMotionEstimationIntraINTEL, // 279
  spv::Capability::SubgroupAvcMotionEstimationINTEL, // 280
  spv::Capability::SubgroupAvcMotionEstimationChromaINTEL, // 281
  spv::Capability::VariableLengthArrayINTEL, // 282
  spv::Capability::ArbitraryPrecisionFloatingPointINTEL, // 283
  spv::Capability::UnstructuredLoopControlsINTEL, // 284
  spv::Capability::BlockingPipesINTEL, // 285
  spv::Capability::FPGARegINTEL, // 286
  spv::Capability::AtomicFloat16AddEXT, // 287
  spv::Capability::AtomicFloat32AddEXT, // 288
  spv::Capability::AtomicFloat64AddEXT, // 289
  spv::Capability::AtomicFloat16VectorNV, // 290
  spv::Capability::LongCompositesINTEL, // 291
  spv::Capability::BFloat16ConversionINTEL, // 292
  spv::Capability::SplitBarrierINTEL, // 293
  spv::Capability::ArithmeticFenceEXT, // 294
  spv::Capability::TaskSequenceINTEL, // 295
  spv::Capability::SubgroupBufferPrefetchINTEL, // 296
  spv::Capability::Subgroup2DBlockTransformINTEL, // 297
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 298
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 299
  spv::Capability::TernaryBitwiseFunctionINTEL, // 300
  spv::Capability::GroupUniformArithmeticKHR, // 301
  spv::Capability::TensorFloat32RoundingINTEL, // 302
  spv::Capability::MaskedGatherScatterINTEL, // 303
  spv::Capability::BindlessImagesINTEL, // 304
  spv::Capability::Float64, // 305
  spv::Capability::InterpolationFunction, // 306
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
  spvtools::Extension::kSPV_QCOM_image_processing2, // 54
  spvtools::Extension::kSPV_AMD_gpu_shader_half_float_fetch, // 55
  spvtools::Extension::kSPV_AMD_texture_gather_bias_lod, // 56
  spvtools::Extension::kSPV_AMD_shader_fragment_mask, // 57
  spvtools::Extension::kSPV_AMD_shader_image_load_store_lod, // 58
  spvtools::Extension::kSPV_EXT_shader_image_int64, // 59
  spvtools::Extension::kSPV_KHR_shader_clock, // 60
  spvtools::Extension::kSPV_AMDX_shader_enqueue, // 61
  spvtools::Extension::kSPV_KHR_quad_control, // 62
  spvtools::Extension::kSPV_INTEL_int4, // 63
  spvtools::Extension::kSPV_KHR_bfloat16, // 64
  spvtools::Extension::kSPV_NV_sample_mask_override_coverage, // 65
  spvtools::Extension::kSPV_NV_geometry_shader_passthrough, // 66
  spvtools::Extension::kSPV_EXT_shader_viewport_index_layer, // 67
  spvtools::Extension::kSPV_NV_viewport_array2, // 68
  spvtools::Extension::kSPV_NV_viewport_array2, // 69
  spvtools::Extension::kSPV_NVX_multiview_per_view_attributes, // 70
  spvtools::Extension::kSPV_NV_shader_image_footprint, // 71
  spvtools::Extension::kSPV_NV_compute_shader_derivatives, // 72
  spvtools::Extension::kSPV_KHR_compute_shader_derivatives, // 73
  spvtools::Extension::kSPV_NV_shader_subgroup_partitioned, // 74
  spvtools::Extension::kSPV_EXT_descriptor_indexing, // 75
  spvtools::Extension::kSPV_KHR_ray_tracing_position_fetch, // 76
  spvtools::Extension::kSPV_KHR_vulkan_memory_model, // 77
  spvtools::Extension::kSPV_NV_cooperative_matrix, // 78
  spvtools::Extension::kSPV_EXT_fragment_shader_interlock, // 79
  spvtools::Extension::kSPV_EXT_demote_to_helper_invocation, // 80
  spvtools::Extension::kSPV_NV_displacement_micromap, // 81
  spvtools::Extension::kSPV_EXT_opacity_micromap, // 82
  spvtools::Extension::kSPV_NV_shader_invocation_reorder, // 83
  spvtools::Extension::kSPV_NV_bindless_texture, // 84
  spvtools::Extension::kSPV_NV_cooperative_vector, // 85
  spvtools::Extension::kSPV_NV_shader_atomic_fp16_vector, // 86
  spvtools::Extension::kSPV_NV_raw_access_chains, // 87
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 88
  spvtools::Extension::kSPV_NV_tensor_addressing, // 89
  spvtools::Extension::kSPV_INTEL_subgroups, // 90
  spvtools::Extension::kSPV_INTEL_media_block_io, // 91
  spvtools::Extension::kSPV_INTEL_float_controls2, // 92
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 93
  spvtools::Extension::kSPV_INTEL_function_pointers, // 94
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 95
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 96
  spvtools::Extension::kSPV_INTEL_vector_compute, // 97
  spvtools::Extension::kSPV_KHR_expect_assume, // 98
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 99
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 100
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 101
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 102
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 103
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 104
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 105
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 106
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 107
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 108
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 109
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 110
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 111
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 112
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 113
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 114
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 115
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 116
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 117
  spvtools::Extension::kSPV_INTEL_io_pipes, // 118
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 119
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 120
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 121
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 122
  spvtools::Extension::kSPV_EXT_replicated_composites, // 123
  spvtools::Extension::kSPV_KHR_bit_instructions, // 124
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 125
  spvtools::Extension::kSPV_KHR_float_controls2, // 126
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 127
  spvtools::Extension::kSPV_INTEL_long_composites, // 128
  spvtools::Extension::kSPV_EXT_optnone, // 129
  spvtools::Extension::kSPV_INTEL_optnone, // 130
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 131
  spvtools::Extension::kSPV_INTEL_debug_module, // 132
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 133
  spvtools::Extension::kSPV_INTEL_split_barrier, // 134
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 135
  spvtools::Extension::kSPV_INTEL_task_sequence, // 136
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 137
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 138
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 139
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 140
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 141
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 142
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 143
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 144
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 145
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 146
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 147
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 148
  spvtools::Extension::kSPV_INTEL_cache_controls, // 149
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 150
  spvtools::Extension::kSPV_INTEL_bindless_images, // 151
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 152
  spvtools::Extension::kSPV_NV_mesh_shader, // 153
  spvtools::Extension::kSPV_EXT_mesh_shader, // 154
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 155
  spvtools::Extension::kSPV_GOOGLE_user_type, // 156
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 157
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 158
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 159
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 160
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 161
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 162
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 163
  spvtools::Extension::kSPV_KHR_variable_pointers, // 164
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 165
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 166
  spvtools::Extension::kSPV_KHR_ray_tracing, // 167
  spvtools::Extension::kSPV_KHR_ray_query, // 168
  spvtools::Extension::kSPV_NV_ray_tracing, // 169
  spvtools::Extension::kSPV_KHR_ray_tracing, // 170
  spvtools::Extension::kSPV_KHR_ray_query, // 171
  spvtools::Extension::kSPV_NV_displacement_micromap, // 172
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 173
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 174
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
  SPV_OPERAND_TYPE_ID, // 415
  SPV_OPERAND_TYPE_ID, // 416
  SPV_OPERAND_TYPE_ID, // 417
  SPV_OPERAND_TYPE_ID, // 418
  SPV_OPERAND_TYPE_ID, // 419
  SPV_OPERAND_TYPE_ID, // 420
  SPV_OPERAND_TYPE_ID, // 421
  SPV_OPERAND_TYPE_ID, // 422
  SPV_OPERAND_TYPE_ID, // 423
  SPV_OPERAND_TYPE_ID, // 424
  SPV_OPERAND_TYPE_ID, // 425
  SPV_OPERAND_TYPE_TYPE_ID, // 426
  SPV_OPERAND_TYPE_RESULT_ID, // 427
  SPV_OPERAND_TYPE_ID, // 428
  SPV_OPERAND_TYPE_ID, // 429
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 430
  SPV_OPERAND_TYPE_TYPE_ID, // 431
  SPV_OPERAND_TYPE_RESULT_ID, // 432
  SPV_OPERAND_TYPE_ID, // 433
  SPV_OPERAND_TYPE_ID, // 434
  SPV_OPERAND_TYPE_ID, // 435
  SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT, // 436
  SPV_OPERAND_TYPE_RESULT_ID, // 437
  SPV_OPERAND_TYPE_ID, // 438
  SPV_OPERAND_TYPE_SCOPE_ID, // 439
  SPV_OPERAND_TYPE_ID, // 440
  SPV_OPERAND_TYPE_ID, // 441
  SPV_OPERAND_TYPE_ID, // 442
  SPV_OPERAND_TYPE_TYPE_ID, // 443
  SPV_OPERAND_TYPE_RESULT_ID, // 444
  SPV_OPERAND_TYPE_ID, // 445
  SPV_OPERAND_TYPE_ID, // 446
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 447
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 448
  SPV_OPERAND_TYPE_ID, // 449
  SPV_OPERAND_TYPE_ID, // 450
  SPV_OPERAND_TYPE_ID, // 451
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 452
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 453
  SPV_OPERAND_TYPE_TYPE_ID, // 454
  SPV_OPERAND_TYPE_RESULT_ID, // 455
  SPV_OPERAND_TYPE_ID, // 456
  SPV_OPERAND_TYPE_ID, // 457
  SPV_OPERAND_TYPE_ID, // 458
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 459
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
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 521
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 522
  SPV_OPERAND_TYPE_TYPE_ID, // 523
  SPV_OPERAND_TYPE_RESULT_ID, // 524
  SPV_OPERAND_TYPE_ID, // 525
  SPV_OPERAND_TYPE_ID, // 526
  SPV_OPERAND_TYPE_ID, // 527
  SPV_OPERAND_TYPE_ID, // 528
  SPV_OPERAND_TYPE_OPTIONAL_IMAGE, // 529
  SPV_OPERAND_TYPE_TYPE_ID, // 530
  SPV_OPERAND_TYPE_RESULT_ID, // 531
  SPV_OPERAND_TYPE_ID, // 532
  SPV_OPERAND_TYPE_ID, // 533
  SPV_OPERAND_TYPE_ID, // 534
  SPV_OPERAND_TYPE_ID, // 535
  SPV_OPERAND_TYPE_ID, // 536
  SPV_OPERAND_TYPE_ID, // 537
  SPV_OPERAND_TYPE_ID, // 538
  SPV_OPERAND_TYPE_ID, // 539
  SPV_OPERAND_TYPE_ID, // 540
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 541
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 542
  SPV_OPERAND_TYPE_ID, // 543
  SPV_OPERAND_TYPE_ID, // 544
  SPV_OPERAND_TYPE_ID, // 545
  SPV_OPERAND_TYPE_ID, // 546
  SPV_OPERAND_TYPE_ID, // 547
  SPV_OPERAND_TYPE_ID, // 548
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 549
  SPV_OPERAND_TYPE_TYPE_ID, // 550
  SPV_OPERAND_TYPE_RESULT_ID, // 551
  SPV_OPERAND_TYPE_ID, // 552
  SPV_OPERAND_TYPE_ID, // 553
  SPV_OPERAND_TYPE_ID, // 554
  SPV_OPERAND_TYPE_ID, // 555
  SPV_OPERAND_TYPE_ID, // 556
  SPV_OPERAND_TYPE_ID, // 557
  SPV_OPERAND_TYPE_ID, // 558
  SPV_OPERAND_TYPE_ID, // 559
  SPV_OPERAND_TYPE_ID, // 560
  SPV_OPERAND_TYPE_ID, // 561
  SPV_OPERAND_TYPE_ID, // 562
  SPV_OPERAND_TYPE_ID, // 563
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 564
  SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS, // 565
  SPV_OPERAND_TYPE_ID, // 566
  SPV_OPERAND_TYPE_ID, // 567
  SPV_OPERAND_TYPE_ID, // 568
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 569
  SPV_OPERAND_TYPE_TYPE_ID, // 570
  SPV_OPERAND_TYPE_RESULT_ID, // 571
  SPV_OPERAND_TYPE_ID, // 572
  SPV_OPERAND_TYPE_ID, // 573
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 574
  SPV_OPERAND_TYPE_ID, // 575
  SPV_OPERAND_TYPE_ID, // 576
  SPV_OPERAND_TYPE_ID, // 577
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 578
  SPV_OPERAND_TYPE_RESULT_ID, // 579
  SPV_OPERAND_TYPE_ID, // 580
  SPV_OPERAND_TYPE_SCOPE_ID, // 581
  SPV_OPERAND_TYPE_ID, // 582
  SPV_OPERAND_TYPE_ID, // 583
  SPV_OPERAND_TYPE_TYPE_ID, // 584
  SPV_OPERAND_TYPE_RESULT_ID, // 585
  SPV_OPERAND_TYPE_ID, // 586
  SPV_OPERAND_TYPE_ID, // 587
  SPV_OPERAND_TYPE_ID, // 588
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 589
  SPV_OPERAND_TYPE_ID, // 590
  SPV_OPERAND_TYPE_ID, // 591
  SPV_OPERAND_TYPE_ID, // 592
  SPV_OPERAND_TYPE_ID, // 593
  SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, // 594
  SPV_OPERAND_TYPE_TYPE_ID, // 595
  SPV_OPERAND_TYPE_RESULT_ID, // 596
  SPV_OPERAND_TYPE_ID, // 597
  SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE, // 598
  SPV_OPERAND_TYPE_ID, // 599
  SPV_OPERAND_TYPE_TYPE_ID, // 600
  SPV_OPERAND_TYPE_RESULT_ID, // 601
  SPV_OPERAND_TYPE_ID, // 602
  SPV_OPERAND_TYPE_ID, // 603
  SPV_OPERAND_TYPE_ID, // 604
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 605
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 606
  SPV_OPERAND_TYPE_ID, // 607
  SPV_OPERAND_TYPE_ID, // 608
  SPV_OPERAND_TYPE_ID, // 609
  SPV_OPERAND_TYPE_MEMORY_ACCESS, // 610
  SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS, // 611
  SPV_OPERAND_TYPE_RESULT_ID, // 612
  SPV_OPERAND_TYPE_ID, // 613
  SPV_OPERAND_TYPE_ID, // 614
  SPV_OPERAND_TYPE_VARIABLE_ID, // 615
  SPV_OPERAND_TYPE_TYPE_ID, // 616
  SPV_OPERAND_TYPE_RESULT_ID, // 617
  SPV_OPERAND_TYPE_ID, // 618
  SPV_OPERAND_TYPE_ID, // 619
  SPV_OPERAND_TYPE_ID, // 620
  SPV_OPERAND_TYPE_ID, // 621
  SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS, // 622
  SPV_OPERAND_TYPE_ID, // 623
  SPV_OPERAND_TYPE_ID, // 624
  SPV_OPERAND_TYPE_ID, // 625
  SPV_OPERAND_TYPE_ID, // 626
  SPV_OPERAND_TYPE_ID, // 627
  SPV_OPERAND_TYPE_TYPE_ID, // 628
  SPV_OPERAND_TYPE_RESULT_ID, // 629
  SPV_OPERAND_TYPE_ID, // 630
  SPV_OPERAND_TYPE_ID, // 631
  SPV_OPERAND_TYPE_LITERAL_STRING, // 632
  SPV_OPERAND_TYPE_LITERAL_STRING, // 633
  SPV_OPERAND_TYPE_TYPE_ID, // 634
  SPV_OPERAND_TYPE_RESULT_ID, // 635
  SPV_OPERAND_TYPE_ID, // 636
  SPV_OPERAND_TYPE_ID, // 637
  SPV_OPERAND_TYPE_ID, // 638
  SPV_OPERAND_TYPE_ID, // 639
  SPV_OPERAND_TYPE_ID, // 640
  SPV_OPERAND_TYPE_ID, // 641
  SPV_OPERAND_TYPE_ID, // 642
  SPV_OPERAND_TYPE_TYPE_ID, // 643
  SPV_OPERAND_TYPE_RESULT_ID, // 644
  SPV_OPERAND_TYPE_ID, // 645
  SPV_OPERAND_TYPE_ID, // 646
  SPV_OPERAND_TYPE_ID, // 647
  SPV_OPERAND_TYPE_ID, // 648
  SPV_OPERAND_TYPE_ID, // 649
  SPV_OPERAND_TYPE_ID, // 650
  SPV_OPERAND_TYPE_ID, // 651
  SPV_OPERAND_TYPE_ID, // 652
  SPV_OPERAND_TYPE_TYPE_ID, // 653
  SPV_OPERAND_TYPE_RESULT_ID, // 654
  SPV_OPERAND_TYPE_ID, // 655
  SPV_OPERAND_TYPE_ID, // 656
  SPV_OPERAND_TYPE_ID, // 657
  SPV_OPERAND_TYPE_ID, // 658
  SPV_OPERAND_TYPE_ID, // 659
  SPV_OPERAND_TYPE_ID, // 660
  SPV_OPERAND_TYPE_ID, // 661
  SPV_OPERAND_TYPE_ID, // 662
  SPV_OPERAND_TYPE_ID, // 663
  SPV_OPERAND_TYPE_ID, // 664
  SPV_OPERAND_TYPE_ID, // 665
  SPV_OPERAND_TYPE_TYPE_ID, // 666
  SPV_OPERAND_TYPE_RESULT_ID, // 667
  SPV_OPERAND_TYPE_ID, // 668
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 669
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 670
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 671
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 672
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 673
  SPV_OPERAND_TYPE_TYPE_ID, // 674
  SPV_OPERAND_TYPE_RESULT_ID, // 675
  SPV_OPERAND_TYPE_ID, // 676
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 677
  SPV_OPERAND_TYPE_ID, // 678
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 679
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 680
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 681
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 682
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 683
  SPV_OPERAND_TYPE_TYPE_ID, // 684
  SPV_OPERAND_TYPE_RESULT_ID, // 685
  SPV_OPERAND_TYPE_ID, // 686
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 687
  SPV_OPERAND_TYPE_ID, // 688
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 689
  SPV_OPERAND_TYPE_RESULT_ID, // 690
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 691
  SPV_OPERAND_TYPE_RESULT_ID, // 692
  SPV_OPERAND_TYPE_ID, // 693
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 694
  SPV_OPERAND_TYPE_VARIABLE_ID, // 695
  SPV_OPERAND_TYPE_TYPE_ID, // 696
  SPV_OPERAND_TYPE_RESULT_ID, // 697
  SPV_OPERAND_TYPE_ID, // 698
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 699
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 700
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 701
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 702
  SPV_OPERAND_TYPE_ID, // 703
  SPV_OPERAND_TYPE_ID, // 704
  SPV_OPERAND_TYPE_ID, // 705
  SPV_OPERAND_TYPE_ID, // 706
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
  SPV_OPERAND_TYPE_ID, // 718
  SPV_OPERAND_TYPE_ID, // 719
  SPV_OPERAND_TYPE_ID, // 720
  SPV_OPERAND_TYPE_ID, // 721
  SPV_OPERAND_TYPE_TYPE_ID, // 722
  SPV_OPERAND_TYPE_RESULT_ID, // 723
  SPV_OPERAND_TYPE_ID, // 724
  SPV_OPERAND_TYPE_ID, // 725
  SPV_OPERAND_TYPE_ID, // 726
  SPV_OPERAND_TYPE_ID, // 727
  SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS, // 728
  SPV_OPERAND_TYPE_TYPE_ID, // 729
  SPV_OPERAND_TYPE_RESULT_ID, // 730
  SPV_OPERAND_TYPE_ID, // 731
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 732
  SPV_OPERAND_TYPE_ID, // 733
  SPV_OPERAND_TYPE_ID, // 734
  SPV_OPERAND_TYPE_ID, // 735
  SPV_OPERAND_TYPE_ID, // 736
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 737
  SPV_OPERAND_TYPE_ID, // 738
  SPV_OPERAND_TYPE_ID, // 739
  SPV_OPERAND_TYPE_ID, // 740
  SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 741
  SPV_OPERAND_TYPE_ID, // 742
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 743
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 744
  SPV_OPERAND_TYPE_ID, // 745
  SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER, // 746
  SPV_OPERAND_TYPE_ID, // 747
  SPV_OPERAND_TYPE_ID, // 748
  SPV_OPERAND_TYPE_ID, // 749
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 750
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 751
  SPV_OPERAND_TYPE_ID, // 752
  SPV_OPERAND_TYPE_ID, // 753
  SPV_OPERAND_TYPE_ID, // 754
  SPV_OPERAND_TYPE_ID, // 755
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 756
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 757
  SPV_OPERAND_TYPE_ID, // 758
  SPV_OPERAND_TYPE_ID, // 759
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 760
  SPV_OPERAND_TYPE_VARIABLE_ID, // 761
  SPV_OPERAND_TYPE_ID, // 762
  SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE, // 763
  SPV_OPERAND_TYPE_ID, // 764
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 765
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 766
  SPV_OPERAND_TYPE_ID, // 767
  SPV_OPERAND_TYPE_ID, // 768
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 769
  SPV_OPERAND_TYPE_VARIABLE_ID, // 770
  SPV_OPERAND_TYPE_ID, // 771
  SPV_OPERAND_TYPE_ID, // 772
  SPV_OPERAND_TYPE_ID, // 773
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 774
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 775
  SPV_OPERAND_TYPE_ID, // 776
  SPV_OPERAND_TYPE_ID, // 777
  SPV_OPERAND_TYPE_ID, // 778
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 779
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 780
  SPV_OPERAND_TYPE_ID, // 781
  SPV_OPERAND_TYPE_ID, // 782
  SPV_OPERAND_TYPE_ID, // 783
  SPV_OPERAND_TYPE_ID, // 784
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 785
  SPV_OPERAND_TYPE_ID, // 786
  SPV_OPERAND_TYPE_ID, // 787
  SPV_OPERAND_TYPE_ID, // 788
  SPV_OPERAND_TYPE_ID, // 789
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 790
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 791
  SPV_OPERAND_TYPE_ID, // 792
  SPV_OPERAND_TYPE_ID, // 793
  SPV_OPERAND_TYPE_ID, // 794
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 795
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 796
  SPV_OPERAND_TYPE_ID, // 797
  SPV_OPERAND_TYPE_ID, // 798
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 799
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 800
  SPV_OPERAND_TYPE_VARIABLE_ID, // 801
  SPV_OPERAND_TYPE_ID, // 802
  SPV_OPERAND_TYPE_ID, // 803
  SPV_OPERAND_TYPE_ID, // 804
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 805
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 806
  SPV_OPERAND_TYPE_ID, // 807
  SPV_OPERAND_TYPE_ID, // 808
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 809
  SPV_OPERAND_TYPE_ID, // 810
  SPV_OPERAND_TYPE_ID, // 811
  SPV_OPERAND_TYPE_ID, // 812
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 813
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 814
  SPV_OPERAND_TYPE_ID, // 815
  SPV_OPERAND_TYPE_ID, // 816
  SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS, // 817
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 818
  SPV_OPERAND_TYPE_ID, // 819
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 820
  SPV_OPERAND_TYPE_ID, // 821
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 822
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 823
  SPV_OPERAND_TYPE_ID, // 824
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 825
  SPV_OPERAND_TYPE_ID, // 826
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 827
  SPV_OPERAND_TYPE_ID, // 828
  SPV_OPERAND_TYPE_ID, // 829
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 830
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 831
  SPV_OPERAND_TYPE_ID, // 832
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 833
  SPV_OPERAND_TYPE_ID, // 834
  SPV_OPERAND_TYPE_ID, // 835
  SPV_OPERAND_TYPE_ID, // 836
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 837
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 838
  SPV_OPERAND_TYPE_ID, // 839
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 840
  SPV_OPERAND_TYPE_DEBUG_OPERATION, // 841
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 842
  SPV_OPERAND_TYPE_ID, // 843
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 844
  SPV_OPERAND_TYPE_ID, // 845
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 846
  SPV_OPERAND_TYPE_ID, // 847
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 848
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 849
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 850
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 851
  SPV_OPERAND_TYPE_ID, // 852
  SPV_OPERAND_TYPE_ID, // 853
  SPV_OPERAND_TYPE_ID, // 854
  SPV_OPERAND_TYPE_ID, // 855
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 856
  SPV_OPERAND_TYPE_ID, // 857
  SPV_OPERAND_TYPE_ID, // 858
  SPV_OPERAND_TYPE_ID, // 859
  SPV_OPERAND_TYPE_ID, // 860
  SPV_OPERAND_TYPE_ID, // 861
  SPV_OPERAND_TYPE_ID, // 862
  SPV_OPERAND_TYPE_ID, // 863
  SPV_OPERAND_TYPE_ID, // 864
  SPV_OPERAND_TYPE_VARIABLE_ID, // 865
  SPV_OPERAND_TYPE_ID, // 866
  SPV_OPERAND_TYPE_ID, // 867
  SPV_OPERAND_TYPE_ID, // 868
  SPV_OPERAND_TYPE_ID, // 869
  SPV_OPERAND_TYPE_ID, // 870
  SPV_OPERAND_TYPE_ID, // 871
  SPV_OPERAND_TYPE_ID, // 872
  SPV_OPERAND_TYPE_ID, // 873
  SPV_OPERAND_TYPE_ID, // 874
  SPV_OPERAND_TYPE_VARIABLE_ID, // 875
  SPV_OPERAND_TYPE_ID, // 876
  SPV_OPERAND_TYPE_ID, // 877
  SPV_OPERAND_TYPE_ID, // 878
  SPV_OPERAND_TYPE_ID, // 879
  SPV_OPERAND_TYPE_ID, // 880
  SPV_OPERAND_TYPE_ID, // 881
  SPV_OPERAND_TYPE_ID, // 882
  SPV_OPERAND_TYPE_ID, // 883
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 884
  SPV_OPERAND_TYPE_ID, // 885
  SPV_OPERAND_TYPE_ID, // 886
  SPV_OPERAND_TYPE_ID, // 887
  SPV_OPERAND_TYPE_ID, // 888
  SPV_OPERAND_TYPE_VARIABLE_ID, // 889
  SPV_OPERAND_TYPE_ID, // 890
  SPV_OPERAND_TYPE_ID, // 891
  SPV_OPERAND_TYPE_ID, // 892
  SPV_OPERAND_TYPE_ID, // 893
  SPV_OPERAND_TYPE_ID, // 894
  SPV_OPERAND_TYPE_ID, // 895
  SPV_OPERAND_TYPE_ID, // 896
  SPV_OPERAND_TYPE_ID, // 897
  SPV_OPERAND_TYPE_ID, // 898
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 899
  SPV_OPERAND_TYPE_ID, // 900
  SPV_OPERAND_TYPE_ID, // 901
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 902
  SPV_OPERAND_TYPE_ID, // 903
  SPV_OPERAND_TYPE_ID, // 904
  SPV_OPERAND_TYPE_ID, // 905
  SPV_OPERAND_TYPE_ID, // 906
  SPV_OPERAND_TYPE_ID, // 907
  SPV_OPERAND_TYPE_ID, // 908
  SPV_OPERAND_TYPE_ID, // 909
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 910
  SPV_OPERAND_TYPE_ID, // 911
  SPV_OPERAND_TYPE_ID, // 912
  SPV_OPERAND_TYPE_ID, // 913
  SPV_OPERAND_TYPE_VARIABLE_ID, // 914
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
  SPV_OPERAND_TYPE_ID, // 973
  SPV_OPERAND_TYPE_ID, // 974
  SPV_OPERAND_TYPE_ID, // 975
  SPV_OPERAND_TYPE_ID, // 976
  SPV_OPERAND_TYPE_ID, // 977
  SPV_OPERAND_TYPE_ID, // 978
  SPV_OPERAND_TYPE_ID, // 979
  SPV_OPERAND_TYPE_ID, // 980
  SPV_OPERAND_TYPE_ID, // 981
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 982
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 983
  SPV_OPERAND_TYPE_ID, // 984
  SPV_OPERAND_TYPE_SOURCE_LANGUAGE, // 985
  SPV_OPERAND_TYPE_ID, // 986
  SPV_OPERAND_TYPE_ID, // 987
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING, // 988
  SPV_OPERAND_TYPE_ID, // 989
  SPV_OPERAND_TYPE_STORAGE_CLASS, // 990
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 991
  SPV_OPERAND_TYPE_ID, // 992
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER, // 993
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 994
  SPV_OPERAND_TYPE_ID, // 995
  SPV_OPERAND_TYPE_VARIABLE_ID, // 996
  SPV_OPERAND_TYPE_ID, // 997
  SPV_OPERAND_TYPE_ID, // 998
  SPV_OPERAND_TYPE_ID, // 999
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1000
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1001
  SPV_OPERAND_TYPE_ID, // 1002
  SPV_OPERAND_TYPE_ID, // 1003
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1004
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1005
  SPV_OPERAND_TYPE_ID, // 1006
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE, // 1007
  SPV_OPERAND_TYPE_ID, // 1008
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1009
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1010
  SPV_OPERAND_TYPE_ID, // 1011
  SPV_OPERAND_TYPE_ID, // 1012
  SPV_OPERAND_TYPE_ID, // 1013
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1014
  SPV_OPERAND_TYPE_VARIABLE_ID, // 1015
  SPV_OPERAND_TYPE_ID, // 1016
  SPV_OPERAND_TYPE_ID, // 1017
  SPV_OPERAND_TYPE_ID, // 1018
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1019
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1020
  SPV_OPERAND_TYPE_ID, // 1021
  SPV_OPERAND_TYPE_ID, // 1022
  SPV_OPERAND_TYPE_ID, // 1023
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1024
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1025
  SPV_OPERAND_TYPE_ID, // 1026
  SPV_OPERAND_TYPE_ID, // 1027
  SPV_OPERAND_TYPE_ID, // 1028
  SPV_OPERAND_TYPE_ID, // 1029
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1030
  SPV_OPERAND_TYPE_ID, // 1031
  SPV_OPERAND_TYPE_ID, // 1032
  SPV_OPERAND_TYPE_ID, // 1033
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1034
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1035
  SPV_OPERAND_TYPE_ID, // 1036
  SPV_OPERAND_TYPE_ID, // 1037
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1038
  SPV_OPERAND_TYPE_ID, // 1039
  SPV_OPERAND_TYPE_ID, // 1040
  SPV_OPERAND_TYPE_ID, // 1041
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1042
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1043
  SPV_OPERAND_TYPE_ID, // 1044
  SPV_OPERAND_TYPE_ID, // 1045
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1046
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1047
  SPV_OPERAND_TYPE_ID, // 1048
  SPV_OPERAND_TYPE_OPTIONAL_ID, // 1049
  SPV_OPERAND_TYPE_ID, // 1050
  SPV_OPERAND_TYPE_ID, // 1051
  SPV_OPERAND_TYPE_ID, // 1052
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1053
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1054
  SPV_OPERAND_TYPE_ID, // 1055
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS, // 1056
  SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER, // 1057
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION, // 1058
  SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER, // 1059
  SPV_OPERAND_TYPE_ID, // 1060
  SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY, // 1061
  SPV_OPERAND_TYPE_ID, // 1062
  SPV_OPERAND_TYPE_ID, // 1063
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1064
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1065
  SPV_OPERAND_TYPE_ID, // 1066
  SPV_OPERAND_TYPE_ID, // 1067
  SPV_OPERAND_TYPE_ID, // 1068
  SPV_OPERAND_TYPE_ID, // 1069
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1070
  SPV_OPERAND_TYPE_ID, // 1071
  SPV_OPERAND_TYPE_ID, // 1072
  SPV_OPERAND_TYPE_ID, // 1073
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1074
  SPV_OPERAND_TYPE_ID, // 1075
  SPV_OPERAND_TYPE_ID, // 1076
  SPV_OPERAND_TYPE_LITERAL_INTEGER, // 1077
  SPV_OPERAND_TYPE_ID, // 1078
  SPV_OPERAND_TYPE_ID, // 1079
  SPV_OPERAND_TYPE_ID, // 1080
  SPV_OPERAND_TYPE_FP_ROUNDING_MODE, // 1081
};
