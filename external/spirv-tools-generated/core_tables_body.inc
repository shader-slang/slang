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
    case Extension::kSPV_INTEL_function_variants: return IR(1872, 28);
    case Extension::kSPV_INTEL_global_variable_fpga_decorations: return IR(1900, 43);
    case Extension::kSPV_INTEL_global_variable_host_access: return IR(1943, 38);
    case Extension::kSPV_INTEL_inline_assembly: return IR(1981, 26);
    case Extension::kSPV_INTEL_int4: return IR(2007, 15);
    case Extension::kSPV_INTEL_io_pipes: return IR(2022, 19);
    case Extension::kSPV_INTEL_kernel_attributes: return IR(2041, 28);
    case Extension::kSPV_INTEL_long_composites: return IR(2069, 26);
    case Extension::kSPV_INTEL_loop_fuse: return IR(2095, 20);
    case Extension::kSPV_INTEL_masked_gather_scatter: return IR(2115, 32);
    case Extension::kSPV_INTEL_maximum_registers: return IR(2147, 28);
    case Extension::kSPV_INTEL_media_block_io: return IR(2175, 25);
    case Extension::kSPV_INTEL_memory_access_aliasing: return IR(2200, 33);
    case Extension::kSPV_INTEL_optnone: return IR(2233, 18);
    case Extension::kSPV_INTEL_runtime_aligned: return IR(2251, 26);
    case Extension::kSPV_INTEL_shader_integer_functions2: return IR(2277, 36);
    case Extension::kSPV_INTEL_split_barrier: return IR(2313, 24);
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(2337, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(2372, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(2418, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(2438, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(2462, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(2498, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(2533, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(2570, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(2600, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(2632, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(2657, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(2679, 21);
    case Extension::kSPV_KHR_bfloat16: return IR(2700, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(2717, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(2742, 35);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(2777, 27);
    case Extension::kSPV_KHR_device_group: return IR(2804, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(2825, 22);
    case Extension::kSPV_KHR_float_controls: return IR(2847, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(2870, 24);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(2894, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(2930, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(2960, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(2988, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(3009, 30);
    case Extension::kSPV_KHR_multiview: return IR(3039, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(3057, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(3092, 26);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(3118, 32);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(3150, 28);
    case Extension::kSPV_KHR_quad_control: return IR(3178, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(3199, 22);
    case Extension::kSPV_KHR_ray_query: return IR(3221, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(3239, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(3259, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(3294, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(3331, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(3365, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(3387, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(3408, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(3439, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(3476, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(3500, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(3538, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(3560, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(3589, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(3624, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(3649, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(3675, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(3703, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(3744, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(3782, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(3806, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(3844, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(3878, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(3904, 27);
    case Extension::kSPV_NV_cooperative_vector: return IR(3931, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(3957, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(3986, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(4021, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(4056, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(4084, 19);
    case Extension::kSPV_NV_raw_access_chains: return IR(4103, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(4128, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(4147, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(4178, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(4215, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(4248, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(4278, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(4311, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(4337, 35);
    case Extension::kSPV_NV_shading_rate: return IR(4372, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(4392, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(4421, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(4446, 23);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(4469, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(4508, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(4534, 27);
    case Extension::kSPV_QCOM_tile_shading: return IR(4561, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(4583, 38);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,160> kExtensionNames{{
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
    {IR(1872, 28), static_cast<uint32_t>(kSPV_INTEL_function_variants)},
    {IR(1900, 43), static_cast<uint32_t>(kSPV_INTEL_global_variable_fpga_decorations)},
    {IR(1943, 38), static_cast<uint32_t>(kSPV_INTEL_global_variable_host_access)},
    {IR(1981, 26), static_cast<uint32_t>(kSPV_INTEL_inline_assembly)},
    {IR(2007, 15), static_cast<uint32_t>(kSPV_INTEL_int4)},
    {IR(2022, 19), static_cast<uint32_t>(kSPV_INTEL_io_pipes)},
    {IR(2041, 28), static_cast<uint32_t>(kSPV_INTEL_kernel_attributes)},
    {IR(2069, 26), static_cast<uint32_t>(kSPV_INTEL_long_composites)},
    {IR(2095, 20), static_cast<uint32_t>(kSPV_INTEL_loop_fuse)},
    {IR(2115, 32), static_cast<uint32_t>(kSPV_INTEL_masked_gather_scatter)},
    {IR(2147, 28), static_cast<uint32_t>(kSPV_INTEL_maximum_registers)},
    {IR(2175, 25), static_cast<uint32_t>(kSPV_INTEL_media_block_io)},
    {IR(2200, 33), static_cast<uint32_t>(kSPV_INTEL_memory_access_aliasing)},
    {IR(2233, 18), static_cast<uint32_t>(kSPV_INTEL_optnone)},
    {IR(2251, 26), static_cast<uint32_t>(kSPV_INTEL_runtime_aligned)},
    {IR(2277, 36), static_cast<uint32_t>(kSPV_INTEL_shader_integer_functions2)},
    {IR(2313, 24), static_cast<uint32_t>(kSPV_INTEL_split_barrier)},
    {IR(2337, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(2372, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(2418, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(2438, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(2462, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(2498, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(2533, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(2570, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(2600, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(2632, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(2657, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(2679, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(2700, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(2717, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(2742, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(2777, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(2804, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(2825, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(2847, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(2870, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(2894, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(2930, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(2960, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(2988, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(3009, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(3039, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(3057, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(3092, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(3118, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(3150, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(3178, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(3199, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(3221, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(3239, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(3259, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(3294, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(3331, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(3365, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(3387, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(3408, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(3439, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(3476, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(3500, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(3538, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(3560, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(3589, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(3624, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(3649, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(3675, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(3703, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(3744, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(3782, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(3806, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(3844, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(3878, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(3904, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(3931, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(3957, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(3986, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(4021, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(4056, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(4084, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(4103, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(4128, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(4147, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(4178, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(4215, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(4248, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(4278, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(4311, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(4337, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(4372, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(4392, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(4421, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(4446, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(4469, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(4508, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(4534, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(4561, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(4583, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
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
static const std::array<OperandDesc, 1187> kOperandsByValue{{
  {0,IR(0, 0),IR(4621, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(4637, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(4647, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(4657, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(4665, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(4686, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(4697, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(4779, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(4795, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(4805, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(4818, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(4831, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(4840, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(4851, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(4941, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(4954, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(5000, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(5048, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(5063, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(5078, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(5088, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(5102, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(5112, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(5123, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(5135, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(5162, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(5177, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(5188, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(5198, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(5215, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(5229, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(5243, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(5255, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(5273, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(5292, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(5313, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(5321, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(5332, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(5354, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(5367, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(5382, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(5429, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(5445, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(5458, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(5479, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(5490, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(5516, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(5528, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(5542, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(5568, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(5581, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(5594, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(5604, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(5617, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(5672, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(5705, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(5738, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(5771, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(5804, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(5830, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(5843, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(5853, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(5900, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(5924, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(5944, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(5959, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(5990, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(6008, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(6026, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(6046, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(6074, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(6100, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(6119, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(6146, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(6171, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(6193, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(6228, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(6275, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(6291, 15)/* ViewportMaskNV */,IR(0, 0),IR(44, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(6327, 20)/* SecondaryPositionNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(6366, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(6390, 18)/* PositionPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(6428, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(47, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(6450, 16)/* FullyCoveredEXT */,IR(0, 0),IR(49, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(6490, 12)/* TaskCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(6502, 17)/* PrimitiveCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(6519, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(6538, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(6560, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(6582, 15)/* LayerPerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(6597, 16)/* MeshViewCountNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(6613, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(6631, 13)/* BaryCoordKHR */,IR(6, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(6679, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(6718, 12)/* FragSizeEXT */,IR(8, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(6764, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(52, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(6809, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(6834, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(6858, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(6886, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(53, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(6903, 12)/* LaunchIdKHR */,IR(10, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(6926, 14)/* LaunchSizeKHR */,IR(11, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(6953, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(6988, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(7029, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(7066, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(7109, 11)/* RayTminKHR */,IR(16, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(7130, 11)/* RayTmaxKHR */,IR(17, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(7151, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(7196, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(7229, 17)/* WorldToObjectKHR */,IR(20, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(7262, 7)/* HitTNV */,IR(0, 0),IR(56, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(7269, 11)/* HitKindKHR */,IR(21, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(7290, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(57, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(7330, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(58, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(7387, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(7454, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(7491, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(7530, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(60, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(7550, 14)/* HitIsSphereNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(7592, 11)/* HitIsLSSNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(7642, 20)/* HitSpherePositionNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(7662, 13)/* WarpsPerSMNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(7694, 10)/* SMCountNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(7704, 9)/* WarpIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(7713, 7)/* SMIDNV */,IR(0, 0),IR(63, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(7720, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(7738, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(7772, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(7805, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(61, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(7823, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(7837, 12)/* ClusterIDNV */,IR(0, 0),IR(64, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(7890, 12)/* CullMaskKHR */,IR(0, 0),IR(65, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(7917, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(4788, 7)/* Shader */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(4863, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(4872, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(4676, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(7924, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(4630, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(7932, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(7941, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(7955, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(7963, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(7971, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(7977, 13)/* Int64Atomics */,IR(0, 0),IR(67, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(7990, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(8001, 15)/* ImageReadWrite */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(8016, 12)/* ImageMipmap */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(8028, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(8034, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(8041, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(8055, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(8070, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(8084, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(8090, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(8112, 18)/* GeometryPointSize */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(8130, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(8150, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(8174, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(8208, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(8241, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(8275, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(4805, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(4818, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(8308, 15)/* ImageCubeArray */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(5144, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(8340, 10)/* ImageRect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(8350, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(8362, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(8377, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(8382, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(8398, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(8414, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(8421, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(8431, 8)/* Image1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(8323, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(8439, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(8453, 12)/* ImageBuffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(8465, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(8478, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(8506, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(8517, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(8535, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(8557, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(8575, 16)/* GeometryStreams */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(8591, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(8621, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(5014, 14)/* MultiViewport */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(8652, 17)/* SubgroupDispatch */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(8669, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(8682, 12)/* PipeStorage */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(5395, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(8694, 20)/* GroupNonUniformVote */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(8714, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(5650, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(8740, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(8763, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(8794, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(8819, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(4960, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(5028, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(8839, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(5552, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(8857, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(8885, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(8913, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(8943, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(8954, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(8991, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(9031, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(9040, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(9068, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(9078, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(77, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(5877, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(5411, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(5815, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(9126, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(9159, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(9202, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(79, 1),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(9246, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(9262, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(9315, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(80, 1),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(9367, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(9389, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(42, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(5912, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(5934, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(9410, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(9440, 17)/* VariablePointers */,IR(0, 0),IR(81, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(9457, 17)/* AtomicStorageOps */,IR(0, 0),IR(82, 1),IR(44, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(9474, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(9502, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(9526, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(83, 1),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(9560, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(46, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(9581, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(9596, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(9614, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(9639, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(9655, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(9671, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(9694, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(48, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(9706, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(9725, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(84, 2),IR(50, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(4898, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(9757, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(9783, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(9804, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(5974, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(9826, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(86, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(9858, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(9881, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(9897, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(9919, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(6211, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(9935, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(9956, 14)/* Int64ImageEXT */,IR(0, 0),IR(3, 1),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(9970, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(6257, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(9985, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(10000, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(10014, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(87, 2),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(10041, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(10057, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(89, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(10079, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(90, 2),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(10108, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(10137, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(69, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(4972, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(92, 1),IR(68, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(6306, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(93, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(6347, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(94, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(6408, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(6466, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(4912, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(10192, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(4926, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(6656, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(10231, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(6745, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(10306, 29)/* GroupNonUniformPartitionedNV */,IR(0, 0),IR(1, 0),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(10335, 17)/* ShaderNonUniform */,IR(29, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(10372, 23)/* RuntimeDescriptorArray */,IR(30, 1),IR(3, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(10421, 36)/* InputAttachmentArrayDynamicIndexing */,IR(31, 1),IR(95, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(10496, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(32, 1),IR(73, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(10577, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(96, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(10658, 37)/* UniformBufferArrayNonUniformIndexing */,IR(34, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(10735, 36)/* SampledImageArrayNonUniformIndexing */,IR(35, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(10810, 37)/* StorageBufferArrayNonUniformIndexing */,IR(36, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(10887, 36)/* StorageImageArrayNonUniformIndexing */,IR(37, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(10962, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(38, 1),IR(98, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(11043, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(39, 1),IR(100, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(11130, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(102, 2),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(7360, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(4885, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(7307, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(11217, 18)/* VulkanMemoryModel */,IR(41, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(11256, 29)/* VulkanMemoryModelDeviceScope */,IR(42, 1),IR(1, 0),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(4748, 31)/* PhysicalStorageBufferAddresses */,IR(43, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(11351, 32)/* ComputeDerivativeGroupLinearKHR */,IR(44, 1),IR(3, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(11414, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(11439, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(79, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(11459, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(11492, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(7675, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(11530, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(11562, 25)/* DemoteToHelperInvocation */,IR(45, 1),IR(3, 1),IR(81, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(11615, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(11638, 29)/* RayTracingOpacityMicromapEXT */,IR(0, 0),IR(3, 1),IR(83, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(11667, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(60, 1),IR(84, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(11693, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(85, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(11711, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(11736, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(11756, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(87, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(7421, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(60, 1),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(11778, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(7564, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(7603, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(11796, 30)/* CooperativeMatrixReductionsNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(11826, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(11857, 40)/* CooperativeMatrixPerElementOperationsNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(11897, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(11933, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(89, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(11963, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(86, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(7849, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(60, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(11991, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(12010, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(12031, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(12058, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(12084, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(92, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(12115, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(12136, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(12159, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(94, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(12182, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(12204, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(12228, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(12237, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(12260, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(12283, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(12306, 19)/* VectorComputeINTEL */,IR(0, 0),IR(104, 1),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(12325, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(12340, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(12356, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(12389, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(12427, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(12466, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(12491, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(12517, 26)/* FPGAMemoryAttributesINTEL */,IR(0, 0),IR(1, 0),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(12543, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(12563, 32)/* ArbitraryPrecisionIntegersINTEL */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(12595, 37)/* ArbitraryPrecisionFloatingPointINTEL */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(12632, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(106, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(12662, 22)/* FPGALoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(107, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(12684, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(12706, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(12732, 24)/* FPGAMemoryAccessesINTEL */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(12756, 27)/* FPGAClusterAttributesINTEL */,IR(0, 0),IR(1, 0),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(12783, 14)/* LoopFuseINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(12797, 20)/* FPGADSPControlINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(12817, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(12843, 40)/* FPGAInvocationPipeliningAttributesINTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(12883, 24)/* FPGABufferLocationINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(12907, 34)/* ArbitraryPrecisionFixedPointINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(12941, 23)/* USMStorageClassesINTEL */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(12964, 29)/* RuntimeAlignedAttributeINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(12993, 13)/* IOPipesINTEL */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(13006, 19)/* BlockingPipesINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(13025, 13)/* FPGARegINTEL */,IR(0, 0),IR(1, 0),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(13038, 19)/* DotProductInputAll */,IR(46, 1),IR(1, 0),IR(122, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(13079, 22)/* DotProductInput4x8Bit */,IR(47, 1),IR(105, 1),IR(122, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(13126, 28)/* DotProductInput4x8BitPacked */,IR(48, 1),IR(1, 0),IR(122, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(13185, 11)/* DotProduct */,IR(49, 1),IR(1, 0),IR(122, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(7902, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(9105, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(123, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(13210, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(13234, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(125, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(13250, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(76, 1),IR(126, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(13275, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(127, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(13290, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(13310, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(128, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(13330, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(129, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(13350, 11)/* OptNoneEXT */,IR(50, 1),IR(1, 0),IR(130, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(13374, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(13394, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(133, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(13415, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(134, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(13439, 18)/* SplitBarrierINTEL */,IR(0, 0),IR(1, 0),IR(135, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(13457, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(136, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(13476, 29)/* FPGAClusterAttributesV2INTEL */,IR(0, 0),IR(106, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(13505, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(107, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(13533, 18)/* TaskSequenceINTEL */,IR(0, 0),IR(1, 0),IR(137, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(13551, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(138, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(13567, 24)/* FPGALatencyControlINTEL */,IR(0, 0),IR(1, 0),IR(139, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(13591, 28)/* FPGAArgumentInterfacesINTEL */,IR(0, 0),IR(1, 0),IR(140, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(13619, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(13649, 35)/* GlobalVariableFPGADecorationsINTEL */,IR(0, 0),IR(1, 0),IR(142, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(13684, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(143, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(13712, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(13735, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(108, 1),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(13765, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(108, 1),IR(144, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(13795, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(145, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(13833, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(146, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(13861, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(13882, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(109, 1),IR(147, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(13904, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(148, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(13930, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(149, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(13957, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(150, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(13982, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(151, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(14001, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(152, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(14021, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(153, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(14041, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14053, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14061, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(14069, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14075, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14082, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14093, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14102, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14115, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14121, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14131, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14137, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14152, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14177, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14193, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14207, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14220, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14232, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14249, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14261, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14276, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14289, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14304, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14322, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14339, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14360, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14380, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14400, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14416, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14432, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14452, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14476, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14482, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14487, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(14526, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(14537, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14546, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14551, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14558, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(14569, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(14576, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14585, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14595, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14608, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14621, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(14632, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(14642, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(14652, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(14662, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(14675, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(14689, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(14703, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(14717, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(14732, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(14748, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(14764, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(14780, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(14799, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(14820, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(14832, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(14844, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(14856, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(14871, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(14896, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(14924, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(14932, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(14959, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(14986, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(15013, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(15045, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(15071, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(15075, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(15082, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(15086, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(15097, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(15108, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(15129, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(15140, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(15154, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(15175, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(14041, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14053, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14061, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14069, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14075, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14082, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14093, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(14102, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14115, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14121, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14131, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14177, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14193, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14207, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14220, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14232, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14249, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14261, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14276, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14289, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14304, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14322, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14339, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14360, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14380, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14400, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14476, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(14482, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(14487, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(14526, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(14537, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(14546, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(14551, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(14558, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(14569, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(14585, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14595, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14608, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(15193, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(15210, 7)/* SpecId */,IR(0, 0),IR(110, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(15217, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(15223, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(15235, 9)/* RowMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(15244, 9)/* ColMajor */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(15253, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(15265, 13)/* MatrixStride */,IR(0, 0),IR(66, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(15278, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(15289, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(15300, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(15334, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(15342, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(15356, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(15361, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(15367, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(15376, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(15383, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(15393, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(15402, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(15410, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(15419, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(15428, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(15437, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(15449, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(15461, 8)/* Uniform */,IR(0, 0),IR(112, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(15495, 10)/* UniformId */,IR(0, 0),IR(112, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(15505, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(15525, 7)/* Stream */,IR(0, 0),IR(114, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(15532, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(15541, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(15551, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(15557, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(15565, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(15579, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(15586, 10)/* XfbBuffer */,IR(0, 0),IR(115, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(15596, 10)/* XfbStride */,IR(0, 0),IR(115, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(15652, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(15700, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(15750, 15)/* FPFastMathMode */,IR(0, 0),IR(116, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(15827, 18)/* LinkageAttributes */,IR(0, 0),IR(118, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(15845, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(15859, 21)/* InputAttachmentIndex */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(15880, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(15890, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(15924, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(15936, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(15952, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(15996, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(154, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(16009, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(154, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(16024, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(16042, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(52, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(16064, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(16086, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(16104, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(16136, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(16156, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(16179, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(16199, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(16224, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(16251, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(16276, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(16304, 19)/* OverrideCoverageNV */,IR(0, 0),IR(120, 1),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(16323, 14)/* PassthroughNV */,IR(0, 0),IR(121, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(16337, 19)/* ViewportRelativeNV */,IR(0, 0),IR(94, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(16356, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(46, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(16384, 16)/* PerPrimitiveEXT */,IR(51, 1),IR(122, 2),IR(155, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(16415, 10)/* PerViewNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(16425, 10)/* PerTaskNV */,IR(0, 0),IR(50, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(16435, 13)/* PerVertexKHR */,IR(52, 1),IR(51, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(16460, 11)/* NonUniform */,IR(53, 1),IR(97, 1),IR(76, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(16485, 16)/* RestrictPointer */,IR(54, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(16520, 15)/* AliasedPointer */,IR(55, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(16553, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(16583, 18)/* BindlessSamplerNV */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(16601, 16)/* BindlessImageNV */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(16617, 15)/* BoundSamplerNV */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(16632, 13)/* BoundImageNV */,IR(0, 0),IR(125, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(16645, 14)/* SIMTCallINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(16659, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(127, 1),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(16685, 13)/* ClobberINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(16698, 17)/* SideEffectsINTEL */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(16715, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(16742, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(16763, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(16790, 15)/* StackCallINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(16805, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(16831, 14)/* CounterBuffer */,IR(56, 1),IR(1, 0),IR(157, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(16869, 13)/* UserSemantic */,IR(57, 1),IR(1, 0),IR(157, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(16901, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(158, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(16916, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(16973, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(16997, 14)/* RegisterINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(17011, 12)/* MemoryINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(17023, 14)/* NumbanksINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(17037, 15)/* BankwidthINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(17052, 22)/* MaxPrivateCopiesINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(17074, 16)/* SinglepumpINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(17090, 16)/* DoublepumpINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(17106, 19)/* MaxReplicatesINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(17125, 20)/* SimpleDualPortINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(17145, 11)/* MergeINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(17198, 14)/* BankBitsINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(17212, 20)/* ForcePow2DepthINTEL */,IR(0, 0),IR(130, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(17232, 16)/* StridesizeINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(17248, 14)/* WordsizeINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(17262, 18)/* TrueDualPortINTEL */,IR(0, 0),IR(130, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(17280, 19)/* BurstCoalesceINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(17299, 15)/* CacheSizeINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(17314, 28)/* DontStaticallyCoalesceINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(17342, 14)/* PrefetchINTEL */,IR(0, 0),IR(131, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(17356, 17)/* StallEnableINTEL */,IR(0, 0),IR(106, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(17373, 25)/* FuseLoopsInFunctionINTEL */,IR(0, 0),IR(132, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(17398, 19)/* MathOpDSPModeINTEL */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(17417, 16)/* AliasScopeINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(17433, 13)/* NoAliasINTEL */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(17446, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(17470, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(17490, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(17510, 20)/* BufferLocationINTEL */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(17530, 19)/* IOPipeStorageINTEL */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(17583, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(17614, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(17639, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(17674, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(17692, 15)/* StallFreeINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(17738, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(139, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(17764, 25)/* LatencyControlLabelINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(17789, 30)/* LatencyControlConstraintINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(17819, 27)/* ConduitKernelArgumentINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(17846, 31)/* RegisterMapKernelArgumentINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(17877, 33)/* MMHostInterfaceAddressWidthINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(17910, 30)/* MMHostInterfaceDataWidthINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(17940, 28)/* MMHostInterfaceLatencyINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(18002, 34)/* MMHostInterfaceReadWriteModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(18036, 29)/* MMHostInterfaceMaxBurstINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(18065, 32)/* MMHostInterfaceWaitRequestINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(18097, 26)/* StableKernelArgumentINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(18162, 16)/* HostAccessINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(18225, 14)/* InitModeINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(18239, 28)/* ImplementInRegisterMapINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(18267, 17)/* ConditionalINTEL */,IR(0, 0),IR(109, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(18320, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(18379, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(18402, 3)/* 1D */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(18405, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(18408, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(18411, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(18416, 5)/* Rect */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(18421, 7)/* Buffer */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(18428, 12)/* SubpassData */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(18440, 17)/* TileImageDataEXT */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(18457, 12)/* Invocations */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(18469, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(18482, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(18504, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(18525, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(18539, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(18554, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(18573, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(18589, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(18605, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(18624, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(18634, 4)/* Xfb */,IR(0, 0),IR(115, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(18638, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(18653, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(18666, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(18676, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(18691, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(18701, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(18715, 12)/* InputPoints */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(18727, 11)/* InputLines */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(18738, 20)/* InputLinesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(18758, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(18768, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(18792, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(18798, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(18807, 15)/* OutputVertices */,IR(0, 0),IR(146, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(18822, 13)/* OutputPoints */,IR(0, 0),IR(150, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(18835, 16)/* OutputLineStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(18851, 20)/* OutputTriangleStrip */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(18871, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(18883, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(18898, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(18910, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(5382, 13)/* SubgroupSize */,IR(0, 0),IR(153, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(18920, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(153, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(18942, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(153, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(18966, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(18978, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(18994, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(19028, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(19062, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(19098, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(19128, 18)/* PostDepthCoverage */,IR(0, 0),IR(156, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(9581, 15)/* DenormPreserve */,IR(0, 0),IR(157, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(9596, 18)/* DenormFlushToZero */,IR(0, 0),IR(158, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(9614, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(159, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(9639, 16)/* RoundingModeRTE */,IR(0, 0),IR(160, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(9655, 16)/* RoundingModeRTZ */,IR(0, 0),IR(161, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(19146, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(19180, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(19200, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(19229, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(19252, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(19267, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(19282, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(19303, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(6275, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(19327, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(19348, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(19376, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(19402, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(19425, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(19452, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(19477, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(161, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(19499, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(162, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(19518, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(162, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(19538, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(19558, 15)/* OutputLinesEXT */,IR(58, 1),IR(122, 2),IR(155, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(19587, 20)/* OutputPrimitivesEXT */,IR(59, 1),IR(122, 2),IR(155, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(19626, 24)/* DerivativeGroupQuadsKHR */,IR(60, 1),IR(163, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(19673, 25)/* DerivativeGroupLinearKHR */,IR(61, 1),IR(164, 1),IR(73, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(19722, 19)/* OutputTrianglesEXT */,IR(62, 1),IR(122, 2),IR(155, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(19759, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(165, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(19784, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(165, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(19811, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(166, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(19837, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(166, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(19865, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(19896, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(167, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(19929, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(19956, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(19977, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(19998, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(20024, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(20051, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(169, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(20073, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(169, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(20089, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(169, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(20109, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(107, 1),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(20131, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(20159, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(20183, 18)/* FPFastMathDefault */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(20201, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(107, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(20225, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(171, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(20251, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(126, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(20274, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(20296, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(20371, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(20398, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(20405, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(20425, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(4863, 9)/* Geometry */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(14576, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(20448, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(4630, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(20458, 7)/* TaskNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(20465, 7)/* MeshNV */,IR(0, 0),IR(50, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(20472, 17)/* RayGenerationKHR */,IR(63, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(20505, 16)/* IntersectionKHR */,IR(64, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(20536, 10)/* AnyHitKHR */,IR(65, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(20555, 14)/* ClosestHitKHR */,IR(66, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(20582, 8)/* MissKHR */,IR(67, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(20597, 12)/* CallableKHR */,IR(68, 1),IR(54, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(20620, 8)/* TaskEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(20628, 8)/* MeshEXT */,IR(0, 0),IR(53, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(20636, 9)/* Preserve */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(20645, 12)/* FlushToZero */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(20657, 12)/* BFloat16KHR */,IR(0, 0),IR(89, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(20669, 14)/* Float8E4M3EXT */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(20683, 14)/* Float8E5M2EXT */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(20697, 5)/* IEEE */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(20702, 4)/* ALT */,IR(0, 0),IR(129, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(20706, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(20713, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(20720, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(20724, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(20735, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(20740, 14)/* AllowContract */,IR(69, 1),IR(173, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(20777, 13)/* AllowReassoc */,IR(70, 1),IR(173, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(20808, 15)/* AllowTransform */,IR(0, 0),IR(170, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(20823, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20827, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(20831, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(20835, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(20839, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(20855, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(20871, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(20889, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(20907, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(20914, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(20925, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(20930, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(13350, 11)/* OptNoneEXT */,IR(50, 1),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(20936, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(20941, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(20946, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(20952, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(20957, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(20965, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(20975, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(20983, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(20995, 20)/* RuntimeAlignedINTEL */,IR(0, 0),IR(176, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(21015, 7)/* Reduce */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(21022, 14)/* InclusiveScan */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(21036, 14)/* ExclusiveScan */,IR(0, 0),IR(177, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(21050, 16)/* ClusteredReduce */,IR(0, 0),IR(180, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(21066, 20)/* PartitionedReduceNV */,IR(0, 0),IR(181, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(21086, 27)/* PartitionedInclusiveScanNV */,IR(0, 0),IR(181, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(21113, 27)/* PartitionedExclusiveScanNV */,IR(0, 0),IR(181, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(21140, 10)/* NoneINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(21150, 10)/* ReadINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(21160, 11)/* WriteINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(21171, 15)/* ReadWriteINTEL */,IR(0, 0),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(21186, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(21191, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(21195, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(21200, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(15579, 7)/* Offset */,IR(0, 0),IR(182, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(21212, 13)/* ConstOffsets */,IR(0, 0),IR(182, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(15376, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(8414, 7)/* MinLod */,IR(0, 0),IR(183, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(21225, 19)/* MakeTexelAvailable */,IR(71, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(21266, 17)/* MakeTexelVisible */,IR(72, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(21303, 16)/* NonPrivateTexel */,IR(73, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(21338, 14)/* VolatileTexel */,IR(74, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(21369, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(21380, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(21391, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(21403, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(21411, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(21421, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(21432, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(21442, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(21453, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(21467, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(21481, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(21496, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(21507, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(21519, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(21531, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(21544, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(21558, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(21572, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(14069, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(21582, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(21593, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(21610, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(21626, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(21646, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(21666, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(21686, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(21705, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(21724, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(21743, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(21759, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(21775, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(21777, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(21779, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(21782, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(21785, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(21789, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(21794, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(21799, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(21804, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(21814, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(21824, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(21827, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(21831, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(21836, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(21842, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(21855, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(21860, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(21866, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(21872, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(21878, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(21883, 27)/* InitOnDeviceReprogramINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(21910, 23)/* InitOnDeviceResetINTEL */,IR(0, 0),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(21933, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(21940, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(21951, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(21965, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(21977, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(21990, 7)/* Export */,IR(0, 0),IR(118, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(21997, 7)/* Import */,IR(0, 0),IR(118, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(22004, 12)/* LinkOnceODR */,IR(0, 0),IR(118, 1),IR(164, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(22016, 14)/* UncachedINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(22030, 12)/* CachedINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(22042, 15)/* StreamingINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(22057, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(22082, 17)/* ConstCachedINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(22099, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(22106, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(22117, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(22136, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(22153, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(22167, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(22181, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(22199, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(22209, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(17446, 24)/* InitiationIntervalINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(17470, 20)/* MaxConcurrencyINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(22222, 21)/* DependencyArrayINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(17490, 20)/* PipelineEnableINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(22243, 18)/* LoopCoalesceINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(22261, 21)/* MaxInterleavingINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(22282, 26)/* SpeculatedIterationsINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(22308, 14)/* NoFusionINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(22322, 15)/* LoopCountINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(22337, 26)/* MaxReinvocationDelayINTEL */,IR(0, 0),IR(185, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(22363, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(22392, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(22421, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(22442, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(22468, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(22491, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(22514, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(22537, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(22560, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(22577, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(22594, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(22620, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(22646, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(22673, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(15410, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(22700, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(21391, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(22708, 21)/* MakePointerAvailable */,IR(75, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(22753, 19)/* MakePointerVisible */,IR(76, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(22794, 18)/* NonPrivatePointer */,IR(77, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(22833, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(134, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(22853, 17)/* NoAliasINTELMask */,IR(0, 0),IR(134, 1),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(22870, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(22877, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(22885, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(22892, 7)/* Vulkan */,IR(78, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(22909, 8)/* Relaxed */,IR(79, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(22917, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(22925, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(22933, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(22948, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(22971, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(22985, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(23000, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(23016, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(23037, 20)/* AtomicCounterMemory */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(23057, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(23069, 13)/* OutputMemory */,IR(80, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(23098, 14)/* MakeAvailable */,IR(81, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(23129, 12)/* MakeVisible */,IR(82, 1),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(15410, 9)/* Volatile */,IR(0, 0),IR(184, 1),IR(78, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(23156, 10)/* AutoINTEL */,IR(0, 0),IR(172, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(23166, 5)/* WRAP */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(23171, 4)/* SAT */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(23175, 9)/* SAT_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(23184, 8)/* SAT_SYM */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(23192, 25)/* PackedVectorFormat4x8Bit */,IR(83, 1),IR(1, 0),IR(122, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(23245, 4)/* TRN */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(23249, 9)/* TRN_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(23258, 4)/* RND */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(23262, 9)/* RND_ZERO */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(23271, 8)/* RND_INF */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(23279, 12)/* RND_MIN_INF */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(23291, 9)/* RND_CONV */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(23300, 13)/* RND_CONV_ODD */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(23313, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(23338, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(14924, 8)/* NoneKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(23361, 10)/* OpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(23371, 12)/* NoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(23383, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(23406, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(23430, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(23457, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(23485, 14)/* CullOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(23499, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(84, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(23515, 17)/* SkipTrianglesKHR */,IR(84, 1),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(23556, 13)/* SkipAABBsKHR */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(23569, 30)/* ForceOpacityMicromap2StateEXT */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(23599, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23640, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23677, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(23714, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(23755, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(23797, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(23830, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(23863, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(23875, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(23881, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(23888, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(23903, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(23911, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(23918, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(23926, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(23934, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(23942, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(23947, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(23953, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(23964, 6)/* Rg32f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(23970, 6)/* Rg16f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(23976, 13)/* R11fG11fB10f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(23989, 5)/* R16f */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(23994, 7)/* Rgba16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(24001, 8)/* Rgb10A2 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(24009, 5)/* Rg16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(24014, 4)/* Rg8 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(24018, 4)/* R16 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(24022, 3)/* R8 */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(24025, 12)/* Rgba16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(24037, 10)/* Rg16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(24047, 9)/* Rg8Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(24056, 9)/* R16Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(24065, 8)/* R8Snorm */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(24073, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(24081, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(24089, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(24096, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(24101, 6)/* Rg32i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(24107, 6)/* Rg16i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(24113, 5)/* Rg8i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(24118, 5)/* R16i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(24123, 4)/* R8i */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(24127, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(24136, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(24145, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(24153, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(24159, 10)/* Rgb10a2ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(24169, 7)/* Rg32ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(24176, 7)/* Rg16ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(24183, 6)/* Rg8ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(24189, 6)/* R16ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(24195, 5)/* R8ui */,IR(0, 0),IR(191, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(24200, 6)/* R64ui */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(24206, 5)/* R64i */,IR(0, 0),IR(192, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(24211, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(24223, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(24230, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(24240, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(24249, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(24260, 12)/* QueueFamily */,IR(85, 1),IR(184, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(24287, 14)/* ShaderCallKHR */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24301, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(24309, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(24321, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(14041, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(14053, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(14061, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(14069, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(14075, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(14082, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(14093, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(14102, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(14115, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(14121, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(14131, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(14137, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(14152, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(14177, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(14193, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(14207, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(14220, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(14232, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(14249, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(14261, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(14276, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(14289, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(14304, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(14322, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(14339, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(14360, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(14380, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(14400, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(14416, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(14432, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(14452, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(24350, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(14476, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(14482, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(14487, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(14526, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(14537, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(14546, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(14551, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(14558, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(14569, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(14576, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(14585, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(14595, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(14608, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(14621, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(23918, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(24376, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(24381, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(24386, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(24395, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(24406, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(24411, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(24426, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(24431, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(24438, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(24443, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(24448, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(24454, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(24458, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(24463, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(24479, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(15461, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(24485, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(24230, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(24492, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(24507, 8)/* Private */,IR(0, 0),IR(193, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(24515, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(24524, 8)/* Generic */,IR(0, 0),IR(195, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(24532, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(24545, 14)/* AtomicCounter */,IR(0, 0),IR(82, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(24559, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(24565, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(165, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(24579, 13)/* TileImageEXT */,IR(0, 0),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(24592, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(24611, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(24627, 16)/* CallableDataKHR */,IR(86, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(24658, 24)/* IncomingCallableDataKHR */,IR(87, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(24705, 14)/* RayPayloadKHR */,IR(88, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(24732, 16)/* HitAttributeKHR */,IR(89, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(24763, 22)/* IncomingRayPayloadKHR */,IR(90, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(24806, 22)/* ShaderRecordBufferKHR */,IR(91, 1),IR(54, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(24849, 22)/* PhysicalStorageBuffer */,IR(92, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(24896, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(124, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(24917, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(53, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(24941, 17)/* CodeSectionINTEL */,IR(0, 0),IR(196, 1),IR(95, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(24958, 16)/* DeviceOnlyINTEL */,IR(0, 0),IR(197, 1),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(24974, 14)/* HostOnlyINTEL */,IR(0, 0),IR(197, 1),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(22016, 14)/* UncachedINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(24988, 18)/* WriteThroughINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(25006, 15)/* WriteBackINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(22042, 15)/* StreamingINTEL */,IR(0, 0),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(14172, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(25021, 11)/* TensorView */,IR(0, 0),IR(198, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(25032, 11)/* DecodeFunc */,IR(0, 0),IR(199, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(25043, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(15419, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(23863, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(23881, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(23888, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(25053, 8)/* NoneARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(25061, 15)/* NontemporalARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(25076, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(25096, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(25120, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(25142, 21)/* NonPrivateElementARM */,IR(0, 0),IR(200, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(133, 273);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(406, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(414, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(417, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(419, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(438, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(448, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(452, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(467, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(471, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(477, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(480, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(483, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(487, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(495, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(498, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(514, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(523, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(526, 144);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(670, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(678, 96);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(774, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(791, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(793, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(796, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(798, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(807, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(811, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(815, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(821, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(830, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(837, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(841, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(858, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(884, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(904, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(906, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(909, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(911, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(913, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(916, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(921, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(941, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(956, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(965, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(969, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(984, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(985, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(989, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(990, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(998, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1001, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1013, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1015, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1018, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1020, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1025, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1027, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1069, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1076, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1079, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1080, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1088, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1091, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1093, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1113, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1123, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1127, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1141, 28);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1169, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1173, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1176, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1181, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(841, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(956, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(989, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(471, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(941, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(998, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(793, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1181, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(133, 273);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1281> kOperandNames{{
  {IR(4621, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4647, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4637, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(4657, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4665, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4686, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4697, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(4721, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6631, 13), 92}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6644, 12), 92}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6026, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6046, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6679, 20), 93}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6699, 19), 93}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6074, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6171, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6100, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6119, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6146, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5830, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5804, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4805, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6538, 22), 87}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7837, 12), 131}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5568, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5542, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5581, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4818, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6560, 22), 88}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7890, 12), 132}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6886, 17), 99}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7290, 17), 113}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5900, 12), 62}, // 32 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5843, 10), 60}, // 33 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5332, 22), 37}, // 34 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5102, 10), 21}, // 35 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5188, 10), 27}, // 36 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6764, 23), 95}, // 37 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6718, 12), 94}, // 38 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6193, 18), 75}, // 39 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6730, 15), 94}, // 40 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5123, 12), 23}, // 41 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6450, 16), 83}, // 42 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5273, 19), 33}, // 43 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5367, 15), 39}, // 44 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5354, 13), 38}, // 45 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5321, 11), 36}, // 46 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5198, 17), 28}, // 47 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7592, 11), 120}, // 48 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7550, 14), 119}, // 49 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7772, 33), 128}, // 50 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7738, 34), 127}, // 51 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7269, 11), 112}, // 52 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7280, 10), 112}, // 53 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7720, 18), 126}, // 54 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7823, 14), 130}, // 55 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7454, 37), 116}, // 56 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7387, 34), 115}, // 57 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7642, 20), 121}, // 58 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7805, 18), 129}, // 59 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7262, 7), 111}, // 60 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7330, 30), 114}, // 61 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7491, 20), 117}, // 62 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7511, 19), 117}, // 63 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7151, 23), 108}, // 64 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7174, 22), 108}, // 65 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4840, 11), 12}, // 66 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5528, 14), 47}, // 67 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4941, 13), 14}, // 68 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6787, 22), 95}, // 69 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6903, 12), 100}, // 70 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6915, 11), 100}, // 71 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6926, 14), 101}, // 72 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6940, 13), 101}, // 73 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4954, 6), 15}, // 74 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6582, 15), 89}, // 75 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5255, 18), 32}, // 76 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5292, 21), 34}, // 77 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6597, 16), 90}, // 78 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6613, 18), 91}, // 79 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5458, 21), 43}, // 80 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5445, 13), 42}, // 81 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5215, 14), 29}, // 82 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7066, 22), 105}, // 83 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7088, 21), 105}, // 84 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7029, 19), 104}, // 85 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7048, 18), 104}, // 86 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7196, 17), 109}, // 87 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7213, 16), 109}, // 88 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5088, 14), 20}, // 89 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5112, 11), 22}, // 90 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4795, 10), 8}, // 91 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4779, 9), 7}, // 92 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6390, 18), 81}, // 93 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6502, 17), 85}, // 94 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4851, 12), 13}, // 95 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6519, 19), 86}, // 96 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6834, 24), 97}, // 97 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6809, 25), 96}, // 98 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5853, 24), 61}, // 99 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6858, 28), 98}, // 100 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7530, 20), 118}, // 101 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7130, 11), 107}, // 102 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7141, 10), 107}, // 103 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7109, 11), 106}, // 104 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7120, 10), 106}, // 105 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6228, 29), 76}, // 106 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7694, 10), 123}, // 107 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7713, 7), 125}, // 108 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5135, 9), 24}, // 109 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5177, 11), 26}, // 110 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5162, 15), 25}, // 111 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6327, 20), 79}, // 112 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6366, 24), 80}, // 113 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6275, 16), 77}, // 114 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5944, 15), 64}, // 115 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5617, 15), 53}, // 116 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5632, 18), 53}, // 117 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5672, 15), 54}, // 118 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5687, 18), 54}, // 119 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5705, 15), 55}, // 120 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5720, 18), 55}, // 121 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5479, 11), 44}, // 122 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5738, 15), 56}, // 123 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5753, 18), 56}, // 124 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5490, 26), 45}, // 125 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5771, 15), 57}, // 126 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5786, 18), 57}, // 127 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5429, 16), 41}, // 128 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5382, 13), 40}, // 129 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6490, 12), 84}, // 130 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5078, 10), 19}, // 131 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5063, 15), 18}, // 132 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5048, 15), 17}, // 133 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6008, 18), 67}, // 134 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5990, 18), 66}, // 135 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5959, 15), 65}, // 136 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4831, 9), 11}, // 137 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5516, 12), 46}, // 138 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5924, 10), 63}, // 139 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5000, 14), 16}, // 140 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6291, 15), 78}, // 141 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6428, 22), 82}, // 142 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5594, 10), 51}, // 143 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7704, 9), 124}, // 144 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5604, 13), 52}, // 145 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7662, 13), 122}, // 146 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5313, 8), 35}, // 147 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5243, 12), 31}, // 148 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(5229, 14), 30}, // 149 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6988, 21), 103}, // 150 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7009, 20), 103}, // 151 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6953, 18), 102}, // 152 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6971, 17), 102}, // 153 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7229, 17), 110}, // 154 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7246, 16), 110}, // 155 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(4676, 10), 137}, // 156 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12907, 34), 359}, // 157 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12595, 37), 347}, // 158 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12563, 32), 346}, // 159 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13457, 19), 383}, // 160 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12228, 9), 332}, // 161 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13374, 20), 379}, // 162 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12283, 23), 335}, // 163 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11756, 22), 310}, // 164 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13290, 20), 375}, // 165 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12237, 23), 333}, // 166 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13310, 20), 376}, // 167 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12260, 23), 334}, // 168 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8070, 14), 153}, // 169 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9457, 17), 229}, // 170 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13415, 24), 381}, // 171 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10079, 29), 263}, // 172 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10057, 22), 262}, // 173 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10041, 16), 261}, // 174 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14021, 20), 405}, // 175 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11693, 18), 307}, // 176 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13234, 16), 372}, // 177 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13006, 19), 363}, // 178 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13982, 19), 403}, // 179 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4805, 13), 163}, // 180 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11351, 32), 296}, // 181 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11383, 31), 296}, // 182 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10231, 31), 275}, // 183 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10262, 30), 275}, // 184 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11933, 30), 319}, // 185 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9826, 32), 248}, // 186 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11826, 31), 316}, // 187 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9105, 21), 370}, // 188 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9040, 28), 211}, // 189 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11439, 20), 298}, // 190 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11857, 40), 317}, // 191 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11796, 30), 315}, // 192 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11897, 36), 318}, // 193 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11736, 20), 309}, // 194 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11963, 28), 320}, // 195 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5552, 16), 203}, // 196 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4818, 13), 164}, // 197 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13394, 21), 380}, // 198 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11562, 25), 303}, // 199 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11587, 28), 303}, // 200 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9596, 18), 235}, // 201 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9581, 15), 234}, // 202 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8517, 18), 182}, // 203 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8041, 14), 151}, // 204 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5912, 12), 225}, // 205 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11615, 23), 304}, // 206 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13185, 11), 368}, // 207 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13079, 22), 366}, // 208 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13101, 25), 366}, // 209 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13126, 28), 367}, // 210 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13154, 31), 367}, // 211 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13038, 19), 365}, // 212 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13057, 22), 365}, // 213 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13196, 14), 368}, // 214 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5815, 15), 216}, // 215 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12340, 16), 338}, // 216 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12543, 20), 345}, // 217 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13591, 28), 389}, // 218 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12883, 24), 358}, // 219 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12756, 27), 353}, // 220 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13476, 29), 384}, // 221 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12797, 20), 355}, // 222 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12843, 40), 357}, // 223 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12706, 26), 351}, // 224 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13505, 28), 385}, // 225 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13567, 24), 388}, // 226 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12662, 22), 349}, // 227 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12732, 24), 352}, // 228 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12517, 26), 344}, // 229 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13025, 13), 364}, // 230 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13551, 16), 387}, // 231 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7955, 8), 142}, // 232 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7941, 14), 141}, // 233 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9881, 16), 250}, // 234 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7963, 8), 143}, // 235 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9078, 27), 213}, // 236 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9068, 10), 212}, // 237 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13275, 15), 374}, // 238 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12136, 23), 328}, // 239 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6656, 23), 274}, // 240 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10209, 22), 274}, // 241 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6745, 19), 276}, // 242 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6466, 24), 270}, // 243 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9919, 16), 252}, // 244 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11530, 32), 302}, // 245 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11459, 33), 299}, // 246 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11492, 38), 300}, // 247 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5877, 23), 214}, // 248 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12491, 26), 343}, // 249 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12182, 22), 330}, // 250 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13882, 22), 399}, // 251 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8362, 15), 169}, // 252 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4863, 9), 135}, // 253 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8112, 18), 156}, // 254 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10137, 28), 265}, // 255 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8575, 16), 185}, // 256 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13649, 35), 391}, // 257 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13619, 30), 390}, // 258 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9031, 9), 210}, // 259 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5395, 16), 192}, // 260 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8714, 26), 194}, // 261 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5650, 22), 195}, // 262 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8794, 25), 198}, // 263 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10306, 29), 277}, // 264 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8819, 20), 199}, // 265 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13250, 25), 373}, // 266 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8740, 23), 196}, // 267 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8763, 31), 197}, // 268 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8694, 20), 193}, // 269 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13904, 26), 400}, // 270 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8034, 7), 150}, // 271 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12993, 13), 362}, // 272 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8431, 8), 175}, // 273 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7990, 11), 146}, // 274 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8453, 12), 178}, // 275 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8308, 15), 165}, // 276 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10192, 17), 272}, // 277 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9897, 22), 251}, // 278 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8130, 20), 157}, // 279 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8465, 13), 179}, // 280 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8016, 12), 148}, // 281 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8506, 11), 181}, // 282 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8001, 15), 147}, // 283 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9935, 21), 254}, // 284 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8340, 10), 167}, // 285 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12204, 24), 331}, // 286 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8382, 16), 171}, // 287 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10421, 36), 280}, // 288 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10457, 39), 280}, // 289 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10962, 39), 287}, // 290 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11001, 42), 287}, // 291 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8084, 6), 154}, // 292 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10014, 27), 260}, // 293 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10000, 14), 259}, // 294 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7971, 6), 144}, // 295 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7977, 13), 145}, // 296 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9956, 14), 255}, // 297 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8377, 5), 170}, // 298 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12159, 23), 329}, // 299 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8535, 22), 183}, // 300 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4630, 7), 139}, // 301 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12684, 22), 350}, // 302 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7924, 8), 138}, // 303 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8055, 15), 152}, // 304 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13330, 20), 377}, // 305 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12783, 14), 354}, // 306 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13957, 25), 402}, // 307 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7917, 7), 133}, // 308 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12817, 26), 356}, // 309 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4926, 15), 273}, // 310 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4912, 14), 271}, // 311 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8414, 7), 173}, // 312 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5934, 10), 226}, // 313 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5014, 14), 188}, // 314 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8669, 13), 190}, // 315 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13350, 11), 378}, // 316 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13361, 13), 378}, // 317 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6408, 20), 269}, // 318 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4748, 31), 295}, // 319 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11317, 34), 295}, // 320 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8682, 12), 191}, // 321 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8028, 6), 149}, // 322 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9985, 15), 258}, // 323 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11778, 18), 312}, // 324 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7902, 15), 369}, // 325 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9694, 12), 240}, // 326 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11711, 25), 308}, // 327 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9671, 23), 239}, // 328 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7849, 41), 321}, // 329 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7421, 33), 311}, // 330 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4898, 14), 243}, // 331 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7603, 39), 314}, // 332 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7307, 23), 292}, // 333 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4885, 13), 291}, // 334 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11638, 29), 305}, // 335 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7360, 27), 290}, // 336 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11414, 25), 297}, // 337 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7564, 28), 313}, // 338 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9725, 32), 242}, // 339 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14001, 20), 404}, // 340 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13210, 24), 371}, // 341 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12115, 21), 327}, // 342 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9639, 16), 237}, // 343 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9655, 16), 238}, // 344 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12964, 29), 361}, // 345 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10372, 23), 279}, // 346 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10395, 26), 279}, // 347 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10108, 29), 264}, // 348 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9474, 28), 230}, // 349 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5144, 18), 166}, // 350 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8421, 10), 174}, // 351 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8439, 14), 177}, // 352 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8323, 17), 176}, // 353 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8208, 33), 160}, // 354 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10735, 36), 284}, // 355 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10771, 39), 284}, // 356 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8350, 12), 168}, // 357 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4788, 7), 134}, // 358 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9970, 15), 256}, // 359 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6257, 18), 257}, // 360 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11667, 26), 306}, // 361 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4960, 12), 200}, // 362 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10335, 17), 278}, // 363 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10352, 20), 278}, // 364 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7675, 19), 301}, // 365 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6347, 19), 268}, // 366 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5028, 20), 201}, // 367 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4972, 28), 266}, // 368 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10165, 27), 266}, // 369 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6306, 21), 267}, // 370 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10292, 14), 276}, // 371 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9614, 25), 236}, // 372 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8398, 16), 172}, // 373 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13861, 21), 398}, // 374 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13439, 18), 382}, // 375 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6211, 17), 253}, // 376 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9262, 25), 221}, // 377 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9502, 24), 231}, // 378 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8241, 34), 161}, // 379 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10810, 37), 285}, // 380 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10847, 40), 285}, // 381 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8275, 33), 162}, // 382 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10887, 36), 286}, // 383 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10923, 39), 286}, // 384 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8478, 28), 180}, // 385 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8150, 24), 158}, // 386 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8591, 30), 186}, // 387 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8621, 31), 187}, // 388 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9389, 21), 224}, // 389 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9367, 22), 223}, // 390 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9560, 21), 233}, // 391 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8954, 37), 208}, // 392 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8991, 40), 209}, // 393 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10577, 39), 282}, // 394 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10616, 42), 282}, // 395 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11130, 42), 289}, // 396 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11172, 45), 289}, // 397 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9350, 17), 222}, // 398 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9287, 28), 221}, // 399 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13712, 23), 393}, // 400 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13735, 30), 394}, // 401 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13765, 30), 395}, // 402 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12427, 39), 341}, // 403 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12356, 33), 339}, // 404 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12389, 38), 340}, // 405 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5411, 18), 215}, // 406 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12031, 27), 324}, // 407 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13684, 28), 392}, // 408 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8652, 17), 189}, // 409 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12058, 26), 325}, // 410 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12084, 31), 326}, // 411 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13795, 38), 396}, // 412 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12010, 21), 323}, // 413 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9246, 16), 220}, // 414 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13533, 18), 386}, // 415 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11991, 19), 322}, // 416 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13930, 27), 401}, // 417 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8943, 11), 207}, // 418 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13833, 28), 397}, // 419 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(4872, 13), 136}, // 420 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8090, 22), 155}, // 421 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9858, 23), 249}, // 422 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9804, 22), 246}, // 423 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9783, 21), 245}, // 424 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9757, 26), 244}, // 425 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8857, 28), 204}, // 426 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8885, 28), 205}, // 427 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8913, 30), 206}, // 428 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5974, 16), 247}, // 429 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8557, 18), 184}, // 430 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12941, 23), 360}, // 431 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9315, 35), 222}, // 432 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9526, 34), 232}, // 433 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8174, 34), 159}, // 434 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10658, 37), 283}, // 435 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10695, 40), 283}, // 436 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8839, 18), 202}, // 437 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10496, 39), 281}, // 438 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10535, 42), 281}, // 439 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11043, 42), 288}, // 440 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11085, 45), 288}, // 441 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12632, 30), 348}, // 442 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9706, 19), 241}, // 443 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12466, 25), 342}, // 444 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9440, 17), 228}, // 445 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9410, 30), 227}, // 446 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7932, 9), 140}, // 447 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12325, 15), 337}, // 448 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12306, 19), 336}, // 449 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11217, 18), 293}, // 450 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11256, 29), 294}, // 451 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11285, 32), 294}, // 452 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11235, 21), 293}, // 453 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9202, 44), 219}, // 454 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9159, 43), 218}, // 455 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9126, 33), 217}, // 456 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14053, 8), 407}, // 457 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14061, 8), 408}, // 458 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14069, 6), 409}, // 459 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14075, 7), 410}, // 460 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14082, 11), 411}, // 461 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14093, 9), 412}, // 462 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14102, 13), 413}, // 463 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14041, 12), 406}, // 464 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14115, 6), 414}, // 465 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14121, 10), 415}, // 466 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14131, 6), 416}, // 467 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14152, 20), 418}, // 468 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14137, 15), 417}, // 469 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14261, 15), 426}, // 470 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14276, 13), 427}, // 471 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14249, 12), 425}, // 472 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14339, 21), 431}, // 473 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14232, 17), 424}, // 474 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14416, 16), 435}, // 475 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14220, 12), 423}, // 476 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14400, 16), 434}, // 477 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14193, 14), 421}, // 478 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14177, 16), 420}, // 479 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14207, 13), 422}, // 480 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14360, 20), 432}, // 481 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14304, 18), 429}, // 482 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14289, 15), 428}, // 483 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14380, 20), 433}, // 484 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14322, 17), 430}, // 485 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14452, 24), 437}, // 486 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14432, 20), 436}, // 487 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14172, 5), 419}, // 488 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(14537, 9), 442}, // 489 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14569, 7), 446}, // 490 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14476, 6), 438}, // 491 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14576, 9), 447}, // 492 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14487, 6), 440}, // 493 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14482, 5), 439}, // 494 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14526, 11), 441}, // 495 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14558, 11), 445}, // 496 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14546, 5), 443}, // 497 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14551, 7), 444}, // 498 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(14621, 11), 451}, // 499 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14585, 10), 448}, // 500 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14608, 13), 450}, // 501 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14595, 13), 449}, // 502 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14632, 10), 452}, // 503 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14642, 10), 453}, // 504 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14652, 10), 454}, // 505 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14820, 12), 465}, // 506 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14832, 12), 466}, // 507 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14675, 14), 456}, // 508 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14689, 14), 457}, // 509 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14703, 14), 458}, // 510 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14662, 13), 455}, // 511 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14780, 19), 463}, // 512 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14732, 16), 460}, // 513 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14748, 16), 461}, // 514 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14764, 16), 462}, // 515 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14717, 15), 459}, // 516 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14799, 21), 464}, // 517 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(14896, 28), 470}, // 518 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14856, 15), 468}, // 519 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14871, 25), 469}, // 520 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14844, 12), 467}, // 521 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(14932, 27), 472}, // 522 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14959, 27), 473}, // 523 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14986, 27), 474}, // 524 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15013, 32), 475}, // 525 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(14924, 8), 471}, // 526 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15045, 26), 476}, // 527 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(15082, 4), 479}, // 528 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15075, 7), 478}, // 529 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15071, 4), 477}, // 530 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(15086, 11), 480}, // 531 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15108, 21), 482}, // 532 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15097, 11), 481}, // 533 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(15140, 14), 484}, // 534 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15154, 21), 485}, // 535 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15129, 11), 483}, // 536 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(15175, 18), 486}, // 537 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(14053, 8), 488}, // 538 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14061, 8), 489}, // 539 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14069, 6), 490}, // 540 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14075, 7), 491}, // 541 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14082, 11), 492}, // 542 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14093, 9), 493}, // 543 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14102, 13), 494}, // 544 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14041, 12), 487}, // 545 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14115, 6), 495}, // 546 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14121, 10), 496}, // 547 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14131, 6), 497}, // 548 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(14261, 15), 505}, // 549 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14276, 13), 506}, // 550 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14249, 12), 504}, // 551 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14339, 21), 510}, // 552 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14232, 17), 503}, // 553 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14220, 12), 502}, // 554 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14400, 16), 513}, // 555 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14193, 14), 500}, // 556 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14177, 16), 499}, // 557 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14207, 13), 501}, // 558 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14360, 20), 511}, // 559 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14304, 18), 508}, // 560 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14289, 15), 507}, // 561 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14380, 20), 512}, // 562 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14322, 17), 509}, // 563 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14172, 5), 498}, // 564 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(14537, 9), 518}, // 565 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14569, 7), 522}, // 566 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14476, 6), 514}, // 567 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14487, 6), 516}, // 568 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14482, 5), 515}, // 569 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14526, 11), 517}, // 570 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14558, 11), 521}, // 571 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14546, 5), 519}, // 572 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14551, 7), 520}, // 573 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(14585, 10), 523}, // 574 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14608, 13), 525}, // 575 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(14595, 13), 524}, // 576 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17417, 16), 640}, // 577 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15402, 8), 545}, // 578 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(16520, 15), 598}, // 579 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16535, 18), 598}, // 580 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15880, 10), 569}, // 581 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(15924, 12), 571}, // 582 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(15253, 12), 532}, // 583 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(17198, 14), 628}, // 584 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17037, 15), 621}, // 585 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15557, 8), 558}, // 586 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(16601, 16), 601}, // 587 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16583, 18), 600}, // 588 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15217, 6), 528}, // 589 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(16064, 22), 578}, // 590 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16042, 22), 577}, // 591 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(16632, 13), 603}, // 592 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16617, 15), 602}, // 593 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15223, 12), 529}, // 594 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(17510, 20), 645}, // 595 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15334, 8), 537}, // 596 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(17280, 19), 633}, // 597 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15300, 8), 536}, // 598 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18320, 22), 668}, // 599 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18379, 23), 669}, // 600 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17299, 15), 634}, // 601 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15367, 9), 541}, // 602 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(16685, 13), 606}, // 603 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15428, 9), 548}, // 604 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(15244, 9), 531}, // 605 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(15541, 10), 556}, // 606 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(18267, 17), 667}, // 607 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17819, 27), 655}, // 608 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15419, 9), 547}, // 609 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(16831, 14), 613}, // 610 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15565, 14), 559}, // 611 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(17314, 28), 635}, // 612 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17090, 16), 624}, // 613 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16086, 18), 579}, // 614 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(15750, 15), 565}, // 615 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(17738, 26), 652}, // 616 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15700, 15), 564}, // 617 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(15356, 5), 539}, // 618 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(17212, 20), 629}, // 619 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15652, 14), 563}, // 620 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(16742, 21), 609}, // 621 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16973, 24), 617}, // 622 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17583, 31), 647}, // 623 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16916, 26), 616}, // 624 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17373, 25), 638}, // 625 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15289, 11), 535}, // 626 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(15278, 11), 534}, // 627 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(16805, 26), 612}, // 628 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16553, 30), 599}, // 629 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16845, 24), 613}, // 630 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(16882, 19), 614}, // 631 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(18162, 16), 664}, // 632 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17530, 19), 646}, // 633 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18239, 28), 666}, // 634 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15551, 6), 557}, // 635 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(18225, 14), 665}, // 636 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17446, 24), 642}, // 637 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15859, 21), 568}, // 638 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(15383, 10), 543}, // 639 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(17789, 30), 654}, // 640 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17764, 25), 653}, // 641 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15827, 18), 566}, // 642 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(15532, 9), 555}, // 643 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(17877, 33), 657}, // 644 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17910, 30), 658}, // 645 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17940, 28), 659}, // 646 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18036, 29), 661}, // 647 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18002, 34), 660}, // 648 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18065, 32), 662}, // 649 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17398, 19), 639}, // 650 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15265, 13), 533}, // 651 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(15890, 14), 570}, // 652 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(15936, 16), 572}, // 653 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(17470, 20), 643}, // 654 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17052, 22), 622}, // 655 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17106, 19), 625}, // 656 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17674, 18), 650}, // 657 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17011, 12), 619}, // 658 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17145, 11), 627}, // 659 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17433, 13), 641}, // 660 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15845, 14), 567}, // 661 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(15342, 14), 538}, // 662 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(15996, 13), 574}, // 663 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(16009, 15), 575}, // 664 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(16136, 20), 581}, // 665 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16104, 32), 580}, // 666 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(15449, 12), 550}, // 667 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(16460, 11), 596}, // 668 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(16471, 14), 596}, // 669 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15437, 12), 549}, // 670 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(17023, 14), 620}, // 671 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15579, 7), 560}, // 672 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(16304, 19), 588}, // 673 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16323, 14), 589}, // 674 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15361, 6), 540}, // 675 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(16276, 28), 587}, // 676 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16251, 25), 586}, // 677 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16199, 25), 584}, // 678 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16179, 20), 583}, // 679 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16224, 27), 585}, // 680 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(16384, 16), 592}, // 681 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16400, 15), 592}, // 682 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16425, 10), 594}, // 683 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16435, 13), 595}, // 684 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(16448, 12), 595}, // 685 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16415, 10), 593}, // 686 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(17490, 20), 644}, // 687 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17342, 14), 636}, // 688 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16659, 26), 605}, // 689 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16997, 14), 618}, // 690 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17846, 31), 656}, // 691 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15193, 17), 526}, // 692 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(15393, 9), 544}, // 693 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(16485, 16), 597}, // 694 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(16501, 19), 597}, // 695 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(15235, 9), 530}, // 696 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(16645, 14), 604}, // 697 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15376, 7), 542}, // 698 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(15505, 20), 553}, // 699 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(15952, 44), 573}, // 700 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(16356, 28), 591}, // 701 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(16698, 17), 607}, // 702 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17125, 20), 626}, // 703 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17614, 25), 648}, // 704 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17074, 16), 623}, // 705 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15210, 7), 527}, // 706 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(18097, 26), 663}, // 707 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16790, 15), 611}, // 708 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17356, 17), 637}, // 709 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17692, 15), 651}, // 710 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15525, 7), 554}, // 711 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(17232, 16), 630}, // 712 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16156, 23), 582}, // 713 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(17262, 18), 632}, // 714 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15461, 8), 551}, // 715 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(15495, 10), 552}, // 716 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(16869, 13), 614}, // 717 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(16901, 15), 615}, // 718 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(17639, 35), 649}, // 719 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16763, 27), 610}, // 720 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16715, 27), 608}, // 721 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(16337, 19), 590}, // 722 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(15410, 9), 546}, // 723 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(16024, 18), 576}, // 724 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(17248, 14), 631}, // 725 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(15586, 10), 561}, // 726 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(15596, 10), 562}, // 727 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18402, 3), 670}, // 728 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18405, 3), 671}, // 729 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18408, 3), 672}, // 730 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18421, 7), 675}, // 731 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18411, 5), 673}, // 732 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18416, 5), 674}, // 733 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18428, 12), 676}, // 734 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(18440, 17), 677}, // 735 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(19252, 15), 730}, // 736 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18883, 15), 708}, // 737 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9596, 18), 722}, // 738 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9581, 15), 721}, // 739 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18653, 13), 691}, // 740 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18666, 10), 692}, // 741 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18638, 15), 690}, // 742 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18676, 15), 693}, // 743 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19673, 25), 748}, // 744 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19698, 24), 748}, // 745 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19626, 24), 747}, // 746 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19650, 23), 747}, // 747 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19200, 29), 728}, // 748 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18605, 19), 687}, // 749 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20183, 18), 767}, // 750 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18910, 10), 710}, // 751 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19998, 26), 759}, // 752 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20024, 27), 760}, // 753 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18898, 12), 709}, // 754 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18727, 11), 697}, // 755 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18738, 20), 698}, // 756 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18715, 12), 696}, // 757 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18768, 24), 700}, // 758 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18457, 12), 678}, // 759 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19267, 15), 731}, // 760 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18798, 9), 702}, // 761 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18691, 10), 694}, // 762 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18701, 14), 695}, // 763 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18978, 16), 715}, // 764 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18966, 12), 714}, // 765 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19282, 21), 732}, // 766 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19327, 21), 735}, // 767 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20073, 16), 762}, // 768 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20051, 22), 761}, // 769 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20159, 24), 766}, // 770 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20274, 22), 771}, // 771 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20296, 24), 772}, // 772 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20251, 23), 770}, // 773 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20371, 27), 773}, // 774 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20089, 20), 763}, // 775 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18994, 34), 716}, // 776 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19028, 34), 717}, // 777 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19062, 36), 718}, // 778 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19146, 34), 726}, // 779 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20109, 22), 764}, // 780 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18589, 16), 686}, // 781 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18573, 16), 685}, // 782 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18835, 16), 705}, // 783 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19558, 15), 745}, // 784 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19573, 14), 745}, // 785 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18822, 13), 704}, // 786 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19587, 20), 746}, // 787 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19607, 19), 746}, // 788 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18851, 20), 706}, // 789 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19722, 19), 749}, // 790 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19741, 18), 749}, // 791 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18807, 15), 703}, // 792 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18554, 19), 684}, // 793 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19759, 25), 750}, // 794 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19784, 27), 751}, // 795 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18624, 10), 688}, // 796 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19128, 18), 720}, // 797 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19499, 19), 742}, // 798 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18792, 6), 701}, // 799 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20225, 26), 769}, // 800 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19518, 20), 743}, // 801 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9639, 16), 724}, // 802 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19977, 21), 758}, // 803 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19956, 21), 757}, // 804 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9655, 16), 725}, // 805 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19811, 26), 752}, // 806 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19837, 28), 753}, // 807 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20131, 28), 765}, // 808 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6275, 16), 734}, // 809 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19865, 31), 754}, // 810 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19896, 33), 755}, // 811 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19929, 27), 756}, // 812 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19538, 20), 744}, // 813 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(9614, 25), 723}, // 814 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18469, 13), 679}, // 815 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18482, 22), 680}, // 816 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18504, 21), 681}, // 817 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19303, 24), 733}, // 818 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19452, 25), 740}, // 819 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19376, 26), 737}, // 820 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19477, 22), 741}, // 821 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19402, 23), 738}, // 822 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19229, 23), 729}, // 823 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19425, 27), 739}, // 824 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19348, 28), 736}, // 825 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20201, 24), 768}, // 826 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(5382, 13), 711}, // 827 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19098, 30), 719}, // 828 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18920, 22), 712}, // 829 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18942, 24), 713}, // 830 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(19180, 20), 727}, // 831 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18758, 10), 699}, // 832 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18871, 12), 707}, // 833 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18539, 15), 683}, // 834 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18525, 14), 682}, // 835 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(18634, 4), 689}, // 836 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(20536, 10), 785}, // 837 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20546, 9), 785}, // 838 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20597, 12), 788}, // 839 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20609, 11), 788}, // 840 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20555, 14), 786}, // 841 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20569, 13), 786}, // 842 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(14576, 9), 778}, // 843 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20448, 10), 779}, // 844 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4863, 9), 777}, // 845 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20505, 16), 784}, // 846 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20521, 15), 784}, // 847 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(4630, 7), 780}, // 848 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20628, 8), 790}, // 849 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20465, 7), 782}, // 850 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20582, 8), 787}, // 851 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20590, 7), 787}, // 852 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20472, 17), 783}, // 853 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20489, 16), 783}, // 854 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20620, 8), 789}, // 855 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20458, 7), 781}, // 856 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20405, 20), 775}, // 857 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20425, 23), 776}, // 858 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20398, 7), 774}, // 859 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(20645, 12), 792}, // 860 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20636, 9), 791}, // 861 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(20657, 12), 793}, // 862 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(20669, 14), 794}, // 863 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20683, 14), 795}, // 864 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(20702, 4), 797}, // 865 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20697, 5), 796}, // 866 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(20740, 14), 804}, // 867 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20754, 23), 804}, // 868 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20777, 13), 805}, // 869 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20790, 18), 805}, // 870 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20724, 11), 802}, // 871 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20808, 15), 806}, // 872 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20735, 5), 803}, // 873 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20720, 4), 801}, // 874 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(14172, 5), 798}, // 875 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20713, 7), 800}, // 876 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20706, 7), 799}, // 877 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(20823, 4), 807}, // 878 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20835, 4), 810}, // 879 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20831, 4), 809}, // 880 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20827, 4), 808}, // 881 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(20871, 18), 813}, // 882 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20889, 18), 814}, // 883 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20839, 16), 811}, // 884 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20855, 16), 812}, // 885 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(20930, 6), 819}, // 886 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20914, 11), 817}, // 887 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20907, 7), 816}, // 888 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(14172, 5), 815}, // 889 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13350, 11), 820}, // 890 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(13361, 13), 820}, // 891 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20925, 5), 818}, // 892 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(20946, 6), 823}, // 893 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20957, 8), 825}, // 894 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20965, 10), 826}, // 895 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20983, 12), 828}, // 896 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20975, 8), 827}, // 897 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20995, 20), 829}, // 898 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20941, 5), 822}, // 899 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20952, 5), 824}, // 900 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(20936, 5), 821}, // 901 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(21050, 16), 833}, // 902 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21036, 14), 832}, // 903 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21022, 14), 831}, // 904 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21113, 27), 836}, // 905 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21086, 27), 835}, // 906 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21066, 20), 834}, // 907 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21015, 7), 830}, // 908 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(21140, 10), 837}, // 909 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21150, 10), 838}, // 910 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21171, 15), 840}, // 911 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21160, 11), 839}, // 912 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(21186, 5), 842}, // 913 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(21200, 12), 845}, // 914 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(21212, 13), 847}, // 915 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(21195, 5), 844}, // 916 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(21191, 4), 843}, // 917 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(21225, 19), 850}, // 918 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(21244, 22), 850}, // 919 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21266, 17), 851}, // 920 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(21283, 20), 851}, // 921 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(8414, 7), 849}, // 922 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(21303, 16), 852}, // 923 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21319, 19), 852}, // 924 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(14172, 5), 841}, // 925 None in SPV_OPERAND_TYPE_IMAGE
  {IR(21391, 12), 856}, // 926 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(15579, 7), 846}, // 927 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(21403, 8), 857}, // 928 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(15376, 7), 848}, // 929 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(21369, 11), 854}, // 930 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(21338, 14), 853}, // 931 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(21352, 17), 853}, // 932 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(21380, 11), 855}, // 933 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(14069, 6), 872}, // 934 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21572, 10), 871}, // 935 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21507, 12), 866}, // 936 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21519, 12), 867}, // 937 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21496, 11), 865}, // 938 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21421, 11), 859}, // 939 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21411, 10), 858}, // 940 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21481, 15), 864}, // 941 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21593, 17), 874}, // 942 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21610, 16), 875}, // 943 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21743, 16), 882}, // 944 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21759, 16), 883}, // 945 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21442, 11), 861}, // 946 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21582, 11), 873}, // 947 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21666, 20), 878}, // 948 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21432, 10), 860}, // 949 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21467, 14), 863}, // 950 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21453, 14), 862}, // 951 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21686, 19), 879}, // 952 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21705, 19), 880}, // 953 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21724, 19), 881}, // 954 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21544, 14), 869}, // 955 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21558, 14), 870}, // 956 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21531, 13), 868}, // 957 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21626, 20), 876}, // 958 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21646, 20), 877}, // 959 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(21777, 2), 885}, // 960 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21878, 5), 903}, // 961 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21799, 5), 891}, // 962 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21794, 5), 890}, // 963 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21836, 6), 897}, // 964 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21842, 13), 898}, // 965 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21804, 10), 892}, // 966 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21814, 10), 893}, // 967 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21775, 2), 884}, // 968 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21782, 3), 887}, // 969 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21779, 3), 886}, // 970 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21785, 4), 888}, // 971 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21789, 5), 889}, // 972 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21831, 5), 896}, // 973 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21827, 4), 895}, // 974 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21824, 3), 894}, // 975 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21872, 6), 902}, // 976 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21855, 5), 899}, // 977 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21866, 6), 901}, // 978 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21860, 6), 900}, // 979 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(21883, 27), 904}, // 980 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21910, 23), 905}, // 981 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(21933, 7), 906}, // 982 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21940, 11), 907}, // 983 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21951, 14), 908}, // 984 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(21965, 12), 910}, // 985 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(14172, 5), 909}, // 986 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(21977, 13), 912}, // 987 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(14172, 5), 911}, // 988 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(21990, 7), 913}, // 989 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(21997, 7), 914}, // 990 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(22004, 12), 915}, // 991 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(22030, 12), 917}, // 992 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22082, 17), 920}, // 993 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22057, 25), 919}, // 994 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22042, 15), 918}, // 995 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22016, 14), 916}, // 996 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(22222, 21), 933}, // 997 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22117, 19), 924}, // 998 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22136, 17), 925}, // 999 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22106, 11), 923}, // 1000 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17446, 24), 931}, // 1001 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22181, 18), 928}, // 1002 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22243, 18), 935}, // 1003 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22322, 15), 939}, // 1004 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17470, 20), 932}, // 1005 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22261, 21), 936}, // 1006 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22167, 14), 927}, // 1007 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22337, 26), 940}, // 1008 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22153, 14), 926}, // 1009 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22308, 14), 938}, // 1010 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(14172, 5), 921}, // 1011 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22209, 13), 930}, // 1012 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22199, 10), 929}, // 1013 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(17490, 20), 934}, // 1014 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22282, 26), 937}, // 1015 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22099, 7), 922}, // 1016 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(22646, 27), 954}, // 1017 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22594, 26), 952}, // 1018 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22514, 23), 948}, // 1019 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22468, 23), 946}, // 1020 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22363, 29), 942}, // 1021 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22560, 17), 950}, // 1022 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22673, 27), 955}, // 1023 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22620, 26), 953}, // 1024 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22537, 23), 949}, // 1025 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22491, 23), 947}, // 1026 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22392, 29), 943}, // 1027 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22577, 17), 951}, // 1028 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22421, 21), 944}, // 1029 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22442, 26), 945}, // 1030 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(14172, 5), 941}, // 1031 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(22833, 20), 963}, // 1032 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22700, 8), 958}, // 1033 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22708, 21), 960}, // 1034 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22729, 24), 960}, // 1035 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22753, 19), 961}, // 1036 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22772, 22), 961}, // 1037 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22853, 17), 964}, // 1038 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22794, 18), 962}, // 1039 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22812, 21), 962}, // 1040 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(14172, 5), 956}, // 1041 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(21391, 12), 959}, // 1042 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(15410, 9), 957}, // 1043 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(22877, 8), 966}, // 1044 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22885, 7), 967}, // 1045 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22870, 7), 965}, // 1046 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22892, 7), 968}, // 1047 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22899, 10), 968}, // 1048 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(22917, 8), 970}, // 1049 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22933, 15), 972}, // 1050 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23037, 20), 978}, // 1051 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23016, 21), 977}, // 1052 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23057, 12), 979}, // 1053 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23098, 14), 981}, // 1054 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23112, 17), 981}, // 1055 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23129, 12), 982}, // 1056 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23141, 15), 982}, // 1057 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(14172, 5), 969}, // 1058 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23069, 13), 980}, // 1059 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23082, 16), 980}, // 1060 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22909, 8), 969}, // 1061 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22925, 8), 971}, // 1062 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22948, 23), 973}, // 1063 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22985, 15), 975}, // 1064 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(22971, 14), 974}, // 1065 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(15410, 9), 983}, // 1066 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23000, 16), 976}, // 1067 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(23156, 10), 984}, // 1068 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(23171, 4), 986}, // 1069 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23184, 8), 988}, // 1070 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23175, 9), 987}, // 1071 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23166, 5), 985}, // 1072 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(23192, 25), 989}, // 1073 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23217, 28), 989}, // 1074 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(23258, 4), 992}, // 1075 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23291, 9), 996}, // 1076 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23300, 13), 997}, // 1077 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23271, 8), 994}, // 1078 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23279, 12), 995}, // 1079 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23262, 9), 993}, // 1080 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23245, 4), 990}, // 1081 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(23249, 9), 991}, // 1082 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(14172, 5), 998}, // 1083 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23313, 25), 999}, // 1084 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23338, 23), 1000}, // 1085 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(23430, 27), 1006}, // 1086 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23457, 28), 1007}, // 1087 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23499, 16), 1009}, // 1088 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23485, 14), 1008}, // 1089 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23569, 30), 1012}, // 1090 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23371, 12), 1003}, // 1091 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(14924, 8), 1001}, // 1092 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23361, 10), 1002}, // 1093 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23556, 13), 1011}, // 1094 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23532, 24), 1010}, // 1095 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23406, 24), 1005}, // 1096 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23515, 17), 1010}, // 1097 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23383, 23), 1004}, // 1098 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(23640, 37), 1014}, // 1099 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23599, 41), 1013}, // 1100 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(23755, 42), 1017}, // 1101 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23677, 37), 1015}, // 1102 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23714, 41), 1016}, // 1103 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(23797, 33), 1018}, // 1104 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23830, 33), 1019}, // 1105 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(23875, 6), 1022}, // 1106 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23863, 12), 1021}, // 1107 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(14172, 5), 1020}, // 1108 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23881, 7), 1023}, // 1109 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23888, 15), 1024}, // 1110 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(23911, 7), 1026}, // 1111 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23903, 8), 1025}, // 1112 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(23976, 13), 1035}, // 1113 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24018, 4), 1041}, // 1114 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24056, 9), 1046}, // 1115 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23989, 5), 1036}, // 1116 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24118, 5), 1055}, // 1117 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24189, 6), 1065}, // 1118 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23942, 5), 1030}, // 1119 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24096, 5), 1051}, // 1120 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24153, 6), 1060}, // 1121 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24206, 5), 1068}, // 1122 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24200, 6), 1067}, // 1123 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24022, 3), 1042}, // 1124 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24065, 8), 1047}, // 1125 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24123, 4), 1056}, // 1126 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24195, 5), 1066}, // 1127 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24009, 5), 1039}, // 1128 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24037, 10), 1044}, // 1129 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23970, 6), 1034}, // 1130 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24107, 6), 1053}, // 1131 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24176, 7), 1063}, // 1132 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23964, 6), 1033}, // 1133 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24101, 6), 1052}, // 1134 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24169, 7), 1062}, // 1135 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24014, 4), 1040}, // 1136 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24047, 9), 1045}, // 1137 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24113, 5), 1054}, // 1138 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24183, 6), 1064}, // 1139 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24001, 8), 1038}, // 1140 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24159, 10), 1061}, // 1141 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23994, 7), 1037}, // 1142 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24025, 12), 1043}, // 1143 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23934, 8), 1029}, // 1144 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24081, 8), 1049}, // 1145 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24136, 9), 1058}, // 1146 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23926, 8), 1028}, // 1147 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24073, 8), 1048}, // 1148 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24127, 9), 1057}, // 1149 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23947, 6), 1031}, // 1150 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23953, 11), 1032}, // 1151 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24089, 7), 1050}, // 1152 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24145, 8), 1059}, // 1153 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(23918, 8), 1027}, // 1154 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(24211, 12), 1069}, // 1155 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24223, 7), 1070}, // 1156 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24249, 11), 1073}, // 1157 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24260, 12), 1074}, // 1158 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24272, 15), 1074}, // 1159 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24287, 14), 1075}, // 1160 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24240, 9), 1072}, // 1161 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24230, 10), 1071}, // 1162 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(24309, 12), 1078}, // 1163 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24301, 8), 1077}, // 1164 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(14172, 5), 1076}, // 1165 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(24321, 29), 1079}, // 1166 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(14053, 8), 1081}, // 1167 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14061, 8), 1082}, // 1168 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14069, 6), 1083}, // 1169 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14075, 7), 1084}, // 1170 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14082, 11), 1085}, // 1171 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14093, 9), 1086}, // 1172 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14102, 13), 1087}, // 1173 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14041, 12), 1080}, // 1174 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(14115, 6), 1088}, // 1175 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14121, 10), 1089}, // 1176 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14131, 6), 1090}, // 1177 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(14152, 20), 1092}, // 1178 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14137, 15), 1091}, // 1179 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(14261, 15), 1100}, // 1180 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14276, 13), 1101}, // 1181 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14249, 12), 1099}, // 1182 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14339, 21), 1105}, // 1183 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14232, 17), 1098}, // 1184 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14416, 16), 1109}, // 1185 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14220, 12), 1097}, // 1186 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14400, 16), 1108}, // 1187 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14193, 14), 1095}, // 1188 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14177, 16), 1094}, // 1189 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14207, 13), 1096}, // 1190 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14360, 20), 1106}, // 1191 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14304, 18), 1103}, // 1192 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14289, 15), 1102}, // 1193 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14380, 20), 1107}, // 1194 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14322, 17), 1104}, // 1195 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14452, 24), 1111}, // 1196 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14432, 20), 1110}, // 1197 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(24350, 26), 1112}, // 1198 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14172, 5), 1093}, // 1199 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(14537, 9), 1117}, // 1200 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14569, 7), 1121}, // 1201 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14476, 6), 1113}, // 1202 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14576, 9), 1122}, // 1203 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14487, 6), 1115}, // 1204 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14482, 5), 1114}, // 1205 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14526, 11), 1116}, // 1206 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14558, 11), 1120}, // 1207 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14546, 5), 1118}, // 1208 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14551, 7), 1119}, // 1209 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(14621, 11), 1126}, // 1210 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14585, 10), 1123}, // 1211 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14608, 13), 1125}, // 1212 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(14595, 13), 1124}, // 1213 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(24411, 15), 1133}, // 1214 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24376, 5), 1128}, // 1215 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24381, 5), 1129}, // 1216 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24431, 7), 1135}, // 1217 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24406, 5), 1132}, // 1218 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24438, 5), 1136}, // 1219 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24386, 9), 1130}, // 1220 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24395, 11), 1131}, // 1221 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24458, 5), 1140}, // 1222 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24426, 5), 1134}, // 1223 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24448, 6), 1138}, // 1224 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(23918, 8), 1127}, // 1225 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24443, 5), 1137}, // 1226 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24454, 4), 1139}, // 1227 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(24545, 14), 1151}, // 1228 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24627, 16), 1157}, // 1229 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24643, 15), 1157}, // 1230 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24941, 17), 1166}, // 1231 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24492, 15), 1146}, // 1232 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24958, 16), 1167}, // 1233 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24515, 9), 1148}, // 1234 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24524, 8), 1149}, // 1235 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24732, 16), 1160}, // 1236 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24748, 15), 1160}, // 1237 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24896, 21), 1164}, // 1238 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24974, 14), 1168}, // 1239 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24559, 6), 1152}, // 1240 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24658, 24), 1158}, // 1241 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24682, 23), 1158}, // 1242 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24763, 22), 1161}, // 1243 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24785, 21), 1161}, // 1244 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24479, 6), 1142}, // 1245 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24611, 16), 1156}, // 1246 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24485, 7), 1144}, // 1247 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24849, 22), 1163}, // 1248 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24871, 25), 1163}, // 1249 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24507, 8), 1147}, // 1250 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24532, 13), 1150}, // 1251 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24705, 14), 1159}, // 1252 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24719, 13), 1159}, // 1253 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24806, 22), 1162}, // 1254 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24828, 21), 1162}, // 1255 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24565, 14), 1153}, // 1256 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24917, 24), 1165}, // 1257 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24592, 19), 1155}, // 1258 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24579, 13), 1154}, // 1259 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(15461, 8), 1143}, // 1260 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24463, 16), 1141}, // 1261 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(24230, 10), 1145}, // 1262 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(22042, 15), 1172}, // 1263 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(22016, 14), 1169}, // 1264 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25006, 15), 1171}, // 1265 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(24988, 18), 1170}, // 1266 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(25032, 11), 1175}, // 1267 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(14172, 5), 1173}, // 1268 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(25021, 11), 1174}, // 1269 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(23863, 12), 1178}, // 1270 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(15419, 9), 1177}, // 1271 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23881, 7), 1179}, // 1272 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(23888, 15), 1180}, // 1273 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(25043, 10), 1176}, // 1274 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(25096, 24), 1184}, // 1275 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25120, 22), 1185}, // 1276 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25142, 21), 1186}, // 1277 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25053, 8), 1181}, // 1278 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25061, 15), 1182}, // 1279 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(25076, 20), 1183}, // 1280 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 148);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(156, 301);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(457, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(465, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(468, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(470, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(489, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(499, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(503, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(518, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(522, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(528, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(531, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(534, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(538, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(546, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(549, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(565, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(574, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(577, 151);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(728, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(736, 101);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(837, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(860, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(862, 3);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(865, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(867, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(878, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(882, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(886, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(893, 9);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(902, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(909, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(913, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(934, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(960, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(980, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(982, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(985, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(987, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(989, 3);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(992, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(997, 20);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1017, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1032, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1044, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1049, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1068, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1069, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1073, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1075, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1083, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1086, 13);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1099, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1101, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1104, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1106, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1111, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1113, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1155, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1163, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1166, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1167, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1175, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1178, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1180, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1200, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1210, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1214, 14);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1228, 35);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1263, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1267, 3);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1270, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1275, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(913, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1032, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1073, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(522, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1017, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1083, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(862, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1275, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(156, 301);
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
static const std::array<InstructionDesc, 820> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(25163, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(25219, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(25225, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(25344, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(25351, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(25367, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(25372, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(25383, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(25390, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(25395, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(25405, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(25465, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(25537, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(25611, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(25654, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(25696, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(25707, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(25716, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(25725, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(25770, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(25780, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(25791, 11), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(25915, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(25925, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(25937, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(25954, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(25964, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(25981, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(25992, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(26034, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(26046, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(26059, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(26069, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(26085, 14), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(26099, 10), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(26109, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(26118, 19), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(26137, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(26150, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(15419, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(26202, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(26298, 16), IR(0, 0), IR(203, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(26314, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(26327, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(26344, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(26362, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(26375, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(26438, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(24515, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(26487, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(26505, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(26517, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(26530, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(26539, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(26597, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(26602, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(26608, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(26619, 16), IR(0, 0), IR(204, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(26635, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(26647, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(26667, 15), IR(0, 0), IR(206, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(26682, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(26694, 23), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(26717, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(26768, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(26777, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(26792, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(26808, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(26867, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(26887, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(26908, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(26928, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(26942, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(26961, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(26978, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(26994, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(27005, 10), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(27015, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(27060, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(27106, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(27129, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(27156, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(27183, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(27210, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(27237, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(27268, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(27299, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(27310, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(27322, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(27338, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(27348, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(24559, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(27359, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(27376, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(27392, 18), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(27410, 15), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(27425, 14), IR(0, 0), IR(212, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(27439, 17), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(27456, 18), IR(0, 0), IR(210, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(27474, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(27486, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(27498, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(27510, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(27522, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(27531, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(27540, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(27549, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(27563, 14), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(27577, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(27592, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(27607, 14), IR(0, 0), IR(201, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(27621, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(27638, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(27655, 25), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(27680, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(27688, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(27696, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(27704, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(27709, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(27714, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(27719, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(27724, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(27729, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(27734, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(27739, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(27744, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(27749, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(27754, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(27759, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(27764, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(27769, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(27774, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(27792, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(27810, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(27828, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(27846, 18), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(27864, 13), IR(0, 0), IR(66, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(27877, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(27881, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(27891, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(27902, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(27915, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(27928, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(27932, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(27936, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(27942, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(27948, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(27957, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(27966, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(27977, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(27991, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(27999, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(28009, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(28022, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(28038, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(28048, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(28059, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(28070, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(28077, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(28084, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(28094, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(28107, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(28120, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(28138, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(28156, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(28166, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(28176, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(28191, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(28206, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(28216, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(28228, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(28241, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(28256, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(28269, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(28284, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(28300, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(28318, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(28336, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(28356, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(28377, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(28400, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(28418, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(28439, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(28456, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(28466, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(28477, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(28488, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(28492, 15), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(28507, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(28524, 17), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(28541, 11), IR(0, 0), IR(213, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(28552, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(28561, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(28566, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(28571, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(28578, 9), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(28587, 9), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(28596, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(28607, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(28618, 11), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(28629, 13), IR(0, 0), IR(215, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(28642, 11), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(28653, 13), IR(0, 0), IR(69, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(28666, 17), IR(0, 0), IR(114, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(28683, 19), IR(0, 0), IR(114, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(28739, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(28754, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(28768, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(28779, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(28791, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(28806, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(28828, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(28854, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(28871, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(28888, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(28899, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(28910, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(28921, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(28932, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(28943, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(28954, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(28964, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(28973, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(28983, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(29017, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(29062, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(29077, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(29083, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(29090, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(29153, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(29160, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(29165, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(29172, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(29184, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(29196, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(29210, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(29223, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(29238, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(29254, 9), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(29263, 9), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(29272, 15), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(29320, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(29330, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(29340, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(29350, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(29360, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(29370, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(29380, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(29390, 10), IR(0, 0), IR(216, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(29400, 9), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(29409, 10), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(29419, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(29436, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(29454, 23), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(29477, 24), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(29501, 15), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(29516, 16), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(29532, 17), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(29549, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(29567, 18), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(29585, 28), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(29613, 29), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(29642, 20), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(29662, 21), IR(0, 0), IR(75, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(29683, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(29697, 14), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(29711, 30), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(29741, 32), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(29773, 23), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(29796, 40), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(29836, 12), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(29848, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(29861, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(29877, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(29890, 19), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(29909, 26), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(29935, 16), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(29951, 13), IR(0, 0), IR(74, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(29964, 29), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(29993, 29), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(30022, 33), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(30055, 33), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(30088, 33), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(30121, 33), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(30154, 37), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(30191, 37), IR(0, 0), IR(217, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(30228, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(30245, 18), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(30263, 22), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(30285, 26), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(30311, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(30318, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(30339, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(30355, 16), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(30371, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(30378, 16), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(30394, 20), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(30414, 26), IR(0, 0), IR(218, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(30440, 35), IR(0, 0), IR(153, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(30475, 25), IR(0, 0), IR(153, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(30500, 17), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(30517, 23), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(30540, 19), IR(0, 0), IR(219, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(30559, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(30575, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(30591, 11), IR(0, 0), IR(1, 0), IR(157, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(30602, 21), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(30623, 19), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(30642, 19), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(30661, 24), IR(0, 0), IR(220, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(30685, 25), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(30710, 30), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(5650, 22), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(30740, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(30769, 32), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(30801, 30), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(30831, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(30860, 29), IR(0, 0), IR(221, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(8740, 23), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(30889, 26), IR(0, 0), IR(222, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(30915, 25), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(30940, 27), IR(0, 0), IR(223, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(30967, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(30987, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(31007, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(31027, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(31047, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(31067, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(31087, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(31107, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(31127, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(31147, 20), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(31167, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(31193, 25), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(31218, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(31244, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(31270, 25), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(31295, 26), IR(0, 0), IR(224, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(31321, 29), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(31350, 24), IR(0, 0), IR(227, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(31374, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(31386, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(31395, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(31407, 8), IR(0, 0), IR(228, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(31415, 23), IR(0, 0), IR(145, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(31438, 23), IR(0, 0), IR(154, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(31461, 25), IR(0, 0), IR(155, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(31486, 14), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(31542, 14), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(31556, 15), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(31571, 19), IR(0, 0), IR(200, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(31590, 17), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(31607, 19), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(9031, 9), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(31626, 14), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(31640, 18), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(31658, 12), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(31670, 13), IR(0, 0), IR(231, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(31683, 20), IR(0, 0), IR(3, 1), IR(167, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(31703, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(31725, 19), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(31744, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(31766, 30), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(5411, 18), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(31796, 27), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(31823, 25), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(31848, 33), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(31881, 22), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(31903, 19), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(31922, 15), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(31937, 15), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(31952, 20), IR(0, 0), IR(234, 1), IR(41, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(13250, 25), IR(0, 0), IR(235, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(31972, 26), IR(0, 0), IR(233, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(31998, 26), IR(0, 0), IR(1, 0), IR(168, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(32024, 25), IR(0, 0), IR(232, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(32049, 12), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(32061, 19), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(32080, 35), IR(0, 0), IR(236, 2), IR(169, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(32115, 22), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(32137, 16), IR(0, 0), IR(60, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(32200, 5), IR(93, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(32213, 5), IR(94, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(32226, 6), IR(95, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(32241, 11), IR(96, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(32266, 11), IR(97, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(32291, 12), IR(98, 1), IR(238, 1), IR(122, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(32318, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(32343, 25), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(32368, 26), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(32448, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(32475, 27), IR(0, 0), IR(86, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32502, 30), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(32532, 34), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(32566, 31), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(32597, 16), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(32613, 22), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(32635, 21), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(32656, 32), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(32688, 31), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(32719, 19), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(32738, 31), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(32769, 24), IR(0, 0), IR(240, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(32793, 19), IR(0, 0), IR(241, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(32812, 23), IR(0, 0), IR(242, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(32835, 23), IR(0, 0), IR(242, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(32858, 17), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(32875, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(32904, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(32933, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(32962, 29), IR(0, 0), IR(244, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(32991, 30), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(33021, 28), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(33049, 20), IR(0, 0), IR(243, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(33069, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(33092, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(33115, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(33138, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(33161, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(33184, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(33207, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(33230, 23), IR(0, 0), IR(216, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(33253, 21), IR(0, 0), IR(245, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(33274, 17), IR(0, 0), IR(245, 1), IR(58, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(33291, 13), IR(0, 0), IR(246, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(33304, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(33329, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(33353, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(33378, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(33407, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(33434, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(33457, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(33476, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(33499, 26), IR(0, 0), IR(162, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(33525, 26), IR(0, 0), IR(162, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(479, 14), IR(33551, 27), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(493, 13), IR(33578, 36), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(506, 7), IR(33614, 28), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(33642, 28), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(33670, 28), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(33698, 33), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(33731, 30), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(493, 13), IR(33761, 26), IR(0, 0), IR(247, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(33787, 39), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(33826, 44), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(33870, 23), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(513, 12), IR(33893, 20), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(493, 13), IR(33913, 21), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(513, 12), IR(33934, 30), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(525, 6), IR(33964, 22), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(33986, 25), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(34011, 26), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(34037, 25), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(34062, 22), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(34084, 29), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(34113, 28), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(34141, 25), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(34166, 34), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(34200, 32), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(34232, 29), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(34261, 22), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(34283, 22), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(34305, 19), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(34324, 17), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(34341, 18), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(531, 3), IR(34359, 29), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(34388, 24), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(34412, 16), IR(0, 0), IR(124, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(534, 7), IR(34428, 23), IR(0, 0), IR(249, 1), IR(72, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeCooperativeVectorNV, true, false, IR(89, 3), IR(34451, 24), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(541, 13), IR(34475, 29), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(554, 7), IR(34504, 42), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(34546, 39), IR(0, 0), IR(251, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(561, 16), IR(34585, 32), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertNV, true, true, IR(160, 3), IR(34617, 27), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(577, 4), IR(34644, 17), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(34661, 18), IR(0, 0), IR(53, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionNV, true, true, IR(160, 3), IR(34679, 27), IR(0, 0), IR(181, 1), IR(75, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(34706, 33), IR(0, 0), IR(50, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(34739, 35), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(34774, 38), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(581, 5), IR(34812, 24), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(586, 4), IR(34836, 25), IR(0, 0), IR(250, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(34861, 22), IR(99, 1), IR(54, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(34904, 21), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(34925, 15), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(34940, 8), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(513, 12), IR(34948, 14), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(513, 12), IR(34962, 17), IR(0, 0), IR(57, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(34979, 50), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(35029, 29), IR(100, 1), IR(255, 4), IR(171, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(35086, 18), IR(0, 0), IR(56, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetClusterIdNV, true, true, IR(172, 4), IR(35104, 23), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(35127, 24), IR(0, 0), IR(64, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(590, 5), IR(35151, 24), IR(0, 0), IR(259, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(595, 6), IR(35175, 24), IR(0, 0), IR(259, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(601, 5), IR(35199, 25), IR(0, 0), IR(259, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(35224, 26), IR(0, 0), IR(259, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(35250, 26), IR(0, 0), IR(259, 1), IR(79, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(35276, 28), IR(0, 0), IR(260, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(35304, 26), IR(0, 0), IR(260, 3), IR(80, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceNV, true, true, IR(606, 5), IR(35373, 26), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(611, 7), IR(35474, 30), IR(0, 0), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(618, 5), IR(35504, 31), IR(0, 0), IR(198, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpNV, true, true, IR(151, 5), IR(35535, 32), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(35567, 19), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(623, 4), IR(35586, 17), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(35603, 21), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(35624, 27), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(35651, 24), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(35675, 20), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(35695, 28), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(35723, 19), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(35742, 25), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(35767, 22), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(11562, 25), IR(45, 1), IR(266, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(35789, 22), IR(0, 0), IR(266, 1), IR(81, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(35811, 20), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(35831, 27), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(35858, 29), IR(0, 0), IR(252, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(35887, 18), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(35905, 20), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(35925, 18), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(35943, 20), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(35963, 25), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(35988, 25), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(36013, 29), IR(0, 0), IR(125, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(627, 7), IR(36094, 17), IR(0, 0), IR(187, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(36111, 40), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(36151, 38), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(36189, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(36227, 34), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(36261, 37), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(36298, 29), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(36327, 27), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(36354, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(36381, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(36404, 23), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(36427, 20), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(36447, 22), IR(0, 0), IR(61, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(36469, 19), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(12010, 21), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(36488, 25), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(36513, 23), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(36536, 24), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(36560, 23), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(36583, 24), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(36607, 28), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(36635, 29), IR(0, 0), IR(269, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(36664, 33), IR(0, 0), IR(270, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(634, 5), IR(36697, 34), IR(0, 0), IR(270, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(36731, 24), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(36755, 25), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(36780, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(36793, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(36806, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(36819, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(36832, 14), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(36846, 14), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(36860, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(36881, 21), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(36902, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(36915, 13), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(36928, 15), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(36943, 15), IR(0, 0), IR(271, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(36958, 29), IR(0, 0), IR(196, 1), IR(95, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(36987, 25), IR(0, 0), IR(196, 1), IR(95, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(37012, 15), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(639, 6), IR(12228, 9), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(37027, 13), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(37040, 14), IR(0, 0), IR(272, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(37054, 14), IR(0, 0), IR(272, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(37068, 14), IR(0, 0), IR(276, 1), IR(99, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(37082, 10), IR(0, 0), IR(276, 1), IR(99, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(37092, 15), IR(101, 1), IR(1, 0), IR(175, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(37128, 21), IR(102, 1), IR(1, 0), IR(175, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(37176, 14), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(37190, 18), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(37208, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(37231, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(37254, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(37277, 23), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(37300, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(37322, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37344, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(37390, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(37434, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(37473, 37), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(37510, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(37532, 22), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37554, 60), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(37614, 53), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37667, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(37714, 40), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37754, 51), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(37805, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(37849, 51), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(37900, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(37956, 50), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(38006, 52), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(38058, 49), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(38107, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(38154, 50), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(38204, 51), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(38255, 56), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(38311, 33), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38344, 52), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(38396, 61), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(38457, 61), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(38518, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(38557, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(38595, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(38634, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(38672, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(38711, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(38749, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(38785, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(38824, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(38867, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(38905, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(38943, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(38981, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(39026, 40), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(39066, 62), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(39128, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(39158, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(39196, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(39232, 33), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(39265, 35), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(39300, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(39339, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(39382, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(39429, 54), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(39483, 34), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(39517, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(39564, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(39609, 55), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(39664, 53), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(39717, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(39773, 54), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(39827, 58), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(39885, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(39941, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(39979, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(40025, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(40069, 49), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(40118, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(40165, 70), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(40235, 68), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(40303, 69), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(40372, 68), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(40440, 66), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(40506, 67), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(40573, 36), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(40609, 48), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(40657, 59), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(40716, 58), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(40774, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(645, 9), IR(40830, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(654, 10), IR(40860, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(40890, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(40929, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(40975, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41018, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41065, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(41110, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(41156, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(41212, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(41250, 30), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(41280, 32), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(654, 10), IR(41312, 36), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(664, 13), IR(41348, 42), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(41390, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(41429, 39), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(41468, 44), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(41512, 48), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(41560, 50), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(41610, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(41653, 48), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(41701, 43), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(41744, 31), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(41775, 47), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(41822, 45), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(41867, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(41913, 56), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(41969, 38), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(42007, 35), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(42042, 44), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(42086, 46), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(42132, 41), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(42173, 36), IR(0, 0), IR(280, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(42209, 50), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(42259, 48), IR(0, 0), IR(278, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(42307, 35), IR(0, 0), IR(277, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(12466, 25), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(42342, 16), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(42358, 19), IR(0, 0), IR(282, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosPiINTEL, true, true, IR(677, 8), IR(42377, 28), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastINTEL, true, true, IR(677, 8), IR(42405, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntINTEL, true, true, IR(677, 8), IR(42429, 31), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntINTEL, true, true, IR(677, 8), IR(42460, 29), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddINTEL, true, true, IR(685, 10), IR(42489, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubINTEL, true, true, IR(685, 10), IR(42512, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulINTEL, true, true, IR(685, 10), IR(42535, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivINTEL, true, true, IR(685, 10), IR(42558, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTINTEL, true, true, IR(695, 6), IR(42581, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEINTEL, true, true, IR(695, 6), IR(42603, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTINTEL, true, true, IR(695, 6), IR(42625, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEINTEL, true, true, IR(695, 6), IR(42647, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQINTEL, true, true, IR(695, 6), IR(42669, 22), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipINTEL, true, true, IR(677, 8), IR(42691, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtINTEL, true, true, IR(677, 8), IR(42716, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtINTEL, true, true, IR(677, 8), IR(42741, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotINTEL, true, true, IR(685, 10), IR(42765, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtINTEL, true, true, IR(677, 8), IR(42790, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(677, 8), IR(42814, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(677, 8), IR(42837, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(677, 8), IR(42861, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(677, 8), IR(42886, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(677, 8), IR(42911, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(677, 8), IR(42934, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(677, 8), IR(42958, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(677, 8), IR(42983, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(677, 8), IR(43008, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(677, 8), IR(43031, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(677, 8), IR(43054, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(677, 8), IR(43080, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(677, 8), IR(43105, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(677, 8), IR(43130, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(677, 8), IR(43154, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(677, 8), IR(43180, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(677, 8), IR(43204, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(677, 8), IR(43230, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(677, 8), IR(43254, 26), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(685, 10), IR(43280, 25), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(685, 10), IR(43305, 23), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(685, 10), IR(43328, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(685, 10), IR(43352, 24), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(43376, 17), IR(0, 0), IR(284, 1), IR(106, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(701, 2), IR(43393, 21), IR(0, 0), IR(134, 1), IR(113, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(703, 3), IR(43414, 20), IR(0, 0), IR(134, 1), IR(113, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(43434, 24), IR(0, 0), IR(134, 1), IR(113, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtINTEL, true, true, IR(677, 8), IR(43458, 15), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipINTEL, true, true, IR(677, 8), IR(43473, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtINTEL, true, true, IR(677, 8), IR(43489, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinINTEL, true, true, IR(677, 8), IR(43505, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosINTEL, true, true, IR(677, 8), IR(43519, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosINTEL, true, true, IR(677, 8), IR(43533, 17), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiINTEL, true, true, IR(677, 8), IR(43550, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiINTEL, true, true, IR(677, 8), IR(43566, 16), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiINTEL, true, true, IR(677, 8), IR(43582, 19), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogINTEL, true, true, IR(677, 8), IR(43601, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpINTEL, true, true, IR(677, 8), IR(43615, 14), IR(0, 0), IR(186, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupINTEL, true, true, IR(160, 3), IR(43629, 29), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrINTEL, true, true, IR(160, 3), IR(43658, 29), IR(0, 0), IR(197, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingINTEL, true, true, IR(172, 4), IR(43687, 22), IR(0, 0), IR(285, 1), IR(120, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipeBlockingINTEL, true, true, IR(172, 4), IR(43709, 23), IR(0, 0), IR(285, 1), IR(120, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpFPGARegINTEL, true, true, IR(160, 3), IR(13025, 13), IR(0, 0), IR(286, 1), IR(121, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(43732, 22), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(43754, 23), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(43777, 28), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(43805, 46), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(43851, 37), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(43888, 65), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(43953, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(43993, 41), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(44034, 39), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(44073, 36), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(44109, 46), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(44155, 45), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(44200, 42), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(44242, 32), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(44274, 29), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(44303, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(44343, 40), IR(0, 0), IR(190, 1), IR(48, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(44383, 14), IR(0, 0), IR(287, 4), IR(128, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(44397, 23), IR(0, 0), IR(126, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(706, 1), IR(44420, 25), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(44445, 32), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(706, 1), IR(44477, 36), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, true, true, IR(107, 3), IR(44513, 33), IR(0, 0), IR(291, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(44546, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(44566, 20), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveINTEL, false, false, IR(221, 3), IR(44586, 26), IR(0, 0), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitINTEL, false, false, IR(221, 3), IR(44612, 24), IR(0, 0), IR(293, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(13457, 19), IR(0, 0), IR(294, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateINTEL, true, true, IR(707, 7), IR(44636, 24), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceAsyncINTEL, false, false, IR(168, 2), IR(44660, 23), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceGetINTEL, true, true, IR(160, 3), IR(44683, 21), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTaskSequenceReleaseINTEL, false, false, IR(10, 1), IR(44704, 25), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeTaskSequenceINTEL, true, false, IR(68, 1), IR(44729, 22), IR(0, 0), IR(295, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(44751, 27), IR(0, 0), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(714, 10), IR(44778, 25), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(714, 10), IR(44803, 34), IR(0, 0), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(714, 10), IR(44837, 34), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(724, 9), IR(44871, 29), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(714, 10), IR(44900, 26), IR(0, 0), IR(108, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(733, 7), IR(13795, 38), IR(0, 0), IR(299, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(44988, 21), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(45009, 26), IR(0, 0), IR(109, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(740, 5), IR(45035, 27), IR(0, 0), IR(109, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(745, 2), IR(45062, 27), IR(0, 0), IR(109, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(747, 4), IR(45089, 24), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(751, 6), IR(45113, 30), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(757, 3), IR(45180, 30), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(45210, 27), IR(0, 0), IR(109, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(45237, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(45250, 13), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(45263, 19), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(45282, 18), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(45300, 19), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(45319, 19), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(45338, 18), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(45356, 19), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(45375, 18), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(760, 6), IR(45393, 18), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(766, 4), IR(45411, 19), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(45430, 26), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(45456, 28), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(45484, 33), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 831> kInstructionNames{{
  {IR(36780, 13), 553}, // 0 AbsISubINTEL
  {IR(36793, 13), 554}, // 1 AbsUSubINTEL
  {IR(26635, 12), 58}, // 2 AccessChain
  {IR(43393, 21), 739}, // 3 AliasDomainDeclINTEL
  {IR(43414, 20), 740}, // 4 AliasScopeDeclINTEL
  {IR(43434, 24), 741}, // 5 AliasScopeListDeclINTEL
  {IR(27932, 4), 143}, // 6 All
  {IR(33304, 25), 425}, // 7 AllocateNodePayloadsAMDX
  {IR(27928, 4), 142}, // 8 Any
  {IR(43180, 24), 730}, // 9 ArbitraryFloatACosINTEL
  {IR(43204, 26), 731}, // 10 ArbitraryFloatACosPiINTEL
  {IR(43130, 24), 728}, // 11 ArbitraryFloatASinINTEL
  {IR(43154, 26), 729}, // 12 ArbitraryFloatASinPiINTEL
  {IR(43280, 25), 734}, // 13 ArbitraryFloatATan2INTEL
  {IR(43230, 24), 732}, // 14 ArbitraryFloatATanINTEL
  {IR(43254, 26), 733}, // 15 ArbitraryFloatATanPiINTEL
  {IR(42489, 23), 701}, // 16 ArbitraryFloatAddINTEL
  {IR(42429, 31), 699}, // 17 ArbitraryFloatCastFromIntINTEL
  {IR(42405, 24), 698}, // 18 ArbitraryFloatCastINTEL
  {IR(42460, 29), 700}, // 19 ArbitraryFloatCastToIntINTEL
  {IR(42741, 24), 712}, // 20 ArbitraryFloatCbrtINTEL
  {IR(43031, 23), 724}, // 21 ArbitraryFloatCosINTEL
  {IR(43105, 25), 727}, // 22 ArbitraryFloatCosPiINTEL
  {IR(42558, 23), 704}, // 23 ArbitraryFloatDivINTEL
  {IR(42669, 22), 709}, // 24 ArbitraryFloatEQINTEL
  {IR(42958, 25), 721}, // 25 ArbitraryFloatExp10INTEL
  {IR(42934, 24), 720}, // 26 ArbitraryFloatExp2INTEL
  {IR(42911, 23), 719}, // 27 ArbitraryFloatExpINTEL
  {IR(42983, 25), 722}, // 28 ArbitraryFloatExpm1INTEL
  {IR(42603, 22), 706}, // 29 ArbitraryFloatGEINTEL
  {IR(42581, 22), 705}, // 30 ArbitraryFloatGTINTEL
  {IR(42765, 25), 713}, // 31 ArbitraryFloatHypotINTEL
  {IR(42647, 22), 708}, // 32 ArbitraryFloatLEINTEL
  {IR(42625, 22), 707}, // 33 ArbitraryFloatLTINTEL
  {IR(42861, 25), 717}, // 34 ArbitraryFloatLog10INTEL
  {IR(42886, 25), 718}, // 35 ArbitraryFloatLog1pINTEL
  {IR(42837, 24), 716}, // 36 ArbitraryFloatLog2INTEL
  {IR(42814, 23), 715}, // 37 ArbitraryFloatLogINTEL
  {IR(42535, 23), 703}, // 38 ArbitraryFloatMulINTEL
  {IR(43305, 23), 735}, // 39 ArbitraryFloatPowINTEL
  {IR(43352, 24), 737}, // 40 ArbitraryFloatPowNINTEL
  {IR(43328, 24), 736}, // 41 ArbitraryFloatPowRINTEL
  {IR(42716, 25), 711}, // 42 ArbitraryFloatRSqrtINTEL
  {IR(42691, 25), 710}, // 43 ArbitraryFloatRecipINTEL
  {IR(43054, 26), 725}, // 44 ArbitraryFloatSinCosINTEL
  {IR(42377, 28), 697}, // 45 ArbitraryFloatSinCosPiINTEL
  {IR(43008, 23), 723}, // 46 ArbitraryFloatSinINTEL
  {IR(43080, 25), 726}, // 47 ArbitraryFloatSinPiINTEL
  {IR(42790, 24), 714}, // 48 ArbitraryFloatSqrtINTEL
  {IR(42512, 23), 702}, // 49 ArbitraryFloatSubINTEL
  {IR(13457, 19), 785}, // 50 ArithmeticFenceEXT
  {IR(26682, 12), 61}, // 51 ArrayLength
  {IR(37027, 13), 569}, // 52 AsmCallINTEL
  {IR(12228, 9), 568}, // 53 AsmINTEL
  {IR(37012, 15), 567}, // 54 AsmTargetINTEL
  {IR(37068, 14), 572}, // 55 AssumeTrueKHR
  {IR(28954, 10), 220}, // 56 AtomicAnd
  {IR(28806, 22), 210}, // 57 AtomicCompareExchange
  {IR(28828, 26), 211}, // 58 AtomicCompareExchangeWeak
  {IR(28791, 15), 209}, // 59 AtomicExchange
  {IR(44383, 14), 775}, // 60 AtomicFAddEXT
  {IR(37054, 14), 571}, // 61 AtomicFMaxEXT
  {IR(37040, 14), 570}, // 62 AtomicFMinEXT
  {IR(30339, 16), 292}, // 63 AtomicFlagClear
  {IR(30318, 21), 291}, // 64 AtomicFlagTestAndSet
  {IR(28888, 11), 214}, // 65 AtomicIAdd
  {IR(28871, 17), 213}, // 66 AtomicIDecrement
  {IR(28854, 17), 212}, // 67 AtomicIIncrement
  {IR(28899, 11), 215}, // 68 AtomicISub
  {IR(28768, 11), 207}, // 69 AtomicLoad
  {IR(28964, 9), 221}, // 70 AtomicOr
  {IR(28932, 11), 218}, // 71 AtomicSMax
  {IR(28910, 11), 216}, // 72 AtomicSMin
  {IR(28779, 12), 208}, // 73 AtomicStore
  {IR(28943, 11), 219}, // 74 AtomicUMax
  {IR(28921, 11), 217}, // 75 AtomicUMin
  {IR(28973, 10), 222}, // 76 AtomicXor
  {IR(35276, 28), 499}, // 77 BeginInvocationInterlockEXT
  {IR(32858, 17), 406}, // 78 BitCastArrayQCOM
  {IR(28552, 9), 191}, // 79 BitCount
  {IR(28492, 15), 187}, // 80 BitFieldInsert
  {IR(28507, 17), 188}, // 81 BitFieldSExtract
  {IR(28524, 17), 189}, // 82 BitFieldUExtract
  {IR(28541, 11), 190}, // 83 BitReverse
  {IR(27680, 8), 114}, // 84 Bitcast
  {IR(28477, 11), 185}, // 85 BitwiseAnd
  {IR(44988, 21), 798}, // 86 BitwiseFunctionINTEL
  {IR(28456, 10), 183}, // 87 BitwiseOr
  {IR(28466, 11), 184}, // 88 BitwiseXor
  {IR(29083, 7), 227}, // 89 Branch
  {IR(29090, 18), 228}, // 90 BranchConditional
  {IR(29951, 13), 277}, // 91 BuildNDRange
  {IR(25696, 11), 15}, // 92 Capability
  {IR(29909, 26), 275}, // 93 CaptureEventProfilingInfo
  {IR(31415, 23), 344}, // 94 ColorAttachmentReadEXT
  {IR(29501, 15), 255}, // 95 CommitReadPipe
  {IR(29516, 16), 256}, // 96 CommitWritePipe
  {IR(26942, 19), 72}, // 97 CompositeConstruct
  {IR(44513, 33), 780}, // 98 CompositeConstructContinuedINTEL
  {IR(32991, 30), 411}, // 99 CompositeConstructCoopMatQCOM
  {IR(32566, 31), 394}, // 100 CompositeConstructReplicateEXT
  {IR(26961, 17), 73}, // 101 CompositeExtract
  {IR(33021, 28), 412}, // 102 CompositeExtractCoopMatQCOM
  {IR(26978, 16), 74}, // 103 CompositeInsert
  {IR(45062, 27), 801}, // 104 ConditionalCapabilityINTEL
  {IR(45210, 27), 805}, // 105 ConditionalCopyObjectINTEL
  {IR(45035, 27), 800}, // 106 ConditionalEntryPointINTEL
  {IR(45009, 26), 799}, // 107 ConditionalExtensionINTEL
  {IR(15419, 9), 39}, // 108 Constant
  {IR(26202, 18), 40}, // 109 ConstantComposite
  {IR(44445, 32), 778}, // 110 ConstantCompositeContinuedINTEL
  {IR(32502, 30), 392}, // 111 ConstantCompositeReplicateEXT
  {IR(26150, 14), 38}, // 112 ConstantFalse
  {IR(36958, 29), 565}, // 113 ConstantFunctionPointerINTEL
  {IR(26314, 13), 42}, // 114 ConstantNull
  {IR(30394, 20), 296}, // 115 ConstantPipeStorage
  {IR(26298, 16), 41}, // 116 ConstantSampler
  {IR(33457, 19), 431}, // 117 ConstantStringAMDX
  {IR(26137, 13), 37}, // 118 ConstantTrue
  {IR(28739, 15), 205}, // 119 ControlBarrier
  {IR(44586, 26), 783}, // 120 ControlBarrierArriveINTEL
  {IR(44612, 24), 784}, // 121 ControlBarrierWaitINTEL
  {IR(44566, 20), 782}, // 122 ConvertBF16ToFINTEL
  {IR(44546, 20), 781}, // 123 ConvertFToBF16INTEL
  {IR(27486, 12), 100}, // 124 ConvertFToS
  {IR(27474, 12), 99}, // 125 ConvertFToU
  {IR(45430, 26), 817}, // 126 ConvertHandleToImageINTEL
  {IR(45484, 33), 819}, // 127 ConvertHandleToSampledImageINTEL
  {IR(45456, 28), 818}, // 128 ConvertHandleToSamplerINTEL
  {IR(35925, 18), 522}, // 129 ConvertImageToUNV
  {IR(27563, 14), 107}, // 130 ConvertPtrToU
  {IR(27498, 12), 101}, // 131 ConvertSToF
  {IR(35988, 25), 525}, // 132 ConvertSampledImageToUNV
  {IR(35943, 20), 523}, // 133 ConvertSamplerToUNV
  {IR(32080, 35), 378}, // 134 ConvertUToAccelerationStructureKHR
  {IR(27510, 12), 102}, // 135 ConvertUToF
  {IR(35887, 18), 520}, // 136 ConvertUToImageNV
  {IR(27607, 14), 110}, // 137 ConvertUToPtr
  {IR(35963, 25), 524}, // 138 ConvertUToSampledImageNV
  {IR(35905, 20), 521}, // 139 ConvertUToSamplerNV
  {IR(34617, 27), 474}, // 140 CooperativeMatrixConvertNV
  {IR(32475, 27), 391}, // 141 CooperativeMatrixLengthKHR
  {IR(35250, 26), 498}, // 142 CooperativeMatrixLengthNV
  {IR(32343, 25), 388}, // 143 CooperativeMatrixLoadKHR
  {IR(35175, 24), 495}, // 144 CooperativeMatrixLoadNV
  {IR(35474, 30), 502}, // 145 CooperativeMatrixLoadTensorNV
  {IR(32448, 27), 390}, // 146 CooperativeMatrixMulAddKHR
  {IR(35224, 26), 497}, // 147 CooperativeMatrixMulAddNV
  {IR(35535, 32), 504}, // 148 CooperativeMatrixPerElementOpNV
  {IR(35373, 26), 501}, // 149 CooperativeMatrixReduceNV
  {IR(32368, 26), 389}, // 150 CooperativeMatrixStoreKHR
  {IR(35199, 25), 496}, // 151 CooperativeMatrixStoreNV
  {IR(35504, 31), 503}, // 152 CooperativeMatrixStoreTensorNV
  {IR(35858, 29), 519}, // 153 CooperativeMatrixTransposeNV
  {IR(34812, 24), 481}, // 154 CooperativeVectorLoadNV
  {IR(34585, 32), 473}, // 155 CooperativeVectorMatrixMulAddNV
  {IR(34475, 29), 470}, // 156 CooperativeVectorMatrixMulNV
  {IR(34504, 42), 471}, // 157 CooperativeVectorOuterProductAccumulateNV
  {IR(34546, 39), 472}, // 158 CooperativeVectorReduceSumAccumulateNV
  {IR(34836, 25), 482}, // 159 CooperativeVectorStoreNV
  {IR(31374, 12), 340}, // 160 CopyLogical
  {IR(26608, 11), 56}, // 161 CopyMemory
  {IR(26619, 16), 57}, // 162 CopyMemorySized
  {IR(26994, 11), 75}, // 163 CopyObject
  {IR(30414, 26), 297}, // 164 CreatePipeFromPipeStorage
  {IR(35603, 21), 507}, // 165 CreateTensorLayoutNV
  {IR(35723, 19), 512}, // 166 CreateTensorViewNV
  {IR(29861, 16), 272}, // 167 CreateUserEvent
  {IR(43658, 29), 754}, // 168 CrossWorkgroupCastToPtrINTEL
  {IR(28561, 5), 192}, // 169 DPdx
  {IR(28607, 11), 198}, // 170 DPdxCoarse
  {IR(28578, 9), 195}, // 171 DPdxFine
  {IR(28566, 5), 193}, // 172 DPdy
  {IR(28618, 11), 199}, // 173 DPdyCoarse
  {IR(28587, 9), 196}, // 174 DPdyFine
  {IR(26768, 9), 64}, // 175 Decorate
  {IR(30591, 11), 305}, // 176 DecorateId
  {IR(37092, 15), 574}, // 177 DecorateString
  {IR(37107, 21), 574}, // 178 DecorateStringGOOGLE
  {IR(26792, 16), 66}, // 179 DecorationGroup
  {IR(11562, 25), 515}, // 180 DemoteToHelperInvocation
  {IR(11587, 28), 515}, // 181 DemoteToHelperInvocationEXT
  {IR(31438, 23), 345}, // 182 DepthAttachmentReadEXT
  {IR(27877, 4), 137}, // 183 Dot
  {IR(34644, 17), 475}, // 184 EmitMeshTasksEXT
  {IR(28666, 17), 203}, // 185 EmitStreamVertex
  {IR(28642, 11), 201}, // 186 EmitVertex
  {IR(35304, 26), 500}, // 187 EndInvocationInterlockEXT
  {IR(28653, 13), 202}, // 188 EndPrimitive
  {IR(28683, 19), 204}, // 189 EndStreamPrimitive
  {IR(29697, 14), 265}, // 190 EnqueueKernel
  {IR(29683, 14), 264}, // 191 EnqueueMarker
  {IR(33329, 24), 426}, // 192 EnqueueNodePayloadsAMDX
  {IR(25611, 11), 13}, // 193 EntryPoint
  {IR(32061, 19), 377}, // 194 ExecuteCallableKHR
  {IR(35086, 18), 491}, // 195 ExecuteCallableNV
  {IR(25654, 14), 14}, // 196 ExecutionMode
  {IR(30575, 16), 304}, // 197 ExecutionModeId
  {IR(37082, 10), 573}, // 198 ExpectKHR
  {IR(25465, 8), 11}, // 199 ExtInst
  {IR(25405, 14), 10}, // 200 ExtInstImport
  {IR(31998, 26), 374}, // 201 ExtInstWithForwardRefsKHR
  {IR(25395, 10), 9}, // 202 Extension
  {IR(33049, 20), 413}, // 203 ExtractSubArrayQCOM
  {IR(27709, 5), 118}, // 204 FAdd
  {IR(27540, 9), 105}, // 205 FConvert
  {IR(27744, 5), 125}, // 206 FDiv
  {IR(27769, 5), 130}, // 207 FMod
  {IR(27729, 5), 122}, // 208 FMul
  {IR(27696, 8), 116}, // 209 FNegate
  {IR(28206, 10), 168}, // 210 FOrdEqual
  {IR(28284, 16), 174}, // 211 FOrdGreaterThan
  {IR(28356, 21), 178}, // 212 FOrdGreaterThanEqual
  {IR(28256, 13), 172}, // 213 FOrdLessThan
  {IR(28318, 18), 176}, // 214 FOrdLessThanEqual
  {IR(28228, 13), 170}, // 215 FOrdNotEqual
  {IR(13025, 13), 757}, // 216 FPGARegINTEL
  {IR(27764, 5), 129}, // 217 FRem
  {IR(27719, 5), 120}, // 218 FSub
  {IR(28216, 12), 169}, // 219 FUnordEqual
  {IR(28300, 18), 175}, // 220 FUnordGreaterThan
  {IR(28377, 23), 179}, // 221 FUnordGreaterThanEqual
  {IR(28269, 15), 173}, // 222 FUnordLessThan
  {IR(28336, 20), 177}, // 223 FUnordLessThanEqual
  {IR(28241, 15), 171}, // 224 FUnordNotEqual
  {IR(34774, 38), 480}, // 225 FetchMicroTriangleVertexBarycentricNV
  {IR(34739, 35), 479}, // 226 FetchMicroTriangleVertexPositionNV
  {IR(33378, 29), 428}, // 227 FinishWritingNodePayloadAMDX
  {IR(43519, 14), 746}, // 228 FixedCosINTEL
  {IR(43566, 16), 749}, // 229 FixedCosPiINTEL
  {IR(43615, 14), 752}, // 230 FixedExpINTEL
  {IR(43601, 14), 751}, // 231 FixedLogINTEL
  {IR(43473, 16), 743}, // 232 FixedRecipINTEL
  {IR(43489, 16), 744}, // 233 FixedRsqrtINTEL
  {IR(43533, 17), 747}, // 234 FixedSinCosINTEL
  {IR(43582, 19), 750}, // 235 FixedSinCosPiINTEL
  {IR(43505, 14), 745}, // 236 FixedSinINTEL
  {IR(43550, 16), 748}, // 237 FixedSinPiINTEL
  {IR(43458, 15), 742}, // 238 FixedSqrtINTEL
  {IR(33274, 17), 423}, // 239 FragmentFetchAMD
  {IR(33253, 21), 422}, // 240 FragmentMaskFetchAMD
  {IR(24515, 9), 48}, // 241 Function
  {IR(26517, 13), 51}, // 242 FunctionCall
  {IR(26505, 12), 50}, // 243 FunctionEnd
  {IR(26487, 18), 49}, // 244 FunctionParameter
  {IR(36987, 25), 566}, // 245 FunctionPointerCallINTEL
  {IR(28571, 7), 194}, // 246 Fwidth
  {IR(28629, 13), 200}, // 247 FwidthCoarse
  {IR(28596, 11), 197}, // 248 FwidthFine
  {IR(27638, 17), 112}, // 249 GenericCastToPtr
  {IR(27655, 25), 113}, // 250 GenericCastToPtrExplicit
  {IR(26694, 23), 62}, // 251 GenericPtrMemSemantics
  {IR(29935, 16), 276}, // 252 GetDefaultQueue
  {IR(30440, 35), 298}, // 253 GetKernelLocalSizeForSubgroupCount
  {IR(30475, 25), 299}, // 254 GetKernelMaxNumSubgroups
  {IR(29741, 32), 267}, // 255 GetKernelNDrangeMaxSubGroupSize
  {IR(29711, 30), 266}, // 256 GetKernelNDrangeSubGroupCount
  {IR(29796, 40), 269}, // 257 GetKernelPreferredWorkGroupSizeMultiple
  {IR(29773, 23), 268}, // 258 GetKernelWorkGroupSize
  {IR(29567, 18), 259}, // 259 GetMaxPipePackets
  {IR(29549, 18), 258}, // 260 GetNumPipePackets
  {IR(9031, 9), 353}, // 261 GraphARM
  {IR(31590, 17), 351}, // 262 GraphConstantARM
  {IR(31658, 12), 356}, // 263 GraphEndARM
  {IR(31607, 19), 352}, // 264 GraphEntryPointARM
  {IR(31626, 14), 354}, // 265 GraphInputARM
  {IR(31640, 18), 355}, // 266 GraphSetOutputARM
  {IR(29254, 9), 238}, // 267 GroupAll
  {IR(29263, 9), 239}, // 268 GroupAny
  {IR(29223, 15), 236}, // 269 GroupAsyncCopy
  {IR(45263, 19), 808}, // 270 GroupBitwiseAndKHR
  {IR(45282, 18), 809}, // 271 GroupBitwiseOrKHR
  {IR(45300, 19), 810}, // 272 GroupBitwiseXorKHR
  {IR(29272, 15), 240}, // 273 GroupBroadcast
  {IR(29642, 20), 262}, // 274 GroupCommitReadPipe
  {IR(29662, 21), 263}, // 275 GroupCommitWritePipe
  {IR(26808, 14), 67}, // 276 GroupDecorate
  {IR(29330, 10), 242}, // 277 GroupFAdd
  {IR(33092, 23), 415}, // 278 GroupFAddNonUniformAMD
  {IR(29370, 10), 246}, // 279 GroupFMax
  {IR(33184, 23), 419}, // 280 GroupFMaxNonUniformAMD
  {IR(29340, 10), 243}, // 281 GroupFMin
  {IR(33115, 23), 416}, // 282 GroupFMinNonUniformAMD
  {IR(45250, 13), 807}, // 283 GroupFMulKHR
  {IR(29320, 10), 241}, // 284 GroupIAdd
  {IR(33069, 23), 414}, // 285 GroupIAddNonUniformAMD
  {IR(45237, 13), 806}, // 286 GroupIMulKHR
  {IR(45319, 19), 811}, // 287 GroupLogicalAndKHR
  {IR(45338, 18), 812}, // 288 GroupLogicalOrKHR
  {IR(45356, 19), 813}, // 289 GroupLogicalXorKHR
  {IR(26867, 20), 68}, // 290 GroupMemberDecorate
  {IR(30623, 19), 307}, // 291 GroupNonUniformAll
  {IR(30661, 24), 309}, // 292 GroupNonUniformAllEqual
  {IR(30642, 19), 308}, // 293 GroupNonUniformAny
  {IR(5650, 22), 312}, // 294 GroupNonUniformBallot
  {IR(30801, 30), 315}, // 295 GroupNonUniformBallotBitCount
  {IR(30769, 32), 314}, // 296 GroupNonUniformBallotBitExtract
  {IR(30831, 29), 316}, // 297 GroupNonUniformBallotFindLSB
  {IR(30860, 29), 317}, // 298 GroupNonUniformBallotFindMSB
  {IR(31167, 26), 332}, // 299 GroupNonUniformBitwiseAnd
  {IR(31193, 25), 333}, // 300 GroupNonUniformBitwiseOr
  {IR(31218, 26), 334}, // 301 GroupNonUniformBitwiseXor
  {IR(30685, 25), 310}, // 302 GroupNonUniformBroadcast
  {IR(30710, 30), 311}, // 303 GroupNonUniformBroadcastFirst
  {IR(30602, 21), 306}, // 304 GroupNonUniformElect
  {IR(30987, 20), 323}, // 305 GroupNonUniformFAdd
  {IR(31147, 20), 331}, // 306 GroupNonUniformFMax
  {IR(31087, 20), 328}, // 307 GroupNonUniformFMin
  {IR(31027, 20), 325}, // 308 GroupNonUniformFMul
  {IR(30967, 20), 322}, // 309 GroupNonUniformIAdd
  {IR(31007, 20), 324}, // 310 GroupNonUniformIMul
  {IR(30740, 29), 313}, // 311 GroupNonUniformInverseBallot
  {IR(31244, 26), 335}, // 312 GroupNonUniformLogicalAnd
  {IR(31270, 25), 336}, // 313 GroupNonUniformLogicalOr
  {IR(31295, 26), 337}, // 314 GroupNonUniformLogicalXor
  {IR(34679, 27), 477}, // 315 GroupNonUniformPartitionNV
  {IR(33499, 26), 433}, // 316 GroupNonUniformQuadAllKHR
  {IR(33525, 26), 434}, // 317 GroupNonUniformQuadAnyKHR
  {IR(31321, 29), 338}, // 318 GroupNonUniformQuadBroadcast
  {IR(31350, 24), 339}, // 319 GroupNonUniformQuadSwap
  {IR(13250, 25), 372}, // 320 GroupNonUniformRotateKHR
  {IR(31107, 20), 329}, // 321 GroupNonUniformSMax
  {IR(31047, 20), 326}, // 322 GroupNonUniformSMin
  {IR(8740, 23), 318}, // 323 GroupNonUniformShuffle
  {IR(30940, 27), 321}, // 324 GroupNonUniformShuffleDown
  {IR(30915, 25), 320}, // 325 GroupNonUniformShuffleUp
  {IR(30889, 26), 319}, // 326 GroupNonUniformShuffleXor
  {IR(31127, 20), 330}, // 327 GroupNonUniformUMax
  {IR(31067, 20), 327}, // 328 GroupNonUniformUMin
  {IR(29585, 28), 260}, // 329 GroupReserveReadPipePackets
  {IR(29613, 29), 261}, // 330 GroupReserveWritePipePackets
  {IR(29390, 10), 248}, // 331 GroupSMax
  {IR(33230, 23), 421}, // 332 GroupSMaxNonUniformAMD
  {IR(29360, 10), 245}, // 333 GroupSMin
  {IR(33161, 23), 418}, // 334 GroupSMinNonUniformAMD
  {IR(29380, 10), 247}, // 335 GroupUMax
  {IR(33207, 23), 420}, // 336 GroupUMaxNonUniformAMD
  {IR(29350, 10), 244}, // 337 GroupUMin
  {IR(33138, 23), 417}, // 338 GroupUMinNonUniformAMD
  {IR(29238, 16), 237}, // 339 GroupWaitEvents
  {IR(33986, 25), 450}, // 340 HitObjectExecuteShaderNV
  {IR(34037, 25), 452}, // 341 HitObjectGetAttributesNV
  {IR(35127, 24), 493}, // 342 HitObjectGetClusterIdNV
  {IR(34011, 26), 451}, // 343 HitObjectGetCurrentTimeNV
  {IR(34113, 28), 455}, // 344 HitObjectGetGeometryIndexNV
  {IR(34062, 22), 453}, // 345 HitObjectGetHitKindNV
  {IR(34166, 34), 457}, // 346 HitObjectGetInstanceCustomIndexNV
  {IR(34141, 25), 456}, // 347 HitObjectGetInstanceIdNV
  {IR(36354, 27), 535}, // 348 HitObjectGetLSSPositionsNV
  {IR(36381, 23), 536}, // 349 HitObjectGetLSSRadiiNV
  {IR(33698, 33), 440}, // 350 HitObjectGetObjectRayDirectionNV
  {IR(33731, 30), 441}, // 351 HitObjectGetObjectRayOriginNV
  {IR(33670, 28), 439}, // 352 HitObjectGetObjectToWorldNV
  {IR(34084, 29), 454}, // 353 HitObjectGetPrimitiveIndexNV
  {IR(34261, 22), 460}, // 354 HitObjectGetRayTMaxNV
  {IR(34283, 22), 461}, // 355 HitObjectGetRayTMinNV
  {IR(33826, 44), 444}, // 356 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(33787, 39), 443}, // 357 HitObjectGetShaderRecordBufferHandleNV
  {IR(36298, 29), 533}, // 358 HitObjectGetSpherePositionNV
  {IR(36327, 27), 534}, // 359 HitObjectGetSphereRadiusNV
  {IR(34200, 32), 458}, // 360 HitObjectGetWorldRayDirectionNV
  {IR(34232, 29), 459}, // 361 HitObjectGetWorldRayOriginNV
  {IR(33642, 28), 438}, // 362 HitObjectGetWorldToObjectNV
  {IR(34305, 19), 462}, // 363 HitObjectIsEmptyNV
  {IR(34324, 17), 463}, // 364 HitObjectIsHitNV
  {IR(36427, 20), 538}, // 365 HitObjectIsLSSHitNV
  {IR(34341, 18), 464}, // 366 HitObjectIsMissNV
  {IR(36404, 23), 537}, // 367 HitObjectIsSphereHitNV
  {IR(33870, 23), 445}, // 368 HitObjectRecordEmptyNV
  {IR(33551, 27), 435}, // 369 HitObjectRecordHitMotionNV
  {IR(33913, 21), 447}, // 370 HitObjectRecordHitNV
  {IR(33578, 36), 436}, // 371 HitObjectRecordHitWithIndexMotionNV
  {IR(33934, 30), 448}, // 372 HitObjectRecordHitWithIndexNV
  {IR(33614, 28), 437}, // 373 HitObjectRecordMissMotionNV
  {IR(33964, 22), 449}, // 374 HitObjectRecordMissNV
  {IR(33761, 26), 442}, // 375 HitObjectTraceRayMotionNV
  {IR(33893, 20), 446}, // 376 HitObjectTraceRayNV
  {IR(27704, 5), 117}, // 377 IAdd
  {IR(27881, 10), 138}, // 378 IAddCarry
  {IR(36806, 13), 555}, // 379 IAddSatINTEL
  {IR(36832, 14), 557}, // 380 IAverageINTEL
  {IR(36860, 21), 559}, // 381 IAverageRoundedINTEL
  {IR(28077, 7), 158}, // 382 IEqual
  {IR(27724, 5), 121}, // 383 IMul
  {IR(36928, 15), 563}, // 384 IMul32x16INTEL
  {IR(28084, 10), 159}, // 385 INotEqual
  {IR(27714, 5), 119}, // 386 ISub
  {IR(27891, 11), 139}, // 387 ISubBorrow
  {IR(36902, 13), 561}, // 388 ISubSatINTEL
  {IR(32115, 22), 379}, // 389 IgnoreIntersectionKHR
  {IR(34904, 21), 484}, // 390 IgnoreIntersectionNV
  {IR(24559, 6), 91}, // 391 Image
  {IR(32962, 29), 410}, // 392 ImageBlockMatchGatherSADQCOM
  {IR(32933, 29), 409}, // 393 ImageBlockMatchGatherSSDQCOM
  {IR(32835, 23), 405}, // 394 ImageBlockMatchSADQCOM
  {IR(32812, 23), 404}, // 395 ImageBlockMatchSSDQCOM
  {IR(32904, 29), 408}, // 396 ImageBlockMatchWindowSADQCOM
  {IR(32875, 29), 407}, // 397 ImageBlockMatchWindowSSDQCOM
  {IR(32793, 19), 403}, // 398 ImageBoxFilterQCOM
  {IR(27322, 16), 88}, // 399 ImageDrefGather
  {IR(27299, 11), 86}, // 400 ImageFetch
  {IR(27310, 12), 87}, // 401 ImageGather
  {IR(27359, 17), 92}, // 402 ImageQueryFormat
  {IR(27439, 17), 97}, // 403 ImageQueryLevels
  {IR(27425, 14), 96}, // 404 ImageQueryLod
  {IR(27376, 16), 93}, // 405 ImageQueryOrder
  {IR(27456, 18), 98}, // 406 ImageQuerySamples
  {IR(27410, 15), 95}, // 407 ImageQuerySize
  {IR(27392, 18), 94}, // 408 ImageQuerySizeLod
  {IR(27338, 10), 89}, // 409 ImageRead
  {IR(27156, 27), 81}, // 410 ImageSampleDrefExplicitLod
  {IR(27129, 27), 80}, // 411 ImageSampleDrefImplicitLod
  {IR(27106, 23), 79}, // 412 ImageSampleExplicitLod
  {IR(34428, 23), 468}, // 413 ImageSampleFootprintNV
  {IR(27060, 23), 78}, // 414 ImageSampleImplicitLod
  {IR(27268, 31), 85}, // 415 ImageSampleProjDrefExplicitLod
  {IR(27237, 31), 84}, // 416 ImageSampleProjDrefImplicitLod
  {IR(27210, 27), 83}, // 417 ImageSampleProjExplicitLod
  {IR(27183, 27), 82}, // 418 ImageSampleProjImplicitLod
  {IR(32769, 24), 402}, // 419 ImageSampleWeightedQCOM
  {IR(30263, 22), 288}, // 420 ImageSparseDrefGather
  {IR(30228, 17), 286}, // 421 ImageSparseFetch
  {IR(30245, 18), 287}, // 422 ImageSparseGather
  {IR(30355, 16), 293}, // 423 ImageSparseRead
  {IR(30055, 33), 281}, // 424 ImageSparseSampleDrefExplicitLod
  {IR(30022, 33), 280}, // 425 ImageSparseSampleDrefImplicitLod
  {IR(29993, 29), 279}, // 426 ImageSparseSampleExplicitLod
  {IR(29964, 29), 278}, // 427 ImageSparseSampleImplicitLod
  {IR(30191, 37), 285}, // 428 ImageSparseSampleProjDrefExplicitLod
  {IR(30154, 37), 284}, // 429 ImageSparseSampleProjDrefImplicitLod
  {IR(30121, 33), 283}, // 430 ImageSparseSampleProjExplicitLod
  {IR(30088, 33), 282}, // 431 ImageSparseSampleProjImplicitLod
  {IR(30285, 26), 289}, // 432 ImageSparseTexelsResident
  {IR(26539, 18), 53}, // 433 ImageTexelPointer
  {IR(27348, 11), 90}, // 434 ImageWrite
  {IR(26647, 20), 59}, // 435 InBoundsAccessChain
  {IR(26717, 23), 63}, // 436 InBoundsPtrAccessChain
  {IR(27948, 9), 146}, // 437 IsFinite
  {IR(35789, 22), 516}, // 438 IsHelperInvocationEXT
  {IR(27942, 6), 145}, // 439 IsInf
  {IR(27936, 6), 144}, // 440 IsNan
  {IR(33434, 23), 430}, // 441 IsNodePayloadValidAMDX
  {IR(27957, 9), 147}, // 442 IsNormal
  {IR(29877, 13), 273}, // 443 IsValidEvent
  {IR(29532, 17), 257}, // 444 IsValidReserveId
  {IR(29160, 5), 230}, // 445 Kill
  {IR(29077, 6), 226}, // 446 Label
  {IR(27977, 14), 149}, // 447 LessOrGreater
  {IR(29196, 14), 234}, // 448 LifetimeStart
  {IR(29210, 13), 235}, // 449 LifetimeStop
  {IR(25390, 5), 8}, // 450 Line
  {IR(26597, 5), 54}, // 451 Load
  {IR(28048, 11), 155}, // 452 LogicalAnd
  {IR(28009, 13), 152}, // 453 LogicalEqual
  {IR(28059, 11), 156}, // 454 LogicalNot
  {IR(28022, 16), 153}, // 455 LogicalNotEqual
  {IR(28038, 10), 154}, // 456 LogicalOr
  {IR(43376, 17), 738}, // 457 LoopControlINTEL
  {IR(29017, 10), 224}, // 458 LoopMerge
  {IR(45393, 18), 815}, // 459 MaskedGatherINTEL
  {IR(45411, 19), 816}, // 460 MaskedScatterINTEL
  {IR(27846, 18), 135}, // 461 MatrixTimesMatrix
  {IR(27792, 18), 132}, // 462 MatrixTimesScalar
  {IR(27828, 18), 134}, // 463 MatrixTimesVector
  {IR(26777, 15), 65}, // 464 MemberDecorate
  {IR(37128, 21), 575}, // 465 MemberDecorateString
  {IR(37149, 27), 575}, // 466 MemberDecorateStringGOOGLE
  {IR(25372, 11), 6}, // 467 MemberName
  {IR(28754, 14), 206}, // 468 MemoryBarrier
  {IR(25537, 12), 12}, // 469 MemoryModel
  {IR(30540, 19), 302}, // 470 MemoryNamedBarrier
  {IR(30559, 16), 303}, // 471 ModuleProcessed
  {IR(25367, 5), 5}, // 472 Name
  {IR(30517, 23), 301}, // 473 NamedBarrierInitialize
  {IR(30311, 7), 290}, // 474 NoLine
  {IR(33407, 27), 429}, // 475 NodePayloadArrayLengthAMDX
  {IR(25163, 4), 0}, // 476 Nop
  {IR(28488, 4), 186}, // 477 Not
  {IR(27991, 8), 150}, // 478 Ordered
  {IR(27864, 13), 136}, // 479 OuterProduct
  {IR(28983, 4), 223}, // 480 Phi
  {IR(26667, 15), 60}, // 481 PtrAccessChain
  {IR(43629, 29), 753}, // 482 PtrCastToCrossWorkgroupINTEL
  {IR(27621, 17), 111}, // 483 PtrCastToGeneric
  {IR(31407, 8), 343}, // 484 PtrDiff
  {IR(31386, 9), 341}, // 485 PtrEqual
  {IR(31395, 12), 342}, // 486 PtrNotEqual
  {IR(27549, 14), 106}, // 487 QuantizeToF16
  {IR(36094, 17), 527}, // 488 RawAccessChainNV
  {IR(32688, 31), 399}, // 489 RayQueryConfirmIntersectionKHR
  {IR(32656, 32), 398}, // 490 RayQueryGenerateIntersectionKHR
  {IR(35104, 23), 492}, // 491 RayQueryGetClusterIdNV
  {IR(44034, 39), 766}, // 492 RayQueryGetIntersectionBarycentricsKHR
  {IR(44109, 46), 768}, // 493 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(44073, 36), 767}, // 494 RayQueryGetIntersectionFrontFaceKHR
  {IR(43953, 40), 764}, // 495 RayQueryGetIntersectionGeometryIndexKHR
  {IR(43805, 46), 761}, // 496 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(43851, 37), 762}, // 497 RayQueryGetIntersectionInstanceIdKHR
  {IR(43888, 65), 763}, // 498 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(36261, 37), 532}, // 499 RayQueryGetIntersectionLSSHitValueNV
  {IR(36189, 38), 530}, // 500 RayQueryGetIntersectionLSSPositionsNV
  {IR(36227, 34), 531}, // 501 RayQueryGetIntersectionLSSRadiiNV
  {IR(44155, 45), 769}, // 502 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(44200, 42), 770}, // 503 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(44303, 40), 773}, // 504 RayQueryGetIntersectionObjectToWorldKHR
  {IR(43993, 41), 765}, // 505 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(36111, 40), 528}, // 506 RayQueryGetIntersectionSpherePositionNV
  {IR(36151, 38), 529}, // 507 RayQueryGetIntersectionSphereRadiusNV
  {IR(43777, 28), 760}, // 508 RayQueryGetIntersectionTKHR
  {IR(34979, 50), 489}, // 509 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(32738, 31), 401}, // 510 RayQueryGetIntersectionTypeKHR
  {IR(44343, 40), 774}, // 511 RayQueryGetIntersectionWorldToObjectKHR
  {IR(43754, 23), 759}, // 512 RayQueryGetRayFlagsKHR
  {IR(43732, 22), 758}, // 513 RayQueryGetRayTMinKHR
  {IR(44242, 32), 771}, // 514 RayQueryGetWorldRayDirectionKHR
  {IR(44274, 29), 772}, // 515 RayQueryGetWorldRayOriginKHR
  {IR(32613, 22), 396}, // 516 RayQueryInitializeKHR
  {IR(36469, 19), 540}, // 517 RayQueryIsLSSHitNV
  {IR(36447, 22), 539}, // 518 RayQueryIsSphereHitNV
  {IR(32719, 19), 400}, // 519 RayQueryProceedKHR
  {IR(32635, 21), 397}, // 520 RayQueryTerminateKHR
  {IR(33291, 13), 424}, // 521 ReadClockKHR
  {IR(29400, 9), 249}, // 522 ReadPipe
  {IR(43687, 22), 755}, // 523 ReadPipeBlockingINTEL
  {IR(29848, 13), 271}, // 524 ReleaseEvent
  {IR(34388, 24), 466}, // 525 ReorderThreadWithHintNV
  {IR(34359, 29), 465}, // 526 ReorderThreadWithHitObjectNV
  {IR(34861, 22), 483}, // 527 ReportIntersectionKHR
  {IR(34883, 21), 483}, // 528 ReportIntersectionNV
  {IR(29454, 23), 253}, // 529 ReserveReadPipePackets
  {IR(29477, 24), 254}, // 530 ReserveWritePipePackets
  {IR(29419, 17), 251}, // 531 ReservedReadPipe
  {IR(29436, 18), 252}, // 532 ReservedWritePipe
  {IR(42358, 19), 696}, // 533 RestoreMemoryINTEL
  {IR(29836, 12), 270}, // 534 RetainEvent
  {IR(29165, 7), 231}, // 535 Return
  {IR(29172, 12), 232}, // 536 ReturnValue
  {IR(45375, 18), 814}, // 537 RoundFToTF32INTEL
  {IR(27531, 9), 104}, // 538 SConvert
  {IR(27739, 5), 124}, // 539 SDiv
  {IR(32200, 5), 381}, // 540 SDot
  {IR(32241, 11), 384}, // 541 SDotAccSat
  {IR(32252, 14), 384}, // 542 SDotAccSatKHR
  {IR(32205, 8), 381}, // 543 SDotKHR
  {IR(28107, 13), 161}, // 544 SGreaterThan
  {IR(28138, 18), 163}, // 545 SGreaterThanEqual
  {IR(28166, 10), 165}, // 546 SLessThan
  {IR(28191, 15), 167}, // 547 SLessThanEqual
  {IR(27759, 5), 128}, // 548 SMod
  {IR(27915, 13), 141}, // 549 SMulExtended
  {IR(27688, 8), 115}, // 550 SNegate
  {IR(27754, 5), 127}, // 551 SRem
  {IR(32226, 6), 383}, // 552 SUDot
  {IR(32291, 12), 386}, // 553 SUDotAccSat
  {IR(32303, 15), 386}, // 554 SUDotAccSatKHR
  {IR(32232, 9), 383}, // 555 SUDotKHR
  {IR(27015, 13), 77}, // 556 SampledImage
  {IR(36013, 29), 526}, // 557 SamplerImageAddressingModeNV
  {IR(27577, 15), 108}, // 558 SatConvertSToU
  {IR(27592, 15), 109}, // 559 SatConvertUToS
  {IR(42342, 16), 695}, // 560 SaveMemoryINTEL
  {IR(28070, 7), 157}, // 561 Select
  {IR(29062, 15), 225}, // 562 SelectionMerge
  {IR(34661, 18), 476}, // 563 SetMeshOutputsEXT
  {IR(29890, 19), 274}, // 564 SetUserEventStatus
  {IR(28439, 17), 182}, // 565 ShiftLeftLogical
  {IR(28418, 21), 181}, // 566 ShiftRightArithmetic
  {IR(28400, 18), 180}, // 567 ShiftRightLogical
  {IR(27966, 11), 148}, // 568 SignBitSet
  {IR(30371, 7), 294}, // 569 SizeOf
  {IR(25344, 7), 3}, // 570 Source
  {IR(25225, 16), 2}, // 571 SourceContinued
  {IR(25351, 16), 4}, // 572 SourceExtension
  {IR(26362, 13), 45}, // 573 SpecConstant
  {IR(45113, 30), 803}, // 574 SpecConstantArchitectureINTEL
  {IR(45180, 30), 804}, // 575 SpecConstantCapabilitiesINTEL
  {IR(26375, 22), 46}, // 576 SpecConstantComposite
  {IR(44477, 36), 779}, // 577 SpecConstantCompositeContinuedINTEL
  {IR(32532, 34), 393}, // 578 SpecConstantCompositeReplicateEXT
  {IR(26344, 18), 44}, // 579 SpecConstantFalse
  {IR(26438, 15), 47}, // 580 SpecConstantOp
  {IR(33476, 23), 432}, // 581 SpecConstantStringAMDX
  {IR(45089, 24), 802}, // 582 SpecConstantTargetINTEL
  {IR(26327, 17), 43}, // 583 SpecConstantTrue
  {IR(31461, 25), 346}, // 584 StencilAttachmentReadEXT
  {IR(26602, 6), 55}, // 585 Store
  {IR(25383, 7), 7}, // 586 String
  {IR(44778, 25), 792}, // 587 Subgroup2DBlockLoadINTEL
  {IR(44803, 34), 793}, // 588 Subgroup2DBlockLoadTransformINTEL
  {IR(44837, 34), 794}, // 589 Subgroup2DBlockLoadTransposeINTEL
  {IR(44871, 29), 795}, // 590 Subgroup2DBlockPrefetchINTEL
  {IR(44900, 26), 796}, // 591 Subgroup2DBlockStoreINTEL
  {IR(31952, 20), 371}, // 592 SubgroupAllEqualKHR
  {IR(31922, 15), 369}, // 593 SubgroupAllKHR
  {IR(31937, 15), 370}, // 594 SubgroupAnyKHR
  {IR(40860, 30), 659}, // 595 SubgroupAvcBmeInitializeINTEL
  {IR(40830, 30), 658}, // 596 SubgroupAvcFmeInitializeINTEL
  {IR(39265, 35), 628}, // 597 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(39300, 39), 629}, // 598 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(39941, 38), 642}, // 599 SubgroupAvcImeConvertToMceResultINTEL
  {IR(39564, 45), 635}, // 600 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(39664, 53), 637}, // 601 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(39885, 56), 641}, // 602 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(39773, 54), 639}, // 603 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(39517, 47), 634}, // 604 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(39609, 55), 636}, // 605 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(39827, 58), 640}, // 606 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(39717, 56), 638}, // 607 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(40573, 36), 653}, // 608 SubgroupAvcImeGetBorderReachedINTEL
  {IR(40025, 44), 644}, // 609 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(39979, 46), 643}, // 610 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(40440, 66), 651}, // 611 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(40372, 68), 650}, // 612 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(40506, 67), 652}, // 613 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(40235, 68), 648}, // 614 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(40165, 70), 647}, // 615 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(40303, 69), 649}, // 616 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(40609, 48), 654}, // 617 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(40657, 59), 655}, // 618 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(40774, 56), 657}, // 619 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(40716, 58), 656}, // 620 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(39128, 30), 624}, // 621 SubgroupAvcImeInitializeINTEL
  {IR(39232, 33), 627}, // 622 SubgroupAvcImeRefWindowSizeINTEL
  {IR(39196, 36), 626}, // 623 SubgroupAvcImeSetDualReferenceINTEL
  {IR(39429, 54), 632}, // 624 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(39339, 43), 630}, // 625 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(39158, 38), 625}, // 626 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(39382, 47), 631}, // 627 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(39483, 34), 633}, // 628 SubgroupAvcImeSetWeightedSadINTEL
  {IR(40118, 47), 646}, // 629 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(40069, 49), 645}, // 630 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(38518, 39), 609}, // 631 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(38557, 38), 610}, // 632 SubgroupAvcMceConvertToImeResultINTEL
  {IR(38595, 39), 611}, // 633 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(38634, 38), 612}, // 634 SubgroupAvcMceConvertToRefResultINTEL
  {IR(38672, 39), 613}, // 635 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(38711, 38), 614}, // 636 SubgroupAvcMceConvertToSicResultINTEL
  {IR(38824, 43), 617}, // 637 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(37956, 50), 598}, // 638 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(37554, 60), 590}, // 639 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(37754, 51), 594}, // 640 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(37900, 56), 597}, // 641 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(37667, 47), 592}, // 642 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(38255, 56), 604}, // 643 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(38154, 50), 602}, // 644 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(37849, 51), 596}, // 645 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(38058, 49), 600}, // 646 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(38006, 52), 599}, // 647 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(38204, 51), 603}, // 648 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(38943, 38), 620}, // 649 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(38785, 39), 616}, // 650 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(38867, 38), 618}, // 651 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(38905, 38), 619}, // 652 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(38981, 45), 621}, // 653 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(39026, 40), 622}, // 654 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(39066, 62), 623}, // 655 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(38749, 36), 615}, // 656 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(38311, 33), 605}, // 657 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(38457, 61), 608}, // 658 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(37614, 53), 591}, // 659 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(37805, 44), 595}, // 660 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(37714, 40), 593}, // 661 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(38107, 47), 601}, // 662 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(38396, 61), 607}, // 663 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(38344, 52), 606}, // 664 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(40890, 39), 660}, // 665 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(41212, 38), 667}, // 666 SubgroupAvcRefConvertToMceResultINTEL
  {IR(41065, 45), 664}, // 667 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(41110, 46), 665}, // 668 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(41156, 56), 666}, // 669 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41018, 47), 663}, // 670 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(40929, 46), 661}, // 671 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(40975, 43), 662}, // 672 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(41348, 42), 671}, // 673 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(41312, 36), 670}, // 674 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(41280, 32), 669}, // 675 SubgroupAvcSicConfigureSkcINTEL
  {IR(41429, 39), 673}, // 676 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(41969, 38), 685}, // 677 SubgroupAvcSicConvertToMceResultINTEL
  {IR(41744, 31), 680}, // 678 SubgroupAvcSicEvaluateIpeINTEL
  {IR(41822, 45), 682}, // 679 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(41867, 46), 683}, // 680 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(41913, 56), 684}, // 681 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(41775, 47), 681}, // 682 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(42086, 46), 688}, // 683 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(42042, 44), 687}, // 684 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(42307, 35), 693}, // 685 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(42173, 36), 690}, // 686 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(42007, 35), 686}, // 687 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(41390, 39), 672}, // 688 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(42132, 41), 689}, // 689 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(42209, 50), 691}, // 690 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(42259, 48), 692}, // 691 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(41250, 30), 668}, // 692 SubgroupAvcSicInitializeINTEL
  {IR(41610, 43), 677}, // 693 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(41701, 43), 679}, // 694 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(41560, 50), 676}, // 695 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(41512, 48), 675}, // 696 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(41468, 44), 674}, // 697 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(41653, 48), 678}, // 698 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(5411, 18), 363}, // 699 SubgroupBallotKHR
  {IR(44751, 27), 791}, // 700 SubgroupBlockPrefetchINTEL
  {IR(36560, 23), 545}, // 701 SubgroupBlockReadINTEL
  {IR(36583, 24), 546}, // 702 SubgroupBlockWriteINTEL
  {IR(31796, 27), 364}, // 703 SubgroupFirstInvocationKHR
  {IR(36607, 28), 547}, // 704 SubgroupImageBlockReadINTEL
  {IR(36635, 29), 548}, // 705 SubgroupImageBlockWriteINTEL
  {IR(36664, 33), 549}, // 706 SubgroupImageMediaBlockReadINTEL
  {IR(36697, 34), 550}, // 707 SubgroupImageMediaBlockWriteINTEL
  {IR(13795, 38), 797}, // 708 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(31972, 26), 373}, // 709 SubgroupReadInvocationKHR
  {IR(36488, 25), 542}, // 710 SubgroupShuffleDownINTEL
  {IR(12010, 21), 541}, // 711 SubgroupShuffleINTEL
  {IR(36513, 23), 543}, // 712 SubgroupShuffleUpINTEL
  {IR(36536, 24), 544}, // 713 SubgroupShuffleXorINTEL
  {IR(29153, 7), 229}, // 714 Switch
  {IR(44660, 23), 787}, // 715 TaskSequenceAsyncINTEL
  {IR(44636, 24), 786}, // 716 TaskSequenceCreateINTEL
  {IR(44683, 21), 788}, // 717 TaskSequenceGetINTEL
  {IR(44704, 25), 789}, // 718 TaskSequenceReleaseINTEL
  {IR(35831, 27), 518}, // 719 TensorLayoutSetBlockSizeNV
  {IR(35695, 28), 511}, // 720 TensorLayoutSetClampValueNV
  {IR(35624, 27), 508}, // 721 TensorLayoutSetDimensionNV
  {IR(35651, 24), 509}, // 722 TensorLayoutSetStrideNV
  {IR(35675, 20), 510}, // 723 TensorLayoutSliceNV
  {IR(31571, 19), 350}, // 724 TensorQuerySizeARM
  {IR(31542, 14), 348}, // 725 TensorReadARM
  {IR(35811, 20), 517}, // 726 TensorViewSetClipNV
  {IR(35742, 25), 513}, // 727 TensorViewSetDimensionNV
  {IR(35767, 22), 514}, // 728 TensorViewSetStrideNV
  {IR(31556, 15), 349}, // 729 TensorWriteARM
  {IR(31683, 20), 358}, // 730 TerminateInvocation
  {IR(32137, 16), 380}, // 731 TerminateRayKHR
  {IR(34925, 15), 485}, // 732 TerminateRayNV
  {IR(34948, 14), 487}, // 733 TraceMotionNV
  {IR(34940, 8), 486}, // 734 TraceNV
  {IR(32049, 12), 376}, // 735 TraceRayKHR
  {IR(34962, 17), 488}, // 736 TraceRayMotionNV
  {IR(27005, 10), 76}, // 737 Transpose
  {IR(35029, 29), 490}, // 738 TypeAccelerationStructureKHR
  {IR(35058, 28), 490}, // 739 TypeAccelerationStructureNV
  {IR(25954, 10), 25}, // 740 TypeArray
  {IR(37473, 37), 587}, // 741 TypeAvcImeDualReferenceStreaminINTEL
  {IR(37208, 23), 578}, // 742 TypeAvcImePayloadINTEL
  {IR(37390, 44), 585}, // 743 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(37322, 22), 583}, // 744 TypeAvcImeResultINTEL
  {IR(37344, 46), 584}, // 745 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(37434, 39), 586}, // 746 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(37277, 23), 581}, // 747 TypeAvcMcePayloadINTEL
  {IR(37300, 22), 582}, // 748 TypeAvcMceResultINTEL
  {IR(37231, 23), 579}, // 749 TypeAvcRefPayloadINTEL
  {IR(37510, 22), 588}, // 750 TypeAvcRefResultINTEL
  {IR(37254, 23), 580}, // 751 TypeAvcSicPayloadINTEL
  {IR(37532, 22), 589}, // 752 TypeAvcSicResultINTEL
  {IR(25716, 9), 17}, // 753 TypeBool
  {IR(44397, 23), 776}, // 754 TypeBufferSurfaceINTEL
  {IR(32318, 25), 387}, // 755 TypeCooperativeMatrixKHR
  {IR(35151, 24), 494}, // 756 TypeCooperativeMatrixNV
  {IR(34451, 24), 469}, // 757 TypeCooperativeVectorNV
  {IR(26069, 16), 32}, // 758 TypeDeviceEvent
  {IR(26059, 10), 31}, // 759 TypeEvent
  {IR(25770, 10), 19}, // 760 TypeFloat
  {IR(26118, 19), 36}, // 761 TypeForwardPointer
  {IR(26046, 13), 30}, // 762 TypeFunction
  {IR(31670, 13), 357}, // 763 TypeGraphARM
  {IR(34412, 16), 467}, // 764 TypeHitObjectNV
  {IR(25915, 10), 22}, // 765 TypeImage
  {IR(25725, 8), 18}, // 766 TypeInt
  {IR(25791, 11), 21}, // 767 TypeMatrix
  {IR(30500, 17), 300}, // 768 TypeNamedBarrier
  {IR(33353, 25), 427}, // 769 TypeNodePayloadArrayAMDX
  {IR(25992, 11), 28}, // 770 TypeOpaque
  {IR(26109, 9), 35}, // 771 TypePipe
  {IR(30378, 16), 295}, // 772 TypePipeStorage
  {IR(26034, 12), 29}, // 773 TypePointer
  {IR(26099, 10), 34}, // 774 TypeQueue
  {IR(32597, 16), 395}, // 775 TypeRayQueryKHR
  {IR(26085, 14), 33}, // 776 TypeReserveId
  {IR(25964, 17), 26}, // 777 TypeRuntimeArray
  {IR(25937, 17), 24}, // 778 TypeSampledImage
  {IR(25925, 12), 23}, // 779 TypeSampler
  {IR(25981, 11), 27}, // 780 TypeStruct
  {IR(44420, 25), 777}, // 781 TypeStructContinuedINTEL
  {IR(44729, 22), 790}, // 782 TypeTaskSequenceINTEL
  {IR(31486, 14), 347}, // 783 TypeTensorARM
  {IR(35567, 19), 505}, // 784 TypeTensorLayoutNV
  {IR(35586, 17), 506}, // 785 TypeTensorViewNV
  {IR(31703, 22), 359}, // 786 TypeUntypedPointerKHR
  {IR(25780, 11), 20}, // 787 TypeVector
  {IR(37190, 18), 577}, // 788 TypeVmeImageINTEL
  {IR(25707, 9), 16}, // 789 TypeVoid
  {IR(36819, 13), 556}, // 790 UAddSatINTEL
  {IR(36846, 14), 558}, // 791 UAverageINTEL
  {IR(36881, 21), 560}, // 792 UAverageRoundedINTEL
  {IR(27522, 9), 103}, // 793 UConvert
  {IR(36731, 24), 551}, // 794 UCountLeadingZerosINTEL
  {IR(36755, 25), 552}, // 795 UCountTrailingZerosINTEL
  {IR(27734, 5), 123}, // 796 UDiv
  {IR(32213, 5), 382}, // 797 UDot
  {IR(32266, 11), 385}, // 798 UDotAccSat
  {IR(32277, 14), 385}, // 799 UDotAccSatKHR
  {IR(32218, 8), 382}, // 800 UDotKHR
  {IR(28094, 13), 160}, // 801 UGreaterThan
  {IR(28120, 18), 162}, // 802 UGreaterThanEqual
  {IR(28156, 10), 164}, // 803 ULessThan
  {IR(28176, 15), 166}, // 804 ULessThanEqual
  {IR(27749, 5), 126}, // 805 UMod
  {IR(36943, 15), 564}, // 806 UMul32x16INTEL
  {IR(27902, 13), 140}, // 807 UMulExtended
  {IR(36915, 13), 562}, // 808 USubSatINTEL
  {IR(25219, 6), 1}, // 809 Undef
  {IR(27999, 10), 151}, // 810 Unordered
  {IR(29184, 12), 233}, // 811 Unreachable
  {IR(31744, 22), 361}, // 812 UntypedAccessChainKHR
  {IR(31881, 22), 367}, // 813 UntypedArrayLengthKHR
  {IR(32024, 25), 375}, // 814 UntypedGroupAsyncCopyKHR
  {IR(31766, 30), 362}, // 815 UntypedInBoundsAccessChainKHR
  {IR(31848, 33), 366}, // 816 UntypedInBoundsPtrAccessChainKHR
  {IR(31903, 19), 368}, // 817 UntypedPrefetchKHR
  {IR(31823, 25), 365}, // 818 UntypedPtrAccessChainKHR
  {IR(31725, 19), 360}, // 819 UntypedVariableKHR
  {IR(26530, 9), 52}, // 820 Variable
  {IR(12466, 25), 694}, // 821 VariableLengthArrayINTEL
  {IR(26887, 21), 69}, // 822 VectorExtractDynamic
  {IR(26908, 20), 70}, // 823 VectorInsertDynamic
  {IR(26928, 14), 71}, // 824 VectorShuffle
  {IR(27810, 18), 133}, // 825 VectorTimesMatrix
  {IR(27774, 18), 131}, // 826 VectorTimesScalar
  {IR(37176, 14), 576}, // 827 VmeImageINTEL
  {IR(34706, 33), 478}, // 828 WritePackedPrimitiveIndices4x8NV
  {IR(29409, 10), 250}, // 829 WritePipe
  {IR(43709, 23), 756}, // 830 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 491> kExtInstByValue{{
    {0,IR(0, 0),IR(45517, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(45531, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(770, 3),IR(45604, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(773, 3),IR(45653, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(776, 2),IR(45708, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(45727, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(45742, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(778, 6),IR(45758, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(45771, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(784, 9),IR(45789, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(793, 9),IR(45841, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(802, 10),IR(45860, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(812, 5),IR(45876, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(45897, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(45918, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(817, 6),IR(45936, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(823, 5),IR(45963, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(828, 5),IR(45998, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(802, 10),IR(46029, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(833, 8),IR(46049, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(841, 11),IR(46074, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(852, 5),IR(46088, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(857, 3),IR(46106, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(860, 2),IR(46137, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(46148, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(862, 3),IR(46161, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(865, 7),IR(46218, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(46237, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(46258, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(46271, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(872, 2),IR(46315, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(706, 1),IR(46330, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(874, 4),IR(46346, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(857, 3),IR(46360, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(46376, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(46382, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(46392, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(46398, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(46403, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(46408, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(46414, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(46420, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(46426, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(46431, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(46437, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(46445, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(46453, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(46457, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(46461, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(46465, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(46470, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(46475, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(46480, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(46485, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(46490, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(46495, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(46501, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(46507, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(46513, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(46519, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(46523, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(46527, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(46531, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(46536, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(46541, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(46546, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(46558, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(46570, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(46584, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(46589, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(46600, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(46605, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(46610, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(46615, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(46620, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(46625, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(46630, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(46637, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(46644, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(46651, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(46656, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(46661, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(46666, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(46677, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(46681, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(46687, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(46699, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(46705, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(46718, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(46731, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(46745, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(46759, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(46772, 15),IR(306, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(46787, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(46803, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(46819, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(46834, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(46849, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(46864, 17),IR(306, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(46881, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(46888, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(46897, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(46903, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(46913, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(46925, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(46933, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(46941, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(46950, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(46959, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(46968, 22),IR(307, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(46990, 20),IR(307, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(47010, 20),IR(307, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(47030, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(47035, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(47040, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(4630, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(878, 5),IR(47047, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(883, 5),IR(47060, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(883, 5),IR(47082, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(554, 7),IR(47098, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(554, 7),IR(47123, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(883, 5),IR(47142, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(883, 5),IR(47166, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(883, 5),IR(47187, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(883, 5),IR(47208, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(883, 5),IR(47224, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(47242, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(47268, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(47293, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(47313, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(47338, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(47368, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(47391, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(47416, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(47442, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(47472, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(47498, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(8055, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(47518, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(47548, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(883, 5),IR(47576, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(554, 7),IR(47604, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(47627, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(47662, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(47700, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(47742, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(525, 6),IR(47787, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(525, 6),IR(47824, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(883, 5),IR(47864, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(883, 5),IR(47891, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(47918, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(47950, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(47990, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(48001, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(48027, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(48059, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(48093, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {0,IR(0, 0),IR(45517, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {1,IR(296, 4),IR(45531, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {2,IR(296, 4),IR(45604, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {3,IR(33, 3),IR(45653, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {4,IR(36, 2),IR(45708, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {5,IR(168, 2),IR(45727, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {6,IR(36, 2),IR(45742, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {7,IR(525, 6),IR(45758, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {8,IR(375, 3),IR(45771, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {9,IR(888, 9),IR(45789, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {10,IR(897, 10),IR(45841, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {11,IR(907, 9),IR(45860, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {12,IR(296, 4),IR(45876, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {13,IR(36, 2),IR(45897, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {14,IR(168, 2),IR(45918, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {15,IR(525, 6),IR(45936, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {16,IR(634, 5),IR(45963, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {17,IR(916, 5),IR(45998, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {18,IR(921, 10),IR(46029, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {19,IR(471, 8),IR(46049, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {20,IR(921, 10),IR(46074, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {21,IR(883, 5),IR(46088, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {22,IR(33, 3),IR(46106, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {23,IR(860, 2),IR(46137, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo_100
    {24,IR(0, 0),IR(46148, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {25,IR(931, 3),IR(46161, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {26,IR(934, 8),IR(46218, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {27,IR(36, 2),IR(46237, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {28,IR(942, 4),IR(46258, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {29,IR(942, 4),IR(46271, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo_100
    {30,IR(168, 2),IR(46315, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {31,IR(706, 1),IR(46330, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {32,IR(577, 4),IR(46346, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {33,IR(33, 3),IR(46360, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {34,IR(506, 7),IR(48115, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {35,IR(860, 2),IR(48135, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo_100
    {101,IR(36, 2),IR(48147, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {102,IR(10, 1),IR(48171, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {103,IR(634, 5),IR(48192, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo_100
    {104,IR(0, 0),IR(48202, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {105,IR(36, 2),IR(48214, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {106,IR(10, 1),IR(48235, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {107,IR(296, 4),IR(48252, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {108,IR(33, 3),IR(48268, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {1,IR(724, 9),IR(48284, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(48298, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(48311, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(48323, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(48337, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(946, 15),IR(48360, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(961, 33),IR(48380, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(994, 19),IR(48399, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(45517, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1013, 4),IR(45531, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1017, 3),IR(45604, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1020, 3),IR(45653, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1023, 2),IR(45708, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(45727, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(45742, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(778, 6),IR(45758, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1025, 3),IR(45771, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1028, 9),IR(45789, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1037, 10),IR(45841, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1047, 10),IR(45860, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1057, 5),IR(45876, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(45897, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(45918, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(817, 6),IR(45936, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(823, 5),IR(45963, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(828, 5),IR(45998, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1047, 10),IR(46029, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1062, 8),IR(46049, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1070, 11),IR(46074, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(852, 5),IR(46088, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(857, 3),IR(46106, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(860, 2),IR(46137, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(46148, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(862, 3),IR(46161, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1081, 8),IR(46218, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(46237, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(46258, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(942, 4),IR(46271, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1089, 2),IR(46315, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(706, 1),IR(46330, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(874, 4),IR(46346, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(857, 3),IR(46360, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1091, 7),IR(48115, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(860, 2),IR(48135, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1098, 8),IR(48720, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(48737, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(48742, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(48748, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(48755, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(48760, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(48766, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(48773, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(48778, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(48784, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(48790, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(48797, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(48805, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(48810, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(48815, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(48824, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(48828, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(48833, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(48839, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(48844, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(48848, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(48852, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(48857, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(48863, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(48869, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(48874, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(48879, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(48885, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(48889, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(48894, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(48899, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(48904, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(48910, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(48916, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(48922, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(48928, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(48934, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(48941, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(48950, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(48954, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(48959, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(48965, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(48971, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(48976, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(48980, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(48987, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(48994, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(48999, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(49003, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(49013, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(49017, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(49022, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(49027, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(49037, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(49044, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(49049, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(49055, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(49061, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(49067, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(49071, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(49078, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(49083, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(49089, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(49094, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(49098, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(49103, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(49109, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(49116, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(49122, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(49131, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(49143, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(49152, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(49162, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(49173, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(49182, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(49192, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(49203, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(49213, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(49224, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(49235, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(49244, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(49254, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(49263, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(49274, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(49288, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(49299, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(49311, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(49324, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(49335, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(49347, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(49360, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(49372, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(49385, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(49398, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(49409, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(49421, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(49432, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(49439, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(49447, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(49459, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(49471, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(49475, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(49483, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(49488, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(49499, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(49504, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(49510, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(49519, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(49526, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(49536, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(49550, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(49562, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(49577, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(49583, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(49594, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(49604, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(49614, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(49621, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(49628, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(49636, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(49644, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(49652, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(49660, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(49664, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(49668, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(49677, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(49687, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(49697, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(49703, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(49709, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(49715, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(49721, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(49730, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(49737, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(49747, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(49757, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(49768, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(49779, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(49788, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(49796, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(49804, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(49812, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1106, 3),IR(49820, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(49827, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(49835, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1106, 3),IR(49846, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(49858, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1109, 4),IR(49870, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(49884, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1109, 4),IR(49897, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1106, 3),IR(49912, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(49925, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1109, 4),IR(49939, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(49955, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(49963, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(49972, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(49979, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(49988, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(49998, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(50005, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(50011, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(50022, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(50031, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(50040, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(50057, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(50074, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(50082, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(50104, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(50132, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(50151, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(50160, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(50183, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(50192, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(50201, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(50210, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(50219, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(50228, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(50237, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(50246, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(50255, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(50264, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(506, 7),IR(50271, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(714, 10),IR(50282, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(714, 10),IR(50289, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(714, 10),IR(50296, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(50313, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(50319, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(634, 5),IR(50326, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(50337, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(724, 9),IR(50344, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(50361, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(50367, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(50371, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(50379, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(50384, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(50388, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(50411, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(50423, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(50434, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(50446, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(50453, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(50465, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(50484, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(50504, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(50515, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(50527, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(50535, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(50543, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(50547, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(50551, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(50555, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(50561, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(50565, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(50577, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(50582, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(50586, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(50590, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(50594, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(50600, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(50604, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(50616, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(50623, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(50634, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(50640, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(50644, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(50651, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(50657, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(50665, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(50679, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(50690, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(50701, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(50712, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(50723, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(50738, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(50749, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(50756, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(50760, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(50768, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(50776, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(50782, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(50787, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(50797, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(50804, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(634, 5),IR(50812, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(50819, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(714, 10),IR(50824, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    {IR(45531, 21), 1}, // DebugCompilationUnit in debuginfo
    {IR(46258, 13), 28}, // DebugDeclare in debuginfo
    {IR(46330, 16), 31}, // DebugExpression in debuginfo
    {IR(46074, 14), 20}, // DebugFunction in debuginfo
    {IR(46049, 25), 19}, // DebugFunctionDeclaration in debuginfo
    {IR(46029, 20), 18}, // DebugGlobalVariable in debuginfo
    {IR(45517, 14), 0}, // DebugInfoNone in debuginfo
    {IR(46161, 15), 25}, // DebugInlinedAt in debuginfo
    {IR(46237, 21), 27}, // DebugInlinedVariable in debuginfo
    {IR(46088, 18), 21}, // DebugLexicalBlock in debuginfo
    {IR(46106, 31), 22}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(46218, 19), 26}, // DebugLocalVariable in debuginfo
    {IR(46346, 14), 32}, // DebugMacroDef in debuginfo
    {IR(46360, 16), 33}, // DebugMacroUndef in debuginfo
    {IR(46148, 13), 24}, // DebugNoScope in debuginfo
    {IR(46315, 15), 30}, // DebugOperation in debuginfo
    {IR(46137, 11), 23}, // DebugScope in debuginfo
    {IR(45727, 15), 5}, // DebugTypeArray in debuginfo
    {IR(45604, 15), 2}, // DebugTypeBasic in debuginfo
    {IR(45841, 19), 10}, // DebugTypeComposite in debuginfo
    {IR(45789, 14), 9}, // DebugTypeEnum in debuginfo
    {IR(45771, 18), 8}, // DebugTypeFunction in debuginfo
    {IR(45876, 21), 12}, // DebugTypeInheritance in debuginfo
    {IR(45860, 16), 11}, // DebugTypeMember in debuginfo
    {IR(45653, 17), 3}, // DebugTypePointer in debuginfo
    {IR(45897, 21), 13}, // DebugTypePtrToMember in debuginfo
    {IR(45708, 19), 4}, // DebugTypeQualifier in debuginfo
    {IR(45918, 18), 14}, // DebugTypeTemplate in debuginfo
    {IR(45936, 27), 15}, // DebugTypeTemplateParameter in debuginfo
    {IR(45998, 31), 17}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(45963, 35), 16}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(45742, 16), 6}, // DebugTypeVector in debuginfo
    {IR(45758, 13), 7}, // DebugTypedef in debuginfo
    {IR(46271, 11), 29}, // DebugValue in debuginfo
    {IR(46470, 5), 50}, // Acos in glsl_std_450
    {IR(46501, 6), 56}, // Acosh in glsl_std_450
    {IR(46465, 5), 49}, // Asin in glsl_std_450
    {IR(46495, 6), 55}, // Asinh in glsl_std_450
    {IR(46475, 5), 51}, // Atan in glsl_std_450
    {IR(46513, 6), 58}, // Atan2 in glsl_std_450
    {IR(46507, 6), 57}, // Atanh in glsl_std_450
    {IR(46426, 5), 42}, // Ceil in glsl_std_450
    {IR(46457, 4), 47}, // Cos in glsl_std_450
    {IR(46485, 5), 53}, // Cosh in glsl_std_450
    {IR(46897, 6), 101}, // Cross in glsl_std_450
    {IR(46445, 8), 45}, // Degrees in glsl_std_450
    {IR(46558, 12), 66}, // Determinant in glsl_std_450
    {IR(46888, 9), 100}, // Distance in glsl_std_450
    {IR(46523, 4), 60}, // Exp in glsl_std_450
    {IR(46531, 5), 62}, // Exp2 in glsl_std_450
    {IR(46398, 5), 37}, // FAbs in glsl_std_450
    {IR(46630, 7), 76}, // FClamp in glsl_std_450
    {IR(46615, 5), 73}, // FMax in glsl_std_450
    {IR(46600, 5), 70}, // FMin in glsl_std_450
    {IR(46651, 5), 79}, // FMix in glsl_std_450
    {IR(46408, 6), 39}, // FSign in glsl_std_450
    {IR(46913, 12), 103}, // FaceForward in glsl_std_450
    {IR(46941, 9), 106}, // FindILsb in glsl_std_450
    {IR(46950, 9), 107}, // FindSMsb in glsl_std_450
    {IR(46959, 9), 108}, // FindUMsb in glsl_std_450
    {IR(46420, 6), 41}, // Floor in glsl_std_450
    {IR(46677, 4), 83}, // Fma in glsl_std_450
    {IR(46431, 6), 43}, // Fract in glsl_std_450
    {IR(46681, 6), 84}, // Frexp in glsl_std_450
    {IR(46687, 12), 85}, // FrexpStruct in glsl_std_450
    {IR(46656, 5), 80}, // IMix in glsl_std_450
    {IR(46968, 22), 109}, // InterpolateAtCentroid in glsl_std_450
    {IR(47010, 20), 111}, // InterpolateAtOffset in glsl_std_450
    {IR(46990, 20), 110}, // InterpolateAtSample in glsl_std_450
    {IR(46546, 12), 65}, // InverseSqrt in glsl_std_450
    {IR(46699, 6), 86}, // Ldexp in glsl_std_450
    {IR(46881, 7), 99}, // Length in glsl_std_450
    {IR(46527, 4), 61}, // Log in glsl_std_450
    {IR(46536, 5), 63}, // Log2 in glsl_std_450
    {IR(46570, 14), 67}, // MatrixInverse in glsl_std_450
    {IR(46584, 5), 68}, // Modf in glsl_std_450
    {IR(46589, 11), 69}, // ModfStruct in glsl_std_450
    {IR(47040, 7), 114}, // NClamp in glsl_std_450
    {IR(47035, 5), 113}, // NMax in glsl_std_450
    {IR(47030, 5), 112}, // NMin in glsl_std_450
    {IR(46903, 10), 102}, // Normalize in glsl_std_450
    {IR(46772, 15), 92}, // PackDouble2x32 in glsl_std_450
    {IR(46759, 13), 91}, // PackHalf2x16 in glsl_std_450
    {IR(46731, 14), 89}, // PackSnorm2x16 in glsl_std_450
    {IR(46705, 13), 87}, // PackSnorm4x8 in glsl_std_450
    {IR(46745, 14), 90}, // PackUnorm2x16 in glsl_std_450
    {IR(46718, 13), 88}, // PackUnorm4x8 in glsl_std_450
    {IR(46519, 4), 59}, // Pow in glsl_std_450
    {IR(46437, 8), 44}, // Radians in glsl_std_450
    {IR(46925, 8), 104}, // Reflect in glsl_std_450
    {IR(46933, 8), 105}, // Refract in glsl_std_450
    {IR(46376, 6), 34}, // Round in glsl_std_450
    {IR(46382, 10), 35}, // RoundEven in glsl_std_450
    {IR(46403, 5), 38}, // SAbs in glsl_std_450
    {IR(46644, 7), 78}, // SClamp in glsl_std_450
    {IR(46625, 5), 75}, // SMax in glsl_std_450
    {IR(46610, 5), 72}, // SMin in glsl_std_450
    {IR(46414, 6), 40}, // SSign in glsl_std_450
    {IR(46453, 4), 46}, // Sin in glsl_std_450
    {IR(46480, 5), 52}, // Sinh in glsl_std_450
    {IR(46666, 11), 82}, // SmoothStep in glsl_std_450
    {IR(46541, 5), 64}, // Sqrt in glsl_std_450
    {IR(46661, 5), 81}, // Step in glsl_std_450
    {IR(46461, 4), 48}, // Tan in glsl_std_450
    {IR(46490, 5), 54}, // Tanh in glsl_std_450
    {IR(46392, 6), 36}, // Trunc in glsl_std_450
    {IR(46637, 7), 77}, // UClamp in glsl_std_450
    {IR(46620, 5), 74}, // UMax in glsl_std_450
    {IR(46605, 5), 71}, // UMin in glsl_std_450
    {IR(46864, 17), 98}, // UnpackDouble2x32 in glsl_std_450
    {IR(46819, 15), 95}, // UnpackHalf2x16 in glsl_std_450
    {IR(46787, 16), 93}, // UnpackSnorm2x16 in glsl_std_450
    {IR(46834, 15), 96}, // UnpackSnorm4x8 in glsl_std_450
    {IR(46803, 16), 94}, // UnpackUnorm2x16 in glsl_std_450
    {IR(46849, 15), 97}, // UnpackUnorm4x8 in glsl_std_450
    {IR(47047, 13), 116}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(47142, 24), 121}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(47098, 25), 119}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(47123, 19), 120}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(47576, 28), 140}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(47604, 23), 141}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(47166, 21), 122}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(47208, 16), 124}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(47060, 22), 117}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(47187, 21), 123}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(47864, 27), 148}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(47082, 16), 118}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(47891, 27), 149}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(47224, 18), 125}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(47918, 32), 150}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(47472, 26), 135}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(47498, 20), 136}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(47742, 45), 145}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(47824, 40), 147}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(47700, 42), 144}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(47787, 37), 146}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(4630, 7), 115}, // Kernel in nonsemantic_clspvreflection
    {IR(8055, 15), 137}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(48059, 34), 155}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(48027, 32), 154}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(48001, 26), 153}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(47990, 11), 152}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(47950, 40), 151}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(47662, 38), 143}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(47627, 35), 142}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(47518, 30), 138}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(47338, 30), 130}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(47313, 25), 129}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(47368, 23), 131}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(47416, 26), 133}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(47442, 30), 134}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(47391, 25), 132}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(47268, 25), 127}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(47548, 28), 139}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(47293, 20), 128}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(47242, 26), 126}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(48093, 22), 156}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(48214, 21), 197}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo_100
    {IR(45531, 21), 158}, // DebugCompilationUnit in nonsemantic_shader_debuginfo_100
    {IR(46258, 13), 185}, // DebugDeclare in nonsemantic_shader_debuginfo_100
    {IR(48252, 16), 199}, // DebugEntryPoint in nonsemantic_shader_debuginfo_100
    {IR(46330, 16), 188}, // DebugExpression in nonsemantic_shader_debuginfo_100
    {IR(46074, 14), 177}, // DebugFunction in nonsemantic_shader_debuginfo_100
    {IR(46049, 25), 176}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo_100
    {IR(48147, 24), 193}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo_100
    {IR(46029, 20), 175}, // DebugGlobalVariable in nonsemantic_shader_debuginfo_100
    {IR(48115, 20), 191}, // DebugImportedEntity in nonsemantic_shader_debuginfo_100
    {IR(45517, 14), 157}, // DebugInfoNone in nonsemantic_shader_debuginfo_100
    {IR(46161, 15), 182}, // DebugInlinedAt in nonsemantic_shader_debuginfo_100
    {IR(46237, 21), 184}, // DebugInlinedVariable in nonsemantic_shader_debuginfo_100
    {IR(46088, 18), 178}, // DebugLexicalBlock in nonsemantic_shader_debuginfo_100
    {IR(46106, 31), 179}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo_100
    {IR(48192, 10), 195}, // DebugLine in nonsemantic_shader_debuginfo_100
    {IR(46218, 19), 183}, // DebugLocalVariable in nonsemantic_shader_debuginfo_100
    {IR(46346, 14), 189}, // DebugMacroDef in nonsemantic_shader_debuginfo_100
    {IR(46360, 16), 190}, // DebugMacroUndef in nonsemantic_shader_debuginfo_100
    {IR(48202, 12), 196}, // DebugNoLine in nonsemantic_shader_debuginfo_100
    {IR(46148, 13), 181}, // DebugNoScope in nonsemantic_shader_debuginfo_100
    {IR(46315, 15), 187}, // DebugOperation in nonsemantic_shader_debuginfo_100
    {IR(46137, 11), 180}, // DebugScope in nonsemantic_shader_debuginfo_100
    {IR(48135, 12), 192}, // DebugSource in nonsemantic_shader_debuginfo_100
    {IR(48171, 21), 194}, // DebugSourceContinued in nonsemantic_shader_debuginfo_100
    {IR(48235, 17), 198}, // DebugStoragePath in nonsemantic_shader_debuginfo_100
    {IR(45727, 15), 162}, // DebugTypeArray in nonsemantic_shader_debuginfo_100
    {IR(45604, 15), 159}, // DebugTypeBasic in nonsemantic_shader_debuginfo_100
    {IR(45841, 19), 167}, // DebugTypeComposite in nonsemantic_shader_debuginfo_100
    {IR(45789, 14), 166}, // DebugTypeEnum in nonsemantic_shader_debuginfo_100
    {IR(45771, 18), 165}, // DebugTypeFunction in nonsemantic_shader_debuginfo_100
    {IR(45876, 21), 169}, // DebugTypeInheritance in nonsemantic_shader_debuginfo_100
    {IR(48268, 16), 200}, // DebugTypeMatrix in nonsemantic_shader_debuginfo_100
    {IR(45860, 16), 168}, // DebugTypeMember in nonsemantic_shader_debuginfo_100
    {IR(45653, 17), 160}, // DebugTypePointer in nonsemantic_shader_debuginfo_100
    {IR(45897, 21), 170}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo_100
    {IR(45708, 19), 161}, // DebugTypeQualifier in nonsemantic_shader_debuginfo_100
    {IR(45918, 18), 171}, // DebugTypeTemplate in nonsemantic_shader_debuginfo_100
    {IR(45936, 27), 172}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45998, 31), 174}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo_100
    {IR(45963, 35), 173}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo_100
    {IR(45742, 16), 163}, // DebugTypeVector in nonsemantic_shader_debuginfo_100
    {IR(45758, 13), 164}, // DebugTypedef in nonsemantic_shader_debuginfo_100
    {IR(46271, 11), 186}, // DebugValue in nonsemantic_shader_debuginfo_100
    {IR(48284, 14), 201}, // Configuration in nonsemantic_vkspreflection
    {IR(48360, 20), 206}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(48380, 19), 207}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(48399, 21), 208}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(48323, 14), 204}, // PushConstants in nonsemantic_vkspreflection
    {IR(48337, 23), 205}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(48298, 13), 202}, // StartCounter in nonsemantic_vkspreflection
    {IR(48311, 12), 203}, // StopCounter in nonsemantic_vkspreflection
    {IR(45531, 21), 210}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(46258, 13), 237}, // DebugDeclare in opencl_debuginfo_100
    {IR(46330, 16), 240}, // DebugExpression in opencl_debuginfo_100
    {IR(46074, 14), 229}, // DebugFunction in opencl_debuginfo_100
    {IR(46049, 25), 228}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(46029, 20), 227}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(48115, 20), 243}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(45517, 14), 209}, // DebugInfoNone in opencl_debuginfo_100
    {IR(46161, 15), 234}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(46237, 21), 236}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(46088, 18), 230}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(46106, 31), 231}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(46218, 19), 235}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(46346, 14), 241}, // DebugMacroDef in opencl_debuginfo_100
    {IR(46360, 16), 242}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(48720, 17), 245}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(46148, 13), 233}, // DebugNoScope in opencl_debuginfo_100
    {IR(46315, 15), 239}, // DebugOperation in opencl_debuginfo_100
    {IR(46137, 11), 232}, // DebugScope in opencl_debuginfo_100
    {IR(48135, 12), 244}, // DebugSource in opencl_debuginfo_100
    {IR(45727, 15), 214}, // DebugTypeArray in opencl_debuginfo_100
    {IR(45604, 15), 211}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(45841, 19), 219}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(45789, 14), 218}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(45771, 18), 217}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(45876, 21), 221}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(45860, 16), 220}, // DebugTypeMember in opencl_debuginfo_100
    {IR(45653, 17), 212}, // DebugTypePointer in opencl_debuginfo_100
    {IR(45897, 21), 222}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(45708, 19), 213}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(45918, 18), 223}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(45936, 27), 224}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(45998, 31), 226}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(45963, 35), 225}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(45742, 16), 215}, // DebugTypeVector in opencl_debuginfo_100
    {IR(45758, 13), 216}, // DebugTypedef in opencl_debuginfo_100
    {IR(46271, 11), 238}, // DebugValue in opencl_debuginfo_100
    {IR(48737, 5), 246}, // acos in opencl_std_100
    {IR(48742, 6), 247}, // acosh in opencl_std_100
    {IR(48748, 7), 248}, // acospi in opencl_std_100
    {IR(48755, 5), 249}, // asin in opencl_std_100
    {IR(48760, 6), 250}, // asinh in opencl_std_100
    {IR(48766, 7), 251}, // asinpi in opencl_std_100
    {IR(48773, 5), 252}, // atan in opencl_std_100
    {IR(48778, 6), 253}, // atan2 in opencl_std_100
    {IR(48797, 8), 256}, // atan2pi in opencl_std_100
    {IR(48784, 6), 254}, // atanh in opencl_std_100
    {IR(48790, 7), 255}, // atanpi in opencl_std_100
    {IR(49988, 10), 402}, // bitselect in opencl_std_100
    {IR(48805, 5), 257}, // cbrt in opencl_std_100
    {IR(48810, 5), 258}, // ceil in opencl_std_100
    {IR(49660, 4), 367}, // clz in opencl_std_100
    {IR(48815, 9), 259}, // copysign in opencl_std_100
    {IR(48824, 4), 260}, // cos in opencl_std_100
    {IR(48828, 5), 261}, // cosh in opencl_std_100
    {IR(48833, 6), 262}, // cospi in opencl_std_100
    {IR(49504, 6), 350}, // cross in opencl_std_100
    {IR(49664, 4), 368}, // ctz in opencl_std_100
    {IR(49439, 8), 342}, // degrees in opencl_std_100
    {IR(49510, 9), 351}, // distance in opencl_std_100
    {IR(48844, 4), 264}, // erf in opencl_std_100
    {IR(48839, 5), 263}, // erfc in opencl_std_100
    {IR(48848, 4), 265}, // exp in opencl_std_100
    {IR(48857, 6), 267}, // exp10 in opencl_std_100
    {IR(48852, 5), 266}, // exp2 in opencl_std_100
    {IR(48863, 6), 268}, // expm1 in opencl_std_100
    {IR(48869, 5), 269}, // fabs in opencl_std_100
    {IR(49536, 14), 354}, // fast_distance in opencl_std_100
    {IR(49550, 12), 355}, // fast_length in opencl_std_100
    {IR(49562, 15), 356}, // fast_normalize in opencl_std_100
    {IR(49432, 7), 341}, // fclamp in opencl_std_100
    {IR(48874, 5), 270}, // fdim in opencl_std_100
    {IR(48879, 6), 271}, // floor in opencl_std_100
    {IR(48885, 4), 272}, // fma in opencl_std_100
    {IR(48889, 5), 273}, // fmax in opencl_std_100
    {IR(49447, 12), 343}, // fmax_common in opencl_std_100
    {IR(48894, 5), 274}, // fmin in opencl_std_100
    {IR(49459, 12), 344}, // fmin_common in opencl_std_100
    {IR(48899, 5), 275}, // fmod in opencl_std_100
    {IR(48904, 6), 276}, // fract in opencl_std_100
    {IR(48910, 6), 277}, // frexp in opencl_std_100
    {IR(49122, 9), 313}, // half_cos in opencl_std_100
    {IR(49131, 12), 314}, // half_divide in opencl_std_100
    {IR(49143, 9), 315}, // half_exp in opencl_std_100
    {IR(49162, 11), 317}, // half_exp10 in opencl_std_100
    {IR(49152, 10), 316}, // half_exp2 in opencl_std_100
    {IR(49173, 9), 318}, // half_log in opencl_std_100
    {IR(49192, 11), 320}, // half_log10 in opencl_std_100
    {IR(49182, 10), 319}, // half_log2 in opencl_std_100
    {IR(49203, 10), 321}, // half_powr in opencl_std_100
    {IR(49213, 11), 322}, // half_recip in opencl_std_100
    {IR(49224, 11), 323}, // half_rsqrt in opencl_std_100
    {IR(49235, 9), 324}, // half_sin in opencl_std_100
    {IR(49244, 10), 325}, // half_sqrt in opencl_std_100
    {IR(49254, 9), 326}, // half_tan in opencl_std_100
    {IR(48916, 6), 278}, // hypot in opencl_std_100
    {IR(48922, 6), 279}, // ilogb in opencl_std_100
    {IR(48928, 6), 280}, // ldexp in opencl_std_100
    {IR(49519, 7), 352}, // length in opencl_std_100
    {IR(48934, 7), 281}, // lgamma in opencl_std_100
    {IR(48941, 9), 282}, // lgamma_r in opencl_std_100
    {IR(48950, 4), 283}, // log in opencl_std_100
    {IR(48959, 6), 285}, // log10 in opencl_std_100
    {IR(48965, 6), 286}, // log1p in opencl_std_100
    {IR(48954, 5), 284}, // log2 in opencl_std_100
    {IR(48971, 5), 287}, // logb in opencl_std_100
    {IR(48976, 4), 288}, // mad in opencl_std_100
    {IR(48980, 7), 289}, // maxmag in opencl_std_100
    {IR(48987, 7), 290}, // minmag in opencl_std_100
    {IR(49471, 4), 345}, // mix in opencl_std_100
    {IR(48994, 5), 291}, // modf in opencl_std_100
    {IR(48999, 4), 292}, // nan in opencl_std_100
    {IR(49263, 11), 327}, // native_cos in opencl_std_100
    {IR(49274, 14), 328}, // native_divide in opencl_std_100
    {IR(49288, 11), 329}, // native_exp in opencl_std_100
    {IR(49311, 13), 331}, // native_exp10 in opencl_std_100
    {IR(49299, 12), 330}, // native_exp2 in opencl_std_100
    {IR(49324, 11), 332}, // native_log in opencl_std_100
    {IR(49347, 13), 334}, // native_log10 in opencl_std_100
    {IR(49335, 12), 333}, // native_log2 in opencl_std_100
    {IR(49360, 12), 335}, // native_powr in opencl_std_100
    {IR(49372, 13), 336}, // native_recip in opencl_std_100
    {IR(49385, 13), 337}, // native_rsqrt in opencl_std_100
    {IR(49398, 11), 338}, // native_sin in opencl_std_100
    {IR(49409, 12), 339}, // native_sqrt in opencl_std_100
    {IR(49421, 11), 340}, // native_tan in opencl_std_100
    {IR(49003, 10), 293}, // nextafter in opencl_std_100
    {IR(49526, 10), 353}, // normalize in opencl_std_100
    {IR(49779, 9), 382}, // popcount in opencl_std_100
    {IR(49013, 4), 294}, // pow in opencl_std_100
    {IR(49017, 5), 295}, // pown in opencl_std_100
    {IR(49022, 5), 296}, // powr in opencl_std_100
    {IR(49979, 9), 401}, // prefetch in opencl_std_100
    {IR(49972, 7), 400}, // printf in opencl_std_100
    {IR(49475, 8), 346}, // radians in opencl_std_100
    {IR(49027, 10), 297}, // remainder in opencl_std_100
    {IR(49037, 7), 298}, // remquo in opencl_std_100
    {IR(49044, 5), 299}, // rint in opencl_std_100
    {IR(49049, 6), 300}, // rootn in opencl_std_100
    {IR(49730, 7), 377}, // rotate in opencl_std_100
    {IR(49055, 6), 301}, // round in opencl_std_100
    {IR(49061, 6), 302}, // rsqrt in opencl_std_100
    {IR(49577, 6), 357}, // s_abs in opencl_std_100
    {IR(49583, 11), 358}, // s_abs_diff in opencl_std_100
    {IR(49594, 10), 359}, // s_add_sat in opencl_std_100
    {IR(49644, 8), 365}, // s_clamp in opencl_std_100
    {IR(49614, 7), 361}, // s_hadd in opencl_std_100
    {IR(49788, 8), 383}, // s_mad24 in opencl_std_100
    {IR(49668, 9), 369}, // s_mad_hi in opencl_std_100
    {IR(49687, 10), 371}, // s_mad_sat in opencl_std_100
    {IR(49697, 6), 372}, // s_max in opencl_std_100
    {IR(49709, 6), 374}, // s_min in opencl_std_100
    {IR(49804, 8), 385}, // s_mul24 in opencl_std_100
    {IR(49721, 9), 376}, // s_mul_hi in opencl_std_100
    {IR(49628, 8), 363}, // s_rhadd in opencl_std_100
    {IR(49737, 10), 378}, // s_sub_sat in opencl_std_100
    {IR(49768, 11), 381}, // s_upsample in opencl_std_100
    {IR(49998, 7), 403}, // select in opencl_std_100
    {IR(49955, 8), 398}, // shuffle in opencl_std_100
    {IR(49963, 9), 399}, // shuffle2 in opencl_std_100
    {IR(49499, 5), 349}, // sign in opencl_std_100
    {IR(49067, 4), 303}, // sin in opencl_std_100
    {IR(49071, 7), 304}, // sincos in opencl_std_100
    {IR(49078, 5), 305}, // sinh in opencl_std_100
    {IR(49083, 6), 306}, // sinpi in opencl_std_100
    {IR(49488, 11), 348}, // smoothstep in opencl_std_100
    {IR(49089, 5), 307}, // sqrt in opencl_std_100
    {IR(49483, 5), 347}, // step in opencl_std_100
    {IR(49094, 4), 308}, // tan in opencl_std_100
    {IR(49098, 5), 309}, // tanh in opencl_std_100
    {IR(49103, 6), 310}, // tanpi in opencl_std_100
    {IR(49109, 7), 311}, // tgamma in opencl_std_100
    {IR(49116, 6), 312}, // trunc in opencl_std_100
    {IR(50005, 6), 404}, // u_abs in opencl_std_100
    {IR(50011, 11), 405}, // u_abs_diff in opencl_std_100
    {IR(49604, 10), 360}, // u_add_sat in opencl_std_100
    {IR(49652, 8), 366}, // u_clamp in opencl_std_100
    {IR(49621, 7), 362}, // u_hadd in opencl_std_100
    {IR(49796, 8), 384}, // u_mad24 in opencl_std_100
    {IR(50031, 9), 407}, // u_mad_hi in opencl_std_100
    {IR(49677, 10), 370}, // u_mad_sat in opencl_std_100
    {IR(49703, 6), 373}, // u_max in opencl_std_100
    {IR(49715, 6), 375}, // u_min in opencl_std_100
    {IR(49812, 8), 386}, // u_mul24 in opencl_std_100
    {IR(50022, 9), 406}, // u_mul_hi in opencl_std_100
    {IR(49636, 8), 364}, // u_rhadd in opencl_std_100
    {IR(49747, 10), 379}, // u_sub_sat in opencl_std_100
    {IR(49757, 11), 380}, // u_upsample in opencl_std_100
    {IR(49835, 11), 389}, // vload_half in opencl_std_100
    {IR(49846, 12), 390}, // vload_halfn in opencl_std_100
    {IR(49912, 13), 395}, // vloada_halfn in opencl_std_100
    {IR(49820, 7), 387}, // vloadn in opencl_std_100
    {IR(49858, 12), 391}, // vstore_half in opencl_std_100
    {IR(49870, 14), 392}, // vstore_half_r in opencl_std_100
    {IR(49884, 13), 393}, // vstore_halfn in opencl_std_100
    {IR(49897, 15), 394}, // vstore_halfn_r in opencl_std_100
    {IR(49925, 14), 396}, // vstorea_halfn in opencl_std_100
    {IR(49939, 16), 397}, // vstorea_halfn_r in opencl_std_100
    {IR(49827, 8), 388}, // vstoren in opencl_std_100
    {IR(50057, 17), 409}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(50040, 17), 408}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(50074, 8), 410}, // TimeAMD in spv_amd_gcn_shader
    {IR(50151, 9), 414}, // MbcntAMD in spv_amd_shader_ballot
    {IR(50082, 22), 411}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(50104, 28), 412}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(50132, 19), 413}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(50160, 23), 415}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(50210, 9), 419}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50237, 9), 422}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50183, 9), 416}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50228, 9), 421}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50255, 9), 424}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50201, 9), 418}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50219, 9), 420}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(50246, 9), 423}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(50192, 9), 417}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(50561, 4), 456}, // ABS in tosa_001000_1
    {IR(50384, 4), 439}, // ADD in tosa_001000_1
    {IR(50264, 7), 425}, // ARGMAX in tosa_001000_1
    {IR(50388, 23), 440}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(50271, 11), 426}, // AVG_POOL2D in tosa_001000_1
    {IR(50411, 12), 441}, // BITWISE_AND in tosa_001000_1
    {IR(50565, 12), 457}, // BITWISE_NOT in tosa_001000_1
    {IR(50423, 11), 442}, // BITWISE_OR in tosa_001000_1
    {IR(50434, 12), 443}, // BITWISE_XOR in tosa_001000_1
    {IR(50819, 5), 489}, // CAST in tosa_001000_1
    {IR(50577, 5), 458}, // CEIL in tosa_001000_1
    {IR(50361, 6), 435}, // CLAMP in tosa_001000_1
    {IR(50582, 4), 459}, // CLZ in tosa_001000_1
    {IR(50749, 7), 479}, // CONCAT in tosa_001000_1
    {IR(50282, 7), 427}, // CONV2D in tosa_001000_1
    {IR(50289, 7), 428}, // CONV3D in tosa_001000_1
    {IR(50586, 4), 460}, // COS in tosa_001000_1
    {IR(50296, 17), 429}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(50651, 6), 470}, // EQUAL in tosa_001000_1
    {IR(50367, 4), 436}, // ERF in tosa_001000_1
    {IR(50590, 4), 461}, // EXP in tosa_001000_1
    {IR(50313, 6), 430}, // FFT2D in tosa_001000_1
    {IR(50594, 6), 462}, // FLOOR in tosa_001000_1
    {IR(50797, 7), 486}, // GATHER in tosa_001000_1
    {IR(50657, 8), 471}, // GREATER in tosa_001000_1
    {IR(50665, 14), 472}, // GREATER_EQUAL in tosa_001000_1
    {IR(50446, 7), 444}, // INTDIV in tosa_001000_1
    {IR(50600, 4), 463}, // LOG in tosa_001000_1
    {IR(50453, 12), 445}, // LOGICAL_AND in tosa_001000_1
    {IR(50465, 19), 446}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(50604, 12), 464}, // LOGICAL_NOT in tosa_001000_1
    {IR(50504, 11), 448}, // LOGICAL_OR in tosa_001000_1
    {IR(50484, 20), 447}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(50515, 12), 449}, // LOGICAL_XOR in tosa_001000_1
    {IR(50319, 7), 431}, // MATMUL in tosa_001000_1
    {IR(50527, 8), 450}, // MAXIMUM in tosa_001000_1
    {IR(50326, 11), 432}, // MAX_POOL2D in tosa_001000_1
    {IR(50535, 8), 451}, // MINIMUM in tosa_001000_1
    {IR(50543, 4), 452}, // MUL in tosa_001000_1
    {IR(50616, 7), 465}, // NEGATE in tosa_001000_1
    {IR(50756, 4), 480}, // PAD in tosa_001000_1
    {IR(50547, 4), 453}, // POW in tosa_001000_1
    {IR(50623, 11), 466}, // RECIPROCAL in tosa_001000_1
    {IR(50679, 11), 473}, // REDUCE_ALL in tosa_001000_1
    {IR(50690, 11), 474}, // REDUCE_ANY in tosa_001000_1
    {IR(50701, 11), 475}, // REDUCE_MAX in tosa_001000_1
    {IR(50712, 11), 476}, // REDUCE_MIN in tosa_001000_1
    {IR(50723, 15), 477}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(50738, 11), 478}, // REDUCE_SUM in tosa_001000_1
    {IR(50824, 8), 490}, // RESCALE in tosa_001000_1
    {IR(50760, 8), 481}, // RESHAPE in tosa_001000_1
    {IR(50812, 7), 488}, // RESIZE in tosa_001000_1
    {IR(50768, 8), 482}, // REVERSE in tosa_001000_1
    {IR(50337, 7), 433}, // RFFT2D in tosa_001000_1
    {IR(50634, 6), 467}, // RSQRT in tosa_001000_1
    {IR(50804, 8), 487}, // SCATTER in tosa_001000_1
    {IR(50644, 7), 469}, // SELECT in tosa_001000_1
    {IR(50371, 8), 437}, // SIGMOID in tosa_001000_1
    {IR(50640, 4), 468}, // SIN in tosa_001000_1
    {IR(50776, 6), 483}, // SLICE in tosa_001000_1
    {IR(50551, 4), 454}, // SUB in tosa_001000_1
    {IR(50555, 6), 455}, // TABLE in tosa_001000_1
    {IR(50379, 5), 438}, // TANH in tosa_001000_1
    {IR(50782, 5), 484}, // TILE in tosa_001000_1
    {IR(50787, 10), 485}, // TRANSPOSE in tosa_001000_1
    {IR(50344, 17), 434}, // TRANSPOSE_CONV2D in tosa_001000_1
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
  "SPV_INTEL_function_variants\0" // IR(1872, 28)
  "SPV_INTEL_global_variable_fpga_decorations\0" // IR(1900, 43)
  "SPV_INTEL_global_variable_host_access\0" // IR(1943, 38)
  "SPV_INTEL_inline_assembly\0" // IR(1981, 26)
  "SPV_INTEL_int4\0" // IR(2007, 15)
  "SPV_INTEL_io_pipes\0" // IR(2022, 19)
  "SPV_INTEL_kernel_attributes\0" // IR(2041, 28)
  "SPV_INTEL_long_composites\0" // IR(2069, 26)
  "SPV_INTEL_loop_fuse\0" // IR(2095, 20)
  "SPV_INTEL_masked_gather_scatter\0" // IR(2115, 32)
  "SPV_INTEL_maximum_registers\0" // IR(2147, 28)
  "SPV_INTEL_media_block_io\0" // IR(2175, 25)
  "SPV_INTEL_memory_access_aliasing\0" // IR(2200, 33)
  "SPV_INTEL_optnone\0" // IR(2233, 18)
  "SPV_INTEL_runtime_aligned\0" // IR(2251, 26)
  "SPV_INTEL_shader_integer_functions2\0" // IR(2277, 36)
  "SPV_INTEL_split_barrier\0" // IR(2313, 24)
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(2337, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(2372, 46)
  "SPV_INTEL_subgroups\0" // IR(2418, 20)
  "SPV_INTEL_task_sequence\0" // IR(2438, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(2462, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(2498, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(2533, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(2570, 30)
  "SPV_INTEL_variable_length_array\0" // IR(2600, 32)
  "SPV_INTEL_vector_compute\0" // IR(2632, 25)
  "SPV_KHR_16bit_storage\0" // IR(2657, 22)
  "SPV_KHR_8bit_storage\0" // IR(2679, 21)
  "SPV_KHR_bfloat16\0" // IR(2700, 17)
  "SPV_KHR_bit_instructions\0" // IR(2717, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(2742, 35)
  "SPV_KHR_cooperative_matrix\0" // IR(2777, 27)
  "SPV_KHR_device_group\0" // IR(2804, 21)
  "SPV_KHR_expect_assume\0" // IR(2825, 22)
  "SPV_KHR_float_controls\0" // IR(2847, 23)
  "SPV_KHR_float_controls2\0" // IR(2870, 24)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(2894, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(2930, 30)
  "SPV_KHR_integer_dot_product\0" // IR(2960, 28)
  "SPV_KHR_linkonce_odr\0" // IR(2988, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(3009, 30)
  "SPV_KHR_multiview\0" // IR(3039, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(3057, 35)
  "SPV_KHR_non_semantic_info\0" // IR(3092, 26)
  "SPV_KHR_physical_storage_buffer\0" // IR(3118, 32)
  "SPV_KHR_post_depth_coverage\0" // IR(3150, 28)
  "SPV_KHR_quad_control\0" // IR(3178, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(3199, 22)
  "SPV_KHR_ray_query\0" // IR(3221, 18)
  "SPV_KHR_ray_tracing\0" // IR(3239, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(3259, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(3294, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(3331, 34)
  "SPV_KHR_shader_ballot\0" // IR(3365, 22)
  "SPV_KHR_shader_clock\0" // IR(3387, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(3408, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(3439, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(3476, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(3500, 38)
  "SPV_KHR_subgroup_vote\0" // IR(3538, 22)
  "SPV_KHR_terminate_invocation\0" // IR(3560, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(3589, 35)
  "SPV_KHR_untyped_pointers\0" // IR(3624, 25)
  "SPV_KHR_variable_pointers\0" // IR(3649, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(3675, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(3703, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(3744, 38)
  "SPV_NV_bindless_texture\0" // IR(3782, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(3806, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(3844, 34)
  "SPV_NV_cooperative_matrix\0" // IR(3878, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(3904, 27)
  "SPV_NV_cooperative_vector\0" // IR(3931, 26)
  "SPV_NV_displacement_micromap\0" // IR(3957, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(3986, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(4021, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(4056, 28)
  "SPV_NV_mesh_shader\0" // IR(4084, 19)
  "SPV_NV_raw_access_chains\0" // IR(4103, 25)
  "SPV_NV_ray_tracing\0" // IR(4128, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(4147, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(4178, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(4215, 33)
  "SPV_NV_shader_image_footprint\0" // IR(4248, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(4278, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(4311, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(4337, 35)
  "SPV_NV_shading_rate\0" // IR(4372, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(4392, 29)
  "SPV_NV_tensor_addressing\0" // IR(4421, 25)
  "SPV_NV_viewport_array2\0" // IR(4446, 23)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(4469, 39)
  "SPV_QCOM_image_processing\0" // IR(4508, 26)
  "SPV_QCOM_image_processing2\0" // IR(4534, 27)
  "SPV_QCOM_tile_shading\0" // IR(4561, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(4583, 38)
  "ReadOnly\0" // IR(4621, 9)
  "Kernel\0" // IR(4630, 7)
  "WriteOnly\0" // IR(4637, 10)
  "ReadWrite\0" // IR(4647, 10)
  "Logical\0" // IR(4657, 8)
  "Physical32\0" // IR(4665, 11)
  "Addresses\0" // IR(4676, 10)
  "Physical64\0" // IR(4686, 11)
  "PhysicalStorageBuffer64\0" // IR(4697, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(4721, 27)
  "PhysicalStorageBufferAddresses\0" // IR(4748, 31)
  "Position\0" // IR(4779, 9)
  "Shader\0" // IR(4788, 7)
  "PointSize\0" // IR(4795, 10)
  "ClipDistance\0" // IR(4805, 13)
  "CullDistance\0" // IR(4818, 13)
  "VertexId\0" // IR(4831, 9)
  "InstanceId\0" // IR(4840, 11)
  "PrimitiveId\0" // IR(4851, 12)
  "Geometry\0" // IR(4863, 9)
  "Tessellation\0" // IR(4872, 13)
  "RayTracingNV\0" // IR(4885, 13)
  "RayTracingKHR\0" // IR(4898, 14)
  "MeshShadingNV\0" // IR(4912, 14)
  "MeshShadingEXT\0" // IR(4926, 15)
  "InvocationId\0" // IR(4941, 13)
  "Layer\0" // IR(4954, 6)
  "ShaderLayer\0" // IR(4960, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(4972, 28)
  "ViewportIndex\0" // IR(5000, 14)
  "MultiViewport\0" // IR(5014, 14)
  "ShaderViewportIndex\0" // IR(5028, 20)
  "TessLevelOuter\0" // IR(5048, 15)
  "TessLevelInner\0" // IR(5063, 15)
  "TessCoord\0" // IR(5078, 10)
  "PatchVertices\0" // IR(5088, 14)
  "FragCoord\0" // IR(5102, 10)
  "PointCoord\0" // IR(5112, 11)
  "FrontFacing\0" // IR(5123, 12)
  "SampleId\0" // IR(5135, 9)
  "SampleRateShading\0" // IR(5144, 18)
  "SamplePosition\0" // IR(5162, 15)
  "SampleMask\0" // IR(5177, 11)
  "FragDepth\0" // IR(5188, 10)
  "HelperInvocation\0" // IR(5198, 17)
  "NumWorkgroups\0" // IR(5215, 14)
  "WorkgroupSize\0" // IR(5229, 14)
  "WorkgroupId\0" // IR(5243, 12)
  "LocalInvocationId\0" // IR(5255, 18)
  "GlobalInvocationId\0" // IR(5273, 19)
  "LocalInvocationIndex\0" // IR(5292, 21)
  "WorkDim\0" // IR(5313, 8)
  "GlobalSize\0" // IR(5321, 11)
  "EnqueuedWorkgroupSize\0" // IR(5332, 22)
  "GlobalOffset\0" // IR(5354, 13)
  "GlobalLinearId\0" // IR(5367, 15)
  "SubgroupSize\0" // IR(5382, 13)
  "GroupNonUniform\0" // IR(5395, 16)
  "SubgroupBallotKHR\0" // IR(5411, 18)
  "SubgroupMaxSize\0" // IR(5429, 16)
  "NumSubgroups\0" // IR(5445, 13)
  "NumEnqueuedSubgroups\0" // IR(5458, 21)
  "SubgroupId\0" // IR(5479, 11)
  "SubgroupLocalInvocationId\0" // IR(5490, 26)
  "VertexIndex\0" // IR(5516, 12)
  "InstanceIndex\0" // IR(5528, 14)
  "CoreIDARM\0" // IR(5542, 10)
  "CoreBuiltinsARM\0" // IR(5552, 16)
  "CoreCountARM\0" // IR(5568, 13)
  "CoreMaxIDARM\0" // IR(5581, 13)
  "WarpIDARM\0" // IR(5594, 10)
  "WarpMaxIDARM\0" // IR(5604, 13)
  "SubgroupEqMask\0" // IR(5617, 15)
  "SubgroupEqMaskKHR\0" // IR(5632, 18)
  "GroupNonUniformBallot\0" // IR(5650, 22)
  "SubgroupGeMask\0" // IR(5672, 15)
  "SubgroupGeMaskKHR\0" // IR(5687, 18)
  "SubgroupGtMask\0" // IR(5705, 15)
  "SubgroupGtMaskKHR\0" // IR(5720, 18)
  "SubgroupLeMask\0" // IR(5738, 15)
  "SubgroupLeMaskKHR\0" // IR(5753, 18)
  "SubgroupLtMask\0" // IR(5771, 15)
  "SubgroupLtMaskKHR\0" // IR(5786, 18)
  "BaseVertex\0" // IR(5804, 11)
  "DrawParameters\0" // IR(5815, 15)
  "BaseInstance\0" // IR(5830, 13)
  "DrawIndex\0" // IR(5843, 10)
  "PrimitiveShadingRateKHR\0" // IR(5853, 24)
  "FragmentShadingRateKHR\0" // IR(5877, 23)
  "DeviceIndex\0" // IR(5900, 12)
  "DeviceGroup\0" // IR(5912, 12)
  "ViewIndex\0" // IR(5924, 10)
  "MultiView\0" // IR(5934, 10)
  "ShadingRateKHR\0" // IR(5944, 15)
  "TileOffsetQCOM\0" // IR(5959, 15)
  "TileShadingQCOM\0" // IR(5974, 16)
  "TileDimensionQCOM\0" // IR(5990, 18)
  "TileApronSizeQCOM\0" // IR(6008, 18)
  "BaryCoordNoPerspAMD\0" // IR(6026, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(6046, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(6074, 26)
  "BaryCoordSmoothAMD\0" // IR(6100, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(6119, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(6146, 25)
  "BaryCoordPullModelAMD\0" // IR(6171, 22)
  "FragStencilRefEXT\0" // IR(6193, 18)
  "StencilExportEXT\0" // IR(6211, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(6228, 29)
  "ShaderEnqueueAMDX\0" // IR(6257, 18)
  "ShaderIndexAMDX\0" // IR(6275, 16)
  "ViewportMaskNV\0" // IR(6291, 15)
  "ShaderViewportMaskNV\0" // IR(6306, 21)
  "SecondaryPositionNV\0" // IR(6327, 20)
  "ShaderStereoViewNV\0" // IR(6347, 19)
  "SecondaryViewportMaskNV\0" // IR(6366, 24)
  "PositionPerViewNV\0" // IR(6390, 18)
  "PerViewAttributesNV\0" // IR(6408, 20)
  "ViewportMaskPerViewNV\0" // IR(6428, 22)
  "FullyCoveredEXT\0" // IR(6450, 16)
  "FragmentFullyCoveredEXT\0" // IR(6466, 24)
  "TaskCountNV\0" // IR(6490, 12)
  "PrimitiveCountNV\0" // IR(6502, 17)
  "PrimitiveIndicesNV\0" // IR(6519, 19)
  "ClipDistancePerViewNV\0" // IR(6538, 22)
  "CullDistancePerViewNV\0" // IR(6560, 22)
  "LayerPerViewNV\0" // IR(6582, 15)
  "MeshViewCountNV\0" // IR(6597, 16)
  "MeshViewIndicesNV\0" // IR(6613, 18)
  "BaryCoordKHR\0" // IR(6631, 13)
  "BaryCoordNV\0" // IR(6644, 12)
  "FragmentBarycentricKHR\0" // IR(6656, 23)
  "BaryCoordNoPerspKHR\0" // IR(6679, 20)
  "BaryCoordNoPerspNV\0" // IR(6699, 19)
  "FragSizeEXT\0" // IR(6718, 12)
  "FragmentSizeNV\0" // IR(6730, 15)
  "FragmentDensityEXT\0" // IR(6745, 19)
  "FragInvocationCountEXT\0" // IR(6764, 23)
  "InvocationsPerPixelNV\0" // IR(6787, 22)
  "PrimitivePointIndicesEXT\0" // IR(6809, 25)
  "PrimitiveLineIndicesEXT\0" // IR(6834, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(6858, 28)
  "CullPrimitiveEXT\0" // IR(6886, 17)
  "LaunchIdKHR\0" // IR(6903, 12)
  "LaunchIdNV\0" // IR(6915, 11)
  "LaunchSizeKHR\0" // IR(6926, 14)
  "LaunchSizeNV\0" // IR(6940, 13)
  "WorldRayOriginKHR\0" // IR(6953, 18)
  "WorldRayOriginNV\0" // IR(6971, 17)
  "WorldRayDirectionKHR\0" // IR(6988, 21)
  "WorldRayDirectionNV\0" // IR(7009, 20)
  "ObjectRayOriginKHR\0" // IR(7029, 19)
  "ObjectRayOriginNV\0" // IR(7048, 18)
  "ObjectRayDirectionKHR\0" // IR(7066, 22)
  "ObjectRayDirectionNV\0" // IR(7088, 21)
  "RayTminKHR\0" // IR(7109, 11)
  "RayTminNV\0" // IR(7120, 10)
  "RayTmaxKHR\0" // IR(7130, 11)
  "RayTmaxNV\0" // IR(7141, 10)
  "InstanceCustomIndexKHR\0" // IR(7151, 23)
  "InstanceCustomIndexNV\0" // IR(7174, 22)
  "ObjectToWorldKHR\0" // IR(7196, 17)
  "ObjectToWorldNV\0" // IR(7213, 16)
  "WorldToObjectKHR\0" // IR(7229, 17)
  "WorldToObjectNV\0" // IR(7246, 16)
  "HitTNV\0" // IR(7262, 7)
  "HitKindKHR\0" // IR(7269, 11)
  "HitKindNV\0" // IR(7280, 10)
  "CurrentRayTimeNV\0" // IR(7290, 17)
  "RayTracingMotionBlurNV\0" // IR(7307, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(7330, 30)
  "RayTracingPositionFetchKHR\0" // IR(7360, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(7387, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(7421, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(7454, 37)
  "IncomingRayFlagsKHR\0" // IR(7491, 20)
  "IncomingRayFlagsNV\0" // IR(7511, 19)
  "RayGeometryIndexKHR\0" // IR(7530, 20)
  "HitIsSphereNV\0" // IR(7550, 14)
  "RayTracingSpheresGeometryNV\0" // IR(7564, 28)
  "HitIsLSSNV\0" // IR(7592, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(7603, 39)
  "HitSpherePositionNV\0" // IR(7642, 20)
  "WarpsPerSMNV\0" // IR(7662, 13)
  "ShaderSMBuiltinsNV\0" // IR(7675, 19)
  "SMCountNV\0" // IR(7694, 10)
  "WarpIDNV\0" // IR(7704, 9)
  "SMIDNV\0" // IR(7713, 7)
  "HitLSSPositionsNV\0" // IR(7720, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(7738, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(7772, 33)
  "HitSphereRadiusNV\0" // IR(7805, 18)
  "HitLSSRadiiNV\0" // IR(7823, 14)
  "ClusterIDNV\0" // IR(7837, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(7849, 41)
  "CullMaskKHR\0" // IR(7890, 12)
  "RayCullMaskKHR\0" // IR(7902, 15)
  "Matrix\0" // IR(7917, 7)
  "Linkage\0" // IR(7924, 8)
  "Vector16\0" // IR(7932, 9)
  "Float16Buffer\0" // IR(7941, 14)
  "Float16\0" // IR(7955, 8)
  "Float64\0" // IR(7963, 8)
  "Int64\0" // IR(7971, 6)
  "Int64Atomics\0" // IR(7977, 13)
  "ImageBasic\0" // IR(7990, 11)
  "ImageReadWrite\0" // IR(8001, 15)
  "ImageMipmap\0" // IR(8016, 12)
  "Pipes\0" // IR(8028, 6)
  "Groups\0" // IR(8034, 7)
  "DeviceEnqueue\0" // IR(8041, 14)
  "LiteralSampler\0" // IR(8055, 15)
  "AtomicStorage\0" // IR(8070, 14)
  "Int16\0" // IR(8084, 6)
  "TessellationPointSize\0" // IR(8090, 22)
  "GeometryPointSize\0" // IR(8112, 18)
  "ImageGatherExtended\0" // IR(8130, 20)
  "StorageImageMultisample\0" // IR(8150, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(8174, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(8208, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(8241, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(8275, 33)
  "ImageCubeArray\0" // IR(8308, 15)
  "SampledCubeArray\0" // IR(8323, 17)
  "ImageRect\0" // IR(8340, 10)
  "SampledRect\0" // IR(8350, 12)
  "GenericPointer\0" // IR(8362, 15)
  "Int8\0" // IR(8377, 5)
  "InputAttachment\0" // IR(8382, 16)
  "SparseResidency\0" // IR(8398, 16)
  "MinLod\0" // IR(8414, 7)
  "Sampled1D\0" // IR(8421, 10)
  "Image1D\0" // IR(8431, 8)
  "SampledBuffer\0" // IR(8439, 14)
  "ImageBuffer\0" // IR(8453, 12)
  "ImageMSArray\0" // IR(8465, 13)
  "StorageImageExtendedFormats\0" // IR(8478, 28)
  "ImageQuery\0" // IR(8506, 11)
  "DerivativeControl\0" // IR(8517, 18)
  "InterpolationFunction\0" // IR(8535, 22)
  "TransformFeedback\0" // IR(8557, 18)
  "GeometryStreams\0" // IR(8575, 16)
  "StorageImageReadWithoutFormat\0" // IR(8591, 30)
  "StorageImageWriteWithoutFormat\0" // IR(8621, 31)
  "SubgroupDispatch\0" // IR(8652, 17)
  "NamedBarrier\0" // IR(8669, 13)
  "PipeStorage\0" // IR(8682, 12)
  "GroupNonUniformVote\0" // IR(8694, 20)
  "GroupNonUniformArithmetic\0" // IR(8714, 26)
  "GroupNonUniformShuffle\0" // IR(8740, 23)
  "GroupNonUniformShuffleRelative\0" // IR(8763, 31)
  "GroupNonUniformClustered\0" // IR(8794, 25)
  "GroupNonUniformQuad\0" // IR(8819, 20)
  "UniformDecoration\0" // IR(8839, 18)
  "TileImageColorReadAccessEXT\0" // IR(8857, 28)
  "TileImageDepthReadAccessEXT\0" // IR(8885, 28)
  "TileImageStencilReadAccessEXT\0" // IR(8913, 30)
  "TensorsARM\0" // IR(8943, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(8954, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(8991, 40)
  "GraphARM\0" // IR(9031, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(9040, 28)
  "Float8EXT\0" // IR(9068, 10)
  "Float8CooperativeMatrixEXT\0" // IR(9078, 27)
  "CooperativeMatrixKHR\0" // IR(9105, 21)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(9126, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(9159, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(9202, 44)
  "SubgroupVoteKHR\0" // IR(9246, 16)
  "StorageBuffer16BitAccess\0" // IR(9262, 25)
  "StorageUniformBufferBlock16\0" // IR(9287, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(9315, 35)
  "StorageUniform16\0" // IR(9350, 17)
  "StoragePushConstant16\0" // IR(9367, 22)
  "StorageInputOutput16\0" // IR(9389, 21)
  "VariablePointersStorageBuffer\0" // IR(9410, 30)
  "VariablePointers\0" // IR(9440, 17)
  "AtomicStorageOps\0" // IR(9457, 17)
  "SampleMaskPostDepthCoverage\0" // IR(9474, 28)
  "StorageBuffer8BitAccess\0" // IR(9502, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(9526, 34)
  "StoragePushConstant8\0" // IR(9560, 21)
  "DenormPreserve\0" // IR(9581, 15)
  "DenormFlushToZero\0" // IR(9596, 18)
  "SignedZeroInfNanPreserve\0" // IR(9614, 25)
  "RoundingModeRTE\0" // IR(9639, 16)
  "RoundingModeRTZ\0" // IR(9655, 16)
  "RayQueryProvisionalKHR\0" // IR(9671, 23)
  "RayQueryKHR\0" // IR(9694, 12)
  "UntypedPointersKHR\0" // IR(9706, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(9725, 32)
  "TextureSampleWeightedQCOM\0" // IR(9757, 26)
  "TextureBoxFilterQCOM\0" // IR(9783, 21)
  "TextureBlockMatchQCOM\0" // IR(9804, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(9826, 32)
  "TextureBlockMatch2QCOM\0" // IR(9858, 23)
  "Float16ImageAMD\0" // IR(9881, 16)
  "ImageGatherBiasLodAMD\0" // IR(9897, 22)
  "FragmentMaskAMD\0" // IR(9919, 16)
  "ImageReadWriteLodAMD\0" // IR(9935, 21)
  "Int64ImageEXT\0" // IR(9956, 14)
  "ShaderClockKHR\0" // IR(9970, 15)
  "QuadControlKHR\0" // IR(9985, 15)
  "Int4TypeINTEL\0" // IR(10000, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(10014, 27)
  "BFloat16TypeKHR\0" // IR(10041, 16)
  "BFloat16DotProductKHR\0" // IR(10057, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(10079, 29)
  "SampleMaskOverrideCoverageNV\0" // IR(10108, 29)
  "GeometryShaderPassthroughNV\0" // IR(10137, 28)
  "ShaderViewportIndexLayerNV\0" // IR(10165, 27)
  "ImageFootprintNV\0" // IR(10192, 17)
  "FragmentBarycentricNV\0" // IR(10209, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(10231, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(10262, 30)
  "ShadingRateNV\0" // IR(10292, 14)
  "GroupNonUniformPartitionedNV\0" // IR(10306, 29)
  "ShaderNonUniform\0" // IR(10335, 17)
  "ShaderNonUniformEXT\0" // IR(10352, 20)
  "RuntimeDescriptorArray\0" // IR(10372, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(10395, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(10421, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(10457, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(10496, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(10535, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(10577, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(10616, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(10658, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(10695, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(10735, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(10771, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(10810, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(10847, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(10887, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(10923, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(10962, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(11001, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(11043, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(11085, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(11130, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(11172, 45)
  "VulkanMemoryModel\0" // IR(11217, 18)
  "VulkanMemoryModelKHR\0" // IR(11235, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(11256, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(11285, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(11317, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(11351, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(11383, 31)
  "RayTracingProvisionalKHR\0" // IR(11414, 25)
  "CooperativeMatrixNV\0" // IR(11439, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(11459, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(11492, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(11530, 32)
  "DemoteToHelperInvocation\0" // IR(11562, 25)
  "DemoteToHelperInvocationEXT\0" // IR(11587, 28)
  "DisplacementMicromapNV\0" // IR(11615, 23)
  "RayTracingOpacityMicromapEXT\0" // IR(11638, 29)
  "ShaderInvocationReorderNV\0" // IR(11667, 26)
  "BindlessTextureNV\0" // IR(11693, 18)
  "RayQueryPositionFetchKHR\0" // IR(11711, 25)
  "CooperativeVectorNV\0" // IR(11736, 20)
  "AtomicFloat16VectorNV\0" // IR(11756, 22)
  "RawAccessChainsNV\0" // IR(11778, 18)
  "CooperativeMatrixReductionsNV\0" // IR(11796, 30)
  "CooperativeMatrixConversionsNV\0" // IR(11826, 31)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(11857, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(11897, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(11933, 30)
  "CooperativeVectorTrainingNV\0" // IR(11963, 28)
  "TensorAddressingNV\0" // IR(11991, 19)
  "SubgroupShuffleINTEL\0" // IR(12010, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(12031, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(12058, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(12084, 31)
  "RoundToInfinityINTEL\0" // IR(12115, 21)
  "FloatingPointModeINTEL\0" // IR(12136, 23)
  "IntegerFunctions2INTEL\0" // IR(12159, 23)
  "FunctionPointersINTEL\0" // IR(12182, 22)
  "IndirectReferencesINTEL\0" // IR(12204, 24)
  "AsmINTEL\0" // IR(12228, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(12237, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(12260, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(12283, 23)
  "VectorComputeINTEL\0" // IR(12306, 19)
  "VectorAnyINTEL\0" // IR(12325, 15)
  "ExpectAssumeKHR\0" // IR(12340, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(12356, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(12389, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(12427, 39)
  "VariableLengthArrayINTEL\0" // IR(12466, 25)
  "FunctionFloatControlINTEL\0" // IR(12491, 26)
  "FPGAMemoryAttributesINTEL\0" // IR(12517, 26)
  "FPFastMathModeINTEL\0" // IR(12543, 20)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(12563, 32)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(12595, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(12632, 30)
  "FPGALoopControlsINTEL\0" // IR(12662, 22)
  "KernelAttributesINTEL\0" // IR(12684, 22)
  "FPGAKernelAttributesINTEL\0" // IR(12706, 26)
  "FPGAMemoryAccessesINTEL\0" // IR(12732, 24)
  "FPGAClusterAttributesINTEL\0" // IR(12756, 27)
  "LoopFuseINTEL\0" // IR(12783, 14)
  "FPGADSPControlINTEL\0" // IR(12797, 20)
  "MemoryAccessAliasingINTEL\0" // IR(12817, 26)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(12843, 40)
  "FPGABufferLocationINTEL\0" // IR(12883, 24)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(12907, 34)
  "USMStorageClassesINTEL\0" // IR(12941, 23)
  "RuntimeAlignedAttributeINTEL\0" // IR(12964, 29)
  "IOPipesINTEL\0" // IR(12993, 13)
  "BlockingPipesINTEL\0" // IR(13006, 19)
  "FPGARegINTEL\0" // IR(13025, 13)
  "DotProductInputAll\0" // IR(13038, 19)
  "DotProductInputAllKHR\0" // IR(13057, 22)
  "DotProductInput4x8Bit\0" // IR(13079, 22)
  "DotProductInput4x8BitKHR\0" // IR(13101, 25)
  "DotProductInput4x8BitPacked\0" // IR(13126, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(13154, 31)
  "DotProduct\0" // IR(13185, 11)
  "DotProductKHR\0" // IR(13196, 14)
  "ReplicatedCompositesEXT\0" // IR(13210, 24)
  "BitInstructions\0" // IR(13234, 16)
  "GroupNonUniformRotateKHR\0" // IR(13250, 25)
  "FloatControls2\0" // IR(13275, 15)
  "AtomicFloat32AddEXT\0" // IR(13290, 20)
  "AtomicFloat64AddEXT\0" // IR(13310, 20)
  "LongCompositesINTEL\0" // IR(13330, 20)
  "OptNoneEXT\0" // IR(13350, 11)
  "OptNoneINTEL\0" // IR(13361, 13)
  "AtomicFloat16AddEXT\0" // IR(13374, 20)
  "DebugInfoModuleINTEL\0" // IR(13394, 21)
  "BFloat16ConversionINTEL\0" // IR(13415, 24)
  "SplitBarrierINTEL\0" // IR(13439, 18)
  "ArithmeticFenceEXT\0" // IR(13457, 19)
  "FPGAClusterAttributesV2INTEL\0" // IR(13476, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(13505, 28)
  "TaskSequenceINTEL\0" // IR(13533, 18)
  "FPMaxErrorINTEL\0" // IR(13551, 16)
  "FPGALatencyControlINTEL\0" // IR(13567, 24)
  "FPGAArgumentInterfacesINTEL\0" // IR(13591, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(13619, 30)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(13649, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(13684, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(13712, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(13735, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(13765, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(13795, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(13833, 28)
  "SpecConditionalINTEL\0" // IR(13861, 21)
  "FunctionVariantsINTEL\0" // IR(13882, 22)
  "GroupUniformArithmeticKHR\0" // IR(13904, 26)
  "TensorFloat32RoundingINTEL\0" // IR(13930, 27)
  "MaskedGatherScatterINTEL\0" // IR(13957, 25)
  "CacheControlsINTEL\0" // IR(13982, 19)
  "RegisterLimitsINTEL\0" // IR(14001, 20)
  "BindlessImagesINTEL\0" // IR(14021, 20)
  "Unspecified\0" // IR(14041, 12)
  "Address\0" // IR(14053, 8)
  "Boolean\0" // IR(14061, 8)
  "Float\0" // IR(14069, 6)
  "Signed\0" // IR(14075, 7)
  "SignedChar\0" // IR(14082, 11)
  "Unsigned\0" // IR(14093, 9)
  "UnsignedChar\0" // IR(14102, 13)
  "Class\0" // IR(14115, 6)
  "Structure\0" // IR(14121, 10)
  "Union\0" // IR(14131, 6)
  "ImportedModule\0" // IR(14137, 15)
  "ImportedDeclaration\0" // IR(14152, 20)
  "None\0" // IR(14172, 5)
  "FlagIsProtected\0" // IR(14177, 16)
  "FlagIsPrivate\0" // IR(14193, 14)
  "FlagIsPublic\0" // IR(14207, 13)
  "FlagIsLocal\0" // IR(14220, 12)
  "FlagIsDefinition\0" // IR(14232, 17)
  "FlagFwdDecl\0" // IR(14249, 12)
  "FlagArtificial\0" // IR(14261, 15)
  "FlagExplicit\0" // IR(14276, 13)
  "FlagPrototyped\0" // IR(14289, 15)
  "FlagObjectPointer\0" // IR(14304, 18)
  "FlagStaticMember\0" // IR(14322, 17)
  "FlagIndirectVariable\0" // IR(14339, 21)
  "FlagLValueReference\0" // IR(14360, 20)
  "FlagRValueReference\0" // IR(14380, 20)
  "FlagIsOptimized\0" // IR(14400, 16)
  "FlagIsEnumClass\0" // IR(14416, 16)
  "FlagTypePassByValue\0" // IR(14432, 20)
  "FlagTypePassByReference\0" // IR(14452, 24)
  "Deref\0" // IR(14476, 6)
  "Plus\0" // IR(14482, 5)
  "Minus\0" // IR(14487, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(14493, 33)
  "PlusUconst\0" // IR(14526, 11)
  "BitPiece\0" // IR(14537, 9)
  "Swap\0" // IR(14546, 5)
  "Xderef\0" // IR(14551, 7)
  "StackValue\0" // IR(14558, 11)
  "Constu\0" // IR(14569, 7)
  "Fragment\0" // IR(14576, 9)
  "ConstType\0" // IR(14585, 10)
  "VolatileType\0" // IR(14595, 13)
  "RestrictType\0" // IR(14608, 13)
  "AtomicType\0" // IR(14621, 11)
  "Float16NV\0" // IR(14632, 10)
  "Float32NV\0" // IR(14642, 10)
  "Float64NV\0" // IR(14652, 10)
  "SignedInt8NV\0" // IR(14662, 13)
  "SignedInt16NV\0" // IR(14675, 14)
  "SignedInt32NV\0" // IR(14689, 14)
  "SignedInt64NV\0" // IR(14703, 14)
  "UnsignedInt8NV\0" // IR(14717, 15)
  "UnsignedInt16NV\0" // IR(14732, 16)
  "UnsignedInt32NV\0" // IR(14748, 16)
  "UnsignedInt64NV\0" // IR(14764, 16)
  "SignedInt8PackedNV\0" // IR(14780, 19)
  "UnsignedInt8PackedNV\0" // IR(14799, 21)
  "FloatE4M3NV\0" // IR(14820, 12)
  "FloatE5M2NV\0" // IR(14832, 12)
  "RowMajorKHR\0" // IR(14844, 12)
  "ColumnMajorKHR\0" // IR(14856, 15)
  "RowBlockedInterleavedARM\0" // IR(14871, 25)
  "ColumnBlockedInterleavedARM\0" // IR(14896, 28)
  "NoneKHR\0" // IR(14924, 8)
  "MatrixASignedComponentsKHR\0" // IR(14932, 27)
  "MatrixBSignedComponentsKHR\0" // IR(14959, 27)
  "MatrixCSignedComponentsKHR\0" // IR(14986, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(15013, 32)
  "SaturatingAccumulationKHR\0" // IR(15045, 26)
  "Row\0" // IR(15071, 4)
  "Column\0" // IR(15075, 7)
  "2x2\0" // IR(15082, 4)
  "MatrixAKHR\0" // IR(15086, 11)
  "MatrixBKHR\0" // IR(15097, 11)
  "MatrixAccumulatorKHR\0" // IR(15108, 21)
  "RowMajorNV\0" // IR(15129, 11)
  "ColumnMajorNV\0" // IR(15140, 14)
  "InferencingOptimalNV\0" // IR(15154, 21)
  "TrainingOptimalNV\0" // IR(15175, 18)
  "RelaxedPrecision\0" // IR(15193, 17)
  "SpecId\0" // IR(15210, 7)
  "Block\0" // IR(15217, 6)
  "BufferBlock\0" // IR(15223, 12)
  "RowMajor\0" // IR(15235, 9)
  "ColMajor\0" // IR(15244, 9)
  "ArrayStride\0" // IR(15253, 12)
  "MatrixStride\0" // IR(15265, 13)
  "GLSLShared\0" // IR(15278, 11)
  "GLSLPacked\0" // IR(15289, 11)
  "CPacked\0" // IR(15300, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(15308, 26)
  "BuiltIn\0" // IR(15334, 8)
  "NoPerspective\0" // IR(15342, 14)
  "Flat\0" // IR(15356, 5)
  "Patch\0" // IR(15361, 6)
  "Centroid\0" // IR(15367, 9)
  "Sample\0" // IR(15376, 7)
  "Invariant\0" // IR(15383, 10)
  "Restrict\0" // IR(15393, 9)
  "Aliased\0" // IR(15402, 8)
  "Volatile\0" // IR(15410, 9)
  "Constant\0" // IR(15419, 9)
  "Coherent\0" // IR(15428, 9)
  "NonWritable\0" // IR(15437, 12)
  "NonReadable\0" // IR(15449, 12)
  "Uniform\0" // IR(15461, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(15469, 26)
  "UniformId\0" // IR(15495, 10)
  "SaturatedConversion\0" // IR(15505, 20)
  "Stream\0" // IR(15525, 7)
  "Location\0" // IR(15532, 9)
  "Component\0" // IR(15541, 10)
  "Index\0" // IR(15551, 6)
  "Binding\0" // IR(15557, 8)
  "DescriptorSet\0" // IR(15565, 14)
  "Offset\0" // IR(15579, 7)
  "XfbBuffer\0" // IR(15586, 10)
  "XfbStride\0" // IR(15596, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(15606, 46)
  "FuncParamAttr\0" // IR(15652, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(15666, 34)
  "FPRoundingMode\0" // IR(15700, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(15715, 35)
  "FPFastMathMode\0" // IR(15750, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(15765, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(15797, 30)
  "LinkageAttributes\0" // IR(15827, 18)
  "NoContraction\0" // IR(15845, 14)
  "InputAttachmentIndex\0" // IR(15859, 21)
  "Alignment\0" // IR(15880, 10)
  "MaxByteOffset\0" // IR(15890, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(15904, 20)
  "AlignmentId\0" // IR(15924, 12)
  "MaxByteOffsetId\0" // IR(15936, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(15952, 44)
  "NoSignedWrap\0" // IR(15996, 13)
  "NoUnsignedWrap\0" // IR(16009, 15)
  "WeightTextureQCOM\0" // IR(16024, 18)
  "BlockMatchTextureQCOM\0" // IR(16042, 22)
  "BlockMatchSamplerQCOM\0" // IR(16064, 22)
  "ExplicitInterpAMD\0" // IR(16086, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(16104, 32)
  "NodeMaxPayloadsAMDX\0" // IR(16136, 20)
  "TrackFinishWritingAMDX\0" // IR(16156, 23)
  "PayloadNodeNameAMDX\0" // IR(16179, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(16199, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(16224, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(16251, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(16276, 28)
  "OverrideCoverageNV\0" // IR(16304, 19)
  "PassthroughNV\0" // IR(16323, 14)
  "ViewportRelativeNV\0" // IR(16337, 19)
  "SecondaryViewportRelativeNV\0" // IR(16356, 28)
  "PerPrimitiveEXT\0" // IR(16384, 16)
  "PerPrimitiveNV\0" // IR(16400, 15)
  "PerViewNV\0" // IR(16415, 10)
  "PerTaskNV\0" // IR(16425, 10)
  "PerVertexKHR\0" // IR(16435, 13)
  "PerVertexNV\0" // IR(16448, 12)
  "NonUniform\0" // IR(16460, 11)
  "NonUniformEXT\0" // IR(16471, 14)
  "RestrictPointer\0" // IR(16485, 16)
  "RestrictPointerEXT\0" // IR(16501, 19)
  "AliasedPointer\0" // IR(16520, 15)
  "AliasedPointerEXT\0" // IR(16535, 18)
  "HitObjectShaderRecordBufferNV\0" // IR(16553, 30)
  "BindlessSamplerNV\0" // IR(16583, 18)
  "BindlessImageNV\0" // IR(16601, 16)
  "BoundSamplerNV\0" // IR(16617, 15)
  "BoundImageNV\0" // IR(16632, 13)
  "SIMTCallINTEL\0" // IR(16645, 14)
  "ReferencedIndirectlyINTEL\0" // IR(16659, 26)
  "ClobberINTEL\0" // IR(16685, 13)
  "SideEffectsINTEL\0" // IR(16698, 17)
  "VectorComputeVariableINTEL\0" // IR(16715, 27)
  "FuncParamIOKindINTEL\0" // IR(16742, 21)
  "VectorComputeFunctionINTEL\0" // IR(16763, 27)
  "StackCallINTEL\0" // IR(16790, 15)
  "GlobalVariableOffsetINTEL\0" // IR(16805, 26)
  "CounterBuffer\0" // IR(16831, 14)
  "HlslCounterBufferGOOGLE\0" // IR(16845, 24)
  "UserSemantic\0" // IR(16869, 13)
  "HlslSemanticGOOGLE\0" // IR(16882, 19)
  "UserTypeGOOGLE\0" // IR(16901, 15)
  "FunctionRoundingModeINTEL\0" // IR(16916, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(16942, 31)
  "FunctionDenormModeINTEL\0" // IR(16973, 24)
  "RegisterINTEL\0" // IR(16997, 14)
  "MemoryINTEL\0" // IR(17011, 12)
  "NumbanksINTEL\0" // IR(17023, 14)
  "BankwidthINTEL\0" // IR(17037, 15)
  "MaxPrivateCopiesINTEL\0" // IR(17052, 22)
  "SinglepumpINTEL\0" // IR(17074, 16)
  "DoublepumpINTEL\0" // IR(17090, 16)
  "MaxReplicatesINTEL\0" // IR(17106, 19)
  "SimpleDualPortINTEL\0" // IR(17125, 20)
  "MergeINTEL\0" // IR(17145, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(17156, 42)
  "BankBitsINTEL\0" // IR(17198, 14)
  "ForcePow2DepthINTEL\0" // IR(17212, 20)
  "StridesizeINTEL\0" // IR(17232, 16)
  "WordsizeINTEL\0" // IR(17248, 14)
  "TrueDualPortINTEL\0" // IR(17262, 18)
  "BurstCoalesceINTEL\0" // IR(17280, 19)
  "CacheSizeINTEL\0" // IR(17299, 15)
  "DontStaticallyCoalesceINTEL\0" // IR(17314, 28)
  "PrefetchINTEL\0" // IR(17342, 14)
  "StallEnableINTEL\0" // IR(17356, 17)
  "FuseLoopsInFunctionINTEL\0" // IR(17373, 25)
  "MathOpDSPModeINTEL\0" // IR(17398, 19)
  "AliasScopeINTEL\0" // IR(17417, 16)
  "NoAliasINTEL\0" // IR(17433, 13)
  "InitiationIntervalINTEL\0" // IR(17446, 24)
  "MaxConcurrencyINTEL\0" // IR(17470, 20)
  "PipelineEnableINTEL\0" // IR(17490, 20)
  "BufferLocationINTEL\0" // IR(17510, 20)
  "IOPipeStorageINTEL\0" // IR(17530, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(17549, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(17583, 31)
  "SingleElementVectorINTEL\0" // IR(17614, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(17639, 35)
  "MediaBlockIOINTEL\0" // IR(17674, 18)
  "StallFreeINTEL\0" // IR(17692, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(17707, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(17738, 26)
  "LatencyControlLabelINTEL\0" // IR(17764, 25)
  "LatencyControlConstraintINTEL\0" // IR(17789, 30)
  "ConduitKernelArgumentINTEL\0" // IR(17819, 27)
  "RegisterMapKernelArgumentINTEL\0" // IR(17846, 31)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(17877, 33)
  "MMHostInterfaceDataWidthINTEL\0" // IR(17910, 30)
  "MMHostInterfaceLatencyINTEL\0" // IR(17940, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(17968, 34)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(18002, 34)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(18036, 29)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(18065, 32)
  "StableKernelArgumentINTEL\0" // IR(18097, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(18123, 39)
  "HostAccessINTEL\0" // IR(18162, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(18178, 47)
  "InitModeINTEL\0" // IR(18225, 14)
  "ImplementInRegisterMapINTEL\0" // IR(18239, 28)
  "ConditionalINTEL\0" // IR(18267, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(18284, 36)
  "CacheControlLoadINTEL\0" // IR(18320, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(18342, 37)
  "CacheControlStoreINTEL\0" // IR(18379, 23)
  "1D\0" // IR(18402, 3)
  "2D\0" // IR(18405, 3)
  "3D\0" // IR(18408, 3)
  "Cube\0" // IR(18411, 5)
  "Rect\0" // IR(18416, 5)
  "Buffer\0" // IR(18421, 7)
  "SubpassData\0" // IR(18428, 12)
  "TileImageDataEXT\0" // IR(18440, 17)
  "Invocations\0" // IR(18457, 12)
  "SpacingEqual\0" // IR(18469, 13)
  "SpacingFractionalEven\0" // IR(18482, 22)
  "SpacingFractionalOdd\0" // IR(18504, 21)
  "VertexOrderCw\0" // IR(18525, 14)
  "VertexOrderCcw\0" // IR(18539, 15)
  "PixelCenterInteger\0" // IR(18554, 19)
  "OriginUpperLeft\0" // IR(18573, 16)
  "OriginLowerLeft\0" // IR(18589, 16)
  "EarlyFragmentTests\0" // IR(18605, 19)
  "PointMode\0" // IR(18624, 10)
  "Xfb\0" // IR(18634, 4)
  "DepthReplacing\0" // IR(18638, 15)
  "DepthGreater\0" // IR(18653, 13)
  "DepthLess\0" // IR(18666, 10)
  "DepthUnchanged\0" // IR(18676, 15)
  "LocalSize\0" // IR(18691, 10)
  "LocalSizeHint\0" // IR(18701, 14)
  "InputPoints\0" // IR(18715, 12)
  "InputLines\0" // IR(18727, 11)
  "InputLinesAdjacency\0" // IR(18738, 20)
  "Triangles\0" // IR(18758, 10)
  "InputTrianglesAdjacency\0" // IR(18768, 24)
  "Quads\0" // IR(18792, 6)
  "Isolines\0" // IR(18798, 9)
  "OutputVertices\0" // IR(18807, 15)
  "OutputPoints\0" // IR(18822, 13)
  "OutputLineStrip\0" // IR(18835, 16)
  "OutputTriangleStrip\0" // IR(18851, 20)
  "VecTypeHint\0" // IR(18871, 12)
  "ContractionOff\0" // IR(18883, 15)
  "Initializer\0" // IR(18898, 12)
  "Finalizer\0" // IR(18910, 10)
  "SubgroupsPerWorkgroup\0" // IR(18920, 22)
  "SubgroupsPerWorkgroupId\0" // IR(18942, 24)
  "LocalSizeId\0" // IR(18966, 12)
  "LocalSizeHintId\0" // IR(18978, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(18994, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(19028, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(19062, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(19098, 30)
  "PostDepthCoverage\0" // IR(19128, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(19146, 34)
  "TileShadingRateQCOM\0" // IR(19180, 20)
  "EarlyAndLateFragmentTestsAMD\0" // IR(19200, 29)
  "StencilRefReplacingEXT\0" // IR(19229, 23)
  "CoalescingAMDX\0" // IR(19252, 15)
  "IsApiEntryAMDX\0" // IR(19267, 15)
  "MaxNodeRecursionAMDX\0" // IR(19282, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(19303, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(19327, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(19348, 28)
  "StencilRefGreaterFrontAMD\0" // IR(19376, 26)
  "StencilRefLessFrontAMD\0" // IR(19402, 23)
  "StencilRefUnchangedBackAMD\0" // IR(19425, 27)
  "StencilRefGreaterBackAMD\0" // IR(19452, 25)
  "StencilRefLessBackAMD\0" // IR(19477, 22)
  "QuadDerivativesKHR\0" // IR(19499, 19)
  "RequireFullQuadsKHR\0" // IR(19518, 20)
  "SharesInputWithAMDX\0" // IR(19538, 20)
  "OutputLinesEXT\0" // IR(19558, 15)
  "OutputLinesNV\0" // IR(19573, 14)
  "OutputPrimitivesEXT\0" // IR(19587, 20)
  "OutputPrimitivesNV\0" // IR(19607, 19)
  "DerivativeGroupQuadsKHR\0" // IR(19626, 24)
  "DerivativeGroupQuadsNV\0" // IR(19650, 23)
  "DerivativeGroupLinearKHR\0" // IR(19673, 25)
  "DerivativeGroupLinearNV\0" // IR(19698, 24)
  "OutputTrianglesEXT\0" // IR(19722, 19)
  "OutputTrianglesNV\0" // IR(19741, 18)
  "PixelInterlockOrderedEXT\0" // IR(19759, 25)
  "PixelInterlockUnorderedEXT\0" // IR(19784, 27)
  "SampleInterlockOrderedEXT\0" // IR(19811, 26)
  "SampleInterlockUnorderedEXT\0" // IR(19837, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(19865, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(19896, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(19929, 27)
  "RoundingModeRTPINTEL\0" // IR(19956, 21)
  "RoundingModeRTNINTEL\0" // IR(19977, 21)
  "FloatingPointModeALTINTEL\0" // IR(19998, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(20024, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(20051, 22)
  "MaxWorkDimINTEL\0" // IR(20073, 16)
  "NoGlobalOffsetINTEL\0" // IR(20089, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(20109, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(20131, 28)
  "MaximallyReconvergesKHR\0" // IR(20159, 24)
  "FPFastMathDefault\0" // IR(20183, 18)
  "StreamingInterfaceINTEL\0" // IR(20201, 24)
  "RegisterMapInterfaceINTEL\0" // IR(20225, 26)
  "NamedBarrierCountINTEL\0" // IR(20251, 23)
  "MaximumRegistersINTEL\0" // IR(20274, 22)
  "MaximumRegistersIdINTEL\0" // IR(20296, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(20320, 51)
  "NamedMaximumRegistersINTEL\0" // IR(20371, 27)
  "Vertex\0" // IR(20398, 7)
  "TessellationControl\0" // IR(20405, 20)
  "TessellationEvaluation\0" // IR(20425, 23)
  "GLCompute\0" // IR(20448, 10)
  "TaskNV\0" // IR(20458, 7)
  "MeshNV\0" // IR(20465, 7)
  "RayGenerationKHR\0" // IR(20472, 17)
  "RayGenerationNV\0" // IR(20489, 16)
  "IntersectionKHR\0" // IR(20505, 16)
  "IntersectionNV\0" // IR(20521, 15)
  "AnyHitKHR\0" // IR(20536, 10)
  "AnyHitNV\0" // IR(20546, 9)
  "ClosestHitKHR\0" // IR(20555, 14)
  "ClosestHitNV\0" // IR(20569, 13)
  "MissKHR\0" // IR(20582, 8)
  "MissNV\0" // IR(20590, 7)
  "CallableKHR\0" // IR(20597, 12)
  "CallableNV\0" // IR(20609, 11)
  "TaskEXT\0" // IR(20620, 8)
  "MeshEXT\0" // IR(20628, 8)
  "Preserve\0" // IR(20636, 9)
  "FlushToZero\0" // IR(20645, 12)
  "BFloat16KHR\0" // IR(20657, 12)
  "Float8E4M3EXT\0" // IR(20669, 14)
  "Float8E5M2EXT\0" // IR(20683, 14)
  "IEEE\0" // IR(20697, 5)
  "ALT\0" // IR(20702, 4)
  "NotNaN\0" // IR(20706, 7)
  "NotInf\0" // IR(20713, 7)
  "NSZ\0" // IR(20720, 4)
  "AllowRecip\0" // IR(20724, 11)
  "Fast\0" // IR(20735, 5)
  "AllowContract\0" // IR(20740, 14)
  "AllowContractFastINTEL\0" // IR(20754, 23)
  "AllowReassoc\0" // IR(20777, 13)
  "AllowReassocINTEL\0" // IR(20790, 18)
  "AllowTransform\0" // IR(20808, 15)
  "RTE\0" // IR(20823, 4)
  "RTZ\0" // IR(20827, 4)
  "RTP\0" // IR(20831, 4)
  "RTN\0" // IR(20835, 4)
  "Vertical2Pixels\0" // IR(20839, 16)
  "Vertical4Pixels\0" // IR(20855, 16)
  "Horizontal2Pixels\0" // IR(20871, 18)
  "Horizontal4Pixels\0" // IR(20889, 18)
  "Inline\0" // IR(20907, 7)
  "DontInline\0" // IR(20914, 11)
  "Pure\0" // IR(20925, 5)
  "Const\0" // IR(20930, 6)
  "Zext\0" // IR(20936, 5)
  "Sext\0" // IR(20941, 5)
  "ByVal\0" // IR(20946, 6)
  "Sret\0" // IR(20952, 5)
  "NoAlias\0" // IR(20957, 8)
  "NoCapture\0" // IR(20965, 10)
  "NoWrite\0" // IR(20975, 8)
  "NoReadWrite\0" // IR(20983, 12)
  "RuntimeAlignedINTEL\0" // IR(20995, 20)
  "Reduce\0" // IR(21015, 7)
  "InclusiveScan\0" // IR(21022, 14)
  "ExclusiveScan\0" // IR(21036, 14)
  "ClusteredReduce\0" // IR(21050, 16)
  "PartitionedReduceNV\0" // IR(21066, 20)
  "PartitionedInclusiveScanNV\0" // IR(21086, 27)
  "PartitionedExclusiveScanNV\0" // IR(21113, 27)
  "NoneINTEL\0" // IR(21140, 10)
  "ReadINTEL\0" // IR(21150, 10)
  "WriteINTEL\0" // IR(21160, 11)
  "ReadWriteINTEL\0" // IR(21171, 15)
  "Bias\0" // IR(21186, 5)
  "Lod\0" // IR(21191, 4)
  "Grad\0" // IR(21195, 5)
  "ConstOffset\0" // IR(21200, 12)
  "ConstOffsets\0" // IR(21212, 13)
  "MakeTexelAvailable\0" // IR(21225, 19)
  "MakeTexelAvailableKHR\0" // IR(21244, 22)
  "MakeTexelVisible\0" // IR(21266, 17)
  "MakeTexelVisibleKHR\0" // IR(21283, 20)
  "NonPrivateTexel\0" // IR(21303, 16)
  "NonPrivateTexelKHR\0" // IR(21319, 19)
  "VolatileTexel\0" // IR(21338, 14)
  "VolatileTexelKHR\0" // IR(21352, 17)
  "SignExtend\0" // IR(21369, 11)
  "ZeroExtend\0" // IR(21380, 11)
  "Nontemporal\0" // IR(21391, 12)
  "Offsets\0" // IR(21403, 8)
  "SnormInt8\0" // IR(21411, 10)
  "SnormInt16\0" // IR(21421, 11)
  "UnormInt8\0" // IR(21432, 10)
  "UnormInt16\0" // IR(21442, 11)
  "UnormShort565\0" // IR(21453, 14)
  "UnormShort555\0" // IR(21467, 14)
  "UnormInt101010\0" // IR(21481, 15)
  "SignedInt8\0" // IR(21496, 11)
  "SignedInt16\0" // IR(21507, 12)
  "SignedInt32\0" // IR(21519, 12)
  "UnsignedInt8\0" // IR(21531, 13)
  "UnsignedInt16\0" // IR(21544, 14)
  "UnsignedInt32\0" // IR(21558, 14)
  "HalfFloat\0" // IR(21572, 10)
  "UnormInt24\0" // IR(21582, 11)
  "UnormInt101010_2\0" // IR(21593, 17)
  "UnormInt10X6EXT\0" // IR(21610, 16)
  "UnsignedIntRaw10EXT\0" // IR(21626, 20)
  "UnsignedIntRaw12EXT\0" // IR(21646, 20)
  "UnormInt2_101010EXT\0" // IR(21666, 20)
  "UnsignedInt10X6EXT\0" // IR(21686, 19)
  "UnsignedInt12X4EXT\0" // IR(21705, 19)
  "UnsignedInt14X2EXT\0" // IR(21724, 19)
  "UnormInt12X4EXT\0" // IR(21743, 16)
  "UnormInt14X2EXT\0" // IR(21759, 16)
  "R\0" // IR(21775, 2)
  "A\0" // IR(21777, 2)
  "RG\0" // IR(21779, 3)
  "RA\0" // IR(21782, 3)
  "RGB\0" // IR(21785, 4)
  "RGBA\0" // IR(21789, 5)
  "BGRA\0" // IR(21794, 5)
  "ARGB\0" // IR(21799, 5)
  "Intensity\0" // IR(21804, 10)
  "Luminance\0" // IR(21814, 10)
  "Rx\0" // IR(21824, 3)
  "RGx\0" // IR(21827, 4)
  "RGBx\0" // IR(21831, 5)
  "Depth\0" // IR(21836, 6)
  "DepthStencil\0" // IR(21842, 13)
  "sRGB\0" // IR(21855, 5)
  "sRGBx\0" // IR(21860, 6)
  "sRGBA\0" // IR(21866, 6)
  "sBGRA\0" // IR(21872, 6)
  "ABGR\0" // IR(21878, 5)
  "InitOnDeviceReprogramINTEL\0" // IR(21883, 27)
  "InitOnDeviceResetINTEL\0" // IR(21910, 23)
  "NoWait\0" // IR(21933, 7)
  "WaitKernel\0" // IR(21940, 11)
  "WaitWorkGroup\0" // IR(21951, 14)
  "CmdExecTime\0" // IR(21965, 12)
  "MayUsePrintf\0" // IR(21977, 13)
  "Export\0" // IR(21990, 7)
  "Import\0" // IR(21997, 7)
  "LinkOnceODR\0" // IR(22004, 12)
  "UncachedINTEL\0" // IR(22016, 14)
  "CachedINTEL\0" // IR(22030, 12)
  "StreamingINTEL\0" // IR(22042, 15)
  "InvalidateAfterReadINTEL\0" // IR(22057, 25)
  "ConstCachedINTEL\0" // IR(22082, 17)
  "Unroll\0" // IR(22099, 7)
  "DontUnroll\0" // IR(22106, 11)
  "DependencyInfinite\0" // IR(22117, 19)
  "DependencyLength\0" // IR(22136, 17)
  "MinIterations\0" // IR(22153, 14)
  "MaxIterations\0" // IR(22167, 14)
  "IterationMultiple\0" // IR(22181, 18)
  "PeelCount\0" // IR(22199, 10)
  "PartialCount\0" // IR(22209, 13)
  "DependencyArrayINTEL\0" // IR(22222, 21)
  "LoopCoalesceINTEL\0" // IR(22243, 18)
  "MaxInterleavingINTEL\0" // IR(22261, 21)
  "SpeculatedIterationsINTEL\0" // IR(22282, 26)
  "NoFusionINTEL\0" // IR(22308, 14)
  "LoopCountINTEL\0" // IR(22322, 15)
  "MaxReinvocationDelayINTEL\0" // IR(22337, 26)
  "MatrixASignedComponentsINTEL\0" // IR(22363, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(22392, 29)
  "MatrixCBFloat16INTEL\0" // IR(22421, 21)
  "MatrixResultBFloat16INTEL\0" // IR(22442, 26)
  "MatrixAPackedInt8INTEL\0" // IR(22468, 23)
  "MatrixBPackedInt8INTEL\0" // IR(22491, 23)
  "MatrixAPackedInt4INTEL\0" // IR(22514, 23)
  "MatrixBPackedInt4INTEL\0" // IR(22537, 23)
  "MatrixATF32INTEL\0" // IR(22560, 17)
  "MatrixBTF32INTEL\0" // IR(22577, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(22594, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(22620, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(22646, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(22673, 27)
  "Aligned\0" // IR(22700, 8)
  "MakePointerAvailable\0" // IR(22708, 21)
  "MakePointerAvailableKHR\0" // IR(22729, 24)
  "MakePointerVisible\0" // IR(22753, 19)
  "MakePointerVisibleKHR\0" // IR(22772, 22)
  "NonPrivatePointer\0" // IR(22794, 18)
  "NonPrivatePointerKHR\0" // IR(22812, 21)
  "AliasScopeINTELMask\0" // IR(22833, 20)
  "NoAliasINTELMask\0" // IR(22853, 17)
  "Simple\0" // IR(22870, 7)
  "GLSL450\0" // IR(22877, 8)
  "OpenCL\0" // IR(22885, 7)
  "Vulkan\0" // IR(22892, 7)
  "VulkanKHR\0" // IR(22899, 10)
  "Relaxed\0" // IR(22909, 8)
  "Acquire\0" // IR(22917, 8)
  "Release\0" // IR(22925, 8)
  "AcquireRelease\0" // IR(22933, 15)
  "SequentiallyConsistent\0" // IR(22948, 23)
  "UniformMemory\0" // IR(22971, 14)
  "SubgroupMemory\0" // IR(22985, 15)
  "WorkgroupMemory\0" // IR(23000, 16)
  "CrossWorkgroupMemory\0" // IR(23016, 21)
  "AtomicCounterMemory\0" // IR(23037, 20)
  "ImageMemory\0" // IR(23057, 12)
  "OutputMemory\0" // IR(23069, 13)
  "OutputMemoryKHR\0" // IR(23082, 16)
  "MakeAvailable\0" // IR(23098, 14)
  "MakeAvailableKHR\0" // IR(23112, 17)
  "MakeVisible\0" // IR(23129, 12)
  "MakeVisibleKHR\0" // IR(23141, 15)
  "AutoINTEL\0" // IR(23156, 10)
  "WRAP\0" // IR(23166, 5)
  "SAT\0" // IR(23171, 4)
  "SAT_ZERO\0" // IR(23175, 9)
  "SAT_SYM\0" // IR(23184, 8)
  "PackedVectorFormat4x8Bit\0" // IR(23192, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(23217, 28)
  "TRN\0" // IR(23245, 4)
  "TRN_ZERO\0" // IR(23249, 9)
  "RND\0" // IR(23258, 4)
  "RND_ZERO\0" // IR(23262, 9)
  "RND_INF\0" // IR(23271, 8)
  "RND_MIN_INF\0" // IR(23279, 12)
  "RND_CONV\0" // IR(23291, 9)
  "RND_CONV_ODD\0" // IR(23300, 13)
  "RobustnessPerComponentNV\0" // IR(23313, 25)
  "RobustnessPerElementNV\0" // IR(23338, 23)
  "OpaqueKHR\0" // IR(23361, 10)
  "NoOpaqueKHR\0" // IR(23371, 12)
  "TerminateOnFirstHitKHR\0" // IR(23383, 23)
  "SkipClosestHitShaderKHR\0" // IR(23406, 24)
  "CullBackFacingTrianglesKHR\0" // IR(23430, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(23457, 28)
  "CullOpaqueKHR\0" // IR(23485, 14)
  "CullNoOpaqueKHR\0" // IR(23499, 16)
  "SkipTrianglesKHR\0" // IR(23515, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(23532, 24)
  "SkipAABBsKHR\0" // IR(23556, 13)
  "ForceOpacityMicromap2StateEXT\0" // IR(23569, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(23599, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(23640, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(23677, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(23714, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(23755, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(23797, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(23830, 33)
  "ClampToEdge\0" // IR(23863, 12)
  "Clamp\0" // IR(23875, 6)
  "Repeat\0" // IR(23881, 7)
  "RepeatMirrored\0" // IR(23888, 15)
  "Nearest\0" // IR(23903, 8)
  "Linear\0" // IR(23911, 7)
  "Unknown\0" // IR(23918, 8)
  "Rgba32f\0" // IR(23926, 8)
  "Rgba16f\0" // IR(23934, 8)
  "R32f\0" // IR(23942, 5)
  "Rgba8\0" // IR(23947, 6)
  "Rgba8Snorm\0" // IR(23953, 11)
  "Rg32f\0" // IR(23964, 6)
  "Rg16f\0" // IR(23970, 6)
  "R11fG11fB10f\0" // IR(23976, 13)
  "R16f\0" // IR(23989, 5)
  "Rgba16\0" // IR(23994, 7)
  "Rgb10A2\0" // IR(24001, 8)
  "Rg16\0" // IR(24009, 5)
  "Rg8\0" // IR(24014, 4)
  "R16\0" // IR(24018, 4)
  "R8\0" // IR(24022, 3)
  "Rgba16Snorm\0" // IR(24025, 12)
  "Rg16Snorm\0" // IR(24037, 10)
  "Rg8Snorm\0" // IR(24047, 9)
  "R16Snorm\0" // IR(24056, 9)
  "R8Snorm\0" // IR(24065, 8)
  "Rgba32i\0" // IR(24073, 8)
  "Rgba16i\0" // IR(24081, 8)
  "Rgba8i\0" // IR(24089, 7)
  "R32i\0" // IR(24096, 5)
  "Rg32i\0" // IR(24101, 6)
  "Rg16i\0" // IR(24107, 6)
  "Rg8i\0" // IR(24113, 5)
  "R16i\0" // IR(24118, 5)
  "R8i\0" // IR(24123, 4)
  "Rgba32ui\0" // IR(24127, 9)
  "Rgba16ui\0" // IR(24136, 9)
  "Rgba8ui\0" // IR(24145, 8)
  "R32ui\0" // IR(24153, 6)
  "Rgb10a2ui\0" // IR(24159, 10)
  "Rg32ui\0" // IR(24169, 7)
  "Rg16ui\0" // IR(24176, 7)
  "Rg8ui\0" // IR(24183, 6)
  "R16ui\0" // IR(24189, 6)
  "R8ui\0" // IR(24195, 5)
  "R64ui\0" // IR(24200, 6)
  "R64i\0" // IR(24206, 5)
  "CrossDevice\0" // IR(24211, 12)
  "Device\0" // IR(24223, 7)
  "Workgroup\0" // IR(24230, 10)
  "Subgroup\0" // IR(24240, 9)
  "Invocation\0" // IR(24249, 11)
  "QueueFamily\0" // IR(24260, 12)
  "QueueFamilyKHR\0" // IR(24272, 15)
  "ShaderCallKHR\0" // IR(24287, 14)
  "Flatten\0" // IR(24301, 8)
  "DontFlatten\0" // IR(24309, 12)
  "IdentifierPossibleDuplicates\0" // IR(24321, 29)
  "FlagUnknownPhysicalLayout\0" // IR(24350, 26)
  "ESSL\0" // IR(24376, 5)
  "GLSL\0" // IR(24381, 5)
  "OpenCL_C\0" // IR(24386, 9)
  "OpenCL_CPP\0" // IR(24395, 11)
  "HLSL\0" // IR(24406, 5)
  "CPP_for_OpenCL\0" // IR(24411, 15)
  "SYCL\0" // IR(24426, 5)
  "HERO_C\0" // IR(24431, 7)
  "NZSL\0" // IR(24438, 5)
  "WGSL\0" // IR(24443, 5)
  "Slang\0" // IR(24448, 6)
  "Zig\0" // IR(24454, 4)
  "Rust\0" // IR(24458, 5)
  "UniformConstant\0" // IR(24463, 16)
  "Input\0" // IR(24479, 6)
  "Output\0" // IR(24485, 7)
  "CrossWorkgroup\0" // IR(24492, 15)
  "Private\0" // IR(24507, 8)
  "Function\0" // IR(24515, 9)
  "Generic\0" // IR(24524, 8)
  "PushConstant\0" // IR(24532, 13)
  "AtomicCounter\0" // IR(24545, 14)
  "Image\0" // IR(24559, 6)
  "StorageBuffer\0" // IR(24565, 14)
  "TileImageEXT\0" // IR(24579, 13)
  "TileAttachmentQCOM\0" // IR(24592, 19)
  "NodePayloadAMDX\0" // IR(24611, 16)
  "CallableDataKHR\0" // IR(24627, 16)
  "CallableDataNV\0" // IR(24643, 15)
  "IncomingCallableDataKHR\0" // IR(24658, 24)
  "IncomingCallableDataNV\0" // IR(24682, 23)
  "RayPayloadKHR\0" // IR(24705, 14)
  "RayPayloadNV\0" // IR(24719, 13)
  "HitAttributeKHR\0" // IR(24732, 16)
  "HitAttributeNV\0" // IR(24748, 15)
  "IncomingRayPayloadKHR\0" // IR(24763, 22)
  "IncomingRayPayloadNV\0" // IR(24785, 21)
  "ShaderRecordBufferKHR\0" // IR(24806, 22)
  "ShaderRecordBufferNV\0" // IR(24828, 21)
  "PhysicalStorageBuffer\0" // IR(24849, 22)
  "PhysicalStorageBufferEXT\0" // IR(24871, 25)
  "HitObjectAttributeNV\0" // IR(24896, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(24917, 24)
  "CodeSectionINTEL\0" // IR(24941, 17)
  "DeviceOnlyINTEL\0" // IR(24958, 16)
  "HostOnlyINTEL\0" // IR(24974, 14)
  "WriteThroughINTEL\0" // IR(24988, 18)
  "WriteBackINTEL\0" // IR(25006, 15)
  "TensorView\0" // IR(25021, 11)
  "DecodeFunc\0" // IR(25032, 11)
  "Undefined\0" // IR(25043, 10)
  "NoneARM\0" // IR(25053, 8)
  "NontemporalARM\0" // IR(25061, 15)
  "OutOfBoundsValueARM\0" // IR(25076, 20)
  "MakeElementAvailableARM\0" // IR(25096, 24)
  "MakeElementVisibleARM\0" // IR(25120, 22)
  "NonPrivateElementARM\0" // IR(25142, 21)
  "Nop\0" // IR(25163, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(25167, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(25192, 27)
  "Undef\0" // IR(25219, 6)
  "SourceContinued\0" // IR(25225, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(25241, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(25274, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(25303, 41)
  "Source\0" // IR(25344, 7)
  "SourceExtension\0" // IR(25351, 16)
  "Name\0" // IR(25367, 5)
  "MemberName\0" // IR(25372, 11)
  "String\0" // IR(25383, 7)
  "Line\0" // IR(25390, 5)
  "Extension\0" // IR(25395, 10)
  "ExtInstImport\0" // IR(25405, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(25419, 46)
  "ExtInst\0" // IR(25465, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(25473, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(25507, 30)
  "MemoryModel\0" // IR(25537, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(25549, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(25582, 29)
  "EntryPoint\0" // IR(25611, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(25622, 32)
  "ExecutionMode\0" // IR(25654, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(25668, 28)
  "Capability\0" // IR(25696, 11)
  "TypeVoid\0" // IR(25707, 9)
  "TypeBool\0" // IR(25716, 9)
  "TypeInt\0" // IR(25725, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(25733, 37)
  "TypeFloat\0" // IR(25770, 10)
  "TypeVector\0" // IR(25780, 11)
  "TypeMatrix\0" // IR(25791, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(25802, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(25834, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(25872, 43)
  "TypeImage\0" // IR(25915, 10)
  "TypeSampler\0" // IR(25925, 12)
  "TypeSampledImage\0" // IR(25937, 17)
  "TypeArray\0" // IR(25954, 10)
  "TypeRuntimeArray\0" // IR(25964, 17)
  "TypeStruct\0" // IR(25981, 11)
  "TypeOpaque\0" // IR(25992, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(26003, 31)
  "TypePointer\0" // IR(26034, 12)
  "TypeFunction\0" // IR(26046, 13)
  "TypeEvent\0" // IR(26059, 10)
  "TypeDeviceEvent\0" // IR(26069, 16)
  "TypeReserveId\0" // IR(26085, 14)
  "TypeQueue\0" // IR(26099, 10)
  "TypePipe\0" // IR(26109, 9)
  "TypeForwardPointer\0" // IR(26118, 19)
  "ConstantTrue\0" // IR(26137, 13)
  "ConstantFalse\0" // IR(26150, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(26164, 38)
  "ConstantComposite\0" // IR(26202, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(26220, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(26261, 37)
  "ConstantSampler\0" // IR(26298, 16)
  "ConstantNull\0" // IR(26314, 13)
  "SpecConstantTrue\0" // IR(26327, 17)
  "SpecConstantFalse\0" // IR(26344, 18)
  "SpecConstant\0" // IR(26362, 13)
  "SpecConstantComposite\0" // IR(26375, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(26397, 41)
  "SpecConstantOp\0" // IR(26438, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(26453, 34)
  "FunctionParameter\0" // IR(26487, 18)
  "FunctionEnd\0" // IR(26505, 12)
  "FunctionCall\0" // IR(26517, 13)
  "Variable\0" // IR(26530, 9)
  "ImageTexelPointer\0" // IR(26539, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(26557, 40)
  "Load\0" // IR(26597, 5)
  "Store\0" // IR(26602, 6)
  "CopyMemory\0" // IR(26608, 11)
  "CopyMemorySized\0" // IR(26619, 16)
  "AccessChain\0" // IR(26635, 12)
  "InBoundsAccessChain\0" // IR(26647, 20)
  "PtrAccessChain\0" // IR(26667, 15)
  "ArrayLength\0" // IR(26682, 12)
  "GenericPtrMemSemantics\0" // IR(26694, 23)
  "InBoundsPtrAccessChain\0" // IR(26717, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(26740, 28)
  "Decorate\0" // IR(26768, 9)
  "MemberDecorate\0" // IR(26777, 15)
  "DecorationGroup\0" // IR(26792, 16)
  "GroupDecorate\0" // IR(26808, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(26822, 45)
  "GroupMemberDecorate\0" // IR(26867, 20)
  "VectorExtractDynamic\0" // IR(26887, 21)
  "VectorInsertDynamic\0" // IR(26908, 20)
  "VectorShuffle\0" // IR(26928, 14)
  "CompositeConstruct\0" // IR(26942, 19)
  "CompositeExtract\0" // IR(26961, 17)
  "CompositeInsert\0" // IR(26978, 16)
  "CopyObject\0" // IR(26994, 11)
  "Transpose\0" // IR(27005, 10)
  "SampledImage\0" // IR(27015, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(27028, 32)
  "ImageSampleImplicitLod\0" // IR(27060, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(27083, 23)
  "ImageSampleExplicitLod\0" // IR(27106, 23)
  "ImageSampleDrefImplicitLod\0" // IR(27129, 27)
  "ImageSampleDrefExplicitLod\0" // IR(27156, 27)
  "ImageSampleProjImplicitLod\0" // IR(27183, 27)
  "ImageSampleProjExplicitLod\0" // IR(27210, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(27237, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(27268, 31)
  "ImageFetch\0" // IR(27299, 11)
  "ImageGather\0" // IR(27310, 12)
  "ImageDrefGather\0" // IR(27322, 16)
  "ImageRead\0" // IR(27338, 10)
  "ImageWrite\0" // IR(27348, 11)
  "ImageQueryFormat\0" // IR(27359, 17)
  "ImageQueryOrder\0" // IR(27376, 16)
  "ImageQuerySizeLod\0" // IR(27392, 18)
  "ImageQuerySize\0" // IR(27410, 15)
  "ImageQueryLod\0" // IR(27425, 14)
  "ImageQueryLevels\0" // IR(27439, 17)
  "ImageQuerySamples\0" // IR(27456, 18)
  "ConvertFToU\0" // IR(27474, 12)
  "ConvertFToS\0" // IR(27486, 12)
  "ConvertSToF\0" // IR(27498, 12)
  "ConvertUToF\0" // IR(27510, 12)
  "UConvert\0" // IR(27522, 9)
  "SConvert\0" // IR(27531, 9)
  "FConvert\0" // IR(27540, 9)
  "QuantizeToF16\0" // IR(27549, 14)
  "ConvertPtrToU\0" // IR(27563, 14)
  "SatConvertSToU\0" // IR(27577, 15)
  "SatConvertUToS\0" // IR(27592, 15)
  "ConvertUToPtr\0" // IR(27607, 14)
  "PtrCastToGeneric\0" // IR(27621, 17)
  "GenericCastToPtr\0" // IR(27638, 17)
  "GenericCastToPtrExplicit\0" // IR(27655, 25)
  "Bitcast\0" // IR(27680, 8)
  "SNegate\0" // IR(27688, 8)
  "FNegate\0" // IR(27696, 8)
  "IAdd\0" // IR(27704, 5)
  "FAdd\0" // IR(27709, 5)
  "ISub\0" // IR(27714, 5)
  "FSub\0" // IR(27719, 5)
  "IMul\0" // IR(27724, 5)
  "FMul\0" // IR(27729, 5)
  "UDiv\0" // IR(27734, 5)
  "SDiv\0" // IR(27739, 5)
  "FDiv\0" // IR(27744, 5)
  "UMod\0" // IR(27749, 5)
  "SRem\0" // IR(27754, 5)
  "SMod\0" // IR(27759, 5)
  "FRem\0" // IR(27764, 5)
  "FMod\0" // IR(27769, 5)
  "VectorTimesScalar\0" // IR(27774, 18)
  "MatrixTimesScalar\0" // IR(27792, 18)
  "VectorTimesMatrix\0" // IR(27810, 18)
  "MatrixTimesVector\0" // IR(27828, 18)
  "MatrixTimesMatrix\0" // IR(27846, 18)
  "OuterProduct\0" // IR(27864, 13)
  "Dot\0" // IR(27877, 4)
  "IAddCarry\0" // IR(27881, 10)
  "ISubBorrow\0" // IR(27891, 11)
  "UMulExtended\0" // IR(27902, 13)
  "SMulExtended\0" // IR(27915, 13)
  "Any\0" // IR(27928, 4)
  "All\0" // IR(27932, 4)
  "IsNan\0" // IR(27936, 6)
  "IsInf\0" // IR(27942, 6)
  "IsFinite\0" // IR(27948, 9)
  "IsNormal\0" // IR(27957, 9)
  "SignBitSet\0" // IR(27966, 11)
  "LessOrGreater\0" // IR(27977, 14)
  "Ordered\0" // IR(27991, 8)
  "Unordered\0" // IR(27999, 10)
  "LogicalEqual\0" // IR(28009, 13)
  "LogicalNotEqual\0" // IR(28022, 16)
  "LogicalOr\0" // IR(28038, 10)
  "LogicalAnd\0" // IR(28048, 11)
  "LogicalNot\0" // IR(28059, 11)
  "Select\0" // IR(28070, 7)
  "IEqual\0" // IR(28077, 7)
  "INotEqual\0" // IR(28084, 10)
  "UGreaterThan\0" // IR(28094, 13)
  "SGreaterThan\0" // IR(28107, 13)
  "UGreaterThanEqual\0" // IR(28120, 18)
  "SGreaterThanEqual\0" // IR(28138, 18)
  "ULessThan\0" // IR(28156, 10)
  "SLessThan\0" // IR(28166, 10)
  "ULessThanEqual\0" // IR(28176, 15)
  "SLessThanEqual\0" // IR(28191, 15)
  "FOrdEqual\0" // IR(28206, 10)
  "FUnordEqual\0" // IR(28216, 12)
  "FOrdNotEqual\0" // IR(28228, 13)
  "FUnordNotEqual\0" // IR(28241, 15)
  "FOrdLessThan\0" // IR(28256, 13)
  "FUnordLessThan\0" // IR(28269, 15)
  "FOrdGreaterThan\0" // IR(28284, 16)
  "FUnordGreaterThan\0" // IR(28300, 18)
  "FOrdLessThanEqual\0" // IR(28318, 18)
  "FUnordLessThanEqual\0" // IR(28336, 20)
  "FOrdGreaterThanEqual\0" // IR(28356, 21)
  "FUnordGreaterThanEqual\0" // IR(28377, 23)
  "ShiftRightLogical\0" // IR(28400, 18)
  "ShiftRightArithmetic\0" // IR(28418, 21)
  "ShiftLeftLogical\0" // IR(28439, 17)
  "BitwiseOr\0" // IR(28456, 10)
  "BitwiseXor\0" // IR(28466, 11)
  "BitwiseAnd\0" // IR(28477, 11)
  "Not\0" // IR(28488, 4)
  "BitFieldInsert\0" // IR(28492, 15)
  "BitFieldSExtract\0" // IR(28507, 17)
  "BitFieldUExtract\0" // IR(28524, 17)
  "BitReverse\0" // IR(28541, 11)
  "BitCount\0" // IR(28552, 9)
  "DPdx\0" // IR(28561, 5)
  "DPdy\0" // IR(28566, 5)
  "Fwidth\0" // IR(28571, 7)
  "DPdxFine\0" // IR(28578, 9)
  "DPdyFine\0" // IR(28587, 9)
  "FwidthFine\0" // IR(28596, 11)
  "DPdxCoarse\0" // IR(28607, 11)
  "DPdyCoarse\0" // IR(28618, 11)
  "FwidthCoarse\0" // IR(28629, 13)
  "EmitVertex\0" // IR(28642, 11)
  "EndPrimitive\0" // IR(28653, 13)
  "EmitStreamVertex\0" // IR(28666, 17)
  "EndStreamPrimitive\0" // IR(28683, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(28702, 37)
  "ControlBarrier\0" // IR(28739, 15)
  "MemoryBarrier\0" // IR(28754, 14)
  "AtomicLoad\0" // IR(28768, 11)
  "AtomicStore\0" // IR(28779, 12)
  "AtomicExchange\0" // IR(28791, 15)
  "AtomicCompareExchange\0" // IR(28806, 22)
  "AtomicCompareExchangeWeak\0" // IR(28828, 26)
  "AtomicIIncrement\0" // IR(28854, 17)
  "AtomicIDecrement\0" // IR(28871, 17)
  "AtomicIAdd\0" // IR(28888, 11)
  "AtomicISub\0" // IR(28899, 11)
  "AtomicSMin\0" // IR(28910, 11)
  "AtomicUMin\0" // IR(28921, 11)
  "AtomicSMax\0" // IR(28932, 11)
  "AtomicUMax\0" // IR(28943, 11)
  "AtomicAnd\0" // IR(28954, 10)
  "AtomicOr\0" // IR(28964, 9)
  "AtomicXor\0" // IR(28973, 10)
  "Phi\0" // IR(28983, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(28987, 30)
  "LoopMerge\0" // IR(29017, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(29027, 35)
  "SelectionMerge\0" // IR(29062, 15)
  "Label\0" // IR(29077, 6)
  "Branch\0" // IR(29083, 7)
  "BranchConditional\0" // IR(29090, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(29108, 45)
  "Switch\0" // IR(29153, 7)
  "Kill\0" // IR(29160, 5)
  "Return\0" // IR(29165, 7)
  "ReturnValue\0" // IR(29172, 12)
  "Unreachable\0" // IR(29184, 12)
  "LifetimeStart\0" // IR(29196, 14)
  "LifetimeStop\0" // IR(29210, 13)
  "GroupAsyncCopy\0" // IR(29223, 15)
  "GroupWaitEvents\0" // IR(29238, 16)
  "GroupAll\0" // IR(29254, 9)
  "GroupAny\0" // IR(29263, 9)
  "GroupBroadcast\0" // IR(29272, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(29287, 33)
  "GroupIAdd\0" // IR(29320, 10)
  "GroupFAdd\0" // IR(29330, 10)
  "GroupFMin\0" // IR(29340, 10)
  "GroupUMin\0" // IR(29350, 10)
  "GroupSMin\0" // IR(29360, 10)
  "GroupFMax\0" // IR(29370, 10)
  "GroupUMax\0" // IR(29380, 10)
  "GroupSMax\0" // IR(29390, 10)
  "ReadPipe\0" // IR(29400, 9)
  "WritePipe\0" // IR(29409, 10)
  "ReservedReadPipe\0" // IR(29419, 17)
  "ReservedWritePipe\0" // IR(29436, 18)
  "ReserveReadPipePackets\0" // IR(29454, 23)
  "ReserveWritePipePackets\0" // IR(29477, 24)
  "CommitReadPipe\0" // IR(29501, 15)
  "CommitWritePipe\0" // IR(29516, 16)
  "IsValidReserveId\0" // IR(29532, 17)
  "GetNumPipePackets\0" // IR(29549, 18)
  "GetMaxPipePackets\0" // IR(29567, 18)
  "GroupReserveReadPipePackets\0" // IR(29585, 28)
  "GroupReserveWritePipePackets\0" // IR(29613, 29)
  "GroupCommitReadPipe\0" // IR(29642, 20)
  "GroupCommitWritePipe\0" // IR(29662, 21)
  "EnqueueMarker\0" // IR(29683, 14)
  "EnqueueKernel\0" // IR(29697, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(29711, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(29741, 32)
  "GetKernelWorkGroupSize\0" // IR(29773, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(29796, 40)
  "RetainEvent\0" // IR(29836, 12)
  "ReleaseEvent\0" // IR(29848, 13)
  "CreateUserEvent\0" // IR(29861, 16)
  "IsValidEvent\0" // IR(29877, 13)
  "SetUserEventStatus\0" // IR(29890, 19)
  "CaptureEventProfilingInfo\0" // IR(29909, 26)
  "GetDefaultQueue\0" // IR(29935, 16)
  "BuildNDRange\0" // IR(29951, 13)
  "ImageSparseSampleImplicitLod\0" // IR(29964, 29)
  "ImageSparseSampleExplicitLod\0" // IR(29993, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(30022, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(30055, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(30088, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(30121, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(30154, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(30191, 37)
  "ImageSparseFetch\0" // IR(30228, 17)
  "ImageSparseGather\0" // IR(30245, 18)
  "ImageSparseDrefGather\0" // IR(30263, 22)
  "ImageSparseTexelsResident\0" // IR(30285, 26)
  "NoLine\0" // IR(30311, 7)
  "AtomicFlagTestAndSet\0" // IR(30318, 21)
  "AtomicFlagClear\0" // IR(30339, 16)
  "ImageSparseRead\0" // IR(30355, 16)
  "SizeOf\0" // IR(30371, 7)
  "TypePipeStorage\0" // IR(30378, 16)
  "ConstantPipeStorage\0" // IR(30394, 20)
  "CreatePipeFromPipeStorage\0" // IR(30414, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(30440, 35)
  "GetKernelMaxNumSubgroups\0" // IR(30475, 25)
  "TypeNamedBarrier\0" // IR(30500, 17)
  "NamedBarrierInitialize\0" // IR(30517, 23)
  "MemoryNamedBarrier\0" // IR(30540, 19)
  "ModuleProcessed\0" // IR(30559, 16)
  "ExecutionModeId\0" // IR(30575, 16)
  "DecorateId\0" // IR(30591, 11)
  "GroupNonUniformElect\0" // IR(30602, 21)
  "GroupNonUniformAll\0" // IR(30623, 19)
  "GroupNonUniformAny\0" // IR(30642, 19)
  "GroupNonUniformAllEqual\0" // IR(30661, 24)
  "GroupNonUniformBroadcast\0" // IR(30685, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(30710, 30)
  "GroupNonUniformInverseBallot\0" // IR(30740, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(30769, 32)
  "GroupNonUniformBallotBitCount\0" // IR(30801, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(30831, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(30860, 29)
  "GroupNonUniformShuffleXor\0" // IR(30889, 26)
  "GroupNonUniformShuffleUp\0" // IR(30915, 25)
  "GroupNonUniformShuffleDown\0" // IR(30940, 27)
  "GroupNonUniformIAdd\0" // IR(30967, 20)
  "GroupNonUniformFAdd\0" // IR(30987, 20)
  "GroupNonUniformIMul\0" // IR(31007, 20)
  "GroupNonUniformFMul\0" // IR(31027, 20)
  "GroupNonUniformSMin\0" // IR(31047, 20)
  "GroupNonUniformUMin\0" // IR(31067, 20)
  "GroupNonUniformFMin\0" // IR(31087, 20)
  "GroupNonUniformSMax\0" // IR(31107, 20)
  "GroupNonUniformUMax\0" // IR(31127, 20)
  "GroupNonUniformFMax\0" // IR(31147, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(31167, 26)
  "GroupNonUniformBitwiseOr\0" // IR(31193, 25)
  "GroupNonUniformBitwiseXor\0" // IR(31218, 26)
  "GroupNonUniformLogicalAnd\0" // IR(31244, 26)
  "GroupNonUniformLogicalOr\0" // IR(31270, 25)
  "GroupNonUniformLogicalXor\0" // IR(31295, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(31321, 29)
  "GroupNonUniformQuadSwap\0" // IR(31350, 24)
  "CopyLogical\0" // IR(31374, 12)
  "PtrEqual\0" // IR(31386, 9)
  "PtrNotEqual\0" // IR(31395, 12)
  "PtrDiff\0" // IR(31407, 8)
  "ColorAttachmentReadEXT\0" // IR(31415, 23)
  "DepthAttachmentReadEXT\0" // IR(31438, 23)
  "StencilAttachmentReadEXT\0" // IR(31461, 25)
  "TypeTensorARM\0" // IR(31486, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(31500, 42)
  "TensorReadARM\0" // IR(31542, 14)
  "TensorWriteARM\0" // IR(31556, 15)
  "TensorQuerySizeARM\0" // IR(31571, 19)
  "GraphConstantARM\0" // IR(31590, 17)
  "GraphEntryPointARM\0" // IR(31607, 19)
  "GraphInputARM\0" // IR(31626, 14)
  "GraphSetOutputARM\0" // IR(31640, 18)
  "GraphEndARM\0" // IR(31658, 12)
  "TypeGraphARM\0" // IR(31670, 13)
  "TerminateInvocation\0" // IR(31683, 20)
  "TypeUntypedPointerKHR\0" // IR(31703, 22)
  "UntypedVariableKHR\0" // IR(31725, 19)
  "UntypedAccessChainKHR\0" // IR(31744, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(31766, 30)
  "SubgroupFirstInvocationKHR\0" // IR(31796, 27)
  "UntypedPtrAccessChainKHR\0" // IR(31823, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(31848, 33)
  "UntypedArrayLengthKHR\0" // IR(31881, 22)
  "UntypedPrefetchKHR\0" // IR(31903, 19)
  "SubgroupAllKHR\0" // IR(31922, 15)
  "SubgroupAnyKHR\0" // IR(31937, 15)
  "SubgroupAllEqualKHR\0" // IR(31952, 20)
  "SubgroupReadInvocationKHR\0" // IR(31972, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(31998, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(32024, 25)
  "TraceRayKHR\0" // IR(32049, 12)
  "ExecuteCallableKHR\0" // IR(32061, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(32080, 35)
  "IgnoreIntersectionKHR\0" // IR(32115, 22)
  "TerminateRayKHR\0" // IR(32137, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(32153, 47)
  "SDot\0" // IR(32200, 5)
  "SDotKHR\0" // IR(32205, 8)
  "UDot\0" // IR(32213, 5)
  "UDotKHR\0" // IR(32218, 8)
  "SUDot\0" // IR(32226, 6)
  "SUDotKHR\0" // IR(32232, 9)
  "SDotAccSat\0" // IR(32241, 11)
  "SDotAccSatKHR\0" // IR(32252, 14)
  "UDotAccSat\0" // IR(32266, 11)
  "UDotAccSatKHR\0" // IR(32277, 14)
  "SUDotAccSat\0" // IR(32291, 12)
  "SUDotAccSatKHR\0" // IR(32303, 15)
  "TypeCooperativeMatrixKHR\0" // IR(32318, 25)
  "CooperativeMatrixLoadKHR\0" // IR(32343, 25)
  "CooperativeMatrixStoreKHR\0" // IR(32368, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(32394, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(32448, 27)
  "CooperativeMatrixLengthKHR\0" // IR(32475, 27)
  "ConstantCompositeReplicateEXT\0" // IR(32502, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(32532, 34)
  "CompositeConstructReplicateEXT\0" // IR(32566, 31)
  "TypeRayQueryKHR\0" // IR(32597, 16)
  "RayQueryInitializeKHR\0" // IR(32613, 22)
  "RayQueryTerminateKHR\0" // IR(32635, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(32656, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(32688, 31)
  "RayQueryProceedKHR\0" // IR(32719, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(32738, 31)
  "ImageSampleWeightedQCOM\0" // IR(32769, 24)
  "ImageBoxFilterQCOM\0" // IR(32793, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(32812, 23)
  "ImageBlockMatchSADQCOM\0" // IR(32835, 23)
  "BitCastArrayQCOM\0" // IR(32858, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(32875, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(32904, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(32933, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(32962, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(32991, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(33021, 28)
  "ExtractSubArrayQCOM\0" // IR(33049, 20)
  "GroupIAddNonUniformAMD\0" // IR(33069, 23)
  "GroupFAddNonUniformAMD\0" // IR(33092, 23)
  "GroupFMinNonUniformAMD\0" // IR(33115, 23)
  "GroupUMinNonUniformAMD\0" // IR(33138, 23)
  "GroupSMinNonUniformAMD\0" // IR(33161, 23)
  "GroupFMaxNonUniformAMD\0" // IR(33184, 23)
  "GroupUMaxNonUniformAMD\0" // IR(33207, 23)
  "GroupSMaxNonUniformAMD\0" // IR(33230, 23)
  "FragmentMaskFetchAMD\0" // IR(33253, 21)
  "FragmentFetchAMD\0" // IR(33274, 17)
  "ReadClockKHR\0" // IR(33291, 13)
  "AllocateNodePayloadsAMDX\0" // IR(33304, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(33329, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(33353, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(33378, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(33407, 27)
  "IsNodePayloadValidAMDX\0" // IR(33434, 23)
  "ConstantStringAMDX\0" // IR(33457, 19)
  "SpecConstantStringAMDX\0" // IR(33476, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(33499, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(33525, 26)
  "HitObjectRecordHitMotionNV\0" // IR(33551, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(33578, 36)
  "HitObjectRecordMissMotionNV\0" // IR(33614, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(33642, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(33670, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(33698, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(33731, 30)
  "HitObjectTraceRayMotionNV\0" // IR(33761, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(33787, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(33826, 44)
  "HitObjectRecordEmptyNV\0" // IR(33870, 23)
  "HitObjectTraceRayNV\0" // IR(33893, 20)
  "HitObjectRecordHitNV\0" // IR(33913, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(33934, 30)
  "HitObjectRecordMissNV\0" // IR(33964, 22)
  "HitObjectExecuteShaderNV\0" // IR(33986, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(34011, 26)
  "HitObjectGetAttributesNV\0" // IR(34037, 25)
  "HitObjectGetHitKindNV\0" // IR(34062, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(34084, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(34113, 28)
  "HitObjectGetInstanceIdNV\0" // IR(34141, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(34166, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(34200, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(34232, 29)
  "HitObjectGetRayTMaxNV\0" // IR(34261, 22)
  "HitObjectGetRayTMinNV\0" // IR(34283, 22)
  "HitObjectIsEmptyNV\0" // IR(34305, 19)
  "HitObjectIsHitNV\0" // IR(34324, 17)
  "HitObjectIsMissNV\0" // IR(34341, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(34359, 29)
  "ReorderThreadWithHintNV\0" // IR(34388, 24)
  "TypeHitObjectNV\0" // IR(34412, 16)
  "ImageSampleFootprintNV\0" // IR(34428, 23)
  "TypeCooperativeVectorNV\0" // IR(34451, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(34475, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(34504, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(34546, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(34585, 32)
  "CooperativeMatrixConvertNV\0" // IR(34617, 27)
  "EmitMeshTasksEXT\0" // IR(34644, 17)
  "SetMeshOutputsEXT\0" // IR(34661, 18)
  "GroupNonUniformPartitionNV\0" // IR(34679, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(34706, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(34739, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(34774, 38)
  "CooperativeVectorLoadNV\0" // IR(34812, 24)
  "CooperativeVectorStoreNV\0" // IR(34836, 25)
  "ReportIntersectionKHR\0" // IR(34861, 22)
  "ReportIntersectionNV\0" // IR(34883, 21)
  "IgnoreIntersectionNV\0" // IR(34904, 21)
  "TerminateRayNV\0" // IR(34925, 15)
  "TraceNV\0" // IR(34940, 8)
  "TraceMotionNV\0" // IR(34948, 14)
  "TraceRayMotionNV\0" // IR(34962, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(34979, 50)
  "TypeAccelerationStructureKHR\0" // IR(35029, 29)
  "TypeAccelerationStructureNV\0" // IR(35058, 28)
  "ExecuteCallableNV\0" // IR(35086, 18)
  "RayQueryGetClusterIdNV\0" // IR(35104, 23)
  "HitObjectGetClusterIdNV\0" // IR(35127, 24)
  "TypeCooperativeMatrixNV\0" // IR(35151, 24)
  "CooperativeMatrixLoadNV\0" // IR(35175, 24)
  "CooperativeMatrixStoreNV\0" // IR(35199, 25)
  "CooperativeMatrixMulAddNV\0" // IR(35224, 26)
  "CooperativeMatrixLengthNV\0" // IR(35250, 26)
  "BeginInvocationInterlockEXT\0" // IR(35276, 28)
  "EndInvocationInterlockEXT\0" // IR(35304, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(35330, 43)
  "CooperativeMatrixReduceNV\0" // IR(35373, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(35399, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(35430, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(35474, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(35504, 31)
  "CooperativeMatrixPerElementOpNV\0" // IR(35535, 32)
  "TypeTensorLayoutNV\0" // IR(35567, 19)
  "TypeTensorViewNV\0" // IR(35586, 17)
  "CreateTensorLayoutNV\0" // IR(35603, 21)
  "TensorLayoutSetDimensionNV\0" // IR(35624, 27)
  "TensorLayoutSetStrideNV\0" // IR(35651, 24)
  "TensorLayoutSliceNV\0" // IR(35675, 20)
  "TensorLayoutSetClampValueNV\0" // IR(35695, 28)
  "CreateTensorViewNV\0" // IR(35723, 19)
  "TensorViewSetDimensionNV\0" // IR(35742, 25)
  "TensorViewSetStrideNV\0" // IR(35767, 22)
  "IsHelperInvocationEXT\0" // IR(35789, 22)
  "TensorViewSetClipNV\0" // IR(35811, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(35831, 27)
  "CooperativeMatrixTransposeNV\0" // IR(35858, 29)
  "ConvertUToImageNV\0" // IR(35887, 18)
  "ConvertUToSamplerNV\0" // IR(35905, 20)
  "ConvertImageToUNV\0" // IR(35925, 18)
  "ConvertSamplerToUNV\0" // IR(35943, 20)
  "ConvertUToSampledImageNV\0" // IR(35963, 25)
  "ConvertSampledImageToUNV\0" // IR(35988, 25)
  "SamplerImageAddressingModeNV\0" // IR(36013, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(36042, 52)
  "RawAccessChainNV\0" // IR(36094, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(36111, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(36151, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(36189, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(36227, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(36261, 37)
  "HitObjectGetSpherePositionNV\0" // IR(36298, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(36327, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(36354, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(36381, 23)
  "HitObjectIsSphereHitNV\0" // IR(36404, 23)
  "HitObjectIsLSSHitNV\0" // IR(36427, 20)
  "RayQueryIsSphereHitNV\0" // IR(36447, 22)
  "RayQueryIsLSSHitNV\0" // IR(36469, 19)
  "SubgroupShuffleDownINTEL\0" // IR(36488, 25)
  "SubgroupShuffleUpINTEL\0" // IR(36513, 23)
  "SubgroupShuffleXorINTEL\0" // IR(36536, 24)
  "SubgroupBlockReadINTEL\0" // IR(36560, 23)
  "SubgroupBlockWriteINTEL\0" // IR(36583, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(36607, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(36635, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(36664, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(36697, 34)
  "UCountLeadingZerosINTEL\0" // IR(36731, 24)
  "UCountTrailingZerosINTEL\0" // IR(36755, 25)
  "AbsISubINTEL\0" // IR(36780, 13)
  "AbsUSubINTEL\0" // IR(36793, 13)
  "IAddSatINTEL\0" // IR(36806, 13)
  "UAddSatINTEL\0" // IR(36819, 13)
  "IAverageINTEL\0" // IR(36832, 14)
  "UAverageINTEL\0" // IR(36846, 14)
  "IAverageRoundedINTEL\0" // IR(36860, 21)
  "UAverageRoundedINTEL\0" // IR(36881, 21)
  "ISubSatINTEL\0" // IR(36902, 13)
  "USubSatINTEL\0" // IR(36915, 13)
  "IMul32x16INTEL\0" // IR(36928, 15)
  "UMul32x16INTEL\0" // IR(36943, 15)
  "ConstantFunctionPointerINTEL\0" // IR(36958, 29)
  "FunctionPointerCallINTEL\0" // IR(36987, 25)
  "AsmTargetINTEL\0" // IR(37012, 15)
  "AsmCallINTEL\0" // IR(37027, 13)
  "AtomicFMinEXT\0" // IR(37040, 14)
  "AtomicFMaxEXT\0" // IR(37054, 14)
  "AssumeTrueKHR\0" // IR(37068, 14)
  "ExpectKHR\0" // IR(37082, 10)
  "DecorateString\0" // IR(37092, 15)
  "DecorateStringGOOGLE\0" // IR(37107, 21)
  "MemberDecorateString\0" // IR(37128, 21)
  "MemberDecorateStringGOOGLE\0" // IR(37149, 27)
  "VmeImageINTEL\0" // IR(37176, 14)
  "TypeVmeImageINTEL\0" // IR(37190, 18)
  "TypeAvcImePayloadINTEL\0" // IR(37208, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(37231, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(37254, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(37277, 23)
  "TypeAvcMceResultINTEL\0" // IR(37300, 22)
  "TypeAvcImeResultINTEL\0" // IR(37322, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(37344, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(37390, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(37434, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(37473, 37)
  "TypeAvcRefResultINTEL\0" // IR(37510, 22)
  "TypeAvcSicResultINTEL\0" // IR(37532, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(37554, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(37614, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(37667, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(37714, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(37754, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(37805, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(37849, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(37900, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(37956, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(38006, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(38058, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(38107, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(38154, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(38204, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(38255, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(38311, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(38344, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(38396, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(38457, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(38518, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(38557, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(38595, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(38634, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(38672, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(38711, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(38749, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(38785, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(38824, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(38867, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(38905, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(38943, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(38981, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(39026, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(39066, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(39128, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(39158, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(39196, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(39232, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(39265, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(39300, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(39339, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(39382, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(39429, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(39483, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(39517, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(39564, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(39609, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(39664, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(39717, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(39773, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(39827, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(39885, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(39941, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(39979, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(40025, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(40069, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(40118, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(40165, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(40235, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(40303, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(40372, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(40440, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(40506, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(40573, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(40609, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(40657, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(40716, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(40774, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(40830, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(40860, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(40890, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(40929, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(40975, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(41018, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(41065, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(41110, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(41156, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(41212, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(41250, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(41280, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(41312, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(41348, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(41390, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(41429, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(41468, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(41512, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(41560, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(41610, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(41653, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(41701, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(41744, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(41775, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(41822, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(41867, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(41913, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(41969, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(42007, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(42042, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(42086, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(42132, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(42173, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(42209, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(42259, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(42307, 35)
  "SaveMemoryINTEL\0" // IR(42342, 16)
  "RestoreMemoryINTEL\0" // IR(42358, 19)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(42377, 28)
  "ArbitraryFloatCastINTEL\0" // IR(42405, 24)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(42429, 31)
  "ArbitraryFloatCastToIntINTEL\0" // IR(42460, 29)
  "ArbitraryFloatAddINTEL\0" // IR(42489, 23)
  "ArbitraryFloatSubINTEL\0" // IR(42512, 23)
  "ArbitraryFloatMulINTEL\0" // IR(42535, 23)
  "ArbitraryFloatDivINTEL\0" // IR(42558, 23)
  "ArbitraryFloatGTINTEL\0" // IR(42581, 22)
  "ArbitraryFloatGEINTEL\0" // IR(42603, 22)
  "ArbitraryFloatLTINTEL\0" // IR(42625, 22)
  "ArbitraryFloatLEINTEL\0" // IR(42647, 22)
  "ArbitraryFloatEQINTEL\0" // IR(42669, 22)
  "ArbitraryFloatRecipINTEL\0" // IR(42691, 25)
  "ArbitraryFloatRSqrtINTEL\0" // IR(42716, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(42741, 24)
  "ArbitraryFloatHypotINTEL\0" // IR(42765, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(42790, 24)
  "ArbitraryFloatLogINTEL\0" // IR(42814, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(42837, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(42861, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(42886, 25)
  "ArbitraryFloatExpINTEL\0" // IR(42911, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(42934, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(42958, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(42983, 25)
  "ArbitraryFloatSinINTEL\0" // IR(43008, 23)
  "ArbitraryFloatCosINTEL\0" // IR(43031, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(43054, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(43080, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(43105, 25)
  "ArbitraryFloatASinINTEL\0" // IR(43130, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(43154, 26)
  "ArbitraryFloatACosINTEL\0" // IR(43180, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(43204, 26)
  "ArbitraryFloatATanINTEL\0" // IR(43230, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(43254, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(43280, 25)
  "ArbitraryFloatPowINTEL\0" // IR(43305, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(43328, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(43352, 24)
  "LoopControlINTEL\0" // IR(43376, 17)
  "AliasDomainDeclINTEL\0" // IR(43393, 21)
  "AliasScopeDeclINTEL\0" // IR(43414, 20)
  "AliasScopeListDeclINTEL\0" // IR(43434, 24)
  "FixedSqrtINTEL\0" // IR(43458, 15)
  "FixedRecipINTEL\0" // IR(43473, 16)
  "FixedRsqrtINTEL\0" // IR(43489, 16)
  "FixedSinINTEL\0" // IR(43505, 14)
  "FixedCosINTEL\0" // IR(43519, 14)
  "FixedSinCosINTEL\0" // IR(43533, 17)
  "FixedSinPiINTEL\0" // IR(43550, 16)
  "FixedCosPiINTEL\0" // IR(43566, 16)
  "FixedSinCosPiINTEL\0" // IR(43582, 19)
  "FixedLogINTEL\0" // IR(43601, 14)
  "FixedExpINTEL\0" // IR(43615, 14)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(43629, 29)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(43658, 29)
  "ReadPipeBlockingINTEL\0" // IR(43687, 22)
  "WritePipeBlockingINTEL\0" // IR(43709, 23)
  "RayQueryGetRayTMinKHR\0" // IR(43732, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(43754, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(43777, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(43805, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(43851, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(43888, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(43953, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(43993, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(44034, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(44073, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(44109, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(44155, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(44200, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(44242, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(44274, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(44303, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(44343, 40)
  "AtomicFAddEXT\0" // IR(44383, 14)
  "TypeBufferSurfaceINTEL\0" // IR(44397, 23)
  "TypeStructContinuedINTEL\0" // IR(44420, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(44445, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(44477, 36)
  "CompositeConstructContinuedINTEL\0" // IR(44513, 33)
  "ConvertFToBF16INTEL\0" // IR(44546, 20)
  "ConvertBF16ToFINTEL\0" // IR(44566, 20)
  "ControlBarrierArriveINTEL\0" // IR(44586, 26)
  "ControlBarrierWaitINTEL\0" // IR(44612, 24)
  "TaskSequenceCreateINTEL\0" // IR(44636, 24)
  "TaskSequenceAsyncINTEL\0" // IR(44660, 23)
  "TaskSequenceGetINTEL\0" // IR(44683, 21)
  "TaskSequenceReleaseINTEL\0" // IR(44704, 25)
  "TypeTaskSequenceINTEL\0" // IR(44729, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(44751, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(44778, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(44803, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(44837, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(44871, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(44900, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(44926, 62)
  "BitwiseFunctionINTEL\0" // IR(44988, 21)
  "ConditionalExtensionINTEL\0" // IR(45009, 26)
  "ConditionalEntryPointINTEL\0" // IR(45035, 27)
  "ConditionalCapabilityINTEL\0" // IR(45062, 27)
  "SpecConstantTargetINTEL\0" // IR(45089, 24)
  "SpecConstantArchitectureINTEL\0" // IR(45113, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(45143, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(45180, 30)
  "ConditionalCopyObjectINTEL\0" // IR(45210, 27)
  "GroupIMulKHR\0" // IR(45237, 13)
  "GroupFMulKHR\0" // IR(45250, 13)
  "GroupBitwiseAndKHR\0" // IR(45263, 19)
  "GroupBitwiseOrKHR\0" // IR(45282, 18)
  "GroupBitwiseXorKHR\0" // IR(45300, 19)
  "GroupLogicalAndKHR\0" // IR(45319, 19)
  "GroupLogicalOrKHR\0" // IR(45338, 18)
  "GroupLogicalXorKHR\0" // IR(45356, 19)
  "RoundFToTF32INTEL\0" // IR(45375, 18)
  "MaskedGatherINTEL\0" // IR(45393, 18)
  "MaskedScatterINTEL\0" // IR(45411, 19)
  "ConvertHandleToImageINTEL\0" // IR(45430, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(45456, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(45484, 33)
  "DebugInfoNone\0" // IR(45517, 14)
  "DebugCompilationUnit\0" // IR(45531, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(45552, 52)
  "DebugTypeBasic\0" // IR(45604, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(45619, 34)
  "DebugTypePointer\0" // IR(45653, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(45670, 38)
  "DebugTypeQualifier\0" // IR(45708, 19)
  "DebugTypeArray\0" // IR(45727, 15)
  "DebugTypeVector\0" // IR(45742, 16)
  "DebugTypedef\0" // IR(45758, 13)
  "DebugTypeFunction\0" // IR(45771, 18)
  "DebugTypeEnum\0" // IR(45789, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(45803, 38)
  "DebugTypeComposite\0" // IR(45841, 19)
  "DebugTypeMember\0" // IR(45860, 16)
  "DebugTypeInheritance\0" // IR(45876, 21)
  "DebugTypePtrToMember\0" // IR(45897, 21)
  "DebugTypeTemplate\0" // IR(45918, 18)
  "DebugTypeTemplateParameter\0" // IR(45936, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(45963, 35)
  "DebugTypeTemplateParameterPack\0" // IR(45998, 31)
  "DebugGlobalVariable\0" // IR(46029, 20)
  "DebugFunctionDeclaration\0" // IR(46049, 25)
  "DebugFunction\0" // IR(46074, 14)
  "DebugLexicalBlock\0" // IR(46088, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(46106, 31)
  "DebugScope\0" // IR(46137, 11)
  "DebugNoScope\0" // IR(46148, 13)
  "DebugInlinedAt\0" // IR(46161, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(46176, 42)
  "DebugLocalVariable\0" // IR(46218, 19)
  "DebugInlinedVariable\0" // IR(46237, 21)
  "DebugDeclare\0" // IR(46258, 13)
  "DebugValue\0" // IR(46271, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(46282, 33)
  "DebugOperation\0" // IR(46315, 15)
  "DebugExpression\0" // IR(46330, 16)
  "DebugMacroDef\0" // IR(46346, 14)
  "DebugMacroUndef\0" // IR(46360, 16)
  "Round\0" // IR(46376, 6)
  "RoundEven\0" // IR(46382, 10)
  "Trunc\0" // IR(46392, 6)
  "FAbs\0" // IR(46398, 5)
  "SAbs\0" // IR(46403, 5)
  "FSign\0" // IR(46408, 6)
  "SSign\0" // IR(46414, 6)
  "Floor\0" // IR(46420, 6)
  "Ceil\0" // IR(46426, 5)
  "Fract\0" // IR(46431, 6)
  "Radians\0" // IR(46437, 8)
  "Degrees\0" // IR(46445, 8)
  "Sin\0" // IR(46453, 4)
  "Cos\0" // IR(46457, 4)
  "Tan\0" // IR(46461, 4)
  "Asin\0" // IR(46465, 5)
  "Acos\0" // IR(46470, 5)
  "Atan\0" // IR(46475, 5)
  "Sinh\0" // IR(46480, 5)
  "Cosh\0" // IR(46485, 5)
  "Tanh\0" // IR(46490, 5)
  "Asinh\0" // IR(46495, 6)
  "Acosh\0" // IR(46501, 6)
  "Atanh\0" // IR(46507, 6)
  "Atan2\0" // IR(46513, 6)
  "Pow\0" // IR(46519, 4)
  "Exp\0" // IR(46523, 4)
  "Log\0" // IR(46527, 4)
  "Exp2\0" // IR(46531, 5)
  "Log2\0" // IR(46536, 5)
  "Sqrt\0" // IR(46541, 5)
  "InverseSqrt\0" // IR(46546, 12)
  "Determinant\0" // IR(46558, 12)
  "MatrixInverse\0" // IR(46570, 14)
  "Modf\0" // IR(46584, 5)
  "ModfStruct\0" // IR(46589, 11)
  "FMin\0" // IR(46600, 5)
  "UMin\0" // IR(46605, 5)
  "SMin\0" // IR(46610, 5)
  "FMax\0" // IR(46615, 5)
  "UMax\0" // IR(46620, 5)
  "SMax\0" // IR(46625, 5)
  "FClamp\0" // IR(46630, 7)
  "UClamp\0" // IR(46637, 7)
  "SClamp\0" // IR(46644, 7)
  "FMix\0" // IR(46651, 5)
  "IMix\0" // IR(46656, 5)
  "Step\0" // IR(46661, 5)
  "SmoothStep\0" // IR(46666, 11)
  "Fma\0" // IR(46677, 4)
  "Frexp\0" // IR(46681, 6)
  "FrexpStruct\0" // IR(46687, 12)
  "Ldexp\0" // IR(46699, 6)
  "PackSnorm4x8\0" // IR(46705, 13)
  "PackUnorm4x8\0" // IR(46718, 13)
  "PackSnorm2x16\0" // IR(46731, 14)
  "PackUnorm2x16\0" // IR(46745, 14)
  "PackHalf2x16\0" // IR(46759, 13)
  "PackDouble2x32\0" // IR(46772, 15)
  "UnpackSnorm2x16\0" // IR(46787, 16)
  "UnpackUnorm2x16\0" // IR(46803, 16)
  "UnpackHalf2x16\0" // IR(46819, 15)
  "UnpackSnorm4x8\0" // IR(46834, 15)
  "UnpackUnorm4x8\0" // IR(46849, 15)
  "UnpackDouble2x32\0" // IR(46864, 17)
  "Length\0" // IR(46881, 7)
  "Distance\0" // IR(46888, 9)
  "Cross\0" // IR(46897, 6)
  "Normalize\0" // IR(46903, 10)
  "FaceForward\0" // IR(46913, 12)
  "Reflect\0" // IR(46925, 8)
  "Refract\0" // IR(46933, 8)
  "FindILsb\0" // IR(46941, 9)
  "FindSMsb\0" // IR(46950, 9)
  "FindUMsb\0" // IR(46959, 9)
  "InterpolateAtCentroid\0" // IR(46968, 22)
  "InterpolateAtSample\0" // IR(46990, 20)
  "InterpolateAtOffset\0" // IR(47010, 20)
  "NMin\0" // IR(47030, 5)
  "NMax\0" // IR(47035, 5)
  "NClamp\0" // IR(47040, 7)
  "ArgumentInfo\0" // IR(47047, 13)
  "ArgumentStorageBuffer\0" // IR(47060, 22)
  "ArgumentUniform\0" // IR(47082, 16)
  "ArgumentPodStorageBuffer\0" // IR(47098, 25)
  "ArgumentPodUniform\0" // IR(47123, 19)
  "ArgumentPodPushConstant\0" // IR(47142, 24)
  "ArgumentSampledImage\0" // IR(47166, 21)
  "ArgumentStorageImage\0" // IR(47187, 21)
  "ArgumentSampler\0" // IR(47208, 16)
  "ArgumentWorkgroup\0" // IR(47224, 18)
  "SpecConstantWorkgroupSize\0" // IR(47242, 26)
  "SpecConstantGlobalOffset\0" // IR(47268, 25)
  "SpecConstantWorkDim\0" // IR(47293, 20)
  "PushConstantGlobalOffset\0" // IR(47313, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(47338, 30)
  "PushConstantGlobalSize\0" // IR(47368, 23)
  "PushConstantRegionOffset\0" // IR(47391, 25)
  "PushConstantNumWorkgroups\0" // IR(47416, 26)
  "PushConstantRegionGroupOffset\0" // IR(47442, 30)
  "ConstantDataStorageBuffer\0" // IR(47472, 26)
  "ConstantDataUniform\0" // IR(47498, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(47518, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(47548, 28)
  "ArgumentPointerPushConstant\0" // IR(47576, 28)
  "ArgumentPointerUniform\0" // IR(47604, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(47627, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(47662, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(47700, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(47742, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(47787, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(47824, 40)
  "ArgumentStorageTexelBuffer\0" // IR(47864, 27)
  "ArgumentUniformTexelBuffer\0" // IR(47891, 27)
  "ConstantDataPointerPushConstant\0" // IR(47918, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(47950, 40)
  "PrintfInfo\0" // IR(47990, 11)
  "PrintfBufferStorageBuffer\0" // IR(48001, 26)
  "PrintfBufferPointerPushConstant\0" // IR(48027, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(48059, 34)
  "WorkgroupVariableSize\0" // IR(48093, 22)
  "DebugImportedEntity\0" // IR(48115, 20)
  "DebugSource\0" // IR(48135, 12)
  "DebugFunctionDefinition\0" // IR(48147, 24)
  "DebugSourceContinued\0" // IR(48171, 21)
  "DebugLine\0" // IR(48192, 10)
  "DebugNoLine\0" // IR(48202, 12)
  "DebugBuildIdentifier\0" // IR(48214, 21)
  "DebugStoragePath\0" // IR(48235, 17)
  "DebugEntryPoint\0" // IR(48252, 16)
  "DebugTypeMatrix\0" // IR(48268, 16)
  "Configuration\0" // IR(48284, 14)
  "StartCounter\0" // IR(48298, 13)
  "StopCounter\0" // IR(48311, 12)
  "PushConstants\0" // IR(48323, 14)
  "SpecializationMapEntry\0" // IR(48337, 23)
  "DescriptorSetBuffer\0" // IR(48360, 20)
  "DescriptorSetImage\0" // IR(48380, 19)
  "DescriptorSetSampler\0" // IR(48399, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(48420, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(48483, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(48528, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(48577, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(48626, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(48670, 50)
  "DebugModuleINTEL\0" // IR(48720, 17)
  "acos\0" // IR(48737, 5)
  "acosh\0" // IR(48742, 6)
  "acospi\0" // IR(48748, 7)
  "asin\0" // IR(48755, 5)
  "asinh\0" // IR(48760, 6)
  "asinpi\0" // IR(48766, 7)
  "atan\0" // IR(48773, 5)
  "atan2\0" // IR(48778, 6)
  "atanh\0" // IR(48784, 6)
  "atanpi\0" // IR(48790, 7)
  "atan2pi\0" // IR(48797, 8)
  "cbrt\0" // IR(48805, 5)
  "ceil\0" // IR(48810, 5)
  "copysign\0" // IR(48815, 9)
  "cos\0" // IR(48824, 4)
  "cosh\0" // IR(48828, 5)
  "cospi\0" // IR(48833, 6)
  "erfc\0" // IR(48839, 5)
  "erf\0" // IR(48844, 4)
  "exp\0" // IR(48848, 4)
  "exp2\0" // IR(48852, 5)
  "exp10\0" // IR(48857, 6)
  "expm1\0" // IR(48863, 6)
  "fabs\0" // IR(48869, 5)
  "fdim\0" // IR(48874, 5)
  "floor\0" // IR(48879, 6)
  "fma\0" // IR(48885, 4)
  "fmax\0" // IR(48889, 5)
  "fmin\0" // IR(48894, 5)
  "fmod\0" // IR(48899, 5)
  "fract\0" // IR(48904, 6)
  "frexp\0" // IR(48910, 6)
  "hypot\0" // IR(48916, 6)
  "ilogb\0" // IR(48922, 6)
  "ldexp\0" // IR(48928, 6)
  "lgamma\0" // IR(48934, 7)
  "lgamma_r\0" // IR(48941, 9)
  "log\0" // IR(48950, 4)
  "log2\0" // IR(48954, 5)
  "log10\0" // IR(48959, 6)
  "log1p\0" // IR(48965, 6)
  "logb\0" // IR(48971, 5)
  "mad\0" // IR(48976, 4)
  "maxmag\0" // IR(48980, 7)
  "minmag\0" // IR(48987, 7)
  "modf\0" // IR(48994, 5)
  "nan\0" // IR(48999, 4)
  "nextafter\0" // IR(49003, 10)
  "pow\0" // IR(49013, 4)
  "pown\0" // IR(49017, 5)
  "powr\0" // IR(49022, 5)
  "remainder\0" // IR(49027, 10)
  "remquo\0" // IR(49037, 7)
  "rint\0" // IR(49044, 5)
  "rootn\0" // IR(49049, 6)
  "round\0" // IR(49055, 6)
  "rsqrt\0" // IR(49061, 6)
  "sin\0" // IR(49067, 4)
  "sincos\0" // IR(49071, 7)
  "sinh\0" // IR(49078, 5)
  "sinpi\0" // IR(49083, 6)
  "sqrt\0" // IR(49089, 5)
  "tan\0" // IR(49094, 4)
  "tanh\0" // IR(49098, 5)
  "tanpi\0" // IR(49103, 6)
  "tgamma\0" // IR(49109, 7)
  "trunc\0" // IR(49116, 6)
  "half_cos\0" // IR(49122, 9)
  "half_divide\0" // IR(49131, 12)
  "half_exp\0" // IR(49143, 9)
  "half_exp2\0" // IR(49152, 10)
  "half_exp10\0" // IR(49162, 11)
  "half_log\0" // IR(49173, 9)
  "half_log2\0" // IR(49182, 10)
  "half_log10\0" // IR(49192, 11)
  "half_powr\0" // IR(49203, 10)
  "half_recip\0" // IR(49213, 11)
  "half_rsqrt\0" // IR(49224, 11)
  "half_sin\0" // IR(49235, 9)
  "half_sqrt\0" // IR(49244, 10)
  "half_tan\0" // IR(49254, 9)
  "native_cos\0" // IR(49263, 11)
  "native_divide\0" // IR(49274, 14)
  "native_exp\0" // IR(49288, 11)
  "native_exp2\0" // IR(49299, 12)
  "native_exp10\0" // IR(49311, 13)
  "native_log\0" // IR(49324, 11)
  "native_log2\0" // IR(49335, 12)
  "native_log10\0" // IR(49347, 13)
  "native_powr\0" // IR(49360, 12)
  "native_recip\0" // IR(49372, 13)
  "native_rsqrt\0" // IR(49385, 13)
  "native_sin\0" // IR(49398, 11)
  "native_sqrt\0" // IR(49409, 12)
  "native_tan\0" // IR(49421, 11)
  "fclamp\0" // IR(49432, 7)
  "degrees\0" // IR(49439, 8)
  "fmax_common\0" // IR(49447, 12)
  "fmin_common\0" // IR(49459, 12)
  "mix\0" // IR(49471, 4)
  "radians\0" // IR(49475, 8)
  "step\0" // IR(49483, 5)
  "smoothstep\0" // IR(49488, 11)
  "sign\0" // IR(49499, 5)
  "cross\0" // IR(49504, 6)
  "distance\0" // IR(49510, 9)
  "length\0" // IR(49519, 7)
  "normalize\0" // IR(49526, 10)
  "fast_distance\0" // IR(49536, 14)
  "fast_length\0" // IR(49550, 12)
  "fast_normalize\0" // IR(49562, 15)
  "s_abs\0" // IR(49577, 6)
  "s_abs_diff\0" // IR(49583, 11)
  "s_add_sat\0" // IR(49594, 10)
  "u_add_sat\0" // IR(49604, 10)
  "s_hadd\0" // IR(49614, 7)
  "u_hadd\0" // IR(49621, 7)
  "s_rhadd\0" // IR(49628, 8)
  "u_rhadd\0" // IR(49636, 8)
  "s_clamp\0" // IR(49644, 8)
  "u_clamp\0" // IR(49652, 8)
  "clz\0" // IR(49660, 4)
  "ctz\0" // IR(49664, 4)
  "s_mad_hi\0" // IR(49668, 9)
  "u_mad_sat\0" // IR(49677, 10)
  "s_mad_sat\0" // IR(49687, 10)
  "s_max\0" // IR(49697, 6)
  "u_max\0" // IR(49703, 6)
  "s_min\0" // IR(49709, 6)
  "u_min\0" // IR(49715, 6)
  "s_mul_hi\0" // IR(49721, 9)
  "rotate\0" // IR(49730, 7)
  "s_sub_sat\0" // IR(49737, 10)
  "u_sub_sat\0" // IR(49747, 10)
  "u_upsample\0" // IR(49757, 11)
  "s_upsample\0" // IR(49768, 11)
  "popcount\0" // IR(49779, 9)
  "s_mad24\0" // IR(49788, 8)
  "u_mad24\0" // IR(49796, 8)
  "s_mul24\0" // IR(49804, 8)
  "u_mul24\0" // IR(49812, 8)
  "vloadn\0" // IR(49820, 7)
  "vstoren\0" // IR(49827, 8)
  "vload_half\0" // IR(49835, 11)
  "vload_halfn\0" // IR(49846, 12)
  "vstore_half\0" // IR(49858, 12)
  "vstore_half_r\0" // IR(49870, 14)
  "vstore_halfn\0" // IR(49884, 13)
  "vstore_halfn_r\0" // IR(49897, 15)
  "vloada_halfn\0" // IR(49912, 13)
  "vstorea_halfn\0" // IR(49925, 14)
  "vstorea_halfn_r\0" // IR(49939, 16)
  "shuffle\0" // IR(49955, 8)
  "shuffle2\0" // IR(49963, 9)
  "printf\0" // IR(49972, 7)
  "prefetch\0" // IR(49979, 9)
  "bitselect\0" // IR(49988, 10)
  "select\0" // IR(49998, 7)
  "u_abs\0" // IR(50005, 6)
  "u_abs_diff\0" // IR(50011, 11)
  "u_mul_hi\0" // IR(50022, 9)
  "u_mad_hi\0" // IR(50031, 9)
  "CubeFaceIndexAMD\0" // IR(50040, 17)
  "CubeFaceCoordAMD\0" // IR(50057, 17)
  "TimeAMD\0" // IR(50074, 8)
  "SwizzleInvocationsAMD\0" // IR(50082, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(50104, 28)
  "WriteInvocationAMD\0" // IR(50132, 19)
  "MbcntAMD\0" // IR(50151, 9)
  "InterpolateAtVertexAMD\0" // IR(50160, 23)
  "FMin3AMD\0" // IR(50183, 9)
  "UMin3AMD\0" // IR(50192, 9)
  "SMin3AMD\0" // IR(50201, 9)
  "FMax3AMD\0" // IR(50210, 9)
  "UMax3AMD\0" // IR(50219, 9)
  "SMax3AMD\0" // IR(50228, 9)
  "FMid3AMD\0" // IR(50237, 9)
  "UMid3AMD\0" // IR(50246, 9)
  "SMid3AMD\0" // IR(50255, 9)
  "ARGMAX\0" // IR(50264, 7)
  "AVG_POOL2D\0" // IR(50271, 11)
  "CONV2D\0" // IR(50282, 7)
  "CONV3D\0" // IR(50289, 7)
  "DEPTHWISE_CONV2D\0" // IR(50296, 17)
  "FFT2D\0" // IR(50313, 6)
  "MATMUL\0" // IR(50319, 7)
  "MAX_POOL2D\0" // IR(50326, 11)
  "RFFT2D\0" // IR(50337, 7)
  "TRANSPOSE_CONV2D\0" // IR(50344, 17)
  "CLAMP\0" // IR(50361, 6)
  "ERF\0" // IR(50367, 4)
  "SIGMOID\0" // IR(50371, 8)
  "TANH\0" // IR(50379, 5)
  "ADD\0" // IR(50384, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(50388, 23)
  "BITWISE_AND\0" // IR(50411, 12)
  "BITWISE_OR\0" // IR(50423, 11)
  "BITWISE_XOR\0" // IR(50434, 12)
  "INTDIV\0" // IR(50446, 7)
  "LOGICAL_AND\0" // IR(50453, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(50465, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(50484, 20)
  "LOGICAL_OR\0" // IR(50504, 11)
  "LOGICAL_XOR\0" // IR(50515, 12)
  "MAXIMUM\0" // IR(50527, 8)
  "MINIMUM\0" // IR(50535, 8)
  "MUL\0" // IR(50543, 4)
  "POW\0" // IR(50547, 4)
  "SUB\0" // IR(50551, 4)
  "TABLE\0" // IR(50555, 6)
  "ABS\0" // IR(50561, 4)
  "BITWISE_NOT\0" // IR(50565, 12)
  "CEIL\0" // IR(50577, 5)
  "CLZ\0" // IR(50582, 4)
  "COS\0" // IR(50586, 4)
  "EXP\0" // IR(50590, 4)
  "FLOOR\0" // IR(50594, 6)
  "LOG\0" // IR(50600, 4)
  "LOGICAL_NOT\0" // IR(50604, 12)
  "NEGATE\0" // IR(50616, 7)
  "RECIPROCAL\0" // IR(50623, 11)
  "RSQRT\0" // IR(50634, 6)
  "SIN\0" // IR(50640, 4)
  "SELECT\0" // IR(50644, 7)
  "EQUAL\0" // IR(50651, 6)
  "GREATER\0" // IR(50657, 8)
  "GREATER_EQUAL\0" // IR(50665, 14)
  "REDUCE_ALL\0" // IR(50679, 11)
  "REDUCE_ANY\0" // IR(50690, 11)
  "REDUCE_MAX\0" // IR(50701, 11)
  "REDUCE_MIN\0" // IR(50712, 11)
  "REDUCE_PRODUCT\0" // IR(50723, 15)
  "REDUCE_SUM\0" // IR(50738, 11)
  "CONCAT\0" // IR(50749, 7)
  "PAD\0" // IR(50756, 4)
  "RESHAPE\0" // IR(50760, 8)
  "REVERSE\0" // IR(50768, 8)
  "SLICE\0" // IR(50776, 6)
  "TILE\0" // IR(50782, 5)
  "TRANSPOSE\0" // IR(50787, 10)
  "GATHER\0" // IR(50797, 7)
  "SCATTER\0" // IR(50804, 8)
  "RESIZE\0" // IR(50812, 7)
  "CAST\0" // IR(50819, 5)
  "RESCALE\0" // IR(50824, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(4721, 27), // 0 PhysicalStorageBuffer64EXT
  IR(5632, 18), // 1 SubgroupEqMaskKHR
  IR(5687, 18), // 2 SubgroupGeMaskKHR
  IR(5720, 18), // 3 SubgroupGtMaskKHR
  IR(5753, 18), // 4 SubgroupLeMaskKHR
  IR(5786, 18), // 5 SubgroupLtMaskKHR
  IR(6644, 12), // 6 BaryCoordNV
  IR(6699, 19), // 7 BaryCoordNoPerspNV
  IR(6730, 15), // 8 FragmentSizeNV
  IR(6787, 22), // 9 InvocationsPerPixelNV
  IR(6915, 11), // 10 LaunchIdNV
  IR(6940, 13), // 11 LaunchSizeNV
  IR(6971, 17), // 12 WorldRayOriginNV
  IR(7009, 20), // 13 WorldRayDirectionNV
  IR(7048, 18), // 14 ObjectRayOriginNV
  IR(7088, 21), // 15 ObjectRayDirectionNV
  IR(7120, 10), // 16 RayTminNV
  IR(7141, 10), // 17 RayTmaxNV
  IR(7174, 22), // 18 InstanceCustomIndexNV
  IR(7213, 16), // 19 ObjectToWorldNV
  IR(7246, 16), // 20 WorldToObjectNV
  IR(7280, 10), // 21 HitKindNV
  IR(7511, 19), // 22 IncomingRayFlagsNV
  IR(9287, 28), // 23 StorageUniformBufferBlock16
  IR(9350, 17), // 24 StorageUniform16
  IR(10165, 27), // 25 ShaderViewportIndexLayerNV
  IR(10209, 22), // 26 FragmentBarycentricNV
  IR(10262, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(10292, 14), // 28 ShadingRateNV
  IR(10352, 20), // 29 ShaderNonUniformEXT
  IR(10395, 26), // 30 RuntimeDescriptorArrayEXT
  IR(10457, 39), // 31 InputAttachmentArrayDynamicIndexingEXT
  IR(10535, 42), // 32 UniformTexelBufferArrayDynamicIndexingEXT
  IR(10616, 42), // 33 StorageTexelBufferArrayDynamicIndexingEXT
  IR(10695, 40), // 34 UniformBufferArrayNonUniformIndexingEXT
  IR(10771, 39), // 35 SampledImageArrayNonUniformIndexingEXT
  IR(10847, 40), // 36 StorageBufferArrayNonUniformIndexingEXT
  IR(10923, 39), // 37 StorageImageArrayNonUniformIndexingEXT
  IR(11001, 42), // 38 InputAttachmentArrayNonUniformIndexingEXT
  IR(11085, 45), // 39 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(11172, 45), // 40 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(11235, 21), // 41 VulkanMemoryModelKHR
  IR(11285, 32), // 42 VulkanMemoryModelDeviceScopeKHR
  IR(11317, 34), // 43 PhysicalStorageBufferAddressesEXT
  IR(11383, 31), // 44 ComputeDerivativeGroupLinearNV
  IR(11587, 28), // 45 DemoteToHelperInvocationEXT
  IR(13057, 22), // 46 DotProductInputAllKHR
  IR(13101, 25), // 47 DotProductInput4x8BitKHR
  IR(13154, 31), // 48 DotProductInput4x8BitPackedKHR
  IR(13196, 14), // 49 DotProductKHR
  IR(13361, 13), // 50 OptNoneINTEL
  IR(16400, 15), // 51 PerPrimitiveNV
  IR(16448, 12), // 52 PerVertexNV
  IR(16471, 14), // 53 NonUniformEXT
  IR(16501, 19), // 54 RestrictPointerEXT
  IR(16535, 18), // 55 AliasedPointerEXT
  IR(16845, 24), // 56 HlslCounterBufferGOOGLE
  IR(16882, 19), // 57 HlslSemanticGOOGLE
  IR(19573, 14), // 58 OutputLinesNV
  IR(19607, 19), // 59 OutputPrimitivesNV
  IR(19650, 23), // 60 DerivativeGroupQuadsNV
  IR(19698, 24), // 61 DerivativeGroupLinearNV
  IR(19741, 18), // 62 OutputTrianglesNV
  IR(20489, 16), // 63 RayGenerationNV
  IR(20521, 15), // 64 IntersectionNV
  IR(20546, 9), // 65 AnyHitNV
  IR(20569, 13), // 66 ClosestHitNV
  IR(20590, 7), // 67 MissNV
  IR(20609, 11), // 68 CallableNV
  IR(20754, 23), // 69 AllowContractFastINTEL
  IR(20790, 18), // 70 AllowReassocINTEL
  IR(21244, 22), // 71 MakeTexelAvailableKHR
  IR(21283, 20), // 72 MakeTexelVisibleKHR
  IR(21319, 19), // 73 NonPrivateTexelKHR
  IR(21352, 17), // 74 VolatileTexelKHR
  IR(22729, 24), // 75 MakePointerAvailableKHR
  IR(22772, 22), // 76 MakePointerVisibleKHR
  IR(22812, 21), // 77 NonPrivatePointerKHR
  IR(22899, 10), // 78 VulkanKHR
  IR(14172, 5), // 79 None
  IR(23082, 16), // 80 OutputMemoryKHR
  IR(23112, 17), // 81 MakeAvailableKHR
  IR(23141, 15), // 82 MakeVisibleKHR
  IR(23217, 28), // 83 PackedVectorFormat4x8BitKHR
  IR(23532, 24), // 84 SkipBuiltinPrimitivesNV
  IR(24272, 15), // 85 QueueFamilyKHR
  IR(24643, 15), // 86 CallableDataNV
  IR(24682, 23), // 87 IncomingCallableDataNV
  IR(24719, 13), // 88 RayPayloadNV
  IR(24748, 15), // 89 HitAttributeNV
  IR(24785, 21), // 90 IncomingRayPayloadNV
  IR(24828, 21), // 91 ShaderRecordBufferNV
  IR(24871, 25), // 92 PhysicalStorageBufferEXT
  IR(32205, 8), // 93 SDotKHR
  IR(32218, 8), // 94 UDotKHR
  IR(32232, 9), // 95 SUDotKHR
  IR(32252, 14), // 96 SDotAccSatKHR
  IR(32277, 14), // 97 UDotAccSatKHR
  IR(32303, 15), // 98 SUDotAccSatKHR
  IR(34883, 21), // 99 ReportIntersectionNV
  IR(35058, 28), // 100 TypeAccelerationStructureNV
  IR(37107, 21), // 101 DecorateStringGOOGLE
  IR(37149, 27), // 102 MemberDecorateStringGOOGLE
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
  spv::Capability::SpecConditionalINTEL, // 109
  spv::Capability::Shader, // 110
  spv::Capability::Kernel, // 111
  spv::Capability::Shader, // 112
  spv::Capability::UniformDecoration, // 113
  spv::Capability::GeometryStreams, // 114
  spv::Capability::TransformFeedback, // 115
  spv::Capability::Kernel, // 116
  spv::Capability::FloatControls2, // 117
  spv::Capability::Linkage, // 118
  spv::Capability::Float8EXT, // 119
  spv::Capability::SampleMaskOverrideCoverageNV, // 120
  spv::Capability::GeometryShaderPassthroughNV, // 121
  spv::Capability::MeshShadingNV, // 122
  spv::Capability::MeshShadingEXT, // 123
  spv::Capability::ShaderInvocationReorderNV, // 124
  spv::Capability::BindlessTextureNV, // 125
  spv::Capability::VectorComputeINTEL, // 126
  spv::Capability::IndirectReferencesINTEL, // 127
  spv::Capability::AsmINTEL, // 128
  spv::Capability::FunctionFloatControlINTEL, // 129
  spv::Capability::FPGAMemoryAttributesINTEL, // 130
  spv::Capability::FPGAMemoryAccessesINTEL, // 131
  spv::Capability::LoopFuseINTEL, // 132
  spv::Capability::FPGADSPControlINTEL, // 133
  spv::Capability::MemoryAccessAliasingINTEL, // 134
  spv::Capability::FPGAInvocationPipeliningAttributesINTEL, // 135
  spv::Capability::FPGABufferLocationINTEL, // 136
  spv::Capability::IOPipesINTEL, // 137
  spv::Capability::FPGAClusterAttributesV2INTEL, // 138
  spv::Capability::FPMaxErrorINTEL, // 139
  spv::Capability::FPGALatencyControlINTEL, // 140
  spv::Capability::FPGAArgumentInterfacesINTEL, // 141
  spv::Capability::GlobalVariableHostAccessINTEL, // 142
  spv::Capability::GlobalVariableFPGADecorationsINTEL, // 143
  spv::Capability::CacheControlsINTEL, // 144
  spv::Capability::TileImageColorReadAccessEXT, // 145
  spv::Capability::Geometry, // 146
  spv::Capability::Tessellation, // 147
  spv::Capability::MeshShadingNV, // 148
  spv::Capability::MeshShadingEXT, // 149
  spv::Capability::Geometry, // 150
  spv::Capability::MeshShadingNV, // 151
  spv::Capability::MeshShadingEXT, // 152
  spv::Capability::SubgroupDispatch, // 153
  spv::Capability::TileImageDepthReadAccessEXT, // 154
  spv::Capability::TileImageStencilReadAccessEXT, // 155
  spv::Capability::SampleMaskPostDepthCoverage, // 156
  spv::Capability::DenormPreserve, // 157
  spv::Capability::DenormFlushToZero, // 158
  spv::Capability::SignedZeroInfNanPreserve, // 159
  spv::Capability::RoundingModeRTE, // 160
  spv::Capability::RoundingModeRTZ, // 161
  spv::Capability::QuadControlKHR, // 162
  spv::Capability::ComputeDerivativeGroupQuadsKHR, // 163
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
  spv::Capability::ReplicatedCompositesEXT, // 239
  spv::Capability::TextureSampleWeightedQCOM, // 240
  spv::Capability::TextureBoxFilterQCOM, // 241
  spv::Capability::TextureBlockMatchQCOM, // 242
  spv::Capability::CooperativeMatrixConversionQCOM, // 243
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
  spv::Capability::FunctionVariantsINTEL, // 301
  spv::Capability::GroupUniformArithmeticKHR, // 302
  spv::Capability::TensorFloat32RoundingINTEL, // 303
  spv::Capability::MaskedGatherScatterINTEL, // 304
  spv::Capability::BindlessImagesINTEL, // 305
  spv::Capability::Float64, // 306
  spv::Capability::InterpolationFunction, // 307
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
  spvtools::Extension::kSPV_NV_cooperative_matrix2, // 89
  spvtools::Extension::kSPV_NV_tensor_addressing, // 90
  spvtools::Extension::kSPV_INTEL_subgroups, // 91
  spvtools::Extension::kSPV_INTEL_media_block_io, // 92
  spvtools::Extension::kSPV_INTEL_float_controls2, // 93
  spvtools::Extension::kSPV_INTEL_shader_integer_functions2, // 94
  spvtools::Extension::kSPV_INTEL_function_pointers, // 95
  spvtools::Extension::kSPV_INTEL_inline_assembly, // 96
  spvtools::Extension::kSPV_EXT_shader_atomic_float_min_max, // 97
  spvtools::Extension::kSPV_INTEL_vector_compute, // 98
  spvtools::Extension::kSPV_KHR_expect_assume, // 99
  spvtools::Extension::kSPV_INTEL_device_side_avc_motion_estimation, // 100
  spvtools::Extension::kSPV_INTEL_variable_length_array, // 101
  spvtools::Extension::kSPV_INTEL_fpga_memory_attributes, // 102
  spvtools::Extension::kSPV_INTEL_fp_fast_math_mode, // 103
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_integers, // 104
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_floating_point, // 105
  spvtools::Extension::kSPV_INTEL_unstructured_loop_controls, // 106
  spvtools::Extension::kSPV_INTEL_fpga_loop_controls, // 107
  spvtools::Extension::kSPV_INTEL_kernel_attributes, // 108
  spvtools::Extension::kSPV_INTEL_fpga_memory_accesses, // 109
  spvtools::Extension::kSPV_INTEL_fpga_cluster_attributes, // 110
  spvtools::Extension::kSPV_INTEL_loop_fuse, // 111
  spvtools::Extension::kSPV_INTEL_fpga_dsp_control, // 112
  spvtools::Extension::kSPV_INTEL_memory_access_aliasing, // 113
  spvtools::Extension::kSPV_INTEL_fpga_invocation_pipelining_attributes, // 114
  spvtools::Extension::kSPV_INTEL_fpga_buffer_location, // 115
  spvtools::Extension::kSPV_INTEL_arbitrary_precision_fixed_point, // 116
  spvtools::Extension::kSPV_INTEL_usm_storage_classes, // 117
  spvtools::Extension::kSPV_INTEL_runtime_aligned, // 118
  spvtools::Extension::kSPV_INTEL_io_pipes, // 119
  spvtools::Extension::kSPV_INTEL_blocking_pipes, // 120
  spvtools::Extension::kSPV_INTEL_fpga_reg, // 121
  spvtools::Extension::kSPV_KHR_integer_dot_product, // 122
  spvtools::Extension::kSPV_KHR_cooperative_matrix, // 123
  spvtools::Extension::kSPV_EXT_replicated_composites, // 124
  spvtools::Extension::kSPV_KHR_bit_instructions, // 125
  spvtools::Extension::kSPV_KHR_subgroup_rotate, // 126
  spvtools::Extension::kSPV_KHR_float_controls2, // 127
  spvtools::Extension::kSPV_EXT_shader_atomic_float_add, // 128
  spvtools::Extension::kSPV_INTEL_long_composites, // 129
  spvtools::Extension::kSPV_EXT_optnone, // 130
  spvtools::Extension::kSPV_INTEL_optnone, // 131
  spvtools::Extension::kSPV_EXT_shader_atomic_float16_add, // 132
  spvtools::Extension::kSPV_INTEL_debug_module, // 133
  spvtools::Extension::kSPV_INTEL_bfloat16_conversion, // 134
  spvtools::Extension::kSPV_INTEL_split_barrier, // 135
  spvtools::Extension::kSPV_EXT_arithmetic_fence, // 136
  spvtools::Extension::kSPV_INTEL_task_sequence, // 137
  spvtools::Extension::kSPV_INTEL_fp_max_error, // 138
  spvtools::Extension::kSPV_INTEL_fpga_latency_control, // 139
  spvtools::Extension::kSPV_INTEL_fpga_argument_interfaces, // 140
  spvtools::Extension::kSPV_INTEL_global_variable_host_access, // 141
  spvtools::Extension::kSPV_INTEL_global_variable_fpga_decorations, // 142
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 143
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 144
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 145
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 146
  spvtools::Extension::kSPV_INTEL_function_variants, // 147
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 148
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 149
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 150
  spvtools::Extension::kSPV_INTEL_cache_controls, // 151
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 152
  spvtools::Extension::kSPV_INTEL_bindless_images, // 153
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 154
  spvtools::Extension::kSPV_NV_mesh_shader, // 155
  spvtools::Extension::kSPV_EXT_mesh_shader, // 156
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 157
  spvtools::Extension::kSPV_GOOGLE_user_type, // 158
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 159
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 160
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 161
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 162
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 163
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 164
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 165
  spvtools::Extension::kSPV_KHR_variable_pointers, // 166
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 167
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 168
  spvtools::Extension::kSPV_KHR_ray_tracing, // 169
  spvtools::Extension::kSPV_KHR_ray_query, // 170
  spvtools::Extension::kSPV_NV_ray_tracing, // 171
  spvtools::Extension::kSPV_KHR_ray_tracing, // 172
  spvtools::Extension::kSPV_KHR_ray_query, // 173
  spvtools::Extension::kSPV_NV_displacement_micromap, // 174
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 175
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 176
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
